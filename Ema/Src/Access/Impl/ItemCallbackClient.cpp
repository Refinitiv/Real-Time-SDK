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
#include "OmmConsumerClient.h"
#include "ReqMsg.h"
#include "PostMsg.h"
#include "GenericMsg.h"
#include "StaticDecoder.h"
#include "Decoder.h"
#include "ReqMsgEncoder.h"
#include "GenericMsgEncoder.h"
#include "PostMsgEncoder.h"
#include "OmmState.h"
#include "OmmConsumerErrorClient.h"
#include "Utilities.h"
#include "RdmUtilities.h"
#include "ExceptionTranslator.h"

#include <new>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

const EmaString ItemCallbackClient::_clientName( "ItemCallbackClient" );
const EmaString SingleItem::_clientName( "SingleItem" );
const EmaString ItemList::_clientName( "ItemList" );
const EmaString BatchItem::_clientName( "BatchItem" );

Item::Item( OmmConsumerImpl& ommConsImpl, OmmConsumerClient& ommConsClient, void* closure ) :
 _domainType( 0 ),
 _streamId( 0 ),
 _closure( closure ),
 _ommConsClient( ommConsClient ),
 _ommConsImpl( ommConsImpl )
{
}

Item::~Item()
{
	_ommConsImpl.getItemCallbackClient().removeFromMap( this );
}

void Item::destroy( Item*& pItem )
{
	if ( pItem )
	{
		delete pItem;
		pItem = 0;
	}
}

OmmConsumerClient& Item::getClient() const
{
	return _ommConsClient;
}

void* Item::getClosure() const
{
	return _closure;
}

OmmConsumerImpl& Item::getOmmConsumerImpl()
{
	return _ommConsImpl;
}

SingleItem* SingleItem::create( OmmConsumerImpl& ommConsImpl, OmmConsumerClient& ommConsClient, void* closure, SingleItem* batchItem )
{
	SingleItem* pItem = 0;

	try {
		pItem = new SingleItem( ommConsImpl, ommConsClient, closure, batchItem );
	}
	catch( std::bad_alloc ) {}

	if ( !pItem )
	{
		const char* temp = "Failed to create SingleItem";
		if ( OmmLoggerClient::ErrorEnum >= ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( ommConsImpl.hasOmmConnsumerErrorClient() )
			ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
		else
			throwMeeException( temp );
	}

	return pItem;
}

SingleItem::SingleItem( OmmConsumerImpl& ommConsImpl, OmmConsumerClient& ommConsClient, void* closure, SingleItem* batchItem ) :
 Item( ommConsImpl, ommConsClient, closure ),
 _pDirectory( 0 ),
 _closedStatusInfo( 0 ),
 _pBatchItem( batchItem )
{
}

SingleItem::~SingleItem()
{
	_ommConsImpl.getItemCallbackClient().removeFromList( this );

	if ( _closedStatusInfo )
	{
		delete _closedStatusInfo;
		_closedStatusInfo = 0;
	}

	if ( _pBatchItem )
		_pBatchItem = 0;
}

const Directory* SingleItem::getDirectory() const
{
	return _pDirectory;
}

bool SingleItem::open( const ReqMsg& reqMsg )
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );
	
	const Directory* pDirectory = 0;

	if ( reqMsgEncoder.hasServiceName() )
	{
		pDirectory = _ommConsImpl.getDirectoryCallbackClient().getDirectory( reqMsgEncoder.getServiceName() );
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
			pDirectory = _ommConsImpl.getDirectoryCallbackClient().getDirectory( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId );
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
	if ( _pBatchItem )
	{
		if ( _pBatchItem->getType() == Item::BatchItemEnum )
			static_cast<BatchItem*>(_pBatchItem)->decreaseItemCount();
	}

	delete this;
}

