package com.thomsonreuters.upa.perftools.common;

import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Date;
import java.util.Iterator;
import java.util.Queue;
import java.util.Set;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgFlags;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.transport.WritePriorities;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.perftools.upajprovperf.IProviderThread;

/**
 * Performs the associated with setting up and using UPA Channels, such as
 * initializing channels, reading, flushing, and checking ping timeouts.
 */
public class ChannelHandler
{
    private Queue<ClientChannelInfo> _activeChannelList;       // List of channels that are active.
    private Queue<ClientChannelInfo> _initializingChannelList; // List of initializing channels.
    private IProviderThread _providerThread;                   // Reference to application-specified data.

    public Selector _selector;
    
    long _divisor = 1;

    private Error _error;
    private ReadArgs _readArgs;
    private InProgInfo _inProgInfo;
    private WriteArgs _writeArgs;
    
    private ReactorErrorInfo _errorInfo; // Used for when application uses VA Reactor instead of UPA Channel.
    
	private Lock _channelLock = new ReentrantLock();
    
    public ChannelHandler(ProviderThread providerThread)
    {
        _activeChannelList = new ConcurrentLinkedQueue<>();
        _initializingChannelList = new ConcurrentLinkedQueue<>();
        _error = TransportFactory.createError();
        _readArgs = TransportFactory.createReadArgs();
        _writeArgs = TransportFactory.createWriteArgs();
        _inProgInfo = TransportFactory.createInProgInfo();
        _providerThread = (IProviderThread) providerThread;
        _errorInfo = ReactorFactory.createReactorErrorInfo();
        try
        {
            _selector = Selector.open();
        }
        catch (Exception exception)
        {
            System.err.println("Error initializing application: Unable to open selector: " + exception.getMessage());
            System.exit(-1);
        }
        if (ProviderPerfConfig.ticksPerSec() > 1000)
        {
            _divisor = 1000000;
        }
     }

    /**
     * Cleans up a ChannelHandler.
     */
    public void cleanup()
    {
        for (ClientChannelInfo channelInfo : _activeChannelList)
        {
            closeChannel(channelInfo, null);
        }
    }

    /**
     * Closes and removes a channel from the channelHandler.
     * 
     * @param clientChannelInfo - client channel to close.
     * @param error If channel becomes inactive because of an error, this object
     *            gives detailed information about the error.
     */
    public void closeChannel(ClientChannelInfo clientChannelInfo, Error error)
    {
        if (clientChannelInfo.reactorChannel == null) // use UPA Channel
        {
            _providerThread.processInactiveChannel(this, clientChannelInfo, error);
            clientChannelInfo.channel.close(_error);
            clientChannelInfo.parentQueue.remove(clientChannelInfo);
        }
        else // use UPA VA Reactor
        {
            clientChannelInfo.reactorChannel.close(_errorInfo);
            clientChannelInfo.parentQueue.remove(clientChannelInfo);
            System.out.println("Channel Closed.");
        }
	}


    /**
     * Adds a connected or accepted channel to the ChannelHandler.
     * 
     * @param channel - connected or accepted channel
     * @param userSpec - user object associated with the channel
     * @param checkPings - flag to check pings or not
     * 
     * @return client channel information for the connected or accepted channel
     */
    public ClientChannelInfo addChannel(Channel channel, ProviderSession userSpec, boolean checkPings)
    {
        ClientChannelInfo channelInfo = new ClientChannelInfo();
        channelInfo.userSpec = userSpec;
        channelInfo.checkPings = checkPings;
        channelInfo.parentQueue = _initializingChannelList;
        channelInfo.channel = channel;
        channelInfo.needRead = false;
        _initializingChannelList.add(channelInfo);

        if (channel.state() == ChannelState.ACTIVE)
        {
            // Channel is active
            processActiveChannel(channelInfo);
        }
        
        return channelInfo;
    }

