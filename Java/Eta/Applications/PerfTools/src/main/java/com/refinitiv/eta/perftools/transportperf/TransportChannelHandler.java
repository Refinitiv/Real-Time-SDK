/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.transportperf;

import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Date;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;

import com.refinitiv.eta.perftools.common.ClientChannelInfo;
import com.refinitiv.eta.perftools.common.PerfToolsReturnCodes;
import com.refinitiv.eta.perftools.common.ShutdownCallback;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InProgFlags;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.ReadArgs;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;

/**
 * Performs the associated with setting up and using ETA Transport Channels, such as
 * initializing channels, reading, flushing, and checking ping timeouts.
 */
public class TransportChannelHandler
{
    private Queue<ClientChannelInfo> _activeChannelList;       // List of channels that are active.
    private Queue<ClientChannelInfo> _initializingChannelList; // List of initializing channels.
    private TransportThread _transportThread;                 // Reference to application-specified data.
    private Object _userSpec;                                 // Reference to application-specified data.


    public Selector _selector;

    private Error _error;
    private ReadArgs _readArgs;
    private InProgInfo _inProgInfo;
    
    private boolean atLeastOneChannel = false;  // set when we have at least one channel to process.

    private ShutdownCallback _shutdownCallback; // shutdown callback to main application

    private boolean _isShutdown;

    /**
     * Instantiates a new transport channel handler.
     *
     * @param shutdownCallback the shutdown callback
     * @param error the error
     */
    public TransportChannelHandler(ShutdownCallback shutdownCallback, Error error)
    {
        _activeChannelList = new LinkedList<>();
        _initializingChannelList = new LinkedList<>();
        _error = error;
        _readArgs = TransportFactory.createReadArgs();
        _inProgInfo = TransportFactory.createInProgInfo();
        _shutdownCallback = shutdownCallback;
    }

    /**
     * Initialize channel handler for a provider thread.
     * 
     * @param transportThread - transport thread
     */
    public void init(TransportThread transportThread)
    {
        _transportThread = transportThread;
        try
        {
            _selector = Selector.open();
        }
        catch (Exception exception)
        {
            System.err.println("Error initializing application: Unable to open selector: " + exception.getMessage());
            System.exit(-1);
        }
    }

    /**
     * Cleans up a ChannelHandler.
     *
     * @param error the error
     */
    public void cleanup(Error error)
    {
        for (ClientChannelInfo channelInfo : _activeChannelList)
        {
            closeChannel(channelInfo, error);
        }
    }

    /**
     * Closes and removes a channel from the channelHandler.
     * 
     * @param clientChannelInfo - client channel to close.
     * @param error If channel becomes inactive because of an error, this object
     *            gives detailed information about the error.
     */
    void closeChannel(ClientChannelInfo clientChannelInfo, Error error)
    {
        _transportThread.processInactiveChannel(this, clientChannelInfo, error);
        clientChannelInfo.channel.close(error);
        clientChannelInfo.parentQueue.remove(clientChannelInfo);
        
        // if there is no more channels, reset atLeastOneChannel variable.
        if (_activeChannelList.size() == 0 && _initializingChannelList.size() == 0)
            atLeastOneChannel = false;
        
        if (!atLeastOneChannel)
            _shutdownCallback.shutdown();
    }

    /**
     * Adds a connected or accepted channel to the ChannelHandler.
     * 
     * @param channel - connected or accepted channel
     * @param checkPings - flag to check pings or not
     * 
     * @return client channel information for the connected or accepted channel
     */
    public ClientChannelInfo addChannel(Channel channel, boolean checkPings)
    {
        ClientChannelInfo channelInfo = new ClientChannelInfo();
        channelInfo.checkPings = checkPings;
        channelInfo.parentQueue = _initializingChannelList;
        channelInfo.channel = channel;
        channelInfo.needRead = false;
        channelInfo.userSpec = new TransportSession(channelInfo);
        _initializingChannelList.add(channelInfo);

        try
        {
            if (channel.connectionType() == ConnectionTypes.SEQUENCED_MCAST)	// We do not use OP_CONNECT SelectionKey for Sequenced Multicast registration
            {
            	channelInfo.channel.selectableChannel().register(_selector, SelectionKey.OP_READ, channelInfo);
            }
            else
            	channelInfo.channel.selectableChannel().register(_selector, SelectionKey.OP_READ | SelectionKey.OP_CONNECT, channelInfo);
        }
        catch (ClosedChannelException e)
        {
            System.err.println(e.getMessage());
            _isShutdown = true;
            _shutdownCallback.shutdown();
            return null;
        }

        if (channel.state() == ChannelState.ACTIVE)
        {
            // Channel is active
            processActiveChannel(channelInfo);
        }

        atLeastOneChannel = true;
        
        return channelInfo;

    }

