package com.thomsonreuters.upa.examples.edfexamples.edfconsumer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.ElementListFlags;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKeyFlags;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.common.ResponseCallback;
import com.thomsonreuters.upa.examples.common.LoginHandler;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFDictionaryHandler;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFChannelSession;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFChannelSession.GapInfo;
import com.thomsonreuters.upa.shared.CommandLine;
import com.thomsonreuters.upa.shared.ConsumerLoginState;
import com.thomsonreuters.upa.shared.PingHandler;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service.ServiceSeqMcastInfo;

/**
 *  Handles interactions with the gap request server. 
 *  
*/

public class GapRequestSession implements ResponseCallback
{
    /* Session structure */
    public class GapRequestSessionState 
    {
        static final int GAPREQUEST_STATE_START = 1;
        static final int GAPREQUEST_STATE_READY = 2;
        static final int GAPREQUEST_STATE_FINISHED = 3;
    }

    Buffer CELOXICA_ENAME_ADDRESS  = CodecFactory.createBuffer();
    Buffer CELOXICA_ENAME_PORT  = CodecFactory.createBuffer();
    Buffer CELOXICA_ENAME_GAPSTART = CodecFactory.createBuffer();
    Buffer CELOXICA_ENAME_GAPEND = CodecFactory.createBuffer();
    
    static final int GAPREQUEST_CUSTOM_DATATYPE = 128;

    
    long runtime;
    int state = GapRequestSessionState.GAPREQUEST_STATE_START;
    
    int streamId = 0;

    private static final int GAPREQUEST_SES_CONNECTION_RETRY_TIME = 15; // seconds

    private LoginHandler loginHandler;
    private EDFDictionaryHandler dictionaryHandler;
    private ChannelInfo channelInfo;

    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private Msg responseMsg = CodecFactory.createMsg();

    static EDFChannelSession channelSession = new EDFChannelSession();
    private static Error error = TransportFactory.createError(); // error
                                                                 // information

    private ServiceSeqMcastInfo seqMcastInfo;
    
    Msg requestMsg = CodecFactory.createMsg();
    EncodeIterator encIter = CodecFactory.createEncodeIterator();
    ElementList elementList = CodecFactory.createElementList();
    ElementEntry elementEntry = CodecFactory.createElementEntry();
    Buffer address = CodecFactory.createBuffer();
    UInt port = CodecFactory.createUInt();
    UInt gapStart = CodecFactory.createUInt();
    UInt gapEnd = CodecFactory.createUInt();
    TransportBuffer msgBuf;

    public GapRequestSession()
    {
        channelInfo = TransportFactory.createChannelInfo();
        loginHandler = new LoginHandler();
        dictionaryHandler = new EDFDictionaryHandler();
        channelSession = new EDFChannelSession(this);
        error = TransportFactory.createError();
    }

    private static void closeChannel()
    {
        channelSession.uninit(error);
    }

