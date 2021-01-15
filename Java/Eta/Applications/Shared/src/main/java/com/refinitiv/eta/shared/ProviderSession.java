package com.refinitiv.eta.shared;

import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Objects;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.shared.network.ChannelHelper;
import com.refinitiv.eta.transport.AcceptOptions;
import com.refinitiv.eta.transport.BindOptions;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelInfo;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InProgFlags;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.InitArgs;
import com.refinitiv.eta.transport.IoctlCodes;
import com.refinitiv.eta.transport.ReadArgs;
import com.refinitiv.eta.transport.Server;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WriteArgs;
import com.refinitiv.eta.transport.WriteFlags;
import com.refinitiv.eta.transport.WritePriorities;


/**
 * Encapsulate ETA transport methods for the interactive provider and generic
 * provider example applications.
 * 
 * <P>
 * Typical usage:
 * <ol>
 * <li>
 * Call {@link #init(boolean, Error)} to create a listening socket. This method
 * initializes etaj transport and binds the server to a local port.</li>
 * <li>Poll socket events from server socket using selector.</li>
 * <li>If a new client session is trying to connect, accept or reject it using
 * {@link #handleNewClientSession(Server, Error)}.</li>
 * <li>For new request from the client, call
 * {@link #read(Channel, Error, ReceivedMsgCallback)} that initializes client socket
 * channel to complete channel handshake and then read and processes the request
 * sage from the client channel.</li> </ol>
 */
public class ProviderSession
{
    // client sessions over this limit gets rejected with NAK mount 
    public static final int NUM_CLIENT_SESSIONS = 10;

    // client sessions over this limit gets disconnected.
    public static final int CLIENT_SESSIONS_LIMIT = 30;
    
    public Selector selector;
    public ClientSessionInfo[] clientSessions = new ClientSessionInfo[CLIENT_SESSIONS_LIMIT];
    
    private Server _server;
    private int _clientSessionCount = 0;
    private Msg _xmlMsg = CodecFactory.createMsg();
    private DecodeIterator _xmlDIter = CodecFactory.createDecodeIterator();
    private boolean shouldXmlTrace = false;
    private DataDictionary _dictionaryForXml;

    private InitArgs _initArgs = TransportFactory.createInitArgs();
    private InProgInfo _inProgInfo = TransportFactory.createInProgInfo();
    private BindOptions _bindOptions = TransportFactory.createBindOptions();
    private AcceptOptions acceptOptions = TransportFactory.createAcceptOptions();
    private WriteArgs _writeArgs = TransportFactory.createWriteArgs();
    private ReadArgs _readArgs = TransportFactory.createReadArgs();
    private Error _error = TransportFactory.createError();
    private ChannelInfo _channelInfo = TransportFactory.createChannelInfo();
    
    private boolean _enableChannelWriteLocking;
 

    /**
     * Initializes etaj transport and binds the server to a local port.
     * 
     * @param globalLock flag to enable global locking on ETA Transport
     * @param error Error information when init fails.
     * 
     * @return {@link TransportReturnCodes#SUCCESS} if successful,
     *         {@link TransportReturnCodes#FAILURE} when bind fails.
     */
    public int init(boolean globalLock, Error error)
    {
        _clientSessionCount = 0;
        for (ClientSessionInfo clientSessionInfo : clientSessions)
        {
            clientSessionInfo.clear();
        }
        try
        {
            selector = Selector.open();
        }
        catch (Exception exception)
        {
            error.text("Unable to open selector: " + exception.getMessage());
            error.errorId(TransportReturnCodes.FAILURE);
            return TransportReturnCodes.FAILURE;
        }

        _initArgs.clear();
        _initArgs.globalLocking(globalLock);
        if (Transport.initialize(_initArgs, error) != TransportReturnCodes.SUCCESS)
            return TransportReturnCodes.FAILURE;
              
        // set bind options
        _bindOptions.guaranteedOutputBuffers(500);
        _bindOptions.majorVersion(Codec.majorVersion());
        _bindOptions.minorVersion(Codec.minorVersion());
        _bindOptions.protocolType(Codec.protocolType());
        
        _server = Transport.bind(_bindOptions, error);
        
        if (_server == null)
        {
            error.text("Unable to bind server: " + error.text());
            Transport.uninitialize();
            return TransportReturnCodes.FAILURE;
        }
        System.out.println("\nServer bound on port " + _server.portNumber());
        try
        {
            _server.selectableChannel().register(selector, SelectionKey.OP_ACCEPT, _server);
        }
        catch (Exception e)
        {
        	System.out.println(" Shut down the provider.");
            _server.close(error);
            
            Transport.uninitialize();
            error.text(e.getMessage());
            error.errorId(TransportReturnCodes.FAILURE);
            return TransportReturnCodes.FAILURE;
        }
        return TransportReturnCodes.SUCCESS;
    }

