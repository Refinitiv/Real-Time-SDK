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
import java.nio.channels.Pipe;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantLock;

import org.slf4j.Logger;

import com.thomsonreuters.ema.access.ConfigManager.ConfigAttributes;
import com.thomsonreuters.ema.access.ConfigManager.ConfigElement;
import com.thomsonreuters.ema.access.OmmConsumer.DispatchReturn;
import com.thomsonreuters.ema.access.OmmConsumer.DispatchTimeout;
import com.thomsonreuters.ema.access.OmmConsumerConfig.OperationModel;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.valueadd.common.VaIteratableQueue;
import com.thomsonreuters.upa.valueadd.reactor.Reactor;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorDispatchOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;

abstract class OmmBaseImpl<T> implements Runnable, TimeoutClient
{
	private final static int SHUTDOWN_TIMEOUT_IN_SECONDS = 3;

	static class OmmImplState
	{
		final static int NOT_INITIALIZED = 0;
		final static int INITIALIZED = 1;
		final static int REACTOR_INITIALIZED = 2;
		final static int RSSLCHANNEL_DOWN = 3;
		final static int RSSLCHANNEL_UP = 4;
		final static int RSSLCHANNEL_UP_STREAM_NOT_OPEN = 5;
		final static int LOGIN_STREAM_OPEN_SUSPECT = 6;
		final static int LOGIN_STREAM_OPEN_OK = 7;
		final static int LOGIN_STREAM_CLOSED = 8;
		final static int DIRECTORY_STREAM_OPEN_SUSPECT = 9;
		final static int DIRECTORY_STREAM_OPEN_OK = 10;
	}
	
	private static int INSTANCE_ID = 0;
	
	protected int _state = OmmImplState.NOT_INITIALIZED;
	private Logger _loggerClient;
	protected StringBuilder _strBuilder = new StringBuilder();
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private OmmInvalidHandleExceptionImpl _ommIHExcept;
	private ActiveConfig _activeConfig;

	protected LoginCallbackClient<T> _loginCallbackClient;
	protected DictionaryCallbackClient<T>  _dictionaryCallbackClient;
	protected DirectoryCallbackClient<T>  _directoryCallbackClient;
	protected ItemCallbackClient<T>  _itemCallbackClient;
	protected ChannelCallbackClient<T> _channelCallbackClient;
	
	
	private ReentrantLock _userLock = new java.util.concurrent.locks.ReentrantLock();
	protected Reactor _rsslReactor;
	private ReactorOptions _rsslReactorOpts = ReactorFactory.createReactorOptions();
	protected ReactorErrorInfo _rsslErrorInfo = ReactorFactory.createReactorErrorInfo();
	private ReactorDispatchOptions _rsslDispatchOptions = ReactorFactory.createReactorDispatchOptions();
	private EncodeIterator _rsslEncIter = CodecFactory.createEncodeIterator();
	private DecodeIterator _rsslDecIter = CodecFactory.createDecodeIterator();
	protected ReactorSubmitOptions _rsslSubmitOptions = ReactorFactory.createReactorSubmitOptions();
	private Selector _selector;
	private ExecutorService _executor;
	private volatile boolean _threadRunning = false;
	protected boolean _eventTimeout;
	protected VaIteratableQueue _timeoutEventQueue = new VaIteratableQueue();
	protected EmaObjectManager _objManager = new EmaObjectManager();
	private Pipe _pipe;
	private int _pipeWriteCount = 0;
	private byte[] _pipeWriteByte = new byte[]{0x00};
	private ByteBuffer _pipeReadByte = ByteBuffer.allocate(1);
	private boolean _eventReceived;
	
	abstract Logger createLoggerClient();
	
	abstract void readCustomConfig(EmaConfigImpl config);
	
	abstract boolean hasErrorClient();
	
	abstract void notifyErrorClient(OmmException ommException);
	
	abstract void handleInvalidUsage(String text);
	
	abstract void handleInvalidHandle(long handle, String text);
	
	abstract ConfigAttributes getAttributes(EmaConfigImpl config);
	
	abstract Object getAttributeValue(EmaConfigImpl config, int AttributeKey);
	
