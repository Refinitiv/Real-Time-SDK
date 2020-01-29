///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
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
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannel;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelEvent;
import com.thomsonreuters.upa.valueadd.reactor.ReactorChannelInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorErrorInfo;
import com.thomsonreuters.upa.valueadd.reactor.ReactorFactory;
import com.thomsonreuters.upa.valueadd.reactor.TunnelStreamSubmitOptions;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.rdm.EmaRdm;

class OmmConsumerImpl extends OmmBaseImpl<OmmConsumerClient> implements OmmConsumer
{
	private OmmConsumerErrorClient _consumerErrorClient;
	private OmmConsumerActiveConfig _activeConfig;
	private TunnelStreamSubmitOptions _rsslTunnelStreamSubmitOptions;
	private ReqMsg loginRequest = EmaFactory.createReqMsg();
	private OmmConsumerClient		_adminClient;
	private Object					_adminClosure;

	OmmConsumerImpl(OmmConsumerConfig config)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		_adminClient = null;
		_adminClosure = null;
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);		
	}
	
	//only for unit test, internal use
	OmmConsumerImpl(OmmConsumerConfig config, boolean internalUse)
	{
		super();
		
		if (internalUse)
		{
			_activeConfig = new OmmConsumerActiveConfig();
			_adminClient = null;
			_adminClosure = null;
			super.initializeForTest(_activeConfig, (OmmConsumerConfigImpl)config);
		}
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerClient client)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = client;
		_adminClosure = null;
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);		
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerClient client, Object closure)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = client;
		_adminClosure = closure;
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
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmConsumerErrorClient client)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = adminClient;
		_adminClosure = null;
		_consumerErrorClient = client;
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);		
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmConsumerErrorClient client, Object closure)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = adminClient;
		_adminClosure = closure;
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
			_consumerErrorClient.onInvalidUsage(ommException.getMessage(), ((OmmInvalidUsageException)ommException).errorCode());
			break;
		default:
			break;
		}
	}

	@Override
	public String formatLogMessage(String clientName, String temp, int level) {
		strBuilder().append("loggerMsg\n").append("    ClientName: ").append(clientName).append("\n")
        .append("    Severity: ").append(OmmLoggerClient.loggerSeverityAsString(level)).append("\n")
        .append("    Text:    ").append(temp).append("\n").append("loggerMsgEnd\n\n");

		return _strBuilder.toString();
	}

	@Override
	public String instanceName() {
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
		_activeConfig.dictionaryConfig = new DictionaryConfig(false);
		
		_activeConfig.dictionaryConfig.dictionaryName = ((OmmConsumerConfigImpl)config).dictionaryName(_activeConfig.configuredName);
		
		ConfigAttributes attributes = config.xmlConfig().getConsumerAttributes(_activeConfig.configuredName);

		ConfigElement ce = null;
		int maxInt = Integer.MAX_VALUE;
		int value = 0;
		
		if (attributes != null)
		{
			if ((ce = attributes.getPrimitiveValue(ConfigManager.ObeyOpenWindow)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeConfig.obeyOpenWindow = value > maxInt ? maxInt : value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.PostAckTimeout)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeConfig.postAckTimeout = value > maxInt ? maxInt : value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.DirectoryRequestTimeOut)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeConfig.directoryRequestTimeOut = value > maxInt ? maxInt : value;
			}

			if ((ce = attributes.getPrimitiveValue(ConfigManager.MaxOutstandingPosts)) != null)
			{
				value = ce.intLongValue();
				if (value >= 0)
					_activeConfig.maxOutstandingPosts = value > maxInt ? maxInt : value;
			}
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
				config.errorTracker().append("no configuration exists in the config file for consumer dictionary [")
						.append(ConfigManager.DICTIONARY_LIST.toString()).append("]. Will use dictionary defaults if not config programmatically.").create(Severity.WARNING);
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
		
		ProgrammaticConfigure pc = null;
		if ( (pc = config.programmaticConfigure()) != null )
			pc.retrieveDictionaryConfig( _activeConfig.dictionaryConfig.dictionaryName, _activeConfig );
		
		_activeConfig.rsslDirectoryRequest = config.directoryReq();
		_activeConfig.rsslFldDictRequest = config.rdmFldDictionaryReq();
		_activeConfig.rsslEnumDictRequest = config.enumDefDictionaryReq();
		_activeConfig.fldDictReqServiceName = config.fidDictReqServiceName();
		_activeConfig.enumDictReqServiceName = config.enumDictReqServiceName();
		
		if ( pc != null )
			pc.retrieveCustomConfig( _activeConfig.configuredName, _activeConfig );
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
				ChannelInfo loginChanInfo = _loginCallbackClient.activeChannelInfo();
				if( loginChanInfo._channelConfig.rsslConnectionType  == ConnectionTypes.SOCKET)
				{
					SocketChannelConfig channelConfig = (SocketChannelConfig) loginChanInfo._channelConfig;
					_strBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
							.append(")");
				}
				
				String excepText = _strBuilder.toString();
				
				if (loggerClient().isErrorEnabled())
					loggerClient().error(formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.DIRECTORY_REQUEST_TIME_OUT);
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
				ChannelInfo loginChanInfo = _loginCallbackClient.activeChannelInfo();
				if( loginChanInfo._channelConfig.rsslConnectionType  == ConnectionTypes.SOCKET)
				{
					SocketChannelConfig channelConfig = (SocketChannelConfig) loginChanInfo._channelConfig;
					_strBuilder.append(channelConfig.hostName).append(":").append(channelConfig.serviceName)
							.append(")");
				}
	
				String excepText = _strBuilder.toString();
				
				if (loggerClient().isErrorEnabled())
					loggerClient().error(formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.DICTIONARY_REQUEST_TIME_OUT);
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
	void handleAdminDomains(EmaConfigImpl config) {
		
		_loginCallbackClient = new LoginCallbackClientConsumer(this);;
		_loginCallbackClient.initialize();

		_dictionaryCallbackClient = new DictionaryCallbackClientConsumer(this);
		_dictionaryCallbackClient.initialize();

		_directoryCallbackClient = new DirectoryCallbackClientConsumer(this);
		_directoryCallbackClient.initialize();

		_itemCallbackClient = new ItemCallbackClientConsumer(this);
		_itemCallbackClient.initialize();
		
		if(_adminClient != null)
		{
			/* RegisterClient does not require a fully encoded login message to set the callbacks */
			loginRequest.clear().domainType(EmaRdm.MMT_LOGIN);
			_itemCallbackClient.registerClient(loginRequest, _adminClient, _adminClosure, 0);
		}

		_channelCallbackClient = new ChannelCallbackClient<>(this,_rsslReactor);
		_channelCallbackClient.initializeConsumerRole(_loginCallbackClient.rsslLoginRequest(), _directoryCallbackClient.rsslDirectoryRequest(), config.clientId());

		handleLoginReqTimeout();
		loadDirectory();
		loadDictionary();
		
	}
	
	@Override
	public void handleInvalidUsage(String text, int errorCode)
	{
		if ( hasErrorClient() )
		{
			_consumerErrorClient.onInvalidUsage(text);
			
			_consumerErrorClient.onInvalidUsage(text, errorCode);
		}
		else
			throw (ommIUExcept().message(text.toString(), errorCode));
		
	}

	@Override
	public void handleInvalidHandle(long handle, String text)
	{	
		if ( hasErrorClient() )
			_consumerErrorClient.onInvalidHandle(handle, text);
		else
			throw (ommIHExcept().message(text, handle));
	}

	TunnelStreamSubmitOptions rsslTunnelStreamSubmitOptions()
	{
		if (_rsslTunnelStreamSubmitOptions == null)
			_rsslTunnelStreamSubmitOptions = ReactorFactory.createTunnelStreamSubmitOptions();

		return _rsslTunnelStreamSubmitOptions;
	}

	@Override
	public int implType() {
		return OmmCommonImpl.ImplementationType.CONSUMER;
	}

	@Override
	public long nextLongId() {
		return LongIdGenerator.nextLongId();
	}

	@Override
	public void channelInformation(ChannelInformation channelInformation)
	{
		if (_loginCallbackClient == null || _loginCallbackClient.loginChannelList().isEmpty()) {
			channelInformation.clear();
			return;
		}

		try {
			super.userLock().lock();

			ReactorChannel reactorChannel = null;
			// return first item in channel list with proper status
			for (ChannelInfo ci : _loginCallbackClient.loginChannelList())
				if (ci.rsslReactorChannel().state() == ReactorChannel.State.READY || ci.rsslReactorChannel().state() == ReactorChannel.State.UP) {
					reactorChannel = ci.rsslReactorChannel();
					break;
				}

			// if reactorChannel is not set, then just use the first element in _loginCallbackClient.loginChannelList()
			if (reactorChannel == null)
				reactorChannel = _loginCallbackClient.loginChannelList().get(0).rsslReactorChannel();

			channelInformation.hostname(reactorChannel.hostname());
			channelInformation.ipAddress("not available for OmmConsumer connections");
			channelInformation.port(reactorChannel.port());

			if (reactorChannel.channel() == null ) {
				channelInformation.componentInfo("unavailable");
			}
			else {
				ReactorChannelInfo rci = ReactorFactory.createReactorChannelInfo();
				ReactorErrorInfo ei = ReactorFactory.createReactorErrorInfo();
				reactorChannel.info(rci, ei);

				if (rci.channelInfo() == null ||
					rci.channelInfo().componentInfo() == null ||
					rci.channelInfo().componentInfo().isEmpty())
					channelInformation.componentInfo("unavailable");
				else {
					channelInformation.componentInfo(rci.channelInfo().componentInfo().get(0).componentVersion().toString());
				}
			}
			channelInformation.channelState(reactorChannel.channel().state());
			if (reactorChannel.channel() != null) {
				channelInformation.connectionType(reactorChannel.channel().connectionType());
				channelInformation.protocolType(reactorChannel.channel().protocolType());
				channelInformation.majorVersion(reactorChannel.channel().majorVersion());
				channelInformation.minorVersion(reactorChannel.channel().minorVersion());
				channelInformation.pingTimeout(reactorChannel.channel().pingTimeout());
			}
			else {
				channelInformation.connectionType(-1);
				channelInformation.protocolType(-1);
				channelInformation.majorVersion(0);
				channelInformation.minorVersion(0);
				channelInformation.pingTimeout(0);
			}
		}
		finally {
			super.userLock().unlock();
		}

	}

	@Override
	public void modifyIOCtl(int code, int value)
	{
		super.userLock().lock();
		
		try
		{
			super.modifyIOCtl(code, value, _loginCallbackClient.activeChannelInfo());
		}
		finally
		{
			super.userLock().unlock();
		}
	}
}
