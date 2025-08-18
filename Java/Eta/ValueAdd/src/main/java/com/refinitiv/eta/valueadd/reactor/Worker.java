/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.io.IOException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.spi.SelectorProvider;
import java.text.ParseException;
import java.util.Date;
import java.util.Iterator;
import java.util.Map;
import java.util.Set;

import org.quartz.CronExpression;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InProgFlags;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.Transport;
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
    
    State _previousReactorChannelState;

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
                    	/* This PH timer event is already canceled when a timer is installed */
                    	if(event._isCanceled)
                    	{
                    		_timerEventQueue.remove(event);
                            event.returnToPool();
                    		continue;
                    	}
                    	
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
                        else if (event.eventType() == WorkerEventTypes.PREFERRED_HOST_TIMER)
                        {
                        	if (event.reactorChannel()._preferredHostOptions.detectionTimeSchedule() != null &&
                      				 !event.reactorChannel()._preferredHostOptions.detectionTimeSchedule().isEmpty())
                      		 	{
                           		// Debug log output that schedule fallback has been initiated
                                   if (_reactor._reactorOptions.debuggerOptions().debugConnectionLevel()) {
                                       _reactor.debugger.writeDebugInfo("Preferred host: Time schedule triggered to attempt switching to preferred host",
                                               _reactor.hashCode(),
                                               this.hashCode()
                                       );
                                   }
                      		 	}
                           	else if (event.reactorChannel()._preferredHostOptions.detectionTimeInterval() > 0)
                     		 	{
                           		// Debug log output that interval fallback has been initiated
                                   if (_reactor._reactorOptions.debuggerOptions().debugConnectionLevel()) {
                                       _reactor.debugger.writeDebugInfo("Preferred host: Time interval triggered to attempt switching to preferred host",
                                               _reactor.hashCode(),
                                               this.hashCode()
                                       );
                                   }
                     		 	}
                        	
                    		// Trigger preferred host change
                         	event.reactorChannel().switchHostToPreferredHost();
                         	
                    		// Set next time up
                    		 if (event.reactorChannel()._preferredHostOptions.detectionTimeSchedule() != null &&
                    				 !event.reactorChannel()._preferredHostOptions.detectionTimeSchedule().isEmpty())
                    		 {
                    			 try {
        							CronExpression tr = new CronExpression(event.reactorChannel()._preferredHostOptions.detectionTimeSchedule());
        							Date currentTime = new Date(System.currentTimeMillis());
        							Date nextTime = tr.getNextValidTimeAfter(currentTime);

        	           		 		event.reactorChannel()._nextReconnectTimeMs = nextTime.getTime();
        	           		 		event.timeout(System.nanoTime() + ((nextTime.getTime() - currentTime.getTime()) * 1000000));
        							
                    			 } catch (ParseException e) {
        							// Incorrect crontime scheduled, log and abandon
                    				 	System.out.println("Worker.run() exception=" + e.getLocalizedMessage());
                    				 	sendWorkerEvent(_reactorReactorChannel, WorkerEventTypes.SHUTDOWN,
                    	                        ReactorReturnCodes.FAILURE, "Worker.run",
                    	                        "exception occurred, " + e.getLocalizedMessage());
                    				 	_timerEventQueue.remove(event);
                    				 	event.returnToPool();
        						}
                    		 }
                    		 else if (event.reactorChannel()._preferredHostOptions.detectionTimeInterval() > 0)
                   		 	 {
                   		 		event.reactorChannel()._nextReconnectTimeMs = System.currentTimeMillis() + event.reactorChannel()._preferredHostOptions.detectionTimeInterval() * 1000;
                   		 		event.timeout(System.nanoTime() + (event.reactorChannel()._preferredHostOptions.detectionTimeInterval() * 1000000000));
                   		 	 }
                        }
                        else if (event.eventType() == WorkerEventTypes.PREFERRED_HOST_START_FALLBACK)
                        {
                        	// First check if fallBackWithInWSBGroup is true, and handle it if possible
                        	if (event.reactorChannel().warmStandByHandlerImpl != null
                        			&& event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel()._preferredHostOptions.fallBackWithInWSBGroup())
                        	{
                        		ReactorWarmStandbyHandler warmStandByHandlerImpl = event.reactorChannel().warmStandByHandlerImpl;
                        		boolean switchingToActive = false;
                        		if (event.reactorChannel().warmStandByHandlerImpl.currentWarmStandbyGroupImpl().warmStandbyMode() == ReactorWarmStandbyMode.SERVICE_BASED)
                        		{
        							Iterator<Map.Entry<WlInteger, ReactorWSBService>> iter = event.reactorChannel().warmStandByHandlerImpl.currentWarmStandbyGroupImpl()._perServiceById
        									.entrySet().iterator();
        							ReactorWSBService service = null;
        							while (iter.hasNext())
        							{
        								service = iter.next().getValue();
        								if (service.activeChannel != null)
        								{
        									// Active channel exists
        									continue;
        								}
        							}
        							if (service.activeChannel != null)
        							{
        								boolean serviceIsStandbyServer = true;
    									for (ReactorChannel channel : warmStandByHandlerImpl.channelList())
    									{
    										if (service.activeChannel == channel)
    										{
    											if (service.activeChannel == warmStandByHandlerImpl.startingReactorChannel())
    											{
    												serviceIsStandbyServer = false;
    											}
    										}
    									}
    									
    									if (serviceIsStandbyServer)
    									{
    										// We are currently on a standby server, check if the starting active server is connected
                        					if (warmStandByHandlerImpl.startingReactorChannel().channel().state() == ChannelState.ACTIVE)
                        					{
                    							ReactorWarmStandbyEvent reactorWarmStandbyEvent = _reactor.reactorWarmStandbyEventPool
                    									.getEvent(event.reactorChannel().getEDPErrorInfo());

                    							reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.PREFERRED_HOST_FALLBACK_IN_GROUP;
                    							reactorWarmStandbyEvent.reactorChannel = event.reactorChannel();

                    							event.reactorChannel().reactor().sendWarmStandbyEvent(event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel(), reactorWarmStandbyEvent, event.reactorChannel().getEDPErrorInfo());
                    							switchingToActive = true;
                    							_timerEventQueue.remove(event);
                                        		event.returnToPool();
                                        		continue;
                        					}
    									}
    									else
    									{
    										// We are currently on the preferred server, break out and abandon this call 
    										_timerEventQueue.remove(event);
                                    		event.returnToPool();
                                    		continue;
    									}
    								
        							}
        							else
        							{
        								// No active channel exists, break out and abandon this call
                                		_timerEventQueue.remove(event);
                                		event.returnToPool();
                                		continue;
        							}
                        		}
                        		else if (event.reactorChannel().warmStandByHandlerImpl.currentWarmStandbyGroupImpl().warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
                        		{
                        			for (ConnectOptionsInfo standbyOptions : warmStandByHandlerImpl.currentWarmStandbyGroupImpl().standbyConnectOptionsInfoList)
                        			{
                        				if (warmStandByHandlerImpl.activeReactorChannel() != null
                        						&& standbyOptions == warmStandByHandlerImpl.activeReactorChannel().getCurrentConnectOptionsInfo())
                        				{
                        					// We are currently on a standby server, check if the starting active server is connected
                        					if (warmStandByHandlerImpl.startingReactorChannel().channel().state() == ChannelState.ACTIVE)
                        					{
                    							ReactorWarmStandbyEvent reactorWarmStandbyEvent = _reactor.reactorWarmStandbyEventPool
                    									.getEvent(event.reactorChannel().getEDPErrorInfo());

                    							reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.PREFERRED_HOST_FALLBACK_IN_GROUP;
                    							reactorWarmStandbyEvent.reactorChannel = event.reactorChannel();

                    							event.reactorChannel().reactor().sendWarmStandbyEvent(event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel(), reactorWarmStandbyEvent, event.reactorChannel().getEDPErrorInfo());
                    							switchingToActive = true;
                                        		_timerEventQueue.remove(event);
                                        		event.returnToPool();
                                        		break;
                        					}
                        				}
                        			}
                        		}

                        		// Send preferred host switchover complete event
								sendWorkerEvent(event.reactorChannel(), WorkerEventTypes.PREFERRED_HOST_COMPLETE, ReactorReturnCodes.SUCCESS, "Worker.run", null);

								// Break out of this loop, we are not going to fallback in other ways when fallBackWithInWSBGroup is true
                    			if (switchingToActive)
                    				continue;
                        	}
                        	if ((event.reactorChannel().switchingToPreferredHost() || event.reactorChannel().switchingToPreferredWSBGroup()) && event.reactorChannel().preferredHostChannel() == null)
                        	{
                        		// If session management enabled, check to make sure we're in a state where we can initialize the connection
                        		if (event.reactorChannel().getCurrentReactorConnectInfo() != null &&
                        				event.reactorChannel().enableSessionManagement() &&
                        				event.reactorChannel().tokenSession() != null &&
                        				event.reactorChannel().tokenSession().authTokenInfo().tokenVersion() == TokenVersion.V2)
                        		{
                                    /* If we have connected with a V2 session management connection and wiled the access token, reset the session management state 
                                     * and reconnect the token session. */
                                	if (event.reactorChannel().state() != State.EDP_RT &&
                        					event.reactorChannel().state() != State.EDP_RT_DONE &&
                        					event.reactorChannel().state() != State.EDP_RT_FAILED)
                        			{
                                        	_previousReactorChannelState = event.reactorChannel().state();
                                        	event.reactorChannel().tokenSession().authTokenInfo().accessToken(null);
                                        	event.reactorChannel().tokenSession().resetSessionMgntState();
                                        	event.reactorChannel().reconnectTokenSession(_error);
                                        	continue;	// Retry later, checking whether the new auth token is ready
                        			}
                                    if (event.reactorChannel().state() == State.EDP_RT ||
                                    		event.reactorChannel().state() == State.EDP_RT_DONE ||
                                    				event.reactorChannel().state() == State.EDP_RT_FAILED)
                                    {
                                        // Check auth reply and initialize new transport if we have it
                                    	if (event.reactorChannel().checkAuthTokenReady(_error) &&
                                    			event.reactorChannel().tokenSession() != null)
                                    	{
                            				if (event.reactorChannel().warmStandByHandlerImpl != null)
                            				{
                                        		// Initialize the new transport channel for WSB Preferred group
                                            	event.reactorChannel().preferredHostChannel(Transport.connect(event.reactorChannel()._reactorConnectOptions._reactorWarmStandyGroupList.get(event.reactorChannel()._preferredHostOptions.warmStandbyGroupListIndex()).startingActiveServer().reactorConnectInfo().connectOptions(), _error));

                                            	if (event.reactorChannel().preferredHostChannel() != null)
                                            		event.reactorChannel().initializationTimeout(event.reactorChannel().getCurrentReactorConnectInfo().initTimeout());
                                            	
                            				}
                            				else
                            				{
                            					// Initialize the new transport channel for connectionList
                            					event.reactorChannel().preferredHostChannel(Transport.connect(event.reactorChannel()._reactorConnectOptions._connectionList.get(event.reactorChannel()._preferredHostOptions.connectionListIndex())._connectOptions, _error));

                                            	if (event.reactorChannel().preferredHostChannel() != null)
                                            		event.reactorChannel().initializationTimeout(event.reactorChannel().getCurrentReactorConnectInfo().initTimeout());
                                            	
                            				}
                                    	}
                                    	else
                                    	{
                                    		continue;	// Retry later, checking whether the new auth token is ready
                                    	}
                                    }
                        		}
                        		else
                        		{
                    				if (event.reactorChannel().warmStandByHandlerImpl != null
                    						&& event.reactorChannel().switchingToPreferredWSBGroup())
                    				{
                    					// Debug log output that fallback is attempting to connect particular WSB group index
                                        if (event.reactorChannel().reactor()._reactorOptions.debuggerOptions().debugConnectionLevel()) {
                                        	event.reactorChannel().reactor().debugger.writeDebugInfo("Attempting to switch to WarmStandByGroup Index: " + event.reactorChannel().warmStandByHandlerImpl.currentWarmStandbyGroupIndex(),
                                        			event.reactorChannel().reactor().hashCode(),
                                                    this.hashCode()
                                            );
                                        }
                    					
                                		// Initialize the new transport channel for WSB Preferred group
                                    	event.reactorChannel().preferredHostChannel(Transport.connect(event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel()._reactorConnectOptions._reactorWarmStandyGroupList.get(event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel()._preferredHostOptions.warmStandbyGroupListIndex()).startingActiveServer().reactorConnectInfo().connectOptions(), _error));

                                    	if (event.reactorChannel().preferredHostChannel() != null)
                                    		event.reactorChannel().initializationTimeout(event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel().getCurrentReactorConnectInfo().initTimeout());
                                    	
                    				}
                    				else
                    				{
                    					// Debug log output that fallback is attempting to connect particular ConnectionList index
                                        if (event.reactorChannel().reactor()._reactorOptions.debuggerOptions().debugConnectionLevel()) {
                                        	event.reactorChannel().reactor().debugger.writeDebugInfo("Attempting to switch to Connection List Index: " + event.reactorChannel()._preferredHostOptions.connectionListIndex(),
                                        			event.reactorChannel().reactor().hashCode(),
                                                    this.hashCode()
                                            );
                                        }
                    					
                    					// Initialize the new transport channel for connectionList
                    					event.reactorChannel().preferredHostChannel(Transport.connect(event.reactorChannel()._reactorConnectOptions._connectionList.get(event.reactorChannel()._preferredHostOptions.connectionListIndex())._connectOptions, _error));

                                    	if (event.reactorChannel().preferredHostChannel() != null)
                                    		event.reactorChannel().initializationTimeout(event.reactorChannel().getCurrentReactorConnectInfo().initTimeout());
                                    	
                    				}
                        		}
                        	}
                        	if ((event.reactorChannel().switchingToPreferredHost() || event.reactorChannel().switchingToPreferredWSBGroup()) && event.reactorChannel().preferredHostChannel() != null)
                        	{
                        		try {
                            		// Check if preferred host channel is the same as the one we are on, in which case we can get rid of this event and continue
                        			if (event.reactorChannel().warmStandByHandlerImpl != null)
                        			{
                        				if (event.reactorChannel().preferredHostChannel().hostname().equals(event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel().channel().hostname()) &&
                                				event.reactorChannel().preferredHostChannel().port() == event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel().channel().port())
                                		{
                    						event.reactorChannel().initiateSwitch(false, false);
                    						if (_previousReactorChannelState != null)
                    						{
                    							event.reactorChannel().state(_previousReactorChannelState);	// Reset state
                    							_previousReactorChannelState = null;
                    						}
                                    		_timerEventQueue.remove(event);
                                    		event.returnToPool();
                                    		continue;
                                		}
                        			}
                        			else
                        			{
                        				if (event.reactorChannel().preferredHostChannel().hostname().equals(event.reactorChannel().channel().hostname()) &&
                                				event.reactorChannel().preferredHostChannel().port() == event.reactorChannel().channel().port())
                                		{
                    						event.reactorChannel().initiateSwitch(false, false);
                    						if (_previousReactorChannelState != null)
                    						{
                    							event.reactorChannel().state(_previousReactorChannelState);	// Reset state
                    							_previousReactorChannelState = null;
                    						}
                                    		_timerEventQueue.remove(event);
                                    		event.returnToPool();
                                    		continue;
                                		}
                        			}
                            		
                        		}
                        		catch (Exception e)
                        		{
                        			// Failed to initiate a switch, likely due to channel being down. Abandon switchover attempt.
            						event.reactorChannel().initiateSwitch(false, false);
            						if (_previousReactorChannelState != null)
            						{
            							event.reactorChannel().state(_previousReactorChannelState);	// Reset state
            							_previousReactorChannelState = null;
            						}
            						sendPreferredHostCompleteEvent(event.reactorChannel());
                            		_timerEventQueue.remove(event);
                            		event.returnToPool();
                            		continue;
                        		}

                        		if (event.reactorChannel().warmStandByHandlerImpl != null)
                        		{
                        			boolean connectingToWSBGroup = false;
                        			// Check if this preferred host channel we are connecting to is an starting active channel of one of the WSB groups
                        			for (ReactorWarmStandbyGroup group : event.reactorChannel().warmStandByHandlerImpl.warmStandbyGroupList())
                        			{
                        				try {
                            				if (group != null &&
                            						event.reactorChannel().preferredHostChannel() != null &&
                            						group.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address().equals(event.reactorChannel().preferredHostChannel().hostname()) &&
                            						group.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName().equals(String.valueOf(event.reactorChannel().preferredHostChannel().port())))
                            				{
                            					// We are attempting to connect to one of the starting active servers
                            					connectingToWSBGroup = true;
                            					int ret = event.reactorChannel().preferredHostChannel().init(_inProg, _error);
                                            	if (ret == TransportReturnCodes.SUCCESS)
                                                {
                                            		// channel.init is complete,
                                                    // save the channel's negotiated ping timeout
                                                    event.reactorChannel().pingHandler().initPingHandler(event.reactorChannel().preferredHostChannel().pingTimeout());
                                                    event.reactorChannel().resetCurrentChannelRetryCount();
                                            		
                                            		// Send event to Reactor to switch the channel over
                                            		sendWorkerEvent(event.reactorChannel(), WorkerEventTypes.PREFERRED_HOST_SWITCH_CHANNEL,
                                                            ReactorReturnCodes.SUCCESS, "Worker.run", null);

                                            		_timerEventQueue.remove(event);

                                            		event.returnToPool();
                                            		break;
                                                }
                                            	else if (ret == TransportReturnCodes.FAILURE || System.currentTimeMillis() > event.reactorChannel().initializationEndTimeMs())
                                            	{
                                            		event.reactorChannel().initiateSwitch(false, false);
                            						if (_previousReactorChannelState != null)
                            						{
                            							event.reactorChannel().state(_previousReactorChannelState);	// Reset state
                            							_previousReactorChannelState = null;
                            						}
                                            		sendPreferredHostCompleteEvent(event.reactorChannel());
                                            		_timerEventQueue.remove(event);
                                            		event.returnToPool();
                                            		break;
                                            	}
                            				}
                        				}
                        				catch (Exception e)
                        				{
                        					if (event.reactorChannel().preferredHostChannel() == null)
                        					{
                        						event.reactorChannel().initiateSwitch(false, false);
                        						if (_previousReactorChannelState != null)
                        						{
                        							event.reactorChannel().state(_previousReactorChannelState);	// Reset state
                        							_previousReactorChannelState = null;
                        						}
                        						sendPreferredHostCompleteEvent(event.reactorChannel());
                                        		_timerEventQueue.remove(event);
                                        		event.returnToPool();
                        						break;
                        					}
                        				}
                        			}
                        			
                        			if (!connectingToWSBGroup)
                        			{
                        				try {
	                        				// We are not connecting to one of the warmstandby groups, and must be in connectionList. 
                        					int ret = event.reactorChannel().preferredHostChannel().init(_inProg, _error);
	                                    	if (ret == TransportReturnCodes.SUCCESS)
	                                        {
	                                    		// Send event to Reactor to switch the channel over
	                                    		sendWorkerEvent(event.reactorChannel(), WorkerEventTypes.PREFERRED_HOST_SWITCH_CHANNEL,
	                                                    ReactorReturnCodes.SUCCESS, "Worker.run", null);
	                                    		
	                                    		_timerEventQueue.remove(event);
	                                    		event.returnToPool();
	                                    		continue;
	                                        }
	                                    	else if (ret == TransportReturnCodes.FAILURE || System.currentTimeMillis() > event.reactorChannel().initializationEndTimeMs())
	                                    	{
	                                    		event.reactorChannel().initiateSwitch(false, false);
	                    						if (_previousReactorChannelState != null)
	                    						{
	                    							event.reactorChannel().state(_previousReactorChannelState);	// Reset state
	                    							_previousReactorChannelState = null;
	                    						}
	                                    		sendPreferredHostCompleteEvent(event.reactorChannel());
	                                    		_timerEventQueue.remove(event);
	                                    		event.returnToPool();
	                                    		continue;
	                                    	}
                        				}
                        				catch (Exception e)
                        				{
                        					if (event.reactorChannel() == null || event.reactorChannel().preferredHostChannel() == null)
                        					{
                        						event.reactorChannel().initiateSwitch(false, false);
                        						if (_previousReactorChannelState != null)
                        						{
                        							event.reactorChannel().state(_previousReactorChannelState);	// Reset state
                        							_previousReactorChannelState = null;
                        						}
                        						sendPreferredHostCompleteEvent(event.reactorChannel());
                                        		_timerEventQueue.remove(event);
                                        		event.returnToPool();
                                        		continue;
                        					}
                        				}
                        			}
                        		}
                        		else
                        		{	
                        			int ret = event.reactorChannel().preferredHostChannel().init(_inProg, _error);
                                	if (ret == TransportReturnCodes.SUCCESS)
                                    {
                                		// Send event to Reactor to switch the channel over
                                		sendWorkerEvent(event.reactorChannel(), WorkerEventTypes.PREFERRED_HOST_SWITCH_CHANNEL,
                                                ReactorReturnCodes.SUCCESS, "Worker.run", null);
                                		_timerEventQueue.remove(event);
                                		event.returnToPool();
                                		continue;
                                    }
                                	else if (ret == TransportReturnCodes.FAILURE || System.currentTimeMillis() > event.reactorChannel().initializationEndTimeMs())
                                	{
                                		event.reactorChannel().initiateSwitch(false, false);
                						if (_previousReactorChannelState != null)
                						{
                							event.reactorChannel().state(_previousReactorChannelState);	// Reset state
                							_previousReactorChannelState = null;
                						}
                                		sendPreferredHostCompleteEvent(event.reactorChannel());
                                		_timerEventQueue.remove(event);
                                		event.returnToPool();
                                		continue;
                                	}
                        		}
                        	}
                        }
                        else if (event.eventType() == WorkerEventTypes.PREFERRED_HOST_IOCTL)
                        {    	
                        	// Ensure we are not currently switching to preferred host or in a preferred host operation
                        	if (event.reactorChannel().switchingToPreferredHost() || event.reactorChannel().switchingToPreferredWSBGroup())
                        	{
                        		// This event is not ready to be acted on, keep this in the event queue
                        		continue;
                        	}
                        	
                        	if (event.reactorChannel()._preferredHostOptionsIoctl == null)
                        	{
                        		// There was no ioctl set in this reactorChannel
                        		
                        		_reactor.populateErrorInfo(event.reactorChannel().getEDPErrorInfo(), ReactorReturnCodes.FAILURE, "ReactorChannel.ioctl",
                						"Failed to complete ioctl call, no ioctl changes made.");
                        		
                                _timerEventQueue.remove(event);
                                event.returnToPool();
                                continue;
                        	}
                        	
                        	// Lock to ensure changes are made before/after using the options elsewhere
                        	event.reactorChannel()._preferredHostLock.lock();

                			// Handle new cron expression
                			if (event.reactorChannel()._preferredHostOptionsIoctl.detectionTimeSchedule() != null
                				&& !event.reactorChannel()._preferredHostOptionsIoctl.detectionTimeSchedule().isEmpty())
							{
                    			if (event.reactorChannel()._cronExpressionIoctl != null)
                    				event.reactorChannel()._cronExpression = event.reactorChannel()._cronExpressionIoctl;
							}
                			
                			// If connection level debugging enabled, check through each option and have a debug statement for any changed options
                            if (_reactor._reactorOptions.debuggerOptions().debugConnectionLevel()) {
	                			if (event.reactorChannel()._preferredHostOptions.isPreferredHostEnabled() != event.reactorChannel()._preferredHostOptionsIoctl.isPreferredHostEnabled())
	                			{
                                    _reactor.debugger.writeDebugInfo("IOCTL Change to isPreferredHostEnabled, new value = " + event.reactorChannel()._preferredHostOptionsIoctl.isPreferredHostEnabled(),
                                            _reactor.hashCode(),
                                            this.hashCode()
                                    );
	                			}
                            	if (event.reactorChannel()._preferredHostOptions.connectionListIndex() != event.reactorChannel()._preferredHostOptionsIoctl.connectionListIndex())
	                			{
                                    _reactor.debugger.writeDebugInfo("IOCTL Change to connectionListIndex, new value = " + event.reactorChannel()._preferredHostOptionsIoctl.connectionListIndex(),
                                            _reactor.hashCode(),
                                            this.hashCode()
                                    );
	                			}
	                			if (event.reactorChannel()._preferredHostOptions.warmStandbyGroupListIndex() != event.reactorChannel()._preferredHostOptionsIoctl.warmStandbyGroupListIndex())
	                			{
                                    _reactor.debugger.writeDebugInfo("IOCTL Change to warmStandbyGroupListIndex, new value = " + event.reactorChannel()._preferredHostOptionsIoctl.warmStandbyGroupListIndex(),
                                            _reactor.hashCode(),
                                            this.hashCode()
                                    );
	                			}
	                			if (event.reactorChannel()._preferredHostOptions.detectionTimeInterval() != event.reactorChannel()._preferredHostOptionsIoctl.detectionTimeInterval())
	                			{
                                    _reactor.debugger.writeDebugInfo("IOCTL Change to detectionTimeInterval, new value = " + event.reactorChannel()._preferredHostOptionsIoctl.detectionTimeInterval(),
                                            _reactor.hashCode(),
                                            this.hashCode()
                                    );
	                			}
	                			if (!event.reactorChannel()._preferredHostOptions.detectionTimeSchedule().equals(event.reactorChannel()._preferredHostOptionsIoctl.detectionTimeSchedule()))
	                			{
                                    _reactor.debugger.writeDebugInfo("IOCTL Change to detectionTimeSchedule, new value = " + event.reactorChannel()._preferredHostOptionsIoctl.detectionTimeSchedule(),
                                            _reactor.hashCode(),
                                            this.hashCode()
                                    );
	                			}
	                			if (event.reactorChannel()._preferredHostOptions.fallBackWithInWSBGroup() != event.reactorChannel()._preferredHostOptionsIoctl.fallBackWithInWSBGroup())
	                			{
                                    _reactor.debugger.writeDebugInfo("IOCTL Change to fallBackWithInWSBGroup, new value = " + event.reactorChannel()._preferredHostOptionsIoctl.fallBackWithInWSBGroup(),
                                            _reactor.hashCode(),
                                            this.hashCode()
                                    );
	                			}
                            }
                            
                            event.reactorChannel()._preferredHostOptions = event.reactorChannel()._preferredHostOptionsIoctl;
                            if (event.reactorChannel()._reactorConnectOptions != null)
                            	event.reactorChannel()._reactorConnectOptions._reactorPreferredHostOptions = event.reactorChannel()._preferredHostOptions;
                			if (event.reactorChannel().warmStandByHandlerImpl != null
                					&& event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel() != null)
                			{
                				event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel()._preferredHostOptions = event.reactorChannel()._preferredHostOptions;
                				event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel()._reactorConnectOptions._reactorPreferredHostOptions = event.reactorChannel()._preferredHostOptions;
                				event.reactorChannel().warmStandByHandlerImpl.startingReactorChannel()._switchingToPreferredWSBGroup = false;
                			}
                             
                			
                			_reactor.sendWorkerEvent(WorkerEventTypes.PREFERRED_HOST_TIMER, event.reactorChannel());
                			event.reactorChannel()._preferredHostOptionsIoctl = null;
                			event.reactorChannel()._cronExpressionIoctl = null;
                            _timerEventQueue.remove(event);
                            event.reactorChannel()._preferredHostLock.unlock();
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
                        if (reactorChannel.nextRecoveryTime() > System.currentTimeMillis()
                        		&& !reactorChannel._reconnectImmedietlyToPH)
                            continue;

                        if (reactorChannel._reconnectImmedietlyToPH)
                        	reactorChannel._reconnectImmedietlyToPH = false;

                        Channel channel = null;
                        ReactorWarmStandbyServerInfo wsbServerImpl = null;
                    
                        if(_reactor.reactorHandlesWarmStandby(reactorChannel)
                        		|| (reactorChannel._preferredHostOptions.isPreferredHostEnabled() 
                        				&& reactorChannel.warmStandByHandlerImpl != null))
                        {
                        	reactorChannel._phSwitchingFromWSBToChannelList = false;	// Reset this flag
                            if (reactorChannel._reconnectAttempts + 1 >= reactorChannel.reconnectAttemptLimit() 
                            		&& reactorChannel.reconnectAttemptLimit() >= 0
                            		&& reactorChannel._haveAttemptedFirstConnection)
                            {
                            	reactorChannel._reconnectAttempts++;
	                        	boolean isAnotherChannelActive = false;
	                        	// Check if we have an active connection in our group already that we've switched to (or should switch to)
	                        	for (ReactorChannel wsbChannel : reactorChannel.warmStandByHandlerImpl.channelList())
	                        	{
	                        		if (wsbChannel.state() == ReactorChannel.State.READY
	                        				|| wsbChannel.state() == ReactorChannel.State.UP)
	                        			isAnotherChannelActive = true;
	                        	}
	                        	
	                        	// If preferred host is enabled, we are not currently trying to reach a preferred host, and we don't have another channel active in our current group
		                   		if (!isAnotherChannelActive && reactorChannel._preferredHostOptions.isPreferredHostEnabled() && reactorChannel.preferredHostChannel() == null)
		                   		{
		                   			// If we were not switching to preferred WSB Group, do this now
		                   			if (!reactorChannel._switchingToPreferredWSBGroup)
			                   		 {
		                   				// Check if we are currently on Channel List and set switch from Channel List to WSB
		                   				if ((reactorChannel.warmStandByHandlerImpl.warmStandbyHandlerState() 
		                   						& ReactorWarmStandbyHandlerState.MOVED_TO_CHANNEL_LIST) != 0)
		                   				{
		                   					reactorChannel._phSwitchingFromChannelListToWSB = true;
		                   				}
		                   					
		                   				reactorChannel._switchingToPreferredWSBGroup = true;
			                   			reactorChannel.warmStandByHandlerImpl.previousWarmStandbyGroupIndex(reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex());
			                   			reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex(reactorChannel.getReactorConnectOptions().reactorPreferredHostOptions().warmStandbyGroupListIndex());
			                   			wsbServerImpl = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl().startingActiveServer();
			                   			
			                   			ReactorWarmStandbyGroupImpl wsbGroup = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl();
			                   			reactorChannel.setCurrentReactorConnectInfo(wsbGroup.startingActiveServer().reactorConnectInfo());
	                            		reactorChannel.setCurrentConnectOptionsInfo(wsbGroup.startingConnectOptionsInfo);
	                            		reactorChannel.userSpecObj(reactorChannel.getCurrentReactorConnectInfo().connectOptions().userSpecObject());
			                   			
			                   			// Set expected channel state
			                   			reactorChannel.warmStandByHandlerImpl
			                   				.warmStandbyHandlerState(ReactorWarmStandbyHandlerState.CONNECTING_TO_A_STARTING_SERVER);
			                   			reactorChannel.warmStandByHandlerImpl.startingReactorChannel()
			                   				.reactorChannelType(ReactorChannelType.WARM_STANDBY);
			                   			
			                    		// Handle service based options
			                        	if (reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl().warmStandbyMode() == ReactorWarmStandbyMode.SERVICE_BASED)
			                    		{
			                        		reactorChannel.copyActiveServiceOptions();
			                    		}
			                   			 
			                            if (_reactor._reactorOptions.debuggerOptions().debugConnectionLevel()) {
			                                _reactor.debugger.writeDebugInfo("Switching to warmStandbyGroup index: " + 
			                                		reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex(),
			                                        _reactor.hashCode(),
			                                        this.hashCode()
			                                );
			                            }
			                   		 }
		                   			 // Otherwise, if we have another WSB Group to check normally still, attempt to connect to that WSB Group
			                   		 else if ((reactorChannel.warmStandByHandlerImpl.previousWarmStandbyGroupIndex() + 1) < reactorChannel.warmStandByHandlerImpl.warmStandbyGroupList().size())
			                   		 {
			                   			boolean checkChannelListInstead = false; // Triggers if we can skip preferred group and have run out of groups to check
			                   			reactorChannel._switchingToPreferredWSBGroup = false;
			                   			
			                   			// Check if we are at the end of our WSB Group List without a channelList
			                            if ((reactorChannel.warmStandByHandlerImpl.previousWarmStandbyGroupIndex() + 1) >= reactorChannel.warmStandByHandlerImpl.warmStandbyGroupList().size()
			                            		&& reactorChannel.getReactorConnectOptions().connectionList().size() == 0)
										{
											// Reset our WSB group index
			                    			reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex(0);
			                    			reactorChannel.warmStandByHandlerImpl.previousWarmStandbyGroupIndex(-1);
										}
			                   			
			                   			// Rollover back to beginning of WSB Group list if needed
			                   			if (reactorChannel._hitEndOfWSBGroups)
			                   			{
			                   				rollbackWsbGroupCurrentIndex(reactorChannel);
			                   				reactorChannel._hitEndOfWSBGroups = false;
			                   			}
			                   			else
			                   			{
			                   				if ((reactorChannel.warmStandByHandlerImpl.previousWarmStandbyGroupIndex() + 1) != reactorChannel._preferredHostOptions.warmStandbyGroupListIndex())
			                   					reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex(reactorChannel.warmStandByHandlerImpl.previousWarmStandbyGroupIndex() + 1);
			                   				else // Skip preferred group, we check that each time already
			                   				{
			                   					if (reactorChannel.warmStandByHandlerImpl.previousWarmStandbyGroupIndex() + 2 < reactorChannel.warmStandByHandlerImpl.warmStandbyGroupList().size())
			                   						reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex(reactorChannel.warmStandByHandlerImpl.previousWarmStandbyGroupIndex() + 2);
			                   					else // Finished checking through WSB Groups, switch to connection list, starting with the preferred one
			                   					{
			                   						checkChannelListInstead = true;
	
			                   					}
			                   				}
			                   				
				                    		// Handle service based options
				                        	if (reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl().warmStandbyMode() == ReactorWarmStandbyMode.SERVICE_BASED)
				                    		{
				                        		reactorChannel.copyActiveServiceOptions();
				                    		}
			                   			}
			                   			
			                   			if (checkChannelListInstead && reactorChannel._checkedPreferredHostInChannelList_WSBEnabled
			                   					&& reactorChannel.getReactorConnectOptions().connectionList().size() > 0)
			                   				checkChannelList(reactorChannel);
			                   			else if (checkChannelListInstead && !reactorChannel._checkedPreferredHostInChannelList_WSBEnabled
			                   					&& reactorChannel.getReactorConnectOptions().connectionList().size() > 0)
			                   				checkPreferredChannelList(reactorChannel);
			                   			else
			                   			{
			                   				wsbServerImpl = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl().startingActiveServer();
				                   			
				                   			ReactorWarmStandbyGroupImpl wsbGroup = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl();
				                   			reactorChannel.setCurrentReactorConnectInfo(wsbGroup.startingActiveServer().reactorConnectInfo());
		                            		reactorChannel.setCurrentConnectOptionsInfo(wsbGroup.startingConnectOptionsInfo);
		                            		reactorChannel.userSpecObj(reactorChannel.getCurrentReactorConnectInfo().connectOptions().userSpecObject());
				                   			
				                   			// Set expected channel state
				                   			reactorChannel.warmStandByHandlerImpl
				                   				.warmStandbyHandlerState(ReactorWarmStandbyHandlerState.CONNECTING_TO_A_STARTING_SERVER);
				                   			reactorChannel.warmStandByHandlerImpl.startingReactorChannel()
				                   				.reactorChannelType(ReactorChannelType.WARM_STANDBY);
				                   			
				                            if (_reactor._reactorOptions.debuggerOptions().debugConnectionLevel()) {
				                                _reactor.debugger.writeDebugInfo("Switching to warmStandbyGroup index: " + 
				                                		reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex(),
				                                        _reactor.hashCode(),
				                                        this.hashCode()
				                                );
				                            }
			                   			}
			                   		 }
		                   			// If we are at the end of our WSB group list and we've already checked preferred WSB Group,
		                   			// 		then if we've already checked preferred channel in ChannelList moving between WSB Group and ChannelList,
		                   			//		check the next regular channel
			                   		 else if (reactorChannel._checkedPreferredHostInChannelList_WSBEnabled
			                   				&& reactorChannel.getReactorConnectOptions().connectionList().size() > 0)
			                   		 {
			                   			checkChannelList(reactorChannel);
			                   		 }
		                   			// If we are at the end of our WSB Group list, checked our preferred WSB Group, and need to check preferred ChannelList
			                   		 else if (!reactorChannel._switchingToPreferredHost && !reactorChannel._checkedPreferredHostInChannelList_WSBEnabled
			                   				&& reactorChannel.getReactorConnectOptions().connectionList().size() > 0)
			                   		 {
			                   			checkPreferredChannelList(reactorChannel);
			                   		 }
		                   			// If there is no connection list, we've already checked through all WSB groups, then rollback our current WSB group index
			                   		 else if (reactorChannel.getReactorConnectOptions().connectionList().size() == 0 &&
			                   				(reactorChannel.warmStandByHandlerImpl.previousWarmStandbyGroupIndex() + 1) >= reactorChannel.warmStandByHandlerImpl.warmStandbyGroupList().size())
			                   		 {
			                   			reactorChannel._switchingToPreferredWSBGroup = false;
			                   			
			                   			rollbackWsbGroupCurrentIndex(reactorChannel);
			                   			
			                   			wsbServerImpl = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl().startingActiveServer();
			                   			
			                   			ReactorWarmStandbyGroupImpl wsbGroup = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl();
			                   			reactorChannel.setCurrentReactorConnectInfo(wsbGroup.startingActiveServer().reactorConnectInfo());
	                            		reactorChannel.setCurrentConnectOptionsInfo(wsbGroup.startingConnectOptionsInfo);
	                            		reactorChannel.userSpecObj(reactorChannel.getCurrentReactorConnectInfo().connectOptions().userSpecObject());
			                   			
			                   			// Set expected channel state
			                   			reactorChannel.warmStandByHandlerImpl
			                   				.warmStandbyHandlerState(ReactorWarmStandbyHandlerState.CONNECTING_TO_A_STARTING_SERVER);
			                   			reactorChannel.warmStandByHandlerImpl.startingReactorChannel()
			                   				.reactorChannelType(ReactorChannelType.WARM_STANDBY);
			                   			
			                    		// Handle service based options
			                        	if (reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl().warmStandbyMode() == ReactorWarmStandbyMode.SERVICE_BASED)
			                    		{
			                        		reactorChannel.copyActiveServiceOptions();
			                    		}
			                   			
			                            if (_reactor._reactorOptions.debuggerOptions().debugConnectionLevel()) {
			                                _reactor.debugger.writeDebugInfo("Switching to warmStandbyGroup index: " + 
			                                		reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex(),
			                                        _reactor.hashCode(),
			                                        this.hashCode()
			                                );
			                            }
			                   		 }
		                   		}
	
	                        	if (!reactorChannel.warmStandByHandlerImpl.startingReactorChannel()._preferredHostOptions.isPreferredHostEnabled())
	                        	{
	                        		if (reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex() + 1 < reactorChannel.warmStandByHandlerImpl.warmStandbyGroupList().size())
	                        		{
	                        			reactorChannel.warmStandByHandlerImpl.incrementWarmStandbyGroupIndex();
	                        			
		                            	ReactorWarmStandbyGroupImpl wsbGroup = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl();
		                            	if(reactorChannel.isStartingServerConfig)
		                            	{
		                            		wsbServerImpl = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl().startingActiveServer();
		                            		reactorChannel.setCurrentReactorConnectInfo(wsbGroup.startingActiveServer().reactorConnectInfo());
		                            		reactorChannel.setCurrentConnectOptionsInfo(wsbGroup.startingConnectOptionsInfo);
		                            		reactorChannel.userSpecObj(reactorChannel.getCurrentReactorConnectInfo().connectOptions().userSpecObject());
		                            		
		                            		reactorChannel.standByGroupListIndex = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex();
		                            	
		                            	}
		                            	else
		                            	{
		                            		wsbServerImpl = reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl().standbyServerList().get(reactorChannel.standByServerListIndex);
		                            		reactorChannel.setCurrentReactorConnectInfo(wsbGroup.standbyServerList().get(reactorChannel.standByServerListIndex).reactorConnectInfo());
		                            		reactorChannel.setCurrentConnectOptionsInfo(wsbGroup.standbyConnectOptionsInfoList.get(reactorChannel.standByServerListIndex));
		                            		reactorChannel.userSpecObj(reactorChannel.getCurrentReactorConnectInfo().connectOptions().userSpecObject());
		                            	}
		                            	
			                    		// Handle service based options
			                        	if (reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl().warmStandbyMode() == ReactorWarmStandbyMode.SERVICE_BASED)
			                    		{
			                        		reactorChannel.copyActiveServiceOptions();
			                    		}
	                        		}
	                        		else if (reactorChannel.getReactorConnectOptions().connectionList().size() > 0 &&
	                        				reactorChannel._connectOptionsInfoList.size() > 0)
	                        		{
	                        			// Move to channel list
	                        			if ((reactorChannel.warmStandByHandlerImpl.warmStandbyHandlerState()
	                        					& ReactorWarmStandbyHandlerState.MOVE_TO_CHANNEL_LIST) == 0)
	                        			{
	                        				reactorChannel.warmStandByHandlerImpl.setMoveToChannelListState();
	                        				reactorChannel._reconnectAttempts = 1;
	    	
	                        				reactorChannel.warmStandByHandlerImpl.startingReactorChannel()
	    											.reactorChannelType(ReactorChannelType.NORMAL);
	                        			}
	                        			
	                        			if (reactorChannel.getReactorConnectOptions().connectionList().size() > 1)
	                        			 {
    	                            		 if (reactorChannel._listIndex == reactorChannel.getReactorConnectOptions().connectionList().size())
    	                            		 {
    	                            			 reactorChannel. _listIndex = 0;
    	                            		 }
	                        			 }

	                        			if (reactorChannel.getReactorConnectOptions().connectionList().size() > 0)
	                        			{
		                            		 reactorChannel.setCurrentReactorConnectInfo(reactorChannel.getReactorConnectOptions().connectionList().get(reactorChannel._listIndex));
		                            		 reactorChannel.setCurrentConnectOptionsInfo(reactorChannel._connectOptionsInfoList.get(reactorChannel._listIndex));
		                            		 reactorChannel._listIndex++;
		                            		 reactorChannel._haveAttemptedFirstConnection = true;
	                        			}
	                        		}
	                        	}
	                        	
	                        	/* Channel has already been closed and cleaned up in the main thread, so we're just removing it from the worker's queues here */
	                        	if(wsbServerImpl != null && wsbServerImpl.isActiveChannelConfig() == false)
	                        	{
	                                _reconnectingChannelQueue.remove(reactorChannel);
	                        	}
                            }
                            else
                            {
                            	if (!reactorChannel._haveAttemptedFirstConnection)
                            	{
                            		reactorChannel._haveAttemptedFirstConnection = true;
                            	}
                            	else
                            		reactorChannel._reconnectAttempts++;
                            }
                            
                        }
                        else
                        {   
                        	 if (reactorChannel.state() != State.EDP_RT &&
                                     reactorChannel.state() != State.EDP_RT_DONE &&
                                     reactorChannel.state() != State.EDP_RT_FAILED)
                             {
                        		 // If PreferredHost is enabled, we're not on preferred host, and we haven't already tried to reconnect to it
                        		 if (reactorChannel._reactorConnectOptions._reactorPreferredHostOptions.isPreferredHostEnabled() &&
                        				 reactorChannel.getReactorConnectOptions().connectionList().size() > 0 &&
                        				 reactorChannel.getCurrentReactorConnectInfo() != reactorChannel.getReactorConnectOptions().connectionList().get(reactorChannel.getReactorConnectOptions().reactorPreferredHostOptions().connectionListIndex()) &&
                        				 !reactorChannel._checkedPreferredHostInChannelList)
                        		 {
                             		// Check if this is the secondary server of a WSB Group
                             		//	In this case, if the primary is down, abandon this and remove it from queue
                             		if (reactorChannel.warmStandByHandlerImpl != null 
                             				&& (reactorChannel.warmStandByHandlerImpl.startingReactorChannel().channel() == null
                             				|| reactorChannel.warmStandByHandlerImpl.startingReactorChannel().channel().state() == ChannelState.CLOSED))
                             		{
                             			_reconnectingChannelQueue.remove(reactorChannel);
                             			reactorChannel.close(null);
                             			continue;
                             		}
                        			 
                        			 reactorChannel._checkedPreferredHostInChannelList = true;
                        			 reactorChannel.setCurrentReactorConnectInfo(reactorChannel.getReactorConnectOptions().connectionList().get(reactorChannel.getReactorConnectOptions().reactorPreferredHostOptions().connectionListIndex()));
                            		 reactorChannel.setCurrentConnectOptionsInfo(reactorChannel._connectOptionsInfoList.get(reactorChannel.getReactorConnectOptions().reactorPreferredHostOptions().connectionListIndex()));
                        		 }
                        		 else
                        		 {
                        			 reactorChannel._checkedPreferredHostInChannelList = false;
                        			 // Check if our connection list has more than one connection and rotate through them appropriately
                        			 if (reactorChannel.getReactorConnectOptions().connectionList().size() > 1)
                        			 {
                        				 // No preferred host enabled, check if we've already attempted first entry and move through list properly
                        				 // Check if our current connection is on the connectionList, but not on the preferred host. In this case, set that we have attempted first connection list entry
                        				 //		in the case that the connection was our first outside reconnection and we hadn't set it yet.
                        				 for (ReactorConnectInfo reactorConnectInfo : reactorChannel.getReactorConnectOptions().connectionList())
                        				 {
                        					 if (reactorConnectInfo == reactorChannel.getCurrentReactorConnectInfo()
                        							 && reactorConnectInfo != reactorChannel.getReactorConnectOptions().connectionList().get(reactorChannel._preferredHostOptions.connectionListIndex()))
                        					 {
                        						 reactorChannel._haveAttemptedFirstConnectionListEntry = true;
                        					 }
                        				 }

                        				 if (!reactorChannel._preferredHostOptions.isPreferredHostEnabled())
                        				 {
                        					 // We have already attempted our first entry (either from connectionList only, or from disabling PH with ioctl later)
                        					 if (reactorChannel._haveAttemptedFirstConnectionListEntry)
                        					 {
                        						 if (++reactorChannel._listIndex == reactorChannel.getReactorConnectOptions().connectionList().size())
        	                            		 {
        	                            			 reactorChannel. _listIndex = 0;
        	                            		 }

        	                            		 reactorChannel.setCurrentReactorConnectInfo(reactorChannel.getReactorConnectOptions().connectionList().get(reactorChannel._listIndex));
        	                            		 reactorChannel.setCurrentConnectOptionsInfo(reactorChannel._connectOptionsInfoList.get(reactorChannel._listIndex));
                        					 }
                        					 else // We have yet to attempt our first entry
                        					 {
                        						 if (reactorChannel._listIndex == reactorChannel.getReactorConnectOptions().connectionList().size())
        	                            		 {
        	                            			 reactorChannel. _listIndex = 0;
        	                            		 }

        	                            		 reactorChannel.setCurrentReactorConnectInfo(reactorChannel.getReactorConnectOptions().connectionList().get(reactorChannel._listIndex));
        	                            		 reactorChannel.setCurrentConnectOptionsInfo(reactorChannel._connectOptionsInfoList.get(reactorChannel._listIndex));
        	                            		 reactorChannel._listIndex++;
        	                            		 reactorChannel._haveAttemptedFirstConnectionListEntry = true;
                        					 }
                        				 }
                        				 else	// Preferred Host is enabled
                        				 {
                        					// We have already attempted our first entry  (either from connectionList only, or from disabling PH with ioctl later)
                        					 if (reactorChannel._haveAttemptedFirstConnectionListEntry)
                        					 {
                        						 if (++reactorChannel._listIndex == reactorChannel.getReactorConnectOptions().connectionList().size())
        	                            		 {
        	                            			 reactorChannel. _listIndex = 0;
        	                            		 }
        	                        			 if ( reactorChannel._listIndex == reactorChannel._reactorConnectOptions.reactorPreferredHostOptions().connectionListIndex())
        	                        			 {
        	                        				 // Skip preferred host entry, then check again if we're over the connection list size to reset index to 0
        	                        				 reactorChannel._listIndex++;
        	                        				 
        	                        				 if (reactorChannel._listIndex == reactorChannel.getReactorConnectOptions().connectionList().size())
                                    				 {
                                            			 reactorChannel._listIndex = 0;
                                            		 }
        	                        			 }
        	                        			 
        	                            		 reactorChannel.setCurrentReactorConnectInfo(reactorChannel.getReactorConnectOptions().connectionList().get(reactorChannel._listIndex));
        	                            		 reactorChannel.setCurrentConnectOptionsInfo(reactorChannel._connectOptionsInfoList.get(reactorChannel._listIndex));
                        					 }
                        					 else	// We have yet to attempt our first entry
                        					 {
                        						 // Skip preferred host index that we are on right now
                        						 if (reactorChannel._listIndex == reactorChannel._reactorConnectOptions.reactorPreferredHostOptions().connectionListIndex())
        	                        				 reactorChannel._listIndex++;
                        						 // Check if we're over the connection list size, reset index to 0 if so
        	                    				 if (reactorChannel._listIndex == reactorChannel.getReactorConnectOptions().connectionList().size())
        	                    				 {
        	                            			 reactorChannel. _listIndex = 0;
        	                            		 }
                                				 reactorChannel. _haveAttemptedFirstConnectionListEntry = true;

                                        		 reactorChannel.setCurrentReactorConnectInfo(reactorChannel.getReactorConnectOptions().connectionList().get(reactorChannel._listIndex));
                                        		 reactorChannel.setCurrentConnectOptionsInfo(reactorChannel._connectOptionsInfoList.get(reactorChannel._listIndex));
                        					 }
                        				 }
                        			 }
                        		 }
                        		 
                    			 reactorChannel._reconnectAttempts++;
                            }
                        }

                        if (reactorChannel.reconnectAttemptLimit() == 0)
                        {
                        	// We've always attempted first connection in these cases, we need to always move to next channel
                        	reactorChannel._haveAttemptedFirstConnection = true;
                        }

                        // Check if we have an active preferred host channel ready to switch to
                        if (reactorChannel.preferredHostChannel() != null && reactorChannel.preferredHostChannel().state() == ChannelState.ACTIVE)
                        {
                        	channel = reactorChannel.preferredHostChannel();
                        	// Replace channel of reactorChannel with the new one
                 			reactorChannel.selectableChannelFromChannel(reactorChannel.preferredHostChannel());
                 			// We've used our preferred host channel, reset it to null
                        	reactorChannel.preferredHostChannel(null); 	
                        	
                        	// Remove from reconnecting queue and reset the reactor channel's ping handler timeout and retry count
                        	_reconnectingChannelQueue.remove(reactorChannel);
                            reactorChannel.pingHandler().initPingHandler(channel.pingTimeout());
                            reactorChannel.resetCurrentChannelRetryCount();
                            reactorChannel.state(State.READY);
                            // Add reactor channel to active queue and send worker event for channel up
                            _activeChannelQueue.add(reactorChannel);
                            sendWorkerEvent(reactorChannel, WorkerEventTypes.CHANNEL_UP,
                                    ReactorReturnCodes.SUCCESS, null, null);

                        }
                        else
                        {
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
    
    private void checkChannelList(ReactorChannel reactorChannel)
    {
    	// We've reached the end of our WSB Groups so we are trying ChannelList
		reactorChannel._switchingToPreferredWSBGroup = false;
		reactorChannel._switchingToPreferredHost = false;
		
		// Reset WSB Groups to beginning
		reactorChannel._hitEndOfWSBGroups = true;
		if (reactorChannel._preferredHostOptions.warmStandbyGroupListIndex() > 0)
			rollbackWsbGroupCurrentIndex(reactorChannel);
		
		
		// Next time we need to check preferred ChannelList
		reactorChannel._checkedPreferredHostInChannelList_WSBEnabled = false;
		
		reactorChannel._phSwitchingFromWSBToChannelList = true;

		// Check if we are on preferred channel, in which case, skip it
		if (reactorChannel._listIndex == reactorChannel._preferredHostOptions.connectionListIndex())
		{
			// If our next ChannelList channel is out of bounds, reset it
			if (++reactorChannel._listIndex == reactorChannel.getReactorConnectOptions().connectionList().size())
			{
				reactorChannel. _listIndex = 0;
			}
		}
		
		// Set that we have attempted our first channel list entry
		reactorChannel._haveAttemptedFirstConnectionListEntry = true;
		 
		// Set to next ChannelList channel
		reactorChannel.setCurrentReactorConnectInfo(reactorChannel.getReactorConnectOptions().connectionList().get(reactorChannel._listIndex));
		reactorChannel.setCurrentConnectOptionsInfo(reactorChannel._connectOptionsInfoList.get(reactorChannel._listIndex));
			 
		// If our next ChannelList channel is out of bounds, reset it
		if (++reactorChannel._listIndex == reactorChannel.getReactorConnectOptions().connectionList().size())
		{
			reactorChannel. _listIndex = 0;
		}
		
		// We only attempt connectionList connections once, so set reconnectionAttempt to max and retry next channel after
		reactorChannel._reconnectAttempts = reactorChannel.reconnectAttemptLimit() - 1;
		reactorChannel._haveAttemptedFirstConnection = true;
		
		// Set expected channel state
		reactorChannel.warmStandByHandlerImpl
			.warmStandbyHandlerState(ReactorWarmStandbyHandlerState.MOVE_TO_CHANNEL_LIST);
		reactorChannel.warmStandByHandlerImpl.startingReactorChannel()
			.reactorChannelType(ReactorChannelType.NORMAL);
    }
    
    private void checkPreferredChannelList(ReactorChannel reactorChannel)
    {
    	// We've reached the end of our WSB Groups so we are checking ChannelList, specifically preferred ChannelList channel
		reactorChannel._switchingToPreferredWSBGroup = false;
		reactorChannel._switchingToPreferredHost = true;
		
		// Reset WSB Groups to beginning of non-PH groups
		reactorChannel._hitEndOfWSBGroups = true;
		if (reactorChannel._preferredHostOptions.warmStandbyGroupListIndex() > 0)
			rollbackWsbGroupCurrentIndex(reactorChannel);
		
		// Next time we don't need to check preferred ChannelList channel
		reactorChannel._checkedPreferredHostInChannelList_WSBEnabled = true;
		
		// Set that we have attempted our first channel list entry
		reactorChannel._haveAttemptedFirstConnectionListEntry = true;
		
		reactorChannel._phSwitchingFromWSBToChannelList = true;

		// Set to preferred ChannelList channel
		reactorChannel.setCurrentReactorConnectInfo(reactorChannel.getReactorConnectOptions().connectionList().get(reactorChannel._preferredHostOptions.connectionListIndex()));
		reactorChannel.setCurrentConnectOptionsInfo(reactorChannel._connectOptionsInfoList.get(reactorChannel._preferredHostOptions.connectionListIndex()));

		// We only attempt connectionList connections once, so set reconnectionAttempt to max and retry next channel after
		reactorChannel._reconnectAttempts = reactorChannel.reconnectAttemptLimit() - 1;
		reactorChannel._haveAttemptedFirstConnection = true;
		
		// Set expected channel state
		reactorChannel.warmStandByHandlerImpl
			.warmStandbyHandlerState(ReactorWarmStandbyHandlerState.MOVE_TO_CHANNEL_LIST);
		reactorChannel.warmStandByHandlerImpl.startingReactorChannel()
			.reactorChannelType(ReactorChannelType.NORMAL);
    }
    
    void rollbackWsbGroupCurrentIndex(ReactorChannel reactorChannel)
    {
		if (reactorChannel._preferredHostOptions.warmStandbyGroupListIndex() == 0
				&& reactorChannel.warmStandByHandlerImpl.warmStandbyGroupList().size() > 1)
		{
			reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex(1);
			reactorChannel.warmStandByHandlerImpl.previousWarmStandbyGroupIndex(0);
		}
		else
		{
			reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupIndex(0);
			reactorChannel.warmStandByHandlerImpl.previousWarmStandbyGroupIndex(-1);
		}
    }
    
    private void sendPreferredHostCompleteEvent(ReactorChannel reactorChannel)
    {
		sendWorkerEvent(reactorChannel, WorkerEventTypes.PREFERRED_HOST_COMPLETE, ReactorReturnCodes.SUCCESS, "Worker.sendPreferredHostCompleteEvent", null);
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
                if (reactorChannel.server() == null && (!event.reactorChannel().recoveryAttemptLimitReached()))
                {
                    /* Go into connection recovery. */
                    if (reactorChannel.nextRecoveryTime() < System.currentTimeMillis()) // We are not in connection recovery already and will not overwrite any reconnect time
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
            case PREFERRED_HOST_TIMER:
            	if (event.reactorChannel()._preferredHostOptions.isPreferredHostEnabled())
            	{
            		// If timer event already exists, then we are calling ioctl to change it. Cancel the old PH timer event.
            		if(event.reactorChannel()._currentPHTimerEvent != null)
            			event.reactorChannel()._currentPHTimerEvent._isCanceled = true;
 
            		 if (event.reactorChannel()._preferredHostOptions.detectionTimeSchedule() != null &&
            				 !event.reactorChannel()._preferredHostOptions.detectionTimeSchedule().isEmpty())
            		 {
						event.reactorChannel()._cronCurrentTime = new Date(System.currentTimeMillis());
						event.reactorChannel()._cronNextTime = event.reactorChannel()._cronExpression.getNextValidTimeAfter(event.reactorChannel()._cronCurrentTime);
           		 		event.reactorChannel()._nextReconnectTimeMs = event.reactorChannel()._cronNextTime.getTime();
           		 		event.timeout(System.nanoTime() + (event.reactorChannel()._cronNextTime.getTime() - event.reactorChannel()._cronCurrentTime.getTime()) * 1000000);
           		 		event.reactorChannel()._currentPHTimerEvent = event; // Update with the current PH timer event
						_timerEventQueue.add(event); 
            		 }
            		else if (event.reactorChannel()._preferredHostOptions.detectionTimeInterval() > 0)
         		 	{
         		 		event.reactorChannel()._nextReconnectTimeMs = System.currentTimeMillis() + event.reactorChannel()._preferredHostOptions.detectionTimeInterval() * 1000;
         		 		event.timeout(System.nanoTime() + (event.reactorChannel()._preferredHostOptions.detectionTimeInterval() * 1000000000));
         		 		event.reactorChannel()._currentPHTimerEvent = event; // Update with the current PH timer event
         		 		_timerEventQueue.add(event); 
         		 		return;
         		 	}
            	}
            	else
            	{
            		/* Cancels the existing one if any from calling ioctl */
            		if(event.reactorChannel()._currentPHTimerEvent != null)
            			event.reactorChannel()._currentPHTimerEvent._isCanceled = true;
            	}
            	
            	return;
            case PREFERRED_HOST_START_FALLBACK:
   		 		_timerEventQueue.add(event); 
   		 		return;
            case PREFERRED_HOST_IOCTL:
        		// Put event into timerEventQueue so we can handle it when we are ready
        		_timerEventQueue.add(event); 
        		return;
            case PREFERRED_HOST_CHANNEL_CLOSE:
            	processChannelClose(reactorChannel);
                if(reactorChannel.warmStandByHandlerImpl != null)
                {
                	/* Remove the channel from any session management lists */
            		_reactor.removeReactorChannel(reactorChannel);

                }
            	sendWorkerEvent(reactorChannel, WorkerEventTypes.PREFERRED_HOST_CHANNEL_CLOSE_ACK,
                        ReactorReturnCodes.SUCCESS, null, null);
            	break;
            case PREFERRED_HOST_CHANNEL_DOWN:
            	processChannelClose(reactorChannel);	
                if(reactorChannel.warmStandByHandlerImpl != null)
                {
                	/* Remove the channel from any session management lists */
            		_reactor.removeReactorChannel(reactorChannel);

                }
            	break;
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
                
                reactorChannel._haveAttemptedFirstConnection = true;
                
                // If Preferred Host enabled, reset our state to say we've switched
                reactorChannel._switchingToPreferredHost = false;
                reactorChannel._switchingToPreferredWSBGroup = false;

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

                    if (reactorChannel.reactor() != null)
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

                    if (reactorChannel.reactor() != null)
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
