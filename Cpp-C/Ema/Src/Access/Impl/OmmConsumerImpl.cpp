/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2022 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumer.h"
#include "OmmConsumerImpl.h"
#include "OmmConsumerConfigImpl.h"
#include "ExceptionTranslator.h"
#include "OmmException.h"
#include "rtr/rsslReactor.h"
#include "DirectoryCallbackClient.h"
#include "LoginCallbackClient.h"
#include "ChannelInfoImpl.h"
#include "ChannelStatsImpl.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

OmmConsumerImpl::OmmConsumerImpl( const OmmConsumerConfig& config ) :
	OmmBaseImpl( _activeConfig )
{
	_activeConfig.operationModel = config._pImpl->operationModel();
	OmmBaseImpl::initialize( config._pImpl );
}

OmmConsumerImpl::OmmConsumerImpl(const OmmConsumerConfig& config, OmmConsumerClient& adminClient, void* adminClosure) :
	OmmBaseImpl(_activeConfig, adminClient, adminClosure)
{
	_activeConfig.operationModel = config._pImpl->operationModel();
	OmmBaseImpl::initialize(config._pImpl);
}

OmmConsumerImpl::OmmConsumerImpl(const OmmConsumerConfig& config, OmmOAuth2ConsumerClient& oAuthClient, void* adminClosure) :
	OmmBaseImpl(_activeConfig, oAuthClient, adminClosure)
{
	_activeConfig.operationModel = config._pImpl->operationModel();
	OmmBaseImpl::initialize(config._pImpl);
}

OmmConsumerImpl::OmmConsumerImpl(const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmOAuth2ConsumerClient& oAuthClient, void* adminClosure) :
	OmmBaseImpl(_activeConfig, adminClient, oAuthClient, adminClosure)
{
	_activeConfig.operationModel = config._pImpl->operationModel();
	OmmBaseImpl::initialize(config._pImpl);
}

OmmConsumerImpl::OmmConsumerImpl( const OmmConsumerConfig& config, OmmConsumerErrorClient& client ) :
	OmmBaseImpl( _activeConfig, client )
{
	_activeConfig.operationModel = config._pImpl->operationModel();
	OmmBaseImpl::initialize( config._pImpl );
}

OmmConsumerImpl::OmmConsumerImpl(const OmmConsumerConfig& config, OmmOAuth2ConsumerClient& oAuthClient, OmmConsumerErrorClient& client, void* adminClosure) :
	OmmBaseImpl(_activeConfig, oAuthClient, client, adminClosure)
{
	_activeConfig.operationModel = config._pImpl->operationModel();
	OmmBaseImpl::initialize(config._pImpl);
}

OmmConsumerImpl::OmmConsumerImpl(const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmConsumerErrorClient& errorClient, void* adminClosure ) :
	OmmBaseImpl(_activeConfig, adminClient, errorClient, adminClosure)
{
	_activeConfig.operationModel = config._pImpl->operationModel();
	OmmBaseImpl::initialize(config._pImpl);
}

OmmConsumerImpl::OmmConsumerImpl(const OmmConsumerConfig& config, OmmConsumerClient& adminClient, OmmOAuth2ConsumerClient& oAuthClient, OmmConsumerErrorClient& errorClient, void* adminClosure) :
	OmmBaseImpl(_activeConfig, adminClient, oAuthClient, errorClient, adminClosure)
{
	_activeConfig.operationModel = config._pImpl->operationModel();
	OmmBaseImpl::initialize(config._pImpl);
}

//only for unit test, internal use
OmmConsumerImpl::OmmConsumerImpl(const OmmConsumerConfig& config, bool unitTest) :
	OmmBaseImpl(_activeConfig)
{
	if (unitTest)
	{
		_activeConfig.operationModel = config._pImpl->operationModel();
		OmmBaseImpl::initializeForTest(config._pImpl);
	}
	else
	{
		_activeConfig.operationModel = config._pImpl->operationModel();
		OmmBaseImpl::initialize(config._pImpl);
	}
}