    /**
     * Write a buffer to a channel.
     * 
     * @param clientChannelInfo - client channel info to write buffer to.
     * @param msgBuffer - buffer to write
     * @param writeFlags - write flags.
     * @param error - Error, to be populated in event of an error
     * @return TransportReturnCodes or the number of bytes pending flush
     */
    public int writeChannel(ClientChannelInfo clientChannelInfo, TransportBuffer msgBuffer, int writeFlags, Error error)
    {
        _writeArgs.clear();
        _writeArgs.priority(WritePriorities.HIGH);
        _writeArgs.flags(writeFlags);

        // write buffer
        int ret = clientChannelInfo.channel.write(msgBuffer, _writeArgs, error);
        if (ret >= TransportReturnCodes.SUCCESS)
        {
            clientChannelInfo.sentMsg = true;
            return ret;
        }

        switch (ret)
        {
            case TransportReturnCodes.WRITE_FLUSH_FAILED:
                if (clientChannelInfo.channel.state() == ChannelState.ACTIVE)
                {
                    //
                    // Channel is still open, but UPA write() tried to flush
                    // internally and failed. Return positive value so the
                    // caller knows there's bytes to flush.
                    //
                    return TransportReturnCodes.SUCCESS + 1;
                }
                return ret;
            default:
                return ret;
        }
    }

    private int readChannel(ClientChannelInfo channelInfo, long stopTimeNsec, Error error)
    {
        int readReturn = readFromChannel(channelInfo, stopTimeNsec, error);

        if ( readReturn < PerfToolsReturnCodes.SUCCESS)
        {
        	// in case of read failure on a specific
        	// client channel failure,
        	// interactive provider needs to continue
        	// running.
        	return readReturn;
        }
        else if (readReturn > PerfToolsReturnCodes.SUCCESS)
        {
            channelInfo.needRead = true;
        }
        else
        {
            channelInfo.needRead = false;
        }
    	return readReturn;
    }
    
