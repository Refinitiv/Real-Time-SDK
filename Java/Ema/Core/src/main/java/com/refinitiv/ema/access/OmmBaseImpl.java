///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedSelectorException;
import java.nio.channels.Pipe;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.concurrent.ConcurrentLinkedQueue;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.atomic.AtomicLong;
import java.util.concurrent.locks.ReentrantLock;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.valueadd.reactor.*;
import org.slf4j.Logger;

import com.refinitiv.ema.access.ConfigManager.ConfigAttributes;
import com.refinitiv.ema.access.ConfigManager.ConfigElement;
import com.refinitiv.ema.access.OmmConsumer.DispatchReturn;
import com.refinitiv.ema.access.OmmConsumer.DispatchTimeout;
import com.refinitiv.ema.access.OmmConsumerConfig.OperationModel;
import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.ema.access.ProgrammaticConfigure.InstanceEntryFlag;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;

interface OmmCommonImpl
{
	static class ImplementationType
	{
		final static int CONSUMER = 0;
		final static int NIPROVIDER = 1;
		final static int IPROVIDER = 2;
	}
	
	void handleInvalidUsage(String text, int errorCode);
	
	void handleInvalidHandle(long handle, String text);

	void handleJsonConverterError(ReactorChannel reactorChannel, int errorCode, String text);

	Logger loggerClient();
	
	String formatLogMessage(String clientName, String temp, int level);
	
	EmaObjectManager objManager();
	
	String instanceName();
	
	StringBuilder strBuilder();
	
	void eventReceived();
	
	ReentrantLock userLock();
	
	int implType();
	
	long nextLongId();

	void channelInformation(ChannelInformation ci);
}