    /**
     * read data from channels until stopTimeNsec is reached.
     * 
     * @param channelInfo - channel information
     * @param stopTimeNsec - stop time in nano seconds. (stopTimeNsec should be based on System.nanoTime()).
     * @param error Gives detailed information about error if any occurred during socket operations.
     * @return true if data is read.
     */
    public boolean readChannel(ClientChannelInfo channelInfo, long stopTimeNsec, Error error)
    {
        if (!_isShutdown)
        {
        int readReturn = readFromChannel(channelInfo, stopTimeNsec, error);

        if ((readReturn < PerfToolsReturnCodes.SUCCESS && channelInfo.channel.state() == ChannelState.ACTIVE))
        {
            System.err.printf("readFromChannel() failed with return code %d <%s>, exiting...\n", readReturn, error.text());
            return false;
        }
        else if (readReturn > PerfToolsReturnCodes.SUCCESS)
        {
            channelInfo.needRead = true;
        }
        else
        {
            channelInfo.needRead = false;
        }
        }
        
        return true;
    }
    
    private int readFromChannel(ClientChannelInfo clientChannelInfo, long stopTimeNsec, Error error)
    {
        int ret = 1;
        int readCount = -1; // allow at least one read

        _readArgs.clear();

        // Read until eta read() indicates that no more bytes are available in
        // the queue.
        do
        {
            if (readCount % 10 == 0)
            {
                if (System.nanoTime() >= stopTimeNsec)
                    return ret;
            }

            TransportBuffer msgBuffer = clientChannelInfo.channel.read(_readArgs, error);
            ret = _readArgs.readRetVal();
            if (msgBuffer != null)
            {           
                // Mark that we received data for ping timeout handling.
                clientChannelInfo.receivedMsg = true;
 
                // Received a Msg Buffer, call the application's processing
                // method.
                if (_transportThread.processMsg(this, clientChannelInfo, msgBuffer, error) < PerfToolsReturnCodes.SUCCESS)
                {
                    ret = TransportReturnCodes.FAILURE;
                    break;
                }
            }
            ++readCount;
        }
        while (ret > TransportReturnCodes.SUCCESS);

        switch (ret)
        {
            case TransportReturnCodes.SUCCESS:
            case TransportReturnCodes.READ_WOULD_BLOCK:
            case TransportReturnCodes.READ_FD_CHANGE:
                return PerfToolsReturnCodes.SUCCESS;
            case TransportReturnCodes.READ_PING:
                clientChannelInfo.receivedMsg = true; // Mark that we received
                                                      // data for ping timeout
                                                      // handling.
                return PerfToolsReturnCodes.SUCCESS;
            default:
                return PerfToolsReturnCodes.FAILURE;
        }
    }

