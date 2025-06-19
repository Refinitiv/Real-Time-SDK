/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2021,2024-2025 LSEG. All rights reserved.
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
#include "OmmServerBaseImpl.h"
#include "OmmIProviderImpl.h"
#include "ServerChannelHandler.h"
#include "OmmInvalidUsageException.h"
#include "ConsumerRoutingSession.h"
#include "ConsumerRoutingChannel.h"

#include "rtr/rsslMsgKey.h"

#include <limits.h>

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;
using namespace std;

const EmaString ItemCallbackClient::_clientName( "ItemCallbackClient" );
const EmaString SingleItem::_clientName( "SingleItem" );
const EmaString NiProviderSingleItem::_clientName( "NiProviderSingleItem" );
const EmaString IProviderSingleItem::_clientName( "IProviderSingleItem" );
const EmaString ItemList::_clientName( "ItemList" );
const EmaString BatchItem::_clientName( "BatchItem" );
const EmaString TunnelItem::_clientName( "TunnelItem" );
const EmaString SubItem::_clientName( "SubItem" );

const EmaString SingleItemString( "SingleItem" );
const EmaString NiProviderSingleItemString( "NiProviderSingleItem" );
const EmaString IProviderSingleItemString("IProviderSingleItem");
const EmaString BatchItemString( "BatchItem" );
const EmaString loginItemString( "LoginItem" );
const EmaString NiProviderLoginItemString( "NiProviderLoginItem" );
const EmaString DirectoryItemString( "DirectoryItem" );
const EmaString DictionaryItemString( "DictionaryItem" );
const EmaString NiProviderDictionaryItemString( "NiProviderDictionaryItem" );
const EmaString IProviderDictionaryItemString("IProviderDictionaryItem");
const EmaString TunnelItemString( "TunnelItem" );
const EmaString SubItemString( "SubItem" );
const EmaString UnknownItemString( "UnknownItem" );

#define CONSUMER_STARTING_STREAM_ID	4
#define PROVIDER_STARTING_STREAM_ID 0
#define CONSUMER_MAX_STREAM_ID_MINUSONE (INT_MAX - 1)
#define INITIAL_ITEM_WATCHLIST_SIZE 10

Item::Item( ) :
	_domainType( 0 ),
	_streamId( 0 ),
	_closedInfo( 0 ),
	_currentItemList(NULL)
{
}

Item::~Item()
{
	if (_closedInfo)
	{
		delete _closedInfo;
		_closedInfo = 0;
	}

	if (_currentItemList)
	{
		_currentItemList->removeItem(this);
	}
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
	case Item::IProviderSingleItemEnum:
		return IProviderSingleItemString;
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
	case Item::IProviderDictionaryItemEnum:
		return IProviderDictionaryItemString;
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

ItemStatusInfo*	Item::getClosedStatusInfo()
{
	return _closedInfo;
}

void Item::setItemList(ItemList* pItemList)
{
	_currentItemList = pItemList;
}

ItemList* Item::getItemList()
{
	return _currentItemList;
}

ConsumerItem::ConsumerItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, Item* pParentItem ) :
	_ommBaseImpl( ommBaseImpl ),
	_client( ommConsClient ),
	_event(ommBaseImpl),
	currentServiceListIndex(0)
{
	_event._handle = ( UInt64 )this;
	_event._closure = closure;
	_event._parentHandle = (UInt64) pParentItem;

	_event._channel = (void*)ommBaseImpl.getRsslReactorChannel();

	sessionChannel = NULL;
	sessionChannelItemClosedList = NULL;
}

ConsumerItem::~ConsumerItem()
{
	if (_currentItemList)
	{
		_currentItemList->removeItem(this);
	}

	if (sessionChannelItemClosedList != NULL)
	{
		for (UInt32 i = 0; i < sessionChannelItemClosedList->size(); i++)
		{
			free((void*)((*sessionChannelItemClosedList)[i]));
		}
		delete sessionChannelItemClosedList;
	}
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

OmmBaseImpl& ConsumerItem::getImpl()
{
	return _ommBaseImpl;
}

Int32 ConsumerItem::getNextStreamId(int numOfItem)
{
	return _ommBaseImpl.getItemCallbackClient().getNextStreamId(numOfItem);
}

void ConsumerItem::setEventChannel(void* channel)
{
	_event._channel = channel;
}


ProviderItem::ProviderItem(OmmCommonImpl& ommCommonImpl,OmmProviderClient& ommProvClient, ItemWatchList* pItemWatchList, void* closure) :
	_ommCommonImpl( ommCommonImpl ),
	_client( ommProvClient ),
	_event(ommCommonImpl),
	_pItemWatchList(pItemWatchList),
	_pClientSession(0),
	_pTimeOut(0),
	_isPrivateStream(false),
	_receivedInitResp(false),
	_timeOutExpired(false),
	_specifiedServiceInReq(false)
{
	
	rsslClearMsgKey( &_msgKey );
	_event._handle = ( UInt64 )this;
	_event._closure = closure;
	_event._clientHandle = 0;
}

ProviderItem::~ProviderItem()
{
	cancelReqTimerEvent();

	if ( _pItemWatchList )
	{
		_pItemWatchList->removeItem(this);
	}
}

const EmaString& ProviderItem::getServiceName()
{
	return _serviceName;
}

UInt64	ProviderItem::getServiceId()
{
	return _serviceId;
}

void ProviderItem::cancelReqTimerEvent()
{
	if ( !_timeOutExpired && !_ommCommonImpl.isAtExit() && ( _pTimeOut && !_pTimeOut->isCanceled() ) )
	{
		_pTimeOut->cancel();
		_timeOutExpired = true;
	}

}

bool ProviderItem::processInitialResp(RsslRefreshMsg* rsslRefreshMsg, bool checkPrivateStream)
{
	bool result = true;

	if (_receivedInitResp == false)
	{
		if (_domainType != rsslRefreshMsg->msgBase.domainType)
		{
			result = false;
		}

		bool respWithPrivateStream = (rsslRefreshMsg->flags & RSSL_RFMF_PRIVATE_STREAM) != 0 ? true : false;

		if ( checkPrivateStream )
		{
			result = ( _isPrivateStream == respWithPrivateStream ) ? true : false;
		}
		else
		{
			_isPrivateStream = respWithPrivateStream;
		}

		if ( rsslCompareMsgKeys(&_msgKey, &rsslRefreshMsg->msgBase.msgKey) != RSSL_RET_SUCCESS )
		{
			return false;
		}

		_receivedInitResp = true;
	}

	return result;
}

void ProviderItem::onAllMsg( const Msg& msg )
{
	_client.onAllMsg( msg, _event );
}

void ProviderItem::onRefreshMsg( const RefreshMsg& msg )
{
	_client.onRefreshMsg( msg, _event );
}

void ProviderItem::onStatusMsg( const StatusMsg& msg )
{
	_client.onStatusMsg( msg, _event );
}

void ProviderItem::onGenericMsg( const GenericMsg& msg )
{
	_client.onGenericMsg( msg, _event );
}

const RsslMsgKey& ProviderItem::getRsslMsgKey()
{
	return _msgKey;
}

bool ProviderItem::isPrivateStream()
{
	return _isPrivateStream;
}

Directory* ProviderItem::getDirectory()
{
	return NULL;
}

void ProviderItem::setProvider(OmmProvider* pOmmProvider)
{
	_event._provider = pOmmProvider;
}

const ClientSession* ProviderItem::getClientSession()
{
	return _pClientSession;
}

void ProviderItem::setClientSession( const ClientSession* pClientSession )
{
	_pClientSession = pClientSession;
	_event._clientHandle = _pClientSession->getClientHandle();
}

TimeOut* ProviderItem::getTimeOut()
{
	return _pTimeOut;
}

bool ProviderItem::modify( const ReqMsg& reqMsg )
{
	if ( _closedInfo ) return false;

	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder());

	if (reqMsgEncoder.hasServiceName())
	{
		if ( _specifiedServiceInReq &&( reqMsgEncoder.getServiceName() == _serviceName ) )
		{
			reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId = (UInt16)_serviceId;
			reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		}
		else
		{
			EmaString temp("Service name of '");
			temp.append(reqMsgEncoder.getServiceName())
				.append("' does not match existing request.")
				.append("Instance name='").append(_ommCommonImpl.getInstanceName()).append("'.");

			_ommCommonImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			return false;
		}
	}
	else if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID )
	{
		if ( !_specifiedServiceInReq || ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId != (UInt16)_serviceId ) )
		{
			EmaString temp("Service id of '");
			temp.append(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId)
				.append("' does not match existing request")
				.append("Instance name='").append(_ommCommonImpl.getInstanceName()).append("'.");

			_ommCommonImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			return false;
		}
	}
	else
	{
		if ( _specifiedServiceInReq )
		{
			reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId = (UInt16)_serviceId;
			reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		}
	}

	if (reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
	{
		if ((reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length != _msgKey.name.length) ||
			memcmp(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.data, _msgKey.name.data, _msgKey.name.length) != 0)
		{
			EmaString temp("Name of '");
			temp.append(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.data)
				.append("' does not match existing request.")
				.append("Instance name='").append(_ommCommonImpl.getInstanceName()).append("'.");

			_ommCommonImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			return false;
		}
	}
	else
	{
		reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.data = _msgKey.name.data;
		reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length = _msgKey.name.length;
		reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.nameType = _msgKey.nameType;
		reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	}

	return submit(static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder()).getRsslRequestMsg());
}

SingleItem* SingleItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, Item* pParentItem )
{
	try {
		return new SingleItem( ommBaseImpl, ommConsClient, closure, pParentItem );
	}
	catch( std::bad_alloc& )
	{
		ommBaseImpl.handleMee( "Failed to create SingleItem" );
	}

	return NULL;
}

SingleItem::SingleItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, Item* pParentItem ) :
	ConsumerItem( ommBaseImpl, ommConsClient, closure, pParentItem ),
	_pDirectory( 0 ),
	_serviceName(),
	_serviceListName(),
	serviceListIter(0),
	_serviceId(0),
	_hasServiceId(false)
{
	rsslClearRequestMsg(&_requestMsg);
}

SingleItem::~SingleItem()
{
	if (_currentItemList)
	{
		_currentItemList->removeItem(this);
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

void SingleItem::setDirectory(Directory* directory)
{
	_pDirectory = directory;
}


bool SingleItem::open( const ReqMsg& reqMsg )
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );
	
	const Directory* pDirectory = 0;

	// Copy the current request message 
 	setReqMsg(reqMsg, NULL);

	if (reqMsgEncoder.hasServiceListName())
	{
		serviceListIter = 0;
		if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
		{
			ServiceList** pServiceListPtr = _ommBaseImpl.getActiveConfig().serviceListByName.find(&_serviceListName);

			if (pServiceListPtr == NULL)
			{
				EmaString temp("Service List name of '");
				temp.append(reqMsgEncoder.getServiceListName()).
					append("' is not found.");
				scheduleItemClosedStatus(reqMsgEncoder, temp);

				return true;
			}

			EmaVector<EmaString>& concreteServiceList = (*pServiceListPtr)->concreteServiceList();

			// Iterate through the concrete service list, if there is a match, set the directory here and continue.
			for (UInt32 i = 0; i < concreteServiceList.size(); ++i)
			{
				pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory(concreteServiceList[i]);
				if (!pDirectory)
				{
					continue;
				}
				_pDirectory = pDirectory;
			}

			if (_pDirectory == NULL)
			{
				EmaString temp("Service List name of '");
				temp.append(reqMsgEncoder.getServiceListName()).
					append("' does not have any active services.");
				scheduleItemClosedStatus(reqMsgEncoder, temp);

				return true;
			}
		}
		else
		{

			ServiceList** pServiceListPtr = _ommBaseImpl.getActiveConfig().serviceListByName.find(&_serviceListName);

			if (pServiceListPtr == NULL)
			{
				EmaString temp("Service List name of '");
				temp.append(reqMsgEncoder.getServiceListName()).
					append("' is not found.");
				scheduleItemClosedStatus(reqMsgEncoder, temp);

				return true;
			}

			if (_ommBaseImpl.getConsumerRoutingSession()->matchRequestToSessionChannel(*this) == false)
			{
				if (reqMsgEncoder.getRsslRequestMsg()->flags & RSSL_RQMF_PRIVATE_STREAM)
				{
					EmaString temp("Service list name of '");
					temp.append(reqMsgEncoder.getServiceListName()).
						append("' is not found. Closing this private stream request.");
					scheduleItemClosedStatus(reqMsgEncoder, temp);

					return true;
				}
				else
				{
					// Add to the pending request list
					sessionChannel = NULL;
					setDirectory(NULL);
					_ommBaseImpl.getConsumerRoutingSession()->pendingRequestList.addItem(this);

					if ((reqMsgEncoder.getRsslRequestMsg()->flags & RSSL_RQMF_HAS_BATCH) != 0)
					{
						/*Send stream close status for the batch stream */
						EmaString temp("Batch request acknowledged.");
						scheduleItemClosedStatus(reqMsgEncoder, temp);
					}
					else
					{
						EmaString temp("Service List name of '");
						temp.append(reqMsgEncoder.getServiceListName()).
							append("' does not match any active services, will request when they become available.");

						scheduleItemSuspectStatus(reqMsgEncoder, temp);
					}
				}

				return true;
			}
		}
	}
	else if ( reqMsgEncoder.hasServiceName() )
	{
		if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
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
			_pDirectory = pDirectory;
		}
		else
		{
			if (_ommBaseImpl.getConsumerRoutingSession()->matchRequestToSessionChannel(*this) == false)
			{
				if (reqMsgEncoder.getRsslRequestMsg()->flags & RSSL_RQMF_PRIVATE_STREAM)
				{
					EmaString temp("Service name of '");
					temp.append(reqMsgEncoder.getServiceName()).
						append("' is not found.");
					scheduleItemClosedStatus(reqMsgEncoder, temp);
				}
				else
				{
					// Add to the pending request list
					this->sessionChannel = NULL;
					this->setDirectory(NULL);
					_ommBaseImpl.getConsumerRoutingSession()->pendingRequestList.addItem(this);

					if ((reqMsgEncoder.getRsslRequestMsg()->flags & RSSL_RQMF_HAS_BATCH) != 0)
					{
						/*Send stream close status for the batch stream */
						EmaString temp("Batch request acknowledged.");
						scheduleItemClosedStatus( reqMsgEncoder, temp );
					}
					else
					{
						EmaString temp("Service List name of '");
						temp.append(reqMsgEncoder.getServiceListName()).
							append("' does not match any active service, will request when it becomes available.");
						scheduleItemSuspectStatus(reqMsgEncoder, temp);
					}
				}

				return true;
			}
		}
	}
	else
	{

		if (reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID)
		{
			setServiceId((UInt32)reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId);
			if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
			{
				pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId);

				if ( !pDirectory )
				{
					EmaString temp( "Service id of '" );
					temp.append( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId ).
						append( "' is not found." );
					scheduleItemClosedStatus( reqMsgEncoder, temp );

					return true;
				}

				_pDirectory = pDirectory;
			}
			else
			{
				if (_ommBaseImpl.getConsumerRoutingSession()->matchRequestToSessionChannel(*this) == false)
				{
					ConsumerRoutingService** pRoutingServicePtr = _ommBaseImpl.getConsumerRoutingSession()->serviceById.find(_serviceId);
					if (!pRoutingServicePtr)
					{
						EmaString temp("Service id of '");
						temp.append(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId).
							append("' is not found.");
						scheduleItemClosedStatus(reqMsgEncoder, temp);
					}
					else
					{
						// Add to the pending request list
						this->sessionChannel = NULL;
						this->setDirectory(NULL);
						_ommBaseImpl.getConsumerRoutingSession()->pendingRequestList.addItem(this);

						EmaString temp("Request with service id of '");
						temp.append(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId).
							append("' does not match any active service, will request when one becomes available.");
						scheduleItemSuspectStatus(reqMsgEncoder, temp);
					}
					

					return true;
	}

				// matchRequestToSessionChannel returning true means that there is a match for the service, so we can just get it and set the name in the service list here.
				ConsumerRoutingService** pRoutingServicePtr = _ommBaseImpl.getConsumerRoutingSession()->serviceById.find(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId);

				_serviceName = (*pRoutingServicePtr)->getName();
			}
		}
		else
		{
			EmaString temp( "Passed in request message does not identify any service." );
			scheduleItemClosedStatus( reqMsgEncoder, temp );

			return true;
		}
	}


	// Only submit the message if a sessionChannel has been defined to route to.
	if (_ommBaseImpl.getConsumerRoutingSession() == NULL || sessionChannel != NULL)
		return submit( reqMsgEncoder.getRsslRequestMsg() );
	else
		return true;
}