    /**
     * Initializes EDF consumer's Gap Request Session
     * 
     * It is responsible for: Initializing command line options used by the
     * application. Parsing command line arguments. Initializing connection. Sending
     * login request. Initializing ping handler.
     * 
     * @param args
     */
    public void init(String[] args, RefDataSession refDataSession)
    {
        // process command line args
        EDFConsumer.addCommandLineArgs();
        try
        {
            CommandLine.parseArgs(args);
        }
        catch (IllegalArgumentException ile)
        {
            System.err.println("Error loading command line arguments:\t");
            System.err.println(ile.getMessage());
            System.err.println();
            System.err.println(CommandLine.optionHelpString());
            System.exit(CodecReturnCodes.FAILURE);
        }

        try
        {
            runtime = System.currentTimeMillis() + CommandLine.intValue("runtime") * 1000;
        }
        catch (NumberFormatException ile)
        {
            System.err.println("Invalid argument, number expected.\t");
            System.err.println(ile.getMessage());
            System.exit(CodecReturnCodes.FAILURE);
        }

        if (channelSession.initTransport(false, error) < CodecReturnCodes.SUCCESS)
            System.exit(error.errorId());

        // load dictionary
        dictionaryHandler = refDataSession.dictionary();

        // enable XML tracing
        if (CommandLine.booleanValue("x"))
        {
            channelSession.enableXmlTrace(dictionaryHandler.dictionary());
        }

        seqMcastInfo = refDataSession.seqMcastInfo();

        PingHandler pingHandler = new PingHandler();
        InProgInfo inProg = TransportFactory.createInProgInfo();
        
        try
        {
            connectRetry(inProg);
        }
        catch (InterruptedException intExp)
        {
            System.out.println("Thread: " + Thread.currentThread() + " interrupted. Error:"
                    + intExp.getLocalizedMessage());
            return;
        }

        // Handle run-time
        if (System.currentTimeMillis() >= runtime)
        {
            System.out.println("SnapshotSession  run-time expired...");
            return;
        }

        if (channelSession.channel().info(channelInfo, error) != TransportReturnCodes.SUCCESS)
        {
            System.out.println("Channel.info() failed");
            closeChannel();
            return;
        }
        
        System.out.printf("Channel Info:\n" + "  Max Fragment Size: %d\n"
                                  + "  Output Buffers: %d Max, %d Guaranteed\n"
                                  + "  Input Buffers: %d\n"
                                  + "  Send/Recv Buffer Sizes: %d/%d\n"
                                  + "  Ping Timeout: %d\n" + "  Connected component version: ",
                          channelInfo.maxFragmentSize(),
                          channelInfo.maxOutputBuffers(),
                          channelInfo.guaranteedOutputBuffers(), channelInfo.numInputBuffers(),
                          channelInfo.sysSendBufSize(), channelInfo.sysRecvBufSize(),
                          channelInfo.pingTimeout());
        int count = channelInfo.componentInfo().size();
        if (count == 0)
            System.out.printf("(No component info)");
        else
        {
            for (int i = 0; i < count; ++i)
            {
                System.out.println(channelInfo.componentInfo().get(i).componentVersion());
                if (i < count - 1)
                    System.out.printf(", ");
            }
        }

        loginHandler.applicationName("GapRequestSession");
        loginHandler.role(Login.RoleTypes.CONS);

        // Send login request message only after initalization, then set to ready state
        if (loginHandler.sendRequest(channelSession, error) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        // Initialize ping handler
        pingHandler.initPingHandler(channelSession.channel().pingTimeout());
        
        // Set buffers for gap request names
        CELOXICA_ENAME_ADDRESS.data("Address");
        CELOXICA_ENAME_PORT.data("Port");
        CELOXICA_ENAME_GAPSTART.data("GapStart");
        CELOXICA_ENAME_GAPEND.data("GapEnd");

        state = GapRequestSessionState.GAPREQUEST_STATE_FINISHED;
    }

    // connection recovery loop
    private void connectRetry(InProgInfo inProg) throws InterruptedException
    {
        String hostName; 
        if ((hostName = CommandLine.value("grsa")) == null)
            hostName =  new String(seqMcastInfo.gapRecoveryServerAddress().data().array());
        String portNumber;
        if ((portNumber= CommandLine.value("grsp")) == null)
            portNumber = seqMcastInfo.gapRecoveryServerPort().toString();

        while (System.currentTimeMillis() < runtime && channelSession.shouldRecoverConnection())
        {
            System.out.println("Starting connection to Gap Request Session...");

            // get connect options from the channel session
            ConnectOptions copts = channelSession.getConnectOptions();

            // set the connection parameters on the connect options
            copts.unifiedNetworkInfo().address(hostName);
            copts.unifiedNetworkInfo().serviceName(portNumber);

            channelSession.connect(inProg, error);

            // connection hand-shake loop
            waitUntilChannelActive(inProg);
            if (channelSession.shouldRecoverConnection())
            {
                /* sleep before trying to recover connection */
                Thread.sleep(GAPREQUEST_SES_CONNECTION_RETRY_TIME * 1000);
                continue;
            }
        }
    }

    /*
     * Wait for channel to become active. This finalizes the three-way
     * handshake.
     */
    private void waitUntilChannelActive(InProgInfo inProg) throws InterruptedException
    {
        while (System.currentTimeMillis() < runtime
                && channelSession.channelState() != ChannelState.ACTIVE)
        {
            if (channelSession.initChannel(inProg, error) < TransportReturnCodes.SUCCESS)
            {
                System.out.println("Error initializing channel, will retry: " + error.text());
            }
            if (channelSession.channel() == null
                    || channelSession.channelState() == ChannelState.ACTIVE)
                break;

            Thread.sleep(1000);
        }
    }

    /**
     * Call back method to process responses from channel. Processing responses
     * consists of performing a high level decode of the message and then
     * calling the applicable specific method for further processing. 
     * @param chnl - The channel of the response 
     * @param buffer - The message buffer containing the response.
     */
    
    @Override
    public void processResponse(ChannelSession chnl, TransportBuffer buffer)
    {
        // clear decode iterator
        dIter.clear();

        // set buffer and version info
        dIter.setBufferAndRWFVersion(buffer, chnl.channel().majorVersion(), chnl.channel()
                .minorVersion());

        int ret = responseMsg.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("\nDecodeMsg(): Error " + ret + " on SessionData Channel="
                    + chnl.channel().selectableChannel() + "  Size "
                    + (buffer.data().limit() - buffer.data().position()));
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        processResponse(chnl, responseMsg, dIter);
    }

    private void processResponse(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
    {
        switch (responseMsg.domainType())
        {
            case DomainTypes.LOGIN:
                processLoginResp(chnl, responseMsg, dIter);
                break;
            case GAPREQUEST_CUSTOM_DATATYPE:
                processGapResp(chnl, responseMsg, dIter);
                break;
            default:
                System.out.println("Unhandled Domain Type: " + responseMsg.domainType());
                break;
        }
    }
    
    private void processGapResp(ChannelSession chnl, Msg responseMsg2, DecodeIterator dIter2)
    {
        switch (responseMsg2.msgClass())
        {
            case MsgClasses.STATUS:
                if (((StatusMsg)responseMsg2).checkHasState())
                {
                   System.out.println("<GapRequest Channel> Received StatusMsg for stream " + responseMsg.streamId());
                   System.out.println("        " + ((StatusMsg)responseMsg2).state().toString());
                }
                break;
            default:
                System.out.println("Unhandled Message Type: " + responseMsg.msgClass());
        }
    }

    private void processLoginResp(ChannelSession chnl, Msg responseMsg2, DecodeIterator dIter2)
    {

        int ret = loginHandler.processResponse(responseMsg, dIter, error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        // Handle login states
        ConsumerLoginState loginState = loginHandler.loginState();
        if (loginState == ConsumerLoginState.OK_SOLICITED)
        {
            System.out.println("Login was successful");
        }
        else if (loginState == ConsumerLoginState.CLOSED)
        {
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
        else if (loginState == ConsumerLoginState.CLOSED_RECOVERABLE)
        {
            ret = channelSession.recoverConnection(error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("Error recovering connection: " + error.text());
                System.exit(TransportReturnCodes.FAILURE);
            }
        }
        else if (loginState == ConsumerLoginState.SUSPECT)
        {
            if (!loginHandler.refreshInfo().checkHasAttrib() || // default
                                                                // behavior when
                                                                // singleopen
                                                                // attrib not
                                                                // set
                    loginHandler.refreshInfo().attrib().singleOpen() == 0)
            {
                // login suspect from no single-open provider: 1) close source
                // directory stream and item streams. 2) reopen streams
                closeDictAndItemStreams();
            }
            // login suspect from single-open provider: provider handles
            // recovery. consumer does nothing in this case.
        }

    }
    
    public  int sendRequests(ChannelSession tcpChnl, EDFChannelSession EDFChannel, boolean isRealGapDetected)
    {
        int ret = 0;
        GapInfo currGapInfo = EDFChannel.channelInfo().gapInfo();
        
        
        if (!isRealGapDetected) // Not a real gap, for demonstration purposes use the first real time session only
        {
            for (EDFChannelSession.ChannelInfo chanInfo : EDFChannel.channels())
            {
                if (chanInfo.callback().getClass() == RealTimeSession.class)
                {
                        chanInfo.gapInfo().address.copy(currGapInfo.address);
                        currGapInfo.port = chanInfo.gapInfo().port;
                        currGapInfo.start = chanInfo.gapInfo().start;
                        currGapInfo.end = chanInfo.gapInfo().start;
    
                        break;
                }
            }
        }
        else // Real gap detected
        {
            for (EDFChannelSession.ChannelInfo chanInfo : EDFChannel.channels())
            {
                if (chanInfo.connectOptions().unifiedNetworkInfo().address() == EDFChannel.channelInfo().connectOptions().unifiedNetworkInfo().address() &&
                    chanInfo.connectOptions().unifiedNetworkInfo().serviceName() == EDFChannel.channelInfo().connectOptions().unifiedNetworkInfo().serviceName())
                    {
                        chanInfo.gapInfo().address.copy(currGapInfo.address);
                        currGapInfo.port = chanInfo.gapInfo().port;
                        currGapInfo.start = chanInfo.gapInfo().start;
                        currGapInfo.end = chanInfo.gapInfo().end;
                        chanInfo.gapInfo().start = chanInfo.gapInfo().end + 1;
    
                        break;
                    }
            }
        }
        
        if (currGapInfo == null)
            return CodecReturnCodes.FAILURE;
        
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(streamId++);
        requestMsg.domainType(GAPREQUEST_CUSTOM_DATATYPE);
        requestMsg.containerType(DataTypes.NO_DATA);
        
        requestMsg.msgKey().flags(MsgKeyFlags.HAS_SERVICE_ID | MsgKeyFlags.HAS_ATTRIB);
        requestMsg.msgKey().serviceId(0);
        requestMsg.msgKey().attribContainerType(DataTypes.ELEMENT_LIST);
        
        encIter.clear();
        
        msgBuf = tcpChnl.getTransportBuffer(3000, false, error);

        if ((ret = encIter.setBufferAndRWFVersion(msgBuf, tcpChnl.getConnectOptions().majorVersion(), tcpChnl.getConnectOptions().minorVersion())) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("Encode Iterator setBufferAndRWFVersion() failed while sending gap fill request");
            return ret;
        }

        // Encode request message
        if ((ret = requestMsg.encodeInit(encIter, 0)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("requestMsg encodeInit() failed while sending gap fill request");
            return ret;
        }

        // Encode the element list
        elementList.clear();
        elementList.flags(ElementListFlags.HAS_STANDARD_DATA);
        if ((ret = elementList.encodeInit(encIter, null, 0)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("ElementList encodeInit() failed while sending gap fill request");
            return ret;
        }
        
        // Address
        address.clear();
        address.data(currGapInfo.address.data());
        
        elementEntry.clear();
        elementEntry.dataType(DataTypes.ASCII_STRING);
        elementEntry.name(CELOXICA_ENAME_ADDRESS);
        if ((ret = elementEntry.encode(encIter, address)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("ElementEntry encode() failed while sending gap fill request");
            return ret;
        }
        
        // Port
        port.clear();
        port.value(currGapInfo.port);
        
        elementEntry.clear();
        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(CELOXICA_ENAME_PORT);
        if ((ret = elementEntry.encode(encIter, port)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("ElementEntry encode() failed while sending gap fill request");
            return ret;
        }
        
        // GapStart
        gapStart.clear();
        gapStart.value(currGapInfo.start);
        
        elementEntry.clear();
        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(CELOXICA_ENAME_GAPSTART);
        if ((ret = elementEntry.encode(encIter, gapStart)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("ElementEntry encode() failed while sending gap fill request");
            return ret;
        }
        
        // GapEnd
        gapEnd.clear();
        gapEnd.value(currGapInfo.end);
        
        elementEntry.clear();
        elementEntry.dataType(DataTypes.UINT);
        elementEntry.name(CELOXICA_ENAME_GAPEND);
        if ((ret = elementEntry.encode(encIter, gapEnd)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("ElementEntry encode() failed while sending gap fill request");
            return ret;
        }
        
        // Complete element list
        if ((ret = elementList.encodeComplete(encIter, true)) < CodecReturnCodes.SUCCESS)
        {
            System.out.println("ElementEntry encodeComplete() failed while sending gap fill request");
            return ret;
        }
        
        // Complete encode key
        if ((ret = requestMsg.encodeKeyAttribComplete(encIter, true)) > CodecReturnCodes.ENCODE_CONTAINER)
        {
            System.out.println("Request Msg encodeKeyAttribComplete() failed while sending gap fill request");
            return ret;
        }

        // Complete encode message
        if ((ret = requestMsg.encodeComplete(encIter, true)) > CodecReturnCodes.SUCCESS)
        {
            System.out.println("Request Msg encodeComplete() failed while sending gap fill request");
            return ret;
        }
 
        // Write the message
        if ((ret = tcpChnl.write(msgBuf, error)) > CodecReturnCodes.SUCCESS)
            return ret;
        
        System.out.println("<GapRequest Channel> Sent gap fill request for message SEQ.NO. Start-End: " +
                gapStart + "-" + gapEnd + " on StreamId " + requestMsg.streamId());
        
        return CodecReturnCodes.SUCCESS;
    }

    private void closeDictAndItemStreams()
    {
        // close dictionary streams if opened
        dictionaryHandler.closeStreams(channelSession, error);
    }
    
    
    private void flushChannel()
    {
        int retval = 1;

        while (retval > TransportReturnCodes.SUCCESS)
        {
            retval = channelSession.flush(error);
        }

        if (retval < TransportReturnCodes.SUCCESS)
        {
            System.out.println("Flush() failed with return code " + retval + "- <" + error
                    .text() + ">");
        }
    }
    
    /**
     * Closes all streams for the Gap request session.
     */
    public void uninitialize()
    {
        System.out.println("GapRequestSession unitializing...");
        if (channelSession.channel() == null)
        {
            channelSession.uninit(error);
            System.exit(TransportReturnCodes.SUCCESS);
        }

        closeDictAndItemStreams();

        // close login stream
        loginHandler.closeStream(channelSession, error);

        // flush before exiting
        flushChannel();

        closeChannel();
    }
    
    
    public EDFChannelSession channelSession()
    {
        return channelSession;
    }
}
