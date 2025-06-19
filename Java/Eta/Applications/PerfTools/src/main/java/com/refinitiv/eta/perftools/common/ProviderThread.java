/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.PrintWriter;
import java.nio.ByteBuffer;
import java.util.Objects;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WriteArgs;
import com.refinitiv.eta.transport.WriteFlags;
import com.refinitiv.eta.transport.WritePriorities;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorSubmitOptions;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamInfo;
import com.refinitiv.eta.valueadd.reactor.TunnelStreamSubmitOptions;

/**
 * ProviderThreads are used to control individual threads. Each thread handles
 * providing data to its open channels.
 */
public class ProviderThread extends Thread
{
    private static final int        LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
    private static final int        ALWAYS_SEND_LATENCY_UPDATE  = -1;
    private static final int        ALWAYS_SEND_LATENCY_GENMSG  = -1;
            
    private long                        _providerIndex;                     // Index given to this provider thread. 
    private long                        _currentTicks;                      // Current tick out of ticks per second.
    private CountStat                   _refreshMsgCount;                   // Counts refreshes sent.
    private CountStat                   _updateMsgCount;                    // Counts updates sent. 
    private CountStat                   _itemRequestCount;                  // Counts requests received.
    private CountStat                   _closeMsgCount;                     // Counts closes received.
    private CountStat                   _postMsgCount;                      // Counts posts received.
    private CountStat                   _outOfBuffersCount;                 // Counts of messages blocked due  not sent due to lack of output buffers.
    private CountStat                   _msgSentCount;                      // Counts total messages sent.
    private CountStat                   _bufferSentCount;                   // Counts total buffers sent(used with
                                                                            // msgSentCount for packing statistics). 
    WriteArgs                   		_writeArgs;
    
    protected XmlMsgData                _xmlMsgData;                        // Msgs from XML 
    protected ItemEncoder               _itemEncoder;                       // item encoder
    private LatencyRandomArray          _updateLatencyRandomArray;          // Updates random latency array
    private LatencyRandomArray          _genMsgLatencyRandomArray;          // Generic Messages random latency array
    private LatencyRandomArrayOptions   _randomArrayOpts;                   // random array options
    
    protected Error _error;                                                 // Error information

    private volatile boolean            _shutdown;                          //Signals thread to shutdown
    private volatile boolean            _shutdownAck;                       //Acknowledges thread is shutdown.

	private ProviderThreadInfo			_provThreadInfo;					// thread information
	
    private ReactorSubmitOptions        _submitOptions;                     // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private ReactorErrorInfo            _errorInfo;                         // Use the VA Reactor instead of the ETA Channel for sending and receiving
    private TunnelStreamSubmitOptions	_tunnelStreamSubmitOptions;         // Use the VA Reactor tunnel stream for submitting messages.
    private TunnelStreamInfo 			_tunnelStreamInfo;
    protected JsonConverterSession		_jsonConverterSession;				// Use the converter library to converter between JSON and RWF messages.
    
    /**
     * Instantiates a new provider thread.
     *
     * @param xmlMsgData the xml msg data
     */
    public ProviderThread(XmlMsgData xmlMsgData)
    {
        super("ProviderThread");
        _refreshMsgCount = new CountStat();
        _updateMsgCount = new CountStat();
        _itemRequestCount = new CountStat();
        _closeMsgCount = new CountStat();
        _postMsgCount = new CountStat();
        _outOfBuffersCount = new CountStat();
        _msgSentCount = new CountStat();
        _bufferSentCount = new CountStat();
        _writeArgs = TransportFactory.createWriteArgs();
        _xmlMsgData  = xmlMsgData;
        _itemEncoder = new ItemEncoder(xmlMsgData);
        _updateLatencyRandomArray = new LatencyRandomArray();
        _genMsgLatencyRandomArray = new LatencyRandomArray();
        _randomArrayOpts = new LatencyRandomArrayOptions();
        _error = TransportFactory.createError();
        _provThreadInfo = new ProviderThreadInfo(); 
        _submitOptions = ReactorFactory.createReactorSubmitOptions();
        _submitOptions.writeArgs().clear();
        _submitOptions.writeArgs().priority(WritePriorities.HIGH);
        _submitOptions.writeArgs().flags(ProviderPerfConfig.directWrite() ? WriteFlags.DIRECT_SOCKET_WRITE : 0);
        _errorInfo = ReactorFactory.createReactorErrorInfo();
        _tunnelStreamSubmitOptions = ReactorFactory.createTunnelStreamSubmitOptions();
        _tunnelStreamSubmitOptions.containerType(DataTypes.MSG);
        _tunnelStreamInfo = ReactorFactory.createTunnelStreamInfo();
        _jsonConverterSession = new JsonConverterSession();
    }

