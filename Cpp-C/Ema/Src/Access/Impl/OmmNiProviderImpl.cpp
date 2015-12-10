/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "OmmNiProvider.h"
#include "OmmNiProviderImpl.h"
#include "OmmNiProviderConfigImpl.h"
#include "ExceptionTranslator.h"
#include "OmmException.h"
#include "ChannelCallbackClient.h"
#include "LoginCallbackClient.h"
#include "RefreshMsgEncoder.h"
#include "UpdateMsgEncoder.h"
#include "StatusMsgEncoder.h"
#include "StaticDecoder.h"

#include <iostream>

using namespace thomsonreuters::ema::access;

OmmNiProviderImpl::OmmNiProviderImpl( const OmmNiProviderConfig& config ) :
  OmmBaseImpl( _activeConfig )
{
	initialize( config._pImpl );
}

OmmNiProviderImpl::OmmNiProviderImpl( const OmmNiProviderConfig& config, OmmNiProviderErrorClient& client ) :
  OmmBaseImpl( _activeConfig, client )
{
  OmmBaseImpl::initialize( config._pImpl );
}

OmmNiProviderImpl::~OmmNiProviderImpl() {}

void OmmNiProviderImpl::uninitialize()
{
  OmmBaseImpl::uninitialize();
}

void OmmNiProviderImpl::addSocket( RsslSocket fd )
{
#ifdef USING_SELECT	
	FD_SET( fd, &_readFds );
	FD_SET( fd, &_exceptFds );
#else
	addFd( fd , POLLIN|POLLERR|POLLHUP);
#endif
}

void OmmNiProviderImpl::removeSocket( RsslSocket fd )
{
#ifdef USING_SELECT
	FD_CLR( fd, &_readFds );
	FD_CLR( fd, &_exceptFds );
#else
	removeFd( fd );
#endif
}

Int64 OmmNiProviderImpl::dispatch( Int64 timeOut )
{
  if ( _activeConfig.userDispatch == OmmNiProviderConfig::UserDispatchEnum && !_atExit )
    return rsslReactorDispatchLoop( timeOut, _activeConfig.maxDispatchCountUserThread ) ? OmmNiProvider::DispatchedEnum : OmmNiProvider::TimeoutEnum;

  return OmmNiProvider::TimeoutEnum;
}

UInt64 OmmNiProviderImpl::registerClient( const ReqMsg& reqMsg, OmmNiProviderClient& ommConsClient,
					void* closure, UInt64 parentHandle )
{
  _theClient = new OmmClient< OmmNiProviderClient >( &ommConsClient );
  _userLock.lock();
  
  UInt64 handle = _pItemCallbackClient ? _pItemCallbackClient->registerClient( reqMsg, _theClient, closure, parentHandle ) : 0;
  
  _userLock.unlock();
  
  return handle;
}

void OmmNiProviderImpl::submit( const RefreshMsg& msg, UInt64 handle ) \
{
  ChannelCallbackClient& ccc( getChannelCallbackClient() );
  const ChannelList& channelList ( ccc.getChannelList() );
  const Channel* channel ( channelList.front() );

  Int32 streamId;
  
  Int32* id = _handleToStreamId.find( handle );
  if ( id )
	streamId = *id;
  else
  {
	streamId = -const_cast< Channel* >(channel)->getNextStreamId();
	_handleToStreamId.insert( handle, streamId );
  }

  channel = getChannelCallbackClient().getChannelList().front();
  RsslReactorSubmitMsgOptions submitMsgOpts;
  rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

  submitMsgOpts.pRsslMsg = (RsslMsg*) static_cast< const RefreshMsgEncoder& >( msg.getEncoder() ).getRsslRefreshMsg();

  submitMsgOpts.pRsslMsg->msgBase.streamId = streamId;
  
  if ( submitMsgOpts.pRsslMsg->msgBase.domainType == ema::rdm::MMT_DIRECTORY )
  {
  	decodeServiceNameAndId( channel, &submitMsgOpts.pRsslMsg->msgBase.encDataBody );
  }
  else
  {
  	if ( !(submitMsgOpts.pRsslMsg->refreshMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) )
  	{
  		EmaString serviceName = static_cast< const RefreshMsgEncoder& >( msg.getEncoder() ).getServiceName();
  		UInt64 serviceId;
  		UInt64* id = _serviceNameToServiceId.find( &serviceName );
  		if ( id )
  			serviceId = *id;
  		else
  		{
			EmaString temp( "Failed to match Service Name: " );
			temp.append( serviceName ).append( " to ServiceId. " );
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			throwIueException( temp );
			return;
  		}
  
  		submitMsgOpts.pRsslMsg->refreshMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
  		submitMsgOpts.pRsslMsg->refreshMsg.msgBase.msgKey.serviceId = serviceId;
  	}
  }

  RsslErrorInfo rsslErrorInfo;
  RsslRet ret( rsslReactorSubmitMsg( channel->getRsslReactor(), channel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo ) );
}

