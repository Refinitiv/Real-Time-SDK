/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.valueadd.reactor.*;
import com.refinitiv.eta.valueadd.reactor.ReactorOAuthCredentialRenewalOptions.RenewalModes;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.refinitiv.ema.access.ConfigManager.ConfigAttributes;
import com.refinitiv.ema.access.ConfigManager.ConfigElement;
import com.refinitiv.ema.access.OmmIProviderConfig.OperationModel;
import com.refinitiv.ema.access.OmmException.ExceptionType;
import com.refinitiv.ema.access.OmmLoggerClient.Severity;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WritePriorities;
import com.refinitiv.ema.rdm.EmaRdm;

class OmmConsumerImpl extends OmmBaseImpl<OmmConsumerClient> implements OmmConsumer, ReactorOAuthCredentialEventCallback
{
	private OmmConsumerErrorClient _consumerErrorClient;
	private OmmConsumerActiveConfig _activeConfig;
	private TunnelStreamSubmitOptions _rsslTunnelStreamSubmitOptions;
	private ReqMsg loginRequest = EmaFactory.createReqMsg();
	private OmmConsumerClient		_adminClient;
	private Object					_adminClosure;
	private OmmEventImpl<OmmConsumerClient>	_OAuthEvent = new OmmEventImpl<OmmConsumerClient>();
	private OmmOAuth2ConsumerClient _OAuthConsumerClient = null;
	private ConsumerSessionInfo sessionInfo = new ConsumerSessionInfo();
	private ReactorJsonConverterOptions jsonConverterOptions = ReactorFactory.createReactorJsonConverterOptions();

