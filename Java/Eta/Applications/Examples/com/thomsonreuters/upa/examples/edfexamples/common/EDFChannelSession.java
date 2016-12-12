package com.thomsonreuters.upa.examples.edfexamples.common;

import java.io.IOException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;
import java.util.Set;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.common.ResponseCallback;
import com.thomsonreuters.upa.shared.PingHandler;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgFlags;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.InitArgs;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.transport.WriteFlags;
import com.thomsonreuters.upa.transport.WritePriorities;

/**
 * Manages UPA EDF channel and provides methods for connection establishment,
 * channel initialization, read, and write methods for EDF consumer example.
 * <P>
 * 
 * Usage of this class within the EDF Consumer application:<br>
 * 1. Call {@link #initTransport(boolean, Error)} to initialize UPA <br>
 * 2. Call {@link #connect(InProgInfo, Error)}.<br>
 * 3. Call {@link #initChannel(InProgInfo, Error)} for TCP connections in loop until channel becomes
 * active that is channel state is ChannelState.ACTIVE. <br>
 * 4. When channel state is ChannelState.ACTIVE for TCP connections, send Login Request. <br>
 * 5. Read and process responses.
 * 
 * <p>
 * This class is used in the EDF Consumer main method also to store a list of
 * all other EDFChannelSessions used. This list is stored so that all of these EDFChannelSessions
 * can be added to one Selector used only by the EDF Consumer to read from all channels.
 * 
 * <p>
 * This class also detects connection failures in TCP and provides a query method for
 * the application to retry connection establishment.
 * 
 * <p>
 * This class is a wrapper utility class for UPA's Channel and Transport classes
 * and handles typical return codes from UPA, for example, file descriptor
 * changes during read, connection failures, registering for write when there is
 * more data in the queue, flushing of data after write, flush failures.
 * 
 * @see Transport#initialize(InitArgs, Error)
 * @see Transport#connect(ConnectOptions, Error)
 * @see Channel#init(InProgInfo, Error)
 * @see Channel#read(ReadArgs, Error)
 * @see Channel#write(TransportBuffer, WriteArgs, Error)
 */
public class EDFChannelSession extends ChannelSession
{
    public static final int MAX_MSG_SIZE = 1024;
    public static final int NUM_CLIENT_SESSIONS = 5;
    private boolean shouldRecoverConnection = true;
    
    public class ChannelInfo
    {
        private Selector selector;
        private Channel channel;
        private ReadArgs readArgs = TransportFactory.createReadArgs();
        private ConnectOptions copts = TransportFactory.createConnectOptions();
        private ResponseCallback callback;
        private GapInfo gapInfo = new GapInfo();
        
        public ReadArgs readArgs()
        {
            return readArgs;
        }
        
        public Channel channel()
        {
            return channel;
        }
        
        public ConnectOptions connectOptions()
        {
            return copts;
        }
        
        public void connectOptions(ConnectOptions newCopts)
        {
            copts = newCopts;
        }
        
        public GapInfo gapInfo()
        {
            return gapInfo;
        }

        public void gapInfo(GapInfo newGapInfo)
        {
            newGapInfo.address.copy(gapInfo.address);
            gapInfo.end = newGapInfo.end;
            gapInfo.port = newGapInfo.port;
            gapInfo.start = newGapInfo.start;
        }
        
        public ResponseCallback callback()
        {
            return callback;
        }

    }
    
    /* Gap Info */
    public class GapInfo
    {
        public long start;
        public long end;
        public Buffer address;
        public long port;
        
        public GapInfo()
        {
            start =0;
            end = 0;
            address = CodecFactory.createBuffer();
            port = 0;
        }
    }
    
    ChannelInfo channelInfo = new ChannelInfo();
    
    private List<ChannelInfo> Channels = new ArrayList<ChannelInfo>();

    private long selectTime = -1;
    private boolean userSpecifiedSelectTime = false;

    private Msg xmlMsg = CodecFactory.createMsg();
    private DecodeIterator xmlIter = CodecFactory.createDecodeIterator();
    private boolean shouldXmlTrace = false;
    private DataDictionary dictionaryForXml;
    private WriteArgs writeArgs = TransportFactory.createWriteArgs();
    
    public EDFChannelSession()
    {
        channelInfo.copts.clear();
    }
    
    public EDFChannelSession(ResponseCallback responseCallback)
    {
        channelInfo.copts.clear();
        channelInfo.callback = responseCallback;
    }