    /**
     * Tries to read data from channels until stopTimeNsec is
     * reached(stopTimeNsec should be based on System.nanoTime()). Also
     * initializes any new channels.
     * 
     * @param stopTimeNsec - stop time in nano seconds
     * @param error Gives detailed information about error if any occurred
     *            during socket operations.
     * @return negative value in case of error, &gt;=0 if no errors.
     */
    public int readChannels(long stopTimeNsec, Error error)
    {
        long currentTime;
        int selRet = 0;

        // Loop on select(), looking for channels with available data, until
        // stopTime is reached.
        // Add active channels to the descriptor sets.
        try 
        {
            for (ClientChannelInfo channelInfo : _activeChannelList)
            {
                if (channelInfo.needFlush)
                    addOption(channelInfo.channel, SelectionKey.OP_WRITE, channelInfo);
                else
                    removeOption(channelInfo.channel, SelectionKey.OP_WRITE, channelInfo);
                
                if (channelInfo.needRead)
                {
                    if (!readChannel(channelInfo, stopTimeNsec, error))
                        return PerfToolsReturnCodes.FAILURE;
                }
            }
    
            // Add initializing channels to the descriptor sets.
            for (ClientChannelInfo channelInfo : _initializingChannelList)
            {
                if (channelInfo.needFlush)
                    addOption(channelInfo.channel, SelectionKey.OP_WRITE, channelInfo);
                else
                    removeOption(channelInfo.channel, SelectionKey.OP_WRITE, channelInfo);
                
                if (channelInfo.needRead)
                {
                    if (!readChannel(channelInfo, stopTimeNsec, error))
                        return PerfToolsReturnCodes.FAILURE;
                }
            }
        
            do
            {
                currentTime = System.nanoTime();
                try
                {
                    long selTime = atLeastOneChannel ? ((stopTimeNsec - currentTime) / 1000000) : 1L;
                    if (selTime <= 0)
                        selRet = _selector.selectNow();
                    else
                        selRet = _selector.select(selTime);
                }
                catch (Exception e)
                {
                    System.err.println(e.getMessage());
                    return PerfToolsReturnCodes.FAILURE;
                }
    
                if (selRet == 0) // timeout
                {
                    return PerfToolsReturnCodes.SUCCESS;
                }
                else if (selRet > 0) // something to read
                {
                    Set<SelectionKey> keySet = _selector.selectedKeys();
                    Iterator<SelectionKey> iter = keySet.iterator();
                    int ret;
                    // Check if each channel has something to read.
                    while (iter.hasNext())
                    {
                        SelectionKey key = iter.next();
                        iter.remove();
                        if (!key.isValid())
                            continue;
    
                        ClientChannelInfo channelInfo = (ClientChannelInfo)key.attachment();
                        if (channelInfo.parentQueue == _activeChannelList)
                        {
                            if (key.isWritable())
                            {
                                // Check if we can flush data from the channel.
                                if ((ret = channelInfo.channel.flush(error)) < TransportReturnCodes.SUCCESS)
                                {
                                    continue;
                                }
                                else if (ret == TransportReturnCodes.SUCCESS)
                                {
                                    // ETA flush() returned 0 instead of a higher
                                    // value, so there's no more data to flush.
                                    _flushDone(channelInfo);
                                }
                            }
                            // active channel
                            if (key.isReadable())
                            {
                                if (!readChannel(channelInfo, stopTimeNsec, error))
                                    return PerfToolsReturnCodes.FAILURE;
                            }
                        }
                        else if (channelInfo.parentQueue == _initializingChannelList)
                        {
                            if (key.isReadable() || key.isWritable() || key.isConnectable())
                            {
                                _flushDone(channelInfo);
                                initializeChannel(channelInfo, error);
                            }
                        }
                    }
                }
                else
                {
                    System.err.println("Selector.select() error");
                    return PerfToolsReturnCodes.FAILURE;
                }
    
            }
            while (currentTime < stopTimeNsec);
        }
        catch (Exception e)
        {
            System.err.println(e.getMessage());
            return PerfToolsReturnCodes.FAILURE;
        }

        return PerfToolsReturnCodes.SUCCESS;
    }

    /**
     * Try to initialize a single channel.
     * 
     * @param clientChannelInfo - client channel information
     * @param error Gives detailed information about error if any occurred
     *            during socket operations.
     * @return negative value in case of error, &gt;=0 if no errors.
     */
    public int initializeChannel(ClientChannelInfo clientChannelInfo, Error error)
    {
        int ret = PerfToolsReturnCodes.SUCCESS;
        _flushDone(clientChannelInfo);

        Channel channel = clientChannelInfo.channel;

        switch (channel.init(_inProgInfo, error))
        {
            case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                if (_inProgInfo.flags() == InProgFlags.SCKT_CHNL_CHANGE)
                    requestFlush(clientChannelInfo);
                ret = TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
                break;
            case TransportReturnCodes.SUCCESS:
                processActiveChannel(clientChannelInfo);
                break;
            default:
                ret = PerfToolsReturnCodes.FAILURE;
                _isShutdown = true;
                _shutdownCallback.shutdown();
                break;
        }
        return ret;
    }

