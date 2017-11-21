///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2016. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.io.IOException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedSelectorException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.Set;
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
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamSubmitOptions;

interface OmmCommonImpl
{
	static class ImplementationType
	{
		final static int CONSUMER = 0;
		final static int NIPROVIDER = 1;
		final static int IPROVIDER = 2;
	}
	
	void handleInvalidUsage(String text);
	
	void handleInvalidHandle(long handle, String text);
	
	Logger loggerClient();
	
	String formatLogMessage(String clientName, String temp, int level);
	
	EmaObjectManager objManager();
	
	String instanceName();
	
	StringBuilder strBuilder();
	
	void eventReceived();
	
	ReentrantLock userLock();
	
	int implType();
	
	long nextLongId();
}

abstract class OmmBaseImpl<T> implements OmmCommonImpl, Runnable, TimeoutClient
{
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
	
	protected volatile int _state = OmmImplState.NOT_INITIALIZED;
	private Logger _loggerClient;
	protected StringBuilder _strBuilder = new StringBuilder(1024);
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private OmmInvalidHandleExceptionImpl _ommIHExcept;
	private ActiveConfig _activeConfig;

	protected LoginCallbackClient<T> _loginCallbackClient;
	protected DictionaryCallbackClient<T>  _dictionaryCallbackClient;
	protected DirectoryCallbackClient<T>  _directoryCallbackClient;
	protected ItemCallbackClient<T>  _itemCallbackClient;
	protected ChannelCallbackClient<T> _channelCallbackClient;
	
	
	private final ReentrantLock _userLock = new java.util.concurrent.locks.ReentrantLock();
	private final Object userDispatchMonitor = new Object();
	protected Reactor _rsslReactor;
	private ReactorOptions _rsslReactorOpts = ReactorFactory.createReactorOptions();
	protected ReactorErrorInfo _rsslErrorInfo = ReactorFactory.createReactorErrorInfo();
	private final ReactorErrorInfo _rsslErrorInfoDispatch = ReactorFactory.createReactorErrorInfo();
	private ReactorDispatchOptions _rsslDispatchOptions = ReactorFactory.createReactorDispatchOptions();
	private EncodeIterator _rsslEncIter = CodecFactory.createEncodeIterator();
	private DecodeIterator _rsslDecIter = CodecFactory.createDecodeIterator();
	protected ReactorSubmitOptions _rsslSubmitOptions = ReactorFactory.createReactorSubmitOptions();
	private volatile Selector _selector;
	private Thread _apiDispatchThread;
	private volatile boolean _apiDispatchThreadRunning;
	protected boolean _eventTimeout;
	protected VaIteratableQueue _timeoutEventQueue = new VaIteratableQueue();
	protected EmaObjectManager _objManager = new EmaObjectManager();
	private volatile boolean _eventReceived;
	
	abstract Logger createLoggerClient();
	
	abstract void readCustomConfig(EmaConfigImpl config);
	
	abstract boolean hasErrorClient();
	
	abstract void notifyErrorClient(OmmException ommException);
	
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
			try {
				_loggerClient = createLoggerClient();
	
				readConfiguration(config);
				
				readCustomConfig( config );
	
				config.errorTracker().log(this, _loggerClient);
	
				try
				{
					_selector = Selector.open();
	
				} catch (Exception e)
				{
					strBuilder().append("Failed to open Selector: ").append(e.getLocalizedMessage());
					String temp = _strBuilder.toString();
					
					if (_loggerClient.isErrorEnabled())
						_loggerClient.error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));
	
