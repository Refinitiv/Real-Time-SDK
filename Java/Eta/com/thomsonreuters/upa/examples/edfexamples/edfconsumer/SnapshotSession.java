package com.thomsonreuters.upa.examples.edfexamples.edfconsumer;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.Qos;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.examples.common.ChannelSession;
import com.thomsonreuters.upa.examples.common.CommandLine;
import com.thomsonreuters.upa.examples.common.ConsumerLoginState;
import com.thomsonreuters.upa.examples.common.LoginHandler;
import com.thomsonreuters.upa.examples.common.PingHandler;
import com.thomsonreuters.upa.examples.common.ResponseCallback;
import com.thomsonreuters.upa.examples.edfexamples.common.DirectoryHandler;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFChannelSession;
import com.thomsonreuters.upa.examples.edfexamples.common.EDFWatchList;
import com.thomsonreuters.upa.examples.edfexamples.edfconsumer.SymbolListHandler.SymbolListEntry;
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
* Handles interactions with the snapshot server. 
*/

/**
* Handles interactions with the snapshot server. 
*/

public class SnapshotSession implements ResponseCallback
{
    /* Session structure */
    public class SnapshotSessionState 
    {
        static final int SNAPSHOT_STATE_START = 1;
        static final int SNAPSHOT_STATE_READY = 2;
        static final int SNAPSHOT_STATE_FINISHED = 3;
    }

    long runtime;
    int state = SnapshotSessionState.SNAPSHOT_STATE_START;

    private static final int SNAPSHOT_SES_CONNECTION_RETRY_TIME = 15; // seconds

    private LoginHandler loginHandler;
    private DictionaryHandler dictionaryHandler;
    private DirectoryHandler srcDirHandler;
    private Buffer symbolListName;
    private ChannelInfo channelInfo;

    private DecodeIterator dIter = CodecFactory.createDecodeIterator();
    private Msg responseMsg = CodecFactory.createMsg();

    static EDFChannelSession channelSession = new EDFChannelSession();
    private static Error error = TransportFactory.createError(); // error
                                                                 // information

    private ServiceSeqMcastInfo seqMcastInfo;
    
    private SymbolListHandler symbolListHandler;
    
    private EDFWatchList watchlist;

    public SnapshotSession(EDFWatchList watchlist)
    {
        channelInfo = TransportFactory.createChannelInfo();
        loginHandler = new LoginHandler();
        dictionaryHandler = new DictionaryHandler();
        symbolListHandler = new SymbolListHandler();
        srcDirHandler = new DirectoryHandler();
        channelSession = new EDFChannelSession(this);
        symbolListName = CodecFactory.createBuffer();
        symbolListName.data(ByteBuffer.allocate(100));
        error = TransportFactory.createError();
        this.watchlist = watchlist;
    }

    private static void closeChannel()
    {
        channelSession.uninit(error);
    }