    /**
     * Attempts initialization on the given channel for the specified period of
     * time.
     * 
     * @param clientChannelInfo - client channel info
     * @param waitTimeMilliSec - wait time in milli seconds to try initialize
     *            channel
     * @param error - error information populated when error occurs
     * @return &lt; 0 if there is any error,
     *         TransportReturnCodes.CHAN_INIT_IN_PROGRESS (2) if channel
     *         initialization in progress
     */
    public int waitForChannelInit(ClientChannelInfo clientChannelInfo, int waitTimeMilliSec, Error error)
    {
        try
        {
            //
            // needFlush indicates if we want to call eta initChannel()
            // immediately.
            //
            if (clientChannelInfo.channel.connectionType() == ConnectionTypes.SEQUENCED_MCAST)	// Sequenced Multicast only uses Read/Write SelectionKeys on Register, never CONNECT
            {
            	clientChannelInfo.channel.selectableChannel().register(_selector, SelectionKey.OP_WRITE | SelectionKey.OP_READ, clientChannelInfo);
            }
            else
            {
                if (clientChannelInfo.needFlush)
                {
                    clientChannelInfo.channel.selectableChannel().register(_selector, SelectionKey.OP_WRITE | SelectionKey.OP_READ | SelectionKey.OP_CONNECT, clientChannelInfo);
                }
                else
                {
                    removeOption(clientChannelInfo.channel, SelectionKey.OP_WRITE, clientChannelInfo);
                    clientChannelInfo.channel.selectableChannel().register(_selector, SelectionKey.OP_READ | SelectionKey.OP_CONNECT, clientChannelInfo);
                }
            }

            /* Call select without checking return value here. The reason is there could
             * be pending notifications after registering selector above. These need to be
             * removed so subsequent selector calls work properly.
             */
            _selector.select(waitTimeMilliSec);
            Set<SelectionKey> keySet = _selector.selectedKeys();
            Iterator<SelectionKey> iter = keySet.iterator();
            // Check if channel has notifications pending
            while (iter.hasNext())
            {
                SelectionKey key = iter.next();
                iter.remove();
                if (!key.isValid())
                    continue;
                // if no notifications pending, return
                if (!key.isReadable() && !key.isWritable() && !key.isConnectable())
                {
                    return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
                }
            }
        }
        catch (Exception e)
        {
            System.err.println(e.getMessage());
            return PerfToolsReturnCodes.FAILURE;
        }

        return initializeChannel(clientChannelInfo, error);
    }

    /**
     * Performs ping timeout checks for channels in this channel handler.
     *
     * @return the int
     */
    public int checkPings()
    {
        if (!_isShutdown)
        {
        long currentTime = System.currentTimeMillis();
        for (ClientChannelInfo channelInfo : _activeChannelList)
        {
            if (!channelInfo.checkPings)
                continue;

            // handle sending pings to clients
            if (currentTime >= channelInfo.nextSendPingTime)
            {
                if (channelInfo.sentMsg)
                {
                    channelInfo.sentMsg = false;
                }
                else
                {
                    /* send ping to remote (connection) */
                    int ret = channelInfo.channel.ping(_error);
                    if (ret > TransportReturnCodes.SUCCESS)
                    {
                        requestFlush(channelInfo);
                    }
                    else if (ret < TransportReturnCodes.SUCCESS)
                    {
                    	if (_error.errorId() == TransportReturnCodes.FAILURE)
                    	{
                    		System.err.println(_error.text());
                    		return TransportReturnCodes.FAILURE;
                    	}
                    }
                }

                // set time to send next ping to client
                channelInfo.nextSendPingTime = currentTime + (channelInfo.channel.pingTimeout() / 3) * 1000;
            }

            // handle receiving pings from client
            if (currentTime >= channelInfo.nextReceivePingTime)
            {
                // check if server received message from client since last time
                if (channelInfo.receivedMsg)
                {
                    // reset flag for client message received
                    channelInfo.receivedMsg = false;

                    // set time server should receive next message/ping from
                    // client
                    channelInfo.nextReceivePingTime = currentTime + channelInfo.channel.pingTimeout() * 1000;
                }
                else
                // lost contact with client
                {
                        System.err.println(new Date() + ": Ping timed out. ping time out:" + (channelInfo.channel.pingTimeout() / 3) * 1000);
                        return TransportReturnCodes.FAILURE;
                }
            }
        }
    }
        return TransportReturnCodes.SUCCESS;
    }

