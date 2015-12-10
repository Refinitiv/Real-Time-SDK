/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
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

using namespace thomsonreuters::ema::access;

void OmmConsumerImpl::downloadDictionary()
{
	UInt64 timeOutLengthInMicroSeconds = _activeConfig.dictionaryRequestTimeOut * 1000;
	_eventTimedOut = false;
	TimeOut* loginWatcher = new TimeOut ( *this, timeOutLengthInMicroSeconds, &OmmBaseImpl::terminateIf, reinterpret_cast< void * >( this ), true );
	while ( !_atExit && ! _eventTimedOut && !_pDictionaryCallbackClient->isDictionaryReady() )
		rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread );

	if ( _eventTimedOut )
	{
		EmaString failureMsg( "dictionary retrieval failed (timed out after waiting " );
		failureMsg.append( _activeConfig.dictionaryRequestTimeOut ).append( " milliseconds) for " );
		ChannelConfig *pChannelcfg = _activeConfig.configChannelSet[0];
		if ( pChannelcfg->getType() == ChannelConfig::SocketChannelEnum )
		{
			SocketChannelConfig * channelConfig( reinterpret_cast< SocketChannelConfig * >( pChannelcfg ) );
			failureMsg.append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append(")");
		}
		if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, failureMsg);
		throwIueException( failureMsg );
		return;
	}
	else
		loginWatcher->cancel();
}

void OmmConsumerImpl::downloadDirectory()
{
	UInt64 timeOutLengthInMicroSeconds = _activeConfig.directoryRequestTimeOut * 1000;
	_eventTimedOut = false;
	TimeOut* loginWatcher = new TimeOut ( *this, timeOutLengthInMicroSeconds, &OmmBaseImpl::terminateIf, reinterpret_cast< void * >( this ), true );
	while ( ! _atExit && ! _eventTimedOut && ( _state < DirectoryStreamOpenOkEnum ) )
		rsslReactorDispatchLoop( _activeConfig.dispatchTimeoutApiThread, _activeConfig.maxDispatchCountApiThread );

	if ( _eventTimedOut )
	{
		EmaString failureMsg( "directory retrieval failed (timed out after waiting " );
		failureMsg.append( _activeConfig.directoryRequestTimeOut ).append( " milliseconds) for " );
		ChannelConfig *pChannelcfg = _activeConfig.configChannelSet[0];
		if ( pChannelcfg->getType() == ChannelConfig::SocketChannelEnum )
		{
			SocketChannelConfig * channelConfig( reinterpret_cast< SocketChannelConfig * >( pChannelcfg ) );
			failureMsg.append( channelConfig->hostName ).append( ":" ).append( channelConfig->serviceName ).append(")");
		}
		if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, failureMsg);
		throwIueException( failureMsg );
		return;
	}
	else
		loginWatcher->cancel();
}

void OmmConsumerImpl::setRsslReactorChannelRole( RsslReactorChannelRole& role )
{
	RsslReactorOMMConsumerRole& consumerRole = role.ommConsumerRole;
	rsslClearOMMConsumerRole( &consumerRole );
	consumerRole.pLoginRequest = getLoginCallbackClient().getLoginRequest();
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
}

OmmConsumerImpl::OmmConsumerImpl( const OmmConsumerConfig& config ) :
 OmmBaseImpl( _activeConfig )
{
	OmmBaseImpl::initialize( config._pImpl );
}

OmmConsumerImpl::OmmConsumerImpl( const OmmConsumerConfig& config, OmmConsumerErrorClient& client ) :
 OmmBaseImpl( _activeConfig, client )
{
	OmmBaseImpl::initialize( config._pImpl );
}

OmmConsumerImpl::~OmmConsumerImpl()
{
	uninitialize();
}

void OmmConsumerImpl::uninitialize( bool caughtExcep )
{
	OmmBaseImpl::uninitialize();
}

void OmmConsumerImpl::addSocket( RsslSocket fd )
{
#ifdef USING_SELECT	
	FD_SET( fd, &_readFds );
	FD_SET( fd, &_exceptFds );
#else
	addFd( fd , POLLIN|POLLERR|POLLHUP);
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

Int64 OmmConsumerImpl::dispatch( Int64 timeOut )
{
	if ( _activeConfig.userDispatch == OmmConsumerConfig::UserDispatchEnum && !_atExit )
	  return rsslReactorDispatchLoop( timeOut, _activeConfig.maxDispatchCountUserThread ) ? OmmConsumer::DispatchedEnum : OmmConsumer::TimeoutEnum;

	return OmmConsumer::TimeoutEnum;
}

UInt64 OmmConsumerImpl::registerClient( const ReqMsg& reqMsg, OmmConsumerClient& ommConsClient,
					void* closure, UInt64 parentHandle )
{
	_theClient = new OmmClient< OmmConsumerClient >( &ommConsClient );
	_userLock.lock();

	UInt64 handle = _pItemCallbackClient ? _pItemCallbackClient->registerClient( reqMsg, _theClient, closure, parentHandle ) : 0;

	_userLock.unlock();

	return handle;
}

UInt64 OmmConsumerImpl::registerClient( const TunnelStreamRequest& tunnelStreamRequest,
                                                                           OmmConsumerClient& ommConsClient, void* closure )
{
	_theClient = new OmmClient< OmmConsumerClient >( &ommConsClient );
    _userLock.lock();

    UInt64 handle = _pItemCallbackClient ? _pItemCallbackClient->registerClient( tunnelStreamRequest, _theClient, closure ) : 0;

    _userLock.unlock();

    return handle;
}

RsslReactorCallbackRet OmmConsumerImpl::tunnelStreamStatusEventCallback( RsslTunnelStream* pTunnelStream, RsslTunnelStreamStatusEvent* pTunnelStreamStatusEvent )
{
	return static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getImpl().getItemCallbackClient().processCallback( pTunnelStream, pTunnelStreamStatusEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::tunnelStreamDefaultMsgCallback( RsslTunnelStream* pTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent )
{
	return static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getImpl().getItemCallbackClient().processCallback( pTunnelStream, pTunnelStreamMsgEvent );
}

RsslReactorCallbackRet OmmConsumerImpl::tunnelStreamQueueMsgCallback( RsslTunnelStream* pTunnelStream, RsslTunnelStreamQueueMsgEvent* pTunnelStreamQueueMsgEvent )
{
	return static_cast<TunnelItem*>( pTunnelStream->userSpecPtr )->getImpl().getItemCallbackClient().processCallback( pTunnelStream, pTunnelStreamQueueMsgEvent );
}

