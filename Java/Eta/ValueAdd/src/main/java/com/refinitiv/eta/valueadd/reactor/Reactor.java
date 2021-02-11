package com.refinitiv.eta.valueadd.reactor;

import java.nio.channels.CancelledKeyException;
import java.nio.channels.SelectionKey;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.json.converter.*;
import com.refinitiv.eta.json.util.JsonFactory;
import com.refinitiv.eta.rdm.ClassesOfService;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.InitArgs;
import com.refinitiv.eta.transport.ReadArgs;
import com.refinitiv.eta.transport.ReadArgsImpl;
import com.refinitiv.eta.transport.Server;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WriteArgs;
import com.refinitiv.eta.transport.WriteArgsImpl;
import com.refinitiv.eta.valueadd.common.SelectableBiDirectionalQueue;
import com.refinitiv.eta.valueadd.common.VaDoubleLinkList;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryConsumerStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequestFlags;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueMsg;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel.State;
import com.refinitiv.eta.valueadd.reactor.ReactorTokenSession.SessionState;

/**
 * The Reactor. Applications create Reactor objects by calling {@link ReactorFactory#createReactor(ReactorOptions, ReactorErrorInfo)},
 * create connections by calling {@link Reactor#connect(ReactorConnectOptions, ReactorRole, ReactorErrorInfo)}/
 * {@link Reactor#accept(com.refinitiv.eta.transport.Server, ReactorAcceptOptions, ReactorRole, ReactorErrorInfo)} and
 * process events by calling {@link ReactorChannel#dispatch(ReactorDispatchOptions, ReactorErrorInfo)}/
 * {@link Reactor#dispatchAll(Set, ReactorDispatchOptions, ReactorErrorInfo)}.
 *
 * @see ReactorChannel
 * @see ReactorFactory#createReactor
 * @see #shutdown
 * @see #connect
 * @see #accept
 * @see ReactorChannel#dispatch
 * @see #dispatchAll
 * @see ReactorChannel#submit
 * @see ReactorChannel#close
 */
public class Reactor
{
    boolean _reactorActive = false;
    final static int SHUTDOWN_TIMEOUT_IN_SECONDS = 5;

    ReactorOptions _reactorOptions = ReactorFactory.createReactorOptions();
    ReactorChannel _reactorChannel = null;
    ReactorChannelInfo _reactorChannelInfo = ReactorFactory.createReactorChannelInfo();

    // queue between reactor and worker
    SelectableBiDirectionalQueue _workerQueue = null;
    Worker _worker = null;
    ExecutorService _esWorker = null;

    // Queue to track ReactorChannels
    VaDoubleLinkList<ReactorChannel>  _reactorChannelQueue = new VaDoubleLinkList<ReactorChannel>();

    Lock _reactorLock = new ReentrantLock();
    int _reactorChannelCount; // used by reactor.dispatchAll

    EncodeIterator _eIter = CodecFactory.createEncodeIterator();
    DecodeIterator _dIter = CodecFactory.createDecodeIterator();
    Msg _msg = CodecFactory.createMsg();
    WriteArgs _writeArgs = TransportFactory.createWriteArgs();
    WriteArgs _writeArgsAggregator = TransportFactory.createWriteArgs();
    ReactorSubmitOptions reactorSubmitOptions = ReactorFactory.createReactorSubmitOptions();
    ReadArgs _readArgsAggregator = TransportFactory.createReadArgs();
    InitArgs _initArgs = TransportFactory.createInitArgs();
    LoginMsg _loginMsg = LoginMsgFactory.createMsg();
    CloseMsg _closeMsg = (CloseMsg)CodecFactory.createMsg();
    DirectoryMsg _directoryMsg = DirectoryMsgFactory.createMsg();
    DictionaryMsg _dictionaryMsg = DictionaryMsgFactory.createMsg();

    private XmlTraceDump xmlDumpTrace = CodecFactory.createXmlTraceDump();
    private StringBuilder xmlString = new StringBuilder(1500);
    private HashMap<Msg, TransportBuffer> _submitMsgMap = new HashMap<Msg, TransportBuffer>();
    private HashMap<MsgBase, TransportBuffer> _submitRdmMsgMap = new HashMap<MsgBase, TransportBuffer>();
    private HashMap<String, ReactorTokenSession> _tokenManagementMap = new HashMap<String, ReactorTokenSession>(5);
    private Lock _tokenManagementLock = new ReentrantLock();

    // REST client support
    RestClient _restClient;
    ReactorTokenSession _tokenSessionForCredentialRenewalCallback;

    // This is used by the queryServiceDiscovery() method.
    private List<ReactorServiceEndpointInfo> _reactorServiceEndpointInfoList = new ArrayList<ReactorServiceEndpointInfo>(50);
    RestReactorOptions _restReactorOptions;

    // tunnel stream support
    private TunnelStreamStateInfo _tunnelStreamStateInfo;
    private TunnelStreamAuthInfo _authInfo = ReactorFactory.createTunnelStreamAuthInfo();
    private com.refinitiv.eta.codec.State _tmpState = CodecFactory.createState();
    private TunnelStreamRequestEvent _tunnelStreamRequestEvent = new TunnelStreamRequestEvent();
    private TunnelStreamSubmitOptions _tunnelStreamSubmitOptions = ReactorFactory.createTunnelStreamSubmitOptions();
    private TunnelStreamRejectOptions _tunnelStreamRejectOptions = ReactorFactory.createTunnelStreamRejectOptions();

    private boolean _finalStatusEvent;

    WlInteger _tempWlInteger = ReactorFactory.createWlInteger();

    // JSON conversion support
    JsonConverter jsonConverter = null;
    Object jsonConverterUserSpec = null;
    private ServiceNameIdConverterClient serviceNameIdConverterClient = null;
    ReactorServiceNameToIdCallback serviceNameToIdCallback;
    ReactorJsonConversionEventCallback JsonConversionEventCallback;
    private ReactorJsonConversionEvent jsonConversionEvent = new ReactorJsonConversionEvent();
    private boolean closeChannelFromFailure = false;
    private ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
    JsonConverterError converterError = ConverterFactory.createJsonConverterError();
    private DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
    private JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
    private Msg jsonDecodeMsg = CodecFactory.createMsg();
    private GetJsonErrorParams jsonErrorParams = ConverterFactory.createJsonErrorParams();
    private Buffer jsonErrorOutputBuffer = CodecFactory.createBuffer();

    RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
    ConversionResults conversionResults = ConverterFactory.createConversionResults();
    GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();

    /* This is used by ReactorChannel for handling packed buffer of the JSON protocol. */
    HashMap<TransportBuffer, ReactorPackedBuffer> packedBufferHashMap = new HashMap<>();

    /* This is used by ReactorChannel for handling the write call again state of the JSON protocol. */
    HashMap<TransportBuffer, TransportBuffer> writeCallAgainMap = new HashMap<>();

    static String JSON_PONG_MESSAGE = "{\"Type\":\"Pong\"}";

    /**
     * The specified ReactorOptions are copied so that it can be re-used by the
     * client application. The ErrorInfo will be populated if an error occurs.
     *
     * @param options
     * @param errorInfo
     */
    Reactor(ReactorOptions options, ReactorErrorInfo errorInfo)
    {
        if (errorInfo == null)
        {
            throw new UnsupportedOperationException("ReactorErrorInfo cannot be null");
        }
        else if (options != null)
        {
            if(options.tokenReissueRatio() < 0.05 || options.tokenReissueRatio() > 0.95)
            {
                populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.constructor",
                        "The token reissue ratio must be in between 0.05 to 0.95.");
                return;
            }

            if(options.reissueTokenAttemptInterval() < 0)
            {
                populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.constructor",
                        "The token reissue attempt interval is less than zero.");
                return;
            }

