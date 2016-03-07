///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.io.IOException;
import java.nio.channels.CancelledKeyException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.ReentrantLock;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.thomsonreuters.ema.access.ConfigManager.ConfigAttributes;
import com.thomsonreuters.ema.access.ConfigManager.ConfigElement;
import com.thomsonreuters.ema.access.OmmConsumerConfig.OperationModel;
import com.thomsonreuters.ema.access.OmmException.ExceptionType;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.valueadd.common.VaIteratableQueue;
import com.thomsonreuters.upa.valueadd.reactor.Reactor;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorDispatchOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.ReactorOptions;
import com.thomsonreuters.upa.valueadd.reactor.ReactorReturnCodes;
import com.thomsonreuters.upa.valueadd.reactor.ReactorSubmitOptions;

class OmmConsumerImpl implements Runnable, OmmConsumer, TimeoutClient
{
	private final static int SHUTDOWN_TIMEOUT_IN_SECONDS = 5;

	static class OmmConsumerState
	{
		final static int NOT_INITIALIZED = 0;
		final static int REACTOR_INITIALIZED = 1;
		final static int RSSLCHANNEL_DOWN = 2;
		final static int RSSLCHANNEL_UP = 3;
		final static int LOGIN_STREAM_OPEN_SUSPECT = 4;
		final static int LOGIN_STREAM_OPEN_OK = 5;
		final static int LOGIN_STREAM_CLOSED = 6;
		final static int DIRECTORY_STREAM_OPEN_SUSPECT = 7;
		final static int DIRECTORY_STREAM_OPEN_OK = 8;
	}

	private int _consumerState = OmmConsumerState.NOT_INITIALIZED;
	private Logger _loggerClient;
	private StringBuilder _consumerStrBuilder = new StringBuilder();
	private OmmInvalidUsageExceptionImpl _ommIUExcept;
	private OmmInvalidHandleExceptionImpl _ommIHExcept;
	private OmmConsumerActiveConfig _activeConfig;

	private LoginCallbackClient _loginCallbackClient;
	private DictionaryCallbackClient _dictionaryCallbackClient;
	private DirectoryCallbackClient _directoryCallbackClient;
	private ItemCallbackClient _itemCallbackClient;
	private ChannelCallbackClient _channelCallbackClient;
	private OmmConsumerErrorClient _consumerErrorClient;
	private ReentrantLock _consumerLock = new java.util.concurrent.locks.ReentrantLock();
	private Reactor _rsslReactor;
	private ReactorOptions _rsslReactorOpts = ReactorFactory.createReactorOptions();
	private ReactorErrorInfo _rsslErrorInfo = ReactorFactory.createReactorErrorInfo();
	private ReactorDispatchOptions _rsslDispatchOptions = ReactorFactory.createReactorDispatchOptions();
	private EncodeIterator _rsslEncIter = CodecFactory.createEncodeIterator();
	private DecodeIterator _rsslDecIter = CodecFactory.createDecodeIterator();
	private ReactorSubmitOptions _rsslSubmitOptions = ReactorFactory.createReactorSubmitOptions();
	private Selector _selector;
	private ExecutorService _executor;
	private volatile boolean _threadRunning = false;
	private VaIteratableQueue _timeoutEventQueue = new VaIteratableQueue();
	private boolean _eventTimeout;

