/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.common;

import java.io.IOException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.shared.JsonConverterInitOptions;
import com.refinitiv.eta.shared.JsonSession;
import com.refinitiv.eta.shared.PingHandler;
import com.refinitiv.eta.shared.network.ChannelHelper;
import com.refinitiv.eta.transport.*;
import com.refinitiv.eta.transport.Error;

/**
 * Manages ETA channel and provides methods for connection establishment,
 * channel initialization, read, and write methods for Consumer and NIProvider
 * example applications.
 * <P>
 *
 * Usage of this class within the Consumer and NIProvider applications:<br>
 * 1. Call {@link #initTransport(boolean, Error)} to initialize ETA <br>
 * 2. Call {@link #connect(InProgInfo, Error)}.<br>
 * 3. Call {@link #initChannel(InProgInfo, Error)} in loop until channel becomes
 * active that is channel state is ChannelState.ACTIVE. <br>
 * 4. When channel state is ChannelState.ACTIVE, send Login Request. <br>
 * 5. Read and process responses.
 *
 * <p>
 * This class also detects connection failures and provides a query method for
 * the application to retry connection establishment.
 *
 * <p>
 * This class is a wrapper utility class for ETA's Channel and Transport classes
 * and handles typical return codes from ETA, for example, file descriptor
 * changes during read, connection failures, registering for write when there is
 * more data in the queue, flushing of data after write, flush failures.
 *
 * @see Transport#initialize(InitArgs, Error)
 * @see Transport#connect(ConnectOptions, Error)
 * @see Channel#init(InProgInfo, Error)
 * @see Channel#read(ReadArgs, Error)
 * @see Channel#write(TransportBuffer, WriteArgs, Error)
 */
public class ChannelSession
{
    public static final int MAX_MSG_SIZE = 1024;
    public static final int NUM_CLIENT_SESSIONS = 5;
    private boolean shouldRecoverConnection = true;

    private Selector selector;
    private Channel channel;
    private ReadArgs readArgs = TransportFactory.createReadArgs();
    private ConnectOptions copts = TransportFactory.createConnectOptions();

    private long selectTime = -1;
    private int selectRetVal;
    private boolean userSpecifiedSelectTime = false;

    private XmlTraceDump xmlTraceDump = CodecFactory.createXmlTraceDump();
    private boolean shouldXmlTrace = false;
    private DataDictionary dictionaryForXml;
    private WriteArgs writeArgs = TransportFactory.createWriteArgs();

    public long loginReissueTime; // represented by epoch time in milliseconds
    public boolean canSendLoginReissue;
    public boolean isLoginReissue;

    private int socketFdValue;
    private JsonSession jsonSession;
    private JsonConverterInitOptions converterInitOptions;
    private StringBuilder dump = new StringBuilder();