    private int readFromChannel(ClientChannelInfo clientChannelInfo, long stopTime, Error error)
    {
        int ret = 1;
        boolean channelClosed = false;
        int readCount = -1; // allow at least one read

        _readArgs.clear();

        // Read until upa read() indicates that no more bytes are available in
        // the queue.
        do
        {
            long currentTime = currentTime();
            
            if (readCount % 10 == 0)
            {
                if (currentTime >= stopTime)
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
                if (_providerThread.processMsg(this, clientChannelInfo, msgBuffer, error) < PerfToolsReturnCodes.SUCCESS)
                {
                    closeChannel(clientChannelInfo, error);
                    channelClosed = true;
                    ret = TransportReturnCodes.FAILURE;
                }
            }
            ++readCount;
        }
        while (ret > TransportReturnCodes.SUCCESS);

        if (channelClosed)
            return PerfToolsReturnCodes.FAILURE;

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
                closeChannel(clientChannelInfo, error);
                return PerfToolsReturnCodes.FAILURE;
        }
    }

    /**
     * Tries to read data from channels until stopTime is
     * reached.
     * 
     * @param stopTime - stop time in milliseconds or nanoseconds
     * @param error Gives detailed information about error if any occurred
     *            during socket operations.
     */
    public void readChannels(long stopTime, Error error)
    {
        long currentTime;
        int selRet = 0;

        // Loop on select(), looking for channels with available data, until
        // stopTime is reached.
        try 
        {
            for (ClientChannelInfo channelInfo : _activeChannelList)
            {
            	if (channelInfo.channel.state() == ChannelState.ACTIVE)
            	{
            		if (channelInfo.needFlush)
            			addOption(channelInfo.channel, SelectionKey.OP_WRITE, channelInfo);
            		else
            			removeOption(channelInfo.channel, SelectionKey.OP_WRITE, channelInfo);

            		if (channelInfo.needRead)
            			readChannel(channelInfo, stopTime, error);
            	}

            }
        }
        catch (Exception e)
        {
        	System.err.println(e.getMessage());
        	return;
        }

        currentTime = currentTime();
        try
        {
        	long selTime = (long)((stopTime - currentTime) / _divisor);
        	if (selTime <= 0)
        		selRet = _selector.selectNow();
        	else
        		selRet = _selector.select(selTime);
        }
        catch (Exception e)
        {
        	System.err.println(e.getMessage());
        	System.exit(-1);
        }

        if (selRet > 0)
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
        		// active channel
        		if (key.isReadable())
        		{
        			if (readChannel(channelInfo, stopTime, error) < TransportReturnCodes.SUCCESS)
        				continue;
        		}
        		if (key.isWritable())
        		{
        			// Check if we can flush data from the channel.
        			if ((ret = channelInfo.channel.flush(error)) < TransportReturnCodes.SUCCESS)
        			{
        				closeChannel(channelInfo, error);
        				continue;
        			}
        			else if (ret == TransportReturnCodes.SUCCESS)
        			{
        				// UPA flush() returned 0 instead of a higher
        				// value, so there's no more data to flush.
        				flushDone(channelInfo);
        			}
        		}
        	}
        }
    }

    /**
     * Iterates over new accepted client channel list and processes them by
     * initializing them and setting them up for read/write.
     */
    public void processNewChannels()
    {
		for (ClientChannelInfo channelInfo : _initializingChannelList)
		{
			initializeChannel(channelInfo, _error);
			if (channelInfo.channel.state() == ChannelState.ACTIVE)
			{
				try
				{
					addOption(channelInfo.channel, SelectionKey.OP_READ, channelInfo);
				}
				catch (ClosedChannelException e)
				{
					System.err.println(e.getMessage());
					System.exit(-1);
				}
			}
		}
    }

    /**
     * Try to initialize a single channel.
     * 
     */
    private void initializeChannel(ClientChannelInfo clientChannelInfo, Error error)
    {
        flushDone(clientChannelInfo);

        Channel channel = clientChannelInfo.channel;

        switch (channel.init(_inProgInfo, error))
        {
            case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                if (_inProgInfo.flags() == InProgFlags.SCKT_CHNL_CHANGE)
                    requestFlush(clientChannelInfo);
                break;
            case TransportReturnCodes.SUCCESS:
            	if (clientChannelInfo.channel.state() == ChannelState.ACTIVE)
            		processActiveChannel(clientChannelInfo);
                break;
            default:
                closeChannel(clientChannelInfo, error);
                break;
        }
        return;
    }

    /**
     * Performs ping timeout checks for channels in this channel handler.
     */
    public void checkPings()
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
                        closeChannel(channelInfo, _error); // Remove client if
                                                           // sending the
                                                           // message failed
                        continue;
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
                    System.out.println(new Date() + ": Ping timed out. ping time out:" + (channelInfo.channel.pingTimeout() / 3) * 1000);
                    System.err.println("Ping timed out.");
                    closeChannel(channelInfo, _error);
                }
            }
        }
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
        if (newoption != 0)
        {
            channel.selectableChannel().register(_selector, newoption, attachment);
        }
        else
        {
            key.cancel();
        }
    }

    private void processActiveChannel(ClientChannelInfo clientChannelInfo)
    {
        // Set the next send/receive ping times.
        long currentTime = System.currentTimeMillis();

        clientChannelInfo.nextSendPingTime = currentTime + (clientChannelInfo.channel.pingTimeout() / 3) * 1000;
        clientChannelInfo.nextReceivePingTime = currentTime + clientChannelInfo.channel.pingTimeout() * 1000;

        int ret = _providerThread.processActiveChannel(this, clientChannelInfo, _error);
        if (ret < PerfToolsReturnCodes.SUCCESS)
        {
            closeChannel(clientChannelInfo, _error);
            return;
        }

        _initializingChannelList.remove(clientChannelInfo);
        clientChannelInfo.parentQueue = _activeChannelList;
        clientChannelInfo.parentQueue.add(clientChannelInfo);
    }

    /**
     * Requests that the ChannelHandler begin calling upa flush() for a channel.
     * Used when a call to upa write() indicates there is still data to be
     * written to the network.
     * 
     * @param clientChannelInfo
     */
    public void requestFlush(ClientChannelInfo clientChannelInfo)
    {
        clientChannelInfo.needFlush = true;
    }

    /*
     * Mark flush not needed for the channel.
     */
    private void flushDone(ClientChannelInfo clientChannelInfo)
    {
        clientChannelInfo.needFlush = false;
    }

    /**
     * @return Provider thread for the channel handler
     */
    public ProviderThread providerThread()
    {
        return _providerThread;
    }

    /**
     * @return List of channels that are active.
     */
    public Queue<ClientChannelInfo> activeChannelList()
    {
        return _activeChannelList;
    }

    /**
     * @return List of initializing channels.
     */
    public Queue<ClientChannelInfo> initializingChannelList()
    {
        return _initializingChannelList;
    }

    private long currentTime()
    {
        long currentTime;
        
        if (ProviderPerfConfig.ticksPerSec() <= 1000) // use millisecond timer for tickRate of 1000 and below
        {
            currentTime = System.currentTimeMillis(); 
        }
        else // use nanosecond timer for tickRate of greater than 1000
        {
            currentTime = System.nanoTime();
        }
        
        return currentTime;
    }
    
    public Lock handlerLock()
    {
    	return _channelLock;
    }
}