bool SingleItem::reSubmit(bool reroute)
{
	const Directory* pDirectory = 0;

	if (reroute == true)
	{
		if (_ommBaseImpl.getConsumerRoutingSession()->matchRequestToSessionChannel(*this) == false)
		{
			// Add to the pending request list
			this->sessionChannel = NULL;
			this->setDirectory(NULL);
			_ommBaseImpl.getConsumerRoutingSession()->pendingRequestList.addItem(this);
		}
	}
	
	// Only submit the message if a sessionChannel has been defined to route to.
	if (sessionChannel != NULL)
		return submit(&_requestMsg);
	else
		return true;
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

bool SingleItem::sendClose()
{
	RsslCloseMsg rsslCloseMsg;
	rsslClearCloseMsg(&rsslCloseMsg);

	rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslCloseMsg.msgBase.domainType = _domainType;

	bool retCode = submit(&rsslCloseMsg);

	return retCode;
}

bool SingleItem::submit( const PostMsg& postMsg )
{
	const PostMsgEncoder& postMsgEncoder = static_cast<const PostMsgEncoder&>( postMsg.getEncoder() );
	if (_ommBaseImpl.getConsumerRoutingSession() != NULL)
	{
		if (sessionChannel == NULL)
		{
			EmaString text("Failed to submit PostMsg. Reason: No current session channel for this handle");

			_ommBaseImpl.handleIue(text, OmmInvalidUsageException::InvalidArgumentEnum);
			return false;
		}

		if (postMsgEncoder.hasServiceName())
		{
			Directory** pDirectoryPtr = sessionChannel->serviceByName.find(&postMsgEncoder.getServiceName());

			if (pDirectoryPtr == NULL)
			{
				EmaString temp("Failed to submit PostMsg on item stream. Reason: Service name of '");
				temp.append(postMsgEncoder.getServiceName()).append("' is not found.");

				_ommBaseImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);

				return false;
			}
			Directory* pDirectory = *pDirectoryPtr;

			const EmaString& serviceName = pDirectory->getName();

			RsslBuffer serviceNameBuffer = RSSL_INIT_BUFFER;
			serviceNameBuffer.data = (char*)serviceName.c_str();
			serviceNameBuffer.length = serviceName.length();

			return submit(static_cast<const PostMsgEncoder&>(postMsg.getEncoder()).getRsslPostMsg(), &serviceNameBuffer);
		}
		else
		{
			ConsumerRoutingService** pRoutingServicePtr = _ommBaseImpl.getConsumerRoutingSession()->serviceById.find(postMsgEncoder.getRsslPostMsg()->msgBase.msgKey.serviceId);
			if (!pRoutingServicePtr)
			{
				EmaString text("Failed to submit PostMsg. Reason: invalid service id");

				_ommBaseImpl.handleIue(text, OmmInvalidUsageException::InvalidArgumentEnum);
				return false;
			}

			ConsumerRoutingService* pRoutingService = *pRoutingServicePtr;
			const EmaString& serviceName = pRoutingService->getName();

			RsslBuffer serviceNameBuffer = RSSL_INIT_BUFFER;
			serviceNameBuffer.data = (char*)serviceName.c_str();
			serviceNameBuffer.length = serviceName.length();

			postMsgEncoder.getRsslPostMsg()->msgBase.msgKey.flags &= ~RSSL_MKF_HAS_SERVICE_ID;

			return submit(static_cast<const PostMsgEncoder&>(postMsg.getEncoder()).getRsslPostMsg(), &serviceNameBuffer);
		}
	}
	else
	{
		if (postMsgEncoder.hasServiceName())
		{
			const Directory* pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory(postMsgEncoder.getServiceName());
			if (!pDirectory)
			{
				EmaString temp("Failed to submit PostMsg on item stream. Reason: Service name of '");
				temp.append(postMsgEncoder.getServiceName()).append("' is not found.");

				_ommBaseImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);

				return false;
			}

			const EmaString& serviceName = pDirectory->getName();

			RsslBuffer serviceNameBuffer = RSSL_INIT_BUFFER;
			serviceNameBuffer.data = (char*)serviceName.c_str();
			serviceNameBuffer.length = serviceName.length();

			return submit(static_cast<const PostMsgEncoder&>(postMsg.getEncoder()).getRsslPostMsg(), &serviceNameBuffer);
		}

		return submit(static_cast<const PostMsgEncoder&>(postMsg.getEncoder()).getRsslPostMsg(), NULL);
	}
}