    /**
     * Allows the user to specify a select timeout. The default select timeout
     * will be one third of the channel's negotiated ping time out. Providers
     * will use this method to control their content update time.
     * 
     * @param selectTime a value greater than zero.
     * @return true if the selectTime was greater than zero.
     */
    public boolean selectTime(long selectTime)
    {
        if (selectTime <= 0)
            return false;
        
        userSpecifiedSelectTime = true;
        this.selectTime = selectTime;
        return true;
    }

    /**
     * Allows the user to trace messages via XML.
     * 
     * @param dictionary dictionary for XML tracing (can be null).
     */
    public void enableXmlTrace(DataDictionary dictionary)
    {
        dictionaryForXml = dictionary;
        shouldXmlTrace = true;
    }

    /**
     * Initializes the UPA transport API and all internal members.<BR>
     * 
     * This is the first method called when using the UPA. It initializes
     * internal data structures.
     * 
     * @param globalLock flag to enable global locking on UPA Transport
     * @param error UPA Error, to be populated in event of an error
     * 
     * @return {@link TransportReturnCodes}
     */
    public int initTransport(boolean globalLock, Error error)
    {
        try
        {
            channelInfo.selector = Selector.open();
        }
        catch (Exception exception)
        {
            error.text(exception.getMessage());
            error.errorId(TransportReturnCodes.FAILURE);
            return TransportReturnCodes.FAILURE;
        }
        /* Initialize */
        channelInfo.copts.guaranteedOutputBuffers(500);
        channelInfo.copts.majorVersion(Codec.majorVersion());
        channelInfo.copts.minorVersion(Codec.minorVersion());
        channelInfo.copts.protocolType(Codec.protocolType());
        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(globalLock);
        return Transport.initialize(initArgs, error);
    }

    public int flush(Error error)
    {
        if (channelState() == ChannelState.INACTIVE)
            return TransportReturnCodes.SUCCESS;
        return channelInfo.channel.flush(error);
    }

    /**
     * Returns underlying UPA channel.
     * 
     * @return UPA channel
     */
    public Channel channel()
    {
        return channelInfo.channel;
    }

    /**
     * Retrieves the {@link TransportBuffer} from channel.
     * 
     * @param size - Size of the transport buffer to retrieve.
     * @param packedBuffer - Set to true if you plan on packing multiple
     *            messages into the same buffer
     * @param error UPA error information in case of failure.
     * @return {@link TransportReturnCodes}
     */
    public TransportBuffer getTransportBuffer(int size, boolean packedBuffer, Error error)
    {
        if (channelInfo.channel == null)
            return null;
        return channelInfo.channel.getBuffer(size, packedBuffer, error);
    }

    /**
     * Retrieves the {@link ConnectOptions} 
     * 
     * @return {@link ConnectOptions}
     */
    public ConnectOptions getConnectOptions()
    {
        return channelInfo.copts;
    }

    /**
     * Transport channel state.
     * 
     * @return {@link ChannelState}
     */
    public int channelState()
    {
        return channelInfo.channel == null ? ChannelState.INACTIVE : channelInfo.channel.state();
    }

    /**
     * @return true if connection recovery flag is set during channel connection
     *         failure.
     */
    public boolean shouldRecoverConnection()
    {
        return shouldRecoverConnection;
    }

    /**
     * Cleans up channel and sets connection recovery flag to true.
     * 
     * @param error UPA error information when channel cleanup fails.
     * @return {@link TransportReturnCodes}
     */
    public int recoverConnection(Error error)
    {
        /* set connection recovery flag */
        shouldRecoverConnection = true;

        /* clean up channel */
        return close(error);
    }

