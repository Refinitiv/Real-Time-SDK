/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ItemCallbackClient.h"
#include "ChannelCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "DictionaryCallbackClient.h"
#include "LoginCallbackClient.h"
#include "OmmConsumerImpl.h"
#include "OmmNiProviderImpl.h"
#include "OmmConsumerClient.h"
#include "OmmProviderClient.h"
#include "ReqMsg.h"
#include "PostMsg.h"
#include "GenericMsg.h"
#include "StaticDecoder.h"
#include "Decoder.h"
#include "ReqMsgEncoder.h"
#include "GenericMsgEncoder.h"
#include "PostMsgEncoder.h"
#include "OmmState.h"
#include "Utilities.h"
#include "RdmUtilities.h"
#include "ExceptionTranslator.h"
#include "TunnelStreamRequest.h"
#include "TunnelStreamLoginReqMsgImpl.h"
#include "StreamId.h"

#include <limits.h>
#include <new>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

const EmaString ItemCallbackClient::_clientName( "ItemCallbackClient" );
const EmaString SingleItem::_clientName( "SingleItem" );
const EmaString NiProviderSingleItem::_clientName( "NiProviderSingleItem" );
const EmaString ItemList::_clientName( "ItemList" );
const EmaString BatchItem::_clientName( "BatchItem" );
const EmaString TunnelItem::_clientName( "TunnelItem" );
const EmaString SubItem::_clientName( "SubItem" );

const EmaString SingleItemString( "SingleItem" );
const EmaString NiProviderSingleItemString( "NiProviderSingleItem" );
const EmaString BatchItemString( "BatchItem" );
const EmaString loginItemString( "LoginItem" );
const EmaString NiProviderLoginItemString( "NiProviderLoginItem" );
const EmaString DirectoryItemString( "DirectoryItem" );
const EmaString DictionaryItemString( "DictionaryItem" );
const EmaString NiProviderDictionaryItemString( "NiProviderDictionaryItem" );
const EmaString TunnelItemString( "TunnelItem" );
const EmaString SubItemString( "SubItem" );
const EmaString UnknownItemString( "UnknownItem" );
//APIQA
#define CONSUMER_STARTING_STREAM_ID 2147483636
//END APIQA
#define CONSUMER_MAX_STREAM_ID_MINUSONE (INT_MAX - 1)

Item::Item( OmmBaseImpl& ommBaseImpl ) :
	_domainType( 0 ),
	_streamId( 0 ),
	_ommBaseImpl( ommBaseImpl )
{
}

Item::~Item()
{
	_ommBaseImpl.getItemCallbackClient().removeFromMap( this );
}

void Item::destroy( Item*& pItem )
{
	if ( pItem )
	{
		delete pItem;
		pItem = 0;
	}
}

const EmaString& Item::getTypeAsString()
{
	switch ( getType() )
	{
	case Item::SingleItemEnum :
		return SingleItemString;
	case Item::NiProviderSingleItemEnum:
		return NiProviderSingleItemString;
	case Item::BatchItemEnum :
		return BatchItemString;
	case Item::LoginItemEnum :
		return loginItemString;
	case Item::NiProviderLoginItemEnum:
		return NiProviderLoginItemString;
	case Item::DirectoryItemEnum :
		return DirectoryItemString;
	case Item::DictionaryItemEnum :
		return DictionaryItemString;
	case Item::NiProviderDictionaryItemEnum:
		return NiProviderDictionaryItemString;
	case Item::TunnelItemEnum :
		return TunnelItemString;
	case Item::SubItemEnum :
		return SubItemString;
	default :
		return UnknownItemString;
	}
}

Int32 Item::getStreamId() const
{
	return _streamId;
}

OmmBaseImpl& Item::getImpl()
{
	return _ommBaseImpl;
}

ConsumerItem::ConsumerItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, Item* pParentItem ) :
	Item( ommBaseImpl ),
	_client( ommConsClient ),
	_event()
{
	_event._handle = ( UInt64 )this;
	_event._closure = closure;
	_event._parentHandle = (UInt64) pParentItem;
}

ConsumerItem::~ConsumerItem()
{
}

void ConsumerItem::onAllMsg( const Msg& msg )
{
	_client.onAllMsg( msg, _event );
}

void ConsumerItem::onRefreshMsg( const RefreshMsg& msg )
{
	_client.onRefreshMsg( msg, _event );
}

void ConsumerItem::onUpdateMsg( const UpdateMsg& msg )
{
	_client.onUpdateMsg( msg, _event );
}

void ConsumerItem::onStatusMsg( const StatusMsg& msg )
{
	_client.onStatusMsg( msg, _event );
}

void ConsumerItem::onAckMsg( const AckMsg& msg )
{
	_client.onAckMsg( msg, _event );
}

void ConsumerItem::onGenericMsg( const GenericMsg& msg )
{
	_client.onGenericMsg( msg, _event );
}

NiProviderItem::NiProviderItem( OmmBaseImpl& ommBaseImpl, OmmProviderClient& ommProvClient, void* closure ) :
	Item( ommBaseImpl ),
	_client( ommProvClient ),
	_event()
{
	_event._handle = ( UInt64 )this;
	_event._closure = closure;
}

NiProviderItem::~NiProviderItem()
{
}

void NiProviderItem::onAllMsg( const Msg& msg )
{
	_client.onAllMsg( msg, _event );
}

void NiProviderItem::onRefreshMsg( const RefreshMsg& msg )
{
	_client.onRefreshMsg( msg, _event );
}

void NiProviderItem::onUpdateMsg( const UpdateMsg& msg )
{
}

void NiProviderItem::onStatusMsg( const StatusMsg& msg )
{
	_client.onStatusMsg( msg, _event );
}

void NiProviderItem::onAckMsg( const AckMsg& msg )
{
}

void NiProviderItem::onGenericMsg( const GenericMsg& msg )
{
	_client.onGenericMsg( msg, _event );
}

const Directory* NiProviderItem::getDirectory()
{
	return 0;
}

SingleItem* SingleItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, Item* pParentItem )
{
	SingleItem* pItem = 0;

	try {
		pItem = new SingleItem( ommBaseImpl, ommConsClient, closure, pParentItem );
	}
	catch( std::bad_alloc ) {}

	if ( !pItem )
		ommBaseImpl.handleMee( "Failed to create SingleItem" );

	return pItem;
}

SingleItem::SingleItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, Item* pParentItem ) :
	ConsumerItem( ommBaseImpl, ommConsClient, closure, pParentItem ),
	_pDirectory( 0 ),
	_closedStatusInfo( 0 )
{
}

SingleItem::~SingleItem()
{
	_ommBaseImpl.getItemCallbackClient().removeFromList( this );

	if ( _closedStatusInfo )
	{
		delete _closedStatusInfo;
		_closedStatusInfo = 0;
	}
}

Item::ItemType SingleItem::getType() const 
{
	return Item::SingleItemEnum;
}

const Directory* SingleItem::getDirectory()
{
	return _pDirectory;
}

bool SingleItem::open( const ReqMsg& reqMsg )
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );
	
	const Directory* pDirectory = 0;

	if ( reqMsgEncoder.hasServiceName() )
	{
		pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory( reqMsgEncoder.getServiceName() );
		if ( !pDirectory )
		{
			EmaString temp( "Service name of '" );
			temp.append( reqMsgEncoder.getServiceName() ).
				append( "' is not found." );
			scheduleItemClosedStatus( reqMsgEncoder, temp );

			return true;
		}
	}
	else
	{
		if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID )
			pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId );
		else
		{
			EmaString temp( "Passed in request message does not identify any service." );
			scheduleItemClosedStatus( reqMsgEncoder, temp );

			return true;
		}

		if ( !pDirectory )
		{
			EmaString temp( "Service id of '" );
			temp.append( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId ).
				append( "' is not found." );
			scheduleItemClosedStatus( reqMsgEncoder, temp );

			return true;
		}
	}

	_pDirectory = pDirectory;

	return submit( reqMsgEncoder.getRsslRequestMsg() );
}

bool SingleItem::modify( const ReqMsg& reqMsg )
{
	return submit( static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() ).getRsslRequestMsg() );
}

bool SingleItem::close()
{
	RsslCloseMsg rsslCloseMsg;
	rsslClearCloseMsg( &rsslCloseMsg );

	rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslCloseMsg.msgBase.domainType = _domainType;

	bool retCode = submit( &rsslCloseMsg );

	remove();

	return retCode;
}

bool SingleItem::submit( const PostMsg& postMsg )
{
	return submit( static_cast<const PostMsgEncoder&>( postMsg.getEncoder() ).getRsslPostMsg() );
}

bool SingleItem::submit( const GenericMsg& genMsg )
{
	return submit( static_cast<const GenericMsgEncoder&>( genMsg.getEncoder() ).getRsslGenericMsg() );
}

void SingleItem::remove()
{
	if ( getType() != Item::BatchItemEnum )
	{
		if ( _event.getParentHandle() )
		{
			if ( reinterpret_cast<Item*>( _event.getParentHandle() )->getType() == Item::BatchItemEnum )
				reinterpret_cast<BatchItem*>( _event.getParentHandle() )->decreaseItemCount();
		}

		delete this;
	}
}