bool SingleItem::submit( const GenericMsg& genMsg )
{
	const GenericMsgEncoder& genericMsgEncoder = static_cast<const GenericMsgEncoder&>(genMsg.getEncoder());
	if (_ommBaseImpl.getConsumerRoutingSession() != NULL)
	{
		if (sessionChannel == NULL)
		{
			EmaString text("Failed to submit GenericMsg. Reason: No current session channel for this handle");

			_ommBaseImpl.handleIue(text, OmmInvalidUsageException::InvalidArgumentEnum);
			return false;
		}

		if(genericMsgEncoder.hasServiceId())
		{
			ConsumerRoutingService** pRoutingServicePtr = _ommBaseImpl.getConsumerRoutingSession()->serviceById.find(genericMsgEncoder.getRsslGenericMsg()->msgBase.msgKey.serviceId);
			if (!pRoutingServicePtr)
			{
				return submit(static_cast<const GenericMsgEncoder&>(genMsg.getEncoder()).getRsslGenericMsg());
			}

			ConsumerRoutingService* pRoutingService = *pRoutingServicePtr;

			if (pRoutingService->routingChannelList[sessionChannel->sessionIndex] != NULL)
			{
				Directory** pDirectoryPtr = sessionChannel->serviceByName.find(&pRoutingService->getName());

				if (pDirectoryPtr == NULL)
				{
					return submit(static_cast<const GenericMsgEncoder&>(genMsg.getEncoder()).getRsslGenericMsg());
				}
				Directory* pDirectory = *pDirectoryPtr;

				genericMsgEncoder.getRsslGenericMsg()->msgBase.msgKey.serviceId = (RsslUInt16)pDirectory->getService()->serviceId;

				return submit(static_cast<const GenericMsgEncoder&>(genMsg.getEncoder()).getRsslGenericMsg());
			}
			else
			{
				return submit(static_cast<const GenericMsgEncoder&>(genMsg.getEncoder()).getRsslGenericMsg());
			}
		}
	}

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

EmaString& SingleItem::getServiceName()
{
	return _serviceName;
}

void SingleItem::setServiceName(EmaString& serviceName)
{
	_serviceName = serviceName;
}

UInt32 SingleItem::getServiceId()
{
	return _serviceId;
}

void SingleItem::setServiceId(UInt32 serviceId)
{
	_serviceId = serviceId;
	_hasServiceId = true;
}

bool SingleItem::hasServiceId()
{
	return _hasServiceId;
}

EmaString& SingleItem::getServiceListName()
{
	return _serviceListName;
}

bool SingleItem::submit( RsslRequestMsg* pRsslRequestMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	RsslReactorChannel* pReactorChannel;

	if (sessionChannel != NULL)
	{
		pReactorChannel = sessionChannel->pReactorChannel;
	}
	else
	{
		pReactorChannel = _ommBaseImpl.getRsslReactorChannel();
	}

	bool serviceIdSet = ( pRsslRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) ? true : false;

	// overwrite the service Id if it is set with the one in the selected directory.  For non-request routing, the cached directory will have the same serviceId as the request.
	if (serviceIdSet)
	{
		pRsslRequestMsg->msgBase.msgKey.serviceId = (RsslUInt16)_pDirectory->getId();
	}

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

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;

	submitMsgOpts.requestMsgOptions.pUserSpec = ( void* )this;
	
	Int32 origStreamId = submitMsgOpts.pRsslMsg->msgBase.streamId;

	if ( !_streamId )
	{
		if ( pRsslRequestMsg->flags & RSSL_RQMF_HAS_BATCH )
		{
			const EmaVector<SingleItem*>& singleItemList = static_cast<BatchItem &>( *this ).getSingleItemList();

			submitMsgOpts.pRsslMsg->msgBase.streamId = getNextStreamId( singleItemList.size() - 1 );
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
			submitMsgOpts.pRsslMsg->msgBase.streamId = getNextStreamId();

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
	if ( ( ret = rsslReactorSubmitMsg(_ommBaseImpl.getRsslReactor(),
		pReactorChannel,
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

		_ommBaseImpl.handleIue( text, ret );

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

	RsslReactorChannel* pReactorChannel;

	if (sessionChannel != NULL)
	{
		pReactorChannel = sessionChannel->pReactorChannel;
	}
	else
	{
		pReactorChannel = _ommBaseImpl.getRsslReactorChannel();
	}

	submitMsgOpts.pRsslMsg = (RsslMsg*)pRsslCloseMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;
	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg(_ommBaseImpl.getRsslReactor(),
										pReactorChannel,
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

		_ommBaseImpl.handleIue( text, ret );

		return false;
	}

	return true;
}

bool SingleItem::submitCloseOnReactorChannel (RsslReactorChannel* pReactorChannel )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	RsslCloseMsg rsslCloseMsg;
	rsslClearCloseMsg(&rsslCloseMsg);

	rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslCloseMsg.msgBase.domainType = _domainType;

	submitMsgOpts.pRsslMsg = (RsslMsg*)&rsslCloseMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;
	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg(_ommBaseImpl.getRsslReactor(),
										pReactorChannel,
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

		_ommBaseImpl.handleIue( text, ret );

		return false;
	}

	return true;
}

bool SingleItem::submit( RsslGenericMsg* pRsslGenericMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	RsslReactorChannel* pReactorChannel;

	if (sessionChannel != NULL)
	{
		pReactorChannel = sessionChannel->pReactorChannel;
	}
	else
	{
		pReactorChannel = _ommBaseImpl.getRsslReactorChannel();
	}

	submitMsgOpts.pRsslMsg = (RsslMsg*)pRsslGenericMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;
	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;
	if (submitMsgOpts.pRsslMsg->msgBase.domainType == 0)
		submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg(_ommBaseImpl.getRsslReactor(),
										pReactorChannel,
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

		_ommBaseImpl.handleIue( text, ret );

		return false;
	}

	return true;
}

bool SingleItem::submit( RsslPostMsg* pRsslPostMsg, RsslBuffer* pServiceName )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	RsslReactorChannel* pReactorChannel;

	if (sessionChannel != NULL)
	{
		pReactorChannel = sessionChannel->pReactorChannel;
	}
	else
	{
		pReactorChannel = _ommBaseImpl.getRsslReactorChannel();
	}

	submitMsgOpts.pRsslMsg = (RsslMsg*)pRsslPostMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;
	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;
	submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	submitMsgOpts.pServiceName = pServiceName;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _ommBaseImpl.getRsslReactor(),
										pReactorChannel,
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

		_ommBaseImpl.handleIue( text, ret );

		return false;
	}

	return true;
}

// This is used for a non-batch request.
void SingleItem::setReqMsg(const ReqMsg& reqMsg, EmaString* name)
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder());


	// Shallow copy the request message here
	_requestMsg = *reqMsgEncoder.getRsslRequestMsg();

	_requestMsg.flags &= ~RSSL_RQMF_HAS_BATCH;

	if (name != NULL)
	{
		// This is only used for the individual batch item requests, so we also need to set the HAS_NAME flag
		itemName = *name;
		_requestMsg.msgBase.msgKey.flags |= RSSL_MKF_HAS_NAME;
	}
	else
		itemName.set(_requestMsg.msgBase.msgKey.name.data, _requestMsg.msgBase.msgKey.name.length);

	_requestMsg.msgBase.msgKey.name.data = (char*)itemName.c_str();
	_requestMsg.msgBase.msgKey.name.length = itemName.length();

	if (_requestMsg.msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB)
	{
		_msgKeyAttrib.setFrom(_requestMsg.msgBase.msgKey.encAttrib.data, _requestMsg.msgBase.msgKey.encAttrib.length);
		_requestMsg.msgBase.msgKey.encAttrib.data = (char*)_msgKeyAttrib.c_buf();
		_requestMsg.msgBase.msgKey.encAttrib.length = _msgKeyAttrib.length();

	}

	if (_requestMsg.flags & RSSL_RQMF_HAS_VIEW)
	{
		if (reqMsgEncoder.getViewPayload(_payload) == true)
		{
			_requestMsg.msgBase.encDataBody.data = (char*)_payload.c_buf();
			_requestMsg.msgBase.encDataBody.length = _payload.length();
		}
		else
		{
			_requestMsg.flags &= ~RSSL_RQMF_HAS_VIEW;
		}
	}

	if (reqMsgEncoder.hasServiceListName())
	{
		_serviceListName = reqMsgEncoder.getServiceListName();
	}

	if (reqMsgEncoder.hasServiceName())
	{
		_serviceName = reqMsgEncoder.getServiceName();
	}
}

RsslRequestMsg* SingleItem::getReqMsg()
{
	return &_requestMsg;
}



NiProviderSingleItem* NiProviderSingleItem::create( OmmBaseImpl& ommBaseImpl, OmmProviderClient& ommProvClient, void* closure, Item* pParentItem )
{
	try
	{
		return new NiProviderSingleItem( ommBaseImpl, ommProvClient, &static_cast<OmmNiProviderImpl&>(ommBaseImpl).getItemWatchList(), closure, pParentItem );
	}
	catch ( std::bad_alloc& )
	{
		ommBaseImpl.handleMee( "Failed to create NiProviderSingleItem" );
	}

	return NULL;
}

NiProviderSingleItem::NiProviderSingleItem( OmmBaseImpl& ommBaseImpl, OmmProviderClient& ommProvClient, ItemWatchList* pItemWatchList, void* closure, Item* ) :
	ProviderItem( ommBaseImpl, ommProvClient, pItemWatchList, closure ),
	_ommBaseImpl( ommBaseImpl )
{
	setProvider( static_cast<OmmNiProviderImpl&>(ommBaseImpl).getProvider() );

	_event._channel = ommBaseImpl.getRsslReactorChannel();
}

NiProviderSingleItem::~NiProviderSingleItem()
{
	if (_currentItemList)
	{
		_currentItemList->removeItem(this);
	}

	_ommBaseImpl.getItemCallbackClient().removeFromMap(this);
}

Item::ItemType NiProviderSingleItem::getType() const
{
	return Item::NiProviderSingleItemEnum;
}

OmmBaseImpl& NiProviderSingleItem::getImpl()
{
	return _ommBaseImpl;
}

Int32 NiProviderSingleItem::getNextStreamId(int numOfItem)
{
	return static_cast<OmmNiProviderImpl&>(_ommBaseImpl).getNextProviderStreamId();
}

bool NiProviderSingleItem::open( const ReqMsg& reqMsg )
{
	EmaString serviceName;
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

		serviceName = reqMsgEncoder.getServiceName();

		reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId = (UInt16) serviceId;
		reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;
		_serviceName = serviceName;

		_specifiedServiceInReq = true;
	}
	else if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID )
	{
		if ( !static_cast<OmmNiProviderImpl&>(_ommBaseImpl).getServiceName(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId, serviceName) )
		{
			EmaString temp("Service id of '");
			temp.append(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId).
				append("' is not found.");
			scheduleItemClosedStatus(reqMsgEncoder, temp);

			return true;
		}

		serviceId = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId;

		_specifiedServiceInReq = true;
	}

	if (reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
	{
		_msgKey.name.data = (char*)malloc(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length);

		if (!_msgKey.name.data)
		{
			const char* text = "Failed to allocate memory in NiProviderSingleItem::open( const ReqMsg& ).";
			throwMeeException(text);
		}

		_msgKey.name.length = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length;
	}

	if (reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB)
	{
		_msgKey.encAttrib.data = (char*)malloc(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.encAttrib.length + 1);
		if (!_msgKey.encAttrib.data)
		{
			throwMeeException("Failed to allocate memory for encoded attrib in NiProviderSingleItem::open( const ReqMsg& ).");
		}
		_msgKey.encAttrib.length = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.encAttrib.length + 1;
	}

	_isPrivateStream = reqMsgEncoder.getPrivateStream();

	if ( _specifiedServiceInReq )
	{
		_serviceName = serviceName;
		_serviceId = serviceId;
	}

	rsslCopyMsgKey(&_msgKey, &reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey);
	_pItemWatchList->addItem(this);

	return submit( reqMsgEncoder.getRsslRequestMsg() );
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

bool NiProviderSingleItem::submit( const GenericMsg& genMsg )
{
	return false;
}

void NiProviderSingleItem::remove()
{
	static_cast<OmmNiProviderImpl&>(_ommBaseImpl).returnProviderStreamId(_streamId);

	delete this;
}

bool NiProviderSingleItem::submit( RsslRequestMsg* pRsslRequestMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	RsslReactorChannel* pReactorChannel = _ommBaseImpl.getRsslReactorChannel();

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

	if (pReactorChannel == NULL )
		{
			EmaString temp( "No active channel to send message." );
			_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::FailureEnum );

			return false;
		}

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;

	if ( !_streamId )
	{
		if (!submitMsgOpts.pRsslMsg->msgBase.streamId)
			submitMsgOpts.pRsslMsg->msgBase.streamId = getNextStreamId();

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
	if ( ( ret = rsslReactorSubmitMsg(_ommBaseImpl.getRsslReactor(), pReactorChannel,
		&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in NiProviderSingleItem::submit( RsslRequestMsg* )" );
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

		_ommBaseImpl.handleIue( text, ret );

		return false;
	}


  int requestTimeout = static_cast<OmmNiProviderImpl&>(_ommBaseImpl).getRequestTimeout();
	if ( requestTimeout > 0 )
	{
		cancelReqTimerEvent();
		_pTimeOut = new TimeOut(_ommBaseImpl, requestTimeout * 1000, ItemWatchList::itemRequestTimeout, this, true); 
		_timeOutExpired = false;
	}

	return true;
}

bool NiProviderSingleItem::submit( RsslCloseMsg* pRsslCloseMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	RsslReactorChannel* pReactorChannel = _ommBaseImpl.getRsslReactorChannel();

	submitMsgOpts.pRsslMsg = (RsslMsg*) pRsslCloseMsg;

	submitMsgOpts.majorVersion = pReactorChannel->majorVersion;
	submitMsgOpts.minorVersion = pReactorChannel->minorVersion;
	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg(_ommBaseImpl.getRsslReactor(), pReactorChannel,
		&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in NiProviderSingleItem::submit( RsslCloseMsg* )" );
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

		_ommBaseImpl.handleIue( text, ret );

		return false;
	}

	return true;
}

void NiProviderSingleItem::scheduleItemClosedStatus( const ReqMsgEncoder& reqMsgEncoder, const EmaString& text )
{
	if ( _closedInfo ) return;

	_closedInfo = new ItemStatusInfo( this, reqMsgEncoder, text );

	new TimeOut( _ommBaseImpl, 1000, ItemCallbackClient::sendItemStatus, _closedInfo, true );
}

void NiProviderSingleItem::scheduleItemClosedRecoverableStatus(const EmaString& text)
{
	if ( _closedInfo ) return;

	cancelReqTimerEvent();

	_closedInfo = new ItemStatusInfo( this, text );

	new TimeOut( _ommBaseImpl, 1000, ItemCallbackClient::sendItemStatus, _closedInfo, true );
}

IProviderSingleItem* IProviderSingleItem::create( OmmServerBaseImpl& ommServerBaseImpl,  OmmProviderClient& ommProvClient, void* closure, Item* pParentItem )
{
	try
	{
		return new IProviderSingleItem(ommServerBaseImpl, ommProvClient, &static_cast<OmmIProviderImpl&>(ommServerBaseImpl).getItemWatchList(), closure, pParentItem);
	}
	catch (std::bad_alloc&)
	{
		ommServerBaseImpl.handleMee("Failed to create IProviderSingleItem");
	}

	return NULL;
}

IProviderSingleItem::IProviderSingleItem( OmmServerBaseImpl& ommServerBaseImpl, OmmProviderClient& ommProvClient, ItemWatchList* pItemWatchList, void* closure, Item* ) :
	ProviderItem(ommServerBaseImpl, ommProvClient, pItemWatchList, closure),
	_ommServerBaseImpl(ommServerBaseImpl)
{
	setProvider( static_cast<OmmIProviderImpl&>(_ommServerBaseImpl).getProvider() );
}

IProviderSingleItem::~IProviderSingleItem()
{
	_ommServerBaseImpl.getItemCallbackClient().removeFromMap(this);

	_ommServerBaseImpl.getItemCallbackClient().removeFromList(this);
}

Item::ItemType IProviderSingleItem::getType() const
{
	return Item::IProviderSingleItemEnum;
}

OmmServerBaseImpl& IProviderSingleItem::getImpl()
{
	return _ommServerBaseImpl;
}

Int32 IProviderSingleItem::getNextStreamId(int numOfItem)
{
	return -_ommServerBaseImpl.getItemCallbackClient().getNextStreamId(numOfItem);
}

bool IProviderSingleItem::open(const ReqMsg& reqMsg)
{
	EmaString serviceName;
	UInt64 serviceId = 0;

	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder());

	if (reqMsgEncoder.hasServiceName())
	{
		if (!static_cast<OmmIProviderImpl&>(_ommServerBaseImpl).getServiceId(reqMsgEncoder.getServiceName(), serviceId))
		{
			EmaString temp("Service name of '");
			temp.append(reqMsgEncoder.getServiceName()).
				append("' is not found.");
			scheduleItemClosedStatus(reqMsgEncoder, temp);

			return true;
		}

		serviceName = reqMsgEncoder.getServiceName();

		reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId = (UInt16)serviceId;
		reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags |= RSSL_MKF_HAS_SERVICE_ID;

		_specifiedServiceInReq = true;
	}
	else if (reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID)
	{
		if (!static_cast<OmmIProviderImpl&>(_ommServerBaseImpl).getServiceName(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId, serviceName))
		{
			EmaString temp("Service id of '");
			temp.append(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId).
				append("' is not found.");
			scheduleItemClosedStatus(reqMsgEncoder, temp);

			return true;
		}

		serviceId = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId;


		_specifiedServiceInReq = true;
	}

	if (reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME)
	{
		_msgKey.name.data = (char*)malloc(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length);

		if (!_msgKey.name.data)
		{
			const char* text = "Failed to allocate memory in IProviderSingleItem::open( const ReqMsg& ).";
			throwMeeException(text);
		}

		_msgKey.name.length = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length;
	}

	if (reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_ATTRIB)
	{
		_msgKey.encAttrib.data = (char*)malloc(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.encAttrib.length + 1);
		if (!_msgKey.encAttrib.data)
		{
			throwMeeException("Failed to allocate memory for encoded attrib in IProviderSingleItem::open( const ReqMsg& ).");
		}
		_msgKey.encAttrib.length = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.encAttrib.length + 1;
	}

	_isPrivateStream = reqMsgEncoder.getPrivateStream();

	if ( _specifiedServiceInReq )
	{
		_serviceName = serviceName;
		_serviceId = serviceId;
	}

	rsslCopyMsgKey(&_msgKey, &reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey);
	_pItemWatchList->addItem(this);

	return submit(reqMsgEncoder.getRsslRequestMsg());
}

bool IProviderSingleItem::close()
{
	RsslCloseMsg rsslCloseMsg;
	rsslClearCloseMsg(&rsslCloseMsg);

	rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslCloseMsg.msgBase.domainType = _domainType;

	bool retCode = submit(&rsslCloseMsg);

	remove();

	return retCode;
}

bool IProviderSingleItem::submit(const GenericMsg& genMsg)
{
	return submit(static_cast<const GenericMsgEncoder&>(genMsg.getEncoder()).getRsslGenericMsg());
}

void IProviderSingleItem::remove()
{
	delete this;
}

bool IProviderSingleItem::submit(RsslRequestMsg* pRsslRequestMsg)
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

	if (!(pRsslRequestMsg->flags & RSSL_RQMF_HAS_QOS))
	{
		pRsslRequestMsg->qos.timeliness = RSSL_QOS_TIME_REALTIME;
		pRsslRequestMsg->qos.rate = RSSL_QOS_RATE_TICK_BY_TICK;
		pRsslRequestMsg->worstQos.rate = RSSL_QOS_RATE_TIME_CONFLATED;
		pRsslRequestMsg->worstQos.timeliness = RSSL_QOS_TIME_DELAYED_UNKNOWN;
		pRsslRequestMsg->worstQos.rateInfo = 65535;
		pRsslRequestMsg->flags |= (RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS);
	}

	submitMsgOpts.pRsslMsg = (RsslMsg*)pRsslRequestMsg;

	if (!_streamId)
	{
		if (!submitMsgOpts.pRsslMsg->msgBase.streamId)
		{
			submitMsgOpts.pRsslMsg->msgBase.streamId = getNextStreamId();
		}

		_streamId = submitMsgOpts.pRsslMsg->msgBase.streamId;
	}
	else
		submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	if (!_domainType)
		_domainType = submitMsgOpts.pRsslMsg->msgBase.domainType;
	else
		submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);
	RsslRet ret;

	if ( !getClientSession() )
	{
		ClientSessionPtr pClientSession = _ommServerBaseImpl.getServerChannelHandler().getClientSessionForDictReq();

		if ( pClientSession )
		{
			setClientSession(pClientSession);
		}
		else
		{
			EmaString temp( "No active channel to send message." );
			_ommServerBaseImpl.handleIue(temp, OmmInvalidUsageException::NoActiveChannelEnum);

		return false;
		}
	}

	submitMsgOpts.majorVersion = getClientSession()->getChannel()->majorVersion;
	submitMsgOpts.minorVersion = getClientSession()->getChannel()->minorVersion;

	if ((ret = rsslReactorSubmitMsg(_ommServerBaseImpl.getRsslReactor(), getClientSession()->getChannel(),
		&submitMsgOpts, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _ommServerBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error: rsslReactorSubmitMsg() failed in IProviderSingleItem::submit( RsslRequestMsg* )");
			temp.append(CR)
				.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);
			_ommServerBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
		}

		EmaString text("Failed to open or modify item request. Reason: ");
		text.append(rsslRetCodeToString(ret))
			.append(". Error text: ")
			.append(rsslErrorInfo.rsslError.text);

		_ommServerBaseImpl.handleIue(text, ret);

		return false;
	}

    int requestTimeout = static_cast<OmmIProviderImpl&>(_ommServerBaseImpl).getRequestTimeout();

	if ( requestTimeout > 0 )
	{
		cancelReqTimerEvent();
		_pTimeOut = new TimeOut(_ommServerBaseImpl, requestTimeout * 1000, ItemWatchList::itemRequestTimeout, this, true);
		_timeOutExpired = false;
	}	

	return true;
}

bool IProviderSingleItem::submit(RsslCloseMsg* pRsslCloseMsg)
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions(&submitMsgOpts);

	submitMsgOpts.pRsslMsg = (RsslMsg*)pRsslCloseMsg;

	submitMsgOpts.majorVersion = getClientSession()->getChannel()->majorVersion;
	submitMsgOpts.minorVersion = getClientSession()->getChannel()->minorVersion;
	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo(&rsslErrorInfo);
	RsslRet ret;
	if ((ret = rsslReactorSubmitMsg(_ommServerBaseImpl.getRsslReactor(), getClientSession()->getChannel(),
		&submitMsgOpts, &rsslErrorInfo)) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _ommServerBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
		{
			EmaString temp("Internal error. rsslReactorSubmitMsg() failed in IProviderSingleItem::submit( RsslCloseMsg* )");
			temp.append(CR)
				.append("RsslChannel ").append(ptrToStringAsHex(rsslErrorInfo.rsslError.channel)).append(CR)
				.append("Error Id ").append(rsslErrorInfo.rsslError.rsslErrorId).append(CR)
				.append("Internal sysError ").append(rsslErrorInfo.rsslError.sysError).append(CR)
				.append("Error Location ").append(rsslErrorInfo.errorLocation).append(CR)
				.append("Error Text ").append(rsslErrorInfo.rsslError.text);
			_ommServerBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
		}

		EmaString text("Failed to close item request. Reason: ");
		text.append(rsslRetCodeToString(ret))
			.append(". Error text: ")
			.append(rsslErrorInfo.rsslError.text);

		_ommServerBaseImpl.handleIue(text, ret);

		return false;
	}

	return true;
}

bool IProviderSingleItem::submit(RsslGenericMsg* pRsslGenericMsg)
{
	return false;
}

void IProviderSingleItem::scheduleItemClosedStatus(const ReqMsgEncoder& reqMsgEncoder, const EmaString& text)
{
	if (_closedInfo) return;

	_closedInfo = new ItemStatusInfo(this, reqMsgEncoder, text);

	new TimeOut(_ommServerBaseImpl, 1000, ItemCallbackClient::sendItemStatus, _closedInfo, true);
}

void IProviderSingleItem::scheduleItemClosedRecoverableStatus( const EmaString& text )
{
	if ( _closedInfo ) return;

	cancelReqTimerEvent();

	_closedInfo = new ItemStatusInfo(this, text);

	new TimeOut( _ommServerBaseImpl, 1000, ItemCallbackClient::sendItemStatus, _closedInfo, true );
}

void ItemCallbackClient::sendItemStatus( void* pInfo )
{
	if ( !pInfo ) return;

	RsslStatusMsg rsslStatusMsg;
	rsslClearStatusMsg( &rsslStatusMsg );

	ItemStatusInfo* pClosedStatusInfo = static_cast<ItemStatusInfo*>( pInfo );

	rsslStatusMsg.msgBase.streamId = pClosedStatusInfo->getStreamId();

	rsslStatusMsg.msgBase.domainType = (UInt8)pClosedStatusInfo->getDomainType();

	if (pClosedStatusInfo->getItem()->getType() == Item::BatchItemEnum)
	{
		rsslStatusMsg.state.dataState = RSSL_DATA_OK;
		rsslStatusMsg.state.code = RSSL_SC_NONE;
	}
	else
	{
		rsslStatusMsg.state.dataState = pClosedStatusInfo->getRsslState().dataState;
		rsslStatusMsg.state.code = pClosedStatusInfo->getRsslState().code;
	}
	rsslStatusMsg.state.streamState = pClosedStatusInfo->getRsslState().streamState;
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

	// Since this is a generated message, set the channel to NULL
	item->setEventChannel(NULL);
	item->onAllMsg( statusMsg );
	item->onStatusMsg( statusMsg );

	// Do not remove the item if the stream state is OPEN
	if(rsslStatusMsg.state.streamState != RSSL_STREAM_OPEN)
	item->remove();
}