    /**
     * Reads from a channel.
     * 
     * @param pingHandler - PingHandler's flag for server message received is
     *            set when read is done.
     * @param error - UPA error information in case of read failure.
     * @return {@link TransportReturnCodes}
     */
    public int read(PingHandler pingHandler, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        Set<SelectionKey> keySet = null;
        try
        {
            if (channelInfo.selector.select(selectTime) > 0)
            {
                keySet = channelInfo.selector.selectedKeys();
            }
        }
        catch (IOException e1)
        {
            error.text(e1.getMessage());
            return TransportReturnCodes.FAILURE;
        }

        // nothing to read
        if (keySet == null)
            return TransportReturnCodes.SUCCESS;

        Iterator<SelectionKey> iter = keySet.iterator();
        
        while (iter.hasNext())
        {
            SelectionKey key = iter.next();
            iter.remove();
            try
            {
                channelInfo.channel = (Channel)key.attachment();
                if (key.isReadable())
                {   
                    if (channelInfo.channel.state() == ChannelState.ACTIVE || 
                            channelInfo.channel.connectionType() == ConnectionTypes.SEQUENCED_MCAST)
                    {
                        for (ChannelInfo inChanInfo : Channels)
                        {
                            if (inChanInfo.channel == channelInfo.channel)
                            {
                                channelInfo.connectOptions(inChanInfo.connectOptions());
                                ret = readInt(pingHandler, inChanInfo.callback, error);
                                if (ret != TransportReturnCodes.SUCCESS)
                                    return ret;
                            }
                        }
                    }
                    else if (channelInfo.channel.state() == ChannelState.CLOSED)
                    {
                        // recover
                        return recoverConnection(error);
                    }
                }

                /* flush for write file descriptor and active state */
                if (key.isWritable())
                {
                    ret = channelInfo.channel.flush(error);
                    if (ret == TransportReturnCodes.SUCCESS)
                    {
                        try
                        {
                            removeOption(SelectionKey.OP_WRITE, channelInfo.channel);
                        }
                        catch (Exception e)
                        {
                            error.text(e.getMessage());
                            return TransportReturnCodes.FAILURE;
                        }
                    }
                }
            }
            catch (CancelledKeyException e)
            {
            } // key can be canceled during shutdown
        }
        
        return TransportReturnCodes.SUCCESS;
    }

    private int reRegister(InProgInfo inProg, Object att, Error error)
    {
        /* cancel old channelInfo.channel read select */
        try
        {
            SelectionKey key;
            if (channelInfo.channel.connectionType() != ConnectionTypes.SEQUENCED_MCAST)
                key = channelInfo.channel.oldSelectableChannel().keyFor(channelInfo.selector);
            else
                key = channelInfo.channel.selectableChannel().keyFor(channelInfo.selector);
            key.cancel();
        }
        catch (Exception e)
        {
            error.text(e.getMessage());
            return TransportReturnCodes.FAILURE;
        }
        /* add new channelInfo.channel read select */
        try
        {
            channelInfo.channel.selectableChannel().register(channelInfo.selector, SelectionKey.OP_READ | SelectionKey.OP_WRITE,
                                                       channelInfo.channel);
        }
        catch (Exception e)
        {
            error.text(e.getMessage());
            return TransportReturnCodes.FAILURE;
        }

        return TransportReturnCodes.SUCCESS;
    }

    /**
     * Initializes UPA non-blocking channelInfo.channel.
     * 
     * @param inProg {@link InProgInfo}
     * @param error UPA error information in case of failure
     * 
     * @return {@link TransportReturnCodes}
     * 
     * @see Channel#init(InProgInfo, Error)
     */
    public int initChannel(InProgInfo inProg, Error error)
    {
        if (channelInfo.channel == null)
            return TransportReturnCodes.FAILURE;
        int retval = channelInfo.channel.init(inProg, error);
        if (retval < TransportReturnCodes.SUCCESS)
        {
            System.out.println("Error initializing  channelInfo.channel: " + error.text() + ". Will retry shortly.");
            return recoverConnection(error);
        }

        switch (retval)
        {
            case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                if (inProg.flags() == InProgFlags.SCKT_CHNL_CHANGE)
                {
                    return reRegister(inProg, channelInfo.channel, error);
                }

                break;
            case TransportReturnCodes.SUCCESS:
                // If the user did not specify a select time, use one third
                // the channelInfo.channel's negotiated ping timeout.
                if (!userSpecifiedSelectTime)
                {
                    selectTime = channelInfo.channel.pingTimeout() / 3;
                }
                shouldRecoverConnection = false;
                break;
            default:
                uninit(error);
                error.text("handleChannelInit: Error - Invalid return code: " + TransportReturnCodes.toString(retval));
                return TransportReturnCodes.FAILURE;
        }

        return retval;
    }

    /**
     * Closes the UPA channelInfo.channel and uninitilizes Transport
     * 
     * @param error error information in case of failure
     * 
     * @return {@link TransportReturnCodes}
     */
    public int uninit(Error error)
    {
        /* clean up channelInfo.channel */
        int ret = close(error);
        if (ret != TransportReturnCodes.SUCCESS)
            return ret;

        /* clear the connect options */
        channelInfo.copts.clear();
        
        return Transport.uninitialize();
    }