	OmmConsumerImpl(OmmConsumerConfig config)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		_adminClient = null;
		_adminClosure = null;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
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
			_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
			super.initializeForTest(_activeConfig, (OmmConsumerConfigImpl)config);
		}
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmOAuth2ConsumerClient OAuthClient)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = null;
		_adminClosure = null;
		_OAuthConsumerClient = OAuthClient;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);		
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmOAuth2ConsumerClient OAuthClient, Object closure)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = null;
		_adminClosure = closure;
		_OAuthConsumerClient = OAuthClient;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);		
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerClient client)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = client;
		_adminClosure = null;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmOAuth2ConsumerClient OAuthClient)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = adminClient;
		_adminClosure = null;
		_OAuthConsumerClient = OAuthClient;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmOAuth2ConsumerClient OAuthClient, Object closure)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = adminClient;
		_adminClosure = closure;
		_OAuthConsumerClient = OAuthClient;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);		
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerClient client, Object closure)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = client;
		_adminClosure = closure;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
	}

	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerErrorClient client)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		_consumerErrorClient = client;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);		
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerErrorClient client, OmmOAuth2ConsumerClient OAuthClient)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = null;
		_adminClosure = null;
		_consumerErrorClient = client;
		_OAuthConsumerClient = OAuthClient;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);		
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerErrorClient client, OmmOAuth2ConsumerClient OAuthClient,  Object closure)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = null;
		_adminClosure = closure;
		_consumerErrorClient = client;
		_OAuthConsumerClient = OAuthClient;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
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
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);		
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmConsumerErrorClient client, OmmOAuth2ConsumerClient OAuthClient, Object closure)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = adminClient;
		_adminClosure = closure;
		_consumerErrorClient = client;
		_OAuthConsumerClient = OAuthClient;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
		super.initialize(_activeConfig, (OmmConsumerConfigImpl)config);
		
		_rsslSubmitOptions.writeArgs().priority(WritePriorities.HIGH);		
	}
	
	OmmConsumerImpl(OmmConsumerConfig config, OmmConsumerClient adminClient, OmmConsumerErrorClient client, OmmOAuth2ConsumerClient OAuthClient)
	{
		super();
		_activeConfig = new OmmConsumerActiveConfig();
		/* the client needs to be set before calling initialize, so the proper item callbacks are set */
		_adminClient = adminClient;
		_adminClosure = null;
		_consumerErrorClient = client;
		_OAuthConsumerClient = OAuthClient;
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
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
		_serviceListMap = ((OmmConsumerConfigImpl)config).serviceListMap();
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
		case ExceptionType.OmmJsonConverterException:
			_consumerErrorClient.onJsonConverterError((ConsumerSessionInfo) ((OmmJsonConverterException) ommException).getSessionInfo(),
					((OmmJsonConverterException) ommException).getErrorCode(), ommException.getMessage());
			break;
		default:
			break;
		}
	}

	@Override
	void onDispatchError(String text, int errorCode)
	{
		_consumerErrorClient.onDispatchError(text, errorCode);
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
		if(((OmmConsumerConfigImpl) config).dataDictionary() == null)
			_activeConfig.dictionaryConfig = new DictionaryConfig(false);
		else
		{
			_activeConfig.dictionaryConfig = new DictionaryConfig(true);
			_activeConfig.dictionaryConfig.dataDictionary = ((OmmConsumerConfigImpl) config).dataDictionary();

			if (loggerClient().isTraceEnabled())
			{
				strBuilder().append("The user specified DataDictionary object is used for dictionary information. ")
						    .append("EMA ignores the DictionaryGroup configuration in either file and programmatic configuration database.");
				loggerClient().trace(formatLogMessage(_activeConfig.instanceName, _strBuilder.toString(), Severity.TRACE));
			}
		}

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

		if(_activeConfig.dictionaryConfig.dataDictionary == null)
		{
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
							.append(ConfigManager.DICTIONARY_LIST.toString()).append("]. Will use dictionary defaults if not config programmatically.").create(Severity.INFO);
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
						.append(_activeConfig.directoryRequestTimeOut).append(" milliseconds) ");

				String excepText = createExceptionMessage();

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
						.append(_activeConfig.dictionaryRequestTimeOut).append(" milliseconds) ");

				String excepText = createExceptionMessage();
				
				if (loggerClient().isErrorEnabled())
					loggerClient().error(formatLogMessage(_activeConfig.instanceName, excepText, Severity.ERROR));
	
				throw ommIUExcept().message(excepText, OmmInvalidUsageException.ErrorCode.DICTIONARY_REQUEST_TIME_OUT);
			} else
				timeoutEvent.cancel();
		}
	}

	private String createExceptionMessage() {
		ChannelInfo loginChanInfo = _loginCallbackClient.activeChannelInfo();
		if (loginChanInfo != null) {
			if( loginChanInfo._channelConfig.rsslConnectionType  == ConnectionTypes.SOCKET ||
					loginChanInfo._channelConfig.rsslConnectionType  == ConnectionTypes.WEBSOCKET)
			{
				SocketChannelConfig channelConfig = (SocketChannelConfig) loginChanInfo._channelConfig;
				_strBuilder.append("for ")
						.append(channelConfig.hostName)
						.append(":")
						.append(channelConfig.serviceName)
						.append(")");
			}
			else if (loginChanInfo._channelConfig.rsslConnectionType == ConnectionTypes.HTTP ||
					loginChanInfo._channelConfig.rsslConnectionType == ConnectionTypes.ENCRYPTED)
			{
				HttpChannelConfig channelConfig = ((HttpChannelConfig) loginChanInfo._channelConfig);
				_strBuilder.append("for ")
						.append(channelConfig.hostName)
						.append(":")
						.append(channelConfig.serviceName)
						.append(")");
			}
		}

		return _strBuilder.toString();
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
		
		/* Checks whether the session channel is enabled */
		if (activeConfig().configSessionChannelSet.size() > 0)
		{
			new ConsumerSession<OmmConsumerClient>(this, _serviceListMap);
			
			/* Turn on SingleOpen feature in the watchlist as it is needed to recover items for the WSB feature */
			_loginCallbackClient.rsslLoginRequest().attrib().applyHasSingleOpen();
			_loginCallbackClient.rsslLoginRequest().attrib().singleOpen(1);
			_loginCallbackClient.rsslLoginRequest().attrib().applyHasAllowSuspectData();
			_loginCallbackClient.rsslLoginRequest().attrib().allowSuspectData(1);
			
			consumerSession().enableSingleOpen(true);
		}

		_dictionaryCallbackClient = new DictionaryCallbackClientConsumer(this);
		_dictionaryCallbackClient.initialize();

		_directoryCallbackClient = new DirectoryCallbackClientConsumer(this);
		_directoryCallbackClient.initialize();

		_itemCallbackClient = new ItemCallbackClientConsumer(this);
		_itemCallbackClient.initialize();


		jsonConverterOptions.clear();
		DataDictionary dictionary = dictionaryCallbackClient().defaultRsslDictionary();
		jsonConverterOptions.dataDictionary(dictionary);
		jsonConverterOptions.serviceNameToIdCallback(this);
		jsonConverterOptions.jsonConversionEventCallback(this);
		jsonConverterOptions.defaultServiceId(_activeConfig.defaultConverterServiceId);
		jsonConverterOptions.jsonExpandedEnumFields(_activeConfig.jsonExpandedEnumFields);
		jsonConverterOptions.catchUnknownJsonKeys(_activeConfig.catchUnknownJsonKeys);
		jsonConverterOptions.catchUnknownJsonFids(_activeConfig.catchUnknownJsonFids);
		jsonConverterOptions.closeChannelFromFailure(_activeConfig.closeChannelFromFailure);
		jsonConverterOptions.jsonConverterPoolsSize(_activeConfig.globalConfig.jsonConverterPoolsSize);
		jsonConverterOptions.sendJsonConvError(_activeConfig.sendJsonConvError);

		if (_rsslReactor.initJsonConverter(jsonConverterOptions, _rsslErrorInfo) != ReactorReturnCodes.SUCCESS) {
			strBuilder().append("Failed to initialize OmmBaseImpl (RWF/JSON Converter).")
					.append("' Error Id='").append(_rsslErrorInfo.error().errorId()).append("' Internal sysError='")
					.append(_rsslErrorInfo.error().sysError()).append("' Error Location='")
					.append(_rsslErrorInfo.location()).append("' Error Text='")
					.append(_rsslErrorInfo.error().text()).append("'. ");

			String temp = _strBuilder.toString();

			if (loggerClient().isErrorEnabled())
				loggerClient().error(formatLogMessage(_activeConfig.instanceName, temp, Severity.ERROR));

			throw (ommIUExcept().message(temp, OmmInvalidUsageException.ErrorCode.INTERNAL_ERROR));
		}

		if(_adminClient != null)
		{
			/* RegisterClient does not require a fully encoded login message to set the callbacks */
			loginRequest.clear().domainType(EmaRdm.MMT_LOGIN);
			_itemCallbackClient.registerClient(loginRequest, _adminClient, _adminClosure, 0);
		}
		
		ReactorOAuthCredentialEventCallback reactorOAuthCallback;
		
		if(_OAuthConsumerClient == null)
		{
			reactorOAuthCallback = null;
		}
		else
		{
			reactorOAuthCallback = this;
		}

		_channelCallbackClient = new ChannelCallbackClient<>(this,_rsslReactor);
		_channelCallbackClient.initializeConsumerRole(_loginCallbackClient.rsslLoginRequest(), _directoryCallbackClient.rsslDirectoryRequest(), config, reactorOAuthCallback);

		if(consumerSession() != null)
		{
			consumerSession().handleLoginReqTimeout();
			consumerSession().loadDirectory();
			consumerSession().loadDictionary();
			consumerSession().reorderSessionChannelInfoForSessionDirectory();
		}
		else
		{	
			handleLoginReqTimeout();
			loadDirectory();
			loadDictionary();
		}
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

	@Override
	public void handleJsonConverterError(ReactorChannel reactorChannel, int errorCode, String text) {
		sessionInfo.loadConsumerSession(reactorChannel);
		if (hasErrorClient()) {
			_consumerErrorClient.onJsonConverterError(sessionInfo, errorCode, text);
		} else {
			if (userLock().isLocked()) {
				userLock().unlock();
			}
			if (_activeConfig.userDispatch != OperationModel.API_DISPATCH) {
				throw (ommJCExcept().message(sessionInfo, errorCode, text));
			}
		}
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
		if(consumerSession() != null)
		{
			StringBuilder temp = strBuilder();
			temp.append("The request routing feature do not support the channelInformation method. The sessionChannelInfo() must be used instead.");
			handleInvalidUsage(temp.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			return;
		}
		
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
			
			ReactorChannelInfo rci = ReactorFactory.createReactorChannelInfo();
			ReactorErrorInfo ei = ReactorFactory.createReactorErrorInfo();
			if( reactorChannel.info(rci, ei) != ReactorReturnCodes.SUCCESS)
			{
				channelInformation.componentInfo("unavailable");
			}
			else
			{
				if (rci.channelInfo() == null ||
					rci.channelInfo().componentInfo() == null ||
					rci.channelInfo().componentInfo().isEmpty())
					channelInformation.componentInfo("unavailable");
				else {
					channelInformation.componentInfo(rci.channelInfo().componentInfo().get(0).componentVersion().toString());
				}
				if (rci.channelInfo() == null ||
					rci.channelInfo().securityProtocol() == null ||
					rci.channelInfo().securityProtocol().isEmpty())
				{
					channelInformation.securityProtocol("unavailable");
				}
				else {
					channelInformation.securityProtocol(rci.channelInfo().securityProtocol());
				}
			}
			
			Channel channel = reactorChannel.channel();
			
			if (channel != null) {
				channelInformation.channelState(channel.state());
				channelInformation.connectionType(channel.connectionType());
				channelInformation.protocolType(channel.protocolType());
				channelInformation.majorVersion(channel.majorVersion());
				channelInformation.minorVersion(channel.minorVersion());
				channelInformation.pingTimeout(channel.pingTimeout());
				channelInformation.encryptedConnectionType(channel.encryptedConnectionType());
			}
			else {
				channelInformation.channelState(ChannelState.INACTIVE);
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
			if(consumerSession() != null)
			{
				 // Applies for all session channels for the preferred host feature
				 for(SessionChannelInfo<OmmConsumerClient> sessionChannelInfo : consumerSession().sessionChannelList())
				 {
					 if(sessionChannelInfo.reactorChannel() != null)
					 {
						 super.modifyIOCtl(code, value, sessionChannelInfo.reactorChannel());
					 }
				 }
			}
			else
			{
				 ReactorChannel reactorChannel = _loginCallbackClient.activeChannelInfo() != null ?
						 _loginCallbackClient.activeChannelInfo().rsslReactorChannel() : null;
				
				super.modifyIOCtl(code, value, reactorChannel);
			}
		}
		finally
		{
			super.userLock().unlock();
		}
	}
	
	public void renewOAuthCredentials(OAuth2CredentialRenewal credentials)
	{
		super.userLock().lock();
		
		try
		{
			int ret;
			if(_inOAuth2Callback == false)
			{
				
				strBuilder().append("Cannot call submitOAuthCredentialRenewal outside of a callback.");
				handleInvalidUsage(_strBuilder.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
				return;	
			}
			
			boolean noActiveChannel = false;
			
			if(consumerSession() != null)
			{
				noActiveChannel = !consumerSession().hasActiveChannel();
			}
			else
			{
				noActiveChannel = ( _loginCallbackClient.activeChannelInfo() == null ||  _loginCallbackClient.activeChannelInfo().rsslReactorChannel() == null);
			}
			
			if(noActiveChannel)
			{
				strBuilder().append("No active channel to submit credential change.");
				handleInvalidUsage(_strBuilder.toString(), OmmInvalidUsageException.ErrorCode.NO_ACTIVE_CHANNEL);
				return;
			}
			
			ReactorOAuthCredentialRenewal creds = ((OAuth2CredentialRenewalImpl)credentials).getReactorOAuthCredentialRenewal();
			
			if(creds.newPassword().length() != 0)
			{
				_OAuthRenewalOpts.renewalModes(RenewalModes.PASSWORD_CHANGE);
			}
			else
			{
				_OAuthRenewalOpts.renewalModes(RenewalModes.PASSWORD);
			}
			
			ret =  _rsslReactor.submitOAuthCredentialRenewal(_OAuthRenewalOpts, creds, _rsslErrorInfo);
			if(ret != TransportReturnCodes.SUCCESS)
			{
				strBuilder().append("Failed to update OAuth credentials. Error text: ")
				.append(_rsslErrorInfo.error().text());
				handleInvalidUsage(_strBuilder.toString(), OmmInvalidUsageException.ErrorCode.FAILURE);
				return;
			}
		}
		finally
		{
			super.userLock().unlock();
		}
	}
	
	public int reactorOAuthCredentialEventCallback(ReactorOAuthCredentialEvent reactorOAuthCredentialEvent)
	{
		_inOAuth2Callback = true;
		
		_OAuthEvent._closure = _adminClosure;
		
		_OAuthConsumerClient.onOAuth2CredentialRenewal(_OAuthEvent);
		
		return ReactorCallbackReturnCodes.SUCCESS;
	}
	
	private void populateChannelInfomation(ChannelInformationImpl channelInfoImpl, ReactorChannel reactorChannel, ChannelInfo channelInfo)
	{
		if (reactorChannel != null) {
			channelInfoImpl.set(reactorChannel);
			
			if(channelInfo != null)
			{
				SessionChannelInfo<OmmConsumerClient> sessionChannelInfo = channelInfo.sessionChannelInfo();
				
				channelInfoImpl.channelName(channelInfo._channelConfig.name);
				
				if(sessionChannelInfo != null)
				{
					channelInfoImpl.sessionChannelName(sessionChannelInfo.sessionChannelConfig().name);
				}
			}
			
			channelInfoImpl.ipAddress("not available for OmmConsumer connections");
			channelInfoImpl.port(reactorChannel.port());
		}
	}

	@Override
	public void sessionChannelInfo(List<ChannelInformation> sessionChannelInfo)
	{
		if(sessionChannelInfo == null)
			return;
		
		userLock().lock();
		
		try
		{
		
			sessionChannelInfo.clear();
			
			ChannelInfo channelInfo = null;
			ChannelInformationImpl channelInfoImpl;
			
			if(consumerSession() != null)
			{
				List<SessionChannelInfo<OmmConsumerClient>> sessionChannelInfoList = consumerSession().sessionChannelList();
				
				for(SessionChannelInfo<OmmConsumerClient> sessionChInfo : sessionChannelInfoList)
				{
					ReactorChannel reactorChannel = sessionChInfo.reactorChannel();
					
					if(reactorChannel != null)
					{
						channelInfo = (ChannelInfo)reactorChannel.userSpecObj();
						
						if(channelInfo != null)
						{
							channelInfoImpl = new ChannelInformationImpl();
							
							populateChannelInfomation(channelInfoImpl, reactorChannel, channelInfo);
							
							sessionChannelInfo.add(channelInfoImpl);
						}
					}
				}
			}
		}
		finally
		{
			userLock().unlock();
		}
	}
}