bool SingleItem::submit( RsslRequestMsg* pRsslRequestMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	bool serviceIdSet = ( pRsslRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ? true : false;
	pRsslRequestMsg->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_SERVICE_ID;

	bool qosSet = false;
	if ( !( pRsslRequestMsg->flags & RSSL_RQMF_HAS_QOS ) )
	{
		pRsslRequestMsg->qos.timeliness = RSSL_QOS_TIME_REALTIME;
		pRsslRequestMsg->qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		pRsslRequestMsg->worstQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		pRsslRequestMsg->worstQos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		pRsslRequestMsg->worstQos.rateInfo = 65535;
		pRsslRequestMsg->flags |= ( RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS );
	}
	else
		qosSet = true;

	bool msgKeyInUpdatesSet = ( pRsslRequestMsg->flags & RSSL_RQMF_MSG_KEY_IN_UPDATES ) ? true : false;

	pRsslRequestMsg->flags |= _ommBaseImpl.getActiveConfig().msgKeyInUpdates ? RSSL_RQMF_MSG_KEY_IN_UPDATES : 0;
	submitMsgOpts.pRsslMsg = (RsslMsg*) pRsslRequestMsg;

	RsslBuffer serviceNameBuffer;
	serviceNameBuffer.data = (char*) _pDirectory->getName().c_str();
	serviceNameBuffer.length = _pDirectory->getName().length();
	submitMsgOpts.pServiceName = &serviceNameBuffer;

	submitMsgOpts.majorVersion = _pDirectory->getChannel()->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = _pDirectory->getChannel()->getRsslChannel()->minorVersion;

	submitMsgOpts.requestMsgOptions.pUserSpec = ( void* )this;
	
	Int32 origStreamId = submitMsgOpts.pRsslMsg->msgBase.streamId;

	if ( !_streamId )
	{
		if ( pRsslRequestMsg->flags & RSSL_RQMF_HAS_BATCH )
		{
			const EmaVector<SingleItem*>& singleItemList = static_cast<BatchItem &>( *this ).getSingleItemList();

			submitMsgOpts.pRsslMsg->msgBase.streamId = _ommBaseImpl.getItemCallbackClient().getNextStreamId( singleItemList.size() - 1 );
			_streamId = submitMsgOpts.pRsslMsg->msgBase.streamId;

			for ( UInt32 i = 1; i < singleItemList.size(); ++i )
			{
				singleItemList[i]->_streamId = _streamId + i;
				singleItemList[i]->_pDirectory = _pDirectory;
				singleItemList[i]->_domainType = submitMsgOpts.pRsslMsg->msgBase.domainType;
			}
		}
		else
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = _ommBaseImpl.getItemCallbackClient().getNextStreamId();

			_streamId = submitMsgOpts.pRsslMsg->msgBase.streamId;
		}
	}
	else
		submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	UInt8 origDomainType = submitMsgOpts.pRsslMsg->msgBase.domainType;

	if ( !_domainType )
		_domainType = submitMsgOpts.pRsslMsg->msgBase.domainType;
	else
		submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _pDirectory->getChannel()->getRsslReactor(),
		_pDirectory->getChannel()->getRsslChannel(),
		&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		pRsslRequestMsg->msgBase.domainType = origDomainType;

		pRsslRequestMsg->msgBase.streamId = origStreamId;

		if ( msgKeyInUpdatesSet )
			pRsslRequestMsg->flags |= RSSL_RQMF_MSG_KEY_IN_UPDATES;
		else
			pRsslRequestMsg->flags &= ~RSSL_RQMF_MSG_KEY_IN_UPDATES;

		if ( !qosSet )
			pRsslRequestMsg->flags &= ~( RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS );

		if ( serviceIdSet )
			pRsslRequestMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in SingleItem::submit( RsslRequestMsg* )" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to open or modify item request. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	pRsslRequestMsg->msgBase.domainType = origDomainType;

	pRsslRequestMsg->msgBase.streamId = origStreamId;

	if ( msgKeyInUpdatesSet )
		pRsslRequestMsg->flags |= RSSL_RQMF_MSG_KEY_IN_UPDATES;
	else
		pRsslRequestMsg->flags &= ~RSSL_RQMF_MSG_KEY_IN_UPDATES;

	if ( !qosSet )
		pRsslRequestMsg->flags &= ~( RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS );

	if ( serviceIdSet )
		pRsslRequestMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

	return true;
}

bool SingleItem::submit( RsslCloseMsg* pRsslCloseMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*)pRsslCloseMsg;

	submitMsgOpts.majorVersion = _pDirectory->getChannel()->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = _pDirectory->getChannel()->getRsslChannel()->minorVersion;
	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _pDirectory->getChannel()->getRsslReactor(),
										_pDirectory->getChannel()->getRsslChannel(),
										&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in SingleItem::submit( RsslCloseMsg* )" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to close item request. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	return true;
}

bool SingleItem::submit( RsslGenericMsg* pRsslGenericMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*)pRsslGenericMsg;

	submitMsgOpts.majorVersion = _pDirectory->getChannel()->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = _pDirectory->getChannel()->getRsslChannel()->minorVersion;
	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;
	if (submitMsgOpts.pRsslMsg->msgBase.domainType == 0)
		submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _pDirectory->getChannel()->getRsslReactor(),
										_pDirectory->getChannel()->getRsslChannel(),
										&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in SingleItem::submit( RsslGenericMsg* )" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to submit GenericMsg on item stream. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	return true;
}

bool SingleItem::submit( RsslPostMsg* pRsslPostMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*)pRsslPostMsg;

	submitMsgOpts.majorVersion = _pDirectory->getChannel()->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = _pDirectory->getChannel()->getRsslChannel()->minorVersion;
	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;
	submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _pDirectory->getChannel()->getRsslReactor(),
										_pDirectory->getChannel()->getRsslChannel(),
										&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in SingleItem::submit( RsslPostMsg* ) " );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to submit PostMsg on item stream. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	return true;
}

NiProviderSingleItem* NiProviderSingleItem::create( OmmBaseImpl& ommBaseImpl, OmmProviderClient& ommProvClient, void* closure, Item* pParentItem )
{
	NiProviderSingleItem* pItem = 0;

	try
	{
		pItem = new NiProviderSingleItem( ommBaseImpl, ommProvClient, closure, pParentItem );
	}
	catch ( std::bad_alloc ) {}

	if ( !pItem )
		ommBaseImpl.handleMee( "Failed to create NiProviderSingleItem" );

	return pItem;
}

NiProviderSingleItem::NiProviderSingleItem( OmmBaseImpl& ommBaseImpl, OmmProviderClient& ommProvClient, void* closure, Item* ) :
	NiProviderItem( ommBaseImpl, ommProvClient, closure ),
	_closedStatusInfo( 0 )
{
}

NiProviderSingleItem::~NiProviderSingleItem()
{
	_ommBaseImpl.getItemCallbackClient().removeFromList( this );

	if ( _closedStatusInfo )
	{
		delete _closedStatusInfo;
		_closedStatusInfo = 0;
	}
}

Item::ItemType NiProviderSingleItem::getType() const
{
	return Item::NiProviderSingleItemEnum;
}

bool NiProviderSingleItem::open( const ReqMsg& reqMsg )
{
	UInt64 serviceId = 0;

	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );

	if ( reqMsgEncoder.hasServiceName() )
	{
		if ( !static_cast<OmmNiProviderImpl&>( _ommBaseImpl ).getServiceId( reqMsgEncoder.getServiceName(), serviceId ) )
		{
			EmaString temp( "Service name of '" );
			temp.append( reqMsgEncoder.getServiceName() ).
				append( "' is not found." );
			scheduleItemClosedStatus( reqMsgEncoder, temp );

			return true;
		}

		reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId = (UInt16) serviceId;
		reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
	}
	else
	{
		if ( !( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) )
		{
			EmaString temp( "Passed in request message does not identify any service." );
			scheduleItemClosedStatus( reqMsgEncoder, temp );

			return true;
		}

		serviceId = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId;
		EmaString serviceName;

		if ( !static_cast<OmmNiProviderImpl&>( _ommBaseImpl ).getServiceName( serviceId, serviceName ) )
		{
			EmaString temp( "Service id of '" );
			temp.append( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId ).
				append( "' is not found." );
			scheduleItemClosedStatus( reqMsgEncoder, temp );

			return true;
		}
	}

	return submit( reqMsgEncoder.getRsslRequestMsg() );
}

bool NiProviderSingleItem::modify( const ReqMsg& reqMsg )
{
	return false;
}

bool NiProviderSingleItem::close()
{
	RsslCloseMsg rsslCloseMsg;
	rsslClearCloseMsg( &rsslCloseMsg );

	rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslCloseMsg.msgBase.domainType = _domainType;

	bool retCode = submit( &rsslCloseMsg );

	remove();

	return retCode;
}

bool NiProviderSingleItem::submit( const PostMsg& postMsg )
{
	return false;
}

bool NiProviderSingleItem::submit( const GenericMsg& genMsg )
{
	return submit( static_cast<const GenericMsgEncoder&>( genMsg.getEncoder() ).getRsslGenericMsg() );
}

void NiProviderSingleItem::remove()
{
	if ( getType() != Item::BatchItemEnum )
		delete this;
}

bool NiProviderSingleItem::submit( RsslRequestMsg* pRsslRequestMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	if ( !( pRsslRequestMsg->flags & RSSL_RQMF_HAS_QOS ) )
	{
		pRsslRequestMsg->qos.timeliness = RSSL_QOS_TIME_REALTIME;
		pRsslRequestMsg->qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		pRsslRequestMsg->worstQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		pRsslRequestMsg->worstQos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		pRsslRequestMsg->worstQos.rateInfo = 65535;
		pRsslRequestMsg->flags |= ( RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS );
	}

	submitMsgOpts.pRsslMsg = (RsslMsg*) pRsslRequestMsg;

	submitMsgOpts.pServiceName = 0;

	Channel* pChannel = _ommBaseImpl.getChannelCallbackClient().getChannelList().front();

	submitMsgOpts.majorVersion = pChannel->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = pChannel->getRsslChannel()->minorVersion;

	submitMsgOpts.requestMsgOptions.pUserSpec = ( void* )this;

	if ( !_streamId )
	{
		if ( !submitMsgOpts.pRsslMsg->msgBase.streamId )
			submitMsgOpts.pRsslMsg->msgBase.streamId = _ommBaseImpl.getItemCallbackClient().getNextStreamId();

		_streamId = submitMsgOpts.pRsslMsg->msgBase.streamId;
	}
	else
		submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	if ( !_domainType )
		_domainType = submitMsgOpts.pRsslMsg->msgBase.domainType;
	else
		submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(),
		&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in SingleItem::submit( RsslRequestMsg* )" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to open or modify item request. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	return true;
}

bool NiProviderSingleItem::submit( RsslCloseMsg* pRsslCloseMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*) pRsslCloseMsg;

	Channel* pChannel = _ommBaseImpl.getChannelCallbackClient().getChannelList().front();

	submitMsgOpts.majorVersion = pChannel->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = pChannel->getRsslChannel()->minorVersion;
	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(),
		&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in SingleItem::submit( RsslCloseMsg* )" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to close item request. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	return true;
}

bool NiProviderSingleItem::submit( RsslGenericMsg* pRsslGenericMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*) pRsslGenericMsg;

	Channel* pChannel = _ommBaseImpl.getChannelCallbackClient().getChannelList().front();

	submitMsgOpts.majorVersion = pChannel->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = pChannel->getRsslChannel()->minorVersion;
	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;
	if ( submitMsgOpts.pRsslMsg->msgBase.domainType == 0 )
		submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( pChannel->getRsslReactor(), pChannel->getRsslChannel(),
		&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in SingleItem::submit( RsslGenericMsg* )" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to submit GenericMsg on item stream. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	return true;
}

bool NiProviderSingleItem::submit( RsslPostMsg* pRsslPostMsg )
{
	return false;
}

void NiProviderSingleItem::scheduleItemClosedStatus( const ReqMsgEncoder& reqMsgEncoder, const EmaString& text )
{
	if ( _closedStatusInfo ) return;

	_closedStatusInfo = new ClosedStatusInfo( this, reqMsgEncoder, text );

	new TimeOut( _ommBaseImpl, 1000, ItemCallbackClient::sendItemClosedStatus, _closedStatusInfo, true );
}