void OmmNiProviderImpl::submit( const UpdateMsg& msg, UInt64 handle ) 
{
  ChannelCallbackClient& ccc( getChannelCallbackClient() );
  const ChannelList& channelList ( ccc.getChannelList() );
  const Channel* channel ( channelList.front() );

  Int32 streamId;
  
  Int32* id = _handleToStreamId.find( handle );
  if ( id )
  	streamId = *id;
  else
  {
  	streamId = -const_cast< Channel* >(channel)->getNextStreamId();
  	_handleToStreamId.insert( handle, streamId );
  }

  channel = getChannelCallbackClient().getChannelList().front();
  RsslReactorSubmitMsgOptions submitMsgOpts;
  rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
  submitMsgOpts.pRsslMsg = (RsslMsg*) static_cast< const UpdateMsgEncoder& >( msg.getEncoder() ).getRsslUpdateMsg();

  submitMsgOpts.pRsslMsg->msgBase.streamId = streamId;

  RsslErrorInfo rsslErrorInfo;
  RsslRet ret( rsslReactorSubmitMsg( channel->getRsslReactor(),
				     channel->getRsslChannel(),
				     &submitMsgOpts,
				     &rsslErrorInfo ) );
}

void OmmNiProviderImpl::submit( const StatusMsg& msg, UInt64 handle ) 
{
  ChannelCallbackClient& ccc( getChannelCallbackClient() );
  const ChannelList& channelList ( ccc.getChannelList() );
  const Channel* channel ( channelList.front() );

  Int32 streamId;
  
  Int32* id = _handleToStreamId.find( handle );
  if ( id )
  	streamId = *id;
  else
  {
  	streamId = -const_cast< Channel* >(channel)->getNextStreamId();
  	_handleToStreamId.insert( handle, streamId );
  }

  channel = getChannelCallbackClient().getChannelList().front();
  RsslReactorSubmitMsgOptions submitMsgOpts;
  rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
  submitMsgOpts.pRsslMsg = (RsslMsg*) static_cast< const StatusMsgEncoder& >( msg.getEncoder() ).getRsslStatusMsg();

  submitMsgOpts.pRsslMsg->msgBase.streamId = streamId;

  RsslErrorInfo rsslErrorInfo;
  RsslRet ret( rsslReactorSubmitMsg( channel->getRsslReactor(),
				     channel->getRsslChannel(),
				     &submitMsgOpts,
				     &rsslErrorInfo ) );
}

