package com.thomsonreuters.upa.examples.genericprov;

import java.io.IOException;
import java.nio.channels.SelectionKey;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.Executors;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.examples.common.ClientSessionInfo;
import com.thomsonreuters.upa.examples.common.CommandLine;
import com.thomsonreuters.upa.examples.common.ProviderDirectoryHandler;
import com.thomsonreuters.upa.examples.common.ProviderLoginHandler;
import com.thomsonreuters.upa.examples.common.ProviderSession;
import com.thomsonreuters.upa.examples.common.ReceivedMsgCallback;
import com.thomsonreuters.upa.examples.common.UnSupportedMsgHandler;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.BindOptions;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.Server;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

/**
 * This is the main class for the UPA Java Generic Provider application. It is
 * a multi-threaded application. The application uses either the operating
 * parameters entered by the user or a default set of parameters.
 * <p>
 * The purpose of this application is to respond to generic messages sent by a
 * generic consumer. The messages from the consumer are requests to retrieve
 * files. First the application initializes the UPA transport and binds
 * the server. After that, it processes login, directory and file requests from
 * a generic consumer and sends the appropriate responses. A new thread is created to
 * respond to each file request. The responses are sent as generic messages with an
 * opaque data payload.
 * <p>
 * This class is also a call back for all events from provider. It dispatches
 * events to domain specific handlers.
 * <p>
 * This application is intended as a basic usage example. Some of the design
 * choices were made to favor simplicity and readability over performance. It is
 * not intended to be used for measuring performance.
 * <p>
 * <H2>Setup Environment</H2>
 * <p>
 * No special setup is required.
 * <p>
 * <H2>Running the application:</H2>
 * <p>
 * Change directory to the <i>Applications/Examples</i> directory and run <i>ant</i> to
 * build.
 * <p>
 * java -cp ./bin;../../Libs/upaValueAdd.jar;../../Libs/upa.jar
 * com.thomsonreuters.upa.examples.genericprov.GenericProv [-p srvrPortNo]
 * [-i interfaceName] [-s serviceName] [-id serviceId] [-x] [-runtime runTime]
 * <p>
 * <ul>
 * <li>-p Server port number. Default is <i>14002</i>.
 * <li>-i Interface name. Default is <i>null</i>.
 * <li>-s Service name. Default is <i>DIRECT_FEED</i>.
 * <li>-id Service id. Default is <i>1</i>.
 * <li>-x Provides XML tracing of messages.
 * <li>-runtime run time. Default is 1200 seconds. Controls the time the
 * application will run before exiting, in seconds.
 * </ul>
 */
public class GenericProv implements ReceivedMsgCallback
{
    private ProviderSession _providerSession;
    private UnSupportedMsgHandler _unSupportedMsgHandler;
    private ProviderLoginHandler _loginHandler;
    private ProviderDirectoryHandler _directoryHandler;
    private EncodeIterator _encIter;
    private RefreshMsg _refreshMsg;

    private DecodeIterator _dIter;
    private Msg _requestMsg;
    private Error _error;
    
    private long _runtime;

    private final int UPDATE_INTERVAL = 1;
    
    /* default server port number */
    private final String defaultSrvrPortNo = "14002";

    /* default service name */
    private final String defaultServiceName = "DIRECT_FEED";

    /* default run time in seconds */
    private final String defaultRuntime = "1200"; // seconds
    
    private final int REFRESH_LEN = 128;

    public GenericProv()
    {
        _providerSession = new ProviderSession();
        _unSupportedMsgHandler = new UnSupportedMsgHandler(_providerSession);
        _loginHandler = new ProviderLoginHandler(_providerSession);
        _directoryHandler = new ProviderDirectoryHandler(_providerSession);
        _dIter = CodecFactory.createDecodeIterator();
        _encIter = CodecFactory.createEncodeIterator();
        _requestMsg = CodecFactory.createMsg();
        _refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        _error = TransportFactory.createError();
    }
    
    /*
     * Parses command line arguments, initializes provider session which creates
     * listening socket. It also initializes the Login Handler.
     */
    void init(String[] args)
    {
        /* process command line args */
        addCommandLineArgs();
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

        System.out.println("portNo: " + CommandLine.value("p"));
        System.out.println("interfaceName: " + CommandLine.value("i"));

        // get bind options from the provider session
        BindOptions bindOptions = _providerSession.getBindOptions();
        
        // set the connection parameters on the bind options
        bindOptions.serviceName(CommandLine.value("p"));
        bindOptions.interfaceName(CommandLine.value("i"));
        
        // enable channel write locking
        _providerSession.enableChannelWriteLocking();
        
        int ret = _providerSession.init(false, _error);
        if (ret != TransportReturnCodes.SUCCESS)
        {
            System.out.println("Error initializing server: " + _error.text());
            System.exit(TransportReturnCodes.FAILURE);
        }

        // enable XML tracing
        if (CommandLine.booleanValue("x"))
        {
            _providerSession.enableXmlTrace(null);
        }

        _loginHandler.init();
        _directoryHandler.init();
        _directoryHandler.serviceName(CommandLine.value("s"));
        _directoryHandler.serviceId(CommandLine.intValue("id"));
        _directoryHandler.enableGenericProvider();
        _runtime = System.currentTimeMillis() + CommandLine.intValue("runtime") * 1000;
    }
    