    /**
     * Initializes a ProviderThread.
     *
     * @param providerIndex the provider index
     * @param providerType the provider type
     */
    protected void init(int providerIndex, ProviderType providerType)
    {
        _refreshMsgCount.init();
        _updateMsgCount.init();
        _itemRequestCount.init();
        _closeMsgCount.init();
        _postMsgCount.init();
        _outOfBuffersCount.init();
        _msgSentCount.init();
        _bufferSentCount.init();
        _currentTicks = 0;
        _providerIndex = providerIndex;
        
    	if (ProviderPerfConfig.updatesPerSec() != 0 && ProviderPerfConfig.latencyUpdateRate() > 0)
    	{
    		_randomArrayOpts.totalMsgsPerSec(ProviderPerfConfig.updatesPerSec());
	        _randomArrayOpts.latencyMsgsPerSec(ProviderPerfConfig.latencyUpdateRate());
	        _randomArrayOpts.ticksPerSec(ProviderPerfConfig.ticksPerSec());
	        _randomArrayOpts.arrayCount(LATENCY_RANDOM_ARRAY_SET_COUNT);
	        
	        if (_updateLatencyRandomArray.create(_randomArrayOpts) != PerfToolsReturnCodes.SUCCESS)
	        {
	            System.err.println("Error initializing application: Failed to create updates latency random array");
	            System.exit(-1);
	        }
    	}

    	if (ProviderPerfConfig.genMsgsPerSec() != 0 && ProviderPerfConfig.latencyGenMsgRate() > 0)
    	{
    		_randomArrayOpts.totalMsgsPerSec(ProviderPerfConfig.genMsgsPerSec());
	        _randomArrayOpts.latencyMsgsPerSec(ProviderPerfConfig.latencyGenMsgRate());
	        _randomArrayOpts.ticksPerSec(ProviderPerfConfig.ticksPerSec());
	        _randomArrayOpts.arrayCount(LATENCY_RANDOM_ARRAY_SET_COUNT);
	        
	        if (_genMsgLatencyRandomArray.create(_randomArrayOpts) != PerfToolsReturnCodes.SUCCESS)
	        {
	            System.err.println("Error initializing application: Failed to create generic messages latency random array");
	            System.exit(-1);
	        }
    	}
	
        // Open stats file.
    	_provThreadInfo.statsFile(new File(ProviderPerfConfig.statsFilename() + (_providerIndex+1) + ".csv"));
        try
        {
            _provThreadInfo.statsFileWriter(new PrintWriter(_provThreadInfo.statsFile()));
        }
        catch (FileNotFoundException e)
        {
            System.err.println("Error initializing application:  Failed to open stats file '" + _provThreadInfo.statsFile().getName() + "'");
            System.exit(-1);
        }
        
        if (providerType == ProviderType.PROVIDER_INTERACTIVE)
            _provThreadInfo.statsFileWriter().println("UTC, Requests received, Images sent, Updates sent, Posts reflected, GenMsgs sent, GenMsg Latencies sent, GenMsgs received, GenMsg Latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%%), Memory (MB)");
        else
            _provThreadInfo.statsFileWriter().println("UTC, Images sent, Updates sent, CPU usage (%), Memory (MB)");
        _provThreadInfo.statsFileWriter().flush();

        // Open latency file if configured.
        if (ProviderPerfConfig.logLatencyToFile())
    	{
            _provThreadInfo.latencyLogFile(new File(ProviderPerfConfig.latencyFilename() + (_providerIndex+1) + ".csv"));
            try
            {
                _provThreadInfo.latencyLogFileWriter(new PrintWriter(_provThreadInfo.latencyLogFile()));
            }
            catch (FileNotFoundException e)
            {
            	System.err.println("Error initializing application:  Failed to open latency file '" + _provThreadInfo.latencyLogFile().getName() + "'");
                System.exit(-1);
            }
            _provThreadInfo.latencyLogFileWriter().println("Message type, Send time, Receive time, Latency (usec)");
            _provThreadInfo.latencyLogFileWriter().flush();
    	}
   }
 
    /**
     * Clean up provider thread.
     */
    public void cleanup()
    {
    }
    
