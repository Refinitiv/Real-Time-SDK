/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022,2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.io.IOException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.spi.SelectorProvider;
import java.util.Iterator;
import java.util.Set;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InProgFlags;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.common.SelectableBiDirectionalQueue;
import com.refinitiv.eta.valueadd.common.VaIteratableQueue;
import com.refinitiv.eta.valueadd.reactor.ReactorAuthTokenInfo.TokenVersion;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel.State;
import com.refinitiv.eta.valueadd.reactor.ReactorTokenSession.SessionState;

/* Internal Worker thread class. */
class Worker implements Runnable
{
    int SELECT_TIME = 100;

    SelectableBiDirectionalQueue _queue = null;
    Selector _selector = null;
    ReactorChannel _workerReactorChannel = null; // The Worker's reactorChannel.
    ReactorChannel _reactorReactorChannel = null; // The Reactor's
    // reactorChannel.
    com.refinitiv.eta.transport.Error _error = TransportFactory.createError();
    com.refinitiv.eta.transport.InProgInfo _inProg = TransportFactory.createInProgInfo();

    VaIteratableQueue _initChannelQueue = new VaIteratableQueue();
    VaIteratableQueue _activeChannelQueue = new VaIteratableQueue();
    VaIteratableQueue _reconnectingChannelQueue = new VaIteratableQueue();

    volatile boolean _running = true;

    VaIteratableQueue _timerEventQueue = new VaIteratableQueue();

    Reactor _reactor;

    Worker(ReactorChannel reactorChannel, SelectableBiDirectionalQueue queue)
    {
        if (reactorChannel == null)
            throw new UnsupportedOperationException("reactorChannel cannot be null");
        else if (queue == null)
            throw new UnsupportedOperationException("queue cannot be null");

        _reactorReactorChannel = reactorChannel;
        _queue = queue;
        _reactor = reactorChannel.reactor();
    }