void ItemCallbackClient::sendItemClosedStatus( void* pInfo )
{
	if ( !pInfo ) return;

	RsslStatusMsg rsslStatusMsg;
	rsslClearStatusMsg( &rsslStatusMsg );

	ClosedStatusInfo* pClosedStatusInfo = static_cast<ClosedStatusInfo*>( pInfo );

	rsslStatusMsg.msgBase.streamId = pClosedStatusInfo->getStreamId();

	rsslStatusMsg.msgBase.domainType = (UInt8)pClosedStatusInfo->getDomainType();

	if (pClosedStatusInfo->getItem()->getType() == Item::BatchItemEnum)
	{
		rsslStatusMsg.state.dataState = RSSL_DATA_OK;
		rsslStatusMsg.state.code = RSSL_SC_NONE;
	}
	else
	{
		rsslStatusMsg.state.dataState = RSSL_DATA_SUSPECT;
		rsslStatusMsg.state.code = RSSL_SC_SOURCE_UNKNOWN;
	}
	rsslStatusMsg.state.streamState = RSSL_STREAM_CLOSED;
	rsslStatusMsg.state.text.data = (char*)pClosedStatusInfo->getStatusText().c_str();
	rsslStatusMsg.state.text.length = pClosedStatusInfo->getStatusText().length();

	rsslStatusMsg.msgBase.msgKey = *(pClosedStatusInfo->getRsslMsgKey());

	rsslStatusMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY;

	if ( pClosedStatusInfo->getPrivateStream() )
		rsslStatusMsg.flags |= RSSL_STMF_PRIVATE_STREAM;

	StatusMsg statusMsg;

	StaticDecoder::setRsslData( &statusMsg, (RsslMsg*)&rsslStatusMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

	statusMsg.getDecoder().setServiceName( pClosedStatusInfo->getServiceName().c_str(), pClosedStatusInfo->getServiceName().length() );

	Item* item = pClosedStatusInfo->getItem();

	item->onAllMsg( statusMsg );
	item->onStatusMsg( statusMsg );

	item->remove();
}

void SingleItem::scheduleItemClosedStatus( const ReqMsgEncoder& reqMsgEncoder, const EmaString& text )
{
	if ( _closedStatusInfo ) return;

	_closedStatusInfo = new ClosedStatusInfo( this, reqMsgEncoder, text );

	new TimeOut( _ommBaseImpl, 1000, ItemCallbackClient::sendItemClosedStatus, _closedStatusInfo, true );
}

ClosedStatusInfo::ClosedStatusInfo( Item* pItem, const ReqMsgEncoder& reqMsgEncoder, const EmaString& text ) :
	_msgKey(),
	_statusText( text ),
	_serviceName(),
	_domainType( reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType ),
	_streamId( reqMsgEncoder.getRsslRequestMsg()->msgBase.streamId ),
	_pItem( pItem ),
	_privateStream( false )
{
	rsslClearMsgKey( &_msgKey );

	if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME )
	{
		_msgKey.name.data = (char*)malloc( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length );

		if ( !_msgKey.name.data )
		{
			const char* text = "Failed to allocate memory in ClosedStatusInfo( Item* , const ReqMsgEncoder& , const EmaString& ).";
			throwMeeException( text );
		}

		_msgKey.name.length = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length;
	}

	if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB )
	{
		_msgKey.encAttrib.data = (char*) malloc( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.encAttrib.length + 1 );
		if (!_msgKey.encAttrib.data)
		{
			throwMeeException("Failed to allocate memory for encoded attrib in ClosedStatusInfo( Item* , const ReqMsgEncoder& , const EmaString& ).");
		}
		_msgKey.encAttrib.length = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.encAttrib.length + 1;
	}

	rsslCopyMsgKey( &_msgKey, &reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey );

	if ( reqMsgEncoder.hasServiceName() )
		_serviceName = reqMsgEncoder.getServiceName();

	_privateStream = reqMsgEncoder.getPrivateStream();
}

ClosedStatusInfo::ClosedStatusInfo( Item* pItem, const TunnelStreamRequest& tunnelStreamRequest, const EmaString& text ) :
	_msgKey(),
	_statusText( text ),
	_serviceName(),
	_domainType( tunnelStreamRequest.getDomainType() ),
	_streamId( 0 ),
	_pItem( pItem ),
	_privateStream( true )
{
	rsslClearMsgKey( &_msgKey );

	if ( tunnelStreamRequest.hasName() )
	{
		_msgKey.name.data = (char*)malloc( tunnelStreamRequest.getName().length() );
		if ( !_msgKey.name.data )
		{
			const char* text = "Failed to allocate memory in ClosedStatusInfo( Item* , const TunnelStreamRequest& , const EmaString& ).";
			throwMeeException( text );
		}

		memcpy( _msgKey.name.data, tunnelStreamRequest.getName().c_str(), tunnelStreamRequest.getName().length() );

		_msgKey.name.length = tunnelStreamRequest.getName().length();

		_msgKey.flags |= RSSL_MKF_HAS_NAME;
	}

	if ( tunnelStreamRequest.hasServiceId() )
	{
		_msgKey.serviceId = tunnelStreamRequest.getServiceId();
		_msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
	}

	if ( tunnelStreamRequest.hasServiceName() )
		_serviceName = tunnelStreamRequest.getServiceName();
}

ClosedStatusInfo::~ClosedStatusInfo()
{
	if ( _msgKey.name.data )
		free( _msgKey.name.data );

	if ( _msgKey.encAttrib.data )
		free( _msgKey.encAttrib.data );
}

BatchItem* BatchItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure )
{
	BatchItem* pItem = 0;

	try {
		pItem = new BatchItem( ommBaseImpl, ommConsClient, closure );
	}
	catch( std::bad_alloc ) {}

	if ( !pItem )
		ommBaseImpl.handleMee( "Failed to create BatchItem." );

	return pItem;
}

BatchItem::BatchItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure ) :
	SingleItem( ommBaseImpl, ommConsClient, closure, 0 ),
	_singleItemList( 10 ),
	_itemCount( 1 )
{
	_singleItemList.push_back( 0 );
}

BatchItem::~BatchItem()
{
	_singleItemList.clear();
}

Item::ItemType BatchItem::getType() const
{
	return Item::BatchItemEnum; 
}

bool BatchItem::open( const ReqMsg& reqMsg )
{
	return SingleItem::open( reqMsg );
}

bool BatchItem::modify( const ReqMsg& reqMsg )
{
	EmaString temp( "Invalid attempt to modify batch stream. " );
	temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp );

	return false;
}

bool BatchItem::close()
{
	EmaString temp( "Invalid attempt to close batch stream. " );
	temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp );

	return false;
}

bool BatchItem::submit( const PostMsg& )
{
	EmaString temp( "Invalid attempt to submit PostMsg on batch stream. " );
	temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp );

	return false;
}

bool BatchItem::submit( const GenericMsg& )
{
	EmaString temp( "Invalid attempt to submit GenericMsg on batch stream. " );
	temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp );

	return false;
}

bool BatchItem::addBatchItems( UInt32 batchSize )
{
	SingleItem* item = 0;

	for ( UInt32 i = 0 ; i < batchSize ; ++i )
	{
		item = SingleItem::create( _ommBaseImpl, _client, _event.getClosure(), this );

		if ( item )
			_singleItemList.push_back( item );
		else
			return false;
	}
	
	_itemCount = _singleItemList.size() - 1;

	return true;
}

const EmaVector<SingleItem*> & BatchItem::getSingleItemList()
{
	return _singleItemList;
}

SingleItem* BatchItem::getSingleItem( Int32 streamId )
{
	return (streamId == _streamId) ? this : _singleItemList[ streamId - _streamId ];
}

void BatchItem::decreaseItemCount()
{	
	if ( --_itemCount == 0 )
		delete this;
}

TunnelItem* TunnelItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure )
{
	TunnelItem* pItem = 0;

	try {
		pItem = new TunnelItem( ommBaseImpl, ommConsClient, closure );
	}
	catch( std::bad_alloc ) {}

	if ( !pItem )
		ommBaseImpl.handleMee( "Failed to create TunnelItem." );

	return pItem;
}

TunnelItem::TunnelItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure ) :
	ConsumerItem( ommBaseImpl, ommConsClient, closure, 0 ),
	_pDirectory( 0 ),
	_pRsslTunnelStream( 0 ),
	_closedStatusInfo( 0 ),
	nextSubItemStreamId( _startingSubItemStreamId ),
	_subItems( 32 )
{
}

TunnelItem::~TunnelItem()
{
	_ommBaseImpl.getItemCallbackClient().removeFromList( this );

	for ( UInt32 i = 0; i < _subItems.size(); ++i )
		if ( _subItems[ i ] )
			removeSubItem (_subItems[ i ]->getStreamId() );

	if ( _closedStatusInfo )
	{
		delete _closedStatusInfo;
		_closedStatusInfo = 0;
	}

    StreamId* streamId = returnedSubItemStreamIds.pop_back();

	while (streamId)
	{
		delete streamId;
		streamId = returnedSubItemStreamIds.pop_back();
	}
}

const Directory* TunnelItem::getDirectory()
{
	return _pDirectory;
}

Item::ItemType TunnelItem::getType() const
{
	return Item::TunnelItemEnum;
}

Int32 TunnelItem::getSubItemStreamId()
{
	if ( returnedSubItemStreamIds.empty() )
		return nextSubItemStreamId++;

	StreamId* tmp( returnedSubItemStreamIds.pop_back() );
	Int32 retVal( (*tmp)() );
	delete tmp;
	return retVal;
}

void TunnelItem::returnSubItemStreamId( Int32 subItemStreamId )
{
	StreamId* sId( new StreamId( subItemStreamId ) );
	returnedSubItemStreamIds.push_back( sId );
}

UInt32 TunnelItem::addSubItem( Item* pSubItem, Int32 streamId )
{
	if ( streamId == 0 )
		streamId = getSubItemStreamId();
	else
	{
		if ( streamId < _startingSubItemStreamId )
		{
			EmaString temp( "Invalid attempt to open a sub stream with streamId smaller than starting stream id. Passed in stream id is " );
			temp.append( streamId );

			_ommBaseImpl.handleIue( temp );
			return 0;
		}

		StreamId* sId( returnedSubItemStreamIds.front() );
		bool foundReturnedStreamId( false );
		while( sId )
		{
			if ( sId->operator()() == streamId )
			{
				returnedSubItemStreamIds.remove( sId );
				delete sId;
				foundReturnedStreamId = true;
				break;
			}
			else
				sId = sId->next();
		}

		if ( ! foundReturnedStreamId )
		{
			if ( static_cast< UInt32 >( streamId ) < _subItems.size() && _subItems[ streamId ] )
			{
				EmaString temp( "Invalid attempt to open a substream: substream streamId (" );
				temp.append( streamId ).append( ") is already in use" );

				_ommBaseImpl.handleIue( temp );
				return 0;
			}
		}
	}

	while ( static_cast< UInt32 >( streamId ) >= _subItems.size() )
			_subItems.push_back( 0 );
	_subItems[ streamId ] = pSubItem;
	return streamId;
}

void TunnelItem::removeSubItem( UInt32 streamId )
{
	if ( streamId < _startingSubItemStreamId )
	{
		if ( streamId > 0 )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error. Current stream Id in TunnelItem::removeSubItem( UInt32 ) is less than the starting stream id." );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}
		}
		return;
	}

	if ( streamId >= _subItems.size() )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. Current stream Id in TunnelItem::removeSubItem( UInt32 ) is greater than the list size." );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}
		return;
	}

    if ( _subItems[ streamId ] )
	{
		_subItems[ streamId ] = 0;
	}
}

