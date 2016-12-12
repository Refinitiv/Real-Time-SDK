package com.thomsonreuters.upa.examples.edfexamples.edfconsumer;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.common.ResponseCallback;
import com.thomsonreuters.upa.examples.common.LoginHandler;
import com.thomsonreuters.upa.examples.edfexamples.common.DirectoryHandler;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFDictionaryHandler;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFChannelSession;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFWatchList;
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
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service.ServiceSeqMcastInfo;

/** 
 * Handles interactions with the ref data server. 
*/

public class RefDataSession implements ResponseCallback
{
    
    /* Session structure */
    public class RefDataSessionState 
    {
        static final int REF_DATA_STATE_START = 1;
        static final int REF_DATA_STATE_LOGIN_REQUESTED = 2;
        static final int REF_DATA_STATE_READY = 3;
        static final int REF_DATA_STATE_FINISHED = 4;
    }

    int state = RefDataSessionState.REF_DATA_STATE_START;
    
    private static final int REFDATA_SES_CONNECTION_RETRY_TIME = 15; // seconds
    
    /* Time at which application will exit. */
    static long runtime;
    
    private EDFWatchList watchlist;
    
    // indicates if requested service is up
    private boolean requestsSent;
    
    private LoginHandler loginHandler;
    private EDFDictionaryHandler dictionaryHandler;
    private DirectoryHandler srcDirHandler;
    private Buffer symbolListName;
    private ChannelInfo channelInfo;
    
    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private Msg responseMsg = CodecFactory.createMsg();
    
    private static Error error = TransportFactory.createError();    // error information
    
    static EDFChannelSession channelSession = new EDFChannelSession();
    
    /* End of session Structure */

    private ServiceSeqMcastInfo seqMcastInfo = new ServiceSeqMcastInfo();
    
    static  int LOGIN_STREAM_ID = 1;
    
    static int fieldDictionaryStreamId, enumDictionaryStreamId, globalSetDefDictionaryStreamId = 0;
    
    int SymbolListCounter = 0;
    
    public RefDataSession( EDFWatchList watchlist)
    {
        channelInfo = TransportFactory.createChannelInfo();
        loginHandler = new LoginHandler();
        srcDirHandler = new DirectoryHandler();
        dictionaryHandler = new EDFDictionaryHandler();
        symbolListName = CodecFactory.createBuffer();
        symbolListName.data(ByteBuffer.allocate(100));
        channelSession = new EDFChannelSession(this);
        error = TransportFactory.createError();
        this.watchlist = watchlist;
    }
    
    private static void closeChannel()
    {
        channelSession.uninit(error);
    }

    /**
     * Initializes EDF consumer's Ref Data Session
     * 
     * It is responsible for: Initializing command line options used by the
     * application. Parsing command line arguments. Initializing connection. Sending
     * login request. Initializing ping handler.
     * 
     * @param args
     */
    void init(String[] args)
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
        
        // display product version information
        System.out.println(Codec.queryVersion().toString());
        System.out.println("RefDataSession initializing...");

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

        // set service name in directory handler
        srcDirHandler.serviceInfo().serviceId(Integer.valueOf(CommandLine.value("serviceId")));

        if (channelSession.initTransport(false, error) < CodecReturnCodes.SUCCESS)
            System.exit(error.errorId());

        // load dictionary
        dictionaryHandler.loadDictionary();

        // enable XML tracing
        if (CommandLine.booleanValue("x"))
        {
            channelSession.enableXmlTrace(dictionaryHandler.dictionary());
        }

        PingHandler pingHandler = new PingHandler();
        InProgInfo inProg = TransportFactory.createInProgInfo();

        try
        {
            connectRetry(inProg);
        }
        catch (InterruptedException intExp)
        {
            System.out.println("Thread: " + Thread.currentThread() + " interrupted. Error:" + intExp.getLocalizedMessage());
            return;
        }

        // Handle run-time
        if (System.currentTimeMillis() >= runtime)
        {
            System.out.println("RefDataSession run-time expired...");
            return;
        }

        if (channelSession.channel().info(channelInfo, error) != TransportReturnCodes.SUCCESS)
        {
            System.out.println("Channel.info() failed");
            closeChannel();
            return;
        }
        
        System.out.printf("Channel Info:\n" +
                    "  Max Fragment Size: %d\n" +
                    "  Output Buffers: %d Max, %d Guaranteed\n" +
                    "  Input Buffers: %d\n" +
                    "  Send/Recv Buffer Sizes: %d/%d\n" +
                    "  Ping Timeout: %d\n" +
                    "  Connected component version: ",
                              channelInfo.maxFragmentSize(),
                              channelInfo.maxOutputBuffers(), channelInfo.guaranteedOutputBuffers(),
                              channelInfo.numInputBuffers(),
                              channelInfo.sysSendBufSize(), channelInfo.sysRecvBufSize(),
                          channelInfo.pingTimeout());

        loginHandler.applicationName("RefDataSession");
        loginHandler.role(Login.RoleTypes.CONS);

        // Send login request message only after initalization, then set to Ready state

        if (loginHandler.sendRequest(channelSession, error) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
        
        // Initialize ping handler
        pingHandler.initPingHandler(channelSession.channel().pingTimeout());

        state = RefDataSessionState.REF_DATA_STATE_READY;
    }