void SingleItem::scheduleItemClosedStatus( const ReqMsgEncoder& reqMsgEncoder, const EmaString& text )
{
	if (_closedInfo == NULL)
	{
		_closedInfo = new ItemStatusInfo(this, reqMsgEncoder, text);
	}

	new TimeOut( _ommBaseImpl, 1000, ItemCallbackClient::sendItemStatus, _closedInfo, true );
}

void SingleItem::scheduleItemSuspectStatus(const ReqMsgEncoder& reqMsgEncoder, const EmaString& text)
{
	if (_closedInfo == NULL)
	{
		_closedInfo = new ItemStatusInfo(this, reqMsgEncoder, text);
}

	((RsslState&)_closedInfo->getRsslState()).streamState = RSSL_STREAM_OPEN;

	new TimeOut(_ommBaseImpl, 1000, ItemCallbackClient::sendItemStatus, _closedInfo, true);
}

ItemStatusInfo::ItemStatusInfo( Item* pItem, const ReqMsgEncoder& reqMsgEncoder, const EmaString& text ) :
	_msgKey(),
	_statusText( text ),
	_serviceName(),
	_domainType( reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType ),
	_streamId( reqMsgEncoder.getRsslRequestMsg()->msgBase.streamId ),
	_pItem( pItem ),
	_privateStream( false )
{
	rsslClearMsgKey( &_msgKey );
	rsslClearState(&_rsslState);

	_rsslState.dataState = RSSL_DATA_SUSPECT;
	_rsslState.streamState = RSSL_STREAM_CLOSED;
	_rsslState.code = RSSL_SC_SOURCE_UNKNOWN;

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

	if (reqMsgEncoder.hasServiceListName())
	{
		_serviceName = reqMsgEncoder.getServiceListName();
	}
	else if (reqMsgEncoder.hasServiceName())
	{
		_serviceName = reqMsgEncoder.getServiceName();
	}

	_privateStream = reqMsgEncoder.getPrivateStream();
}

ItemStatusInfo::ItemStatusInfo( Item* pItem, const TunnelStreamRequest& tunnelStreamRequest, const EmaString& text ) :
	_msgKey(),
	_statusText( text ),
	_serviceName(),
	_domainType( tunnelStreamRequest.getDomainType() ),
	_streamId( 0 ),
	_pItem( pItem ),
	_privateStream( true )
{
	rsslClearMsgKey( &_msgKey );
	rsslClearState(&_rsslState);

	_rsslState.dataState = RSSL_DATA_SUSPECT;
	_rsslState.streamState = RSSL_STREAM_CLOSED;
	_rsslState.code = RSSL_SC_SOURCE_UNKNOWN;

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

ItemStatusInfo::ItemStatusInfo(ProviderItem* pProviderItem, const EmaString& text) :
	_msgKey(),
	_statusText(text),
	_serviceName(),
	_domainType(static_cast<UInt16>(pProviderItem->getDomainType())),
	_streamId(pProviderItem->getStreamId()),
	_pItem(pProviderItem),
	_privateStream(false)
{
	rsslClearMsgKey( &_msgKey );
	rsslClearState(&_rsslState);

	_rsslState.dataState = RSSL_DATA_SUSPECT;
	_rsslState.streamState = RSSL_STREAM_CLOSED_RECOVER;
	_rsslState.code = RSSL_SC_NONE;

	_msgKey.flags = pProviderItem->getRsslMsgKey().flags;

	_msgKey.nameType = pProviderItem->getRsslMsgKey().nameType;

	if ( pProviderItem->getRsslMsgKey().flags & RSSL_MKF_HAS_NAME )
	{
		_msgKey.name.data = pProviderItem->getRsslMsgKey().name.data;
		_msgKey.name.length = pProviderItem->getRsslMsgKey().name.length;
	}

	_msgKey.attribContainerType = pProviderItem->getRsslMsgKey().attribContainerType;

	if ( pProviderItem->getRsslMsgKey().flags & RSSL_MKF_HAS_ATTRIB )
	{
		_msgKey.encAttrib.data = pProviderItem->getRsslMsgKey().encAttrib.data;
		_msgKey.encAttrib.length = pProviderItem->getRsslMsgKey().encAttrib.length;
	}

	_msgKey.filter = pProviderItem->getRsslMsgKey().filter;
	_msgKey.identifier = pProviderItem->getRsslMsgKey().identifier;
	_msgKey.serviceId = pProviderItem->getRsslMsgKey().serviceId;

	_serviceName = pProviderItem->getServiceName();

	_privateStream = pProviderItem->isPrivateStream();
}

ItemStatusInfo::~ItemStatusInfo()
{
	if ( _msgKey.name.data )
		free( _msgKey.name.data );

	if ( _msgKey.encAttrib.data )
		free( _msgKey.encAttrib.data );
}

BatchItem* BatchItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure )
{
	try {
		return new BatchItem( ommBaseImpl, ommConsClient, closure );
	}
	catch( std::bad_alloc& )
	{
		ommBaseImpl.handleMee( "Failed to create BatchItem." );
	}

	return NULL;
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
	if (_currentItemList)
	{
		_currentItemList->removeItem(this);
	}
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

	_ommBaseImpl.handleIue( temp, RSSL_RET_INVALID_ARGUMENT);

	return false;
}

bool BatchItem::close()
{
	EmaString temp( "Invalid attempt to close batch stream. " );
	temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp, RSSL_RET_INVALID_ARGUMENT);

	return false;
}

bool BatchItem::submit( const PostMsg& )
{
	EmaString temp( "Invalid attempt to submit PostMsg on batch stream. " );
	temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );

	return false;
}

bool BatchItem::submit( const GenericMsg& )
{
	EmaString temp( "Invalid attempt to submit GenericMsg on batch stream. " );
	temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );

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
	// This is why _singleItemList is 1 indexed.  
	return (streamId == _streamId) ? this : _singleItemList[ streamId - _streamId ];
}

void BatchItem::decreaseItemCount()
{	
	if ( --_itemCount == 0 )
		delete this;
}

TunnelItem* TunnelItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure )
{
	try {
		return new TunnelItem( ommBaseImpl, ommConsClient, closure );
	}
	catch( std::bad_alloc& )
	{
		ommBaseImpl.handleMee( "Failed to create TunnelItem." );
	}

	return NULL;
}

TunnelItem::TunnelItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure ) :
	SingleItem( ommBaseImpl, ommConsClient, closure, 0 ),
	_pRsslTunnelStream( 0 ),
	nextSubItemStreamId( _startingSubItemStreamId ),
	_subItems( 32 )
{
}

TunnelItem::~TunnelItem()
{
	if (_currentItemList)
	{
		_currentItemList->removeItem(this);
	}

	for ( UInt32 i = 0; i < _subItems.size(); ++i )
		if ( _subItems[ i ] )
			removeSubItem (_subItems[ i ]->getStreamId() );

    StreamId* streamId = returnedSubItemStreamIds.pop_back();

	while (streamId)
	{
		delete streamId;
		streamId = returnedSubItemStreamIds.pop_back();
	}
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

			_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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

				_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );
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
		_serviceName = tunnelStreamRequest.getServiceName();
		if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
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
			_pDirectory = pDirectory;

		}
		else
		{
			// Set up the cached request message to do the following matching.
			setUpReqMsg(tunnelStreamRequest);
			if (_ommBaseImpl.getConsumerRoutingSession()->matchRequestToSessionChannel(*this) == false)
			{
				EmaString temp("Service name of '");
				temp.append(tunnelStreamRequest.getServiceName()).
					append("' is not found.");
				scheduleItemClosedStatus(tunnelStreamRequest, temp);

				return true;
			}
		}
	}
	else if ( tunnelStreamRequest.hasServiceId() )
	{
		setServiceId(tunnelStreamRequest.getServiceId());
		if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
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
			_pDirectory = pDirectory;

	}
		else
		{
			setUpReqMsg(tunnelStreamRequest);
			if (_ommBaseImpl.getConsumerRoutingSession()->matchRequestToSessionChannel(*this) == false)
			{
				EmaString temp("Service name of '");
				temp.append(tunnelStreamRequest.getServiceName()).
					append("' is not found.");
				scheduleItemClosedStatus(tunnelStreamRequest, temp);

				return true;
			}

			// matchRequestToSessionChannel returning true means that there is a match for the service, so we can just get it and set the name in the service list here.
			ConsumerRoutingService** pRoutingServicePtr = _ommBaseImpl.getConsumerRoutingSession()->serviceById.find((UInt16)_serviceId);

			_serviceName = (*pRoutingServicePtr)->getName();
		}
	}

	return submit( tunnelStreamRequest );
}

void TunnelItem::scheduleItemClosedStatus( const TunnelStreamRequest& tunnelStreamRequest, const EmaString& text )
{
	if ( _closedInfo ) return;

	_closedInfo = new ItemStatusInfo( this, tunnelStreamRequest, text );

	new TimeOut( _ommBaseImpl, 1000, ItemCallbackClient::sendItemStatus, _closedInfo, true );
}

void TunnelItem::rsslTunnelStream( RsslTunnelStream* pRsslTunnelStream )
{
	_pRsslTunnelStream = pRsslTunnelStream;
}