bool SingleItem::submit( RsslRequestMsg* pRsslRequestMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	pRsslRequestMsg->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_SERVICE_ID;	
	
	if ( !( pRsslRequestMsg->flags & RSSL_RQMF_HAS_QOS ) )
	{
		pRsslRequestMsg->qos.timeliness = RSSL_QOS_TIME_REALTIME;
		pRsslRequestMsg->qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		pRsslRequestMsg->worstQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		pRsslRequestMsg->worstQos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		pRsslRequestMsg->worstQos.rateInfo = 65535;
		pRsslRequestMsg->flags |= (RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS);
	}	

	pRsslRequestMsg->flags |= _ommConsImpl.getActiveConfig().channelConfig->msgKeyInUpdates ? RSSL_RQMF_MSG_KEY_IN_UPDATES : 0;
	submitMsgOpts.pRsslMsg = (RsslMsg*)pRsslRequestMsg;

	RsslBuffer serviceNameBuffer;
	serviceNameBuffer.data = (char*)_pDirectory->getName().c_str();
	serviceNameBuffer.length = _pDirectory->getName().length();
	submitMsgOpts.pServiceName = &serviceNameBuffer;

	submitMsgOpts.majorVersion = _pDirectory->getChannel()->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = _pDirectory->getChannel()->getRsslChannel()->minorVersion;

	submitMsgOpts.requestMsgOptions.pUserSpec = (void*)this;

	if ( !_streamId )
	{
		if ( pRsslRequestMsg->flags & RSSL_RQMF_HAS_BATCH )
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = _pDirectory->getChannel()->getNextStreamId();
			_streamId = submitMsgOpts.pRsslMsg->msgBase.streamId;

			const EmaVector<SingleItem*>& singleItemList = static_cast<BatchItem &>(*this).getSingleItemList();

			for( UInt32 i = 1; i < singleItemList.size(); ++i )
			{
				singleItemList[i]->_streamId = _pDirectory->getChannel()->getNextStreamId();
				singleItemList[i]->_pDirectory = _pDirectory;
				singleItemList[i]->_domainType = submitMsgOpts.pRsslMsg->msgBase.domainType;
			}
		}
		else
		{
			if ( !submitMsgOpts.pRsslMsg->msgBase.streamId )
				submitMsgOpts.pRsslMsg->msgBase.streamId = _pDirectory->getChannel()->getNextStreamId();
			_streamId = submitMsgOpts.pRsslMsg->msgBase.streamId;
		}
	}
	else
		submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	if ( !_domainType )
		_domainType = submitMsgOpts.pRsslMsg->msgBase.domainType;
	else
		submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _pDirectory->getChannel()->getRsslReactor(),
										_pDirectory->getChannel()->getRsslChannel(),
										&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in submit( RsslRequestMsg* )" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to open or modify item request. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
			_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( text );
		else
			throwIueException( text );

		return false;
	}

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
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _pDirectory->getChannel()->getRsslReactor(),
										_pDirectory->getChannel()->getRsslChannel(),
										&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in submit( RsslCloseMsg* )" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to close item request. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
			_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( text );
		else
			throwIueException( text );

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
	submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _pDirectory->getChannel()->getRsslReactor(),
										_pDirectory->getChannel()->getRsslChannel(),
										&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in submit( RsslGenericMsg* )" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to submit GenericMsg on item stream. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
			_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( text );
		else
			throwIueException( text );

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
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _pDirectory->getChannel()->getRsslReactor(),
										_pDirectory->getChannel()->getRsslChannel(),
										&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in submit( RsslPostMsg* ) " );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to submit PostMsg on item stream. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
			.append( ". Error text: " )
			.append( rsslErrorInfo.rsslError.text );

		if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
			_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( text );
		else
			throwIueException( text );

		return false;
	}

	return true;
}

void ItemCallbackClient::sendItemClosedStatus( void* pInfo )
{
	if ( !pInfo ) return;

	RsslStatusMsg rsslStatusMsg;
	rsslClearStatusMsg( &rsslStatusMsg );

	ClosedStatusInfo* pClosedStatusInfo = static_cast<ClosedStatusInfo*>( pInfo );

	rsslStatusMsg.msgBase.domainType = pClosedStatusInfo->getDomainType();

	rsslStatusMsg.state.code = RSSL_SC_SOURCE_UNKNOWN;
	rsslStatusMsg.state.dataState = RSSL_DATA_SUSPECT;
	rsslStatusMsg.state.streamState = RSSL_STREAM_CLOSED;
	rsslStatusMsg.state.text.data = (char*)pClosedStatusInfo->getStatusText().c_str();
	rsslStatusMsg.state.text.length = pClosedStatusInfo->getStatusText().length();

	rsslStatusMsg.msgBase.msgKey = *(pClosedStatusInfo->getRsslMsgKey());

	rsslStatusMsg.msgBase.containerType = RSSL_DT_NO_DATA;

	rsslStatusMsg.flags |= RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY;

	StatusMsg statusMsg;

	StaticDecoder::setRsslData( &statusMsg, (RsslMsg*)&rsslStatusMsg, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, 0 );

	statusMsg.getDecoder().setServiceName( pClosedStatusInfo->getServiceName().c_str(), pClosedStatusInfo->getServiceName().length() );

	OmmConsumerEvent event;

	event._pItem = pClosedStatusInfo->getItem();

	event._pItem->getClient().onAllMsg( statusMsg, event );
	event._pItem->getClient().onStatusMsg( statusMsg, event );

	event._pItem->remove();
}

