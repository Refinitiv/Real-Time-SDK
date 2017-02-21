///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

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
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantLock;

import org.slf4j.Logger;

import com.thomsonreuters.ema.access.ConfigManager.ConfigAttributes;
import com.thomsonreuters.ema.access.ConfigManager.ConfigElement;
import com.thomsonreuters.ema.access.OmmProvider.DispatchReturn;
import com.thomsonreuters.ema.access.OmmProvider.DispatchTimeout;
import com.thomsonreuters.ema.access.OmmIProviderConfig.OperationModel;
import com.thomsonreuters.ema.access.OmmException.ExceptionType;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.transport.BindOptions;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.Server;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.WriteFlags;
import com.thomsonreuters.upa.transport.WritePriorities;
import com.thomsonreuters.upa.valueadd.common.VaIteratableQueue;
import com.thomsonreuters.upa.valueadd.reactor.ProviderRole;
import com.thomsonreuters.upa.valueadd.reactor.Reactor;
import com.thomsonreuters.upa.valueadd.reactor.ReactorAcceptOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorDispatchOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;

abstract class OmmServerBaseImpl implements OmmCommonImpl, Runnable, TimeoutClient
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
	
	private Logger _loggerClient;
	protected StringBuilder _strBuilder = new StringBuilder();
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private OmmInvalidHandleExceptionImpl _ommIHExcept;
	private LongObject	_longValue = new LongObject();
	
	private HashMap<LongObject, ItemInfo>	_itemInfoMap;
	
	private ReentrantLock _userLock = new java.util.concurrent.locks.ReentrantLock();
	private Reactor _rsslReactor;
	protected ReactorOptions _rsslReactorOpts = ReactorFactory.createReactorOptions();
	protected ReactorErrorInfo _rsslErrorInfo = ReactorFactory.createReactorErrorInfo();
	private ReactorDispatchOptions _rsslDispatchOptions = ReactorFactory.createReactorDispatchOptions();

	private Selector _selector;
	private ExecutorService _executor;
	private volatile boolean _threadRunning = false;
	private Pipe _pipe;
	private int _pipeWriteCount = 0;
	private byte[] _pipeWriteByte = new byte[]{0x00};
	private ByteBuffer _pipeReadByte = ByteBuffer.allocate(1);
	private boolean _eventReceived;
	private OmmProviderErrorClient _ommProviderErrorClient;
	private OmmProviderClient _ommProviderClient;
	private OmmEventImpl<OmmProviderEvent> _ommProviderEvent;
	private Object _closure;
	
	private Server _server;
	private BindOptions _bindOptions = TransportFactory.createBindOptions();
	private com.thomsonreuters.upa.transport.Error _transportError = TransportFactory.createError();
	private ReactorAcceptOptions reactorAcceptOptions = ReactorFactory.createReactorAcceptOptions();
	private ProviderRole _providerRole = ReactorFactory.createProviderRole();
	
	private ReqMsgImpl                  _reqMsgImpl;
	private RefreshMsgImpl				_refreshMsgImpl;
	private StatusMsgImpl				_statusMsgImpl;
	private GenericMsgImpl				_genericMsgImpl;
	
	private RequestMsg 					_rsslReqMsg;
	
	protected int _state = OmmImplState.NOT_INITIALIZED;
	
	protected boolean _eventTimeout;
	protected VaIteratableQueue _timeoutEventQueue = new VaIteratableQueue();
	protected EmaObjectManager _objManager = new EmaObjectManager();
	
	protected ReactorSubmitOptions _rsslSubmitOptions = ReactorFactory.createReactorSubmitOptions();
	
	protected ActiveServerConfig _activeServerConfig;
	
	protected DictionaryHandler _dictionaryHandler;
	protected DirectoryHandler _directoryHandler;
	protected LoginHandler _loginHandler;
	protected MarketItemHandler _marketItemHandler;
	protected ServerChannelHandler _serverChannelHandler;
	
	abstract String instanceName();
	
	abstract OmmProvider provider();
	
	abstract Logger createLoggerClient();
	
	abstract ConfigAttributes getAttributes(EmaConfigServerImpl config);
	
	abstract Object getAttributeValue(EmaConfigServerImpl config, int AttributeKey);
	
	abstract void readCustomConfig(EmaConfigServerImpl config);
	
	abstract DirectoryServiceStore directoryServiceStore();
	
	OmmServerBaseImpl(OmmProviderClient ommProviderClient, Object closure)
	{
		_itemInfoMap = new HashMap<>();
		_ommProviderClient = ommProviderClient;
		_closure = closure;
		_ommProviderEvent = new OmmEventImpl<OmmProviderEvent>();
	}

	OmmServerBaseImpl(OmmProviderClient ommProviderClient, OmmProviderErrorClient providerErrorClient, Object closure)
	{
		_itemInfoMap = new HashMap<>();
		_ommProviderClient = ommProviderClient;
		_ommProviderErrorClient = providerErrorClient;
		_closure = closure;
		_ommProviderEvent = new OmmEventImpl<OmmProviderEvent>();
	}
	
	void initialize(ActiveServerConfig activeConfig,EmaConfigServerImpl config)
	{
		_activeServerConfig = activeConfig;
		
		try
		{
			_objManager.initialize();
			
			GlobalPool.lock();
			GlobalPool.initialize();
			ServerPool.initialize(this,10, _activeServerConfig.itemCountHint);
			GlobalPool.unlock();
			
			_userLock.lock();
			
			_loggerClient = createLoggerClient();

			readConfiguration(config);
			
			readCustomConfig(config);
			
			config.errorTracker().log(this, _loggerClient);
			
			try
			{
				_pipe = Pipe.open();
				_selector = Selector.open();

			} 
			catch (Exception e)
			{
				_threadRunning = false;

				strBuilder().append("Failed to open Selector: ").append(e.getLocalizedMessage());
				String temp = _strBuilder.toString();
				
				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp));
			}

			if (_loggerClient.isTraceEnabled())
				_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName, "Successfully open Selector.", Severity.TRACE));

			if (_activeServerConfig.xmlTraceEnable)
				_rsslReactorOpts.enableXmlTracing();

			_rsslReactorOpts.userSpecObj(this);

			_rsslReactor = ReactorFactory.createReactor(_rsslReactorOpts, _rsslErrorInfo);
			if (ReactorReturnCodes.SUCCESS != _rsslErrorInfo.code())
			{
				_threadRunning = false;

				strBuilder().append("Failed to initialize OmmServerBaseImpl (ReactorFactory.createReactor).")
						.append("' Error Id='").append(_rsslErrorInfo.error().errorId()).append("' Internal sysError='")
						.append(_rsslErrorInfo.error().sysError()).append("' Error Location='")
						.append(_rsslErrorInfo.location()).append("' Error Text='")
						.append(_rsslErrorInfo.error().text()).append("'. ");
				
				String temp = _strBuilder.toString();
				
				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp));
				
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
			
			_providerRole.channelEventCallback(_serverChannelHandler);
			_providerRole.loginMsgCallback(_loginHandler);
			_providerRole.directoryMsgCallback(_directoryHandler);
	        _providerRole.dictionaryMsgCallback(_dictionaryHandler);
	        _providerRole.defaultMsgCallback(_marketItemHandler);
	        
			_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);
			if (_activeServerConfig.serverConfig.rsslConnectionType == ConnectionTypes.SOCKET && ((SocketServerConfig) _activeServerConfig.serverConfig).directWrite )
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
			
			String productVersion =  Package.getPackage("com.thomsonreuters.ema.access").getImplementationVersion();
	        if ( productVersion == null)
	        	productVersion = "EMA Java Edition";
	        
	        _bindOptions.componentVersion(productVersion);
			
			_server = Transport.bind(_bindOptions, _transportError);
			if (_server == null)
		    {
				_threadRunning = false;

				strBuilder().append("Failed to initialize OmmServerBaseImpl (Transport.bind).")
					.append("' Error Id='").append(_transportError.errorId()).append("' Internal sysError='")
					.append(_transportError.sysError()).append("' Error Text='")
					.append(_transportError.text()).append("'. ");
					
				String temp = _strBuilder.toString();
					
				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp));
		    }
			
			if (_loggerClient.isTraceEnabled())
				_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName, "Provider bound on port = " + _server.portNumber() + ".", Severity.TRACE));
				
			try
			{
				_server.selectableChannel().register(_selector, SelectionKey.OP_ACCEPT, _server);
			}
			catch(ClosedChannelException closedChannelExcep)
			{
				_threadRunning = false;

				strBuilder().append("Failed to register selector: " + closedChannelExcep.getLocalizedMessage());
				String temp = _strBuilder.toString();

				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp));
			}
			
			try
			{
				_rsslReactor.reactorChannel().selectableChannel().register(_selector, SelectionKey.OP_READ,
						_rsslReactor.reactorChannel());
				_pipe.source().configureBlocking(false);
				_pipe.source().register(_selector, SelectionKey.OP_READ, _rsslReactor.reactorChannel());
			} catch (IOException e)
			{
				_threadRunning = false;

				strBuilder().append("Failed to register selector: " + e.getLocalizedMessage());
				String temp = _strBuilder.toString();

				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp));
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
	
	void uninitialize()
	{
		if (_state == OmmImplState.NOT_INITIALIZED)
			return;
		
		_state = OmmImplState.UNINITIALIZING;

		try
		{
			_userLock.lock();
			
			_threadRunning = false;
			
			pipeRead();
			
			if (_rsslReactor != null)
			{
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

				if (_selector != null)
				{
					_selector.close();
					_selector = null;
				}

				if (_executor != null)
				{
					_executor.shutdown();
					_executor.awaitTermination(SHUTDOWN_TIMEOUT_IN_SECONDS, TimeUnit.SECONDS);
					_executor = null;
				}
			}

			_rsslReactor = null;
			_state = OmmImplState.NOT_INITIALIZED;
		} catch (InterruptedException | IOException e)
		{
			strBuilder().append("OmmServerBaseImpl unintialize(), Exception occurred, exception=")
					.append(e.getLocalizedMessage());

			if (_loggerClient.isErrorEnabled())
				_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, _strBuilder.toString(), Severity.ERROR));

		} finally
		{
			_userLock.unlock();
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
		int maxInt = Integer.MAX_VALUE;

		if (attributes != null)
		{
			if ((ce = attributes.getPrimitiveValue(ConfigManager.ItemCountHint)) != null)
				_activeServerConfig.itemCountHint = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ServiceCountHint)) != null)
				_activeServerConfig.serviceCountHint = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.DispatchTimeoutApiThread)) != null)
				_activeServerConfig.dispatchTimeoutApiThread = ce.intValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.MaxDispatchCountApiThread)) != null)
				_activeServerConfig.maxDispatchCountApiThread = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.MaxDispatchCountUserThread)) != null)
				_activeServerConfig.maxDispatchCountUserThread = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();
				
			if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceToStdout)) != null)
			{
				_activeServerConfig.isSetCorrectConfigGroup = true;
				_activeServerConfig.xmlTraceEnable = ce.booleanValue();
			}
		}

		// .........................................................................
		// Channel
		//

		String serverName =  (String)config.xmlConfig().getIProviderAttributeValue(
				_activeServerConfig.configuredName, ConfigManager.IProviderServerName);
		
		if ( serverName != null)
			readServerConfig(config, serverName);
		else
		{
			SocketServerConfig socketServerConfig = new SocketServerConfig();
			if (socketServerConfig.rsslConnectionType == ConnectionTypes.SOCKET)
			{
				String tempService = config.getUserSpecifiedPort();
				if (tempService == null)
					socketServerConfig.serviceName = tempService;
			}
			_activeServerConfig.serverConfig = socketServerConfig;
		}
		
		_activeServerConfig.userDispatch = config.operationModel();
	}
	
	void readServerConfig(EmaConfigServerImpl configImpl, String serverName)
	{
		int maxInt = Integer.MAX_VALUE;

		ConfigAttributes attributes = null;
		ConfigElement ce = null;
		int serverType = ConnectionTypes.SOCKET;

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
		case ConnectionTypes.SOCKET:
		{
			SocketServerConfig socketServerConfig = new SocketServerConfig();
			_activeServerConfig.serverConfig = socketServerConfig;

			String tempService = configImpl.getUserSpecifiedPort();
			if (tempService == null)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ServerPort)) != null)
					socketServerConfig.serviceName = ce.asciiValue();
			}
			else
				socketServerConfig.serviceName = tempService;
			
			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ServerTcpNodelay)) != null)
				socketServerConfig.tcpNodelay = ce.booleanValue();

			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ServerDirectSocketWrite)) != null)
				socketServerConfig.directWrite = ce.booleanValue();
			
			break;
		}
		default:
		{
			configImpl.errorTracker().append("Not supported server type. Type = ")
					.append(ConnectionTypes.toString(serverType));
			throw ommIUExcept().message(configImpl.errorTracker().text());
		}
		}

		ServerConfig currentServerConfig = _activeServerConfig.serverConfig;
		currentServerConfig.name = serverName;

		if (attributes != null)
		{
			if((ce = attributes.getPrimitiveValue(ConfigManager.InterfaceName)) != null)
				currentServerConfig.interfaceName = ce.asciiValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ServerCompressionType)) != null)
				currentServerConfig.compressionType = ce.intValue() < 0 ? ActiveConfig.DEFAULT_COMPRESSION_TYPE : ce.intValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.GuaranteedOutputBuffers)) != null)
				currentServerConfig.guaranteedOutputBuffers = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_GUARANTEED_OUTPUT_BUFFERS : ce.intLongValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.NumInputBuffers)) != null)
				currentServerConfig.numInputBuffers = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_NUM_INPUT_BUFFERS : ce.intLongValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ServerCompressionThreshold)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentServerConfig.compressionThreshold = maxInt;
				else
					currentServerConfig.compressionThreshold = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_COMPRESSION_THRESHOLD : ce.intLongValue();
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.SysRecvBufSize)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentServerConfig.sysRecvBufSize = maxInt;
				else
					currentServerConfig.sysRecvBufSize = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_SYS_RECEIVE_BUFFER_SIZE : ce.intLongValue();
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.SysSendBufSize)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentServerConfig.sysSendBufSize = maxInt;
				else
					currentServerConfig.sysSendBufSize = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_SYS_SEND_BUFFER_SIZE : ce.intLongValue();
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.HighWaterMark)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentServerConfig.highWaterMark = maxInt;
				else
					currentServerConfig.highWaterMark = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_HIGH_WATER_MARK : ce.intLongValue();
			}
	
			/* The following code will be removed once the deprecated XmlTraceToStdout is removed. */
			if( (!_activeServerConfig.isSetCorrectConfigGroup && (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceToStdout)) != null))
			{
					configImpl.errorTracker().append("This ConfigValue is no longer configured on a per-server basis; configure it instead in the IProvider instance.")
					.create(Severity.WARNING);
					_activeServerConfig.xmlTraceEnable = ce.booleanValue();
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ConnectionPingTimeout)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentServerConfig.connectionPingTimeout = maxInt;
				else
					currentServerConfig.connectionPingTimeout = ce.intLongValue() < 0 ? ActiveServerConfig.DEFAULT_CONNECTION_PINGTIMEOUT : ce.intLongValue();
			}
            
            if( (ce = attributes.getPrimitiveValue(ConfigManager.ConnectionMinPingTimeout)) != null)
            {
                if ( ce.intLongValue()  > maxInt )
                    currentServerConfig.connectionMinPingTimeout = maxInt;
                else
                    currentServerConfig.connectionMinPingTimeout = ce.intLongValue() < 0 ? ActiveServerConfig.DEFAULT_CONNECTION_MINPINGTIMEOUT : ce.intLongValue();
            }
		}
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
	
	StringBuilder strBuilder()
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
			break;
		default:
			break;
		}
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
	
	public void handleInvalidUsage(String text)
	{
		if ( hasErrorClient() )
			_ommProviderErrorClient.onInvalidUsage(text);
		else
			throw (ommIUExcept().message(text.toString()));
		
	}

	public void handleInvalidHandle(long handle, String text)
	{	
		if ( hasErrorClient() )
			_ommProviderErrorClient.onInvalidHandle(handle, text);
		else
			throw (ommIHExcept().message(text, handle));
	}
	
	public Logger loggerClient()
	{
		return _loggerClient;
	}
	
	ReentrantLock userLock()
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
		if (++_pipeWriteCount == 1)
			_pipe.sink().write(ByteBuffer.wrap(_pipeWriteByte));
	}
	
	void pipeRead() throws IOException
	{
		if (--_pipeWriteCount == 0)
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
		_eventReceived = false;
		_rsslDispatchOptions.maxMessages(count);
		int ret = ReactorReturnCodes.SUCCESS;
		int loopCount = 0;
		long startTime = System.nanoTime();
		long endTime = 0;
		
		if (_selector == null)
			return false;

		timeOut = timeOut*1000;
		long userTimeout = TimeoutEvent.userTimeOutExist(_timeoutEventQueue);
		boolean userTimeoutExist = false;
		if (userTimeout >= 0)
		{
			if (timeOut > 0 && timeOut < userTimeout)
				userTimeoutExist = false;
			else
			{
				userTimeoutExist = true;
				timeOut = (userTimeout > 0 ? userTimeout : 1000000);
			}
		}

		try
		{
				do
				{
					_userLock.lock();
					ret = _rsslReactor != null  ? _rsslReactor.dispatchAll(null, _rsslDispatchOptions, _rsslErrorInfo) : ReactorReturnCodes.SUCCESS;
					_userLock.unlock();
				}
				while ( ret > ReactorReturnCodes.SUCCESS && !_eventReceived && ++loopCount < 15 );
				
				if (ret < ReactorReturnCodes.SUCCESS)//
				{
						if (_loggerClient.isErrorEnabled())
						{
							strBuilder().append("Call to rsslReactorDispatchLoop() failed. Internal sysError='")
										.append(_rsslErrorInfo.error().sysError()).append("' Error text='")
										.append(_rsslErrorInfo.error().text()).append("'. ");
	
							if (_loggerClient.isErrorEnabled())
								_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
						}
	
						return false;
				} 
			
				if ( _eventReceived || _rsslReactor == null )
					return true;
				
				endTime = System.nanoTime();
	
				if ( timeOut > 0 )
				{
					timeOut -= endTime - startTime;
					if ( timeOut <=0 )
					{
						if (userTimeoutExist)
							TimeoutEvent.execute(_timeoutEventQueue);
		
						return _eventReceived ? true : false;
					}
					else if ( timeOut < 1000000  )
							timeOut = 1000000;
				}

			do
			{
				startTime = endTime;
			
				int selectCount = _selector.select(timeOut/1000000);
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
								
								if (_rsslReactor.accept(_server, reactorAcceptOptions, _providerRole, _rsslErrorInfo) == ReactorReturnCodes.FAILURE)
		                        {
									strBuilder().append("Failed to initialize OmmServerBaseImpl (Reactor.accept).")
									.append("' Error Id='").append(_rsslErrorInfo.error().errorId()).append("' Internal sysError='")
									.append(_rsslErrorInfo.error().sysError()).append("' Error Location='")
									.append(_rsslErrorInfo.location()).append("' Error Text='")
									.append(_rsslErrorInfo.error().text()).append("'. ");
							
									String temp = _strBuilder.toString();
							
									if (_loggerClient.isErrorEnabled())
										_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, temp, Severity.ERROR));
									
									clientSession.returnToPool();
									
									return false;
		                        }
							}
							if (key.isReadable())
							{
									_userLock.lock();
									ret = ((ReactorChannel) key.attachment()).dispatch(_rsslDispatchOptions, _rsslErrorInfo);
									_userLock.unlock();
							}
						}
						 catch (CancelledKeyException e)
						{
							continue;
						}
					}
					
					if (_pipeWriteCount == 1)
						pipeRead();
					

						loopCount = 0;
						do
						{
							_userLock.lock();
							ret = _rsslReactor != null ? _rsslReactor.dispatchAll(null, _rsslDispatchOptions, _rsslErrorInfo) : ReactorReturnCodes.SUCCESS;
							_userLock.unlock();
						}
						while ( ret > ReactorReturnCodes.SUCCESS && !_eventReceived && ++loopCount < 15 );
						
						if (ret < ReactorReturnCodes.SUCCESS)
						{
								if (_loggerClient.isErrorEnabled())
								{
									strBuilder().append("Call to rsslReactorDispatchLoop() failed. Internal sysError='")
												.append(_rsslErrorInfo.error().sysError()).append("' Error text='")
												.append(_rsslErrorInfo.error().text()).append("'. ");
	
									if (_loggerClient.isErrorEnabled())
										_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
								}
	
								return false;
						} 
						
						if ( _eventReceived || _rsslReactor == null ) return true;
	
						TimeoutEvent.execute(_timeoutEventQueue);
						
						if ( _eventReceived ) 	return true;
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
					if (timeOut < 1000000) return false;
				}
				
				if (Thread.currentThread().isInterrupted())
				{
					_threadRunning = false;
	
					if (_loggerClient.isTraceEnabled())
					{
						strBuilder().append("Call to rsslReactorDispatchLoop() received thread interruption signal.");
	
						_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName, _strBuilder.toString(), Severity.TRACE));
					}
				}
			}
			while (true);
			
		} //end of Try		
		catch (CancelledKeyException e)
		{
			if (_loggerClient.isTraceEnabled())
			{
				strBuilder().append("Call to rsslReactorDispatchLoop() received cancelled key exception.");

				_loggerClient.trace(formatLogMessage(_activeServerConfig.instanceName, _strBuilder.toString(), Severity.TRACE));
			}

			return true;
		} 
		catch (IOException | ClosedSelectorException e)
		{
			if (_loggerClient.isErrorEnabled())
			{
				strBuilder().append("Call to rsslReactorDispatchLoop() failed. Received exception,")
						.append(" exception text= ").append(e.getLocalizedMessage()).append(". ");

				_loggerClient.error(formatLogMessage(_activeServerConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
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
			.append("Client handle ").append(itemInfo.clientSession().clientHandle().value()).append(OmmLoggerClient.CR);
			
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

		itemInfo.returnToPool();
		
		if ( loggerClient().isTraceEnabled( ))
		{
			StringBuilder temp = strBuilder();
			temp.append("Removed ItemInfo ").append(itemInfo.handle().value()).append(" from ItemInfoMap" ).append( OmmLoggerClient.CR )
			.append("Client handle ").append(itemInfo.clientSession().clientHandle().value()).append(OmmLoggerClient.CR);
			
			loggerClient().trace(formatLogMessage(instanceName(), temp.toString(), Severity.TRACE));
		}
		
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
			return rsslReactorDispatchLoop(DispatchTimeout.NO_WAIT, _activeServerConfig.maxDispatchCountUserThread)
					? DispatchReturn.DISPATCHED : DispatchReturn.TIMEOUT;

		return DispatchReturn.TIMEOUT;
	}

	public long dispatch(long timeOut)
	{
		if (_activeServerConfig.userDispatch == OperationModel.USER_DISPATCH)
			return rsslReactorDispatchLoop(timeOut, _activeServerConfig.maxDispatchCountUserThread)
					? DispatchReturn.DISPATCHED : DispatchReturn.TIMEOUT;

		return DispatchReturn.TIMEOUT;
	}
	
	void eventReceived()
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
	
	public RequestMsg rsslRequestMsg()
	{
		if (_rsslReqMsg == null)
			_rsslReqMsg = (RequestMsg)CodecFactory.createMsg();
		else
			_rsslReqMsg.clear();
		
		_rsslReqMsg.msgClass(MsgClasses.REQUEST);
		return _rsslReqMsg;
	}
	
}
