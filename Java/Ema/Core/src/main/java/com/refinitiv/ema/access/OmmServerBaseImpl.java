///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019, 2024-2025 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.ClosedSelectorException;
import java.nio.channels.Pipe;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.valueadd.reactor.*;
import org.slf4j.Logger;

import com.refinitiv.ema.access.ConfigManager.ConfigAttributes;
import com.refinitiv.ema.access.ConfigManager.ConfigElement;
import com.refinitiv.ema.access.OmmProvider.DispatchReturn;
import com.refinitiv.ema.access.OmmProvider.DispatchTimeout;
import com.refinitiv.ema.access.ProgrammaticConfigure.InstanceEntryFlag;
import com.refinitiv.ema.access.OmmIProviderConfig.OperationModel;
import com.refinitiv.ema.access.OmmException.ExceptionType;
import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.eta.transport.BindOptions;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Server;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WriteFlags;
import com.refinitiv.eta.transport.WritePriorities;
abstract class OmmServerBaseImpl implements OmmCommonImpl, Runnable, TimeoutClient, ReactorServiceNameToIdCallback, ReactorJsonConversionEventCallback
{
	private final static int SHUTDOWN_TIMEOUT_IN_SECONDS = 3;
	
	static class OmmImplState
	{
		final static int NOT_INITIALIZED = 0;
		final static int INITIALIZED = 1;
		final static int REACTOR_INITIALIZED = 2;
		final static int UNINITIALIZING = 3;
	}
	
	private static int INSTANCE_ID = 0;
	private final static int MIN_TIME_FOR_SELECT = 1000000;
	private final static int MIN_TIME_FOR_SELECT_IN_MILLISEC = 1;
	private final static int DISPATCH_LOOP_COUNT = 15;
	
	private Logger _loggerClient;
	protected StringBuilder _strBuilder = new StringBuilder();
	protected StringBuilder _dispatchStrBuilder = new StringBuilder(1024);
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private OmmInvalidHandleExceptionImpl _ommIHExcept;
	private OmmJsonConverterExceptionImpl ommJCExcept;
	protected LongObject _longValue = new LongObject();
	
	private HashMap<LongObject, ItemInfo>	_itemInfoMap;
	
	private ReentrantLock _userLock = new java.util.concurrent.locks.ReentrantLock();
	private ReentrantLock _dispatchLock = new java.util.concurrent.locks.ReentrantLock();	
	private Reactor _rsslReactor;
	protected ReactorOptions _rsslReactorOpts = ReactorFactory.createReactorOptions();
	protected ReactorErrorInfo _rsslErrorInfo = ReactorFactory.createReactorErrorInfo();
	private ReactorDispatchOptions _rsslDispatchOptions = ReactorFactory.createReactorDispatchOptions();
	private ReactorJsonConverterOptions jsonConverterOptions = ReactorFactory.createReactorJsonConverterOptions();

	private Selector _selector;
	private ExecutorService _executor;
	private volatile boolean _threadRunning = false;
	private Pipe _pipe;
	private AtomicLong _pipeWriteCount = new AtomicLong();
	private byte[] _pipeWriteByte = new byte[]{0x00};
	private ByteBuffer _pipeReadByte = ByteBuffer.allocate(1);
	private volatile boolean _eventReceived;
	private OmmProviderErrorClient _ommProviderErrorClient;
	private OmmProviderClient _ommProviderClient;
	private OmmEventImpl<OmmProviderEvent> _ommProviderEvent;
	private Object _closure;
	
	protected Server _server;
	private BindOptions _bindOptions = TransportFactory.createBindOptions();
	private com.refinitiv.eta.transport.Error _transportError = TransportFactory.createError();
	private ReactorAcceptOptions reactorAcceptOptions = ReactorFactory.createReactorAcceptOptions();
	private ProviderRole _providerRole = ReactorFactory.createProviderRole();
	
	private ReqMsgImpl                  _reqMsgImpl;
	private RefreshMsgImpl				_refreshMsgImpl;
	private StatusMsgImpl				_statusMsgImpl;
	private GenericMsgImpl				_genericMsgImpl;
	private PostMsgImpl 				_postMsgImpl;
	
	private RequestMsg 					_rsslReqMsg;
	
	protected volatile int _state = OmmImplState.NOT_INITIALIZED;
	private boolean _logError = true;	
	
	protected boolean _eventTimeout;
	protected ConcurrentLinkedQueue<TimeoutEvent> _timeoutEventQueue = new ConcurrentLinkedQueue<TimeoutEvent>();
	protected EmaObjectManager _objManager = new EmaObjectManager();
	
	protected ReactorSubmitOptions _rsslSubmitOptions = ReactorFactory.createReactorSubmitOptions();
	
	protected ActiveServerConfig _activeServerConfig;
	
	protected DictionaryHandler _dictionaryHandler;
	protected DirectoryHandler _directoryHandler;
	protected LoginHandler _loginHandler;
	protected MarketItemHandler _marketItemHandler;
	protected ServerChannelHandler _serverChannelHandler;
	protected ItemCallbackClient<OmmProviderClient> _itemCallbackClient;
	private SelectionKey _pipeSelectKey;
	private ArrayList<ReactorChannel> _connectedChannels;

	private ProviderSessionInfo sessionInfo = new ProviderSessionInfo();

	abstract OmmProvider provider();
	
	abstract Logger createLoggerClient();
	
	abstract ConfigAttributes getAttributes(EmaConfigServerImpl config);
	
	abstract Object getAttributeValue(EmaConfigServerImpl config, int AttributeKey);
	
	abstract void readCustomConfig(EmaConfigServerImpl config);
	
	abstract DirectoryServiceStore directoryServiceStore();
	
	abstract void processChannelEvent( ReactorChannelEvent reactorChannelEvent);
	
	void addConnectedChannel(ReactorChannel reactorChannel) {
		_connectedChannels.add(reactorChannel);
	}
	void removeConnectedChannel(ReactorChannel reactorChannel) {
		_connectedChannels.remove(reactorChannel);
	}

	ArrayList<ReactorChannel> connectedChannels() {
		return _connectedChannels;
	}

	OmmServerBaseImpl(OmmProviderClient ommProviderClient, Object closure)
	{
		_itemInfoMap = new HashMap<>();
		_ommProviderClient = ommProviderClient;
		_closure = closure;
		_ommProviderEvent = new OmmEventImpl<OmmProviderEvent>();
		_connectedChannels = new ArrayList<ReactorChannel>();
	}

	OmmServerBaseImpl(OmmProviderClient ommProviderClient, OmmProviderErrorClient providerErrorClient, Object closure)
	{
		_itemInfoMap = new HashMap<>();
		_ommProviderClient = ommProviderClient;
		_ommProviderErrorClient = providerErrorClient;
		_closure = closure;
		_ommProviderEvent = new OmmEventImpl<OmmProviderEvent>();
		_connectedChannels = new ArrayList<ReactorChannel>();
	}
	
	void initialize(ActiveServerConfig activeConfig,EmaConfigServerImpl config)
	{
		_activeServerConfig = activeConfig;
		
		try
		{
			_objManager.initialize(EmaObjectManager.DATA_POOL_INITIAL_SIZE);
			
			GlobalPool.lock();
			GlobalPool.initialize();
			ServerPool.initialize(this,10, _activeServerConfig.itemCountHint);
			GlobalPool.unlock();
			
			_userLock.lock();
			
			_loggerClient = createLoggerClient();

			readConfiguration(config);
			
			readCustomConfig(config);
			
			config.errorTracker().log(this, _loggerClient);
			
			if (_loggerClient.isTraceEnabled())
			{
				_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName, 
					"Print out active configuration detail." + _activeServerConfig.configTrace().toString(), Severity.TRACE));
			}

			ReactorFactory.setReactorMsgEventPoolLimit(activeConfig.globalConfig.reactorMsgEventPoolLimit);
			ReactorFactory.setReactorChannelEventPoolLimit(activeConfig.globalConfig.reactorChannelEventPoolLimit);
			ReactorFactory.setWorkerEventPoolLimit(activeConfig.globalConfig.workerEventPoolLimit);
			ReactorFactory.setTunnelStreamMsgEventPoolLimit(activeConfig.globalConfig.tunnelStreamMsgEventPoolLimit);
			ReactorFactory.setTunnelStreamStatusEventPoolLimit(activeConfig.globalConfig.tunnelStreamStatusEventPoolLimit);
			