    /*
     * Add interest set option to the channel's registered interest options. 
     */
    private void addOption(Channel channel, int option, Object attachment) throws ClosedChannelException
    {
        SelectionKey key = channel.selectableChannel().keyFor(_selector);
        int newoption = option;
        int oldoption = 0;
        if (key != null)
        {
            oldoption = key.interestOps();
            newoption |= oldoption;
        }
        channel.selectableChannel().register(_selector, newoption, attachment);        
    }

    /*
     * Remove interest set option to the channel's registered interest options. 
     */
    private void removeOption(Channel channel, int option, Object attachment)
            throws ClosedChannelException
    {
        SelectionKey key = channel.selectableChannel().keyFor(_selector);
        if (key == null || !key.isValid())
            return;
        if ((option & key.interestOps()) == 0)
            return;
        int newoption = key.interestOps() - option;
        if (option != 0)
        {
            channel.selectableChannel().register(_selector, newoption, attachment);
        }
        else
        {
            key.cancel();
        }
    }

    /**
     * Requests that the ChannelHandler begin calling eta flush() for a channel.
     * Used when a call to eta write() indicates there is still data to be
     * written to the network.
     *
     * @param clientChannelInfo the client channel info
     */
    public void requestFlush(ClientChannelInfo clientChannelInfo)
    {
        clientChannelInfo.needFlush = true;
    }

    private void processActiveChannel(ClientChannelInfo clientChannelInfo)
    {
        // Set the next send/receive ping times.
        long currentTime = System.currentTimeMillis();

        clientChannelInfo.nextSendPingTime = currentTime + (clientChannelInfo.channel.pingTimeout() / 3) * 1000;
        clientChannelInfo.nextReceivePingTime = currentTime + clientChannelInfo.channel.pingTimeout() * 1000;

        _flushDone(clientChannelInfo);

        int ret = _transportThread.processActiveChannel(this, clientChannelInfo, _error);
        if (ret < PerfToolsReturnCodes.SUCCESS)
        {
            return;
        }

        _initializingChannelList.remove(clientChannelInfo);
        clientChannelInfo.parentQueue = _activeChannelList;
        clientChannelInfo.parentQueue.add(clientChannelInfo);
    }

    /*
     * Mark flush not needed for the channel.
     */
    private void _flushDone(ClientChannelInfo clientChannelInfo)
    {
        clientChannelInfo.needFlush = false;
    }

    /**
     * Transport thread.
     *
     * @return Provider thread for the channel handler
     */
    public TransportThread transportThread()
    {
        return _transportThread;
    }

    /**
     * Active channel list.
     *
     * @return List of channels that are active.
     */
    public Queue<ClientChannelInfo> activeChannelList()
    {
        return _activeChannelList;
    }

    /**
     * Initializing channel list.
     *
     * @return List of initializing channels.
     */
    public Queue<ClientChannelInfo> initializingChannelList()
    {
        return _initializingChannelList;
    }

    /**
     * User spec.
     *
     * @return Reference to application-specified data.
     */
    public Object userSpec()
    {
        return _userSpec;
    }

    /**
     * User spec.
     *
     * @param userSpec Reference to application-specified data.
     */
    public void userSpec(Object userSpec)
    {
        _userSpec = userSpec;
    }
}
