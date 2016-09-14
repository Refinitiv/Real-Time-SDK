///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.thomsonreuters.ema.access.ConfigManager.ConfigAttributes;
import com.thomsonreuters.ema.access.ConfigManager.ConfigElement;
import com.thomsonreuters.ema.access.OmmException.ExceptionType;
import com.thomsonreuters.ema.access.OmmLoggerClient.Severity;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.WritePriorities;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;

class OmmConsumerImpl extends OmmBaseImpl<OmmConsumerClient> implements OmmConsumer
{
	private OmmConsumerErrorClient _consumerErrorClient;
	private OmmConsumerActiveConfig _activeConfig;

	OmmConsumerImpl(OmmConsumerConfig config)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);		
	}

	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerErrorClient client)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		_consumerErrorClient = client;
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);		
	}
	
	@Override
	public void uninitialize()
	{
		super.uninitialize();
	}

	@Override
	public String consumerName()
	{
		return _activeConfig.instanceName;
	}

	@Override
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client, Object closure, long parentHandle)
	{
		return super.registerClient(reqMsg, client, closure, parentHandle);
	}

	@Override
	public long registerClient(TunnelStreamRequest tunnelStreamRequest, OmmConsumerClient client, Object closure)
	{
		return super.registerClient(tunnelStreamRequest, client, closure);
	}

	@Override
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client)
	{
		return super.registerClient(reqMsg, client);
	}

	@Override
	public long registerClient(ReqMsg reqMsg, OmmConsumerClient client, Object closure)
	{
		return super.registerClient(reqMsg, client, closure);
	}

	@Override
	public long registerClient(TunnelStreamRequest tunnelStreamRequest, OmmConsumerClient client)
	{
		return super.registerClient(tunnelStreamRequest, client);
	}

	@Override
	public void reissue(ReqMsg reqMsg, long handle)
	{
		super.reissue(reqMsg, handle);
	}

	@Override
	public void unregister(long handle)
	{
		super.unregister(handle);
	}

	@Override
	public void submit(GenericMsg genericMsg, long handle)
	{
		super.submit(genericMsg, handle);
	}

	@Override
	public void submit(PostMsg postMsg, long handle)
	{
		super.submit(postMsg, handle);
	}

	@Override
	public long dispatch(long timeOut)
	{
		return super.dispatch(timeOut);
	}

	@Override
	public long dispatch()
	{
		return super.dispatch();
	}
	
	boolean hasConsumerErrorClient()
	{
		return (_consumerErrorClient != null);
	}

	OmmConsumerErrorClient consumerErrorClient()
	{
		return _consumerErrorClient;
	}
	
	@Override
	void notifyErrorClient(OmmException ommException)
	{
		switch (ommException.exceptionType())
		{
		case ExceptionType.OmmInvalidHandleException:
			_consumerErrorClient.onInvalidHandle(((OmmInvalidHandleException) ommException).handle(), ommException.getMessage());
			break;
		case ExceptionType.OmmInvalidUsageException:
			_consumerErrorClient.onInvalidUsage(ommException.getMessage());
			break;
		default:
			break;
		}
	}

	@Override
	String formatLogMessage(String clientName, String temp, int level) {
		strBuilder().append("loggerMsg\n").append("    ClientName: ").append(clientName).append("\n")
        .append("    Severity: ").append(OmmLoggerClient.loggerSeverityAsString(level)).append("\n")
        .append("    Text:    ").append(temp).append("\n").append("loggerMsgEnd\n\n");

		return _strBuilder.toString();
	}

	@Override
	String instanceName() {
		return _activeConfig.instanceName;
	}
	
	@Override
	boolean hasErrorClient()
	{
		return _consumerErrorClient != null ? true: false;
	}

	@Override
	void readCustomConfig(EmaConfigImpl config)
	{
		_activeConfig.dictionaryConfig = (DictionaryConfig) new DictionaryConfig(false);
		
		_activeConfig.dictionaryConfig.dictionaryName = ((OmmConsumerConfigImpl)config).dictionaryName(_activeConfig.configuredName);
		
		ConfigAttributes attributes = config.xmlConfig().getConsumerAttributes(_activeConfig.configuredName);

		ConfigElement ce = null;
		int maxInt = Integer.MAX_VALUE;

		if (attributes != null)
		{
			if ((ce = attributes.getPrimitiveValue(ConfigManager.ObeyOpenWindow)) != null)
				_activeConfig.obeyOpenWindow = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.PostAckTimeout)) != null)
				_activeConfig.postAckTimeout = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.RequestTimeout)) != null)
				_activeConfig.requestTimeout = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.DirectoryRequestTimeOut)) != null)
				_activeConfig.directoryRequestTimeOut = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();

			if ((ce = attributes.getPrimitiveValue(ConfigManager.MaxOutstandingPosts)) != null)
				_activeConfig.maxOutstandingPosts = ce.intLongValue() > maxInt ? maxInt : ce.intLongValue();
		}
		
		if (_activeConfig.dictionaryConfig.dictionaryName == null)
		{
			_activeConfig.dictionaryConfig.dictionaryName = "Dictionary";
			_activeConfig.dictionaryConfig.isLocalDictionary = false;
			_activeConfig.dictionaryConfig.enumtypeDefFileName = null;
			_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName = null;
		} else
		{
			attributes = config.xmlConfig().getDictionaryAttributes(_activeConfig.dictionaryConfig.dictionaryName);
			if (attributes == null)
			{
				config.errorTracker().append("no configuration exists for consumer dictionary [")
						.append(ConfigManager.DICTIONARY_LIST.toString()).create(Severity.ERROR);
			}
			else
			{
				ce = attributes.getPrimitiveValue(ConfigManager.DictionaryType);
				if ( ce != null && ce.booleanValue() == true)
				{
					_activeConfig.dictionaryConfig.isLocalDictionary = true;
					
					if ((ce = attributes.getPrimitiveValue(ConfigManager.DictionaryRDMFieldDictFileName)) == null)
						_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName = "./RDMFieldDictionary";
					else
						_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName = ce.asciiValue();
	
					if ((ce = attributes.getPrimitiveValue(ConfigManager.DictionaryEnumTypeDefFileName)) == null)
						_activeConfig.dictionaryConfig.enumtypeDefFileName = "./enumtype.def";
					else
						_activeConfig.dictionaryConfig.enumtypeDefFileName = ce.asciiValue();
				}
			}
		}
		
		_activeConfig.rsslDirectoryRequest = config.directoryReq();
		_activeConfig.rsslFldDictRequest = config.rdmFldDictionaryReq();
		_activeConfig.rsslEnumDictRequest = config.enumDefDictionaryReq();
		_activeConfig.fldDictReqServiceName = config.fidDictReqServiceName();
		_activeConfig.enumDictReqServiceName = config.enumDictReqServiceName();
	}

	void loadDirectory()
	{	
		if (_activeConfig.directoryRequestTimeOut == 0)
		{
			while (_state < OmmImplState.DIRECTORY_STREAM_OPEN_OK)
				rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
		}
		else
		{
			_eventTimeout = false;
			TimeoutEvent timeoutEvent = addTimeoutEvent(_activeConfig.directoryRequestTimeOut * 1000, this);
	
			while (!_eventTimeout && (_state < OmmImplState.DIRECTORY_STREAM_OPEN_OK))
				rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
	
			if (_eventTimeout)
			{
				strBuilder().append("directory retrieval failed (timed out after waiting ")
						.append(_activeConfig.directoryRequestTimeOut).append(" milliseconds) for ");
				int count = _loginCallbackClient.loginChannelList().size();
				ChannelInfo loginChanInfo = _loginCallbackClient.loginChannelList().get(count - 1);
				if( loginChanInfo._channelConfig.rsslConnectionType  == ConnectionTypes.SOCKET)
				{
					SocketChannelConfig channelConfig = (SocketChannelConfig) loginChanInfo._channelConfig;
					_strBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
							.append(")");
				}
				
				String excepText = _strBuilder.toString();
				
				if (loggerClient().isErrorEnabled())
					loggerClient().error(formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw ommIUExcept().message(excepText);
			} else
				timeoutEvent.cancel();
		}
	}

	void loadDictionary()
	{
		if (_activeConfig.dictionaryRequestTimeOut == 0)
		{
			while (!dictionaryCallbackClient().isDictionaryReady())
				rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
		}
		else
		{
			_eventTimeout = false;
			TimeoutEvent timeoutEvent = addTimeoutEvent(_activeConfig.dictionaryRequestTimeOut * 1000, this);
	
			while (!_eventTimeout && !dictionaryCallbackClient().isDictionaryReady())
				rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread);
	
			if (_eventTimeout)
			{
				strBuilder().append("dictionary retrieval failed (timed out after waiting ")
						.append(_activeConfig.dictionaryRequestTimeOut).append(" milliseconds) for ");
				int count = _loginCallbackClient.loginChannelList().size();
				ChannelInfo loginChanInfo = _loginCallbackClient.loginChannelList().get(count - 1);
				if( loginChanInfo._channelConfig.rsslConnectionType  == ConnectionTypes.SOCKET)
				{
					SocketChannelConfig channelConfig = (SocketChannelConfig) loginChanInfo._channelConfig;
					_strBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
							.append(")");
				}
	
				String excepText = _strBuilder.toString();
				
				if (loggerClient().isErrorEnabled())
					loggerClient().error(formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw ommIUExcept().message(excepText);
			} else
				timeoutEvent.cancel();
		}
	}

	@Override
	void processChannelEvent(ReactorChannelEvent reactorChannelEvent)
	{
	}

	@Override
	Logger createLoggerClient() {
		return LoggerFactory.getLogger(OmmConsumerImpl.class);
	}

	@Override
	ConfigAttributes getAttributes(EmaConfigImpl config)
	{
		return config.xmlConfig().getConsumerAttributes(_activeConfig.configuredName);
	}
	
	@Override
	Object getAttributeValue(EmaConfigImpl config, int attributeKey)
	{
		return config.xmlConfig().getConsumerAttributeValue(_activeConfig.configuredName, attributeKey);
	}

	@Override
	void handleAdminDomains() {
		
		_loginCallbackClient = new LoginCallbackClientConsumer(this);;
		_loginCallbackClient.initialize();

		_dictionaryCallbackClient = new DictionaryCallbackClientConsumer(this);
		_dictionaryCallbackClient.initialize();

		_directoryCallbackClient = new DirectoryCallbackClientConsumer(this);
		_directoryCallbackClient.initialize();

		_itemCallbackClient = new ItemCallbackClientConsumer(this);
		_itemCallbackClient.initialize();

		_channelCallbackClient = new ChannelCallbackClient<>(this,_rsslReactor);
		_channelCallbackClient.initializeConsumerRole(_loginCallbackClient.rsslLoginRequest(), _directoryCallbackClient.rsslDirectoryRequest());

		handleLoginReqTimeout();
		loadDirectory();
		loadDictionary();
		
	}
	
	@Override
	void handleInvalidUsage(String text)
	{
		if ( hasErrorClient() )
			_consumerErrorClient.onInvalidUsage(text);
		else
			throw (ommIUExcept().message(text.toString()));
		
	}

	@Override
	void handleInvalidHandle(long handle, String text)
	{	
		if ( hasErrorClient() )
			_consumerErrorClient.onInvalidHandle(handle, text);
		else
			throw (ommIHExcept().message(text, handle));
	}
}