void OmmNiProviderImpl::setRsslReactorChannelRole( RsslReactorChannelRole& role )
{
  RsslReactorOMMNIProviderRole& niProviderRole = role.ommNIProviderRole;
  rsslClearOMMNIProviderRole( &niProviderRole );
  niProviderRole.pLoginRequest = getLoginCallbackClient().getLoginRequest();
  niProviderRole.pDirectoryRefresh = 0;
  niProviderRole.loginMsgCallback = OmmBaseImpl::loginCallback;
  niProviderRole.base.defaultMsgCallback = OmmBaseImpl::itemCallback;
  niProviderRole.base.channelEventCallback = OmmBaseImpl::channelCallback;
}

 void OmmNiProviderImpl::decodeServiceNameAndId( const Channel* channel, RwfBuffer* buffer )
 {
	RsslRet ret = 0;
	RsslMsg rsslMsg = RSSL_INIT_MSG;
	RsslDecodeIterator dIter;

	rsslClearDecodeIterator( &dIter );

	rsslSetDecodeIteratorRWFVersion( &dIter, channel->getRsslChannel()->majorVersion, channel->getRsslChannel()->minorVersion );

	if( ( ret = rsslSetDecodeIteratorBuffer( &dIter, buffer ) ) != RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to Set Decode Iterator Buffer. Reason='" );
		temp.append( rsslRetCodeToString( ret ) ).append( "'. " );
		if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
		throwIueException( temp );
		return;
	}

	RsslMap map;
	RsslMapEntry mEntry;
	RsslFilterList rsslFilterList;
	RsslFilterEntry filterListItem;
	UInt64 serviceId;

	if ( ( ret = rsslDecodeMap( &dIter, &map ) ) < RSSL_RET_SUCCESS )
	{ 
		EmaString temp( "Failed to Decode Map. Reason='" );
		temp.append( rsslRetCodeToString( ret ) ).append( "'. " );
		if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
		throwIueException( temp );
		return;
	}

	if ( map.keyPrimitiveType != RSSL_DT_UINT )
	{
		EmaString temp( "Expected UInt type for Primitive Type. Received type='" );
		temp.append(  map.keyPrimitiveType ).append( "'. " );
		if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
			_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
		throwIueException( temp );
		return;
	}

	while ( ( ret = rsslDecodeMapEntry( &dIter, &mEntry, &serviceId ) ) != RSSL_RET_END_OF_CONTAINER )
	{
		if ( ret == RSSL_RET_SUCCESS )
		{
			if ( ret != RSSL_RET_SUCCESS && ret != RSSL_RET_BLANK_DATA )
			{
				EmaString temp( "Failed to Decode Map Entry. Reason='" );
				temp.append( rsslRetCodeToString( ret ) ).append( "'. " );
				if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
					_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
				throwIueException( temp );
				return;
			}

			if ( ( ret = rsslDecodeFilterList( &dIter, &rsslFilterList ) ) < RSSL_RET_SUCCESS )
			{
				EmaString temp( "Failed to Decode Filter List. Reason='" );
				temp.append( rsslRetCodeToString( ret ) ).append( "'. " );
				if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
					_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
				throwIueException( temp );
				return;
			}

			while ( ( ret = rsslDecodeFilterEntry( &dIter, &filterListItem ) ) != RSSL_RET_END_OF_CONTAINER )
			{
				if ( ret == RSSL_RET_SUCCESS )
				{
					if ( filterListItem.id == RDM_DIRECTORY_SERVICE_INFO_ID )
					{
						RsslElementList	elementList;
						RsslElementEntry element;

						if ( ( ret = rsslDecodeElementList( &dIter, &elementList, NULL ) ) < RSSL_RET_SUCCESS )
						{
							EmaString temp( "Failed to Decode Element List. Reason='" );
							temp.append( rsslRetCodeToString( ret ) ).append( "'. " );
							if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
								_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
							throwIueException( temp );
							return;
						}

						while ( ( ret = rsslDecodeElementEntry( &dIter, &element ) ) != RSSL_RET_END_OF_CONTAINER )
						{
							if ( ret == RSSL_RET_SUCCESS )
							{
								if ( rsslBufferIsEqual( &element.name, &RSSL_ENAME_NAME ) )
								{
									EmaString* serviceName = new EmaString( element.encData.data, element.encData.length );
									_serviceNameToServiceId.insert( serviceName, serviceId );
								}
							}
							else
							{
								EmaString temp( "Failed to Decode Element Entry. Reason='" );
								temp.append( rsslRetCodeToString( ret ) ).append( "'. " );
								if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
									_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
								throwIueException( temp );
								return;
							}
						}
					}
				}
				else
				{
					EmaString temp( "Failed to Decode Filter Entry. Reason='" );
					temp.append( rsslRetCodeToString( ret ) ).append( "'. " );
					if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
						_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
					throwIueException( temp );
					return;
				}
			}
		}
		else
		{
			EmaString temp( "Failed to Decode Map Entry. Reason='" );
			temp.append( rsslRetCodeToString( ret ) ).append( "'. " );
			if ( OmmLoggerClient::ErrorEnum >= _activeConfig.loggerConfig.minLoggerSeverity )
				_pLoggerClient->log(_activeConfig.instanceName, OmmLoggerClient::ErrorEnum, temp);
			throwIueException( temp );
			return;
		}
	}
 }

size_t OmmNiProviderImpl::UInt64rHasher::operator()( const UInt64& value ) const
{
	return value;
}

bool OmmNiProviderImpl::UInt64Equal_To::operator()( const UInt64& x, const UInt64& y ) const
{
	return x == y;
}

size_t OmmNiProviderImpl::EmaStringPtrHasher::operator()( const EmaStringPtr& value ) const
{
	size_t result = 0;
	size_t magic = 8388593;

	const char *s = value->c_str();
	UInt32 n = value->length();
	while (n--)
		result = ((result % magic) << 8) + (size_t) *s++;
	return result;
}

bool OmmNiProviderImpl::EmaStringPtrEqual_To::operator()( const EmaStringPtr& x, const EmaStringPtr& y ) const
{
	return *x == *y;
}