void SingleItem::scheduleItemClosedStatus( const ReqMsgEncoder& reqMsgEncoder, const EmaString& text )
{
	if ( _closedStatusInfo ) return;

	_closedStatusInfo = new ClosedStatusInfo( this, reqMsgEncoder, text );

	new TimeOut( _ommConsImpl, 1000, ItemCallbackClient::sendItemClosedStatus, _closedStatusInfo );
}

ClosedStatusInfo::ClosedStatusInfo( Item* pItem, const ReqMsgEncoder& reqMsgEncoder, const EmaString& text ) :
 _msgKey(),
 _statusText( text ),
 _serviceName(),
 _domainType( reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType ),
 _pItem( pItem )
{
	rsslClearMsgKey( &_msgKey );

	if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME )
	{
		_msgKey.name.data = (char*)malloc( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length + 1 );
		_msgKey.name.length = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length + 1;
	}

	if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB )
	{
		_msgKey.encAttrib.data = (char*) malloc( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.encAttrib.length + 1 );
		_msgKey.encAttrib.length = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.encAttrib.length + 1;
	}

	rsslCopyMsgKey( &_msgKey, &reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey );

	_serviceName = reqMsgEncoder.getServiceName();
}

ClosedStatusInfo::~ClosedStatusInfo()
{
	if ( _msgKey.name.data )
		free( _msgKey.name.data );

	if ( _msgKey.encAttrib.data )
		free( _msgKey.encAttrib.data );
}

BatchItem* BatchItem::create( OmmConsumerImpl& ommConsImpl, OmmConsumerClient& ommConsClient, void* closure )
{
	BatchItem* pItem = 0;

	try {
		pItem = new BatchItem( ommConsImpl, ommConsClient, closure );
	}
	catch( std::bad_alloc ) {}

	if ( !pItem )
	{
		const char* temp = "Failed to create BatchItem.";
		if ( OmmLoggerClient::ErrorEnum >= ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( ommConsImpl.hasOmmConnsumerErrorClient() )
			ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
		else
			throwMeeException( temp );
	}

	return pItem;
}

BatchItem::BatchItem( OmmConsumerImpl& ommConsImpl, OmmConsumerClient& ommConsClient,
					 void* closure ) :
 SingleItem( ommConsImpl, ommConsClient, closure, 0 ),
 _singleItemList( 10 ),
 _itemCount( 1 )
{
	_singleItemList.push_back( 0 );
}

BatchItem::~BatchItem()
{
	_singleItemList.clear();
}

bool BatchItem::open( const ReqMsg& reqMsg )
{
	return SingleItem::open( reqMsg );
}

bool BatchItem::modify( const ReqMsg& reqMsg )
{
	EmaString temp( "Invalid attempt to modify batch stream. " );
	temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

	if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

	if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
		_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( temp );
	else
		throwIueException( temp );

	return false;
}

bool BatchItem::close()
{
	EmaString temp( "Invalid attempt to close batch stream. " );
	temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

	if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

	if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
		_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( temp );
	else
		throwIueException( temp );

	return false;
}

bool BatchItem::submit( const PostMsg& )
{
	EmaString temp( "Invalid attempt to submit PostMsg on batch stream. " );
	temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

	if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

	if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
		_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( temp );
	else
		throwIueException( temp );

	return false;
}

bool BatchItem::submit( const GenericMsg& )
{
	EmaString temp( "Invalid attempt to submit GenericMsg on batch stream. " );
	temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );
	if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

	if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
		_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( temp );
	else
		throwIueException( temp );

	return false;
}