Item* TunnelItem::getSubItem( UInt32 streamId )
{
	if ( streamId < _startingSubItemStreamId )
	{
		if ( streamId > 0 )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error. Current stream Id in TunnelItem::getSubItem( UInt32 ) is less than the starting stream id." );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}
		}
		return 0;
	}

	if ( streamId >= _subItems.size() )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. Current stream Id in TunnelItem::getSubItem( UInt32 ) is greater than the list size." );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}
		return 0;
	}

	return _subItems[ streamId ];	
}

bool TunnelItem::open( const TunnelStreamRequest& tunnelStreamRequest )
{
	const Directory* pDirectory = 0;

	if ( tunnelStreamRequest.hasServiceName() )
	{
		pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory( tunnelStreamRequest.getServiceName() );
		if ( !pDirectory )
		{
			EmaString temp( "Service name of '" );
			temp.append( tunnelStreamRequest.getServiceName() ).
				append( "' is not found." );

			scheduleItemClosedStatus( tunnelStreamRequest, temp );

			return true;
		}
	}
	else if ( tunnelStreamRequest.hasServiceId() )
	{
		pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory( tunnelStreamRequest.getServiceId() );

		if ( !pDirectory )
		{
			EmaString temp( "Service id of '" );
			temp.append( tunnelStreamRequest.getServiceId() ).
				append( "' is not found." );

			scheduleItemClosedStatus( tunnelStreamRequest, temp );

			return true;
		}
	}

	_pDirectory = pDirectory;

	return submit( tunnelStreamRequest );
}

void TunnelItem::scheduleItemClosedStatus( const TunnelStreamRequest& tunnelStreamRequest, const EmaString& text )
{
	if ( _closedStatusInfo ) return;

	_closedStatusInfo = new ClosedStatusInfo( this, tunnelStreamRequest, text );

	new TimeOut( _ommBaseImpl, 1000, ItemCallbackClient::sendItemClosedStatus, _closedStatusInfo, true );
}

void TunnelItem::rsslTunnelStream( RsslTunnelStream* pRsslTunnelStream )
{
	_pRsslTunnelStream = pRsslTunnelStream;
}