			checkServerSharedSocketProperty();

			try
			{
				_pipe = Pipe.open();
				_selector = Selector.open();

			} 
			catch (Exception e)
			{
				strBuilder().append("Failed to open Selector: ").append(e.getLocalizedMessage());
				String temp = _strBuilder.toString();

				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR));
			}

			if (_loggerClient.isTraceEnabled())
				_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName, "Successfully open Selector.", Severity.TRACE));

			if (_activeServerConfig.xmlTraceEnable)
				_rsslReactorOpts.enableXmlTracing();

			if (_activeServerConfig.xmlTraceEnable || _activeServerConfig.xmlTraceToFileEnable){
				if ( _activeServerConfig.xmlTraceEnable) _rsslReactorOpts.enableXmlTracing();
				if ( _activeServerConfig.xmlTraceToFileEnable ) _rsslReactorOpts.enableXmlTraceToFile();

				_rsslReactorOpts.setXmlTraceMaxFileSize(_activeServerConfig.xmlTraceMaxFileSize);
				_rsslReactorOpts.setXmlTraceFileName(_activeServerConfig.xmlTraceFileName);
				if (_activeServerConfig.xmlTraceToMultipleFilesEnable) _rsslReactorOpts.enableXmlTraceToMultipleFiles();
				_rsslReactorOpts.xmlTraceWrite(_activeServerConfig.xmlTraceWriteEnable);
				_rsslReactorOpts.xmlTraceRead(_activeServerConfig.xmlTraceReadEnable);
				_rsslReactorOpts.xmlTracePing(_activeServerConfig.xmlTracePingEnable);
			}

			_rsslReactorOpts.userSpecObj(this);

			_rsslReactor = ReactorFactory.createReactor(_rsslReactorOpts, _rsslErrorInfo);
			if (ReactorReturnCodes.SUCCESS != _rsslErrorInfo.code())
			{
				strBuilder().append("Failed to initialize OmmServerBaseImpl (ReactorFactory.createReactor).")
						.append("' Error Id='").append(_rsslErrorInfo.error().errorId()).append("' Internal sysError='")
						.append(_rsslErrorInfo.error().sysError()).append("' Error Location='")
						.append(_rsslErrorInfo.location()).append("' Error Text='")
						.append(_rsslErrorInfo.error().text()).append("'. ");

				String temp = _strBuilder.toString();

				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR));
			} else
			{
				if (_loggerClient.isTraceEnabled())
					_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName, "Successfully created Reactor.", Severity.TRACE));
			}
			
			_serverChannelHandler = new ServerChannelHandler(this);
			_serverChannelHandler.initialize();
			
			_loginHandler = new LoginHandler(this);
			_loginHandler.initialize();
			
			_directoryHandler = new DirectoryHandler(this);
			_directoryHandler.initialize();
			
			_dictionaryHandler = new DictionaryHandler(this);
			_dictionaryHandler.initialize();
			
			_marketItemHandler = new MarketItemHandler(this);
			_marketItemHandler.initialize();
			
			 _itemCallbackClient = new ItemCallbackClientProvider(this);
			 _itemCallbackClient.initialize();


			jsonConverterOptions.clear();
			jsonConverterOptions.dataDictionary(getDefaultDataDictionary());
			jsonConverterOptions.serviceNameToIdCallback(this);
			jsonConverterOptions.jsonConversionEventCallback(this);
			jsonConverterOptions.defaultServiceId(activeConfig.defaultConverterServiceId);
			jsonConverterOptions.jsonExpandedEnumFields(activeConfig.jsonExpandedEnumFields);
			jsonConverterOptions.catchUnknownJsonKeys(activeConfig.catchUnknownJsonKeys);
			jsonConverterOptions.catchUnknownJsonFids(activeConfig.catchUnknownJsonFids);
			jsonConverterOptions.closeChannelFromFailure(activeConfig.closeChannelFromFailure);
			jsonConverterOptions.jsonConverterPoolsSize(activeConfig.globalConfig.jsonConverterPoolsSize);
			jsonConverterOptions.sendJsonConvError(activeConfig.sendJsonConvError);

			if (_rsslReactor.initJsonConverter(jsonConverterOptions, _rsslErrorInfo) != ReactorReturnCodes.SUCCESS) {
				strBuilder().append("Failed to initialize OmmServerBaseImpl (RWF/JSON Converter).")
						.append("' Error Id='").append(_rsslErrorInfo.error().errorId()).append("' Internal sysError='")
						.append(_rsslErrorInfo.error().sysError()).append("' Error Location='")
						.append(_rsslErrorInfo.location()).append("' Error Text='")
						.append(_rsslErrorInfo.error().text()).append("'. ");

				String temp = _strBuilder.toString();

				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(activeConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR));
			}
			
			_providerRole.channelEventCallback(_serverChannelHandler);
			_providerRole.loginMsgCallback(_loginHandler);
			_providerRole.directoryMsgCallback(_directoryHandler);
	        _providerRole.dictionaryMsgCallback(_dictionaryHandler);
	        _providerRole.defaultMsgCallback(_marketItemHandler);
	        
			_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);
			if ((_activeServerConfig.serverConfig.rsslConnectionType == ConnectionTypes.SOCKET || _activeServerConfig.serverConfig.rsslConnectionType == ConnectionTypes.WEBSOCKET)
					&& ((SocketServerConfig) _activeServerConfig.serverConfig).directWrite )
			{
				_rsslSubmitOptions.writeArgs().flags( _rsslSubmitOptions.writeArgs().flags() |  WriteFlags.DIRECT_SOCKET_WRITE);
			}
	        
	        _bindOptions.connectionType(_activeServerConfig.serverConfig.rsslConnectionType);
			_bindOptions.guaranteedOutputBuffers(_activeServerConfig.serverConfig.guaranteedOutputBuffers);
			_bindOptions.numInputBuffers(_activeServerConfig.serverConfig.numInputBuffers);
			_bindOptions.majorVersion(Codec.majorVersion());
			_bindOptions.minorVersion(Codec.minorVersion());
			_bindOptions.protocolType(Codec.protocolType());
			_bindOptions.serviceName( ((SocketServerConfig)_activeServerConfig.serverConfig).serviceName);
			_bindOptions.interfaceName(_activeServerConfig.serverConfig.interfaceName);
			_bindOptions.pingTimeout(_activeServerConfig.serverConfig.connectionPingTimeout/1000);
			_bindOptions.minPingTimeout(_activeServerConfig.serverConfig.connectionMinPingTimeout/1000);
			_bindOptions.sysRecvBufSize(_activeServerConfig.serverConfig.sysRecvBufSize);
			_bindOptions.sysRecvBufSize(_activeServerConfig.serverConfig.sysSendBufSize);
			_bindOptions.compressionType(_activeServerConfig.serverConfig.compressionType);
			_bindOptions.serverSharedSocket(_activeServerConfig.serverConfig.serverSharedSocket);
			_bindOptions.maxFragmentSize(_activeServerConfig.serverConfig.maxFragmentSize);
			_bindOptions.wSocketOpts().protocols(_activeServerConfig.serverConfig.wsProtocols);
			_bindOptions.encryptionOptions().keystoreFile(_activeServerConfig.serverConfig.keystoreFile);
			_bindOptions.encryptionOptions().keystorePasswd(_activeServerConfig.serverConfig.keystorePasswd);
			if(_activeServerConfig.serverConfig.keystoreType != null)
				_bindOptions.encryptionOptions().keystoreType(_activeServerConfig.serverConfig.keystoreType);
			if(_activeServerConfig.serverConfig.securityProtocol != null)
				_bindOptions.encryptionOptions().securityProtocol(_activeServerConfig.serverConfig.securityProtocol);
			if(_activeServerConfig.serverConfig.securityProtocolVersions != null)
				_bindOptions.encryptionOptions().securityProtocolVersions(_activeServerConfig.serverConfig.securityProtocolVersions);
			if(_activeServerConfig.serverConfig.securityProvider != null)
				_bindOptions.encryptionOptions().securityProvider(_activeServerConfig.serverConfig.securityProvider);
			if(_activeServerConfig.serverConfig.trustManagerAlgorithm != null)
				_bindOptions.encryptionOptions().trustManagerAlgorithm(_activeServerConfig.serverConfig.trustManagerAlgorithm);
			_bindOptions.tcpOpts().tcpNoDelay(_activeServerConfig.serverConfig.tcpNoDelay);

			String productVersion = OmmServerBaseImpl.class.getPackage().getImplementationVersion();
			if ( productVersion == null)
				productVersion = "EMA Java Edition";

			_bindOptions.componentVersion(productVersion);

			_server = Transport.bind(_bindOptions, _transportError);
			if (_server == null)
			{
				strBuilder().append("Failed to initialize OmmServerBaseImpl (Transport.bind).")
					.append("' Error Id='").append(_transportError.errorId()).append("' Internal sysError='")
					.append(_transportError.sysError()).append("' Error Text='")
					.append(_transportError.text()).append("'. ");

				String temp = _strBuilder.toString();

				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.FAILURE));
			}

			if (_loggerClient.isTraceEnabled())
				_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName, "Provider bound on port = " + _server.portNumber() + ".", Severity.TRACE));

			try
			{
				_server.selectableChannel().register(_selector, SelectionKey.OP_ACCEPT, _server);
			}
			catch(ClosedChannelException closedChannelExcep)
			{
				strBuilder().append("Failed to register selector: " + closedChannelExcep.getLocalizedMessage());
				String temp = _strBuilder.toString();

				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR));
			}
			
			try
			{
				_rsslReactor.reactorChannel().selectableChannel().register(_selector, SelectionKey.OP_READ,
						_rsslReactor.reactorChannel());
				_pipe.source().configureBlocking(false);
				_pipeSelectKey = _pipe.source().register(_selector, SelectionKey.OP_READ, _rsslReactor.reactorChannel());
				
			} catch (IOException e)
			{
				strBuilder().append("Failed to register selector: " + e.getLocalizedMessage());
				String temp = _strBuilder.toString();

				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR));
			}
			
			_state = OmmImplState.REACTOR_INITIALIZED;
			
			if (_activeServerConfig.userDispatch == OperationModel.API_DISPATCH)
			{
				_threadRunning = true;

				if (_executor == null)
					_executor = Executors.newSingleThreadExecutor();

				_executor.execute(this);
			}
	    } 
		catch (OmmException exception)
	    {
			uninitialize();

			if (hasErrorClient())
				notifyErrorClient(exception);
			else
				throw exception;
	    } finally
	    {
	    	if (_userLock.isLocked())
	    		_userLock.unlock();
	    }
	}
	
	private void checkServerSharedSocketProperty()
	{
		if (_activeServerConfig.serverConfig.serverSharedSocket &&
					System.getProperty("os.name").toLowerCase().contains("windows") &&
					Boolean.parseBoolean(System.getProperty("sun.net.useExclusiveBind", "true")))
		{
			String errorText = "Failed to initialize OmmServerBaseImpl. " +
									   "serverSharedSocket option is set to true, but system property " +
									   "sun.net.useExclusiveBind is not set to false on Windows platform." +
									   "sun.net.useExclusiveBind property must be set to false " +
									   "when serverSharedSocket option is enabled.";

			if (_loggerClient.isErrorEnabled())
				_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, errorText, Severity.ERROR));

			throw (ommIUExcept().message(errorText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION));
		}

	}

	//intenal use, only for junit test
	void initializeForTest(ActiveServerConfig activeConfig,EmaConfigServerImpl config)
	{
		_activeServerConfig = activeConfig;
		
		try
		{
			_objManager.initialize(EmaObjectManager.DATA_POOL_INITIAL_SIZE);
			
			GlobalPool.lock();
			GlobalPool.initialize();
			GlobalPool.unlock();
			
			_userLock.lock();
			
			_loggerClient = createLoggerClient();

			readConfiguration(config);
			
			readCustomConfig(config);
			
			config.errorTracker().log(this, _loggerClient);
			
			if (_loggerClient.isTraceEnabled())
			{
				_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName, 
					"Print out active configuration detail." + _activeServerConfig.configTrace().toString(), Severity.TRACE));
			}
	    } 
		catch (OmmException exception)
		{
				throw exception;
		} finally
		{
			if (_userLock.isLocked())
				_userLock.unlock();
		}
	}

	@Override
	public int reactorJsonConversionEventCallback(ReactorJsonConversionEvent jsonConversionEvent) {
		final Error error = jsonConversionEvent.error();
		handleJsonConverterError(jsonConversionEvent.reactorChannel(), error.errorId(), error.text());
		return ReactorReturnCodes.SUCCESS;
	}

	@Override
	public int reactorServiceNameToIdCallback(ReactorServiceNameToId serviceNameToId, ReactorServiceNameToIdEvent serviceNameToIdEvent) {
		if (serviceNameToId.serviceName() == null || serviceNameToId.serviceName().isEmpty()) {
			return ReactorReturnCodes.FAILURE;
		}
		DirectoryServiceStore.ServiceIdInteger serviceId = directoryServiceStore().serviceId(serviceNameToId.serviceName());
		if (serviceId != null) {
			serviceNameToId.serviceId(serviceId.value());
			return ReactorReturnCodes.SUCCESS;
		}
		return ReactorReturnCodes.FAILURE;
	}

	void uninitialize()
	{
		try
		{
			if (_activeServerConfig.userDispatch == OperationModel.API_DISPATCH)
			{
				if (_executor != null)
				{
					_executor.shutdown();
					_threadRunning = false;
					_eventReceived = true;
					_selector.wakeup();
					if (!_executor.awaitTermination(SHUTDOWN_TIMEOUT_IN_SECONDS, TimeUnit.SECONDS))
					{
						if (_loggerClient.isErrorEnabled())
						{
							strBuilder().append("Failed to uninitialize OmmBaseImpl (_executor.awaitTermination() timed out).");
							_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
						}
					}
				}			
				_userLock.lock();
			}
			else
			{
				_userLock.lock();
				_threadRunning = false;
				_eventReceived = true;
				_selector.wakeup();
			}
			
			if (_state == OmmImplState.NOT_INITIALIZED)
				return;
			
			_state = OmmImplState.NOT_INITIALIZED;
			
			if (ReactorReturnCodes.SUCCESS != _rsslReactor.shutdown(_rsslErrorInfo))
			{
				if (_loggerClient.isErrorEnabled())
				{
					strBuilder().append("Failed to uninitialize OmmBaseImpl (Reactor.shutdown).")
							.append("Error Id ").append(_rsslErrorInfo.error().errorId())
							.append("Internal sysError ").append(_rsslErrorInfo.error().sysError())
							.append("Error Location ").append(_rsslErrorInfo.location()).append("Error Text ")
							.append(_rsslErrorInfo.error().text()).append("'. ");

					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
				}
			}
			
			if (_serverChannelHandler != null)
			{
				_serverChannelHandler.closeActiveSessions();
				_serverChannelHandler = null;
			}
			
			if(_server != null && _server.state() == ChannelState.ACTIVE &&  TransportReturnCodes.SUCCESS != _server.close(_rsslErrorInfo.error()))
			{
				if (_loggerClient.isErrorEnabled())
				{
					strBuilder().append("Server.Close() failed while uninitializing OmmServerBaseImpl.")
							.append("Error Id ").append(_rsslErrorInfo.error().errorId())
							.append("Internal sysError ").append(_rsslErrorInfo.error().sysError())
							.append("Error Text ")
							.append(_rsslErrorInfo.error().text()).append("'. ");

					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
				}
			}

			_pipe.sink().close();
			_pipe.source().close();
			_selector.close();
			
		} catch (InterruptedException | IOException e)
		{
			strBuilder().append("OmmServerBaseImpl unintialize(), Exception occurred, exception=")
					.append(e.getLocalizedMessage());

			if (_loggerClient.isErrorEnabled())
				_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, _strBuilder.toString(), Severity.ERROR));

		} finally
		{
			_userLock.unlock();
			_rsslReactor = null;
			_server = null;
		}
	}
	
	int state()
	{
		return _state;
	}
	
	void readConfiguration(EmaConfigServerImpl config)
	{
		_activeServerConfig.configuredName = config.configuredName();

		_activeServerConfig.instanceName = strBuilder().append(_activeServerConfig.configuredName).append("_").append(Integer.toString(++INSTANCE_ID)).toString();

		ConfigAttributes attributes = getAttributes(config);

		ConfigElement ce = null;
		int value = 0;
		
		if (attributes != null)
		{
			if ((ce = attributes.getPrimitiveValue(ConfigManager.ItemCountHint)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeServerConfig.itemCountHint = value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ServiceCountHint)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeServerConfig.serviceCountHint = value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.RequestTimeout)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeServerConfig.requestTimeout = value;
			}
	            
			if ((ce = attributes.getPrimitiveValue(ConfigManager.DispatchTimeoutApiThread)) != null)
			{
				value = ce.intValue();
				if (value >= 0)
					_activeServerConfig.dispatchTimeoutApiThread = value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.MaxDispatchCountApiThread)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeServerConfig.maxDispatchCountApiThread = value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.MaxDispatchCountUserThread)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeServerConfig.maxDispatchCountUserThread = value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.DefaultServiceID)) != null) {
				value = ce.intLongValue();
				if (value >= 0) {
					_activeServerConfig.defaultConverterServiceId = ce.intLongValue();
				}
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.JsonExpandedEnumFields)) != null) {
				_activeServerConfig.jsonExpandedEnumFields = ce.intLongValue() > 0;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.CatchUnknownJsonKeys)) != null) {
				_activeServerConfig.catchUnknownJsonKeys = ce.intLongValue() > 0;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.CatchUnknownJsonFids)) != null) {
				_activeServerConfig.catchUnknownJsonFids = ce.intLongValue() > 0;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.CloseChannelFromConverterFailure)) != null) {
				_activeServerConfig.closeChannelFromFailure = ce.intLongValue() > 0;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.SendJsonConvError)) != null) {
				_activeServerConfig.sendJsonConvError = ce.intLongValue() > 0;
			}
				
			if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceToStdout)) != null)
			{
				_activeServerConfig.xmlTraceEnable = ce.intLongValue() != 0;
			}
			if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceToFile)) != null)
			{
				_activeServerConfig.xmlTraceToFileEnable = ce.intLongValue() != 0;
			}
			if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceMaxFileSize)) != null)
			{
				long xmlTraceMaxFileSize = ce.intLongValue();
				if (xmlTraceMaxFileSize > 0)
					_activeServerConfig.xmlTraceMaxFileSize = xmlTraceMaxFileSize;
			}
			if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceFileName)) != null)
			{
				_activeServerConfig.xmlTraceFileName = (String) ce.value();
			}
			if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceToMultipleFiles)) != null)
			{
				_activeServerConfig.xmlTraceToMultipleFilesEnable = ce.intLongValue() != 0;
			}
			if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceWrite)) != null)
			{
				_activeServerConfig.xmlTraceWriteEnable = ce.intLongValue() != 0;
			}
			if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceRead)) != null)
			{
				_activeServerConfig.xmlTraceReadEnable = ce.intLongValue() != 0;
			}
			if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTracePing)) != null)
			{
				_activeServerConfig.xmlTracePingEnable = ce.intLongValue() != 0;
			}
		}

		// .........................................................................
		// Server
		//
		String serverName =  config.serverName(_activeServerConfig.configuredName);
		if ( serverName != null)
			_activeServerConfig.serverConfig = readServerConfig(config, serverName);
		else
		{
			SocketServerConfig socketServerConfig = new SocketServerConfig();
			if (socketServerConfig.rsslConnectionType == ConnectionTypes.SOCKET || socketServerConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
			{
				String tempService = config.getUserSpecifiedPort();
				if (tempService != null)
					socketServerConfig.serviceName = tempService;
			}
			_activeServerConfig.serverConfig = socketServerConfig;
		}
		
		_activeServerConfig.serverConfig.keystoreFile = ((OmmIProviderConfigImpl)config).keystoreFile();
		_activeServerConfig.serverConfig.keystorePasswd = ((OmmIProviderConfigImpl)config).keystorePasswd();
		_activeServerConfig.serverConfig.keystoreType = ((OmmIProviderConfigImpl)config).keystoreType();
		_activeServerConfig.serverConfig.securityProtocol = ((OmmIProviderConfigImpl)config).securityProtocol();
		_activeServerConfig.serverConfig.securityProtocolVersions = ((OmmIProviderConfigImpl)config).securityProtocolVersions();
		_activeServerConfig.serverConfig.securityProvider = ((OmmIProviderConfigImpl)config).securityProvider();
		_activeServerConfig.serverConfig.keyManagerAlgorithm = ((OmmIProviderConfigImpl)config).keyManagerAlgorithm();
		_activeServerConfig.serverConfig.trustManagerAlgorithm = ((OmmIProviderConfigImpl)config).trustManagerAlgorithm();

		ConfigAttributes globalConfigAttributes = config.xmlConfig().getGlobalConfig();

		if(globalConfigAttributes != null){
			_activeServerConfig.globalConfig = new GlobalConfig();
			if( (ce = globalConfigAttributes.getPrimitiveValue(ConfigManager.WorkerEventPoolLimit)) != null)
			{
				_activeServerConfig.globalConfig.workerEventPoolLimit = ce.intValue();
			}
			if( (ce = globalConfigAttributes.getPrimitiveValue(ConfigManager.ReactorChannelEventPoolLimit)) != null)
			{
				_activeServerConfig.globalConfig.reactorChannelEventPoolLimit = ce.intValue();
			}

			if( (ce = globalConfigAttributes.getPrimitiveValue(ConfigManager.ReactorMsgEventPoolLimit)) != null)
			{
				_activeServerConfig.globalConfig.reactorMsgEventPoolLimit = ce.intValue();
			}

			if( (ce = globalConfigAttributes.getPrimitiveValue(ConfigManager.TunnelStreamMsgEventPoolLimit)) != null)
			{
				_activeServerConfig.globalConfig.tunnelStreamMsgEventPoolLimit = ce.intValue();
			}

			if( (ce = globalConfigAttributes.getPrimitiveValue(ConfigManager.TunnelStreamStatusEventPoolLimit)) != null)
			{
				_activeServerConfig.globalConfig.tunnelStreamStatusEventPoolLimit = ce.intValue();
			}
			if( (ce = globalConfigAttributes.getPrimitiveValue(ConfigManager.JsonConverterPoolsSize)) != null)
			{
				_activeServerConfig.globalConfig.jsonConverterPoolsSize =
						getJsonConverterPoolsSize(ce, _activeServerConfig, strBuilder(), _loggerClient);
			}
		}
		
		ProgrammaticConfigure pc = config.programmaticConfigure();
		if (pc != null)
		{
			pc.retrieveCommonConfig(_activeServerConfig.configuredName, _activeServerConfig);
			serverName = pc.activeEntryNames(_activeServerConfig.configuredName, InstanceEntryFlag.SERVER_FLAG);

			if ( serverName != null && !serverName.isEmpty() )
			{
				_activeServerConfig.serverConfig = null;
				ServerConfig fileServerConfig = readServerConfig(config, serverName);

				int serverConfigByFuncCall = 0;
				if (config.getUserSpecifiedPort() != null)
					serverConfigByFuncCall = ActiveConfig.SOCKET_SERVER_PORT_CONFIG_BY_FUNCTION_CALL;

				pc.retrieveServerConfig(serverName, _activeServerConfig, serverConfigByFuncCall, fileServerConfig);
				if (_activeServerConfig.serverConfig == null)
					_activeServerConfig.serverConfig = fileServerConfig;
				else
				{
					fileServerConfig = null;
				}
			}
			
			GlobalConfig globalConfig = pc.retrieveGlobalConfig();
			if(globalConfig != null){
				_activeServerConfig.globalConfig = globalConfig;
			}
		}

		_activeServerConfig.userDispatch = config.operationModel();
	}

	ServerConfig readServerConfig(EmaConfigServerImpl configImpl, String serverName)
	{
		int maxInt = Integer.MAX_VALUE;

		ConfigAttributes attributes = null;
		ConfigElement ce = null;
		int serverType = ConnectionTypes.SOCKET;
		ServerConfig newServerConfig = null;
		
		attributes = configImpl.xmlConfig().getServerAttributes(serverName);
		if (attributes != null) 
			ce = attributes.getPrimitiveValue(ConfigManager.ServerType);

		if (ce == null)
		{
			if (configImpl.getUserSpecifiedPort() != null)
				serverType = ConnectionTypes.SOCKET;
		} else
		{
			serverType = ce.intValue();
		}

		switch (serverType)
		{
		case ConnectionTypes.ENCRYPTED:
		case ConnectionTypes.SOCKET:
		case ConnectionTypes.WEBSOCKET:
		{
			SocketServerConfig socketServerConfig = new SocketServerConfig();
			newServerConfig = socketServerConfig;
			
			newServerConfig.rsslConnectionType = serverType;

			String tempService = configImpl.getUserSpecifiedPort();
			if (tempService == null)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ServerPort)) != null)
					socketServerConfig.serviceName = ce.asciiValue();
			}
			else
				socketServerConfig.serviceName = tempService;
			
			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ServerTcpNodelay)) != null)
				socketServerConfig.tcpNodelay = ce.intLongValue() == 0 ? false : ActiveConfig.DEFAULT_TCP_NODELAY;

			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ServerDirectSocketWrite)) != null)
				socketServerConfig.directWrite = ce.intLongValue() == 1 ? true : ActiveConfig.DEFAULT_DIRECT_SOCKET_WRITE;
			
			break;
		}
		default:
		{
			configImpl.errorTracker().append("Not supported server type. Type = ")
					.append(ConnectionTypes.toString(serverType));
			throw ommIUExcept().message(configImpl.errorTracker().text(), OmmInvalidUsageException.ErrorCode.UNSUPPORTED_SERVER_TYPE );
		}
		}

		newServerConfig.name = serverName;

		if (attributes != null)
		{
			if((ce = attributes.getPrimitiveValue(ConfigManager.InterfaceName)) != null)
				newServerConfig.interfaceName = ce.asciiValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.GuaranteedOutputBuffers)) != null)
				newServerConfig.guaranteedOutputBuffers(ce.intLongValue());
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.NumInputBuffers)) != null)
				newServerConfig.numInputBuffers(ce.intLongValue());
	
			boolean setCompressionThresholdFromConfigFile = false;
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ServerCompressionThreshold)) != null)
			{
				newServerConfig.compressionThresholdSet = true;
				setCompressionThresholdFromConfigFile = true;
				if ( ce.intLongValue()  > maxInt )
					newServerConfig.compressionThreshold = maxInt;
				else
					newServerConfig.compressionThreshold = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_COMPRESSION_THRESHOLD : ce.intLongValue();
			}

			if( (ce = attributes.getPrimitiveValue(ConfigManager.ServerCompressionType)) != null) {
				newServerConfig.compressionType = ce.intValue() < 0 ? ActiveConfig.DEFAULT_COMPRESSION_TYPE : ce.intValue();
				if (newServerConfig.compressionType == com.refinitiv.eta.transport.CompressionTypes.LZ4 &&
						!setCompressionThresholdFromConfigFile)
					newServerConfig.compressionThreshold = ActiveConfig.DEFAULT_COMPRESSION_THRESHOLD_LZ4;
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.SysRecvBufSize)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					newServerConfig.sysRecvBufSize = maxInt;
				else
					newServerConfig.sysRecvBufSize = ce.intLongValue() <= 0 ? ActiveConfig.DEFAULT_SYS_RECEIVE_BUFFER_SIZE : ce.intLongValue();
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.SysSendBufSize)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					newServerConfig.sysSendBufSize = maxInt;
				else
					newServerConfig.sysSendBufSize = ce.intLongValue() <= 0 ? ActiveConfig.DEFAULT_SYS_SEND_BUFFER_SIZE : ce.intLongValue();
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.HighWaterMark)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					newServerConfig.highWaterMark = maxInt;
				else
					newServerConfig.highWaterMark = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_HIGH_WATER_MARK : ce.intLongValue();
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ConnectionPingTimeout)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					newServerConfig.connectionPingTimeout = maxInt;
				else
					newServerConfig.connectionPingTimeout = ce.intLongValue() < 0 ? ActiveServerConfig.DEFAULT_CONNECTION_PINGTIMEOUT : ce.intLongValue();
			}
            
            if( (ce = attributes.getPrimitiveValue(ConfigManager.ConnectionMinPingTimeout)) != null)
            {
                if ( ce.intLongValue()  > maxInt )
                	newServerConfig.connectionMinPingTimeout = maxInt;
                else
                	newServerConfig.connectionMinPingTimeout = ce.intLongValue() < 0 ? ActiveServerConfig.DEFAULT_CONNECTION_MINPINGTIMEOUT : ce.intLongValue();
            }
            
            if( (ce = attributes.getPrimitiveValue(ConfigManager.ServerInitTimeout)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					newServerConfig.initializationTimeout = maxInt;
				else
					newServerConfig.initializationTimeout = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_INITIALIZATION_ACCEPT_TIMEOUT : ce.intLongValue();
			}

            if ((ce = attributes.getPrimitiveValue(ConfigManager.ServerMaxFragmentSize)) != null) {
				if ( ce.intLongValue()  > maxInt ) {
					newServerConfig.maxFragmentSize = maxInt;
				} else {
					newServerConfig.maxFragmentSize = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_INITIALIZATION_ACCEPT_TIMEOUT : ce.intLongValue();
				}
			}

            if ((ce = attributes.getPrimitiveValue(ConfigManager.ServerWsProtocols)) != null) {
            	newServerConfig.wsProtocols = ce.asciiValue();
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ServerSharedSocket)) != null)
				newServerConfig.serverSharedSocket = ce.intLongValue() == 1 ? true : ActiveServerConfig.DEFAULT_SERVER_SHARED_SOCKET;
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ServerTcpNodelay)) != null)
			{
				newServerConfig.tcpNoDelay = ce.intLongValue() != 0;
			}
		}
		
		return newServerConfig;
	}
	
	OmmProviderClient ommProviderClient()
	{
		return _ommProviderClient;
	}
	
	OmmEventImpl<OmmProviderEvent> ommProviderEvent()
	{
		return _ommProviderEvent;
	}
	
	Object closure()
	{
		return _closure;
	}
	
	ActiveServerConfig activeConfig()
	{
		return _activeServerConfig;
	}
	
	@Override
	public StringBuilder strBuilder()
	{
		_strBuilder.setLength(0);
		return _strBuilder;
	}
	
	boolean hasErrorClient()
	{
		return (_ommProviderErrorClient != null);
	}
	
	void notifyErrorClient(OmmException ommException)
	{
		switch (ommException.exceptionType())
		{
		case ExceptionType.OmmInvalidHandleException:
			_ommProviderErrorClient.onInvalidHandle(((OmmInvalidHandleException) ommException).handle(), ommException.getMessage());
			break;
		case ExceptionType.OmmInvalidUsageException:
			_ommProviderErrorClient.onInvalidUsage(ommException.getMessage());
			_ommProviderErrorClient.onInvalidUsage(ommException.getMessage(), ((OmmInvalidUsageException)ommException).errorCode());
			break;
		case ExceptionType.OmmJsonConverterException:
			_ommProviderErrorClient.onJsonConverterError((ProviderSessionInfo) ((OmmJsonConverterException) ommException).getSessionInfo(),
					((OmmJsonConverterException) ommException).getErrorCode(), ommException.getMessage());
			break;
		default:
			break;
		}
	}

	void onDispatchError(String text, int errorCode)
	{
		_ommProviderErrorClient.onDispatchError(text, errorCode);
	}

	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();

		return _ommIUExcept;
	}

	OmmInvalidHandleExceptionImpl ommIHExcept()
	{
		if (_ommIHExcept == null)
			_ommIHExcept = new OmmInvalidHandleExceptionImpl();

		return _ommIHExcept;
	}

	OmmJsonConverterExceptionImpl ommJCExcept() {
		if (ommJCExcept == null) {
			ommJCExcept = new OmmJsonConverterExceptionImpl();
		}
		return ommJCExcept;
	}
	
	public void handleInvalidUsage(String text, int errorCode)
	{
		if ( hasErrorClient() )
		{
			_ommProviderErrorClient.onInvalidUsage(text);
			
			_ommProviderErrorClient.onInvalidUsage(text, errorCode);
		}
		else
			throw (ommIUExcept().message(text.toString(), errorCode));
		
	}

	public void handleInvalidHandle(long handle, String text)
	{	
		if ( hasErrorClient() )
			_ommProviderErrorClient.onInvalidHandle(handle, text);
		else
			throw (ommIHExcept().message(text, handle));
	}

	@Override
	public void handleJsonConverterError(ReactorChannel reactorChannel, int errorCode, String text) {
		sessionInfo.loadProviderSession(provider(), reactorChannel);
		if (hasErrorClient()) {
			_ommProviderErrorClient.onJsonConverterError(sessionInfo, errorCode, text);
		} else {
			if (userLock().isLocked()) {
				userLock().unlock();
			}
			if (_activeServerConfig.userDispatch != OperationModel.API_DISPATCH) {
				throw (ommJCExcept().message(sessionInfo, errorCode, text));
			}
		}
	}

	public Logger loggerClient()
	{
		return _loggerClient;
	}
	
	@Override
	public ReentrantLock userLock()
	{
		return _userLock;
	}
	
	TimeoutEvent addTimeoutEvent(long timeoutInMicroSec, TimeoutClient client)
	{
		TimeoutEvent timeoutEvent = (TimeoutEvent) _objManager._timeoutEventPool.poll();
		if (timeoutEvent == null)
		{
			timeoutEvent = new TimeoutEvent(timeoutInMicroSec * 1000, client);
			_objManager._timeoutEventPool.updatePool(timeoutEvent);
		} else
			timeoutEvent.timeoutInNanoSec(timeoutInMicroSec * 1000, client);

		_timeoutEventQueue.add(timeoutEvent);
		
		try 
		{
			installTimeOut();
		} 
		catch (IOException e) 
		{
			if (_loggerClient.isErrorEnabled())
			{
				strBuilder().append("Write to pipe failed on addTimeoutEvent. Received exception, ")
						.append(" exception text= ").append(e.getLocalizedMessage()).append(". ");

				_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
			}

			uninitialize();
		}
		
		return timeoutEvent;
	}
	
	void installTimeOut() throws IOException
	{
		pipeWrite();
	}
	
	void pipeWrite() throws IOException
	{
		if (_pipe.sink().isOpen() && _pipeWriteCount.incrementAndGet() == 1)
			_pipe.sink().write(ByteBuffer.wrap(_pipeWriteByte));
	}
	
	void pipeRead() throws IOException
	{
		if (_pipe.source().isOpen() && _pipeWriteCount.decrementAndGet() == 0)
		{
			_pipeReadByte.clear();
			_pipe.source().read(_pipeReadByte);
		}
	}
	
	Server server()
	{
		return _server;
	}
	
	Selector selector()
	{
		return _selector;
	}
	
	boolean rsslReactorDispatchLoop(long timeOut, int count)
	{
		if (_state == OmmImplState.NOT_INITIALIZED)
		{
			if (_loggerClient.isErrorEnabled() && _logError)
			{
				_logError = false;				
				_dispatchStrBuilder.setLength(0);
				_dispatchStrBuilder.append("Call to rsslReactorDispatchLoop() failed. The _state is set to OmmImplState.NOT_INITIALIZED");

				_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, _dispatchStrBuilder.toString(), Severity.ERROR));
			}		
			return false;
		}
		
		_eventReceived = false;
		_rsslDispatchOptions.maxMessages(count);
		int ret = ReactorReturnCodes.SUCCESS;
		int loopCount = 0;
		long startTime = System.nanoTime();
		long endTime = 0;

		boolean noWait = timeOut == OmmProvider.DispatchTimeout.NO_WAIT;
		timeOut = timeOut * 1000;
		long userTimeout = TimeoutEvent.userTimeOutExist(_timeoutEventQueue);
		boolean userTimeoutExist = false;
		if (userTimeout >= 0)
		{
			if (timeOut > 0 && timeOut < userTimeout)
				userTimeoutExist = false;
			else
			{
				userTimeoutExist = true;
				/*if userTimeout is less than 1000000, need to reset to 10000000 because
				 *the select() call will wait forever and not return if timeout is 0 */
				timeOut = (userTimeout > MIN_TIME_FOR_SELECT ? userTimeout : MIN_TIME_FOR_SELECT);
			}
		}

		try
		{
				endTime = System.nanoTime();
	
				if ( timeOut > 0 )
				{
					timeOut -= endTime - startTime;
					if ( timeOut <=0 )
					{
						if (userTimeoutExist)
							TimeoutEvent.execute(_timeoutEventQueue);
					}
				}
				
				if ( timeOut < MIN_TIME_FOR_SELECT  )
					timeOut = MIN_TIME_FOR_SELECT;

			while (_state != OmmImplState.NOT_INITIALIZED)
			{
				startTime = endTime;
			
				int selectTimeout = (int)(timeOut/MIN_TIME_FOR_SELECT);
				int selectCount = 0;
				if (noWait)
				{
					selectCount = _selector.selectNow();
				}
				else
				{
					selectCount = _selector.select(selectTimeout > 0 ? selectTimeout : MIN_TIME_FOR_SELECT_IN_MILLISEC);
				}

				if (selectCount > 0 || !_selector.selectedKeys().isEmpty())
				{
					Iterator<SelectionKey> iter = _selector.selectedKeys().iterator();
					while (iter.hasNext())
					{
						SelectionKey key = iter.next();
						iter.remove();
						try
						{
							if (!key.isValid())
								continue;
							if (key.isAcceptable()) 
							{
								reactorAcceptOptions.clear();
								ClientSession clientSession = ServerPool.getClientSession(this);
								reactorAcceptOptions.acceptOptions().userSpecObject(clientSession);
								reactorAcceptOptions.acceptOptions().nakMount(false);
								reactorAcceptOptions.initTimeout(_activeServerConfig.serverConfig.initializationTimeout);
								
								if (_rsslReactor.accept(_server, reactorAcceptOptions, _providerRole, _rsslErrorInfo) != ReactorReturnCodes.SUCCESS)
								{
									if (_loggerClient.isErrorEnabled()) 
									{
										_dispatchStrBuilder.setLength(0);
										_dispatchStrBuilder
												.append("Failed to initialize OmmServerBaseImpl (Reactor.accept).")
												.append("' Error Id='").append(_rsslErrorInfo.error().errorId())
												.append("' Internal sysError='")
												.append(_rsslErrorInfo.error().sysError()).append("' Error Location='")
												.append(_rsslErrorInfo.location()).append("' Error Text='")
												.append(_rsslErrorInfo.error().text()).append("'. ");

										String temp = _dispatchStrBuilder.toString();

										_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp,
												Severity.ERROR));
									}
									
									clientSession.returnToPool();
									
									return false;
								}
							}
							if (key.isReadable())
							{
								if (_pipeSelectKey == key) pipeRead();
								
								loopCount = 0;
								do {
									_userLock.lock();
									try{
										ret = ((ReactorChannel) key.attachment()).dispatch(_rsslDispatchOptions, _rsslErrorInfo);
									}
									finally{
										if(_userLock.isLocked())
										{
											_userLock.unlock();
										}
									}
								}
								while ( ret > ReactorReturnCodes.SUCCESS && !_eventReceived && ++loopCount < DISPATCH_LOOP_COUNT );
							}
						}
						 catch (CancelledKeyException e)
						{
							continue;
						}
					}
					
					if ( _eventReceived ) return true;
					
					loopCount = 0;
					do {
						_userLock.lock();
						try
						{
							ret = _rsslReactor != null ? _rsslReactor.dispatchAll(null, _rsslDispatchOptions, _rsslErrorInfo) : ReactorReturnCodes.SUCCESS;
						}
						finally {
							if(_userLock.isLocked())
							{
								_userLock.unlock();
							}
						}
					} while (ret > ReactorReturnCodes.SUCCESS && !_eventReceived && ++loopCount < 15);

					if (ret < ReactorReturnCodes.SUCCESS) 
					{
						_userLock.lock();
						try {
							if (_loggerClient.isErrorEnabled() || hasErrorClient())
							{
								strBuilder().append("Call to rsslReactorDispatchLoop() failed. Internal sysError='")
										.append(_rsslErrorInfo.error().sysError()).append("' Error text='")
										.append(_rsslErrorInfo.error().text()).append("'. ");
							}

							if (_loggerClient.isErrorEnabled())
							{
								_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName,
										_strBuilder.toString(), Severity.ERROR));
							}

							if (hasErrorClient())
							{
								onDispatchError(_strBuilder.toString(), ret);
							}
						} finally {
							_userLock.unlock();
						}

						return false;
					}

					if ( _eventReceived ) return true;
					
					TimeoutEvent.execute(_timeoutEventQueue);
					
					if ( _eventReceived ) return true;
				} //selectCount > 0
				else if (selectCount == 0)
				{
					TimeoutEvent.execute(_timeoutEventQueue);
						
					if ( _eventReceived ) return true;
				}
		
				endTime = System.nanoTime();
				if ( timeOut > 0 )
				{
					timeOut -= ( endTime - startTime );
					if (timeOut < MIN_TIME_FOR_SELECT) return false;
				}
				
				if (Thread.currentThread().isInterrupted())
				{
					_userLock.lock();
					try {
						_threadRunning = false;

						if (_loggerClient.isTraceEnabled())
						{
							_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName,
									"Call to rsslReactorDispatchLoop() received thread interruption signal.", Severity.TRACE));
						}
					} finally {
						_userLock.unlock();
					}
				}
			}
			
			return false;
			
		} //end of Try		
		catch (CancelledKeyException e)
		{
			_userLock.lock();
			try {
				if (_loggerClient.isTraceEnabled() && _state != OmmImplState.NOT_INITIALIZED )
				{
					_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName,
							"Call to rsslReactorDispatchLoop() received cancelled key exception.", Severity.TRACE));
				}
			} finally {
				_userLock.unlock();
			}

			return true;
		} 
		catch (ClosedSelectorException e)
		{
			_userLock.lock();
			try {
				if (_loggerClient.isTraceEnabled() && _state != OmmImplState.NOT_INITIALIZED )
				{
					_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName,
							"Call to rsslReactorDispatchLoop() received closed selector exception.", Severity.TRACE));
				}
			} finally{
				_userLock.unlock();
			}

			return true;
		} 
		catch (IOException e)
		{
			_userLock.lock();
			try {
				if (_loggerClient.isErrorEnabled()) {
					_dispatchStrBuilder.setLength(0);
					_dispatchStrBuilder.append("Call to rsslReactorDispatchLoop() failed. Received exception,")
							.append(" exception text= ").append(e.getLocalizedMessage()).append(". ");

					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, _dispatchStrBuilder.toString(), Severity.ERROR));
				}
			} finally {
				_userLock.lock();
			}
			uninitialize();
			return false;
		}
	}
	
	public void addItemInfo(ClientSession clientSession, ItemInfo itemInfo)
	{
		_userLock.lock();
		
		_itemInfoMap.put(itemInfo.handle(), itemInfo);
		itemInfo.clientSession().addItemInfo(itemInfo);
		
		if ( loggerClient().isTraceEnabled( ))
		{
			StringBuilder temp = strBuilder();
			temp.append("Added ItemInfo ").append(itemInfo.handle().value()).append(" to ItemInfoMap" ).append( OmmLoggerClient.CR )
			.append("Client handle ").append(itemInfo.clientSession().clientHandle().value());
			loggerClient().trace(formatLogMessage(instanceName(), temp.toString(), Severity.TRACE));
		}
		
		_userLock.unlock();
	}
	
	public void removeItemInfo(ItemInfo itemInfo, boolean eraseItemGroup)
	{
		_userLock.lock();
		
		_itemInfoMap.remove(itemInfo.handle());
		itemInfo.clientSession().removeItemInfo(itemInfo);
		
		if ( eraseItemGroup && itemInfo.hasItemGroup() )
		{
			removeItemGroup(itemInfo);
		}
		
		if ( loggerClient().isTraceEnabled( ))
		{
			StringBuilder temp = strBuilder();
			temp.append("Removed ItemInfo ").append(itemInfo.handle().value()).append(" from ItemInfoMap" ).append( OmmLoggerClient.CR )
			.append("Client handle ").append(itemInfo.clientSession().clientHandle().value());
			loggerClient().trace(formatLogMessage(instanceName(), temp.toString(), Severity.TRACE));
		}
		
		itemInfo.returnToPool();

		_userLock.unlock();
	}
	
	public ItemInfo getItemInfo(long handle)
	{
		_userLock.lock();
		
		_longValue.value(handle);
		
		ItemInfo itemInfo = _itemInfoMap.get(_longValue);
		
		_userLock.unlock();
		
		return itemInfo;
	}
	
	void addItemGroup(ItemInfo itemInfo, Buffer groupId)
	{
		_userLock.lock();
		
		_longValue.value(itemInfo.serviceId());
		
		HashMap<Buffer, ArrayList<ItemInfo>> groupIdToItemInfoList = itemInfo.clientSession().serviceGroupIdToItemInfoMap().get(_longValue);
		
		if( groupIdToItemInfoList != null )
		{
			ArrayList<ItemInfo> itemInfoList = groupIdToItemInfoList.get(groupId);
			
			if( itemInfoList != null )
			{
				itemInfoList.add(itemInfo);
			}
			else
			{
				itemInfoList = new ArrayList<ItemInfo>(1000);
				itemInfoList.add(itemInfo);
				
				groupIdToItemInfoList.put(groupId, itemInfoList);
			}
		}
		else
		{
			groupIdToItemInfoList = new HashMap<Buffer, ArrayList<ItemInfo>>();
			
			ArrayList<ItemInfo> itemInfoList = new ArrayList<ItemInfo>(100);
			itemInfoList.add(itemInfo);
			
			groupIdToItemInfoList.put(groupId, itemInfoList);
			itemInfo.clientSession().serviceGroupIdToItemInfoMap().put(_longValue, groupIdToItemInfoList);
		}
		
		_userLock.unlock();
	}
	
	void updateItemGroup(ItemInfo itemInfo, Buffer newGroupId)
	{
		removeItemGroup(itemInfo);
		addItemGroup(itemInfo, newGroupId);
	}
	
	void removeItemGroup(ItemInfo itemInfo)
	{
		_userLock.lock();
		
		_longValue.value(itemInfo.serviceId());
		
		HashMap<Buffer, ArrayList<ItemInfo>> groupIdToItemInfoList = itemInfo.clientSession().serviceGroupIdToItemInfoMap().get(_longValue);
		
		if( groupIdToItemInfoList != null )
		{
			ArrayList<ItemInfo> itemInfoList = groupIdToItemInfoList.get(itemInfo.itemGroup());
			
			if( itemInfoList != null )
			{
				itemInfoList.remove(itemInfo);
			}
			
			if(itemInfoList.size() == 0)
			{
				groupIdToItemInfoList.remove(itemInfo.itemGroup());
			}
		}
		
		int flags = itemInfo.flags();
		flags &= ~ItemInfo.ItemInfoFlags.ITEM_GROUP;
		itemInfo.flags(flags);
		
		_userLock.unlock();
	}
	
	void removeServiceId(ClientSession clientSession, int serviceId)
	{
		_userLock.lock();
		
		_longValue.value(serviceId);
		
		HashMap<Buffer, ArrayList<ItemInfo>> groupIdToItemInfoList = clientSession.serviceGroupIdToItemInfoMap().get(_longValue);
		
		if( groupIdToItemInfoList != null )
		{
			Iterator<ArrayList<ItemInfo>> iterator = groupIdToItemInfoList.values().iterator();
			ArrayList<ItemInfo> itemInfoList;
			
			while(iterator.hasNext())
			{
				itemInfoList = iterator.next();
				
				itemInfoList.clear();
			}
			
			groupIdToItemInfoList.clear();
			
			clientSession.serviceGroupIdToItemInfoMap().remove(_longValue);
		}
		
		Iterator<ItemInfo> itemInfoIt = clientSession.itemInfoList().iterator();
		
		while(itemInfoIt.hasNext())
		{
			ItemInfo itemInfo = itemInfoIt.next();
			
			if( itemInfo.msgKey().checkHasServiceId() && itemInfo.serviceId() == serviceId )
			{
				if ( itemInfo.domainType() > 5 )
				{
					removeItemInfo(itemInfo, false);
				}
			}
		}
		
		_userLock.unlock();
	}
	
	void removeGroupId(ClientSession clientSession, int serviceId, Buffer groupId)
	{
		_userLock.lock();
		
		_longValue.value(serviceId);
		
		HashMap<Buffer, ArrayList<ItemInfo>> groupIdToItemInfoList = clientSession.serviceGroupIdToItemInfoMap().get(_longValue);
		
		if( groupIdToItemInfoList != null )
		{
			ArrayList<ItemInfo> itemInfoList = groupIdToItemInfoList.get(groupId);
			
			if( itemInfoList != null )
			{
				for(int index = 0; index < itemInfoList.size(); index++ )
				{
					removeItemInfo(itemInfoList.get(index), false);
				}
				
				itemInfoList.clear();
				
				groupIdToItemInfoList.remove(groupId);
			}
		}
		
		_userLock.unlock();
	}
	
	void mergeToGroupId(ClientSession clientSession, int serviceId, Buffer groupId, Buffer newGroupId)
	{
		_userLock.lock();
		
		if ( groupId.equals(newGroupId) )
		{
			return;
		}

		_longValue.value(serviceId);
		
		HashMap<Buffer, ArrayList<ItemInfo>> groupIdToItemInfoList = clientSession.serviceGroupIdToItemInfoMap().get(_longValue);
		
		if( groupIdToItemInfoList != null )
		{
			ArrayList<ItemInfo> oldItemInfoList = groupIdToItemInfoList.get(groupId);
			
			if( oldItemInfoList != null )
			{
				ArrayList<ItemInfo> mergeItemInfoList = groupIdToItemInfoList.get(newGroupId);
				
				if( mergeItemInfoList != null)
				{
					for(int index = 0; index < oldItemInfoList.size(); index++)
					{
						mergeItemInfoList.add(oldItemInfoList.get(index));
					}
					
					oldItemInfoList.clear();
					
					groupIdToItemInfoList.remove(groupId);
				}
				else
				{
					mergeItemInfoList = new ArrayList<ItemInfo>(oldItemInfoList.size() + 100);
					
					for(int index = 0; index < oldItemInfoList.size(); index++)
					{
						mergeItemInfoList.add(oldItemInfoList.get(index));
					}
					
					oldItemInfoList.clear();
					
					groupIdToItemInfoList.remove(groupId);
					
					groupIdToItemInfoList.put(newGroupId, mergeItemInfoList);
				}
			}
		}
		
		_userLock.unlock();
	}
	
	@Override
	public void run()
	{
		while (_threadRunning)
			rsslReactorDispatchLoop(_activeServerConfig.dispatchTimeoutApiThread, _activeServerConfig.maxDispatchCountApiThread);
	}

	@Override
	public void handleTimeoutEvent() 
	{
		_eventTimeout = true;	
	}
	
	public long dispatch()
	{
		if (_activeServerConfig.userDispatch == OperationModel.USER_DISPATCH)
		{
			_dispatchLock.lock();
			
			if (rsslReactorDispatchLoop(DispatchTimeout.NO_WAIT, _activeServerConfig.maxDispatchCountUserThread))
			{
				_dispatchLock.unlock();
				return DispatchReturn.DISPATCHED;
			}
			else
			{
				_dispatchLock.unlock();
				return DispatchReturn.TIMEOUT;
			}
		}
		
		return DispatchReturn.TIMEOUT;
	}

	public long dispatch(long timeOut)
	{
		if (_activeServerConfig.userDispatch == OperationModel.USER_DISPATCH)
		{
			_dispatchLock.lock();
			
			if (rsslReactorDispatchLoop(timeOut, _activeServerConfig.maxDispatchCountUserThread))
			{
				_dispatchLock.unlock();
				return DispatchReturn.DISPATCHED;
			}
			else
			{
				_dispatchLock.unlock();
				return DispatchReturn.TIMEOUT;
			}
		}
		
		return DispatchReturn.TIMEOUT;
	}
	
	@Override
	public void eventReceived()
	{
		_eventReceived = true;
	}
	
	public DictionaryHandler dictionaryHandler()
	{
			return _dictionaryHandler;
	}

	public  DirectoryHandler directoryHandler()
	{
		return _directoryHandler;
	}
	
	public LoginHandler loginHandler()
	{
		return _loginHandler;
	}
	
	public MarketItemHandler marketItemHandler()
	{
		return _marketItemHandler;
	}
	
	public ServerChannelHandler serverChannelHandler()
	{
		return _serverChannelHandler;
	}
	
	@SuppressWarnings("unchecked")
	<T> ItemCallbackClient<T> itemCallbackClient()
	{
		return (ItemCallbackClient<T>) _itemCallbackClient;
	}
	
	public ReqMsgImpl reqMsg()
	{
		if (_reqMsgImpl == null)
			_reqMsgImpl = new ReqMsgImpl(_objManager);
		else
			_reqMsgImpl.clear();
	
		return _reqMsgImpl;
	}
	
	public RefreshMsgImpl refreshMsg()
	{
		if (_refreshMsgImpl == null)
			_refreshMsgImpl = new RefreshMsgImpl(_objManager);
		else
			_refreshMsgImpl.clear();
	
		return _refreshMsgImpl;
	}
	
	public StatusMsgImpl statusMsg()
	{
		if (_statusMsgImpl == null)
			_statusMsgImpl = new StatusMsgImpl(_objManager);
		else
			_statusMsgImpl.clear();
		
		return _statusMsgImpl;
	}

	public GenericMsgImpl genericMsg()
	{
		if (_genericMsgImpl == null)
			_genericMsgImpl = new GenericMsgImpl(_objManager);
		else
			_genericMsgImpl.clear();
		
		return _genericMsgImpl;
	}

	public PostMsgImpl postMsg()
	{
		if (_postMsgImpl == null)
			_postMsgImpl = new PostMsgImpl(_objManager);
		else
			_postMsgImpl.clear();

		return _postMsgImpl;
	}
	
	public RequestMsg rsslRequestMsg()
	{
		if (_rsslReqMsg == null)
			_rsslReqMsg = (RequestMsg)CodecFactory.createMsg();
		else
			_rsslReqMsg.clear();
		
		_rsslReqMsg.msgClass(MsgClasses.REQUEST);
		return _rsslReqMsg;
	}

	public ReactorSubmitOptions rsslSubmitOptions() {
		return _rsslSubmitOptions;
	}

	public ReactorErrorInfo rsslErrorInfo() {
		return _rsslErrorInfo;
	}

	//Find default dictionary
	private DataDictionary getDefaultDataDictionary() {
		DataDictionary dataDictionary = null;
		if (_activeServerConfig.getServiceDictionaryConfigCollection() != null) {
			for (ServiceDictionaryConfig serviceDictionaryConfig : _activeServerConfig.getServiceDictionaryConfigCollection()) {
				int serviceId = serviceDictionaryConfig.serviceId;
				dataDictionary = dictionaryHandler().getDictionaryByServiceId(serviceId);
				if (dataDictionary != null) {
					break;
				}
			}
		}
		return dataDictionary;
	}
}