            _reactorOptions.copy(options);
        }
        else
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.constructor",
                    "options was null and cannot continue.");
            return;
        }

        if (initializeTransport(errorInfo) != ReactorReturnCodes.SUCCESS)
        {
            return;
        }

        if (initializeReactor(errorInfo) != ReactorReturnCodes.SUCCESS)
        {
            return;
        }

        _tunnelStreamStateInfo = new TunnelStreamStateInfo();

        errorInfo.clear();
        _reactorActive = true;
        _finalStatusEvent = true;
    }

    /* Invokes Transport.initialize() with global locking true. */
    int initializeTransport(ReactorErrorInfo errorInfo)
    {
        _initArgs.clear();
        _initArgs.globalLocking(true);
        if (Transport.initialize(_initArgs, errorInfo.error()) != TransportReturnCodes.SUCCESS)
        {
            return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                    "Reactor.initializeTransport", errorInfo.error().text());
        }
        return ReactorReturnCodes.SUCCESS;
    }

    int initializeReactor(ReactorErrorInfo errorInfo)
    {
        try
        {
            // create SelectableBiDirectionalQueue
            _workerQueue = new SelectableBiDirectionalQueue();

            // create a new ReactorChannel and populate with the readChannel
            // side of our _workerQueue.
            _reactorChannel = ReactorFactory.createReactorChannel();
            _reactorChannel.reactor(this);
            _reactorChannel.userSpecObj(this);


            _reactorChannel.selectableChannel(_workerQueue.readChannel());
            // Set ping handler aggregation
            _reactorChannel.pingHandler().trackPings(_reactorOptions.pingStatSet());
            // create the worker thread.
            _worker = new Worker(_reactorChannel, _workerQueue.remote());
            _esWorker = Executors.newSingleThreadExecutor();
            _esWorker.execute(_worker);
        }
        catch (RejectedExecutionException | NullPointerException e)
        {
            return populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.initializeReactor",
                    "failed to initialize the Worker, exception="
                    + e.getLocalizedMessage());
        }

        return ReactorReturnCodes.SUCCESS;
    }

    /* Clears then populates the specified errorInfo object. */
    int populateErrorInfo(ReactorErrorInfo errorInfo, int reactorReturnCode, String location, String text)
    {
        errorInfo.clear();
        errorInfo.code(reactorReturnCode).location(location);
        errorInfo.error().errorId(reactorReturnCode);
        if (text != null)
            errorInfo.error().text(text);
        return reactorReturnCode;
    }

    /**
     * Returns the userSpecObj that was specified in the ReactorOptions when
     * this Reactor was created.
     *
     * @return the userSpecObj
     */
    public Object userSpecObj()
    {
        return _reactorOptions.userSpecObj();
    }

    /**
     * Returns whether or not the Reactor is shutdown.
     *
     * @return true if the Reactor is shutdown, or false if it isn't
     */
    public boolean isShutdown()
    {
        return !_reactorActive;
    }

    /**
     * Shuts down and cleans up a Reactor. Stops the ETA Reactor if necessary and
     * sends ReactorChannelEvents to all active channels indicating that they are down.
     * Once this call is made, the Reactor is destroyed and no further calls should
     * be made with it.
     *
     * @param errorInfo Error structure to be populated in the event of an error
     *
     * @return {@link ReactorReturnCodes} indicating success or failure
     */
    public int shutdown(ReactorErrorInfo errorInfo)
    {
        int retval = ReactorReturnCodes.SUCCESS;

        _reactorLock.lock();

        try
        {
            if (!_reactorActive)
                return retval;

            if (_restClient != null)
                _restClient.shutdown();

            /*
             * For all reactorChannels, send CHANNEL_DOWN to worker and
             * application (via callback).
             */
            for(ReactorChannel reactorChannel = _reactorChannelQueue.start(ReactorChannel.REACTOR_CHANNEL_LINK);
                reactorChannel != null;
                reactorChannel = _reactorChannelQueue.forth(ReactorChannel.REACTOR_CHANNEL_LINK))
            {
                if (reactorChannel == null || reactorChannel.state() == State.CLOSED)
                    continue;

                // ignore user app ReactorCallbackReturnCodes during
                // shutdown.
                if (errorInfo.error().text() == null)
                {
                    errorInfo.error().text("Reactor shutting down...");
                }
                sendChannelEventCallback(ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);

                if (reactorChannel.state() != State.CLOSED)
                {
                    closeChannel(reactorChannel, errorInfo);
                }

            }
            _reactorChannelQueue = null;
            _reactorChannelCount = 0;

            // Terminate Worker by sending shutdown, then wait for the Worker to
            // terminate.
            sendWorkerEvent(WorkerEventTypes.SHUTDOWN, null);
            _esWorker.shutdown();
            while (!_esWorker.awaitTermination(SHUTDOWN_TIMEOUT_IN_SECONDS, TimeUnit.SECONDS));
            _esWorker = null;
            _worker = null;
            
            if(packedBufferHashMap.size() > 0)
            {
            	for(ReactorPackedBuffer packedBuffer : packedBufferHashMap.values())
            	{
            		packedBuffer.returnToPool();
            	}
            	
            	packedBufferHashMap.clear();
            }
            
            writeCallAgainMap.clear();

            // shutdown the workerQueue.
            _workerQueue.shutdown();
            _workerQueue = null;
            _reactorChannel.returnToPool();
            _reactorChannel = null;

            int tRetCode = Transport.uninitialize();
            if (tRetCode != TransportReturnCodes.SUCCESS)
                retval = ReactorReturnCodes.FAILURE;

        }
        catch (InterruptedException e)
        {
            return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.shutdown",
                    "Exception occurred while waiting for Worker thread to terminate, exception="
                    + e.getLocalizedMessage());
        }
        finally
        {
            _reactorActive = false;
            _reactorLock.unlock();
        }

        return retval;
    }

    /**
     * The Reactor's internal channel that's used to communicate with the worker thread.
     *
     * @return the Reactor's internal channel
     */
    public ReactorChannel reactorChannel()
    {
        return _reactorChannel;
    }

    /**
     * Adds a server-side channel to the Reactor. Once the channel is initialized,
     * the channelEventCallback will receive an event indicating that the channel
     * is up.
     *
     * @param server server that is accepting this connection (a server can be created with {@link com.refinitiv.eta.transport.Transport#bind(com.refinitiv.eta.transport.BindOptions, com.refinitiv.eta.transport.Error)})
     * @param reactorAcceptOptions options for this connection
     * @param role role of this connection
     * @param errorInfo error structure to be populated in the event of failure
     *
     * @return {@link ReactorReturnCodes} indicating success or failure
     */
    public int accept(Server server, ReactorAcceptOptions reactorAcceptOptions, ReactorRole role, ReactorErrorInfo errorInfo)
    {
        _reactorLock.lock();

        try
        {
            if (errorInfo == null)
            {
                System.out.println("Reactor.accept(): ReactorErrorInfo cannot be null, aborting.");
                return ReactorReturnCodes.FAILURE;
            }

            if (!_reactorActive)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.accept", "Reactor is not active, aborting.");
            }
            else if (reactorAcceptOptions == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.accept",
                        "reactorAcceptOptions cannot be null, aborting.");
            }
            else if (role == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.accept", "role cannot be null, aborting.");
            }
            else if (role.channelEventCallback() == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.accept",
                        "role must have a channelEventCallback defined, aborting.");
            }
            else if (role.defaultMsgCallback() == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.accept",
                        "role must have a defaultMsgCallback defined, aborting.");
            }
            else if (role.type() != ReactorRoleTypes.PROVIDER)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.accept",
                        "role must be Provider Role, aborting.");
            }

            if (reactorAcceptOptions.initTimeout() < 1)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.accept",
                        "ReactorAcceptOptions.timeout must be greater than zero, aborting.");
            }

            // create a ReactorChannel and add it to the initChannelQueue.
            ReactorChannel reactorChannel = ReactorFactory.createReactorChannel();
            reactorChannel.state(State.INITIALIZING);
            reactorChannel.role(role);
            reactorChannel.reactor(this);
            reactorChannel.initializationTimeout(reactorAcceptOptions.initTimeout());
            reactorChannel.server(server);
            _reactorChannelQueue.pushBack(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);

            // enable channel read/write locking for reactor since it's multi-threaded with worker thread
            reactorAcceptOptions.acceptOptions().channelReadLocking(true);
            reactorAcceptOptions.acceptOptions().channelWriteLocking(true);

            // call Server.accept to accept a new Channel
            Channel channel = server.accept(reactorAcceptOptions.acceptOptions(),
                    errorInfo.error());
            if (channel != null)
            {
                reactorChannel.selectableChannelFromChannel(channel);
                reactorChannel.userSpecObj(reactorAcceptOptions.acceptOptions().userSpecObject());
            }
            else
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.accept", "Server.accept() failed, error="
                                          + errorInfo.error().text());
            }

            // send a WorkerEvent to the Worker to initialize this channel.
            if (!sendWorkerEvent(WorkerEventTypes.CHANNEL_INIT, reactorChannel))
            {
                // sendWorkerEvent() failed, send channel down
                reactorChannel.state(State.DOWN);
                sendAndHandleChannelEventCallback("Reactor.accept",
                        ReactorChannelEventTypes.CHANNEL_DOWN,
                        reactorChannel, errorInfo);
                return populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        "Reactor.accept",
                        "sendWorkerEvent() failed");
            }

            /* Set additional accept options for WebSocket connections */
            reactorChannel.sendPingMessage(reactorAcceptOptions.websocketAcceptOptions().sendPingMessage);
        }
        finally
        {
            _reactorLock.unlock();
        }

        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * Adds a client-side channel to the Reactor. Once the channel is initialized,
     * the channelEventCallback will receive an event indicating that the channel
     * is up.
     *
     * @param reactorConnectOptions options for this connection
     * @param role role of this connection
     * @param errorInfo error structure to be populated in the event of failure
     *
     * @return {@link ReactorReturnCodes} indicating success or failure
     */
    public int connect(ReactorConnectOptions reactorConnectOptions, ReactorRole role, ReactorErrorInfo errorInfo)
    {
        _reactorLock.lock();

        boolean sendAuthTokenEvent = false;
        ReactorTokenSession tokenSession = null;

        try
        {
            if (errorInfo == null)
            {
                System.out.println("Reactor.connect(): ReactorErrorInfo cannot be null, aborting.");
                return ReactorReturnCodes.FAILURE;
            }

            if (!_reactorActive)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.connect", "Reactor is not active, aborting.");
            }
            else if (reactorConnectOptions == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.connect",
                        "reactorConnectOptions cannot be null, aborting.");
            }
            else if (role == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.connect", "role cannot be null, aborting.");
            }
            else if (role.channelEventCallback() == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.connect",
                        "role must have a channelEventCallback defined, aborting.");
            }
            else if (role.defaultMsgCallback() == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.connect",
                        "role must have a defaultMsgCallback defined, aborting.");
            }
            else if (role.type() == ReactorRoleTypes.CONSUMER)
            {
                if (((ConsumerRole)role).rdmDirectoryRequest() != null
                    && ((ConsumerRole)role).rdmLoginRequest() == null)
                {
                    return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.connect",
                            "Must specify an rdmLoginRequest if specifying an rdmDirectoryRequest, aborting.");
                }

                if (((ConsumerRole)role).dictionaryDownloadMode() == DictionaryDownloadModes.FIRST_AVAILABLE
                    && ((ConsumerRole)role).watchlistOptions().enableWatchlist())
                {
                    return populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE,
                            "Reactor.connect",
                            "Cannot specify a dictionary download when watchlist is enabled.");
                }

                if (((ConsumerRole)role).dictionaryDownloadMode() == DictionaryDownloadModes.FIRST_AVAILABLE
                    && ((ConsumerRole)role).rdmDirectoryRequest() == null)
                {
                    return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.connect",
                            "Must specify an rdmDirectoryRequest if specifying a dictionary download, aborting.");
                }
            }
            else if (role.type() == ReactorRoleTypes.NIPROVIDER)
            {
                if (((NIProviderRole)role).rdmDirectoryRefresh() != null
                    && ((NIProviderRole)role).rdmLoginRequest() == null)
                {
                    return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.connect",
                            "Must specify an rdmLoginRequest if specifying an rdmDirectoryRequest, aborting.");
                }
            }
            else if (role.type() == ReactorRoleTypes.PROVIDER)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.connect",
                        "role must be Consumer or NIProvider Role, aborting.");
            }

            if (reactorConnectOptions.connectionList().size() == 0)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.connect",
                        "ReactorConnectOptions.connectionList() must have at least one ReactorConnectInfo, aborting.");
            }
            else if (reactorConnectOptions.connectionList().get(0).initTimeout() < 1)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.connect",
                        "ReactorConnectOptions.timeout must be greater than zero, aborting.");
            }
            else if (reactorConnectOptions.connectionList().get(0).connectOptions().blocking() == true)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.connect",
                        "ReactorConnectOptions.connectOptions.blocking must be false, aborting.");
            }

            // create a ReactorChannel
            ReactorChannel reactorChannel = ReactorFactory.createReactorChannel();
            reactorChannel.reactor(this);
            reactorChannel.userSpecObj(reactorConnectOptions.connectionList().get(0).connectOptions().userSpecObject());
            reactorChannel.initializationTimeout(reactorConnectOptions.connectionList().get(0).initTimeout());
            reactorChannel.reactorConnectOptions(reactorConnectOptions);

            if(enableSessionManagement(reactorConnectOptions))
            {
                ReactorOAuthCredential oAuthCredential;

                // setup a rest client if any connections are requiring the session management
                try
                {
                    setupRestClient(errorInfo);
                }
                catch(Exception e)
                {
                    reactorChannel.returnToPool();
                    return populateErrorInfo(errorInfo,
                            ReactorReturnCodes.FAILURE,
                            "Reactor.setupRestClient",
                            "failed to initialize the RESTClient, exception="
                            + e.getLocalizedMessage());
                }

                if ( (oAuthCredential = retriveOAuthCredentialFromConsumerRole(role, errorInfo)) != null)
                {
                    tokenSession = getTokenSession(oAuthCredential, errorInfo);

                    if(tokenSession == null)
                    {
                        reactorChannel.returnToPool();
                        return errorInfo.code();
                    }

                    reactorChannel.tokenSession(tokenSession);
                }
                else
                {
                    reactorChannel.returnToPool();
                    return errorInfo.code();
                }
            }

            if (reactorConnectOptions.connectionList().get(0).enableSessionManagement())
            {
                if (sessionManagementStartup(tokenSession, reactorConnectOptions.connectionList().get(0), role,
                        reactorChannel, true, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    removeReactorChannel(reactorChannel);
                    reactorChannel.returnToPool();
                    return errorInfo.code();
                }

                reactorChannel.applyAccessToken();

                /* Set original expires in when sending request using password grant type*/
                tokenSession.originalExpiresIn(tokenSession.authTokenInfo().expiresIn());

                sendAuthTokenEvent = true;

                /* Clears OAuth sensitive information if the callback is specified */
                if(tokenSession.oAuthCredential().reactorOAuthCredentialEventCallback() != null)
                {
                    tokenSession.oAuthCredential().password().clear();
                    tokenSession.oAuthCredential().clientSecret().clear();
                }
            }

            reactorChannel.state(State.INITIALIZING);
            reactorChannel.role(role);

            // Add it to the initChannelQueue.
            _reactorChannelQueue.pushBack(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);

            // enable channel read/write locking for reactor since it's multi-threaded with worker thread
            ConnectOptions connectOptions = reactorChannel.getReactorConnectInfo().connectOptions();
            connectOptions.channelReadLocking(true);
            connectOptions.channelWriteLocking(true);

            // create watchlist if enabled
            if (role.type() == ReactorRoleTypes.CONSUMER &&
                ((ConsumerRole)role).watchlistOptions().enableWatchlist())
            {
                Watchlist watchlist = ReactorFactory.createWatchlist(reactorChannel, (ConsumerRole)role);
                reactorChannel.watchlist(watchlist);

                // call channelOpenCallback if callback defined
                if (((ConsumerRole)role).watchlistOptions().channelOpenCallback() != null)
                {
                    sendAndHandleChannelEventCallback("Reactor.connect",
                            ReactorChannelEventTypes.CHANNEL_OPENED,
                            reactorChannel, errorInfo);
                }
            }
            // call Transport.connect to create a new Channel
            Channel channel = Transport.connect(connectOptions,
                    errorInfo.error());
            reactorChannel.selectableChannelFromChannel(channel);

            if (sendAuthTokenEvent)
            {
                sendAuthTokenEventCallback(reactorChannel, tokenSession.authTokenInfo(), errorInfo);
                reactorChannel.sessionMgntState(ReactorChannel.SessionMgntState.RECEIVED_AUTH_TOKEN);
                if (!tokenSession.isInitialized() && !sendAuthTokenWorkerEvent(tokenSession))
                {
                    removeReactorChannel(reactorChannel);
                    reactorChannel.returnToPool();
                    _reactorChannelQueue.remove(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);
                    return populateErrorInfo(errorInfo,
                            ReactorReturnCodes.FAILURE,
                            "Reactor.connect",
                            "sendAuthTokenWorkerEvent() failed");
                }
            }

            if (channel == null)
            {
                if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
                {
                    reactorChannel.state(State.DOWN_RECONNECTING);

                    // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
                    sendAndHandleChannelEventCallback("Reactor.connect",
                            ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                            reactorChannel, errorInfo);
                }
                else // server channel or no more retries
                {
                    reactorChannel.state(State.DOWN);

                    // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
                    sendAndHandleChannelEventCallback("Reactor.connect",
                            ReactorChannelEventTypes.CHANNEL_DOWN,
                            reactorChannel, errorInfo);
                }
            }
            // send a WorkerEvent to the Worker to initialize this channel.
            else if (!sendWorkerEvent(WorkerEventTypes.CHANNEL_INIT, reactorChannel))
            {
                // sendWorkerEvent() failed, send channel down
                reactorChannel.state(State.DOWN);
                sendAndHandleChannelEventCallback("Reactor.connect",
                        ReactorChannelEventTypes.CHANNEL_DOWN,
                        reactorChannel, errorInfo);

                removeReactorChannel(reactorChannel);
                reactorChannel.returnToPool();
                _reactorChannelQueue.remove(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);
                return populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        "Reactor.connect",
                        "sendWorkerEvent() failed");
            }
        }
        finally
        {
            _reactorLock.unlock();
        }

        return ReactorReturnCodes.SUCCESS;
    }

    final static boolean compareOAuthCredential(Reactor reactor, ReactorOAuthCredential current, ReactorOAuthCredential other, ReactorErrorInfo errorInfo)
    {
        if(current.reactorOAuthCredentialEventCallback() != other.reactorOAuthCredentialEventCallback())
        {
            reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.compareOAuthCredential",
                    "The ReactorOAuthCredentialEventCallback of ReactorOAuthCredential is not equal for the existing token session.");
            return false;
        }

        if(current.reactorOAuthCredentialEventCallback() == null)
        {
            if(current.clientSecret().equals(other.clientSecret()) == false)
            {
                reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.compareOAuthCredential",
                        "The Client secret of ReactorOAuthCredential is not equal for the existing token session.");
                return false;
            }

            if(current.password().equals(other.password()) == false)
            {
                reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.compareOAuthCredential",
                        "The password of ReactorOAuthCredential is not equal for the existing token session.");
                return false;
            }
        }

        if(current.clientId().equals(other.clientId()) == false)
        {
            reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.compareOAuthCredential",
                    "The Client ID of ReactorOAuthCredential is not equal for the existing token session.");
            return false;
        }

        if(current.tokenScope().equals(other.tokenScope()) == false)
        {
            reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.compareOAuthCredential",
                    "The token scope of ReactorOAuthCredential is not equal for the existing token session.");
            return false;
        }

        if(current.takeExclusiveSignOnControl() != other.takeExclusiveSignOnControl())
        {
            reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.compareOAuthCredential",
                    "The takeExclusiveSignOnControl of ReactorOAuthCredential is not equal for the existing token session.");
            return false;
        }

        return true;
    }

    ReactorTokenSession getTokenSession(ReactorOAuthCredential oAuthCredential, ReactorErrorInfo errorInfo)
    {
        ReactorTokenSession tokenSession;

        _tokenManagementLock.lock();

        try
        {
            String userName = oAuthCredential.userName().toString();
            tokenSession = _tokenManagementMap.get(oAuthCredential.userName().toString());

            if(tokenSession == null)
            {
                tokenSession = new ReactorTokenSession(this, oAuthCredential );
                _tokenManagementMap.put(userName, tokenSession);
            }
            else
            {
                /* Checks whether there is sufficient time to reissue the access token for this channel. */
                if ( tokenSession.checkMiniumTimeForReissue(errorInfo) == false )
                {
                    return null;
                }

                /* Validates the credential with the existing token session for the same user name */
                if ( compareOAuthCredential(this, tokenSession.oAuthCredential(), oAuthCredential, errorInfo) == false)
                {
                    return null;
                }
            }
        }
        finally
        {
            _tokenManagementLock.unlock();
        }

        return tokenSession;
    }

    void removeTokenSession(ReactorTokenSession tokenSession)
    {
        if(tokenSession == null)
            return;

        _tokenManagementLock.lock();

        try
        {
            _tokenManagementMap.remove(tokenSession.oAuthCredential().userName().toString());
        }
        finally
        {
            _tokenManagementLock.unlock();
        }
    }

    void removeAllTokenSession()
    {
        _tokenManagementLock.lock();

        try
        {
            Iterator<ReactorTokenSession> iter = _tokenManagementMap.values().iterator();

            ReactorTokenSession tokenSession;

            while(iter.hasNext())
            {
                tokenSession = iter.next();

                tokenSession.removeAllReactorChannel();
            }

            _tokenManagementMap.clear();
        }
        finally
        {
            _tokenManagementLock.unlock();
        }
    }

    int numberOfTokenSession()
    {
        _tokenManagementLock.lock();

        try
        {
            return _tokenManagementMap.size();
        }
        finally
        {
            _tokenManagementLock.unlock();
        }
    }

    void removeReactorChannel(ReactorChannel reactorChannel)
    {
        ReactorTokenSession tokenSession = reactorChannel.tokenSession();

        if( tokenSession != null)
        {
            if ( tokenSession.removeReactorChannel(reactorChannel) == 0 )
            {
                removeTokenSession(tokenSession);
            }
        }
    }


    /* Checks whether the session management is enabled */
    final static boolean enableSessionManagement(ReactorConnectOptions reactorConnectOptions)
    {
        for (int i = 0; i < reactorConnectOptions.connectionList().size(); i++)
        {
            if (reactorConnectOptions.connectionList().get(i).enableSessionManagement())
            {
                return true;
            }
        }

        return false;
    }

    @SuppressWarnings("deprecation")
    ReactorOAuthCredential retriveOAuthCredentialFromConsumerRole(ReactorRole role, ReactorErrorInfo errorInfo)
    {
        LoginRequest loginRequest = null;
        ReactorOAuthCredential oauthCredential = null;
        ReactorOAuthCredential oauthCredentialOut = null;

        if (role.type() == ReactorRoleTypes.CONSUMER)
        {
            oauthCredential = ((ConsumerRole)role).reactorOAuthCredential();
            loginRequest = ((ConsumerRole)role).rdmLoginRequest();
        }
        else if(role.type() == ReactorRoleTypes.NIPROVIDER)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.copyOAuthCredentialForSessionManagement",
                    "The session management supports only on the ReactorRoleTypes.CONSUMER type.");
            return null;
        }

        if (loginRequest == null && oauthCredential == null)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.copyOAuthCredentialForSessionManagement",
                    "There is no user credential available for enabling session management.");
            return null;
        }

        Buffer userName = (oauthCredential != null && oauthCredential.userName().length() != 0) ? oauthCredential.userName() :
                (loginRequest != null && loginRequest.userName().length() != 0) ? loginRequest.userName() :  null;
        Buffer password = (oauthCredential != null && oauthCredential.password().length() != 0) ? oauthCredential.password() :
                (loginRequest != null && loginRequest.password().length() != 0) ? loginRequest.password() :  null;

        Buffer clientId = (oauthCredential != null && oauthCredential.clientId().length() != 0) ? oauthCredential.clientId() : ((ConsumerRole)role).clientId();

        if(userName == null || userName.length() == 0)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.copyOAuthCredentialForSessionManagement",
                    "Failed to copy OAuth credential for enabling the session management; OAuth user name does not exist.");
            return null;
        }

        if(clientId == null || clientId.length() == 0)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.copyOAuthCredentialForSessionManagement",
                    "Failed to copy OAuth credential for enabling the session management; OAuth client ID does not exist.");
            return null;
        }

        if(password == null || password.length() == 0)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.copyOAuthCredentialForSessionManagement",
                    "Failed to copy OAuth credential for enabling the session management; OAuth password does not exist.");
            return null;
        }

        oauthCredentialOut = ReactorFactory.createReactorOAuthCredential();

        oauthCredentialOut.userName().data(userName.toString());
        oauthCredentialOut.password().data(password.toString());
        oauthCredentialOut.clientId().data(clientId.toString());

        if(oauthCredential != null)
        {
            if(oauthCredential.clientSecret().length() != 0)
            {
                oauthCredentialOut.clientSecret().data(oauthCredential.clientSecret().toString());
            }

            if(oauthCredential.tokenScope().length() != 0)
            {
                oauthCredentialOut.tokenScope().data(oauthCredential.tokenScope().toString());
            }

            oauthCredentialOut.takeExclusiveSignOnControl(oauthCredential.takeExclusiveSignOnControl());
            oauthCredentialOut.reactorOAuthCredentialEventCallback(oauthCredential.reactorOAuthCredentialEventCallback());
            oauthCredentialOut.userSpecObj(oauthCredential.userSpecObj());
        }

        return oauthCredentialOut;
    }

    int sessionManagementStartup(ReactorTokenSession tokenSession, ReactorConnectInfo reactorConnectInfo, ReactorRole role, ReactorChannel reactorChannel, boolean isBlocking, ReactorErrorInfo errorInfo)
    {
        // save login information
        LoginRequest loginRequest = null;

        if (role.type() == ReactorRoleTypes.CONSUMER)
        {
            loginRequest = ((ConsumerRole)role).rdmLoginRequest();
        }

        if ( (loginRequest != null) && (reactorChannel._loginRequestForEDP == null) )
        {
            reactorChannel._loginRequestForEDP = (LoginRequest)LoginMsgFactory.createMsg();
            reactorChannel._loginRequestForEDP.rdmMsgType(LoginMsgType.REQUEST);

            loginRequest.copy(reactorChannel._loginRequestForEDP);
            reactorChannel._loginRequestForEDP.userNameType(Login.UserIdTypes.AUTHN_TOKEN);
            // Do not send the password
            reactorChannel._loginRequestForEDP.flags(reactorChannel._loginRequestForEDP.flags() & ~LoginRequestFlags.HAS_PASSWORD);
        }

        switch (reactorConnectInfo.connectOptions().connectionType())
        {
            case ConnectionTypes.ENCRYPTED:

                if(reactorConnectInfo.connectOptions().encryptionOptions().connectionType() == ConnectionTypes.WEBSOCKET)
                {
                    /* Query endpoints for the websocket connection type. */
                    reactorChannel.restConnectOptions().transport(ReactorDiscoveryTransportProtocol.RD_TP_WEBSOCKET);
                    reactorChannel.restConnectOptions().dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2);
                }
                else
                {
                    reactorChannel.restConnectOptions().transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
                    reactorChannel.restConnectOptions().dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);
                }

                break;
            default:
                populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.connect",
                        "Reactor.connect(): Invalid connection type: " +
                        ConnectionTypes.toString(reactorConnectInfo.connectOptions().connectionType()) +
                        " for requesting EDP-RT service discovery.");
                return ReactorReturnCodes.PARAMETER_INVALID;
        }

        tokenSession.lock();

        reactorChannel.state(State.EDP_RT);
        tokenSession.setProxyInfo(reactorConnectInfo);

        try
        {
            if ( tokenSession.sessionMgntState() == SessionState.REQUEST_TOKEN_FAILURE || tokenSession.sessionMgntState() == SessionState.STOP_TOKEN_REQUEST)
            {
                return ReactorReturnCodes.SUCCESS;
            }

            if( !tokenSession.hasAccessToken() )
            {
                if ( _restClient.getAuthAccessTokenInfo(tokenSession.authOptoins(), tokenSession.restConnectOptions(),
                        tokenSession.authTokenInfo(), isBlocking, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    return errorInfo.code();
                }

                /* The service discovery will be requested in the callback of the token request if needed.*/
                if(!isBlocking)
                {
                    return ReactorReturnCodes.SUCCESS;
                }
            }
        }
        finally
        {
            tokenSession.unlock();
        }

        if (requestServiceDiscovery(reactorConnectInfo))
        {
            if ( _restClient.getServiceDiscovery(reactorChannel.restConnectOptions(), tokenSession.authTokenInfo(), isBlocking,
                    reactorChannel.reactorServiceEndpointInfoList(), errorInfo) != ReactorReturnCodes.SUCCESS)
            {
                return errorInfo.code();
            }

            if ( isBlocking )
            {
                if(reactorChannel.applyServiceDiscoveryEndpoint(errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    return errorInfo.code();
                }
                else
                {
                    reactorChannel.state(State.EDP_RT_DONE);
                }
            }
            else
            {
                /* Waits for a service discovery response for non-blocking request. */
                reactorChannel.state(State.EDP_RT);
            }
        }
        else
        {
            reactorChannel.state(State.EDP_RT_DONE);
        }

        return ReactorReturnCodes.SUCCESS;
    }

    static boolean requestServiceDiscovery(ReactorConnectInfo reactorConnectInfo)
    {
        // only use the EDP-RT connection information if not specified by the user
        if((reactorConnectInfo.connectOptions().unifiedNetworkInfo().address() == null &&
            reactorConnectInfo.connectOptions().unifiedNetworkInfo().serviceName() == null) ||
           (	reactorConnectInfo.connectOptions().unifiedNetworkInfo().address() != null &&
                reactorConnectInfo.connectOptions().unifiedNetworkInfo().address().equals("") &&
                reactorConnectInfo.connectOptions().unifiedNetworkInfo().serviceName() != null &&
                reactorConnectInfo.connectOptions().unifiedNetworkInfo().serviceName().equals("")))
        {
            return true;
        }
        return false;
    }

    ReactorOptions reactorOptions()
    {
        return _reactorOptions;
    }

    private static final int sendQueryServiceDiscoveryEvent(ReactorServiceDiscoveryOptions options, List<ReactorServiceEndpointInfo> reactorServiceEndpointInfoList,
                                                            ReactorErrorInfo errorInfo)
    {
        ReactorServiceEndpointEvent reactorServiceEndpointEvent = ReactorFactory.createReactorServiceEndpointEvent();

        ReactorErrorInfo errorInfoTemp = reactorServiceEndpointEvent._errorInfo;

        if(reactorServiceEndpointInfoList != null)
        {
            reactorServiceEndpointEvent._reactorServiceEndpointInfoList = reactorServiceEndpointInfoList;
        }
        else
        {
            reactorServiceEndpointEvent._errorInfo = errorInfo;
        }

        reactorServiceEndpointEvent._userSpecObject = options.userSpecObject();

        options.reactorServiceEndpointEventCallback().reactorServiceEndpointEventCallback(reactorServiceEndpointEvent);

        reactorServiceEndpointEvent._errorInfo = errorInfoTemp;

        reactorServiceEndpointEvent.returnToPool();

        return errorInfo.code();
    }

    /**
     * Queries EDP-RT service discovery to get service endpoint information.
     *
     * @param options The {@link ReactorServiceDiscoveryOptions} to configure options and specify the
     * 		ReactorServiceEndpointEventCallback to receive service endpoint information.
     * @param errorInfo error structure to be populated in the event of failure
     *
     * @return {@link ReactorReturnCodes} indicating success or failure
     */
    public int queryServiceDiscovery(ReactorServiceDiscoveryOptions options, ReactorErrorInfo errorInfo)
    {
        _reactorLock.lock();

        try {

            ReactorAuthTokenInfo authTokenInfo;
            RestConnectOptions connOptions;

            if (errorInfo == null)
            {
                return ReactorReturnCodes.PARAMETER_INVALID;
            }

            if (!_reactorActive)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.queryServiceDiscovery", "Reactor is not active, aborting.");
            }

            if (options == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.queryServiceDiscovery",
                        "Reactor.queryServiceDiscovery(): options cannot be null, aborting.");
            }

            if (options.reactorServiceEndpointEventCallback() == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.queryServiceDiscovery",
                        "Reactor.queryServiceDiscovery(): ReactorServiceEndpointEventCallback cannot be null, aborting.");
            }

            if (options.userName() == null || options.userName().length() == 0)
            {
                return populateErrorInfo(errorInfo,
                        ReactorReturnCodes.PARAMETER_INVALID,
                        "Reactor.queryServiceDiscovery",
                        "Required parameter username is not set");
            }

            if (options.password() == null || options.password().length() == 0)
            {
                return populateErrorInfo(errorInfo,
                        ReactorReturnCodes.PARAMETER_INVALID,
                        "Reactor.queryServiceDiscovery",
                        "Required parameter password is not set");
            }

            if (options.clientId() == null || options.clientId().length() == 0)
            {
                return populateErrorInfo(errorInfo,
                        ReactorReturnCodes.PARAMETER_INVALID,
                        "Reactor.queryServiceDiscovery",
                        "Required parameter clientId is not set");
            }

            switch (options.transport())
            {
                case ReactorDiscoveryTransportProtocol.RD_TP_INIT:
                case ReactorDiscoveryTransportProtocol.RD_TP_TCP:
                case ReactorDiscoveryTransportProtocol.RD_TP_WEBSOCKET:
                    break;
                default:

                    populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_OUT_OF_RANGE, "Reactor.queryServiceDiscovery",
                            "Reactor.queryServiceDiscovery(): Invalid transport protocol type " + options.transport());
                    return ReactorReturnCodes.PARAMETER_OUT_OF_RANGE;
            }

            switch (options.dataFormat())
            {
                case ReactorDiscoveryDataFormatProtocol.RD_DP_INIT:
                case ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2:
                case ReactorDiscoveryDataFormatProtocol.RD_DP_RWF:
                    break;
                default:

                    populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_OUT_OF_RANGE, "Reactor.queryServiceDiscovery",
                            "Reactor.queryServiceDiscovery(): Invalid dataformat protocol type " + options.dataFormat());
                    return ReactorReturnCodes.PARAMETER_OUT_OF_RANGE;
            }

            try
            {
                setupRestClient(errorInfo);
            }
            catch(Exception e)
            {
                return populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        "Reactor.setupRestClient",
                        "failed to initialize the RESTClient, exception="
                        + e.getLocalizedMessage());
            }

            _tokenManagementLock.lock();
            ReactorTokenSession tokenSession = null;

            try
            {
                tokenSession = _tokenManagementMap.get(options.userName().toString());
            }
            finally
            {
                _tokenManagementLock.unlock();
            }

            if (tokenSession == null)
            {
                RestAuthOptions authOptions = new RestAuthOptions(options.takeExclusiveSignOnControl());
                authTokenInfo = new ReactorAuthTokenInfo();
                connOptions = new RestConnectOptions(reactorOptions());

                authOptions.username(options.userName().toString());
                authOptions.password(options.password().toString());
                authOptions.clientId(options.clientId().toString());
                authOptions.clientSecret(options.clientSecret().toString());
                authOptions.tokenScope(options.tokenScope().toString());

                connOptions.applyServiceDiscoveryOptions(options);

                if (_restClient.getAuthAccessTokenInfo(authOptions, connOptions, authTokenInfo, true, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    Reactor.sendQueryServiceDiscoveryEvent(options, null, errorInfo);
                    return ReactorReturnCodes.SUCCESS;
                }
            }
            else
            {
                authTokenInfo = tokenSession.authTokenInfo();
                connOptions = tokenSession.restConnectOptions();
            }

            _reactorServiceEndpointInfoList.clear();

            if ( _restClient.getServiceDiscovery(connOptions, authTokenInfo, true, _reactorServiceEndpointInfoList, errorInfo) != ReactorReturnCodes.SUCCESS)
            {
                Reactor.sendQueryServiceDiscoveryEvent(options, null, errorInfo);
            }
            else
            {
                Reactor.sendQueryServiceDiscoveryEvent(options, _reactorServiceEndpointInfoList, errorInfo);
            }
        }
        finally
        {
            _reactorLock.unlock();
        }

        return ReactorReturnCodes.SUCCESS;
    }

    /**
     * Submit OAuth credential renewal with password or password change.
     *
     * @param renewalOptions The {@link ReactorOAuthCredentialRenewalOptions} to configure OAuth credential renewal options.
     * @param oAuthCredentialRenewal The {@link ReactorOAuthCredentialRenewal} to configure credential renewal information.
     * @param errorInfo error structure to be populated in the event of failure
     * @return {@link ReactorReturnCodes} indicating success or failure
     */
    public int submitOAuthCredentialRenewal(ReactorOAuthCredentialRenewalOptions renewalOptions, ReactorOAuthCredentialRenewal oAuthCredentialRenewal, ReactorErrorInfo errorInfo)
    {
        _reactorLock.lock();

        try {

            ReactorTokenSession tokenSession = null;
            ReactorOAuthCredentialRenewal oAuthCredentialRenewalCopy;

            if (errorInfo == null)
            {
                return ReactorReturnCodes.PARAMETER_INVALID;
            }

            if (!_reactorActive)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.submitOAuthCredentialRenewal", "Reactor is not active, aborting.");
            }

            if (renewalOptions == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.submitOAuthCredentialRenewal",
                        "renewalOptions cannot be null, aborting.");
            }

            if (oAuthCredentialRenewal == null)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.submitOAuthCredentialRenewal",
                        "oAuthCredentialRenewal cannot be null, aborting.");
            }


            /* Checks whether the token session is available from calling with in the callback method */
            tokenSession = _tokenSessionForCredentialRenewalCallback;

            if( tokenSession == null )
            {
                if ( oAuthCredentialRenewal.userName() == null ||oAuthCredentialRenewal.userName().isBlank() )
                {
                    return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.submitOAuthCredentialRenewal",
                            "ReactorOAuthCredentialRenewal.userName() not provided, aborting.");
                }



                if ( renewalOptions.reactorAuthTokenEventCallback() == null )
                {
                    return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.submitOAuthCredentialRenewal",
                            "ReactorOAuthCredentialRenewalOptions.reactorAuthTokenEventCallback() not provided, aborting.");
                }
            }

            if (oAuthCredentialRenewal.password() == null || oAuthCredentialRenewal.password().isBlank())
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.submitOAuthCredentialRenewal",
                        "ReactorOAuthCredentialRenewal.password() not provided, aborting.");
            }

            try
            {
                setupRestClient(errorInfo);
            }
            catch(Exception e)
            {
                return populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        "Reactor.setupRestClient",
                        "failed to initialize the RESTClient, exception="
                        + e.getLocalizedMessage());
            }

            switch(renewalOptions.renewalModes())
            {
                case ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD:
                    break;
                case ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD_CHANGE:
                {
                    if(tokenSession != null)
                    {
                        if(tokenSession.oAuthCredential().reactorOAuthCredentialEventCallback() == null)
                        {
                            return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.submitOAuthCredentialRenewal",
                                    "Support changing password of the token session when ReactorOAuthCredential.reactorOAuthCredentialEventCallback() is specified only., aborting.");
                        }
                    }

                    if (oAuthCredentialRenewal.newPassword() == null || oAuthCredentialRenewal.newPassword().isBlank())
                    {
                        return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.submitOAuthCredentialRenewal",
                                "ReactorOAuthCredentialRenewal.newPassword() not provided, aborting.");
                    }

                    break;
                }
                default:
                    return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.submitOAuthCredentialRenewal",
                            "Invalid ReactorOAuthCredentialRenewalOptions.RenewalModes(" + renewalOptions.renewalModes()  +"), aborting.");
            }

            oAuthCredentialRenewalCopy = copyReactorOAuthCredentialRenewal(tokenSession, renewalOptions, oAuthCredentialRenewal);

            if(tokenSession != null)
            {
                tokenSession.sendAuthRequestWithSensitiveInfo(oAuthCredentialRenewalCopy.password().toString(), oAuthCredentialRenewalCopy.newPassword().toString(),
                        oAuthCredentialRenewalCopy.clientSecret().toString());
            }
            else
            {
                RestAuthOptions restAuthOptions = new RestAuthOptions(oAuthCredentialRenewal.takeExclusiveSignOnControl());
                RestConnectOptions restConnectOptions = new RestConnectOptions(_reactorOptions);
                ReactorAuthTokenInfo authTokenInfo = new ReactorAuthTokenInfo();

                restAuthOptions.username(oAuthCredentialRenewalCopy.userName().toString());
                restAuthOptions.clientId(oAuthCredentialRenewalCopy.clientId().toString());
                restAuthOptions.password(oAuthCredentialRenewalCopy.password().toString());
                restAuthOptions.newPassword(oAuthCredentialRenewalCopy.newPassword().toString());
                restAuthOptions.clientSecret(oAuthCredentialRenewalCopy.clientSecret().toString());


                if ( _restClient.getAuthAccessTokenInfo(restAuthOptions, restConnectOptions, authTokenInfo, true, errorInfo) != ReactorReturnCodes.SUCCESS)
                {
                    return errorInfo.code();
                }
                else
                {
                    ReactorErrorInfo errorInfoTemp;
                    ReactorAuthTokenEventCallback callback = renewalOptions.reactorAuthTokenEventCallback();

                    ReactorAuthTokenEvent reactorAuthTokenEvent = ReactorFactory.createReactorAuthTokenEvent();
                    reactorAuthTokenEvent.reactorChannel(null);
                    reactorAuthTokenEvent.reactorAuthTokenInfo(authTokenInfo);
                    errorInfoTemp = reactorAuthTokenEvent._errorInfo ;
                    reactorAuthTokenEvent._errorInfo = errorInfo;

                    callback.reactorAuthTokenEventCallback(reactorAuthTokenEvent);
                    reactorAuthTokenEvent._errorInfo = errorInfoTemp;

                    reactorAuthTokenEvent.returnToPool();
                }
            }

            oAuthCredentialRenewalCopy.clear();

        }
        finally
        {
            _reactorLock.unlock();
        }

        return ReactorReturnCodes.SUCCESS;
    }

    private static ReactorOAuthCredentialRenewal copyReactorOAuthCredentialRenewal(ReactorTokenSession tokenSession, ReactorOAuthCredentialRenewalOptions renewalOptions,
                                                                                   ReactorOAuthCredentialRenewal oAuthCredentialRenewal)
    {
        ReactorOAuthCredentialRenewal oAuthCredentialRenewalOut = new ReactorOAuthCredentialRenewal();

        /* This is the case to submit credential renewal without the token session*/
        if(tokenSession == null)
        {
            oAuthCredentialRenewalOut.userName().data(oAuthCredentialRenewal.userName().toString());
            oAuthCredentialRenewalOut.clientId().data(oAuthCredentialRenewal.clientId().toString());
            oAuthCredentialRenewalOut.tokenScope().data(oAuthCredentialRenewal.tokenScope().toString());
        }

        oAuthCredentialRenewalOut.password().data(oAuthCredentialRenewal.password().toString());

        if(oAuthCredentialRenewal.clientSecret().length() != 0)
        {
            oAuthCredentialRenewalOut.clientSecret().data(oAuthCredentialRenewal.clientSecret().toString());
        }

        if(renewalOptions.renewalModes() == ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD_CHANGE)
        {
            oAuthCredentialRenewalOut.newPassword().data(oAuthCredentialRenewal.newPassword().toString());
        }

        return oAuthCredentialRenewalOut;
    }

    private void setupRestClient(ReactorErrorInfo errorInfo)
    {
        if(_restClient != null)
            return;

        _restReactorOptions = new RestReactorOptions();
        _restReactorOptions.connectTimeout(0);
        _restReactorOptions.soTimeout(_reactorOptions.restRequestTimeout());

        _restClient = new RestClient(_restReactorOptions, errorInfo);
    }

    void loginReissue(ReactorChannel reactorChannel, String authToken, ReactorErrorInfo errorInfo)
    {
        if (reactorChannel.state() == State.CLOSED || reactorChannel.state() == State.DOWN || reactorChannel.state() == State.EDP_RT)
            return;

        LoginRequest loginRequest = null;

        if (reactorChannel.enableSessionManagement())
        {
            if(reactorChannel._loginRequestForEDP != null)
            {
                loginRequest = reactorChannel._loginRequestForEDP;
                loginRequest.userName().data(authToken);
            }
        }
        else
        {
            switch (reactorChannel.role().type())
            {
                case ReactorRoleTypes.CONSUMER:
                    loginRequest = ((ConsumerRole)reactorChannel.role()).rdmLoginRequest();
                    break;
                case ReactorRoleTypes.NIPROVIDER:
                    loginRequest = ((NIProviderRole)reactorChannel.role()).rdmLoginRequest();
                    break;
                default:
                    break;
            }
        }

        if (loginRequest != null)
        {
            _reactorLock.lock();

            try
            {
                reactorChannel.watchlist().loginHandler().sendLoginRequest(true, errorInfo);
            }
            finally
            {
                _reactorLock.unlock();
            }
        }
    }

    boolean sendAuthTokenWorkerEvent(ReactorTokenSession tokenSession)
    {
        boolean retVal = true;

        tokenSession.isInitialized(true);

        WorkerEvent event = ReactorFactory.createWorkerEvent();
        event.eventType(WorkerEventTypes.TOKEN_MGNT);
        event._restClient = _restClient;
        event._tokenSession = tokenSession;
        retVal = _workerQueue.write(event);

        return retVal;
    }

    boolean sendAuthTokenEvent(ReactorChannel reactorChannel, ReactorTokenSession tokenSession, ReactorErrorInfo reactorErrorInfo)
    {
        boolean retVal = true;

        WorkerEvent event = ReactorFactory.createWorkerEvent();
        event.eventType(WorkerEventTypes.TOKEN_MGNT);
        event._restClient = _restClient;
        event._tokenSession = tokenSession;
        event._reactorChannel = reactorChannel;
        populateErrorInfo(event.errorInfo(), reactorErrorInfo.code(), reactorErrorInfo.location(), reactorErrorInfo.error().text());
        retVal = _workerQueue.remote().write(event);

        return retVal;
    }

    boolean sendCredentialRenewalEvent(ReactorTokenSession tokenSession, ReactorErrorInfo reactorErrorInfo)
    {
        boolean retVal = true;

        WorkerEvent event = ReactorFactory.createWorkerEvent();
        event.eventType(WorkerEventTypes.TOKEN_CREDENTIAL_RENEWAL);
        event._restClient = _restClient;
        event._tokenSession = tokenSession;
        populateErrorInfo(event.errorInfo(), reactorErrorInfo.code(), reactorErrorInfo.location(), reactorErrorInfo.error().text());
        retVal = _workerQueue.remote().write(event);

        return retVal;
    }

    boolean sendWorkerEvent(WorkerEventTypes eventType, ReactorChannel reactorChannel)
    {
        boolean retVal = true;

        WorkerEvent event = ReactorFactory.createWorkerEvent();
        event.eventType(eventType);
        event.reactorChannel(reactorChannel);
        retVal = _workerQueue.write(event);

        return retVal;
    }

    boolean sendWorkerEvent(WorkerEventTypes eventType, ReactorChannel reactorChannel, long timeout)
    {
        boolean retVal = true;

        WorkerEvent event = ReactorFactory.createWorkerEvent();
        event.eventType(eventType);
        event.reactorChannel(reactorChannel);
        event.timeout(timeout);
        retVal = _workerQueue.write(event);

        return retVal;
    }

    boolean sendWorkerEvent(WorkerEventTypes eventType, ReactorChannel reactorChannel, TunnelStream tunnelStream, long timeout)
    {
        boolean retVal = true;

        WorkerEvent event = ReactorFactory.createWorkerEvent();
        event.eventType(eventType);
        event.reactorChannel(reactorChannel);
        event.tunnelStream(tunnelStream);
        event.timeout(timeout);
        retVal = _workerQueue.write(event);

        return retVal;
    }

    boolean sendDispatchNowEvent(ReactorChannel reactorChannel)
    {
        boolean retVal = true;

        WorkerEvent event = ReactorFactory.createWorkerEvent();
        event.eventType(WorkerEventTypes.TUNNEL_STREAM_DISPATCH_NOW);
        event.reactorChannel(reactorChannel);
        retVal = _workerQueue.remote().write(event);

        return retVal;
    }

    boolean sendWatchlistDispatchNowEvent(ReactorChannel reactorChannel)
    {
        boolean retVal = true;

        WorkerEvent event = ReactorFactory.createWorkerEvent();
        event.eventType(WorkerEventTypes.WATCHLIST_DISPATCH_NOW);
        event.reactorChannel(reactorChannel);
        retVal = _workerQueue.remote().write(event);

        return retVal;
    }

    /* This is used to send warning events for session management */
    boolean sendChannelWarningEvent(ReactorChannel reactorChannel, ReactorErrorInfo reactorErrorInfo)
    {
        boolean retVal = true;

        WorkerEvent event = ReactorFactory.createWorkerEvent();
        event._restClient = _restClient;
        event.eventType(WorkerEventTypes.WARNING);
        event.reactorChannel(reactorChannel);
        populateErrorInfo(event.errorInfo(), reactorErrorInfo.code(), reactorErrorInfo.location(), reactorErrorInfo.error().text());

        retVal = _workerQueue.remote().write(event);

        return retVal;
    }

    private int sendChannelEventCallback(int eventType, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        ReactorChannelEvent reactorChannelEvent = ReactorFactory.createReactorChannelEvent();
        reactorChannelEvent.reactorChannel(reactorChannel);
        reactorChannelEvent.eventType(eventType);
        populateErrorInfo(reactorChannelEvent.errorInfo(), errorInfo.code(), errorInfo.location(), errorInfo.error().text());

        int retval = reactorChannel.role().channelEventCallback().reactorChannelEventCallback(reactorChannelEvent);
        reactorChannelEvent.returnToPool();

        return retval;
    }

    int sendAuthTokenEventCallback(ReactorChannel reactorChannel, ReactorAuthTokenInfo authTokenInfo, ReactorErrorInfo errorInfo)
    {
        int retval;
        ReactorErrorInfo errorInfoTemp;
        ReactorAuthTokenEventCallback callback = reactorChannel.reactorAuthTokenEventCallback();

        if (callback != null && reactorChannel.enableSessionManagement())
        {
            ReactorAuthTokenEvent reactorAuthTokenEvent = ReactorFactory.createReactorAuthTokenEvent();
            reactorAuthTokenEvent.reactorChannel(reactorChannel);
            reactorAuthTokenEvent.reactorAuthTokenInfo(authTokenInfo);
            errorInfoTemp = reactorAuthTokenEvent._errorInfo ;
            reactorAuthTokenEvent._errorInfo = errorInfo;

            retval = callback.reactorAuthTokenEventCallback(reactorAuthTokenEvent);
            reactorAuthTokenEvent._errorInfo = errorInfoTemp;

            reactorAuthTokenEvent.returnToPool();

            if (retval != ReactorCallbackReturnCodes.SUCCESS)
            {
                // retval is not a valid ReactorReturnCodes.
                populateErrorInfo(errorInfo, retval, "Reactor.sendAuthTokenEventCallback", "retval of "
                                                                                           + retval + " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
                shutdown(errorInfo);
                return ReactorReturnCodes.FAILURE;
            }
        }
        else
        {
            // callback is undefined, raise it to defaultMsgCallback.
            retval = ReactorCallbackReturnCodes.RAISE;
        }

        return retval;
    }

    int sendOAuthCredentialEventCallback(ReactorTokenSession tokenSession, ReactorErrorInfo errorInfo)
    {
        int retval = ReactorReturnCodes.SUCCESS;
        ReactorErrorInfo errorInfoTemp;
        ReactorOAuthCredential oAuthCredential = tokenSession.oAuthCredential();
        ReactorOAuthCredentialEventCallback callback = oAuthCredential.reactorOAuthCredentialEventCallback();

        if (callback != null)
        {
            ReactorOAuthCredentialEvent reactorOAuthCredentialEvent = ReactorFactory.createReactorOAuthCredentialEvent();

            reactorOAuthCredentialEvent.reactorOAuthCredentialRenewal(tokenSession.oAuthCredentialRenewal());
            reactorOAuthCredentialEvent.reactorChannel(null);
            reactorOAuthCredentialEvent._reactor = this;
            reactorOAuthCredentialEvent._userSpecObj = tokenSession.oAuthCredential().userSpecObj();

            errorInfoTemp = reactorOAuthCredentialEvent._errorInfo ;
            reactorOAuthCredentialEvent._errorInfo = errorInfo;

            retval = callback.reactorOAuthCredentialEventCallback(reactorOAuthCredentialEvent);
            reactorOAuthCredentialEvent._errorInfo = errorInfoTemp;

            reactorOAuthCredentialEvent.returnToPool();

            if (retval != ReactorCallbackReturnCodes.SUCCESS)
            {
                // retval is not a valid ReactorReturnCodes.
                populateErrorInfo(errorInfo, retval, "Reactor.sendOAuthCredentialEventCallback", "retval of "
                                                                                                 + retval + " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
                shutdown(errorInfo);
                return ReactorReturnCodes.FAILURE;
            }
        }

        return retval;
    }

    // returns ReactorCallbackReturnCodes and populates errorInfo if needed.
    int sendAndHandleChannelEventCallback(String location, int eventType, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        int retval = sendChannelEventCallback(eventType, reactorChannel, errorInfo);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from reactorChannelEventCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval == ReactorCallbackReturnCodes.RAISE)
        {
            // RAISE is not a valid return code for the
            // reactorChannelEventCallback.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "ReactorCallbackReturnCodes.RAISE is not a valid return code from reactorChannelEventCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;

        }
        else if (retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }

        if (eventType == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
        {
            if (reactorChannel.role().type() == ReactorRoleTypes.CONSUMER) {
                ((ConsumerRole) (reactorChannel.role())).receivedFieldDictionaryResp(false);
                ((ConsumerRole) (reactorChannel.role())).receivedEnumDictionaryResp(false);
            }
        }

        if (eventType == ReactorChannelEventTypes.CHANNEL_DOWN || eventType == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
        {
            if (reactorChannel.watchlist() == null)
            {
                _tmpState.clear();
                _tmpState.streamState(StreamStates.CLOSED_RECOVER);
                _tmpState.dataState(DataStates.SUSPECT);
                _tmpState.text().data("Channel closed.");

                /* Remove all associated tunnel streams. */
                for (TunnelStream tunnelStream = reactorChannel.tunnelStreamManager()._tunnelStreamList.start(TunnelStream.MANAGER_LINK);
                     tunnelStream != null;
                     tunnelStream = reactorChannel.tunnelStreamManager()._tunnelStreamList.forth(TunnelStream.MANAGER_LINK))
                {
                    sendAndHandleTunnelStreamStatusEventCallback(location, reactorChannel, tunnelStream, null, null, _tmpState, errorInfo);
                }
            }
            else // /* If watchlist is on, it will send status messages to the tunnel streams (so don't do it ourselves). */
            {
                reactorChannel.watchlist().channelDown();
            }

            /* Channel callback complete. If channel is not already closed, notify worker. */
            if (reactorChannel.state() != State.CLOSED)
            {
                sendWorkerEvent(WorkerEventTypes.CHANNEL_DOWN, reactorChannel);
            }
        }

        return retval;
    }

    private int sendDefaultMsgCallback(ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, WatchlistStreamInfo streamInfo)
    {
        ReactorMsgEvent reactorMsgEvent = ReactorFactory.createReactorMsgEvent();
        reactorMsgEvent.reactorChannel(reactorChannel);
        reactorMsgEvent.transportBuffer(transportBuffer);
        reactorMsgEvent.msg(msg);
        if (streamInfo != null)
        {
            reactorMsgEvent.streamInfo().serviceName(streamInfo.serviceName());
            reactorMsgEvent.streamInfo().userSpecObject(streamInfo.userSpecObject());
        }
        else
        {
            reactorMsgEvent.streamInfo().clear();
        }

        int retval = reactorChannel.role().defaultMsgCallback().defaultMsgCallback(reactorMsgEvent);
        reactorMsgEvent.returnToPool();

        return retval;
    }

    // adds WatchlistStreamInfo, returns ReactorCallbackReturnCodes and populates errorInfo if needed.
    int sendAndHandleDefaultMsgCallback(String location, ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, WatchlistStreamInfo streamInfo, ReactorErrorInfo errorInfo)
    {
        TunnelStream tunnelStream;

        _tempWlInteger.value(msg.streamId());
        if ((tunnelStream = reactorChannel.streamIdtoTunnelStreamTable().get(_tempWlInteger)) != null)
            return handleTunnelStreamMsg(reactorChannel, tunnelStream, transportBuffer, msg, errorInfo);

        int retval = sendDefaultMsgCallback(reactorChannel, transportBuffer, msg, streamInfo);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from defaultMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval == ReactorCallbackReturnCodes.RAISE)
        {
            // RAISE is not a valid return code for the
            // reactorChannelEventCallback.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "ReactorCallbackReturnCodes.RAISE is not a valid return code from defaultMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        return retval;
    }

    // returns ReactorCallbackReturnCodes and populates errorInfo if needed.
    private int sendAndHandleDefaultMsgCallback(String location, ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, ReactorErrorInfo errorInfo)
    {
        TunnelStream tunnelStream;

        _tempWlInteger.value(msg.streamId());
        if ((tunnelStream = reactorChannel.streamIdtoTunnelStreamTable().get(_tempWlInteger)) != null)
            return handleTunnelStreamMsg(reactorChannel, tunnelStream, transportBuffer, msg, errorInfo);

        int retval = sendDefaultMsgCallback(reactorChannel, transportBuffer, msg, null);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from defaultMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval == ReactorCallbackReturnCodes.RAISE)
        {
            // RAISE is not a valid return code for the
            // reactorChannelEventCallback.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "ReactorCallbackReturnCodes.RAISE is not a valid return code from defaultMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        return retval;
    }

    int sendTunnelStreamStatusEventCallback(ReactorChannel reactorChannel, TunnelStream tunnelStream, TransportBuffer transportBuffer, Msg msg, com.refinitiv.eta.codec.State state, LoginMsg loginMsg, ReactorErrorInfo errorInfo)
    {
        if (tunnelStream.statusEventCallback() == null)
        {
            return ReactorReturnCodes.FAILURE;
        }

        TunnelStreamStatusEvent tunnelStreamEvent = ReactorFactory.createTunnelStreamStatusEvent();

        _tempWlInteger.value(tunnelStream.streamId());
        if (state != null && state.streamState() != StreamStates.OPEN &&
            reactorChannel.streamIdtoTunnelStreamTable().containsKey(_tempWlInteger))
        {
            /* Check for any untransmitted QueueData messages with immediate timeouts. */
            tunnelStream.expireImmediateMessages(errorInfo.error());

            // remove from table
            TunnelStream tempTunnelStream = reactorChannel.streamIdtoTunnelStreamTable().remove(_tempWlInteger);
            tempTunnelStream.tableKey().returnToPool();
            // close TunnelStreamHandler for this streamId
            if (state.streamState() != StreamStates.CLOSED && state.streamState() != StreamStates.CLOSED_RECOVER)
                tunnelStream.close(_finalStatusEvent, errorInfo.error());
            else
                reactorChannel.tunnelStreamManager().removeTunnelStream(tunnelStream);
        }

        tunnelStreamEvent.reactorChannel(reactorChannel);
        tunnelStreamEvent.transportBuffer(transportBuffer);
        tunnelStreamEvent.msg(msg);
        if (state != null)
        {
            state.copy(tunnelStreamEvent.state());
        }
        tunnelStreamEvent.tunnelStream(tunnelStream);
        if (loginMsg != null)
        {
            /* Authentication was performed; attach authInfo and login message. */
            _authInfo.clear();
            _authInfo.loginMsg(loginMsg);
            tunnelStreamEvent.authInfo(_authInfo);
        }

        int retval = tunnelStream.statusEventCallback().statusEventCallback(tunnelStreamEvent);
        _authInfo.clear();
        tunnelStreamEvent.returnToPool();

        return retval;
    }

    // returns ReactorCallbackReturnCodes and populates errorInfo if needed.
    int sendAndHandleTunnelStreamStatusEventCallback(String location, ReactorChannel reactorChannel,
                                                     TunnelStream tunnelStream, TransportBuffer transportBuffer, Msg msg,
                                                     com.refinitiv.eta.codec.State state, ReactorErrorInfo errorInfo)
    {
        int retval = sendTunnelStreamStatusEventCallback(reactorChannel, tunnelStream, transportBuffer, msg, state, null, errorInfo);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from tunnelStreamStatusEventCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval == ReactorCallbackReturnCodes.RAISE)
        {
            // RAISE is not a valid return code for the
            // tunnelStreamStatusEventCallback.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "ReactorCallbackReturnCodes.RAISE is not a valid return code from tunnelStreamStatusEventCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        return retval;
    }

    private int sendTunnelStreamMsgCallback(ReactorChannel reactorChannel, TunnelStream tunnelStream, TransportBuffer transportBuffer, Msg msg, int containerType)
    {
        if (tunnelStream.defaultMsgCallback() == null)
        {
            return ReactorReturnCodes.FAILURE;
        }

        TunnelStreamMsgEvent tunnelStreamMsgEvent = ReactorFactory.createTunnelStreamMsgEvent();

        tunnelStreamMsgEvent.reactorChannel(reactorChannel);
        tunnelStreamMsgEvent.transportBuffer(transportBuffer);
        tunnelStreamMsgEvent.msg(msg);
        tunnelStreamMsgEvent.tunnelStream(tunnelStream);
        tunnelStreamMsgEvent.containerType(containerType);

        int retval = tunnelStream.defaultMsgCallback().defaultMsgCallback(tunnelStreamMsgEvent);
        tunnelStreamMsgEvent.returnToPool();

        return retval;
    }

    // returns ReactorCallbackReturnCodes and populates errorInfo if needed.
    int sendAndHandleTunnelStreamMsgCallback(String location, ReactorChannel reactorChannel, TunnelStream tunnelStream, TransportBuffer transportBuffer, Msg msg, int containerType, ReactorErrorInfo errorInfo)
    {
        int retval = sendTunnelStreamMsgCallback(reactorChannel, tunnelStream, transportBuffer, msg, containerType);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from tunnelStreamDefaultMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval == ReactorCallbackReturnCodes.RAISE)
        {
            // RAISE is not a valid return code for the
            // tunnelStreamDefaultMsgCallback.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "ReactorCallbackReturnCodes.RAISE is not a valid return code from tunnelStreamDefaultMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        return retval;
    }

    private int sendQueueMsgCallback(ReactorChannel reactorChannel, TunnelStream tunnelStream, TransportBuffer transportBuffer, Msg msg, QueueMsg queueMsg)
    {
        int retval;

        if (tunnelStream.queueMsgCallback() != null)
        {
            TunnelStreamQueueMsgEvent queueMsgEvent = ReactorFactory.createQueueMsgEvent();

            queueMsgEvent.reactorChannel(reactorChannel);
            queueMsgEvent.transportBuffer(transportBuffer);
            queueMsgEvent.msg(msg);
            queueMsgEvent.tunnelStream(tunnelStream);
            queueMsgEvent.queueMsg(queueMsg);

            retval = tunnelStream.queueMsgCallback().queueMsgCallback(queueMsgEvent);
            queueMsgEvent.returnToPool();
        }
        else
        {
            // callback is undefined, raise it to tunnelStreamDefaultMsgCallback.
            retval = ReactorCallbackReturnCodes.RAISE;
        }

        return retval;
    }

    // returns ReactorCallbackReturnCodes and populates errorInfo if needed.
    int sendAndHandleQueueMsgCallback(String location, ReactorChannel reactorChannel, TunnelStream tunnelStream, TransportBuffer transportBuffer, Msg msg, QueueMsg queueMsg, ReactorErrorInfo errorInfo)
    {
        int retval = sendQueueMsgCallback(reactorChannel, tunnelStream, transportBuffer, msg, queueMsg);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from queueMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " was returned from tunnelStreamQueueMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        return retval;
    }

    private int sendLoginMsgCallback(ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, LoginMsg loginMsg, WatchlistStreamInfo streamInfo)
    {
        int retval;
        RDMLoginMsgCallback callback = null;

        switch (reactorChannel.role().type())
        {
            case ReactorRoleTypes.CONSUMER:
                callback = ((ConsumerRole)reactorChannel.role()).loginMsgCallback();
                break;
            case ReactorRoleTypes.PROVIDER:
                callback = ((ProviderRole)reactorChannel.role()).loginMsgCallback();
                break;
            case ReactorRoleTypes.NIPROVIDER:
                callback = ((NIProviderRole)reactorChannel.role()).loginMsgCallback();
                break;
            default:
                break;
        }

        if (callback != null)
        {
            RDMLoginMsgEvent rdmLoginMsgEvent = ReactorFactory.createRDMLoginMsgEvent();
            rdmLoginMsgEvent.reactorChannel(reactorChannel);
            rdmLoginMsgEvent.transportBuffer(transportBuffer);
            rdmLoginMsgEvent.msg(msg);
            rdmLoginMsgEvent.rdmLoginMsg(loginMsg);
            if (streamInfo != null)
            {
                rdmLoginMsgEvent.streamInfo().serviceName(streamInfo.serviceName());
                rdmLoginMsgEvent.streamInfo().userSpecObject(streamInfo.userSpecObject());
            }
            else
            {
                rdmLoginMsgEvent.streamInfo().clear();
            }

            retval = callback.rdmLoginMsgCallback(rdmLoginMsgEvent);
            rdmLoginMsgEvent.returnToPool();
        }
        else
        {
            // callback is undefined, raise it to defaultMsgCallback.
            retval = ReactorCallbackReturnCodes.RAISE;
        }

        return retval;
    }

    // adds WatchlistStreamInfo, returns ReactorCallbackReturnCodes and populates errorInfo if needed
    int sendAndHandleLoginMsgCallback(String location, ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, LoginMsg loginMsg, WatchlistStreamInfo streamInfo, ReactorErrorInfo errorInfo)
    {
        int retval = sendLoginMsgCallback(reactorChannel, transportBuffer, msg, loginMsg, streamInfo);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from rdmLoginMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " was returned from rdmLoginMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        return retval;
    }

    // returns ReactorCallbackReturnCodes and populates errorInfo if needed.
    private int sendAndHandleLoginMsgCallback(String location, ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, LoginMsg loginMsg, ReactorErrorInfo errorInfo)
    {
        int retval = sendLoginMsgCallback(reactorChannel, transportBuffer, msg, loginMsg, null);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from rdmLoginMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " was returned from rdmLoginMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        return retval;
    }

    private int sendDirectoryMsgCallback(ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, DirectoryMsg directoryMsg, WatchlistStreamInfo streamInfo)
    {
        int retval;
        RDMDirectoryMsgCallback callback = null;

        switch (reactorChannel.role().type())
        {
            case ReactorRoleTypes.CONSUMER:
                callback = ((ConsumerRole)reactorChannel.role()).directoryMsgCallback();
                break;
            case ReactorRoleTypes.PROVIDER:
                callback = ((ProviderRole)reactorChannel.role()).directoryMsgCallback();
                break;
            case ReactorRoleTypes.NIPROVIDER:
                // no directory callback for NIProvider.
                break;
            default:
                break;
        }

        if (callback != null)
        {
            RDMDirectoryMsgEvent rdmDirectoryMsgEvent = ReactorFactory.createRDMDirectoryMsgEvent();
            rdmDirectoryMsgEvent.reactorChannel(reactorChannel);
            rdmDirectoryMsgEvent.transportBuffer(transportBuffer);
            rdmDirectoryMsgEvent.msg(msg);
            rdmDirectoryMsgEvent.rdmDirectoryMsg(directoryMsg);
            if (streamInfo != null)
            {
                rdmDirectoryMsgEvent.streamInfo().serviceName(streamInfo.serviceName());
                rdmDirectoryMsgEvent.streamInfo().userSpecObject(streamInfo.userSpecObject());
            }
            else
            {
                rdmDirectoryMsgEvent.streamInfo().clear();
            }

            retval = callback.rdmDirectoryMsgCallback(rdmDirectoryMsgEvent);
            rdmDirectoryMsgEvent.returnToPool();
        }
        else
        {
            // callback is undefined, raise it to defaultMsgCallback.
            retval = ReactorCallbackReturnCodes.RAISE;
        }

        return retval;
    }

    // returns ReactorCallbackReturnCodes and populates errorInfo if needed.
    int sendAndHandleDirectoryMsgCallback(String location, ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, DirectoryMsg directoryMsg, WatchlistStreamInfo streamInfo, ReactorErrorInfo errorInfo)
    {
        int retval = sendDirectoryMsgCallback(reactorChannel, transportBuffer, msg, directoryMsg, streamInfo);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from rdmDirectoryMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " was returned from rdmDirectoryMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        return retval;
    }

    // returns ReactorCallbackReturnCodes and populates errorInfo if needed.
    private int sendAndHandleDirectoryMsgCallback(String location, ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, DirectoryMsg directoryMsg, ReactorErrorInfo errorInfo)
    {
        int retval = sendDirectoryMsgCallback(reactorChannel, transportBuffer, msg, directoryMsg, null);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from rdmDirectoryMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " was returned from rdmDirectoryMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        return retval;
    }

    private int sendDictionaryMsgCallback(ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, DictionaryMsg dictionaryMsg, WatchlistStreamInfo streamInfo)
    {
        int retval;
        RDMDictionaryMsgCallback callback = null;

        switch (reactorChannel.role().type())
        {
            case ReactorRoleTypes.CONSUMER:
                callback = ((ConsumerRole)reactorChannel.role()).dictionaryMsgCallback();
                break;
            case ReactorRoleTypes.PROVIDER:
                callback = ((ProviderRole)reactorChannel.role()).dictionaryMsgCallback();
                break;
            case ReactorRoleTypes.NIPROVIDER:
                // no dictionary callback for NIProvider.
            default:
                break;
        }

        if (callback != null)
        {
            RDMDictionaryMsgEvent rdmDictionaryMsgEvent = ReactorFactory.createRDMDictionaryMsgEvent();
            rdmDictionaryMsgEvent.reactorChannel(reactorChannel);
            rdmDictionaryMsgEvent.transportBuffer(transportBuffer);
            rdmDictionaryMsgEvent.msg(msg);
            rdmDictionaryMsgEvent.rdmDictionaryMsg(dictionaryMsg);
            if (streamInfo != null)
            {
                rdmDictionaryMsgEvent.streamInfo().serviceName(streamInfo.serviceName());
                rdmDictionaryMsgEvent.streamInfo().userSpecObject(streamInfo.userSpecObject());
            }
            else
            {
                rdmDictionaryMsgEvent.streamInfo().clear();
            }

            retval = callback.rdmDictionaryMsgCallback(rdmDictionaryMsgEvent);
            rdmDictionaryMsgEvent.returnToPool();
        }
        else
        {
            // callback is undefined, raise it to defaultMsgCallback.
            retval = ReactorCallbackReturnCodes.RAISE;
        }

        return retval;
    }

    // returns ReactorCallbackReturnCodes and populates errorInfo if needed.
    int sendAndHandleDictionaryMsgCallback(String location, ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, DictionaryMsg dictionaryMsg, WatchlistStreamInfo streamInfo, ReactorErrorInfo errorInfo)
    {
        int retval = sendDictionaryMsgCallback(reactorChannel, transportBuffer, msg, dictionaryMsg, streamInfo);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from rdmDictionaryMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " was returned from rdmDictionaryMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        return retval;
    }

    // returns ReactorCallbackReturnCodes and populates errorInfo if needed.
    private int sendAndHandleDictionaryMsgCallback(String location, ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg, DictionaryMsg dictionaryMsg, ReactorErrorInfo errorInfo)
    {
        int retval = sendDictionaryMsgCallback(reactorChannel, transportBuffer, msg, dictionaryMsg, null);

        // check return code from callback.
        if (retval == ReactorCallbackReturnCodes.FAILURE)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
                    "ReactorCallbackReturnCodes.FAILURE was returned from rdmDictionaryMsgCallback(). This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
        {
            // retval is not a valid ReactorReturnCodes.
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of "
                                                                               + retval + " was returned from rdmDictionaryMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
            shutdown(errorInfo);
            return ReactorReturnCodes.FAILURE;
        }
        return retval;
    }

    // return success if no more to dispatch, positive value if there are more
    // to dispatch, or a non-success ReactorReturnCode if an error occurred.
    int dispatchChannel(ReactorChannel reactorChannel, ReactorDispatchOptions dispatchOptions, ReactorErrorInfo errorInfo)
    {
        _reactorLock.lock();

        try
        {
            if (reactorChannel.state() == ReactorChannel.State.CLOSED)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.dispatchChannel", "ReactorChannel is closed, aborting.");
            }
            else if (reactorChannel != _reactorChannel)
            {
                int maxMessages = dispatchOptions.maxMessages();
                int msgCount = 0;
                int retval = ReactorReturnCodes.SUCCESS;

                if (!isReactorChannelReady(reactorChannel))
                {
                    return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.dispatchChannel", "ReactorChannel is not active, aborting.");
                }
                else
                {
                    do
                    {
                        msgCount++;
                        retval = performChannelRead(reactorChannel, dispatchOptions.readArgs(), errorInfo);
                    } while (isReactorChannelReady(reactorChannel) && msgCount < maxMessages && retval > 0);
                }

                return retval;
            }
            else
            {
                int maxMessages = dispatchOptions.maxMessages();
                int msgCount = 0;
                int retval = 0;
                while (msgCount < maxMessages && _workerQueue.readQueueSize() > 0)
                {
                    msgCount++;
                    if ((retval = processWorkerEvent(errorInfo)) < ReactorReturnCodes.SUCCESS)
                        return retval;
                }

                return _workerQueue.readQueueSize();
            }
        }
        finally
        {
            _reactorLock.unlock();
        }
    }

    int submitChannel(ReactorChannel reactorChannel, TransportBuffer buffer, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;
        TransportBuffer writeBuffer = buffer;
        boolean releaseUserBuffer = false; /* This is used to indicate whether to release the buffer owned by user */

        _reactorLock.lock();

        try
        {
            if (!isReactorChannelReady(reactorChannel))
            {
                ret = ReactorReturnCodes.FAILURE;
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.submitChannel", "ReactorChannel is closed, aborting.");
            }
            else if (reactorChannel != _reactorChannel)
            {
            	ReactorPackedBuffer packedBufferImpl = null;
            	boolean isPackedBuffer = packedBufferHashMap.size() == 0 ? false : ((packedBufferImpl = packedBufferHashMap.get(buffer)) != null);
            	
            	if(isPackedBuffer)
            	{
            		if(packedBufferImpl.nextRWFBufferPosition() == 0)
            		{
            			isPackedBuffer = false;
            		}
            	}
            	
                // Checks the channel's protocol type to perform auto conversion for the JSON protocol
                if(reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE && !isPackedBuffer )
                {
                    TransportBuffer writeAgainBuffer = writeCallAgainMap.size() > 0 ?  writeCallAgainMap.remove(buffer) : null; 

                    if(Objects.isNull(writeAgainBuffer))
                    {
                        if( Objects.isNull(jsonConverter))
                        {
                            return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                    "Reactor.submitChannel", "The JSON converter library has not been initialized properly.");
                        }

                        jsonDecodeMsg.clear();
                        _dIter.clear();
                        ret = _dIter.setBufferAndRWFVersion(buffer, reactorChannel.majorVersion(), reactorChannel.minorVersion());

                        ret = jsonDecodeMsg.decode(_dIter);

                        if(ret == CodecReturnCodes.SUCCESS)
                        {
                            converterError.clear();
                            rwfToJsonOptions.clear();
                            rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

                            if( jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults, converterError) != CodecReturnCodes.SUCCESS)
                            {
                                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                        "Reactor.submitChannel", "Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());
                            }

                            TransportBuffer jsonBuffer = reactorChannel.getBuffer(conversionResults.getLength(), false, errorInfo);

                            if(Objects.isNull(jsonBuffer))
                            {
                                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                        "Reactor.submitChannel", "Failed to get a buffer for sending JSON message. Error text: " + errorInfo.error().text());
                            }

                            getJsonMsgOptions.clear();
                            getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                            getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false );

                            if (jsonConverter.getJsonBuffer(jsonBuffer, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
                            {
                                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                        "Reactor.submitChannel", "Failed to get converted JSON message. Error text: " + converterError.getText());
                            }

                            /* Releases the user's buffer when this function writes the JSON buffer successfully. */
                            releaseUserBuffer = true;

                            /* Changes to JSON buffer */
                            writeBuffer = jsonBuffer;
                        }
                    }
                    else
                    {
                        /* Releases the user's buffer when this function writes the JSON buffer successfully. */
                        releaseUserBuffer = true;

                        /* Changes to JSON buffer */
                        writeBuffer = writeAgainBuffer;
                    }
                }

                if (_reactorOptions.xmlTracing()) {
                    xmlString.setLength(0);
                    xmlString
                            .append("\n<!-- Outgoing Reactor message -->\n")
                            .append("<!-- ").append(reactorChannel.selectableChannel().toString()).append(" -->\n")
                            .append("<!-- ").append(new java.util.Date()).append(" -->\n");
                    xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), writeBuffer, null, xmlString, errorInfo.error());
                    System.out.println(xmlString);
                }

                ret = reactorChannel.channel().write(writeBuffer, submitOptions.writeArgs(), errorInfo.error());
                if (ret > TransportReturnCodes.SUCCESS ||
                    ret == TransportReturnCodes.WRITE_FLUSH_FAILED ||
                    ret == TransportReturnCodes.WRITE_CALL_AGAIN)
                {
                    if (sendFlushRequest(reactorChannel, "Reactor.submitChannel", errorInfo) != ReactorReturnCodes.SUCCESS)
                        return ReactorReturnCodes.FAILURE;

                    if (ret != TransportReturnCodes.WRITE_CALL_AGAIN)
                    {
                         if (Objects.nonNull(packedBufferImpl))
                         {
                             packedBufferHashMap.remove(buffer);
                             packedBufferImpl.returnToPool();
                         }
                    	
                        ret = ReactorReturnCodes.SUCCESS;
                    }
                    else
                    {
                        if(buffer != writeBuffer)
                        {
                            writeCallAgainMap.put(buffer, writeBuffer);
                        }

                        releaseUserBuffer = false;
                        ret = ReactorReturnCodes.WRITE_CALL_AGAIN;
                    }
                }
                else if (ret < TransportReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "channel write failure chnl="
                                                     + reactorChannel.channel().selectableChannel() + " errorId="
                                                     + errorInfo.error().errorId() + " errorText="
                                                     + errorInfo.error().text());

                    releaseUserBuffer = false;
                    ret = ReactorReturnCodes.FAILURE;
                }
                else
                {
                    reactorChannel.flushAgain(false);
                 
                    if (Objects.nonNull(packedBufferImpl))
                    {
                        packedBufferHashMap.remove(buffer);
                        packedBufferImpl.returnToPool();
                    }
                }

                /* Releases the user's buffer when the Reactor successfully writes the converted JSON buffer if any. */
                if(releaseUserBuffer)
                {
                    reactorChannel.releaseBuffer(buffer, errorInfo);
                }
            }
        }
        finally
        {
            _reactorLock.unlock();
        }

        // update ping handler for message sent
        if (ret == ReactorReturnCodes.SUCCESS)
        {
            reactorChannel.pingHandler().sentMsg();
        }

        return ret;
    }

    int submitChannel(ReactorChannel reactorChannel, Msg msg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;

        _reactorLock.lock();

        try
        {
            if (!isReactorChannelReady(reactorChannel))
            {
                ret = ReactorReturnCodes.FAILURE;
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.submitChannel", "ReactorChannel is closed, aborting.");
            }
            else if (reactorChannel != _reactorChannel)
            {
                // check first if write for Msg is pending
                if (_submitMsgMap.size() > 0 && _submitMsgMap.containsKey(msg))
                {
                    TransportBuffer writeBuffer = _submitMsgMap.get(msg);
                    ret = submitChannel(reactorChannel, writeBuffer, submitOptions, errorInfo);
                    if (ret != ReactorReturnCodes.WRITE_CALL_AGAIN)
                    {
                        _submitMsgMap.remove(msg);
                    }
                    return ret;
                }
                // Msg not pending - proceed
                int msgSize = encodedMsgSize(msg);
                while (true) // try to get buffer and encode until success or error
                {
                    TransportBuffer writeBuffer
                            = reactorChannel.channel().getBuffer(msgSize, false, errorInfo.error());

                    if (writeBuffer != null)
                    {
                        _eIter.clear();
                        _eIter.setBufferAndRWFVersion(writeBuffer, reactorChannel.channel().majorVersion(), reactorChannel.channel().minorVersion());
                        ret = msg.encode(_eIter);
                        if (ret == CodecReturnCodes.SUCCESS)
                        {
                            ret = submitChannel(reactorChannel, writeBuffer, submitOptions, errorInfo);
                            // add to pending Msg write map if return code is WRITE_CALL_AGAIN
                            if (ret == ReactorReturnCodes.WRITE_CALL_AGAIN)
                            {
                                _submitMsgMap.put(msg, writeBuffer);
                            }
                            break;
                        }
                        else if (ret == CodecReturnCodes.BUFFER_TOO_SMALL) // resize buffer and try again
                        {
                            // release buffer that's too small
                            reactorChannel.channel().releaseBuffer(writeBuffer, errorInfo.error());
                            // resize
                            msgSize *= 2;
                            continue;
                        }
                        else // encoding failure
                        {
                            // release buffer that caused encoding failure
                            reactorChannel.channel().releaseBuffer(writeBuffer, errorInfo.error());

                            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                    "Reactor.submitChannel", "message encoding failure chnl="
                                                             + reactorChannel.channel().selectableChannel() + " errorId="
                                                             + errorInfo.error().errorId() + " errorText="
                                                             + errorInfo.error().text());

                            ret = ReactorReturnCodes.FAILURE;
                            break;
                        }
                    }
                    else // return NO_BUFFERS
                    {
                        if (sendFlushRequest(reactorChannel, "Reactor.submitChannel", errorInfo) != ReactorReturnCodes.SUCCESS)
                            return ReactorReturnCodes.FAILURE;

                        populateErrorInfo(errorInfo, ReactorReturnCodes.NO_BUFFERS,
                                "Reactor.submitChannel", "channel out of buffers chnl="
                                                         + reactorChannel.channel().selectableChannel() + " errorId="
                                                         + errorInfo.error().errorId() + " errorText="
                                                         + errorInfo.error().text());

                        ret = ReactorReturnCodes.NO_BUFFERS;
                        break;
                    }
                }
            }
        }
        finally
        {
            _reactorLock.unlock();
        }

        return ret;
    }

    public int submitChannel(ReactorChannel reactorChannel, MsgBase rdmMsg, ReactorSubmitOptions submitOptions, ReactorErrorInfo errorInfo)
    {
        int ret = ReactorReturnCodes.SUCCESS;

        _reactorLock.lock();

        try
        {
            if (!isReactorChannelReady(reactorChannel))
            {
                ret = ReactorReturnCodes.FAILURE;
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.submitChannel", "ReactorChannel is closed, aborting.");
            }
            else if (reactorChannel != _reactorChannel)
            {
                // check first if write for Msg is pending
                if (_submitRdmMsgMap.size() > 0 && _submitRdmMsgMap.containsKey(rdmMsg))
                {
                    TransportBuffer writeBuffer = _submitRdmMsgMap.get(rdmMsg);
                    ret = submitChannel(reactorChannel, writeBuffer, submitOptions, errorInfo);
                    if (ret != ReactorReturnCodes.WRITE_CALL_AGAIN)
                    {
                        _submitRdmMsgMap.remove(rdmMsg);
                    }
                    return ret;
                }
                // Msg not pending - proceed
                int bufferSize;
                if ((bufferSize = getMaxFragmentSize(reactorChannel, errorInfo)) < 0)
                {
                    return bufferSize;
                }

                while (true) // try to get buffer and encode until success or error
                {
                    TransportBuffer writeBuffer
                            = reactorChannel.channel().getBuffer(bufferSize, false, errorInfo.error());

                    if (writeBuffer != null)
                    {
                        _eIter.clear();
                        _eIter.setBufferAndRWFVersion(writeBuffer, reactorChannel.channel().majorVersion(), reactorChannel.channel().minorVersion());
                        ret = rdmMsg.encode(_eIter);
                        if (ret == CodecReturnCodes.SUCCESS)
                        {
                            ret = submitChannel(reactorChannel, writeBuffer, submitOptions, errorInfo);
                            // add to pending Msg write map if return code is WRITE_CALL_AGAIN
                            if (ret == ReactorReturnCodes.WRITE_CALL_AGAIN)
                            {
                                _submitRdmMsgMap.put(rdmMsg, writeBuffer);
                            }
                            break;
                        }
                        else if (ret == CodecReturnCodes.BUFFER_TOO_SMALL) // resize buffer and try again
                        {
                            // release buffer that's too small
                            reactorChannel.channel().releaseBuffer(writeBuffer, errorInfo.error());
                            // resize
                            bufferSize *= 2;
                            continue;
                        }
                        else // encoding failure
                        {
                            // release buffer that caused encoding failure
                            reactorChannel.channel().releaseBuffer(writeBuffer, errorInfo.error());

                            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                    "Reactor.submitChannel", "message encoding failure chnl="
                                                             + reactorChannel.channel().selectableChannel() + " errorId="
                                                             + errorInfo.error().errorId() + " errorText="
                                                             + errorInfo.error().text());

                            ret = ReactorReturnCodes.FAILURE;
                            break;
                        }
                    }
                    else // return NO_BUFFERS
                    {
                        if (sendFlushRequest(reactorChannel, "Reactor.submitChannel", errorInfo) != ReactorReturnCodes.SUCCESS)
                            return ReactorReturnCodes.FAILURE;

                        populateErrorInfo(errorInfo, ReactorReturnCodes.NO_BUFFERS,
                                "Reactor.submitChannel", "channel out of buffers chnl="
                                                         + reactorChannel.channel().selectableChannel() + " errorId="
                                                         + errorInfo.error().errorId() + " errorText="
                                                         + errorInfo.error().text());

                        ret = ReactorReturnCodes.NO_BUFFERS;
                        break;
                    }
                }
            }
        }
        finally
        {
            _reactorLock.unlock();
        }

        return ret;
    }



    /* Disconnects a channel and notifies application that the channel is down. */
    int disconnect(ReactorChannel reactorChannel, String location, ReactorErrorInfo errorInfo)
    {
        if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
        {
            reactorChannel.state(State.DOWN_RECONNECTING);
        }
        else // server channel or no more retries
        {
            reactorChannel.state(State.DOWN);
        }

        if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
        {
            // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
            return sendAndHandleChannelEventCallback(location,
                    ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                    reactorChannel, errorInfo);
        }
        else // server channel or no more retries
        {
            // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
            return sendAndHandleChannelEventCallback(location,
                    ReactorChannelEventTypes.CHANNEL_DOWN,
                    reactorChannel, errorInfo);
        }
    }

    private int processRwfMessage(TransportBuffer transportBuffer, Buffer buffer, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        // inspect the message and dispatch it to the application.
        _dIter.clear();

        if(Objects.nonNull(buffer))
        {
            _dIter.setBufferAndRWFVersion(buffer, reactorChannel.channel().majorVersion(),
                    reactorChannel.channel().minorVersion());
        }
        else
        {
            _dIter.setBufferAndRWFVersion(transportBuffer, reactorChannel.channel().majorVersion(),
                    reactorChannel.channel().minorVersion());
        }

        _msg.clear();
        int retval = _msg.decode(_dIter);
        if (retval != CodecReturnCodes.SUCCESS)
        {
            return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                    "Reactor.performChannelRead",
                    "initial decode of msg failed, errorId="
                    + errorInfo.error().errorId() + " text="
                    + errorInfo.error().text());
        }

        // determine if for watchlist and process by watchlist
        WlStream wlStream = null;
        _tempWlInteger.value(_msg.streamId());
        if (reactorChannel.watchlist() != null &&
            (wlStream = reactorChannel.watchlist().streamIdtoWlStreamTable().get(_tempWlInteger)) != null)
        {
            if ((retval = reactorChannel.watchlist().readMsg(wlStream, _dIter, _msg, errorInfo)) < ReactorReturnCodes.SUCCESS)
            {
                return retval;
            }
        }
        else // not for watchlist
        {
            // check first if this is a TunnelStream Request for a Provider
            if (reactorChannel.server() != null &&
                _msg.msgClass() == MsgClasses.REQUEST &&
                ((RequestMsg)_msg).checkPrivateStream() &&
                ((RequestMsg)_msg).checkQualifiedStream())
            {
                if ((retval = handleTunnelStreamRequest(reactorChannel, ((RequestMsg)_msg), errorInfo)) != ReactorReturnCodes.SUCCESS)
                    return retval;
            }

            // only process if watchlist not enabled
            else if (reactorChannel.role().type() != ReactorRoleTypes.CONSUMER ||
                     !((ConsumerRole)reactorChannel.role()).watchlistOptions().enableWatchlist())
            {
                if (processChannelMessage(reactorChannel, _dIter, _msg, transportBuffer, errorInfo) == ReactorReturnCodes.FAILURE)
                    return ReactorReturnCodes.FAILURE;
            }
            else
            {
                _closeMsg.msgClass(MsgClasses.CLOSE);
                _closeMsg.streamId(_msg.streamId());
                _closeMsg.domainType(_msg.domainType());
                ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
                retval = submitChannel(reactorChannel, _closeMsg, submitOptions, errorInfo);
                if (retval != CodecReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo,
                            retval,
                            "Reactor.submit",
                            "Submit of CloseMsg failed: <"
                            + TransportReturnCodes.toString(retval) + ">");
                    return ReactorReturnCodes.FAILURE;
                }
            }
        }

        return ReactorReturnCodes.SUCCESS;
    }

    private int sendJSONMessage(TransportBuffer msgBuffer, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        reactorSubmitOptions.clear();
        int ret = reactorChannel.channel().write(msgBuffer, reactorSubmitOptions.writeArgs(), errorInfo.error());
        if (ret > TransportReturnCodes.SUCCESS ||
            ret == TransportReturnCodes.WRITE_FLUSH_FAILED)
        {
            if (sendFlushRequest(reactorChannel, "Reactor.sendJSONMessage", errorInfo) != ReactorReturnCodes.SUCCESS)
                return ReactorReturnCodes.FAILURE;

            ret = ReactorReturnCodes.SUCCESS;
        }
        else if (ret < TransportReturnCodes.SUCCESS)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                    "Reactor.sendJSONMessage", "channel write failure chnl="
                                               + reactorChannel.channel().selectableChannel() + " errorId="
                                               + errorInfo.error().errorId() + " errorText="
                                               + errorInfo.error().text());

            reactorChannel.releaseBuffer(msgBuffer, errorInfo);

            ret = ReactorReturnCodes.FAILURE;
        }
        else
            reactorChannel.flushAgain(false);

        return ret;
    }

    // returns the errorInfo.code() or readArgs.readRetVal.
    private int performChannelRead(ReactorChannel reactorChannel, ReadArgs readArgs, ReactorErrorInfo errorInfo)
    {
        TransportBuffer msgBuf = reactorChannel.channel().read(readArgs, errorInfo.error());
        int retval;

        if (msgBuf != null)
        {
            if (_reactorOptions.xmlTracing()) {
                xmlString.setLength(0);
                xmlString
                        .append("\n<!-- Incoming Reactor message -->\n")
                        .append("<!-- ").append(reactorChannel.selectableChannel().toString()).append(" -->\n")
                        .append("<!-- ").append(new java.util.Date()).append(" -->\n");
                xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null, xmlString, errorInfo.error());
                System.out.println(xmlString);
            }

            // update ping handler
            reactorChannel.pingHandler().receivedMsg();

            // Checks the channel's protocol type to perform auto conversion for the JSON protocol
            if(reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
            {
                boolean failedToConvertJSONMsg = true;
                String jsonErrorMsg = null;

                if(Objects.isNull(jsonConverter))
                {
                    return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.performChannelRead", "The JSON converter library has not been initialized properly.");
                }

                parseJsonOptions.clear();
                parseJsonOptions.setProtocolType(reactorChannel.channel().protocolType());

                converterError.clear();
                retval = jsonConverter.parseJsonBuffer(msgBuf, parseJsonOptions, converterError);

                if(retval == CodecReturnCodes.SUCCESS)
                {
                    decodeJsonMsgOptions.clear();
                    decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                    decodeJsonMsgOptions.setMajorVersion(reactorChannel.channel().majorVersion());
                    decodeJsonMsgOptions.setMinorVersion(reactorChannel.channel().minorVersion());

                    /* Set the ReactorChannel so that users can get it in the ReactorServiceNameToIdCallback callback */
                    if(Objects.nonNull(serviceNameIdConverterClient))
                    {
                        serviceNameIdConverterClient.setReactorChannel(reactorChannel);
                    }

                    jsonMsg.clear();

                    while ( (retval = jsonConverter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions, converterError)) != CodecReturnCodes.END_OF_CONTAINER)
                    {
                        if(retval != CodecReturnCodes.SUCCESS)
                        {
                            /* Failed to convert a JSON message. */
                            break;
                        }

                        switch(jsonMsg.jsonMsgClass())
                        {
                            case JsonMsgClasses.RSSL_MESSAGE:
                            {
                                failedToConvertJSONMsg = false;

                                if (_reactorOptions.xmlTracing()) {
                                    xmlString.setLength(0);
                                    xmlString
                                            .append("\n<!-- Dump Reactor message -->\n")
                                            .append("<!-- ").append(reactorChannel.selectableChannel().toString()).append(" -->\n")
                                            .append("<!-- ").append(new java.util.Date()).append(" -->\n");
                                    xmlDumpTrace.dumpBuffer(reactorChannel.majorVersion(), reactorChannel.minorVersion(), Codec.RWF_PROTOCOL_TYPE,
                                            jsonMsg.rwfMsg().encodedMsgBuffer(), null, xmlString, errorInfo.error());
                                    System.out.println(xmlString);
                                }

                                // inspect the converted message and dispatch it to the application.
                                retval = processRwfMessage(msgBuf, jsonMsg.rwfMsg().encodedMsgBuffer(), reactorChannel, errorInfo);
                                if (retval != ReactorReturnCodes.SUCCESS)
                                {
                                    return retval;
                                }

                                break;
                            }
                            case JsonMsgClasses.PING:
                            {
                                failedToConvertJSONMsg = false;

                                TransportBuffer msgBuffer = reactorChannel.getBuffer(JSON_PONG_MESSAGE.length(), false, errorInfo);

                                if(Objects.nonNull(msgBuffer))
                                {
                                    msgBuffer.data().put(JSON_PONG_MESSAGE.getBytes());

                                    if (_reactorOptions.xmlTracing()) {
                                        xmlString.setLength(0);
                                        xmlString
                                                .append("\n<!-- Outgoing Reactor message -->\n")
                                                .append("<!-- ").append(reactorChannel.selectableChannel().toString()).append(" -->\n")
                                                .append("<!-- ").append(new java.util.Date()).append(" -->\n");
                                        xmlDumpTrace.dumpBuffer(reactorChannel.channel(), Codec.JSON_PROTOCOL_TYPE, msgBuffer, null, xmlString, errorInfo.error());
                                        System.out.println(xmlString);
                                    }

                                    /* Reply with JSON PONG message to the sender */
                                    retval = sendJSONMessage(msgBuffer, reactorChannel, errorInfo);
                                }
                                else
                                {
                                    retval = ReactorReturnCodes.FAILURE;
                                }

                                break;
                            }
                            case JsonMsgClasses.PONG:
                            {
                                failedToConvertJSONMsg = false;
                                /* Do nothing as the ping handle is already updated. */
                                break;
                            }
                            case JsonMsgClasses.ERROR:
                            {
                            	xmlString.setLength(0);
                                xmlDumpTrace.dumpBuffer(reactorChannel.channel(), Codec.JSON_PROTOCOL_TYPE, msgBuf, null, xmlString, errorInfo.error());
                                jsonErrorMsg = xmlString.toString();
                           
                                populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                        "Reactor.performChannelRead", "Received JSON error message: " + jsonErrorMsg);

                                failedToConvertJSONMsg = false;
                                retval = ReactorReturnCodes.FAILURE;
                                break;
                            }
                        }

                        if(retval != ReactorReturnCodes.SUCCESS)
                            break;

                        failedToConvertJSONMsg = true; /* Reset the flag to its initial state. */
                    }
                }
                else
                {
                    failedToConvertJSONMsg = false;

                    /* Failed to parse JSON buffer */
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.performChannelRead", "Failed to parse JSON message: " + converterError.getText());
                }

                if(retval < ReactorReturnCodes.SUCCESS)
                {
                    if(failedToConvertJSONMsg)
                    {
                        /* Send JSON error message back when it fails to decode JSON message. */
                        jsonErrorParams.clear();
                        jsonErrorParams.fillParams(converterError, jsonMsg.rwfMsg().streamId());
                        jsonErrorOutputBuffer.clear();

                        if( (retval = jsonConverter.getErrorMessage(jsonErrorOutputBuffer, jsonErrorParams, converterError)) == CodecReturnCodes.SUCCESS)
                        {
                            TransportBuffer msgBuffer = reactorChannel.getBuffer(jsonErrorOutputBuffer.length(), false, errorInfo);

                            if(Objects.nonNull(msgBuffer))
                            {
                                msgBuffer.data().put(jsonErrorOutputBuffer.data());

                                /* Reply with JSON ERROR message to the sender */
                                retval = sendJSONMessage(msgBuffer, reactorChannel, errorInfo);
                            }
                            else
                            {
                                retval = ReactorReturnCodes.FAILURE;
                            }
                        }
                    }

                    /* Notifies JSON conversion error messages if the callback is specified by users */
                    if ( Objects.nonNull(JsonConversionEventCallback) && (converterError.getCode() != JsonConverterErrorCodes.JSON_ERROR_NO_ERROR_CODE))
                    {
                        jsonConversionEvent.clear();
                        jsonConversionEvent.reactorChannel(reactorChannel);

                        if(failedToConvertJSONMsg)
                        {
                            populateErrorInfo(jsonConversionEvent.errorInfo(), ReactorReturnCodes.FAILURE,
                                    "Reactor.performChannelRead", "Failed to convert JSON message: " + jsonErrorOutputBuffer.toString());
                        }
                        else
                        {
                            populateErrorInfo(jsonConversionEvent.errorInfo(), ReactorReturnCodes.FAILURE,
                                    "Reactor.performChannelRead", "Failed to convert JSON message: " + converterError.getText());
                        }

                        jsonConversionEvent.userSpec = jsonConverterUserSpec;
                        jsonConversionEvent.error().text(jsonConversionEvent.errorInfo().error().text());
                        jsonConversionEvent.error().errorId(CodecReturnCodes.FAILURE);

                        int cret = JsonConversionEventCallback.reactorJsonConversionEventCallback(jsonConversionEvent);

                        if (cret == ReactorCallbackReturnCodes.FAILURE)
                        {
                            return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                    "Reactor.performChannelRead", "Error return code" + cret + " from the ReactorJsonConversionEventCallback callback.");
                        }
                    }

                    /* Don't closes the channel when this function can reply the JSON ERROR message back. */
                    if ( closeChannelFromFailure && (retval != ReactorReturnCodes.SUCCESS))
                    {
                        if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
                        {
                            reactorChannel.state(State.DOWN_RECONNECTING);
                        }
                        else // server channel or no more retries
                        {
                            reactorChannel.state(State.DOWN);
                        }

                        if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
                        {
                            // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
                            sendAndHandleChannelEventCallback("Reactor.performChannelRead",
                                    ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                                    reactorChannel, errorInfo);
                        }
                        else // server channel or no more retries
                        {
                            // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
                            sendAndHandleChannelEventCallback("Reactor.performChannelRead",
                                    ReactorChannelEventTypes.CHANNEL_DOWN,
                                    reactorChannel, errorInfo);
                        }

                        return ReactorReturnCodes.SUCCESS; /* Problem handled, so return success */
                    }
                }
            }
            else
            {
                // inspect the message and dispatch it to the application.
                retval = processRwfMessage(msgBuf, null, reactorChannel, errorInfo);
                if (retval != ReactorReturnCodes.SUCCESS)
                {
                    return retval;
                }
            }
        }
        else
        {
            if (readArgs.readRetVal() == TransportReturnCodes.FAILURE)
            {
                if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
                {
                    reactorChannel.state(State.DOWN_RECONNECTING);
                }
                else // server channel or no more retries
                {
                    reactorChannel.state(State.DOWN);
                }

                if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
                {
                    // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
                    sendAndHandleChannelEventCallback("Reactor.performChannelRead",
                            ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                            reactorChannel, errorInfo);
                }
                else // server channel or no more retries
                {
                    // send CHANNEL_DOWN to user app via reactorChannelEventCallback.
                    sendAndHandleChannelEventCallback("Reactor.performChannelRead",
                            ReactorChannelEventTypes.CHANNEL_DOWN,
                            reactorChannel, errorInfo);
                }

            }
            else if (readArgs.readRetVal() == TransportReturnCodes.READ_FD_CHANGE)
            {
                // reset selectable channel on ReactorChannel to new one
                reactorChannel.selectableChannelFromChannel(reactorChannel.channel());

                // set oldSelectableChannel on ReactorChannel
                reactorChannel.oldSelectableChannel(reactorChannel.channel().oldSelectableChannel());

                // send FD_CHANGE WorkerEvent to Worker.
                if (!sendWorkerEvent(WorkerEventTypes.FD_CHANGE, reactorChannel))
                {
                    // sendWorkerEvent() failed, send channel down
                    reactorChannel.state(State.DOWN);
                    sendAndHandleChannelEventCallback("Reactor.performChannelRead",
                            ReactorChannelEventTypes.CHANNEL_DOWN,
                            reactorChannel, errorInfo);
                    return populateErrorInfo(errorInfo,
                            ReactorReturnCodes.FAILURE,
                            "Reactor.performChannelRead",
                            "sendWorkerEvent() failed");
                }

                // send FD_CHANGE to user app via reactorChannelEventCallback.
                sendAndHandleChannelEventCallback("Reactor.performChannelRead",
                        ReactorChannelEventTypes.FD_CHANGE,
                        reactorChannel, errorInfo);
            }
            else if (readArgs.readRetVal() == TransportReturnCodes.READ_PING)
            {
                reactorChannel.pingHandler().receivedMsg();
            }
        }

        if (readArgs.readRetVal() == TransportReturnCodes.READ_PING)
        {
            // update ping handler
            if(_reactorOptions.pingStatSet())
                reactorChannel.pingHandler().receivedPing();
        }

        // Aggregate number of bytes read
        if(_reactorOptions.readStatSet() == true)
        {
            ((ReadArgsImpl)_readArgsAggregator).bytesRead(overflowSafeAggregate(_readArgsAggregator.bytesRead(), readArgs.bytesRead()));
            ((ReadArgsImpl)_readArgsAggregator).uncompressedBytesRead(overflowSafeAggregate(_readArgsAggregator.uncompressedBytesRead(), readArgs.uncompressedBytesRead()));
        }

        if (readArgs.readRetVal() > 0)
        {
            return readArgs.readRetVal();
        }

        return ReactorReturnCodes.SUCCESS;
    }

    private int handleTunnelStreamRequest(ReactorChannel reactorChannel, RequestMsg msg, ReactorErrorInfo errorInfo)
    {
        int ret;
        boolean isValid = false;
        String rejectString = null;

        _tunnelStreamRequestEvent.clear();

        if (((ProviderRole)reactorChannel.role()).tunnelStreamListenerCallback() != null)
        {
            // reject if TunnelStream already exists
            _tempWlInteger.value(msg.streamId());
            if (!reactorChannel.streamIdtoTunnelStreamTable().containsKey(_tempWlInteger))
            {
                if (msg.containerType() == DataTypes.FILTER_LIST &&
                    msg.msgKey().checkHasFilter())
                {
                    if ((ret = _tunnelStreamRequestEvent.classOfService().decodeCommonProperties(_reactorChannel, msg.encodedDataBody(), errorInfo)) == ReactorReturnCodes.SUCCESS)
                    {
                        // check if stream version is less than or equal to current stream version
                        int requestedStreamVersion = _tunnelStreamRequestEvent.classOfService().common().streamVersion();
                        if (requestedStreamVersion <= CosCommon.CURRENT_STREAM_VERSION)
                        {
                            isValid = true;
                        }
                        else
                        {
                            rejectString = "Unsupported class of service stream version: " + requestedStreamVersion;
                        }
                    }
                    else
                    {
                        rejectString = "Class of service common properties decode failed with return code: " + ret + " <" + errorInfo.error().text() + ">";
                    }
                }
                else // doesn't contain FILTER_LIST and have filter in message key
                {
                    rejectString = "TunnelStream request must contain FILTER_LIST and have filter in message key";
                }
            }
            else // TunnelStream already exists, reject
            {
                rejectString = "TunnelStream is already open for stream id " + msg.streamId();
            }
        }
        else  // no listener callback, tunnel streams aren't supported
        {
            rejectString = "Provider does not support TunnelStreams";
        }

        if (isValid)
        {
            _tunnelStreamRequestEvent.reactorChannel(reactorChannel);
            _tunnelStreamRequestEvent.domainType(msg.domainType());
            _tunnelStreamRequestEvent.streamId(msg.streamId());
            _tunnelStreamRequestEvent.serviceId(msg.msgKey().serviceId());
            _tunnelStreamRequestEvent.name(msg.msgKey().name().toString());
            _tunnelStreamRequestEvent.msg(msg);
            _tunnelStreamRequestEvent.classOfServiceFilter(msg.msgKey().filter());
            _tunnelStreamRequestEvent.errorInfo(errorInfo);

            return ((ProviderRole)reactorChannel.role()).tunnelStreamListenerCallback().listenerCallback(_tunnelStreamRequestEvent);
        }
        else
        {
            // reject TunnelStream here
            _tunnelStreamRejectOptions.clear();
            _tunnelStreamRejectOptions.state().streamState(StreamStates.CLOSED);
            _tunnelStreamRejectOptions.state().dataState(DataStates.SUSPECT);
            _tunnelStreamRejectOptions.state().code(StateCodes.NONE);
            _tunnelStreamRejectOptions.state().text().data(rejectString);

            _tunnelStreamRequestEvent.clear();
            _tunnelStreamRequestEvent.reactorChannel(reactorChannel);
            _tunnelStreamRequestEvent.domainType(((RequestMsg)_msg).domainType());
            _tunnelStreamRequestEvent.streamId(((RequestMsg)_msg).streamId());
            _tunnelStreamRequestEvent.serviceId(((RequestMsg)_msg).msgKey().serviceId());
            _tunnelStreamRequestEvent.name(((RequestMsg)_msg).msgKey().name().toString());

            if (reactorChannel.rejectTunnelStream(_tunnelStreamRequestEvent, _tunnelStreamRejectOptions, errorInfo) < ReactorReturnCodes.SUCCESS)
            {
                return ReactorReturnCodes.FAILURE;
            }

            // send warning event to reactor channel
            sendAndHandleChannelEventCallback("Reactor.handleTunnelStreamRequest",
                    ReactorChannelEventTypes.WARNING,
                    reactorChannel, errorInfo);
        }

        return ReactorReturnCodes.SUCCESS;
    }

    // returns the errorInfo.code().
    @SuppressWarnings("fallthrough")
    private int processWorkerEvent(ReactorErrorInfo errorInfo)
    {
        WorkerEvent event = (WorkerEvent)_workerQueue.read();
        if (event == null)
            return 0;

        populateErrorInfo(errorInfo, event.errorInfo().code(), event.errorInfo().location(), event.errorInfo().error().text());
        WorkerEventTypes eventType = event.eventType();

        ReactorChannel reactorChannel = event.reactorChannel();
        int ret = ReactorReturnCodes.SUCCESS;

        switch (eventType)
        {
            case FLUSH_DONE:
                reactorChannel.flushRequested(false);
                if (reactorChannel.flushAgain())
                {
                    /* Channel wrote a message since its last flush request, request flush again
                     * in case that message was not flushed. */
                    if (sendFlushRequest(reactorChannel, "Reactor.processWorkerEvent", errorInfo) != ReactorReturnCodes.SUCCESS)
                    {
                        event.returnToPool();
                        return ReactorReturnCodes.FAILURE;
                    }
                }

                /* Dispatch watchlist; it may be waiting on a flush to complete if it ran out of output buffers. */
                if (reactorChannel.watchlist() != null)
                {
                    if ((ret = reactorChannel.watchlist().dispatch(errorInfo)) != ReactorReturnCodes.SUCCESS)
                    {
                        populateErrorInfo(errorInfo, ret,
                                "Reactor.processWorkerEvent",
                                "Watchlist dispatch failed - " + errorInfo.error().text());
                        event.returnToPool();
                        return ret;
                    }
                }
                break;
            case CHANNEL_UP:
                processChannelUp(event, errorInfo);
                break;
            case CHANNEL_DOWN:
                if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.SUCCESS,
                            "Reactor.processWorkerEvent",
                            errorInfo.error().text());
                }
                else // server channel or no more retries
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.SUCCESS,
                            "Reactor.processWorkerEvent",
                            errorInfo.error().text());
                }
                processChannelDown(event, errorInfo);
                break;
            case CHANNEL_CLOSE_ACK:
                /* Worker is done with channel. Safe to release it. */
                reactorChannel.returnToPool();
                break;

            case WARNING:
                /* Override the error code as this is not a failure */
                populateErrorInfo(errorInfo, ReactorReturnCodes.SUCCESS,
                        "Reactor.processWorkerEvent",
                        errorInfo.error().text());

                sendChannelEventCallback(ReactorChannelEventTypes.WARNING, event.reactorChannel(), errorInfo);
                break;
            case TOKEN_MGNT:

                populateErrorInfo(errorInfo, errorInfo.code(),
                        "Reactor.processWorkerEvent",
                        errorInfo.error().text());

                sendAuthTokenEventCallback(reactorChannel, event._tokenSession.authTokenInfo(), errorInfo);

                /* Override the error code as this is not a failure */
                errorInfo.code(ReactorReturnCodes.SUCCESS);

                break;

            case TOKEN_CREDENTIAL_RENEWAL:

                /* Override the error code as this is not a failure */
                populateErrorInfo(errorInfo, ReactorReturnCodes.SUCCESS,
                        "Reactor.processWorkerEvent",
                        errorInfo.error().text());

                /* This is used to check whether the submitOAuthCredentialRenewal() is called in the callback */
                _tokenSessionForCredentialRenewalCallback = event._tokenSession;
                sendOAuthCredentialEventCallback(event._tokenSession, errorInfo);
                _tokenSessionForCredentialRenewalCallback = null;

                break;

            case SHUTDOWN:
                processWorkerShutdown(event, "Reactor.processWorkerEvent", errorInfo);
                break;
            case TUNNEL_STREAM_DISPATCH_TIMEOUT:
                reactorChannel.clearTunnelStreamManagerExpireTime();
                /* (Fall through) */
            case TUNNEL_STREAM_DISPATCH_NOW:
                while ((ret = reactorChannel.tunnelStreamManager().dispatch(errorInfo.error())) > ReactorReturnCodes.SUCCESS);
                if (ret < ReactorReturnCodes.SUCCESS)
                {
                    // send channel down event if channel error occurred
                    if (ret == ReactorReturnCodes.CHANNEL_ERROR)
                    {
                        if (reactorChannel.state() != State.DOWN && reactorChannel.state() != State.DOWN_RECONNECTING)
                        {
                            if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
                            {
                                reactorChannel.state(State.DOWN_RECONNECTING);
                                sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
                                        ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                                        reactorChannel, errorInfo);
                            }
                            else // server channel or no more retries
                            {
                                reactorChannel.state(State.DOWN);
                                sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
                                        ReactorChannelEventTypes.CHANNEL_DOWN,
                                        reactorChannel, errorInfo);
                            }
                        }
                        // return SUCCESS on dispatch() for CHANNEL_ERROR, application should close ReactorChannel
                        ret = ReactorReturnCodes.SUCCESS;
                    }
                    else
                    {
                        populateErrorInfo(errorInfo, ret,
                                "Reactor.processWorkerEvent",
                                "TunnelStream dispatch failed - " + errorInfo.error().text());
                    }
                }

                if ((ret = reactorChannel.checkTunnelManagerEvents(errorInfo)) != ReactorReturnCodes.SUCCESS)
                {
                    event.returnToPool();
                    return ret;
                }

                break;
            case WATCHLIST_TIMEOUT:
                if (reactorChannel.watchlist() != null)
                {
                    if ((ret = reactorChannel.watchlist().timeout(errorInfo)) != ReactorReturnCodes.SUCCESS)
                    {
                        event.returnToPool();
                        return ret;
                    }
                }
                break;
            case WATCHLIST_DISPATCH_NOW:
                if (reactorChannel.watchlist() != null)
                {
                    if ((ret = reactorChannel.watchlist().dispatch(errorInfo)) != ReactorReturnCodes.SUCCESS)
                    {
                        event.returnToPool();
                        return ret;
                    }
                }
                break;
            default:
                event.returnToPool();
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.processWorkerEvent",
                        "received an unexpected WorkerEventType of " + eventType);
        }

        event.returnToPool();
        return errorInfo.code();
    }

    private void processWorkerShutdown(WorkerEvent event, String location, ReactorErrorInfo errorInfo)
    {
        populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "Worker has shutdown, "
                                                                           + event.errorInfo().toString());
    }

    private void processChannelDown(WorkerEvent event, ReactorErrorInfo errorInfo)
    {
        ReactorChannel reactorChannel = event.reactorChannel();

        if (reactorChannel.state() != State.CLOSED)
        {
            if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
            {
                // send CHANNEL_DOWN_RECONNECTING
                reactorChannel.state(State.DOWN_RECONNECTING);

                // send channel_down to user app via reactorChannelEventCallback.
                sendAndHandleChannelEventCallback("Reactor.processChannelDown",
                        ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                        reactorChannel, errorInfo);
            }
            else // server channel or no more retries
            {
                // send CHANNEL_DOWN since server channels are not recovered
                reactorChannel.state(State.DOWN);

                // send channel_down to user app via reactorChannelEventCallback.
                sendAndHandleChannelEventCallback("Reactor.processChannelDown",
                        ReactorChannelEventTypes.CHANNEL_DOWN,
                        reactorChannel, errorInfo);
            }
        }
    }

    private void processChannelUp(WorkerEvent event, ReactorErrorInfo errorInfo)
    {
        ReactorChannel reactorChannel = event.reactorChannel();
        ReactorRole reactorRole = reactorChannel.role();

        if (reactorChannel.state() == State.CLOSED || reactorChannel.state() == State.DOWN)
        {
            return;
        }

        // handle queue messaging
        reactorChannel.tunnelStreamManager().setChannel(reactorChannel, errorInfo.error());

        // update ReactorChannel's state to ACTIVE.
        reactorChannel.state(State.UP);

        // If channel has no watchlist, consider connection established and reset the
        // reconnect timer.
        if (reactorChannel.watchlist() == null)
            reactorChannel.resetReconnectTimers();

        // Reset aggregating statistics
        _readArgsAggregator.clear();
        _writeArgsAggregator.clear();
        reactorChannel.pingHandler().resetAggregatedStats();

        // send channel_up to user app via reactorChannelEventCallback.
        if (sendAndHandleChannelEventCallback("Reactor.processChannelUp",
                ReactorChannelEventTypes.CHANNEL_UP, reactorChannel,
                errorInfo) != ReactorCallbackReturnCodes.SUCCESS)
        {
            return;
        }

        // check role and start sending predefined messages (if specified).
        // if none are specified, send channel_ready.
        if (reactorRole.type() == ReactorRoleTypes.CONSUMER)
        {
            if (reactorChannel.state() == State.CLOSED || reactorChannel.state() == State.DOWN)
                return;

            LoginRequest loginRequest = null;

            if (reactorChannel.enableSessionManagement())
                loginRequest = reactorChannel._loginRequestForEDP;
            else
                loginRequest = ((ConsumerRole)reactorRole).rdmLoginRequest();

            if (loginRequest != null)
            {
                if (reactorChannel.watchlist() == null) // watchlist not enabled
                {
                    // a rdmLoginRequest was specified, send it out.
                    encodeAndWriteLoginRequest(loginRequest, reactorChannel, errorInfo);
                }
                else // watchlist enabled
                {
                    if (reactorChannel.watchlist()._loginHandler != null
                        && reactorChannel.watchlist()._loginHandler._loginRequestForEDP != null)
                    {
                        reactorChannel.watchlist()._loginHandler._loginRequestForEDP.userName(loginRequest.userName());
                    }
                    reactorChannel.watchlist()._loginHandler.rttEnabled =
                            loginRequest.attrib().checkHasSupportRoundTripLatencyMonitoring();
                    reactorChannel.watchlist().channelUp(errorInfo);
                }
            }
            else
            {
                // no rdmLoginRequest defined, so just send CHANNEL_READY
                reactorChannel.state(State.READY);
                if (sendAndHandleChannelEventCallback("Reactor.processChannelUp",
                        ReactorChannelEventTypes.CHANNEL_READY,
                        reactorChannel, errorInfo) != ReactorCallbackReturnCodes.SUCCESS)
                {
                    return;
                }
            }
        }
        else if (reactorRole.type() == ReactorRoleTypes.NIPROVIDER)
        {
            if (reactorChannel.state() == State.CLOSED || reactorChannel.state() == State.DOWN)
                return;

            LoginRequest loginRequest = ((NIProviderRole)reactorRole).rdmLoginRequest();
            if (loginRequest != null)
            {
                // a rdmLoginRequest was specified, send it out.
                encodeAndWriteLoginRequest(loginRequest, reactorChannel, errorInfo);
            }
            else
            {
                // no rdmLoginRequest defined, so just send CHANNEL_READY
                reactorChannel.state(State.READY);
                if (sendAndHandleChannelEventCallback("Reactor.processChannelUp",
                        ReactorChannelEventTypes.CHANNEL_READY,
                        reactorChannel, errorInfo) != ReactorCallbackReturnCodes.SUCCESS)
                {
                    return;
                }
            }
        }
        else if (reactorRole.type() == ReactorRoleTypes.PROVIDER)
        {
            // send CHANNEL_READY
            reactorChannel.state(State.READY);
            if (sendAndHandleChannelEventCallback("Reactor.processChannelUp",
                    ReactorChannelEventTypes.CHANNEL_READY,
                    reactorChannel, errorInfo) != ReactorCallbackReturnCodes.SUCCESS)
            {
                return;
            }
        }
    }

    private void encodeAndWriteLoginRequest(LoginRequest loginRequest, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        // get a buffer for the login request
        Channel channel = reactorChannel.channel();
        if (channel == null)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteLoginRequest",
                    "Failed to obtain an action channel");
            return;
        }

        int bufSize;
        if ((bufSize = getMaxFragmentSize(reactorChannel, errorInfo)) < 0)
        {
            return;
        }

        TransportBuffer msgBuf = channel.getBuffer(bufSize, false, errorInfo.error());

        if (msgBuf == null)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteLoginRequest",
                    "Failed to obtain a TransportBuffer, reason="
                    + errorInfo.error().text());
            return;
        }

        _eIter.clear();
        _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

        int retval = loginRequest.encode(_eIter);
        if (retval != CodecReturnCodes.SUCCESS)
        {
            // set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo)
            reactorChannel.state(State.DOWN);
            sendAndHandleChannelEventCallback("Reactor.encodeAndWriteLoginRequest",
                    ReactorChannelEventTypes.CHANNEL_DOWN,
                    reactorChannel, errorInfo);
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteLoginRequest",
                    "Encoding of login request failed: <"
                    + TransportReturnCodes.toString(retval) + ">");
            return;
        }

        // Checks the channel's protocol type to perform auto conversion for the JSON protocol
        if(reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
        {
            if( Objects.isNull(jsonConverter))
            {
                populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.submitChannel", "The JSON converter library has not been initialized properly.");
                return;
            }

            jsonDecodeMsg.clear();
            _dIter.clear();
            int ret = _dIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());

            ret = jsonDecodeMsg.decode(_dIter);

            if(ret == CodecReturnCodes.SUCCESS)
            {
                converterError.clear();
                rwfToJsonOptions.clear();
                rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

                if( jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults, converterError) != CodecReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());

                    return;
                }

                TransportBuffer buffer = reactorChannel.getBuffer(conversionResults.getLength(), false, errorInfo);

                if(Objects.isNull(buffer))
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to get a buffer for sending JSON message. Error text: " + errorInfo.error().text());

                    return;
                }

                getJsonMsgOptions.clear();
                getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false );

                if (jsonConverter.getJsonBuffer(buffer, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to get converted JSON message. Error text: " + converterError.getText());
                    return;
                }

                /* Release the original buffer */
                reactorChannel.releaseBuffer(msgBuf, errorInfo);

                msgBuf = buffer;
            }
        }

        if (_reactorOptions.xmlTracing()) {
            xmlString.setLength(0);
            xmlString
                    .append("\n<!-- Outgoing Reactor message -->\n")
                    .append("<!-- ").append(reactorChannel.selectableChannel().toString()).append(" -->\n")
                    .append("<!-- ").append(new java.util.Date()).append(" -->\n");
            xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null, xmlString, errorInfo.error());
            System.out.println(xmlString);
        }
        retval = channel.write(msgBuf, _writeArgs, errorInfo.error());

        // Aggregate number of bytes written
        if(_reactorOptions.writeStatSet() == true)
        {
            ((WriteArgsImpl)_writeArgsAggregator).bytesWritten(overflowSafeAggregate(_writeArgsAggregator.bytesWritten(),_writeArgs.bytesWritten()));
            ((WriteArgsImpl)_writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(_writeArgsAggregator.uncompressedBytesWritten(),_writeArgs.uncompressedBytesWritten()));
        }

        if (retval > TransportReturnCodes.SUCCESS)
        {
            sendFlushRequest(reactorChannel, "Reactor.encodeAndWriteLoginRequest", errorInfo);
        }
        else if (retval < TransportReturnCodes.SUCCESS)
        {
            // write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
            // for user application.
            // also, set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo))
            if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
            {
                reactorChannel.state(State.DOWN_RECONNECTING);
                sendAndHandleChannelEventCallback("Reactor.encodeAndWriteLoginRequest",
                        ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                        reactorChannel, errorInfo);
            }
            else // server channel or no more retries
            {
                reactorChannel.state(State.DOWN);
                sendAndHandleChannelEventCallback("Reactor.encodeAndWriteLoginRequest",
                        ReactorChannelEventTypes.CHANNEL_DOWN,
                        reactorChannel, errorInfo);
            }
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteLoginRequest",
                    "Channel.write failed to write login request: <"
                    + CodecReturnCodes.toString(retval) + ">" + " error="
                    + errorInfo.error().text());
        }
        else
            reactorChannel.flushAgain(false);
    }

    /**
     *
     * @param msg - generic msg which should be decoded and handled.
     * @param reactorChannel - channel connection between client and server.
     * @param errorInfo - Warning/Error information buffer.
     * @param decodeIterator - An iterator for decoding the RWF content.
     * @return true when message must be proceeded. Method is returned false if result of this method should be ignored.
     * For instance: when {@link LoginMsg#rdmMsgType()} is {@link LoginMsgType#RTT} and RTT messaging is not supported by a
     * {@link ConsumerRole}
     */
    @SuppressWarnings("fallthrough")
    private boolean proceedLoginGenericMsg(ReactorChannel reactorChannel, DecodeIterator decodeIterator,
                                           Msg msg, ReactorErrorInfo errorInfo) {
        LoginMsg loginGenericMsg = _loginMsg;
        if (Objects.equals(DataTypes.ELEMENT_LIST, msg.containerType())) {
            loginGenericMsg.rdmMsgType(LoginMsgType.RTT);
            switch (reactorChannel.role().type()) {
                case ReactorRoleTypes.PROVIDER:
                    break;
                case ReactorRoleTypes.CONSUMER: {
                    ConsumerRole consumerRole = (ConsumerRole) reactorChannel.role();
                    if (consumerRole.rttEnabled()) {
                        returnBackRTTMessage(msg, reactorChannel, errorInfo);
                        break;
                    }
                }
                default: {
                    /*return false when it is not enabled for consumer or when it is NIProvider
                    for preventing further handling*/
                    return false;
                }
            }
        } else {
            loginGenericMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
        }
        loginGenericMsg.decode(decodeIterator, msg);
        return true;
    }

    private void returnBackRTTMessage(Msg msg, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo) {
        reactorSubmitOptions.clear();
        int retval = submitChannel(reactorChannel, msg, reactorSubmitOptions, errorInfo);

        if (retval != CodecReturnCodes.SUCCESS) {
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.returnBackRTTMessage",
                    "Reactor.submitChannel failed to return back login RTT message: <"
                    + CodecReturnCodes.toString(retval) + ">" + " error="
                    + errorInfo.error().text());
        }
    }

    private void encodeAndWriteDirectoryRequest(DirectoryRequest directoryRequest, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        // get a buffer for the directory request
        Channel channel = reactorChannel.channel();

        int bufSize;
        if ((bufSize = getMaxFragmentSize(reactorChannel, errorInfo)) < 0)
        {
            return;
        }

        TransportBuffer msgBuf = channel.getBuffer(bufSize, false, errorInfo.error());

        if (msgBuf == null)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDirectoryRequest",
                    "Failed to obtain a TransportBuffer, reason="
                    + errorInfo.error().text());
            return;
        }

        _eIter.clear();
        _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

        int retval = directoryRequest.encode(_eIter);
        if (retval != CodecReturnCodes.SUCCESS)
        {
            // set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo)
            reactorChannel.state(State.DOWN);
            sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRequest",
                    ReactorChannelEventTypes.CHANNEL_DOWN,
                    reactorChannel, errorInfo);
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDirectoryRequest",
                    "Encoding of directory request failed: <"
                    + TransportReturnCodes.toString(retval) + ">");
            return;
        }

        // Checks the channel's protocol type to perform auto conversion for the JSON protocol
        if(reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
        {
            if( Objects.isNull(jsonConverter))
            {
                populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.submitChannel", "The JSON converter library has not been initialized properly.");
                return;
            }

            jsonDecodeMsg.clear();
            _dIter.clear();
            int ret = _dIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());

            ret = jsonDecodeMsg.decode(_dIter);

            if(ret == CodecReturnCodes.SUCCESS)
            {
                converterError.clear();
                rwfToJsonOptions.clear();
                rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

                if( jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults, converterError) != CodecReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());

                    return;
                }

                TransportBuffer buffer = reactorChannel.getBuffer(conversionResults.getLength(), false, errorInfo);

                if(Objects.isNull(buffer))
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to get a buffer for sending JSON message. Error text: " + errorInfo.error().text());

                    return;
                }

                getJsonMsgOptions.clear();
                getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false );

                if (jsonConverter.getJsonBuffer(buffer, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to get converted JSON message. Error text: " + converterError.getText());
                    return;
                }

                /* Release the original buffer */
                reactorChannel.releaseBuffer(msgBuf, errorInfo);

                msgBuf = buffer;
            }
        }

        if (_reactorOptions.xmlTracing()) {
            xmlString.setLength(0);
            xmlString
                    .append("\n<!-- Outgoing Reactor message -->\n")
                    .append("<!-- ").append(reactorChannel.selectableChannel().toString()).append(" -->\n")
                    .append("<!-- ").append(new java.util.Date()).append(" -->\n");
            xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null, xmlString, errorInfo.error());
            System.out.println(xmlString);
        }

        retval = channel.write(msgBuf, _writeArgs, errorInfo.error());

        // Aggregate number of bytes written
        if(_reactorOptions.writeStatSet() == true)
        {
            ((WriteArgsImpl)_writeArgsAggregator).bytesWritten(overflowSafeAggregate(_writeArgsAggregator.bytesWritten(), _writeArgs.uncompressedBytesWritten()));
            ((WriteArgsImpl)_writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(_writeArgsAggregator.uncompressedBytesWritten(), _writeArgs.uncompressedBytesWritten()));
        }

        if (retval > TransportReturnCodes.SUCCESS)
        {
            sendFlushRequest(reactorChannel, "Reactor.encodeAndWriteDirectoryRequest", errorInfo);
        }
        else if (retval < TransportReturnCodes.SUCCESS)
        {
            // write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
            // for user application.
            // also, set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo))
            if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
            {
                reactorChannel.state(State.DOWN_RECONNECTING);
                sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRequest",
                        ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                        reactorChannel, errorInfo);
            }
            else // server channel or no more retries
            {
                reactorChannel.state(State.DOWN);
                sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRequest",
                        ReactorChannelEventTypes.CHANNEL_DOWN,
                        reactorChannel, errorInfo);
            }
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDirectoryRequest",
                    "Channel.write failed to write directory request: <"
                    + CodecReturnCodes.toString(retval) + ">" + " error="
                    + errorInfo.error().text());
        }
        else
            reactorChannel.flushAgain(false);
    }

    private void encodeAndWriteDirectoryRefresh(DirectoryRefresh directoryRefresh, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        // get a buffer for the directory request
        Channel channel = reactorChannel.channel();

        int bufSize;
        if ((bufSize = getMaxFragmentSize(reactorChannel, errorInfo)) < 0)
        {
            return;
        }

        TransportBuffer msgBuf = channel.getBuffer(bufSize, false, errorInfo.error());

        if (msgBuf == null)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDirectoryRefresh",
                    "Failed to obtain a TransportBuffer, reason="
                    + errorInfo.error().text());
            return;
        }

        _eIter.clear();
        _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

        int retval = directoryRefresh.encode(_eIter);
        if (retval != CodecReturnCodes.SUCCESS)
        {
            // set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo)
            reactorChannel.state(State.DOWN);
            sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRefresh",
                    ReactorChannelEventTypes.CHANNEL_DOWN,
                    reactorChannel, errorInfo);
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDirectoryRefresh",
                    "Encoding of directory refresh failed: <"
                    + TransportReturnCodes.toString(retval) + ">");
            return;
        }

        // Checks the channel's protocol type to perform auto conversion for the JSON protocol
        if(reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
        {
            if( Objects.isNull(jsonConverter))
            {
                populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.submitChannel", "The JSON converter library has not been initialized properly.");
                return;
            }

            jsonDecodeMsg.clear();
            _dIter.clear();
            int ret = _dIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());

            ret = jsonDecodeMsg.decode(_dIter);

            if(ret == CodecReturnCodes.SUCCESS)
            {
                converterError.clear();
                rwfToJsonOptions.clear();
                rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

                if( jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults, converterError) != CodecReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());

                    return;
                }

                TransportBuffer buffer = reactorChannel.getBuffer(conversionResults.getLength(), false, errorInfo);

                if(Objects.isNull(buffer))
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to get a buffer for sending JSON message. Error text: " + errorInfo.error().text());

                    return;
                }

                getJsonMsgOptions.clear();
                getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false );

                if (jsonConverter.getJsonBuffer(buffer, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to get converted JSON message. Error text: " + converterError.getText());
                    return;
                }

                /* Release the original buffer */
                reactorChannel.releaseBuffer(msgBuf, errorInfo);

                msgBuf = buffer;
            }
        }

        if (_reactorOptions.xmlTracing()) {
            xmlString.setLength(0);
            xmlString
                    .append("\n<!-- Outgoing Reactor message -->\n")
                    .append("<!-- ").append(reactorChannel.selectableChannel().toString()).append(" -->\n")
                    .append("<!-- ").append(new java.util.Date()).append(" -->\n");
            xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null, xmlString, errorInfo.error());
            System.out.println(xmlString);
        }
        retval = channel.write(msgBuf, _writeArgs, errorInfo.error());

        // Aggregate number of bytes written
        if(_reactorOptions.writeStatSet() == true)
        {
            ((WriteArgsImpl)_writeArgsAggregator).bytesWritten(overflowSafeAggregate(_writeArgsAggregator.bytesWritten(), _writeArgs.bytesWritten()));
            ((WriteArgsImpl)_writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(_writeArgsAggregator.uncompressedBytesWritten(), _writeArgs.uncompressedBytesWritten()));
        }

        if (retval > TransportReturnCodes.SUCCESS)
        {
            sendFlushRequest(reactorChannel, "Reactor.encodeAndWriteDirectoryRefresh", errorInfo);
        }
        else if (retval < TransportReturnCodes.SUCCESS)
        {
            // write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
            // for user application.
            // also, set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo))
            if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
            {
                reactorChannel.state(State.DOWN_RECONNECTING);
                sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRefresh",
                        ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                        reactorChannel, errorInfo);
            }
            else // server channel or no more retries
            {
                reactorChannel.state(State.DOWN);
                sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRefresh",
                        ReactorChannelEventTypes.CHANNEL_DOWN,
                        reactorChannel, errorInfo);
            }
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDirectoryRefresh",
                    "Channel.write failed to write directory refresh: <"
                    + CodecReturnCodes.toString(retval) + ">" + " error="
                    + errorInfo.error().text());
        }
        else
            reactorChannel.flushAgain(false);
    }

    private void encodeAndWriteDictionaryRequest(DictionaryRequest dictionaryRequest, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        // get a buffer for the dictionary request
        Channel channel = reactorChannel.channel();

        int bufSize;
        if ((bufSize = getMaxFragmentSize(reactorChannel, errorInfo)) < 0)
        {
            return;
        }

        TransportBuffer msgBuf = channel.getBuffer(bufSize, false, errorInfo.error());

        if (msgBuf == null)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDictionaryRequest",
                    "Failed to obtain a TransportBuffer, reason="
                    + errorInfo.error().text());
            return;
        }

        _eIter.clear();
        _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

        int retval = dictionaryRequest.encode(_eIter);
        if (retval != CodecReturnCodes.SUCCESS)
        {
            // set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo)
            reactorChannel.state(State.DOWN);
            sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryRequest",
                    ReactorChannelEventTypes.CHANNEL_DOWN,
                    reactorChannel, errorInfo);
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDictionaryRequest",
                    "Encoding of dictionary request failed: <"
                    + TransportReturnCodes.toString(retval) + ">");
            return;
        }

        // Checks the channel's protocol type to perform auto conversion for the JSON protocol
        if(reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
        {
            if( Objects.isNull(jsonConverter))
            {
                populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.submitChannel", "The JSON converter library has not been initialized properly.");
                return;
            }

            jsonDecodeMsg.clear();
            _dIter.clear();
            int ret = _dIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());

            ret = jsonDecodeMsg.decode(_dIter);

            if(ret == CodecReturnCodes.SUCCESS)
            {
                converterError.clear();
                rwfToJsonOptions.clear();
                rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

                if( jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults, converterError) != CodecReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());
                    return;
                }

                TransportBuffer jsonBuffer = reactorChannel.getBuffer(conversionResults.getLength(), false, errorInfo);

                if(Objects.isNull(jsonBuffer))
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to get a buffer for sending JSON message. Error text: " + errorInfo.error().text());
                    return;
                }

                getJsonMsgOptions.clear();
                getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false );

                if (jsonConverter.getJsonBuffer(jsonBuffer, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to get converted JSON message. Error text: " + converterError.getText());
                    return;
                }

                /* Releases the original message buffer */
                reactorChannel.releaseBuffer(msgBuf, errorInfo);

                msgBuf = jsonBuffer;
            }
        }

        if (_reactorOptions.xmlTracing()) {
            xmlString.setLength(0);
            xmlString
                    .append("\n<!-- Outgoing Reactor message -->\n")
                    .append("<!-- ").append(reactorChannel.selectableChannel().toString()).append(" -->\n")
                    .append("<!-- ").append(new java.util.Date()).append(" -->\n");
            xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null, xmlString, errorInfo.error());
            System.out.println(xmlString);
        }
        retval = channel.write(msgBuf, _writeArgs, errorInfo.error());

        // Aggregate number of bytes written
        if(_reactorOptions.writeStatSet() == true)
        {
            ((WriteArgsImpl)_writeArgsAggregator).bytesWritten(overflowSafeAggregate(_writeArgsAggregator.bytesWritten(), _writeArgs.bytesWritten()));
            ((WriteArgsImpl)_writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(_writeArgsAggregator.uncompressedBytesWritten(), _writeArgs.uncompressedBytesWritten()));
        }
        if (retval > TransportReturnCodes.SUCCESS)
        {
            sendFlushRequest(reactorChannel, "Reactor.encodeAndWriteDictionaryRequest", errorInfo);
        }
        else if (retval < TransportReturnCodes.SUCCESS)
        {
            // write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
            // for user application.
            // also, set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo))
            if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
            {
                reactorChannel.state(State.DOWN_RECONNECTING);
                sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryRequest",
                        ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                        reactorChannel, errorInfo);
            }
            else // server channel or no more retries
            {
                reactorChannel.state(State.DOWN);
                sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryRequest",
                        ReactorChannelEventTypes.CHANNEL_DOWN,
                        reactorChannel, errorInfo);
            }
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDictionaryRequest",
                    "Channel.write failed to write dictionary request: <"
                    + CodecReturnCodes.toString(retval) + ">" + " error="
                    + errorInfo.error().text());
        }
        else
            reactorChannel.flushAgain(false);
    }

    private void encodeAndWriteDictionaryClose(DictionaryClose dictionaryClose, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        // get a buffer for the dictionary close
        Channel channel = reactorChannel.channel();

        if (channel == null || channel.state() != ChannelState.ACTIVE)
            return;

        int bufSize;
        if ((bufSize = getMaxFragmentSize(reactorChannel, errorInfo)) < 0)
        {
            return;
        }

        TransportBuffer msgBuf = channel.getBuffer(bufSize, false, errorInfo.error());

        if (msgBuf == null)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDictionaryClose",
                    "Failed to obtain a TransportBuffer, reason="
                    + errorInfo.error().text());
            return;
        }

        _eIter.clear();
        _eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

        int retval = dictionaryClose.encode(_eIter);
        if (retval != CodecReturnCodes.SUCCESS)
        {
            // set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo)
            reactorChannel.state(State.DOWN);
            sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryClose",
                    ReactorChannelEventTypes.CHANNEL_DOWN,
                    reactorChannel, errorInfo);
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDictionaryClose",
                    "Encoding of dictionary close failed: <"
                    + TransportReturnCodes.toString(retval) + ">");
            return;
        }

        // Checks the channel's protocol type to perform auto conversion for the JSON protocol
        if(reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
        {
            if( Objects.isNull(jsonConverter))
            {
                populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.submitChannel", "The JSON converter library has not been initialized properly.");
                return;
            }

            jsonDecodeMsg.clear();
            _dIter.clear();
            int ret = _dIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(), reactorChannel.minorVersion());

            ret = jsonDecodeMsg.decode(_dIter);

            if(ret == CodecReturnCodes.SUCCESS)
            {
                converterError.clear();
                rwfToJsonOptions.clear();
                rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

                if( jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults, converterError) != CodecReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());

                    return;
                }

                TransportBuffer buffer = reactorChannel.getBuffer(conversionResults.getLength(), false, errorInfo);

                if(Objects.isNull(buffer))
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to get a buffer for sending JSON message. Error text: " + errorInfo.error().text());

                    return;
                }

                getJsonMsgOptions.clear();
                getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
                getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false );

                if (jsonConverter.getJsonBuffer(buffer, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
                {
                    populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                            "Reactor.submitChannel", "Failed to get converted JSON message. Error text: " + converterError.getText());
                    return;
                }

                /* Release the original buffer */
                reactorChannel.releaseBuffer(msgBuf, errorInfo);

                msgBuf = buffer;
            }
        }

        if (_reactorOptions.xmlTracing()) {
            xmlString.setLength(0);
            xmlString
                    .append("\n<!-- Outgoing Reactor message -->\n")
                    .append("<!-- ").append(reactorChannel.selectableChannel().toString()).append(" -->\n")
                    .append("<!-- ").append(new java.util.Date()).append(" -->\n");
            xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null, xmlString, errorInfo.error());
            System.out.println(xmlString);
        }
        retval = channel.write(msgBuf, _writeArgs, errorInfo.error());

        // Aggregate number of bytes written
        if(_reactorOptions.writeStatSet() == true)
        {
            ((WriteArgsImpl)_writeArgsAggregator).bytesWritten(overflowSafeAggregate(_writeArgsAggregator.bytesWritten(), _writeArgs.bytesWritten()));
            ((WriteArgsImpl)_writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(_writeArgsAggregator.uncompressedBytesWritten(), _writeArgs.uncompressedBytesWritten()));
        }

        if (retval > TransportReturnCodes.SUCCESS)
        {
            sendFlushRequest(reactorChannel, "Reactor.encodeAndWriteDictionaryClose", errorInfo);
        }
        else if (retval < TransportReturnCodes.SUCCESS)
        {
            // write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
            // for user application.
            // also, set reactorChannel.state(State.DOWN) and notify the application (via reactorChannelCallback(CHANNEL_DOWN, errorInfo))
            if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
            {
                reactorChannel.state(State.DOWN_RECONNECTING);
                sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryClose",
                        ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                        reactorChannel, errorInfo);
            }
            else // server channel or no more retries
            {
                reactorChannel.state(State.DOWN);
                sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryClose",
                        ReactorChannelEventTypes.CHANNEL_DOWN,
                        reactorChannel, errorInfo);
            }
            populateErrorInfo(errorInfo,
                    ReactorReturnCodes.FAILURE,
                    "Reactor.encodeAndWriteDictionaryClose",
                    "Channel.write failed to write dictionary close: <"
                    + CodecReturnCodes.toString(retval) + ">" + " error="
                    + errorInfo.error().text());
        }
        else
            reactorChannel.flushAgain(false);
    }

    int processChannelMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg, TransportBuffer transportBuffer, ReactorErrorInfo errorInfo)
    {
        int retval = ReactorCallbackReturnCodes.SUCCESS;

        switch (msg.domainType())
        {
            case DomainTypes.LOGIN:
                retval = processLoginMessage(reactorChannel, dIter, msg, transportBuffer, errorInfo);
                break;
            case DomainTypes.SOURCE:
                retval = processDirectoryMessage(reactorChannel, dIter, msg, transportBuffer, errorInfo);
                break;
            case DomainTypes.DICTIONARY:
                retval = processDictionaryMessage(reactorChannel, dIter, msg, transportBuffer, errorInfo);
                break;
            default:
                retval = sendAndHandleDefaultMsgCallback("Reactor.processChannelMessage", reactorChannel, transportBuffer, msg, errorInfo);
                break;
        }

        return retval;
    }

    private int processLoginMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg, TransportBuffer transportBuffer, ReactorErrorInfo errorInfo)
    {
        int retval = ReactorCallbackReturnCodes.SUCCESS;
        LoginMsg loginMsg = null;

        _loginMsg.clear();
        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
                LoginRequest loginRequest = (LoginRequest)_loginMsg;
                loginRequest.rdmMsgType(LoginMsgType.REQUEST);
                loginRequest.decode(dIter, msg);
                loginMsg = _loginMsg;
                break;
            case MsgClasses.REFRESH:
                LoginRefresh loginRefresh = (LoginRefresh)_loginMsg;
                loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
                loginRefresh.decode(dIter, msg);
                loginMsg = _loginMsg;
                break;
            case MsgClasses.STATUS:
                LoginStatus loginStatus = (LoginStatus)_loginMsg;
                loginStatus.rdmMsgType(LoginMsgType.STATUS);
                loginStatus.decode(dIter, msg);
                loginMsg = _loginMsg;
                break;
            case MsgClasses.CLOSE:
                LoginClose loginClose = (LoginClose)_loginMsg;
                loginClose.rdmMsgType(LoginMsgType.CLOSE);
                loginClose.decode(dIter, msg);
                loginMsg = _loginMsg;
                break;
            case MsgClasses.GENERIC:
                if (!proceedLoginGenericMsg(reactorChannel, dIter, msg, errorInfo)) {
                    return ReactorCallbackReturnCodes.SUCCESS;
                }
                loginMsg = _loginMsg;
                break;
            case MsgClasses.POST:
            case MsgClasses.ACK:
                _loginMsg.rdmMsgType(LoginMsgType.UNKNOWN);
                loginMsg = null;
                break;
            default:
                break;
        }

        if (retval != ReactorCallbackReturnCodes.FAILURE) {
            retval = sendAndHandleLoginMsgCallback("Reactor.processLoginMessage", reactorChannel, transportBuffer, msg, loginMsg, errorInfo);

            if (retval == ReactorCallbackReturnCodes.RAISE)
                retval = sendAndHandleDefaultMsgCallback("Reactor.processLoginMessage", reactorChannel, transportBuffer, msg, errorInfo);

            if (retval == ReactorCallbackReturnCodes.SUCCESS) {
                ReactorRole reactorRole = reactorChannel.role();
                /*
                 * check if this is a reactorChannel's role is CONSUMER, a Login REFRESH, if the reactorChannel State is UP,
                 * and that the loginRefresh's state was OK.
                 * If all this is true, check if a directoryRequest is populated. If so, send the directoryRequest. if not, change the
                 * reactorChannel state to READY.
                 */
                if (reactorChannel.state() == State.UP
                    && reactorChannel.role().type() == ReactorRoleTypes.CONSUMER
                    && msg.streamId() == ((ConsumerRole) reactorRole).rdmLoginRequest().streamId()
                    && _loginMsg.rdmMsgType() == LoginMsgType.REFRESH
                    && ((LoginRefresh) _loginMsg).state().streamState() == StreamStates.OPEN
                    && ((LoginRefresh) _loginMsg).state().dataState() == DataStates.OK) {
                    DirectoryRequest directoryRequest = ((ConsumerRole) reactorRole).rdmDirectoryRequest();
                    if (directoryRequest != null) {
                        // a rdmDirectoryRequest was specified, send it out.
                        encodeAndWriteDirectoryRequest(directoryRequest, reactorChannel, errorInfo);
                    } else {
                        // no rdmDirectoryRequest defined, so just send CHANNEL_READY
                        reactorChannel.state(State.READY);
                        if ((retval = sendAndHandleChannelEventCallback("Reactor.processLoginMessage",
                                ReactorChannelEventTypes.CHANNEL_READY,
                                reactorChannel, errorInfo)) != ReactorCallbackReturnCodes.SUCCESS) {
                            return retval;
                        }
                    }
                }

                /*
                 * check if this is a reactorChannel's role is NIPROVIDER, a Login REFRESH, if the reactorChannel State is UP,
                 * and that the loginRefresh's state was OK.
                 * If all this is true, check if a directoryRefresh is populated. If so, send the directoryRefresh. if not, change the
                 * reactorChannel state to READY.
                 */
                if (reactorChannel.state() == State.UP
                    && reactorChannel.role().type() == ReactorRoleTypes.NIPROVIDER
                    && msg.streamId() == ((NIProviderRole) reactorRole).rdmLoginRequest().streamId()
                    && _loginMsg.rdmMsgType() == LoginMsgType.REFRESH
                    && ((LoginRefresh) _loginMsg).state().streamState() == StreamStates.OPEN
                    && ((LoginRefresh) _loginMsg).state().dataState() == DataStates.OK) {

                    DirectoryRefresh directoryRefresh = ((NIProviderRole) reactorRole).rdmDirectoryRefresh();
                    if (directoryRefresh != null) {
                        // a rdmDirectoryRefresh was specified, send it out.
                        encodeAndWriteDirectoryRefresh(directoryRefresh, reactorChannel, errorInfo);
                    }

                    // send CHANNEL_READY
                    reactorChannel.state(State.READY);
                    if ((retval = sendAndHandleChannelEventCallback("Reactor.processLoginMessage",
                            ReactorChannelEventTypes.CHANNEL_READY,
                            reactorChannel, errorInfo)) != ReactorCallbackReturnCodes.SUCCESS) {
                        return retval;
                    }
                }
            }
        }

        return retval;
    }

    private int processDirectoryMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg, TransportBuffer transportBuffer, ReactorErrorInfo errorInfo)
    {
        int retval = ReactorCallbackReturnCodes.SUCCESS;

        _directoryMsg.clear();
        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
                DirectoryRequest directoryRequest = (DirectoryRequest)_directoryMsg;
                directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
                directoryRequest.decode(dIter, msg);
                break;
            case MsgClasses.REFRESH:
                DirectoryRefresh directoryRefresh = (DirectoryRefresh)_directoryMsg;
                directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
                directoryRefresh.decode(dIter, msg);
                break;
            case MsgClasses.STATUS:
                DirectoryStatus directoryStatus = (DirectoryStatus)_directoryMsg;
                directoryStatus.rdmMsgType(DirectoryMsgType.STATUS);
                directoryStatus.decode(dIter, msg);
                break;
            case MsgClasses.CLOSE:
                DirectoryClose directoryClose = (DirectoryClose)_directoryMsg;
                directoryClose.rdmMsgType(DirectoryMsgType.CLOSE);
                directoryClose.decode(dIter, msg);
                break;
            case MsgClasses.GENERIC:
                DirectoryConsumerStatus directoryCS = (DirectoryConsumerStatus)_directoryMsg;
                directoryCS.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);
                directoryCS.decode(dIter, msg);
                break;
            case MsgClasses.UPDATE:
                DirectoryUpdate directoryUpdate = (DirectoryUpdate)_directoryMsg;
                directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
                directoryUpdate.decode(dIter, msg);
                break;
            default:
                break;
        }

        retval = sendAndHandleDirectoryMsgCallback("Reactor.processDirectoryMessage", reactorChannel, transportBuffer, msg, _directoryMsg, errorInfo);

        if (retval == ReactorCallbackReturnCodes.RAISE)
            retval = sendAndHandleDefaultMsgCallback("Reactor.processDirectoryMessage", reactorChannel, transportBuffer, msg, errorInfo);

        if (retval == ReactorCallbackReturnCodes.SUCCESS)
        {
            /*
             * check if this is a reactorChannel's role is CONSUMER, a Directory REFRESH,
             * and if the reactorChannel State is UP.
             * If all this is true, check dictionaryDownloadMode is FIRST_AVAILABLE.
             */
            ReactorRole reactorRole = reactorChannel.role();
            if (reactorChannel.state() == State.UP
                && msg.streamId() == ((ConsumerRole)reactorRole).rdmDirectoryRequest().streamId()
                && reactorChannel.role().type() == ReactorRoleTypes.CONSUMER
                && _directoryMsg.rdmMsgType() == DirectoryMsgType.REFRESH)
            {

                if (((ConsumerRole)reactorRole).dictionaryDownloadMode() == DictionaryDownloadModes.FIRST_AVAILABLE)
                {
                    DirectoryRefresh directoryRefresh = (DirectoryRefresh)_directoryMsg;
                    int serviceId = 0;
                    boolean hasFieldDictionary = false;
                    boolean hasEnumTypeDictionary = false;
                    // find first directory message service that has RWFFld and RWFEnum available
                    for (Service service : directoryRefresh.serviceList())
                    {
                        if (service.checkHasInfo())
                        {
                            for (String dictionaryName : service.info().dictionariesProvidedList())
                            {
                                if (dictionaryName.equals(((ConsumerRole)reactorRole).fieldDictionaryName().toString()))
                                    hasFieldDictionary = true;

                                if (dictionaryName.equals(((ConsumerRole)reactorRole).enumTypeDictionaryName().toString()))
                                    hasEnumTypeDictionary = true;

                                if (hasFieldDictionary && hasEnumTypeDictionary)
                                {
                                    serviceId = service.serviceId();
                                    break;
                                }
                            }
                        }

                        // send field and enum type dictionary requests
                        if (hasFieldDictionary && hasEnumTypeDictionary)
                        {
                            DictionaryRequest dictionaryRequest;

                            ((ConsumerRole)reactorRole).initDefaultRDMFieldDictionaryRequest();
                            dictionaryRequest = ((ConsumerRole)reactorRole).rdmFieldDictionaryRequest();
                            dictionaryRequest.serviceId(serviceId);
                            encodeAndWriteDictionaryRequest(dictionaryRequest, reactorChannel, errorInfo);

                            ((ConsumerRole)reactorRole).initDefaultRDMEnumDictionaryRequest();
                            dictionaryRequest = ((ConsumerRole)reactorRole).rdmEnumDictionaryRequest();
                            dictionaryRequest.serviceId(serviceId);
                            encodeAndWriteDictionaryRequest(dictionaryRequest, reactorChannel, errorInfo);

                            break;
                        }
                    }

                    // check if dictionary download not supported by the provider
                    if (!hasFieldDictionary || !hasEnumTypeDictionary)
                    {
                        System.out.println("Dictionary download not supported by the indicated provider");
                    }
                }
                else
                {
                    // dictionaryDownloadMode is NONE, so just send CHANNEL_READY
                    reactorChannel.state(State.READY);
                    if ((retval = sendAndHandleChannelEventCallback("Reactor.processDirectoryMessage",
                            ReactorChannelEventTypes.CHANNEL_READY,
                            reactorChannel, errorInfo)) != ReactorCallbackReturnCodes.SUCCESS)
                    {
                        return retval;
                    }
                }
            }
        }

        return retval;
    }

    private int processDictionaryMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg, TransportBuffer transportBuffer, ReactorErrorInfo errorInfo)
    {
        int retval = ReactorCallbackReturnCodes.SUCCESS;
        DictionaryRefresh dictionaryRefresh = null;

        _dictionaryMsg.clear();
        switch (msg.msgClass())
        {
            case MsgClasses.REQUEST:
                DictionaryRequest dictionaryRequest = (DictionaryRequest)_dictionaryMsg;
                dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
                dictionaryRequest.decode(dIter, msg);
                break;
            case MsgClasses.REFRESH:
                dictionaryRefresh = (DictionaryRefresh)_dictionaryMsg;
                dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
                dictionaryRefresh.decode(dIter, msg);
                break;
            case MsgClasses.STATUS:
                DictionaryStatus dictionaryStatus = (DictionaryStatus)_dictionaryMsg;
                dictionaryStatus.rdmMsgType(DictionaryMsgType.STATUS);
                dictionaryStatus.decode(dIter, msg);
                break;
            case MsgClasses.CLOSE:
                DictionaryClose dictionaryClose = (DictionaryClose)_dictionaryMsg;
                dictionaryClose.rdmMsgType(DictionaryMsgType.CLOSE);
                dictionaryClose.decode(dIter, msg);
                break;
            default:
                break;
        }

        retval = sendAndHandleDictionaryMsgCallback("Reactor.processDictionaryMessage", reactorChannel, transportBuffer, msg, _dictionaryMsg, errorInfo);

        if (retval == ReactorCallbackReturnCodes.RAISE)
            retval = sendAndHandleDefaultMsgCallback("Reactor.processDictionaryMessage", reactorChannel, transportBuffer, msg, errorInfo);

        if (retval == ReactorCallbackReturnCodes.SUCCESS)
        {
            /*
             * check if this is a reactorChannel's role is CONSUMER, a Dictionary REFRESH,
             * reactorChannel State is UP, and dictionaryDownloadMode is FIRST_AVAILABLE.
             * If all this is true, close dictionary stream for this refresh.
             */
            ReactorRole reactorRole = reactorChannel.role();
            if (reactorChannel.state() == State.UP
                && reactorChannel.role().type() == ReactorRoleTypes.CONSUMER
                && _dictionaryMsg.rdmMsgType() == DictionaryMsgType.REFRESH
                && ((ConsumerRole)reactorRole).dictionaryDownloadMode() == DictionaryDownloadModes.FIRST_AVAILABLE)
            {
                // field dictionary
                if (msg.streamId() == ((ConsumerRole)reactorRole).rdmFieldDictionaryRequest().streamId() &&
                    dictionaryRefresh != null && dictionaryRefresh.checkRefreshComplete())
                {
                    /* Close stream so its streamID is free for the user. When connecting to an ADS,
                     * there won't be any further messages on this stream -- the consumer will be
                     * disconnected if the dictionary version is changed. */
                    ((ConsumerRole)reactorRole).receivedFieldDictionaryResp(true);

                    encodeAndWriteDictionaryClose(((ConsumerRole)reactorRole).rdmFieldDictionaryClose(), reactorChannel, errorInfo);
                }

                // enum type dictionary
                if (msg.streamId() == ((ConsumerRole)reactorRole).rdmEnumDictionaryRequest().streamId() &&
                    dictionaryRefresh != null && dictionaryRefresh.checkRefreshComplete())
                {
                    /* Close stream so its streamID is free for the user.  When connecting to an ADS,  there won't be any further messages on this stream --
                     * the consumer will be disconnected if the dictionary version is changed. */
                    ((ConsumerRole)reactorRole).receivedEnumDictionaryResp(true);

                    encodeAndWriteDictionaryClose(((ConsumerRole)reactorRole).rdmEnumDictionaryClose(), reactorChannel, errorInfo);
                }

                // if both field and enum type refreshes received, send CHANNEL_READY
                if (((ConsumerRole)reactorRole).receivedFieldDictionaryResp() &&
                    ((ConsumerRole)reactorRole).receivedEnumDictionaryResp())
                {
                    reactorChannel.state(State.READY);
                    if ((retval = sendAndHandleChannelEventCallback("Reactor.processDictionaryMessage",
                            ReactorChannelEventTypes.CHANNEL_READY,
                            reactorChannel, errorInfo)) != ReactorCallbackReturnCodes.SUCCESS)
                    {
                        return retval;
                    }
                }
            }
        }

        return retval;
    }

    /**
     * Process all channels' events and messages from the Reactor. These are
     * passed to the calling application via the the callback methods associated
     * with each of the channels.
     *
     * If keySet parameter is non-null, events and messages are processed from
     * the channels associated with keys in the selected key set. Once processed,
     * keys are removed from the selected key set.
     *
     * If keySet parameter is null, events and messages are processed from
     * all channels in a round-robin manner.
     *
     * @param keySet key set from the selector's registered channels
     * @param dispatchOptions options for how to dispatch
     * @param errorInfo error structure to be populated in the event of failure
     *
     * @return a positive value if dispatching succeeded and there are more messages to process or
     * {@link ReactorReturnCodes#SUCCESS} if dispatching succeeded and there are no more messages to process or
     * {@link ReactorReturnCodes#FAILURE}, if dispatching failed (refer to errorInfo for additional information)
     */
    public int dispatchAll(Set<SelectionKey> keySet, ReactorDispatchOptions dispatchOptions, ReactorErrorInfo errorInfo)
    {
        int maxMessages = dispatchOptions.maxMessages();
        int msgCount = 0;
        int retval = ReactorReturnCodes.SUCCESS;

        _reactorLock.lock();

        try
        {
            if (!_reactorActive)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.dispatchAll", "Reactor is not active, aborting.");
            }

            // handle Reactor's channel before individual channels
            while (msgCount < maxMessages && _workerQueue.readQueueSize() > 0)
            {
                msgCount++;
                if ((retval = processWorkerEvent(errorInfo)) < ReactorReturnCodes.SUCCESS)
                    return retval;
            }

            // remove Reactor's channel key(s) from keySet
            if (keySet != null)
            {
                Iterator<SelectionKey> iter = keySet.iterator();
                while (iter.hasNext())
                {
                    SelectionKey key = iter.next();
                    if (key.isReadable() && _reactorChannel == (ReactorChannel)key.attachment())
                    {
                        iter.remove();
                    }
                }
            }

            // handle other channels
            if (msgCount < maxMessages) // maxMessages not reached
            {
                if (keySet != null) // keySet available
                {
                    Iterator<SelectionKey> iter = keySet.iterator();
                    while (iter.hasNext())
                    {
                        retval = 1;
                        SelectionKey key = iter.next();
                        iter.remove();
                        try
                        {
                            if (key.isReadable())
                            {
                                // retrieve associated reactor channel and read on that channel
                                ReactorChannel reactorChnl = (ReactorChannel)key.attachment();

                                if (!isReactorChannelReady(reactorChnl))
                                {
                                    return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                                            "Reactor.dispatchAll", "ReactorChannel is not active, aborting.");
                                }

                                while (isReactorChannelReady(reactorChnl) && msgCount < maxMessages && retval > 0)
                                {
                                    msgCount++;
                                    if ((retval = performChannelRead(reactorChnl, dispatchOptions.readArgs(),
                                            errorInfo)) < ReactorReturnCodes.SUCCESS)
                                    {
                                        if (reactorChnl.state() != ReactorChannel.State.CLOSED &&
                                            reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
                                        {
                                            return retval;
                                        }
                                        else
                                        {
                                            // return success since close or reconnecting is not an error
                                            retval = ReactorReturnCodes.SUCCESS;

                                        }
                                    }
                                }
                            }
                        }
                        catch (CancelledKeyException e)
                        {
                        } // key can be canceled during shutdown

                        if (msgCount == maxMessages)
                        {
                            // update retval
                            retval = keySet.size() + retval;
                            break;
                        }
                    }
                }
                else // no keySet, round robin through all channels
                {
                    _reactorChannelCount = 0;

                    for(ReactorChannel reactorChnl = _reactorChannelQueue.start(ReactorChannel.REACTOR_CHANNEL_LINK);
                        reactorChnl != null;
                        reactorChnl = _reactorChannelQueue.forth(ReactorChannel.REACTOR_CHANNEL_LINK))
                    {
                        retval = 1;
                        _reactorChannelCount++;

                        if (!isReactorChannelReady(reactorChnl))
                        {
                            continue;
                        }

                        while (isReactorChannelReady(reactorChnl) && msgCount < maxMessages && retval > 0)
                        {
                            int bytesReadBefore = dispatchOptions.readArgs().uncompressedBytesRead();
                            if ((retval = performChannelRead(reactorChnl, dispatchOptions.readArgs(),
                                    errorInfo)) < ReactorReturnCodes.SUCCESS)
                            {
                                if (reactorChnl.state() != ReactorChannel.State.CLOSED &&
                                    reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
                                {
                                    return retval;
                                }
                                else
                                {
                                    // return success since close or reconnecting is not an error
                                    retval = ReactorReturnCodes.SUCCESS;

                                }
                            }
                            // only increment msgCount if bytes are actually read
                            if ((dispatchOptions.readArgs().uncompressedBytesRead() - bytesReadBefore) > 0)
                            {
                                msgCount++;
                            }
                        }

                        if (msgCount == maxMessages || _reactorChannelCount == _reactorChannelQueue.count())
                        {
                            // update retval
                            retval = (_reactorChannelCount < _reactorChannelQueue.count() ? 1 : 0) + retval;
                            break;
                        }
                    }
                }
            }
            else // maxMessages reached
            {
                // update retval
                retval = _workerQueue.readQueueSize() +
                         (keySet != null ? keySet.size() : (_reactorChannelCount < _reactorChannelQueue.count() ? 1 : 0));
            }
        }
        finally
        {
            _reactorLock.unlock();
        }

        return retval;
    }

    /**
     * Initializes this Reactor to be able to convert messages to and from RWF and JSON protocol.
     *
     * @param jsonConverterOptions specifies the options in {@link ReactorJsonConverterOptions} to initialize the converter library.
     * @param errorInfo specifies the {@link ReactorErrorInfo} to be populated in the event of an error.
     *
     * @return {@link ReactorReturnCodes#SUCCESS} if initialization succeeded; failure codes otherwise.
     *
     * @see ReactorJsonConverterOptions
     */
    public int initJsonConverter(ReactorJsonConverterOptions jsonConverterOptions, ReactorErrorInfo errorInfo)
    {
        JsonConverterError converterError = ConverterFactory.createJsonConverterError();

        _reactorLock.lock();

        try
        {
            if (!_reactorActive)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.initJsonConverter", "Reactor is not active, aborting.");
            }

            if(Objects.nonNull(jsonConverter))
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.initJsonConverter", "JsonConverter library is already initialized.");
            }

            if(jsonConverterOptions.defaultServiceId() > 65535)
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.initJsonConverter", "The service ID must be in a range between 0 to 65535.");
            }

            JsonFactory.initPools(3);
            JsonConverterBuilder jsonConverterBuilder = ConverterFactory.createJsonConverterBuilder();

            if(Objects.isNull(serviceNameIdConverterClient))
                serviceNameIdConverterClient = new ServiceNameIdConverterClient(this);

            jsonConverterBuilder.setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
                    .setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS, jsonConverterOptions.jsonExpandedEnumFields())
                    .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS, jsonConverterOptions.catchUnknownJsonKeys())
                    .setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_FIDS, jsonConverterOptions.catchUnknownJsonFids())
                    .setProperty(JsonConverterProperties.JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS, true) /* Always enable this feature */
                    .setServiceConverter(serviceNameIdConverterClient)
                    .setDictionary(jsonConverterOptions.dataDictionary());

            /* Sets the default service ID if specified by users. */
            if(jsonConverterOptions.defaultServiceId() >= 0)
            {
                jsonConverterBuilder.setProperty(JsonConverterProperties.JSON_CPC_DEFAULT_SERVICE_ID, jsonConverterOptions.defaultServiceId());
            }

            jsonConverter = jsonConverterBuilder.build(converterError);

            if(Objects.isNull(jsonConverter))
            {
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                        "Reactor.initJsonConverter", converterError.getText());
            }

            jsonConverterUserSpec = jsonConverterOptions.userSpec();
            serviceNameToIdCallback = jsonConverterOptions.serviceNameToIdCallback();
            JsonConversionEventCallback = jsonConverterOptions.jsonConversionEventCallback();
            closeChannelFromFailure = jsonConverterOptions.closeChannelFromFailure();
        }
        finally
        {
            _reactorLock.unlock();
        }

        return ReactorReturnCodes.SUCCESS;
    }

    int closeChannel(ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        _reactorLock.lock();

        try
        {
            if (errorInfo == null)
                return ReactorReturnCodes.FAILURE;
            else if (reactorChannel == null)
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.closeChannel",
                        "reactorChannel cannot be null");
            else if (_reactorActive == false)
                return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.closeChannel",
                        "Reactor is shutdown, closeChannel ignored");
            else if (reactorChannel.state() == State.CLOSED)
                return ReactorReturnCodes.SUCCESS;

            // set the ReactorChannel's state to CLOSED.
            // and remove it from the queue.
            reactorChannel.state(State.CLOSED);
            _reactorChannelQueue.remove(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);

            // send CHANNEL_CLOSED WorkerEvent to Worker.
            if (!sendWorkerEvent(WorkerEventTypes.CHANNEL_CLOSE, reactorChannel))
            {
                // sendWorkerEvent() failed, send channel down
                reactorChannel.state(State.DOWN);
                sendAndHandleChannelEventCallback("Reactor.closeChannel",
                        ReactorChannelEventTypes.CHANNEL_DOWN,
                        reactorChannel, errorInfo);
                return populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        "Reactor.closeChannel",
                        "sendWorkerEvent() failed");
            }
        }
        finally
        {
            _reactorLock.unlock();
        }

        return ReactorReturnCodes.SUCCESS;
    }

    // estimate encoded message size
    // used by submit(msg)
    private int encodedMsgSize(Msg msg)
    {
        int msgSize = 128;
        MsgKey key = msg.msgKey();

        msgSize += msg.encodedDataBody().length();

        if (key != null)
        {
            if (key.checkHasName())
                msgSize += key.name().length();

            if (key.checkHasAttrib())
                msgSize += key.encodedAttrib().length();
        }

        return msgSize;
    }

    private void printTunnelStreamStateInfo(TunnelStream tunnelStream)
    {
        int ret;

        /* Print information about the current tunnel stream. */
        if ((ret = tunnelStream.getStateInfo(_tunnelStreamStateInfo)) != ReactorReturnCodes.SUCCESS)
        {
            System.out.println("TunnelStreamInt.getInfo() failed: " + CodecReturnCodes.toString(ret));
            return;
        }

        System.out.println("TunnelStreamStateInfo:\n"
                           + "                  Stream State: " + _tunnelStreamStateInfo.streamState().toString() + "\n"
                           + "   Outbound Untransmitted Msgs: " + _tunnelStreamStateInfo.outboundMsgsQueued() + "\n"
                           + "         Outbound Unacked Msgs: " + _tunnelStreamStateInfo.outboundMsgsWaitingForAck() + "\n"
                           + "   Outbound/Inbound Bytes Open: " + _tunnelStreamStateInfo.outboundBytesOpen() + "/" + _tunnelStreamStateInfo.inboundBytesOpen() + "\n");
    }

    private int sendTunnelStreamLogin(TunnelStream tunnelStream, ReactorErrorInfo errorInfo)
    {
        LoginRequest loginRequest = tunnelStream.authLoginRequest();

        TransportBuffer msgBuf = tunnelStream.getBuffer(getMaxFragmentSize(tunnelStream), errorInfo);
        if (msgBuf == null)
        {
            populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
                    "Reactor.sendTunnelStreamLogin",
                    "Failed to obtain a TransportBuffer, reason="
                    + errorInfo.error().text());
            return ReactorReturnCodes.FAILURE;
        }

        _eIter.clear();
        _eIter.setBufferAndRWFVersion(msgBuf, tunnelStream.reactorChannel().majorVersion(), tunnelStream.reactorChannel().minorVersion());

        int retval = loginRequest.encode(_eIter);
        if (retval != CodecReturnCodes.SUCCESS)
        {
            populateErrorInfo(errorInfo,
                    retval,
                    "Reactor.sendTunnelStreamLogin",
                    "Encoding of login request failed: <"
                    + TransportReturnCodes.toString(retval) + ">");
            return ReactorReturnCodes.FAILURE;
        }

        _tunnelStreamSubmitOptions.clear();
        _tunnelStreamSubmitOptions.containerType(DataTypes.MSG);
        retval = tunnelStream.submit(msgBuf, _tunnelStreamSubmitOptions, errorInfo);
        if (retval != CodecReturnCodes.SUCCESS)
        {
            populateErrorInfo(errorInfo,
                    retval,
                    "Reactor.sendTunnelStreamLogin",
                    "Submit of login request failed: <"
                    + TransportReturnCodes.toString(retval) + ">");
            return ReactorReturnCodes.FAILURE;
        }

        return ReactorReturnCodes.SUCCESS;
    }

    int getMaxFragmentSize(ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
    {
        int ret;
        _reactorChannelInfo.clear();
        if ((ret = reactorChannel.info(_reactorChannelInfo, errorInfo)) < ReactorReturnCodes.SUCCESS)
        {
            return ret;
        }
        return _reactorChannelInfo.channelInfo().maxFragmentSize();
    }

    int getMaxFragmentSize(TunnelStream tunnelStream)
    {
        return tunnelStream.classOfService().common().maxFragmentSize();
    }

    /* Process a message received for a known TunnelStream. */
    private int handleTunnelStreamMsg(ReactorChannel reactorChannel, TunnelStream tunnelStream, TransportBuffer msgBuf, Msg msg, ReactorErrorInfo errorInfo)
    {
        int retval;

        // dispatch anything for tunnel stream
        while ((retval = reactorChannel.tunnelStreamManager().dispatch(errorInfo.error())) > ReactorReturnCodes.SUCCESS);
        if (retval < ReactorReturnCodes.SUCCESS)
        {
            // send channel down event if channel error occurred
            if (retval == ReactorReturnCodes.CHANNEL_ERROR)
            {
                if (reactorChannel.state() != State.DOWN && reactorChannel.state() != State.DOWN_RECONNECTING)
                {
                    if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
                    {
                        reactorChannel.state(State.DOWN_RECONNECTING);
                        sendAndHandleChannelEventCallback("Reactor.performChannelRead",
                                ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                                reactorChannel, errorInfo);
                    }
                    else // server channel or no more retries
                    {
                        reactorChannel.state(State.DOWN);
                        sendAndHandleChannelEventCallback("Reactor.performChannelRead",
                                ReactorChannelEventTypes.CHANNEL_DOWN,
                                reactorChannel, errorInfo);
                    }
                }
                // return SUCCESS on dispatch() for CHANNEL_ERROR, application should close ReactorChannel
                retval = ReactorReturnCodes.SUCCESS;
            }
            else
            {
                return populateErrorInfo(errorInfo, retval,
                        "Reactor.performChannelRead",
                        "TunnelStream dispatch failed - " + errorInfo.error().text());
            }
        }

        if (msg.msgClass() == MsgClasses.CLOSE)
        {
            // forward status to tunnel stream event callback
            _tmpState.clear();
            _tmpState.streamState(StreamStates.CLOSED);
            _tmpState.dataState(DataStates.SUSPECT);

            sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel, tunnelStream, msgBuf, msg, _tmpState, errorInfo);
        }

        // set flag for TunnelStream read
        // status and close messages are never read by TunnelStream
        else if (msg.msgClass() != MsgClasses.STATUS)
        {
            // read anything for TunnelStream
            if ((retval = reactorChannel.tunnelStreamManager().readMsg(tunnelStream, _msg, errorInfo.error())) < ReactorReturnCodes.SUCCESS)
            {
                // send channel down event if channel error occurred
                if (retval == ReactorReturnCodes.CHANNEL_ERROR)
                {
                    if (reactorChannel.state() != State.DOWN && reactorChannel.state() != State.DOWN_RECONNECTING)
                    {
                        if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
                        {
                            reactorChannel.state(State.DOWN_RECONNECTING);
                            sendAndHandleChannelEventCallback("Reactor.performChannelRead",
                                    ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
                                    reactorChannel, errorInfo);
                        }
                        else // server channel or no more retries
                        {
                            reactorChannel.state(State.DOWN);
                            sendAndHandleChannelEventCallback("Reactor.performChannelRead",
                                    ReactorChannelEventTypes.CHANNEL_DOWN,
                                    reactorChannel, errorInfo);
                        }
                    }
                    // return SUCCESS on dispatch() for CHANNEL_ERROR, application should close ReactorChannel
                    retval = ReactorReturnCodes.SUCCESS;
                }
                else
                {
                    return populateErrorInfo(errorInfo, retval,
                            "Reactor.performChannelRead",
                            "TunnelStream readMsg failed - " + errorInfo.error().text());
                }
            }

            // handle refresh message 
            if (msg.msgClass() == MsgClasses.REFRESH)
            {
                RefreshMsg refreshMsg = (RefreshMsg)msg;

                if (reactorChannel.watchlist() != null)
                {
                    WlRequest wlRequest;

                    _tempWlInteger.value(refreshMsg.streamId());
                    if ((wlRequest = reactorChannel.watchlist().streamIdtoWlRequestTable().get(_tempWlInteger))
                        == null)
                    {
                        return populateErrorInfo(errorInfo, retval,
                                "Reactor.performChannelRead",
                                "Internal Error: TunnelStream watchlist request entry not found.");
                    }

                    if (wlRequest.stream() == null)
                    {
                        return populateErrorInfo(errorInfo, retval,
                                "Reactor.performChannelRead",
                                "Internal Error: TunnelStream watchlist stream entry not found.");
                    }

                    tunnelStream.channelStreamId(wlRequest.stream().streamId());
                }
                else
                    tunnelStream.channelStreamId(refreshMsg.streamId());

                if (refreshMsg.containerType() == DataTypes.FILTER_LIST &&
                    refreshMsg.msgKey().checkHasFilter())
                {
                    int ret;
                    if ((ret = tunnelStream.classOfService().decode(_reactorChannel, refreshMsg.encodedDataBody(), errorInfo)) != ReactorReturnCodes.SUCCESS)
                    {
                        _tmpState.clear();
                        _tmpState.streamState(StreamStates.CLOSED);
                        _tmpState.dataState(DataStates.SUSPECT);
                        _tmpState.text().data("Class of service decode failed with return code: " + ret + " <" + errorInfo.error().text() + ">");

                        sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel, tunnelStream, msgBuf, msg, _tmpState, errorInfo);
                    }

                    /* If recvWindowSize was -1, set it to reflect actual default value. */
                    /* If recvWindowSize was less than received maxFragmentSize, set it to received maxFragmentSize. */
                    if (tunnelStream.classOfService().flowControl().recvWindowSize() == -1)
                        tunnelStream.classOfService().flowControl().recvWindowSize(TunnelStream.DEFAULT_RECV_WINDOW);
                    if (tunnelStream.classOfService().flowControl().recvWindowSize() < tunnelStream.classOfService().common().maxFragmentSize())
                        tunnelStream.classOfService().flowControl().recvWindowSize(tunnelStream.classOfService().common().maxFragmentSize());
                    /* If sendWindowSize was -1, set it to reflect actual default value. */
                    /* If sendWindowSize was less than received maxFragmentSize, set it to received maxFragmentSize. */
                    if (tunnelStream.classOfService().flowControl().sendWindowSize() == -1)
                        tunnelStream.classOfService().flowControl().sendWindowSize(TunnelStream.DEFAULT_RECV_WINDOW);
                    if (tunnelStream.classOfService().flowControl().sendWindowSize() < tunnelStream.classOfService().common().maxFragmentSize())
                        tunnelStream.classOfService().flowControl().sendWindowSize(tunnelStream.classOfService().common().maxFragmentSize());

                    if (!tunnelStream.isProvider())
                    {
                        // do the _bufferPool here
                        tunnelStream.setupBufferPool();
                    }
                }
                else
                {
                    _tmpState.clear();
                    _tmpState.streamState(StreamStates.CLOSED);
                    _tmpState.dataState(DataStates.SUSPECT);
                    _tmpState.text().data("TunnelStream refresh must contain FILTER_LIST and have filter in message key");

                    sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel, tunnelStream, msgBuf, msg, _tmpState, errorInfo);
                }

                /* Check if received class of service stream version is less than or equal to current class
                 * of service stream version. If not, callback with closed state.
                 */
                CosCommon commonProperties = tunnelStream.classOfService().common();
                if (commonProperties.streamVersion() <= CosCommon.CURRENT_STREAM_VERSION)
                {
                    if (tunnelStream.classOfService().authentication().type() != ClassesOfService.AuthenticationTypes.OMM_LOGIN)
                    {
                        // no substream login, send refresh message as event callback
                        refreshMsg.state().copy(tunnelStream.state());
                        sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel, tunnelStream, msgBuf, refreshMsg, refreshMsg.state(), errorInfo);
                    }
                    else // substream login enabled
                    {
                        // send TunnelStream login here
                        if ((sendTunnelStreamLogin(tunnelStream, errorInfo)) < ReactorReturnCodes.SUCCESS)
                        {
                            _tmpState.clear();
                            _tmpState.streamState(StreamStates.CLOSED);
                            _tmpState.dataState(DataStates.SUSPECT);
                            _tmpState.text().data("sendTunnelStreamLogin() failed <" + errorInfo.error().text() + ">");

                            sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel, tunnelStream, msgBuf, msg, _tmpState, errorInfo);
                        }
                    }
                }
                else // class of service stream version doesn't match, callback with closed state
                {
                    _tmpState.clear();
                    _tmpState.streamState(StreamStates.CLOSED);
                    _tmpState.dataState(DataStates.SUSPECT);
                    _tmpState.text().data("Unsupported class of service stream version: " + commonProperties.streamVersion());

                    sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel, tunnelStream, msgBuf, msg, _tmpState, errorInfo);
                }
            }

            if (tunnelStream.traceFlags() > 0)
                printTunnelStreamStateInfo(tunnelStream);
        }
        else // status 
        {
            // forward status to tunnel stream event callback
            if (!tunnelStream.handleRequestRetry())
            {
                StatusMsg statusMsg = (StatusMsg)msg;

                if (statusMsg.checkHasState())
                {
                    statusMsg.state().copy(tunnelStream.state());
                }
                sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel, tunnelStream, msgBuf, statusMsg, statusMsg.state(), errorInfo);

                // always close tunnel stream handler for status message close
                if (statusMsg.state().streamState() == StreamStates.CLOSED || statusMsg.state().streamState() == StreamStates.CLOSED_RECOVER)
                    tunnelStream.close(_finalStatusEvent, errorInfo.error());
            }
        }

        if ((retval = reactorChannel.checkTunnelManagerEvents(errorInfo)) != ReactorReturnCodes.SUCCESS)
            return retval;

        return ReactorReturnCodes.SUCCESS;
    }

    /* Request that the Worker start flushing this channel.  */
    private int sendFlushRequest(ReactorChannel reactorChannel, String location, ReactorErrorInfo errorInfo)
    {
        if (reactorChannel.flushRequested())
            reactorChannel.flushAgain(true); /* Flush already in progress; wait till FLUSH_DONE is received, then request again. */
        else
        {
            if (!sendWorkerEvent(WorkerEventTypes.FLUSH, reactorChannel))
            {
                // sendWorkerEvent() failed, send channel down
                reactorChannel.state(State.DOWN);
                sendAndHandleChannelEventCallback(location,
                        ReactorChannelEventTypes.CHANNEL_DOWN,
                        reactorChannel, errorInfo);
                return populateErrorInfo(errorInfo,
                        ReactorReturnCodes.FAILURE,
                        location,
                        "sendWorkerEvent() failed while requesting flush");
            }

            reactorChannel.flushAgain(false);
            reactorChannel.flushRequested(true);
        }

        return ReactorReturnCodes.SUCCESS;
    }

    boolean isReactorChannelReady(ReactorChannel reactorChannel)
    {
        return reactorChannel.state() == ReactorChannel.State.UP ||
               reactorChannel.state() == ReactorChannel.State.READY;
    }

    int overflowSafeAggregate(int a, int b)
    {
        long sum = (long)a + (long)b;
        if (sum < Integer.MAX_VALUE)
            return (int)sum;
        else
            return Integer.MAX_VALUE;
    }
}