OmmConsumerImpl::~OmmConsumerImpl()
{
	OmmBaseImplMap<OmmBaseImpl>::acquireCleanupLock();

	uninitialize( false, false );

	OmmBaseImplMap<OmmBaseImpl>::releaseCleanupLock();
}

void OmmConsumerImpl::readCustomConfig( EmaConfigImpl* pConfigImpl )
{
	pConfigImpl->getDictionaryName( _activeConfig.configuredName, _activeConfig.dictionaryConfig.dictionaryName );

	if ( _activeConfig.dictionaryConfig.dictionaryName.empty() )
	{
		_activeConfig.dictionaryConfig.dictionaryName.set( "Dictionary" );
		_activeConfig.dictionaryConfig.dictionaryType = Dictionary::ChannelDictionaryEnum;
		_activeConfig.dictionaryConfig.enumtypeDefFileName.clear();
		_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName.clear();
	}
	else
	{
		EmaString dictionaryNodeName( "DictionaryGroup|DictionaryList|Dictionary." );
		dictionaryNodeName.append( _activeConfig.dictionaryConfig.dictionaryName ).append( "|" );

		EmaString name;
		if ( !pConfigImpl->get< EmaString >( dictionaryNodeName + "Name", name ) )
		{
			EmaString errorMsg( "no configuration exists in the config file for consumer dictionary [" );
			errorMsg.append( dictionaryNodeName ).append( "]; will use dictionary defaults if not config programmatically" );
			pConfigImpl->appendConfigError( errorMsg, OmmLoggerClient::VerboseEnum );
		}

		if ( !pConfigImpl->get<Dictionary::DictionaryType>( dictionaryNodeName + "DictionaryType", _activeConfig.dictionaryConfig.dictionaryType ) )
			_activeConfig.dictionaryConfig.dictionaryType = Dictionary::ChannelDictionaryEnum;

		if ( _activeConfig.dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum )
		{
			if ( !pConfigImpl->get<EmaString>( dictionaryNodeName + "RdmFieldDictionaryFileName", _activeConfig.dictionaryConfig.rdmfieldDictionaryFileName ) )
				_activeConfig.dictionaryConfig.rdmfieldDictionaryFileName.set( "./RDMFieldDictionary" );
			if ( !pConfigImpl->get<EmaString>( dictionaryNodeName + "EnumTypeDefFileName", _activeConfig.dictionaryConfig.enumtypeDefFileName ) )
				_activeConfig.dictionaryConfig.enumtypeDefFileName.set( "./enumtype.def" );
		}
	}

	if ( ProgrammaticConfigure* ppc = pConfigImpl->getProgrammaticConfigure() )
	{
		ppc->retrieveDictionaryConfig( _activeConfig.dictionaryConfig.dictionaryName, _activeConfig );
	}

	const UInt32 maxUInt32( 0xFFFFFFFF );
	UInt64 tmp;
	EmaString instanceNodeName( pConfigImpl->getInstanceNodeName() );
	instanceNodeName.append( _activeConfig.configuredName ).append( "|" );

	if ( pConfigImpl->get<UInt64>( instanceNodeName + "ObeyOpenWindow", tmp ) )
		_activeConfig.obeyOpenWindow = static_cast<UInt32>( tmp > 0 ? 1 : 0 );

	if ( pConfigImpl->get<UInt64>( instanceNodeName + "PostAckTimeout", tmp ) )
		_activeConfig.postAckTimeout = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );

	if ( pConfigImpl->get< UInt64 >( instanceNodeName + "DictionaryRequestTimeOut", tmp ) )
		_activeConfig.dictionaryRequestTimeOut = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );

	if ( pConfigImpl->get< UInt64 >( instanceNodeName + "DirectoryRequestTimeOut", tmp ) )
		_activeConfig.directoryRequestTimeOut = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );

	if ( pConfigImpl->get<UInt64>( instanceNodeName + "MaxOutstandingPosts", tmp ) )
		_activeConfig.maxOutstandingPosts = static_cast<UInt32>( tmp > maxUInt32 ? maxUInt32 : tmp );

	_activeConfig.pRsslDirectoryRequestMsg = pConfigImpl->getDirectoryReq();

	_activeConfig.pRsslEnumDefRequestMsg = pConfigImpl->getEnumDefDictionaryReq();

	_activeConfig.pRsslRdmFldRequestMsg = pConfigImpl->getRdmFldDictionaryReq();

	if ( ProgrammaticConfigure* ppc = pConfigImpl->getProgrammaticConfigure() )
	{
		ppc->retrieveCustomConfig( _activeConfig.configuredName, _activeConfig );
	}
}