    /*
     * Main loop that handles new client connections, reading and
     * processing requests from the channel.
     */
    void run()
    {
        int ret = 0;
        // main loop
        while (true)
        {
            Set<SelectionKey> keySet = null;
            try
            {
                if (_providerSession.selector.select(UPDATE_INTERVAL * 1000) > 0)
                {
                    keySet = _providerSession.selector.selectedKeys();
                }
            }
            catch (IOException e1)
            {
                System.out.println(e1.getMessage());
                cleanupAndExit();
            }

            if (keySet != null)
            {
                Iterator<SelectionKey> iter = keySet.iterator();
                while (iter.hasNext())
                {
                    SelectionKey key = iter.next();
                    iter.remove();
                    if(!key.isValid())
                        continue;
                    if (key.isAcceptable())
                    {
                        ret = _providerSession.handleNewClientSession((Server)key.attachment(), _error);
                        if (ret != TransportReturnCodes.SUCCESS)
                        {
                            System.out.println("accept error, text: " + _error.text());
                            cleanupAndExit();
                        }
                    }
                    else if (key.isReadable())
                    {
                        ret = _providerSession.read((Channel)key.attachment(), _error, this);
                        if (ret != TransportReturnCodes.SUCCESS)
                        {
                            System.out.println("read error, text: " + _error.text());
                            cleanupAndExit();
                        }
                    }
                    else if (key.isWritable() && ((Channel)key.attachment()).state() == ChannelState.ACTIVE)
                    {
                        _providerSession.flush(key, _error);
                    }
                }
            }

            /* Handle pings */
            _providerSession.handlePings();

            /* Handle run-time */
            handleRuntime();
        }
    }

    /*
     * Handles the run-time for the Generic Provider. Sends close status messages to
     * all streams on all channels after run-time has elapsed.
     */
    private void handleRuntime()
    {
        int ret = 0;
        // get current time
        long currentTime = System.currentTimeMillis();

        if (currentTime >= _runtime)
        {
            // send close status messages to all streams on all channels
            for (ClientSessionInfo clientSessionInfo : _providerSession.clientSessions)
            {
                if ((clientSessionInfo.clientChannel() != null))
                {
                    // send close status message to login stream 
                    ret = _loginHandler.sendCloseStatus(clientSessionInfo.clientChannel(), _error);
                    if (ret != 0)
                        System.out.println("Error sending login close: " + _error.text());

                    // send close status message to source directory stream
                    ret = _directoryHandler.sendCloseStatus(clientSessionInfo.clientChannel(), _error);
                    if (ret != 0)
                        System.out.println("Error sending directory close: " + _error.text());

                    // flush before exiting 
                    SelectionKey key = clientSessionInfo.clientChannel().selectableChannel().keyFor(_providerSession.selector);
                    if (key.isWritable())
                    {
                        ret = 1;
                        while (ret > TransportReturnCodes.SUCCESS)
                        {
                            ret = clientSessionInfo.clientChannel().flush(_error);
                        }
                        if (ret < TransportReturnCodes.SUCCESS)
                        {
                            System.out.println("clientChannel.flush() failed with return code " + ret + _error.text());
                        }
                    }
                }
            }
            System.out.println("Generic Provider run-time expired...");
            uninit();
            System.exit(0);
        }

    }

    /**
     * Call back for socket read for client requests.
     */
    @Override
    public void processReceivedMsg(Channel channel, TransportBuffer msgBuf)
    {
        /* clear decode iterator */
        _dIter.clear();

        /* set buffer and version info */
        int ret = _dIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("DecodeIterator.setBufferAndRWFVersion() failed with return code: " + CodecReturnCodes.toString(ret));
            cleanupAndExit();
        }
        
        ret = _requestMsg.decode(_dIter);
        if (ret != CodecReturnCodes.SUCCESS)
        {
            System.out.println("RequestMsg.decode() failed with return code: " + CodecReturnCodes.toString(ret) + " on SessionData " + channel.selectableChannel() + "  Size " + (msgBuf.data().limit() - msgBuf.data().position()));
            cleanupAndExit();
        }

