/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.SelectionKey;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Objects;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.RejectedExecutionException;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Lock;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.CopyMsgFlags;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.MsgKeyFlags;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.RequestMsgFlags;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StatusMsgFlags;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.XmlTraceDump;
import com.refinitiv.eta.json.converter.ConversionResults;
import com.refinitiv.eta.json.converter.ConverterFactory;
import com.refinitiv.eta.json.converter.DecodeJsonMsgOptions;
import com.refinitiv.eta.json.converter.GetJsonErrorParams;
import com.refinitiv.eta.json.converter.GetJsonMsgOptions;
import com.refinitiv.eta.json.converter.JsonConverter;
import com.refinitiv.eta.json.converter.JsonConverterBuilder;
import com.refinitiv.eta.json.converter.JsonConverterError;
import com.refinitiv.eta.json.converter.JsonConverterErrorCodes;
import com.refinitiv.eta.json.converter.JsonConverterProperties;
import com.refinitiv.eta.json.converter.JsonMsg;
import com.refinitiv.eta.json.converter.JsonMsgClasses;
import com.refinitiv.eta.json.converter.JsonProtocol;
import com.refinitiv.eta.json.converter.ParseJsonOptions;
import com.refinitiv.eta.json.converter.RWFToJsonOptions;
import com.refinitiv.eta.json.util.JsonFactory;
import com.refinitiv.eta.rdm.ClassesOfService;
import com.refinitiv.eta.rdm.Dictionary;
import com.refinitiv.eta.rdm.Directory.WarmStandbyDirectoryServiceTypes;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.InitArgs;
import com.refinitiv.eta.transport.IoctlCodes;
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
import com.refinitiv.eta.valueadd.common.VaQueue;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.ConsumerStatusServiceFlags;
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
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service.ServiceFlags;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginClose;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginConsumerConnectionStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginConsumerConnectionStatusFlags;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequestFlags;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.queue.QueueMsg;
import com.refinitiv.eta.valueadd.reactor.ReactorAuthTokenInfo.TokenVersion;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel.State;
import com.refinitiv.eta.valueadd.reactor.ReactorTokenSession.SessionState;