void OmmConsumerImpl::loadDictionary()
{
	if (getActiveConfig().dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum)
	{
		if (!_atExit)
		{
			_pDictionaryCallbackClient->loadDictionaryFromFile();
		}
	}
	else
	{
		UInt64 timeOutLengthInMicroSeconds = _activeConfig.dictionaryRequestTimeOut * 1000;
		_eventTimedOut = false;

		TimeOut* pWatcher = 0;

		try {
			pWatcher = new TimeOut(*this, timeOutLengthInMicroSeconds, &OmmBaseImpl::terminateIf, reinterpret_cast<void*>(this), true);
		}
		catch (std::bad_alloc&) {
			throwMeeException("Failed to allocate memory in OmmConsumerImpl::loadDictionary().");
			return;
		}

		while (!_atExit && !_eventTimedOut && !_pDictionaryCallbackClient->isDictionaryReady())
			rsslReactorDispatchLoop(_activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread, _bEventReceived);

		if (_eventTimedOut)
		{
			Channel *pChannel = _pLoginCallbackClient->getActiveChannel();
			ChannelConfig* pChannelcfg = NULL;
			if (pChannel != NULL && pChannel->getReactorChannelType() == Channel::NORMAL)
			{
				pChannelcfg = _activeConfig.findChannelConfig(pChannel);
				if (!pChannelcfg && _activeConfig.configChannelSet.size() > 0)
					pChannelcfg = _activeConfig.configChannelSet[_activeConfig.configChannelSet.size() - 1];
			}

			EmaString failureMsg("dictionary retrieval failed (timed out after waiting ");
			failureMsg.append(_activeConfig.dictionaryRequestTimeOut).append(" milliseconds)");
			if (pChannelcfg != NULL &&  pChannelcfg->getType() == ChannelConfig::SocketChannelEnum)
			{
				SocketChannelConfig* channelConfig(reinterpret_cast<SocketChannelConfig*>(pChannelcfg));
				failureMsg.append(" for ").append(channelConfig->hostName).append(":").append(channelConfig->serviceName).append(")");
			}
			if (OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity)
				_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, failureMsg);
			throwIueException(failureMsg, OmmInvalidUsageException::DictionaryRequestTimeOutEnum);
			return;
		}
		else
		{
			if (timeOutLengthInMicroSeconds != 0) pWatcher->cancel();
		}
	}

	if (_atExit)
	{
		throwIueException("Application or user initiated exit while waiting for dictionary response.", OmmInvalidUsageException::InvalidOperationEnum);
	}
}