					throw (ommIUExcept().message(temp));
				}
	
				if (_loggerClient.isTraceEnabled())
					_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, "Successfully open Selector.", Severity.TRACE));
				
				if (_activeConfig.xmlTraceEnable)
					_rsslReactorOpts.enableXmlTracing();
	
				_rsslReactorOpts.userSpecObj(this);
	
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
				} catch (IOException e)
				{
					strBuilder().append("Failed to register selector: " + e.getLocalizedMessage());
					String temp = _strBuilder.toString();
	
					if (_loggerClient.isErrorEnabled())
						_loggerClient.error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));
	
					throw (ommIUExcept().message(temp));
				}
	
				handleAdminDomains();
	
				if (_activeConfig.userDispatch == OperationModel.API_DISPATCH)
				{       
					// create API dispatch thread
					final Thread apiDispatchThread = new Thread(this, "APIDispatchThread");
					if (apiDispatchThread.isDaemon()) {
						apiDispatchThread.setDaemon(false);
					}
					if (apiDispatchThread.getPriority() != Thread.NORM_PRIORITY) {
						apiDispatchThread.setPriority(Thread.NORM_PRIORITY);
					}
					
					// try to start
					try {
						_apiDispatchThreadRunning = true;
						apiDispatchThread.start();
					} catch(final Throwable t) {
						_apiDispatchThreadRunning = false;
						throw t;
					}
					
					// api dispatch thread created and running
					_apiDispatchThread = apiDispatchThread;
				}
			} finally {
				_userLock.unlock();
			}
		} catch (OmmException exception)
		{
			uninitialize();

			if (hasErrorClient())
				notifyErrorClient(exception);
			else
				throw exception;
		} 
	}
	
	void uninitialize()
	{
		if (_state == OmmImplState.NOT_INITIALIZED)
			return;

		boolean userLockLocked = false;
		_userLock.lock();
		try
		{
			userLockLocked = true;
			
			// wait for the API dispatch thread to finish
			if(_apiDispatchThread != null) {
				// indicate should not be running
				_apiDispatchThreadRunning = false;
				
				// break loops by indicating an event message received
				_eventReceived = true;
				
				if(Thread.currentThread() == _apiDispatchThread) {
					// we are the API dispatch thread, special case for the IOException calling uninitialize,
					// rather unwanted to be called from the dispatch thread, the call should be eliminated 
					;
				} else {
					// wake up selector
					Selector selector = this._selector;
					if(selector != null) {
						selector.wakeup();
					}

					// join the thread, note it might block forever if the dispatch thread is in a deadlock
					// very ugly as we must unlock the _userLock to be able to join thread, no better solution at this moment, 
					// would require complete refactoring of the locking model
					_userLock.unlock();
					try {
						userLockLocked = false;
					  _apiDispatchThread.join();
					} finally {
						_userLock.lock();
						userLockLocked = true;
					}
				}		       
				
				_apiDispatchThread = null;
			}
						
			if (_rsslReactor != null)
			{
				if (_loginCallbackClient != null) {
					rsslReactorDispatchLoop(10000, _loginCallbackClient.sendLoginClose());
				}

				if (_channelCallbackClient != null) {
					_channelCallbackClient.closeChannels();
				}

				// shutdown reactor
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
				
				// close selector
				{
					final Selector selector = this._selector;
					if (selector != null)
					{
						try {
							selector.close();
						} catch(final Exception e) {
							strBuilder().append("Failed to close OmmBaseImpl (selector.close), exception=").append(e.getLocalizedMessage());
							_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
						} finally {
							_selector = null;
						}
					}
				}
				
				_rsslReactor = null;
			}

			_state = OmmImplState.NOT_INITIALIZED;
		} catch (InterruptedException e)
		{
			strBuilder().append("OmmBaseImpl unintialize(), Exception occurred, exception=")
					.append(e.getLocalizedMessage());

			if (_loggerClient.isErrorEnabled())
				_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));

		} finally
		{
			if(userLockLocked) {
				_userLock.unlock();
			}
		}
	}
	
	protected long registerClient(ReqMsg reqMsg, T client, Object closure, long parentHandle)
	{
		_userLock.lock();
		try
		{
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
		_userLock.lock();
		try
		{
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
		_userLock.lock();
		try
		{
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
		_userLock.lock();
		try
		{
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
		_userLock.lock();
		try
		{
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
		_userLock.lock();
		try
		{
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
		_userLock.lock();
		try
		{
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
		_userLock.lock();
		try
		{
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
		_userLock.lock();
		try
		{
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
		_userLock.lock();
		try
		{
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
		_userLock.lock();
		try
		{
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
		_userLock.lock();
		try
		{
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
		if (_activeConfig.userDispatch == OperationModel.USER_DISPATCH) {
			synchronized(this.userDispatchMonitor) {
				return rsslReactorDispatchLoop(timeOut, _activeConfig.maxDispatchCountUserThread)
						? DispatchReturn.DISPATCHED : DispatchReturn.TIMEOUT;
			}
		}

		return DispatchReturn.TIMEOUT;
	}
	
	public long dispatch()
	{
		if (_activeConfig.userDispatch == OperationModel.USER_DISPATCH) {
			synchronized(this.userDispatchMonitor) {
				return rsslReactorDispatchLoop(DispatchTimeout.NO_WAIT, _activeConfig.maxDispatchCountUserThread)
						? DispatchReturn.DISPATCHED : DispatchReturn.TIMEOUT;
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
		
		installTimeOut();
		
		return timeoutEvent;
	}
	
	void installTimeOut() 
	{
		Selector selector = this._selector;
		if(selector != null) {
			selector.wakeup();
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
				
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ReconnectAttemptLimit)) != null)
			{
				_activeConfig.isSetCorrectConfigGroup = true;
				_activeConfig.reconnectAttemptLimit(ce.intValue());
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ReconnectMinDelay)) != null)
			{
				_activeConfig.isSetCorrectConfigGroup = true;
				_activeConfig.reconnectMinDelay(ce.intValue());
			}
			
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ReconnectMaxDelay)) != null)
			{
				_activeConfig.isSetCorrectConfigGroup = true;
				_activeConfig.reconnectMaxDelay(ce.intValue());
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.MsgKeyInUpdates)) != null)
			{
				_activeConfig.isSetCorrectConfigGroup = true;
				_activeConfig.msgKeyInUpdates = ce.intLongValue() == 0 ? false : ActiveConfig.DEFAULT_MSGKEYINUPDATES;
			}
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceToStdout)) != null)
			{
				_activeConfig.isSetCorrectConfigGroup = true;
				_activeConfig.xmlTraceEnable = ce.intLongValue() == 1 ? true : ActiveConfig.DEFAULT_XML_TRACE_ENABLE;
			}
		}

		// .........................................................................
		// Channel
		//

		String channelName = config.channelName(_activeConfig.configuredName);
		if (channelName != null  && channelName.trim().length() > 0)
		{
			String[] pieces = channelName.split(",");
			int lastChannelIndex = pieces.length - 1;
			for (int i = 0; i < pieces.length; i++)
			{
				readChannelConfig(config,  pieces[i], ( i == lastChannelIndex ? true : false) );
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
	
	void readChannelConfig(EmaConfigImpl configImpl, String channelName, boolean lastChannel)
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
				socketChannelConfig.tcpNodelay = ce.intLongValue() == 0 ? false : ActiveConfig.DEFAULT_TCP_NODELAY;

			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelDirectSocketWrite)) != null)
				socketChannelConfig.directWrite = ce.intLongValue() == 1 ? true : ActiveConfig.DEFAULT_DIRECT_SOCKET_WRITE;
			
			currentChannelConfig = socketChannelConfig;
			
			break;
		}
		case ConnectionTypes.HTTP:
//		{
//			HttpChannelConfig httpChannelCfg = new HttpChannelConfig();
//			
//			String tempHost = configImpl.getUserSpecifiedHostname();
//			if (tempHost == null)
//			{
//				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
//					httpChannelCfg.hostName = ce.asciiValue();
//			}
//			else
//				httpChannelCfg.hostName = tempHost;
//
//			String tempService = configImpl.getUserSpecifiedPort();
//			if (tempService == null)
//			{
//				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
//					httpChannelCfg.serviceName = ce.asciiValue();
//			}
//			else
//				httpChannelCfg.serviceName = tempService;
//
//			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelTcpNodelay)) != null)
//				httpChannelCfg.tcpNodelay = ce.intLongValue() == 0 ? false : ActiveConfig.DEFAULT_TCP_NODELAY;
//
//			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelObjectName)) != null)
//				httpChannelCfg.objectName = ce.asciiValue();
//			
//			currentChannelConfig =  httpChannelCfg;
//			
//			break;
//		}
		case ConnectionTypes.ENCRYPTED:
//		{
//			EncryptedChannelConfig encryptedChannelCfg = new EncryptedChannelConfig();
//
//			String tempHost = configImpl.getUserSpecifiedHostname();
//			if (tempHost == null)
//			{
//				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
//					encryptedChannelCfg.hostName = ce.asciiValue();
//			}
//			else
//				encryptedChannelCfg.hostName = tempHost;
//
//			String tempService = configImpl.getUserSpecifiedPort();
//			if (tempService == null)
//			{
//				if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
//					encryptedChannelCfg.serviceName = ce.asciiValue();
//			}
//			else
//				encryptedChannelCfg.serviceName = tempService;
//
//			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelTcpNodelay)) != null)
//				encryptedChannelCfg.tcpNodelay = ce.intLongValue() == 0 ? false : ActiveConfig.DEFAULT_TCP_NODELAY;
//
//			if (attributes != null && (ce = attributes.getPrimitiveValue(ConfigManager.ChannelObjectName)) != null)
//				encryptedChannelCfg.objectName = ce.asciiValue();
//
//			currentChannelConfig =  encryptedChannelCfg;
//			
//			break;
//		}
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
			if((ce = attributes.getPrimitiveValue(ConfigManager.InterfaceName)) != null)
				currentChannelConfig.interfaceName = ce.asciiValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelCompressionType)) != null)
				currentChannelConfig.compressionType = ce.intValue() < 0 ? ActiveConfig.DEFAULT_COMPRESSION_TYPE : ce.intValue();
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.GuaranteedOutputBuffers)) != null)
				currentChannelConfig.guaranteedOutputBuffers(ce.intLongValue());
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.NumInputBuffers)) != null)
				currentChannelConfig.numInputBuffers(ce.intLongValue());
	
			if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelCompressionThreshold)) != null)
			{
				if ( ce.intLongValue()  > maxInt )
					currentChannelConfig.compressionThreshold = maxInt;
				else
					currentChannelConfig.compressionThreshold = ce.intLongValue() < 0 ? ActiveConfig.DEFAULT_COMPRESSION_THRESHOLD : ce.intLongValue();
			}
			
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
	
			/* The following code will be removed once deprecated definitions from ConfigManager are removed. */
			if (!_activeConfig.isSetCorrectConfigGroup && lastChannel)
			{
				if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelReconnectAttemptLimit)) != null)
				{
						_activeConfig.reconnectAttemptLimit(ce.intValue());
						configImpl.errorTracker().append( "ChannelReconnectAttemptLimit is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance." )
						.create(Severity.WARNING);
				}
		
				if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelReconnectMinDelay)) != null)
				{
						_activeConfig.reconnectMinDelay(ce.intValue());
						configImpl.errorTracker().append( "ChannelReconnectMinDelay is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance." )
						.create(Severity.WARNING);
				}
		
				if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelReconnectMaxDelay)) != null)
				{
						_activeConfig.reconnectMaxDelay(ce.intValue());
						configImpl.errorTracker().append( "ChannelReconnectMaxDelay is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance." )
						.create(Severity.WARNING);
				}
		
				if( (ce = attributes.getPrimitiveValue(ConfigManager.ChannelMsgKeyInUpdates)) != null)
				{
						_activeConfig.msgKeyInUpdates = ce.intLongValue() == 0 ? false : ActiveConfig.DEFAULT_MSGKEYINUPDATES;
						configImpl.errorTracker().append( "ChannelMsgKeyInUpdates is no longer configured on a per-channel basis; configure it instead in the Consumer instance." )
						.create(Severity.WARNING);
				}
		
				if( (ce = attributes.getPrimitiveValue(ConfigManager.XmlTraceToStdout)) != null)
				{
						_activeConfig.xmlTraceEnable = ce.intLongValue() == 1 ? true : ActiveConfig.DEFAULT_XML_TRACE_ENABLE;
						configImpl.errorTracker().append(  "XmlTraceToStdout is no longer configured on a per-channel basis; configure it instead in the Consumer/NIProvider instance.")
						.create(Severity.WARNING);
				}
			}
		}
		
		_activeConfig.channelConfigSet.add( currentChannelConfig );
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
				ChannelInfo channelInfo = _loginCallbackClient.activeChannelInfo();
				ChannelConfig currentConfig = (channelInfo != null ) ? channelInfo._channelConfig : _activeConfig.channelConfigSet.get(  _activeConfig.channelConfigSet.size() -1 );
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
				do
				{       _userLock.lock();
					try {
						ret = _rsslReactor.reactorChannel()  != null  ? _rsslReactor.dispatchAll(null, _rsslDispatchOptions, _rsslErrorInfoDispatch) : ReactorReturnCodes.SUCCESS;
					} finally {
						_userLock.unlock();
					}
				}
				while (ret > ReactorReturnCodes.SUCCESS && !_eventReceived && loopCount++ < 5);
				
				if (ret < ReactorReturnCodes.SUCCESS)
				{
						if (_loggerClient.isErrorEnabled())
						{
							strBuilder().append("Call to rsslReactorDispatchLoop() failed. Internal sysError='")
										.append(_rsslErrorInfoDispatch.error().sysError()).append("' Error text='")
										.append(_rsslErrorInfoDispatch.error().text()).append("'. ");
	
							if (_loggerClient.isErrorEnabled())
								_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
						}
	
						return false;
				} 
			
				if ( _eventReceived ) return true;
				
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
					else if ( timeOut < MIN_TIME_FOR_SELECT  )
							timeOut = MIN_TIME_FOR_SELECT;
				}
			} // end if (_state >= OmmImplState.RSSLCHANNEL_UP)

			
			// selector - just one volatile load
			Selector selector = this._selector;

			while(true)
			{
				startTime = endTime;
			
				int selectCount = selector.select(timeOut/MIN_TIME_FOR_SELECT);
				Set<SelectionKey> selectedKeys = selector.selectedKeys();
				if (selectCount > 0 || !selectedKeys.isEmpty())
				{
					Iterator<SelectionKey> iter = selectedKeys.iterator();
					while (iter.hasNext())
					{
						SelectionKey key = iter.next();
						iter.remove();
						try
						{
							if (!key.isValid()) continue;
							
							if (key.isReadable())
							{
								_userLock.lock();
								try {
									ret = ((ReactorChannel) key.attachment()).dispatch(_rsslDispatchOptions, _rsslErrorInfoDispatch);
								} finally {
									_userLock.unlock();
								}
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
			} // while		      
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
		catch (ClosedSelectorException e)
		{
			if (_loggerClient.isTraceEnabled())
			{
				strBuilder().append("Call to rsslReactorDispatchLoop() received closed selector exception.");

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
	
	TunnelStreamSubmitOptions rsslTunnelStreamSubmitOptions()
	{
		return null;
	}

	@Override
	public void run() 
	{
		while (_apiDispatchThreadRunning)
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
		if (rsslReactorChannel.close(_rsslErrorInfo) != ReactorReturnCodes.SUCCESS)
		{
			if (_loggerClient.isErrorEnabled())
			{
				_userLock.lock();
				try {
					strBuilder().append("Failed to close reactor channel (rsslReactorChannel).")
							.append("' RsslChannel='")
							.append(Integer.toHexString(_rsslErrorInfo.error().channel() != null ? _rsslErrorInfo.error().channel().hashCode() : 0))
							.append("Error Id ").append(_rsslErrorInfo.error().errorId())
							.append("Internal sysError ").append(_rsslErrorInfo.error().sysError())
							.append("Error Location ").append(_rsslErrorInfo.location())
							.append("Error Text ").append(_rsslErrorInfo.error().text()).append("'. ");
	
					_loggerClient.error(
							formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.ERROR));
				} finally {
					_userLock.unlock();
				}
			}
		}

		_channelCallbackClient.removeChannel( (ChannelInfo) rsslReactorChannel.userSpecObj());
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
}