/**
 * The Reactor. Applications create Reactor objects by calling
 * {@link ReactorFactory#createReactor(ReactorOptions, ReactorErrorInfo)},
 * create connections by calling
 * {@link Reactor#connect(ReactorConnectOptions, ReactorRole, ReactorErrorInfo)}/
 * {@link Reactor#accept(com.refinitiv.eta.transport.Server, ReactorAcceptOptions, ReactorRole, ReactorErrorInfo)}
 * and process events by calling
 * {@link ReactorChannel#dispatch(ReactorDispatchOptions, ReactorErrorInfo)}/
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
	FileDumper _fileDumper;

	// queue between reactor and worker
	SelectableBiDirectionalQueue _workerQueue = null;
	Worker _worker = null;
	ExecutorService _esWorker = null;

	// Queue to track ReactorChannels
	VaDoubleLinkList<ReactorChannel> _reactorChannelQueue = new VaDoubleLinkList<ReactorChannel>();

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
	CloseMsg _closeMsg = (CloseMsg) CodecFactory.createMsg();
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
	private String _defaultTokenURLV1String = "https://api.refinitiv.com/auth/oauth2/v1/token";
	private Buffer _tokenURLV1String = CodecFactory.createBuffer();
	private String _defaultTokenURLV2String = "https://api.refinitiv.com/auth/oauth2/v2/token";
	private Buffer _tokenURLV2String = CodecFactory.createBuffer();
	private String _defaultServiceDiscoveryString = "https://api.refinitiv.com/streaming/pricing/v1/";
	private Buffer _seviceDiscoveryString = CodecFactory.createBuffer();

	// This is used by the queryServiceDiscovery() method.
	private List<ReactorServiceEndpointInfo> _reactorServiceEndpointInfoList = new ArrayList<ReactorServiceEndpointInfo>(
			50);
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

	private boolean sendJsonConvError = false;
	private ParseJsonOptions parseJsonOptions = ConverterFactory.createParseJsonOptions();
	JsonConverterError converterError = ConverterFactory.createJsonConverterError();
	JsonConverterError getMessageError = ConverterFactory.createJsonConverterError();
	private DecodeJsonMsgOptions decodeJsonMsgOptions = ConverterFactory.createDecodeJsonMsgOptions();
	private JsonMsg jsonMsg = ConverterFactory.createJsonMsg();
	private Msg jsonDecodeMsg = CodecFactory.createMsg();
	private GetJsonErrorParams jsonErrorParams = ConverterFactory.createJsonErrorParams();
	private Buffer jsonErrorOutputBuffer = CodecFactory.createBuffer();

	RWFToJsonOptions rwfToJsonOptions = ConverterFactory.createRWFToJsonOptions();
	ConversionResults conversionResults = ConverterFactory.createConversionResults();
	GetJsonMsgOptions getJsonMsgOptions = ConverterFactory.createGetJsonMsgOptions();

	/*
	 * This is used by ReactorChannel for handling packed buffer of the JSON
	 * protocol.
	 */
	HashMap<TransportBuffer, ReactorPackedBuffer> packedBufferHashMap = new HashMap<>();

	/*
	 * This is used by ReactorChannel for handling the write call again state of the
	 * JSON protocol.
	 */
	HashMap<TransportBuffer, TransportBuffer> writeCallAgainMap = new HashMap<>();

	// TODO find out how to set this properly, where it's configured, when it's set,
	// etc
	ReactorWarmStandbyEventPool reactorWarmStandbyEventPool = new ReactorWarmStandbyEventPool(30);

	VaQueue warmstandbyChannelPool; // Pool of available ReactorWarmStandbyHandler structures
	VaQueue closingWarmStandbyChannel; // Keeps a list ReactorWarmStandbyHandler being closed

	static String JSON_PONG_MESSAGE = "{\"Type\":\"Pong\"}";

	ReactorDebugger debugger;

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
		} else if (options != null)
		{
			if (options.tokenReissueRatio() < 0.05 || options.tokenReissueRatio() > 0.95)
			{
				populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.constructor",
						"The token reissue ratio must be in between 0.05 to 0.95.");
				return;
			}

			if (options.reissueTokenAttemptInterval() < 0)
			{
				populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.constructor",
						"The token reissue attempt interval is less than zero.");
				return;
			}

			_reactorOptions.copy(options);
			debugger = ReactorFactory.createReactorDebugger(_reactorOptions.debuggerOptions().outputStream(),
					_reactorOptions.debuggerOptions().capacity());

			if (options.xmlTraceToFile()){
			_fileDumper = new FileDumper(options.xmlTraceFileName(), options.xmlTraceToMultipleFiles(), options.xmlTraceMaxFileSize());
		}

		} else
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
			return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.initializeTransport",
					errorInfo.error().text());
		}
		return ReactorReturnCodes.SUCCESS;
	}

	int initializeReactor(ReactorErrorInfo errorInfo)
	{
		try
		{
			// Initialize default URLs, if necessary
			if (_reactorOptions.serviceDiscoveryURL() == null || _reactorOptions.serviceDiscoveryURL().length() == 0)
			{
				_seviceDiscoveryString.data(_defaultServiceDiscoveryString);
				_reactorOptions.serviceDiscoveryURL(_seviceDiscoveryString);
			}

			if (_reactorOptions.tokenServiceURL_V1() == null || _reactorOptions.tokenServiceURL_V1().length() == 0)
			{
				_tokenURLV1String.data(_defaultTokenURLV1String);
				_reactorOptions.tokenServiceURL_V1(_tokenURLV1String);
			}

			if (_reactorOptions.tokenServiceURL_V2() == null || _reactorOptions.tokenServiceURL_V2().length() == 0)
			{
				_tokenURLV2String.data(_defaultTokenURLV2String);
				_reactorOptions.tokenServiceURL_V2(_tokenURLV2String);
			}

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
		} catch (RejectedExecutionException | NullPointerException e)
		{
			return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.initializeReactor",
					"failed to initialize the Worker, exception=" + e.getLocalizedMessage());
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
	 * Returns the userSpecObj that was specified in the ReactorOptions when this
	 * Reactor was created.
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
	 * sends ReactorChannelEvents to all active channels indicating that they are
	 * down. Once this call is made, the Reactor is destroyed and no further calls
	 * should be made with it.
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
			 * For all reactorChannels, send CHANNEL_DOWN to worker and application (via
			 * callback).
			 */
			for (ReactorChannel reactorChannel = _reactorChannelQueue.start(
					ReactorChannel.REACTOR_CHANNEL_LINK); reactorChannel != null; reactorChannel = _reactorChannelQueue
							.forth(ReactorChannel.REACTOR_CHANNEL_LINK))
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
			while (!_esWorker.awaitTermination(SHUTDOWN_TIMEOUT_IN_SECONDS, TimeUnit.SECONDS))
				;
			_esWorker = null;
			_worker = null;

			if (packedBufferHashMap.size() > 0)
			{
				for (ReactorPackedBuffer packedBuffer : packedBufferHashMap.values())
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

			// Releases all references for the JSON converter library.
			serviceNameToIdCallback = null;
			JsonConversionEventCallback = null;
			jsonConverter = null;
			jsonConverterUserSpec = null;
			serviceNameIdConverterClient = null;

			int tRetCode = Transport.uninitialize();
			if (tRetCode != TransportReturnCodes.SUCCESS)
				retval = ReactorReturnCodes.FAILURE;

		} catch (InterruptedException e)
		{
			return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.shutdown",
					"Exception occurred while waiting for Worker thread to terminate, exception="
							+ e.getLocalizedMessage());
		} finally
		{
			_reactorActive = false;
			_reactorLock.unlock();
		}

		return retval;
	}

	/**
	 * The Reactor's internal channel that's used to communicate with the worker
	 * thread.
	 *
	 * @return the Reactor's internal channel
	 */
	public ReactorChannel reactorChannel()
	{
		return _reactorChannel;
	}

	/**
	 * Adds a server-side channel to the Reactor. Once the channel is initialized,
	 * the channelEventCallback will receive an event indicating that the channel is
	 * up.
	 *
	 * @param server               server that is accepting this connection (a
	 *                             server can be created with
	 *                             {@link com.refinitiv.eta.transport.Transport#bind(com.refinitiv.eta.transport.BindOptions, com.refinitiv.eta.transport.Error)})
	 * @param reactorAcceptOptions options for this connection
	 * @param role                 role of this connection
	 * @param errorInfo            error structure to be populated in the event of
	 *                             failure
	 *
	 * @return {@link ReactorReturnCodes} indicating success or failure
	 */
	public int accept(Server server, ReactorAcceptOptions reactorAcceptOptions, ReactorRole role,
			ReactorErrorInfo errorInfo)
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
				return populateErrorInfo(errorInfo, ReactorReturnCodes.SHUTDOWN, "Reactor.accept",
						"Reactor is not active, aborting.");
			} else if (reactorAcceptOptions == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.accept",
						"reactorAcceptOptions cannot be null, aborting.");
			} else if (role == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.accept",
						"role cannot be null, aborting.");
			} else if (role.channelEventCallback() == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.accept",
						"role must have a channelEventCallback defined, aborting.");
			} else if (role.defaultMsgCallback() == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.accept",
						"role must have a defaultMsgCallback defined, aborting.");
			} else if (role.type() != ReactorRoleTypes.PROVIDER)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.accept",
						"role must be Provider Role, aborting.");
			}

			if (reactorAcceptOptions.initTimeout() < 1)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.accept",
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

			// enable channel read/write locking for reactor since it's multi-threaded with
			// worker thread
			reactorAcceptOptions.acceptOptions().channelReadLocking(true);
			reactorAcceptOptions.acceptOptions().channelWriteLocking(true);

			// call Server.accept to accept a new Channel
			Channel channel = server.accept(reactorAcceptOptions.acceptOptions(), errorInfo.error());
			if (channel != null)
			{
				reactorChannel.selectableChannelFromChannel(channel);
				reactorChannel.userSpecObj(reactorAcceptOptions.acceptOptions().userSpecObject());
			} else
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.accept",
						"Server.accept() failed, error=" + errorInfo.error().text());
			}

			// send a WorkerEvent to the Worker to initialize this channel.
			if (!sendWorkerEvent(WorkerEventTypes.CHANNEL_INIT, reactorChannel))
			{
				// sendWorkerEvent() failed, send channel down
				reactorChannel.state(State.DOWN);
				sendAndHandleChannelEventCallback("Reactor.accept", ReactorChannelEventTypes.CHANNEL_DOWN,
						reactorChannel, errorInfo);
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.accept",
						"sendWorkerEvent() failed");
			}

			/* Set additional accept options for WebSocket connections */
			reactorChannel.sendPingMessage(reactorAcceptOptions.websocketAcceptOptions().sendPingMessage);
			if (_reactorOptions.debuggerOptions().debugConnectionLevel())
			{
				debugger.writeDebugInfo(ReactorDebugger.CONNECTION_SERVER_ACCEPT, this.hashCode(), server.hashCode(),
						reactorChannel.hashCode(), ReactorDebugger.getChannelId(reactorChannel));
			}
		} finally
		{
			_reactorLock.unlock();
		}

		return ReactorReturnCodes.SUCCESS;
	}

	/**
	 * Adds a client-side channel to the Reactor. Once the channel is initialized,
	 * the channelEventCallback will receive an event indicating that the channel is
	 * up.
	 *
	 * @param reactorConnectOptions options for this connection
	 * @param role                  role of this connection
	 * @param errorInfo             error structure to be populated in the event of
	 *                              failure
	 *
	 * @return {@link ReactorReturnCodes} indicating success or failure
	 */
	public int connect(ReactorConnectOptions reactorConnectOptions, ReactorRole role, ReactorErrorInfo errorInfo)
	{
		_reactorLock.lock();

		boolean sendAuthTokenEvent = false;
		ReactorTokenSession tokenSession = null;
		ReactorWarmStandbyHandler warmStandbyHandlerImpl = null;

		try
		{
			if (errorInfo == null)
			{
				System.out.println("Reactor.connect(): ReactorErrorInfo cannot be null, aborting.");
				return ReactorReturnCodes.FAILURE;
			}

			if (!_reactorActive)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.SHUTDOWN, "Reactor.connect",
						"Reactor is not active, aborting.");
			} else if (reactorConnectOptions == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
						"reactorConnectOptions cannot be null, aborting.");
			} else if (role == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
						"role cannot be null, aborting.");
			} else if (role.channelEventCallback() == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
						"role must have a channelEventCallback defined, aborting.");
			} else if (role.defaultMsgCallback() == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
						"role must have a defaultMsgCallback defined, aborting.");
			} else if (role.type() == ReactorRoleTypes.CONSUMER)
			{
				if (((ConsumerRole) role).rdmDirectoryRequest() != null
						&& ((ConsumerRole) role).rdmLoginRequest() == null)
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
							"Must specify an rdmLoginRequest if specifying an rdmDirectoryRequest, aborting.");
				}

				if (((ConsumerRole) role).dictionaryDownloadMode() == DictionaryDownloadModes.FIRST_AVAILABLE
						&& ((ConsumerRole) role).watchlistOptions().enableWatchlist())
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.connect",
							"Cannot specify a dictionary download when watchlist is enabled.");
				}

				if (((ConsumerRole) role).dictionaryDownloadMode() == DictionaryDownloadModes.FIRST_AVAILABLE
						&& ((ConsumerRole) role).rdmDirectoryRequest() == null)
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
							"Must specify an rdmDirectoryRequest if specifying a dictionary download, aborting.");
				}
			} else if (role.type() == ReactorRoleTypes.NIPROVIDER)
			{
				if (((NIProviderRole) role).rdmDirectoryRefresh() != null
						&& ((NIProviderRole) role).rdmLoginRequest() == null)
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
							"Must specify an rdmLoginRequest if specifying an rdmDirectoryRequest, aborting.");
				}
			} else if (role.type() == ReactorRoleTypes.PROVIDER)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
						"role must be Consumer or NIProvider Role, aborting.");
			}

			if (reactorConnectOptions.connectionList().size() == 0
					&& reactorConnectOptions.reactorWarmStandbyGroupList() == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
						"ReactorConnectOptions.connectionList() must have at least one ReactorConnectInfo or warm standby group configured, aborting.");
			}

			// create a ReactorChannel
			ReactorChannel reactorChannel = ReactorFactory.createReactorChannel();
			reactorChannel.reactor(this);

			reactorChannel.reactorConnectOptions(reactorConnectOptions);
			if(reactorConnectOptions.connectionList().size() > 0)
			{
				reactorChannel.setCurrentReactorConnectInfo(reactorChannel.getReactorConnectOptions().connectionList().get(0));
				reactorChannel.setCurrentConnectOptionsInfo(reactorChannel._connectOptionsInfoList.get(0));
				reactorChannel.userSpecObj(reactorChannel.getReactorConnectOptions().connectionList().get(0).connectOptions().userSpecObject());
			}
			reactorChannel.role(role);

			// create watchlist if enabled
			if (role.type() == ReactorRoleTypes.CONSUMER && ((ConsumerRole) role).watchlistOptions().enableWatchlist())
			{
				Watchlist watchlist = ReactorFactory.createWatchlist(reactorChannel, (ConsumerRole) role);
				reactorChannel.watchlist(watchlist);

				// If warm standby is enabled
				if (reactorConnectOptions.reactorWarmStandbyGroupList() != null
						&& reactorConnectOptions.reactorWarmStandbyGroupList().size() > 0)
				{
					watchlist.watchlistOptions().enableWarmStandby(true);

					warmStandbyHandlerImpl = new ReactorWarmStandbyHandler();

					// Copy the Warm Standby Group info
					warmStandbyHandlerImpl.connectionOptions(reactorChannel.getReactorConnectOptions());
					warmStandbyHandlerImpl.currentWarmStandbyGroupIndex(0);
					
					for (int i = 0; i < reactorChannel.getReactorConnectOptions().reactorWarmStandbyGroupList()
							.size(); i++)
					{
						ReactorWarmStandbyGroupImpl reactorWarmStandbyGroupImpl = (ReactorWarmStandbyGroupImpl) reactorConnectOptions
								.reactorWarmStandbyGroupList().get(i);

						if (reactorWarmStandbyGroupImpl.warmStandbyMode() == ReactorWarmStandbyMode.SERVICE_BASED)
						{
							// Copy the active serviceOpts
							ReactorPerServiceBasedOptions serviceOpts = reactorWarmStandbyGroupImpl
									.startingActiveServer().perServiceBasedOptions();
							for (int j = 0; j < serviceOpts.serviceNameList().size(); j++)
							{
								ReactorWSBService service = ReactorFactory.createWsbService();
								Buffer serviceName = serviceOpts.serviceNameList().get(j);
								ByteBuffer nameBytes = ByteBuffer.allocate(serviceName.length());
								service.serviceName.data(nameBytes);

								serviceName.copy(service.serviceName);
								service.standbyListIndex = ReactorWarmStandbyGroupImpl.REACTOR_WSB_STARTING_SERVER_INDEX;
								reactorWarmStandbyGroupImpl._startupServiceNameList.put(service.serviceName, service);
							}

							// Now copy the Standby service Opts for the standbys
							for (int j = 0; j < reactorWarmStandbyGroupImpl.standbyServerList().size(); j++)
							{
								ReactorWarmStandbyServerInfo serverInfo = reactorWarmStandbyGroupImpl
										.standbyServerList().get(j);
								for (int k = 0; k < serverInfo.perServiceBasedOptions().serviceNameList().size(); k++)
								{
									ReactorWSBService service = ReactorFactory.createWsbService();
									
									Buffer serviceName = serverInfo.perServiceBasedOptions().serviceNameList().get(k);
									ByteBuffer nameBytes = ByteBuffer.allocate(serviceName.length());
									service.serviceName.data(nameBytes);

									serviceName.copy(service.serviceName);
									service.standbyListIndex = k;
									reactorWarmStandbyGroupImpl._startupServiceNameList.put(service.serviceName,
											service);
								}
							}
						} else if (reactorWarmStandbyGroupImpl.warmStandbyMode() != ReactorWarmStandbyMode.LOGIN_BASED)
						{
							reactorChannel.returnToPool();
							return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
									"Invalid Warm Standby Configuration");
						}
					}

					reactorChannel.setCurrentReactorConnectInfo(warmStandbyHandlerImpl.currentWarmStandbyGroupImpl()
							.startingActiveServer().reactorConnectInfo());
					reactorChannel.setCurrentConnectOptionsInfo(warmStandbyHandlerImpl.currentWarmStandbyGroupImpl().startingConnectOptionsInfo);

					if (reactorChannel._reactorConnectOptions._connectionList.size() > 0)
					{
						warmStandbyHandlerImpl.hasConnectionList(true);
					}

					if (reactorChannel.getCurrentReactorConnectInfo().connectOptions().unifiedNetworkInfo().address() != null
							&& reactorChannel.getCurrentReactorConnectInfo().connectOptions().unifiedNetworkInfo()
									.serviceName() != null)
					{
						// We have a potential connection set in the starting active server
						reactorChannel.isStartingServerConfig = true;
						reactorChannel.standByGroupListIndex = 0;
						reactorChannel.initializationTimeout(reactorChannel.getCurrentReactorConnectInfo().initTimeout());
						reactorChannel.warmStandByHandlerImpl = warmStandbyHandlerImpl;
						reactorChannel
								.userSpecObj(reactorChannel.getCurrentReactorConnectInfo().connectOptions().userSpecObject());
					} else
					{
						if (warmStandbyHandlerImpl.currentWarmStandbyGroupImpl().startingActiveServer()
								.reactorConnectInfo().enableSessionManagement())
						{
							// We can get an endpoint from service discovery later
							reactorChannel.isStartingServerConfig = true;
							reactorChannel.initializationTimeout(reactorChannel.getCurrentReactorConnectInfo().initTimeout());
							reactorChannel.warmStandByHandlerImpl = warmStandbyHandlerImpl;
							reactorChannel.userSpecObj(
									reactorChannel.getCurrentReactorConnectInfo().connectOptions().userSpecObject());
						} else
						{
							// No potential connection and no session management, send channel down
							reactorChannel.state(State.DOWN);
							sendAndHandleChannelEventCallback("Reactor.connect", ReactorChannelEventTypes.CHANNEL_DOWN,
									reactorChannel, errorInfo);

							removeReactorChannel(reactorChannel);
							reactorChannel.returnToPool();
							return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
									"There is no valid connection information for a starting server of the warm standby feature.");
						}
					}

					reactorChannel.standByServerListIndex = ReactorWarmStandbyGroupImpl.REACTOR_WSB_STARTING_SERVER_INDEX;

					warmStandbyHandlerImpl.mainReactorChannelImpl(ReactorFactory.createReactorChannel());
					warmStandbyHandlerImpl.mainReactorChannelImpl().warmStandByHandlerImpl = warmStandbyHandlerImpl;
					warmStandbyHandlerImpl.mainReactorChannelImpl().role(reactorChannel.role());
					warmStandbyHandlerImpl.setConnectingToStartingServerState();
					warmStandbyHandlerImpl.mainReactorChannelImpl().reactor(this);
					warmStandbyHandlerImpl.mainReactorChannelImpl().warmStandByHandlerImpl = warmStandbyHandlerImpl;
					warmStandbyHandlerImpl.mainReactorChannelImpl().state(ReactorChannel.State.UP);
					warmStandbyHandlerImpl
							.currentWarmStandbyGroupImpl().currentStartingServerIndex = ReactorWarmStandbyGroupImpl.REACTOR_WSB_STARTING_SERVER_INDEX;

					warmStandbyHandlerImpl.startingReactorChannel(reactorChannel);
					warmStandbyHandlerImpl.mainReactorChannelImpl().reactorChannelType(ReactorChannelType.WARM_STANDBY);
					warmStandbyHandlerImpl.channelList().add(reactorChannel);
				}
			}

			if (reactorChannel.getCurrentReactorConnectInfo().initTimeout() < 1)
			{
				removeReactorChannel(reactorChannel);
				reactorChannel.returnToPool();
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
						"ReactorConnectOptions.timeout must be greater than zero, aborting.");
			} else if (reactorChannel.getCurrentReactorConnectInfo().connectOptions().blocking() == true)
			{
				removeReactorChannel(reactorChannel);
				reactorChannel.returnToPool();
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
						"ReactorConnectOptions.connectOptions.blocking must be false, aborting.");
			}

			if (enableSessionManagement(reactorConnectOptions))
			{
				ReactorOAuthCredential oAuthCredential;

				// setup a rest client if any connections are requiring the session management
				try
				{
					setupRestClient(errorInfo);
				} catch (Exception e)
				{
					reactorChannel.returnToPool();
					return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.setupRestClient",
							"failed to initialize the RESTClient, exception=" + e.getLocalizedMessage());
				}

				if ((oAuthCredential = retriveOAuthCredentialFromConsumerRole(role, errorInfo)) != null)
				{
					tokenSession = getTokenSession(oAuthCredential, errorInfo);

					if (tokenSession == null)
					{
						reactorChannel.returnToPool();
						return errorInfo.code();
					}

					reactorChannel.tokenSession(tokenSession);
				} else
				{
					reactorChannel.returnToPool();
					return errorInfo.code();
				}
			}

			if (reactorChannel.getCurrentReactorConnectInfo().enableSessionManagement())
			{
				if (sessionManagementStartup(tokenSession, reactorChannel.getCurrentReactorConnectInfo(), role, reactorChannel,
						true, errorInfo) != ReactorReturnCodes.SUCCESS)
				{
					removeReactorChannel(reactorChannel);
					reactorChannel.returnToPool();
					return errorInfo.code();
				}

				reactorChannel.applyAccessToken();

				/* Set original expires in when sending request using password grant type */
				tokenSession.originalExpiresIn(tokenSession.authTokenInfo().expiresIn());

				sendAuthTokenEvent = true;

				/* Clears OAuth sensitive information if the callback is specified */
				if (tokenSession.oAuthCredential().reactorOAuthCredentialEventCallback() != null)
				{
					tokenSession.oAuthCredential().password().clear();
					tokenSession.oAuthCredential().clientSecret().clear();
				}
			}

			if (_reactorOptions.debuggerOptions().debugConnectionLevel())
			{
				debugger.writeDebugInfo(ReactorDebugger.CONNECTION_SESSION_STARTUP_DONE, this.hashCode(),
						reactorChannel.hashCode(), ReactorDebugger.getChannelId(reactorChannel),
						enableSessionManagement(reactorConnectOptions));
			}

			reactorChannel.userSpecObj(reactorChannel.getCurrentReactorConnectInfo().connectOptions().userSpecObject());
			reactorChannel.initializationTimeout(reactorChannel.getCurrentReactorConnectInfo().initTimeout());

			reactorChannel.state(State.INITIALIZING);

			// Add it to the initChannelQueue.
			_reactorChannelQueue.pushBack(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);

			// enable channel read/write locking for reactor since it's multi-threaded with
			// worker thread
			ConnectOptions connectOptions = reactorChannel.getCurrentReactorConnectInfo().connectOptions();
			connectOptions.channelReadLocking(true);
			connectOptions.channelWriteLocking(true);

			// call Transport.connect to create a new Channel
			Channel channel = Transport.connect(connectOptions, errorInfo.error());
			reactorChannel.selectableChannelFromChannel(channel);
			
			// call channelOpenCallback if callback defined in the consumer role
			if (role.type() == ReactorRoleTypes.CONSUMER && ((ConsumerRole) role).watchlistOptions().enableWatchlist() && ((ConsumerRole) role).watchlistOptions().channelOpenCallback() != null)
			{
				sendAndHandleChannelEventCallback("Reactor.connect", ReactorChannelEventTypes.CHANNEL_OPENED,
						reactorChannel, errorInfo);
			}

			if (sendAuthTokenEvent)
			{
				sendAuthTokenEventCallback(reactorChannel, tokenSession.authTokenInfo(), errorInfo);
				reactorChannel.sessionMgntState(ReactorChannel.SessionMgntState.RECEIVED_AUTH_TOKEN);
				if (!tokenSession.isInitialized())
				{
					boolean retVal;
					if (tokenSession.authTokenInfo().tokenVersion() == TokenVersion.V1)
					{
						retVal = sendAuthTokenWorkerEvent(tokenSession);
					} else
					{
						retVal = sendAuthTokenWorkerEvent(reactorChannel, tokenSession);
					}

					if (retVal == false)
					{
						removeReactorChannel(reactorChannel);
						reactorChannel.returnToPool();
						_reactorChannelQueue.remove(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);
						return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
								"sendAuthTokenWorkerEvent() failed");
					}
				}
			}

			if (_reactorOptions.debuggerOptions().debugConnectionLevel())
			{
				if (channel != null)
				{
					debugger.writeDebugInfo(ReactorDebugger.CONNECTION_CONNECTING_PERFORMED, this.hashCode(),
							reactorChannel.hashCode(), ReactorDebugger.getChannelId(reactorChannel));
				} else
				{
					debugger.writeDebugInfo(ReactorDebugger.CONNECTION_CHANNEL_DOWN, this.hashCode(),
							reactorChannel.hashCode(), ReactorDebugger.getChannelId(reactorChannel));
				}
			}

			if (channel == null)
			{
				if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
				{
					reactorChannel.state(State.DOWN_RECONNECTING);

					// send CHANNEL_DOWN to user app via reactorChannelEventCallback.
					sendAndHandleChannelEventCallback("Reactor.connect",
							ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
				} else // server channel or no more retries
				{
					reactorChannel.state(State.DOWN);

					// send CHANNEL_DOWN to user app via reactorChannelEventCallback.
					sendAndHandleChannelEventCallback("Reactor.connect", ReactorChannelEventTypes.CHANNEL_DOWN,
							reactorChannel, errorInfo);
				}
			}
			// send a WorkerEvent to the Worker to initialize this channel.
			else if (!sendWorkerEvent(WorkerEventTypes.CHANNEL_INIT, reactorChannel))
			{
				// sendWorkerEvent() failed, send channel down
				reactorChannel.state(State.DOWN);
				sendAndHandleChannelEventCallback("Reactor.connect", ReactorChannelEventTypes.CHANNEL_DOWN,
						reactorChannel, errorInfo);

				removeReactorChannel(reactorChannel);
				reactorChannel.returnToPool();
				_reactorChannelQueue.remove(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
						"sendWorkerEvent() failed");
			}
		} finally
		{
			_reactorLock.unlock();
		}

		return ReactorReturnCodes.SUCCESS;
	}

	final static boolean compareOAuthCredential(Reactor reactor, ReactorOAuthCredential current,
			ReactorOAuthCredential other, ReactorErrorInfo errorInfo)
	{
		if (current.reactorOAuthCredentialEventCallback() != other.reactorOAuthCredentialEventCallback())
		{
			reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.compareOAuthCredential",
					"The ReactorOAuthCredentialEventCallback of ReactorOAuthCredential is not equal for the existing token session.");
			return false;
		}

		if (current.reactorOAuthCredentialEventCallback() == null)
		{
			if (current.clientSecret().equals(other.clientSecret()) == false)
			{
				reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.compareOAuthCredential",
						"The Client secret of ReactorOAuthCredential is not equal for the existing token session.");
				return false;
			}

			if (current.password().equals(other.password()) == false)
			{
				reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.compareOAuthCredential",
						"The password of ReactorOAuthCredential is not equal for the existing token session.");
				return false;
			}
		}

		if (current.clientId().equals(other.clientId()) == false)
		{
			reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.compareOAuthCredential",
					"The Client ID of ReactorOAuthCredential is not equal for the existing token session.");
			return false;
		}

		if (current.tokenScope().equals(other.tokenScope()) == false)
		{
			reactor.populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE, "Reactor.compareOAuthCredential",
					"The token scope of ReactorOAuthCredential is not equal for the existing token session.");
			return false;
		}

		if (current.takeExclusiveSignOnControl() != other.takeExclusiveSignOnControl())
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

		/*
		 * If a userName is present, then this is V1, so we need to make sure that a
		 * session has not already been created
		 */
		if (oAuthCredential.userName().length() != 0)
		{
			try
			{
				String userName = oAuthCredential.userName().toString();

				_tokenManagementLock.lock();
				tokenSession = _tokenManagementMap.get(oAuthCredential.userName().toString());

				if (tokenSession == null)
				{
					tokenSession = new ReactorTokenSession(this, oAuthCredential);
					_tokenManagementMap.put(userName, tokenSession);
				} else
				{
					/*
					 * Checks whether there is sufficient time to reissue the access token for this
					 * channel.
					 */
					if (tokenSession.checkMiniumTimeForReissue(errorInfo) == false)
					{
						return null;
					}

					/*
					 * Validates the credential with the existing token session for the same user
					 * name
					 */
					if (compareOAuthCredential(this, tokenSession.oAuthCredential(), oAuthCredential,
							errorInfo) == false)
					{
						return null;
					}
				}
			} finally
			{
				_tokenManagementLock.unlock();
			}
		} else
		{
			/*
			 * V2, create new token session. The version will be determined when the
			 * ReactorTokenSession is created
			 */
			tokenSession = new ReactorTokenSession(this, oAuthCredential);
		}

		return tokenSession;
	}

	void removeTokenSession(ReactorTokenSession tokenSession)
	{
		if (tokenSession == null)
			return;

		_tokenManagementLock.lock();

		try
		{
			_tokenManagementMap.remove(tokenSession.oAuthCredential().userName().toString());
		} finally
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

			while (iter.hasNext())
			{
				tokenSession = iter.next();

				tokenSession.removeAllReactorChannel();
			}

			_tokenManagementMap.clear();
		} finally
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
		} finally
		{
			_tokenManagementLock.unlock();
		}
	}

	void removeReactorChannel(ReactorChannel reactorChannel)
	{
		ReactorTokenSession tokenSession = reactorChannel.tokenSession();

		if (tokenSession != null)
		{
			if (tokenSession.removeReactorChannel(reactorChannel) == 0)
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

		for (int i = 0; i < reactorConnectOptions.reactorWarmStandbyGroupList().size(); i++)
		{
			if (reactorConnectOptions.reactorWarmStandbyGroupList().get(i).startingActiveServer().reactorConnectInfo()
					.enableSessionManagement())
			{
				return true;
			}

			for (int j = 0; j < reactorConnectOptions.reactorWarmStandbyGroupList().get(i).standbyServerList()
					.size(); j++)
			{
				if (reactorConnectOptions.reactorWarmStandbyGroupList().get(i).standbyServerList().get(j)
						.reactorConnectInfo().enableSessionManagement())
				{
					return true;
				}
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
			oauthCredential = ((ConsumerRole) role).reactorOAuthCredential();
			loginRequest = ((ConsumerRole) role).rdmLoginRequest();
		} else if (role.type() == ReactorRoleTypes.NIPROVIDER)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE,
					"Reactor.copyOAuthCredentialForSessionManagement",
					"The session management supports only on the ReactorRoleTypes.CONSUMER type.");
			return null;
		}

		if (loginRequest == null && oauthCredential == null)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE,
					"Reactor.copyOAuthCredentialForSessionManagement",
					"There is no user credential available for enabling session management.");
			return null;
		}

		Buffer userName = (oauthCredential != null && oauthCredential.userName().length() != 0)
				? oauthCredential.userName()
				: (loginRequest != null && loginRequest.userName().length() != 0) ? loginRequest.userName() : null;
		Buffer password = (oauthCredential != null && oauthCredential.password().length() != 0)
				? oauthCredential.password()
				: (loginRequest != null && loginRequest.password().length() != 0) ? loginRequest.password() : null;

		Buffer clientId = (oauthCredential != null && oauthCredential.clientId().length() != 0)
				? oauthCredential.clientId()
				: ((ConsumerRole) role).clientId();

		Buffer clientSecret = (oauthCredential != null && oauthCredential.clientSecret().length() != 0)
				? oauthCredential.clientSecret()
				: null;
		
		Buffer clientJwk = (oauthCredential != null && oauthCredential.clientJwk().length() != 0)
				? oauthCredential.clientJwk()
				: null;

		if ((clientSecret == null || clientSecret.length() == 0) && (clientJwk == null || clientJwk.length() == 0))
		{
			if (userName == null || userName.length() == 0)
			{
				populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE,
						"Reactor.copyOAuthCredentialForSessionManagement",
						"Failed to copy OAuth credential for enabling the session management; OAuth user name does not exist.");
				return null;
			}

			if (clientId == null || clientId.length() == 0)
			{
				populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE,
						"Reactor.copyOAuthCredentialForSessionManagement",
						"Failed to copy OAuth credential for enabling the session management; OAuth client ID does not exist.");
				return null;
			}

			if (password == null || password.length() == 0)
			{
				populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE,
						"Reactor.copyOAuthCredentialForSessionManagement",
						"Failed to copy OAuth credential for enabling the session management; OAuth password does not exist.");
				return null;
			}

			oauthCredentialOut = ReactorFactory.createReactorOAuthCredential();

			oauthCredentialOut.userName().data(userName.toString());
			oauthCredentialOut.password().data(password.toString());
			oauthCredentialOut.clientId().data(clientId.toString());

			if (oauthCredential != null)
			{
				if (oauthCredential.clientSecret().length() != 0)
				{
					oauthCredentialOut.clientSecret().data(oauthCredential.clientSecret().toString());
				}

				if (oauthCredential.tokenScope().length() != 0)
				{
					oauthCredentialOut.tokenScope().data(oauthCredential.tokenScope().toString());
				}

				oauthCredentialOut.takeExclusiveSignOnControl(oauthCredential.takeExclusiveSignOnControl());
				oauthCredentialOut
						.reactorOAuthCredentialEventCallback(oauthCredential.reactorOAuthCredentialEventCallback());
				oauthCredentialOut.userSpecObj(oauthCredential.userSpecObj());
			}

			return oauthCredentialOut;
		}

		if ((clientSecret == null || clientSecret.length() == 0) && (clientJwk == null || clientJwk.length() == 0))
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE,
					"Reactor.copyOAuthCredentialForSessionManagement",
					"Failed to copy OAuth credential for enabling the session management; OAuth client secret or JWK does not exist.");
			return null;
		}

		if (clientId == null || clientId.length() == 0)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.INVALID_USAGE,
					"Reactor.copyOAuthCredentialForSessionManagement",
					"Failed to copy OAuth credential for enabling the session management; OAuth client Id does not exist.");
			return null;
		}

		oauthCredentialOut = ReactorFactory.createReactorOAuthCredential();

		if(clientSecret != null)
		{
			oauthCredentialOut.clientSecret().data(clientSecret.toString());
		}
		
		if(clientJwk != null)
		{
			oauthCredentialOut.clientJwk().data(clientJwk.toString());
		}

		oauthCredentialOut.clientId().data(clientId.toString());

		if (oauthCredential != null)
		{

			if (oauthCredential.tokenScope().length() != 0)
			{
				oauthCredentialOut.tokenScope().data(oauthCredential.tokenScope().toString());
			}
			
			if (oauthCredential.audience() != null && oauthCredential.audience().length() != 0)
			{
				oauthCredentialOut.audience().data(oauthCredential.audience().toString());
			}

			oauthCredentialOut
					.reactorOAuthCredentialEventCallback(oauthCredential.reactorOAuthCredentialEventCallback());
			oauthCredentialOut.userSpecObj(oauthCredential.userSpecObj());
		}

		return oauthCredentialOut;
	}

	int sessionManagementStartup(ReactorTokenSession tokenSession, ReactorConnectInfo reactorConnectInfo,
			ReactorRole role, ReactorChannel reactorChannel, boolean isBlocking, ReactorErrorInfo errorInfo)
	{
		// save login information
		LoginRequest loginRequest = null;

		if (role.type() == ReactorRoleTypes.CONSUMER)
		{
			loginRequest = ((ConsumerRole) role).rdmLoginRequest();
		}

		if ((loginRequest != null) && (reactorChannel._loginRequestForEDP == null))
		{
			reactorChannel._loginRequestForEDP = (LoginRequest) LoginMsgFactory.createMsg();
			reactorChannel._loginRequestForEDP.rdmMsgType(LoginMsgType.REQUEST);

			loginRequest.copy(reactorChannel._loginRequestForEDP);
			reactorChannel._loginRequestForEDP.userNameType(Login.UserIdTypes.AUTHN_TOKEN);
			// Do not send the password
			reactorChannel._loginRequestForEDP
					.flags(reactorChannel._loginRequestForEDP.flags() & ~LoginRequestFlags.HAS_PASSWORD);
		}

		if (requestServiceDiscovery(reactorConnectInfo))
		{
			switch (reactorConnectInfo.connectOptions().connectionType())
			{
			case ConnectionTypes.ENCRYPTED:

				if (reactorConnectInfo.connectOptions().encryptionOptions()
						.connectionType() == ConnectionTypes.WEBSOCKET)
				{
					/* Query endpoints for the websocket connection type. */
					reactorChannel.restConnectOptions().transport(ReactorDiscoveryTransportProtocol.RD_TP_WEBSOCKET);
					reactorChannel.restConnectOptions().dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2);
				} else
				{
					reactorChannel.restConnectOptions().transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
					reactorChannel.restConnectOptions().dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);
				}

				break;
			default:
				populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID, "Reactor.connect",
						"Reactor.connect(): Invalid connection type: "
								+ ConnectionTypes.toString(reactorConnectInfo.connectOptions().connectionType())
								+ " for requesting EDP-RT service discovery.");
				return ReactorReturnCodes.PARAMETER_INVALID;
			}
		}

		tokenSession.lock();

		reactorChannel.state(State.EDP_RT);
		tokenSession.setProxyInfo(reactorConnectInfo);

		try
		{
			if (tokenSession.sessionMgntState() == SessionState.REQUEST_TOKEN_FAILURE
					|| tokenSession.sessionMgntState() == SessionState.STOP_TOKEN_REQUEST
					|| tokenSession.sessionMgntState() == SessionState.REQ_AUTH_TOKEN_USING_PASSWORD
					|| tokenSession.sessionMgntState() == SessionState.REQ_AUTH_TOKEN_USING_REFRESH_TOKEN)
			{
				return ReactorReturnCodes.SUCCESS;
			}

			if (!tokenSession.hasAccessToken())
			{
				if (_restClient.getAuthAccessTokenInfo(tokenSession.authOptoins(), tokenSession.restConnectOptions(),
						tokenSession.authTokenInfo(), isBlocking, errorInfo) != ReactorReturnCodes.SUCCESS)
				{
					return errorInfo.code();
				}

				/*
				 * The service discovery will be requested in the callback of the token request
				 * if needed.
				 */
				if (!isBlocking)
				{
					return ReactorReturnCodes.SUCCESS;
				}
			}
		} finally
		{
			tokenSession.unlock();
		}

		if (requestServiceDiscovery(reactorConnectInfo))
		{
			if (_restClient.getServiceDiscovery(reactorChannel.restConnectOptions(), tokenSession.authTokenInfo(),
					isBlocking, reactorChannel.reactorServiceEndpointInfoList(),
					errorInfo) != ReactorReturnCodes.SUCCESS)
			{
				return errorInfo.code();
			}

			if (isBlocking)
			{
				if (reactorChannel.applyServiceDiscoveryEndpoint(errorInfo) != ReactorReturnCodes.SUCCESS)
				{
					return errorInfo.code();
				} else
				{
					reactorChannel.state(State.EDP_RT_DONE);
				}
			}
		} else
		{
			reactorChannel.state(State.EDP_RT_DONE);
		}

		return ReactorReturnCodes.SUCCESS;
	}

	static boolean requestServiceDiscovery(ReactorConnectInfo reactorConnectInfo)
	{
		// only use the EDP-RT connection information if not specified by the user
		if ((reactorConnectInfo.connectOptions().unifiedNetworkInfo().address() == null
				&& reactorConnectInfo.connectOptions().unifiedNetworkInfo().serviceName() == null)
				|| (reactorConnectInfo.connectOptions().unifiedNetworkInfo().address() != null
						&& reactorConnectInfo.connectOptions().unifiedNetworkInfo().address().equals("")
						&& reactorConnectInfo.connectOptions().unifiedNetworkInfo().serviceName() != null
						&& reactorConnectInfo.connectOptions().unifiedNetworkInfo().serviceName().equals("")))
		{
			return true;
		}
		return false;
	}

	ReactorOptions reactorOptions()
	{
		return _reactorOptions;
	}

	private static final int sendQueryServiceDiscoveryEvent(ReactorServiceDiscoveryOptions options,
			List<ReactorServiceEndpointInfo> reactorServiceEndpointInfoList, ReactorErrorInfo errorInfo)
	{
		ReactorServiceEndpointEvent reactorServiceEndpointEvent = ReactorFactory.createReactorServiceEndpointEvent();

		ReactorErrorInfo errorInfoTemp = reactorServiceEndpointEvent._errorInfo;

		if (reactorServiceEndpointInfoList != null)
		{
			reactorServiceEndpointEvent._reactorServiceEndpointInfoList = reactorServiceEndpointInfoList;
		} else
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
	 * @param options   The {@link ReactorServiceDiscoveryOptions} to configure
	 *                  options and specify the ReactorServiceEndpointEventCallback
	 *                  to receive service endpoint information.
	 * @param errorInfo error structure to be populated in the event of failure
	 *
	 * @return {@link ReactorReturnCodes} indicating success or failure
	 */
	public int queryServiceDiscovery(ReactorServiceDiscoveryOptions options, ReactorErrorInfo errorInfo)
	{
		_reactorLock.lock();

		try
		{

			ReactorAuthTokenInfo authTokenInfo;
			RestConnectOptions connOptions;

			if (errorInfo == null)
			{
				return ReactorReturnCodes.PARAMETER_INVALID;
			}

			if (!_reactorActive)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.SHUTDOWN, "Reactor.queryServiceDiscovery",
						"Reactor is not active, aborting.");
			}

			if (options == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
						"Reactor.queryServiceDiscovery",
						"Reactor.queryServiceDiscovery(): options cannot be null, aborting.");
			}

			if (options.reactorServiceEndpointEventCallback() == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
						"Reactor.queryServiceDiscovery",
						"Reactor.queryServiceDiscovery(): ReactorServiceEndpointEventCallback cannot be null, aborting.");
			}

			if ((options.userName() == null || options.userName().length() == 0))
			{
				if (options.clientId() == null || options.clientId().length() == 0)
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
							"Reactor.queryServiceDiscovery", "Required parameter username or clientId are not set");
				}
			} else if (options.clientId() == null || options.clientId().length() == 0)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
						"Reactor.queryServiceDiscovery", "Required parameter clientId is not set");
			}

			if ((options.password() == null || options.password().length() == 0)
					&& (options.clientSecret() == null || options.clientSecret().length() == 0) && 
					(options.clientJWK() == null || options.clientJWK().length() == 0))
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
						"Reactor.queryServiceDiscovery", "Required parameter(one of the following) password, clientSecret, and clientJWK are not set");
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
			} catch (Exception e)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.setupRestClient",
						"failed to initialize the RESTClient, exception=" + e.getLocalizedMessage());
			}

			_tokenManagementLock.lock();
			ReactorTokenSession tokenSession = null;

			try
			{
				tokenSession = _tokenManagementMap.get(options.userName().toString());
			} finally
			{
				_tokenManagementLock.unlock();
			}

			if (tokenSession == null)
			{
				RestAuthOptions authOptions = new RestAuthOptions(options.takeExclusiveSignOnControl());
				authTokenInfo = new ReactorAuthTokenInfo();
				connOptions = new RestConnectOptions(reactorOptions());
				if (options.userName() == null || options.userName().length() == 0)
				{
					authTokenInfo.tokenVersion(TokenVersion.V2);
				} else
				{
					authTokenInfo.tokenVersion(TokenVersion.V1);
				}
				authOptions.username(options.userName().toString());
				authOptions.password(options.password().toString());
				authOptions.clientId(options.clientId().toString());
				authOptions.clientSecret(options.clientSecret().toString());
				authOptions.clientJwk(options.clientJWK().toString());
				authOptions.audience(options.audience().toString());
				authOptions.tokenScope(options.tokenScope().toString());

				connOptions.applyServiceDiscoveryOptions(options);

				if (_restClient.getAuthAccessTokenInfo(authOptions, connOptions, authTokenInfo, true,
						errorInfo) != ReactorReturnCodes.SUCCESS)
				{
					Reactor.sendQueryServiceDiscoveryEvent(options, null, errorInfo);
					return ReactorReturnCodes.SUCCESS;
				}
			} else
			{
				authTokenInfo = tokenSession.authTokenInfo();
				connOptions = tokenSession.restConnectOptions();
			}

			_reactorServiceEndpointInfoList.clear();

			if (_restClient.getServiceDiscovery(connOptions, authTokenInfo, true, _reactorServiceEndpointInfoList,
					errorInfo) != ReactorReturnCodes.SUCCESS)
			{
				Reactor.sendQueryServiceDiscoveryEvent(options, null, errorInfo);
			} else
			{
				Reactor.sendQueryServiceDiscoveryEvent(options, _reactorServiceEndpointInfoList, errorInfo);
			}
		} finally
		{
			_reactorLock.unlock();
		}

		return ReactorReturnCodes.SUCCESS;
	}

	/**
	 * Submit OAuth credential renewal with password or password change.
	 *
	 * @param renewalOptions         The
	 *                               {@link ReactorOAuthCredentialRenewalOptions} to
	 *                               configure OAuth credential renewal options.
	 * @param oAuthCredentialRenewal The {@link ReactorOAuthCredentialRenewal} to
	 *                               configure credential renewal information.
	 * @param errorInfo              error structure to be populated in the event of
	 *                               failure
	 * @return {@link ReactorReturnCodes} indicating success or failure
	 */
	public int submitOAuthCredentialRenewal(ReactorOAuthCredentialRenewalOptions renewalOptions,
			ReactorOAuthCredentialRenewal oAuthCredentialRenewal, ReactorErrorInfo errorInfo)
	{
		_reactorLock.lock();

		try
		{

			ReactorTokenSession tokenSession = null;
			ReactorOAuthCredentialRenewal oAuthCredentialRenewalCopy;

			if (errorInfo == null)
			{
				return ReactorReturnCodes.PARAMETER_INVALID;
			}

			if (!_reactorActive)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.SHUTDOWN, "Reactor.submitOAuthCredentialRenewal",
						"Reactor is not active, aborting.");
			}

			if (renewalOptions == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
						"Reactor.submitOAuthCredentialRenewal", "renewalOptions cannot be null, aborting.");
			}

			if (oAuthCredentialRenewal == null)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
						"Reactor.submitOAuthCredentialRenewal", "oAuthCredentialRenewal cannot be null, aborting.");
			}

			/*
			 * Checks whether the token session is available from calling with in the
			 * callback method
			 */
			tokenSession = _tokenSessionForCredentialRenewalCallback;

			if (tokenSession == null)
			{
				if (oAuthCredentialRenewal.userName() == null || oAuthCredentialRenewal.userName().isBlank())
				{
					if (oAuthCredentialRenewal.clientId() == null || oAuthCredentialRenewal.clientId().isBlank())
					{
						return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
								"Reactor.submitOAuthCredentialRenewal",
								"ReactorOAuthCredentialRenewal.userName() or clientId() not provided, aborting.");
					}
				}

				if (oAuthCredentialRenewal.clientId() == null || oAuthCredentialRenewal.clientId().isBlank())
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
							"Reactor.submitOAuthCredentialRenewal",
							"ReactorOAuthCredentialRenewal.clientId() not provided, aborting.");
				}

				if (renewalOptions.reactorAuthTokenEventCallback() == null)
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
							"Reactor.submitOAuthCredentialRenewal",
							"ReactorOAuthCredentialRenewalOptions.reactorAuthTokenEventCallback() not provided, aborting.");
				}
			}

			if ((oAuthCredentialRenewal.password() == null || oAuthCredentialRenewal.password().isBlank())
					&& (oAuthCredentialRenewal.clientSecret() == null
							|| oAuthCredentialRenewal.clientSecret().isBlank()))
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
						"Reactor.submitOAuthCredentialRenewal",
						"ReactorOAuthCredentialRenewal.password() or clientSecret not provided, aborting.");
			}

			try
			{
				setupRestClient(errorInfo);
			} catch (Exception e)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.setupRestClient",
						"failed to initialize the RESTClient, exception=" + e.getLocalizedMessage());
			}

			switch (renewalOptions.renewalModes())
			{
			case ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD:
				break;
			case ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD_CHANGE:
			{
				if (tokenSession != null)
				{
					if (tokenSession.oAuthCredential().reactorOAuthCredentialEventCallback() == null)
					{
						return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
								"Reactor.submitOAuthCredentialRenewal",
								"Support changing password of the token session when ReactorOAuthCredential.reactorOAuthCredentialEventCallback() is specified only., aborting.");
					}
				}

				if (oAuthCredentialRenewal.newPassword() == null || oAuthCredentialRenewal.newPassword().isBlank())
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
							"Reactor.submitOAuthCredentialRenewal",
							"ReactorOAuthCredentialRenewal.newPassword() not provided, aborting.");
				}

				break;
			}
			default:
				return populateErrorInfo(errorInfo, ReactorReturnCodes.PARAMETER_INVALID,
						"Reactor.submitOAuthCredentialRenewal",
						"Invalid ReactorOAuthCredentialRenewalOptions.RenewalModes(" + renewalOptions.renewalModes()
								+ "), aborting.");
			}

			oAuthCredentialRenewalCopy = copyReactorOAuthCredentialRenewal(tokenSession, renewalOptions,
					oAuthCredentialRenewal);

			if (tokenSession != null)
			{
				tokenSession.sendAuthRequestWithSensitiveInfo(oAuthCredentialRenewalCopy.password().toString(),
						oAuthCredentialRenewalCopy.newPassword().toString(),
						oAuthCredentialRenewalCopy.clientSecret().toString(),
						oAuthCredentialRenewalCopy.clientJWK().toString());
			} else
			{
				RestAuthOptions restAuthOptions = new RestAuthOptions(
						oAuthCredentialRenewal.takeExclusiveSignOnControl());
				RestConnectOptions restConnectOptions = new RestConnectOptions(_reactorOptions);
				ReactorAuthTokenInfo authTokenInfo = new ReactorAuthTokenInfo();

				restAuthOptions.username(oAuthCredentialRenewalCopy.userName().toString());
				restAuthOptions.clientId(oAuthCredentialRenewalCopy.clientId().toString());
				restAuthOptions.password(oAuthCredentialRenewalCopy.password().toString());
				restAuthOptions.newPassword(oAuthCredentialRenewalCopy.newPassword().toString());
				restAuthOptions.clientSecret(oAuthCredentialRenewalCopy.clientSecret().toString());
				restAuthOptions.clientJwk(oAuthCredentialRenewalCopy.clientJWK().toString());
				restAuthOptions.audience(oAuthCredentialRenewalCopy.audience().toString());

				if (!restAuthOptions.username().isEmpty())
				{
					authTokenInfo.tokenVersion(TokenVersion.V1);
				} else
				{
					authTokenInfo.tokenVersion(TokenVersion.V2);
				}

				if (_restClient.getAuthAccessTokenInfo(restAuthOptions, restConnectOptions, authTokenInfo, true,
						errorInfo) != ReactorReturnCodes.SUCCESS)
				{
					return errorInfo.code();
				} else
				{
					ReactorErrorInfo errorInfoTemp;
					ReactorAuthTokenEventCallback callback = renewalOptions.reactorAuthTokenEventCallback();

					ReactorAuthTokenEvent reactorAuthTokenEvent = ReactorFactory.createReactorAuthTokenEvent();
					reactorAuthTokenEvent.reactorChannel(null);
					reactorAuthTokenEvent.reactorAuthTokenInfo(authTokenInfo);
					errorInfoTemp = reactorAuthTokenEvent._errorInfo;
					reactorAuthTokenEvent._errorInfo = errorInfo;

					callback.reactorAuthTokenEventCallback(reactorAuthTokenEvent);
					reactorAuthTokenEvent._errorInfo = errorInfoTemp;

					reactorAuthTokenEvent.returnToPool();
				}
			}

			oAuthCredentialRenewalCopy.clear();

		} finally
		{
			_reactorLock.unlock();
		}

		return ReactorReturnCodes.SUCCESS;
	}

	private static ReactorOAuthCredentialRenewal copyReactorOAuthCredentialRenewal(ReactorTokenSession tokenSession,
			ReactorOAuthCredentialRenewalOptions renewalOptions, ReactorOAuthCredentialRenewal oAuthCredentialRenewal)
	{
		ReactorOAuthCredentialRenewal oAuthCredentialRenewalOut = new ReactorOAuthCredentialRenewal();

		/* This is the case to submit credential renewal without the token session */
		if (tokenSession == null)
		{
			oAuthCredentialRenewalOut.userName().data(oAuthCredentialRenewal.userName().toString());
			oAuthCredentialRenewalOut.clientId().data(oAuthCredentialRenewal.clientId().toString());
			oAuthCredentialRenewalOut.tokenScope().data(oAuthCredentialRenewal.tokenScope().toString());
			oAuthCredentialRenewalOut.audience().data(oAuthCredentialRenewal.audience().toString());
		}

		oAuthCredentialRenewalOut.password().data(oAuthCredentialRenewal.password().toString());

		if (oAuthCredentialRenewal.clientSecret().length() != 0)
		{
			oAuthCredentialRenewalOut.clientSecret().data(oAuthCredentialRenewal.clientSecret().toString());
		}
		
		if (oAuthCredentialRenewal.clientJWK().length() != 0)
		{
			oAuthCredentialRenewalOut.clientJWK().data(oAuthCredentialRenewal.clientJWK().toString());
		}

		if (renewalOptions.renewalModes() == ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD_CHANGE)
		{
			oAuthCredentialRenewalOut.newPassword().data(oAuthCredentialRenewal.newPassword().toString());
		}

		return oAuthCredentialRenewalOut;
	}

	private void setupRestClient(ReactorErrorInfo errorInfo)
	{
		if (_restClient != null)
			return;

		_restReactorOptions = new RestReactorOptions();
		_restReactorOptions.connectTimeout(0);
		_restReactorOptions.soTimeout(_reactorOptions.restRequestTimeout());

		_restClient = new RestClient(_restReactorOptions, errorInfo);
	}

	void loginReissue(ReactorChannel reactorChannel, String authToken, ReactorErrorInfo errorInfo)
	{
		if (reactorChannel.state() == State.CLOSED || reactorChannel.state() == State.DOWN
				|| reactorChannel.state() == State.EDP_RT)
			return;

		LoginRequest loginRequest = null;

		if (reactorChannel.enableSessionManagement())
		{
			if (reactorChannel._loginRequestForEDP != null)
			{
				loginRequest = reactorChannel._loginRequestForEDP;
				loginRequest.userName().data(authToken);
			}
		} else
		{
			switch (reactorChannel.role().type())
			{
			case ReactorRoleTypes.CONSUMER:
				loginRequest = ((ConsumerRole) reactorChannel.role()).rdmLoginRequest();
				break;
			case ReactorRoleTypes.NIPROVIDER:
				loginRequest = ((NIProviderRole) reactorChannel.role()).rdmLoginRequest();
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
			} finally
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

	boolean sendAuthTokenWorkerEvent(ReactorChannel reactorChannel, ReactorTokenSession tokenSession)
	{
		boolean retVal = true;

		tokenSession.isInitialized(true);

		WorkerEvent event = ReactorFactory.createWorkerEvent();
		event.eventType(WorkerEventTypes.TOKEN_MGNT);
		event._restClient = _restClient;
		event._tokenSession = tokenSession;
		event._reactorChannel = reactorChannel;
		retVal = _workerQueue.write(event);

		return retVal;
	}

	boolean sendAuthTokenEvent(ReactorChannel reactorChannel, ReactorTokenSession tokenSession,
			ReactorErrorInfo reactorErrorInfo)
	{
		boolean retVal = true;

		WorkerEvent event = ReactorFactory.createWorkerEvent();
		event.eventType(WorkerEventTypes.TOKEN_MGNT);
		event._restClient = _restClient;
		event._tokenSession = tokenSession;
		event._reactorChannel = reactorChannel;
		populateErrorInfo(event.errorInfo(), reactorErrorInfo.code(), reactorErrorInfo.location(),
				reactorErrorInfo.error().text());
		retVal = _workerQueue.remote().write(event);

		return retVal;
	}

	boolean sendWarmStandbyEvent(ReactorChannel reactorChannel, ReactorWarmStandbyEvent warmStandbyEvent,
			ReactorErrorInfo reactorErrorInfo)
	{
		boolean retVal = true;

		WorkerEvent event = ReactorFactory.createWorkerEvent();
		event.eventType(WorkerEventTypes.WARM_STANDBY);
		event._restClient = _restClient;
		event._reactorChannel = reactorChannel;
		event._warmStandbyEventType = warmStandbyEvent.warmStandbyEventType();
		event._warmStandbyHandler = reactorChannel.warmStandByHandlerImpl;
		event._serviceId = warmStandbyEvent.serviceID;
		event._streamId = warmStandbyEvent.streamID;
		event._warmStandbyService = warmStandbyEvent.wsbService;
		populateErrorInfo(event.errorInfo(), reactorErrorInfo.code(), reactorErrorInfo.location(),
				reactorErrorInfo.error().text());
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
		populateErrorInfo(event.errorInfo(), reactorErrorInfo.code(), reactorErrorInfo.location(),
				reactorErrorInfo.error().text());
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

	boolean sendWorkerEvent(WorkerEventTypes eventType, ReactorChannel reactorChannel, TunnelStream tunnelStream,
			long timeout)
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

		if (_reactorOptions.debuggerOptions().debugTunnelStreamLevel())
		{
			debugger.writeDebugInfo(ReactorDebugger.TUNNELSTREAM_DISPATCH_NOW, this.hashCode(),
					reactorChannel.hashCode(), ReactorDebugger.getChannelId(reactorChannel));
		}

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
		populateErrorInfo(event.errorInfo(), reactorErrorInfo.code(), reactorErrorInfo.location(),
				reactorErrorInfo.error().text());

		retVal = _workerQueue.remote().write(event);

		return retVal;
	}

	private int sendChannelEventCallback(int eventType, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
	{
		ReactorChannelEvent reactorChannelEvent = ReactorFactory.createReactorChannelEvent();
		reactorChannelEvent.eventType(eventType);
		populateErrorInfo(reactorChannelEvent.errorInfo(), errorInfo.code(), errorInfo.location(),
				errorInfo.error().text());

		ReactorChannel callbackChannel = reactorChannel;
		/*
		 * If this is a warm standby channel, set the callback channel to the main
		 * reactorChannel in the wsbHandler.
		 */
		if (reactorHandlesWarmStandby(reactorChannel))
		{
			callbackChannel = reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl();
			callbackChannel.selectableChannelFromChannel(reactorChannel.channel());
			callbackChannel.userSpecObj(reactorChannel.userSpecObj());
		}

		reactorChannelEvent.reactorChannel(callbackChannel);

		int retval = reactorChannel.role().channelEventCallback().reactorChannelEventCallback(reactorChannelEvent);
		reactorChannelEvent.returnToPool();

		return retval;
	}

	int sendAuthTokenEventCallback(ReactorChannel reactorChannel, ReactorAuthTokenInfo authTokenInfo,
			ReactorErrorInfo errorInfo)
	{
		int retval;
		ReactorErrorInfo errorInfoTemp;
		ReactorAuthTokenEventCallback callback = reactorChannel.reactorAuthTokenEventCallback();

		if (callback != null && reactorChannel.enableSessionManagement())
		{
			ReactorAuthTokenEvent reactorAuthTokenEvent = ReactorFactory.createReactorAuthTokenEvent();
			reactorAuthTokenEvent.reactorAuthTokenInfo(authTokenInfo);
			errorInfoTemp = reactorAuthTokenEvent._errorInfo;
			reactorAuthTokenEvent._errorInfo = errorInfo;

			ReactorChannel callbackChannel = reactorChannel;
			/*
			 * If this is a warm standby channel, set the callback channel to the main
			 * reactorChannel in the wsbHandler.
			 */
			if (reactorHandlesWarmStandby(reactorChannel))
			{
				callbackChannel = reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl();
				callbackChannel.selectableChannelFromChannel(reactorChannel.channel());
				callbackChannel.userSpecObj(reactorChannel.userSpecObj());
			}

			reactorAuthTokenEvent.reactorChannel(callbackChannel);

			retval = callback.reactorAuthTokenEventCallback(reactorAuthTokenEvent);
			reactorAuthTokenEvent._errorInfo = errorInfoTemp;

			reactorAuthTokenEvent.returnToPool();

			if (retval != ReactorCallbackReturnCodes.SUCCESS)
			{
				// retval is not a valid ReactorReturnCodes.
				populateErrorInfo(errorInfo, retval, "Reactor.sendAuthTokenEventCallback", "retval of " + retval
						+ " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
				shutdown(errorInfo);
				return ReactorReturnCodes.FAILURE;
			}
		} else
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
			ReactorOAuthCredentialEvent reactorOAuthCredentialEvent = ReactorFactory
					.createReactorOAuthCredentialEvent();

			reactorOAuthCredentialEvent.reactorOAuthCredentialRenewal(tokenSession.oAuthCredentialRenewal());
			reactorOAuthCredentialEvent.reactorChannel(null);
			reactorOAuthCredentialEvent._reactor = this;
			reactorOAuthCredentialEvent._userSpecObj = tokenSession.oAuthCredential().userSpecObj();

			errorInfoTemp = reactorOAuthCredentialEvent._errorInfo;
			reactorOAuthCredentialEvent._errorInfo = errorInfo;

			retval = callback.reactorOAuthCredentialEventCallback(reactorOAuthCredentialEvent);
			reactorOAuthCredentialEvent._errorInfo = errorInfoTemp;

			reactorOAuthCredentialEvent.returnToPool();

			if (retval != ReactorCallbackReturnCodes.SUCCESS)
			{
				// retval is not a valid ReactorReturnCodes.
				populateErrorInfo(errorInfo, retval, "Reactor.sendOAuthCredentialEventCallback", "retval of " + retval
						+ " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
				shutdown(errorInfo);
				return ReactorReturnCodes.FAILURE;
			}
		}

		return retval;
	}

	// returns ReactorCallbackReturnCodes and populates errorInfo if needed.
	int sendAndHandleChannelEventCallback(String location, int eventType, ReactorChannel reactorChannel,
			ReactorErrorInfo errorInfo)
	{

		int originalEventType = eventType;
		boolean closeChannelAfterCallback = false;
		boolean incrementWsbGroupIndex = false;
		ReactorWarmStandbyHandler warmStandbyHandler = null;
		ReactorWarmStandbyGroupImpl warmStandbyGroupImpl = null;

		if (eventType == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
		{
			if (reactorChannel.role().type() == ReactorRoleTypes.CONSUMER)
			{
				((ConsumerRole) (reactorChannel.role())).receivedFieldDictionaryResp(false);
				((ConsumerRole) (reactorChannel.role())).receivedEnumDictionaryResp(false);
			} else if (reactorChannel.role().type() == ReactorRoleTypes.NIPROVIDER)
			{
				((NIProviderRole) (reactorChannel.role())).receivedFieldDictionaryResp(false);
				((NIProviderRole) (reactorChannel.role())).receivedEnumDictionaryResp(false);
			}
		}

		if (eventType == ReactorChannelEventTypes.CHANNEL_DOWN
				|| eventType == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
		{
			if (reactorChannel.watchlist() == null)
			{
				_tmpState.clear();
				_tmpState.streamState(StreamStates.CLOSED_RECOVER);
				_tmpState.dataState(DataStates.SUSPECT);
				_tmpState.text().data("Channel closed.");

				/* Remove all associated tunnel streams. */
				for (TunnelStream tunnelStream = reactorChannel.tunnelStreamManager()._tunnelStreamList
						.start(TunnelStream.MANAGER_LINK); tunnelStream != null; tunnelStream = reactorChannel
								.tunnelStreamManager()._tunnelStreamList.forth(TunnelStream.MANAGER_LINK))
				{
					sendAndHandleTunnelStreamStatusEventCallback(location, reactorChannel, tunnelStream, null, null,
							_tmpState, errorInfo);
				}
			} else
			{
				if (reactorHandlesWarmStandby(reactorChannel) && this._reactorActive == true)
				{
					warmStandbyHandler = reactorChannel.warmStandByHandlerImpl;
					warmStandbyGroupImpl = warmStandbyHandler.currentWarmStandbyGroupImpl();

					// If Watchlist enabled and warm standby enabled, handle channel down
					if (eventType == ReactorChannelEventTypes.CHANNEL_DOWN)
					{

						boolean recoverChannel = false;
						int numberOfStandbyChannels = warmStandbyHandler.warmStandbyGroupList().get(reactorChannel.standByGroupListIndex).standbyServerList().size();
						ReactorWarmStandbyGroupImpl wsbChannelGroup = (ReactorWarmStandbyGroupImpl)warmStandbyHandler.warmStandbyGroupList().get(reactorChannel.standByGroupListIndex);
						
						/*
						 * If this is the initial connection channel for the WSB Group, keep this
						 * channel, and set it to move to the next WSB group when all of the other
						 * channels are gone
						 */
						if (reactorChannel.isStartingServerConfig)
						{
							// Checks whether there is an additional warm standby group or a channel list to
							// switch to.
							if ((warmStandbyHandler.currentWarmStandbyGroupIndex() + 1) < warmStandbyHandler
									.getConnectionOptions().reactorWarmStandbyGroupList().size())
							{
								warmStandbyHandler.setMoveToNextWSBGroupState();
								recoverChannel = true;
							} else if (warmStandbyHandler.getConnectionOptions().connectionList().size() > 0)
							{
								recoverChannel = true;
							}

							if (recoverChannel)
							{
								/*
								 * The initial starting channel is now dead, so move the WSB handler into a
								 * state where it's waiting for the other channels are closed before really
								 * reconnecting.
								 */
								warmStandbyHandler.setClosingStandbyChannelsState();
								eventType = ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING;
								reactorChannel.resetReconnectTimers();
								/* Reset the channel information here */
							}

							if (reactorChannel.selectableChannel() != null)
							{
								warmStandbyHandler.mainReactorChannelImpl().warmStandbyChannelInfo()
										.selectableChannelList().remove(reactorChannel.channel().selectableChannel());
							}
							
							if (!isWarmStandbyChannelClosed(warmStandbyHandler, reactorChannel))
							{
								/*
								 * Other channels are still connected, so send an FD_CHANGE event instead of
								 * DOWN or DOWN_RECONNECTING
								 */

								eventType = ReactorChannelEventTypes.FD_CHANGE;
							} else
							{
								if (eventType == ReactorChannelEventTypes.CHANNEL_DOWN)
								{
									reactorChannel.warmStandByHandlerImpl
											.mainChannelState(ReactorWarmStandbyHandlerChannelStateImpl.DOWN);
									reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().state(State.DOWN);
								} else
								{
									reactorChannel.warmStandByHandlerImpl.mainChannelState(
											ReactorWarmStandbyHandlerChannelStateImpl.DOWN_RECONNECTING);
									reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().state(State.DOWN_RECONNECTING);

								}
							}

							warmStandbyHandler.currentWarmStandbyGroupImpl().startingServerIsDown = true;
						} else
						{
							if ((warmStandbyHandler.currentWarmStandbyGroupIndex() + 1) < warmStandbyHandler
									.getConnectionOptions().reactorWarmStandbyGroupList().size()
									|| warmStandbyHandler.hasConnectionList())
							{
								eventType = ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING;
							}

							if (!isWarmStandbyChannelClosed(warmStandbyHandler, reactorChannel))
							{
								/*
								 * Other channels are still connected, so send an FD_CHANGE event instead of
								 * DOWN or DOWN_RECONNECTING
								 */
								if (reactorChannel.selectableChannel() != null)
								{
									warmStandbyHandler.mainReactorChannelImpl().warmStandbyChannelInfo()
											.selectableChannelList()
											.remove(reactorChannel.channel().selectableChannel());
								}

								eventType = ReactorChannelEventTypes.FD_CHANGE;
							}
							
							closeChannelAfterCallback = true;
							warmStandbyHandler.channelList().remove(reactorChannel);

							// Increment the closing count for the group here
							wsbChannelGroup.incrementClosingStandbyCount();
						}

						if (warmStandbyHandler.currentWarmStandbyGroupImpl().startingServerIsDown
								&& numberOfStandbyChannels == wsbChannelGroup.closingStandbyCount())
						{
							int state = warmStandbyHandler.warmStandbyHandlerState();
							state &= ~ReactorWarmStandbyHandlerState.CLOSING_STANDBY_CHANNELS;
							warmStandbyHandler.warmStandbyHandlerState(state);
							incrementWsbGroupIndex = true;
							
							if ((warmStandbyHandler.warmStandbyHandlerState()
									& ReactorWarmStandbyHandlerState.MOVE_TO_NEXT_WSB_GROUP) != 0)
							{
								warmStandbyHandler.warmStandbyHandlerState(
										ReactorWarmStandbyHandlerState.CONNECTING_TO_A_STARTING_SERVER);

								warmStandbyHandler.startingReactorChannel()
										.reactorChannelType(ReactorChannelType.WARM_STANDBY);
								warmStandbyHandler.startingReactorChannel().resetReconnectTimers();
								
								queueRequestsForWSBGroupRecovery(warmStandbyHandler, errorInfo);
							} else
							{
								warmStandbyHandler
										.warmStandbyHandlerState(ReactorWarmStandbyHandlerState.MOVE_TO_CHANNEL_LIST);
								warmStandbyHandler.startingReactorChannel()
										.reactorChannelType(ReactorChannelType.NORMAL);
								warmStandbyHandler.startingReactorChannel().resetReconnectTimers();
							}
						}
						
					} else if (eventType == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
					{
						if (reactorChannel.selectableChannel() != null)
						{
							reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().warmStandbyChannelInfo()
									.selectableChannelList().remove(reactorChannel.selectableChannel());
						}
						if ((reactorChannel.warmStandByHandlerImpl.warmStandbyHandlerState()
								& ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_LOGIN_RESPONSE) != 0 && !isWarmStandbyChannelClosed(reactorChannel.warmStandByHandlerImpl, reactorChannel))
							eventType = ReactorChannelEventTypes.FD_CHANGE;
					}
					
					/* If the channel has closed, handle the transitions for WSB */
					if (reactorChannel.channel() != null && reactorChannel.channel().state() == ChannelState.CLOSED)
					{
						if (warmStandbyGroupImpl.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
						{
							if (reactorChannel.isActiveServer)
							{
								ReactorChannel nextReactorChannel;

								/*
								 * Submits to a channel that belongs to the warm standby feature and it is
								 * active
								 */

								for (int i = 0; i < warmStandbyHandler.channelList().size(); i++)
								{
									nextReactorChannel = warmStandbyHandler.channelList().get(i);

									if (reactorChannel != nextReactorChannel && nextReactorChannel.channel() != null
											&& nextReactorChannel.channel().state() == State.UP.ordinal())
									{
										ReactorWarmStandbyEvent reactorWarmStandbyEvent = reactorWarmStandbyEventPool
												.getEvent(errorInfo);

										reactorChannel.warmStandByHandlerImpl
												.nextActiveReactorChannel(nextReactorChannel);

										reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.CHANGE_ACTIVE_TO_STANDBY_SERVER;
										reactorWarmStandbyEvent.reactorChannel = reactorChannel;
										reactorWarmStandbyEvent.nextReactorChannel = nextReactorChannel;

										sendWarmStandbyEvent(reactorChannel, reactorWarmStandbyEvent, errorInfo);
										break;
									}
								}

							} else
							{
								// Checks whether there is an active channel
								if (warmStandbyHandler.activeReactorChannel() == null)
								{
									boolean selectedAsActiveServer = false;
									if (warmStandbyGroupImpl.downloadConfigActiveServer == ReactorWarmStandbyGroupImpl.REACTOR_WSB_STARTING_SERVER_INDEX
											&& reactorChannel.isStartingServerConfig)
									{
										selectedAsActiveServer = true;
									} else if (!reactorChannel.isStartingServerConfig
											&& warmStandbyGroupImpl.downloadConfigActiveServer == reactorChannel.standByServerListIndex)
									{
										selectedAsActiveServer = true;
									}

									// This channel failed to connect so select other servers as an active server
									// instead
									if (selectedAsActiveServer)
									{
										ReactorChannel nextReactorChannel;

										// Submits to a channel that belongs to the warm standby feature and it is
										// active
										for (int i = 0; i < warmStandbyHandler.channelList().size(); ++i)
										{
											nextReactorChannel = warmStandbyHandler.channelList().get(i);
											if (reactorChannel != nextReactorChannel
													&& nextReactorChannel.channel() != null
													&& nextReactorChannel.channel().state() == ChannelState.ACTIVE)
											{
												ReactorWarmStandbyEvent reactorWarmStandbyEvent = reactorWarmStandbyEventPool
														.getEvent(errorInfo);

												reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.CHANGE_ACTIVE_TO_STANDBY_SERVER;
												reactorWarmStandbyEvent.reactorChannel = reactorChannel;
												reactorWarmStandbyEvent.nextReactorChannel = nextReactorChannel;

												sendWarmStandbyEvent(reactorChannel, reactorWarmStandbyEvent,
														errorInfo);
												break;
											}
										}
									}
								}
							}
						} else
						{
							ReactorWarmStandbyEvent reactorWarmStandbyEvent = reactorWarmStandbyEventPool
									.getEvent(errorInfo);

							// Per service based warm standby
							reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.CHANGE_ACTIVE_TO_STANDBY_SERVICE_CHANNEL_DOWN;
							reactorWarmStandbyEvent.reactorChannel = reactorChannel;

							sendWarmStandbyEvent(reactorChannel, reactorWarmStandbyEvent, errorInfo);
						}
					}
				} else
				{
					if(reactorChannel.warmStandByHandlerImpl != null)
					{
						if(eventType == ReactorChannelEventTypes.CHANNEL_DOWN)
						{
							reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().state(State.DOWN);
						}
						else
						{
							reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().state(State.DOWN_RECONNECTING);
						}
					}
				}
			}

			if (_reactorOptions.debuggerOptions().debugConnectionLevel())
			{
				debugger.writeDebugInfo(ReactorDebugger.CONNECTION_CHANNEL_DOWN, this.hashCode(),
						reactorChannel.hashCode(), ReactorDebugger.getChannelId(reactorChannel));
			}

		} else if (eventType == ReactorChannelEventTypes.CHANNEL_UP)
		{
			if (reactorHandlesWarmStandby(reactorChannel))
			{
				reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().warmStandbyChannelInfo()
						.selectableChannelList().add(reactorChannel.selectableChannel());

				if (reactorChannel.warmStandByHandlerImpl
						.mainChannelState() < ReactorWarmStandbyHandlerChannelStateImpl.UP)
				{
					reactorChannel.warmStandByHandlerImpl
							.mainChannelState(ReactorWarmStandbyHandlerChannelStateImpl.UP);
					reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().state(State.UP);
				} else
				{
					eventType = ReactorChannelEventTypes.FD_CHANGE;

					if ((reactorChannel.warmStandByHandlerImpl.ioCtlCodes()
							& ReactorWsbIoctlCodes.MAX_NUM_BUFFERS) != 0)
					{
						reactorChannel.channel().ioctl(IoctlCodes.MAX_NUM_BUFFERS,
								reactorChannel.warmStandByHandlerImpl.maxNumBuffers(), errorInfo.error());
					}

					if ((reactorChannel.warmStandByHandlerImpl.ioCtlCodes()
							& ReactorWsbIoctlCodes.NUM_GUARANTEED_BUFFERS) != 0)
					{
						reactorChannel.channel().ioctl(IoctlCodes.NUM_GUARANTEED_BUFFERS,
								reactorChannel.warmStandByHandlerImpl.numGuaranteedBuffers(), errorInfo.error());
					}

					if ((reactorChannel.warmStandByHandlerImpl.ioCtlCodes()
							& ReactorWsbIoctlCodes.HIGH_WATER_MARK) != 0)
					{
						reactorChannel.channel().ioctl(IoctlCodes.HIGH_WATER_MARK,
								reactorChannel.warmStandByHandlerImpl.highWaterMark(), errorInfo.error());
					}

					if ((reactorChannel.warmStandByHandlerImpl.ioCtlCodes()
							& ReactorWsbIoctlCodes.SYSTEM_WRITE_BUFFERS) != 0)
					{
						reactorChannel.channel().ioctl(IoctlCodes.SYSTEM_WRITE_BUFFERS,
								reactorChannel.warmStandByHandlerImpl.systemWriteBuffers(), errorInfo.error());
					}

					if ((reactorChannel.warmStandByHandlerImpl.ioCtlCodes()
							& ReactorWsbIoctlCodes.SYSTEM_READ_BUFFERS) != 0)
					{
						reactorChannel.channel().ioctl(IoctlCodes.SYSTEM_READ_BUFFERS,
								reactorChannel.warmStandByHandlerImpl.systemReadBuffers(), errorInfo.error());
					}

					if ((reactorChannel.warmStandByHandlerImpl.ioCtlCodes()
							& ReactorWsbIoctlCodes.COMPRESSION_THRESHOLD) != 0)
					{
						reactorChannel.channel().ioctl(IoctlCodes.COMPRESSION_THRESHOLD,
								reactorChannel.warmStandByHandlerImpl.compressionThreshold(), errorInfo.error());
					}
				}

			}
			else if(reactorChannel.warmStandByHandlerImpl != null)
			{
				reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().state(State.UP);
			}
		} else if (eventType == ReactorChannelEventTypes.CHANNEL_READY)
		{
			if (reactorHandlesWarmStandby(reactorChannel))
			{
				if (reactorChannel.warmStandByHandlerImpl
						.mainChannelState() == ReactorWarmStandbyHandlerChannelStateImpl.UP)
				{
					reactorChannel.warmStandByHandlerImpl
							.mainChannelState(ReactorWarmStandbyHandlerChannelStateImpl.READY);
					reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().state(State.READY);
				}
			}
			else if(reactorChannel.warmStandByHandlerImpl != null)
			{
				reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().state(State.READY);
			}
		}

		int retval = sendChannelEventCallback(eventType, reactorChannel, errorInfo);

		if (reactorHandlesWarmStandby(reactorChannel)
				&& (eventType == ReactorChannelEventTypes.FD_CHANGE || eventType == ReactorChannelEventTypes.CHANNEL_UP)
				&& reactorChannel.selectableChannel() != null)
		{
			if (originalEventType != ReactorChannelEventTypes.CHANNEL_UP)
				reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().warmStandbyChannelInfo()
						.oldSelectableChannelList().remove(reactorChannel.selectableChannel());
			else
				reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().warmStandbyChannelInfo()
						.oldSelectableChannelList().add(reactorChannel.selectableChannel());
		}

		/*
		 * Channel callback complete. If channel is not already closed(and is in a
		 * CHANNEL_DOWN or RECONNECTING STATE), notify worker.
		 */
		if (originalEventType == ReactorChannelEventTypes.CHANNEL_DOWN
				|| originalEventType == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
		{
			if(reactorChannel.watchlist() != null)
			{
				// If watchlist is on, it will send status messages to the tunnel streams (so
				// don't do it ourselves). 
				reactorChannel.watchlist().channelDown();
			}
			/* Now set the active channels to null, if the current channel is a warm standby active */
			if (reactorHandlesWarmStandby(reactorChannel) && this._reactorActive == true)
			{
				if (warmStandbyGroupImpl.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
				{
					if (reactorChannel.isActiveServer)
					{
						reactorChannel.isActiveServer = false;
						reactorChannel.warmStandByHandlerImpl.activeReactorChannel(null);
					}
				}
			}
			
			if(incrementWsbGroupIndex)
			{
				warmStandbyHandler.incrementWarmStandbyGroupIndex();

			}
			
			if (reactorChannel.state() != State.CLOSED)
			{
				sendWorkerEvent(WorkerEventTypes.CHANNEL_DOWN, reactorChannel);
			}
		}
		
		/* This channel is a secondary WSB channel, so we can clean it up here */
		if(closeChannelAfterCallback)
		{
			closeChannel(reactorChannel, errorInfo);
		}

		// check return code from callback.
		if (retval == ReactorCallbackReturnCodes.FAILURE)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.FAILURE was returned from reactorChannelEventCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval == ReactorCallbackReturnCodes.RAISE)
		{
			// RAISE is not a valid return code for the
			// reactorChannelEventCallback.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.RAISE is not a valid return code from reactorChannelEventCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;

		} else if (retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}

		return retval;
	}

	private int sendDefaultMsgCallback(ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg,
			WlRequest wlRequest)
	{
		/* Check to see if we're going to call the client back here. */
		boolean callbackToClient = warmStandbySendCallback(msg, wlRequest, reactorChannel);

		ReactorChannel callbackChannel = reactorChannel;

		if (callbackToClient == true)
		{
			ReactorMsgEvent reactorMsgEvent = ReactorFactory.createReactorMsgEvent();
			reactorMsgEvent.transportBuffer(transportBuffer);
			reactorMsgEvent.msg(msg);

			/*
			 * If this is a warm standby channel, set the callback channel to the main
			 * reactorChannel in the wsbHandler.
			 */
			if (reactorHandlesWarmStandby(reactorChannel))
			{
				callbackChannel = reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl();
				callbackChannel.selectableChannelFromChannel(reactorChannel.channel());
				callbackChannel.userSpecObj(reactorChannel.userSpecObj());
			}

			reactorMsgEvent.reactorChannel(callbackChannel);

			if (wlRequest != null)
			{
				reactorMsgEvent.streamInfo().serviceName(wlRequest.streamInfo().serviceName());
				reactorMsgEvent.streamInfo().userSpecObject(wlRequest.streamInfo().userSpecObject());
			} else
			{
				reactorMsgEvent.streamInfo().clear();
			}

			int retval = callbackChannel.role().defaultMsgCallback().defaultMsgCallback(reactorMsgEvent);
			reactorMsgEvent.returnToPool();

			return retval;
		} else
		{
			/*
			 * callbackToClient can only be false if warm standby is enabled. In this case,
			 * if the message is a status message, and the stream is
			 * CLOSED/CLOSED_RECOVER/REDIRECTED, cache the response in the closed message
			 * queue. These will be fanned out to the user if/when this channel becomes
			 * active.
			 */
			if (reactorChannel.warmStandByHandlerImpl.readMsgChannel() == reactorChannel)
			{
				if (msg != null && msg.msgClass() == MsgClasses.STATUS)
				{
					StatusMsg statusMsg = (StatusMsg) msg;

					if (statusMsg.state().streamState() == StreamStates.CLOSED
							|| statusMsg.state().streamState() == StreamStates.CLOSED_RECOVER
							|| statusMsg.state().streamState() == StreamStates.REDIRECTED)
					{
						ReactorWSRecoveryMsgInfo msgInfo = new ReactorWSRecoveryMsgInfo();

						msgInfo._containerType = DataTypes.NO_DATA;
						msgInfo._domainType = msg.domainType();
						msgInfo._streamId = msg.streamId();
						msgInfo._userSpecObject = wlRequest.streamInfo()._userSpecObject;

						msgInfo._flags |= StatusMsgFlags.HAS_STATE;
						msgInfo._msgState.streamState(StreamStates.CLOSED_RECOVER);
						msgInfo._msgState.dataState(statusMsg.state().dataState());
						msgInfo._msgState.code(statusMsg.state().code());

						if (statusMsg.state().text().length() != 0)
						{
							// Deep copy the state text
							ByteBuffer textBytes = ByteBuffer.allocate(statusMsg.state().text().length());
							statusMsg.state().text().copy(textBytes);
							msgInfo._msgState.text().data(textBytes);
						}

						if (wlRequest != null && !wlRequest.streamInfo()._serviceName.isEmpty())
						{
							ByteBuffer serviceBytes = ByteBuffer.allocate(wlRequest.streamInfo()._serviceName.length());
							serviceBytes.put(wlRequest.streamInfo()._serviceName.getBytes());
							msgInfo._serviceName.data(serviceBytes);
						}

						if (msg.msgKey().flags() != 0)
						{
							msgInfo._flags |= StatusMsgFlags.HAS_MSG_KEY;

							if ((msg.msgKey().flags() & MsgKeyFlags.HAS_SERVICE_ID) != 0)
							{
								msgInfo._msgKey.applyHasServiceId();
								msgInfo._msgKey.serviceId(msg.msgKey().serviceId());
							}

							if ((msg.msgKey().flags() & MsgKeyFlags.HAS_NAME) != 0)
							{
								// Deep copy the name
								ByteBuffer nameBytes = ByteBuffer.allocate(statusMsg.state().text().length());
								statusMsg.state().text().copy(nameBytes);
								msgInfo._msgKey.name().data(nameBytes);
								msgInfo._msgKey.applyHasName();
							}

							if ((msg.msgKey().flags() & MsgKeyFlags.HAS_NAME_TYPE) != 0)
							{
								msgInfo._msgKey.applyHasNameType();
								msgInfo._msgKey.nameType(msg.msgKey().nameType());
							}

							if ((msg.msgKey().flags() & MsgKeyFlags.HAS_FILTER) != 0)
							{
								msgInfo._msgKey.applyHasFilter();
								msgInfo._msgKey.filter(msg.msgKey().filter());
							}

							if ((msg.msgKey().flags() & MsgKeyFlags.HAS_IDENTIFIER) != 0)
							{
								msgInfo._msgKey.applyHasIdentifier();
								msgInfo._msgKey.identifier(msg.msgKey().identifier());
							}

						}

						reactorChannel._watchlistRecoveryMsgList.add(msgInfo);
					}
				}
			}
		}

		return ReactorCallbackReturnCodes.SUCCESS;
	}

	// adds WatchlistStreamInfo, returns ReactorCallbackReturnCodes and populates
	// errorInfo if needed.
	int sendAndHandleDefaultMsgCallback(String location, ReactorChannel reactorChannel, TransportBuffer transportBuffer,
			Msg msg, WlRequest wlRequest, ReactorErrorInfo errorInfo)
	{
		TunnelStream tunnelStream;

		_tempWlInteger.value(msg.streamId());
		if ((tunnelStream = reactorChannel.streamIdtoTunnelStreamTable().get(_tempWlInteger)) != null)
			return handleTunnelStreamMsg(reactorChannel, tunnelStream, transportBuffer, msg, errorInfo);

		int retval = sendDefaultMsgCallback(reactorChannel, transportBuffer, msg, wlRequest);

		// check return code from callback.
		if (retval == ReactorCallbackReturnCodes.FAILURE)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.FAILURE was returned from defaultMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval == ReactorCallbackReturnCodes.RAISE)
		{
			// RAISE is not a valid return code for the
			// reactorChannelEventCallback.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.RAISE is not a valid return code from defaultMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}
		return retval;
	}

	// returns ReactorCallbackReturnCodes and populates errorInfo if needed.
	private int sendAndHandleDefaultMsgCallback(String location, ReactorChannel reactorChannel,
			TransportBuffer transportBuffer, Msg msg, ReactorErrorInfo errorInfo)
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
		} else if (retval == ReactorCallbackReturnCodes.RAISE)
		{
			// RAISE is not a valid return code for the
			// reactorChannelEventCallback.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.RAISE is not a valid return code from defaultMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}
		return retval;
	}

	int sendTunnelStreamStatusEventCallback(ReactorChannel reactorChannel, TunnelStream tunnelStream,
			TransportBuffer transportBuffer, Msg msg, com.refinitiv.eta.codec.State state, LoginMsg loginMsg,
			ReactorErrorInfo errorInfo)
	{
		if (tunnelStream.statusEventCallback() == null)
		{
			return ReactorReturnCodes.FAILURE;
		}

		TunnelStreamStatusEvent tunnelStreamEvent = ReactorFactory.createTunnelStreamStatusEvent();

		_tempWlInteger.value(tunnelStream.streamId());
		if (state != null && state.streamState() != StreamStates.OPEN
				&& reactorChannel.streamIdtoTunnelStreamTable().containsKey(_tempWlInteger))
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

			if (_reactorOptions.debuggerOptions().debugTunnelStreamLevel())
			{
				debugger.writeDebugInfo(ReactorDebugger.TUNNELSTREAM_STREAM_CLOSE, this.hashCode(),
						reactorChannel.hashCode(), tunnelStream.streamId(),
						ReactorDebugger.getChannelId(reactorChannel));
			}
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
			TunnelStream tunnelStream, TransportBuffer transportBuffer, Msg msg, com.refinitiv.eta.codec.State state,
			ReactorErrorInfo errorInfo)
	{
		int retval = sendTunnelStreamStatusEventCallback(reactorChannel, tunnelStream, transportBuffer, msg, state,
				null, errorInfo);

		// check return code from callback.
		if (retval == ReactorCallbackReturnCodes.FAILURE)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.FAILURE was returned from tunnelStreamStatusEventCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval == ReactorCallbackReturnCodes.RAISE)
		{
			// RAISE is not a valid return code for the
			// tunnelStreamStatusEventCallback.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.RAISE is not a valid return code from tunnelStreamStatusEventCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}
		return retval;
	}

	private int sendTunnelStreamMsgCallback(ReactorChannel reactorChannel, TunnelStream tunnelStream,
			TransportBuffer transportBuffer, Msg msg, int containerType)
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
	int sendAndHandleTunnelStreamMsgCallback(String location, ReactorChannel reactorChannel, TunnelStream tunnelStream,
			TransportBuffer transportBuffer, Msg msg, int containerType, ReactorErrorInfo errorInfo)
	{
		int retval = sendTunnelStreamMsgCallback(reactorChannel, tunnelStream, transportBuffer, msg, containerType);

		// check return code from callback.
		if (retval == ReactorCallbackReturnCodes.FAILURE)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.FAILURE was returned from tunnelStreamDefaultMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval == ReactorCallbackReturnCodes.RAISE)
		{
			// RAISE is not a valid return code for the
			// tunnelStreamDefaultMsgCallback.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.RAISE is not a valid return code from tunnelStreamDefaultMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}
		return retval;
	}

	private int sendQueueMsgCallback(ReactorChannel reactorChannel, TunnelStream tunnelStream,
			TransportBuffer transportBuffer, Msg msg, QueueMsg queueMsg)
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
		} else
		{
			// callback is undefined, raise it to tunnelStreamDefaultMsgCallback.
			retval = ReactorCallbackReturnCodes.RAISE;
		}

		return retval;
	}

	// returns ReactorCallbackReturnCodes and populates errorInfo if needed.
	int sendAndHandleQueueMsgCallback(String location, ReactorChannel reactorChannel, TunnelStream tunnelStream,
			TransportBuffer transportBuffer, Msg msg, QueueMsg queueMsg, ReactorErrorInfo errorInfo)
	{
		int retval = sendQueueMsgCallback(reactorChannel, tunnelStream, transportBuffer, msg, queueMsg);

		// check return code from callback.
		if (retval == ReactorCallbackReturnCodes.FAILURE)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.FAILURE was returned from queueMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " was returned from tunnelStreamQueueMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}
		return retval;
	}

	private int sendLoginMsgCallback(ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg,
			LoginMsg loginMsg, WlRequest wlRequest)
	{
		int retval;
		boolean sendCallback = false;
		RDMLoginMsgCallback callback = null;

		switch (reactorChannel.role().type())
		{
		case ReactorRoleTypes.CONSUMER:
			callback = ((ConsumerRole) reactorChannel.role()).loginMsgCallback();
			break;
		case ReactorRoleTypes.PROVIDER:
			callback = ((ProviderRole) reactorChannel.role()).loginMsgCallback();
			break;
		case ReactorRoleTypes.NIPROVIDER:
			callback = ((NIProviderRole) reactorChannel.role()).loginMsgCallback();
			break;
		default:
			break;
		}

		RDMLoginMsgEvent rdmLoginMsgEvent = ReactorFactory.createRDMLoginMsgEvent();
		rdmLoginMsgEvent.reactorChannel(reactorChannel);
		rdmLoginMsgEvent.transportBuffer(transportBuffer);
		rdmLoginMsgEvent.msg(msg);
		rdmLoginMsgEvent.rdmLoginMsg(loginMsg);
		if (wlRequest != null)
		{
			rdmLoginMsgEvent.streamInfo().serviceName(wlRequest.streamInfo().serviceName());
			rdmLoginMsgEvent.streamInfo().userSpecObject(wlRequest.streamInfo().userSpecObject());
		} else
		{
			rdmLoginMsgEvent.streamInfo().clear();
		}

		// Check and handle warm standby feature on Login Refresh

		if (reactorHandlesWarmStandby(reactorChannel))
		{
			if (rdmLoginMsgEvent.rdmLoginMsg().rdmMsgType() == LoginMsgType.REFRESH)
			{
				LoginRefresh refreshRdmMsg = (LoginRefresh) rdmLoginMsgEvent._loginMsg;
				if (refreshRdmMsg.state().streamState() == StreamStates.OPEN
						&& refreshRdmMsg.state().dataState() == DataStates.OK)
				{
					if (reactorChannel.warmStandByHandlerImpl.rdmLoginState().streamState() == StreamStates.OPEN
							&& reactorChannel.warmStandByHandlerImpl.rdmLoginState().dataState() == DataStates.SUSPECT)
					{
						refreshRdmMsg.state().copy(reactorChannel.warmStandByHandlerImpl.rdmLoginState());
						sendCallback = true;
					}
				}

				if (refreshRdmMsg.features().checkHasSupportStandby())
				{
					if(refreshRdmMsg.features().supportStandby() == 1)
					{
						// Check if SupportStandbyMode is set
						if (refreshRdmMsg.features().checkHasSupportStandbyMode())
						{
							if ((refreshRdmMsg.features().supportStandbyMode()
									& ReactorWarmStandbyMode.SERVICE_BASED) != ReactorWarmStandbyMode.SERVICE_BASED
									&& reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl()
											.warmStandbyMode() == ReactorWarmStandbyMode.SERVICE_BASED)
							{
								ReactorWarmStandbyEvent reactorWarmStandbyEvent = reactorWarmStandbyEventPool
										.getEvent(rdmLoginMsgEvent.errorInfo());
	
								populateErrorInfo(rdmLoginMsgEvent.errorInfo(), ReactorReturnCodes.FAILURE,
										"sendLoginMsgCallback",
										"The login response does not support Service-based warm standby functionality.");
								reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.REMOVE_SERVER_FROM_WSB_GROUP;
	
								sendWarmStandbyEvent(reactorChannel, reactorWarmStandbyEvent, rdmLoginMsgEvent.errorInfo());
								/* Return SUCCESS here to avoid closing the reactor or raising this callback */
								rdmLoginMsgEvent.returnToPool();
								return ReactorReturnCodes.SUCCESS;
							} else if ((refreshRdmMsg.features().supportStandbyMode()
									& ReactorWarmStandbyMode.LOGIN_BASED) != ReactorWarmStandbyMode.LOGIN_BASED
									&& reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl()
											.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
							{
								ReactorWarmStandbyEvent reactorWarmStandbyEvent = reactorWarmStandbyEventPool
										.getEvent(rdmLoginMsgEvent.errorInfo());
								reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.REMOVE_SERVER_FROM_WSB_GROUP;
								populateErrorInfo(rdmLoginMsgEvent.errorInfo(), ReactorReturnCodes.FAILURE,
										"sendLoginMsgCallback",
										"The login response does not support Login-based warm standby functionality.");
								sendWarmStandbyEvent(reactorChannel, reactorWarmStandbyEvent, rdmLoginMsgEvent.errorInfo());
								/* Return SUCCESS here to avoid closing the reactor or raising this callback */
								rdmLoginMsgEvent.returnToPool();
	
								return ReactorReturnCodes.SUCCESS;
							}
						}
					}
					else
					{
						ReactorWarmStandbyEvent reactorWarmStandbyEvent = reactorWarmStandbyEventPool
								.getEvent(rdmLoginMsgEvent.errorInfo());
						reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.REMOVE_SERVER_FROM_WSB_GROUP;
						populateErrorInfo(rdmLoginMsgEvent.errorInfo(), ReactorReturnCodes.FAILURE,
								"sendLoginMsgCallback",
								"The login response does not support the warm standby functionality.");
						sendWarmStandbyEvent(reactorChannel, reactorWarmStandbyEvent, rdmLoginMsgEvent.errorInfo());
						/* Return SUCCESS here to avoid closing the reactor or raising this callback */
						rdmLoginMsgEvent.returnToPool();

						return ReactorReturnCodes.SUCCESS;
					}

					if ((reactorChannel.warmStandByHandlerImpl.warmStandbyHandlerState()
							& ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_LOGIN_RESPONSE) == 0)
					{
						sendCallback = true;
						/* Copy the response to the warmStandbyHandler */
						reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh(refreshRdmMsg);

						reactorChannel.warmStandByHandlerImpl.setPrimaryLoginResponseState();

						/*
						 * This connection is now the active, since it is the first one to get a login
						 * response with LOGIN_BASED WSB, so send the active consumer connection status
						 * message
						 */

						if (reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl()
								.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
						{

							reactorChannel._loginConsumerStatus.clear();
							reactorChannel._loginConsumerStatus.streamId(msg.streamId());
							reactorChannel._loginConsumerStatus
									.flags(LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO);
							reactorChannel._loginConsumerStatus.warmStandbyInfo()
									.warmStandbyMode(Login.ServerTypes.ACTIVE);

							/*
							 * Write directly to the channel without involving the watchlist or wsb message
							 * queues
							 */
							if (submitChannel(reactorChannel, reactorChannel._loginConsumerStatus, reactorSubmitOptions, rdmLoginMsgEvent.errorInfo()) < ReactorReturnCodes.SUCCESS)
							{
								if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client
																														// channel
								{
									reactorChannel.state(State.DOWN_RECONNECTING);
									sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
											ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel,
											rdmLoginMsgEvent.errorInfo());
									{
										/* Return SUCCESS here to avoid closing the reactor or raising this callback */
										rdmLoginMsgEvent.returnToPool();

										return ReactorReturnCodes.SUCCESS;
									}
								} else // server channel or no more retries
								{
									reactorChannel.state(State.DOWN);
									sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
											ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel,
											rdmLoginMsgEvent.errorInfo());
									/* Return SUCCESS here to avoid closing the reactor or raising this callback */
									rdmLoginMsgEvent.returnToPool();
									return ReactorReturnCodes.SUCCESS;
								}
							}

							reactorChannel.isActiveServer = true;
							reactorChannel.warmStandByHandlerImpl.activeReactorChannel(reactorChannel);

						}

					} else
					{
						/*
						 * This is from a secondary connection, so check to see if the incoming login's
						 * options match. Since this has already been checked by the watchlist, we only
						 * really need to check for: SingleOpen(match option) AllowSuspect(match option)
						 * provide perm exp provide perm profile support post
						 * 
						 */
						boolean responseMatch = true;
						
						if(refreshRdmMsg.checkHasAttrib())
						{
							if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().checkHasAttrib())
							{
								if (refreshRdmMsg.attrib().checkHasSingleOpen())
								{
									if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().attrib().checkHasSingleOpen())
									{
										if(refreshRdmMsg.attrib().singleOpen() != reactorChannel.warmStandByHandlerImpl
											.rdmLoginRefresh().attrib().singleOpen())
										{
											responseMatch = false;
										}
									}
									else
									{
										responseMatch = false;
									}
								}
								else if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().attrib().checkHasSingleOpen())
								{
									responseMatch = false;
								}
								
								if (refreshRdmMsg.attrib().checkHasAllowSuspectData())
								{
									if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().attrib().checkHasAllowSuspectData())
									{
										if(refreshRdmMsg.attrib().allowSuspectData() != reactorChannel.warmStandByHandlerImpl
											.rdmLoginRefresh().attrib().allowSuspectData())
										{
											responseMatch = false;
										}
									}
									else
									{
										responseMatch = false;
									}
								}
								else if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().attrib().checkHasAllowSuspectData())
								{
									responseMatch = false;
								}
		
								if (refreshRdmMsg.attrib().checkHasProvidePermissionExpressions())
								{
									if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().attrib().checkHasProvidePermissionExpressions())
									{
										if(refreshRdmMsg.attrib().providePermissionExpressions() != reactorChannel.warmStandByHandlerImpl
											.rdmLoginRefresh().attrib().providePermissionExpressions())
										{
											responseMatch = false;
										}
									}
									else
									{
										responseMatch = false;
									}
								}
								else if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().attrib().checkHasProvidePermissionExpressions())
								{
									responseMatch = false;
								}
		
								if (refreshRdmMsg.attrib().checkHasProvidePermissionProfile())
								{
									if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().attrib().checkHasProvidePermissionProfile())
									{
										if(refreshRdmMsg.attrib().providePermissionProfile() != reactorChannel.warmStandByHandlerImpl
											.rdmLoginRefresh().attrib().providePermissionProfile())
										{
											responseMatch = false;
										}
									}
									else
									{
										responseMatch = false;
									}
								}
								else if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().attrib().checkHasProvidePermissionProfile())
								{
									responseMatch = false;
								}
		
								if (refreshRdmMsg.attrib().checkHasProvidePermissionProfile())
								{
									if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().attrib().checkHasProvidePermissionProfile())
									{
										if(refreshRdmMsg.attrib().providePermissionProfile() != reactorChannel.warmStandByHandlerImpl
											.rdmLoginRefresh().attrib().providePermissionProfile())
										{
											responseMatch = false;
										}
									}
									else
									{
										responseMatch = false;
									}
								}
								else if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().attrib().checkHasProvidePermissionProfile())
								{
									responseMatch = false;
								}
							}
						}
						else if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().checkHasAttrib())
						{
							responseMatch = false;
						}
						
						if(refreshRdmMsg.checkHasFeatures())
						{
							if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().checkHasFeatures())
							{
								if (refreshRdmMsg.features().checkHasSupportPost())
								{
									if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().features().checkHasSupportPost())
									{
										if(refreshRdmMsg.features().supportOMMPost() != reactorChannel.warmStandByHandlerImpl
											.rdmLoginRefresh().features().supportOMMPost())
										{
											responseMatch = false;
										}
									}
									else
									{
										responseMatch = false;
									}
								}
								else if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().features().checkHasSupportPost())
								{
									responseMatch = false;
								}
							}
						}
						else if(reactorChannel.warmStandByHandlerImpl.rdmLoginRefresh().checkHasFeatures())
						{
							responseMatch = false;
						}
						
						if (responseMatch == false)
						{
							ReactorWarmStandbyEvent reactorWarmStandbyEvent = reactorWarmStandbyEventPool
									.getEvent(rdmLoginMsgEvent.errorInfo());
							reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.REMOVE_SERVER_FROM_WSB_GROUP;
							populateErrorInfo(rdmLoginMsgEvent.errorInfo(), ReactorReturnCodes.FAILURE,
									"sendLoginMsgCallback",
									"The login response does not support the same features as the primary warm standby login.");
							sendWarmStandbyEvent(reactorChannel, reactorWarmStandbyEvent, rdmLoginMsgEvent.errorInfo());
							/* Return SUCCESS here to avoid closing the reactor or raising this callback */
							rdmLoginMsgEvent.returnToPool();
							return ReactorReturnCodes.SUCCESS;
						}

						if (reactorChannel.warmStandByHandlerImpl.activeReactorChannel() != null)
						{
							if (reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl()
									.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
							{
								reactorChannel._loginConsumerStatus.clear();
								reactorChannel._loginConsumerStatus.streamId(msg.streamId());
								reactorChannel._loginConsumerStatus
										.flags(LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO);
								reactorChannel._loginConsumerStatus.warmStandbyInfo()
										.warmStandbyMode(Login.ServerTypes.STANDBY);

								/*
								 * Write directly to the channel without involving the watchlist or wsb message
								 * queues
								 */
								if (submitChannel(reactorChannel, reactorChannel._loginConsumerStatus,
										reactorSubmitOptions, rdmLoginMsgEvent.errorInfo()) < ReactorReturnCodes.SUCCESS)
								{
									if (reactorChannel.server() == null
											&& !reactorChannel.recoveryAttemptLimitReached()) // client channel
									{
										reactorChannel.state(State.DOWN_RECONNECTING);
										sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
												ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel,
												rdmLoginMsgEvent.errorInfo());
										{
											/*
											 * Return SUCCESS here to avoid closing the reactor or raising this callback
											 */
											rdmLoginMsgEvent.returnToPool();
											return ReactorReturnCodes.SUCCESS;
										}
									} else // server channel or no more retries
									{
										reactorChannel.state(State.DOWN);
										sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
												ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel,
												rdmLoginMsgEvent.errorInfo());
										/* Return SUCCESS here to avoid closing the reactor or raising this callback */
										rdmLoginMsgEvent.returnToPool();
										return ReactorReturnCodes.SUCCESS;
									}
								}
							}
						} else
						{
							if (reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl()
									.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
							{
								/*
								 * There is no active channel, so the current server will become the new active
								 */
								reactorChannel._loginConsumerStatus.clear();
								reactorChannel._loginConsumerStatus.streamId(msg.streamId());
								reactorChannel._loginConsumerStatus
										.flags(LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO);
								reactorChannel._loginConsumerStatus.warmStandbyInfo()
										.warmStandbyMode(Login.ServerTypes.ACTIVE);

								if (reactorChannel.submit(reactorChannel._loginConsumerStatus, reactorSubmitOptions,
										rdmLoginMsgEvent.errorInfo()) < ReactorReturnCodes.SUCCESS)
								{
									if (reactorChannel.server() == null
											&& !reactorChannel.recoveryAttemptLimitReached()) // client channel
									{
										reactorChannel.state(State.DOWN_RECONNECTING);
										sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
												ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel,
												rdmLoginMsgEvent.errorInfo());
										{
											/*
											 * Return SUCCESS here to avoid closing the reactor or raising this callback
											 */
											rdmLoginMsgEvent.returnToPool();
											return ReactorReturnCodes.SUCCESS;
										}
									} else // server channel or no more retries
									{
										reactorChannel.state(State.DOWN);
										sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
												ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel,
												rdmLoginMsgEvent.errorInfo());
										/* Return SUCCESS here to avoid closing the reactor or raising this callback */
										rdmLoginMsgEvent.returnToPool();
										return ReactorReturnCodes.SUCCESS;
									}
								}

								reactorChannel.isActiveServer = true;
								reactorChannel.warmStandByHandlerImpl.activeReactorChannel(reactorChannel);
							}
						}
					}
				} else
				{
					/*
					 * Warm standby is not supported by this connection, so remove it from the WSB
					 * group.
					 */
					ReactorWarmStandbyEvent reactorWarmStandbyEvent = reactorWarmStandbyEventPool
							.getEvent(rdmLoginMsgEvent.errorInfo());
					populateErrorInfo(rdmLoginMsgEvent.errorInfo(), ReactorReturnCodes.FAILURE, "sendLoginMsgCallback",
							"The login response does not support warm standby functionality.");
					reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.REMOVE_SERVER_FROM_WSB_GROUP;

					sendWarmStandbyEvent(reactorChannel, reactorWarmStandbyEvent, rdmLoginMsgEvent.errorInfo());
					/* Return SUCCESS here to avoid closing the reactor or raising this callback */
					rdmLoginMsgEvent.returnToPool();
					return ReactorReturnCodes.SUCCESS;
				}

			} else if (rdmLoginMsgEvent.rdmLoginMsg().rdmMsgType() == LoginMsgType.STATUS)
			{
				LoginStatus statusRdmMsg = (LoginStatus) rdmLoginMsgEvent.rdmLoginMsg();
				if (isWarmStandbyChannelClosed(reactorChannel.warmStandByHandlerImpl, null))
				{
					sendCallback = true;
					statusRdmMsg.state().copy(reactorChannel.warmStandByHandlerImpl.rdmLoginState());
				} else
				{
					if (statusRdmMsg.state().streamState() == StreamStates.CLOSED)
					{
						ReactorWarmStandbyEvent reactorWarmStandbyEvent = reactorWarmStandbyEventPool
								.getEvent(rdmLoginMsgEvent.errorInfo());
						populateErrorInfo(rdmLoginMsgEvent.errorInfo(), ReactorReturnCodes.FAILURE,
								"sendLoginMsgCallback", "Server has rejected the Login request.");
						reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.REMOVE_SERVER_FROM_WSB_GROUP;

						sendWarmStandbyEvent(reactorChannel, reactorWarmStandbyEvent, rdmLoginMsgEvent.errorInfo());
					}

					/* If this is the last channel, notify the application */
					if (reactorChannel.warmStandByHandlerImpl.channelList().size() == 1)
					{
						sendCallback = true;

						/*
						 * Set the watchlist streamstate to open/suspect so if another channel is in
						 * recovery, it can reconnect and become the new active
						 */
						reactorChannel.warmStandByHandlerImpl.rdmLoginState().streamState(StreamStates.OPEN);
						reactorChannel.warmStandByHandlerImpl.rdmLoginState().dataState(DataStates.SUSPECT);
					}
				}
			}

			if (sendCallback)
			{
				reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl()
						.selectableChannelFromChannel(reactorChannel.channel());
				reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl()
						.userSpecObj(reactorChannel.userSpecObj());
				rdmLoginMsgEvent.reactorChannel(reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl());
			}
		} else
		{
			sendCallback = true;
		}

		if (callback != null && sendCallback)
		{
			if (reactorHandlesWarmStandby(reactorChannel) == true)
			{
				rdmLoginMsgEvent.reactorChannel(reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl());
				if (reactorChannel.channel() != null)
					reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl()
							.selectableChannelFromChannel(reactorChannel.channel());
				reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl()
						.userSpecObj(reactorChannel.userSpecObj());
			}
			retval = callback.rdmLoginMsgCallback(rdmLoginMsgEvent);
		} else if (sendCallback)
		{
			// callback is undefined, raise it to defaultMsgCallback.
			retval = ReactorCallbackReturnCodes.RAISE;
		} else
		{
			retval = ReactorCallbackReturnCodes.SUCCESS;
		}

		rdmLoginMsgEvent.returnToPool();

		return retval;
	}

	// adds WatchlistStreamInfo, returns ReactorCallbackReturnCodes and populates
	// errorInfo if needed
	int sendAndHandleLoginMsgCallback(String location, ReactorChannel reactorChannel, TransportBuffer transportBuffer,
			Msg msg, LoginMsg loginMsg, WlRequest wlRequest, ReactorErrorInfo errorInfo)
	{
		int retval = sendLoginMsgCallback(reactorChannel, transportBuffer, msg, loginMsg, wlRequest);

		// check return code from callback.
		if (retval == ReactorCallbackReturnCodes.FAILURE)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.FAILURE was returned from rdmLoginMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " was returned from rdmLoginMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}
		return retval;
	}

	// returns ReactorCallbackReturnCodes and populates errorInfo if needed.
	private int sendAndHandleLoginMsgCallback(String location, ReactorChannel reactorChannel,
			TransportBuffer transportBuffer, Msg msg, LoginMsg loginMsg, ReactorErrorInfo errorInfo)
	{
		int retval = sendLoginMsgCallback(reactorChannel, transportBuffer, msg, loginMsg, null);

		// check return code from callback.
		if (retval == ReactorCallbackReturnCodes.FAILURE)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.FAILURE was returned from rdmLoginMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " was returned from rdmLoginMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}
		return retval;
	}

	private int sendDirectoryMsgCallback(ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg,
			DirectoryMsg directoryMsg, WlRequest wlRequest)
	{
		int retval;
		RDMDirectoryMsgCallback callback = null;

		switch (reactorChannel.role().type())
		{
		case ReactorRoleTypes.CONSUMER:
			callback = ((ConsumerRole) reactorChannel.role()).directoryMsgCallback();
			break;
		case ReactorRoleTypes.PROVIDER:
			callback = ((ProviderRole) reactorChannel.role()).directoryMsgCallback();
			break;
		case ReactorRoleTypes.NIPROVIDER:
			// no directory callback for NIProvider.
			break;
		default:
			break;
		}

		if (callback != null)
		{

			ReactorChannel callbackChannel = reactorChannel;

			RDMDirectoryMsgEvent rdmDirectoryMsgEvent = ReactorFactory.createRDMDirectoryMsgEvent();
			rdmDirectoryMsgEvent.transportBuffer(transportBuffer);
			rdmDirectoryMsgEvent.msg(msg);
			rdmDirectoryMsgEvent.rdmDirectoryMsg(directoryMsg);

			/*
			 * If this is a warm standby channel, set the callback channel to the main
			 * reactorChannel in the wsbHandler.
			 */
			if (reactorHandlesWarmStandby(reactorChannel))
			{
				callbackChannel = reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl();
				callbackChannel.selectableChannelFromChannel(reactorChannel.channel());
				callbackChannel.userSpecObj(reactorChannel.userSpecObj());
				boolean isChannelClosed = isWarmStandbyChannelClosed(reactorChannel.warmStandByHandlerImpl, null);
				ReactorWarmStandbyGroupImpl wsbGroup = reactorChannel.warmStandByHandlerImpl
						.currentWarmStandbyGroupImpl();

				if ((reactorChannel.warmStandByHandlerImpl.warmStandbyHandlerState()
						& ReactorWarmStandbyHandlerState.RECEIVED_SECONDARY_DIRECTORY_RESPONSE) == 0
						&& !isChannelClosed)
				{
					/*
					 * We have just received the initial source directory response, so we do not
					 * need to aggregate anything and can just send out the response received from
					 * the provider. Clear the updated service list.
					 */

					for (int i = 0; i < wsbGroup._updateServiceList.size(); i++)
					{
						wsbGroup._updateServiceList.get(i).updateServiceFilter = ServiceFlags.NONE;
					}

					wsbGroup._updateServiceList.clear();
				} else
				{
					/*
					 * The current connection for this reactorChannel has gone down, so we will need
					 * to remove the generated service status from the watchlist from the warm
					 * standby service cache
					 */
					if (reactorChannel.channel() == null || reactorChannel.channel().state() != ChannelState.ACTIVE)
					{
						Iterator<WlService> serviceIter = reactorChannel
								.watchlist()._directoryHandler._serviceCache._serviceList.iterator();
						while (serviceIter.hasNext())
						{
							WlService watchlistService = serviceIter.next();
							ReactorWSBService wsbService = wsbGroup._perServiceById.get(watchlistService._tableKey);

							if (wsbService != null)
							{
								// Check to see if other channels have this service
								if (wsbService.channels.contains(reactorChannel))
								{
									wsbService.channels.remove(reactorChannel);
								}

								if (wsbService.channels.size() == 0)
								{
									wsbService.serviceAction = MapEntryActions.DELETE;
									wsbGroup._updateServiceList.add(wsbService);
								}
							}
						}
					}


					if (wsbGroup._updateServiceList.size() > 0)
					{
						DirectoryUpdate updateMsg = (DirectoryUpdate) reactorChannel._wsbDirectoryUpdate;
						updateMsg.clear();
						updateMsg.streamId(directoryMsg.streamId());
	
						for (int i = 0; i < wsbGroup._updateServiceList.size(); i++)
						{
							ReactorWSBService service = wsbGroup._updateServiceList.get(i);
							service.serviceInfo.flags(service.updateServiceFilter);
							service.serviceInfo.action(service.serviceAction);
	
							updateMsg.serviceList().add(service.serviceInfo);
	
							service.updateServiceFilter = ServiceFlags.NONE;
	
							if (service.serviceInfo.action() == MapEntryActions.DELETE)
							{
								wsbGroup._perServiceById.remove(service.serviceId);
							}
						}						
						
						rdmDirectoryMsgEvent.transportBuffer(null);
						rdmDirectoryMsgEvent.msg(null);
						rdmDirectoryMsgEvent.rdmDirectoryMsg(updateMsg);
					} else
					{
						// Nothing has changed, so there's nothing to give to the user, just return SUCCESS.
						return ReactorCallbackReturnCodes.SUCCESS;
					}
				}
			}

			rdmDirectoryMsgEvent.reactorChannel(callbackChannel);

			if (wlRequest != null)
			{
				rdmDirectoryMsgEvent.streamInfo().serviceName(wlRequest.streamInfo().serviceName());
				rdmDirectoryMsgEvent.streamInfo().userSpecObject(wlRequest.streamInfo().userSpecObject());
			} else
			{
				rdmDirectoryMsgEvent.streamInfo().clear();
			}

			retval = callback.rdmDirectoryMsgCallback(rdmDirectoryMsgEvent);
			// We can now clear the wsb group directory update list.
			if (reactorHandlesWarmStandby(reactorChannel))
			{
				reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl()._updateServiceList.clear();
			}
			
			rdmDirectoryMsgEvent.returnToPool();
		} else
		{
			// callback is undefined, raise it to defaultMsgCallback.
			retval = ReactorCallbackReturnCodes.RAISE;
		}

		return retval;
	}

	// returns ReactorCallbackReturnCodes and populates errorInfo if needed.
	int sendAndHandleDirectoryMsgCallback(String location, ReactorChannel reactorChannel,
			TransportBuffer transportBuffer, Msg msg, DirectoryMsg directoryMsg, WlRequest wlRequest,
			ReactorErrorInfo errorInfo)
	{
		int retval = sendDirectoryMsgCallback(reactorChannel, transportBuffer, msg, directoryMsg, wlRequest);

		// check return code from callback.
		if (retval == ReactorCallbackReturnCodes.FAILURE)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.FAILURE was returned from rdmDirectoryMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " was returned from rdmDirectoryMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}
		return retval;
	}

	// returns ReactorCallbackReturnCodes and populates errorInfo if needed.
	private int sendAndHandleDirectoryMsgCallback(String location, ReactorChannel reactorChannel,
			TransportBuffer transportBuffer, Msg msg, DirectoryMsg directoryMsg, ReactorErrorInfo errorInfo)
	{
		int retval = sendDirectoryMsgCallback(reactorChannel, transportBuffer, msg, directoryMsg, null);

		// check return code from callback.
		if (retval == ReactorCallbackReturnCodes.FAILURE)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.FAILURE was returned from rdmDirectoryMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " was returned from rdmDirectoryMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}
		return retval;
	}

	private int sendDictionaryMsgCallback(ReactorChannel reactorChannel, TransportBuffer transportBuffer, Msg msg,
			DictionaryMsg dictionaryMsg, WlRequest wlRequest, ReactorErrorInfo errorInfo)
	{
		int retval;
		RDMDictionaryMsgCallback callback = null;
		boolean callbackToClient = warmStandbySendCallback(msg, wlRequest, reactorChannel);

		switch (reactorChannel.role().type())
		{
		case ReactorRoleTypes.CONSUMER:
			callback = ((ConsumerRole) reactorChannel.role()).dictionaryMsgCallback();
			break;
		case ReactorRoleTypes.PROVIDER:
			callback = ((ProviderRole) reactorChannel.role()).dictionaryMsgCallback();
			break;
		case ReactorRoleTypes.NIPROVIDER:
			callback = ((NIProviderRole) reactorChannel.role()).dictionaryMsgCallback();
			break;
		default:
			break;
		}

		if (callback != null)
		{
			if (reactorHandlesWarmStandby(reactorChannel) && dictionaryMsg.rdmMsgType() == DictionaryMsgType.REFRESH)
			{
				DictionaryRefresh dictRefresh = (DictionaryRefresh) dictionaryMsg;
				if (dictRefresh.checkHasInfo())
				{
					switch (dictRefresh.dictionaryType())
					{
					case Dictionary.Types.FIELD_DEFINITIONS:
					{
						/* CallbackToClient is set to true if this the active server for warm standby */
						if (callbackToClient)
						{
							if ((reactorChannel.warmStandByHandlerImpl.warmStandbyHandlerState()
									& ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_FIELD_DICTIONARY_RESPONSE) == 0)
							{
								reactorChannel.warmStandByHandlerImpl
										.rdmFieldVersion(getMajorDictionaryVersion(dictRefresh.version()));
								reactorChannel.warmStandByHandlerImpl.setReceivedPrimaryFieldDictionaryState();
							}
						} else
						{
							if ((reactorChannel.warmStandByHandlerImpl.warmStandbyHandlerState()
									& ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_FIELD_DICTIONARY_RESPONSE) != 0)
							{
								/* This is a standby connection */
								int majorVersion = getMajorDictionaryVersion(dictRefresh.version());

								if (reactorChannel.warmStandByHandlerImpl.rdmFieldVersion() != majorVersion)
								{
									ReactorWarmStandbyEvent reactorWarmStandbyEvent = reactorWarmStandbyEventPool
											.getEvent(errorInfo);
									populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
											"sendDictionaryMsgCallback",
											"The major version of field dictionary response from standby server does not match with the primary server.");
									reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.REMOVE_SERVER_FROM_WSB_GROUP;

									if (sendWarmStandbyEvent(reactorChannel, reactorWarmStandbyEvent,
											errorInfo) == false)
										return ReactorReturnCodes.FAILURE;
								}
							}
						}
						break;
					}
					case Dictionary.Types.ENUM_TABLES:
					{
						if (callbackToClient)
						{
							if ((reactorChannel.warmStandByHandlerImpl.warmStandbyHandlerState()
									& ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_ENUM_DICTIONARY_RESPONSE) == 0)
							{
								reactorChannel.warmStandByHandlerImpl
										.rdmEnumTypeVersion(getMajorDictionaryVersion(dictRefresh.version()));
								reactorChannel.warmStandByHandlerImpl.setReceivedPrimaryEnumDictionaryState();
							} else if ((reactorChannel.warmStandByHandlerImpl.warmStandbyHandlerState()
									& ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_ENUM_DICTIONARY_RESPONSE) != 0)
							{
								int majorVersion = getMajorDictionaryVersion(dictRefresh.version());

								if (reactorChannel.warmStandByHandlerImpl.rdmEnumTypeVersion() != majorVersion)
								{
									ReactorWarmStandbyEvent reactorWarmStandbyEvent = reactorWarmStandbyEventPool
											.getEvent(errorInfo);
									populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
											"sendDictionaryMsgCallback",
											"The major version of enum type dictionary response from standby server does not match with the primary server.");
									reactorWarmStandbyEvent.eventType = ReactorWarmStandbyEventTypes.REMOVE_SERVER_FROM_WSB_GROUP;

									if (sendWarmStandbyEvent(reactorChannel, reactorWarmStandbyEvent,
											errorInfo) == false)
										return ReactorReturnCodes.FAILURE;
								}
							}
						}
						break;
					}
					}
				}
			}

			if (callbackToClient)
			{
				ReactorChannel callbackChannel = reactorChannel;

				RDMDictionaryMsgEvent rdmDictionaryMsgEvent = ReactorFactory.createRDMDictionaryMsgEvent();
				rdmDictionaryMsgEvent.transportBuffer(transportBuffer);
				rdmDictionaryMsgEvent.msg(msg);
				rdmDictionaryMsgEvent.rdmDictionaryMsg(dictionaryMsg);

				if (reactorHandlesWarmStandby(reactorChannel))
				{
					callbackChannel = reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl();
					callbackChannel.selectableChannelFromChannel(reactorChannel.channel());
					callbackChannel.userSpecObj(reactorChannel.userSpecObj());
				}

				rdmDictionaryMsgEvent.reactorChannel(callbackChannel);

				if (wlRequest != null)
				{
					rdmDictionaryMsgEvent.streamInfo().serviceName(wlRequest.streamInfo().serviceName());
					rdmDictionaryMsgEvent.streamInfo().userSpecObject(wlRequest.streamInfo().userSpecObject());
				} else
				{
					rdmDictionaryMsgEvent.streamInfo().clear();
				}

				retval = callback.rdmDictionaryMsgCallback(rdmDictionaryMsgEvent);
				rdmDictionaryMsgEvent.returnToPool();
			} else
			{
				retval = ReactorCallbackReturnCodes.SUCCESS;
			}
		} else
		{
			// callback is undefined, raise it to defaultMsgCallback.
			retval = ReactorCallbackReturnCodes.RAISE;
		}

		return retval;
	}

	// returns ReactorCallbackReturnCodes and populates errorInfo if needed.
	int sendAndHandleDictionaryMsgCallback(String location, ReactorChannel reactorChannel,
			TransportBuffer transportBuffer, Msg msg, DictionaryMsg dictionaryMsg, WlRequest wlRequest,
			ReactorErrorInfo errorInfo)
	{
		int retval = sendDictionaryMsgCallback(reactorChannel, transportBuffer, msg, dictionaryMsg, wlRequest,
				errorInfo);

		// check return code from callback.
		if (retval == ReactorCallbackReturnCodes.FAILURE)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.FAILURE was returned from rdmDictionaryMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " was returned from rdmDictionaryMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}
		return retval;
	}

	// returns ReactorCallbackReturnCodes and populates errorInfo if needed.
	private int sendAndHandleDictionaryMsgCallback(String location, ReactorChannel reactorChannel,
			TransportBuffer transportBuffer, Msg msg, DictionaryMsg dictionaryMsg, ReactorErrorInfo errorInfo)
	{
		int retval = sendDictionaryMsgCallback(reactorChannel, transportBuffer, msg, dictionaryMsg, null, errorInfo);

		// check return code from callback.
		if (retval == ReactorCallbackReturnCodes.FAILURE)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
					"ReactorCallbackReturnCodes.FAILURE was returned from rdmDictionaryMsgCallback(). This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		} else if (retval != ReactorCallbackReturnCodes.RAISE && retval != ReactorCallbackReturnCodes.SUCCESS)
		{
			// retval is not a valid ReactorReturnCodes.
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location, "retval of " + retval
					+ " was returned from rdmDictionaryMsgCallback() and is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
			shutdown(errorInfo);
			return ReactorReturnCodes.FAILURE;
		}
		return retval;
	}

	// return success if no more to dispatch, positive value if there are more
	// to dispatch, or a non-success ReactorReturnCode if an error occurred.
	int dispatchChannel(ReactorChannel reactorChannel, ReactorDispatchOptions dispatchOptions,
			ReactorErrorInfo errorInfo)
	{
		_reactorLock.lock();
		if (_reactorOptions.debuggerOptions().debugConnectionLevel())
		{
			debugger.incNumOfDispatchCalls();
		}
		try
		{
			if (reactorChannel.state() == ReactorChannel.State.CLOSED)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.dispatchChannel",
						"ReactorChannel is closed, aborting.");
			} else if (reactorChannel != _reactorChannel)
			{
				if (_reactorOptions.debuggerOptions().debugEventQueueLevel())
				{
					debugger.writeDebugInfo(ReactorDebugger.EVENTQUEUE_COUNT_SPECIFIED, this.hashCode(),
							reactorChannel.hashCode(),
							_workerQueue.countNumberOfReadQueueElements(
									node -> ((WorkerEvent) node).reactorChannel() == reactorChannel),
							ReactorDebugger.getChannelId(reactorChannel));
				}
				int maxMessages = dispatchOptions.maxMessages();
				int msgCount = 0;
				int retval = ReactorReturnCodes.SUCCESS;

				if (!isReactorChannelReady(reactorChannel))
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.dispatchChannel",
							"ReactorChannel is not active, aborting.");
				} else
				{
					do
					{
						msgCount++;
						retval = performChannelRead(reactorChannel, dispatchOptions.readArgs(), errorInfo);
					} while (isReactorChannelReady(reactorChannel) && msgCount < maxMessages && retval > 0);
				}

				return retval;
			} else
			{
				if (_reactorOptions.debuggerOptions().debugEventQueueLevel())
				{
					debugger.writeDebugInfo(ReactorDebugger.EVENTQUEUE_COUNT_REACTOR, this.hashCode(),
							_workerQueue.countNumberOfReadQueueElements(
									node -> ((WorkerEvent) node).reactorChannel() == this._reactorChannel));
					debugger.writeDebugInfo(ReactorDebugger.EVENTQUEUE_COUNT_ALL, this.hashCode(),
							_workerQueue.countNumberOfReadQueueElements(
									node -> ((WorkerEvent) node).reactorChannel() != this._reactorChannel
											&& ((WorkerEvent) node).reactorChannel() != null));
				}
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
		} finally
		{
			_reactorLock.unlock();
		}
	}

	int submitChannel(ReactorChannel reactorChannel, TransportBuffer buffer, ReactorSubmitOptions submitOptions,
			ReactorErrorInfo errorInfo)
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
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
						"ReactorChannel is closed, aborting.");
			} else if (reactorChannel != _reactorChannel)
			{
				ReactorPackedBuffer packedBufferImpl = null;
				boolean isPackedBuffer = packedBufferHashMap.size() == 0 ? false
						: ((packedBufferImpl = packedBufferHashMap.get(buffer)) != null);

				if (isPackedBuffer)
				{
					if (packedBufferImpl.nextRWFBufferPosition() == 0)
					{
						isPackedBuffer = false;
					}
				}

				// Checks the channel's protocol type to perform auto conversion for the JSON
				// protocol
				if (reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE && !isPackedBuffer)
				{
					TransportBuffer writeAgainBuffer = writeCallAgainMap.size() > 0 ? writeCallAgainMap.remove(buffer)
							: null;

					if (Objects.isNull(writeAgainBuffer))
					{
						if (Objects.isNull(jsonConverter))
						{
							return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
									"The JSON converter library has not been initialized properly.");
						}

						jsonDecodeMsg.clear();
						_dIter.clear();
						ret = _dIter.setBufferAndRWFVersion(buffer, reactorChannel.majorVersion(),
								reactorChannel.minorVersion());

						ret = jsonDecodeMsg.decode(_dIter);

						if (ret == CodecReturnCodes.SUCCESS)
						{
							converterError.clear();
							rwfToJsonOptions.clear();
							rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

							if (jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults,
									converterError) != CodecReturnCodes.SUCCESS)
							{
								return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
										"Failed to convert RWF to JSON protocol. Error text: "
												+ converterError.getText());
							}

							TransportBuffer jsonBuffer = reactorChannel.getBuffer(conversionResults.getLength(), false,
									errorInfo);

							if (Objects.isNull(jsonBuffer))
							{
								return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
										"Failed to get a buffer for sending JSON message. Error text: "
												+ errorInfo.error().text());
							}

							getJsonMsgOptions.clear();
							getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
							getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false);

							if (jsonConverter.getJsonBuffer(jsonBuffer, getJsonMsgOptions,
									converterError) != CodecReturnCodes.SUCCESS)
							{
								return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
										"Failed to get converted JSON message. Error text: "
												+ converterError.getText());
							}

							/*
							 * Releases the user's buffer when this function writes the JSON buffer
							 * successfully.
							 */
							releaseUserBuffer = true;

							/* Changes to JSON buffer */
							writeBuffer = jsonBuffer;
						}
					} else
					{
						/*
						 * Releases the user's buffer when this function writes the JSON buffer
						 * successfully.
						 */
						releaseUserBuffer = true;

						/* Changes to JSON buffer */
						writeBuffer = writeAgainBuffer;
					}
				}

				if (_reactorOptions.xmlTraceWrite())
				{
					xmlString.setLength(0);
					xmlString.append("\n<!-- Outgoing Reactor message -->\n").append("<!-- ")
							.append(reactorChannel.selectableChannel().toString()).append(" -->\n").append("<!-- ")
							.append(new java.util.Date()).append(" -->\n");
					xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(),
							writeBuffer, null, xmlString, errorInfo.error());
					if (_reactorOptions.xmlTracing()){
						System.out.println(xmlString);
					}
					if (_reactorOptions.xmlTraceToFile()) {

						_fileDumper.dump(xmlString.toString());

					}
				}

				ret = reactorChannel.channel().write(writeBuffer, submitOptions.writeArgs(), errorInfo.error());
				if (ret > TransportReturnCodes.SUCCESS || ret == TransportReturnCodes.WRITE_FLUSH_FAILED
						|| ret == TransportReturnCodes.WRITE_CALL_AGAIN)
				{
					if (sendFlushRequest(reactorChannel, "Reactor.submitChannel",
							errorInfo) != ReactorReturnCodes.SUCCESS)
						return ReactorReturnCodes.FAILURE;

					if (ret != TransportReturnCodes.WRITE_CALL_AGAIN)
					{
						if (Objects.nonNull(packedBufferImpl))
						{
							packedBufferHashMap.remove(buffer);
							packedBufferImpl.returnToPool();
						}

						ret = ReactorReturnCodes.SUCCESS;
					} else
					{
						if (buffer != writeBuffer)
						{
							writeCallAgainMap.put(buffer, writeBuffer);
						}

						releaseUserBuffer = false;
						ret = ReactorReturnCodes.WRITE_CALL_AGAIN;
					}
				} else if (ret < TransportReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"channel write failure chnl=" + reactorChannel.channel().selectableChannel() + " errorId="
									+ errorInfo.error().errorId() + " errorText=" + errorInfo.error().text());

					releaseUserBuffer = false;
					ret = ReactorReturnCodes.FAILURE;
				} else
				{
					reactorChannel.flushAgain(false);

					if (Objects.nonNull(packedBufferImpl))
					{
						packedBufferHashMap.remove(buffer);
						packedBufferImpl.returnToPool();
					}
				}

				/*
				 * Releases the user's buffer when the Reactor successfully writes the converted
				 * JSON buffer if any.
				 */
				if (releaseUserBuffer)
				{
					reactorChannel.releaseBuffer(buffer, errorInfo);
				}
			}
		} finally
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

	int submitChannel(ReactorChannel reactorChannel, Msg msg, ReactorSubmitOptions submitOptions,
			ReactorErrorInfo errorInfo)
	{
		int ret = ReactorReturnCodes.SUCCESS;

		_reactorLock.lock();

		try
		{
			if (!isReactorChannelReady(reactorChannel))
			{
				ret = ReactorReturnCodes.FAILURE;
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
						"ReactorChannel is closed, aborting.");
			} else if (reactorChannel != _reactorChannel)
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
					TransportBuffer writeBuffer = reactorChannel.channel().getBuffer(msgSize, false, errorInfo.error());

					if (writeBuffer != null)
					{
						_eIter.clear();
						_eIter.setBufferAndRWFVersion(writeBuffer, reactorChannel.channel().majorVersion(),
								reactorChannel.channel().minorVersion());
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
						} else if (ret == CodecReturnCodes.BUFFER_TOO_SMALL) // resize buffer and try again
						{
							// release buffer that's too small
							reactorChannel.channel().releaseBuffer(writeBuffer, errorInfo.error());
							// resize
							msgSize *= 2;
							continue;
						} else // encoding failure
						{
							// release buffer that caused encoding failure
							reactorChannel.channel().releaseBuffer(writeBuffer, errorInfo.error());

							populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
									"message encoding failure chnl=" + reactorChannel.channel().selectableChannel()
											+ " errorId=" + errorInfo.error().errorId() + " errorText="
											+ errorInfo.error().text());

							ret = ReactorReturnCodes.FAILURE;
							break;
						}
					} else // return NO_BUFFERS
					{
						if (sendFlushRequest(reactorChannel, "Reactor.submitChannel",
								errorInfo) != ReactorReturnCodes.SUCCESS)
							return ReactorReturnCodes.FAILURE;

						populateErrorInfo(errorInfo, ReactorReturnCodes.NO_BUFFERS, "Reactor.submitChannel",
								"channel out of buffers chnl=" + reactorChannel.channel().selectableChannel()
										+ " errorId=" + errorInfo.error().errorId() + " errorText="
										+ errorInfo.error().text());

						ret = ReactorReturnCodes.NO_BUFFERS;
						break;
					}
				}
			}
		} finally
		{
			_reactorLock.unlock();
		}

		return ret;
	}

	public int submitChannel(ReactorChannel reactorChannel, MsgBase rdmMsg, ReactorSubmitOptions submitOptions,
			ReactorErrorInfo errorInfo)
	{
		int ret = ReactorReturnCodes.SUCCESS;

		_reactorLock.lock();

		try
		{
			if (!_reactorActive)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.SHUTDOWN, "Reactor.submitChannel",
						"Reactor is not active, aborting.");
			}
			if (!isReactorChannelReady(reactorChannel))
			{
				ret = ReactorReturnCodes.FAILURE;
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
						"ReactorChannel is closed, aborting.");
			} else if (reactorChannel != _reactorChannel)
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
					TransportBuffer writeBuffer = reactorChannel.channel().getBuffer(bufferSize, false,
							errorInfo.error());

					if (writeBuffer != null)
					{
						_eIter.clear();
						_eIter.setBufferAndRWFVersion(writeBuffer, reactorChannel.channel().majorVersion(),
								reactorChannel.channel().minorVersion());
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
						} else if (ret == CodecReturnCodes.BUFFER_TOO_SMALL) // resize buffer and try again
						{
							// release buffer that's too small
							reactorChannel.channel().releaseBuffer(writeBuffer, errorInfo.error());
							// resize
							bufferSize *= 2;
							continue;
						} else // encoding failure
						{
							// release buffer that caused encoding failure
							reactorChannel.channel().releaseBuffer(writeBuffer, errorInfo.error());

							populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
									"message encoding failure chnl=" + reactorChannel.channel().selectableChannel()
											+ " errorId=" + errorInfo.error().errorId() + " errorText="
											+ errorInfo.error().text());

							ret = ReactorReturnCodes.FAILURE;
							break;
						}
					} else // return NO_BUFFERS
					{
						if (sendFlushRequest(reactorChannel, "Reactor.submitChannel",
								errorInfo) != ReactorReturnCodes.SUCCESS)
							return ReactorReturnCodes.FAILURE;

						populateErrorInfo(errorInfo, ReactorReturnCodes.NO_BUFFERS, "Reactor.submitChannel",
								"channel out of buffers chnl=" + reactorChannel.channel().selectableChannel()
										+ " errorId=" + errorInfo.error().errorId() + " errorText="
										+ errorInfo.error().text());

						ret = ReactorReturnCodes.NO_BUFFERS;
						break;
					}
				}
			}
		} finally
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
		} else // server channel or no more retries
		{
			reactorChannel.state(State.DOWN);
		}

		if (_reactorOptions.debuggerOptions().debugConnectionLevel())
		{
			debugger.writeDebugInfo(ReactorDebugger.CONNECTION_DISCONNECT, this.hashCode(), reactorChannel.hashCode(),
					reactorChannel.state().equals(State.DOWN) ? "DOWN" : "DOWN_RECONNECTING",
					ReactorDebugger.getChannelId(reactorChannel));
		}

		if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
		{
			// send CHANNEL_DOWN to user app via reactorChannelEventCallback.
			return sendAndHandleChannelEventCallback(location, ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
					reactorChannel, errorInfo);
		} else // server channel or no more retries
		{
			// send CHANNEL_DOWN to user app via reactorChannelEventCallback.
			return sendAndHandleChannelEventCallback(location, ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel,
					errorInfo);
		}
	}

	private int processRwfMessage(TransportBuffer transportBuffer, Buffer buffer, ReactorChannel reactorChannel,
			ReactorErrorInfo errorInfo)
	{
		// inspect the message and dispatch it to the application.
		_dIter.clear();

		if (Objects.nonNull(buffer))
		{
			_dIter.setBufferAndRWFVersion(buffer, reactorChannel.channel().majorVersion(),
					reactorChannel.channel().minorVersion());
		} else
		{
			_dIter.setBufferAndRWFVersion(transportBuffer, reactorChannel.channel().majorVersion(),
					reactorChannel.channel().minorVersion());
		}

		_msg.clear();
		int retval = _msg.decode(_dIter);
		if (retval != CodecReturnCodes.SUCCESS)
		{
			return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.performChannelRead",
					"initial decode of msg failed, errorId=" + errorInfo.error().errorId() + " text="
							+ errorInfo.error().text());
		}

		// determine if for watchlist and process by watchlist
		WlStream wlStream = null;
		_tempWlInteger.value(_msg.streamId());
		if (reactorChannel.watchlist() != null
				&& (wlStream = reactorChannel.watchlist().streamIdtoWlStreamTable().get(_tempWlInteger)) != null)
		{
			if (reactorChannel.reactor().reactorHandlesWarmStandby(reactorChannel))
			{
				if ((retval = reactorChannel.watchlist().readMsgWSB(wlStream, reactorChannel, _dIter, _msg,
						errorInfo)) < ReactorReturnCodes.SUCCESS)
				{
					return retval;
				}
			} else
			{
				if ((retval = reactorChannel.watchlist().readMsg(wlStream, _dIter, _msg,
						errorInfo)) < ReactorReturnCodes.SUCCESS)
				{
					return retval;
				}
			}
		} else // not for watchlist
		{
			// check first if this is a TunnelStream Request for a Provider
			if (reactorChannel.server() != null && _msg.msgClass() == MsgClasses.REQUEST
					&& ((RequestMsg) _msg).checkPrivateStream() && ((RequestMsg) _msg).checkQualifiedStream())
			{
				if ((retval = handleTunnelStreamRequest(reactorChannel, ((RequestMsg) _msg),
						errorInfo)) != ReactorReturnCodes.SUCCESS)
					return retval;
			}

			// only process if watchlist not enabled
			else if (reactorChannel.role().type() != ReactorRoleTypes.CONSUMER
					|| !((ConsumerRole) reactorChannel.role()).watchlistOptions().enableWatchlist())
			{
				if (reactorHandlesWarmStandby(reactorChannel))
				{
					reactorChannel.warmStandByHandlerImpl.readMsgChannel(reactorChannel);
					if (processChannelMessage(reactorChannel, _dIter, _msg, transportBuffer,
							errorInfo) == ReactorReturnCodes.FAILURE)
						return ReactorReturnCodes.FAILURE;
					reactorChannel.warmStandByHandlerImpl.readMsgChannel(null);
				} else
				{
					if (processChannelMessage(reactorChannel, _dIter, _msg, transportBuffer,
							errorInfo) == ReactorReturnCodes.FAILURE)
						return ReactorReturnCodes.FAILURE;
				}
			} else
			{
				_closeMsg.msgClass(MsgClasses.CLOSE);
				_closeMsg.streamId(_msg.streamId());
				_closeMsg.domainType(_msg.domainType());
				ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
				retval = submitChannel(reactorChannel, _closeMsg, submitOptions, errorInfo);
				if (retval != CodecReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, retval, "Reactor.submit",
							"Submit of CloseMsg failed: <" + TransportReturnCodes.toString(retval) + ">");
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
		if (ret > TransportReturnCodes.SUCCESS || ret == TransportReturnCodes.WRITE_FLUSH_FAILED)
		{
			if (sendFlushRequest(reactorChannel, "Reactor.sendJSONMessage", errorInfo) != ReactorReturnCodes.SUCCESS)
				return ReactorReturnCodes.FAILURE;

			ret = ReactorReturnCodes.SUCCESS;
		} else if (ret < TransportReturnCodes.SUCCESS)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.sendJSONMessage",
					"channel write failure chnl=" + reactorChannel.channel().selectableChannel() + " errorId="
							+ errorInfo.error().errorId() + " errorText=" + errorInfo.error().text());

			reactorChannel.releaseBuffer(msgBuffer, errorInfo);

			ret = ReactorReturnCodes.FAILURE;
		} else
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
			if (_reactorOptions.xmlTracePing())
			{
				xmlString.setLength(0);
				xmlString.append("\n<!-- Incoming Reactor message -->\n").append("<!-- ")
						.append(reactorChannel.selectableChannel().toString()).append(" -->\n").append("<!-- ")
						.append(new java.util.Date()).append(" -->\n");
				xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null,
						xmlString, errorInfo.error());
				if (_reactorOptions.xmlTracing()){
					System.out.println(xmlString);
				}
				if (_reactorOptions.xmlTraceToFile()) {
					_fileDumper.dump(xmlString.toString());
				}
			}

			// update ping handler
			reactorChannel.pingHandler().receivedMsg();

			// Checks the channel's protocol type to perform auto conversion for the JSON
			// protocol
			if (reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
			{
				boolean failedToConvertJSONMsg = true;
				String jsonErrorMsg = null;

				if (Objects.isNull(jsonConverter))
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.performChannelRead",
							"The JSON converter library has not been initialized properly.");
				}

				parseJsonOptions.clear();
				parseJsonOptions.setProtocolType(reactorChannel.channel().protocolType());

				converterError.clear();
				retval = jsonConverter.parseJsonBuffer(msgBuf, parseJsonOptions, converterError);

				if (retval == CodecReturnCodes.SUCCESS)
				{
					decodeJsonMsgOptions.clear();
					decodeJsonMsgOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
					decodeJsonMsgOptions.setMajorVersion(reactorChannel.channel().majorVersion());
					decodeJsonMsgOptions.setMinorVersion(reactorChannel.channel().minorVersion());

					/*
					 * Set the ReactorChannel so that users can get it in the
					 * ReactorServiceNameToIdCallback callback
					 */
					if (Objects.nonNull(serviceNameIdConverterClient))
					{
						serviceNameIdConverterClient.setReactorChannel(reactorChannel);
					}

					jsonMsg.clear();

					while ((retval = jsonConverter.decodeJsonMsg(jsonMsg, decodeJsonMsgOptions,
							converterError)) != CodecReturnCodes.END_OF_CONTAINER)
					{
						if (retval != CodecReturnCodes.SUCCESS)
						{
							/* Failed to convert a JSON message. */
							break;
						}

						switch (jsonMsg.jsonMsgClass())
						{
						case JsonMsgClasses.RSSL_MESSAGE:
						{
							failedToConvertJSONMsg = false;

							if (_reactorOptions.xmlTracing() || _reactorOptions.xmlTraceToFile())
							{
								xmlString.setLength(0);
								xmlString.append("\n<!-- Dump Reactor message -->\n").append("<!-- ")
										.append(reactorChannel.selectableChannel().toString()).append(" -->\n")
										.append("<!-- ").append(new java.util.Date()).append(" -->\n");
								xmlDumpTrace.dumpBuffer(reactorChannel.majorVersion(), reactorChannel.minorVersion(),
										Codec.RWF_PROTOCOL_TYPE, jsonMsg.rwfMsg().encodedMsgBuffer(), null, xmlString,
										errorInfo.error());
								if (_reactorOptions.xmlTracing()){
									System.out.println(xmlString);
								}
								if (_reactorOptions.xmlTraceToFile()) {
									_fileDumper.dump(xmlString.toString());
								}
							}

							// inspect the converted message and dispatch it to the application.
							retval = processRwfMessage(msgBuf, jsonMsg.rwfMsg().encodedMsgBuffer(), reactorChannel,
									errorInfo);
							if (retval != ReactorReturnCodes.SUCCESS)
							{
								return retval;
							}

							break;
						}
						case JsonMsgClasses.PING:
						{
							failedToConvertJSONMsg = false;

							TransportBuffer msgBuffer = reactorChannel.getBuffer(JSON_PONG_MESSAGE.length(), false,
									errorInfo);

							if (Objects.nonNull(msgBuffer))
							{
								msgBuffer.data().put(JSON_PONG_MESSAGE.getBytes());

								if (_reactorOptions.xmlTracing() || _reactorOptions.xmlTraceToFile())
								{
									xmlString.setLength(0);
									xmlString.append("\n<!-- Outgoing Reactor message -->\n").append("<!-- ")
											.append(reactorChannel.selectableChannel().toString()).append(" -->\n")
											.append("<!-- ").append(new java.util.Date()).append(" -->\n");
									xmlDumpTrace.dumpBuffer(reactorChannel.channel(), Codec.JSON_PROTOCOL_TYPE,
											msgBuffer, null, xmlString, errorInfo.error());
									if (_reactorOptions.xmlTracing()){
										System.out.println(xmlString);
									}
									if (_reactorOptions.xmlTraceToFile()) {
										_fileDumper.dump(xmlString.toString());
									}
								}

								/* Reply with JSON PONG message to the sender */
								retval = sendJSONMessage(msgBuffer, reactorChannel, errorInfo);
							} else
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
							xmlDumpTrace.dumpBuffer(reactorChannel.channel(), Codec.JSON_PROTOCOL_TYPE, msgBuf, null,
									xmlString, errorInfo.error());
							jsonErrorMsg = xmlString.toString();

							populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.performChannelRead",
									"Received JSON error message: " + jsonErrorMsg);

							failedToConvertJSONMsg = false;
							retval = ReactorReturnCodes.FAILURE;
							break;
						}
						}

						if (retval != ReactorReturnCodes.SUCCESS)
							break;

						failedToConvertJSONMsg = true; /* Reset the flag to its initial state. */
					}
				} else
				{
					failedToConvertJSONMsg = false;

					/* Failed to parse JSON buffer */
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.performChannelRead",
							"Failed to parse JSON message: " + converterError.getText());
				}

				if (retval < ReactorReturnCodes.SUCCESS)
				{
					if (failedToConvertJSONMsg)
					{
						/* Send JSON error message back when it fails to decode JSON message. */
						jsonErrorParams.clear();
						jsonErrorParams.fillParams(converterError, jsonMsg.rwfMsg().streamId());
						jsonErrorOutputBuffer.clear();

						getMessageError.clear();
						if ((retval = jsonConverter.getErrorMessage(jsonErrorOutputBuffer, jsonErrorParams,
								getMessageError)) == CodecReturnCodes.SUCCESS && sendJsonConvError)
						{
							TransportBuffer msgBuffer = reactorChannel.getBuffer(jsonErrorOutputBuffer.length(), false,
									errorInfo);

							if (Objects.nonNull(msgBuffer))
							{
								msgBuffer.data().put(jsonErrorOutputBuffer.data());

								/* Reply with JSON ERROR message to the sender */
								retval = sendJSONMessage(msgBuffer, reactorChannel, errorInfo);
							} else
							{
								retval = ReactorReturnCodes.FAILURE;
							}
						}
					}

					/*
					 * Notifies JSON conversion error messages if the callback is specified by users
					 */
					if (Objects.nonNull(JsonConversionEventCallback)
							&& (converterError.getCode() != JsonConverterErrorCodes.JSON_ERROR_NO_ERROR_CODE))
					{
						jsonConversionEvent.clear();
						jsonConversionEvent.reactorChannel(reactorChannel);

						if (failedToConvertJSONMsg)
						{
							populateErrorInfo(jsonConversionEvent.errorInfo(), ReactorReturnCodes.FAILURE,
									"Reactor.performChannelRead",
									"Failed to convert JSON message: " + jsonErrorOutputBuffer.toString());
						} else
						{
							populateErrorInfo(jsonConversionEvent.errorInfo(), ReactorReturnCodes.FAILURE,
									"Reactor.performChannelRead",
									"Failed to convert JSON message: " + converterError.getText());
						}

						jsonConversionEvent.userSpec = jsonConverterUserSpec;
						jsonConversionEvent.error().text(jsonConversionEvent.errorInfo().error().text());
						jsonConversionEvent.error().errorId(CodecReturnCodes.FAILURE);

						int cret = JsonConversionEventCallback.reactorJsonConversionEventCallback(jsonConversionEvent);

						if (cret == ReactorCallbackReturnCodes.FAILURE)
						{
							return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
									"Reactor.performChannelRead", "Error return code" + cret
											+ " from the ReactorJsonConversionEventCallback callback.");
						}
					}

					/*
					 * Don't closes the channel when this function can reply the JSON ERROR message
					 * back.
					 */
					if (closeChannelFromFailure && (retval != ReactorReturnCodes.SUCCESS))
					{
						if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client
																												// channel
						{
							reactorChannel.state(State.DOWN_RECONNECTING);
						} else // server channel or no more retries
						{
							reactorChannel.state(State.DOWN);
						}

						if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client
																												// channel
						{
							// send CHANNEL_DOWN to user app via reactorChannelEventCallback.
							sendAndHandleChannelEventCallback("Reactor.performChannelRead",
									ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
						} else // server channel or no more retries
						{
							// send CHANNEL_DOWN to user app via reactorChannelEventCallback.
							sendAndHandleChannelEventCallback("Reactor.performChannelRead",
									ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
						}

						return ReactorReturnCodes.SUCCESS; /* Problem handled, so return success */
					}
				}
			} else
			{
				// inspect the message and dispatch it to the application.
				retval = processRwfMessage(msgBuf, null, reactorChannel, errorInfo);
				if (retval != ReactorReturnCodes.SUCCESS)
				{
					return retval;
				}
			}
		} else
		{
			if (readArgs.readRetVal() == TransportReturnCodes.FAILURE)
			{
				if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
				{
					reactorChannel.state(State.DOWN_RECONNECTING);
				} else // server channel or no more retries
				{
					reactorChannel.state(State.DOWN);
				}

				if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
				{
					// send CHANNEL_DOWN to user app via reactorChannelEventCallback.
					sendAndHandleChannelEventCallback("Reactor.performChannelRead",
							ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
				} else // server channel or no more retries
				{
					// send CHANNEL_DOWN to user app via reactorChannelEventCallback.
					sendAndHandleChannelEventCallback("Reactor.performChannelRead",
							ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
				}

			} else if (readArgs.readRetVal() == TransportReturnCodes.READ_FD_CHANGE)
			{
				// reset selectable channel on ReactorChannel to new one
				reactorChannel.selectableChannelFromChannel(reactorChannel.channel());

				// set oldSelectableChannel on ReactorChannel
				reactorChannel.oldSelectableChannel(reactorChannel.channel().oldSelectableChannel());

				/*
				 * If this is warm standby, update the selectable channel list in the
				 * warmstandbychannelinfo
				 */
				if (reactorHandlesWarmStandby(reactorChannel))
				{
					reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().warmStandbyChannelInfo()
							.selectableChannelList().remove(reactorChannel.channel().oldSelectableChannel());
					reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().warmStandbyChannelInfo()
							.selectableChannelList().add(reactorChannel.channel().selectableChannel());
				}

				// send FD_CHANGE WorkerEvent to Worker.
				if (!sendWorkerEvent(WorkerEventTypes.FD_CHANGE, reactorChannel))
				{
					// sendWorkerEvent() failed, send channel down
					reactorChannel.state(State.DOWN);
					sendAndHandleChannelEventCallback("Reactor.performChannelRead",
							ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
					return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.performChannelRead",
							"sendWorkerEvent() failed");
				}

				// send FD_CHANGE to user app via reactorChannelEventCallback.
				sendAndHandleChannelEventCallback("Reactor.performChannelRead", ReactorChannelEventTypes.FD_CHANGE,
						reactorChannel, errorInfo);

				/*
				 * If this is warm standby, remove the old selectable channel from the old
				 * channel list in the warmstandbychannelinfo
				 */
				if (reactorHandlesWarmStandby(reactorChannel))
				{
					reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl().warmStandbyChannelInfo()
							.oldSelectableChannelList().remove(reactorChannel.channel().oldSelectableChannel());
				}
			} else if (readArgs.readRetVal() == TransportReturnCodes.READ_PING)
			{
				reactorChannel.pingHandler().receivedMsg();
			}
		}

		if (readArgs.readRetVal() == TransportReturnCodes.READ_PING)
		{
			// update ping handler
			if (_reactorOptions.pingStatSet())
				reactorChannel.pingHandler().receivedPing();
		}

		// Aggregate number of bytes read
		if (_reactorOptions.readStatSet() == true)
		{
			((ReadArgsImpl) _readArgsAggregator)
					.bytesRead(overflowSafeAggregate(_readArgsAggregator.bytesRead(), readArgs.bytesRead()));
			((ReadArgsImpl) _readArgsAggregator).uncompressedBytesRead(overflowSafeAggregate(
					_readArgsAggregator.uncompressedBytesRead(), readArgs.uncompressedBytesRead()));
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

		if (((ProviderRole) reactorChannel.role()).tunnelStreamListenerCallback() != null)
		{
			// reject if TunnelStream already exists
			_tempWlInteger.value(msg.streamId());
			if (!reactorChannel.streamIdtoTunnelStreamTable().containsKey(_tempWlInteger))
			{
				if (msg.containerType() == DataTypes.FILTER_LIST && msg.msgKey().checkHasFilter())
				{
					if ((ret = _tunnelStreamRequestEvent.classOfService().decodeCommonProperties(_reactorChannel,
							msg.encodedDataBody(), errorInfo)) == ReactorReturnCodes.SUCCESS)
					{
						// check if stream version is less than or equal to current stream version
						int requestedStreamVersion = _tunnelStreamRequestEvent.classOfService().common()
								.streamVersion();
						if (requestedStreamVersion <= CosCommon.CURRENT_STREAM_VERSION)
						{
							isValid = true;
						} else
						{
							rejectString = "Unsupported class of service stream version: " + requestedStreamVersion;
						}
					} else
					{
						rejectString = "Class of service common properties decode failed with return code: " + ret
								+ " <" + errorInfo.error().text() + ">";
					}

					if (_reactorOptions.debuggerOptions().debugTunnelStreamLevel())
					{
						debugger.writeDebugInfo(ReactorDebugger.TUNNELSTREAM_STREAM_REQUEST, this.hashCode(),
								reactorChannel.hashCode(), msg.streamId(),
								ReactorDebugger.getChannelId(reactorChannel));
					}
				} else // doesn't contain FILTER_LIST and have filter in message key
				{
					rejectString = "TunnelStream request must contain FILTER_LIST and have filter in message key";
				}
			} else // TunnelStream already exists, reject
			{
				rejectString = "TunnelStream is already open for stream id " + msg.streamId();
			}
		} else // no listener callback, tunnel streams aren't supported
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

			return ((ProviderRole) reactorChannel.role()).tunnelStreamListenerCallback()
					.listenerCallback(_tunnelStreamRequestEvent);
		} else
		{
			// reject TunnelStream here
			_tunnelStreamRejectOptions.clear();
			_tunnelStreamRejectOptions.state().streamState(StreamStates.CLOSED);
			_tunnelStreamRejectOptions.state().dataState(DataStates.SUSPECT);
			_tunnelStreamRejectOptions.state().code(StateCodes.NONE);
			_tunnelStreamRejectOptions.state().text().data(rejectString);

			_tunnelStreamRequestEvent.clear();
			_tunnelStreamRequestEvent.reactorChannel(reactorChannel);
			_tunnelStreamRequestEvent.domainType(((RequestMsg) _msg).domainType());
			_tunnelStreamRequestEvent.streamId(((RequestMsg) _msg).streamId());
			_tunnelStreamRequestEvent.serviceId(((RequestMsg) _msg).msgKey().serviceId());
			_tunnelStreamRequestEvent.name(((RequestMsg) _msg).msgKey().name().toString());

			if (reactorChannel.rejectTunnelStream(_tunnelStreamRequestEvent, _tunnelStreamRejectOptions,
					errorInfo) < ReactorReturnCodes.SUCCESS)
			{
				return ReactorReturnCodes.FAILURE;
			}

			// send warning event to reactor channel
			sendAndHandleChannelEventCallback("Reactor.handleTunnelStreamRequest", ReactorChannelEventTypes.WARNING,
					reactorChannel, errorInfo);
		}

		return ReactorReturnCodes.SUCCESS;
	}

	// returns the errorInfo.code().
	@SuppressWarnings("fallthrough")
	private int processWorkerEvent(ReactorErrorInfo errorInfo)
	{
		WorkerEvent event = (WorkerEvent) _workerQueue.read();
		if (event == null)
			return 0;

		populateErrorInfo(errorInfo, event.errorInfo().code(), event.errorInfo().location(),
				event.errorInfo().error().text());
		WorkerEventTypes eventType = event.eventType();

		ReactorChannel reactorChannel = event.reactorChannel();
		int ret = ReactorReturnCodes.SUCCESS;

		switch (eventType)
		{
		case FLUSH_DONE:
			reactorChannel.flushRequested(false);
			if (reactorChannel.flushAgain())
			{
				/*
				 * Channel wrote a message since its last flush request, request flush again in
				 * case that message was not flushed.
				 */
				if (sendFlushRequest(reactorChannel, "Reactor.processWorkerEvent",
						errorInfo) != ReactorReturnCodes.SUCCESS)
				{
					event.returnToPool();
					return ReactorReturnCodes.FAILURE;
				}
			}

			/*
			 * Dispatch watchlist; it may be waiting on a flush to complete if it ran out of
			 * output buffers.
			 */
			if (reactorChannel.watchlist() != null)
			{
				if ((ret = reactorChannel.watchlist().dispatch(errorInfo)) != ReactorReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ret, "Reactor.processWorkerEvent",
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
				populateErrorInfo(errorInfo, ReactorReturnCodes.SUCCESS, "Reactor.processWorkerEvent",
						errorInfo.error().text());
			} else // server channel or no more retries
			{
				populateErrorInfo(errorInfo, ReactorReturnCodes.SUCCESS, "Reactor.processWorkerEvent",
						errorInfo.error().text());
			}
			processChannelDown(event, errorInfo);
			break;
		case CHANNEL_CLOSE_ACK:
			/* Worker is done with channel. Safe to release it. */

			if (reactorChannel.watchlist() != null)
			{
				reactorChannel.watchlist().close();

				/* This watch list is returned to the pool when it is closed. */
				reactorChannel.watchlist(null);
			}

			reactorChannel.returnToPool();
			break;

		case WARNING:
			/* Override the error code as this is not a failure */
			populateErrorInfo(errorInfo, ReactorReturnCodes.SUCCESS, "Reactor.processWorkerEvent",
					errorInfo.error().text());

			sendChannelEventCallback(ReactorChannelEventTypes.WARNING, event.reactorChannel(), errorInfo);
			break;
		case TOKEN_MGNT:

			populateErrorInfo(errorInfo, errorInfo.code(), "Reactor.processWorkerEvent", errorInfo.error().text());

			sendAuthTokenEventCallback(reactorChannel, event._tokenSession.authTokenInfo(), errorInfo);

			/* Override the error code as this is not a failure */
			errorInfo.code(ReactorReturnCodes.SUCCESS);

			break;

		case TOKEN_CREDENTIAL_RENEWAL:

			/* Override the error code as this is not a failure */
			populateErrorInfo(errorInfo, ReactorReturnCodes.SUCCESS, "Reactor.processWorkerEvent",
					errorInfo.error().text());

			/*
			 * This is used to check whether the submitOAuthCredentialRenewal() is called in
			 * the callback
			 */
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
			while ((ret = reactorChannel.tunnelStreamManager()
					.dispatch(errorInfo.error())) > ReactorReturnCodes.SUCCESS)
				;
			if (ret < ReactorReturnCodes.SUCCESS)
			{
				// send channel down event if channel error occurred
				if (ret == ReactorReturnCodes.CHANNEL_ERROR)
				{
					if (reactorChannel.state() != State.DOWN && reactorChannel.state() != State.DOWN_RECONNECTING)
					{
						if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client
																												// channel
						{
							reactorChannel.state(State.DOWN_RECONNECTING);
							sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
						} else // server channel or no more retries
						{
							reactorChannel.state(State.DOWN);
							sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
						}
					}
					// return SUCCESS on dispatch() for CHANNEL_ERROR, application should close
					// ReactorChannel
					ret = ReactorReturnCodes.SUCCESS;
				} else
				{
					populateErrorInfo(errorInfo, ret, "Reactor.processWorkerEvent",
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
		case WARM_STANDBY:
			if (reactorChannel.watchlist() != null)
			{
				if ((ret = reactorChannel.watchlist().timeout(errorInfo)) != ReactorReturnCodes.SUCCESS)
				{
					event.returnToPool();
					return ret;
				}
				if (reactorChannel.watchlist().watchlistOptions().enableWarmStandby())
				{
					ReactorChannel pReactorChannel = event.reactorChannel();
					ReactorWarmStandbyHandler warmStandByHandlerImpl = null;
					ReactorWarmStandbyGroupImpl warmStandbyGroup = null;
					ReactorChannel processReactorChannel = null;
					ReactorWSBService service = event.warmStandbyService();
					WlService wlService = null;

					if (pReactorChannel != null)
					{
						warmStandByHandlerImpl = pReactorChannel.warmStandByHandlerImpl;
						warmStandbyGroup = pReactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl();
					}

					ReactorChannel startingReactorChannel = null;

					switch (event.warmStandbyEventType())
					{
					case ReactorWarmStandbyEventTypes.INIT:
						break;
					case ReactorWarmStandbyEventTypes.ACTIVE_SERVER_SERVICE_STATE_FROM_DOWN_TO_UP:
						int queueCount = 0;

						/* Fanout to all standby servers to send pending requests if any. */
						while (queueCount < warmStandByHandlerImpl.channelList().size())
						{
							processReactorChannel = warmStandByHandlerImpl.channelList().get(queueCount);

							if (processReactorChannel != reactorChannel)
							{
								sendWatchlistDispatchNowEvent(processReactorChannel);
							}
							queueCount++;
						}

						break;						
					case ReactorWarmStandbyEventTypes.CHANGE_ACTIVE_TO_STANDBY_SERVER:
						// We are moving from a previously closed active server to an new active server.
						// The previous active is in either a DOWN or DOWN_RECONNECTING state.
						if (warmStandbyGroup.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED
								&& warmStandByHandlerImpl.nextActiveReactorChannel() != null)
						{
							if (reactorChannel._loginConsumerStatus == null)
							{
								reactorChannel._loginConsumerStatus = (LoginConsumerConnectionStatus) LoginMsgFactory
										.createMsg();
								reactorChannel._loginConsumerStatus.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
							}

							/*
							 * Send a login consumer status message indicating that this is the new active
							 * This should trigger the upstream provider to start sending data.
							 */
							reactorChannel._loginConsumerStatus.clear();
							reactorChannel._loginConsumerStatus.streamId(1);
							reactorChannel._loginConsumerStatus
									.flags(LoginConsumerConnectionStatusFlags.HAS_WARM_STANDBY_INFO);
							reactorChannel._loginConsumerStatus.warmStandbyInfo()
									.warmStandbyMode(Login.ServerTypes.ACTIVE);

							/*
							 * Write directly to the channel without involving the watchlist or wsb message
							 * queues
							 */
							if (submitChannel(warmStandByHandlerImpl.nextActiveReactorChannel(),
									reactorChannel._loginConsumerStatus, reactorSubmitOptions,
									errorInfo) < ReactorReturnCodes.SUCCESS)
							{

								if (warmStandByHandlerImpl.nextActiveReactorChannel().server() == null
										&& !warmStandByHandlerImpl.nextActiveReactorChannel()
												.recoveryAttemptLimitReached()) // client channel
								{
									warmStandByHandlerImpl.nextActiveReactorChannel().state(State.DOWN_RECONNECTING);
									sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
											ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
											warmStandByHandlerImpl.nextActiveReactorChannel(), errorInfo);
									break;
								} else // server channel or no more retries
								{
									warmStandByHandlerImpl.nextActiveReactorChannel().state(State.DOWN);
									sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
											ReactorChannelEventTypes.CHANNEL_DOWN,
											warmStandByHandlerImpl.nextActiveReactorChannel(), errorInfo);
									break;
								}
							}

							warmStandByHandlerImpl.nextActiveReactorChannel().isActiveServer = true;
							reactorChannel.warmStandByHandlerImpl
									.activeReactorChannel(warmStandByHandlerImpl.nextActiveReactorChannel());

							/*
							 * Fanout any previously closed streams on the new active to the user and also
							 * close them in all of the other channels
							 */
							reactorWSBFanoutStatusMsg(warmStandByHandlerImpl.nextActiveReactorChannel(), errorInfo);
							warmStandByHandlerImpl.nextActiveReactorChannel(null);

						}

						break;
					case ReactorWarmStandbyEventTypes.CHANGE_ACTIVE_TO_STANDBY_SERVICE_CHANNEL_DOWN:
						if (warmStandbyGroup.warmStandbyMode() == ReactorWarmStandbyMode.SERVICE_BASED)
						{
							/*
							 * Submits to a channel that belongs to the warm standby feature and it is
							 * active
							 */

							Iterator<Map.Entry<WlInteger, ReactorWSBService>> iter = warmStandbyGroup._perServiceById
									.entrySet().iterator();
							
							while (iter.hasNext())
							{
								service = iter.next().getValue();

								if (reactorChannel != service.activeChannel)
								{
									/* This channel does is not the active channel for this service, continue */
									continue;
								} else
								{
									service.activeChannel = null;
								}

								for (int i = 0; i < service.channels.size(); i++)
								{
									processReactorChannel = service.channels.get(i);

									wlService = processReactorChannel.watchlist()
											.directoryHandler()._serviceCache._servicesByIdTable.get(service.serviceId);

									if (wlService != null)
									{
										/* service is down on this channel */
										if (wlService._rdmService.state().serviceState() == 0)
										{
											continue;
										}
										
										if (isReactorChannelActive(processReactorChannel))
										{
											/*
											 * Fanout any previously closed streams on the new active to the user and also
											 * close them in all of the other channels
											 */
											reactorWSBFanoutStatusMsg(processReactorChannel, errorInfo);
										}

										processReactorChannel._directoryConsumerStatus.clear();
										processReactorChannel._directoryConsumerStatus.streamId(processReactorChannel
												.watchlist().directoryHandler()._directoryStreamId);
										processReactorChannel._serviceConsumerStatus.clear();
										processReactorChannel._serviceConsumerStatus
												.flags(ConsumerStatusServiceFlags.HAS_WARM_STANDY_MODE);
										processReactorChannel._serviceConsumerStatus
												.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
										processReactorChannel._serviceConsumerStatus
												.serviceId(wlService._rdmService.serviceId());
										processReactorChannel._directoryConsumerStatus.consumerServiceStatusList()
												.add(processReactorChannel._serviceConsumerStatus);

										/*
										 * Write directly to the channel without involving the watchlist or wsb message
										 * queues
										 */
										if (submitChannel(processReactorChannel, processReactorChannel._directoryConsumerStatus,
												reactorSubmitOptions, errorInfo) < ReactorReturnCodes.SUCCESS)
										{
											if (processReactorChannel.server() == null
													&& !processReactorChannel.recoveryAttemptLimitReached()) // client
																												// channel
											{
												processReactorChannel.state(State.DOWN_RECONNECTING);
												sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
														ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
														processReactorChannel, errorInfo);
												break;
											} else // server channel or no more retries
											{
												processReactorChannel.state(State.DOWN);
												sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
														ReactorChannelEventTypes.CHANNEL_DOWN, processReactorChannel,
														errorInfo);
												break;
											}
										}

										service.activeChannel = processReactorChannel;
										
										// Break out of this for loop
										break;
									}
								}
							}

						}
						break;
					case ReactorWarmStandbyEventTypes.MOVE_WSB_HANDLER_BACK_TO_POOL:

						ReactorWarmStandbyHandler returnWarmStandbyHandler = event._warmStandbyHandler;

						if (returnWarmStandbyHandler != null)
						{
							// Removes from the closing queue and returns warm standby handler back to pool
							// to reuse
							closingWarmStandbyChannel.remove(returnWarmStandbyHandler.reactorQueueLink());

							warmstandbyChannelPool.add(returnWarmStandbyHandler.reactorQueueLink());
						}
						break;
					case ReactorWarmStandbyEventTypes.REMOVE_SERVER_FROM_WSB_GROUP:
						/*
						 * This case should only be sent if the provider has sent a bad login response
						 * or bad source directory response
						 */

						ReactorChannel callbackChannel;
						int callbackEventType;

						// Mark this channel info as inactive and remove from the recovery list.
						if (reactorChannel.isStartingServerConfig)
						{
							warmStandbyGroup.startingActiveServer().isActiveChannelConfig(false);
						} else
						{
							warmStandbyGroup.standbyServerList().get(reactorChannel.standByServerListIndex)
									.isActiveChannelConfig(false);
						}

						callbackChannel = reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl();
						callbackChannel.selectableChannelFromChannel(reactorChannel.channel());
						callbackChannel.userSpecObj(reactorChannel.userSpecObj());

						event.reactorChannel(callbackChannel);

						// Sends the channel down event if this is the only channel
						if (reactorChannel.warmStandByHandlerImpl.channelList().size() == 1)
						{
							callbackEventType = ReactorChannelEventTypes.CHANNEL_DOWN;
						} else
						{
							callbackEventType = ReactorChannelEventTypes.FD_CHANGE;
						}
						if (event.errorInfo() != null)
						{
							event._errorInfo = event.errorInfo();
						}

						callbackChannel.warmStandbyChannelInfo().selectableChannelList()
								.remove(reactorChannel.channel().selectableChannel());

						int retval = sendChannelEventCallback(callbackEventType, callbackChannel, event._errorInfo);

						callbackChannel.warmStandbyChannelInfo().oldSelectableChannelList()
								.remove(reactorChannel.channel().selectableChannel());

						// check return code from callback.
						if (retval == ReactorCallbackReturnCodes.FAILURE)
						{
							populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
									"processWorkerCallback.RemoveServerFromWSBGroup",
									"ReactorCallbackReturnCodes.FAILURE was returned from reactorChannelEventCallback(). This caused the Reactor to shutdown.");
							shutdown(errorInfo);
							return ReactorReturnCodes.FAILURE;
						} else if (retval == ReactorCallbackReturnCodes.RAISE)
						{
							// RAISE is not a valid return code for the
							// reactorChannelEventCallback.
							populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
									"processWorkerCallback.RemoveServerFromWSBGroup",
									"ReactorCallbackReturnCodes.RAISE is not a valid return code from reactorChannelEventCallback(). This caused the Reactor to shutdown.");
							shutdown(errorInfo);
							return ReactorReturnCodes.FAILURE;

						} else if (retval != ReactorCallbackReturnCodes.SUCCESS)
						{
							// retval is not a valid ReactorReturnCodes.
							populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
									"processWorkerCallback.RemoveServerFromWSBGroup", "retval of " + retval
											+ " is not a valid ReactorCallbackReturnCodes. This caused the Reactor to shutdown.");
							shutdown(errorInfo);
							return ReactorReturnCodes.FAILURE;
						}
						
						// Increment the closing count for the group here
						((ReactorWarmStandbyGroupImpl)reactorChannel.warmStandByHandlerImpl
							.warmStandbyGroupList().get(reactorChannel.standByGroupListIndex))
								.incrementClosingStandbyCount();
						
						reactorChannel.warmStandByHandlerImpl.channelList().remove(reactorChannel);
						
						ret = closeChannel(reactorChannel, errorInfo);
						if (ret != ReactorReturnCodes.SUCCESS)
							return ret;

						break;
					case ReactorWarmStandbyEventTypes.CONNECT_SECONDARY_SERVER:
						int index = 0;
						ReactorWarmStandbyServerInfo warmStandbyServerInfo = null;
						ReactorChannel standbyReactorChannel = null;
						Watchlist startingWatchlist = null;
						ReactorTokenSession tokenSession = null;

						startingReactorChannel = warmStandByHandlerImpl.startingReactorChannel();
						startingWatchlist = startingReactorChannel.watchlist();

						// Checks whether all servers has been initiated a connection.
						if (warmStandByHandlerImpl.channelList().size() == warmStandbyGroup.standbyServerList().size()
								+ 1)
						{
							break;
						}

						for (; index < warmStandbyGroup.standbyServerList().size(); index++)
						{
							tokenSession = null;
							warmStandbyServerInfo = warmStandbyGroup.standbyServerList().get(index);

							standbyReactorChannel = ReactorFactory.createReactorChannel();

							/* just need to store the reference here */
							standbyReactorChannel._reactorConnectOptions = startingReactorChannel
									.getReactorConnectOptions();

							standbyReactorChannel
									.initializationTimeout(warmStandbyServerInfo.reactorConnectInfo().initTimeout());
							standbyReactorChannel.userSpecObj(startingReactorChannel.userSpecObj());
							standbyReactorChannel.flags(startingReactorChannel.flags);
							standbyReactorChannel.reconnectAttemptLimit(startingReactorChannel.reconnectAttemptLimit());
							standbyReactorChannel.reconnectMinDelay(startingReactorChannel.reconnectMinDelay());
							standbyReactorChannel.reconnectMaxDelay(startingReactorChannel.reconnectMaxDelay());
							standbyReactorChannel.reconnectAttemptCount(0);
							standbyReactorChannel.standByServerListIndex = index;
							standbyReactorChannel.standByGroupListIndex = warmStandByHandlerImpl
									.currentWarmStandbyGroupIndex();

							_reactorChannelQueue.pushBack(standbyReactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);

							
							standbyReactorChannel.setCurrentReactorConnectInfo(warmStandbyServerInfo.reactorConnectInfo());
							standbyReactorChannel.setCurrentConnectOptionsInfo(warmStandbyGroup.standbyConnectOptionsInfoList.get(index));

							// Copies the same channel role from the active server
							standbyReactorChannel.role(ReactorFactory.createConsumerRole());
							((ConsumerRole) standbyReactorChannel.role())
									.copy(((ConsumerRole) startingReactorChannel.role()));

							if (standbyReactorChannel.role().type() == ReactorRoleTypes.CONSUMER
									&& ((ConsumerRole) standbyReactorChannel.role()).watchlistOptions()
											.enableWatchlist())
							{
								Watchlist watchlist = new Watchlist(standbyReactorChannel,
										(ConsumerRole) standbyReactorChannel.role());
								standbyReactorChannel.watchlist(watchlist);
								watchlist.watchlistOptions().enableWarmStandby(true);
								watchlist._reactor = this;
								standbyReactorChannel.warmStandByHandlerImpl = warmStandByHandlerImpl;
								standbyReactorChannel.isActiveServer = false;

								if (warmStandbyServerInfo.reactorConnectInfo().enableSessionManagement())
								{

									// setup a rest client if any connections are requiring the session management
									try
									{
										setupRestClient(errorInfo);
									} catch (Exception e)
									{
										removeReactorChannel(standbyReactorChannel);
										standbyReactorChannel.returnToPool();
										_reactorChannelQueue.remove(standbyReactorChannel,
												ReactorChannel.REACTOR_CHANNEL_LINK);
										return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
												"Reactor.setupRestClient",
												"failed to initialize the RESTClient, exception="
														+ e.getLocalizedMessage());
									}
									ReactorOAuthCredential oAuthCredential;

									if ((oAuthCredential = retriveOAuthCredentialFromConsumerRole(
											standbyReactorChannel.role(), errorInfo)) != null)
									{
										tokenSession = getTokenSession(oAuthCredential, errorInfo);

										if (tokenSession == null)
										{
											standbyReactorChannel.returnToPool();
											return errorInfo.code();
										}

										standbyReactorChannel.tokenSession(tokenSession);
									} else
									{
										standbyReactorChannel.returnToPool();
										return errorInfo.code();
									}

									if (sessionManagementStartup(tokenSession,
											warmStandbyServerInfo.reactorConnectInfo(), standbyReactorChannel.role(),
											standbyReactorChannel, true, errorInfo) != ReactorReturnCodes.SUCCESS)
									{
										removeReactorChannel(standbyReactorChannel);
										standbyReactorChannel.returnToPool();
										return errorInfo.code();
									}

									standbyReactorChannel.applyAccessToken();

									/* Set original expires in when sending request using password grant type */
									tokenSession.originalExpiresIn(tokenSession.authTokenInfo().expiresIn());

									sendAuthTokenEventCallback(standbyReactorChannel, tokenSession.authTokenInfo(),
											errorInfo);
									standbyReactorChannel
											.sessionMgntState(ReactorChannel.SessionMgntState.RECEIVED_AUTH_TOKEN);
									if (!tokenSession.isInitialized())
									{
										boolean retVal;
										if (tokenSession.authTokenInfo().tokenVersion() == TokenVersion.V1)
										{
											retVal = sendAuthTokenWorkerEvent(tokenSession);
										} else
										{
											retVal = sendAuthTokenWorkerEvent(standbyReactorChannel, tokenSession);
										}

										if (retVal == false)
										{
											removeReactorChannel(standbyReactorChannel);
											standbyReactorChannel.returnToPool();
											_reactorChannelQueue.remove(standbyReactorChannel,
													ReactorChannel.REACTOR_CHANNEL_LINK);
											return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
													"Reactor.connect", "sendAuthTokenWorkerEvent() failed");
										}
									}
									/* Clears OAuth sensitive information if the callback is specified */
									if (tokenSession.oAuthCredential().reactorOAuthCredentialEventCallback() != null)
									{
										tokenSession.oAuthCredential().password().clear();
										tokenSession.oAuthCredential().clientSecret().clear();
									}

								}

								if (watchlist != null)
								{
									// Add consumer requests, if provided
									if (((ConsumerRole) standbyReactorChannel.role()).rdmLoginRequest() != null)
									{
										if (watchlist.submitMsg(
												((ConsumerRole) standbyReactorChannel.role()).rdmLoginRequest(),
												reactorSubmitOptions, errorInfo) != ReactorReturnCodes.SUCCESS)
										{
											++_reactorChannelCount;
											cleanUpWSBRequestQueue(standbyReactorChannel.warmStandByHandlerImpl,
													standbyReactorChannel.warmStandByHandlerImpl
															.currentWarmStandbyGroupImpl());
											// Increment the closing count for the group here
											((ReactorWarmStandbyGroupImpl)reactorChannel.warmStandByHandlerImpl.warmStandbyGroupList().get(reactorChannel.standByGroupListIndex))
												.incrementClosingStandbyCount();

											if (closeChannel(standbyReactorChannel,
													errorInfo) != ReactorReturnCodes.SUCCESS)
												return ReactorReturnCodes.FAILURE;

											continue;
										}
									} else if (startingWatchlist.loginHandler()._loginRequest != null)
									{
										if (watchlist.submitMsg(startingWatchlist.loginHandler()._loginRequest,
												reactorSubmitOptions, errorInfo) != ReactorReturnCodes.SUCCESS)
										{
											++_reactorChannelCount;
											cleanUpWSBRequestQueue(standbyReactorChannel.warmStandByHandlerImpl,
													standbyReactorChannel.warmStandByHandlerImpl
															.currentWarmStandbyGroupImpl());
											
											// Increment the closing count for the group here
											((ReactorWarmStandbyGroupImpl)reactorChannel.warmStandByHandlerImpl
												.warmStandbyGroupList().get(reactorChannel.standByGroupListIndex))
													.incrementClosingStandbyCount();
											
											if (closeChannel(standbyReactorChannel,
													errorInfo) != ReactorReturnCodes.SUCCESS)
												return ReactorReturnCodes.FAILURE;
											continue;
										}
									}

									if (((ConsumerRole) standbyReactorChannel.role()).rdmDirectoryRequest() != null)
									{
										if (watchlist.submitMsg(
												((ConsumerRole) standbyReactorChannel.role()).rdmDirectoryRequest(),
												reactorSubmitOptions, errorInfo) != ReactorReturnCodes.SUCCESS)
										{
											++_reactorChannelCount;
											cleanUpWSBRequestQueue(standbyReactorChannel.warmStandByHandlerImpl,
													standbyReactorChannel.warmStandByHandlerImpl
															.currentWarmStandbyGroupImpl());
											
											// Increment the closing count for the group here
											((ReactorWarmStandbyGroupImpl)reactorChannel.warmStandByHandlerImpl
												.warmStandbyGroupList().get(reactorChannel.standByGroupListIndex))
													.incrementClosingStandbyCount();
											
											if (closeChannel(standbyReactorChannel,
													errorInfo) != ReactorReturnCodes.SUCCESS)
												return ReactorReturnCodes.FAILURE;
											continue;
										}
									}
								}

								warmStandByHandlerImpl.channelList().add(standbyReactorChannel);

							
								// call Transport.connect to create a new Channel
								Channel channel = Transport.connect(
										warmStandbyServerInfo.reactorConnectInfo().connectOptions(), errorInfo.error());
								standbyReactorChannel.selectableChannelFromChannel(channel);
								standbyReactorChannel.reactor(this);
								++_reactorChannelCount;

								// TODO consider adding sendAuthTokenEvent here

								if (_reactorOptions.debuggerOptions().debugConnectionLevel())
								{
									if (channel != null)
									{
										debugger.writeDebugInfo(ReactorDebugger.CONNECTION_CONNECTING_PERFORMED,
												this.hashCode(), standbyReactorChannel.hashCode(),
												ReactorDebugger.getChannelId(standbyReactorChannel));
									} else
									{
										debugger.writeDebugInfo(ReactorDebugger.CONNECTION_CHANNEL_DOWN,
												this.hashCode(), standbyReactorChannel.hashCode(),
												ReactorDebugger.getChannelId(standbyReactorChannel));
									}
								}
								
								// The channel has been set to EDP_RT_DONE if session management is turned on, so set the state to 
								// INITIALIZING here
								standbyReactorChannel.state(ReactorChannel.State.INITIALIZING);
								

								if (channel == null)
								{
									if (standbyReactorChannel.server() == null
											&& !standbyReactorChannel.recoveryAttemptLimitReached()) // client
																										// channel
									{
										standbyReactorChannel.state(State.DOWN_RECONNECTING);

										// send CHANNEL_DOWN to user app via reactorChannelEventCallback.
										sendAndHandleChannelEventCallback("Reactor.connect",
												ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING,
												standbyReactorChannel, errorInfo);
									} else // server channel or no more retries
									{
										standbyReactorChannel.state(State.DOWN);

										// send CHANNEL_DOWN to user app via reactorChannelEventCallback.
										sendAndHandleChannelEventCallback("Reactor.connect",
												ReactorChannelEventTypes.CHANNEL_DOWN, standbyReactorChannel,
												errorInfo);
									}
								}
								// send a WorkerEvent to the Worker to initialize this channel.
								else if (!sendWorkerEvent(WorkerEventTypes.CHANNEL_INIT, standbyReactorChannel))
								{
									// sendWorkerEvent() failed, send channel down
									standbyReactorChannel.state(State.DOWN);
									sendAndHandleChannelEventCallback("Reactor.connect",
											ReactorChannelEventTypes.CHANNEL_DOWN, standbyReactorChannel, errorInfo);

									removeReactorChannel(standbyReactorChannel);
									standbyReactorChannel.returnToPool();
									_reactorChannelQueue.remove(standbyReactorChannel,
											ReactorChannel.REACTOR_CHANNEL_LINK);
									return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.connect",
											"sendWorkerEvent() failed");
								}
							}
						}
						break;
					default:
						break;
					}
				}
			}
			break;
		default:
			event.returnToPool();
			return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.processWorkerEvent",
					"received an unexpected WorkerEventType of " + eventType);
		}

		event.returnToPool();
		return errorInfo.code();
	}

	/*
	 * Go through the current channel's services and set them to active if the
	 * service is looking for it. This will be called after the initial
	 * directory response has been received and parsed for both configured active and
	 * standby channels
	 */
	void reactorWSBHandleServiceActiveStandby(ReactorChannel reactorChannel,
			ReactorWarmStandbyGroupImpl warmStandbyGroup, boolean initialActive, ReactorErrorInfo errorInfo)
	{
		ReactorWSBService service;
		WlService wlService;
		if (warmStandbyGroup.warmStandbyMode() == ReactorWarmStandbyMode.SERVICE_BASED)
		{
			for (int i = 0; i < reactorChannel.watchlist().directoryHandler()._serviceCache._serviceList
					.size(); i++)
			{
				wlService = reactorChannel.watchlist().directoryHandler()._serviceCache._serviceList
						.get(i);
				boolean activeService = this.wsbServiceInStartupList(warmStandbyGroup, wlService,
						reactorChannel);
				service = warmStandbyGroup._perServiceById.get(wlService._tableKey);
				
				if(service.activeChannel == reactorChannel)
					continue;

				if (service.activeChannel == null && wlService._rdmService.state().serviceState() == 1 && activeService)
				{
					reactorChannel._directoryConsumerStatus.clear();
					reactorChannel._directoryConsumerStatus
							.streamId(reactorChannel.watchlist().directoryHandler()._directoryStreamId);
					reactorChannel._serviceConsumerStatus.clear();
					reactorChannel._serviceConsumerStatus
							.flags(ConsumerStatusServiceFlags.HAS_WARM_STANDY_MODE);
					reactorChannel._serviceConsumerStatus
							.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
					reactorChannel._serviceConsumerStatus.serviceId(wlService._rdmService.serviceId());
					reactorChannel._directoryConsumerStatus.consumerServiceStatusList()
							.add(reactorChannel._serviceConsumerStatus);

					/*
					 * Write directly to the channel without involving the watchlist or wsb message
					 * queues
					 */
					if (submitChannel(reactorChannel, reactorChannel._directoryConsumerStatus,
							reactorSubmitOptions, errorInfo) < ReactorReturnCodes.SUCCESS)
					{
						if (reactorChannel.server() == null
								&& !reactorChannel.recoveryAttemptLimitReached()) // client channel
						{
							reactorChannel.state(State.DOWN_RECONNECTING);
							sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel,
									errorInfo);
							return;
						} else // server channel or no more retries
						{
							reactorChannel.state(State.DOWN);
							sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel,
									errorInfo);
							return;
						}
					}

					service.activeChannel = reactorChannel;
				} else
				{
					
					reactorChannel._directoryConsumerStatus.clear();
					reactorChannel._directoryConsumerStatus
							.streamId(reactorChannel.watchlist().directoryHandler()._directoryStreamId);
					reactorChannel._serviceConsumerStatus.clear();
					reactorChannel._serviceConsumerStatus
							.flags(ConsumerStatusServiceFlags.HAS_WARM_STANDY_MODE);
					reactorChannel._serviceConsumerStatus
							.warmStandbyMode(WarmStandbyDirectoryServiceTypes.STANDBY);
					reactorChannel._serviceConsumerStatus.serviceId(wlService._rdmService.serviceId());
					reactorChannel._directoryConsumerStatus.consumerServiceStatusList()
							.add(reactorChannel._serviceConsumerStatus);

					/*
					 * Write directly to the channel without involving the watchlist or wsb message
					 * queues
					 */
					if (submitChannel(reactorChannel, reactorChannel._directoryConsumerStatus,
							reactorSubmitOptions, errorInfo) < ReactorReturnCodes.SUCCESS)
					{
						if (reactorChannel.server() == null
								&& !reactorChannel.recoveryAttemptLimitReached()) // client channel
						{
							reactorChannel.state(State.DOWN_RECONNECTING);
							sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel,
									errorInfo);
							return;
						} else // server channel or no more retries
						{
							reactorChannel.state(State.DOWN);
							sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
									ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel,
									errorInfo);
							return;
						}
					}
				}
			}
			
			if(!initialActive)
			{
				if ((submitWSBRequestQueue(reactorChannel.warmStandByHandlerImpl,
						reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl(), reactorChannel,
						errorInfo)) != ReactorReturnCodes.SUCCESS)
				{
					/* Submit has failed, close out the server */
					if (reactorChannel.server() == null
							&& !reactorChannel.recoveryAttemptLimitReached()) // client channel
					{
						reactorChannel.state(State.DOWN_RECONNECTING);
						sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
								ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel,
								errorInfo);
						return;
					} else // server channel or no more retries
					{
						reactorChannel.state(State.DOWN);
						sendAndHandleChannelEventCallback("Reactor.processWorkerEvent",
								ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel,
								errorInfo);
						return;
					}
				}
			}
		}
	}

	private void processWorkerShutdown(WorkerEvent event, String location, ReactorErrorInfo errorInfo)
	{
		populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
				"Worker has shutdown, " + event.errorInfo().toString());
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
						ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
			} else // server channel or no more retries
			{
				// send CHANNEL_DOWN since server channels are not recovered
				reactorChannel.state(State.DOWN);

				// send channel_down to user app via reactorChannelEventCallback.
				sendAndHandleChannelEventCallback("Reactor.processChannelDown", ReactorChannelEventTypes.CHANNEL_DOWN,
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

		reactorChannel.state(State.UP);

		// If channel has no watchlist, consider connection established and reset the
		// reconnect timer.
		if (reactorChannel.watchlist() == null)
			reactorChannel.resetReconnectTimers();

		// Reset aggregating statistics
		_readArgsAggregator.clear();
		_writeArgsAggregator.clear();
		reactorChannel.pingHandler().resetAggregatedStats();

		if (_reactorOptions.debuggerOptions().debugConnectionLevel())
		{
			debugger.writeDebugInfo(ReactorDebugger.CONNECTION_CHANNEL_UP, this.hashCode(), reactorChannel.hashCode(),
					ReactorDebugger.getChannelId(reactorChannel));
		}

		// send channel_up to user app via reactorChannelEventCallback.
		if (sendAndHandleChannelEventCallback("Reactor.processChannelUp", ReactorChannelEventTypes.CHANNEL_UP,
				reactorChannel, errorInfo) != ReactorCallbackReturnCodes.SUCCESS)
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
				loginRequest = ((ConsumerRole) reactorRole).rdmLoginRequest();

			if (loginRequest != null)
			{
				if (reactorChannel.watchlist() == null) // watchlist not enabled
				{
					// a rdmLoginRequest was specified, send it out.
					encodeAndWriteLoginRequest(loginRequest, reactorChannel, errorInfo);
				} else // watchlist enabled
				{
					if (reactorChannel.watchlist()._loginHandler != null
							&& reactorChannel.watchlist()._loginHandler._loginRequestForEDP != null)
					{
						reactorChannel.watchlist()._loginHandler._loginRequestForEDP.userName(loginRequest.userName());
					}
					reactorChannel.watchlist()._loginHandler.rttEnabled = loginRequest.attrib()
							.checkHasSupportRoundTripLatencyMonitoring();
					reactorChannel.watchlist().channelUp(errorInfo);
				}
			} else
			{
				// no rdmLoginRequest defined, so just send CHANNEL_READY
				reactorChannel.state(State.READY);

				if (sendAndHandleChannelEventCallback("Reactor.processChannelUp",
						ReactorChannelEventTypes.CHANNEL_READY, reactorChannel,
						errorInfo) != ReactorCallbackReturnCodes.SUCCESS)
				{
					return;
				}
			}
		} else if (reactorRole.type() == ReactorRoleTypes.NIPROVIDER)
		{
			if (reactorChannel.state() == State.CLOSED || reactorChannel.state() == State.DOWN)
				return;

			LoginRequest loginRequest = ((NIProviderRole) reactorRole).rdmLoginRequest();
			if (loginRequest != null)
			{
				// a rdmLoginRequest was specified, send it out.
				encodeAndWriteLoginRequest(loginRequest, reactorChannel, errorInfo);
			} else
			{
				// no rdmLoginRequest defined, so just send CHANNEL_READY
				reactorChannel.state(State.READY);
				reactorChannel.clearAccessTokenForV2();
				if (sendAndHandleChannelEventCallback("Reactor.processChannelUp",
						ReactorChannelEventTypes.CHANNEL_READY, reactorChannel,
						errorInfo) != ReactorCallbackReturnCodes.SUCCESS)
				{
					return;
				}
			}
		} else if (reactorRole.type() == ReactorRoleTypes.PROVIDER)
		{
			// send CHANNEL_READY
			reactorChannel.state(State.READY);
			if (sendAndHandleChannelEventCallback("Reactor.processChannelUp", ReactorChannelEventTypes.CHANNEL_READY,
					reactorChannel, errorInfo) != ReactorCallbackReturnCodes.SUCCESS)
			{
				return;
			}
		}
	}

	private void encodeAndWriteLoginRequest(LoginRequest loginRequest, ReactorChannel reactorChannel,
			ReactorErrorInfo errorInfo)
	{
		// get a buffer for the login request
		Channel channel = reactorChannel.channel();
		if (channel == null)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteLoginRequest",
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
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteLoginRequest",
					"Failed to obtain a TransportBuffer, reason=" + errorInfo.error().text());
			return;
		}

		_eIter.clear();
		_eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

		int retval = loginRequest.encode(_eIter);
		if (retval != CodecReturnCodes.SUCCESS)
		{
			// set reactorChannel.state(State.DOWN) and notify the application (via
			// reactorChannelCallback(CHANNEL_DOWN, errorInfo)
			reactorChannel.state(State.DOWN);
			sendAndHandleChannelEventCallback("Reactor.encodeAndWriteLoginRequest",
					ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteLoginRequest",
					"Encoding of login request failed: <" + TransportReturnCodes.toString(retval) + ">");
			return;
		}

		// Checks the channel's protocol type to perform auto conversion for the JSON
		// protocol
		if (reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
		{
			if (Objects.isNull(jsonConverter))
			{
				populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
						"The JSON converter library has not been initialized properly.");
				return;
			}

			jsonDecodeMsg.clear();
			_dIter.clear();
			int ret = _dIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(),
					reactorChannel.minorVersion());

			ret = jsonDecodeMsg.decode(_dIter);

			if (ret == CodecReturnCodes.SUCCESS)
			{
				converterError.clear();
				rwfToJsonOptions.clear();
				rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

				if (jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults,
						converterError) != CodecReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());

					return;
				}

				TransportBuffer buffer = reactorChannel.getBuffer(conversionResults.getLength(), false, errorInfo);

				if (Objects.isNull(buffer))
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to get a buffer for sending JSON message. Error text: " + errorInfo.error().text());

					return;
				}

				getJsonMsgOptions.clear();
				getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
				getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false);

				if (jsonConverter.getJsonBuffer(buffer, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to get converted JSON message. Error text: " + converterError.getText());
					return;
				}

				/* Release the original buffer */
				reactorChannel.releaseBuffer(msgBuf, errorInfo);

				msgBuf = buffer;
			}
		}

		if (_reactorOptions.xmlTraceWrite())
		{
			xmlString.setLength(0);
			xmlString.append("\n<!-- Outgoing Reactor message -->\n").append("<!-- ")
					.append(reactorChannel.selectableChannel().toString()).append(" -->\n").append("<!-- ")
					.append(new java.util.Date()).append(" -->\n");
			xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null,
					xmlString, errorInfo.error());
			System.out.println(xmlString);
			if (_reactorOptions.xmlTracing()){
				System.out.println(xmlString);
			}
			if (_reactorOptions.xmlTraceToFile()) {
				_fileDumper.dump(xmlString.toString());
			}
		}
		retval = channel.write(msgBuf, _writeArgs, errorInfo.error());

		// Aggregate number of bytes written
		if (_reactorOptions.writeStatSet() == true)
		{
			((WriteArgsImpl) _writeArgsAggregator).bytesWritten(
					overflowSafeAggregate(_writeArgsAggregator.bytesWritten(), _writeArgs.bytesWritten()));
			((WriteArgsImpl) _writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(
					_writeArgsAggregator.uncompressedBytesWritten(), _writeArgs.uncompressedBytesWritten()));
		}

		if (retval > TransportReturnCodes.SUCCESS)
		{
			sendFlushRequest(reactorChannel, "Reactor.encodeAndWriteLoginRequest", errorInfo);
		} else if (retval < TransportReturnCodes.SUCCESS)
		{
			// write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
			// for user application.
			// also, set reactorChannel.state(State.DOWN) and notify the application (via
			// reactorChannelCallback(CHANNEL_DOWN, errorInfo))
			if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
			{
				reactorChannel.state(State.DOWN_RECONNECTING);
				sendAndHandleChannelEventCallback("Reactor.encodeAndWriteLoginRequest",
						ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
			} else // server channel or no more retries
			{
				reactorChannel.state(State.DOWN);
				sendAndHandleChannelEventCallback("Reactor.encodeAndWriteLoginRequest",
						ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
			}
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteLoginRequest",
					"Channel.write failed to write login request: <" + CodecReturnCodes.toString(retval) + ">"
							+ " error=" + errorInfo.error().text());
		} else
			reactorChannel.flushAgain(false);
	}

	/**
	 *
	 * @param msg            - generic msg which should be decoded and handled.
	 * @param reactorChannel - channel connection between client and server.
	 * @param errorInfo      - Warning/Error information buffer.
	 * @param decodeIterator - An iterator for decoding the RWF content.
	 * @return true when message must be proceeded. Method is returned false if
	 *         result of this method should be ignored. For instance: when
	 *         {@link LoginMsg#rdmMsgType()} is {@link LoginMsgType#RTT} and RTT
	 *         messaging is not supported by a {@link ConsumerRole}
	 */
	@SuppressWarnings("fallthrough")
	private boolean proceedLoginGenericMsg(ReactorChannel reactorChannel, DecodeIterator decodeIterator, Msg msg,
			ReactorErrorInfo errorInfo)
	{
		LoginMsg loginGenericMsg = _loginMsg;
		if (Objects.equals(DataTypes.ELEMENT_LIST, msg.containerType()))
		{
			loginGenericMsg.rdmMsgType(LoginMsgType.RTT);
			switch (reactorChannel.role().type())
			{
			case ReactorRoleTypes.PROVIDER:
				break;
			case ReactorRoleTypes.CONSUMER:
			{
				ConsumerRole consumerRole = (ConsumerRole) reactorChannel.role();
				if (consumerRole.rttEnabled())
				{
					returnBackRTTMessage(msg, reactorChannel, errorInfo);
					break;
				}
			}
			default:
			{
				/*
				 * return false when it is not enabled for consumer or when it is NIProvider for
				 * preventing further handling
				 */
				return false;
			}
			}
		} else
		{
			loginGenericMsg.rdmMsgType(LoginMsgType.CONSUMER_CONNECTION_STATUS);
		}
		loginGenericMsg.decode(decodeIterator, msg);
		return true;
	}

	private void returnBackRTTMessage(Msg msg, ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
	{
		reactorSubmitOptions.clear();
		int retval = submitChannel(reactorChannel, msg, reactorSubmitOptions, errorInfo);

		if (retval != CodecReturnCodes.SUCCESS)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.returnBackRTTMessage",
					"Reactor.submitChannel failed to return back login RTT message: <"
							+ CodecReturnCodes.toString(retval) + ">" + " error=" + errorInfo.error().text());
		}
	}

	private void encodeAndWriteDirectoryRequest(DirectoryRequest directoryRequest, ReactorChannel reactorChannel,
			ReactorErrorInfo errorInfo)
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
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDirectoryRequest",
					"Failed to obtain a TransportBuffer, reason=" + errorInfo.error().text());
			return;
		}

		_eIter.clear();
		_eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

		int retval = directoryRequest.encode(_eIter);
		if (retval != CodecReturnCodes.SUCCESS)
		{
			// set reactorChannel.state(State.DOWN) and notify the application (via
			// reactorChannelCallback(CHANNEL_DOWN, errorInfo)
			reactorChannel.state(State.DOWN);
			sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRequest",
					ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDirectoryRequest",
					"Encoding of directory request failed: <" + TransportReturnCodes.toString(retval) + ">");
			return;
		}

		// Checks the channel's protocol type to perform auto conversion for the JSON
		// protocol
		if (reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
		{
			if (Objects.isNull(jsonConverter))
			{
				populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
						"The JSON converter library has not been initialized properly.");
				return;
			}

			jsonDecodeMsg.clear();
			_dIter.clear();
			int ret = _dIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(),
					reactorChannel.minorVersion());

			ret = jsonDecodeMsg.decode(_dIter);

			if (ret == CodecReturnCodes.SUCCESS)
			{
				converterError.clear();
				rwfToJsonOptions.clear();
				rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

				if (jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults,
						converterError) != CodecReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());

					return;
				}

				TransportBuffer buffer = reactorChannel.getBuffer(conversionResults.getLength(), false, errorInfo);

				if (Objects.isNull(buffer))
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to get a buffer for sending JSON message. Error text: " + errorInfo.error().text());

					return;
				}

				getJsonMsgOptions.clear();
				getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
				getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false);

				if (jsonConverter.getJsonBuffer(buffer, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to get converted JSON message. Error text: " + converterError.getText());
					return;
				}

				/* Release the original buffer */
				reactorChannel.releaseBuffer(msgBuf, errorInfo);

				msgBuf = buffer;
			}
		}

		if (_reactorOptions.xmlTraceWrite())
		{
			xmlString.setLength(0);
			xmlString.append("\n<!-- Outgoing Reactor message -->\n").append("<!-- ")
					.append(reactorChannel.selectableChannel().toString()).append(" -->\n").append("<!-- ")
					.append(new java.util.Date()).append(" -->\n");
			xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null,
					xmlString, errorInfo.error());
			System.out.println(xmlString);
			if (_reactorOptions.xmlTracing()){
				System.out.println(xmlString);
			}
			if (_reactorOptions.xmlTraceToFile()) {
				_fileDumper.dump(xmlString.toString());
			}
		}

		retval = channel.write(msgBuf, _writeArgs, errorInfo.error());

		// Aggregate number of bytes written
		if (_reactorOptions.writeStatSet() == true)
		{
			((WriteArgsImpl) _writeArgsAggregator).bytesWritten(
					overflowSafeAggregate(_writeArgsAggregator.bytesWritten(), _writeArgs.uncompressedBytesWritten()));
			((WriteArgsImpl) _writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(
					_writeArgsAggregator.uncompressedBytesWritten(), _writeArgs.uncompressedBytesWritten()));
		}

		if (retval > TransportReturnCodes.SUCCESS)
		{
			sendFlushRequest(reactorChannel, "Reactor.encodeAndWriteDirectoryRequest", errorInfo);
		} else if (retval < TransportReturnCodes.SUCCESS)
		{
			// write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
			// for user application.
			// also, set reactorChannel.state(State.DOWN) and notify the application (via
			// reactorChannelCallback(CHANNEL_DOWN, errorInfo))
			if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
			{
				reactorChannel.state(State.DOWN_RECONNECTING);
				sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRequest",
						ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
			} else // server channel or no more retries
			{
				reactorChannel.state(State.DOWN);
				sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRequest",
						ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
			}
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDirectoryRequest",
					"Channel.write failed to write directory request: <" + CodecReturnCodes.toString(retval) + ">"
							+ " error=" + errorInfo.error().text());
		} else
			reactorChannel.flushAgain(false);
	}

	private void encodeAndWriteDirectoryRefresh(DirectoryRefresh directoryRefresh, ReactorChannel reactorChannel,
			ReactorErrorInfo errorInfo)
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
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDirectoryRefresh",
					"Failed to obtain a TransportBuffer, reason=" + errorInfo.error().text());
			return;
		}

		_eIter.clear();
		_eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

		int retval = directoryRefresh.encode(_eIter);
		if (retval != CodecReturnCodes.SUCCESS)
		{
			// set reactorChannel.state(State.DOWN) and notify the application (via
			// reactorChannelCallback(CHANNEL_DOWN, errorInfo)
			reactorChannel.state(State.DOWN);
			sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRefresh",
					ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDirectoryRefresh",
					"Encoding of directory refresh failed: <" + TransportReturnCodes.toString(retval) + ">");
			return;
		}

		// Checks the channel's protocol type to perform auto conversion for the JSON
		// protocol
		if (reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
		{
			if (Objects.isNull(jsonConverter))
			{
				populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
						"The JSON converter library has not been initialized properly.");
				return;
			}

			jsonDecodeMsg.clear();
			_dIter.clear();
			int ret = _dIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(),
					reactorChannel.minorVersion());

			ret = jsonDecodeMsg.decode(_dIter);

			if (ret == CodecReturnCodes.SUCCESS)
			{
				converterError.clear();
				rwfToJsonOptions.clear();
				rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

				if (jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults,
						converterError) != CodecReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());

					return;
				}

				TransportBuffer buffer = reactorChannel.getBuffer(conversionResults.getLength(), false, errorInfo);

				if (Objects.isNull(buffer))
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to get a buffer for sending JSON message. Error text: " + errorInfo.error().text());

					return;
				}

				getJsonMsgOptions.clear();
				getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
				getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false);

				if (jsonConverter.getJsonBuffer(buffer, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to get converted JSON message. Error text: " + converterError.getText());
					return;
				}

				/* Release the original buffer */
				reactorChannel.releaseBuffer(msgBuf, errorInfo);

				msgBuf = buffer;
			}
		}

		if (_reactorOptions.xmlTraceWrite())
		{
			xmlString.setLength(0);
			xmlString.append("\n<!-- Outgoing Reactor message -->\n").append("<!-- ")
					.append(reactorChannel.selectableChannel().toString()).append(" -->\n").append("<!-- ")
					.append(new java.util.Date()).append(" -->\n");
			xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null,
					xmlString, errorInfo.error());
			System.out.println(xmlString);
			if (_reactorOptions.xmlTracing()){
				System.out.println(xmlString);
			}
			if (_reactorOptions.xmlTraceToFile()) {
				_fileDumper.dump(xmlString.toString());
			}
		}
		retval = channel.write(msgBuf, _writeArgs, errorInfo.error());

		// Aggregate number of bytes written
		if (_reactorOptions.writeStatSet() == true)
		{
			((WriteArgsImpl) _writeArgsAggregator).bytesWritten(
					overflowSafeAggregate(_writeArgsAggregator.bytesWritten(), _writeArgs.bytesWritten()));
			((WriteArgsImpl) _writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(
					_writeArgsAggregator.uncompressedBytesWritten(), _writeArgs.uncompressedBytesWritten()));
		}

		if (retval > TransportReturnCodes.SUCCESS)
		{
			sendFlushRequest(reactorChannel, "Reactor.encodeAndWriteDirectoryRefresh", errorInfo);
		} else if (retval < TransportReturnCodes.SUCCESS)
		{
			// write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
			// for user application.
			// also, set reactorChannel.state(State.DOWN) and notify the application (via
			// reactorChannelCallback(CHANNEL_DOWN, errorInfo))
			if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
			{
				reactorChannel.state(State.DOWN_RECONNECTING);
				sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRefresh",
						ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
			} else // server channel or no more retries
			{
				reactorChannel.state(State.DOWN);
				sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDirectoryRefresh",
						ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
			}
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDirectoryRefresh",
					"Channel.write failed to write directory refresh: <" + CodecReturnCodes.toString(retval) + ">"
							+ " error=" + errorInfo.error().text());
		} else
			reactorChannel.flushAgain(false);
	}

	private void encodeAndWriteDictionaryRequest(DictionaryRequest dictionaryRequest, ReactorChannel reactorChannel,
			ReactorErrorInfo errorInfo)
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
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDictionaryRequest",
					"Failed to obtain a TransportBuffer, reason=" + errorInfo.error().text());
			return;
		}

		_eIter.clear();
		_eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

		int retval = dictionaryRequest.encode(_eIter);
		if (retval != CodecReturnCodes.SUCCESS)
		{
			// set reactorChannel.state(State.DOWN) and notify the application (via
			// reactorChannelCallback(CHANNEL_DOWN, errorInfo)
			reactorChannel.state(State.DOWN);
			sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryRequest",
					ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDictionaryRequest",
					"Encoding of dictionary request failed: <" + TransportReturnCodes.toString(retval) + ">");
			return;
		}

		// Checks the channel's protocol type to perform auto conversion for the JSON
		// protocol
		if (reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
		{
			if (Objects.isNull(jsonConverter))
			{
				populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
						"The JSON converter library has not been initialized properly.");
				return;
			}

			jsonDecodeMsg.clear();
			_dIter.clear();
			int ret = _dIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(),
					reactorChannel.minorVersion());

			ret = jsonDecodeMsg.decode(_dIter);

			if (ret == CodecReturnCodes.SUCCESS)
			{
				converterError.clear();
				rwfToJsonOptions.clear();
				rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

				if (jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults,
						converterError) != CodecReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());
					return;
				}

				TransportBuffer jsonBuffer = reactorChannel.getBuffer(conversionResults.getLength(), false, errorInfo);

				if (Objects.isNull(jsonBuffer))
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to get a buffer for sending JSON message. Error text: " + errorInfo.error().text());
					return;
				}

				getJsonMsgOptions.clear();
				getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
				getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false);

				if (jsonConverter.getJsonBuffer(jsonBuffer, getJsonMsgOptions,
						converterError) != CodecReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to get converted JSON message. Error text: " + converterError.getText());
					return;
				}

				/* Releases the original message buffer */
				reactorChannel.releaseBuffer(msgBuf, errorInfo);

				msgBuf = jsonBuffer;
			}
		}

		if (_reactorOptions.xmlTraceWrite())
		{
			xmlString.setLength(0);
			xmlString.append("\n<!-- Outgoing Reactor message -->\n").append("<!-- ")
					.append(reactorChannel.selectableChannel().toString()).append(" -->\n").append("<!-- ")
					.append(new java.util.Date()).append(" -->\n");
			xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null,
					xmlString, errorInfo.error());
			System.out.println(xmlString);
			if (_reactorOptions.xmlTracing()){
				System.out.println(xmlString);
			}
			if (_reactorOptions.xmlTraceToFile()) {
				_fileDumper.dump(xmlString.toString());
			}
		}
		retval = channel.write(msgBuf, _writeArgs, errorInfo.error());

		// Aggregate number of bytes written
		if (_reactorOptions.writeStatSet() == true)
		{
			((WriteArgsImpl) _writeArgsAggregator).bytesWritten(
					overflowSafeAggregate(_writeArgsAggregator.bytesWritten(), _writeArgs.bytesWritten()));
			((WriteArgsImpl) _writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(
					_writeArgsAggregator.uncompressedBytesWritten(), _writeArgs.uncompressedBytesWritten()));
		}
		if (retval > TransportReturnCodes.SUCCESS)
		{
			sendFlushRequest(reactorChannel, "Reactor.encodeAndWriteDictionaryRequest", errorInfo);
		} else if (retval < TransportReturnCodes.SUCCESS)
		{
			// write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
			// for user application.
			// also, set reactorChannel.state(State.DOWN) and notify the application (via
			// reactorChannelCallback(CHANNEL_DOWN, errorInfo))
			if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
			{
				reactorChannel.state(State.DOWN_RECONNECTING);
				sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryRequest",
						ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
			} else // server channel or no more retries
			{
				reactorChannel.state(State.DOWN);
				sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryRequest",
						ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
			}
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDictionaryRequest",
					"Channel.write failed to write dictionary request: <" + CodecReturnCodes.toString(retval) + ">"
							+ " error=" + errorInfo.error().text());
		} else
			reactorChannel.flushAgain(false);
	}

	private void encodeAndWriteDictionaryClose(DictionaryClose dictionaryClose, ReactorChannel reactorChannel,
			ReactorErrorInfo errorInfo)
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
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDictionaryClose",
					"Failed to obtain a TransportBuffer, reason=" + errorInfo.error().text());
			return;
		}

		_eIter.clear();
		_eIter.setBufferAndRWFVersion(msgBuf, channel.majorVersion(), channel.minorVersion());

		int retval = dictionaryClose.encode(_eIter);
		if (retval != CodecReturnCodes.SUCCESS)
		{
			// set reactorChannel.state(State.DOWN) and notify the application (via
			// reactorChannelCallback(CHANNEL_DOWN, errorInfo)
			reactorChannel.state(State.DOWN);
			sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryClose",
					ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDictionaryClose",
					"Encoding of dictionary close failed: <" + TransportReturnCodes.toString(retval) + ">");
			return;
		}

		// Checks the channel's protocol type to perform auto conversion for the JSON
		// protocol
		if (reactorChannel.channel().protocolType() == Codec.JSON_PROTOCOL_TYPE)
		{
			if (Objects.isNull(jsonConverter))
			{
				populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
						"The JSON converter library has not been initialized properly.");
				return;
			}

			jsonDecodeMsg.clear();
			_dIter.clear();
			int ret = _dIter.setBufferAndRWFVersion(msgBuf, reactorChannel.majorVersion(),
					reactorChannel.minorVersion());

			ret = jsonDecodeMsg.decode(_dIter);

			if (ret == CodecReturnCodes.SUCCESS)
			{
				converterError.clear();
				rwfToJsonOptions.clear();
				rwfToJsonOptions.setJsonProtocolType(JsonProtocol.JSON_JPT_JSON2);

				if (jsonConverter.convertRWFToJson(jsonDecodeMsg, rwfToJsonOptions, conversionResults,
						converterError) != CodecReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to convert RWF to JSON protocol. Error text: " + converterError.getText());

					return;
				}

				TransportBuffer buffer = reactorChannel.getBuffer(conversionResults.getLength(), false, errorInfo);

				if (Objects.isNull(buffer))
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to get a buffer for sending JSON message. Error text: " + errorInfo.error().text());

					return;
				}

				getJsonMsgOptions.clear();
				getJsonMsgOptions.jsonProtocolType(JsonProtocol.JSON_JPT_JSON2);
				getJsonMsgOptions.isCloseMsg(jsonDecodeMsg.msgClass() == MsgClasses.CLOSE ? true : false);

				if (jsonConverter.getJsonBuffer(buffer, getJsonMsgOptions, converterError) != CodecReturnCodes.SUCCESS)
				{
					populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.submitChannel",
							"Failed to get converted JSON message. Error text: " + converterError.getText());
					return;
				}

				/* Release the original buffer */
				reactorChannel.releaseBuffer(msgBuf, errorInfo);

				msgBuf = buffer;
			}
		}

		if (_reactorOptions.xmlTraceWrite())
		{
			xmlString.setLength(0);
			xmlString.append("\n<!-- Outgoing Reactor message -->\n").append("<!-- ")
					.append(reactorChannel.selectableChannel().toString()).append(" -->\n").append("<!-- ")
					.append(new java.util.Date()).append(" -->\n");
			xmlDumpTrace.dumpBuffer(reactorChannel.channel(), reactorChannel.channel().protocolType(), msgBuf, null,
					xmlString, errorInfo.error());
			System.out.println(xmlString);
			if (_reactorOptions.xmlTracing()){
				System.out.println(xmlString);
			}
			if (_reactorOptions.xmlTraceToFile()) {
				_fileDumper.dump(xmlString.toString());
			}
		}
		retval = channel.write(msgBuf, _writeArgs, errorInfo.error());

		// Aggregate number of bytes written
		if (_reactorOptions.writeStatSet() == true)
		{
			((WriteArgsImpl) _writeArgsAggregator).bytesWritten(
					overflowSafeAggregate(_writeArgsAggregator.bytesWritten(), _writeArgs.bytesWritten()));
			((WriteArgsImpl) _writeArgsAggregator).uncompressedBytesWritten(overflowSafeAggregate(
					_writeArgsAggregator.uncompressedBytesWritten(), _writeArgs.uncompressedBytesWritten()));
		}

		if (retval > TransportReturnCodes.SUCCESS)
		{
			sendFlushRequest(reactorChannel, "Reactor.encodeAndWriteDictionaryClose", errorInfo);
		} else if (retval < TransportReturnCodes.SUCCESS)
		{
			// write failed, send CHANNEL_DOWN to Worker and populate ErrorInfo
			// for user application.
			// also, set reactorChannel.state(State.DOWN) and notify the application (via
			// reactorChannelCallback(CHANNEL_DOWN, errorInfo))
			if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client channel
			{
				reactorChannel.state(State.DOWN_RECONNECTING);
				sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryClose",
						ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
			} else // server channel or no more retries
			{
				reactorChannel.state(State.DOWN);
				sendAndHandleChannelEventCallback("Reactor.encodeAndWriteDictionaryClose",
						ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
			}
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.encodeAndWriteDictionaryClose",
					"Channel.write failed to write dictionary close: <" + CodecReturnCodes.toString(retval) + ">"
							+ " error=" + errorInfo.error().text());
		} else
			reactorChannel.flushAgain(false);
	}

	int processChannelMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg,
			TransportBuffer transportBuffer, ReactorErrorInfo errorInfo)
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
			retval = sendAndHandleDefaultMsgCallback("Reactor.processChannelMessage", reactorChannel, transportBuffer,
					msg, errorInfo);
			break;
		}

		return retval;
	}

	private int processLoginMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg,
			TransportBuffer transportBuffer, ReactorErrorInfo errorInfo)
	{
		int retval = ReactorCallbackReturnCodes.SUCCESS;
		LoginMsg loginMsg = null;

		_loginMsg.clear();
		switch (msg.msgClass())
		{
		case MsgClasses.REQUEST:
			LoginRequest loginRequest = (LoginRequest) _loginMsg;
			loginRequest.rdmMsgType(LoginMsgType.REQUEST);
			loginRequest.decode(dIter, msg);
			loginMsg = _loginMsg;
			break;
		case MsgClasses.REFRESH:
			LoginRefresh loginRefresh = (LoginRefresh) _loginMsg;
			loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
			loginRefresh.decode(dIter, msg);
			loginMsg = _loginMsg;
			break;
		case MsgClasses.STATUS:
			LoginStatus loginStatus = (LoginStatus) _loginMsg;
			loginStatus.rdmMsgType(LoginMsgType.STATUS);
			loginStatus.decode(dIter, msg);
			loginMsg = _loginMsg;
			break;
		case MsgClasses.CLOSE:
			LoginClose loginClose = (LoginClose) _loginMsg;
			loginClose.rdmMsgType(LoginMsgType.CLOSE);
			loginClose.decode(dIter, msg);
			loginMsg = _loginMsg;
			break;
		case MsgClasses.GENERIC:
			if (!proceedLoginGenericMsg(reactorChannel, dIter, msg, errorInfo))
			{
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

		if (retval != ReactorCallbackReturnCodes.FAILURE)
		{
			retval = sendAndHandleLoginMsgCallback("Reactor.processLoginMessage", reactorChannel, transportBuffer, msg,
					loginMsg, errorInfo);

			if (retval == ReactorCallbackReturnCodes.RAISE)
				retval = sendAndHandleDefaultMsgCallback("Reactor.processLoginMessage", reactorChannel, transportBuffer,
						msg, errorInfo);

			if (retval == ReactorCallbackReturnCodes.SUCCESS)
			{
				ReactorRole reactorRole = reactorChannel.role();
				/*
				 * check if this is a reactorChannel's role is CONSUMER, a Login REFRESH, if the
				 * reactorChannel State is UP, and that the loginRefresh's state was OK. If all
				 * this is true, check if a directoryRequest is populated. If so, send the
				 * directoryRequest. if not, change the reactorChannel state to READY.
				 */
				if (reactorChannel.state() == State.UP && reactorChannel.role().type() == ReactorRoleTypes.CONSUMER
						&& msg.streamId() == ((ConsumerRole) reactorRole).rdmLoginRequest().streamId()
						&& _loginMsg.rdmMsgType() == LoginMsgType.REFRESH
						&& ((LoginRefresh) _loginMsg).state().streamState() == StreamStates.OPEN
						&& ((LoginRefresh) _loginMsg).state().dataState() == DataStates.OK)
				{
					DirectoryRequest directoryRequest = ((ConsumerRole) reactorRole).rdmDirectoryRequest();
					if (directoryRequest != null)
					{
						// a rdmDirectoryRequest was specified, send it out.
						encodeAndWriteDirectoryRequest(directoryRequest, reactorChannel, errorInfo);
					} else
					{
						// no rdmDirectoryRequest defined, so just send CHANNEL_READY
						reactorChannel.state(State.READY);
						reactorChannel.clearAccessTokenForV2();
						if ((retval = sendAndHandleChannelEventCallback("Reactor.processLoginMessage",
								ReactorChannelEventTypes.CHANNEL_READY, reactorChannel,
								errorInfo)) != ReactorCallbackReturnCodes.SUCCESS)
						{
							return retval;
						}
					}
				}

				/*
				 * check if this is a reactorChannel's role is NIPROVIDER, a Login REFRESH, if
				 * the reactorChannel State is UP, and that the loginRefresh's state was OK. If
				 * all this is true, check if a directoryRefresh is populated. If so, send the
				 * directoryRefresh. if not, change the reactorChannel state to READY.
				 */
				if (reactorChannel.state() == State.UP && reactorChannel.role().type() == ReactorRoleTypes.NIPROVIDER
						&& msg.streamId() == ((NIProviderRole) reactorRole).rdmLoginRequest().streamId()
						&& _loginMsg.rdmMsgType() == LoginMsgType.REFRESH
						&& ((LoginRefresh) _loginMsg).state().streamState() == StreamStates.OPEN
						&& ((LoginRefresh) _loginMsg).state().dataState() == DataStates.OK)
				{
					DirectoryRefresh directoryRefresh = ((NIProviderRole) reactorRole).rdmDirectoryRefresh();
					if (directoryRefresh != null)
					{
						// a rdmDirectoryRefresh was specified, send it out.
						encodeAndWriteDirectoryRefresh(directoryRefresh, reactorChannel, errorInfo);
						if (((NIProviderRole) reactorRole)
								.dictionaryDownloadMode() == DictionaryDownloadModes.FIRST_AVAILABLE)
						{
							if (((LoginRefresh) _loginMsg).checkHasFeatures()
									&& ((LoginRefresh) _loginMsg).features().checkHasSupportProviderDictionaryDownload()
									&& ((LoginRefresh) _loginMsg).features().supportProviderDictionaryDownload() == 1)
							{
								int serviceId = ((NIProviderRole) reactorRole).rdmDirectoryRefresh().serviceList()
										.get(0).serviceId();
								DictionaryRequest dictionaryRequest;

								((NIProviderRole) reactorRole).initDefaultRDMFieldDictionaryRequest();
								dictionaryRequest = ((NIProviderRole) reactorRole).rdmFieldDictionaryRequest();
								dictionaryRequest.serviceId(serviceId);
								encodeAndWriteDictionaryRequest(dictionaryRequest, reactorChannel, errorInfo);

								((NIProviderRole) reactorRole).initDefaultRDMEnumDictionaryRequest();
								dictionaryRequest = ((NIProviderRole) reactorRole).rdmEnumDictionaryRequest();
								dictionaryRequest.serviceId(serviceId);
								encodeAndWriteDictionaryRequest(dictionaryRequest, reactorChannel, errorInfo);
							}
						}
					}

					// send CHANNEL_READY if dictionary downloading is not needed
					if (((NIProviderRole) reactorRole)
							.dictionaryDownloadMode() != DictionaryDownloadModes.FIRST_AVAILABLE)
					{
						reactorChannel.state(State.READY);
						if ((retval = sendAndHandleChannelEventCallback("Reactor.processLoginMessage",
								ReactorChannelEventTypes.CHANNEL_READY, reactorChannel,
								errorInfo)) != ReactorCallbackReturnCodes.SUCCESS)
						{
							return retval;
						}
					}
				}
			}
		}

		return retval;
	}

	private int processDirectoryMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg,
			TransportBuffer transportBuffer, ReactorErrorInfo errorInfo)
	{
		int retval = ReactorCallbackReturnCodes.SUCCESS;

		_directoryMsg.clear();
		switch (msg.msgClass())
		{
		case MsgClasses.REQUEST:
			DirectoryRequest directoryRequest = (DirectoryRequest) _directoryMsg;
			directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
			directoryRequest.decode(dIter, msg);
			break;
		case MsgClasses.REFRESH:
			DirectoryRefresh directoryRefresh = (DirectoryRefresh) _directoryMsg;
			directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
			directoryRefresh.decode(dIter, msg);
			break;
		case MsgClasses.STATUS:
			DirectoryStatus directoryStatus = (DirectoryStatus) _directoryMsg;
			directoryStatus.rdmMsgType(DirectoryMsgType.STATUS);
			directoryStatus.decode(dIter, msg);
			break;
		case MsgClasses.CLOSE:
			DirectoryClose directoryClose = (DirectoryClose) _directoryMsg;
			directoryClose.rdmMsgType(DirectoryMsgType.CLOSE);
			directoryClose.decode(dIter, msg);
			break;
		case MsgClasses.GENERIC:
			DirectoryConsumerStatus directoryCS = (DirectoryConsumerStatus) _directoryMsg;
			directoryCS.rdmMsgType(DirectoryMsgType.CONSUMER_STATUS);
			directoryCS.decode(dIter, msg);
			break;
		case MsgClasses.UPDATE:
			DirectoryUpdate directoryUpdate = (DirectoryUpdate) _directoryMsg;
			directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
			directoryUpdate.decode(dIter, msg);
			break;
		default:
			break;
		}

		retval = sendAndHandleDirectoryMsgCallback("Reactor.processDirectoryMessage", reactorChannel, transportBuffer,
				msg, _directoryMsg, errorInfo);

		if (retval == ReactorCallbackReturnCodes.RAISE)
			retval = sendAndHandleDefaultMsgCallback("Reactor.processDirectoryMessage", reactorChannel, transportBuffer,
					msg, errorInfo);

		if (retval == ReactorCallbackReturnCodes.SUCCESS)
		{
			/*
			 * check if this is a reactorChannel's role is CONSUMER, a Directory REFRESH,
			 * and if the reactorChannel State is UP. If all this is true, check
			 * dictionaryDownloadMode is FIRST_AVAILABLE.
			 */
			ReactorRole reactorRole = reactorChannel.role();
			if (reactorChannel.state() == State.UP
					&& msg.streamId() == ((ConsumerRole) reactorRole).rdmDirectoryRequest().streamId()
					&& reactorChannel.role().type() == ReactorRoleTypes.CONSUMER
					&& _directoryMsg.rdmMsgType() == DirectoryMsgType.REFRESH)
			{

				if (((ConsumerRole) reactorRole).dictionaryDownloadMode() == DictionaryDownloadModes.FIRST_AVAILABLE)
				{
					DirectoryRefresh directoryRefresh = (DirectoryRefresh) _directoryMsg;
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
								if (dictionaryName
										.equals(((ConsumerRole) reactorRole).fieldDictionaryName().toString()))
									hasFieldDictionary = true;

								if (dictionaryName
										.equals(((ConsumerRole) reactorRole).enumTypeDictionaryName().toString()))
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

							((ConsumerRole) reactorRole).initDefaultRDMFieldDictionaryRequest();
							dictionaryRequest = ((ConsumerRole) reactorRole).rdmFieldDictionaryRequest();
							dictionaryRequest.serviceId(serviceId);
							encodeAndWriteDictionaryRequest(dictionaryRequest, reactorChannel, errorInfo);

							((ConsumerRole) reactorRole).initDefaultRDMEnumDictionaryRequest();
							dictionaryRequest = ((ConsumerRole) reactorRole).rdmEnumDictionaryRequest();
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
				} else
				{
					// dictionaryDownloadMode is NONE, so just send CHANNEL_READY
					reactorChannel.state(State.READY);
					reactorChannel.clearAccessTokenForV2();
					if ((retval = sendAndHandleChannelEventCallback("Reactor.processDirectoryMessage",
							ReactorChannelEventTypes.CHANNEL_READY, reactorChannel,
							errorInfo)) != ReactorCallbackReturnCodes.SUCCESS)
					{
						return retval;
					}
				}
			}
		}

		return retval;
	}

	private int processDictionaryMessage(ReactorChannel reactorChannel, DecodeIterator dIter, Msg msg,
			TransportBuffer transportBuffer, ReactorErrorInfo errorInfo)
	{
		int retval = ReactorCallbackReturnCodes.SUCCESS;
		DictionaryRefresh dictionaryRefresh = null;

		_dictionaryMsg.clear();
		switch (msg.msgClass())
		{
		case MsgClasses.REQUEST:
			DictionaryRequest dictionaryRequest = (DictionaryRequest) _dictionaryMsg;
			dictionaryRequest.rdmMsgType(DictionaryMsgType.REQUEST);
			dictionaryRequest.decode(dIter, msg);
			break;
		case MsgClasses.REFRESH:
			dictionaryRefresh = (DictionaryRefresh) _dictionaryMsg;
			dictionaryRefresh.rdmMsgType(DictionaryMsgType.REFRESH);
			dictionaryRefresh.decode(dIter, msg);
			break;
		case MsgClasses.STATUS:
			DictionaryStatus dictionaryStatus = (DictionaryStatus) _dictionaryMsg;
			dictionaryStatus.rdmMsgType(DictionaryMsgType.STATUS);
			dictionaryStatus.decode(dIter, msg);
			break;
		case MsgClasses.CLOSE:
			DictionaryClose dictionaryClose = (DictionaryClose) _dictionaryMsg;
			dictionaryClose.rdmMsgType(DictionaryMsgType.CLOSE);
			dictionaryClose.decode(dIter, msg);
			break;
		default:
			break;
		}

		retval = sendAndHandleDictionaryMsgCallback("Reactor.processDictionaryMessage", reactorChannel, transportBuffer,
				msg, _dictionaryMsg, errorInfo);

		if (retval == ReactorCallbackReturnCodes.RAISE)
			retval = sendAndHandleDefaultMsgCallback("Reactor.processDictionaryMessage", reactorChannel,
					transportBuffer, msg, errorInfo);

		if (retval == ReactorCallbackReturnCodes.SUCCESS)
		{
			boolean receivedFieldDictResponse = false;
			boolean receivedEnumTypeResponse = false;
			/*
			 * check if this is a reactorChannel's role is CONSUMER, a Dictionary REFRESH,
			 * reactorChannel State is UP, and dictionaryDownloadMode is FIRST_AVAILABLE. If
			 * all this is true, close dictionary stream for this refresh.
			 */
			ReactorRole reactorRole = reactorChannel.role();
			if (reactorChannel.state() == State.UP && reactorChannel.role().type() == ReactorRoleTypes.CONSUMER
					&& _dictionaryMsg.rdmMsgType() == DictionaryMsgType.REFRESH
					&& ((ConsumerRole) reactorRole).dictionaryDownloadMode() == DictionaryDownloadModes.FIRST_AVAILABLE)
			{
				// field dictionary
				if (msg.streamId() == ((ConsumerRole) reactorRole).rdmFieldDictionaryRequest().streamId()
						&& dictionaryRefresh != null && dictionaryRefresh.checkRefreshComplete())
				{
					/*
					 * Close stream so its streamID is free for the user. When connecting to an ADS,
					 * there won't be any further messages on this stream -- the consumer will be
					 * disconnected if the dictionary version is changed.
					 */
					((ConsumerRole) reactorRole).receivedFieldDictionaryResp(true);
					encodeAndWriteDictionaryClose(((ConsumerRole) reactorRole).rdmFieldDictionaryClose(),
							reactorChannel, errorInfo);
				}

				// enum type dictionary
				if (msg.streamId() == ((ConsumerRole) reactorRole).rdmEnumDictionaryRequest().streamId()
						&& dictionaryRefresh != null && dictionaryRefresh.checkRefreshComplete())
				{
					/*
					 * Close stream so its streamID is free for the user. When connecting to an ADS,
					 * there won't be any further messages on this stream -- the consumer will be
					 * disconnected if the dictionary version is changed.
					 */
					((ConsumerRole) reactorRole).receivedEnumDictionaryResp(true);
					encodeAndWriteDictionaryClose(((ConsumerRole) reactorRole).rdmEnumDictionaryClose(), reactorChannel,
							errorInfo);
				}
				receivedFieldDictResponse = ((ConsumerRole) reactorRole).receivedFieldDictionaryResp();
				receivedEnumTypeResponse = ((ConsumerRole) reactorRole).receivedEnumDictionaryResp();

			} else if (reactorChannel.state() == State.UP && reactorChannel.role().type() == ReactorRoleTypes.NIPROVIDER
					&& _dictionaryMsg.rdmMsgType() == DictionaryMsgType.REFRESH && ((NIProviderRole) reactorRole)
							.dictionaryDownloadMode() == DictionaryDownloadModes.FIRST_AVAILABLE)
			{
				// field dictionary
				if (msg.streamId() == ((NIProviderRole) reactorRole).rdmFieldDictionaryRequest().streamId()
						&& dictionaryRefresh != null && dictionaryRefresh.checkRefreshComplete())
				{
					/*
					 * Close stream so its streamID is free for the user. When connecting to an ADS,
					 * there won't be any further messages on this stream -- the consumer will be
					 * disconnected if the dictionary version is changed.
					 */
					((NIProviderRole) reactorRole).receivedFieldDictionaryResp(true);

					encodeAndWriteDictionaryClose(((NIProviderRole) reactorRole).rdmFieldDictionaryClose(),
							reactorChannel, errorInfo);
				}

				// enum type dictionary
				if (msg.streamId() == ((NIProviderRole) reactorRole).rdmEnumDictionaryRequest().streamId()
						&& dictionaryRefresh != null && dictionaryRefresh.checkRefreshComplete())
				{
					/*
					 * Close stream so its streamID is free for the user. When connecting to an ADS,
					 * there won't be any further messages on this stream -- the consumer will be
					 * disconnected if the dictionary version is changed.
					 */
					((NIProviderRole) reactorRole).receivedEnumDictionaryResp(true);
					receivedEnumTypeResponse = true;
					encodeAndWriteDictionaryClose(((NIProviderRole) reactorRole).rdmEnumDictionaryClose(),
							reactorChannel, errorInfo);
				}

				receivedFieldDictResponse = ((NIProviderRole) reactorRole).receivedFieldDictionaryResp();
				receivedEnumTypeResponse = ((NIProviderRole) reactorRole).receivedEnumDictionaryResp();
			}

			// if both field and enum type refreshes received, send CHANNEL_READY
			if (receivedFieldDictResponse && receivedEnumTypeResponse)
			{
				reactorChannel.state(State.READY);
				reactorChannel.clearAccessTokenForV2();
				if ((retval = sendAndHandleChannelEventCallback("Reactor.processDictionaryMessage",
						ReactorChannelEventTypes.CHANNEL_READY, reactorChannel,
						errorInfo)) != ReactorCallbackReturnCodes.SUCCESS)
				{
					return retval;
				}
			}
		}

		return retval;
	}

	/**
	 * Process all channels' events and messages from the Reactor. These are passed
	 * to the calling application via the the callback methods associated with each
	 * of the channels.
	 *
	 * If keySet parameter is non-null, events and messages are processed from the
	 * channels associated with keys in the selected key set. Once processed, keys
	 * are removed from the selected key set.
	 *
	 * If keySet parameter is null, events and messages are processed from all
	 * channels in a round-robin manner.
	 *
	 * @param keySet          key set from the selector's registered channels
	 * @param dispatchOptions options for how to dispatch
	 * @param errorInfo       error structure to be populated in the event of
	 *                        failure
	 *
	 * @return a positive value if dispatching succeeded and there are more messages
	 *         to process or {@link ReactorReturnCodes#SUCCESS} if dispatching
	 *         succeeded and there are no more messages to process or
	 *         {@link ReactorReturnCodes#FAILURE}, if dispatching failed (refer to
	 *         errorInfo for additional information)
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
				return populateErrorInfo(errorInfo, ReactorReturnCodes.SHUTDOWN, "Reactor.dispatchAll",
						"Reactor is not active, aborting.");
			}

			if (_reactorOptions.debuggerOptions().debugEventQueueLevel())
			{
				debugger.writeDebugInfo(ReactorDebugger.EVENTQUEUE_COUNT_REACTOR, this.hashCode(),
						_workerQueue
								.countNumberOfReadQueueElements(node -> ((WorkerEvent) node).reactorChannel() == null
										|| ((WorkerEvent) node).reactorChannel() == this._reactorChannel));
				debugger.writeDebugInfo(ReactorDebugger.EVENTQUEUE_COUNT_ALL, this.hashCode(),
						_workerQueue.countNumberOfReadQueueElements(
								node -> ((WorkerEvent) node).reactorChannel() != this._reactorChannel
										&& ((WorkerEvent) node).reactorChannel() != null));
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
					if (key.isReadable() && _reactorChannel == (ReactorChannel) key.attachment())
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
								ReactorChannel reactorChnl = (ReactorChannel) key.attachment();

								if (!isReactorChannelReady(reactorChnl))
								{
									return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE,
											"Reactor.dispatchAll", "ReactorChannel is not active, aborting.");
								}

								/*
								 * Round robin through the channels associated with this warm standby handler,
								 * since the user can only register the main WSB channel.
								 */
								if (reactorChnl.reactor().reactorHandlesWarmStandby(reactorChnl))
								{
									for (int i = 0; i < reactorChnl.warmStandByHandlerImpl.channelList().size(); i++)
									{
										ReactorChannel channel = reactorChnl.warmStandByHandlerImpl.channelList()
												.get(i);

										if (key.channel() == channel.selectableChannel())
										{
											while (isReactorChannelReady(channel) && msgCount < maxMessages
													&& retval > 0)
											{
												int bytesReadBefore = dispatchOptions.readArgs()
														.uncompressedBytesRead();

												if ((retval = performChannelRead(channel, dispatchOptions.readArgs(),
														errorInfo)) < ReactorReturnCodes.SUCCESS)
												{
													if (channel.state() != ReactorChannel.State.CLOSED && channel
															.state() != ReactorChannel.State.DOWN_RECONNECTING)
													{
														return retval;
													} else
													{
														retval = ReactorReturnCodes.SUCCESS;
													}
												}

												if ((dispatchOptions.readArgs().uncompressedBytesRead()
														- bytesReadBefore) > 0)
												{
													msgCount++;
												}
											}
										}
									}
								} else
								{
									if (reactorChnl.warmStandByHandlerImpl != null && reactorChnl.warmStandByHandlerImpl
											.mainReactorChannelImpl() == reactorChnl)
									{
										reactorChnl = reactorChnl.warmStandByHandlerImpl.startingReactorChannel();
									}

									while (isReactorChannelReady(reactorChnl) && msgCount < maxMessages && retval > 0)
									{
										int bytesReadBefore = dispatchOptions.readArgs().uncompressedBytesRead();

										if ((retval = performChannelRead(reactorChnl, dispatchOptions.readArgs(),
												errorInfo)) < ReactorReturnCodes.SUCCESS)
										{
											if (reactorChnl.state() != ReactorChannel.State.CLOSED
													&& reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
											{
												return retval;
											} else
											{
												// return success since close or reconnecting is not an error
												retval = ReactorReturnCodes.SUCCESS;

											}
										}

										if ((dispatchOptions.readArgs().uncompressedBytesRead() - bytesReadBefore) > 0)
										{
											msgCount++;
										}
									}
								}
							}
						} catch (CancelledKeyException e)
						{
						} // key can be canceled during shutdown

						if (msgCount == maxMessages)
						{
							// update retval
							retval = keySet.size() + retval;
							break;
						}
					}
				} else // no keySet, round robin through all channels
				{
					_reactorChannelCount = 0;

					for (ReactorChannel reactorChnl = _reactorChannelQueue.start(
							ReactorChannel.REACTOR_CHANNEL_LINK); reactorChnl != null; reactorChnl = _reactorChannelQueue
									.forth(ReactorChannel.REACTOR_CHANNEL_LINK))
					{
						retval = 1;
						_reactorChannelCount++;

						if (!isReactorChannelReady(reactorChnl) || (reactorChnl.warmStandByHandlerImpl != null
								&& reactorChnl == reactorChnl.warmStandByHandlerImpl.mainReactorChannelImpl()))
						{
							continue;
						}

						while (isReactorChannelReady(reactorChnl) && msgCount < maxMessages && retval > 0)
						{
							int bytesReadBefore = dispatchOptions.readArgs().uncompressedBytesRead();
							if ((retval = performChannelRead(reactorChnl, dispatchOptions.readArgs(),
									errorInfo)) < ReactorReturnCodes.SUCCESS)
							{
								
								if (reactorChnl.state() != ReactorChannel.State.CLOSED
										&& reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
								{
									return retval;
								} else
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
			} else // maxMessages reached
			{
				// update retval
				retval = _workerQueue.readQueueSize() + (keySet != null ? keySet.size()
						: (_reactorChannelCount < _reactorChannelQueue.count() ? 1 : 0));
			}
		} finally
		{
			_reactorLock.unlock();
		}

		return retval;
	}

	/**
	 * Initializes this Reactor to be able to convert messages to and from RWF and
	 * JSON protocol.
	 *
	 * @param jsonConverterOptions specifies the options in
	 *                             {@link ReactorJsonConverterOptions} to initialize
	 *                             the converter library.
	 * @param errorInfo            specifies the {@link ReactorErrorInfo} to be
	 *                             populated in the event of an error.
	 *
	 * @return {@link ReactorReturnCodes#SUCCESS} if initialization succeeded;
	 *         failure codes otherwise.
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
				return populateErrorInfo(errorInfo, ReactorReturnCodes.SHUTDOWN, "Reactor.initJsonConverter",
						"Reactor is not active, aborting.");
			}

			if (Objects.nonNull(jsonConverter))
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.initJsonConverter",
						"JsonConverter library is already initialized.");
			}

			if (jsonConverterOptions.defaultServiceId() > 65535)
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.initJsonConverter",
						"The service ID must be in a range between 0 to 65535.");
			}

            jsonConverterUserSpec = jsonConverterOptions.userSpec();
            serviceNameToIdCallback = jsonConverterOptions.serviceNameToIdCallback();
            JsonConversionEventCallback = jsonConverterOptions.jsonConversionEventCallback();
            closeChannelFromFailure = jsonConverterOptions.closeChannelFromFailure();

            JsonFactory.initPools(jsonConverterOptions.jsonConverterPoolsSize());
            JsonConverterBuilder jsonConverterBuilder = ConverterFactory.createJsonConverterBuilder();

			if (Objects.isNull(serviceNameIdConverterClient))
				serviceNameIdConverterClient = new ServiceNameIdConverterClient(this);

			jsonConverterBuilder
					.setProperty(JsonConverterProperties.JSON_CPC_PROTOCOL_VERSION, JsonProtocol.JSON_JPT_JSON2)
					.setProperty(JsonConverterProperties.JSON_CPC_EXPAND_ENUM_FIELDS,
							jsonConverterOptions.jsonExpandedEnumFields())
					.setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_KEYS,
							jsonConverterOptions.catchUnknownJsonKeys())
					.setProperty(JsonConverterProperties.JSON_CPC_CATCH_UNKNOWN_JSON_FIDS,
							jsonConverterOptions.catchUnknownJsonFids())
					.setProperty(JsonConverterProperties.JSON_CPC_ALLOW_ENUM_DISPLAY_STRINGS, true) /*
																									 * Always enable
																									 * this feature
																									 */
					.setServiceConverter(serviceNameIdConverterClient)
					.setDictionary(jsonConverterOptions.dataDictionary());

			/* Sets the default service ID if specified by users. */
			if (jsonConverterOptions.defaultServiceId() >= 0)
			{
				jsonConverterBuilder.setProperty(JsonConverterProperties.JSON_CPC_DEFAULT_SERVICE_ID,
						jsonConverterOptions.defaultServiceId());
			}

			jsonConverter = jsonConverterBuilder.build(converterError);

			if (Objects.isNull(jsonConverter))
			{
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.initJsonConverter",
						converterError.getText());
			}

			sendJsonConvError = jsonConverterOptions.sendJsonConvError();
		} finally
		{
			_reactorLock.unlock();
		}

		return ReactorReturnCodes.SUCCESS;
	}

	int closeChannel(ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
	{
		_reactorLock.lock();

		if (_reactorOptions.debuggerOptions().debugConnectionLevel())
		{
			debugger.incNumOfCloseCalls();
			debugger.writeDebugInfo(ReactorDebugger.CONNECTION_CHANNEL_CLOSE, this.hashCode(),
					reactorChannel.hashCode(), ReactorDebugger.getChannelId(reactorChannel));
			debugger.writeDebugInfo(ReactorDebugger.CONNECTION_CHANNEL_CLOSE_NUM_OF_CALLS, this.hashCode(),
					reactorChannel.hashCode(), debugger.getNumOfCloseCalls(), debugger.getNumOfDispatchCalls(),
					ReactorDebugger.getChannelId(reactorChannel));
		}

		try
		{
			if (errorInfo == null)
				return ReactorReturnCodes.FAILURE;
			else if (reactorChannel == null)
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.closeChannel",
						"reactorChannel cannot be null");
			else if (_reactorActive == false)
				return populateErrorInfo(errorInfo, ReactorReturnCodes.SHUTDOWN, "Reactor.closeChannel",
						"Reactor is shutdown, closeChannel ignored");
			else if (reactorChannel.state() == State.CLOSED)
				return ReactorReturnCodes.SUCCESS;

			// set the ReactorChannel's state to CLOSED.
			// and remove it from the queue.
			reactorChannel.state(State.CLOSED);
			_reactorChannelQueue.remove(reactorChannel, ReactorChannel.REACTOR_CHANNEL_LINK);

			if (reactorChannel.warmStandByHandlerImpl != null)
			{
				reactorChannel.warmStandByHandlerImpl.channelList().remove(reactorChannel);
			}

			// send CHANNEL_CLOSED WorkerEvent to Worker.
			if (!sendWorkerEvent(WorkerEventTypes.CHANNEL_CLOSE, reactorChannel))
			{
				// sendWorkerEvent() failed, send channel down
				reactorChannel.state(State.DOWN);
				sendAndHandleChannelEventCallback("Reactor.closeChannel", ReactorChannelEventTypes.CHANNEL_DOWN,
						reactorChannel, errorInfo);
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.closeChannel",
						"sendWorkerEvent() failed");
			}

			reactorChannel.tunnelStreamManager().close();

			if (reactorChannel.watchlist() != null)
			{
				reactorChannel.watchlist().close();
				reactorChannel.watchlist(null);	
			}
			
		} finally
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

		System.out.println("TunnelStreamStateInfo:\n" + "                  Stream State: "
				+ _tunnelStreamStateInfo.streamState().toString() + "\n" + "   Outbound Untransmitted Msgs: "
				+ _tunnelStreamStateInfo.outboundMsgsQueued() + "\n" + "         Outbound Unacked Msgs: "
				+ _tunnelStreamStateInfo.outboundMsgsWaitingForAck() + "\n" + "   Outbound/Inbound Bytes Open: "
				+ _tunnelStreamStateInfo.outboundBytesOpen() + "/" + _tunnelStreamStateInfo.inboundBytesOpen() + "\n");
	}

	private int sendTunnelStreamLogin(TunnelStream tunnelStream, ReactorErrorInfo errorInfo)
	{
		LoginRequest loginRequest = tunnelStream.authLoginRequest();

		TransportBuffer msgBuf = tunnelStream.getBuffer(getMaxFragmentSize(tunnelStream), errorInfo);
		if (msgBuf == null)
		{
			populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "Reactor.sendTunnelStreamLogin",
					"Failed to obtain a TransportBuffer, reason=" + errorInfo.error().text());
			return ReactorReturnCodes.FAILURE;
		}

		_eIter.clear();
		_eIter.setBufferAndRWFVersion(msgBuf, tunnelStream.reactorChannel().majorVersion(),
				tunnelStream.reactorChannel().minorVersion());

		int retval = loginRequest.encode(_eIter);
		if (retval != CodecReturnCodes.SUCCESS)
		{
			populateErrorInfo(errorInfo, retval, "Reactor.sendTunnelStreamLogin",
					"Encoding of login request failed: <" + TransportReturnCodes.toString(retval) + ">");
			return ReactorReturnCodes.FAILURE;
		}

		_tunnelStreamSubmitOptions.clear();
		_tunnelStreamSubmitOptions.containerType(DataTypes.MSG);
		tunnelStream.startRequestTimer();
		retval = tunnelStream.submit(msgBuf, _tunnelStreamSubmitOptions, errorInfo);
		if (retval != CodecReturnCodes.SUCCESS)
		{
			populateErrorInfo(errorInfo, retval, "Reactor.sendTunnelStreamLogin",
					"Submit of login request failed: <" + TransportReturnCodes.toString(retval) + ">");
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
	private int handleTunnelStreamMsg(ReactorChannel reactorChannel, TunnelStream tunnelStream, TransportBuffer msgBuf,
			Msg msg, ReactorErrorInfo errorInfo)
	{
		int retval;

		// dispatch anything for tunnel stream
		while ((retval = reactorChannel.tunnelStreamManager().dispatch(errorInfo.error())) > ReactorReturnCodes.SUCCESS)
			;
		if (retval < ReactorReturnCodes.SUCCESS)
		{
			// send channel down event if channel error occurred
			if (retval == ReactorReturnCodes.CHANNEL_ERROR)
			{
				if (reactorChannel.state() != State.DOWN && reactorChannel.state() != State.DOWN_RECONNECTING)
				{
					if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client
																											// channel
					{
						reactorChannel.state(State.DOWN_RECONNECTING);
						sendAndHandleChannelEventCallback("Reactor.performChannelRead",
								ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
					} else // server channel or no more retries
					{
						reactorChannel.state(State.DOWN);
						sendAndHandleChannelEventCallback("Reactor.performChannelRead",
								ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
					}
				}
				// return SUCCESS on dispatch() for CHANNEL_ERROR, application should close
				// ReactorChannel
				retval = ReactorReturnCodes.SUCCESS;
			} else
			{
				return populateErrorInfo(errorInfo, retval, "Reactor.performChannelRead",
						"TunnelStream dispatch failed - " + errorInfo.error().text());
			}
		}

		if (msg.msgClass() == MsgClasses.CLOSE)
		{
			// forward status to tunnel stream event callback
			_tmpState.clear();
			_tmpState.streamState(StreamStates.CLOSED);
			_tmpState.dataState(DataStates.SUSPECT);

			sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel, tunnelStream,
					msgBuf, msg, _tmpState, errorInfo);
		}

		// set flag for TunnelStream read
		// status and close messages are never read by TunnelStream
		else if (msg.msgClass() != MsgClasses.STATUS)
		{
			// read anything for TunnelStream
			if ((retval = reactorChannel.tunnelStreamManager().readMsg(tunnelStream, _msg,
					errorInfo.error())) < ReactorReturnCodes.SUCCESS)
			{
				// send channel down event if channel error occurred
				if (retval == ReactorReturnCodes.CHANNEL_ERROR)
				{
					if (reactorChannel.state() != State.DOWN && reactorChannel.state() != State.DOWN_RECONNECTING)
					{
						if (reactorChannel.server() == null && !reactorChannel.recoveryAttemptLimitReached()) // client
																												// channel
						{
							reactorChannel.state(State.DOWN_RECONNECTING);
							sendAndHandleChannelEventCallback("Reactor.performChannelRead",
									ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, reactorChannel, errorInfo);
						} else // server channel or no more retries
						{
							reactorChannel.state(State.DOWN);
							sendAndHandleChannelEventCallback("Reactor.performChannelRead",
									ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel, errorInfo);
						}
					}
					// return SUCCESS on dispatch() for CHANNEL_ERROR, application should close
					// ReactorChannel
					retval = ReactorReturnCodes.SUCCESS;
				} else
				{
					return populateErrorInfo(errorInfo, retval, "Reactor.performChannelRead",
							"TunnelStream readMsg failed - " + errorInfo.error().text());
				}
			}

			// handle refresh message
			if (msg.msgClass() == MsgClasses.REFRESH)
			{
				RefreshMsg refreshMsg = (RefreshMsg) msg;

				if (reactorChannel.watchlist() != null)
				{
					WlRequest wlRequest;

					_tempWlInteger.value(refreshMsg.streamId());
					if ((wlRequest = reactorChannel.watchlist().streamIdtoWlRequestTable().get(_tempWlInteger)) == null)
					{
						return populateErrorInfo(errorInfo, retval, "Reactor.performChannelRead",
								"Internal Error: TunnelStream watchlist request entry not found.");
					}

					if (wlRequest.stream() == null)
					{
						return populateErrorInfo(errorInfo, retval, "Reactor.performChannelRead",
								"Internal Error: TunnelStream watchlist stream entry not found.");
					}

					tunnelStream.channelStreamId(wlRequest.stream().streamId());
				} else
					tunnelStream.channelStreamId(refreshMsg.streamId());

				if (refreshMsg.containerType() == DataTypes.FILTER_LIST && refreshMsg.msgKey().checkHasFilter())
				{
					int ret;
					if ((ret = tunnelStream.classOfService().decode(_reactorChannel, refreshMsg.encodedDataBody(),
							errorInfo)) != ReactorReturnCodes.SUCCESS)
					{
						_tmpState.clear();
						_tmpState.streamState(StreamStates.CLOSED);
						_tmpState.dataState(DataStates.SUSPECT);
						_tmpState.text().data("Class of service decode failed with return code: " + ret + " <"
								+ errorInfo.error().text() + ">");

						sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel,
								tunnelStream, msgBuf, msg, _tmpState, errorInfo);
					}

					/* If recvWindowSize was -1, set it to reflect actual default value. */
					/*
					 * If recvWindowSize was less than received maxFragmentSize, set it to received
					 * maxFragmentSize.
					 */
					if (tunnelStream.classOfService().flowControl().recvWindowSize() == -1)
						tunnelStream.classOfService().flowControl().recvWindowSize(TunnelStream.DEFAULT_RECV_WINDOW);
					if (tunnelStream.classOfService().flowControl().recvWindowSize() < tunnelStream.classOfService()
							.common().maxFragmentSize())
						tunnelStream.classOfService().flowControl()
								.recvWindowSize(tunnelStream.classOfService().common().maxFragmentSize());
					/* If sendWindowSize was -1, set it to reflect actual default value. */
					/*
					 * If sendWindowSize was less than received maxFragmentSize, set it to received
					 * maxFragmentSize.
					 */
					if (tunnelStream.classOfService().flowControl().sendWindowSize() == -1)
						tunnelStream.classOfService().flowControl().sendWindowSize(TunnelStream.DEFAULT_RECV_WINDOW);
					if (tunnelStream.classOfService().flowControl().sendWindowSize() < tunnelStream.classOfService()
							.common().maxFragmentSize())
						tunnelStream.classOfService().flowControl()
								.sendWindowSize(tunnelStream.classOfService().common().maxFragmentSize());

					if (!tunnelStream.isProvider())
					{
						// do the _bufferPool here
						tunnelStream.setupBufferPool();
					}
				} else
				{
					_tmpState.clear();
					_tmpState.streamState(StreamStates.CLOSED);
					_tmpState.dataState(DataStates.SUSPECT);
					_tmpState.text()
							.data("TunnelStream refresh must contain FILTER_LIST and have filter in message key");

					sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel,
							tunnelStream, msgBuf, msg, _tmpState, errorInfo);
				}

				/*
				 * Check if received class of service stream version is less than or equal to
				 * current class of service stream version. If not, callback with closed state.
				 */
				CosCommon commonProperties = tunnelStream.classOfService().common();
				if (commonProperties.streamVersion() <= CosCommon.CURRENT_STREAM_VERSION)
				{
					if (tunnelStream.classOfService().authentication()
							.type() != ClassesOfService.AuthenticationTypes.OMM_LOGIN)
					{
						// no substream login, send refresh message as event callback
						refreshMsg.state().copy(tunnelStream.state());
						sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel,
								tunnelStream, msgBuf, refreshMsg, refreshMsg.state(), errorInfo);
					} else // substream login enabled
					{
						// send TunnelStream login here
						if ((sendTunnelStreamLogin(tunnelStream, errorInfo)) < ReactorReturnCodes.SUCCESS)
						{
							_tmpState.clear();
							_tmpState.streamState(StreamStates.CLOSED);
							_tmpState.dataState(DataStates.SUSPECT);
							_tmpState.text().data("sendTunnelStreamLogin() failed <" + errorInfo.error().text() + ">");

							sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel,
									tunnelStream, msgBuf, msg, _tmpState, errorInfo);
						}
					}
				} else // class of service stream version doesn't match, callback with closed state
				{
					_tmpState.clear();
					_tmpState.streamState(StreamStates.CLOSED);
					_tmpState.dataState(DataStates.SUSPECT);
					_tmpState.text()
							.data("Unsupported class of service stream version: " + commonProperties.streamVersion());

					sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel,
							tunnelStream, msgBuf, msg, _tmpState, errorInfo);
				}
			}

			if (tunnelStream.traceFlags() > 0)
				printTunnelStreamStateInfo(tunnelStream);
		} else // status
		{
			// forward status to tunnel stream event callback
			if (!tunnelStream.handleRequestRetry())
			{
				StatusMsg statusMsg = (StatusMsg) msg;

				if (statusMsg.checkHasState())
				{
					statusMsg.state().copy(tunnelStream.state());
				}
				sendAndHandleTunnelStreamStatusEventCallback("Reactor.performChannelRead", reactorChannel, tunnelStream,
						msgBuf, statusMsg, statusMsg.state(), errorInfo);

				// always close tunnel stream handler for status message close
				if (statusMsg.state().streamState() == StreamStates.CLOSED
						|| statusMsg.state().streamState() == StreamStates.CLOSED_RECOVER)
					tunnelStream.close(_finalStatusEvent, errorInfo.error());
			}
		}

		if ((retval = reactorChannel.checkTunnelManagerEvents(errorInfo)) != ReactorReturnCodes.SUCCESS)
			return retval;

		return ReactorReturnCodes.SUCCESS;
	}

	/* Request that the Worker start flushing this channel. */
	private int sendFlushRequest(ReactorChannel reactorChannel, String location, ReactorErrorInfo errorInfo)
	{
		if (reactorChannel.flushRequested())
			reactorChannel.flushAgain(true); /*
												 * Flush already in progress; wait till FLUSH_DONE is received, then
												 * request again.
												 */
		else
		{
			if (!sendWorkerEvent(WorkerEventTypes.FLUSH, reactorChannel))
			{
				// sendWorkerEvent() failed, send channel down
				reactorChannel.state(State.DOWN);
				sendAndHandleChannelEventCallback(location, ReactorChannelEventTypes.CHANNEL_DOWN, reactorChannel,
						errorInfo);
				return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, location,
						"sendWorkerEvent() failed while requesting flush");
			}

			reactorChannel.flushAgain(false);
			reactorChannel.flushRequested(true);
		}

		return ReactorReturnCodes.SUCCESS;
	}

	boolean isReactorChannelReady(ReactorChannel reactorChannel)
	{
		return reactorChannel.state() == ReactorChannel.State.UP
				|| reactorChannel.state() == ReactorChannel.State.READY;
	}

	int overflowSafeAggregate(int a, int b)
	{
		long sum = (long) a + (long) b;
		if (sum < Integer.MAX_VALUE)
			return (int) sum;
		else
			return Integer.MAX_VALUE;
	}

	/*
	 * Checks if the warm standby channels are closed Returns TRUE if all channels
	 * are closed except for the current one. FALSE if any channels are active
	 * PREREQUESITE: This channel is a warm standby channel
	 */
	boolean isWarmStandbyChannelClosed(ReactorWarmStandbyHandler wsbHandler, ReactorChannel reactorChannel)
	{
		ReactorChannel tmp = null;

		for (int i = 0; i < wsbHandler.channelList().size(); i++)
		{
			tmp = wsbHandler.channelList().get(i);

			if (reactorChannel != null && tmp == reactorChannel)
			{
				/*
				 * This is called from the channel down handler, so we need to ignore the
				 * current channel
				 */
				if (reactorChannel.channel() != null && reactorChannel.channel().state() == ChannelState.ACTIVE)
				{
					continue;
				}
			}

			/*
			 * Do not need to check the state of the reactorChannel, because the underlying
			 * RSSL channel can only be present and active if the channel has connected
			 */
			if (tmp.channel() != null && tmp.channel().state() == ChannelState.ACTIVE)
			{
				return false;
			}
		}
		return true;
	}

	/* Checks if this channel is currently a warm standby channel */
	boolean reactorHandlesWarmStandby(ReactorChannel reactorChannel)
	{
		if (reactorChannel.warmStandByHandlerImpl != null)
		{
			if ((reactorChannel.warmStandByHandlerImpl.warmStandbyHandlerState()
					& ReactorWarmStandbyHandlerState.MOVE_TO_CHANNEL_LIST) == 0)
			{
				return true;
			}
		}
		return false;
	}

	/*
	 * Checks if the current reactorChannel is an active channel for sending a
	 * callback
	 */
	boolean warmStandbySendCallback(Msg msg, WlRequest wlRequest, ReactorChannel reactorChannel)
	{
		if (wlRequest != null && reactorHandlesWarmStandby(reactorChannel))
		{

			/* Send the callback if all channels are down */
			if ((wlRequest.statusFlags() & WlStreamStatusFlags.SEND_STATUS) == 0
					&& isWarmStandbyChannelClosed(reactorChannel.warmStandByHandlerImpl, null))
			{
				/* Clear the streamInfo now that we're about to send the message */
				wlRequest.statusFlags(WlStreamStatusFlags.NONE);
				return true;
			}

			if (reactorChannel.warmStandByHandlerImpl.currentWarmStandbyGroupImpl()
					.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
			{
				return reactorChannel.isActiveServer;
			} else
			{
				ReactorWarmStandbyGroupImpl wsbGroup = reactorChannel.warmStandByHandlerImpl
						.currentWarmStandbyGroupImpl();
				
				ReactorWSBService service;
				
				if (msg.msgKey() != null && msg.msgKey().checkHasServiceId())
				{
					_tempWlInteger.value(msg.msgKey().serviceId());
					

					service = wsbGroup._perServiceById.get(_tempWlInteger);

					if (service != null)
					{
						return (service.activeChannel == reactorChannel);
					} else
					{
						return false;
					}
				} else
				{
					if(wlRequest.hasServiceId())
					{
						_tempWlInteger.value((int)wlRequest.serviceId());
						service = wsbGroup._perServiceById.get(_tempWlInteger);
	
						if (service != null)
						{
							return (service.activeChannel == reactorChannel);
						} else
						{
							return false;
						}

					}
					else
					{
						WlService wlService = reactorChannel.watchlist()
								.directoryHandler()._serviceCache._servicesByNameTable.get(wlRequest.streamInfo().serviceName());
						if (wlService != null)
						{
							service = reactorChannel.warmStandByHandlerImpl
									.currentWarmStandbyGroupImpl()._perServiceById.get(wlService.tableKey());
	
							if (service != null)
							{
								return (service.activeChannel == reactorChannel);
							} else
							{
								return false;
							}
						} else
						{
							return false;
						}
					}
				}
			}
		}
		/* Default to true for non-warm standby cases */
		return true;

	}

	/*
	 * Fans out any cached closed/closed_recover/redirect status messages received
	 * by a standby channel that were not sent to the user because the previous
	 * active was able to service the items. The items will also be closed on every
	 * other connection.
	 */
	private int reactorWSBFanoutStatusMsg(ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
	{
		while (reactorChannel._watchlistRecoveryMsgList.size() != 0)
		{
			ReactorWSRecoveryMsgInfo msgInfo = reactorChannel._watchlistRecoveryMsgList.poll();

			StatusMsg statusMsg = (StatusMsg) CodecFactory.createMsg();
			statusMsg.msgClass(MsgClasses.STATUS);

			statusMsg.containerType(msgInfo._containerType);
			statusMsg.domainType(msgInfo._domainType);
			statusMsg.streamId(msgInfo._streamId);
			statusMsg.flags(msgInfo._flags);
			msgInfo._msgKey.copy(statusMsg.msgKey());
			msgInfo._msgState.copy(statusMsg.state());

			ReactorMsgEvent reactorMsgEvent = ReactorFactory.createReactorMsgEvent();
			reactorMsgEvent.msg(statusMsg);
			ReactorChannel callbackChannel = reactorChannel;
			/*
			 * If this is a warm standby channel, set the callback channel to the main
			 * reactorChannel in the wsbHandler.
			 */
			if (reactorHandlesWarmStandby(reactorChannel))
			{
				callbackChannel = reactorChannel.warmStandByHandlerImpl.mainReactorChannelImpl();
				callbackChannel.selectableChannelFromChannel(reactorChannel.channel());
				callbackChannel.userSpecObj(reactorChannel.userSpecObj());
			}

			reactorMsgEvent.reactorChannel(callbackChannel);
			reactorMsgEvent.streamInfo().serviceName(msgInfo._serviceName.toString());
			reactorMsgEvent.streamInfo().userSpecObject(msgInfo._userSpecObject);

			reactorChannel.role().defaultMsgCallback().defaultMsgCallback(reactorMsgEvent);
			reactorMsgEvent.returnToPool();

			/* Fan out close messages to every other channel in the WSB list */
			CloseMsg closeMsg = (CloseMsg) CodecFactory.createMsg();
			closeMsg.msgClass(MsgClasses.CLOSE);

			closeMsg.streamId(msgInfo._streamId);

			for (int i = 0; i < reactorChannel.warmStandByHandlerImpl.channelList().size(); ++i)
			{
				ReactorChannel channel = reactorChannel.warmStandByHandlerImpl.channelList().get(i);
				reactorSubmitOptions.clear();

				channel.watchlist().submitMsg(closeMsg, reactorSubmitOptions, errorInfo);
			}
		}

		return ReactorReturnCodes.SUCCESS;
	}

	/*
	 * Fan out a codec message to all warm standby channels. If all channels have
	 * not connected yet, cache the requests to be sent out when the connection is
	 * established.
	 */
	int submitWSBMsg(ReactorChannel reactorChannel, Msg msg, ReactorSubmitOptions submitOptions,
			ReactorErrorInfo errorInfo)
	{
		boolean stopSubmitLoop = false;
		boolean addMsgToQueue = false;
		int ret = ReactorReturnCodes.SUCCESS;

		ReactorWarmStandbyHandler wsbHandler = reactorChannel.warmStandByHandlerImpl;
		ReactorWarmStandbyGroupImpl wsbGroup = wsbHandler.currentWarmStandbyGroupImpl();

		ReactorWLSubmitMsgOptions submitOpts = null;
		ReactorChannel submitChannel = null;
		ReactorWSBService wsbService = null;


		if (wsbHandler.channelList().size() == 0)
		{
			return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "reactor.submitWSBMsg",
					"Warm Standby channel is not active.");
		}

		/*
		 * Check to see if all connections are up(and therefore we've requested
		 * everything so far). If this is false, we will queue the messages to be sent
		 * when the other standby channels become active
		 */
		if (wsbGroup.sendQueueReqForAll == false)
		{
			if (msg.domainType() != DomainTypes.LOGIN || msg.msgClass() != MsgClasses.REQUEST)
			{
				if (wsbHandler.freeSubmitMsgQueue().size() > 0)
					submitOpts = wsbHandler.freeSubmitMsgQueue().remove(0);
				else
					submitOpts = new ReactorWLSubmitMsgOptions();

				if (msg.copy(submitOpts.msg, CopyMsgFlags.ALL_FLAGS) == CodecReturnCodes.FAILURE)
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "reactor.submitWSBMsg",
							"Cannot copy submit message options");
				}

				submitOpts.submitOptions.serviceName(submitOptions.serviceName());
				submitOpts.submitOptions.requestMsgOptions()
						.userSpecObj(submitOptions.requestMsgOptions().userSpecObj());

				addMsgToQueue = true;
				submitOpts.submitTime = System.nanoTime();
				// If the current wsb connection has not gotten to the initial directory response, queue up the message.
				// It will be sent after the primary directory response is received.
				if((wsbHandler.warmStandbyHandlerState() & ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_DIRECTORY_RESPONSE) == 0)
				{
					wsbHandler.submitMsgQueue().add(submitOpts);
					return ReactorReturnCodes.SUCCESS;
				}
			}
				
		}

		/* Fan out the message to the WSB channels */
		for (int i = 0; i < wsbHandler.channelList().size(); i++)
		{
			submitChannel = wsbHandler.channelList().get(i);
			
			if(submitChannel.watchlist().directoryHandler()._serviceCache.initDirectory == false)
				continue;

			switch (msg.msgClass())
			{
				case MsgClasses.REQUEST:
				{
					/* Private streams go on the active only */
					if ((msg.flags() & RequestMsgFlags.PRIVATE_STREAM) != 0)
					{
						if (wsbGroup.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
						{
							if (submitChannel.isActiveServer == false)
								continue;
						} else
						{
							wsbService = null;
							if (submitOptions.serviceName() != null && !submitOptions.serviceName().isEmpty())
							{
	
								WlService service = submitChannel.watchlist()
										.directoryHandler()._serviceCache._servicesByNameTable
												.get(submitOptions.serviceName());
								if (service != null)
								{
									wsbService = submitChannel.warmStandByHandlerImpl
											.currentWarmStandbyGroupImpl()._perServiceById.get(service._tableKey);
									if (wsbService != null)
									{
										if (wsbService.activeChannel != submitChannel)
											continue;
									} else
									{
										continue;
									}
								} else
								{
									continue;
								}
							} else
							{
								if ((msg.msgKey().flags() & MsgKeyFlags.HAS_SERVICE_ID) != 0)
								{
									submitChannel.watchlist()._tempWlInteger.value(msg.msgKey().serviceId());
									wsbService = submitChannel.warmStandByHandlerImpl
											.currentWarmStandbyGroupImpl()._perServiceById
													.get(submitChannel.watchlist()._tempWlInteger);
	
									if (wsbService != null)
									{
										if (wsbService.activeChannel != submitChannel)
											continue;
									}
								}
							}
						}
	
						ret = submitChannel.watchlist().submitMsg(msg, submitOptions, errorInfo);
						stopSubmitLoop = true;
					} else if((msg.flags() & RequestMsgFlags.PAUSE) != 0)
					{
						return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "reactor.submitWSBMsg",
								"Pause not supported with Warm Standby.");
					}
					else
					{
						ret = submitChannel.watchlist().submitMsg(msg, submitOptions, errorInfo);
					}
					break;
				}
				case MsgClasses.GENERIC:
				{
					if (isReactorChannelActive(submitChannel) == false)
					{
						continue;
					}
					
					if (wsbGroup.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
					{
						if (submitChannel.isActiveServer == false)
							continue;
					} else
					{
						wsbService = null;
						if (submitOptions.serviceName() != null && !submitOptions.serviceName().isEmpty())
						{

							WlService service = submitChannel.watchlist()
									.directoryHandler()._serviceCache._servicesByNameTable
											.get(submitOptions.serviceName());
							if (service != null)
							{
								wsbService = submitChannel.warmStandByHandlerImpl
										.currentWarmStandbyGroupImpl()._perServiceById.get(service._tableKey);
								if (wsbService != null)
								{
									if (wsbService.activeChannel != submitChannel)
										continue;
								} else
								{
									continue;
								}
							} else
							{
								continue;
							}
						} else
						{
							if ((msg.msgKey().flags() & MsgKeyFlags.HAS_SERVICE_ID) != 0)
							{
								submitChannel.watchlist()._tempWlInteger.value(msg.msgKey().serviceId());
								wsbService = submitChannel.warmStandByHandlerImpl
										.currentWarmStandbyGroupImpl()._perServiceById
												.get(submitChannel.watchlist()._tempWlInteger);

								if (wsbService != null)
								{
									if (wsbService.activeChannel != submitChannel)
										continue;
								}
							}
						}
					}
	
					ret = submitChannel.watchlist().submitMsg(msg, submitOptions, errorInfo);
	
					break;
				}
				case MsgClasses.POST:
				{
					if (isReactorChannelActive(submitChannel) == false)
					{
						continue;
					}
					/* Check to make sure that current channel provides the service associated with the post */
					wsbService = null;
					if (submitOptions.serviceName() != null && !submitOptions.serviceName().isEmpty())
					{

						WlService service = submitChannel.watchlist()
								.directoryHandler()._serviceCache._servicesByNameTable
										.get(submitOptions.serviceName());
						if (service != null)
						{
							wsbService = submitChannel.warmStandByHandlerImpl
									.currentWarmStandbyGroupImpl()._perServiceById.get(service._tableKey);
							if (wsbService == null)
							{
								continue;
							}
						} else
						{
							continue;
						}
					} else
					{
						if ((msg.msgKey().flags() & MsgKeyFlags.HAS_SERVICE_ID) != 0)
						{
							submitChannel.watchlist()._tempWlInteger.value(msg.msgKey().serviceId());
							wsbService = submitChannel.warmStandByHandlerImpl
									.currentWarmStandbyGroupImpl()._perServiceById
											.get(submitChannel.watchlist()._tempWlInteger);

							if (wsbService == null)
							{
								continue;
							}
						}
					}
					
					ret = submitChannel.watchlist().submitMsg(msg, submitOptions, errorInfo);
	
					break;
				}
				default:
				{
					ret = submitChannel.watchlist().submitMsg(msg, submitOptions, errorInfo);
					break;
				}
			}

			if (ret != ReactorReturnCodes.SUCCESS || stopSubmitLoop)
			{
				break;
			}
		}
		
		this._reactorChannel.lastSubmitOptionsTime = System.nanoTime();

		if (addMsgToQueue)
		{
			if (ret == ReactorReturnCodes.SUCCESS)
			{
				wsbHandler.submitMsgQueue().add(submitOpts);
			} else
			{
				submitOpts.clear();
				wsbHandler.freeSubmitMsgQueue().add(submitOpts);
			}
		}

		return ret;
	}

	/*
	 * Fan out an RDM message to all warm standby channels. If all channels have not
	 * connected yet, cache the requests to be sent out when the connection is
	 * established. This function will convert the RDM message and then send it out.
	 */
	int submitWSBRDMMsg(ReactorChannel reactorChannel, MsgBase msg, ReactorSubmitOptions submitOptions,
			ReactorErrorInfo errorInfo)
	{
		boolean stopSubmitLoop = false;
		boolean addMsgToQueue = false;
		int ret = ReactorReturnCodes.SUCCESS;

		ReactorWarmStandbyHandler wsbHandler = reactorChannel.warmStandByHandlerImpl;
		ReactorWarmStandbyGroupImpl wsbGroup = wsbHandler.currentWarmStandbyGroupImpl();

		ReactorWLSubmitMsgOptions submitOpts = null;
		ReactorChannel submitChannel = null;

		if (wsbHandler.channelList().size() == 0)
		{
			return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "reactor.submitWSBMsg",
					"Warm Standby channel is not active.");
		}

		/*
		 * Convert the msgBase to a codec message. This will be done in
		 * watchlist.submit(msgBase...) anyway, so we'll do it here to simplify the
		 * caching
		 */
		reactorChannel.watchlist().convertRDMToCodecMsg(msg, _msg);

		/*
		 * Check to see if all connections are up(and therefore we've requested
		 * everything so far). If this is false, we will queue the messages to be sent
		 * when the other standby channels become active
		 */
		if (wsbGroup.sendQueueReqForAll == false)
		{
			if (_msg.domainType() != DomainTypes.LOGIN || _msg.msgClass() != MsgClasses.REQUEST)
			{
				if (wsbHandler.freeSubmitMsgQueue().size() > 0)
					submitOpts = wsbHandler.freeSubmitMsgQueue().remove(0);
				else
					submitOpts = new ReactorWLSubmitMsgOptions();

				if (_msg.copy(submitOpts.msg, CopyMsgFlags.ALL_FLAGS) == CodecReturnCodes.FAILURE)
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "reactor.submitWSBMsg",
							"Cannot copy submit message options");
				}

				submitOpts.submitOptions.serviceName(submitOptions.serviceName());
				submitOpts.submitOptions.requestMsgOptions()
						.userSpecObj(submitOptions.requestMsgOptions().userSpecObj());

				addMsgToQueue = true;
				submitOpts.submitTime = System.nanoTime();
				// If the current wsb connection has not gotten to the initial directory response, queue up the message.
				// It will be sent after the primary directory response is received.
				if((wsbHandler.warmStandbyHandlerState() & ReactorWarmStandbyHandlerState.RECEIVED_PRIMARY_DIRECTORY_RESPONSE) == 0)
				{
					wsbHandler.submitMsgQueue().add(submitOpts);
					return ReactorReturnCodes.SUCCESS;
				}
			}
		}

		for (int i = 0; i < wsbHandler.channelList().size(); i++)
		{
			submitChannel = wsbHandler.channelList().get(i);
			
			if(submitChannel.watchlist().directoryHandler()._serviceCache.initDirectory == false)
				continue;

			switch (_msg.msgClass())
			{
			case MsgClasses.REQUEST:
			{
				/* Private streams go on the active only */
				if ((_msg.flags() & RequestMsgFlags.PRIVATE_STREAM) != 0)
				{
					if (wsbGroup.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
					{
						if (submitChannel.isActiveServer == false)
							continue;
					} else
					{
						ReactorWSBService wsbService = null;
						if (submitOptions.serviceName() != null && !submitOptions.serviceName().isEmpty())
						{

							WlService service = submitChannel.watchlist()
									.directoryHandler()._serviceCache._servicesByNameTable
											.get(submitOptions.serviceName());
							if (service != null)
							{
								wsbService = submitChannel.warmStandByHandlerImpl
										.currentWarmStandbyGroupImpl()._perServiceById.get(service._tableKey);
								if (wsbService != null)
								{
									if (wsbService.activeChannel != submitChannel)
										continue;
								} else
								{
									continue;
								}
							} else
							{
								continue;
							}
						} else
						{
							if ((_msg.msgKey().flags() & MsgKeyFlags.HAS_SERVICE_ID) != 0)
							{
								submitChannel.watchlist()._tempWlInteger.value(_msg.msgKey().serviceId());
								wsbService = submitChannel.warmStandByHandlerImpl
										.currentWarmStandbyGroupImpl()._perServiceById
												.get(submitChannel.watchlist()._tempWlInteger);

								if (wsbService != null)
								{
									if (wsbService.activeChannel != submitChannel)
										continue;
								}
							}
						}
					}

					ret = submitChannel.watchlist().submitMsg(_msg, submitOptions, errorInfo);
					stopSubmitLoop = true;
				}  else if((_msg.flags() & RequestMsgFlags.PAUSE) != 0)
				{
					return populateErrorInfo(errorInfo, ReactorReturnCodes.FAILURE, "reactor.submitWSBMsg",
							"Pause not supported with Warm Standby.");
				}
				else
				{
					ret = submitChannel.watchlist().submitMsg(_msg, submitOptions, errorInfo);
				}
				break;
			}
			case MsgClasses.GENERIC:
			case MsgClasses.POST:
			{
				if (isReactorChannelActive(submitChannel) == false)
				{
					continue;
				}
				
				if (wsbGroup.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
				{
					if (submitChannel.isActiveServer == false)
						continue;
				} else
				{
					ReactorWSBService wsbService = null;
					if (submitOptions.serviceName() != null && !submitOptions.serviceName().isEmpty())
					{

						WlService service = submitChannel.watchlist()
								.directoryHandler()._serviceCache._servicesByNameTable
										.get(submitOptions.serviceName());
						if (service != null)
						{
							wsbService = submitChannel.warmStandByHandlerImpl
									.currentWarmStandbyGroupImpl()._perServiceById.get(service._tableKey);
							if (wsbService != null)
							{
								if (wsbService.activeChannel != submitChannel)
									continue;
							} else
							{
								continue;
							}
						} else
						{
							continue;
						}
					} else
					{
						if ((_msg.msgKey().flags() & MsgKeyFlags.HAS_SERVICE_ID) != 0)
						{
							submitChannel.watchlist()._tempWlInteger.value(_msg.msgKey().serviceId());
							wsbService = submitChannel.warmStandByHandlerImpl
									.currentWarmStandbyGroupImpl()._perServiceById
											.get(submitChannel.watchlist()._tempWlInteger);

							if (wsbService != null)
							{
								if (wsbService.activeChannel != submitChannel)
									continue;
							}
						}
					}
				}

				ret = submitChannel.watchlist().submitMsg(_msg, submitOptions, errorInfo);

				break;
			}
			default:
			{
				ret = submitChannel.watchlist().submitMsg(_msg, submitOptions, errorInfo);
				break;
			}

			}

			if (ret != ReactorReturnCodes.SUCCESS || stopSubmitLoop)
			{
				break;
			}
		}
		
		this._reactorChannel.lastSubmitOptionsTime = System.nanoTime();

		if (addMsgToQueue)
		{
			if (ret == ReactorReturnCodes.SUCCESS)
			{
				wsbHandler.submitMsgQueue().add(submitOpts);
			} else
			{
				submitOpts.clear();
				wsbHandler.freeSubmitMsgQueue().add(submitOpts);
			}
		}

		return ReactorReturnCodes.SUCCESS;
	}

	void cleanUpWSBRequestQueue(ReactorWarmStandbyHandler wsbHandler, ReactorWarmStandbyGroupImpl wsbGroup)
	{
		if (wsbGroup.sendReqQueueCount <= wsbGroup.standbyServerList().size())
		{
			if ((wsbGroup.sendReqQueueCount) == wsbGroup.standbyServerList().size())
			{
				wsbGroup.sendQueueReqForAll = true;
				wsbGroup.sendReqQueueCount = wsbGroup.standbyServerList().size() + 1;
			} else
			{
				wsbGroup.sendReqQueueCount++;
			}
		}

		if (wsbGroup.sendQueueReqForAll == true)
		{
			ReactorWLSubmitMsgOptions submitOpts = null;
			while ((submitOpts = wsbHandler.submitMsgQueue().remove(0)) != null)
			{
				submitOpts.clear();
				wsbHandler.freeSubmitMsgQueue().add(submitOpts);
			}
		}

	}

	/* Submit any messages in the wsb message queue for the wsbHandler. */
	int submitWSBRequestQueue(ReactorWarmStandbyHandler wsbHandler, ReactorWarmStandbyGroupImpl wsbGroup,
			ReactorChannel reactorChannel, ReactorErrorInfo errorInfo)
	{
		int retVal = ReactorReturnCodes.SUCCESS;
		if (wsbGroup.sendReqQueueCount <= wsbGroup.standbyServerList().size())
		{
			if ((wsbGroup.sendReqQueueCount) == wsbGroup.standbyServerList().size())
			{
				wsbGroup.sendQueueReqForAll = true;
				wsbGroup.sendReqQueueCount = wsbGroup.standbyServerList().size() + 1;
			} else
			{
				wsbGroup.sendReqQueueCount++;
			}
		}

		for (int i = 0; i < wsbHandler.submitMsgQueue().size(); ++i)
		{
			if (wsbHandler.submitMsgQueue().get(i).submitTime > reactorChannel.lastSubmitOptionsTime)
			{
				RequestMsg msg = (RequestMsg)wsbHandler.submitMsgQueue().get(i).msg;
				
				if(msg.checkPrivateStream())
				{
					if(wsbGroup.warmStandbyMode() == ReactorWarmStandbyMode.LOGIN_BASED)
					{
						if(!reactorChannel.isActiveServer)
							continue;
					}
					else
					{
						ReactorWSBService wsbService;
						if(msg.msgKey().checkHasServiceId())
						{
							_tempWlInteger.value(msg.msgKey().serviceId());
							wsbService = wsbGroup._perServiceById.get(_tempWlInteger);
							
							if(wsbService != null)
							{
								if(reactorChannel != wsbService.activeChannel)
									continue;
							}
						}
						else
						{
							if(wsbHandler.submitMsgQueue().get(i).submitOptions._serviceName != null)
							{
								WlService wlService = reactorChannel.watchlist().directoryHandler()._serviceCache._servicesByNameTable.get(wsbHandler.submitMsgQueue().get(i).submitOptions._serviceName);
								if(wlService != null)
								{
									wsbService = wsbGroup._perServiceById.get(wlService._tableKey);
									
									if(wsbService != null)
									{
										if(reactorChannel != wsbService.activeChannel)
											continue;
									}
								}
							}
						}		
					}
				}
				
				if (reactorChannel.watchlist().submitMsg(wsbHandler.submitMsgQueue().get(i).msg,
						wsbHandler.submitMsgQueue().get(i).submitOptions, errorInfo) != ReactorReturnCodes.SUCCESS)
				{
					retVal = ReactorReturnCodes.FAILURE;
				}
			}
		}

		if (wsbGroup.sendQueueReqForAll == true)
		{
			ReactorWLSubmitMsgOptions submitOpts = null;
			while (wsbHandler.submitMsgQueue().size() != 0)
			{
				submitOpts = wsbHandler.submitMsgQueue().remove(0);
				submitOpts.clear();
				wsbHandler.freeSubmitMsgQueue().add(submitOpts);
			}
		}
		
		reactorChannel.lastSubmitOptionsTime = System.nanoTime();

		return retVal;
	}

	/*
	 * Queue up all the current requests on the main WSB channel for fanout to all
	 * standbys when connecting to a new WSB group
	 */
	int queueRequestsForWSBGroupRecovery(ReactorWarmStandbyHandler wsbHandler, ReactorErrorInfo errorInfo)
	{
		if (wsbHandler.startingReactorChannel() != null)
		{
			ReactorChannel reactorChannel = wsbHandler.startingReactorChannel();

			Watchlist watchlist = reactorChannel.watchlist();

			if (!wsbHandler.queuedRecoveryMessage())
			{
				Iterator<Map.Entry<WlInteger, WlRequest>> iter = watchlist._streamIdtoWlRequestTable.entrySet()
						.iterator();

				while (iter.hasNext())
				{
					Map.Entry<WlInteger, WlRequest> request = iter.next();

					ReactorWLSubmitMsgOptions submitOpts;
					if (wsbHandler.freeSubmitMsgQueue().size() != 0)
					{
						submitOpts = wsbHandler.freeSubmitMsgQueue().remove(0);
					} else
					{
						submitOpts = new ReactorWLSubmitMsgOptions();
					}
					
					RequestMsg tmpMsg = request.getValue().requestMsg();
					
					if(tmpMsg.domainType() == DomainTypes.LOGIN || tmpMsg.domainType() == DomainTypes.SOURCE || tmpMsg.domainType() == DomainTypes.DICTIONARY)
					{
						continue;
					}
					
					tmpMsg.copy(submitOpts.msg, CopyMsgFlags.ALL_FLAGS);

					submitOpts.submitOptions.serviceName(request.getValue().streamInfo()._serviceName);
					submitOpts.submitOptions.requestMsgOptions()
							.userSpecObj(request.getValue().streamInfo().userSpecObject());submitOpts.submitTime = System.nanoTime();

					wsbHandler.submitMsgQueue().add(submitOpts);
				}

				wsbHandler.queuedRecoveryMessage(true);
			}
		}

		return ReactorReturnCodes.SUCCESS;
	}

	/*
	 * Checks the provided service to see if it's supposed to be provided by this
	 * reactorChannel
	 */
	boolean wsbServiceInStartupList(ReactorWarmStandbyGroupImpl wsbGroup, WlService service,
			ReactorChannel reactorChannel)
	{
		ReactorWSBService startupService = wsbGroup._startupServiceNameList
				.get(service._rdmService.info().serviceName());

		if (startupService != null)
		{
			if (startupService.standbyListIndex != reactorChannel.standByServerListIndex)
			{
				return false;
			}

			wsbGroup._startupServiceNameList.remove(service._rdmService.info().serviceName(), startupService);
			startupService.returnToPool();
		}

		return true;
	}

	/* returns true if the underlying ETA channel is active */
	boolean isReactorChannelActive(ReactorChannel reactorChannel)
	{
		if (reactorChannel.channel() != null)
		{
			if (reactorChannel.channel().state() == ChannelState.ACTIVE)
			{
				return true;
			}
		}

		return false;
	}

	/**
	 * Enables the provided debugging level
	 * 
	 * @param level the debugging level to be enabled, see
	 *              {@link ReactorDebuggerLevels}
	 */
	public void enableDebuggingLevel(int level)
	{
		_reactorOptions.debuggerOptions().enableLevel(level);
	}

	/**
	 * Disables the provided debugging level
	 * 
	 * @param level the debugging level to be disabled, see
	 *              {@link ReactorDebuggerLevels}
	 */
	public void disableDebuggingLevel(int level)
	{
		_reactorOptions.debuggerOptions().disableLevel(level);
	}

	/**
	 * Getter for the debugging levels currently set for this Reactor instance
	 * 
	 * @return integer that represents debugging levels currently enabled
	 */
	public int debuggingLevels()
	{
		return _reactorOptions.debuggerOptions().debuggingLevels();
	}

	/**
	 * Determines whether the CONNECTION debugging level is enabled
	 * 
	 * @return true if the CONNECTION debugging level is enabled, false otherwise
	 */
	boolean debugConnectionLevel()
	{
		return _reactorOptions.debuggerOptions().debugConnectionLevel();
	}

	/**
	 * Determines whether the EVENTQUEUE debugging level is enabled
	 * 
	 * @return true if the EVENTQUEUE debugging level is enabled, false otherwise
	 */
	boolean debugEventQueueLevel()
	{
		return _reactorOptions.debuggerOptions().debugEventQueueLevel();
	}

	/**
	 * Determines whether the TUNNELSTREAM debugging level is enabled
	 * 
	 * @return true if the TUNNELSTREAM debugging level is enabled, false otherwise
	 */
	boolean debugTunnelStreamLevel()
	{
		return _reactorOptions.debuggerOptions().debugTunnelStreamLevel();
	}

	/**
	 * Determines whether any debugging is done at all
	 * 
	 * @return true if at least one debugging level is enabled, false otherwise
	 */
	boolean debugEnabled()
	{
		return _reactorOptions.debuggerOptions().debugEnabled();
	}

	/**
	 * Provides access to the messages debugged up to this point in case the
	 * underlying debugger stream is an instance of ByteArrayOutputStream (which is
	 * the case when, e.g., no OutputStream was provided in the
	 * {@link ReactorDebuggerOptions} while creating the Reactor instance
	 * 
	 * @return byte array that contains debugging messages (up to a certain
	 *         capacity) currently written from the beginning of debugging or since
	 *         the last call to this method. In case debugging is not enabled at the
	 *         point when this method is called, one of the following can be
	 *         returned: - null in case the user has provided a debug output stream
	 *         that is not an instance of ByteArrayOutputStream - otherwise, a byte
	 *         array; if debugging was at some point enabled and this method has not
	 *         been priorly called, the array will contain the previously logged
	 *         messages
	 */
	public byte[] getDebuggingInfo()
	{
		return debugger.toByteArray();
	}

	private int getMajorDictionaryVersion(Buffer version)
	{
		String versionString = version.toString();
		int majorVersion = 0;

		String[] versionList = versionString.split(".");

		if (versionList.length != 0)
			majorVersion = Integer.parseInt(versionList[0]);

		return majorVersion;
	}

}