    /**
     * Instantiates a new provider session.
     */
    public ProviderSession()
    {
        for (int i = 0; i < CLIENT_SESSIONS_LIMIT; i++)
        {
            clientSessions[i] = new ClientSessionInfo();
        }
        
        _bindOptions.clear();
    }

    /**
     * Handle new client connection.
     * 
     * @param server - Server session
     * @param error - error information when accept fails.
     * @return {@link TransportReturnCodes#SUCCESS} or
     *         {@link TransportReturnCodes#FAILURE}
     */
    public int handleNewClientSession(Server server, Error error)
    {    	  	    	
        _clientSessionCount++;
        acceptOptions.clear();
        if (_clientSessionCount <= NUM_CLIENT_SESSIONS)
        {
            acceptOptions.nakMount(false);
        }
        else
        {
            acceptOptions.nakMount(true);    
        }
        acceptOptions.channelWriteLocking(_enableChannelWriteLocking);
        
        Channel clientChannel = server.accept(acceptOptions, error);
                
        if (clientChannel == null)
            return TransportReturnCodes.FAILURE;

        ClientSessionInfo clientSessionInfo = null;
        
        // find an available client session
        boolean clientSessionFound = false;
        for (int i = 0; i < clientSessions.length; i++)
        {
            clientSessionInfo = clientSessions[i];
             	 
            if (clientSessionInfo.clientChannel == null) 
            {
                clientSessionInfo.clientChannel = clientChannel;
                clientSessionInfo.start_time = System.currentTimeMillis();  
                clientSessionFound = true;
                break;
            }
            if (clientSessionInfo.clientChannel == clientChannel)
            {
            	clientSessionInfo.start_time = System.currentTimeMillis();  
                clientSessionFound = true;
                break;
            }      
        }
                              
        if (clientSessionFound == false) 
        {
            for (int i = 0; i < clientSessions.length; i++)
            {
            	if (clientSessionInfo.clientChannel.selectableChannel() == null 
            			|| !clientSessionInfo.clientChannel.selectableChannel().isOpen())
                {            		
                    removeChannel(clientSessionInfo.clientChannel, error);
                	clientSessionInfo.clientChannel = clientChannel;
                	clientSessionInfo.start_time = System.currentTimeMillis();
                    clientSessionFound = true;
                    break;
                } 
            }
        }

        // close channel if no more client sessions
        if (clientSessionFound == false) 
        {       	
            _clientSessionCount--;
            System.out.println("Rejected client:" + clientChannel.selectableChannel() + " " + _clientSessionCount);
            removeChannel(clientChannel, error);
            return TransportReturnCodes.SUCCESS;          
        } else {
            clientSessionInfo.socketFdValue = ChannelHelper.defineFdValueOfSelectableChannel(clientChannel.selectableChannel());
        }
                
        System.out.println("New client: " + clientChannel.selectableChannel());

        try
        {
            clientChannel.selectableChannel().register(selector, SelectionKey.OP_READ, clientChannel);
        }
        catch (Exception e)
        {
        	removeChannel(clientChannel, error);
            Transport.uninitialize();
            error.text("register select Exception: " + e.getMessage());
            error.errorId(TransportReturnCodes.FAILURE);
            return TransportReturnCodes.SUCCESS;
        }
        return TransportReturnCodes.SUCCESS;
    }