    /**
     * Initializes EDF consumer's Snapshot Session
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
        
        // get Directory Handler
        srcDirHandler = refDataSession.directoryHandler();
        
        // get symbol list name
        refDataSession.symbolListName().copy(symbolListName);
        symbolListName.data().limit(refDataSession.symbolListName().data().limit());
        
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
    
        loginHandler.applicationName("SnapshotSession");
            loginHandler.role(Login.RoleTypes.CONS);
    
            // Send login request message only after initalization, then set to ready state
            if (loginHandler.sendRequest(channelSession, error) != CodecReturnCodes.SUCCESS)
            {
                closeChannel();
                System.exit(TransportReturnCodes.FAILURE);
            }
        // Initialize ping handler
        pingHandler.initPingHandler(channelSession.channel().pingTimeout());

        state = SnapshotSessionState.SNAPSHOT_STATE_READY;
    }

    // connection recovery loop
    private void connectRetry(InProgInfo inProg) throws InterruptedException
    {
        String hostName; 
        if ((hostName = CommandLine.value("ssa")) == null)
            hostName = new String(seqMcastInfo.snapshotServerAddress().data().array());
        String portNumber;
        if ((portNumber= CommandLine.value("ssp")) == null)
            portNumber = seqMcastInfo.snapshotServerPort().toString();

        while (System.currentTimeMillis() < runtime && channelSession.shouldRecoverConnection())
        {
            System.out.println("Starting connection to Snapshot Session...");

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
                Thread.sleep(SNAPSHOT_SES_CONNECTION_RETRY_TIME * 1000);
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
            case DomainTypes.MARKET_PRICE:
            case DomainTypes.MARKET_BY_ORDER:
            case DomainTypes.MARKET_BY_PRICE:
                watchlist.processMsg((Msg)responseMsg, dIter, this);
                break;
            case DomainTypes.SYMBOL_LIST:
                processSymbolListResp(responseMsg, dIter);
                break;
            default:
                System.out.println("Unhandled Domain Type: " + responseMsg.domainType());
                break;
        }
    }
    
    private void sendSymbolListRequests(ChannelSession chnl)
    {
        if (!srcDirHandler.serviceInfo().checkHasInfo())
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
        Service.ServiceInfo info = srcDirHandler.serviceInfo().info();
        Qos qos = info.qosList().get(0);
        symbolListHandler.qos().dynamic(qos.isDynamic());
        symbolListHandler.qos().rate(qos.rate());
        symbolListHandler.qos().timeliness(qos.timeliness());
        symbolListHandler.capabilities().addAll(info.capabilitiesList());
        symbolListHandler.serviceId(srcDirHandler.serviceInfo().serviceId());
        symbolListHandler.symbolListName().data(symbolListName.data());
        
        if (symbolListHandler.sendRequest(chnl, error) != CodecReturnCodes.SUCCESS)
        {
            System.out.println(error.text());
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
    }
    
    private void processSymbolListResp(Msg responseMsg, DecodeIterator dIter)
    {
        if (symbolListHandler.processResponse(responseMsg, dIter, dictionaryHandler.dictionary()) != CodecReturnCodes.SUCCESS)
        {
            closeChannel();
            System.exit(TransportReturnCodes.FAILURE);
        }
        
        // Make sure we have items we are looking for before we claim "Finished"
        if (CommandLine.hasArg("mp") && CommandLine.values("mp").size() > 0)
        {
            SymbolListEntry entry;
            Buffer temp = CodecFactory.createBuffer();
            
            for (int i = 0; i < CommandLine.values("mp").size(); ++i)
            {
                if ((entry = symbolListHandler.symbolList().get(CommandLine.values("mp").get(i))) != null)
                {
                    temp.data(CommandLine.values("mp").get(i));
                    if (!watchlist.containsItem(temp))
                        watchlist.addItem(entry.getId(), temp, DomainTypes.MARKET_PRICE, 0, entry.getGapFillChannelId(DomainTypes.MARKET_PRICE), entry.getRealTimeChannelId(DomainTypes.MARKET_PRICE));
                }
            }
        if (CommandLine.hasArg("mbo") && CommandLine.values("mbo").size() > 0)
            for (int i = 0; i < CommandLine.values("mbo").size(); ++i)
            {
                if ((entry = symbolListHandler.symbolList().get(CommandLine.values("mbo").get(i))) != null)
                {
                    temp.data(CommandLine.values("mbo").get(i));
                    if (!watchlist.containsItem(temp))
                        watchlist.addItem(entry.getId(), temp, DomainTypes.MARKET_BY_ORDER, 0, entry.getGapFillChannelId(DomainTypes.MARKET_BY_ORDER), entry.getRealTimeChannelId(DomainTypes.MARKET_BY_ORDER));
                }
            }
        if (CommandLine.hasArg("mbp") && CommandLine.values("mbp").size() > 0)
            for (int i = 0; i < CommandLine.values("mbp").size(); ++i)
            {
                if ((entry = symbolListHandler.symbolList().get(CommandLine.values("mbp").get(i))) != null)
                {
                    temp.data(CommandLine.values("mbp").get(i));
                    if (!watchlist.containsItem(temp))
                        watchlist.addItem(entry.getId(), temp, DomainTypes.MARKET_BY_PRICE, 0, entry.getGapFillChannelId(DomainTypes.MARKET_BY_PRICE), entry.getRealTimeChannelId(DomainTypes.MARKET_BY_PRICE));
                }
            }
        }
        
        if (((RefreshMsg) responseMsg).checkRefreshComplete())
            state = SnapshotSessionState.SNAPSHOT_STATE_FINISHED;
        // Finished
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
            // Request items
            
            System.out.println("Requesting items...");
            
            sendSymbolListRequests(chnl);
            
            watchlist.sendRequests(chnl, error);
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
     * Closes all streams for the snapshot session.
     */
    public void uninitialize()
    {
        System.out.println("SnapshotSession unitializing...");
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
    
    public SymbolListHandler symbolListHandler()
    {
        return symbolListHandler;
    }
}