abstract class OmmBaseImpl<T> implements OmmCommonImpl, Runnable, TimeoutClient, ReactorServiceNameToIdCallback, ReactorJsonConversionEventCallback
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
	private final static int MIN_TIME_FOR_SELECT = 1000000;
	private final static int MIN_TIME_FOR_SELECT_IN_MILLISEC = 1;
	private final static int DISPATCH_LOOP_COUNT = 20;
	
	protected volatile int _state = OmmImplState.NOT_INITIALIZED;
	private boolean _logError = true;

	private Logger _loggerClient;
	protected StringBuilder _strBuilder = new StringBuilder(1024);
	protected StringBuilder _dispatchStrBuilder = new StringBuilder(1024);
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private OmmInvalidHandleExceptionImpl _ommIHExcept;
	private OmmJsonConverterExceptionImpl _ommJCExcept;
	private ActiveConfig _activeConfig;

	protected LoginCallbackClient<T> _loginCallbackClient;
	protected DictionaryCallbackClient<T>  _dictionaryCallbackClient;
	protected DirectoryCallbackClient<T>  _directoryCallbackClient;
	protected ItemCallbackClient<T>  _itemCallbackClient;
	protected ChannelCallbackClient<T> _channelCallbackClient;
	
	
	private ReentrantLock _userLock = new java.util.concurrent.locks.ReentrantLock();
	private ReentrantLock _dispatchLock = new java.util.concurrent.locks.ReentrantLock();	
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
	protected ConcurrentLinkedQueue<TimeoutEvent> _timeoutEventQueue = new ConcurrentLinkedQueue<TimeoutEvent>();
	protected EmaObjectManager _objManager = new EmaObjectManager();
	private Pipe _pipe;
	private AtomicLong _pipeWriteCount = new AtomicLong();
	private byte[] _pipeWriteByte = new byte[]{0x00};
	private ByteBuffer _pipeReadByte = ByteBuffer.allocate(1);
	private volatile boolean _eventReceived;
	private SelectionKey _pipeSelectKey;
	
	abstract Logger createLoggerClient();
	
	abstract void readCustomConfig(EmaConfigImpl config);
	
	abstract boolean hasErrorClient();
	
	abstract void notifyErrorClient(OmmException ommException);
	
	abstract ConfigAttributes getAttributes(EmaConfigImpl config);
	
	abstract Object getAttributeValue(EmaConfigImpl config, int AttributeKey);
	
	abstract void handleAdminDomains(EmaConfigImpl config);
	
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

			if (_loggerClient.isTraceEnabled())
			{
				_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, 
					"Print out active configuration detail." + activeConfig.configTrace().toString(), Severity.TRACE));
			}
			
			ReactorFactory.setReactorMsgEventPoolLimit(activeConfig.globalConfig.reactorMsgEventPoolLimit);
			ReactorFactory.setReactorChannelEventPoolLimit(activeConfig.globalConfig.reactorChannelEventPoolLimit);
			ReactorFactory.setWorkerEventPoolLimit(activeConfig.globalConfig.workerEventPoolLimit);
			ReactorFactory.setTunnelStreamMsgEventPoolLimit(activeConfig.globalConfig.tunnelStreamMsgEventPoolLimit);
			ReactorFactory.setTunnelStreamStatusEventPoolLimit(activeConfig.globalConfig.tunnelStreamStatusEventPoolLimit);
			
			try
			{
				_pipe = Pipe.open();
				_selector = Selector.open();

			} catch (Exception e)
			{
				strBuilder().append("Failed to open Selector: ").append(e.getLocalizedMessage());
				String temp = _strBuilder.toString();
				
				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR));
			}

			if (_loggerClient.isTraceEnabled())
				_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, "Successfully open Selector.", Severity.TRACE));
			
			if (_activeConfig.xmlTraceEnable)
				_rsslReactorOpts.enableXmlTracing();

			_rsslReactorOpts.userSpecObj(this);
			
			// Overrides the default service discovery URL if specified by user
			if(config.serviceDiscoveryUrl().length() != 0)
			{
				_rsslReactorOpts.serviceDiscoveryURL(config.serviceDiscoveryUrl());
			}
			
			// Overrides the default token service URL if specified by user
			if(config.tokenServiceUrl().length() != 0)
			{
				_rsslReactorOpts.tokenServiceURL(config.tokenServiceUrl());
			}
			
			/* Configuration parameters for handling token reissue and REST request timeout */
			_rsslReactorOpts.reissueTokenAttemptLimit(_activeConfig.reissueTokenAttemptLimit);
			_rsslReactorOpts.reissueTokenAttemptInterval(_activeConfig.reissueTokenAttemptInterval);
			_rsslReactorOpts.restRequestTimeout(_activeConfig.restRequestTimeout);
			_rsslReactorOpts.tokenReissueRatio(_activeConfig.tokenReissueRatio);

			_rsslReactor = ReactorFactory.createReactor(_rsslReactorOpts, _rsslErrorInfo);
			if (ReactorReturnCodes.SUCCESS != _rsslErrorInfo.code())
			{
				strBuilder().append("Failed to initialize OmmBaseImpl (ReactorFactory.createReactor).")
						.append("' Error Id='").append(_rsslErrorInfo.error().errorId()).append("' Internal sysError='")
						.append(_rsslErrorInfo.error().sysError()).append("' Error Location='")
						.append(_rsslErrorInfo.location()).append("' Error Text='")
						.append(_rsslErrorInfo.error().text()).append("'. ");
				
				String temp = _strBuilder.toString();
				
				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR));
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
				_pipeSelectKey = _pipe.source().register(_selector, SelectionKey.OP_READ, _rsslReactor.reactorChannel());

			} catch (IOException e)
			{
				strBuilder().append("Failed to register selector: " + e.getLocalizedMessage());
				String temp = _strBuilder.toString();

				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR));
			}

			handleAdminDomains(config);

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

	@Override
	public int reactorServiceNameToIdCallback(ReactorServiceNameToId serviceNameToId, ReactorServiceNameToIdEvent serviceNameToIdEvent) {
		Directory directory = directoryCallbackClient().directory(serviceNameToId.serviceName());
		if (directory != null) {
			serviceNameToId.serviceId(directory.service().serviceId());
			return ReactorReturnCodes.SUCCESS;
		}
		return ReactorReturnCodes.FAILURE;
	}

	@Override
	public int reactorJsonConversionEventCallback(ReactorJsonConversionEvent jsonConversionEvent) {
		if (jsonConversionEvent != null && jsonConversionEvent.error() != null) {
			handleJsonConverterError(jsonConversionEvent.reactorChannel(), jsonConversionEvent.error().errorId(), jsonConversionEvent.error().text());
		}
		return ReactorCallbackReturnCodes.SUCCESS;
	}

	//only for unit test, internal use
	void initializeForTest(ActiveConfig activeConfig,EmaConfigImpl config)
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
			
			if (_loggerClient.isTraceEnabled())
			{
				_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, 
					"Print out active configuration detail." + activeConfig.configTrace().toString(), Severity.TRACE));
			}
			
		} catch (OmmException exception)
		{
				throw exception;
		} finally
		{
			if (_userLock.isLocked())
				_userLock.unlock();
		}
	}
	
	void uninitialize()
	{
		try
		{
			if (_activeConfig.userDispatch == OperationModel.API_DISPATCH)
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
							_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
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
						
			if (_loginCallbackClient != null)
			_loginCallbackClient.sendLoginClose();

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

			_pipe.sink().close();
			_pipe.source().close();
			_selector.close();
				
		} catch (InterruptedException | IOException e)
		{
			strBuilder().append("OmmBaseImpl unintialize(), Exception occurred, exception=")
					.append(e.getLocalizedMessage());

			if (_loggerClient.isErrorEnabled())
				_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));

		} finally
		{
			_userLock.unlock();
			_rsslReactor = null;
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
		{
			_dispatchLock.lock();
			
			if (rsslReactorDispatchLoop(timeOut, _activeConfig.maxDispatchCountUserThread))
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
	
	public long dispatch()
	{
		if (_activeConfig.userDispatch == OperationModel.USER_DISPATCH)
		{
			_dispatchLock.lock();
			
			if (rsslReactorDispatchLoop(DispatchTimeout.NO_WAIT, _activeConfig.maxDispatchCountUserThread))
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
	
	void ommImplState(int state)
	{
		_state = state;
	}
	
	int ommImplState()
	{
		return _state;
	}

	ConcurrentLinkedQueue<TimeoutEvent> timerEventQueue()
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
	
	void readConfiguration(EmaConfigImpl config)
	{
		_activeConfig.configuredName = config.configuredName();

		_activeConfig.instanceName = strBuilder().append(_activeConfig.configuredName).append("_").append(Integer.toString(++INSTANCE_ID)).toString();

		ConfigAttributes attributes = getAttributes(config);

		ConfigElement ce = null;
		int value = 0;
		
		if (attributes != null)
		{
			if ((ce = attributes.getPrimitiveValue(ConfigManager.ItemCountHint)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeConfig.itemCountHint = value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ServiceCountHint)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeConfig.serviceCountHint = value;
			}
				
			if ((ce = attributes.getPrimitiveValue(ConfigManager.RequestTimeout)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeConfig.requestTimeout = value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.LoginRequestTimeOut)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeConfig.loginRequestTimeOut = value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.DictionaryRequestTimeOut)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeConfig.dictionaryRequestTimeOut = value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.DispatchTimeoutApiThread)) != null)
			{
				value = ce.intValue();
				if (value >= 0)
					_activeConfig.dispatchTimeoutApiThread = value;
			}
			
			if ((ce = attributes.getPrimitiveValue(ConfigManager.MaxDispatchCountApiThread)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeConfig.maxDispatchCountApiThread = value;
			}
			
			if ((ce = attributes.getPrimitiveValue(ConfigManager.MaxDispatchCountUserThread)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeConfig.maxDispatchCountUserThread = value;
			}
				
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ReconnectAttemptLimit)) != null)
			{
				_activeConfig.reconnectAttemptLimit(ce.intValue());
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ReconnectMinDelay)) != null)
			{
				_activeConfig.reconnectMinDelay(ce.intValue());
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ReconnectMaxDelay)) != null)
			{
				_activeConfig.reconnectMaxDelay(ce.intValue());
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.MsgKeyInUpdates)) != null)
			{
				_activeConfig.msgKeyInUpdates = ce.intLongValue() == 0 ? false : ActiveConfig.DEFAULT_MSGKEYINUPDATES;
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceToStdout)) != null)
			{
				_activeConfig.xmlTraceEnable = ce.intLongValue() == 1 ? true : ActiveConfig.DEFAULT_XML_TRACE_ENABLE;
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ReissueTokenAttemptLimit)) != null)
			{
				_activeConfig.reissueTokenAttemptLimit(ce.intValue());
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ReissueTokenAttemptInterval)) != null)
			{
				_activeConfig.reissueTokenAttemptInterval(ce.intValue());
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.RestRequestTimeout)) != null)
			{
				_activeConfig.restRequestTimeout(ce.intLongValue());
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.TokenReissueRatio)) != null)
			{
				double doubleValue = ce.doubleValue();
				
				if(doubleValue > 0)
					_activeConfig.tokenReissueRatio = doubleValue;
			}
			if( (ce = attributes.getPrimitiveValue(ConfigManager.EnableRtt)) != null)
			{
				long rttVal = ce.intLongValue();
				if (rttVal > 0) {
					config.loginReq().attrib().applyHasSupportRoundTripLatencyMonitoring();
				}
			}
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
				_activeConfig.channelConfigSet.add( readChannelConfig(config,  pieces[i].trim()));
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

		ConfigAttributes globalConfigAttributes = config.xmlConfig().getGlobalConfig();

		if(globalConfigAttributes != null){
			_activeConfig.globalConfig = new GlobalConfig();
			if( (ce = globalConfigAttributes.getPrimitiveValue(ConfigManager.WorkerEventPoolLimit)) != null)
			{
				_activeConfig.globalConfig.workerEventPoolLimit = ce.intValue();
			}
			if( (ce = globalConfigAttributes.getPrimitiveValue(ConfigManager.ReactorChannelEventPoolLimit)) != null)
			{
				_activeConfig.globalConfig.reactorChannelEventPoolLimit = ce.intValue();
			}

			if( (ce = globalConfigAttributes.getPrimitiveValue(ConfigManager.ReactorMsgEventPoolLimit)) != null)
			{
				_activeConfig.globalConfig.reactorMsgEventPoolLimit = ce.intValue();
			}

			if( (ce = globalConfigAttributes.getPrimitiveValue(ConfigManager.TunnelStreamMsgEventPoolLimit)) != null)
			{
				_activeConfig.globalConfig.tunnelStreamMsgEventPoolLimit = ce.intValue();
			}

			if( (ce = globalConfigAttributes.getPrimitiveValue(ConfigManager.TunnelStreamStatusEventPoolLimit)) != null)
			{
				_activeConfig.globalConfig.tunnelStreamStatusEventPoolLimit = ce.intValue();
			}
		}
		
		ProgrammaticConfigure pc = config.programmaticConfigure();
		if ( pc != null)
		{
			pc .retrieveCommonConfig( _activeConfig.configuredName, _activeConfig );

			GlobalConfig globalConfig = pc.retrieveGlobalConfig();
			if(globalConfig != null){
				_activeConfig.globalConfig = globalConfig;
			}
			
			String channelOrChannelSet = pc .activeEntryNames( _activeConfig.configuredName, InstanceEntryFlag.CHANNEL_FLAG );
			if (channelOrChannelSet == null)
				channelOrChannelSet = pc .activeEntryNames( _activeConfig.configuredName, InstanceEntryFlag.CHANNELSET_FLAG );

			if ( channelOrChannelSet != null && !channelOrChannelSet.isEmpty() )
			{
				_activeConfig.channelConfigSet.clear();
				String[] pieces = channelOrChannelSet.split(",");
				for (int i = 0; i < pieces.length; i++)
				{
					ChannelConfig fileChannelConfig = readChannelConfig( config,  pieces[i].trim());

					int chanConfigByFuncCall = 0;
					if (config.getUserSpecifiedHostname() != null && config.getUserSpecifiedHostname().length() > 0)
						chanConfigByFuncCall = ActiveConfig.SOCKET_CONN_HOST_CONFIG_BY_FUNCTION_CALL;
					else
					{
						HttpChannelConfig tunnelingConfig = config.tunnelingChannelCfg();
						if ( tunnelingConfig.httpProxyHostName != null && tunnelingConfig.httpProxyHostName.length() > 0 )
							chanConfigByFuncCall |= ActiveConfig.TUNNELING_PROXY_HOST_CONFIG_BY_FUNCTION_CALL;
						if (tunnelingConfig.httpProxyPort != null && tunnelingConfig.httpProxyPort.length() > 0 )
							chanConfigByFuncCall |= ActiveConfig.TUNNELING_PROXY_PORT_CONFIG_BY_FUNCTION_CALL;
						if (tunnelingConfig.objectName != null && tunnelingConfig.objectName.length() > 0)
							chanConfigByFuncCall |= ActiveConfig.TUNNELING_OBJNAME_CONFIG_BY_FUNCTION_CALL;
					}
					
					pc .retrieveChannelConfig( pieces[i].trim(), _activeConfig, chanConfigByFuncCall, fileChannelConfig );
					if ( _activeConfig.channelConfigSet.size() == i )
						_activeConfig.channelConfigSet.add( fileChannelConfig );
					else
						fileChannelConfig = null;
				}
			}
		}
		
		// Only assigns the default hostname and port for the encrypted connections when the session management feature is disable
		EncryptedChannelConfig encryptedChannelConfig;
		for(int i = 0; i < _activeConfig.channelConfigSet.size(); i++)
		{
			if(_activeConfig.channelConfigSet.get(i).rsslConnectionType ==  ConnectionTypes.ENCRYPTED && _activeConfig.channelConfigSet.get(i).encryptedProtocolType == ConnectionTypes.HTTP )
			{
				encryptedChannelConfig = (EncryptedChannelConfig)_activeConfig.channelConfigSet.get(i);
				
				if (encryptedChannelConfig.enableSessionMgnt == false)
				{
					if(encryptedChannelConfig.hostName == null || encryptedChannelConfig.hostName.isEmpty())
						encryptedChannelConfig.hostName = ActiveConfig.DEFAULT_HOST_NAME;
					
					if(encryptedChannelConfig.serviceName == null || encryptedChannelConfig.serviceName.isEmpty())
						encryptedChannelConfig.serviceName = ActiveConfig.defaultServiceName;
				}
			}
		}

		_activeConfig.userDispatch = config.operationModel();
		_activeConfig.rsslRDMLoginRequest = config.loginReq();
	}
	
	ChannelConfig readChannelConfig(EmaConfigImpl configImpl, String channelName)
	{
		int maxInt = Integer.MAX_VALUE;		
		ChannelConfig currentChannelConfig = null;

		ConfigAttributes attributes = null;
		ConfigElement ce = null;
		ConfigElement ep = null;
		int connectionType = -1;
		int encrypedProtocol = -1;

		attributes = configImpl.xmlConfig().getChannelAttributes(channelName);
		if (attributes != null) 
		{
			ce = attributes.getPrimitiveValue(ConfigManager.ChannelType);
			ep = attributes.getPrimitiveValue(ConfigManager.EncryptedProtocolType);
		}
		

		if (configImpl.getUserSpecifiedHostname() != null)
			connectionType = ConnectionTypes.SOCKET;
		else
		{
			ProgrammaticConfigure pc = configImpl.programmaticConfigure();
			if (pc != null)
				connectionType = pc.retrieveChannelTypeConfig(channelName);
		
			if (connectionType < 0)
				connectionType = (ce == null) ? ConnectionTypes.SOCKET : ce.intValue();
			
			if(connectionType == ConnectionTypes.ENCRYPTED)
			{
				if(pc != null)
					encrypedProtocol = pc.retrieveEncryptedProtocolConfig(channelName);
				
				if(encrypedProtocol < 0)
					encrypedProtocol = (ep == null) ? ConnectionTypes.HTTP : ep.intValue();
			}
		}
		
		
		
		switch (connectionType)
		{
		case ConnectionTypes.ENCRYPTED:
		{
			if(encrypedProtocol == ConnectionTypes.HTTP)
			{
				HttpChannelConfig tunnelingChannelCfg = new EncryptedChannelConfig();
				tunnelingChannelCfg.rsslConnectionType = ConnectionTypes.ENCRYPTED;
				tunnelingChannelCfg.encryptedProtocolType = ConnectionTypes.HTTP;
				
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelEnableSessionMgnt)) != null)
					((EncryptedChannelConfig)tunnelingChannelCfg).enableSessionMgnt = ce.intLongValue() == 0 ? false : true;
				
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelLocation)) != null)
					((EncryptedChannelConfig)tunnelingChannelCfg).location = ce.asciiValue();
				
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
					tunnelingChannelCfg.hostName = ce.asciiValue();

				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
					tunnelingChannelCfg.serviceName = ce.asciiValue();

				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelTcpNodelay)) != null)
					tunnelingChannelCfg.tcpNodelay = ce.intLongValue() == 0 ? false : ActiveConfig.DEFAULT_TCP_NODELAY;

				HttpChannelConfig programTunnelingChannelCfg = configImpl.tunnelingChannelCfg();
				tunnelingChannelCfg.objectName = programTunnelingChannelCfg.objectName;
				if (tunnelingChannelCfg.objectName == null || tunnelingChannelCfg.objectName.length() == 0)
				{
					if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelObjectName)) != null)
						tunnelingChannelCfg.objectName = ce.asciiValue();
				}
				
				tunnelingChannelCfg.httpProxyHostName = programTunnelingChannelCfg.httpProxyHostName;
				if ( tunnelingChannelCfg.httpProxyHostName == null || tunnelingChannelCfg.httpProxyHostName.length() == 0)
				{
					if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelProxyHost)) != null)
						tunnelingChannelCfg.httpProxyHostName = ce.asciiValue();
				}
				
				tunnelingChannelCfg.httpProxyPort = programTunnelingChannelCfg.httpProxyPort;
				if (tunnelingChannelCfg.httpProxyPort == null || tunnelingChannelCfg.httpProxyPort.length() == 0)
				{
					if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelProxyPort)) != null)
						tunnelingChannelCfg.httpProxyPort = ce.asciiValue();
				}
				
				if ( (tunnelingChannelCfg.httpProxyPort != null && tunnelingChannelCfg.httpProxyPort.length()  > 0) ||
				     (tunnelingChannelCfg.httpProxyHostName != null && tunnelingChannelCfg.httpProxyHostName.length() > 0))
					tunnelingChannelCfg.httpProxy = true;
				
				if (tunnelingChannelCfg.httpProxy)
				{
					tunnelingChannelCfg.httpproxyPasswd = programTunnelingChannelCfg.httpproxyPasswd;
					tunnelingChannelCfg.httpProxyDomain = programTunnelingChannelCfg.httpProxyDomain;
					tunnelingChannelCfg.httpProxyUserName = programTunnelingChannelCfg.httpProxyUserName;
					tunnelingChannelCfg.httpProxyKRB5ConfigFile = programTunnelingChannelCfg.httpProxyKRB5ConfigFile;
					tunnelingChannelCfg.httpProxyLocalHostName = programTunnelingChannelCfg.httpProxyLocalHostName;
				}
				
				((EncryptedChannelConfig) tunnelingChannelCfg).encryptionConfig.copy(configImpl.encryptionCfg());
				
				currentChannelConfig =  tunnelingChannelCfg;
			}
			else if(encrypedProtocol == ConnectionTypes.SOCKET || encrypedProtocol == ConnectionTypes.WEBSOCKET)
			{
				EncryptedChannelConfig encryptedChannelConfig = new EncryptedChannelConfig();
				encryptedChannelConfig.rsslConnectionType = ConnectionTypes.ENCRYPTED;
				encryptedChannelConfig.encryptedProtocolType = encrypedProtocol;
				
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelEnableSessionMgnt)) != null)
					encryptedChannelConfig.enableSessionMgnt = ce.intLongValue() == 0 ? false : true;
				
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelLocation)) != null)
					encryptedChannelConfig.location = ce.asciiValue();
				
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
					encryptedChannelConfig.hostName = ce.asciiValue();

				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
					encryptedChannelConfig.serviceName = ce.asciiValue();

				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelTcpNodelay)) != null)
					encryptedChannelConfig.tcpNodelay = ce.intLongValue() == 0 ? false : ActiveConfig.DEFAULT_TCP_NODELAY;
				
				encryptedChannelConfig.encryptionConfig.copy(configImpl.encryptionCfg());

				HttpChannelConfig programTunnelingChannelCfg = configImpl.tunnelingChannelCfg();
				
				encryptedChannelConfig.httpProxyHostName = programTunnelingChannelCfg.httpProxyHostName;
				if ( encryptedChannelConfig.httpProxyHostName == null || encryptedChannelConfig.httpProxyHostName.length() == 0)
				{
					if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelProxyHost)) != null)
						encryptedChannelConfig.httpProxyHostName = ce.asciiValue();
				}
				
				encryptedChannelConfig.httpProxyPort = programTunnelingChannelCfg.httpProxyPort;
				if (encryptedChannelConfig.httpProxyPort == null || encryptedChannelConfig.httpProxyPort.length() == 0)
				{
					if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelProxyPort)) != null)
						encryptedChannelConfig.httpProxyPort = ce.asciiValue();
				}
				
				if ( (encryptedChannelConfig.httpProxyPort != null && encryptedChannelConfig.httpProxyPort.length()  > 0) ||
				     (encryptedChannelConfig.httpProxyHostName != null && encryptedChannelConfig.httpProxyHostName.length() > 0))
					encryptedChannelConfig.httpProxy = true;
				
				if (encryptedChannelConfig.httpProxy)
				{
					encryptedChannelConfig.httpproxyPasswd = programTunnelingChannelCfg.httpproxyPasswd;
					encryptedChannelConfig.httpProxyDomain = programTunnelingChannelCfg.httpProxyDomain;
					encryptedChannelConfig.httpProxyUserName = programTunnelingChannelCfg.httpProxyUserName;
					encryptedChannelConfig.httpProxyKRB5ConfigFile = programTunnelingChannelCfg.httpProxyKRB5ConfigFile;
					encryptedChannelConfig.httpProxyLocalHostName = programTunnelingChannelCfg.httpProxyLocalHostName;
				}
				
				currentChannelConfig = encryptedChannelConfig;
			}
			else
			{
				configImpl.errorTracker().append("Not supported channel type. Type = ")
						.append(ConnectionTypes.toString(configImpl.encryptionCfg().ConnectionType));
				throw ommIUExcept().message(configImpl.errorTracker().text(), OmmInvalidUsageException.ErrorCode.UNSUPPORTED_CHANNEL_TYPE);
			}
			
			break;
		}
		case ConnectionTypes.WEBSOCKET:
		case ConnectionTypes.SOCKET:
		{
			SocketChannelConfig socketChannelConfig = new SocketChannelConfig();

			socketChannelConfig.rsslConnectionType = connectionType;
			readSocketChannelConfig(configImpl, attributes, socketChannelConfig);
			
			HttpChannelConfig programTunnelingChannelCfg = configImpl.tunnelingChannelCfg();
			
			socketChannelConfig.httpProxyHostName = programTunnelingChannelCfg.httpProxyHostName;
			if ( socketChannelConfig.httpProxyHostName == null || socketChannelConfig.httpProxyHostName.length() == 0)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelProxyHost)) != null)
					socketChannelConfig.httpProxyHostName = ce.asciiValue();
			}
			
			socketChannelConfig.httpProxyPort = programTunnelingChannelCfg.httpProxyPort;
			if (socketChannelConfig.httpProxyPort == null || socketChannelConfig.httpProxyPort.length() == 0)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelProxyPort)) != null)
					socketChannelConfig.httpProxyPort = ce.asciiValue();
			}
			
			if ( (socketChannelConfig.httpProxyPort != null && socketChannelConfig.httpProxyPort.length()  > 0) ||
			     (socketChannelConfig.httpProxyHostName != null && socketChannelConfig.httpProxyHostName.length() > 0))
				socketChannelConfig.httpProxy = true;
			
			if (socketChannelConfig.httpProxy == true)
			{
				socketChannelConfig.httpproxyPasswd = programTunnelingChannelCfg.httpproxyPasswd;				
				socketChannelConfig.httpProxyDomain = programTunnelingChannelCfg.httpProxyDomain;
				socketChannelConfig.httpProxyUserName = programTunnelingChannelCfg.httpProxyUserName;
				socketChannelConfig.httpProxyKRB5ConfigFile = programTunnelingChannelCfg.httpProxyKRB5ConfigFile;
				socketChannelConfig.httpProxyLocalHostName = programTunnelingChannelCfg.httpProxyLocalHostName;
			}
			
			currentChannelConfig = socketChannelConfig;
			
			break;
		}
		case ConnectionTypes.HTTP:
		{
			HttpChannelConfig tunnelingChannelCfg;
			tunnelingChannelCfg = new HttpChannelConfig();
			
			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
				tunnelingChannelCfg.hostName = ce.asciiValue();

			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
				tunnelingChannelCfg.serviceName = ce.asciiValue();

			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelTcpNodelay)) != null)
				tunnelingChannelCfg.tcpNodelay = ce.intLongValue() == 0 ? false : ActiveConfig.DEFAULT_TCP_NODELAY;

			HttpChannelConfig programTunnelingChannelCfg = configImpl.tunnelingChannelCfg();
			tunnelingChannelCfg.objectName = programTunnelingChannelCfg.objectName;
			if (tunnelingChannelCfg.objectName == null || tunnelingChannelCfg.objectName.length() == 0)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelObjectName)) != null)
					tunnelingChannelCfg.objectName = ce.asciiValue();
			}
			
			tunnelingChannelCfg.httpProxyHostName = programTunnelingChannelCfg.httpProxyHostName;
			if ( tunnelingChannelCfg.httpProxyHostName == null || tunnelingChannelCfg.httpProxyHostName.length() == 0)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelProxyHost)) != null)
					tunnelingChannelCfg.httpProxyHostName = ce.asciiValue();
			}
			
			tunnelingChannelCfg.httpProxyPort = programTunnelingChannelCfg.httpProxyPort;
			if (tunnelingChannelCfg.httpProxyPort == null || tunnelingChannelCfg.httpProxyPort.length() == 0)
			{
				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelProxyPort)) != null)
					tunnelingChannelCfg.httpProxyPort = ce.asciiValue();
			}
			
			if ( (tunnelingChannelCfg.httpProxyPort != null && tunnelingChannelCfg.httpProxyPort.length()  > 0) ||
			     (tunnelingChannelCfg.httpProxyHostName != null && tunnelingChannelCfg.httpProxyHostName.length() > 0))
				tunnelingChannelCfg.httpProxy = true;
			
			if (tunnelingChannelCfg.httpProxy)
			{
				tunnelingChannelCfg.httpproxyPasswd = programTunnelingChannelCfg.httpproxyPasswd;				
				tunnelingChannelCfg.httpProxyDomain = programTunnelingChannelCfg.httpProxyDomain;
				tunnelingChannelCfg.httpProxyUserName = programTunnelingChannelCfg.httpProxyUserName;
				tunnelingChannelCfg.httpProxyKRB5ConfigFile = programTunnelingChannelCfg.httpProxyKRB5ConfigFile;
				tunnelingChannelCfg.httpProxyLocalHostName = programTunnelingChannelCfg.httpProxyLocalHostName;
			}
			
			currentChannelConfig =  tunnelingChannelCfg;
			
			break;
		}
		default:
		{
			configImpl.errorTracker().append("Not supported channel type. Type = ")
					.append(ConnectionTypes.toString(connectionType));
			throw ommIUExcept().message(configImpl.errorTracker().text(), OmmInvalidUsageException.ErrorCode.UNSUPPORTED_CHANNEL_TYPE);
		}
		}

		currentChannelConfig.name = channelName;

		if (attributes != null)
		{
			if((ce = attributes.getPrimitiveValue(ConfigManager.InterfaceName)) != null)
				currentChannelConfig.interfaceName = ce.asciiValue();

			boolean setCompressionThresholdFromConfigFile = false;
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelCompressionThreshold)) != null)
			{
				currentChannelConfig.compressionThresholdSet = true;
				setCompressionThresholdFromConfigFile = true;
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.compressionThreshold = maxInt;
				else
					currentChannelConfig.compressionThreshold = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_COMPRESSION_THRESHOLD : ce.intLongValue();
			}

			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelCompressionType)) != null)
			{
				currentChannelConfig.compressionType = ce.intValue() < 0 ? ActiveConfig.DEFAULT_COMPRESSION_TYPE : ce.intValue();
				if (currentChannelConfig.compressionType == com.refinitiv.eta.transport.CompressionTypes.LZ4 &&
						!setCompressionThresholdFromConfigFile)
					currentChannelConfig.compressionThreshold = ActiveConfig.DEFAULT_COMPRESSION_THRESHOLD_LZ4;
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.GuaranteedOutputBuffers)) != null)
				currentChannelConfig.guaranteedOutputBuffers(ce.intLongValue());
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.NumInputBuffers)) != null)
				currentChannelConfig.numInputBuffers(ce.intLongValue());
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ConnectionPingTimeout)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.connectionPingTimeout = maxInt;
				else
					currentChannelConfig.connectionPingTimeout = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_CONNECTION_PINGTIMEOUT : ce.intLongValue();
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.SysRecvBufSize)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.sysRecvBufSize = maxInt;
				else
					currentChannelConfig.sysRecvBufSize = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_SYS_RECEIVE_BUFFER_SIZE : ce.intLongValue();
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.SysSendBufSize)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.sysSendBufSize = maxInt;
				else
					currentChannelConfig.sysSendBufSize = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_SYS_SEND_BUFFER_SIZE : ce.intLongValue();
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.HighWaterMark)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.highWaterMark = maxInt;
				else
					currentChannelConfig.highWaterMark = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_HIGH_WATER_MARK : ce.intLongValue();
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelInitTimeout)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.initializationTimeout = maxInt;
				else
					currentChannelConfig.initializationTimeout = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_INITIALIZATION_TIMEOUT : ce.intLongValue();
			}

			if (connectionType == ConnectionTypes.WEBSOCKET || encrypedProtocol == ConnectionTypes.WEBSOCKET) {
				if((ce = attributes.getPrimitiveValue(ConfigManager.WsProtocols)) != null) {
					currentChannelConfig.wsProtocols = ce.asciiValue();
				}

				if ((ce = attributes.getPrimitiveValue(ConfigManager.WsMaxMsgSize)) != null) {
					currentChannelConfig.wsMaxMsgSize = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_WS_MAX_MSG_SIZE : ce.intLongValue();
				}
			}
		}
		
		return currentChannelConfig;
	}

	private void readSocketChannelConfig(EmaConfigImpl configImpl, ConfigAttributes attributes, SocketChannelConfig socketChannelConfig)
	{
		ConfigElement ce;
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
			socketChannelConfig.tcpNodelay = ce.intLongValue() == 0 ? false : ActiveConfig.DEFAULT_TCP_NODELAY;

		if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelDirectSocketWrite)) != null)
			socketChannelConfig.directWrite = ce.intLongValue() == 1 ? true : ActiveConfig.DEFAULT_DIRECT_SOCKET_WRITE;
	}

	@Override
	public StringBuilder strBuilder()
	{
		_strBuilder.setLength(0);
		return _strBuilder;
	}
	
	void handleLoginReqTimeout()
	{
		if (_activeConfig.loginRequestTimeOut == 0)
		{
			while (_state < OmmImplState.LOGIN_STREAM_OPEN_OK && _state != OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN)
				rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
			
			/* Throws OmmInvalidUsageException when EMA receives login reject from the data source. */
			if(_state == OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN)
			{
				throw ommIUExcept().message(_loginCallbackClient.loginFailureMessage(), OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_REJECTED);
			}
		}
		else
		{
			_eventTimeout = false;
			TimeoutEvent timeoutEvent = addTimeoutEvent(_activeConfig.loginRequestTimeOut * 1000, this);
	
			while (!_eventTimeout && (_state < OmmImplState.LOGIN_STREAM_OPEN_OK) && (_state != OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN))
			{
				rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
			}
	
			if (_eventTimeout)
			{
				ChannelInfo channelInfo = _loginCallbackClient.activeChannelInfo();
				ChannelConfig currentConfig = (channelInfo != null ) ? channelInfo._channelConfig : _activeConfig.channelConfigSet.get(  _activeConfig.channelConfigSet.size() -1 );
				strBuilder().append("login failed (timed out after waiting ")
						.append(_activeConfig.loginRequestTimeOut).append(" milliseconds) for ");
				if (currentConfig.rsslConnectionType == ConnectionTypes.SOCKET ||
						currentConfig.rsslConnectionType == ConnectionTypes.WEBSOCKET)
				{
					SocketChannelConfig channelConfig = (SocketChannelConfig) currentConfig;
					_strBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
							.append(")");
				} else if (currentConfig.rsslConnectionType == ConnectionTypes.HTTP || 
						 currentConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
				{
					HttpChannelConfig channelConfig = ((HttpChannelConfig) currentConfig);
					_strBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
							.append(")");
				}

				String excepText = _strBuilder.toString();
	
				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_TIME_OUT);
			} 
			else if (_state == OmmImplState.RSSLCHANNEL_UP_STREAM_NOT_OPEN) /* Throws OmmInvalidUsageException when EMA receives login reject from the data source. */
			{
				timeoutEvent.cancel();
				throw ommIUExcept().message(_loginCallbackClient.loginFailureMessage(), OmmInvalidUsageException.ErrorCode.LOGIN_REQUEST_REJECTED);
			}
			else
				timeoutEvent.cancel();
		}
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

				_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _dispatchStrBuilder.toString(), Severity.ERROR));
			}		
			return false;
		}		
		
		_eventReceived = false;
		_rsslDispatchOptions.maxMessages(count);
		int ret = ReactorReturnCodes.SUCCESS;
		int loopCount = 0;
		long startTime = System.nanoTime();
		long endTime = 0;
		
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
				/*if userTimeout is less than 1000000, need to reset to 10000000 because
				 *the select() call will wait forever and not return if timeout is 0 */
				timeOut = (userTimeout > MIN_TIME_FOR_SELECT ? userTimeout : MIN_TIME_FOR_SELECT);
			}
		}

		try
		{
			if (_state >= OmmImplState.RSSLCHANNEL_UP)
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

			} // end if (_state >= OmmImplState.RSSLCHANNEL_UP)

			while (_state != OmmImplState.NOT_INITIALIZED)
			{
				startTime = endTime;
				
				int selectTimeout = (int)(timeOut/MIN_TIME_FOR_SELECT); 
				int selectCount = _selector.select(selectTimeout > 0 ? selectTimeout : MIN_TIME_FOR_SELECT_IN_MILLISEC);
				if (selectCount > 0 || !_selector.selectedKeys().isEmpty())
				{
					Iterator<SelectionKey> iter = _selector.selectedKeys().iterator();
					while (iter.hasNext())
					{
						SelectionKey key = iter.next();
						iter.remove();
						try
						{
							if (!key.isValid()) continue;
							
							if (key.isReadable())
							{
								if (_pipeSelectKey == key) pipeRead();
								
								loopCount = 0;
								do {
									_userLock.lock();
									try{
										ret = ((ReactorChannel) key.attachment()).dispatch(_rsslDispatchOptions,	_rsslErrorInfo);
									}
									finally
									{
										if (_userLock.isLocked()) {
											_userLock.unlock();
										}
									}
								}
								while (ret > ReactorReturnCodes.SUCCESS && !_eventReceived && ++loopCount < DISPATCH_LOOP_COUNT);
							}
						}
						catch (CancelledKeyException e)
						{
							continue;
						}
					}
					
					if (_eventReceived) return true;

					TimeoutEvent.execute(_timeoutEventQueue);

					if (_eventReceived) return true;
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
					_threadRunning = false;
	
					if (_loggerClient.isTraceEnabled())
					{
						_loggerClient.trace(formatLogMessage(_activeConfig.instanceName,
								"Call to rsslReactorDispatchLoop() received thread interruption signal.", Severity.TRACE));
					}
				}
			}
			
			return false;
			
		} //end of Try		
		catch (CancelledKeyException e)
		{
			if (_loggerClient.isTraceEnabled() && _state != OmmImplState.NOT_INITIALIZED )
			{
				_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, 
						"Call to rsslReactorDispatchLoop() received cancelled key exception.", Severity.TRACE));
			}

			return true;
		} 
		catch (ClosedSelectorException e)
		{
			if (_loggerClient.isTraceEnabled() && _state != OmmImplState.NOT_INITIALIZED )
			{
				_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, 
						"Call to rsslReactorDispatchLoop() received closed selector exception.", Severity.TRACE));
			}

			return true;
		} 
		catch (IOException e)
		{
			if (_loggerClient.isErrorEnabled())
			{
				_dispatchStrBuilder.setLength(0);
				_dispatchStrBuilder.append("Call to rsslReactorDispatchLoop() failed. Received exception,")
						.append(" exception text= ").append(e.getLocalizedMessage()).append(". ");

				_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _dispatchStrBuilder.toString(), Severity.ERROR));
			}

			uninitialize();

			return false;
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

	OmmJsonConverterExceptionImpl ommJCExcept() {
		if (_ommJCExcept == null) {
			_ommJCExcept = new OmmJsonConverterExceptionImpl();
		}
		return _ommJCExcept;
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
	
	TunnelStreamSubmitOptions rsslTunnelStreamSubmitOptions()
	{
		return null;
	}

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
	
	@Override
	public Logger loggerClient()
	{
		return _loggerClient;
	}
	
	@Override
	public ReentrantLock userLock()
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
		ChannelInfo channelInfo = (ChannelInfo) rsslReactorChannel.userSpecObj();
		if (rsslReactorChannel.reactor() != null && rsslReactorChannel.close(_rsslErrorInfo) != ReactorReturnCodes.SUCCESS)
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

		_channelCallbackClient.removeChannel(channelInfo);
	}
	
	void processChannelEvent( ReactorChannelEvent reactorChannelEvent){}
	
	void reLoadDirectory() {}
	
	@Override
	public void eventReceived()
	{
		_eventReceived = true;
	}
	
	public EmaObjectManager objManager()
	{
		return _objManager;
	}

	void setActiveRsslReactorChannel(ChannelInfo activeChannelInfo) {}
	
	void unsetActiveRsslReactorChannel(ChannelInfo cancelChannelInfo) {}
	
	
	protected void modifyIOCtl(int code, int value, ChannelInfo activeChannelInfo)
	{
		if(activeChannelInfo == null || activeChannelInfo.rsslReactorChannel() == null)
		{
			strBuilder().append("No active channel to modify I/O option.");
			handleInvalidUsage(_strBuilder.toString(), OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL);
			return;
		}
		
		Channel channel = activeChannelInfo.rsslReactorChannel().channel();
		Error error = TransportFactory.createError();
		
		int ret = channel.ioctl(code, value, error);
		
		if(code == IOCtlCode.MAX_NUM_BUFFERS || code == IOCtlCode.NUM_GUARANTEED_BUFFERS)
		{
			if(ret != value)
			{
				ret = TransportReturnCodes.FAILURE;
			}
			else
			{
				ret = TransportReturnCodes.SUCCESS;
			}
		}
		
		if(ret != TransportReturnCodes.SUCCESS)
		{
			strBuilder().append("Failed to modify I/O option = ")
			.append(code).append(". Reason: ")
			.append(ReactorReturnCodes.toString(ret))
			.append(". Error text: ")
			.append(error.text());
			
			handleInvalidUsage(_strBuilder.toString(), OmmInvalidUsageException.ErrorCode.FAILURE);
		}
	}
}