    @Override
    public void run()
    {
        if (initializeWorker() != ReactorReturnCodes.SUCCESS)
        {
            System.out.println("Worker.run(): failed to initialize, shutting down");
            shutdown();
            return;
        }

        while (_running || _queue.readQueueSize() > 0)
        {
            try
            {
                int selectorCount = _selector.select(SELECT_TIME);
                if (selectorCount > 0 || !_selector.selectedKeys().isEmpty())
                {
                    Iterator<SelectionKey> iter = _selector.selectedKeys().iterator();
                    while (iter.hasNext())
                    {
                        SelectionKey key = iter.next();
                        iter.remove();
                        if (!key.isValid())
                            continue;
                        if (key.isConnectable())
                        {
                            ReactorChannel reactorChannel = (ReactorChannel)key.attachment();
                            // this is an extra measure because on the Solaris OS initial notification is different
                            if (reactorChannel.channel() != null &&
                                (reactorChannel.channel().state() == ChannelState.INACTIVE || reactorChannel.channel().state() == ChannelState.INITIALIZING))
                            {
                                initializeChannel(reactorChannel);
                            }
                            if (!key.isValid())
                                continue;
                        }
                        if (key.isReadable())
                        {
                            ReactorChannel reactorChannel = (ReactorChannel)key.attachment();
                            if (_workerReactorChannel == reactorChannel)
                            {
                                processWorkerEvent();
                            }
                            else
                            {
                                // this is an extra measure because on the Solaris OS initial notification is different
                                if (reactorChannel.channel() != null &&
                                    (reactorChannel.channel().state() == ChannelState.INACTIVE || reactorChannel.channel().state() == ChannelState.INITIALIZING))
                                {
                                    initializeChannel(reactorChannel);
                                }
                                if (!key.isValid())
                                    continue;
                            }

                        }
                        if (key.isWritable())
                        {
                            processChannelFlush((ReactorChannel)key.attachment());
                        }
                    }
                }

                if (Thread.currentThread().isInterrupted())
                {
                    _running = false;
                }

                // check guaranteed messaging timers
                _timerEventQueue.rewind();
                while (_timerEventQueue.hasNext())
                {
                    WorkerEvent event = (WorkerEvent)_timerEventQueue.next();
                    if (System.nanoTime() >= event.timeout())
                    {
                        if (event.eventType() == WorkerEventTypes.TOKEN_MGNT)
                        {
                        	ReactorTokenSession tokenSession = event._tokenSession;
                        	if(tokenSession != null && (tokenSession.authTokenInfo().tokenVersion() != TokenVersion.V2 || (event._reactorChannel != null && event._reactorChannel.state() != State.READY && event._reactorChannel.state() != State.UP)))
                        	{
                        		tokenSession.handleTokenReissue();
                        	}
                        	
                        	/* Removes worker event from the token session */
                        	if(tokenSession != null)
                        	{
                        		tokenSession.tokenReissueEvent(null);
                        	}

                            _timerEventQueue.remove(event);
                            event.returnToPool();
                        }
                        else
                        {
                            WorkerEventTypes eventType = WorkerEventTypes.TUNNEL_STREAM_DISPATCH_TIMEOUT;
                            if (event.eventType() == WorkerEventTypes.START_WATCHLIST_TIMER)
                            {
                                eventType = WorkerEventTypes.WATCHLIST_TIMEOUT;
                            }

                            sendWorkerEvent(event.reactorChannel(), eventType, event.tunnelStream(),
                                    ReactorReturnCodes.SUCCESS, null, null);

                            _timerEventQueue.remove(event);
                            event.returnToPool();
                        }

                    }
                }

                // initialize channels and check if initialization timeout occurred
                _initChannelQueue.rewind();
                while (_initChannelQueue.hasNext())
                {
                    ReactorChannel reactorChannel = (ReactorChannel)_initChannelQueue.next();
                    // handle initialization timeout
                    if (reactorChannel.state() == ReactorChannel.State.INITIALIZING)
                    {
                        // this is an extra measure because on the Solaris OS initial notification is different
                        if (reactorChannel.channel() != null &&
                            (reactorChannel.channel().state() == ChannelState.INACTIVE || reactorChannel.channel().state() == ChannelState.INITIALIZING))
                        {
                            initializeChannel(reactorChannel);
                        }

                    }
                }

                // handle pings
                _activeChannelQueue.rewind();
                while (_activeChannelQueue.hasNext())
                {
                    ReactorChannel reactorChannel = (ReactorChannel)_activeChannelQueue.next();
                    if (reactorChannel != null)
                    {
                        if (reactorChannel.channel() != null && reactorChannel.channel().state() == ChannelState.ACTIVE
                            && reactorChannel.state() != ReactorChannel.State.DOWN_RECONNECTING
                            && reactorChannel.state() != ReactorChannel.State.DOWN
                            && reactorChannel.state() != ReactorChannel.State.CLOSED
                            && reactorChannel.state() != ReactorChannel.State.EDP_RT
                            && reactorChannel.state() != ReactorChannel.State.EDP_RT_DONE
                            && reactorChannel.state() != ReactorChannel.State.EDP_RT_FAILED )
                        {
                            if (reactorChannel.pingHandler().handlePings(reactorChannel, _error)
                                < TransportReturnCodes.SUCCESS)
                            {
                                reactorChannel.state(State.DOWN);
                                sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_DOWN,
                                        ReactorReturnCodes.FAILURE, "Worker.run()",
                                        "Ping error for channel: " + _error.text());
                            }
                        }
                    }
                }


                // handle connection recovery (only for client connections)
                _reconnectingChannelQueue.rewind();
                while (_reconnectingChannelQueue.hasNext())
                {
                    ReactorChannel reactorChannel = (ReactorChannel)_reconnectingChannelQueue.next();
                    if (reactorChannel != null)
                    {
                        if (reactorChannel.nextRecoveryTime() > System.currentTimeMillis())
                            continue;

                        Channel channel = null;
                        ReactorWarmStandbyServerInfo wsbServerImpl;

                        if(_reactor.reactorHandlesWarmStandby(reactorChannel))
                        {
                        	reactorChannel._reconnectAttempts++;
                        	ReactorWarmStandbyGroupImpl wsbGroup = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl();
                        	if(reactorChannel.isStartingServerConfig)
                        	{
                        		wsbServerImpl = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl().startingActiveServer();
                        		reactorChannel.setCurrentReactorConnectInfo(wsbGroup.startingActiveServer().reactorConnectInfo());
                        		reactorChannel.setCurrentConnectOptionsInfo(wsbGroup.startingConnectOptionsInfo);
                        		reactorChannel.userSpecObj(reactorChannel.getCurrentReactorConnectInfo().connectOptions().userSpecObject());
                        		
                        		reactorChannel.standByGroupListIndex = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex();
                        	
	                        	if((reactorChannel.warmStandByHandlerImpl.warmStandbyHandlerState() & 
	                        			ReactorWarmStandbyHandlerState.CLOSING_STANDBY_CHANNELS) != 0)
	                        	{
	                        		continue;
	                        	}
                        	}
                        	else
                        	{
                        		wsbServerImpl = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl().standbyServerList().get(reactorChannel.standByServerListIndex);
                        		reactorChannel.setCurrentReactorConnectInfo(wsbGroup.standbyServerList().get(reactorChannel.standByServerListIndex).reactorConnectInfo());
                        		reactorChannel.setCurrentConnectOptionsInfo(wsbGroup.standbyConnectOptionsInfoList.get(reactorChannel.standByServerListIndex));
                        		reactorChannel.userSpecObj(reactorChannel.getCurrentReactorConnectInfo().connectOptions().userSpecObject());
                        	}
                        	
                        	/* Channel has already been closed and cleaned up in the main thread, so we're just removing it from the worker's queues here */
                        	if(wsbServerImpl.isActiveChannelConfig() == false)
                        	{
                                _reconnectingChannelQueue.remove(reactorChannel);
                        	}

                        }
                        else
                        {   
                        	 if (reactorChannel.state() != State.EDP_RT &&
                                     reactorChannel.state() != State.EDP_RT_DONE &&
                                     reactorChannel.state() != State.EDP_RT_FAILED)
                             {
                        		 reactorChannel._reconnectAttempts++;
                        		 if (++reactorChannel._listIndex == reactorChannel.getReactorConnectOptions().connectionList().size())
                        		 {
                        			 reactorChannel. _listIndex = 0;
                        		 }
                        		 reactorChannel.setCurrentReactorConnectInfo(reactorChannel.getReactorConnectOptions().connectionList().get(reactorChannel._listIndex));
                        		 reactorChannel.setCurrentConnectOptionsInfo(reactorChannel._connectOptionsInfoList.get(reactorChannel._listIndex));
                            }
                        }


                        if (reactorChannel.state() != State.EDP_RT &&
                            reactorChannel.state() != State.EDP_RT_DONE &&
                            reactorChannel.state() != State.EDP_RT_FAILED)
                        {
                            channel = reactorChannel.reconnectReactorChannel(_error);
                        }

                        if (reactorChannel.state() == State.EDP_RT ||
                            reactorChannel.state() == State.EDP_RT_DONE ||
                            reactorChannel.state() == State.EDP_RT_FAILED)
                        {
                            channel = reactorChannel.reconnectEDP(_error);
                        }

                        if (channel == null && reactorChannel.state() != State.EDP_RT)
                        {
                            // Reconnect attempt failed -- send channel down event.
                            _reconnectingChannelQueue.remove(reactorChannel);
                            if(reactorChannel.tokenSession() != null && reactorChannel.tokenSession().sessionMgntState() == SessionState.STOP_TOKEN_REQUEST)
                            {
                            	/* This is a terminal state.  There was an REST error that we cannot recover from, so set the reconnectLimit to 0 */ 
                            	reactorChannel.getReactorConnectOptions().reconnectAttemptLimit(0);
                            }
                            
                            sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_DOWN,
                                    ReactorReturnCodes.FAILURE, "Worker.run()",
                                    "Reconnection failed: " + _error.text());
                            continue;
                        }

                        if (reactorChannel.state() != State.EDP_RT)
                        {
                            reactorChannel.selectableChannelFromChannel(channel);
                            reactorChannel.state(State.INITIALIZING);
                            _reconnectingChannelQueue.remove(reactorChannel);

                            processChannelInit(reactorChannel);
                        }
                    }
                }
            }
            catch (CancelledKeyException e)
            {
                // this could happen if the channel is closing while we're getting a notification
                continue;
            }
            catch (IOException e)
            {
                System.out.println("Worker.run() exception=" + e.getLocalizedMessage());
                sendWorkerEvent(_reactorReactorChannel, WorkerEventTypes.SHUTDOWN,
                        ReactorReturnCodes.FAILURE, "Worker.run",
                        "exception occurred, " + e.getLocalizedMessage());
                break;
            }
        }

        shutdown();
    }

    private void processWorkerEvent()
    {
        WorkerEvent event = (WorkerEvent)_queue.read();
        WorkerEventTypes eventType = event.eventType();
        ReactorChannel reactorChannel = event.reactorChannel();

        switch (eventType)
        {
            case CHANNEL_INIT:
                processChannelInit(reactorChannel);
                break;
            case CHANNEL_DOWN:
                processChannelClose(reactorChannel);
                if (reactorChannel.server() == null && !event.reactorChannel().recoveryAttemptLimitReached())
                {
                    /* Go into connection recovery. */
                    reactorChannel.calculateNextReconnectTime();
                    /* If we have connected with a V2 session management connection and wiled the access token, reset the session management state */
                    if(reactorChannel.tokenSession() != null && reactorChannel.tokenSession().authTokenInfo().tokenVersion() == TokenVersion.V2 && reactorChannel.tokenSession().hasAccessToken() == false)
                    {
                    	reactorChannel.tokenSession().resetSessionMgntState();
                    }
                    
                    _reconnectingChannelQueue.add(reactorChannel);
                }
                break;
            case CHANNEL_CLOSE:
                processChannelClose(reactorChannel);
                if(reactorChannel.warmStandByHandlerImpl != null)
                {
                	/* Remove the channel from any session management lists */
            		_reactor.removeReactorChannel(reactorChannel);

                }
                sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_CLOSE_ACK,
                        ReactorReturnCodes.SUCCESS, null, null);
                break;
            case SHUTDOWN:
                _running = false;
                break;
            case FLUSH:
                processChannelFlush(reactorChannel);
                break;
            case FD_CHANGE:
                processChannelFDChange(reactorChannel);
                break;
            case TOKEN_MGNT:
                // Setup a timer for token management
                ReactorTokenSession tokenSession = event._tokenSession;

                if(tokenSession.sessionMgntState() == SessionState.REQUEST_TOKEN_FAILURE)
                {
                    event.timeout(tokenSession.nextTokenReissueAttemptReqTime());
                }
                else if (tokenSession.sessionMgntState() == SessionState.AUTHENTICATE_USING_PASSWD_GRANT)
                {
                    event.timeout(System.nanoTime()); /* Sends a request to get an access token now */
                }
                else
                {
                    tokenSession.calculateNextAuthTokenRequestTime(tokenSession.authTokenInfo().expiresIn());
                    event.timeout(tokenSession.nextAuthTokenRequestTime());
                }
                
                if(tokenSession.authTokenInfo().tokenVersion() == TokenVersion.V1)
                {
	                /* Remove token session from the previous event and set a new event to the token session */
	                if(tokenSession.tokenReissueEvent() != null && tokenSession.tokenReissueEvent() != event)
	                {
	                	tokenSession.tokenReissueEvent()._tokenSession = null;
	                	tokenSession.tokenReissueEvent().timeout(System.nanoTime());
	                	tokenSession.tokenReissueEvent(event);
	                }
	                else
	                {
	                	tokenSession.tokenReissueEvent(event);
	                }
                }

                _timerEventQueue.add(event);
                return;
            case START_DISPATCH_TIMER:
            case START_WATCHLIST_TIMER:
                _timerEventQueue.add(event);
                return;

            default:
                System.out.println("Worker.processWorkerEvent(): received unexpected eventType=" + eventType);
                break;
        }

        event.returnToPool();
    }

    private void processChannelInit(ReactorChannel reactorChannel)
    {
        // add the reactorChannel to the init queue
        _initChannelQueue.add(reactorChannel);

        // register the channel with the selector
        try
        {
            if (reactorChannel.selectableChannel() != null)
            {
                reactorChannel.selectableChannel()
                        .register(_selector, SelectionKey.OP_CONNECT | SelectionKey.OP_READ, reactorChannel);
            }
        }
        catch (ClosedChannelException e)
        {
            // the channel was already closed. Send this as a failure.
            if (reactorChannel.state() != State.CLOSED)
                reactorChannel.state(State.CLOSED);

            sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_DOWN,
                    ReactorReturnCodes.FAILURE, "Worker.processChannelInit", "Exception="
                                                                             + e.getLocalizedMessage());
        }
    }

    private void processChannelClose(ReactorChannel reactorChannel)
    {
        if (reactorChannel == null)
            return;
        if (reactorChannel.channel() != null && reactorChannel.channel().state() != ChannelState.INACTIVE)
        {
            // sckt.close will implicitly cancel any registered keys.
            reactorChannel.channel().close(_error);
            reactorChannel.selectableChannelFromChannel(null);
            reactorChannel.flushRequested(false);
        }

        if (_activeChannelQueue.remove(reactorChannel) == false)
            if (_initChannelQueue.remove(reactorChannel) == false)
            {
                _reconnectingChannelQueue.remove(reactorChannel);
            }
    }

    private void processChannelFlush(ReactorChannel reactorChannel)
    {
        if (reactorChannel == null)
            return;
        Channel channel = reactorChannel.channel();
        if (channel != null && channel.state() != ChannelState.INACTIVE && channel.state() != ChannelState.CLOSED)
        {
            // attempt to flush
            int retval = channel.flush(_error);
            if (retval > TransportReturnCodes.SUCCESS)
            {
                // flush returned positive, register this channel with the
                // selector for OP_WRITE in order to flush later.
                if (!addSelectOption(reactorChannel, SelectionKey.OP_WRITE))
                {
                    // Add select option failed for this reactorChannel.
                    // Close this reactorChannel.
                    if (reactorChannel.state() != State.CLOSED && reactorChannel.state() != State.DOWN
                        && reactorChannel.state() != State.DOWN_RECONNECTING)
                    {
                        reactorChannel.state(State.DOWN);
                        sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_DOWN,
                                ReactorReturnCodes.FAILURE, "Worker.processChannelFlush",
                                "failed to add OP_WRITE to selectableChannel.");
                    }
                }

            }
            else if (retval == TransportReturnCodes.SUCCESS)
            {
                // flush succeeded
                if (!removeSelectOption(reactorChannel, SelectionKey.OP_WRITE))
                {
                    // Remove select option failed for this reactorChannel.
                    // Close this reactorChannel.
                    if (reactorChannel.state() != State.CLOSED && reactorChannel.state() != State.DOWN
                        && reactorChannel.state() != State.DOWN_RECONNECTING)
                    {
                        reactorChannel.state(State.DOWN);
                        sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_DOWN,
                                ReactorReturnCodes.FAILURE, "Worker.processChannelFlush",
                                "failed to remove OP_WRITE to selectableChannel.");
                    }
                }

                sendWorkerEvent(reactorChannel, WorkerEventTypes.FLUSH_DONE, ReactorReturnCodes.SUCCESS, null, null);
            }
            else if (retval < TransportReturnCodes.SUCCESS)
            {
                if (retval != TransportReturnCodes.WRITE_FLUSH_FAILED && retval != TransportReturnCodes.WRITE_CALL_AGAIN)
                {
                    // flush failed. Close this reactorChannel.
                    if (reactorChannel.state() != State.CLOSED && reactorChannel.state() != State.DOWN
                        && reactorChannel.state() != State.DOWN_RECONNECTING)
                    {
                        reactorChannel.state(State.DOWN);
                        sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_DOWN,
                                ReactorReturnCodes.FAILURE, "Worker.processChannelFlush",
                                "failed to flush selectableChannel, errorId=" + _error.errorId()
                                + " errorText=" + _error.text());
                    }
                }
            }
        }
    }

    private void processChannelFDChange(ReactorChannel reactorChannel)
    {
        int options = 0;

        // cancel old reactorChannel select
        try
        {
            SelectionKey key = reactorChannel.oldSelectableChannel().keyFor(_selector);
            if (key != null)
            {
                options = key.interestOps();
                key.cancel();
            }
        }
        catch (Exception e)
        {
        } // old channel may be null so ignore

        // register selector with channel event's new reactorChannel
        try
        {
            if (options != 0)
            {
                reactorChannel.selectableChannel().register(_selector,
                        options,
                        reactorChannel);
            }
        }
        catch (Exception e)
        {
            // selector register failed for this reactorChannel.
            // Close this reactorChannel.
            if (reactorChannel.state() != State.CLOSED && reactorChannel.state() != State.DOWN
                && reactorChannel.state() != State.DOWN_RECONNECTING)
            {
                reactorChannel.state(State.DOWN);

                sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_DOWN,
                        ReactorReturnCodes.FAILURE, "Worker.processChannelFDChange",
                        "selector register failed.");
            }
        }
    }

    private void initializeChannel(ReactorChannel reactorChannel)
    {
        Channel channel = reactorChannel.channel();
        int retval = channel.init(_inProg, _error);

        if (retval < TransportReturnCodes.SUCCESS)
        {
            cancelRegister(reactorChannel);
            reactorChannel.state(ReactorChannel.State.DOWN);
            sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_DOWN,
                    ReactorReturnCodes.FAILURE, "Worker.initializeChannel",
                    "Error initializing channel: errorId=" + _error.errorId() + " text="
                    + _error.text());
            return;
        }

        switch (retval)
        {
            case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                if (_inProg.flags() == InProgFlags.SCKT_CHNL_CHANGE)
                {
                    if ((retval = reRegister(_inProg, reactorChannel, _error)) != ReactorReturnCodes.SUCCESS)
                    {
                        cancelRegister(reactorChannel);
                        sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_DOWN,
                                ReactorReturnCodes.FAILURE, "Worker.initializeChannel",
                                "Error - failed to re-register on SCKT_CHNL_CHANGE: "
                                + _error.text());
                    }
                }
                else
                {
                    // check if initialization timeout occurred.
                    if (System.currentTimeMillis() > reactorChannel.initializationEndTimeMs())
                    {
                        cancelRegister(reactorChannel);
                        sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_DOWN,
                                ReactorReturnCodes.FAILURE, "Worker.initializeChannel",
                                "Error - exceeded initialization timeout ("
                                + reactorChannel.initializationTimeout() + " s)");
                    }
                }

                break;
            case TransportReturnCodes.SUCCESS:
                // init is complete, cancel selector registration.
                cancelRegister(reactorChannel);

                // channel.init is complete,
                // save the channel's negotiated ping timeout
                reactorChannel.pingHandler().initPingHandler(channel.pingTimeout());
                reactorChannel.resetCurrentChannelRetryCount();

                // move the channel from the initQueue to the activeQueue
                _initChannelQueue.remove(reactorChannel);
                _activeChannelQueue.add(reactorChannel);
                sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_UP,
                        ReactorReturnCodes.SUCCESS, null, null);
                break;
            default:
                cancelRegister(reactorChannel);
                sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_DOWN,
                        ReactorReturnCodes.FAILURE, "Worker.initializeChannel",
                        "Error - invalid return code: " + retval);
        }
    }

    private void cancelRegister(ReactorChannel reactorChannel)
    {
        try
        {
            SelectionKey key = reactorChannel.channel().selectableChannel().keyFor(_selector);
            if (key != null)
            {
                key.cancel();
            }
        }
        catch (Exception e)
        {
        }
    }

    private boolean addSelectOption(ReactorChannel reactorChannel, int options)
    {
        if (reactorChannel == null || (reactorChannel.state() != State.INITIALIZING &&
                                       reactorChannel.state() != State.UP &&
                                       reactorChannel.state() != State.READY))
            return false;

        Channel channel = reactorChannel.channel();
        if (channel == null)
            return false;

        //
        SelectionKey key = channel.selectableChannel().keyFor(_selector);
        if (key != null)
        {
            if ((key.interestOps() & options) != 0)
                // options are already registered
                return true;
            else
                // add the options to the current interestOps
                options |= key.interestOps();
        }

        try
        {
            channel.selectableChannel().register(_selector, options, reactorChannel);
        } catch (Exception e) { } // channel may be closed

        return true;
    }

    private boolean removeSelectOption(ReactorChannel reactorChannel, int options)
    {
        if (reactorChannel == null || (reactorChannel.state() != State.INITIALIZING &&
                                       reactorChannel.state() != State.UP &&
                                       reactorChannel.state() != State.READY))
            return false;

        Channel channel = reactorChannel.channel();
        if (channel == null)
            return false;

        SelectionKey key = channel.selectableChannel().keyFor(_selector);
        if (key != null)
        {
            try
            {
                int newOptions = key.interestOps() - options;
                if (newOptions != 0)
                {
                    try
                    {
                        channel.selectableChannel().register(_selector, newOptions, reactorChannel);
                    }
                    catch (ClosedChannelException e)
                    {
                        return false;
                    }
                }
                else
                    key.cancel();
            } catch (Exception e) { } // channel may be closed
        }
        return true;
    }

    private int reRegister(InProgInfo inProg, ReactorChannel reactorChannel, Error error)
    {
        // cancel old channel read select
        try
        {
            SelectionKey key = inProg.oldSelectableChannel().keyFor(_selector);
            if (key != null)
            {
                key.cancel();
            }
        }
        catch (Exception e)
        {
            error.text(e.getMessage());
            return ReactorReturnCodes.FAILURE;
        }

        // add new channel read select
        try
        {
            reactorChannel.channel().selectableChannel()
                    .register(_selector, SelectionKey.OP_READ, reactorChannel);
        }
        catch (Exception e)
        {
            error.text(e.getMessage());
            return ReactorReturnCodes.FAILURE;
        }

        // reset selectable channel on ReactorChannel to new one
        reactorChannel.selectableChannelFromChannel(reactorChannel.channel());

        // set oldSelectableChannel on ReactorChannel
        reactorChannel.oldSelectableChannel(inProg.oldSelectableChannel());

        return ReactorReturnCodes.SUCCESS;
    }

    private void sendWorkerEvent(ReactorChannel reactorChannel, WorkerEventTypes eventType, int reactorReturnCode, String location, String text)
    {
        WorkerEvent event = ReactorFactory.createWorkerEvent();
        event.reactorChannel(reactorChannel);
        event.eventType(eventType);
        event.errorInfo().code(reactorReturnCode);
        event.errorInfo().error().errorId(reactorReturnCode);
        if (location != null)
            event.errorInfo().location(location);
        if (text != null)
            event.errorInfo().error().text(text);
        _queue.write(event);
    }

    private void sendWorkerEvent(ReactorChannel reactorChannel, WorkerEventTypes eventType, TunnelStream tunnelStream, int reactorReturnCode, String location, String text)
    {
        WorkerEvent event = ReactorFactory.createWorkerEvent();
        event.reactorChannel(reactorChannel);
        event.eventType(eventType);
        event.tunnelStream(tunnelStream);
        event.errorInfo().code(reactorReturnCode);
        event.errorInfo().error().errorId(reactorReturnCode);
        if (location != null)
            event.errorInfo().location(location);
        if (text != null)
            event.errorInfo().error().text(text);
        _queue.write(event);
    }

    // - if registered, unregister RC
    // - if in active list, remove RC.
    // - close channel.
    // - add RC to Pool.
    private void shutdown()
    {
        _running = false;

        if (_selector != null)
        {
            Set<SelectionKey> keys = _selector.keys();
            if (keys != null)
            {
                for (SelectionKey key : keys)
                {
                    key.cancel();
                    ReactorChannel reactorChannel = (ReactorChannel)key.attachment();
                    if (reactorChannel != null)
                    {
                        if (reactorChannel.channel() != null)
                        {
                            reactorChannel.channel().close(_error);
                            if (_activeChannelQueue.remove(reactorChannel) == false)
                                if (_initChannelQueue.remove(reactorChannel) == false)
                                    _reconnectingChannelQueue.remove(reactorChannel);
                        }
                        else if (reactorChannel == _workerReactorChannel)
                        {
                            // The Reactor will call _queue.shutdown()
                            _queue = null;
                            _workerReactorChannel = null;
                        }
                        reactorChannel.returnToPool();
                    }
                }
            }

            try
            {
                _selector.close();
            }
            catch (IOException e)
            {
            }
            _selector = null;

            while (_initChannelQueue.size() > 0)
            {
                ReactorChannel reactorChannel = (ReactorChannel)_initChannelQueue.poll();
                if (reactorChannel != null)
                {
                    if (reactorChannel.channel() != null)
                    {
                        reactorChannel.channel().close(_error);
                    }

                    reactorChannel.reactor().removeReactorChannel(reactorChannel);
                    reactorChannel.returnToPool();
                }
            }

            while (_activeChannelQueue.size() > 0)
            {
                ReactorChannel reactorChannel = (ReactorChannel)_activeChannelQueue.poll();
                if (reactorChannel != null)
                {
                    if (reactorChannel.channel() != null)
                    {
                        reactorChannel.channel().close(_error);
                    }

                    reactorChannel.reactor().removeReactorChannel(reactorChannel);
                    reactorChannel.returnToPool();
                }
            }

            while (_reconnectingChannelQueue.size() > 0)
            {
                ReactorChannel reactorChannel = (ReactorChannel)_reconnectingChannelQueue.poll();
                if (reactorChannel != null)
                {
                    if (reactorChannel.channel() != null)
                    {
                        reactorChannel.channel().close(_error);
                    }

                    reactorChannel.reactor().removeReactorChannel(reactorChannel);
                    reactorChannel.returnToPool();
                }
            }

            if(_reactor.numberOfTokenSession() != 0)
            {
                _reactor.removeAllTokenSession();
            }
        }

        _error = null;
    }

    private int initializeWorker()
    {
        try
        {
            _selector = SelectorProvider.provider().openSelector();
            _workerReactorChannel = ReactorFactory.createReactorChannel(_reactor);
            _workerReactorChannel.selectableChannel(_queue.readChannel());
            _queue.readChannel().register(_selector, SelectionKey.OP_READ, _workerReactorChannel);
        }
        catch (IOException e)
        {
            _running = false;
            System.out.println("Worker.initializeWorker() failed, exception="
                               + e.getLocalizedMessage());
            return ReactorReturnCodes.FAILURE;
        }

		return ReactorReturnCodes.SUCCESS;
	}
}