    // connection recovery loop
    private void connectRetry(InProgInfo inProg) throws InterruptedException
    {
        String hostName = CommandLine.value("rdsa");
        String portNumber = CommandLine.value("rdsp");

        while (System.currentTimeMillis() < runtime && channelSession.shouldRecoverConnection())
        {
            System.out.println("Starting connection to Ref Data Session...");
            
            requestsSent = false;

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
                Thread.sleep(REFDATA_SES_CONNECTION_RETRY_TIME * 1000);
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
        while (System.currentTimeMillis() < runtime && channelSession.channelState() != ChannelState.ACTIVE)
        {
            if (channelSession.initChannel(inProg, error) < TransportReturnCodes.SUCCESS)
            {
                System.out.println("Error initializing channel, will retry: " + error.text());
            }
            if (channelSession.channel() == null || channelSession.channelState() == ChannelState.ACTIVE)
                break;

            Thread.sleep(1000);
        }
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
     * Closes all streams for the ref data session.
     */
    public void uninitialize()
    {
        System.out.println("RefDataSession unitializing...");
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
        dIter.setBufferAndRWFVersion(buffer, chnl.channel().majorVersion(), chnl.channel().minorVersion());

        int ret = responseMsg.decode(dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("\nDecodeMsg(): Error " + ret + " on SessionData Channel="
                    + chnl.channel().selectableChannel() + "  Size " + (buffer.data().limit() - buffer.data().position()));
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
            case DomainTypes.SOURCE:
                processSourceDirectoryResp(chnl, responseMsg);
                break;
            case DomainTypes.DICTIONARY:
                processDictionaryResp(chnl, responseMsg, dIter);
                break;
            default:
                System.out.println("Unhandled Domain Type: " + responseMsg.domainType());
                break;
        }
    }
    
    private void processDictionaryResp(ChannelSession chnl, Msg responseMsg, DecodeIterator dIter)
    {
        if (dictionaryHandler.processResponse(chnl.channel(), responseMsg, dIter, error) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }

        if (dictionaryHandler.isFieldDictionaryLoaded() &&
                dictionaryHandler.isEnumTypeDictionaryLoaded() &&
                dictionaryHandler.isGlobalDefsDictionaryLoaded() &&
                responseMsg.msgClass() == MsgClasses.REFRESH)
        {
            watchlist.dictionary(dictionaryHandler.dictionary(), dictionaryHandler.fieldSetDefDb());
            
            System.out.println("Dictionary ready.");
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
            ret = srcDirHandler.sendRequest(chnl, error);
            if (ret != CodecReturnCodes.SUCCESS)
            {
                System.out.println("Error sending directory request: " + error.text());
                closeChannel();
                System.exit(TransportReturnCodes.FAILURE);
            }
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

                // reopen directory stream, which in turn reopens other streams
                // (dict, item streams)
                ret = srcDirHandler.closeStream(channelSession, error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("Error closing directory stream: " + error.text());
                    closeChannel();
                    System.exit(TransportReturnCodes.FAILURE);
                }

                ret = srcDirHandler.sendRequest(chnl, error);
                if (ret != CodecReturnCodes.SUCCESS)
                {
                    System.out.println("Error sending directory request: " + error.text());
                    closeChannel();
                    System.exit(TransportReturnCodes.FAILURE);
                }
            }
            // login suspect from single-open provider: provider handles
            // recovery. consumer does nothing in this case.
        }
    
    }
    
    private void processSourceDirectoryResp(ChannelSession chnl, Msg responseMsg)
    {
        int ret = srcDirHandler.processResponse(chnl.channel(), responseMsg, dIter, error);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
        
        seqMcastInfo = srcDirHandler.serviceInfo().seqMcastInfo();

        if (srcDirHandler.isRequestedServiceUp())
        {
            watchlist.setServiceInfo(srcDirHandler.serviceInfo());
            sendRequests(chnl);
        }
        else
        {
            // service not up or
            // previously up service went down
            requestsSent = false;

            System.out.println("Requested service '" + CommandLine.value("s") + "' not up. Waiting for service to be up...");
        }
        
        srcDirHandler.serviceInfo().info().itemList().copy(symbolListName);
        symbolListName.data().limit(srcDirHandler.serviceInfo().info().itemList().length());
        
        state = RefDataSessionState.REF_DATA_STATE_FINISHED;
    }
    
    private void sendRequests(ChannelSession chnl)
    {
        if (requestsSent)
            return;

        // first load dictionaries. send item requests and post
        // messages only after dictionaries are loaded.
        if (!isDictionariesLoaded())
        {
            sendDictionaryRequests(chnl);
            return;
        }
        
        requestsSent = true;
    }
    
    private boolean isDictionariesLoaded()
    {
        return dictionaryHandler.isFieldDictionaryLoaded()
                && dictionaryHandler.isEnumTypeDictionaryLoaded()
                && dictionaryHandler.isGlobalDefsDictionaryLoaded();
    }

    private void sendDictionaryRequests(ChannelSession chnl)
    {
        dictionaryHandler.serviceId(srcDirHandler.serviceInfo().serviceId());
        
        if (dictionaryHandler.sendRequests(chnl, CommandLine.value("setDefDictName"), error) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }
    
    private void closeDictAndItemStreams()
    {
        // close dictionary streams if opened
        dictionaryHandler.closeStreams(channelSession, error);
    }
    
    public ServiceSeqMcastInfo seqMcastInfo()
    {
        return seqMcastInfo;
    }
    
    public Service service()
    {
        return srcDirHandler.serviceInfo();
    }
    
    public EDFDictionaryHandler dictionary()
    {
        return dictionaryHandler;
    }
    
    public EDFChannelSession channelSession()
    {
        return channelSession;
    }
    
    public EDFDictionaryHandler dictionaryHandler()
    {
        return dictionaryHandler;
    }
    
    public DirectoryHandler directoryHandler()
    {
        return srcDirHandler;
    }
    
    public Buffer symbolListName()
    {
        return symbolListName;
    }
    
}