        switch (_requestMsg.domainType())
        {
            case DomainTypes.LOGIN:
                if (_loginHandler.processRequest(channel, _requestMsg, _dIter, _error) != 0)
                {
                    System.out.println("Error processing login request: " + _error.text());
                    cleanupAndExit();
                }
                break;
            case DomainTypes.SOURCE:
                if (_directoryHandler.processRequest(channel, _requestMsg, _dIter, _error) != 0)
                {
                    System.out.println("Error processing directory request: " + _error.text());
                    cleanupAndExit();
                }
                break;
            case ProviderDirectoryHandler.GENERIC_DOMAIN:
                if (_requestMsg.msgClass() == MsgClasses.GENERIC) 
                {
                    // process file request from generic consumer
                    processGenericConsumerRequest(channel, (GenericMsg)_requestMsg);
                }
                else if (_requestMsg.msgClass() == MsgClasses.REQUEST)
                {
                    // process request to open stream from generic consumer
                    processRequest(channel, (RequestMsg)_requestMsg);
                }
                break;
            default:
                if (_unSupportedMsgHandler.sendStatus(channel, _requestMsg, _error) != 0)
                {
                    System.out.println("Error sending message: " + _error.text());
                    cleanupAndExit();
                }
                break;
        }
    }

    /* Processes a request to open stream from a generic consumer. */
    private void processRequest(Channel channel, RequestMsg requestMsg)
    {
        int ret;
        TransportBuffer buffer = channel.getBuffer(REFRESH_LEN, false, _error);
        
        _encIter.clear();
        _encIter.setBufferAndRWFVersion(buffer, channel.majorVersion(), channel.minorVersion());
        _refreshMsg.clear();
        _refreshMsg.msgClass(MsgClasses.REFRESH);
        _refreshMsg.domainType(requestMsg.domainType());
        _refreshMsg.streamId(requestMsg.streamId());
        _refreshMsg.applyRefreshComplete();
        _refreshMsg.applySolicited();
        _refreshMsg.applyHasMsgKey();
        _refreshMsg.msgKey().applyHasName();
        _refreshMsg.msgKey().name().data(requestMsg.msgKey().name().toString());
        _refreshMsg.msgKey().applyHasServiceId();
        _refreshMsg.msgKey().serviceId(requestMsg.msgKey().serviceId());
        _refreshMsg.state().dataState(DataStates.OK);
        _refreshMsg.state().streamState(StreamStates.OPEN);
        _refreshMsg.state().code(StateCodes.NONE);
        _refreshMsg.state().text().data("Generic stream is open...");

        if ((ret = _refreshMsg.encode(_encIter)) != CodecReturnCodes.SUCCESS)
        {
            System.out.println("RefreshMsg encoding failure: " + ret);
            System.exit(-1);
        }
        
        // send message
        if (( ret = _providerSession.write(channel, buffer, _error)) != TransportReturnCodes.SUCCESS)
        {
            System.out.println("RefreshMsg write failure: " + ret);
            System.exit(-1);
        }
    }

    /* Processes a request from a generic consumer. Uses a thread for each file requested. */
    private void processGenericConsumerRequest(Channel channel, GenericMsg genericMsg)
    {
        // create thread to send file response
        SendFileResponseThread sendFileResponseThread = new SendFileResponseThread(channel, genericMsg.msgKey().name().toString(), genericMsg.streamId(), genericMsg.domainType(), _providerSession);
        Executors.newSingleThreadExecutor().execute(sendFileResponseThread);
    }

    /* Processes a channel close from the ProviderSession. Closes any open streams.*/
    @Override
    public void processChannelClose(Channel channel)
    {
        _loginHandler.closeRequest(channel);
        _directoryHandler.closeRequest(channel);
    }

    /* Configures command line arguments for the generic provider application. */
    private void addCommandLineArgs()
    {
        CommandLine.programName("Generic Provider");
        CommandLine.addOption("p", defaultSrvrPortNo, "Server port number");
        CommandLine.addOption("i", (String)null, "Interface name");
        CommandLine.addOption("s", defaultServiceName, "Service name");
        CommandLine.addOption("id", "1", "Service id");
        CommandLine.addOption("runtime", defaultRuntime, "Program runtime in seconds");
        CommandLine.addOption("x", "Provides XML tracing of messages.");
    }

    /* Cleans up and exits the application. */
    private void cleanupAndExit()
    {
        _providerSession.uninit();
        System.exit(TransportReturnCodes.FAILURE);
    }

    /* Uninitializes the generic provider application. */
    void uninit()
    {
        _providerSession.uninit();
    }

    public static void main(String[] args)
    {
        GenericProv genericprov = new GenericProv();
        genericprov.init(args);
        genericprov.run();
        genericprov.uninit();
    }
}