    private int readInt(PingHandler pingHandler, ResponseCallback callbackclient, Error error)
    {
        TransportBuffer msgBuf;
        do /* read until no more to read */
        {
            msgBuf = channelInfo.channel.read(channelInfo.readArgs, error);
            if (msgBuf != null)
            {
                if (shouldXmlTrace)
                {
                    xmlIter.clear();
                    xmlIter.setBufferAndRWFVersion(msgBuf, channelInfo.channel.majorVersion(), channelInfo.channel.minorVersion());
                    System.out.println(xmlMsg.decodeToXml(xmlIter, dictionaryForXml));
                }
                
                callbackclient.processResponse(this, msgBuf);

                //set flag for server message received
                pingHandler.receivedMsg();
            }
            else
            {
                if (channelInfo.readArgs.readRetVal() == TransportReturnCodes.FAILURE)
                {
                    System.out.println("channelInactive portno="
                            + channelInfo.channel.selectableChannel() + "<"
                            + error.text() + ">" + ". Will retry shortly.");

                    return recoverConnection(error);
               }
                else if (channelInfo.readArgs.readRetVal() == TransportReturnCodes.READ_FD_CHANGE)
                {
                    handleFDChange();
                }
                else if (channelInfo.readArgs.readRetVal() == TransportReturnCodes.READ_PING)
                {
                    //set flag for server message received
                    pingHandler.receivedMsg();
                }
            }
        }
        while (channelInfo.readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

        return TransportReturnCodes.SUCCESS;
    }

    private void handleFDChange()
    {
        System.out.println("Read() channelInfo.channel Change - Old channelInfo.channel: "
                + channelInfo.channel.oldSelectableChannel() + " New channelInfo.channel: "
                + channelInfo.channel.selectableChannel());
        
        // cancel old channelInfo.channel read select
        try
        {
            SelectionKey key;
            if (channelInfo.channel.connectionType() != ConnectionTypes.SEQUENCED_MCAST)
                key = channelInfo.channel.oldSelectableChannel().keyFor(channelInfo.selector);
            else
                key = channelInfo.channel.selectableChannel().keyFor(channelInfo.selector);
            key.cancel();
        }
        catch (Exception e)
        {
        } // old channelInfo.channel may be null so ignore

        //add new channelInfo.channel read select
        try
        {
            channelInfo.channel.selectableChannel().register(channelInfo.selector, SelectionKey.OP_READ | SelectionKey.OP_WRITE,
                                                       channelInfo.channel);
        }
        catch (Exception e)
        {
        }
    }

    private int close(Error error)
    {
        int cstate = channelState();
        if (cstate != ChannelState.INACTIVE && cstate != ChannelState.CLOSED)
        {
            /* cancel channelInfo.channel select */
            for (SelectionKey key : channelInfo.selector.keys())
            {
                if (key.isValid())
                {
                    /* flush before exiting */
                    if (key.isWritable())
                    {
                        int ret = channelInfo.channel.flush(error);
                        if (ret < TransportReturnCodes.SUCCESS)
                        {
                            System.out.println("channelInfo.channel flush failed with return code: " + ret + "  - " + error.text());
                        }
                    }
                    key.cancel();
                }
            }
            int ret = channelInfo.channel.close(error);
            if (ret != TransportReturnCodes.SUCCESS)
            {
                return ret;
            }

            channelInfo.channel = null;
            
            // Close all channels
            for (ChannelInfo channelInfo : Channels)
            {
                ret = channelInfo.channel.close(error);
                if (ret != TransportReturnCodes.SUCCESS)
                {
                    return ret;
                }
                
                channelInfo.channel = null;
            }

        }
        return TransportReturnCodes.SUCCESS;
    }
    
    /**
     * Writes the content of the {@link TransportBuffer} to the UPA channelInfo.channel.
     * 
     * @param msgBuf
     * @param error
     * @return {@link TransportReturnCodes}
     * @see Channel#write(TransportBuffer, WriteArgs, Error)
     */
    public int write(TransportBuffer msgBuf, Error error)
    {
        if (channelInfo.channel == null)
            return TransportReturnCodes.FAILURE;

        if (shouldXmlTrace)
        {
            xmlIter.clear();
            xmlIter.setBufferAndRWFVersion(msgBuf, channelInfo.channel.majorVersion(), channelInfo.channel.minorVersion());
            System.out.println(xmlMsg.decodeToXml(xmlIter, dictionaryForXml));
        }

        // write data to the channelInfo.channel
        writeArgs.clear();
        writeArgs.priority(WritePriorities.HIGH);
        writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
        int retval = channelInfo.channel.write(msgBuf, writeArgs,
                               error);

        if (retval > TransportReturnCodes.FAILURE)
        {
            if (retval > TransportReturnCodes.SUCCESS)
            {
                // register for write if there's still data queued
                try
                {
                    addOption(SelectionKey.OP_WRITE, channelInfo.channel);
                }
                catch (Exception e)
                {
                    error.text(e.getMessage());
                    return TransportReturnCodes.FAILURE;
                }
            }
        }
        else
        {
            if (retval == TransportReturnCodes.WRITE_CALL_AGAIN)
            {
                //call flush and write again until there is data in the queue
                while (retval == TransportReturnCodes.WRITE_CALL_AGAIN)
                {
                    retval = channelInfo.channel.flush(error);
                    if (retval < TransportReturnCodes.SUCCESS)
                        System.out.println("channelInfo.channel flush failed with returned code: " + retval + " - " + error.text());
                    retval = channelInfo.channel.write(msgBuf, writeArgs,
                                           error);
                }

                //register for write if there's still data queued
                if (retval > TransportReturnCodes.SUCCESS)
                {
                    try
                    {
                        addOption(SelectionKey.OP_WRITE, channelInfo.channel);
                    }
                    catch (Exception e)
                    {
                        error.text(e.getMessage());
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }
            else if (retval == TransportReturnCodes.WRITE_FLUSH_FAILED && channelInfo.channel.state() != ChannelState.CLOSED)
            {
                /*
                 * register for write if flush failed so that we can be notified
                 * when write buffer is available for write/flush again
                 */
                try
                {
                    addOption(SelectionKey.OP_WRITE, channelInfo.channel);
                }
                catch (Exception e)
                {
                    error.text(e.getMessage());
                    return TransportReturnCodes.FAILURE;
                }
            }
            else
            {
                /* write failed, release buffer */
                String writeError = error.text();
                channelInfo.channel.releaseBuffer(msgBuf, error);
                error.text(writeError);
                return retval;
            }
        }

        return TransportReturnCodes.SUCCESS;
    }

    private void addOption(int option, Object attachment) throws ClosedChannelException
    {
        SelectionKey key = channelInfo.channel.selectableChannel().keyFor(channelInfo.selector);
        int newoption = option;
        int oldoption = 0;
        if (key != null)
        {
            oldoption = key.interestOps();
            newoption |= oldoption;
        }
        channelInfo.channel.selectableChannel().register(channelInfo.selector, newoption, attachment);
    }

    private void removeOption(int option, Object attachment)
            throws ClosedChannelException
    {
        SelectionKey key = channelInfo.channel.selectableChannel().keyFor(channelInfo.selector);
        if (key == null)
            return;
        if ((option & key.interestOps()) == 0)
            return;
        int newoption = key.interestOps() - option;
        if (newoption != 0)
            channelInfo.channel.selectableChannel().register(channelInfo.selector, newoption, attachment);
        else
            key.cancel();
    }

    /**
     * Establishes the outbound connection to the UPA server. If connection is
     * established, registers the channelInfo.channel for read and write select operation.
     * It recovers connection in case of connection failure. It assumes
     * application has called {@link #initTransport(boolean, Error)} before calling this
     * method.
     * <p>
     * Typical usage is for establishing connection is:<br>
     * 1. Call {@link #connect(InProgInfo, Error)}.<br>
     * 2. Call {@link #initChannel(InProgInfo, Error)} in loop until channelInfo.channel
     * becomes active, ChannelState.ACTIVE
     * 
     * @param inProg - {@link InProgInfo}
     * @param error - UPA error information in case of connection failure.
     * @return {@link TransportReturnCodes}
     * @see Transport#connect(ConnectOptions, Error)
     */
    public int connect(InProgInfo inProg, Error error)
    {
        String logstr = null;
        if (channelInfo.copts.connectionType() == ConnectionTypes.SEQUENCED_MCAST)
        {
            if (channelInfo.copts.segmentedNetworkInfo().recvAddress() != null)
            {
                logstr = "Attempting segmented connect...\n";
                logstr +=String.format("Reading from: %s:%s unicastPort %s...\n", 
                                       channelInfo.copts.segmentedNetworkInfo().recvAddress(),
                                       channelInfo.copts.segmentedNetworkInfo().recvServiceName(),
                                       channelInfo.copts.segmentedNetworkInfo().unicastServiceName());
                if (channelInfo.copts.segmentedNetworkInfo().sendAddress() != null)
                {
                    logstr += String.format("Writing to: %s:%s",
                                           channelInfo.copts.segmentedNetworkInfo().sendAddress(),
                                           channelInfo.copts.segmentedNetworkInfo().sendServiceName());
                }
            }
        }
        else if (channelInfo.copts.connectionType() != ConnectionTypes.RELIABLE_MCAST)
        {
            logstr = String.format("Attempting to connect to server %s:%s ...",
                                    channelInfo.copts.unifiedNetworkInfo().address(), 
                                    channelInfo.copts.unifiedNetworkInfo().serviceName() );
        }
        else
        {
            logstr = String.format("Attempting segmented connect to server %s:%s  %s:%s unicastPort %s...",
                                    channelInfo.copts.segmentedNetworkInfo().sendAddress(),
                                    channelInfo.copts.segmentedNetworkInfo().sendServiceName(),
                                    channelInfo.copts.segmentedNetworkInfo().recvAddress(),
                                    channelInfo.copts.segmentedNetworkInfo().recvServiceName(),
                                    channelInfo.copts.segmentedNetworkInfo().unicastServiceName());
        }

        System.out.println( logstr );

        if ((channelInfo.channel = Transport.connect(channelInfo.copts, error)) == null)
        {
            System.out.println("Connection failure: " + error.text() + ". Will retry shortly.");
            return recoverConnection(error);
        }
       
        shouldRecoverConnection = false;
        return TransportReturnCodes.SUCCESS;
    }
    
    /**
     * 
     * @return channelInfo
     */
    
    public ChannelInfo channelInfo()
    {
        return channelInfo;
    }
    
    /**
     * Allows the user to add specific channel Infos to this channel's list of ChannelInfo, as well as registers them to 
     * this channel's Selector.
     * 
     * @param chnlInfo
     */
    public void addAndRegisterChannelInfo(ChannelInfo chnlInfo)
    {
        try
        {
            chnlInfo.channel.selectableChannel().register(channelInfo.selector, SelectionKey.OP_READ | SelectionKey.OP_WRITE,
                    chnlInfo.channel);
        
            Channels.add(chnlInfo);
        }
        catch (ClosedChannelException e)
        {
            e.printStackTrace();
        }
    }
    
    /**
     * Allows the user to specify the connect options.
     * 
     * @param connectOptions connect options to set
     */
    public void connectOptions(ConnectOptions connectOptions)
    {
        channelInfo.copts.blocking(connectOptions.blocking());
        channelInfo.copts.channelReadLocking(connectOptions.channelReadLocking());
        channelInfo.copts.channelWriteLocking(connectOptions.channelWriteLocking());
        channelInfo.copts.compressionType(connectOptions.compressionType());
        channelInfo.copts.connectionType(connectOptions.connectionType());
        channelInfo.copts.guaranteedOutputBuffers(connectOptions.guaranteedOutputBuffers());
        channelInfo.copts.numInputBuffers(connectOptions.numInputBuffers());
        if (connectOptions.sysSendBufSize() > 0)
        {
            channelInfo.copts.sysSendBufSize(connectOptions.sysSendBufSize());
        }
        if (connectOptions.sysRecvBufSize() > 0)
        {
            channelInfo.copts.sysRecvBufSize(connectOptions.sysRecvBufSize());
        }
        channelInfo.copts.pingTimeout(connectOptions.pingTimeout());
        
        if (connectOptions.userSpecObject() != null)
        {
            channelInfo.copts.userSpecObject(connectOptions.userSpecObject());
        }
    }

    /**
     * Allows the user to specify the connection type.
     * 
     * @param connectionType connection type to set
     */
    public void setConnectionType(int connectionType)
    {
        channelInfo.copts.connectionType(connectionType);
    }
    
    public long selectTime()
    {
        return selectTime;
    }
    
    public List<ChannelInfo> channels()
    {
        return Channels;
    }
}