bool BatchItem::addBatchItems( const EmaVector<EmaString>& batchItemList )
{
	SingleItem * item = 0;

	for( UInt32 i = 0 ; i < batchItemList.size() ; i++ )
	{
		item = SingleItem::create( _ommConsImpl, _ommConsClient, 0, this );

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
	return _singleItemList[ streamId - _streamId ];
}

void BatchItem::decreaseItemCount()
{
	_itemCount--;
	
	if ( _itemCount == 0 )
		remove();
}

ItemList* ItemList::create( OmmConsumerImpl& ommConsImpl )
{
	ItemList* pItemList = 0;

	try {
		pItemList = new ItemList( ommConsImpl );
	}
	catch( std::bad_alloc ) {}

	if ( !pItemList )
	{
		const char* temp = "Failed to create ItemList";
		if ( OmmLoggerClient::ErrorEnum >= ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( ommConsImpl.hasOmmConnsumerErrorClient() )
			ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
		else
			throwMeeException( temp );
	}

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

ItemList::ItemList( OmmConsumerImpl& ommConsImpl ) :
 _ommConsImpl( ommConsImpl )
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

	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "Added Item " );
		temp.append( ptrToStringAsHex( pItem ) ).append( " to ItemList" ).append( CR )
			.append( "OmmConsumer name " ).append( _ommConsImpl .getConsumerName() );
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}

	return _list.size();
}

void ItemList::removeItem( Item* pItem )
{
	_list.remove( pItem );

	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "Removed Item " );
		temp.append( ptrToStringAsHex( pItem ) ).append( " from ItemList" ).append( CR )
			.append( "OmmConsumer name " ).append( _ommConsImpl .getConsumerName() );
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}
}

ItemCallbackClient::ItemCallbackClient( OmmConsumerImpl& ommConsImpl ) :
 _refreshMsg(),
 _updateMsg(),
 _statusMsg(),
 _genericMsg(),
 _ackMsg(),
 _event(),
 _ommConsImpl( ommConsImpl ),
 _itemMap( ommConsImpl.getActiveConfig().itemCountHint )
{
    _itemList = ItemList::create( ommConsImpl );

	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "Created ItemCallbackClient." );
		temp.append( " OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}
}

ItemCallbackClient::~ItemCallbackClient()
{
	ItemList::destroy( _itemList );

	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "Destroyed ItemCallbackClient." );
		temp.append( " OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}
}