    /**
     * Reads from a channel. chnl - The channel to be read from
     *
     * @param channel the channel
     * @param error the error
     * @param callback the callback
     * @return the int
     */
    public int read(Channel channel, Error error, ReceivedMsgCallback callback)
    {   	
    	int ret;
        /* It is possible for a consumer connection to be active, and the consumer immediately sends a login.  
         * So the provider should attempt to read immediately after in case a login has been sent by the consumer.
         */
        if (channel.selectableChannel() != null && channel.state() == ChannelState.INITIALIZING)
        {
            ret = initChannel(channel, error, callback);

            if(ret != TransportReturnCodes.SUCCESS || channel.state() == ChannelState.INITIALIZING)
            	return ret;
        }

        if (channel.selectableChannel() != null && channel.state() == ChannelState.ACTIVE)
        {
            return readLoop(channel, error, callback);
        }
        else if (channel.state() == ChannelState.CLOSED)
        {
            System.out.println("channelClosed portno="
                    + channel.selectableChannel() + "<"
                    + error.text() + ">");
        	
            removeClientSessionForChannel(channel, error);
            callback.processChannelClose(channel);
        }

        return TransportReturnCodes.SUCCESS;
    }

    /**
     * flush for write file descriptor and active state.
     *
     * @param key the key
     * @param error the error
     */
    public void flush(SelectionKey key, Error error)
    {
        ClientSessionInfo clientSessionInfo = getClientSessionForChannel((Channel)key.attachment());

        // flush the data
        int ret = clientSessionInfo.clientChannel.flush(error);
        if (ret < TransportReturnCodes.SUCCESS)
        {
            System.out.println("Flush() failed with return code: " + error.text());
        }
        else if (ret == TransportReturnCodes.SUCCESS)
        {
            // remove write notification
            try
            {
                removeOption(clientSessionInfo.clientChannel, SelectionKey.OP_WRITE);
            }
            catch (ClosedChannelException e)
            {
                System.out.println("selector remove write notification failed, Exception: " + e.getMessage());
            }
        }
    }

    /**
     * Closes server session and uninitialize ETAJ Transport.
     */
    public void uninit()
    {
        Error error = TransportFactory.createError();
        error.clear();
        /* clean up client sessions */
        for (ClientSessionInfo clientSessionInfo : clientSessions)
        {
            if (clientSessionInfo.clientChannel != null && clientSessionInfo.clientChannel.state() == ChannelState.ACTIVE)
            {
                removeChannel(clientSessionInfo.clientChannel, error);
            }
        }

        /* clean up server */
        try
        {
            removeOption(_server, SelectionKey.OP_READ);
        }
        catch (ClosedChannelException e)
        {
            // System.out.println("Error cancelling read selector during shutdown:"
            // + e.getMessage());
        }
        System.out.println( "Shutdown provider...");
        _server.close(error);

        /* clear the bind options */
        _bindOptions.clear();
        
        /* free memory for dictionary */
        Transport.uninitialize();
    }