    /**
     * Write current buffer in use by the client channel session.
     * 
     */
    @SuppressWarnings("fallthrough")
	private int writeCurrentBuffer(ProviderSession session, Error error)
    {
    	int packedBufferCount = session.packedBufferCount();
    	
        // Reset the session write buffer packed count,
        // so that packing can continue in the next buffer
        session.packedBufferCount(0);
        
        if (!NIProvPerfConfig.useReactor() && !ProviderPerfConfig.useReactor()) // use ETA Channel for sending and receiving
        {
        	Channel channel = session.clientChannelInfo().channel;
        	int ret = TransportReturnCodes.SUCCESS;
        	
            _writeArgs.clear();
            _writeArgs.priority(WritePriorities.HIGH);
            _writeArgs.flags(ProviderPerfConfig.directWrite() ? WriteFlags.DIRECT_SOCKET_WRITE : 0);

            ret = channel.write(session.writingBuffer(), _writeArgs, error);

            // call flush and write again
            while (ret == TransportReturnCodes.WRITE_CALL_AGAIN)
            {
                ret = channel.flush(error);
                if (ret < TransportReturnCodes.SUCCESS)
                {
                    return ret;
                }
                ret = channel.write(session.writingBuffer(), _writeArgs, error);
            }

            if (ret >= TransportReturnCodes.SUCCESS)
            {
                session.writingBuffer(null);
                bufferSentCount().increment();
                return ret;
            }
            else if (ret == TransportReturnCodes.WRITE_FLUSH_FAILED && channel.state() == ChannelState.ACTIVE) {
                // If FLUSH_FAILED is received, check the channel state.
                // if it is still active, it's okay, just need to flush.
                session.writingBuffer(null);
                bufferSentCount().increment();
                return TransportReturnCodes.SUCCESS;
            }
            else { // Otherwise treat as error, fall through to default.
                error.text("Channel.write() failed with return code: " + ret);
                error.errorId(ret);
                return TransportReturnCodes.FAILURE;
            }
        }
        else // use ETA VA Reactor for sending and receiving
        {
            int retval = ReactorReturnCodes.SUCCESS;
            ReactorChannel reactorChannel = session.clientChannelInfo().reactorChannel;

            if (Objects.isNull(session.clientChannelInfo().tunnelStream))
            {
            	retval = reactorChannel.submit(session.writingBuffer(), _submitOptions, _errorInfo);
            }
            else
            {
            	retval = session.clientChannelInfo().tunnelStream.submit(session.writingBuffer(), _tunnelStreamSubmitOptions, _errorInfo);
            }

            if (retval == ReactorReturnCodes.WRITE_CALL_AGAIN)
            {
                //call flush and write again until there is data in the queue
                while (retval == ReactorReturnCodes.WRITE_CALL_AGAIN)
                {
                	if (Objects.isNull(session.clientChannelInfo().tunnelStream))
                	{
                		retval = reactorChannel.submit(session.writingBuffer(), _submitOptions, _errorInfo);
                	}
                	else
                	{
                		retval = session.clientChannelInfo().tunnelStream.submit(session.writingBuffer(), _tunnelStreamSubmitOptions, _errorInfo);
                	}
                }
            }
            else if (retval < ReactorReturnCodes.SUCCESS)
            {
                // write failed, release buffer and shut down
                if (reactorChannel.state() != ReactorChannel.State.CLOSED)
                {
                    reactorChannel.releaseBuffer(session.writingBuffer(), _errorInfo);
                }
                error.text("ReactorChannel.submit() failed with return code: " + retval + " <" + _errorInfo.error().text() + ">");
                error.errorId(retval);
                return TransportReturnCodes.FAILURE;
            }
            else { // if (retval >= ReactorReturnCodes.SUCCESS)
                session.writingBuffer(null);
                bufferSentCount().increment();
            }

            return retval;
        }
    }