bool TunnelItem::open( const ReqMsg& )
{
	EmaString temp( "Invalid attempt to open tunnel stream using ReqMsg." );
	temp.append( "OmmConsumer name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );

	return false;
}

bool TunnelItem::modify( const ReqMsg& )
{
	EmaString temp( "Invalid attempt to reissue tunnel stream using ReqMsg." );
	temp.append( "OmmConsumer name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );

	return false;
}

bool TunnelItem::submit( const PostMsg& )
{
	EmaString temp( "Invalid attempt to submit PostMsg on tunnel stream." );
	temp.append( "OmmConsumer name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );

	return false;
}

bool TunnelItem::submit( const GenericMsg& )
{
	EmaString temp( "Invalid attempt to submit GenericMsg on tunnel stream." );
	temp.append( "OmmConsumer name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );

	return false;
}

bool TunnelItem::submit( const TunnelStreamRequest& tunnelStreamRequest )
{
	_domainType = (UInt8)tunnelStreamRequest.getDomainType();
	_streamId = getNextStreamId();

	RsslReactorChannel* pReactorChannel = NULL;

	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
	{
		pReactorChannel = _ommBaseImpl.getRsslReactorChannel();
	}
	else
	{
		pReactorChannel = sessionChannel->pReactorChannel;
	}

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
		RsslRet retCode;
		if ( RSSL_RET_SUCCESS != ( retCode = rsslSetDecodeIteratorRWFVersion( &dIter, pReactorChannel->majorVersion, pReactorChannel->minorVersion ) ) )
		{
			free( rsslBuffer.data );
			_ommBaseImpl.handleIue( "Internal Error. Failed to set decode iterator version in TunnelItem::submit( const TunnelStreamRequest& )", retCode );
			return false;
		}

		if ( RSSL_RET_SUCCESS != ( retCode = rsslSetDecodeIteratorBuffer( &dIter, tunnelStreamRequest._pImpl->getRsslBuffer() ) ) )
		{
			free( rsslBuffer.data );
			_ommBaseImpl.handleIue( "Internal Error. Failed to set decode iterator buffer in TunnelItem::submit( const TunnelStreamRequest& )", retCode );
			return false;
		}

		RsslMsg* pRsslMsg = tunnelStreamRequest._pImpl->getRsslMsg();

		retCode = rsslDecodeRDMLoginMsg( &dIter, pRsslMsg, &rsslRdmLoginMsg, &rsslBuffer, &rsslErrorInfo );

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
			_ommBaseImpl.handleIue( "Internal Error. Failed to decode login request in TunnelItem::submit( const TunnelStreamRequest& )", retCode );
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
	if ( ( ret = rsslReactorOpenTunnelStream( pReactorChannel, &tsOpenOptions, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
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

		_ommBaseImpl.handleIue( text, ret );

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

		_ommBaseImpl.handleIue( text, ret );

		return false;
	}

	remove();

	return true;
}

void TunnelItem::setUpReqMsg(const TunnelStreamRequest& tsRequest)
{
	// Just need to set the request's domain type on the request message, since QoS is handled with the quality of service options.
	// EMA will not recover any tunnel stream automatically, so we do not need to cache any more information.
	// In addition, the _requestMsg should already have been cleared at TunnelItem creation.
	_requestMsg.msgBase.domainType = tsRequest._domainType;
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

	RsslRet retCode;
	if ( (retCode = rsslSetEncodeIteratorRWFVersion( &eIter, _pRsslTunnelStream->pReactorChannel->majorVersion, _pRsslTunnelStream->pReactorChannel->minorVersion )) != RSSL_RET_SUCCESS )
	{
		_ommBaseImpl.handleIue( "Internal Error. Failed to set encode iterator version in TunnelItem::submitSubItemMsg( RsslMsg* ).", retCode );
		return false;
	}

	if ( (retCode = rsslSetEncodeIteratorBuffer( &eIter, pRsslBuffer )) != RSSL_RET_SUCCESS )
	{
		_ommBaseImpl.handleIue( "Internal Error. Failed to set encode iterator buffer in TunnelItem::submitSubItemMsg( RsslMsg* ).", retCode );
		return false;
	}

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

		if ( (retCode = rsslSetEncodeIteratorRWFVersion( &eIter, _pRsslTunnelStream->pReactorChannel->majorVersion, _pRsslTunnelStream->pReactorChannel->minorVersion )) != RSSL_RET_SUCCESS )
		{
			_ommBaseImpl.handleIue( "Internal Error. Failed to set encode iterator version in TunnelItem::submitSubItemMsg( RsslMsg* ).", retCode );
			return false;
		}

		if ( (retCode = rsslSetEncodeIteratorBuffer( &eIter, pRsslBuffer )) != RSSL_RET_SUCCESS )
		{
			_ommBaseImpl.handleIue( "Internal Error. Failed to set encode iterator buffer in TunnelItem::submitSubItemMsg( RsslMsg* ).", retCode );
			return false;
		}
	}

	if ( retCode != RSSL_RET_SUCCESS )
	{
		rsslTunnelStreamReleaseBuffer( pRsslBuffer, &rsslErrorInfo );

		_ommBaseImpl.handleIue( "Internal Error. Failed to encode message in TunnelItem::submitSubItemMsg( RsslMsg* ).", retCode );
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

		_ommBaseImpl.handleIue( "Internal Error. Failed to submit message in TunnelItem::submitSubItemMsg( RsslMsg* ).", retCode );
		return false;
	}

	return true;
}

SubItem* SubItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, Item* parent )
{
	try {
		return new SubItem( ommBaseImpl, ommConsClient, closure, parent );
	}
	catch( std::bad_alloc& )
	{
		ommBaseImpl.handleMee( "Failed to create SubItem" );
	}

	return NULL;
}

SubItem::SubItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, Item* parent ) :
	ConsumerItem( ommBaseImpl, ommConsClient, closure, parent )
{
}

SubItem::~SubItem()
{
	reinterpret_cast<TunnelItem*>( _event.getParentHandle() )->removeSubItem( _streamId );

	_ommBaseImpl.getItemCallbackClient().removeFromList( this );

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
	if ( _closedInfo ) return;

	_closedInfo = new ItemStatusInfo( this, reqMsgEncoder, text );

	new TimeOut( _ommBaseImpl, 1000, ItemCallbackClient::sendItemStatus, _closedInfo, true );
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
				_ommBaseImpl.getErrorClientHandler().onInvalidUsage( temp, OmmInvalidUsageException::InvalidArgumentEnum );
				return false;
			}
			else
			{
				throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
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

ItemList* ItemList::create(OmmCommonImpl& ommCommonImpl)
{
	try {
		return new ItemList(ommCommonImpl);
	}
	catch( std::bad_alloc& )
	{
		ommCommonImpl.handleMee("Failed to create ItemList");
	}

	return NULL;
}

void ItemList::destroy( ItemList*& pItemList )
{
	if ( pItemList )
	{
		delete pItemList;
		pItemList = 0;
	}
}

ItemList::ItemList( OmmCommonImpl& ommCommonImpl ) :
	_ommCommonImpl( ommCommonImpl )
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
	// Remove the item from the previous item list, if set.
	if (pItem->getItemList())
	{
		pItem->getItemList()->removeItem(pItem);
	}

	_list.push_back( pItem );

	if ( OmmLoggerClient::VerboseEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
	{
		EmaString temp( "Added Item " );
		temp.append( ptrToStringAsHex( pItem ) ).append(" of StreamId ").append(pItem->getStreamId()).append( " to ItemList" ).append( CR )
			.append( "Instance name " ).append( _ommCommonImpl.getInstanceName() );
		_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
	}

	pItem->setItemList(this);

	return _list.size();
}


EmaList<Item*>& ItemList::getList()
{
	return _list;
}

void ItemList::removeItem( Item* pItem )
{
	_list.remove( pItem );
	pItem->setItemList(NULL);

	if ( OmmLoggerClient::VerboseEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
	{
		EmaString temp( "Removed Item " );
		temp.append( ptrToStringAsHex( pItem ) ).append(" of StreamId ").append(pItem->getStreamId()).append( " from ItemList" ).append( CR )
			.append("Instance name ").append( _ommCommonImpl.getInstanceName() );
		_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
	}
}

ItemCallbackClient::ItemCallbackClient( OmmBaseImpl& ommBaseImpl ) :
	_refreshMsg(),
	_updateMsg(),
	_statusMsg(),
	_genericMsg(),
	_ackMsg(),
	_ommCommonImpl( ommBaseImpl ),
	_itemMap( ommBaseImpl.getActiveConfig().itemCountHint ),
	_streamIdMap(ommBaseImpl.getActiveConfig().itemCountHint),
	_nextStreamIdWrapAround(false),
	_streamIdAccessMutex()
{
    _itemList = ItemList::create( ommBaseImpl );

	if ( ommBaseImpl.getImplType() == OmmCommonImpl::ConsumerEnum )
	{
		_nextStreamId = CONSUMER_STARTING_STREAM_ID;
	}
	else
	{
		_nextStreamId = PROVIDER_STARTING_STREAM_ID;
	}

	if ( OmmLoggerClient::VerboseEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
	{
		EmaString temp( "Created ItemCallbackClient." );
		temp.append( " Instance name='" ).append( _ommCommonImpl.getInstanceName() ).append( "'." );
		_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
	}
}

ItemCallbackClient::ItemCallbackClient( OmmServerBaseImpl& ommServerBaseImpl ) :
	_refreshMsg(),
	_updateMsg(),
	_statusMsg(),
	_genericMsg(),
	_ackMsg(),
	_ommCommonImpl( ommServerBaseImpl ),
	_itemMap( ommServerBaseImpl.getActiveConfig().itemCountHint ),
	_streamIdMap( ommServerBaseImpl.getActiveConfig().itemCountHint ),
	_nextStreamIdWrapAround( false ),
	_streamIdAccessMutex()
{
	_itemList = ItemList::create( ommServerBaseImpl );

	_nextStreamId = PROVIDER_STARTING_STREAM_ID;

	if (OmmLoggerClient::VerboseEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity)
	{
		EmaString temp("Created ItemCallbackClient.");
		temp.append(" Instance name='").append(_ommCommonImpl.getInstanceName()).append("'.");
		_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
	}
}

ItemCallbackClient::~ItemCallbackClient()
{
	ItemList::destroy( _itemList );

	if ( OmmLoggerClient::VerboseEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
	{
		EmaString temp( "Destroyed ItemCallbackClient [" );
		temp.append( _ommCommonImpl.getInstanceName()).append("]");
		_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
	}
}

ItemCallbackClient* ItemCallbackClient::create( OmmBaseImpl& ommBaseImpl )
{
	try {
		return new ItemCallbackClient( ommBaseImpl );
	}
	catch ( std::bad_alloc& )
	{
		ommBaseImpl.handleMee( "Failed to create ItemCallbackClient" );
	}

	return NULL;
}

ItemCallbackClient* ItemCallbackClient::create(OmmServerBaseImpl& ommServerBaseImpl)
{
	try {
		return new ItemCallbackClient(ommServerBaseImpl);
	}
	catch (std::bad_alloc&)
	{
		ommServerBaseImpl.handleMee( "Failed to create ItemCallbackClient" );
	}

	return NULL;
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
		if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream status event without the pRsslTunnelStream in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamStatusEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamStatusEvent->pReactorChannel->pRsslChannel ) );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if ( !pRsslTunnelStream->userSpecPtr )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream status event without the userSpecPtr in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamStatusEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamStatusEvent->pReactorChannel->pRsslChannel ) );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
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
	if (channel->getParentChannel())
	{
		channel = channel->getParentChannel();
	}

	StaticDecoder::setRsslData(&_statusMsg, (RsslMsg*)&rsslStatusMsg,
		pTunnelStreamStatusEvent->pReactorChannel->majorVersion,
		pTunnelStreamStatusEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	static_cast<TunnelItem*>( pRsslTunnelStream->userSpecPtr )->rsslTunnelStream( pRsslTunnelStream );

	Item* item = (Item*)( pRsslTunnelStream->userSpecPtr );

	_statusMsg.getDecoder().setServiceName( item->getDirectory()->getName().c_str(), item->getDirectory()->getName().length() );

	_ommCommonImpl.msgDispatched();
	item->setEventChannel((void*)pRsslTunnelStream->pReactorChannel);
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
		if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream message event without the pRsslTunnelStream in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamMsgEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if ( !pRsslTunnelStream->userSpecPtr )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream message event without the userSpecPtr in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamMsgEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) ).append( CR )
				.append( "Error Id " ).append( pTunnelStreamMsgEvent->pErrorInfo->rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( pTunnelStreamMsgEvent->pErrorInfo->rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( pTunnelStreamMsgEvent->pErrorInfo->errorLocation ).append( CR )
				.append( "Error Text " ).append( pTunnelStreamMsgEvent->pErrorInfo->rsslError.rsslErrorId ? pTunnelStreamMsgEvent->pErrorInfo->rsslError.text : "" );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if ( pTunnelStreamMsgEvent->containerType != RSSL_DT_MSG )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream message event containing an unsupported data type of " );
			temp += DataType((DataType::DataTypeEnum)pTunnelStreamMsgEvent->containerType).toString();

			temp.append( CR )
				.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) ).append( CR )
				.append( "Tunnel Stream Handle " ).append( (UInt64)pRsslTunnelStream->userSpecPtr ).append( CR )
				.append( "Tunnel Stream name " ).append( pRsslTunnelStream->name ).append( CR )
				.append( "Tunnel Stream serviceId " ).append( pRsslTunnelStream->serviceId ).append( CR )
				.append( "Tunnel Stream streamId " ).append( pRsslTunnelStream->streamId );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
		}

		return RSSL_RC_CRET_SUCCESS;
	}
	else if ( !pTunnelStreamMsgEvent->pRsslMsg )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream message event containing no sub stream message." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) ).append( CR )
				.append( "Tunnel Stream Handle " ).append( (UInt64)pRsslTunnelStream->userSpecPtr ).append( CR )
				.append( "Tunnel Stream name " ).append( pRsslTunnelStream->name ).append( CR )
				.append( "Tunnel Stream serviceId " ).append( pRsslTunnelStream->serviceId ).append( CR )
				.append( "Tunnel Stream streamId " ).append( pRsslTunnelStream->streamId );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	Item* item = static_cast<TunnelItem*>( pRsslTunnelStream->userSpecPtr )->getSubItem( pTunnelStreamMsgEvent->pRsslMsg->msgBase.streamId );
	
	if ( ! item )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream message event containing sub stream message with unknown streamId " );
			temp.append( pTunnelStreamMsgEvent->pRsslMsg->msgBase.streamId ).append( ". Message is dropped." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) ).append( CR )
				.append( "Tunnel Stream Handle " ).append( (UInt64)pRsslTunnelStream->userSpecPtr ).append( CR )
				.append( "Tunnel Stream name " ).append( pRsslTunnelStream->name ).append( CR )
				.append( "Tunnel Stream serviceId " ).append( pRsslTunnelStream->serviceId ).append( CR )
				.append( "Tunnel Stream streamId " ).append( pRsslTunnelStream->streamId );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	switch ( pTunnelStreamMsgEvent->pRsslMsg->msgBase.msgClass )
	{
	default :
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
			{
				EmaString temp( "Received a tunnel stream message event containing an unsupported message type of " );
				temp += DataType( msgDataType[ pTunnelStreamMsgEvent->pRsslMsg->msgBase.msgClass ] ).toString();

				temp.append( CR )
					.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
					.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamMsgEvent->pReactorChannel->pRsslChannel ) );
				_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
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
		if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream queue message event without the pRsslTunnelStream in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamQueueMsgEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamQueueMsgEvent->base.pReactorChannel->pRsslChannel ) );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if ( !pRsslTunnelStream->userSpecPtr )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
		{
			EmaString temp( "Received a tunnel stream queue message event without the userSpecPtr in ItemCallbackClient::processCallback( RsslTunnelStream* , RsslTunnelStreamQueueMsgEvent* )." );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pTunnelStreamQueueMsgEvent->base.pReactorChannel->pRsslChannel ) ).append( CR )
				.append( "Error Id " ).append( pTunnelStreamQueueMsgEvent->base.pErrorInfo->rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( pTunnelStreamQueueMsgEvent->base.pErrorInfo->rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( pTunnelStreamQueueMsgEvent->base.pErrorInfo->errorLocation ).append( CR )
				.append( "Error Text " ).append( pTunnelStreamQueueMsgEvent->base.pErrorInfo->rsslError.rsslErrorId ? pTunnelStreamQueueMsgEvent->base.pErrorInfo->rsslError.text : "" );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace());
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processAckMsg( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent, Item* item )
{
	Channel* channel = static_cast<Channel*>(pTunnelStreamMsgEvent->pReactorChannel->userSpecPtr);
	if (channel->getParentChannel())
	{
		channel = channel->getParentChannel();
	}

	StaticDecoder::setRsslData( &_ackMsg, pTunnelStreamMsgEvent->pRsslMsg,
		pTunnelStreamMsgEvent->pReactorChannel->majorVersion,
		pTunnelStreamMsgEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	_ommCommonImpl.msgDispatched();
	item->setEventChannel((void*)pRsslTunnelStream->pReactorChannel);
	item->onAllMsg( _ackMsg );
	item->onAckMsg( _ackMsg );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processGenericMsg( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent, Item* item )
{
	Channel* channel = static_cast<Channel*>(pTunnelStreamMsgEvent->pReactorChannel->userSpecPtr);
	if (channel->getParentChannel())
	{
		channel = channel->getParentChannel();
	}

	StaticDecoder::setRsslData( &_genericMsg, pTunnelStreamMsgEvent->pRsslMsg,
		pTunnelStreamMsgEvent->pReactorChannel->majorVersion,
		pTunnelStreamMsgEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	_ommCommonImpl.msgDispatched();

	item->setEventChannel((void*)pRsslTunnelStream->pReactorChannel);
	item->onAllMsg( _genericMsg );
	item->onGenericMsg( _genericMsg );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processRefreshMsg( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent, Item* item )
{
	Channel* channel = static_cast<Channel*>(pTunnelStreamMsgEvent->pReactorChannel->userSpecPtr);
	if (channel->getParentChannel())
	{
		channel = channel->getParentChannel();
	}

	StaticDecoder::setRsslData( &_refreshMsg, pTunnelStreamMsgEvent->pRsslMsg,
		pTunnelStreamMsgEvent->pReactorChannel->majorVersion,
		pTunnelStreamMsgEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	_ommCommonImpl.msgDispatched();
	item->setEventChannel((void*)pRsslTunnelStream->pReactorChannel);
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
	if (channel->getParentChannel())
	{
		channel = channel->getParentChannel();
	}

	StaticDecoder::setRsslData( &_updateMsg, pTunnelStreamMsgEvent->pRsslMsg,
		pTunnelStreamMsgEvent->pReactorChannel->majorVersion,
		pTunnelStreamMsgEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	_ommCommonImpl.msgDispatched();

	item->setEventChannel((void*)pRsslTunnelStream->pReactorChannel);
	item->onAllMsg( _updateMsg );
	item->onUpdateMsg( _updateMsg );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processStatusMsg( RsslTunnelStream* pRsslTunnelStream, RsslTunnelStreamMsgEvent* pTunnelStreamMsgEvent, Item* item )
{
	Channel* channel = static_cast<Channel*>(pTunnelStreamMsgEvent->pReactorChannel->userSpecPtr);
	if (channel->getParentChannel())
	{
		channel = channel->getParentChannel();
	}

	StaticDecoder::setRsslData( &_statusMsg, pTunnelStreamMsgEvent->pRsslMsg,
		pTunnelStreamMsgEvent->pReactorChannel->majorVersion,
		pTunnelStreamMsgEvent->pReactorChannel->minorVersion,
		channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0);

	_ommCommonImpl.msgDispatched();

	item->setEventChannel((void*)pRsslTunnelStream->pReactorChannel);
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
		if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
		{
			RsslErrorInfo* pError = pEvent->pErrorInfo;

			EmaString temp( "Received an item event without RsslMsg message" );
			temp.append( CR )
				.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pError->rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( pError->rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( pError->rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( pError->errorLocation ).append( CR )
				.append( "Error Text " ).append( pError->rsslError.rsslErrorId ? pError->rsslError.text : "" );
			_ommCommonImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	Channel* channel = static_cast<Channel*>(pRsslReactorChannel->userSpecPtr);
	if (channel->getParentChannel())
	{
		channel = channel->getParentChannel();
	}

	const RsslDataDictionary* pRsslDataDictionary = channel->getDictionary() ? channel->getDictionary()->getRsslDictionary() : 0;

	Item* pItem = 0;

	if (pRsslMsg->msgBase.streamId != EMA_LOGIN_STREAM_ID)
	{
		if (!pEvent->pStreamInfo || !pEvent->pStreamInfo->pUserSpec)
		{
			if (_ommCommonImpl.getImplType() != OmmCommonImpl::ConsumerEnum)
			{
				Item** pItemPointer = _streamIdMap.find(pRsslMsg->msgBase.streamId);

				if (pItemPointer)
				{
					pItem = *pItemPointer;

					ProviderItem* providerItem = reinterpret_cast<ProviderItem*>(pItem);

					providerItem->setEventChannel(pRsslReactorChannel);

					providerItem->cancelReqTimerEvent();

					return processProviderCallback(pRsslReactor, pRsslReactorChannel, pRsslMsg, providerItem, pRsslDataDictionary);
				}
			}
			else
			{
				if (OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity)
				{
					EmaString temp("Received an item event without user specified pointer or stream info");
					temp.append(CR)
						.append("Instance Name ").append(_ommCommonImpl.getInstanceName()).append(CR)
						.append("RsslReactor ").append(ptrToStringAsHex(pRsslReactor)).append(CR)
						.append("RsslReactorChannel ").append(ptrToStringAsHex(pRsslReactorChannel)).append(CR)
						.append("RsslSocket ").append((UInt64)pRsslReactorChannel->socketId);

					_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
				}

				return RSSL_RC_CRET_SUCCESS;
			}
		}
		else
		{
			pItem = (Item*)pEvent->pStreamInfo->pUserSpec;
		}
	}

	switch ( pRsslMsg->msgBase.msgClass )
	{
	case RSSL_MC_ACK :
		if ( pRsslMsg->msgBase.streamId == EMA_LOGIN_STREAM_ID)
		{
			return (static_cast<OmmBaseImpl&>(_ommCommonImpl)).getLoginCallbackClient().processAckMsg(pRsslMsg, pRsslReactorChannel, 0);
		}
		else
			return processAckMsg( pRsslMsg, pRsslReactorChannel, pItem, pRsslDataDictionary );
	case RSSL_MC_GENERIC :
		if (pRsslMsg->msgBase.streamId == EMA_LOGIN_STREAM_ID)
			return (static_cast<OmmBaseImpl&>(_ommCommonImpl)).getLoginCallbackClient().processGenericMsg(pRsslMsg, pRsslReactorChannel, 0);
		else
			return processGenericMsg( pRsslMsg, pRsslReactorChannel, pItem, pRsslDataDictionary );
	case RSSL_MC_REFRESH :
		return processRefreshMsg( pRsslMsg, pRsslReactorChannel, pItem, pRsslDataDictionary );
	case RSSL_MC_STATUS :
		return processStatusMsg( pRsslMsg, pRsslReactorChannel, pItem, pRsslDataDictionary );
	case RSSL_MC_UPDATE :
		return processUpdateMsg( pRsslMsg, pRsslReactorChannel, pItem, pRsslDataDictionary );
	default :
		if ( OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity )
		{
			EmaString temp( "Received an item event with message containing unhandled message class" );
			temp.append( CR )
				.append( "Rssl Message Class " ).append( pRsslMsg->msgBase.msgClass ).append( CR )
				.append( "Instance Name " ).append( _ommCommonImpl.getInstanceName() ).append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslReactorChannel " ).append( ptrToStringAsHex( pRsslReactorChannel ) ).append( CR )
				.append( "RsslSocket" ).append( (UInt64)pRsslReactorChannel->socketId );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}
		break;
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processIProviderMsgCallback( RsslReactor* pReactor, RsslReactorChannel* pRsslReactorChannel, RsslMsgEvent* pMsgEvent, 
	const RsslDataDictionary* pRsslDataDictionary )
{
	RsslMsg* pRsslMsg = pMsgEvent->pRsslMsg;
	ClientSession* clientSession = (ClientSession*)pRsslReactorChannel->userSpecPtr;

	Item** pItemPointer = _streamIdMap.find(pRsslMsg->msgBase.streamId);

	if ( pItemPointer == 0 )
	{
		if (OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity)
		{
			EmaString temp("Received an item event without a matching stream Id ");
			temp.append(pRsslMsg->msgBase.streamId).append(CR)
				.append("Instance Name ").append(_ommCommonImpl.getInstanceName()).append(CR)
				.append("RsslReactor ").append(ptrToStringAsHex(pReactor)).append(CR)
				.append("RsslReactorChannel ").append(ptrToStringAsHex(pRsslReactorChannel)).append(CR)
				.append("RsslSocket ").append((UInt64)pRsslReactorChannel->socketId);

			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	ProviderItem* providerItem = reinterpret_cast<ProviderItem*>(*pItemPointer);

	providerItem->cancelReqTimerEvent();

	return processProviderCallback(pReactor, pRsslReactorChannel, pRsslMsg, providerItem, pRsslDataDictionary);
}

RsslReactorCallbackRet ItemCallbackClient::processProviderCallback(RsslReactor* pReactor, RsslReactorChannel* pRsslReactorChannel, RsslMsg* pRsslMsg, ProviderItem* providerItem,
	const RsslDataDictionary* pRsslDataDictionary)
{
	bool bCopiedMsg;

	int streamId = pRsslMsg->msgBase.streamId;

	pRsslMsg = ItemWatchList::processRsslMsg(pRsslMsg, providerItem, bCopiedMsg);

	if (pRsslMsg == NULL)
	{
		if (OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity)
		{
			EmaString temp("Received response is mismatch with the initlal request for stream Id ");
			temp.append(streamId).append(CR)
				.append("Instance Name ").append(_ommCommonImpl.getInstanceName()).append(CR)
				.append("RsslReactor ").append(ptrToStringAsHex(pReactor)).append(CR)
				.append("RsslReactorChannel ").append(ptrToStringAsHex(pRsslReactorChannel)).append(CR)
				.append("RsslSocket ").append((UInt64)pRsslReactorChannel->socketId);

			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	switch (pRsslMsg->msgBase.msgClass)
	{
	case RSSL_MC_REFRESH:
	{
		processRefreshMsg(pRsslMsg, pRsslReactorChannel, providerItem, pRsslDataDictionary);

		pRsslMsg->refreshMsg.flags = pRsslMsg->refreshMsg.flags & ~RSSL_RFMF_HAS_MSG_KEY;

		if (bCopiedMsg)
		{
			rsslReleaseCopiedMsg(pRsslMsg);
		}

		return RSSL_RC_CRET_SUCCESS;
	}
	case RSSL_MC_STATUS:
	{
		processStatusMsg(pRsslMsg, pRsslReactorChannel, providerItem, pRsslDataDictionary);

		pRsslMsg->statusMsg.flags = pRsslMsg->statusMsg.flags & ~RSSL_STMF_HAS_MSG_KEY;

		if (bCopiedMsg)
		{
			rsslReleaseCopiedMsg(pRsslMsg);
		}

		return RSSL_RC_CRET_SUCCESS;
	}
	default:
		if (OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity)
		{
			EmaString temp("Received an item event with message containing unhandled message class");
			temp.append(CR)
				.append("Rssl Message Class ").append(pRsslMsg->msgBase.msgClass).append(CR)
				.append("Instance Name ").append(_ommCommonImpl.getInstanceName()).append(CR)
				.append("RsslReactor ").append(ptrToStringAsHex(pReactor)).append(CR)
				.append("RsslReactorChannel ").append(ptrToStringAsHex(pRsslReactorChannel)).append(CR)
				.append("RsslSocket").append((UInt64)pRsslReactorChannel->socketId);
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);
		}
		break;
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processRefreshMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, Item* item, const RsslDataDictionary* pRsslDataDictionary )
{
	SingleItem* pSingleItem = NULL;
	NiProviderDictionaryItem* pNIProvDictionaryItem = NULL;
	IProviderDictionaryItem* pIProvDictionaryItem = NULL;

	StaticDecoder::setRsslData( &_refreshMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		pRsslDataDictionary );
	
	switch (item->getType())
	{
		case Item::BatchItemEnum:
			// Fall through to do the single item processing, because we will never get a refresh message with the initial batch request-the watchlist will always generate a status message
			item = static_cast<BatchItem*>(item)->getSingleItem(pRsslMsg->msgBase.streamId);
		case Item::SingleItemEnum:
			pSingleItem = static_cast<SingleItem*>(item);

			// Set the service name here if it's present.
			if (pSingleItem->getDirectory() != NULL)
			{
				if (!pSingleItem->getServiceListName().empty())
				{
					_refreshMsg.getDecoder().setServiceName(pSingleItem->getServiceListName().c_str(), pSingleItem->getServiceListName().length());
				}
				else
				{
					_refreshMsg.getDecoder().setServiceName(pSingleItem->getDirectory()->getName().c_str(), pSingleItem->getDirectory()->getName().length());
				}

				if (pSingleItem->getDirectory()->hasGeneratedServiceId())
				{
					_refreshMsg.getDecoder().setServiceId((UInt16)pSingleItem->getDirectory()->getGeneratedServiceId());
				}
			}
			else
			{
				_refreshMsg.getDecoder().setServiceName(NULL, 0);
			}

			if (_refreshMsg.getState().getStreamState() == OmmState::OpenEnum && _refreshMsg.getState().getDataState() == OmmState::OkEnum)
			{
				if (_ommCommonImpl.getImplType() == OmmCommonImpl::ConsumerEnum)
				{
					OmmBaseImpl& ommBaseImpl = (OmmBaseImpl&)_ommCommonImpl;
					if (pSingleItem->sessionChannelItemClosedList != NULL)
					{

						for (UInt32 i = 0; i < pSingleItem->sessionChannelItemClosedList->size(); i++)
						{
							bool* tmpbool = (*(pSingleItem->sessionChannelItemClosedList))[i];
							memset((void*)((*(pSingleItem->sessionChannelItemClosedList))[i]), 0, (sizeof(bool) * pSingleItem->closedListSize));
						}
					}

					// This should cover the OPEN/OK refresh.  The watchlist should never send an OPEN/SUSPECT refresh, those should always be status messages(below)
					if (pSingleItem->getItemList() == &ommBaseImpl.getConsumerRoutingSession()->pendingRequestList)
					{
						// This item was in pending, but the stream is now active, so move it to the session channel's request list.
						pSingleItem->sessionChannel->routedRequestList.addItem(pSingleItem);
					}
				}
			}
			
			break;
		case Item::NiProviderDictionaryItemEnum:
			pNIProvDictionaryItem = static_cast<NiProviderDictionaryItem*>(item);
			
			_refreshMsg.getDecoder().setServiceName(pNIProvDictionaryItem->getServiceName().c_str(), pNIProvDictionaryItem->getServiceName().length());
			_refreshMsg.getDecoder().setServiceId((UInt16)pNIProvDictionaryItem->getServiceId());
			break;
		case Item::IProviderDictionaryItemEnum:
			pIProvDictionaryItem = static_cast<IProviderDictionaryItem*>(item);

			_refreshMsg.getDecoder().setServiceName(pIProvDictionaryItem->getServiceName().c_str(), pIProvDictionaryItem->getServiceName().length());
			_refreshMsg.getDecoder().setServiceId((UInt16)pIProvDictionaryItem->getServiceId());
			break;
	}

	_ommCommonImpl.msgDispatched();

	item->setEventChannel((void*)pRsslReactorChannel);
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

RsslReactorCallbackRet ItemCallbackClient::processUpdateMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, Item* item, const RsslDataDictionary* pRsslDataDictionary )
{
	SingleItem* pSingleItem = NULL;
	NiProviderDictionaryItem* pNIProvDictionaryItem = NULL;
	IProviderDictionaryItem* pIProvDictionaryItem = NULL;

	StaticDecoder::setRsslData( &_updateMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		pRsslDataDictionary );

	switch (item->getType())
	{
		case Item::BatchItemEnum:
			// Fall through to do the single item processing, because we will never get a refresh message with the initial batch request-the watchlist will always generate a status message
			item = static_cast<BatchItem*>(item)->getSingleItem(pRsslMsg->msgBase.streamId);
		case Item::SingleItemEnum:
			pSingleItem = static_cast<SingleItem*>(item);

			// Set the service name here if it's present.
			if (pSingleItem->getDirectory() != NULL)
			{
				if (!pSingleItem->getServiceListName().empty())
				{
					_updateMsg.getDecoder().setServiceName(pSingleItem->getServiceListName().c_str(), pSingleItem->getServiceListName().length());
				}
				else
				{
					_updateMsg.getDecoder().setServiceName(pSingleItem->getDirectory()->getName().c_str(), pSingleItem->getDirectory()->getName().length());
				}

				if (pSingleItem->getDirectory()->hasGeneratedServiceId())
				{
					_updateMsg.getDecoder().setServiceId((UInt16)pSingleItem->getDirectory()->getGeneratedServiceId());
				}
			}
			else
			{
				_updateMsg.getDecoder().setServiceName(NULL, 0);
			}
			break;
		case Item::NiProviderDictionaryItemEnum:
			pNIProvDictionaryItem = static_cast<NiProviderDictionaryItem*>(item);

			_updateMsg.getDecoder().setServiceName(pNIProvDictionaryItem->getServiceName().c_str(), pNIProvDictionaryItem->getServiceName().length());
			_updateMsg.getDecoder().setServiceId((UInt16)pNIProvDictionaryItem->getServiceId());
			break;
		case Item::IProviderDictionaryItemEnum:
			pIProvDictionaryItem = static_cast<IProviderDictionaryItem*>(item);

			_updateMsg.getDecoder().setServiceName(pIProvDictionaryItem->getServiceName().c_str(), pIProvDictionaryItem->getServiceName().length());
			_updateMsg.getDecoder().setServiceId((UInt16)pIProvDictionaryItem->getServiceId());
			break;
	}

	_ommCommonImpl.msgDispatched();

	item->setEventChannel((void*)pRsslReactorChannel);
	item->onAllMsg( _updateMsg );
	item->onUpdateMsg( _updateMsg );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processStatusMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, Item* item, const RsslDataDictionary* pRsslDataDictionary )
{
	Channel* pChannel = (Channel*)pRsslReactorChannel->userSpecPtr;
	RsslState origState = pRsslMsg->statusMsg.state;
	RsslState updatedState = origState;
	bool isWsb = false;

	RsslStatusMsg* pStatusMsg = (RsslStatusMsg*)pRsslMsg;

	StaticDecoder::setRsslData( &_statusMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		pRsslDataDictionary );

	bool sendClose = false;
	bool submitRequest = false;
	bool rerouteRequest = false;
	bool sendCloseOnOldReactorChannel = false;

	if (pChannel->getParentChannel() != NULL && pChannel->getParentChannel()->getReactorChannelType() == Channel::WARM_STANDBY)
		isWsb = true;

	if (_ommCommonImpl.getImplType() == OmmCommonImpl::ConsumerEnum)
	{
		OmmBaseImpl& ommBaseImpl = (OmmBaseImpl&)_ommCommonImpl;

		if ( item->getType() == Item::BatchItemEnum )
		{
			item = static_cast<BatchItem *>(item)->getSingleItem( pRsslMsg->msgBase.streamId );
			SingleItem* pSingleItem = static_cast<SingleItem*>(item);
	
			if (item->getType() == Item::BatchItemEnum)
			{
				// Set the service name here if it's present for the batch item.  This will not be processed beyond this point.
				if (pSingleItem->getDirectory() != NULL)
				{
					if (!pSingleItem->getServiceListName().empty())
					{
						_statusMsg.getDecoder().setServiceName(pSingleItem->getServiceListName().c_str(), pSingleItem->getServiceListName().length());
					}
					else
					{
						_statusMsg.getDecoder().setServiceName(pSingleItem->getDirectory()->getName().c_str(), pSingleItem->getDirectory()->getName().length());
					}

					if (pSingleItem->getDirectory()->hasGeneratedServiceId())
					{
						_statusMsg.getDecoder().setServiceId((UInt16)pSingleItem->getDirectory()->getGeneratedServiceId());
					}
				}
				else
				{
					_statusMsg.getDecoder().setServiceName(NULL, 0);
				}

				// Move this request to the item callback handler's list if this is request routing.  It will stay there until the OmmConsumer closes, like a standard batch request.
				// This makes sure that it will stay in memory for the rest of the batch requests, and will not be re-requested.
				if (ommBaseImpl.getConsumerRoutingSession())
				{
					ommBaseImpl.getItemCallbackClient().addToList(item);
				}
			}
		}
		
		if (item->getType() == Item::SingleItemEnum)
		{
			SingleItem* pSingleItem = static_cast<SingleItem*>(item);

			// Set the service name here if it's present.
			if (pSingleItem->getDirectory() != NULL)
			{
				if (!pSingleItem->getServiceListName().empty())
				{
					_statusMsg.getDecoder().setServiceName(pSingleItem->getServiceListName().c_str(), pSingleItem->getServiceListName().length());
				}
				else
				{
					_statusMsg.getDecoder().setServiceName(pSingleItem->getDirectory()->getName().c_str(), pSingleItem->getDirectory()->getName().length());
				}

				if (pSingleItem->getDirectory()->hasGeneratedServiceId())
				{
					_statusMsg.getDecoder().setServiceId((UInt16)pSingleItem->getDirectory()->getGeneratedServiceId());
				}
			}
			else
			{
				_statusMsg.getDecoder().setServiceName(NULL, 0);
			}

			// This checks to see if the enhanced routing is turned on.  If so, we may need to immediately re-route the requests.
			if (ommBaseImpl.getConsumerRoutingSession())
			{
				// At this point, if a service or connection/all underlying wsb connections has gone down, we will have already received a directory update.
				// If a connection has gone down, the connection event will happen after the item fanouts, so the rerouting on channel down will happen then.
				ConsumerRoutingService** pService = ommBaseImpl.getConsumerRoutingSession()->serviceByName.find(&pSingleItem->getDirectory()->getName());
				// Re-assign the service ID 
				pStatusMsg->msgBase.msgKey.serviceId = (RsslUInt16)(*pService)->getId();

				// If this is a batch item, let it flow through because the item's about to be closed.
				// This can only be a batch, dictionary, or single item type.

				// Do not do any parsing if this is a private stream.
				if ((pSingleItem->getReqMsg()->flags & RSSL_RQMF_PRIVATE_STREAM) == 0)
				{
					if (pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_STATE)
					{
						if (pRsslMsg->statusMsg.state.streamState != RSSL_STREAM_OPEN)
						{
							if (pRsslMsg->statusMsg.state.streamState == RSSL_STREAM_CLOSED)
							{
								// Set the bool flag on the current servicelistIndex in the sessionChannelItemClosedList's index that corresponds to the current channel for this item.
								if(pSingleItem->sessionChannelItemClosedList != NULL)
									((*(pSingleItem->sessionChannelItemClosedList))[pSingleItem->sessionChannel->sessionIndex])[pSingleItem->currentServiceListIndex] = true;

								if (ommBaseImpl.getConsumerRoutingSession()->matchRequestToSessionChannel(*pSingleItem) == false)
								{
									// This means there aren't any matching services, so all services we're aware of have closed this item.
									// Set the stream state to CLOSED/SUSPECT
									pRsslMsg->statusMsg.state.streamState = RSSL_STREAM_CLOSED;
								}
								else
								{
									pRsslMsg->statusMsg.state.streamState = RSSL_STREAM_OPEN;
									pRsslMsg->statusMsg.state.dataState = RSSL_DATA_SUSPECT;
									submitRequest = true;
								}
							}
							else
							{

								pRsslMsg->statusMsg.state.streamState = RSSL_STREAM_OPEN;
								pRsslMsg->statusMsg.state.dataState = RSSL_DATA_SUSPECT;

								// If the stream state is not CLOSED or CLOSED_RECOVER(meaning that the underlying watchlist has closed it), close the request.
								if (origState.streamState != RSSL_STREAM_CLOSED && origState.streamState != RSSL_STREAM_CLOSED_RECOVER)
									sendClose = true;
								// Re-route and submit this item.  This will move the item out of the current session channel's item list
								submitRequest = true;
								rerouteRequest = true;
							}
						}
						// This covers the case for OPEN/SUSPECT.
						else if (origState.streamState == RSSL_STREAM_OPEN && origState.dataState == RSSL_DATA_SUSPECT)
						{
							// If this is being driven by a case where the directory has gone down, but the channel has not, recover to a different channel.
							// This is checking the RsslChannel because the reactor's channel event will happen after the watchlist fans out all of the OPEN/SUSPECT
							// states.  In the case where the reactor channel is DOWN or DOWN_RECONNECTING, the processing in the ChannelCallbackClient will handle it.
							// If this is warm standby, recovery will happen only when a service goes down for the full WSB group/the full WSB connection is lost
							if (isWsb == false && pChannel->getConsumerRoutingChannel()->inPreferredHost == false && ommBaseImpl.getConsumerRoutingSession()->enhancedItemRecovery == true && (pRsslReactorChannel->pRsslChannel != NULL && pRsslReactorChannel->pRsslChannel->state == RSSL_CH_STATE_ACTIVE))
							{
								// Attempt to re-route the item.  If this succeeds, close the item and submit it on the other channel.  Otherwise, leave the item on this session channel
								ConsumerRoutingSessionChannel* pOldChannel = pSingleItem->sessionChannel;
								if (ommBaseImpl.getConsumerRoutingSession()->matchRequestToSessionChannel(*pSingleItem) == false)
								{
									pSingleItem->sessionChannel = pOldChannel;
									// Add the item to the pending list here.  If we get an OPEN/OK response at all, we'll move this back to the channel's list.
									ommBaseImpl.getConsumerRoutingSession()->pendingRequestList.addItem(pSingleItem);
								}
								else
								{
									// Close the item on the old channel
									sendCloseOnOldReactorChannel = true;
									// Submit this item.  Do not reroute, because that has already been done.
									submitRequest = true;
								}
							}
						}
						else if (origState.streamState == RSSL_STREAM_OPEN && origState.dataState == RSSL_DATA_OK)
						{
							if (pSingleItem->sessionChannelItemClosedList != NULL)
							{
								for (UInt32 i = 0; i < pSingleItem->sessionChannelItemClosedList->size(); i++)
								{
									bool* tmpbool = (*(pSingleItem->sessionChannelItemClosedList))[i];
									memset((void*)((*(pSingleItem->sessionChannelItemClosedList))[i]), 0, (sizeof(bool)* pSingleItem->closedListSize));
								}
							}

							if (pSingleItem->getItemList() == &ommBaseImpl.getConsumerRoutingSession()->pendingRequestList)
							{
								// This item was in pending, but the stream is now active, so move it to the session channel's request list.
								pSingleItem->sessionChannel->routedRequestList.addItem(pSingleItem);
							}
						}
					}
				}
			}

		}
		else if(item->getType() == Item::NiProviderDictionaryItemEnum)
		{
			NiProviderDictionaryItem* pNIProvDictionaryItem = static_cast<NiProviderDictionaryItem*>(item);

			_statusMsg.getDecoder().setServiceName(pNIProvDictionaryItem->getServiceName().c_str(), pNIProvDictionaryItem->getServiceName().length());
			_statusMsg.getDecoder().setServiceId((UInt16)pNIProvDictionaryItem->getServiceId());
		}
		else if (item->getType() == Item::IProviderDictionaryItemEnum)
		{
			IProviderDictionaryItem* pIProvDictionaryItem = static_cast<IProviderDictionaryItem*>(item);

			_statusMsg.getDecoder().setServiceName(pIProvDictionaryItem->getServiceName().c_str(), pIProvDictionaryItem->getServiceName().length());
			_statusMsg.getDecoder().setServiceId((UInt16)pIProvDictionaryItem->getServiceId());
		}
	}

	_ommCommonImpl.msgDispatched();

	item->setEventChannel((void*)pRsslReactorChannel);
	item->onAllMsg( _statusMsg );
	item->onStatusMsg( _statusMsg );

	if(sendClose == true)
		((SingleItem*)item)->sendClose();
	if(sendCloseOnOldReactorChannel == true)
		((SingleItem*)item)->submitCloseOnReactorChannel(pRsslReactorChannel);
	// Immediately re-route and submit this item.  This will move the item out of the current session channel's item list
	if(submitRequest == true)
		((SingleItem*)item)->reSubmit(rerouteRequest);

	// This wil remove the item in all cases from the lists. 
	if ( pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_STATE )
	{
		if ( _statusMsg.getState().getStreamState() != OmmState::OpenEnum )
		{
			item->remove();
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processGenericMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, Item* item, const RsslDataDictionary* pRsslDataDictionary )
{
	SingleItem* pSingleItem = NULL;
	NiProviderDictionaryItem* pNIProvDictionaryItem = NULL;
	IProviderDictionaryItem* pIProvDictionaryItem = NULL;

	StaticDecoder::setRsslData( &_genericMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		pRsslDataDictionary );

	switch (item->getType())
	{
		case Item::BatchItemEnum:
			// Fall through to do the single item processing, because we will never get a refresh message with the initial batch request-the watchlist will always generate a status message
			item = static_cast<BatchItem*>(item)->getSingleItem(pRsslMsg->msgBase.streamId);
		case Item::SingleItemEnum:
			pSingleItem = static_cast<SingleItem*>(item);

			// Set the service name here if it's present.
			if (pSingleItem->getDirectory() != NULL)
			{
				if (!pSingleItem->getServiceListName().empty())
				{
					_genericMsg.getDecoder().setServiceName(pSingleItem->getServiceListName().c_str(), pSingleItem->getServiceListName().length());
				}
				else
				{
					_genericMsg.getDecoder().setServiceName(pSingleItem->getDirectory()->getName().c_str(), pSingleItem->getDirectory()->getName().length());
				}

				if (pSingleItem->getDirectory()->hasGeneratedServiceId())
				{
					_genericMsg.getDecoder().setServiceId((UInt16)pSingleItem->getDirectory()->getGeneratedServiceId());
				}
			}
			else
			{
				_genericMsg.getDecoder().setServiceName(NULL, 0);
			}
			break;
		case Item::NiProviderDictionaryItemEnum:
			pNIProvDictionaryItem = static_cast<NiProviderDictionaryItem*>(item);

			_genericMsg.getDecoder().setServiceName(pNIProvDictionaryItem->getServiceName().c_str(), pNIProvDictionaryItem->getServiceName().length());
			_genericMsg.getDecoder().setServiceId((UInt16)pNIProvDictionaryItem->getServiceId());
			break;
		case Item::IProviderDictionaryItemEnum:
			pIProvDictionaryItem = static_cast<IProviderDictionaryItem*>(item);

			_genericMsg.getDecoder().setServiceName(pIProvDictionaryItem->getServiceName().c_str(), pIProvDictionaryItem->getServiceName().length());
			_genericMsg.getDecoder().setServiceId((UInt16)pIProvDictionaryItem->getServiceId());
			break;
	}


	_ommCommonImpl.msgDispatched();

	item->setEventChannel((void*)pRsslReactorChannel);
	item->onAllMsg( _genericMsg );
	item->onGenericMsg( _genericMsg );

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet ItemCallbackClient::processAckMsg( RsslMsg* pRsslMsg, RsslReactorChannel* pRsslReactorChannel, Item* item, const RsslDataDictionary* pRsslDataDictionary )
{
	SingleItem* pSingleItem = NULL;
	NiProviderDictionaryItem* pNIProvDictionaryItem = NULL;
	IProviderDictionaryItem* pIProvDictionaryItem = NULL;

	StaticDecoder::setRsslData( &_ackMsg, pRsslMsg,
		pRsslReactorChannel->majorVersion,
		pRsslReactorChannel->minorVersion,
		pRsslDataDictionary );

	switch (item->getType())
	{
		case Item::BatchItemEnum:
			// Fall through to do the single item processing, because we will never get a refresh message with the initial batch request-the watchlist will always generate a status message
			item = static_cast<BatchItem*>(item)->getSingleItem(pRsslMsg->msgBase.streamId);
		case Item::SingleItemEnum:
			pSingleItem = static_cast<SingleItem*>(item);

			// Set the service name here if it's present.
			if (pSingleItem->getDirectory() != NULL)
			{
				if (!pSingleItem->getServiceListName().empty())
				{
					_ackMsg.getDecoder().setServiceName(pSingleItem->getServiceListName().c_str(), pSingleItem->getServiceListName().length());
				}
				else
				{
					_ackMsg.getDecoder().setServiceName(pSingleItem->getDirectory()->getName().c_str(), pSingleItem->getDirectory()->getName().length());
				}

				if (pSingleItem->getDirectory()->hasGeneratedServiceId())
				{
					_ackMsg.getDecoder().setServiceId((UInt16)pSingleItem->getDirectory()->getGeneratedServiceId());
				}
			}
			else
			{
				_ackMsg.getDecoder().setServiceName(NULL, 0);
			}
			break;
		case Item::NiProviderDictionaryItemEnum:
			pNIProvDictionaryItem = static_cast<NiProviderDictionaryItem*>(item);

			_ackMsg.getDecoder().setServiceName(pNIProvDictionaryItem->getServiceName().c_str(), pNIProvDictionaryItem->getServiceName().length());
			_ackMsg.getDecoder().setServiceId((UInt16)pNIProvDictionaryItem->getServiceId());
			break;
		case Item::IProviderDictionaryItemEnum:
			pIProvDictionaryItem = static_cast<IProviderDictionaryItem*>(item);

			_ackMsg.getDecoder().setServiceName(pIProvDictionaryItem->getServiceName().c_str(), pIProvDictionaryItem->getServiceName().length());
			_ackMsg.getDecoder().setServiceId((UInt16)pIProvDictionaryItem->getServiceId());
			break;
	}

	_ommCommonImpl.msgDispatched();

	item->setEventChannel((void*)pRsslReactorChannel);
	item->onAllMsg( _ackMsg );
	item->onAckMsg( _ackMsg );

	return RSSL_RC_CRET_SUCCESS;
}

UInt64 ItemCallbackClient::registerClient( const ReqMsg& reqMsg, OmmConsumerClient& ommConsClient, void* closure, UInt64 parentHandle )
{
	OmmBaseImpl& ommBaseImpl = static_cast<OmmBaseImpl&>(_ommCommonImpl);

	if ( ommBaseImpl.isAtExit() )
	{
		throwIueException("Application or user initiated exit.", OmmInvalidUsageException::InvalidOperationEnum);
	}

	if ( !parentHandle )
	{
		const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );

		switch ( reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType )
		{
		case RSSL_DMT_LOGIN :
			{
				LoginItem* pItem = ommBaseImpl.getLoginCallbackClient().getLoginItem(reqMsg, ommConsClient, closure);

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
					temp.append(". Instance name='").append( _ommCommonImpl.getInstanceName()).append("'.");
					_ommCommonImpl.handleIue(temp , OmmInvalidUsageException::InvalidArgumentEnum);
					return 0;
				}

				if ( ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) == 0 )
				{
					EmaString temp( "ReqMsg's name is not defined. " );
					temp.append( "Instance name='" ).append( _ommCommonImpl.getInstanceName() ).append( "'." );
					_ommCommonImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
					return 0;
				}

				DictionaryItem* pItem = DictionaryItem::create( ommBaseImpl, ommConsClient, closure );

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
			DirectoryItem* pItem = DirectoryItem::create( ommBaseImpl, ommConsClient, closure, NULL );

			if ( pItem )
			{
				try {
					if ( !pItem->open( reqMsg ) )
						Item::destroy( (Item*&)pItem );
					else
					{
						if (ommBaseImpl.getConsumerRoutingSession() == NULL)
						{
							addToList( pItem );
						}
						else
						{
							ommBaseImpl.getDirectoryCallbackClient().addItem(pItem);
						}
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
					BatchItem* pBatchItem = BatchItem::create( ommBaseImpl, ommConsClient, closure );
					EmaVector<EmaString> nameList;

					int numOfItems = reqMsgEncoder.getBatchItemList(&nameList);
					if ( pBatchItem )
					{
						try 
						{
							/* Start splitting the batch request into individual item request if _nextStreamIdWrapAround is true. */
							if (ommBaseImpl.getItemCallbackClient().nextStreamIdWrapAround(numOfItems))
							{
								if (!splitAndSendSingleRequest(reqMsg, ommConsClient, closure))
								{
									Item::destroy((Item*&)pBatchItem);

									EmaString temp("Failed to split a batch request into single item requests on registerClient(). ");
									temp.append("Instance name='").append(ommBaseImpl.getInstanceName()).append("'.");
									ommBaseImpl.handleIue(temp, OmmInvalidUsageException::InternalErrorEnum);

									return (UInt64)0;
								}

								// This will ensure that the batch request gets properly cleaned up in both request request routing and normal operation.
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
									// The request has already been added to the channel that it's going to be routed to or the pending list
									// It will get moved to this(itemCallbackClient) list when the generated close message is received from the watchlist
									if (ommBaseImpl.getConsumerRoutingSession() == NULL)
										addToList(pBatchItem);

									addToMap(pBatchItem);

									// Setup the individual request information in the batch
									// The single item list vector is 1 indexed, so we need decrement namelist indexes 
									for (UInt32 i = 1; i < pBatchItem->getSingleItemList().size(); i++)
									{
										pBatchItem->getSingleItemList()[i]->setReqMsg(reqMsg, &nameList[i-1]);

										pBatchItem->getSingleItemList()[i]->setServiceName(pBatchItem->getServiceName());

										if (pBatchItem->getImpl().getConsumerRoutingSession())
										{
											if (pBatchItem->sessionChannel != NULL)
											{
												pBatchItem->sessionChannel->routedRequestList.addItem(pBatchItem->getSingleItemList()[i]);
												pBatchItem->getSingleItemList()[i]->sessionChannel = pBatchItem->sessionChannel;
											}
											else
											{
												EmaString temp;
												if (reqMsgEncoder.hasServiceListName())
												{
													temp.append("Service list name of '");
													temp.append(reqMsgEncoder.getServiceListName()).
														append("' does not match any active services, will request when they become available.");
												}
												else 
												{
													temp.append("Service name of '");
													temp.append(reqMsgEncoder.getServiceName()).
														append("' does not match any active services, will request when it becomes available.");
												}
												pBatchItem->getImpl().getConsumerRoutingSession()->pendingRequestList.addItem(pBatchItem->getSingleItemList()[i]);
												pBatchItem->getSingleItemList()[i]->scheduleItemSuspectStatus(reqMsgEncoder, temp);
											}

											// If this is a consumer routing session, set up the closed directory list.
											if (ommBaseImpl.getConsumerRoutingSession() != NULL)
											{
												int serviceCount = 1;
												if (reqMsgEncoder.hasServiceListName())
												{
													// We've already checked in the send above, so this service list name is valid.
													ServiceList** pServiceList = ommBaseImpl.getActiveConfig().serviceListByName.find(&(reqMsgEncoder.getServiceListName()));
													serviceCount = (*pServiceList)->concreteServiceList().size();
												}

												pBatchItem->getSingleItemList()[i]->sessionChannelItemClosedList = new EmaVector<bool*>();
												pBatchItem->getSingleItemList()[i]->closedListSize = serviceCount;

												for (UInt32 j = 0; j < ommBaseImpl.getConsumerRoutingSession()->routingChannelList.size(); ++j)
												{
													bool* pDirectoryFlagList = (bool*)malloc(sizeof(bool) * serviceCount);
													memset((void*)pDirectoryFlagList, 0, sizeof(bool) * serviceCount);
													pBatchItem->getSingleItemList()[i]->sessionChannelItemClosedList->push_back(pDirectoryFlagList);
												}
											}
										}
										else
										{
											addToList(pBatchItem->getSingleItemList()[i]);
										}

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
					pItem = SingleItem::create(ommBaseImpl, ommConsClient, closure, 0);

					if ( pItem )
					{
						try {
							if ( !pItem->open( reqMsg ) )
								Item::destroy( (Item*&)pItem );
							else
							{
								// The request has already been added to the channel that it's going to be routed to or the pending list
								if(ommBaseImpl.getConsumerRoutingSession() == NULL)
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
			temp.append( "Instance name='" ).append( ommBaseImpl.getInstanceName() ).append( "'." );
			ommBaseImpl.handleIhe(parentHandle, temp);
			return 0;
		}

		if ( ((Item*)parentHandle)->getType() != Item::TunnelItemEnum )
		{
			EmaString temp( "Invalid attempt to use " );
			temp += ((Item*)parentHandle)->getTypeAsString();
			temp.append( " as parentHandle on registerClient(). " );
			temp.append( "Instance name='" ).append( ommBaseImpl.getInstanceName() ).append( "'." );
			ommBaseImpl.handleIhe(parentHandle, temp);
			return 0;
		}

		SubItem* pItem = SubItem::create( ommBaseImpl, ommConsClient, closure, (Item*)parentHandle );

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
	if (static_cast<OmmBaseImpl&>(_ommCommonImpl).isAtExit())
	{
		throwIueException("Application or user initiated exit.", OmmInvalidUsageException::InvalidOperationEnum);
	}

	if ( !parentHandle )
	{
		const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );

		switch ( reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType )
		{
		case RSSL_DMT_LOGIN:
		{
			NiProviderLoginItem* pItem = static_cast<OmmBaseImpl&>(_ommCommonImpl).getLoginCallbackClient().getLoginItem( reqMsg, ommProvClient, closure );

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
				temp.append( ". Instance name='" ).append( _ommCommonImpl.getInstanceName() ).append( "'." );
				_ommCommonImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
				return 0;
			}

			if ( ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_NAME ) == 0 )
			{
				EmaString temp("ReqMsg's name is not defined. ");
				temp.append("Instance name='").append(_ommCommonImpl.getInstanceName()).append("'.");
				_ommCommonImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
				return 0;
			}

			ProviderItem* pItem;

			if (_ommCommonImpl.getImplType() == OmmCommonImpl::NiProviderEnum)
			{
				pItem = NiProviderDictionaryItem::create(static_cast<OmmBaseImpl&>(_ommCommonImpl), ommProvClient, closure);
			}
			else
			{
				pItem = IProviderDictionaryItem::create(static_cast<OmmServerBaseImpl&>(_ommCommonImpl), ommProvClient, closure);
			}

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
			temp.append(". Instance name='").append( _ommCommonImpl.getInstanceName()).append("'.");
			_ommCommonImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
			return 0;
		}
		}
	}
	else
	{
		EmaString temp( "Attempt to use parentHandle on OmmProvider::registerClient(). " );
		temp.append( "Instance name='" ).append( _ommCommonImpl.getInstanceName() ).append( "'." );
		_ommCommonImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum);
		return 0;
	}
}

UInt64 ItemCallbackClient::registerClient( const TunnelStreamRequest& tunnelStreamRequest, OmmConsumerClient& ommConsClient, void* closure )
{
	if (static_cast<OmmBaseImpl&>(_ommCommonImpl).isAtExit())
	{
		throwIueException("Application or user initiated exit.", OmmInvalidUsageException::InvalidOperationEnum);
	}

	TunnelItem* pItem = TunnelItem::create( static_cast<OmmBaseImpl&>(_ommCommonImpl), ommConsClient, closure );

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
	if ( !_itemMap.find( handle ) || ((Item*)handle)->getClosedStatusInfo())
	{
		EmaString temp( "Attempt to use invalid Handle on reissue(). " );
		temp.append( "Instance name='" ).append( _ommCommonImpl.getInstanceName() ).append( "'." );
		_ommCommonImpl.handleIhe(handle, temp);
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
		temp.append( "Instance name='" ).append( _ommCommonImpl.getInstanceName() ).append( "'." );
		_ommCommonImpl.handleIhe(handle, temp);
		return;
	}

	((Item*)handle)->submit( postMsg );
}

void ItemCallbackClient::submit( const GenericMsg& genericMsg, UInt64 handle )
{
	if ( !_itemMap.find( handle ) )
	{
		EmaString temp( "Attempt to use invalid Handle on submit( const GenericMsg& ). " );
		temp.append( "Instance name='" ).append( _ommCommonImpl.getInstanceName() ).append( "'." );
		_ommCommonImpl.handleIhe(handle, temp);
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
	_streamIdMap.insert(pItem->getStreamId(), pItem);
}

void ItemCallbackClient::removeFromMap(Item* pItem)
{
	_ommCommonImpl.getUserMutex().lock();

	_itemMap.erase( (UInt64)pItem );

	if ( pItem->getStreamId() != 0 )
		_streamIdMap.erase( pItem->getStreamId() );
	
	_ommCommonImpl.getUserMutex().unlock();
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
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder());
	RsslRequestMsg* rsslReqMsg = reqMsgEncoder.getRsslRequestMsg();
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
						SingleItem* pItem = NULL;

						while (rsslDecodeArrayEntry(&decodeIter, &rsslBuffer) != RSSL_RET_END_OF_CONTAINER)
						{
							if (rsslDecodeBuffer(&decodeIter, &rsslReqMsg->msgBase.msgKey.name) == RSSL_RET_SUCCESS)
							{
								pItem = SingleItem::create(static_cast<OmmBaseImpl&>(_ommCommonImpl), ommConsClient, closure, 0);

								if (pItem)
								{
									try {
										if (!pItem->open(reqMsg))
											Item::destroy((Item*&)pItem);
										else
										{
											// The request has already been added to the channel that it's going to be routed to or the pending list if this is request routing.
											if (static_cast<OmmBaseImpl&>(_ommCommonImpl).getConsumerRoutingSession() == NULL)
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
		if ( _ommCommonImpl.getImplType() == OmmCommonImpl::ConsumerEnum )
		{
			_nextStreamId = CONSUMER_STARTING_STREAM_ID;
		}
		else
		{
			_nextStreamId = PROVIDER_STARTING_STREAM_ID;
		}

		_nextStreamIdWrapAround = true; 

		if (OmmLoggerClient::VerboseEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity)
		{
			EmaString temp("Reach max number available for next stream id, will wrap around");
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp.trimWhitespace());
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
		while ( isStreamIdInUse(++_nextStreamId) );
		_streamIdAccessMutex.unlock();

		if (_nextStreamId < 0)
		{
			EmaString temp("Unable to obtain next available stream id for item request.");
			if (OmmLoggerClient::ErrorEnum >= _ommCommonImpl.getActiveLoggerConfig().minLoggerSeverity)
				_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, temp);

			if (_ommCommonImpl.hasErrorClientHandler())
			{
				_ommCommonImpl.getErrorClientHandler().onInvalidUsage(temp);
				_ommCommonImpl.getErrorClientHandler().onInvalidUsage(temp, OmmInvalidUsageException::InternalErrorEnum);
			}
			else
				throwIueException( temp, OmmInvalidUsageException::InternalErrorEnum );
		}

		return _nextStreamId;
	}
}

bool ItemCallbackClient::nextStreamIdWrapAround(UInt32 numberOfBatchItems)
{
	return ((UInt32)_nextStreamId > CONSUMER_MAX_STREAM_ID_MINUSONE - numberOfBatchItems);
}

ItemWatchList::ItemWatchList() :
	_itemList(INITIAL_ITEM_WATCHLIST_SIZE)
{
}

ItemWatchList::~ItemWatchList()
{
	_itemList.clear();
}

void ItemWatchList::addItem(ProviderItem* item)
{
	_itemList.push_back(item);
}

void ItemWatchList::removeItem(ProviderItem* item)
{
	_itemList.removeValue(item);
}

RsslMsg* ItemWatchList::processRsslMsg(RsslMsg* pRsslMsg, ProviderItem* pProviderItem, bool& addedMsgKey)
{
	addedMsgKey = false;

	switch ( pRsslMsg->msgBase.msgClass )
	{
		case RSSL_MC_REFRESH:
		{
			if (pProviderItem->processInitialResp(&pRsslMsg->refreshMsg, pRsslMsg->msgBase.domainType > MMT_DICTIONARY) == false)
			{
				pProviderItem->scheduleItemClosedRecoverableStatus("received response is mismatch with the initial request");

				return 0;
			}

			if ( ( pRsslMsg->refreshMsg.flags & RSSL_RFMF_HAS_MSG_KEY ) == 0 )
			{
				RsslMsg* copiedMsg = rsslCopyMsg( pRsslMsg, RSSL_CMF_ALL_FLAGS & ~(RSSL_CMF_KEY | RSSL_CMF_MSG_BUFFER), 0, NULL );

				if (copiedMsg != NULL)
				{
					copiedMsg->msgBase.msgKey = pProviderItem->getRsslMsgKey();
					copiedMsg->refreshMsg.flags |= RSSL_RFMF_HAS_MSG_KEY;
					addedMsgKey = true;
					return copiedMsg;
				}
			}

			break;
		}
		case RSSL_MC_STATUS:
		{
			if ( ( pRsslMsg->statusMsg.flags & RSSL_STMF_HAS_MSG_KEY ) == 0 )
			{
				RsslMsg* copiedMsg = rsslCopyMsg( pRsslMsg, RSSL_CMF_ALL_FLAGS & ~( RSSL_CMF_KEY | RSSL_CMF_MSG_BUFFER ), 0, NULL );

				if (copiedMsg != NULL)
				{
					copiedMsg->msgBase.msgKey = pProviderItem->getRsslMsgKey();
					copiedMsg->statusMsg.flags |= RSSL_STMF_HAS_MSG_KEY;
					addedMsgKey = true;
					return copiedMsg;
				}
			}

			break;
		}
	}

	return pRsslMsg;
}

void ItemWatchList::itemRequestTimeout( void* pInfo )
{
	ProviderItem* pItem = reinterpret_cast<ProviderItem*>(pInfo);

	pItem->_timeOutExpired = true;

    if ( pItem->_closedInfo ) return;

	RsslCloseMsg rsslCloseMsg;
	rsslClearCloseMsg(&rsslCloseMsg);

	rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslCloseMsg.msgBase.domainType = pItem->getDomainType();

	pItem->submit(&rsslCloseMsg);

	ItemStatusInfo closedStatusInfo(pItem, "request is timeout");

	ItemCallbackClient::sendItemStatus(&closedStatusInfo);
}

// This is only called for NIProv and IProv
void ItemWatchList::processChannelEvent(RsslReactorChannelEvent* pChannelEvent)
{
	switch (pChannelEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_DOWN:
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:
			notifyClosedRecoverableStatusMessage();
			break;
	}
}

void ItemWatchList::processCloseLogin( const ClientSession* pClientSession )
{
	for (unsigned int index = 0; index < _itemList.size(); index++)
	{
		ProviderItem* pItem = _itemList[index];

		if ( pItem->getClientSession() == pClientSession )
		{
			pItem->scheduleItemClosedRecoverableStatus("channel is closed");
		}
	}
}

void ItemWatchList::processServiceDelete(const ClientSession* clientSession, UInt64 serviceId)
{
	for (unsigned int index = 0; index < _itemList.size(); index++)
	{
		ProviderItem* pItem = _itemList[index];

		if ( pItem->_specifiedServiceInReq )
		{
			if ( clientSession )
			{
				if ( pItem->getClientSession() == clientSession && pItem->getServiceId() == serviceId)
				{
					pItem->scheduleItemClosedRecoverableStatus("service is deleted");
				}
			}
			else if ( pItem->getServiceId() == serviceId)
			{
				pItem->scheduleItemClosedRecoverableStatus("service is deleted");
			}
		}
	}
}

void ItemWatchList::notifyClosedRecoverableStatusMessage()
{
	for ( unsigned int index = 0; index < _itemList.size(); index++ )
	{
		ProviderItem* pItem = _itemList[index];

		pItem->scheduleItemClosedRecoverableStatus("channel down");
	}
}