bool TunnelItem::open( const ReqMsg& )
{
	EmaString temp( "Invalid attempt to open tunnel stream using ReqMsg." );
	temp.append( "OmmConsumer name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp );

	return false;
}

bool TunnelItem::modify( const ReqMsg& )
{
	EmaString temp( "Invalid attempt to reissue tunnel stream using ReqMsg." );
	temp.append( "OmmConsumer name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp );

	return false;
}

bool TunnelItem::submit( const PostMsg& )
{
	EmaString temp( "Invalid attempt to submit PostMsg on tunnel stream." );
	temp.append( "OmmConsumer name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp );

	return false;
}

bool TunnelItem::submit( const GenericMsg& )
{
	EmaString temp( "Invalid attempt to submit GenericMsg on tunnel stream." );
	temp.append( "OmmConsumer name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp );

	return false;
}

bool TunnelItem::submit( const TunnelStreamRequest& tunnelStreamRequest )
{
	_domainType = (UInt8)tunnelStreamRequest.getDomainType();
	_streamId = _ommBaseImpl.getItemCallbackClient().getNextStreamId();

	RsslTunnelStreamOpenOptions tsOpenOptions;
	rsslClearTunnelStreamOpenOptions( &tsOpenOptions );

	tsOpenOptions.domainType = _domainType;
	tsOpenOptions.name = tunnelStreamRequest.hasName() ? (char*)tunnelStreamRequest.getName().c_str() : 0;

	RsslDecodeIterator dIter;
	rsslClearDecodeIterator( &dIter );

	RsslBuffer rsslBuffer;
	rsslBuffer.data = (char*)malloc( sizeof( char ) * 4096 );

	if ( !rsslBuffer.data )
	{
		const char* temp = "Failed to allocate memory in TunnelItem::submit( const TunnelStreamRequest& ).";

		if ( _ommBaseImpl.hasErrorClientHandler() )
			_ommBaseImpl.getErrorClientHandler().onMemoryExhaustion( temp );
		else
			throwMeeException( temp );

		return false;
	}

	rsslBuffer.length = 4096;
	
	RsslRDMLoginMsg rsslRdmLoginMsg;
	rsslClearRDMLoginMsg( &rsslRdmLoginMsg );

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );

	if ( tunnelStreamRequest.hasLoginReqMsg() )
	{
		if ( RSSL_RET_SUCCESS != rsslSetDecodeIteratorRWFVersion( &dIter, _pDirectory->getChannel()->getRsslChannel()->majorVersion, _pDirectory->getChannel()->getRsslChannel()->minorVersion ) )
		{
			free( rsslBuffer.data );
			_ommBaseImpl.handleIue( "Internal Error. Failed to set decode iterator version in TunnelItem::submit( const TunnelStreamRequest& )" );
			return false;
		}

		if ( RSSL_RET_SUCCESS != rsslSetDecodeIteratorBuffer( &dIter, tunnelStreamRequest._pImpl->getRsslBuffer() ) )
		{
			free( rsslBuffer.data );
			_ommBaseImpl.handleIue( "Internal Error. Failed to set decode iterator buffer in TunnelItem::submit( const TunnelStreamRequest& )" );
			return false;
		}

		RsslMsg* pRsslMsg = tunnelStreamRequest._pImpl->getRsslMsg();

		RsslRet retCode = rsslDecodeRDMLoginMsg( &dIter, pRsslMsg, &rsslRdmLoginMsg, &rsslBuffer, &rsslErrorInfo );

		while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
		{
			free( rsslBuffer.data );

			rsslBuffer.length += rsslBuffer.length;

			rsslBuffer.data = (char*)malloc( sizeof( char ) * rsslBuffer.length );

			if ( !rsslBuffer.data )
			{
				_ommBaseImpl.handleMee( "Failed to allocate memory in TunnelItem::submit( const TunnelStreamRequest& )." );
				return false;
			}

			retCode = rsslDecodeRDMLoginMsg( &dIter, pRsslMsg, &rsslRdmLoginMsg, &rsslBuffer, &rsslErrorInfo );
		}

		if ( RSSL_RET_SUCCESS != retCode )
		{
			free( rsslBuffer.data );
			_ommBaseImpl.handleIue( "Internal Error. Failed to decode login request in TunnelItem::submit( const TunnelStreamRequest& )" );
			return false;
		}

		tsOpenOptions.pAuthLoginRequest = &rsslRdmLoginMsg.request;
	}
	
	tsOpenOptions.guaranteedOutputBuffers = tunnelStreamRequest.getGuaranteedOutputBuffers();
	tsOpenOptions.responseTimeout = tunnelStreamRequest.getResponseTimeOut();
	tsOpenOptions.serviceId = (UInt16)_pDirectory->getId();
	tsOpenOptions.streamId = _streamId;

	tsOpenOptions.userSpecPtr = this;

	tsOpenOptions.defaultMsgCallback = OmmConsumerImpl::tunnelStreamDefaultMsgCallback;
	tsOpenOptions.queueMsgCallback = OmmConsumerImpl::tunnelStreamQueueMsgCallback;
	tsOpenOptions.statusEventCallback = OmmConsumerImpl::tunnelStreamStatusEventCallback;

	tsOpenOptions.classOfService.common.maxMsgSize = tunnelStreamRequest.getClassOfService().getCommon().getMaxMsgSize();

	tsOpenOptions.classOfService.authentication.type = (RDMClassOfServiceAuthenticationType)tunnelStreamRequest.getClassOfService().getAuthentication().getType();

	tsOpenOptions.classOfService.dataIntegrity.type = (RsslUInt)tunnelStreamRequest.getClassOfService().getDataIntegrity().getType();

	tsOpenOptions.classOfService.flowControl.recvWindowSize = tunnelStreamRequest.getClassOfService().getFlowControl().getRecvWindowSize();
	tsOpenOptions.classOfService.flowControl.sendWindowSize = tunnelStreamRequest.getClassOfService().getFlowControl().getSendWindowSize();
	tsOpenOptions.classOfService.flowControl.type = (RsslUInt)tunnelStreamRequest.getClassOfService().getFlowControl().getType();

	tsOpenOptions.classOfService.guarantee.type = (RsslUInt)tunnelStreamRequest.getClassOfService().getGuarantee().getType();
	tsOpenOptions.classOfService.guarantee.persistLocally = tunnelStreamRequest.getClassOfService().getGuarantee().getPersistLocally() ? RSSL_TRUE : RSSL_FALSE;
	tsOpenOptions.classOfService.guarantee.persistenceFilePath = (char*)tunnelStreamRequest.getClassOfService().getGuarantee().getPersistenceFilePath().c_str();

	RsslRet ret;
	if ( ( ret = rsslReactorOpenTunnelStream( _pDirectory->getChannel()->getRsslChannel(), &tsOpenOptions, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorOpenTunnelStream() failed in TunnelItem::submit( const TunnelStreamRequest& )" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		free( rsslBuffer.data );

		EmaString text( "Failed to open tunnel stream request. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	free( rsslBuffer.data );

	return true;
}

bool TunnelItem::close()
{
	RsslTunnelStreamCloseOptions tunnelStreamCloseOptions;
	rsslClearTunnelStreamCloseOptions( &tunnelStreamCloseOptions );

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorCloseTunnelStream( _pRsslTunnelStream, &tunnelStreamCloseOptions, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorCloseTunnelStream() failed in TunnelItem::close()" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to close tunnel stream request. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	remove();

	return true;
}

void TunnelItem::remove()
{
	UInt32 last( _subItems.size() );
	for ( UInt32 i = 0; i < last; ++i )
		if (_subItems[i] ) 
			_subItems[i]->remove();

	delete this;
}

bool TunnelItem::submitSubItemMsg( RsslMsg* pRsslMsg )
{
	RsslTunnelStreamGetBufferOptions rsslGetBufferOpts;
	rsslClearTunnelStreamGetBufferOptions( &rsslGetBufferOpts );
	rsslGetBufferOpts.size = 256;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslBuffer* pRsslBuffer = rsslTunnelStreamGetBuffer( _pRsslTunnelStream, &rsslGetBufferOpts, &rsslErrorInfo );

	if ( !pRsslBuffer )
	{
		_ommBaseImpl.handleMee( "Internal Error. Failed to allocate RsslTunnelStreamBuffer in TunnelItem::submitSubItemMsg( RsslMsg* )." );
		return false;
	}

	RsslEncodeIterator eIter;
	rsslClearEncodeIterator( &eIter );

	if ( rsslSetEncodeIteratorRWFVersion( &eIter, _pRsslTunnelStream->pReactorChannel->majorVersion, _pRsslTunnelStream->pReactorChannel->minorVersion ) != RSSL_RET_SUCCESS )
	{
		_ommBaseImpl.handleIue( "Internal Error. Failed to set encode iterator version in TunnelItem::submitSubItemMsg( RsslMsg* )." );
		return false;
	}

	if ( rsslSetEncodeIteratorBuffer( &eIter, pRsslBuffer ) != RSSL_RET_SUCCESS )
	{
		_ommBaseImpl.handleIue( "Internal Error. Failed to set encode iterator buffer in TunnelItem::submitSubItemMsg( RsslMsg* )." );
		return false;
	}

	RsslRet retCode;
	while ( ( retCode = rsslEncodeMsg( &eIter, pRsslMsg ) ) == RSSL_RET_BUFFER_TOO_SMALL )
	{
		rsslTunnelStreamReleaseBuffer( pRsslBuffer, &rsslErrorInfo );

		rsslGetBufferOpts.size += rsslGetBufferOpts.size;

		pRsslBuffer = rsslTunnelStreamGetBuffer( _pRsslTunnelStream, &rsslGetBufferOpts, &rsslErrorInfo );

		if ( !pRsslBuffer )
		{
			_ommBaseImpl.handleMee( "Internal Error. Failed to allocate RsslTunnelStreamBuffer in TunnelItem::submitSubItemMsg( RsslMsg* )." );
			return false;
		}

		rsslClearEncodeIterator( &eIter );

		if ( rsslSetEncodeIteratorRWFVersion( &eIter, _pRsslTunnelStream->pReactorChannel->majorVersion, _pRsslTunnelStream->pReactorChannel->minorVersion ) != RSSL_RET_SUCCESS )
		{
			_ommBaseImpl.handleIue( "Internal Error. Failed to set encode iterator version in TunnelItem::submitSubItemMsg( RsslMsg* )." );
			return false;
		}

		if ( rsslSetEncodeIteratorBuffer( &eIter, pRsslBuffer ) != RSSL_RET_SUCCESS )
		{
			_ommBaseImpl.handleIue( "Internal Error. Failed to set encode iterator buffer in TunnelItem::submitSubItemMsg( RsslMsg* )." );
			return false;
		}
	}

	if ( retCode != RSSL_RET_SUCCESS )
	{
		rsslTunnelStreamReleaseBuffer( pRsslBuffer, &rsslErrorInfo );

		_ommBaseImpl.handleIue( "Internal Error. Failed to encode message in TunnelItem::submitSubItemMsg( RsslMsg* )." );
		return false;
	}

	pRsslBuffer->length = rsslGetEncodedBufferLength( &eIter );

	RsslTunnelStreamSubmitOptions rsslTunnelStreamSubmitOptions;
	rsslClearTunnelStreamSubmitOptions( &rsslTunnelStreamSubmitOptions );
	rsslTunnelStreamSubmitOptions.containerType = RSSL_DT_MSG;

	retCode = rsslTunnelStreamSubmit( _pRsslTunnelStream, pRsslBuffer, &rsslTunnelStreamSubmitOptions, &rsslErrorInfo );

	if ( retCode != RSSL_RET_SUCCESS )
	{
		rsslTunnelStreamReleaseBuffer( pRsslBuffer, &rsslErrorInfo );

		_ommBaseImpl.handleIue( "Internal Error. Failed to submit message in TunnelItem::submitSubItemMsg( RsslMsg* )." );
		return false;
	}

	return true;
}

SubItem* SubItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, Item* parent )
{
	SubItem* pItem = 0;

	try {
		pItem = new SubItem( ommBaseImpl, ommConsClient, closure, parent );
	}
	catch( std::bad_alloc ) {}

	if ( !pItem )
		ommBaseImpl.handleMee( "Failed to create SubItem" );

	return pItem;
}

SubItem::SubItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, Item* parent ) :
	ConsumerItem( ommBaseImpl, ommConsClient, closure, parent ),
	_closedStatusInfo( 0 )
{
}

SubItem::~SubItem()
{
	reinterpret_cast<TunnelItem*>( _event.getParentHandle() )->removeSubItem( _streamId );

	_ommBaseImpl.getItemCallbackClient().removeFromList( this );

	if ( _closedStatusInfo )
	{
		delete _closedStatusInfo;
		_closedStatusInfo = 0;
	}

	/*Need to set to 0 to avoid remove a wrong SingleItem of the same stream id from _streamIdMap*/
	_streamId = 0;
}

Item::ItemType SubItem::getType() const 
{
	return Item::SubItemEnum;
}

const Directory* SubItem::getDirectory()
{
	return 0;
}

void SubItem::scheduleItemClosedStatus( const ReqMsgEncoder& reqMsgEncoder, const EmaString& text )
{
	if ( _closedStatusInfo ) return;

	_closedStatusInfo = new ClosedStatusInfo( this, reqMsgEncoder, text );

	new TimeOut( _ommBaseImpl, 1000, ItemCallbackClient::sendItemClosedStatus, _closedStatusInfo, true );
}

bool SubItem::open( const ReqMsg& reqMsg )
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );
	
	if ( reqMsgEncoder.hasServiceName() )
	{
		EmaString temp( "Invalid attempt to open sub stream using serviceName." );
		scheduleItemClosedStatus( reqMsgEncoder, temp );

		return true;
	}

	if ( !reqMsgEncoder.getRsslRequestMsg()->msgBase.streamId )
	{
		_streamId = reinterpret_cast<TunnelItem*>( _event.getParentHandle() )->addSubItem( this );
		reqMsgEncoder.getRsslRequestMsg()->msgBase.streamId = _streamId;
	}
	else
	{
		if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.streamId < 0 )
		{
			EmaString temp( "Invalid attempt to assign negative streamId to a sub stream." );
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

			if ( _ommBaseImpl.hasErrorClientHandler() )
			{
				_ommBaseImpl.getErrorClientHandler().onInvalidUsage( temp );
				return false;
			}
			else
			{
				throwIueException( temp );
				return false;
			}
		}
		else
		{
			_streamId = reinterpret_cast<TunnelItem*>( _event.getParentHandle() )->addSubItem( this, reqMsgEncoder.getRsslRequestMsg()->msgBase.streamId );
		}
	}

	_domainType = (UInt8)reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType;

	return reinterpret_cast<TunnelItem*>( _event.getParentHandle() )->submitSubItemMsg( (RsslMsg*)reqMsgEncoder.getRsslRequestMsg() );
}

bool SubItem::modify( const ReqMsg& reqMsg )
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );

	reqMsgEncoder.getRsslRequestMsg()->msgBase.streamId = _streamId;

	return reinterpret_cast<TunnelItem*>( _event.getParentHandle() )->submitSubItemMsg( (RsslMsg*)reqMsgEncoder.getRsslRequestMsg() );
}

bool SubItem::submit( const PostMsg& postMsg )
{
	const PostMsgEncoder& postMsgEncoder = static_cast<const PostMsgEncoder&>( postMsg.getEncoder() );

	postMsgEncoder.getRsslPostMsg()->msgBase.streamId = _streamId;

	return reinterpret_cast<TunnelItem*>( _event.getParentHandle() )->submitSubItemMsg( (RsslMsg*)postMsgEncoder.getRsslPostMsg() );
}

bool SubItem::submit( const GenericMsg& genMsg )
{
	const GenericMsgEncoder& genMsgEncoder = static_cast<const GenericMsgEncoder&>( genMsg.getEncoder() );

	genMsgEncoder.getRsslGenericMsg()->msgBase.streamId = _streamId;
	if (genMsgEncoder.getRsslGenericMsg()->msgBase.domainType == 0)
		genMsgEncoder.getRsslGenericMsg()->msgBase.domainType = _domainType;

	return reinterpret_cast<TunnelItem*>( _event.getParentHandle() )->submitSubItemMsg( (RsslMsg*)genMsgEncoder.getRsslGenericMsg() );
}

bool SubItem::close()
{
	RsslCloseMsg rsslCloseMsg;
	rsslClearCloseMsg( &rsslCloseMsg );

	rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslCloseMsg.msgBase.domainType = _domainType;
	rsslCloseMsg.msgBase.streamId = _streamId;
    bool retCode = reinterpret_cast<TunnelItem*>( _event.getParentHandle() )->submitSubItemMsg( (RsslMsg*)&rsslCloseMsg );
	
	remove();

	return retCode;
}

void SubItem::remove()
{
	reinterpret_cast<TunnelItem*>(_event.getParentHandle())->returnSubItemStreamId(_streamId);

	delete this;
}

ItemList* ItemList::create( OmmBaseImpl& ommBaseImpl )
{
	ItemList* pItemList = 0;

	try {
		pItemList = new ItemList( ommBaseImpl );
	}
	catch( std::bad_alloc ) {}

	if ( !pItemList )
		ommBaseImpl.handleMee( "Failed to create ItemList" );

	return pItemList;
}

void ItemList::destroy( ItemList*& pItemList )
{
	if ( pItemList )
	{
		delete pItemList;
		pItemList = 0;
	}
}

ItemList::ItemList( OmmBaseImpl& ommBaseImpl ) :
	_ommBaseImpl( ommBaseImpl )
{
}

ItemList::~ItemList()
{
	Item* temp = _list.back();

	while ( temp )
	{
		Item::destroy( temp );
		temp = _list.back();
	}
}

Int32 ItemList::addItem( Item* pItem )
{
	_list.push_back( pItem );

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "Added Item " );
		temp.append( ptrToStringAsHex( pItem ) ).append(" of StreamId ").append(pItem->getStreamId()).append( " to ItemList" ).append( CR )
			.append( "OmmConsumer name " ).append( _ommBaseImpl .getInstanceName() );
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}

	return _list.size();
}

void ItemList::removeItem( Item* pItem )
{
	_list.remove( pItem );

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "Removed Item " );
		temp.append( ptrToStringAsHex( pItem ) ).append(" of StreamId ").append(pItem->getStreamId()).append( " from ItemList" ).append( CR )
			.append( "Instance name " ).append( _ommBaseImpl .getInstanceName() );
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}
}

ItemCallbackClient::ItemCallbackClient( OmmBaseImpl& ommBaseImpl ) :
	_refreshMsg(),
	_updateMsg(),
	_statusMsg(),
	_genericMsg(),
	_ackMsg(),
	_ommBaseImpl( ommBaseImpl ),
	_itemMap( ommBaseImpl.getActiveConfig().itemCountHint ),
	_streamIdMap(ommBaseImpl.getActiveConfig().itemCountHint),
	_nextStreamIdWrapAround(false),
	_streamIdAccessMutex()
{
    _itemList = ItemList::create( ommBaseImpl );

	_nextStreamId = CONSUMER_STARTING_STREAM_ID;

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "Created ItemCallbackClient." );
		temp.append( " Instance name='" ).append( _ommBaseImpl.getInstanceName() ).append( "'." );
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}
}

ItemCallbackClient::~ItemCallbackClient()
{
	ItemList::destroy( _itemList );

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "Destroyed ItemCallbackClient [" );
		temp.append( _ommBaseImpl .getInstanceName() ).append( "]" );
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}
}

ItemCallbackClient* ItemCallbackClient::create( OmmBaseImpl& ommBaseImpl )
{
	ItemCallbackClient* pClient = 0;

	try {
		pClient = new ItemCallbackClient( ommBaseImpl );
	}
	catch ( std::bad_alloc ) {}

	if ( !pClient )
		ommBaseImpl.handleMee( "Failed to create ItemCallbackClient" );

	return pClient;
}