    @SuppressWarnings("fallthrough")
    private int encodeAndWriteCurrentBuffer(ProviderSession session, Error error)
    {
        int packedBufferCount = session.packedBufferCount();

        // Reset the session write buffer packed count,
        // so that packing can continue in the next buffer
        session.packedBufferCount(0);

        if (!NIProvPerfConfig.useReactor() && !ProviderPerfConfig.useReactor()) // use ETA Channel for sending and receiving
        {
            Channel channel = session.clientChannelInfo().channel;
            int ret = TransportReturnCodes.SUCCESS;

            _writeArgs.clear();
            _writeArgs.priority(WritePriorities.HIGH);
            _writeArgs.flags(ProviderPerfConfig.directWrite() ? WriteFlags.DIRECT_SOCKET_WRITE : 0);

            if (channel.protocolType() == Codec.JSON_PROTOCOL_TYPE)
            {
            	TransportBuffer msgBuffer = null;

            	do
            	{
            		if (Objects.nonNull(session.tempWriteBuffer()))
            		{
            			msgBuffer = _jsonConverterSession.convertToJsonMsg(channel, session.tempWriteBuffer(), error);
            		}
            		else
            		{
            			msgBuffer = _jsonConverterSession.convertToJsonMsg(channel, session.writingBuffer(), error);
            		}

            		if (msgBuffer == null)
            		{
            			if(error.errorId() == TransportReturnCodes.NO_BUFFERS)
            			{
            				if ((ret = channel.flush(error)) < TransportReturnCodes.SUCCESS)
            				{
            					System.out.println("rsslFlush() failed with return code " + ret + "<" + error.text() + ">");
            					return ret;
            				}
            			}
            			else
            			{
            				System.out.println("convertToJsonMsg(): Failed to convert RWF > JSON with error text: " + error.text());
            				channel.releaseBuffer(session.writingBuffer(), error);
            				return TransportReturnCodes.FAILURE;
            			}

            		}
            	} while(error.errorId() == TransportReturnCodes.NO_BUFFERS);

            	if(Objects.nonNull(msgBuffer))
            	{
            		if(ProviderPerfConfig.totalBuffersPerPack() == 1) // Not packing.
            		{
            			// Release the original RWF buffer and point the writing buffer to the JSON buffer
            			channel.releaseBuffer(session.writingBuffer(), error);
            			session.writingBuffer(msgBuffer);
            		}
            		else
            		{
            			// Ensure there is enough space to copy data
            			if (session.remaingPackedBufferLength() >= msgBuffer.length())
            			{
            				session.writingBuffer().data().put(msgBuffer.data().array(), msgBuffer.dataStartPosition(), msgBuffer.data().position() - msgBuffer.dataStartPosition());
            				channel.releaseBuffer(msgBuffer, error);
            			}
            			else
            			{
            				System.out.println("writeCurrentBuffer: The remaining packed buffer length " + session.remaingPackedBufferLength() +
            						" is not enough to copy data length " + msgBuffer.length());
            				channel.releaseBuffer(session.writingBuffer(), error);
            				return TransportReturnCodes.FAILURE;
            			}
            		}
            	}

            }


            ret = channel.write(session.writingBuffer(), _writeArgs, error);

            // call flush and write again
            while (ret == TransportReturnCodes.WRITE_CALL_AGAIN)
            {
                ret = channel.flush(error);
                if(ret < TransportReturnCodes.SUCCESS)
                {
                    return ret;
                }
                ret = channel.write(session.writingBuffer(), _writeArgs, error);
            }

            if (ret >= TransportReturnCodes.SUCCESS)
            {
                session.writingBuffer(null);
                bufferSentCount().increment();
                return ret;
            }
            else if (ret == TransportReturnCodes.WRITE_FLUSH_FAILED && channel.state() == ChannelState.ACTIVE) {
                // If FLUSH_FAILED is received, check the channel state.
                // if it is still active, it's okay, just need to flush.
                session.writingBuffer(null);
                bufferSentCount().increment();
                return TransportReturnCodes.SUCCESS;
            }
            else { // Otherwise treat as error, fall through to default.
                error.text("Channel.write() failed with return code: " + ret);
                error.errorId(ret);
                return TransportReturnCodes.FAILURE;
            }
        }
        else // use ETA VA Reactor for sending and receiving
        {
            int retval = ReactorReturnCodes.SUCCESS;
            ReactorChannel reactorChannel = session.clientChannelInfo().reactorChannel;

            if (reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE && packedBufferCount > 0)
            {
                if (reactorChannel.packBuffer(session.writingBuffer(), _errorInfo) < ReactorReturnCodes.SUCCESS)
                {
                    error.errorId(_errorInfo.error().errorId());
                    error.text(_errorInfo.error().text());
                    return ReactorReturnCodes.FAILURE;
                }
            }

            if (Objects.isNull(session.clientChannelInfo().tunnelStream))
            {
                retval = reactorChannel.submit(session.writingBuffer(), _submitOptions, _errorInfo);
            }
            else
            {
                retval = session.clientChannelInfo().tunnelStream.submit(session.writingBuffer(), _tunnelStreamSubmitOptions, _errorInfo);
            }

            if (retval == ReactorReturnCodes.WRITE_CALL_AGAIN)
            {
                //call flush and write again until there is data in the queue
                while (retval == ReactorReturnCodes.WRITE_CALL_AGAIN)
                {
                    if(Objects.isNull(session.clientChannelInfo().tunnelStream))
                    {
                        retval = reactorChannel.submit(session.writingBuffer(), _submitOptions, _errorInfo);
                    }
                    else
                    {
                        retval = session.clientChannelInfo().tunnelStream.submit(session.writingBuffer(), _tunnelStreamSubmitOptions, _errorInfo);
                    }
                }
            }
            else if (retval < ReactorReturnCodes.SUCCESS)
            {
                // write failed, release buffer and shut down
                if (reactorChannel.state() != ReactorChannel.State.CLOSED)
                {
                    reactorChannel.releaseBuffer(session.writingBuffer(), _errorInfo);
                }
                error.text("ReactorChannel.submit() failed with return code: " + retval + " <" + _errorInfo.error().text() + ">");
                error.errorId(retval);
                return TransportReturnCodes.FAILURE;
            }
            else { // if (retval >= ReactorReturnCodes.SUCCESS)
                session.writingBuffer(null);
                bufferSentCount().increment();
            }

            return retval;
        }
    }