	abstract void handleAdminDomains();
	
	OmmBaseImpl()
	{
	}
	
	void initialize(ActiveConfig activeConfig,EmaConfigImpl config)
	{
		_activeConfig = activeConfig;
		
		try
		{
			_objManager.initialize();
			
			GlobalPool.lock();
			GlobalPool.initialize();
			GlobalPool.unlock();
			
			_userLock.lock();
			
			_loggerClient = createLoggerClient();

			readConfiguration(config);
			
			readCustomConfig( config );

			config.errorTracker().log(this, _loggerClient);

			try
			{
				_pipe = Pipe.open();
				_selector = Selector.open();

			} catch (Exception e)
			{
				_threadRunning = false;

				strBuilder().append("Failed to open Selector: ").append(e.getLocalizedMessage());
				String temp = _strBuilder.toString();
				
				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp));
			}

			if (_loggerClient.isTraceEnabled())
				_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, "Successfully open Selector.", Severity.TRACE));
			
			int channelSetSize = _activeConfig.channelConfigSet.size();
			if (_activeConfig.channelConfigSet.get( channelSetSize - 1 ).xmlTraceEnable)
				_rsslReactorOpts.enableXmlTracing();

			_rsslReactorOpts.userSpecObj(this);

			_rsslReactor = ReactorFactory.createReactor(_rsslReactorOpts, _rsslErrorInfo);
			if (ReactorReturnCodes.SUCCESS != _rsslErrorInfo.code())
			{
				_threadRunning = false;

				strBuilder().append("Failed to initialize OmmBaseImpl (ReactorFactory.createReactor).")
						.append("' Error Id='").append(_rsslErrorInfo.error().errorId()).append("' Internal sysError='")
						.append(_rsslErrorInfo.error().sysError()).append("' Error Location='")
						.append(_rsslErrorInfo.location()).append("' Error Text='")
						.append(_rsslErrorInfo.error().text()).append("'. ");
				
				String temp = _strBuilder.toString();
				
				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp));
			} else
			{
				if (_loggerClient.isTraceEnabled())
					_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, "Successfully created Reactor.", Severity.TRACE));
			}

			_state = OmmImplState.REACTOR_INITIALIZED;

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
					_loggerClient.error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp));
			}

			handleAdminDomains();

			if (_activeConfig.userDispatch == OperationModel.API_DISPATCH)
			{
				_threadRunning = true;

				if (_executor == null)
					_executor = Executors.newSingleThreadExecutor();

				_executor.execute(this);
			}
		} catch (OmmException exception)
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

		try
		{
			_userLock.lock();
			
			_threadRunning = false;
			
			pipeRead();
			
			if (_rsslReactor != null)
			{
				if (_loginCallbackClient != null)
					rsslReactorDispatchLoop(10000, _loginCallbackClient.sendLoginClose());

				if (_channelCallbackClient != null)
					_channelCallbackClient.closeChannels();

				if (ReactorReturnCodes.SUCCESS != _rsslReactor.shutdown(_rsslErrorInfo))
				{
					if (_loggerClient.isErrorEnabled())
					{
						strBuilder().append("Failed to uninitialize OmmBaseImpl (Reactor.shutdown).")
								.append("Error Id ").append(_rsslErrorInfo.error().errorId())
								.append("Internal sysError ").append(_rsslErrorInfo.error().sysError())
								.append("Error Location ").append(_rsslErrorInfo.location()).append("Error Text ")
								.append(_rsslErrorInfo.error().text()).append("'. ");

						_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
					}
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
			strBuilder().append("OmmBaseImpl unintialize(), Exception occurred, exception=")
					.append(e.getLocalizedMessage());

			if (_loggerClient.isErrorEnabled())
				_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));

		} finally
		{
			_userLock.unlock();
		}
	}
	
	protected long registerClient(ReqMsg reqMsg, T client, Object closure, long parentHandle)
	{
		try
		{
			_userLock.lock();
			long handle = _itemCallbackClient != null ? _itemCallbackClient.registerClient(reqMsg, client, closure, parentHandle) : 0;
			return handle;
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	protected long registerClient(TunnelStreamRequest tunnelStreamRequest, T client, Object closure)
	{
		try
		{
			_userLock.lock();
			long handle = _itemCallbackClient != null ? _itemCallbackClient.registerClient(tunnelStreamRequest, client, closure) : 0;
			return handle;
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	protected long registerClient(ReqMsg reqMsg, T client)
	{
		try
		{
			_userLock.lock();
			long handle = _itemCallbackClient != null ? _itemCallbackClient.registerClient(reqMsg, client, null, 0) : 0;
			return handle;
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	protected long registerClient(ReqMsg reqMsg, T client, Object closure)
	{
		try
		{
			_userLock.lock();
			long handle = _itemCallbackClient != null ? _itemCallbackClient.registerClient(reqMsg, client, closure, 0) : 0;
			return handle;
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	protected long registerClient(TunnelStreamRequest tunnelStreamRequest, T client)
	{
		try
		{
			_userLock.lock();
			long handle = _itemCallbackClient != null ? _itemCallbackClient.registerClient(tunnelStreamRequest, client, null) : 0;
			return handle;
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	protected void reissue(ReqMsg reqMsg, long handle)
	{
		try
		{
			_userLock.lock();
			if (_itemCallbackClient != null)
				_itemCallbackClient.reissue(reqMsg, handle);
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	protected void unregister(long handle)
	{
		try
		{
			_userLock.lock();
			if (_itemCallbackClient != null)
				_itemCallbackClient.unregister(handle);
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	protected void submit(GenericMsg genericMsg, long handle)
	{
		try
		{
			_userLock.lock();
			if (_itemCallbackClient != null)
				_itemCallbackClient.submit(genericMsg, handle);
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	protected void submit(PostMsg postMsg, long handle)
	{
		try
		{
			_userLock.lock();
			if (_itemCallbackClient != null)
				_itemCallbackClient.submit(postMsg, handle);
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	protected void submit(RefreshMsg refreshMsg, long handle)
	{
		try
		{
			_userLock.lock();
			if (_itemCallbackClient != null)
				_itemCallbackClient.submit(refreshMsg, handle);
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	protected void submit(UpdateMsg updateMsg, long handle)
	{
		try
		{
			_userLock.lock();
			if (_itemCallbackClient != null)
				_itemCallbackClient.submit(updateMsg, handle);
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
	protected void submit(StatusMsg statusMsg, long handle)
	{
		try
		{
			_userLock.lock();
			if (_itemCallbackClient != null)
				_itemCallbackClient.submit(statusMsg, handle);
		}
		finally
		{
			_userLock.unlock();
		}
	}
	
    public long dispatch(long timeOut)
	{
		if (_activeConfig.userDispatch == OperationModel.USER_DISPATCH)
			return rsslReactorDispatchLoop(timeOut, _activeConfig.maxDispatchCountUserThread)
					? DispatchReturn.DISPATCHED : DispatchReturn.TIMEOUT;

		return DispatchReturn.TIMEOUT;
	}
	
	public long dispatch()
	{
		if (_activeConfig.userDispatch == OperationModel.USER_DISPATCH)
			return rsslReactorDispatchLoop(DispatchTimeout.NO_WAIT, _activeConfig.maxDispatchCountUserThread)
					? DispatchReturn.DISPATCHED : DispatchReturn.TIMEOUT;

		return DispatchReturn.TIMEOUT;
	}
	
	void ommImplState(int state)
	{
		_state = state;
	}
	
	int ommImplState()
	{
		return _state;
	}

	VaIteratableQueue timerEventQueue()
	{
		return _timeoutEventQueue;
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

				_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
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
	
	void readConfiguration(EmaConfigImpl config)
	{
		_activeConfig.configuredName = config.configuredName();

		_activeConfig.instanceName = strBuilder().append(_activeConfig.configuredName).append("_").append(Integer.toString(++INSTANCE_ID)).toString();

		ConfigAttributes attributes = getAttributes(config);

		ConfigElement ce = null;
		int maxInt = Integer.MAX_VALUE;

		if (attributes != null)
		{
			if ((ce = attributes.getPrimitiveValue(ConfigManager.ItemCountHint)) != null)
				_activeConfig.itemCountHint = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ServiceCountHint)) != null)
				_activeConfig.serviceCountHint = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.LoginRequestTimeOut)) != null)
				_activeConfig.loginRequestTimeOut = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.DictionaryRequestTimeOut)) != null)
				_activeConfig.dictionaryRequestTimeOut = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.DispatchTimeoutApiThread)) != null)
				_activeConfig.dispatchTimeoutApiThread = ce.intValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.MaxDispatchCountApiThread)) != null)
				_activeConfig.maxDispatchCountApiThread = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.MaxDispatchCountUserThread)) != null)
				_activeConfig.maxDispatchCountUserThread = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();
		}

		// .........................................................................
		// Channel
		//

		String channelName = config.channelName(_activeConfig.configuredName);
		if (channelName != null  && channelName.trim().length() > 0)
		{
			String[] pieces = channelName.split(",");

			for (int i = 0; i < pieces.length; i++)
			{
				readChannelConfig(config,  pieces[i]);
			}
		}
		else
		{
			SocketChannelConfig socketChannelConfig = new SocketChannelConfig();
			if (socketChannelConfig.rsslConnectionType == ConnectionTypes.SOCKET)
			{
				String tempHost = config.getUserSpecifiedHostname();
				if (tempHost == null)
				{
					if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
						socketChannelConfig.hostName = ce.asciiValue();
				}
				else
					socketChannelConfig.hostName = tempHost;

				String tempService = config.getUserSpecifiedPort();
				if (tempService == null)
				{
					if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
						socketChannelConfig.serviceName = ce.asciiValue();
				}
				else
					socketChannelConfig.serviceName = tempService;
			}
			socketChannelConfig.name = "Channel";
			_activeConfig.channelConfigSet.add( socketChannelConfig);
		}

		//
		// Channel
		// .........................................................................

		// TODO: Handle programmatic configuration

		_activeConfig.userDispatch = config.operationModel();
		_activeConfig.rsslRDMLoginRequest = config.loginReq();
	}
	
	void readChannelConfig(EmaConfigImpl configImpl, String channelName)
	{
		int maxInt = Integer.MAX_VALUE;		
		ChannelConfig currentChannelConfig = null;

		ConfigAttributes attributes = null;
		ConfigElement ce = null;
		int connectionType = ConnectionTypes.SOCKET;

		attributes = configImpl.xmlConfig().getChannelAttributes(channelName);
		if (attributes != null) 
			ce = attributes.getPrimitiveValue(ConfigManager.ChannelType);

		if (ce == null || configImpl.getUserSpecifiedHostname() != null)
		{
				connectionType = ConnectionTypes.SOCKET;
		} 
		else
		{
			connectionType = ce.intValue();
		}

		switch (connectionType)
		{
		case ConnectionTypes.SOCKET:
		{
			SocketChannelConfig socketChannelConfig = new SocketChannelConfig();
			
			String tempHost = configImpl.getUserSpecifiedHostname();
			if (tempHost == null)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
					socketChannelConfig.hostName = ce.asciiValue();
			}
			else
				socketChannelConfig.hostName = tempHost;

			String tempService = configImpl.getUserSpecifiedPort();
			if (tempService == null)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
					socketChannelConfig.serviceName = ce.asciiValue();
			}
			else
				socketChannelConfig.serviceName = tempService;
			
			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelTcpNodelay)) != null)
				socketChannelConfig.tcpNodelay = ce.booleanValue();

			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelDirectSocketWrite)) != null)
				socketChannelConfig.directWrite = ce.booleanValue();
			
			currentChannelConfig = socketChannelConfig;
			
			break;
		}
		case ConnectionTypes.HTTP:
		{
			HttpChannelConfig httpChannelCfg = new HttpChannelConfig();
			
			String tempHost = configImpl.getUserSpecifiedHostname();
			if (tempHost == null)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
					httpChannelCfg.hostName = ce.asciiValue();
			}
			else
				httpChannelCfg.hostName = tempHost;

			String tempService = configImpl.getUserSpecifiedPort();
			if (tempService == null)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
					httpChannelCfg.serviceName = ce.asciiValue();
			}
			else
				httpChannelCfg.serviceName = tempService;

			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelTcpNodelay)) != null)
				httpChannelCfg.tcpNodelay = ce.booleanValue();

			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelObjectName)) != null)
				httpChannelCfg.objectName = ce.asciiValue();
			
			currentChannelConfig =  httpChannelCfg;
			
			break;
		}
		case ConnectionTypes.ENCRYPTED:
		{
			EncryptedChannelConfig encryptedChannelCfg = new EncryptedChannelConfig();

			String tempHost = configImpl.getUserSpecifiedHostname();
			if (tempHost == null)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
					encryptedChannelCfg.hostName = ce.asciiValue();
			}
			else
				encryptedChannelCfg.hostName = tempHost;

			String tempService = configImpl.getUserSpecifiedPort();
			if (tempService == null)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
					encryptedChannelCfg.serviceName = ce.asciiValue();
			}
			else
				encryptedChannelCfg.serviceName = tempService;

			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelTcpNodelay)) != null)
				encryptedChannelCfg.tcpNodelay = ce.booleanValue();

			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelObjectName)) != null)
				encryptedChannelCfg.objectName = ce.asciiValue();

			currentChannelConfig =  encryptedChannelCfg;
			
			break;
		}
		default:
		{
			configImpl.errorTracker().append("Not supported channel type. Type = ")
					.append(ConnectionTypes.toString(connectionType));
			throw ommIUExcept().message(configImpl.errorTracker().text());
		}
		}

		currentChannelConfig.name = channelName;

		if (attributes != null)
		{
			if((ce = attributes.getPrimitiveValue(ConfigManager.ChannelInterfaceName)) != null)
				currentChannelConfig.interfaceName = ce.asciiValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelCompressionType)) != null)
				currentChannelConfig.compressionType = ce.intValue() < 0 ? ActiveConfig.DEFAULT_COMPRESSION_TYPE : ce.intValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelGuaranteedOutputBuffers)) != null)
				currentChannelConfig.guaranteedOutputBuffers = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_GUARANTEED_OUTPUT_BUFFERS : ce.intLongValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelNumInputBuffers)) != null)
				currentChannelConfig.numInputBuffers = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_NUM_INPUT_BUFFERS : ce.intLongValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelCompressionThreshold)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.compressionThreshold = maxInt;
				else
					currentChannelConfig.compressionThreshold = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_COMPRESSION_THRESHOLD : ce.intLongValue();
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelConnectionPingTimeout)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.connectionPingTimeout = maxInt;
				else
					currentChannelConfig.connectionPingTimeout = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_CONNECTION_PINGTIMEOUT : ce.intLongValue();
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelSysRecvBufSize)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.sysRecvBufSize = maxInt;
				else
					currentChannelConfig.sysRecvBufSize = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_SYS_RECEIVE_BUFFER_SIZE : ce.intLongValue();
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelSysSendBufSize)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.sysSendBufSize = maxInt;
				else
					currentChannelConfig.sysSendBufSize = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_SYS_SEND_BUFFER_SIZE : ce.intLongValue();
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelHighWaterMark)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.highWaterMark = maxInt;
				else
					currentChannelConfig.highWaterMark = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_HIGH_WATER_MARK : ce.intLongValue();
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelReconnectAttemptLimit)) != null)
				currentChannelConfig.reconnectAttemptLimit = ce.intValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelReconnectMinDelay)) != null)
				currentChannelConfig.reconnectMinDelay = ce.intValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelReconnectMaxDelay)) != null)
				currentChannelConfig.reconnectMaxDelay = ce.intValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelMsgKeyInUpdates)) != null)
				currentChannelConfig.msgKeyInUpdates = ce.booleanValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelXmlTraceToStdout)) != null)
				currentChannelConfig.xmlTraceEnable = ce.booleanValue();
		}
		
		_activeConfig.channelConfigSet.add( currentChannelConfig );
	}
	
	StringBuilder strBuilder()
	{
		_strBuilder.setLength(0);
		return _strBuilder;
	}
	
	void handleLoginReqTimeout()
	{
		if (_activeConfig.loginRequestTimeOut == 0)
		{
			while (_state < OmmImplState.LOGIN_STREAM_OPEN_OK)
				rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
		}
		else
		{
			_eventTimeout = false;
			TimeoutEvent timeoutEvent = addTimeoutEvent(_activeConfig.loginRequestTimeOut * 1000, this);
	
			while (!_eventTimeout && (_state < OmmImplState.LOGIN_STREAM_OPEN_OK))
				rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
	
			if (_eventTimeout)
			{
				ChannelConfig currentConfig = _activeConfig.channelConfigSet.get( 0 );
				strBuilder().append("login failed (timed out after waiting ")
						.append(_activeConfig.loginRequestTimeOut).append(" milliseconds) for ");
				if (currentConfig.rsslConnectionType == ConnectionTypes.SOCKET)
				{
					SocketChannelConfig channelConfig = (SocketChannelConfig) currentConfig;
					_strBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
							.append(")");
				} else if (currentConfig.rsslConnectionType == ConnectionTypes.HTTP)
				{
					HttpChannelConfig channelConfig = ((HttpChannelConfig) currentConfig);
					_strBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
							.append(")");
				} else if (currentConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
				{
					EncryptedChannelConfig channelConfig = (EncryptedChannelConfig) currentConfig;
					_strBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
							.append(")");
				}
				
				String excepText = _strBuilder.toString();
	
				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw ommIUExcept().message(excepText);
			} else
				timeoutEvent.cancel();
		}
	}
	
	boolean rsslReactorDispatchLoop(long timeOut, int count)
	{
		_eventReceived = false;
		_rsslDispatchOptions.maxMessages(count);
		int ret = ReactorReturnCodes.SUCCESS;
		int loopCount = 0;
		
		if (_selector == null)
			return false;

		timeOut = timeOut*1000;
		long userTimeout = TimeoutEvent.userTimeOutExist(this);
		boolean userTimeoutExist = false;
		if (userTimeout >= 0)
		{
			if (timeOut > 0 && timeOut < userTimeout)
				userTimeoutExist = false;
			else
			{
				userTimeoutExist = true;
				timeOut = (userTimeout > 0 ? userTimeout : 1);
			}
		}

		long startTime = System.nanoTime();
		long endTime = 0;
		
		try
		{
			if (_state >= OmmImplState.RSSLCHANNEL_UP)
			{
				do
				{
					ret = _rsslReactor.reactorChannel()  != null  ? _rsslReactor.dispatchAll(null, _rsslDispatchOptions, _rsslErrorInfo) : ReactorReturnCodes.SUCCESS;
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
								_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
						}
	
						return false;
				} 
			
				if ( _eventReceived )
					return true;
				
				endTime = System.nanoTime();
	
				if ( ( timeOut > 0 ) && ( endTime - startTime >= timeOut ) )
				{
					if (userTimeoutExist)
						TimeoutEvent.execute(this);
	
					return _eventReceived ? true : false;
				}
	
				if ( timeOut >= 0 )
				{
					timeOut -= endTime - startTime;
					if ( timeOut < 0 )
						timeOut = 0;
				}
			}

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
						if (key.isReadable())
								ret = ((ReactorChannel) key.attachment()).dispatch(_rsslDispatchOptions, _rsslErrorInfo);
					}
					 catch (CancelledKeyException e)
					{
						continue;
					}
				}
				
				if (_pipeWriteCount == 1)
					pipeRead();
				
				if (_state >= OmmImplState.RSSLCHANNEL_UP)
				{
					loopCount = 0;
					do
					{
						ret = _rsslReactor.reactorChannel()  != null  ? _rsslReactor.dispatchAll(null, _rsslDispatchOptions, _rsslErrorInfo) : ReactorReturnCodes.SUCCESS;
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
									_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
							}

							return false;
					} 
					
					if ( _eventReceived ) return true;

					TimeoutEvent.execute(this);
					
					if ( _eventReceived ) 	return true;
					}
			} //selectCount > 0
			else if (selectCount == 0)
			{
				TimeoutEvent.execute(this);
					
				if ( _eventReceived ) return true;
			}
	
			if (Thread.currentThread().isInterrupted())
			{
				_threadRunning = false;

				if (_loggerClient.isTraceEnabled())
				{
					strBuilder().append("Call to rsslReactorDispatchLoop() received thread interruption signal.");

					_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.TRACE));
				}
			}
		} //end of Try		
		catch (CancelledKeyException e)
		{
			if (_loggerClient.isTraceEnabled())
			{
				strBuilder().append("Call to rsslReactorDispatchLoop() received cancelled key exception.");

				_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.TRACE));
			}

			return true;
		} 
		catch (IOException e)
		{
			if (_loggerClient.isErrorEnabled())
			{
				strBuilder().append("Call to rsslReactorDispatchLoop() failed. Received exception,")
						.append(" exception text= ").append(e.getLocalizedMessage()).append(". ");

				_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
			}

			uninitialize();

			return false;
		}
		
		return true;
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

	EncodeIterator rsslEncIter()
	{
		return _rsslEncIter;
	}

	DecodeIterator rsslDecIter()
	{
		return _rsslDecIter;
	}

	ReactorErrorInfo rsslErrorInfo()
	{
		return _rsslErrorInfo;
	}

	ReactorSubmitOptions rsslSubmitOptions()
	{
		return _rsslSubmitOptions;
	}
	
	abstract String formatLogMessage(String clientName, String temp, int level);
	
	abstract String instanceName();

	@Override
	public void run() 
	{
		while (_threadRunning)
			rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
	}
	
	@Override
	public void handleTimeoutEvent()
	{
		_eventTimeout = true;
	}
	
	ActiveConfig activeConfig()
	{
		return _activeConfig;
	}

	Logger loggerClient()
	{
		return _loggerClient;
	}
	
	ReentrantLock userLock()
	{
		return _userLock;
	}

	ItemCallbackClient<T> itemCallbackClient()
	{
		return _itemCallbackClient;
	}

	DictionaryCallbackClient<T> dictionaryCallbackClient()
	{
		return _dictionaryCallbackClient;
	}

	DirectoryCallbackClient<T> directoryCallbackClient()
	{
		return _directoryCallbackClient;
	}

	LoginCallbackClient<T> loginCallbackClient()
	{
		return _loginCallbackClient;
	}

	ChannelCallbackClient<T> channelCallbackClient()
	{
		return _channelCallbackClient;
	}

	Selector selector()
	{
		return _selector;
	}

	ReactorChannel rsslReactorChannel()
	{
		return _rsslReactor.reactorChannel();
	}

	void closeRsslChannel(ReactorChannel rsslReactorChannel)
	{
		if (rsslReactorChannel == null)
			return;

		_rsslErrorInfo.clear();
		if (rsslReactorChannel.close(_rsslErrorInfo) != ReactorReturnCodes.SUCCESS)
		{
			if (_loggerClient.isErrorEnabled())
			{
				_userLock.lock();

				strBuilder().append("Failed to close reactor channel (rsslReactorChannel).")
						.append("' RsslChannel='")
						.append(Integer.toHexString(_rsslErrorInfo.error().channel() != null ? _rsslErrorInfo.error().channel().hashCode() : 0))
						.append("Error Id ").append(_rsslErrorInfo.error().errorId())
						.append("Internal sysError ").append(_rsslErrorInfo.error().sysError())
						.append("Error Location ").append(_rsslErrorInfo.location())
						.append("Error Text ").append(_rsslErrorInfo.error().text()).append("'. ");

				_loggerClient.error(
						formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));

				_userLock.unlock();
			}
		}

		_channelCallbackClient.removeChannel( (ChannelInfo) rsslReactorChannel.userSpecObj());
	}
	
	void processChannelEvent( ReactorChannelEvent reactorChannelEvent){}
	
	void reLoadDirectory() {}
	
	void eventReceived()
	{
		_eventReceived = true;
	}
}