void ItemCallbackClient::destroy( ItemCallbackClient*& pClient )
{
	if ( pClient )
	{
		delete pClient;
		pClient = 0;
	}
}

void ItemCallbackClient::initialize()
{
}

RsslReactorCallbackRet ItemCallbackClient::processCallback( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamStatusEvent* pTunnelStreamStatusEvent )
{
	if ( !pRsslTunnelStream )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream status event without the pRsslTunnelStream in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamStatusEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamStatusEvent->pReactorChannel->pRsslChannel ) );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if ( !pRsslTunnelStream->userSpecPtr )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream status event without the userSpecPtr in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamStatusEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamStatusEvent->pReactorChannel->pRsslChannel ) );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	RsslStatusMsg rsslStatusMsg;
	rsslClearStatusMsg( &rsslStatusMsg );

	rsslStatusMsg.flags |= RSSL_STMF_PRIVATE_STREAM | RSSL_STMF_CLEAR_CACHE | RSSL_STMF_HAS_MSG_KEY;

	rsslStatusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslStatusMsg.msgBase.domainType = pRsslTunnelStream->domainType;
	rsslStatusMsg.msgBase.streamId = pRsslTunnelStream->streamId;

	if ( pTunnelStreamStatusEvent->pState )
	{
		rsslStatusMsg.state = *pTunnelStreamStatusEvent->pState;
		rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE;
	}

	rsslStatusMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
	rsslStatusMsg.msgBase.msgKey.name.data = pRsslTunnelStream->name;
	rsslStatusMsg.msgBase.msgKey.name.length = (UInt32)strlen( pRsslTunnelStream->name );

	rsslStatusMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
	rsslStatusMsg.msgBase.msgKey.serviceId = pRsslTunnelStream->serviceId;

	Channel* channel = static_cast<Channel*>(pTunnelStreamStatusEvent->pReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData(&_statusMsg, (RsslMsg*)&rsslStatusMsg,
		pTunnelStreamStatusEvent->pReactorChannel->majorVersion,
		pTunnelStreamStatusEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	static_cast<TunnelItem*>( pRsslTunnelStream->userSpecPtr )->rsslTunnelStream( pRsslTunnelStream );

	Item* item = (Item*)( pRsslTunnelStream->userSpecPtr );

	_statusMsg.getDecoder().setServiceName( item->getDirectory()->getName().c_str(), item->getDirectory()->getName().length() );

	_ommBaseImpl.msgDispatched();

	item->onAllMsg( _statusMsg );
	item->onStatusMsg( _statusMsg );

	if ( pTunnelStreamStatusEvent->pState )
		if ( _statusMsg.getState().getStreamState() != OmmState::OpenEnum )
			item->remove();

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processCallback( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent )
{
	if ( !pRsslTunnelStream )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream message event without the pRsslTunnelStream in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamMsgEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if ( !pRsslTunnelStream->userSpecPtr )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream message event without the userSpecPtr in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamMsgEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) ).append( CR )
				.append( "Error Id " ).append( pTunnelStreamMsgEvent->pErrorInfo->rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( pTunnelStreamMsgEvent->pErrorInfo->rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( pTunnelStreamMsgEvent->pErrorInfo->errorLocation ).append( CR )
				.append( "Error Text " ).append( pTunnelStreamMsgEvent->pErrorInfo->rsslError.rsslErrorId ? pTunnelStreamMsgEvent->pErrorInfo->rsslError.text : "" );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if ( pTunnelStreamMsgEvent->containerType != RSSL_DT_MSG )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream message event containing an unsupported data type of " );
			temp += DataType((DataType::DataTypeEnum)pTunnelStreamMsgEvent->containerType).toString();

			temp.append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) ).append( CR )
				.append( "Tunnel Stream Handle " ).append( (UInt64)pRsslTunnelStream->userSpecPtr ).append( CR )
				.append( "Tunnel Stream name " ).append( pRsslTunnelStream->name ).append( CR )
				.append( "Tunnel Stream serviceId " ).append( pRsslTunnelStream->serviceId ).append( CR )
				.append( "Tunnel Stream streamId " ).append( pRsslTunnelStream->streamId );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}
	else if ( !pTunnelStreamMsgEvent->pRsslMsg )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream message event containing no sub stream message." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) ).append( CR )
				.append( "Tunnel Stream Handle " ).append( (UInt64)pRsslTunnelStream->userSpecPtr ).append( CR )
				.append( "Tunnel Stream name " ).append( pRsslTunnelStream->name ).append( CR )
				.append( "Tunnel Stream serviceId " ).append( pRsslTunnelStream->serviceId ).append( CR )
				.append( "Tunnel Stream streamId " ).append( pRsslTunnelStream->streamId );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	Item* item = static_cast<TunnelItem*>( pRsslTunnelStream->userSpecPtr )->getSubItem( pTunnelStreamMsgEvent->pRsslMsg->msgBase.streamId );
	
	if ( ! item )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream message event containing sub stream message with unknown streamId " );
			temp.append( pTunnelStreamMsgEvent->pRsslMsg->msgBase.streamId ).append( ". Message is dropped." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) ).append( CR )
				.append( "Tunnel Stream Handle " ).append( (UInt64)pRsslTunnelStream->userSpecPtr ).append( CR )
				.append( "Tunnel Stream name " ).append( pRsslTunnelStream->name ).append( CR )
				.append( "Tunnel Stream serviceId " ).append( pRsslTunnelStream->serviceId ).append( CR )
				.append( "Tunnel Stream streamId " ).append( pRsslTunnelStream->streamId );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	switch ( pTunnelStreamMsgEvent->pRsslMsg->msgBase.msgClass )
	{
	default :
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received a tunnel stream message event containing an unsupported message type of " );
				temp += DataType( msgDataType[ pTunnelStreamMsgEvent->pRsslMsg->msgBase.msgClass ] ).toString();

				temp.append( CR )
					.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
					.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}
		}
		break;
	case RSSL_MC_GENERIC :
		return processGenericMsg( pRsslTunnelStream, pTunnelStreamMsgEvent, item );
	case RSSL_MC_ACK :
		return processAckMsg( pRsslTunnelStream, pTunnelStreamMsgEvent, item );
	case RSSL_MC_REFRESH :
		return processRefreshMsg( pRsslTunnelStream, pTunnelStreamMsgEvent, item );
	case RSSL_MC_UPDATE :
		return processUpdateMsg( pRsslTunnelStream, pTunnelStreamMsgEvent, item );
	case RSSL_MC_STATUS :
		return processStatusMsg( pRsslTunnelStream, pTunnelStreamMsgEvent, item );
	};

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processCallback( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamQueueMsgEvent* pTunnelStreamQueueMsgEvent )
{
	if ( !pRsslTunnelStream )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream queue message event without the pRsslTunnelStream in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamQueueMsgEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamQueueMsgEvent->base.pReactorChannel->pRsslChannel ) );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if ( !pRsslTunnelStream->userSpecPtr )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream queue message event without the userSpecPtr in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamQueueMsgEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamQueueMsgEvent->base.pReactorChannel->pRsslChannel ) ).append( CR )
				.append( "Error Id " ).append( pTunnelStreamQueueMsgEvent->base.pErrorInfo->rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( pTunnelStreamQueueMsgEvent->base.pErrorInfo->rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( pTunnelStreamQueueMsgEvent->base.pErrorInfo->errorLocation ).append( CR )
				.append( "Error Text " ).append( pTunnelStreamQueueMsgEvent->base.pErrorInfo->rsslError.rsslErrorId ? pTunnelStreamQueueMsgEvent->base.pErrorInfo->rsslError.text : "" );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processAckMsg( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent, Item* item )
{
	Channel* channel = static_cast<Channel*>(pTunnelStreamMsgEvent->pReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_ackMsg, pTunnelStreamMsgEvent->pRsslMsg,
		pTunnelStreamMsgEvent->pReactorChannel->majorVersion,
		pTunnelStreamMsgEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	_ommBaseImpl.msgDispatched();

	item->onAllMsg( _ackMsg );
	item->onAckMsg( _ackMsg );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processGenericMsg( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent, Item* item )
{
	Channel* channel = static_cast<Channel*>(pTunnelStreamMsgEvent->pReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_genericMsg, pTunnelStreamMsgEvent->pRsslMsg,
		pTunnelStreamMsgEvent->pReactorChannel->majorVersion,
		pTunnelStreamMsgEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	_ommBaseImpl.msgDispatched();

	item->onAllMsg( _genericMsg );
	item->onGenericMsg( _genericMsg );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processRefreshMsg( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent, Item* item )
{
	Channel* channel = static_cast<Channel*>(pTunnelStreamMsgEvent->pReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_refreshMsg, pTunnelStreamMsgEvent->pRsslMsg,
		pTunnelStreamMsgEvent->pReactorChannel->majorVersion,
		pTunnelStreamMsgEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	_ommBaseImpl.msgDispatched();

	item->onAllMsg( _refreshMsg );
	item->onRefreshMsg( _refreshMsg );

	if ( _refreshMsg.getState().getStreamState() == OmmState::NonStreamingEnum )
	{
		if ( _refreshMsg.getComplete() )
			item->remove();
	}
	else if ( _refreshMsg.getState().getStreamState() != OmmState::OpenEnum )
	{
		item->remove();
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processUpdateMsg( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent, Item* item )
{
	Channel* channel = static_cast<Channel*>(pTunnelStreamMsgEvent->pReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_updateMsg, pTunnelStreamMsgEvent->pRsslMsg,
		pTunnelStreamMsgEvent->pReactorChannel->majorVersion,
		pTunnelStreamMsgEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	_ommBaseImpl.msgDispatched();

	item->onAllMsg( _updateMsg );
	item->onUpdateMsg( _updateMsg );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processStatusMsg( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent, Item* item )
{
	Channel* channel = static_cast<Channel*>(pTunnelStreamMsgEvent->pReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_statusMsg, pTunnelStreamMsgEvent->pRsslMsg,
		pTunnelStreamMsgEvent->pReactorChannel->majorVersion,
		pTunnelStreamMsgEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	_ommBaseImpl.msgDispatched();

	item->onAllMsg( _statusMsg );
	item->onStatusMsg( _statusMsg );

	if ( pTunnelStreamMsgEvent->pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_STATE )
		if ( _statusMsg.getState().getStreamState() != OmmState::OpenEnum )
			item->remove();

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processCallback( RsslReactor* pRsslReactor,
															RsslReactorChannel* pRsslReactorChannel,
															RsslMsgEvent* pEvent )
{
	RsslMsg* pRsslMsg = pEvent->pRsslMsg;

	if ( !pRsslMsg )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			RsslErrorInfo* pError = pEvent->pErrorInfo;

			EmaString temp( "Received an item event without RsslMsg message" );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pError->rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( pError->rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( pError->rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( pError->errorLocation ).append( CR )
				.append( "Error Text " ).append( pError->rsslError.rsslErrorId ? pError->rsslError.text : "" );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if ( pRsslMsg->msgBase.streamId != 1 && 
		( !pEvent->pStreamInfo || !pEvent->pStreamInfo->pUserSpec ) )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received an item event without user specified pointer or stream info" );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslReactorChannel " ).append( ptrToStringAsHex( pRsslReactorChannel ) ).append( CR )
				.append( "RsslSocket " ).append( (UInt64)pRsslReactorChannel->socketId );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	switch ( pRsslMsg->msgBase.msgClass )
	{
	case RSSL_MC_ACK :
		if ( pRsslMsg->msgBase.streamId == 1 )
			return _ommBaseImpl.getLoginCallbackClient().processAckMsg( pRsslMsg, pRsslReactorChannel, 0 );
		else
			return processAckMsg( pRsslMsg, pRsslReactorChannel, pEvent );
	case RSSL_MC_GENERIC :
		return processGenericMsg( pRsslMsg, pRsslReactorChannel, pEvent );
	case RSSL_MC_REFRESH :
		return processRefreshMsg( pRsslMsg, pRsslReactorChannel, pEvent );
	case RSSL_MC_STATUS :
		return processStatusMsg( pRsslMsg, pRsslReactorChannel, pEvent );
	case RSSL_MC_UPDATE :
		return processUpdateMsg( pRsslMsg, pRsslReactorChannel, pEvent );
	default :
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received an item event with message containing unhandled message class" );
			temp.append( CR )
				.append( "Rssl Message Class " ).append( pRsslMsg->msgBase.msgClass ).append( CR )
				.append( "Instance Name " ).append( _ommBaseImpl.getInstanceName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslReactorChannel " ).append( ptrToStringAsHex( pRsslReactorChannel ) ).append( CR )
				.append( "RsslSocket" ).append( (UInt64)pRsslReactorChannel->socketId );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}
		break;
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processRefreshMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	Channel* channel = static_cast<Channel*>(pRsslReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_refreshMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	Item* pItem = (Item*)( pEvent->pStreamInfo->pUserSpec );

	if ( pItem->getType() == Item::BatchItemEnum )
		pItem = static_cast<BatchItem *>(pItem)->getSingleItem( pRsslMsg->msgBase.streamId );
	
	_refreshMsg.getDecoder().setServiceName( pItem->getDirectory()->getName().c_str(), pItem->getDirectory()->getName().length() );

	_ommBaseImpl.msgDispatched();

	pItem->onAllMsg( _refreshMsg );
	pItem->onRefreshMsg( _refreshMsg );

	if ( _refreshMsg.getState().getStreamState() == OmmState::NonStreamingEnum )
	{
		if ( _refreshMsg.getComplete() )
			pItem->remove();
	}
	else if ( _refreshMsg.getState().getStreamState() != OmmState::OpenEnum )
	{
		pItem->remove();
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processUpdateMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	Channel* channel = static_cast<Channel*>(pRsslReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_updateMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	Item* item = (Item*) pEvent->pStreamInfo->pUserSpec;

	if ( item->getType() == Item::BatchItemEnum )
		item = static_cast<BatchItem *>(item)->getSingleItem( pRsslMsg->msgBase.streamId );
	
	_updateMsg.getDecoder().setServiceName( item->getDirectory()->getName().c_str(), item->getDirectory()->getName().length() );

	_ommBaseImpl.msgDispatched();

	item->onAllMsg( _updateMsg );
	item->onUpdateMsg( _updateMsg );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processStatusMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	Channel* channel = static_cast<Channel*>(pRsslReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_statusMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	Item* item = (Item*) pEvent->pStreamInfo->pUserSpec;

	if ( item->getType() == Item::BatchItemEnum )
		item = static_cast<BatchItem *>(item)->getSingleItem( pRsslMsg->msgBase.streamId );
	
	_statusMsg.getDecoder().setServiceName( item->getDirectory()->getName().c_str(), item->getDirectory()->getName().length() );

	_ommBaseImpl.msgDispatched();

	item->onAllMsg( _statusMsg );
	item->onStatusMsg( _statusMsg );

	if ( pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_STATE )
		if ( _statusMsg.getState().getStreamState() != OmmState::OpenEnum )
			item->remove();

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processGenericMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	Channel* channel = static_cast<Channel*>(pRsslReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_genericMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	Item* item = (Item*) pEvent->pStreamInfo->pUserSpec;

	if ( item->getType() == Item::BatchItemEnum )
		item  = static_cast<BatchItem *>(item )->getSingleItem( pRsslMsg->msgBase.streamId );

	_ommBaseImpl.msgDispatched();

	item->onAllMsg( _genericMsg );
	item->onGenericMsg( _genericMsg );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processAckMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	Channel* channel = static_cast<Channel*>(pRsslReactorChannel->userSpecPtr);

	StaticDecoder::setRsslData( &_ackMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	Item* item = (Item*) pEvent->pStreamInfo->pUserSpec;

	if ( item->getType() == Item::BatchItemEnum )
		item = static_cast<BatchItem *>(item)->getSingleItem( pRsslMsg->msgBase.streamId );
	
	_ackMsg.getDecoder().setServiceName( item->getDirectory()->getName().c_str(), item->getDirectory()->getName().length() );

	_ommBaseImpl.msgDispatched();

	item->onAllMsg( _ackMsg );
	item->onAckMsg( _ackMsg );

	return RSSL_RC_CRET_SUCCESS;
}

UInt64 ItemCallbackClient::registerClient( const ReqMsg& reqMsg, OmmConsumerClient& ommConsClient, void* closure, UInt64 parentHandle )
{
	if ( !parentHandle )
	{
		const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );

		switch ( reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType )
		{
		case RSSL_DMT_LOGIN :
			{
				LoginItem* pItem = _ommBaseImpl.getLoginCallbackClient().getLoginItem( reqMsg, ommConsClient, closure );

				if ( pItem )
				{
					addToList( pItem );
					addToItemMap( pItem );
				}

				return (UInt64)pItem;
			}
		case RSSL_DMT_DICTIONARY :
			{
				if ( ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.nameType != INSTRUMENT_NAME_UNSPECIFIED ) &&
					( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.nameType != rdm::INSTRUMENT_NAME_RIC ) )
				{
					EmaString temp( "Invalid ReqMsg's name type : " );
					temp.append( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.nameType );
					temp.append( ". Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );
					_ommBaseImpl.handleIue( temp );
					return 0;
				}

				if ( ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) == 0 )
				{
					EmaString temp( "ReqMsg's name is not defined. " );
					temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );
					_ommBaseImpl.handleIue( temp );
					return 0;
				}

				DictionaryItem* pItem = DictionaryItem::create( _ommBaseImpl, ommConsClient, closure );

				if ( pItem )
				{
					if ( !pItem->open( reqMsg ) )
					{
						Item::destroy( (Item*&)pItem );
					}
					else
					{
						addToList( pItem );
						addToMap( pItem );
					}
				}

				return (UInt64)pItem;
			}
		case RSSL_DMT_SOURCE :
			{
				const ChannelList& channels( _ommBaseImpl.getChannelCallbackClient().getChannelList() );
				Channel* c( channels.front() );

				DirectoryItem* pItem = DirectoryItem::create( _ommBaseImpl, ommConsClient, closure, c );

				if ( pItem )
				{
					try {
						if ( !pItem->open( reqMsg ) )
							Item::destroy( (Item*&)pItem );
						else
						{
							addToList( pItem );
							addToMap( pItem );
						}
					}
					catch ( ... )
					{
						Item::destroy( (Item*&)pItem );
						throw;
					}
				}

				return (UInt64)pItem;

			}
		default :
			{
				SingleItem* pItem = 0;

				if ( reqMsgEncoder.getRsslRequestMsg()->flags & RSSL_RQMF_HAS_BATCH )
				{
					BatchItem* pBatchItem = BatchItem::create( _ommBaseImpl, ommConsClient, closure );

					int numOfItems = reqMsgEncoder.getBatchItemListSize();
					if ( pBatchItem )
					{
						try 
						{
							/* Start splitting the batch request into individual item request if _nextStreamIdWrapAround is true. */
							if (_ommBaseImpl.getItemCallbackClient().nextStreamIdWrapAround(numOfItems))
							{
								if (!splitAndSendSingleRequest(reqMsg, ommConsClient, closure))
								{
									Item::destroy((Item*&)pBatchItem);

									EmaString temp("Failed to split a batch request into single item requests on registerClient(). ");
									temp.append("Instance name='").append(_ommBaseImpl.getInstanceName()).append("'.");
									_ommBaseImpl.handleIue(temp);

									return (UInt64)0;
								}

								addToList(pBatchItem);
								addToItemMap(pBatchItem);

								/*Send stream close status for the batch stream */
								EmaString temp("Batch request acknowledged.");
								pBatchItem->scheduleItemClosedStatus(reqMsgEncoder, temp);

								return (UInt64)pBatchItem;
							}
							else
							{
								pItem = pBatchItem;
								pBatchItem->addBatchItems(numOfItems);

								if (!pBatchItem->open(reqMsg))
								{
									for (UInt32 i = 1; i < pBatchItem->getSingleItemList().size(); i++)
									{
										Item::destroy((Item*&)pBatchItem->getSingleItemList()[i]);
									}

									Item::destroy((Item*&)pItem);
								}
								else
								{
									addToList(pBatchItem);
									addToMap(pBatchItem);

									for (UInt32 i = 1; i < pBatchItem->getSingleItemList().size(); i++)
									{
										addToList(pBatchItem->getSingleItemList()[i]);
										addToMap(pBatchItem->getSingleItemList()[i]);
									}
								}
							}
						}
						catch ( ... )
						{
							for ( UInt32 i = 1 ; i < pBatchItem->getSingleItemList().size() ; i++ )
							{
								Item::destroy( (Item*&)pBatchItem->getSingleItemList()[i] );
							}

							Item::destroy( (Item*&)pItem );

							throw;
						}
					}
				}
				else
				{
					pItem = SingleItem::create( _ommBaseImpl, ommConsClient, closure, 0 );

					if ( pItem )
					{
						try {
							if ( !pItem->open( reqMsg ) )
								Item::destroy( (Item*&)pItem );
							else
							{
								addToList( pItem );
								addToMap( pItem );
							}
						}
						catch ( ... )
						{
							Item::destroy( (Item*&)pItem );
							throw;
						}
					}
				}

				return (UInt64)pItem;
			}
		}
	}
	else
	{
		if ( !_itemMap.find( parentHandle ) )
		{
			EmaString temp( "Attempt to use invalid parentHandle on registerClient(). " );
			temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );
			_ommBaseImpl.handleIhe( parentHandle, temp );
			return 0;
		}

		if ( ((Item*)parentHandle)->getType() != Item::TunnelItemEnum )
		{
			EmaString temp( "Invalid attempt to use " );
			temp += ((Item*)parentHandle)->getTypeAsString();
			temp.append( " as parentHandle on registerClient(). " );
			temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );
			_ommBaseImpl.handleIhe( parentHandle, temp );
			return 0;
		}

		SubItem* pItem = SubItem::create( _ommBaseImpl, ommConsClient, closure, (Item*)parentHandle );

		if ( pItem )
		{
			try {
				if ( !pItem->open( reqMsg ) )
					Item::destroy( (Item*&)pItem );
				else
				{
					addToList( pItem );
					addToItemMap( pItem );
				}
			}
			catch ( ... )
			{
				Item::destroy( (Item*&)pItem );
				throw;
			}
		}

		return (UInt64)pItem;
	}
}

UInt64 ItemCallbackClient::registerClient( const ReqMsg& reqMsg, OmmProviderClient& ommProvClient, void* closure, UInt64 parentHandle )
{
	if ( !parentHandle )
	{
		const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );

		switch ( reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType )
		{
		case RSSL_DMT_LOGIN:
		{
			NiProviderLoginItem* pItem = _ommBaseImpl.getLoginCallbackClient().getLoginItem( reqMsg, ommProvClient, closure );

			if ( pItem )
			{
				addToList( pItem );
				addToItemMap( pItem );
			}

			return (UInt64) pItem;
		}
		case RSSL_DMT_DICTIONARY:
		{
			if ( ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.nameType != INSTRUMENT_NAME_UNSPECIFIED ) &&
				( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.nameType != rdm::INSTRUMENT_NAME_RIC ) )
			{
				EmaString temp( "Invalid ReqMsg's name type : " );
				temp.append( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.nameType );
				temp.append( ". Instance name='" ).append( _ommBaseImpl.getInstanceName() ).append( "'." );
				_ommBaseImpl.handleIue( temp );
				return 0;
			}

			if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME )
			{
				EmaString name( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.data, reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length );

				if ( ( name != "RWFFld" ) && ( name != "RWFEnum" ) )
				{
					EmaString temp( "Invalid ReqMsg's name : " );
					temp.append( name );
					temp.append( "\nReqMsg's name must be \"RWFFld\" or \"RWFEnum\" for MMT_DICTIONARY domain type. " );
					temp.append( "Instance name='" ).append( _ommBaseImpl.getInstanceName() ).append( "'." );
					_ommBaseImpl.handleIue( temp );
					return 0;
				}
			}
			else
			{
				EmaString temp( "ReqMsg's name is not defined. " );
				temp.append( "Instance name='" ).append( _ommBaseImpl.getInstanceName() ).append( "'." );
				_ommBaseImpl.handleIue( temp );
				return 0;
			}

			// todo ... add dictionary download feature
			DictionaryItem* pItem = 0; // DictionaryItem::create( _ommBaseImpl, ommProvClient, closure );

			if ( pItem )
			{
				if ( !pItem->open( reqMsg ) )
				{
					Item::destroy( (Item*&) pItem );
				}
				else
				{
					addToList( pItem );
					addToMap( pItem );
				}
			}

			return (UInt64) pItem;
		}

		default:
		{
			EmaString temp( "Invalid ReqMsg's domain type : " );
			temp.append( reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType );
			temp.append( ". Instance name='" ).append( _ommBaseImpl.getInstanceName() ).append( "'." );
			_ommBaseImpl.handleIue( temp );
			return 0;
		}
		}
	}
	else
	{
		EmaString temp( "Attempt to use parentHandle on OmmProvider::registerClient(). " );
		temp.append( "Instance name='" ).append( _ommBaseImpl.getInstanceName() ).append( "'." );
		_ommBaseImpl.handleIue( temp );
		return 0;
	}
}

UInt64 ItemCallbackClient::registerClient( const TunnelStreamRequest& tunnelStreamRequest, OmmConsumerClient& ommConsClient, void* closure )
{
	TunnelItem* pItem = TunnelItem::create( _ommBaseImpl, ommConsClient, closure );

	if ( pItem )
	{
		try {
			if ( !pItem->open( tunnelStreamRequest ) )
				Item::destroy( (Item*&)pItem );
			else
			{
				addToList( pItem );
				addToMap( pItem );
			}
		}
		catch ( ... )
		{
			Item::destroy( (Item*&)pItem );
			throw;
		}
	}

	return (UInt64)pItem;
}

void ItemCallbackClient::reissue( const ReqMsg& reqMsg, UInt64 handle )
{
	if ( !_itemMap.find( handle ) )
	{
		EmaString temp( "Attempt to use invalid Handle on reissue(). " );
		temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );
		_ommBaseImpl.handleIhe( handle, temp );
		return;
	}

	((Item*)handle)->modify( reqMsg );
}

void ItemCallbackClient::unregister( UInt64 handle )
{
	if ( !_itemMap.find( handle ) ) return;

	((Item*)handle)->close();
}

void ItemCallbackClient::submit( const PostMsg& postMsg, UInt64 handle )
{
	if ( !_itemMap.find( handle ) )
	{
		EmaString temp( "Attempt to use invalid Handle on submit( const PostMsg& ). " );
		temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );
		_ommBaseImpl.handleIhe( handle, temp );
		return;
	}

	((Item*)handle)->submit( postMsg );
}

void ItemCallbackClient::submit( const GenericMsg& genericMsg, UInt64 handle )
{
	if ( !_itemMap.find( handle ) )
	{
		EmaString temp( "Attempt to use invalid Handle on submit( const GenericMsg& ). " );
		temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );
		_ommBaseImpl.handleIhe( handle, temp );
		return;
	}

	((Item*)handle)->submit( genericMsg );
}

size_t ItemCallbackClient::UInt64rHasher::operator()( const UInt64& value ) const
{
	return value;
}

bool ItemCallbackClient::UInt64Equal_To::operator()( const UInt64& x, const UInt64& y ) const
{
	return x == y;
}

Int32 ItemCallbackClient::Int32rHasher::operator()(const Int32& value) const
{
	return value;
}

bool ItemCallbackClient::Int32Equal_To::operator()(const Int32& x, const Int32& y) const
{
	return x == y;
}

void ItemCallbackClient::addToList( Item* pItem )
{
	_itemList->addItem( pItem );
}

void ItemCallbackClient::removeFromList( Item* pItem )
{
	_itemList->removeItem( pItem );
}

void ItemCallbackClient::addToMap( Item* pItem )
{
	_itemMap.insert( (UInt64)pItem, pItem );
	_streamIdMap.insert(pItem->getStreamId(), NULL);
}

void ItemCallbackClient::removeFromMap(Item* pItem)
{
	_ommBaseImpl.getUserLock().lock();

	_itemMap.erase( (UInt64)pItem );

	if ( pItem->getStreamId() > 0 )
		_streamIdMap.erase( pItem->getStreamId() );
	
	_ommBaseImpl.getUserLock().unlock();
}

void ItemCallbackClient::addToItemMap(Item* pItem)
{
	_itemMap.insert((UInt64)pItem, pItem);
}

bool ItemCallbackClient::isStreamIdInUse( int nextStreamId )
{
	return _streamIdMap.find(nextStreamId) != 0;
}

bool ItemCallbackClient::splitAndSendSingleRequest(const ReqMsg& reqMsg, OmmConsumerClient& ommConsClient, void* closure)
{
	RsslRequestMsg* rsslReqMsg = static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder()).getRsslRequestMsg();
	rsslReqMsg->flags &= ~RSSL_RQMF_HAS_BATCH;

	/*decode to get item names */
	RsslElementList	rsslElementList;
	RsslElementEntry rsslElementEntry;
	RsslDecodeIterator decodeIter;

	rsslClearDecodeIterator(&decodeIter);
	rsslClearElementEntry(&rsslElementEntry);
	rsslClearElementList(&rsslElementList);

	RsslBuffer tempRsslBuffer = rsslReqMsg->msgBase.encDataBody;

	/*No need to check return code because they have been checked in getItemListSize() function before this call*/
	rsslSetDecodeIteratorBuffer(&decodeIter, &tempRsslBuffer);
	rsslSetDecodeIteratorRWFVersion(&decodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION);
	rsslDecodeElementList(&decodeIter, &rsslElementList, 0);
	while (true)
	{
		if (rsslDecodeElementEntry(&decodeIter, &rsslElementEntry) == RSSL_RET_SUCCESS)
		{
			if (rsslBufferIsEqual(&rsslElementEntry.name, &RSSL_ENAME_BATCH_ITEM_LIST) && rsslElementEntry.dataType == RSSL_DT_ARRAY)
			{
				RsslArray rsslArray;
				rsslClearArray(&rsslArray);
				rsslArray.encData = rsslElementEntry.encData;

				if (rsslDecodeArray(&decodeIter, &rsslArray) >= RSSL_RET_SUCCESS)
				{
					if (rsslArray.primitiveType == RSSL_DT_ASCII_STRING)
					{
						RsslBuffer rsslBuffer;
						rsslReqMsg->msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
						Item* pItem = NULL;

						while (rsslDecodeArrayEntry(&decodeIter, &rsslBuffer) != RSSL_RET_END_OF_CONTAINER)
						{
							if (rsslDecodeBuffer(&decodeIter, &rsslReqMsg->msgBase.msgKey.name) == RSSL_RET_SUCCESS)
							{
								pItem = SingleItem::create(_ommBaseImpl, ommConsClient, closure, 0);

								if (pItem)
								{
									try {
										if (!pItem->open(reqMsg))
											Item::destroy((Item*&)pItem);
										else
										{
											addToList(pItem);
											addToMap(pItem);
										}
									}
									catch (...)
									{
										Item::destroy((Item*&)pItem);
										throw;
									}
								}

							}
						}
					}
				}
				rsslReqMsg->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_NAME;
				return true;
			}
		}
	}
	return false;
}

Int32 ItemCallbackClient::getNextStreamId(UInt32 numberOfBatchItems)
{
	if ((UInt32)_nextStreamId > CONSUMER_MAX_STREAM_ID_MINUSONE - numberOfBatchItems)
	{
		_nextStreamId = CONSUMER_STARTING_STREAM_ID;
		_nextStreamIdWrapAround = true; 

		if (OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Reach max number available for next stream id, will wrap around");
			_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp.trimWhitespace());
		}
	}

	if (!_nextStreamIdWrapAround)
	{
		if (numberOfBatchItems > 0)
		{
			int retVal = ++_nextStreamId;
			_nextStreamId += numberOfBatchItems;
			return retVal;
		}

		return ++_nextStreamId;
	}
	else
	{
		_streamIdAccessMutex.lock();
		while (_ommBaseImpl.getItemCallbackClient().isStreamIdInUse(++_nextStreamId));
		_streamIdAccessMutex.unlock();

		if (_nextStreamId < 0)
		{
			EmaString temp("Unable to obtain next available stream id for item request.");
			if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
				_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);

			if (_ommBaseImpl.hasErrorClientHandler())
				_ommBaseImpl.getErrorClientHandler().onInvalidUsage(temp);
			else
				throwIueException(temp);
		}

		return _nextStreamId;
	}
}

bool ItemCallbackClient::nextStreamIdWrapAround(UInt32 numberOfBatchItems)
{
	return ((UInt32)_nextStreamId > CONSUMER_MAX_STREAM_ID_MINUSONE - numberOfBatchItems);
}