	OmmConsumerImpl(OmmConsumerConfig config)
	{
		initialize(config);
	}

	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerErrorClient client)
	{
		_consumerErrorClient = client;
		initialize(config);
	}

	void initialize(OmmConsumerConfig config)
	{
		try
		{
			_consumerLock.lock();

			GlobalPool.intialize();

			_loggerClient = LoggerFactory.getLogger(OmmConsumerImpl.class);

			readConfiguration(config);

			((OmmConsumerConfigImpl) config).errorTracker().log(this, _loggerClient);

			try
			{
				_selector = Selector.open();

			} catch (Exception e)
			{
				_threadRunning = false;

				consumerStrBuilder().append("Failed to open Selector: ").append(e.getLocalizedMessage());
				String temp = _consumerStrBuilder.toString();
				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp));
			}

			if (_loggerClient.isTraceEnabled())
				_loggerClient.trace(
						formatLogMessage(_activeConfig.instanceName, "Successfully open Selector.", Severity.TRACE));

			if (_activeConfig.channelConfig.xmlTraceEnable)
				_rsslReactorOpts.enableXmlTracing();

			_rsslReactorOpts.userSpecObj(this);

			_rsslReactor = ReactorFactory.createReactor(_rsslReactorOpts, _rsslErrorInfo);
			if (ReactorReturnCodes.SUCCESS != _rsslErrorInfo.code())
			{
				_threadRunning = false;

				consumerStrBuilder().append("Failed to initialize OmmConsumer (ReactorFactory.createReactor).")
						.append("' Error Id='").append(_rsslErrorInfo.error().errorId()).append("' Internal sysError='")
						.append(_rsslErrorInfo.error().sysError()).append("' Error Location='")
						.append(_rsslErrorInfo.location()).append("' Error Text='")
						.append(_rsslErrorInfo.error().text()).append("'. ");
				String temp = _consumerStrBuilder.toString();
				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp));
			} else
			{
				if (_loggerClient.isTraceEnabled())
					_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, "Successfully created Reactor.",
							Severity.TRACE));
			}

			_consumerState = OmmConsumerState.REACTOR_INITIALIZED;

			try
			{
				_rsslReactor.reactorChannel().selectableChannel().register(_selector, SelectionKey.OP_READ,
						_rsslReactor.reactorChannel());
			} catch (ClosedChannelException e)
			{
				_threadRunning = false;

				consumerStrBuilder().append("Failed to register selector: " + e.getLocalizedMessage());
				String temp = _consumerStrBuilder.toString();

				if (_loggerClient.isErrorEnabled())
					_loggerClient.error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));

				throw (ommIUExcept().message(temp));
			}

			_loginCallbackClient = new LoginCallbackClient(this);
			_loginCallbackClient.initialize();

			_dictionaryCallbackClient = new DictionaryCallbackClient(this);
			_dictionaryCallbackClient.initialize();

			_directoryCallbackClient = new DirectoryCallbackClient(this);
			_directoryCallbackClient.initialize();

			_itemCallbackClient = new ItemCallbackClient(this);
			_itemCallbackClient.initialize();

			_channelCallbackClient = new ChannelCallbackClient(this, _rsslReactor);
			_channelCallbackClient.initialize(_loginCallbackClient.rsslLoginRequest(),
					_directoryCallbackClient.rsslDirectoryRequest());

			handleAdminReqTimeout();

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

			if (hasConsumerErrorClient())
				notifyOmmConsumerErrorClient(exception, _consumerErrorClient);
			else
				throw exception;
		} finally
		{
			if (_consumerLock.isLocked())
				_consumerLock.unlock();
		}
	}

	void uninitialize()
	{
		if (_consumerState == OmmConsumerState.NOT_INITIALIZED)
			return;

		try
		{
			_consumerLock.lock();
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
						consumerStrBuilder().append("Failed to uninitialize OmmConsumer (Reactor.shutdown).")
								.append("Error Id ").append(_rsslErrorInfo.error().errorId())
								.append("Internal sysError ").append(_rsslErrorInfo.error().sysError())
								.append("Error Location ").append(_rsslErrorInfo.location()).append("Error Text ")
								.append(_rsslErrorInfo.error().text()).append("'. ");

						_loggerClient.error(formatLogMessage(_activeConfig.instanceName, _consumerStrBuilder.toString(),
								Severity.ERROR));
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
			_consumerState = OmmConsumerState.NOT_INITIALIZED;
		} catch (InterruptedException | IOException e)
		{
			consumerStrBuilder().append("OmmConsumer unintialize(), Exception occurred, exception=")
					.append(e.getLocalizedMessage());

			if (_loggerClient.isErrorEnabled())
				_loggerClient.error(
						formatLogMessage(_activeConfig.instanceName, _consumerStrBuilder.toString(), Severity.ERROR));

		} finally
		{
			_consumerLock.unlock();
		}
	}

	@Override
	public String consumerName()
	{
		return _activeConfig.instanceName;
	}

	@Override
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client, Object closure, long parentHandle)
	{
		long handle;
		try
		{
			_consumerLock.lock();

			handle = _itemCallbackClient != null
					? _itemCallbackClient.registerClient(reqMsg, client, closure, parentHandle) : 0;
		} finally
		{
			_consumerLock.unlock();
		}

		return handle;
	}

	@Override
	public long registerClient(TunnelStreamRequest tunnelStreamRequest, OmmConsumerClient client, Object closure)
	{
		long handle;
		try
		{
			_consumerLock.lock();

			handle = _itemCallbackClient != null
					? _itemCallbackClient.registerClient(tunnelStreamRequest, client, closure) : 0;
		} finally
		{
			_consumerLock.unlock();
		}

		return handle;
	}

	@Override
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client)
	{
		long handle;
		try
		{
			_consumerLock.lock();

			handle = _itemCallbackClient != null ? _itemCallbackClient.registerClient(reqMsg, client, null, 0) : 0;
		} finally
		{
			_consumerLock.unlock();
		}

		return handle;
	}

	@Override
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client, Object closure)
	{
		long handle;
		try
		{
			_consumerLock.lock();

			handle = _itemCallbackClient != null ? _itemCallbackClient.registerClient(reqMsg, client, closure, 0) : 0;
		} finally
		{
			_consumerLock.unlock();
		}

		return handle;
	}

	@Override
	public long registerClient(TunnelStreamRequest tunnelStreamRequest, OmmConsumerClient client)
	{
		long handle;
		try
		{
			_consumerLock.lock();

			handle = _itemCallbackClient != null ? _itemCallbackClient.registerClient(tunnelStreamRequest, client, null)
					: 0;
		} finally
		{
			_consumerLock.unlock();
		}

		return handle;
	}

	@Override
	public void reissue(ReqMsg reqMsg, long handle)
	{
		try
		{
			_consumerLock.lock();

			if (_itemCallbackClient != null)
				_itemCallbackClient.reissue(reqMsg, handle);
		} finally
		{
			_consumerLock.unlock();
		}
	}

	@Override
	public void unregister(long handle)
	{
		try
		{
			_consumerLock.lock();

			if (_itemCallbackClient != null)
				_itemCallbackClient.unregister(handle);
		} finally
		{
			_consumerLock.unlock();
		}

	}

	@Override
	public void submit(GenericMsg genericMsg, long handle)
	{
		try
		{
			_consumerLock.lock();

			if (_itemCallbackClient != null)
				_itemCallbackClient.submit(genericMsg, handle);
		} finally
		{
			_consumerLock.unlock();
		}
	}

	@Override
	public void submit(PostMsg postMsg, long handle)
	{
		try
		{
			_consumerLock.lock();

			if (_itemCallbackClient != null)
				_itemCallbackClient.submit(postMsg, handle);
		} finally
		{
			_consumerLock.unlock();
		}
	}

	@Override
	public long dispatch(long timeOut)
	{
		if (_activeConfig.userDispatch == OperationModel.USER_DISPATCH)
			return rsslReactorDispatchLoop(timeOut, _activeConfig.maxDispatchCountUserThread)
					? DispatchReturn.DISPATCHED : DispatchReturn.TIMEOUT;

		return DispatchReturn.TIMEOUT;
	}

	@Override
	public long dispatch()
	{
		if (_activeConfig.userDispatch == OperationModel.USER_DISPATCH)
			return rsslReactorDispatchLoop(DispatchTimeout.NO_WAIT, _activeConfig.maxDispatchCountUserThread)
					? DispatchReturn.DISPATCHED : DispatchReturn.TIMEOUT;

		return DispatchReturn.TIMEOUT;
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

	boolean rsslReactorDispatchLoop(long timeOut, int count)
	{
		if (_selector == null)
			return false;

		long userTimeout = TimeoutEvent.userTimeOutExist(this);
		boolean userTimeoutExist = false;
		if (userTimeout >= 0)
		{
			userTimeout = userTimeout / 1000000;
			if (timeOut > 0 && timeOut < userTimeout)
				userTimeoutExist = false;
			else
			{
				userTimeoutExist = true;
				timeOut = (userTimeout > 0 ? userTimeout : 1);
			}
		}

		try
		{
			int selectCount = _selector.select(timeOut);
			if (selectCount > 0 && _selector.selectedKeys() != null)//
			{
				Iterator<SelectionKey> iter = _selector.selectedKeys().iterator();
				int ret = ReactorReturnCodes.SUCCESS;
				while (iter.hasNext())
				{
					SelectionKey key = iter.next();
					iter.remove();
					try
					{
						if (!key.isValid())
							continue;
						if (key.isReadable())
						{
							ReactorChannel reactorChnl = (ReactorChannel) key.attachment();

							_rsslDispatchOptions.maxMessages(count);
							_rsslErrorInfo.clear();

							ret = reactorChnl.dispatch(_rsslDispatchOptions, _rsslErrorInfo);
							if (ret < ReactorReturnCodes.SUCCESS)//
							{
								if (reactorChnl.state() != ReactorChannel.State.CLOSED
										&& reactorChnl.state() != ReactorChannel.State.DOWN_RECONNECTING)
								{
									if (_loggerClient.isErrorEnabled())
									{
										consumerStrBuilder()
												.append("Call to rsslReactorDispatchLoop() failed. Internal sysError='")
												.append(_rsslErrorInfo.error().sysError()).append("' Error text='")
												.append(_rsslErrorInfo.error().text()).append("'. ");

										_consumerLock.lock();
										if (_loggerClient.isErrorEnabled())
											_loggerClient.error(formatLogMessage(_activeConfig.instanceName,
													_consumerStrBuilder.toString(), Severity.ERROR));
										_consumerLock.unlock();
									}

									return false;
								}
							} else
							{
								if (userTimeoutExist && timeOut == 1)
									TimeoutEvent.execute(this);

								if (ret > ReactorReturnCodes.SUCCESS)
									return true;
								else
									return false;
							}
						}
					} catch (CancelledKeyException e)
					{
						continue;
					}
				}
			} else if (selectCount == 0 && userTimeoutExist)
			{
				TimeoutEvent.execute(this);
			}

			if (Thread.currentThread().isInterrupted())
			{
				_threadRunning = false;

				if (_loggerClient.isTraceEnabled())
				{
					consumerStrBuilder()
							.append("Call to rsslReactorDispatchLoop() received thread interruption signal.");

					_consumerLock.lock();
					_loggerClient.trace(formatLogMessage(_activeConfig.instanceName, _consumerStrBuilder.toString(),
							Severity.TRACE));
					_consumerLock.unlock();
				}
			}
		} //
		catch (CancelledKeyException e)
		{
			if (_loggerClient.isTraceEnabled())
			{
				consumerStrBuilder().append("Call to rsslReactorDispatchLoop() received cancelled key exception.");

				_consumerLock.lock();
				_loggerClient.trace(
						formatLogMessage(_activeConfig.instanceName, _consumerStrBuilder.toString(), Severity.TRACE));
				_consumerLock.unlock();
			}

			return true;
		} catch (IOException e)
		{
			if (_loggerClient.isErrorEnabled())
			{
				consumerStrBuilder().append("Call to rsslReactorDispatchLoop() failed. Received exception,")
						.append(" exception text= ").append(e.getLocalizedMessage()).append(". ");

				_consumerLock.lock();
				_loggerClient.error(
						formatLogMessage(_activeConfig.instanceName, _consumerStrBuilder.toString(), Severity.ERROR));
				_consumerLock.unlock();
			}

			uninitialize();

			return false;
		}

		return true;
	}

	boolean hasConsumerErrorClient()
	{
		return (_consumerErrorClient != null);
	}

	OmmConsumerErrorClient consumerErrorClient()
	{
		return _consumerErrorClient;
	}

	void readConfiguration(OmmConsumerConfig config)
	{
		int id = 0;

		OmmConsumerConfigImpl configImpl = (OmmConsumerConfigImpl) config;

		if (_activeConfig == null)
			_activeConfig = new OmmConsumerActiveConfig(this);

		_activeConfig.consumerName = configImpl.consumerName();

		_activeConfig.dictionaryConfig = (DictionaryConfig) new DictionaryConfig();
		_activeConfig.dictionaryConfig.dictionaryName = configImpl.dictionaryName(_activeConfig.consumerName);

		_activeConfig.instanceName = _activeConfig.consumerName;
		// TODO GC??
		_activeConfig.instanceName.concat("_").concat(Integer.toString(id));

		ConfigAttributes attributes = configImpl.xmlConfig().getConsumerAttributes(_activeConfig.consumerName);

		ConfigElement ce = null;
		int maxInt = Integer.MAX_VALUE;

		if (attributes != null)
		{
			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerItemCountHint)) != null)
				_activeConfig.itemCountHint = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerServiceCountHint)) != null)
				_activeConfig.serviceCountHint = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerObeyOpenWindow)) != null)
				_activeConfig.obeyOpenWindow = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerPostAckTimeout)) != null)
				_activeConfig.postAckTimeout = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerRequestTimeout)) != null)
				_activeConfig.requestTimeout = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerLoginRequestTimeOut)) != null)
				_activeConfig.loginRequestTimeOut = ce.intValue() > maxInt ? maxInt : ce.intValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerDirectoryRequestTimeOut)) != null)
				_activeConfig.directoryRequestTimeOut = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerDictionaryRequestTimeOut)) != null)
				_activeConfig.dictionaryRequestTimeOut = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerMaxOutstandingPosts)) != null)
				_activeConfig.maxOutstandingPosts = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerDispatchTimeoutApiThread)) != null)
				_activeConfig.dispatchTimeoutApiThread = ce.intValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerCatchUnhandledException)) != null)
				_activeConfig.catchUnhandledException = ce.intLongValue() > 0 ? true : false;

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerMaxDispatchCountApiThread)) != null)
				_activeConfig.maxDispatchCountApiThread = ce.intValue() > maxInt ? maxInt : ce.intValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ConsumerMaxDispatchCountUserThread)) != null)
				_activeConfig.maxDispatchCountUserThread = ce.intValue() > maxInt ? maxInt : ce.intValue();
		}

		// .........................................................................
		// dictionary
		//
		if (_activeConfig.dictionaryConfig.dictionaryName == null)
		{
			_activeConfig.dictionaryConfig.dictionaryName = "Dictionary";
			_activeConfig.dictionaryConfig.isLocalDictionary = false;
			_activeConfig.dictionaryConfig.enumtypeDefFileName = null;
			_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName = null;
		} else
		{
			attributes = configImpl.xmlConfig().getDictionaryAttributes(_activeConfig.dictionaryConfig.dictionaryName);
			if (attributes == null)
			{
				configImpl.errorTracker().append("no configuration exists for consumer dictionary [")
						.append(ConfigManager.DICTIONARY_LIST.toString()).create(Severity.ERROR);
			}

			if (attributes != null && ((ce = attributes.getPrimitiveValue(ConfigManager.DictionaryType)) == null))
				_activeConfig.dictionaryConfig.isLocalDictionary = false;

			if (ce.booleanValue() == true)
			{
				if (attributes != null
						&& ((ce = attributes.getPrimitiveValue(ConfigManager.DictionaryRDMFieldDictFileName)) == null))
					_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName = "./RDMFieldDictionary";
				else
					_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName = ce.asciiValue();

				if (attributes != null
						&& ((ce = attributes.getPrimitiveValue(ConfigManager.DictionaryEnumTypeDefFileName)) == null))
					_activeConfig.dictionaryConfig.enumtypeDefFileName = "./enumtype.def";
				else
					_activeConfig.dictionaryConfig.enumtypeDefFileName = ce.asciiValue();
			}
		}
		//
		// dictionary
		// .........................................................................

		// IGNORE LOGGER

		// .........................................................................
		// Channel
		//

		String channelName = configImpl.channelName(_activeConfig.consumerName);
		if (channelName != null)
		{
			readChannelConfig(configImpl, channelName);
		} else
		{
			String checkValue = (String) configImpl.xmlConfig().getConsumerAttributeValue(_activeConfig.consumerName,
					ConfigManager.ConsumerChannelSet);
			if (checkValue != null)
			{
				String[] pieces = checkValue.split(",");

				for (int i = 0; i < pieces.length; i++)
				{
					channelName = pieces[i];
					readChannelConfig(configImpl, channelName);
				}
			} else
			{
				SocketChannelConfig socketChannelConfig = new SocketChannelConfig();
				if (socketChannelConfig.rsslConnectionType == ConnectionTypes.SOCKET)
				{
					socketChannelConfig.hostName = configImpl.getUserSpecifiedHostname();
					if (socketChannelConfig.hostName == null)
					{
						if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
							socketChannelConfig.hostName = ce.asciiValue();
						else
							configImpl.errorTracker().append("no configuration exists for [")
									.append(ConfigManager.nodeName(ConfigManager.ChannelHost)).append("]")
									.create(Severity.ERROR);
					}

					socketChannelConfig.serviceName = configImpl.getUserSpecifiedPort();
					if (socketChannelConfig.serviceName == null)
					{
						if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
							socketChannelConfig.serviceName = ce.asciiValue();
						else
							configImpl.errorTracker().append("no configuration exists for [")
									.append(ConfigManager.nodeName(ConfigManager.ChannelPort)).append("]")
									.create(Severity.ERROR);
					}
				}
				_activeConfig.channelConfig = socketChannelConfig;
			}
		}

		//
		// Channel
		// .........................................................................

		/*
		 * if ( ProgrammaticConfigure * const ppc =
		 * pConfigImpl->pProgrammaticConfigure() ) {
		 * ppc->retrieveConsumerConfig( _activeConfig.consumerName,
		 * _activeConfig ); bool isProgmaticCfgChannelName =
		 * ppc->getActiveChannelName(_activeConfig.consumerName, channelName);
		 * bool isProgramatiCfgChannelset =
		 * ppc->getActiveChannelSet(_activeConfig.consumerName, channelSet); if(
		 * isProgmaticCfgChannelName ) ppc->retrieveChannelConfig( channelName,
		 * _activeConfig, pConfigImpl->getUserSpecifiedHostname().length() > 0
		 * ); else { _activeConfig.clearChannelSet(); char *pToken = NULL;
		 * pToken = strtok(const_cast<char *>(channelSet.c_str()), ",");
		 * while(pToken != NULL) { channelName = pToken;
		 * ppc->retrieveChannelConfig( channelName.trimWhitespace(),
		 * _activeConfig, pConfigImpl->getUserSpecifiedHostname().length() > 0
		 * ); pToken = strtok(NULL, ","); } }
		 * 
		 * ppc->retrieveLoggerConfig( _activeConfig.loggerConfig.loggerName ,
		 * _activeConfig ); ppc->retrieveDictionaryConfig(
		 * _activeConfig.dictionaryConfig.dictionaryName, _activeConfig ); }
		 */

		_activeConfig.userDispatch = configImpl.operationModel();
		_activeConfig.rsslRDMLoginRequest = configImpl.loginReq();
		_activeConfig.rsslDirectoryRequest = configImpl.directoryReq();
		_activeConfig.rsslEnumDictRequest = configImpl.enumDefDictionaryReq();
		_activeConfig.rsslFldDictRequest = configImpl.rdmFldDictionaryReq();
		;
	}

	void readChannelConfig(OmmConsumerConfigImpl configImpl, String channelName)
	{
		int maxInt = Integer.MAX_VALUE;

		ConfigAttributes attributes = null;
		ConfigElement ce = null;
		int connectionType = ConnectionTypes.SOCKET;

		attributes = configImpl.xmlConfig().getChannelAttributes(channelName);
		ce = attributes.getPrimitiveValue(ConfigManager.ChannelType);

		if (ce == null)
		{
			if (configImpl.getUserSpecifiedHostname() != null)
				connectionType = ConnectionTypes.SOCKET;
		} else
		{
			connectionType = ce.intValue();
		}

		switch (connectionType)
		{
		case ConnectionTypes.SOCKET:
		{
			SocketChannelConfig socketChannelConfig = new SocketChannelConfig();
			_activeConfig.channelConfig = socketChannelConfig;

			socketChannelConfig.hostName = configImpl.getUserSpecifiedHostname();
			if (socketChannelConfig.hostName == null)
			{
				if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
					socketChannelConfig.hostName = ce.asciiValue();
				else
					configImpl.errorTracker().append("no configuration exists for channel host [")
							.append(ConfigManager.nodeName(ConfigManager.ChannelHost)).create(Severity.ERROR);
			}

			socketChannelConfig.serviceName = configImpl.getUserSpecifiedPort();
			if (socketChannelConfig.serviceName == null)
			{
				if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
					socketChannelConfig.serviceName = ce.asciiValue();
				else
					configImpl.errorTracker().append("no configuration exists for [")
							.append(ConfigManager.nodeName(ConfigManager.ChannelPort)).append("]")
							.create(Severity.ERROR);
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelTcpNodelay)) != null)
				socketChannelConfig.tcpNodelay = ce.booleanValue();

			break;
		}
		case ConnectionTypes.HTTP:
		{
			HttpChannelConfig httpChannelCfg = new HttpChannelConfig();
			_activeConfig.channelConfig = httpChannelCfg;

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
				httpChannelCfg.hostName = ce.asciiValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
				httpChannelCfg.serviceName = ce.asciiValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelTcpNodelay)) != null)
				httpChannelCfg.tcpNodelay = ce.booleanValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelObjectName)) != null)
				httpChannelCfg.objectName = ce.asciiValue();

			break;
		}
		case ConnectionTypes.ENCRYPTED:
		{
			EncryptedChannelConfig encryptedChannelCfg = new EncryptedChannelConfig();
			_activeConfig.channelConfig = encryptedChannelCfg;

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelHost)) != null)
				encryptedChannelCfg.hostName = ce.asciiValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelPort)) != null)
				encryptedChannelCfg.serviceName = ce.asciiValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelTcpNodelay)) != null)
				encryptedChannelCfg.tcpNodelay = ce.booleanValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelObjectName)) != null)
				encryptedChannelCfg.objectName = ce.asciiValue();

			break;
		}
		default:
		{
			configImpl.errorTracker().append("Not supported channel type. Type = ")
					.append(ConnectionTypes.toString(connectionType));
			throw ommIUExcept().message(configImpl.errorTracker().text());
		}
		}

		ChannelConfig currentChannelConfig = _activeConfig.channelConfig;
		currentChannelConfig.name = channelName;

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelInterfaceName)) != null)
			currentChannelConfig.interfaceName = ce.asciiValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelCompressionType)) != null)
			currentChannelConfig.compressionType = ce.intValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelGuaranteedOutputBuffers)) != null)
			currentChannelConfig.guaranteedOutputBuffers = ce.intLongValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelNumInputBuffers)) != null)
			currentChannelConfig.numInputBuffers = ce.intLongValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelCompressionThreshold)) != null)
			currentChannelConfig.compressionThreshold = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelConnectionPingTimeout)) != null)
			currentChannelConfig.connectionPingTimeout = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelSysRecvBufSize)) != null)
			currentChannelConfig.sysRecvBufSize = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelSysSendBufSize)) != null)
			currentChannelConfig.sysSendBufSize = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelReconnectAttemptLimit)) != null)
			currentChannelConfig.reconnectAttemptLimit = ce.intValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelReconnectMinDelay)) != null)
			currentChannelConfig.reconnectMinDelay = ce.intValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelReconnectMaxDelay)) != null)
			currentChannelConfig.reconnectMaxDelay = ce.intValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelMsgKeyInUpdates)) != null)
			currentChannelConfig.msgKeyInUpdates = ce.booleanValue();

		if ((ce = attributes.getPrimitiveValue(ConfigManager.ChannelXmlTraceToStdout)) != null)
			currentChannelConfig.xmlTraceEnable = ce.booleanValue();
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

	String formatLogMessage(String clientName, String temp, int level)
	{
		consumerStrBuilder().append("loggerMsg\n").append("    ClientName: ").append(clientName).append("\n")
				.append("    Severity: ").append(OmmLoggerClient.loggerSeverityAsString(level)).append("\n")
				.append("    Text:    ").append(temp).append("\n").append("loggerMsgEnd\n\n");

		return _consumerStrBuilder.toString();
	}

	StringBuilder consumerStrBuilder()
	{
		_consumerStrBuilder.setLength(0);
		return _consumerStrBuilder;
	}

	OmmConsumerActiveConfig activeConfig()
	{
		return _activeConfig;
	}

	Logger loggerClient()
	{
		return _loggerClient;
	}

	ItemCallbackClient itemCallbackClient()
	{
		return _itemCallbackClient;
	}

	DictionaryCallbackClient dictionaryCallbackClient()
	{
		return _dictionaryCallbackClient;
	}

	DirectoryCallbackClient directoryCallbackClient()
	{
		return _directoryCallbackClient;
	}

	LoginCallbackClient loginCallbackClient()
	{
		return _loginCallbackClient;
	}

	ChannelCallbackClient channelCallbackClient()
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
				_consumerLock.lock();

				consumerStrBuilder().append("Failed to close reactor channel (rsslReactorChannel).")
						.append("' RsslChannel='")
						.append(Integer.toHexString(_rsslErrorInfo.error().channel().hashCode())).append("Error Id ")
						.append(_rsslErrorInfo.error().errorId()).append("Internal sysError ")
						.append(_rsslErrorInfo.error().sysError()).append("Error Location ")
						.append(_rsslErrorInfo.location()).append("Error Text ").append(_rsslErrorInfo.error().text())
						.append("'. ");

				_loggerClient.error(
						formatLogMessage(_activeConfig.instanceName, _consumerStrBuilder.toString(), Severity.ERROR));

				_consumerLock.unlock();
			}
		}

		_channelCallbackClient.removeChannel(rsslReactorChannel);
	}

	void ommConsumerState(int state)
	{
		_consumerState = state;
	}

	VaIteratableQueue timerEventQueue()
	{
		return _timeoutEventQueue;
	}

	TimeoutEvent addTimeoutEvent(long timeoutInMicroSec, TimeoutClient client)
	{
		TimeoutEvent timeoutEvent = (TimeoutEvent) GlobalPool._timeoutEventPool.poll();
		if (timeoutEvent == null)
		{
			timeoutEvent = new TimeoutEvent(timeoutInMicroSec * 1000, client);
			GlobalPool._timeoutEventPool.updatePool(timeoutEvent);
		} else
			timeoutEvent.timeoutInNanoSec(timeoutInMicroSec * 1000, client);

		_timeoutEventQueue.add(timeoutEvent);

		return timeoutEvent;
	}

	void handleAdminReqTimeout()
	{
		_eventTimeout = false;
		TimeoutEvent timeoutEvent = addTimeoutEvent(_activeConfig.loginRequestTimeOut * 1000, this);

		while (!_eventTimeout && (_consumerState < OmmConsumerState.LOGIN_STREAM_OPEN_OK))
			rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);

		if (_eventTimeout)
		{
			consumerStrBuilder().append("login failed (timed out after waiting ")
					.append(_activeConfig.loginRequestTimeOut).append(" milliseconds) for ");
			if (_activeConfig.channelConfig.rsslConnectionType == ConnectionTypes.SOCKET)
			{
				SocketChannelConfig channelConfig = (SocketChannelConfig) _activeConfig.channelConfig;
				_consumerStrBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
						.append(")");
			} else if (_activeConfig.channelConfig.rsslConnectionType == ConnectionTypes.HTTP)
			{
				HttpChannelConfig channelConfig = ((HttpChannelConfig) _activeConfig.channelConfig);
				_consumerStrBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
						.append(")");
			} else if (_activeConfig.channelConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
			{
				EncryptedChannelConfig channelConfig = (EncryptedChannelConfig) _activeConfig.channelConfig;
				_consumerStrBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
						.append(")");
			}

			if (_loggerClient.isErrorEnabled())
				_loggerClient.error(
						formatLogMessage(_activeConfig.instanceName, _consumerStrBuilder.toString(), Severity.ERROR));

			throw ommIUExcept().message(_consumerStrBuilder.toString());
		} else
			timeoutEvent.cancel();

		_eventTimeout = false;
		timeoutEvent = addTimeoutEvent(_activeConfig.directoryRequestTimeOut * 1000, this);

		while (!_eventTimeout && (_consumerState < OmmConsumerState.DIRECTORY_STREAM_OPEN_OK))
			rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);

		if (_eventTimeout)
		{
			consumerStrBuilder().append("directory retrieval failed (timed out after waiting ")
					.append(_activeConfig.directoryRequestTimeOut).append(" milliseconds) for ");
			if (_activeConfig.channelConfig.rsslConnectionType == ConnectionTypes.SOCKET)
			{
				SocketChannelConfig channelConfig = (SocketChannelConfig) _activeConfig.channelConfig;
				_consumerStrBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
						.append(")");
			}
			if (_loggerClient.isErrorEnabled())
				_loggerClient.error(
						formatLogMessage(_activeConfig.instanceName, _consumerStrBuilder.toString(), Severity.ERROR));

			throw ommIUExcept().message(_consumerStrBuilder.toString());
		} else
			timeoutEvent.cancel();

		_eventTimeout = false;
		timeoutEvent = addTimeoutEvent(_activeConfig.dictionaryRequestTimeOut * 1000, this);

		while (!_eventTimeout && !_dictionaryCallbackClient.isDictionaryReady())
			rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);

		if (_eventTimeout)
		{
			consumerStrBuilder().append("dictionary retrieval failed (timed out after waiting ")
					.append(_activeConfig.dictionaryRequestTimeOut).append(" milliseconds) for ");
			if (_activeConfig.channelConfig.rsslConnectionType == ConnectionTypes.SOCKET)
			{
				SocketChannelConfig channelConfig = (SocketChannelConfig) _activeConfig.channelConfig;
				_consumerStrBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
						.append(")");
			}

			if (_loggerClient.isErrorEnabled())
				_loggerClient.error(
						formatLogMessage(_activeConfig.instanceName, _consumerStrBuilder.toString(), Severity.ERROR));

			throw ommIUExcept().message(_consumerStrBuilder.toString());
		} else
			timeoutEvent.cancel();
	}

	void notifyOmmConsumerErrorClient(OmmException ommException, OmmConsumerErrorClient errorClient)
	{
		switch (ommException.exceptionType())
		{
		case ExceptionType.OmmInvalidHandleException:
			errorClient.onInvalidHandle(((OmmInvalidHandleException) ommException).handle(), ommException.getMessage());
			break;
		case ExceptionType.OmmInvalidUsageException:
			errorClient.onInvalidUsage(ommException.getMessage());
			break;
		default:
			break;
		}
	}
}