    /**
     * Gets a TransportBuffer for encoding a message. This method handles packing
     * of messages, if packing is configured -- it will pack as long as
     * appropriate, stopping to write if the present buffer is too full to
     * accommodate the requested length.
     * 
     * @param session - Channel session to get transport buffer from
     * @param length - buffer size to get
     * @param error - error populated when get transport buffer fails
     * 
     * @return &lt;0 if get buffer fails, 0 otherwise.
     */
    public int getItemMsgBuffer(ProviderSession session, int length, Error error)
    {
        if (ProviderPerfConfig.totalBuffersPerPack() == 1 || Objects.nonNull(session.clientChannelInfo().tunnelStream))  // Not packing or tunnel stream
        {
            int ret = getNewBuffer(session, length, error);
            if (ret < TransportReturnCodes.SUCCESS)
                return ret;
        }
        else
        {
            // We are packing, and may need to do something different based on the size of
            // the message and how much room is left in the present buffer.
            if (length > ProviderPerfConfig.packingBufferLength())
            {
               // Message too large for our packing buffer, write and get a bigger one.
                if (session.writingBuffer() != null)
                {
                    int ret = writeCurrentBuffer(session, error);
                    if (ret < TransportReturnCodes.SUCCESS)
                    {
                        return ret;
                    }
                }
            }
            else if (session.writingBuffer() == null)
            {
                // Have no buffer currently, so get a new one.
                int ret = getNewBuffer(session, ProviderPerfConfig.packingBufferLength(), error);
                if(ret < TransportReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
            else if (length > session.writingBuffer().data().limit() - session.writingBuffer().data().position())
            {
                //Out of room in current packing buffer. Write the current one and get a new one.
                int ret = writeCurrentBuffer(session, error);
                if(ret < TransportReturnCodes.SUCCESS)
                {
                    return ret;
                }
                
                ret = getNewBuffer(session, ProviderPerfConfig.packingBufferLength(), error);
                if(ret < TransportReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
            else
            {
                //Enough room in current packing buffer, don't need a new one.
                return TransportReturnCodes.SUCCESS;
            }
        }
        
        if (session.writingBuffer() != null)
        {
            return TransportReturnCodes.SUCCESS;
        }
        else
        {
            int ret = getNewBuffer(session, length, error);
            if(ret < TransportReturnCodes.SUCCESS)
                return ret;

            return TransportReturnCodes.SUCCESS;
        }
    }
    
    
    /*
     * Retrieves a transport buffer from a channel.
     */
    private int getNewBuffer(ProviderSession session, int length, Error error)
    {
       TransportBuffer msgBuf;
       
       if (!NIProvPerfConfig.useReactor() && !ProviderPerfConfig.useReactor()) // use ETA Channel for sending and receiving
       {
    	   boolean packedBuffer = ProviderPerfConfig.totalBuffersPerPack() > 1 ? true : false;
    	   
           msgBuf = session.clientChannelInfo().channel.getBuffer(length, packedBuffer, error);
           
           if(session.clientChannelInfo().channel.protocolType() == Codec.JSON_PROTOCOL_TYPE && packedBuffer)
           {
        	   session.remaingPackedBufferLength(length);
        	   
        	   if (session.tempWriteBuffer() == null)
        	   {
        		   Buffer tempBuffer = CodecFactory.createBuffer();
        		   tempBuffer.data(ByteBuffer.allocate(length));
        		   session.tempWriteBuffer(tempBuffer);
        	   }
           }
       }
       else // use ETA VA Reactor for sending and receiving
       {
           if (session.clientChannelInfo().reactorChannel.state() == ReactorChannel.State.READY)
           {
        	   if(Objects.isNull(session.clientChannelInfo().tunnelStream))
        	   {
        		   msgBuf = session.clientChannelInfo().reactorChannel.getBuffer(length, ProviderPerfConfig.totalBuffersPerPack() > 1, _errorInfo);
        	   }
        	   else
        	   {
        		   msgBuf = session.clientChannelInfo().tunnelStream.getBuffer(length, _errorInfo);
        	   }
           }
           else
           {
               msgBuf = null;
           }
       }
       
       if(msgBuf == null)
       {
           return TransportReturnCodes.NO_BUFFERS;
       }
       
       session.writingBuffer(msgBuf);
       
       return TransportReturnCodes.SUCCESS;        
    }
    
    /**
     * Sends a completed transport buffer. This method packs messages, if packing is
     * configured. The allowPack option may be used to prevent packing if
     * needed(for example, we just encoded the last message of a burst so it is
     * time to write to the transport).
     * 
     * @param session - client channel session 
     * @param allowPack - if false, write buffer without packing. if true, pack according to configuration.
     * @param error - error information populated in case of failure.
     * @return &lt;0 in case of error, 0 otherwise
     */
    public int sendItemMsgBuffer(ProviderSession session, boolean allowPack, Error error)
    {
       _msgSentCount.increment();
 
       // Make sure we stop packing at the end of a burst of updates
       // in case the next burst is for a different channel. 
       // (This will also prevent any latency updates from sitting in the pack for a tick).
       if (session.packedBufferCount() == (ProviderPerfConfig.totalBuffersPerPack() -1) || !allowPack || Objects.nonNull(session.clientChannelInfo().tunnelStream))
       {
          int ret = encodeAndWriteCurrentBuffer(session, error);
          
          /* Gets tunnel stream buffer usage */
          if (ProviderPerfConfig.tunnelStreamBufsUsed() && Objects.nonNull(session.clientChannelInfo().tunnelStream))
          {
        	  if (session.clientChannelInfo().tunnelStream.info(_tunnelStreamInfo, _errorInfo) == ReactorReturnCodes.SUCCESS)
        	  {
          			getProvThreadInfo().stats().tunnelStreamBufUsageStats().update(_tunnelStreamInfo.buffersUsed());
        	  }
          }
          
          return ret;
       }
       else
       {
           //Pack the buffer and continue using it.
           session.packedBufferCount(session.packedBufferCount()+1);
           if (!NIProvPerfConfig.useReactor() && !ProviderPerfConfig.useReactor()) // use ETA Channel for sending and receiving
           {
        	   Channel channel = session.clientChannelInfo().channel;
        	   TransportBuffer msgBuffer = null;
        	   
        	   if(channel.protocolType() == Codec.JSON_PROTOCOL_TYPE)
        	   {
        		   int ret = TransportReturnCodes.SUCCESS;
        		   
	        		do 
	               	{
	               		if ((msgBuffer = _jsonConverterSession.convertToJsonMsg(channel, session.tempWriteBuffer(), error)) == null)
	               		{
	               			if(error.errorId() == TransportReturnCodes.NO_BUFFERS)
	               			{
	               				if ((ret = channel.flush(error)) < TransportReturnCodes.SUCCESS)
	               				{
	               					System.out.println("rsslFlush() failed with return code " + ret + "<" + error.text() + ">");
	               					return ret;
	               				}
	               			}
	               			else
	               			{
	               				System.out.println("convertToJsonMsg(): Failed to convert RWF > JSON with error text: " + error.text());
	               				channel.releaseBuffer(session.writingBuffer(), error);
	               				return TransportReturnCodes.FAILURE;
	               			}
	               			
	               		}
	               	} while(error.errorId() == TransportReturnCodes.NO_BUFFERS);
	        		
	        		/* Ensure there is enough space to copy data */
	         	   if(session.remaingPackedBufferLength() >= msgBuffer.length())
	         	   {
	         		  session.writingBuffer().data().put(msgBuffer.data().array(), msgBuffer.dataStartPosition(), msgBuffer.data().position() - msgBuffer.dataStartPosition());
	         		  channel.releaseBuffer(msgBuffer, error);
	 	   			}
	 	   			else
	 		   		{
	 	        		System.out.println("writeCurrentBuffer: The remaining packed buffer length " + session.remaingPackedBufferLength() +
	 	   						" is not enough to copy data length " + msgBuffer.length());
	 	        		channel.releaseBuffer(session.writingBuffer(), error);
                        return TransportReturnCodes.FAILURE;
	 		   		}
        	   }
        	   
        	   int remainingLength = 0;
        	   
               if ( (remainingLength = channel.packBuffer(session.writingBuffer(), error)) < TransportReturnCodes.SUCCESS)
               {
                   return TransportReturnCodes.FAILURE;
               }
               
               session.remaingPackedBufferLength(remainingLength);
           }
           else // use ETA VA Reactor for sending and receiving
           {
               if (session.clientChannelInfo().reactorChannel.packBuffer(session.writingBuffer(), _errorInfo) < ReactorReturnCodes.SUCCESS)
               {
                   error.errorId(_errorInfo.error().errorId());
                   error.text(_errorInfo.error().text());
                   return ReactorReturnCodes.FAILURE;
               }
           }
           return TransportReturnCodes.SUCCESS;
       }
    }
    
    /**
     * Sends a burst of refreshes for items that currently need to send one.
     *
     * @param providerSession the provider session
     * @param error the error
     * @return the int
     */
    protected int sendRefreshBurst(ProviderSession providerSession, Error error)
    {
        int refreshLeft = providerSession.refreshItemList().count();
        if(refreshLeft > ProviderPerfConfig.refreshBurstSize())
            refreshLeft = ProviderPerfConfig.refreshBurstSize();
        
        int ret = PerfToolsReturnCodes.SUCCESS;
        for(; refreshLeft > 0; --refreshLeft)
        {
            ItemInfo itemInfo = providerSession.refreshItemList().getFront();
            
            ret = getItemMsgBuffer(providerSession, _itemEncoder.estimateRefreshBufferLength(itemInfo, providerSession.clientChannelInfo().channel.protocolType()), error);
            if(ret < TransportReturnCodes.SUCCESS)
                return ret;
            
            // Encode the message with data appropriate for the domain
            if (Objects.nonNull(providerSession.tempWriteBuffer()))
            {
            	ByteBuffer byteBuffer = providerSession.tempWriteBuffer().data();
            	byteBuffer.clear();
            	providerSession.tempWriteBuffer().data(byteBuffer);
            	ret = _itemEncoder.encodeRefresh(providerSession.clientChannelInfo().channel, itemInfo, providerSession.tempWriteBuffer(), null, 0, error);
            }
            else
            {
            	ret = _itemEncoder.encodeRefresh(providerSession.clientChannelInfo().channel, itemInfo, providerSession.writingBuffer(), null, 0, error);
            }
            
            if(ret < CodecReturnCodes.SUCCESS)
                return PerfToolsReturnCodes.FAILURE;

            ret = sendItemMsgBuffer(providerSession, refreshLeft > 1, error);
            if(ret < TransportReturnCodes.SUCCESS)
                return PerfToolsReturnCodes.FAILURE;
           
            _refreshMsgCount.increment();
            
            providerSession.refreshItemList().removeFront();
            
            //If it's not a streaming request, don't add it to the update list. 
            if(!((itemInfo.itemFlags() & ItemFlags.IS_STREAMING_REQ) > 0))
            {
                continue;
            }
            
            providerSession.updateItemList().add(itemInfo);
            providerSession.genMsgItemList().add(itemInfo);
        }
        
        return ret;
    }
    
    /**
     * Sends a burst of item updates.
     *
     * @param providerSession the provider session
     * @param error the error
     * @return the int
     */
    protected int sendUpdateBurst(ProviderSession providerSession, Error error)
    {
        //Determine updates to send out. Spread the remainder out over the first ticks 
        int updatesLeft = ProviderPerfConfig.updatesPerTick();
        int updatesPerTickRemainder = ProviderPerfConfig.updatesPerTickRemainder();
        if(updatesPerTickRemainder > _currentTicks)
            ++updatesLeft;
        
        int latencyUpdateNumber = (ProviderPerfConfig.latencyUpdateRate() > 0) ? _updateLatencyRandomArray.next() : -1;
                
        if(providerSession.updateItemList().count() == 0)
            return PerfToolsReturnCodes.SUCCESS;
        
        int ret = TransportReturnCodes.SUCCESS;
        for(; updatesLeft > 0; --updatesLeft)
        {
            long latencyStartTime;
            ItemInfo nextItem = providerSession.updateItemList().getNext();
            
            // When appropriate, provide a latency timestamp for the updates.
            if(ProviderPerfConfig.latencyUpdateRate() == ALWAYS_SEND_LATENCY_UPDATE || latencyUpdateNumber == (updatesLeft -1))
                latencyStartTime = System.nanoTime()/1000;
            else
                latencyStartTime = 0;
            
            // get a buffer for the response
            ret = getItemMsgBuffer(providerSession, _itemEncoder.estimateUpdateBufferLength(nextItem, providerSession.clientChannelInfo().channel.protocolType()), error);
            if(ret < TransportReturnCodes.SUCCESS)
            {
                if(ret == TransportReturnCodes.NO_BUFFERS)
                    outOfBuffersCount().add(updatesLeft);
                    
                return ret;
            }
            
            if(Objects.nonNull(providerSession.tempWriteBuffer()))
            {
            	ByteBuffer byteBuffer = providerSession.tempWriteBuffer().data();
            	byteBuffer.clear();
            	providerSession.tempWriteBuffer().data(byteBuffer);
            	ret = _itemEncoder.encodeUpdate(providerSession.clientChannelInfo().channel, nextItem, providerSession.tempWriteBuffer(), null, latencyStartTime, error);
            }
            else
            {
            	ret = _itemEncoder.encodeUpdate(providerSession.clientChannelInfo().channel, nextItem, providerSession.writingBuffer(), null, latencyStartTime, error);
            }
            
            if(ret < CodecReturnCodes.SUCCESS)
                return PerfToolsReturnCodes.FAILURE;

            ret = sendItemMsgBuffer(providerSession, updatesLeft > 1, error);
            if(ret < TransportReturnCodes.SUCCESS)
                return PerfToolsReturnCodes.FAILURE;
           	
            _updateMsgCount.increment();
        }
        
        if(++_currentTicks == ProviderPerfConfig.ticksPerSec())
            _currentTicks = 0;
        
        return ret;
       
    }
    
    /**
     * Sends a burst of item generic messages.
     *
     * @param providerSession the provider session
     * @param error the error
     * @return the int
     */
    protected int sendGenMsgBurst(ProviderSession providerSession, Error error)
    {
        //Determine generic messages to send out. Spread the remainder out over the first ticks 
        int genMsgsLeft = ProviderPerfConfig.genMsgsPerTick();
        int genMsgsPerTickRemainder = ProviderPerfConfig.genMsgsPerTickRemainder();
        if(genMsgsPerTickRemainder > _currentTicks)
            ++genMsgsLeft;
        
        int latencyGenMsgNumber = (ProviderPerfConfig.latencyGenMsgRate() > 0) ? _genMsgLatencyRandomArray.next() : -1;
                
        if(providerSession.genMsgItemList().count() == 0)
            return PerfToolsReturnCodes.SUCCESS;
        
        int ret = TransportReturnCodes.SUCCESS;
        for(; genMsgsLeft > 0; --genMsgsLeft)
        {
            long latencyStartTime;
            ItemInfo nextItem = providerSession.genMsgItemList().getNext();
            
            // When appropriate, provide a latency timestamp for the generic messages.
            if(ProviderPerfConfig.latencyGenMsgRate() == ALWAYS_SEND_LATENCY_GENMSG || latencyGenMsgNumber == (genMsgsLeft -1))
            {
                _provThreadInfo.stats().latencyGenMsgSentCount().increment();
                latencyStartTime = System.nanoTime()/1000;
            }
            else
                latencyStartTime = 0;
            
            // get a buffer for the response
            ret = getItemMsgBuffer(providerSession, _itemEncoder.estimateGenMsgBufferLength(nextItem), error);
            if(ret < TransportReturnCodes.SUCCESS)
            {
                if(ret == TransportReturnCodes.NO_BUFFERS)
                    outOfBuffersCount().add(genMsgsLeft);
                    
                return ret;
            }
            
            _provThreadInfo.stats().genMsgBufLenStats().update(providerSession.writingBuffer().length());
            
            ret = _itemEncoder.encodeItemGenMsg(providerSession.clientChannelInfo().channel, nextItem, providerSession.writingBuffer(), latencyStartTime);
            if(ret < CodecReturnCodes.SUCCESS)
                return PerfToolsReturnCodes.FAILURE;

            ret = sendItemMsgBuffer(providerSession, genMsgsLeft > 1, error);
            if(ret < TransportReturnCodes.SUCCESS)
                return PerfToolsReturnCodes.FAILURE;
           	
            _provThreadInfo.stats().genMsgSentCount().increment();
            
            if(getProvThreadInfo().stats().firstGenMsgSentTime() == 0)
            	getProvThreadInfo().stats().firstGenMsgSentTime(System.nanoTime());
        }
        
        if(++_currentTicks == ProviderPerfConfig.ticksPerSec())
            _currentTicks = 0;
        
        return ret;
       
    }
    
    /**
     * Refresh message burst count sent.
     *
     * @return the count stat
     */
    public CountStat refreshMsgCount()
    {
        return _refreshMsgCount;
    }

    /**
     * Update message burst count sent.
     *
     * @return the count stat
     */
    public CountStat updateMsgCount()
    {
        return _updateMsgCount;
    }

    /**
     * Item requests received.
     *
     * @return the count stat
     */
    public CountStat itemRequestCount()
    {
        return _itemRequestCount;
    }

    /**
     * Item close requests received.
     *
     * @return the count stat
     */
    public CountStat closeMsgCount()
    {
        return _closeMsgCount;
    }

    /**
     * Posts received.
     *
     * @return the count stat
     */
    public CountStat postMsgCount()
    {
        return _postMsgCount;
    }

    /**
     * Number of messages blocked due  not sent due to lack of output buffers.
     *
     * @return the count stat
     */
    public CountStat outOfBuffersCount()
    {
        return _outOfBuffersCount;
    }

    /**
     * Counts total messages sent.
     *
     * @return the count stat
     */
    public CountStat msgSentCount()
    {
        return _msgSentCount;
    }

    /**
     * Total buffers sent(used with msgSentCount for packing statistics).
     *
     * @return the count stat
     */
    public CountStat bufferSentCount()
    {
        return _bufferSentCount;
    }

    /**
     * Index given to this provider thread.
     *
     * @return the long
     */
	public long providerIndex()
	{
		return _providerIndex;
	}

	/**
	 * Item encoder.
	 *
	 * @return the item encoder
	 */
	public ItemEncoder itemEncoder()
	{
		return _itemEncoder;
	}
	
    /**
     *  Signals thread to shutdown.
     *
     * @return true, if successful
     */
    public boolean shutdown()
    {
        return _shutdown;
    }

    /**
     *  Signals thread to shutdown.
     *
     * @param value the value
     */
    public void shutdown(boolean value)
    {
        _shutdown = value;
    }

    /**
     *  Acknowledges thread is shutdown.
     *
     * @return true, if successful
     */
    public boolean shutdownAck()
    {
        return _shutdownAck;
    }

    /**
     *  Acknowledges thread is shutdown.
     *
     * @param value the value
     */
    public void shutdownAck(boolean value)
    {
        _shutdownAck = value;
    }

    /**
     * Gets the prov thread info.
     *
     * @return the prov thread info
     */
    public ProviderThreadInfo getProvThreadInfo()
    {
    	return _provThreadInfo;
    }
    
    /**
     * Gets the JSON converter session.
     *
     * @return the JSON converter session
     */
    public JsonConverterSession getJsonConverterSession()
    {
    	return _jsonConverterSession;
    }
}