void OmmConsumerImpl::loadDirectory()
{
	UInt64 timeOutLengthInMicroSeconds = _activeConfig.directoryRequestTimeOut * 1000;
	_eventTimedOut = false;

	TimeOut* pWatcher = 0;
	
	try {
		pWatcher = new TimeOut( *this, timeOutLengthInMicroSeconds, &OmmBaseImpl::terminateIf, reinterpret_cast< void* >( this ), true );
	}
	catch ( std::bad_alloc& ) {
		throwMeeException( "Failed to allocate memory in OmmConsumerImpl::downloadDirectory()." );
		return;
	}
	
	while ( ! _atExit && ! _eventTimedOut && ( _state < DirectoryStreamOpenOkEnum ) )
		rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread, _bEventReceived );

	if ( _eventTimedOut )
	{
		Channel *pChannel = _pLoginCallbackClient->getActiveChannel();
		ChannelConfig* pChannelcfg = NULL;
		if (pChannel != NULL && pChannel->getReactorChannelType() == Channel::NORMAL)
		{
			pChannelcfg = _activeConfig.findChannelConfig(pChannel);
			if (!pChannelcfg && _activeConfig.configChannelSet.size() > 0)
				pChannelcfg = _activeConfig.configChannelSet[_activeConfig.configChannelSet.size() - 1];
		}

		EmaString failureMsg( "directory retrieval failed (timed out after waiting " );
		failureMsg.append( _activeConfig.directoryRequestTimeOut ).append( " milliseconds)" );
		if (pChannelcfg != NULL && pChannelcfg->getType() == ChannelConfig::SocketChannelEnum )
		{
			SocketChannelConfig* channelConfig( reinterpret_cast< SocketChannelConfig* >( pChannelcfg ) );
			failureMsg.append(" for ").append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append( ")" );
		}
		if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			_pLoggerClient->log( _activeConfig.instanceName, OmmLoggerClient::ErrorEnum, failureMsg );
		throwIueException( failureMsg, OmmInvalidUsageException::DirectoryRequestTimeOutEnum );
		return;
	}
	else
	{
		if ( timeOutLengthInMicroSeconds != 0 ) pWatcher->cancel();
	}

	if ( _atExit )
	{
		throwIueException( "Application or user initiated exit while waiting for directory response.", OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void OmmConsumerImpl::reLoadDirectory()
{
}

void OmmConsumerImpl::processChannelEvent( RsslReactorChannelEvent* )
{
}

void OmmConsumerImpl::setRsslReactorChannelRole( RsslReactorChannelRole& role)
{
	RsslReactorOMMConsumerRole& consumerRole = role.ommConsumerRole;
	rsslClearOMMConsumerRole( &consumerRole );
	if (_LoginRequestMsgs.size() == 1)
	{
		consumerRole.pLoginRequest = getLoginCallbackClient().getLoginRequest();
	}
	else
	{
		consumerRole.pLoginRequestList = _LoginReactorConfig;
		consumerRole.loginRequestMsgCredentialCount = _LoginRequestMsgs.size();
	}
	consumerRole.pOAuthCredentialList = _OAuthReactorConfig;
	consumerRole.oAuthCredentialCount = _oAuth2Credentials.size();
	consumerRole.pDirectoryRequest = getDirectoryCallbackClient().getDirectoryRequest();
	consumerRole.dictionaryDownloadMode = RSSL_RC_DICTIONARY_DOWNLOAD_NONE;

	consumerRole.loginMsgCallback = OmmBaseImpl::loginCallback;
	consumerRole.directoryMsgCallback = OmmBaseImpl::directoryCallback;
	consumerRole.dictionaryMsgCallback = OmmBaseImpl::dictionaryCallback;
	consumerRole.base.channelEventCallback = OmmBaseImpl::channelCallback;
	consumerRole.base.defaultMsgCallback = OmmBaseImpl::itemCallback;
	consumerRole.watchlistOptions.channelOpenCallback = OmmBaseImpl::channelOpenCallback;

	consumerRole.watchlistOptions.enableWatchlist = RSSL_TRUE;
	consumerRole.watchlistOptions.itemCountHint = getActiveConfig().itemCountHint;
	consumerRole.watchlistOptions.obeyOpenWindow = getActiveConfig().obeyOpenWindow > 0 ? RSSL_TRUE : RSSL_FALSE;
	consumerRole.watchlistOptions.postAckTimeout = getActiveConfig().postAckTimeout;
	consumerRole.watchlistOptions.requestTimeout = getActiveConfig().requestTimeout;
	consumerRole.watchlistOptions.maxOutstandingPosts = getActiveConfig().maxOutstandingPosts;

	if (getActiveConfig().enableRtt)
	{
		consumerRole.pLoginRequest->flags |= RDM_LG_RQF_RTT_SUPPORT;
	}
}

void OmmConsumerImpl::addSocket( RsslSocket fd )
{
#ifdef USING_SELECT
	FD_SET( fd, &_readFds );
	FD_SET( fd, &_exceptFds );
#else
	addFd( fd , POLLIN | POLLERR | POLLHUP );
#endif
}

void OmmConsumerImpl::removeSocket( RsslSocket fd )
{
#ifdef USING_SELECT
	FD_CLR( fd, &_readFds );
	FD_CLR( fd, &_exceptFds );
#else
	removeFd( fd );
#endif
}

void OmmConsumerImpl::removeAllSocket()
{
#ifdef USING_SELECT
	FD_ZERO(&_readFds);
	FD_ZERO(&_exceptFds);
#else
	removeAllFd();
#endif
}

Int64 OmmConsumerImpl::dispatch( Int64 timeOut )
{
	if ( _activeConfig.operationModel == OmmConsumerConfig::UserDispatchEnum && !_atExit )
		return rsslReactorDispatchLoop( timeOut, _activeConfig.maxDispatchCountUserThread, _bMsgDispatched );

	return OmmConsumer::TimeoutEnum;
}

UInt64 OmmConsumerImpl::registerClient( const ReqMsg& reqMsg, OmmConsumerClient& ommConsClient,
                                        void* closure, UInt64 parentHandle )
{
	try
	{
		_userLock.lock();

		UInt64 handle = _pItemCallbackClient ? _pItemCallbackClient->registerClient(reqMsg, ommConsClient, closure, parentHandle) : 0;

		_userLock.unlock();
		return handle;
	}
	catch (...)
	{
		_userLock.unlock();
		throw;
	}
}

UInt64 OmmConsumerImpl::registerClient( const TunnelStreamRequest& tunnelStreamRequest,
                                        OmmConsumerClient& ommConsClient, void* closure )
{
	try
	{
		_userLock.lock();

		UInt64 handle = _pItemCallbackClient ? _pItemCallbackClient->registerClient(tunnelStreamRequest, ommConsClient, closure) : 0;

		_userLock.unlock();
		return handle;
	}
	catch (...)
	{
		_userLock.unlock();
		throw;
	}
}

RsslReactorCallbackRet OmmConsumerImpl::tunnelStreamStatusEventCallback( RsslTunnelStream* pTunnelStream, RsslTunnelStreamStatusEvent* pTunnelStreamStatusEvent )
{
	static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getImpl().eventReceived();
	return static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getImpl().getItemCallbackClient().processCallback( pTunnelStream, pTunnelStreamStatusEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::tunnelStreamDefaultMsgCallback( RsslTunnelStream* pTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent )
{
	static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getImpl().eventReceived();
	return static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getImpl().getItemCallbackClient().processCallback( pTunnelStream, pTunnelStreamMsgEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::tunnelStreamQueueMsgCallback( RsslTunnelStream* pTunnelStream, RsslTunnelStreamQueueMsgEvent* pTunnelStreamQueueMsgEvent )
{
	static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getImpl().eventReceived();
	return static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getImpl().getItemCallbackClient().processCallback( pTunnelStream, pTunnelStreamQueueMsgEvent );
}

void OmmConsumerImpl::createDictionaryCallbackClient( DictionaryCallbackClient*& dictionaryCallbackClient, OmmBaseImpl& impl )
{
	dictionaryCallbackClient = DictionaryCallbackClient::create( impl );
	dictionaryCallbackClient->initialize();
}

void OmmConsumerImpl::createDirectoryCallbackClient( DirectoryCallbackClient*& directoryCallbackClient, OmmBaseImpl& impl )
{
	directoryCallbackClient = DirectoryCallbackClient::create( impl );
	directoryCallbackClient->initialize();
}

bool OmmConsumerImpl::isApiDispatching() const
{
	return _activeConfig.operationModel == OmmConsumerConfig::ApiDispatchEnum ? true : false;
}

OmmCommonImpl::ImplementationType OmmConsumerImpl::getImplType()
{
	return OmmCommonImpl::ConsumerEnum;
}

void OmmConsumerImpl::getChannelInformation(ChannelInformation& ci) {
  Channel* pChannel;
  if (_state == NotInitializedEnum ||
	  (pChannel = getLoginCallbackClient().getActiveChannel()) == 0) {
	ci.clear();
	return;
  }
  return getChannelInformationImpl(pChannel->getRsslChannel(), OmmCommonImpl::ConsumerEnum, ci);
}

void OmmConsumerImpl::getChannelStatistics(ChannelStatistics& cs) {
	Channel* pChannel;
	RsslErrorInfo rsslErrorInfo;
	RsslReactorChannelStats rsslReactorChannelStats;

	cs.clear();

	_userLock.lock();

	if (_state == NotInitializedEnum ||
		(pChannel = getLoginCallbackClient().getActiveChannel()) == 0) {
		_userLock.unlock();
		EmaString temp("No active channel to getChannelStatistics.");
		handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
		return;
	}

	// if channel is closed, rsslReactorGetChannelStats does not succeed
	if (rsslReactorGetChannelStats(const_cast<RsslReactorChannel*>(pChannel->getRsslChannel()),
		&rsslReactorChannelStats, &rsslErrorInfo) != RSSL_RET_SUCCESS)
	{
		_userLock.unlock();
		EmaString temp("Call to rsslReactorGetChannelStats() failed. Internal sysError='");
		temp.append(rsslErrorInfo.rsslError.sysError)
			.append("' Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append("' ")
			.append("' Error Location='").append(rsslErrorInfo.errorLocation).append("' ")
			.append("' Error text='").append(rsslErrorInfo.rsslError.text).append("'. ");
		handleIue(temp, OmmInvalidUsageException::InvalidOperationEnum);
		return;
	}

	if (rsslReactorChannelStats.rsslChannelStats.tcpStats.flags & RSSL_TCP_STATS_RETRANSMIT)
	{
		cs.tcpRetransmitCount(rsslReactorChannelStats.rsslChannelStats.tcpStats.tcpRetransmitCount);
	}

	_userLock.unlock();
}

void OmmConsumerImpl::modifyIOCtl(Int32 code, Int32 value)
{
	_userLock.lock();

	Channel* pChannel;
	if ( ( pChannel = getLoginCallbackClient().getActiveChannel() ) == NULL || ( pChannel->getRsslChannel() == NULL ) ) 
	{
		_userLock.unlock();
		EmaString temp("No active channel to modify I/O option.");
		handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);
		return;
	}

	RsslError rsslError;
	RsslRet ret = rsslIoctl( pChannel->getRsslChannel()->pRsslChannel, (RsslIoctlCodes)code, &value, &rsslError );

	if (ret != RSSL_RET_SUCCESS)
	{
		_userLock.unlock();
		EmaString temp("Failed to modify I/O option for code = ");
			temp.append(code).append(".").append(CR)
			.append("RsslChannel ").append(ptrToStringAsHex(rsslError.channel)).append(CR)
			.append("Error Id ").append(rsslError.rsslErrorId).append(CR)
			.append("Internal sysError ").append(rsslError.sysError).append(CR)
			.append("Error Text ").append(rsslError.text);
		handleIue(temp, ret);
		return;
	}

	_userLock.unlock();
}
void OmmConsumerImpl::renewOAuth2Credentials(OAuth2CredentialRenewal& credentials) {
	RsslErrorInfo rsslErrorInfo;
	RsslReactorOAuthCredentialRenewal credentialRenewal;
	RsslReactorOAuthCredentialRenewalOptions renewalOpts;
	RsslRet ret;

	_userLock.lock();

	if (getInOAuthCallback() == false)
	{
		_userLock.unlock();
		EmaString temp("Cannot call OmmConsumer::renewOAuth2Credentials outside of an OmmOAuth2ConsumerClient callback.");
		handleIue(temp, OmmInvalidUsageException::InvalidOperationEnum);
		return;
	}

	rsslClearReactorOAuthCredentialRenewal(&credentialRenewal);
	rsslClearReactorOAuthCredentialRenewalOptions(&renewalOpts);

	renewalOpts.renewalMode = RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD;

	if (!credentials.getUserName().empty())
	{
		credentialRenewal.userName.data = (char*)credentials.getUserName().c_str();
		credentialRenewal.userName.length = credentials.getUserName().length();
	}

	if (!credentials.getPassword().empty())
	{
		credentialRenewal.password.data = (char*)credentials.getPassword().c_str();
		credentialRenewal.password.length = credentials.getPassword().length();
	}

	if (!credentials.getNewPassword().empty())
	{
		credentialRenewal.newPassword.data = (char*)credentials.getNewPassword().c_str();
		credentialRenewal.newPassword.length = credentials.getNewPassword().length();
		renewalOpts.renewalMode = RSSL_ROC_RT_RENEW_TOKEN_WITH_PASSWORD_CHANGE;
	}

	if (!credentials.getClientId().empty())
	{
		credentialRenewal.clientId.data = (char*)credentials.getClientId().c_str();
		credentialRenewal.clientId.length = credentials.getClientId().length();
	}

	if (!credentials.getClientSecret().empty())
	{
		credentialRenewal.clientSecret.data = (char*)credentials.getClientSecret().c_str();
		credentialRenewal.clientSecret.length = credentials.getClientSecret().length();
	}

	if (!credentials.getClientJWK().empty())
	{
		credentialRenewal.clientJWK.data = (char*)credentials.getClientJWK().c_str();
		credentialRenewal.clientJWK.length = credentials.getClientJWK().length();
	}

	if (!credentials.getTokenScope().empty())
	{
		credentialRenewal.tokenScope.data = (char*)credentials.getTokenScope().c_str();
		credentialRenewal.tokenScope.length = credentials.getTokenScope().length();
	}

	credentialRenewal.takeExclusiveSignOnControl = credentials.getTakeExclusiveSignOnControl();


	ret = rsslReactorSubmitOAuthCredentialRenewal(_pRsslReactor, &renewalOpts, &credentialRenewal, &rsslErrorInfo);

	if (ret != RSSL_RET_SUCCESS)
	{
		_userLock.unlock();
		EmaString temp("Failed to renew OAuth credentials.  ");
		temp.append(CR)
			.append("Error Text ").append(rsslErrorInfo.rsslError.text);
		handleIue(temp, ret);
		return;
	}

	_userLock.unlock();
}


void OmmConsumerImpl::renewLoginMsgCredentials(LoginMsgCredentialRenewal& credentials) {
	RsslErrorInfo rsslErrorInfo;
	RsslRet ret;
	RsslReactorLoginCredentialRenewalOptions opts;

	_userLock.lock();

	if (getInLoginCredentialCallback() == false)
	{
		_userLock.unlock();
		EmaString temp("Cannot call OmmConsumer::renewLoginMsgCredentials outside of an OmmOAuth2ConsumerClient callback.");
		handleIue(temp, OmmInvalidUsageException::InvalidOperationEnum);
		return;
	}

	rsslClearReactorLoginCredentialRenewalOptions(&opts);
	// OmmBaseImpl::_tmpLoginMsg was set immediately before this call in the callback.
	if (credentials.getUserName().length() != 0)
	{
		opts.userName.data = const_cast<char*>(credentials.getUserName().c_str());
		opts.userName.length = credentials.getUserName().length();
		_tmpLoginMsg->username(EmaString(credentials.getUserName().c_str(), credentials.getUserName().length()));
	}
	else
	{
		opts.userName = _tmpLoginMsg->get()->userName;
	}

	if (credentials.getAuthenticationExtended().length() != 0)
	{
		opts.authenticationExtended.data = const_cast<char*>(credentials.getAuthenticationExtended().c_buf());
		opts.authenticationExtended.length = credentials.getAuthenticationExtended().length();
		_tmpLoginMsg->authenticationExtended(EmaBuffer(credentials.getAuthenticationExtended().c_buf(), credentials.getAuthenticationExtended().length()));
	}
	else
	{
		opts.authenticationExtended = _tmpLoginMsg->get()->authenticationExtended;
	}

	ret = rsslReactorSubmitLoginCredentialRenewal(_pRsslReactor, _tmpChnl, &opts, &rsslErrorInfo);

	if (ret != RSSL_RET_SUCCESS)
	{
		_userLock.unlock();
		EmaString temp("Failed to renew Login credentials.  ");
		temp.append(CR)
			.append("Error Text ").append(rsslErrorInfo.rsslError.text);
		handleIue(temp, ret);
		return;
	}

	_userLock.unlock();
}