    /**
     * Instantiates a new channel session.
     */
    public ChannelSession()
    {
        copts.clear();
        jsonSession = new JsonSession();
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
     * Initializes the ETA transport API and all internal members.<BR>
     *
     * This is the first method called when using the ETA. It initializes
     * internal data structures.
     *
     * @param globalLock flag to enable global locking on ETA Transport
     * @param error ETA Error, to be populated in event of an error
     *
     * @return {@link TransportReturnCodes}
     */
    public int initTransport(boolean globalLock, Error error)
    {
        try
        {
            selector = Selector.open();
        }
        catch (Exception exception)
        {
            error.text(exception.getMessage());
            error.errorId(TransportReturnCodes.FAILURE);
            return TransportReturnCodes.FAILURE;
        }
        /* Initialize */
        copts.guaranteedOutputBuffers(500);
        copts.majorVersion(Codec.majorVersion());
        copts.minorVersion(Codec.minorVersion());
        copts.protocolType(Codec.protocolType());
        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(globalLock);
        return Transport.initialize(initArgs, error);
    }

    /**
     * Flush.
     *
     * @param error the error
     * @return the int
     */
    public int flush(Error error)
    {
        if (channelState() == ChannelState.INACTIVE)
            return TransportReturnCodes.SUCCESS;
        return channel.flush(error);
    }

    /**
     * Returns underlying ETA channel.
     *
     * @return ETA channel
     */
    public Channel channel()
    {
        return channel;
    }

    /**
     * Retrieves the {@link TransportBuffer} from channel.
     *
     * @param size - Size of the transport buffer to retrieve.
     * @param packedBuffer - Set to true if you plan on packing multiple
     *            messages into the same buffer
     * @param error ETA error information in case of failure.
     * @return {@link TransportReturnCodes}
     */
    public TransportBuffer getTransportBuffer(int size, boolean packedBuffer, Error error)
    {
        if (channel == null)
            return null;
        return channel.getBuffer(size, packedBuffer, error);
    }

    /**
     * Retrieves the {@link ConnectOptions} .
     *
     * @return {@link ConnectOptions}
     */
    public ConnectOptions getConnectOptions()
    {
        return copts;
    }

    /**
     * Transport channel state.
     *
     * @return {@link ChannelState}
     */
    public int channelState()
    {
        return channel == null ? ChannelState.INACTIVE : channel.state();
    }

    /**
     * Should recover connection.
     *
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
     * @param error ETA error information when channel cleanup fails.
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
     * @param callbackclient - Callback client to call when message is received.
     * @param error - ETA error information in case of read failure.
     * @return {@link TransportReturnCodes}
     */
    public int read(PingHandler pingHandler, ResponseCallback callbackclient, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        if (channel == null)
            return TransportReturnCodes.FAILURE;
        Set<SelectionKey> keySet = null;
        try
        {
            selectRetVal = selector.select(selectTime);
            if (selectRetVal > 0)
            {
                keySet = selector.selectedKeys();
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
                if (key.isReadable())
                {
                    if (channel.state() == ChannelState.ACTIVE)
                    {
                        ret = readInt(pingHandler, callbackclient, error);
                        if (ret != TransportReturnCodes.SUCCESS)
                            return ret;
                    }
                    else if (channel.state() == ChannelState.CLOSED)
                    {
                        // recover
                        return recoverConnection(error);
                    }
                }

                /* flush for write file descriptor and active state */
                if (key.isWritable())
                {
                    ret = channel.flush(error);
                    if (ret == TransportReturnCodes.SUCCESS)
                    {
                        try
                        {
                            removeOption(SelectionKey.OP_WRITE, channel);
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
        /* cancel old channel read select */
        try
        {
            SelectionKey key = inProg.oldSelectableChannel().keyFor(selector);
            key.cancel();
        }
        catch (Exception e)
        {
            error.text(e.getMessage());
            return TransportReturnCodes.FAILURE;
        }
        /* add new channel read select */
        try
        {
            channel.selectableChannel().register(selector, SelectionKey.OP_READ | SelectionKey.OP_WRITE,
                    channel);
        }
        catch (Exception e)
        {
            error.text(e.getMessage());
            return TransportReturnCodes.FAILURE;
        }

        //define new fd value
        socketFdValue = ChannelHelper.defineFdValueOfSelectableChannel(channel.selectableChannel());

        return TransportReturnCodes.SUCCESS;
    }

    /**
     * Initializes ETA non-blocking channel.
     *
     * @param inProg {@link InProgInfo}
     * @param error ETA error information in case of failure
     *
     * @return {@link TransportReturnCodes}
     *
     * @see Channel#init(InProgInfo, Error)
     */
    public int initChannel(InProgInfo inProg, Error error)
    {
        if (channel == null)
            return TransportReturnCodes.FAILURE;
        int retval = channel.init(inProg, error);
        if (retval < TransportReturnCodes.SUCCESS)
        {
            System.out.println("Error initializing  channel: " + error.text() + ". Will retry shortly.");
            return recoverConnection(error);
        }

        switch (retval)
        {
            case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                if (inProg.flags() == InProgFlags.SCKT_CHNL_CHANGE)
                {
                    return reRegister(inProg, channel, error);
                }

                break;
            case TransportReturnCodes.SUCCESS:
                // If the user did not specify a select time, use one third
                // the channel's negotiated ping timeout.
                if (!userSpecifiedSelectTime)
                {
                    selectTime = channel.pingTimeout() / 3;
                }
                //define fd value of channel
                socketFdValue = ChannelHelper.defineFdValueOfSelectableChannel(channel.selectableChannel());
                shouldRecoverConnection = false;
                if (channel.protocolType() == Codec.JSON_PROTOCOL_TYPE) {
                    jsonSession.initialize(channel, converterInitOptions, error);
                }
                break;
            default:
                uninit(error);
                error.text("handleChannelInit: Error - Invalid return code: " + TransportReturnCodes.toString(retval));
                return TransportReturnCodes.FAILURE;
        }

        return retval;
    }

    /**
     * Closes the ETA channel and uninitilizes Transport.
     *
     * @param error error information in case of failure
     * @return {@link TransportReturnCodes}
     */
    public int uninit(Error error)
    {
        /* clean up channel */
        int ret = close(error);
        if (ret != TransportReturnCodes.SUCCESS)
            return ret;

        /* clear the connect options */
        copts.clear();

        return Transport.uninitialize();
    }

    private int readInt(PingHandler pingHandler, ResponseCallback callbackclient, Error error)
    {
        TransportBuffer msgBuf;
        do /* read until no more to read */
        {
            msgBuf = channel.read(readArgs, error);
            if (msgBuf != null)
            {
                if (shouldXmlTrace)
                {
                    dump.setLength(0);
                    dump.append("\nRead message: ");
                    xmlTraceDump.dumpBuffer(channel, channel.protocolType(), msgBuf, dictionaryForXml, dump, error);
                    System.out.println(dump.toString());
                }
                callbackclient.processResponse(this, msgBuf);

                //set flag for server message received
                pingHandler.receivedMsg();
            }
            else
            {
                if (readArgs.readRetVal() == TransportReturnCodes.FAILURE)
                {
                    System.out.println("channelInactive portno="
                            + channel.selectableChannel() + "<"
                            + error.text() + ">" + ". Will retry shortly.");

                    return recoverConnection(error);
                }
                else if (readArgs.readRetVal() == TransportReturnCodes.READ_FD_CHANGE)
                {
                    handleFDChange();
                }
                else if (readArgs.readRetVal() == TransportReturnCodes.READ_PING)
                {
                    //set flag for server message received
                    pingHandler.receivedMsg();
                }
            }
        }
        while (readArgs.readRetVal() > TransportReturnCodes.SUCCESS);

        return TransportReturnCodes.SUCCESS;
    }

    private void handleFDChange()
    {
        System.out.println("Read() Channel Change - Old Channel: "
                + channel.oldSelectableChannel() + " New Channel: "
                + channel.selectableChannel());

        // cancel old channel read select
        try
        {
            SelectionKey key = channel.oldSelectableChannel().keyFor(selector);
            key.cancel();
        }
        catch (Exception e)
        {
        } // old channel may be null so ignore

        //add new channel read select
        try
        {
            channel.selectableChannel().register(selector, SelectionKey.OP_READ | SelectionKey.OP_WRITE,
                    channel);
        }
        catch (Exception e)
        {
        }
    }

    private int close(Error error)
    {
        int cstate = channelState();
        if (cstate != ChannelState.INACTIVE)
        {
            /* cancel channel select */
            for (SelectionKey key : selector.keys())
            {
                if (key.isValid())
                {
                    /* flush before exiting */
                    if (key.isWritable())
                    {
                        int ret = channel.flush(error);
                        if (ret < TransportReturnCodes.SUCCESS)
                        {
                            System.out.println("Channel flush failed with return code: " + ret + "  - " + error.text());
                        }
                    }
                }
                key.cancel();
            }
            // call select to ensure channel is immediately de-registered from selector after canceling keys
            try
            {
                selector.selectNow();
            }
            catch (IOException e) { }
            int ret = channel.close(error);
            if (ret != TransportReturnCodes.SUCCESS)
            {
                return ret;
            }

            channel = null;

        }
        return TransportReturnCodes.SUCCESS;
    }



    /**
     * Writes the content of the {@link TransportBuffer} to the ETA channel.
     *
     * @param msgBuf the msg buf
     * @param error the error
     * @return {@link TransportReturnCodes}
     * @see Channel#write(TransportBuffer, WriteArgs, Error)
     */
    public int write(TransportBuffer msgBuf, Error error)
    {
        TransportBuffer tempBuf = msgBuf;
        if (channel == null)
            return TransportReturnCodes.FAILURE;

        if (shouldXmlTrace)
        {
            dump.setLength(0);
            dump.append("\nWriting message (RWF):");
            xmlTraceDump.dumpBuffer(channel, Codec.RWF_PROTOCOL_TYPE, tempBuf, dictionaryForXml, dump, error);
            System.out.println(dump.toString());
        }

        if (channel.protocolType() == Codec.JSON_PROTOCOL_TYPE) {
        	
            if( jsonSession.convertToJson(msgBuf, error) != CodecReturnCodes.SUCCESS)
            {
            	return TransportReturnCodes.FAILURE;
            }
            
            tempBuf = jsonSession.getTransportJsonBuffer(error);
            
            if(tempBuf == null)
            {
            	return TransportReturnCodes.FAILURE;
            }

            if (shouldXmlTrace)
            {
                dump.setLength(0);
                dump.append("\nWriting message (JSON):");
                xmlTraceDump.dumpBuffer(channel, channel.protocolType(), tempBuf, dictionaryForXml, dump, error);
                System.out.println(dump.toString());
            }
        }

        // write data to the channel
        writeArgs.clear();
        writeArgs.priority(WritePriorities.HIGH);
        writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
        int retval = channel.write(tempBuf, writeArgs,
                error);

        if (retval > TransportReturnCodes.FAILURE)
        {
            if (retval > TransportReturnCodes.SUCCESS)
            {
                // register for write if there's still data queued
                try
                {
                    addOption(SelectionKey.OP_WRITE, channel);
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
                    retval = channel.flush(error);
                    if (retval < TransportReturnCodes.SUCCESS)
                        System.out.println("Channel flush failed with returned code: " + retval + " - " + error.text());
                    retval = channel.write(tempBuf, writeArgs,
                            error);
                }

                //register for write if there's still data queued
                if (retval > TransportReturnCodes.SUCCESS)
                {
                    try
                    {
                        addOption(SelectionKey.OP_WRITE, channel);
                    }
                    catch (Exception e)
                    {
                        error.text(e.getMessage());
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }
            else if (retval == TransportReturnCodes.WRITE_FLUSH_FAILED && channel.state() != ChannelState.CLOSED)
            {
                /*
                 * register for write if flush failed so that we can be notified
                 * when write buffer is available for write/flush again
                 */
                try
                {
                    addOption(SelectionKey.OP_WRITE, channel);
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
                channel.releaseBuffer(tempBuf, error);
                error.text(writeError);
                return retval;
            }
        }

        return TransportReturnCodes.SUCCESS;
    }

    private void addOption(int option, Object attachment) throws ClosedChannelException
    {
        SelectionKey key = channel.selectableChannel().keyFor(selector);
        int newoption = option;
        int oldoption = 0;
        if (key != null)
        {
            oldoption = key.interestOps();
            newoption |= oldoption;
        }
        channel.selectableChannel().register(selector, newoption, attachment);
    }

    private void removeOption(int option, Object attachment)
            throws ClosedChannelException
    {
        SelectionKey key = channel.selectableChannel().keyFor(selector);
        if (key == null)
            return;
        if ((option & key.interestOps()) == 0)
            return;
        int newoption = key.interestOps() - option;
        if (newoption != 0)
            channel.selectableChannel().register(selector, newoption, attachment);
        else
            key.cancel();
    }

    /**
     * Establishes the outbound connection to the ETA server. If connection is
     * established, registers the channel for read and write select operation.
     * It recovers connection in case of connection failure. It assumes
     * application has called {@link #initTransport(boolean, Error)} before calling this
     * method.
     * <p>
     * Typical usage is for establishing connection is:<br>
     * 1. Call {@link #connect(InProgInfo, Error)}.<br>
     * 2. Call {@link #initChannel(InProgInfo, Error)} in loop until channel
     * becomes active, ChannelState.ACTIVE
     *
     * @param inProg - {@link InProgInfo}
     * @param error - ETA error information in case of connection failure.
     * @return {@link TransportReturnCodes}
     * @see Transport#connect(ConnectOptions, Error)
     */
    public int connect(InProgInfo inProg, Error error)
    {
        String logstr = null;
        if (copts.connectionType() == ConnectionTypes.SEQUENCED_MCAST)
        {
            if (copts.segmentedNetworkInfo().recvAddress() != null)
            {
                logstr = "Attempting segmented connect...\n";
                logstr +=String.format("Reading from: %s:%s unicastPort %s...\n",
                        copts.segmentedNetworkInfo().recvAddress(),
                        copts.segmentedNetworkInfo().recvServiceName(),
                        copts.segmentedNetworkInfo().unicastServiceName());
                if (copts.segmentedNetworkInfo().sendAddress() != null)
                {
                    logstr += String.format("Writing to: %s:%s",
                            copts.segmentedNetworkInfo().sendAddress(),
                            copts.segmentedNetworkInfo().sendServiceName());
                }
            }
        }
        else if (copts.connectionType() != ConnectionTypes.RELIABLE_MCAST)
        {
            logstr = String.format("Attempting to connect to server %s:%s ...",
                    copts.unifiedNetworkInfo().address(),
                    copts.unifiedNetworkInfo().serviceName() );
        }
        else
        {
            logstr = String.format("Attempting segmented connect to server %s:%s  %s:%s unicastPort %s...",
                    copts.segmentedNetworkInfo().sendAddress(),
                    copts.segmentedNetworkInfo().sendServiceName(),
                    copts.segmentedNetworkInfo().recvAddress(),
                    copts.segmentedNetworkInfo().recvServiceName(),
                    copts.segmentedNetworkInfo().unicastServiceName());
        }

        System.out.println( logstr );

        if ((channel = Transport.connect(copts, error)) == null)
        {
            System.out.println("Connection failure: " + error.text() + ". Will retry shortly.");
            return recoverConnection(error);
        }

        try
        {
            channel.selectableChannel().register(selector, SelectionKey.OP_READ | SelectionKey.OP_WRITE,
                    channel);

        }
        catch (ClosedChannelException cce)
        {
            System.out.println("register(selector) Exception: " + cce.getMessage() + ". Will retry shortly.");
            return recoverConnection(error);
        }
        catch (Exception e)
        {
            uninit(error);
            error.text("register(selector) Exception: " + e.getMessage());
            return TransportReturnCodes.FAILURE;
        }

        shouldRecoverConnection = false;
        return TransportReturnCodes.SUCCESS;
    }

    /**
     * Allows the user to specify the connect options.
     *
     * @param connectOptions connect options to set
     */
    public void connectOptions(ConnectOptions connectOptions)
    {
        copts.blocking(connectOptions.blocking());
        copts.channelReadLocking(connectOptions.channelReadLocking());
        copts.channelWriteLocking(connectOptions.channelWriteLocking());
        copts.compressionType(connectOptions.compressionType());
        copts.connectionType(connectOptions.connectionType());
        copts.guaranteedOutputBuffers(connectOptions.guaranteedOutputBuffers());
        copts.numInputBuffers(connectOptions.numInputBuffers());
        if (connectOptions.sysSendBufSize() > 0)
        {
            copts.sysSendBufSize(connectOptions.sysSendBufSize());
        }
        if (connectOptions.sysRecvBufSize() > 0)
        {
            copts.sysRecvBufSize(connectOptions.sysRecvBufSize());
        }
        copts.pingTimeout(connectOptions.pingTimeout());

        if (connectOptions.userSpecObject() != null)
        {
            copts.userSpecObject(connectOptions.userSpecObject());
        }
    }

    /**
     * Select ret val.
     *
     * @return the int
     */
    public int selectRetVal()
    {
        return selectRetVal;
    }

    /**
     * Allows the user to specify the connection type.
     *
     * @param connectionType connection type to set
     */
    public void setConnectionType(int connectionType)
    {
        copts.connectionType(connectionType);
    }

    /**
     * Allows the user to specify the tunneling connect options.
     *
     * @param connectOptions tunneling connect options to set
     */
    public void tunnelingConnectOptions(ConnectOptions connectOptions)
    {
        copts.encryptionOptions().connectionType(connectOptions.encryptionOptions().connectionType());
        copts.tunnelingInfo().HTTPproxy(connectOptions.tunnelingInfo().HTTPproxy());
        if (connectOptions.tunnelingInfo().HTTPproxyHostName() != null)
        {
            copts.tunnelingInfo().HTTPproxyHostName(connectOptions.tunnelingInfo().HTTPproxyHostName());
        }
        copts.tunnelingInfo().HTTPproxyPort(connectOptions.tunnelingInfo().HTTPproxyPort());
        copts.tunnelingInfo().objectName(connectOptions.tunnelingInfo().objectName());
        copts.encryptionOptions().KeystoreType(connectOptions.encryptionOptions().KeystoreType());
        if (connectOptions.encryptionOptions().KeystoreFile() != null)
        {
            copts.encryptionOptions().KeystoreFile(connectOptions.encryptionOptions().KeystoreFile());
        }
        if (connectOptions.encryptionOptions().KeystorePasswd() != null)
        {
            copts.encryptionOptions().KeystorePasswd(connectOptions.encryptionOptions().KeystorePasswd());
        }
        copts.encryptionOptions().SecurityProtocol(connectOptions.encryptionOptions().SecurityProtocol());
        if (connectOptions.encryptionOptions().SecurityProtocolVersions() != null)
        {
        	copts.encryptionOptions().SecurityProtocolVersions(connectOptions.encryptionOptions().SecurityProtocolVersions());
        }
        copts.encryptionOptions().SecurityProvider(connectOptions.encryptionOptions().SecurityProvider());
        copts.encryptionOptions().KeyManagerAlgorithm(connectOptions.encryptionOptions().KeyManagerAlgorithm());
        copts.encryptionOptions().TrustManagerAlgorithm(connectOptions.encryptionOptions().TrustManagerAlgorithm());
    }

    /**
     * Allows the user to specify the proxy credentials connect options.
     *
     * @param connectOptions credentials options to set
     */
    public void credentialsConnectOptions(ConnectOptions connectOptions)
    {
        copts.credentialsInfo().HTTPproxyUsername(connectOptions.credentialsInfo().HTTPproxyUsername());
        copts.credentialsInfo().HTTPproxyPasswd(connectOptions.credentialsInfo().HTTPproxyPasswd());
        copts.credentialsInfo().HTTPproxyDomain(connectOptions.credentialsInfo().HTTPproxyDomain());
        copts.credentialsInfo().HTTPproxyLocalHostname(connectOptions.credentialsInfo().HTTPproxyLocalHostname());
        copts.credentialsInfo().HTTPproxyKRB5configFile(connectOptions.credentialsInfo().HTTPproxyKRB5configFile());
    }

    public int socketFdValue() {
        return socketFdValue;
    }

    public void setConverterInitOptions(JsonConverterInitOptions options) {
        this.converterInitOptions = options;
    }

    public int getChannelProtocolType() {
        return channel.protocolType();
    }

    public JsonSession getJsonSession() {
        return jsonSession;
    }
}