    private int initChannel(Channel channel, Error error, ReceivedMsgCallback callback)
    {
        int rcvBufSize = 65535;
        int sendBufSize = 65535;

        try
        {
            removeOption(channel, SelectionKey.OP_WRITE);
        }
        catch (ClosedChannelException e)
        {
            error.text(e.getMessage());
            error.errorId(TransportReturnCodes.FAILURE);
            return TransportReturnCodes.FAILURE;
        }
        _inProgInfo.clear();
        int ret = channel.init(_inProgInfo, error);
        if (ret < TransportReturnCodes.SUCCESS)
        {
            System.out.println("sessionInactive: " + error.text());            
            removeClientSessionForChannel(channel, error);           
            callback.processChannelClose(channel);

            return TransportReturnCodes.SUCCESS;
        }

        switch (ret)
        {
            case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                if (_inProgInfo.flags() == InProgFlags.SCKT_CHNL_CHANGE)
                {
                    if (reRegister(channel, _inProgInfo, channel, error) < TransportReturnCodes.SUCCESS)
                    {
                        return TransportReturnCodes.FAILURE;
                    }
                    else
                    {
                        final ClientSessionInfo clientSessionInfo = getClientSessionForChannel(channel);
                        if (Objects.nonNull(clientSessionInfo)) {
                            clientSessionInfo.socketFdValue = ChannelHelper.defineFdValueOfSelectableChannel(channel.selectableChannel());
                        }
                        System.out.println("Channel connection in progress");
                    }
                }
                break;
            case TransportReturnCodes.SUCCESS:
                System.out.println("Client channel is now ACTIVE");
                if (channel.info(_channelInfo, _error) == TransportReturnCodes.SUCCESS)
                {
                    System.out.printf( "Channel Info:\n" +
                    				   "  Connected Hostname: %s\n" +
                    				   "  Connected IP: %s\n" +
                                       "  Max Fragment Size: %d\n" +
                                       "  Output Buffers: %d Max, %d Guaranteed\n" +
                                       "  Input Buffers: %d\n" +
                                       "  Send/Recv Buffer Sizes: %d/%d\n" + 
                                       "  Ping Timeout: %d\n",  
                                       _channelInfo.clientHostname(),
                                       _channelInfo.clientIP(),
                                       _channelInfo.maxFragmentSize(),
                                       _channelInfo.maxOutputBuffers(), _channelInfo.guaranteedOutputBuffers(),
                                       _channelInfo.numInputBuffers(),
                                       _channelInfo.sysSendBufSize(), _channelInfo.sysRecvBufSize(),
                                       _channelInfo.pingTimeout()
                                       );
                    System.out.println( "  Client To Server Pings: " + _channelInfo.clientToServerPings() +
                    					"\n  Server To Client Pings: " + _channelInfo.serverToClientPings() +
                    					"\n");
                    System.out.printf("  Connected component version: ");

                    int count = _channelInfo.componentInfo().size();
                    if(count == 0)
                        System.out.printf("(No component info)");
                    else
                    {
                        for(int i = 0; i < count; ++i)
                        {
                            System.out.println(_channelInfo.componentInfo().get(i).componentVersion());
                           if (i < count - 1)
                               System.out.printf(", ");
                        }
                    }
                }
                        
                System.out.printf("\n\n");

                initPingHandler(channel);
                
                if (channel.ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS, sendBufSize, error) != TransportReturnCodes.SUCCESS)
                    System.out.println("channel.ioctl() failed: " + error.text());

                if (channel.ioctl(IoctlCodes.SYSTEM_READ_BUFFERS, rcvBufSize, error) != TransportReturnCodes.SUCCESS)
                    System.out.println("channel.ioctl() failed: " + error.text());
                break;
   
            default:
                System.out.println("Bad return value ret=" + ret + " " + error.text());
                removeClientSessionForChannel(channel, error);
                break;
        }