ItemCallbackClient* ItemCallbackClient::create( OmmConsumerImpl& ommConsImpl )
{
	ItemCallbackClient* pClient = 0;

	try {
		pClient = new ItemCallbackClient( ommConsImpl );
	}
	catch ( std::bad_alloc ) {}

	if ( !pClient )
	{
		const char* temp = "Failed to create ItemCallbackClient";
		if ( OmmLoggerClient::ErrorEnum >= ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		throwMeeException( temp );
	}

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

RsslReactorCallbackRet ItemCallbackClient::processCallback( RsslReactor* pRsslReactor,
															RsslReactorChannel* pRsslReactorChannel,
															RsslMsgEvent* pEvent )
{
	RsslMsg* pRsslMsg = pEvent->pRsslMsg;

	if ( !pRsslMsg )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			RsslErrorInfo* pError = pEvent->pErrorInfo;

			EmaString temp( "Received an item event without RsslMsg message" );
			temp.append( CR )
				.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pError->rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( pError->rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( pError->rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( pError->errorLocation ).append( CR )
				.append( "Error Text " ).append( pError->rsslError.text );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if ( pRsslMsg->msgBase.streamId != 1 && 
		( !pEvent->pStreamInfo || !pEvent->pStreamInfo->pUserSpec ) )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received an item event without user specified pointer or stream info" );
			temp.append( CR )
				.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslReactorChannel " ).append( ptrToStringAsHex( pRsslReactorChannel ) ).append( CR )
				.append( "RsslSocket " ).append( (UInt64)pRsslReactorChannel->socketId );

			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	switch ( pRsslMsg->msgBase.msgClass )
	{
	case RSSL_MC_ACK :
		if ( pRsslMsg->msgBase.streamId == 1 )
			return _ommConsImpl.getLoginCallbackClient().processAckMsg( pRsslMsg, pRsslReactorChannel, 0 );
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
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received an item event with message containing unhandled message class" );
			temp.append( CR )
				.append( "Rssl Message Class " ).append( pRsslMsg->msgBase.msgClass ).append( CR )
				.append( "Consumer Name " ).append( _ommConsImpl.getConsumerName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslReactorChannel " ).append( ptrToStringAsHex( pRsslReactorChannel ) ).append( CR )
				.append( "RsslSocket" ).append( (UInt64)pRsslReactorChannel->socketId );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}
		break;
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processRefreshMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	StaticDecoder::setRsslData( &_refreshMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
 		static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->getDictionary()->getRsslDictionary() );

	_event._pItem = (Item*)( pEvent->pStreamInfo->pUserSpec );

	if ( _event._pItem->getType() == Item::BatchItemEnum )
		_event._pItem = static_cast<BatchItem *>(_event._pItem)->getSingleItem( pRsslMsg->msgBase.streamId );
	
	_refreshMsg.getDecoder().setServiceName( static_cast<SingleItem*>(_event._pItem)->getDirectory()->getName().c_str(),
		static_cast<SingleItem*>(_event._pItem)->getDirectory()->getName().length() );

	_event._pItem->getClient().onAllMsg( _refreshMsg, _event );
	_event._pItem->getClient().onRefreshMsg( _refreshMsg, _event );

	if ( _refreshMsg.getState().getStreamState() != OmmState::OpenEnum )
		_event._pItem->remove();

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processUpdateMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	StaticDecoder::setRsslData( &_updateMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
 		static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->getDictionary()->getRsslDictionary() );

	_event._pItem = (Item*)( pEvent->pStreamInfo->pUserSpec );

	if ( _event._pItem->getType() == Item::BatchItemEnum )
		_event._pItem = static_cast<BatchItem *>(_event._pItem)->getSingleItem( pRsslMsg->msgBase.streamId );
	
	_updateMsg.getDecoder().setServiceName( static_cast<SingleItem*>(_event._pItem)->getDirectory()->getName().c_str(),
		static_cast<SingleItem*>(_event._pItem)->getDirectory()->getName().length() );

	_event._pItem->getClient().onAllMsg( _updateMsg, _event );
	_event._pItem->getClient().onUpdateMsg( _updateMsg, _event );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processStatusMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	StaticDecoder::setRsslData( &_statusMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
 		static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->getDictionary()->getRsslDictionary() );

	_event._pItem = (Item*)( pEvent->pStreamInfo->pUserSpec );

	if ( _event._pItem->getType() == Item::BatchItemEnum )
		_event._pItem = static_cast<BatchItem *>(_event._pItem)->getSingleItem( pRsslMsg->msgBase.streamId );
	
	_statusMsg.getDecoder().setServiceName( reinterpret_cast<const SingleItem*>(_event._pItem)->getDirectory()->getName().c_str(),
		static_cast<SingleItem*>(_event._pItem)->getDirectory()->getName().length() );

	_event._pItem->getClient().onAllMsg( _statusMsg, _event );
	_event._pItem->getClient().onStatusMsg( _statusMsg, _event );

	if ( pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_STATE )
		if ( _statusMsg.getState().getStreamState() != OmmState::OpenEnum )
			_event._pItem->remove();

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processGenericMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	StaticDecoder::setRsslData( &_genericMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->getDictionary()->getRsslDictionary() );

	_event._pItem = (Item*)( pEvent->pStreamInfo->pUserSpec );

	if ( _event._pItem->getType() == Item::BatchItemEnum )
		_event._pItem  = static_cast<BatchItem *>(_event._pItem )->getSingleItem( pRsslMsg->msgBase.streamId );

	OmmConsumerClient& client = _event._pItem->getClient();
	client.onAllMsg( _genericMsg, _event );
	client.onGenericMsg( _genericMsg, _event );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processAckMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pEvent )
{
	StaticDecoder::setRsslData( &_ackMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->getDictionary()->getRsslDictionary() );

	_event._pItem = (Item*)( pEvent->pStreamInfo->pUserSpec );

	if ( _event._pItem->getType() == Item::BatchItemEnum )
		_event._pItem = static_cast<BatchItem *>(_event._pItem)->getSingleItem( pRsslMsg->msgBase.streamId );
	
	_ackMsg.getDecoder().setServiceName( reinterpret_cast<const SingleItem*>(_event._pItem)->getDirectory()->getName().c_str(),
		static_cast<SingleItem*>(_event._pItem)->getDirectory()->getName().length() );

	OmmConsumerClient& client = _event._pItem->getClient();
	client.onAllMsg( _ackMsg, _event );
	client.onAckMsg( _ackMsg, _event );

	return RSSL_RC_CRET_SUCCESS;
}

UInt64 ItemCallbackClient::registerClient( const ReqMsg& reqMsg, OmmConsumerClient& ommConsClient,
											void* closure )
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );

	switch ( reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType )
	{
	case RSSL_DMT_LOGIN :
		{
			SingleItem* pItem = _ommConsImpl.getLoginCallbackClient().getLoginItem( reqMsg, ommConsClient, closure );

			if ( pItem )
			{
				addToList( pItem );
				addToMap( pItem );
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
				temp.append( ". OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

				if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
					_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( temp );
				else
					throwIueException( temp );

				return 0;
			}

			if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME )
			{
				EmaString name( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.data, reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length );

				if ( ( name != "RWFFld" ) && ( name != "RWFEnum" ) )
				{
					EmaString temp( "Invalid ReqMsg's name : " );
					temp.append( name );
					temp.append( "\nReqMsg's name must be \"RWFFld\" or \"RWFEnum\" for MMT_DICTIONARY domain type. ");
					temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

					if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
						_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

					if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
						_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( temp );
					else
						throwIueException( temp );

					return 0;
				}
			}
			else
			{
				EmaString temp( "ReqMsg's name is not defined. " );
				temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

				if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
					_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( temp );
				else
					throwIueException( temp );

				return 0;
			}

			DictionaryItem* pItem = DictionaryItem::create( _ommConsImpl, ommConsClient, closure );

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
			const ChannelList& channels( _ommConsImpl.getChannelCallbackClient().getChannelList() );
			const Channel* c( channels.front() );
			while( c )
			{
				DirectoryItem* pItem = DirectoryItem::create( _ommConsImpl, ommConsClient, closure, c );

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
				
				c = c->next();
			}

			return 0;
		}
	default :
		{
			SingleItem* pItem  = 0;

			if ( reqMsgEncoder.getRsslRequestMsg()->flags & RSSL_RQMF_HAS_BATCH )
			{
				BatchItem* pBatchItem = BatchItem::create( _ommConsImpl, ommConsClient, closure );

				if ( pBatchItem )
				{
					try {
						pItem = pBatchItem;
						pBatchItem->addBatchItems( reqMsgEncoder.getBatchItemList() );

						if ( !pBatchItem->open( reqMsg ) )
						{
							for ( UInt32 i = 1 ; i < pBatchItem->getSingleItemList().size() ; i++ )
							{
								Item::destroy( (Item*&)pBatchItem->getSingleItemList()[i] );
							}

							Item::destroy( (Item*&)pItem );
						}
						else
						{
							addToList( pBatchItem );
							addToMap( pBatchItem );

							for ( UInt32 i = 1 ; i < pBatchItem->getSingleItemList().size() ; i++ )
							{
								addToList( pBatchItem->getSingleItemList()[i] );
								addToMap( pBatchItem->getSingleItemList()[i] );
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
				pItem = SingleItem::create( _ommConsImpl, ommConsClient, closure, 0 );

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

void ItemCallbackClient::reissue( const ReqMsg& reqMsg, UInt64 handle )
{
	if ( !_itemMap.find( handle ) )
	{
		EmaString temp( "Attempt to use invalid Handle on reissue(). " );
		temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
			_ommConsImpl.getOmmConsumerErrorClient().onInvalidHandle( handle, temp );
		else
			throwIheException( handle, temp );

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
		temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
			_ommConsImpl.getOmmConsumerErrorClient().onInvalidHandle( handle, temp );
		else
			throwIheException( handle, temp );

		return;
	}

	((Item*)handle)->submit( postMsg );
}

void ItemCallbackClient::submit( const GenericMsg& genericMsg, UInt64 handle )
{
	if ( !_itemMap.find( handle ) )
	{
		EmaString temp( "Attempt to use invalid Handle on submit( const GenericMsg& ). " );
		temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
			_ommConsImpl.getOmmConsumerErrorClient().onInvalidHandle( handle, temp );
		else
			throwIheException( handle, temp );

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
}

void ItemCallbackClient::removeFromMap( Item* pItem )
{
	_itemMap.erase( (UInt64)pItem );
}