        return TransportReturnCodes.SUCCESS;
    }

    private int readLoop(Channel channel, Error error, ReceivedMsgCallback callback)
    {
        _readArgs.clear();
        do
        {
            TransportBuffer msgBuf = channel.read(_readArgs, error);
            if (msgBuf != null)
            {
                if (shouldXmlTrace)
                {
                    _xmlDIter.clear();
                    _xmlDIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
                    System.out.println(_xmlMsg.decodeToXml(_xmlDIter, _dictionaryForXml));
                }
                callback.processReceivedMsg(channel, msgBuf);
                setMsgReceived(channel);
            }
            else
            {
                switch (_readArgs.readRetVal())
                {
                    case TransportReturnCodes.FAILURE:
                    case TransportReturnCodes.INIT_NOT_INITIALIZED:

                        System.out.println("channelInactive chnl ="
                                + channel.selectableChannel() + " < "
                                + error.text() + ">"
                                + ",  state = " + channel.state());     
                                                
                        removeClientSessionForChannel(channel, error);
                        callback.processChannelClose(channel);
                        return TransportReturnCodes.FAILURE;
                    case TransportReturnCodes.READ_FD_CHANGE:
                        handleFDChange(channel);
                        break;
                    case TransportReturnCodes.READ_PING:
                        /* Update ping monitor for server message received */
                        setMsgReceived(channel);
                        break;
                    default:
                       if (_readArgs.readRetVal() < 0 && _readArgs.readRetVal() != TransportReturnCodes.READ_WOULD_BLOCK)
                       {
                            System.out.println("Read error=" + error.text() + "<" + _readArgs.readRetVal() + ">");
                       }
                }
            }
        }
        while (_readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

        return TransportReturnCodes.SUCCESS;
    
    }

    /*
     * Sets the ReceivedClientMsg flag for a channel. 
     * chnl - The channel to set the ReceivedClientMsg flag for
     */
    private void setMsgReceived(Channel chnl)
    {
        for (ClientSessionInfo clientSessionInfo : clientSessions)
        {
            if (clientSessionInfo.clientChannel == chnl)
            {
                clientSessionInfo.pingHandler.receivedMsg();
                break;
            }
        }
    }

    /*
     * Init pinghandler for the client channel that just become active.
     * chnl - The channel to init pinghandler for.
     * 
     */
    private void initPingHandler(Channel chnl)
    {
        for (ClientSessionInfo clientSessionInfo : clientSessions)
        {
            if (clientSessionInfo.clientChannel == chnl)
            {
                clientSessionInfo.pingHandler.initPingHandler(clientSessionInfo.clientChannel.pingTimeout());
                break;
            }
        }
    }
    
    private void handleFDChange(Channel channel)
    {
        System.out.println("Read() Channel Change - Old Channel: "
                + channel.oldSelectableChannel() + " New Channel: "
                + channel.selectableChannel());
        /* cancel old channel read select */
        try
        {
            SelectionKey key = channel.oldSelectableChannel().keyFor(selector);
            key.cancel();           
        }
        catch (Exception e)
        {
 
        } // old channel may be null so ignore

        /* add new channel read select */
        try
        {
            channel.selectableChannel().register(selector, SelectionKey.OP_READ, channel);
        }
        catch (Exception e)
        {
        }
    }
    
    private int reRegister(Channel channel, InProgInfo inProg, Object att, Error error)
    {
        /* cancel old channel read select */
        try
        {
            SelectionKey key = inProg.oldSelectableChannel().keyFor(selector);
            key.cancel();
        }
        catch (Exception e)
        {
            error.text(e.getMessage());
            return CodecReturnCodes.FAILURE;
        }
        /* add new channel read select */
        try
        {                                         
        	inProg.newSelectableChannel().register(selector, SelectionKey.OP_READ , 
                    channel);            
        }
        catch (Exception e)
        {
            error.text(e.getMessage());
            return CodecReturnCodes.FAILURE;
        }

        return CodecReturnCodes.SUCCESS;
    }

    /*
     * Returns a client session for a channel. chnl - The channel to remove the
     * client session for
     */
    public ClientSessionInfo getClientSessionForChannel(Channel chnl)
    {
        for (ClientSessionInfo clientSessionInfo : clientSessions)
        {
            if (clientSessionInfo.clientChannel == chnl)
            {
                return clientSessionInfo;
            }
        }

        return null;
    }

    /**
     * Retrieves the {@link BindOptions} .
     *
     * @return {@link BindOptions}
     */
    public BindOptions getBindOptions()
    {
        return _bindOptions;
    }

    /**
     * Removes the client session for channel.
     *
     * @param chnl the chnl
     * @param error the error
     */
    /*
     * Removes a client session for a channel. chnl - The channel to remove the
     * client session for
     */
    public void removeClientSessionForChannel(Channel chnl, Error error)
    {
       for (ClientSessionInfo clientSessionInfo : clientSessions)
       {
    	   if (clientSessionInfo.clientChannel == chnl)
    	   {
    		   removeClientSession(clientSessionInfo, error);
    		   break;
    	   }    	  
       }
    }

    /*
     * Removes a client session. clientSessionInfo - The client session to be
     * removed
     */
    private void removeClientSession(ClientSessionInfo clientSessionInfo, Error error)
    {
        _clientSessionCount--;   
        removeChannel(clientSessionInfo.clientChannel, error);
    }

    /**
     * Removes the inactive client session for channel.
     *
     * @param clientSessionInfo the client session info
     * @param error the error
     */
    /*
     * Removes a inactive client session for a channel. chnl -f  The channel to remove the
     * client session for
     */
    public void removeInactiveClientSessionForChannel(ClientSessionInfo clientSessionInfo, Error error)
    {      	
    	clientSessionInfo.clear();
    }    
    
    /*
     * Removes a channel. chnl - The channel to be removed
     */
    private void removeChannel(Channel chnl, Error error)
    {    	 
        try
        {
            removeOption(chnl, SelectionKey.OP_READ);
            removeOption(chnl, SelectionKey.OP_WRITE);
        }
        catch (ClosedChannelException e)
        {
            System.out.println("selector remove write failed, Exception: " + e.getMessage());
        }
        if (chnl.state() != ChannelState.INACTIVE)
        {
       
            int ret = chnl.close(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                System.out.println("channel.close() failed with return code: " + ret);
            }
        }
    }

    private void removeOption(Channel channel, int option)
            throws ClosedChannelException
    { 	
    	if (channel.selectableChannel() == null) return;
    	
        SelectionKey key = channel.selectableChannel().keyFor(selector);
        if (key == null || !key.isValid())
            return;
        if ((option & key.interestOps()) == 0)
            return;
        int newoption = key.interestOps() - option;
        if (newoption != 0)
            channel.selectableChannel().register(selector, newoption, channel);
        else
        {
            key.cancel();
        }
    }

    private void removeOption(Server server, int option)
            throws ClosedChannelException
    {
        SelectionKey key = server.selectableChannel().keyFor(selector);
        if (key == null || !key.isValid())
            return;
        if ((option & key.interestOps()) == 0)
            return;
        int newoption = key.interestOps() - option;
        if (newoption != 0)
            server.selectableChannel().register(selector, newoption, server);
        else
            key.cancel();
    }

    private void addOption(Channel channel, int option) throws ClosedChannelException
    {
        SelectionKey key = channel.selectableChannel().keyFor(selector);
        int newoption = option;
        int oldoption = 0;
        if (key != null)
        {
            oldoption = key.interestOps();
            newoption |= oldoption;
        }
        channel.selectableChannel().register(selector, newoption, channel);
    }

    /**
     * Writes the content of the {@link TransportBuffer} to the ETA channel.
     *
     * @param channel the channel
     * @param msgBuf the msg buf
     * @param error the error
     * @return {@link TransportReturnCodes}
     * @see Channel#write(TransportBuffer, WriteArgs, Error)
     */
    public int write(Channel channel, TransportBuffer msgBuf, Error error)
    {
        if (channel == null)
            return TransportReturnCodes.FAILURE;

        if (shouldXmlTrace)
        {
            _xmlDIter.clear();
            _xmlDIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
            System.out.println(_xmlMsg.decodeToXml(_xmlDIter, _dictionaryForXml));
        }

        _writeArgs.clear();
        _writeArgs.priority(WritePriorities.HIGH);
        _writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);

        // write data to the channel
        int retval = channel.write(msgBuf, _writeArgs, error);

        if (retval > TransportReturnCodes.FAILURE)
        {
            setMsgSent(channel);
            /*
             * The write was successful and there is more data queued in ETA
             * Transport. Use selector to be notified when output space becomes
             * available on a connection.
             */
            if (retval > TransportReturnCodes.SUCCESS)
                return regForWriteNotification(channel, error);
        }
        else
        {
            // Handle return codes appropriately, not all return values are
            // failure conditions
            switch (retval)
            {
                case TransportReturnCodes.SUCCESS:
                {
                    // Successful write and all data has been passed to the
                    // connection
                    // Continue with next operations.
                }
                    break;
                case TransportReturnCodes.WRITE_CALL_AGAIN:
                {
                    /*
                     * Large buffer is being split by transport, but out of
                     * output buffers
                     */
                    /*
                     * Schedule a call to flush and then call the write method
                     * again with this same exact buffer to continue the
                     * fragmentation process.
                     */
                    while (retval == TransportReturnCodes.WRITE_CALL_AGAIN)
                    {
                        retval = channel.flush(error);
                        if (retval < TransportReturnCodes.SUCCESS)
                            System.out.println("channel flush failed with returned code: " + retval + " - " + error.text());
                        retval = channel.write(msgBuf, _writeArgs,
                                               error);
                    }

                    /*
                     * The write was successful and there is more data queued in
                     * ETA Transport. Use selector to be notified when output
                     * space becomes available on a connection.
                     */
                    if (retval > TransportReturnCodes.SUCCESS)
                    {
                        return regForWriteNotification(channel, error);
                    }
                }
                    break;
                case TransportReturnCodes.WRITE_FLUSH_FAILED:
                {
                    /*
                     * The write was successful, but an attempt to flush failed.
                     * ETA will release buffer.
                     */
                    if (channel.state() == ChannelState.CLOSED)
                    {
                        error.text("Error (" + error.errorId() + ") (errno: " + error.sysError() + ") encountered with write. Error text: " + error.text());

                        // Connection should be closed, return failure
                        return CodecReturnCodes.FAILURE;
                    }
                    else
                    {

                        /*
                         * The write was successful and there is more data
                         * queued in ETA Transport. Use selector to be notified
                         * when output space becomes available on a connection.
                         */

                        return regForWriteNotification(channel, error);
                    }
                }
                case TransportReturnCodes.INIT_NOT_INITIALIZED:
                case TransportReturnCodes.FAILURE:
                {
                    /* write failed, release buffer */
                    String writeError = error.text();
                    channel.releaseBuffer(msgBuf, error);
                    error.text(writeError);
                    return TransportReturnCodes.FAILURE;
                }
                default:
                    System.out.println("Unexpected return code (" + retval + ") encountered!");
                    return retval;
            }
        }

        return CodecReturnCodes.SUCCESS;
    }

    private int regForWriteNotification(Channel channel, Error error)
    {
        try
        {
            addOption(channel, SelectionKey.OP_WRITE);
        }
        catch (Exception e)
        {
            error.text(e.getMessage());
            return TransportReturnCodes.FAILURE;
        }

        return TransportReturnCodes.SUCCESS;
    }

    /**
     * Handles the ping processing for all active client sessions. Sends a ping
     * to the client if the next send ping time has arrived and checks if a ping
     * has been received from a client within the next receive ping time.
     * */
    public void handlePings()
    {
        for (ClientSessionInfo clientSessionInfo : clientSessions)
        {
            if (clientSessionInfo.clientChannel != null && clientSessionInfo.clientChannel.state() == ChannelState.ACTIVE)
            {
                int ret = clientSessionInfo.pingHandler.handlePings(clientSessionInfo.clientChannel, _error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println(_error.text());
                    _error.clear();
                    removeClientSession(clientSessionInfo, _error);
                }
            }
        }
    }

    /**
     * checks if a message has been sent to the client.
     *
     * @param channel the new msg sent
     */
    public void setMsgSent(Channel channel)
    {
        for (ClientSessionInfo clientSessionInfo : clientSessions)
        {
            if (clientSessionInfo.clientChannel == channel)
            {
                clientSessionInfo.pingHandler.sentMsg();
            }
        }
    }

    /**
     * Allows the user to trace messages via XML.
     * 
     * @param dictionary dictionary for XML tracing (can be null).
     */
    public void enableXmlTrace(DataDictionary dictionary)
    {
        _dictionaryForXml = dictionary;
        shouldXmlTrace = true;
    }
    
    /**
     * Allows the user to enable write locking for accepted channels.
     */
    public void enableChannelWriteLocking()
    {
        _enableChannelWriteLocking = true;
    }
        
}
