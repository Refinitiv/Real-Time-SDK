/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ChannelCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "LoginCallbackClient.h"
#include "OmmConsumerClient.h"
#include "OmmConsumerErrorClient.h"
#include "OmmConsumerEvent.h"
#include "OmmConsumerImpl.h"
#include "ReqMsg.h"
#include "ReqMsgEncoder.h"
#include "GenericMsgEncoder.h"
#include "StaticDecoder.h"
#include "Utilities.h"

#include <new>

using namespace thomsonreuters::ema::access;

#define DEFAULT_DIRECTORY_RESP_MSG_SIZE 8192

const EmaString DirectoryCallbackClient::_clientName( "DirectoryCallbackClient" );

DictionaryList::DictionaryList() :
	_toString(),
	_toStringSet( false )
{
}

DictionaryList::~DictionaryList()
{
}

DictionaryList::DictionaryList( const DictionaryList& other ) :
	_toString(),
	_toStringSet( false )
{
	UInt32 size = other._list.size();

	for ( UInt32 idx = 0; idx < size; ++idx )
		_list.push_back( other._list[idx] );
}

DictionaryList& DictionaryList::operator=( const DictionaryList& other )
{
	if ( this != &other )
	{
		_list.clear();

		_toStringSet = false;

		UInt32 size = other._list.size();

		for ( UInt32 idx = 0; idx < size; ++idx )
			_list.push_back( other._list[idx] );
	}

	return *this;
}

DictionaryList& DictionaryList::clear()
{
	_list.clear();
	_toStringSet = false;
	return *this;
}

DictionaryList& DictionaryList::addDictionary( const EmaString& dictionaryName )
{
	_list.push_back( dictionaryName );
	_toStringSet = false;
	return *this;
}

const EmaVector<EmaString>& DictionaryList::getDictionaryList() const
{
	return _list;
}

const EmaString& DictionaryList::toString() const
{
	if ( !_toStringSet )
	{
		_toStringSet = true;

		_toString.set( "'" );

		UInt32 size = _list.size();

		for ( UInt32 idx = 0; idx < size; ++idx )
			_toString.append( _list[idx] ).append( " " );

		_toString.append( "' " );
	}

	return _toString;
}

Info::Info() :
	_name(),
	_vendor(),
	_itemList(),
	_dictionariesProvided(),
	_dictionariesUsed(),
	_toString(),
	_isSource( 0 ),
	_toStringSet( false )
{
}

Info::~Info()
{
}

Info& Info::operator=( const Info& other )
{
	if ( this == &other ) return *this;

	_name = other._name;
	_vendor = other._vendor;
	_itemList = other._itemList;
	_dictionariesProvided = other._dictionariesProvided;
	_dictionariesUsed = other._dictionariesUsed;
	_isSource = other._isSource;
	_toStringSet = false;
	return *this;
}

Info& Info::clear()
{
	_name.clear();
	_vendor.clear();
	_itemList.clear();
	_dictionariesProvided.clear();
	_dictionariesUsed.clear();
	_isSource = 0;
	_toStringSet = false;
	return *this;
}

const EmaString& Info::getName() const
{
	return _name;
}

Info& Info::setName( const EmaString& name )
{
	_toStringSet = false;
	_name = name;
	return *this;
}

Info& Info::setName( const char* pName, UInt32 length )
{
	_toStringSet = false;
	_name.set( pName, length );
	return *this;
}

const EmaString& Info::getVendor() const
{
	return _vendor;
}

Info& Info::setVendor( const EmaString& vendor )
{
	_vendor = vendor;
	_toStringSet = false;
	return *this;
}

Info& Info::setVendor( const char* pVendor, UInt32 length )
{
	_toStringSet = false;
	_vendor.set( pVendor, length );
	return *this;
}

const EmaString& Info::getItemList() const
{
	return _itemList;
}

Info& Info::setItemList( const EmaString& itemList )
{
	_itemList = itemList;
	_toStringSet = false;
	return *this;
}

Info& Info::setItemList( const char* pItemList, UInt32 length )
{
	_toStringSet = false;
	_itemList.set( pItemList, length );
	return *this;
}

UInt64 Info::getIsSource() const
{
	return _isSource;
}

Info& Info::setIsSource( UInt64 isSource )
{
	_isSource = isSource;
	_toStringSet = false;
	return *this;
}

const DictionaryList& Info::getDictionariesProvided() const
{
	return _dictionariesProvided;
}

Info& Info::addDictionaryProvided( const EmaString& dictionary )
{
	_toStringSet = false;
	_dictionariesProvided.addDictionary( dictionary );
	return *this;
}

Info& Info::addDictionaryProvided( const char* pDictionary, UInt32 length )
{
	_toStringSet = false;
	_dictionariesProvided.addDictionary( EmaString( pDictionary, length ) );
	return *this;
}

const DictionaryList& Info::getDictionariesUsed() const
{
	return _dictionariesUsed;
}

Info& Info::addDictionaryUsed( const EmaString& dictionary )
{
	_toStringSet = false;
	_dictionariesUsed.addDictionary( dictionary );
	return *this;
}

Info& Info::addDictionaryUsed( const char* pDictionary, UInt32 length )
{
	_toStringSet = false;
	_dictionariesUsed.addDictionary( EmaString( pDictionary, length ) );
	return *this;
}

State::State() :
	_serviceState( 0 ),
	_acceptingRequests( 1 ),
	_deleted( false ),
	_toString(),
	_toStringSet( false )
{
}

State::~State()
{
}

State& State::clear()
{
	_serviceState = 0;
	_acceptingRequests = 0;
	_deleted = false;
	_toStringSet = false;
	return *this;
}

State& State::operator=( const State& other )
{
	if ( this == &other ) return *this;

	_serviceState = other._serviceState;
	_acceptingRequests = other._acceptingRequests;
	_deleted = other._deleted;
	_toStringSet = false;

	return *this;
}

UInt64 State::getServiceState() const
{
	return _serviceState;
}

State& State::setServiceState( UInt64 serviceState )
{
	_serviceState = serviceState;
	_toStringSet = false;
	return *this;
}

UInt64 State::getAcceptingRequests() const
{
	return _acceptingRequests;
}

State& State::setAcceptingRequests( UInt64 acceptingRequests )
{
	_acceptingRequests = acceptingRequests;
	_toStringSet = false;
	return *this;
}

State& State::markDeleted()
{
	_deleted = true;
	_acceptingRequests = 0;
	_serviceState = 0;

	return *this;
}

const EmaString& State::toString() const
{
	if ( !_toStringSet )
	{
		_toStringSet = true;

		_toString.set( "'" );

		if ( !_deleted )
		{
			if ( _serviceState )
				_toString.append( "Service is Up " );
			else
				_toString.append( "Service is Down " );

			if ( _acceptingRequests )
				_toString.append( "Service Accepts Requests" );
			else
				_toString.append( "Service Does Not Accept Requests" );
		}
		else
			_toString.append( "Service was deleted" );

		_toString.append( "' " );
	}

	return _toString;
}

Directory* Directory::create( OmmCommonImpl& ommCommonImpl )
{
	Directory* pDirectory = 0;

	try
	{
		pDirectory = new Directory();
	}
	catch ( std::bad_alloc ) {}

	if ( !pDirectory )
		ommCommonImpl.handleMee("Failed to create Directory.");

	return pDirectory;
}

void Directory::destroy( Directory*& pDirectory )
{
	if ( pDirectory )
	{
		delete pDirectory;
		pDirectory = 0;
	}
}

Directory::Directory() :
	_info(),
	_state(),
	_name(),
	_toString(),
	_id( 0 ),
	_hasInfo( false ),
	_hasState( false ),
	_toStringSet( false ),
	_pChannel( 0 )
{
}

Directory::~Directory()
{
}

Directory& Directory::clear()
{
	_info.clear();
	_state.clear();
	_name.clear();
	_id = 0;
	_hasInfo = false;
	_hasState = false;
	_toStringSet = false;
	_pChannel = 0;
	return *this;
}

Directory& Directory::markDeleted()
{
	_toStringSet = false;
	_state.markDeleted();
	return *this;
}

const EmaString& Directory::getName() const
{
	return _name;
}

EmaStringPtr Directory::getNamePtr() const
{
	return &_name;
}

Directory& Directory::setName( const EmaString& name )
{
	_toStringSet = false;
	_name = name;
	return *this;
}

Directory& Directory::setName( const char* pName, UInt32 length )
{
	_toStringSet = false;
	_name.set( pName, length );
	return *this;
}

const Info& Directory::getInfo() const
{
	return _info;
}

Directory& Directory::setInfo( const Info& info )
{
	_info = info;
	_hasInfo = true;
	_toStringSet = false;
	return *this;
}

const State& Directory::getState() const
{
	return _state;
}

Directory& Directory::setState( const State& state )
{
	_state = state;
	_hasState = true;
	_toStringSet = false;
	return *this;
}

UInt64 Directory::getId() const
{
	return _id;
}

Directory& Directory::setId( UInt64 id )
{
	_id = id;
	_toStringSet = false;
	return *this;
}

Channel* Directory::getChannel() const
{
	return _pChannel;
}

Directory& Directory::setChannel( Channel* pChannel )
{
	_toStringSet = false;
	_pChannel = pChannel;
	return *this;
}

DirectoryCallbackClient::DirectoryCallbackClient( OmmBaseImpl& ommBaseImpl ) :
	_directoryByIdHt( ommBaseImpl.getActiveConfig().serviceCountHint ),
	_directoryByNameHt( ommBaseImpl.getActiveConfig().serviceCountHint ),
	_ommBaseImpl( ommBaseImpl ),
	_refreshMsg(),
	_updateMsg(),
	_statusMsg(),
	_genericMsg()
{
	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Created DirectoryCallbackClient" );
	}
}

DirectoryCallbackClient::~DirectoryCallbackClient()
{
	Directory* directory = _directoryList.front();

	while ( directory )
	{
		removeDirectory( directory );
		directory = _directoryList.front();
	}

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Destroyed DirectoryCallbackClient" );
	}
}

DirectoryCallbackClient* DirectoryCallbackClient::create( OmmBaseImpl& ommBaseImpl )
{
	DirectoryCallbackClient* pClient = 0;

	try
	{
		pClient = new DirectoryCallbackClient( ommBaseImpl );
	}
	catch ( std::bad_alloc ) {}

	if ( !pClient )
	{
		const char* temp = "Failed to create DirectoryCallbackClient";
		if ( OmmLoggerClient::ErrorEnum >= ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		throwMeeException( temp );
	}

	return pClient;
}

void DirectoryCallbackClient::destroy( DirectoryCallbackClient*& pClient )
{
	if ( pClient )
	{
		delete pClient;
		pClient = 0;
	}
}

void DirectoryCallbackClient::initialize()
{
	rsslClearRDMDirectoryRequest( &_directoryRequest );

	_directoryRequest.rdmMsgBase.streamId = 2;

	if ( !_ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg )
	{
		_directoryRequest.filter = ( RDM_DIRECTORY_SERVICE_INFO_FILTER
		                             | RDM_DIRECTORY_SERVICE_STATE_FILTER
		                             | RDM_DIRECTORY_SERVICE_GROUP_FILTER
		                             | RDM_DIRECTORY_SERVICE_LOAD_FILTER
		                             | RDM_DIRECTORY_SERVICE_DATA_FILTER
		                             | RDM_DIRECTORY_SERVICE_LINK_FILTER );

		_directoryRequest.flags = RDM_DR_RQF_STREAMING;
	}
	else
	{
		if ( _ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER )
			_directoryRequest.filter = _ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.filter;
		else
		{
			if ( _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity <= OmmLoggerClient::WarningEnum )
			{
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum,
				                                       "Configured source directory request message contains no filter. Will request all filters" );
			}

			_directoryRequest.filter = ( RDM_DIRECTORY_SERVICE_INFO_FILTER
			                             | RDM_DIRECTORY_SERVICE_STATE_FILTER
			                             | RDM_DIRECTORY_SERVICE_GROUP_FILTER
			                             | RDM_DIRECTORY_SERVICE_LOAD_FILTER
			                             | RDM_DIRECTORY_SERVICE_DATA_FILTER
			                             | RDM_DIRECTORY_SERVICE_LINK_FILTER );
		}

		if ( _ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID )
		{
			_directoryRequest.serviceId = _ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.serviceId;
			_directoryRequest.flags |= RDM_DR_RQF_HAS_SERVICE_ID;
		}

		if ( !( _ommBaseImpl.getActiveConfig().pRsslDirectoryRequestMsg->flags & RSSL_RQMF_STREAMING ) )
		{
			if ( _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity <= OmmLoggerClient::WarningEnum )
			{
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum,
				                                       "Configured source directory request message contains no streaming flag. Will request streaming" );
			}
		}

		_directoryRequest.flags = RDM_DR_RQF_STREAMING;
	}

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "RDMDirectoryRequest message was populated with Filter(s)" );
		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_INFO_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_INFO_FILTER" );

		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_STATE_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_STATE_FILTER" );

		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_GROUP_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_GROUP_FILTER" );

		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_LOAD_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_LOAD_FILTER" );

		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_DATA_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_DATA_FILTER" );

		if ( _directoryRequest.filter & RDM_DIRECTORY_SERVICE_LINK_FILTER )
			temp.append( CR ).append( "RDM_DIRECTORY_SERVICE_LINK_FILTER" );

		if ( _directoryRequest.flags & RDM_DR_RQF_HAS_SERVICE_ID )
			temp.append( CR ).append( "requesting serviceId " ).append( _directoryRequest.serviceId );
		else
			temp.append( CR ).append( "requesting all services" );

		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}
}

RsslRDMDirectoryRequest* DirectoryCallbackClient::getDirectoryRequest()
{
	return &_directoryRequest;
}

RsslReactorCallbackRet DirectoryCallbackClient::processCallback( RsslReactor* pRsslReactor,
    RsslReactorChannel* pRsslReactorChannel,
    RsslRDMDirectoryMsgEvent* pEvent )
{
	RsslRDMDirectoryMsg* pDirectoryMsg = pEvent->pRDMDirectoryMsg;

	if ( !pDirectoryMsg )
	{
		_ommBaseImpl.closeChannel( pRsslReactorChannel );

		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			RsslErrorInfo* pError = pEvent->baseMsgEvent.pErrorInfo;

			EmaString temp( "Received event without RDMDirectory message" );
			temp.append( CR )
			.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( pError->rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( pError->rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( pError->rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( pError->errorLocation ).append( CR )
			.append( "Error Text " ).append( pError->rsslError.rsslErrorId ? pError->rsslError.text : "" );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if (pEvent && pEvent->baseMsgEvent.pStreamInfo && pEvent->baseMsgEvent.pStreamInfo->pUserSpec)
	{
		SingleItem* pItem = (SingleItem*)pEvent->baseMsgEvent.pStreamInfo->pUserSpec;

		return processCallback(pRsslReactor, pRsslReactorChannel, pEvent, pItem);
	}
	else
	{
		switch ( pDirectoryMsg->rdmMsgBase.rdmMsgType )
		{
		case RDM_DR_MT_REFRESH:
		{
			RsslState* pState = &pDirectoryMsg->refresh.state;
	
			if ( pState->streamState != RSSL_STREAM_OPEN )
			{
				_ommBaseImpl.closeChannel( pRsslReactorChannel );
	
				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );
	
					EmaString temp( "RDMDirectory stream was closed with refresh message " );
					temp.append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
	
				processDirectoryPayload( pDirectoryMsg->refresh.serviceCount, pDirectoryMsg->refresh.serviceList, pRsslReactorChannel->userSpecPtr );
	
				break;
			}
			else if ( pState->dataState == RSSL_DATA_SUSPECT )
			{
				if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );
	
					EmaString temp( "RDMDirectory stream state was changed to suspect with refresh message " );
					temp.append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
				}
	
				_ommBaseImpl.setState( OmmBaseImpl::DirectoryStreamOpenSuspectEnum );
	
				processDirectoryPayload( pDirectoryMsg->refresh.serviceCount, pDirectoryMsg->refresh.serviceList, pRsslReactorChannel->userSpecPtr );
				break;
			}
	
			_ommBaseImpl.setState( OmmBaseImpl::DirectoryStreamOpenOkEnum );
	
			if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );
	
				EmaString temp( "RDMDirectory stream was open with refresh message " );
				temp.append( tempState );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
	
			processDirectoryPayload( pDirectoryMsg->refresh.serviceCount, pDirectoryMsg->refresh.serviceList, pRsslReactorChannel->userSpecPtr );
			break;
		}
		case RDM_DR_MT_STATUS:
		{
			if ( pDirectoryMsg->status.flags & RDM_DR_STF_HAS_STATE )
			{
				RsslState* pState = &pDirectoryMsg->status.state;
	
				if ( pState->streamState != RSSL_STREAM_OPEN )
				{
					_ommBaseImpl.closeChannel( pRsslReactorChannel );
	
					if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString tempState( 0, 256 );
						stateToString( pState, tempState );
	
						EmaString temp( "RDMDirectory stream was closed with status message " );
						temp.append( tempState );
						_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
					}
					break;
				}
				else if ( pState->dataState == RSSL_DATA_SUSPECT )
				{
					if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString tempState( 0, 256 );
						stateToString( pState, tempState );
	
						EmaString temp( "RDMDirectory stream state was changed to suspect with status message " );
						temp.append( tempState );
						_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
					}
	
					_ommBaseImpl.setState( OmmBaseImpl::DirectoryStreamOpenSuspectEnum );
					break;
				}
	
				if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );
	
					EmaString temp( "RDMDirectory stream was open with status message " );
					temp.append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
				}
	
				_ommBaseImpl.setState( OmmBaseImpl::DirectoryStreamOpenOkEnum );
			}
			else
			{
				if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, "Received RDMDirectory status message without the state" );
				}
			}
			break;
		}
		case RDM_DR_MT_UPDATE:
		{
			if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Received RDMDirectory update message" );
			}
	
			processDirectoryPayload( pDirectoryMsg->update.serviceCount, pDirectoryMsg->update.serviceList, pRsslReactorChannel->userSpecPtr );
			break;
		}
		default:
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received unknown RDMDirectory message type" );
				temp.append( CR )
				.append( "message type value " )
				.append( pDirectoryMsg->rdmMsgBase.rdmMsgType );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}
			break;
		}
		}
	}

	return RSSL_RC_CRET_SUCCESS;
}

void DirectoryCallbackClient::processDirectoryPayload( UInt32 count, RsslRDMService* pServiceList, void* userSpecPtr )
{
	if ( !pServiceList && count )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Received RDMDirectory message indicating a number of services but without a service list" );
		}
		return;
	}

	if ( !userSpecPtr )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Internal error: no RsslReactorChannel->userSpecPtr" );
		}
		return;
	}

	for ( UInt32 jdx = 0; jdx < count; ++jdx )
	{
		switch ( pServiceList[jdx].action )
		{
		case RSSL_MPEA_ADD_ENTRY :
		{
			EmaString tempName( pServiceList[jdx].info.serviceName.data, pServiceList[jdx].info.serviceName.length );
			DirectoryPtr* pDeletedDirectoryPtr = _directoryByNameHt.find( &tempName );

			Directory* pDirectory = 0;

			if ( !pDeletedDirectoryPtr )
				pDirectory = Directory::create( _ommBaseImpl );
			else
				pDirectory = *pDeletedDirectoryPtr;

			if ( pServiceList[jdx].flags & RDM_SVCF_HAS_INFO )
			{
				Info info;

				info.setName( pServiceList[jdx].info.serviceName.data, pServiceList[jdx].info.serviceName.length );

				if ( pServiceList[jdx].info.flags & RDM_SVC_IFF_HAS_VENDOR )
					info.setVendor( pServiceList[jdx].info.vendor.data, pServiceList[jdx].info.vendor.length );

				if ( pServiceList[jdx].info.flags & RDM_SVC_IFF_HAS_ITEM_LIST )
					info.setItemList( pServiceList[jdx].info.itemList.data, pServiceList[jdx].info.itemList.length );

				if ( pServiceList[jdx].info.flags & RDM_SVC_IFF_HAS_IS_SOURCE )
					info.setIsSource( pServiceList[jdx].info.isSource );

				if ( pServiceList[jdx].info.flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED )
					for ( UInt32 idx = 0; idx < pServiceList[jdx].info.dictionariesProvidedCount; ++idx )
						info.addDictionaryProvided( pServiceList[jdx].info.dictionariesProvidedList[idx].data,
						                            pServiceList[jdx].info.dictionariesProvidedList[idx].length );

				if ( pServiceList[jdx].info.flags & RDM_SVC_IFF_HAS_DICTS_USED )
					for ( UInt32 idx = 0; idx < pServiceList[jdx].info.dictionariesUsedCount; ++idx )
						info.addDictionaryUsed( pServiceList[jdx].info.dictionariesUsedList[idx].data,
						                        pServiceList[jdx].info.dictionariesUsedList[idx].length );

				pDirectory->setInfo( info );
			}
			else
			{
				Directory::destroy( pDirectory );

				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Received RsslRDMService with Add action but no Service Info" );
				}
				break;
			}

			if ( pServiceList[jdx].flags & RDM_SVCF_HAS_STATE )
			{
				State state;

				state.setServiceState( pServiceList[jdx].state.serviceState );

				if ( pServiceList[jdx].state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS )
					state.setAcceptingRequests( pServiceList[jdx].state.acceptingRequests );

				pDirectory->setState( state );
			}

			if ( pDeletedDirectoryPtr )
			{
				if ( pDirectory->getId() != pServiceList[jdx].serviceId )
				{
					_directoryByIdHt.erase( pDirectory->getId() );
					pDirectory->setId( pServiceList[jdx].serviceId );
					_directoryByIdHt.insert( pServiceList[jdx].serviceId, pDirectory );
				}
				if ( ( *pDeletedDirectoryPtr )->getChannel() != ( ( Channel* ) userSpecPtr ) )
				{
					static_cast<Channel*>( userSpecPtr )->setDictionary( ( *pDeletedDirectoryPtr )->getChannel()->getDictionary() );
					( *pDeletedDirectoryPtr )->setChannel( ( Channel* ) userSpecPtr );
					static_cast<Channel*>( userSpecPtr )->addDirectory( *pDeletedDirectoryPtr );
				}
			}
			else
			{
				pDirectory->setName( pServiceList[jdx].info.serviceName.data, pServiceList[jdx].info.serviceName.length );

				pDirectory->setId( pServiceList[jdx].serviceId );

				pDirectory->setChannel( ( Channel* )userSpecPtr );

				static_cast<Channel*>( userSpecPtr )->addDirectory( pDirectory );

				addDirectory( pDirectory );
			}

			break;
		}
		case RSSL_MPEA_UPDATE_ENTRY :
		{
			DirectoryPtr* pDirectoryPtr = _directoryByIdHt.find( pServiceList[jdx].serviceId );

			if ( !pDirectoryPtr )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Received Update action for unknown RsslRDMService with service id " );
					temp.append( pServiceList[jdx].serviceId );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
				break;
			}
			else if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received Update action for RsslRDMService" );
				temp.append( CR )
				.append( "Service name " ).append( ( *pDirectoryPtr )->getName() ).append( CR )
				.append( "Service id " ).append( pServiceList[ jdx ].serviceId );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
			if ( ( *pDirectoryPtr )->getChannel() != ( ( Channel* ) userSpecPtr ) )
			{
				static_cast<Channel*>( userSpecPtr )->setDictionary( ( *pDirectoryPtr )->getChannel()->getDictionary() );
				( *pDirectoryPtr )->setChannel( ( Channel* ) userSpecPtr );
				static_cast<Channel*>( userSpecPtr )->addDirectory( *pDirectoryPtr );
			}
			if ( pServiceList[jdx].flags & RDM_SVCF_HAS_INFO )
			{
				Info info;

				info.setName( pServiceList[jdx].info.serviceName.data, pServiceList[jdx].info.serviceName.length );

				if ( info.getName() != ( *pDirectoryPtr )->getName() )
				{
					if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString temp( "Received Update action for RsslRDMService" );
						temp.append( CR )
						.append( "Service name " ).append( ( *pDirectoryPtr )->getName() ).append( CR )
						.append( "Service id " ).append( pServiceList[jdx].serviceId ).append( CR )
						.append( "attempting to change service name to " ).append( info.getName() );
						_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
					}
					break;
				}

				if ( pServiceList[jdx].info.flags & RDM_SVC_IFF_HAS_VENDOR )
					info.setVendor( pServiceList[jdx].info.vendor.data, pServiceList[jdx].info.vendor.length );

				if ( pServiceList[jdx].info.flags & RDM_SVC_IFF_HAS_ITEM_LIST )
					info.setItemList( pServiceList[jdx].info.itemList.data, pServiceList[jdx].info.itemList.length );

				if ( pServiceList[jdx].info.flags & RDM_SVC_IFF_HAS_IS_SOURCE )
					info.setIsSource( pServiceList[jdx].info.isSource );

				if ( pServiceList[jdx].info.flags & RDM_SVC_IFF_HAS_DICTS_PROVIDED )
					for ( UInt32 idx = 0; idx < pServiceList[jdx].info.dictionariesProvidedCount; ++idx )
						info.addDictionaryProvided( pServiceList[jdx].info.dictionariesProvidedList[idx].data,
						                            pServiceList[jdx].info.dictionariesProvidedList[idx].length );

				if ( pServiceList[jdx].info.flags & RDM_SVC_IFF_HAS_DICTS_USED )
					for ( UInt32 idx = 0; idx < pServiceList[jdx].info.dictionariesUsedCount; ++idx )
						info.addDictionaryProvided( pServiceList[jdx].info.dictionariesUsedList[idx].data,
						                            pServiceList[jdx].info.dictionariesUsedList[idx].length );

				( *pDirectoryPtr )->setInfo( info );
			}

			if ( pServiceList[jdx].flags & RDM_SVCF_HAS_STATE )
			{
				State state;

				state.setServiceState( pServiceList[jdx].state.serviceState );

				if ( pServiceList[jdx].state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS )
					state.setAcceptingRequests( pServiceList[jdx].state.acceptingRequests );

				( *pDirectoryPtr )->setState( state );
			}

			break;
		}
		case RSSL_MPEA_DELETE_ENTRY :
		{
			DirectoryPtr* pDirectoryPtr = _directoryByIdHt.find( pServiceList[jdx].serviceId );

			if ( !pDirectoryPtr )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Received Delete action for unknown RsslRDMService with service id " );
					temp.append( pServiceList[jdx].serviceId );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
				break;
			}
			else if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received Delete action for RsslRDMService" );
				temp.append( CR )
				.append( "Service name " ).append( ( *pDirectoryPtr )->getName() ).append( CR )
				.append( "Service id " ).append( pServiceList[jdx].serviceId );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}

			( *pDirectoryPtr )->markDeleted();

			break;
		}
		default :
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received unknown action for RsslRDMService. Action value " );
				temp.append( pServiceList[jdx].action );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}
			break;
		}
		}
	}
}

size_t DirectoryCallbackClient::UInt64rHasher::operator()( const UInt64& value ) const
{
	return value;
}

bool DirectoryCallbackClient::UInt64Equal_To::operator()( const UInt64& x, const UInt64& y ) const
{
	return x == y ? true : false;
}

size_t DirectoryCallbackClient::EmaStringPtrHasher::operator()( const EmaStringPtr& value ) const
{
	size_t result = 0;
	size_t magic = 8388593;

	const char* s = value->c_str();
	UInt32 n = value->length();
	while ( n-- )
		result = ( ( result % magic ) << 8 ) + ( size_t ) * s++;
	return result;
}

bool DirectoryCallbackClient::EmaStringPtrEqual_To::operator()( const EmaStringPtr& x, const EmaStringPtr& y ) const
{
	return *x == *y;
}

void DirectoryCallbackClient::addDirectory( Directory* pDirectory )
{
	_directoryList.push_back( pDirectory );

	_directoryByIdHt.insert( pDirectory->getId(), pDirectory );

	_directoryByNameHt.insert( pDirectory->getNamePtr(), pDirectory );

	if ( _ommBaseImpl.getActiveConfig().dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum ||
		(pDirectory->getState().getAcceptingRequests() == 1 && pDirectory->getState().getServiceState() == 1) )
		_ommBaseImpl.getDictionaryCallbackClient().downloadDictionary( *pDirectory );
}

void DirectoryCallbackClient::removeDirectory( Directory* pDirectory )
{
	_directoryByNameHt.erase( pDirectory->getNamePtr() );

	_directoryByIdHt.erase( pDirectory->getId() );

	_directoryList.remove( pDirectory );

	Directory::destroy( pDirectory );
}

const Directory* DirectoryCallbackClient::getDirectory( const EmaString& name ) const
{
	DirectoryPtr* pDirectoryPtr = _directoryByNameHt.find( &name );

	return pDirectoryPtr ? *pDirectoryPtr : 0;
}

const Directory* DirectoryCallbackClient::getDirectory( UInt32 id ) const
{
	UInt64 id64 = id;
	DirectoryPtr* pDirectoryPtr = _directoryByIdHt.find( id64 );

	return pDirectoryPtr ? *pDirectoryPtr : 0;
}

RsslReactorCallbackRet DirectoryCallbackClient::processCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel,
    RsslRDMDirectoryMsgEvent* pEvent, SingleItem* pItem )
{
	RsslRet retCode;
	RsslBuffer rsslMsgBuffer;
	RsslEncIterator eIter;
	rsslClearEncodeIterator( &eIter );

	if ( allocateAndSetEncodeIteratorBuffer( &rsslMsgBuffer, DEFAULT_DIRECTORY_RESP_MSG_SIZE, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
		&eIter, "DirectoryCallbackClient::processCallback" ) != RSSL_RET_SUCCESS )
	{
		return RSSL_RC_CRET_SUCCESS;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	retCode = rsslEncodeRDMDirectoryMsg( &eIter, pEvent->pRDMDirectoryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );

	while ( rsslErrorInfo.rsslError.rsslErrorId == RSSL_RET_BUFFER_TOO_SMALL )
	{
		free( rsslMsgBuffer.data );

		rsslClearEncodeIterator( &eIter );
		if ( allocateAndSetEncodeIteratorBuffer( &rsslMsgBuffer, rsslMsgBuffer.length * 2, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
			&eIter, "DirectoryCallbackClient::processCallback" ) != RSSL_RET_SUCCESS )
		{
			return RSSL_RC_CRET_SUCCESS;
		}

		clearRsslErrorInfo( &rsslErrorInfo );
		retCode = rsslEncodeRDMDirectoryMsg( &eIter, pEvent->pRDMDirectoryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );
	}

	if ( retCode != RSSL_RET_SUCCESS )
	{
		free( rsslMsgBuffer.data );

		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: failed to encode RsslRDMDirectoryMsg in DirectoryCallbackClient::processCallback()" );
			temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}
		return RSSL_RC_CRET_SUCCESS;
	}

	switch ( pEvent->pRDMDirectoryMsg->rdmMsgBase.rdmMsgType )
	{
	case RDM_DR_MT_REFRESH :
	{
		StaticDecoder::setRsslData( &_refreshMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		                            pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
		                            0 );

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
	}
	break;
	case RDM_DR_MT_UPDATE :
	{
		StaticDecoder::setRsslData( &_updateMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		                            pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
		                            0 );

		_ommBaseImpl.msgDispatched();
		pItem->onAllMsg( _updateMsg );
		pItem->onUpdateMsg( _updateMsg );
	}
	break;
	case RDM_DR_MT_STATUS :
	{
		StaticDecoder::setRsslData( &_statusMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		                            pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
		                            0 );

		_ommBaseImpl.msgDispatched();
		pItem->onAllMsg( _statusMsg );
		pItem->onStatusMsg( _statusMsg );

		if ( _statusMsg.hasState() && ( _statusMsg.getState().getStreamState() != OmmState::OpenEnum ) )
			pItem->remove();
	}
	break;
	case RDM_DR_MT_CONSUMER_STATUS :
	{
		StaticDecoder::setRsslData( &_genericMsg, &rsslMsgBuffer, RSSL_DT_MSG,
		                            pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
		                            0 );

		_ommBaseImpl.msgDispatched();
		pItem->onAllMsg( _genericMsg );
		pItem->onGenericMsg( _genericMsg );
	}
	break;
	default :
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
			                                       "Internal error. Received unexpected type of RsslRDMDirectoryMsg in DirectoryCallbackClient::processCallback()" );
		break;
	}
	}

	free( rsslMsgBuffer.data );

	return RSSL_RC_CRET_SUCCESS;
}

int DirectoryCallbackClient::allocateAndSetEncodeIteratorBuffer( RsslBuffer* rsslBuffer, UInt32 allocateBufferSize, UInt8 majorVersion, UInt8 minorVersion,
	RsslEncodeIterator* rsslEncodeIterator, const char* methodName )
{
	rsslBuffer->length = allocateBufferSize;

	rsslBuffer->data = (char*)malloc( sizeof(char) * rsslBuffer->length );

	if ( !rsslBuffer->data )
	{
		EmaString text( "Failed to allocate memory in " );
		text.append( methodName );
		_ommBaseImpl.handleMee( text.c_str() );
		return RSSL_RET_FAILURE;
	}

	int retCode = rsslSetEncodeIteratorRWFVersion( rsslEncodeIterator, majorVersion, minorVersion );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString text( "Internal error. Failed to set encode iterator RWF version in " );
		text.append( methodName );
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
				text.c_str() );

		free( rsslBuffer->data );
		return retCode;
	}

	retCode = rsslSetEncodeIteratorBuffer( rsslEncodeIterator, rsslBuffer );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString text( "Internal error. Failed to set encode iterator buffer in " );
		text.append( methodName );
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
				text.c_str() );

		free( rsslBuffer->data );
		return retCode;
	}

	return RSSL_RET_SUCCESS;
}

const EmaString DirectoryItem::_clientName( "DirectoryCallbackClient" );

DirectoryItem::DirectoryItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, const Channel* channel ) :
	ConsumerItem( ommBaseImpl, ommConsClient, closure, 0 ),
	_channel( channel ),
	_pDirectory( 0 )
{
}

DirectoryItem::~DirectoryItem()
{
	_ommBaseImpl.getItemCallbackClient().removeFromList( this );
}

DirectoryItem* DirectoryItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure, const Channel* channel )
{
	DirectoryItem* pItem = 0;
	try
	{
		pItem = new DirectoryItem( ommBaseImpl, ommConsClient, closure, channel );
	}
	catch ( std::bad_alloc ) {}

	if ( !pItem )
		ommBaseImpl.handleMee( "Failed to create DirectoryItem" );

	return pItem;
}

const Directory* DirectoryItem::getDirectory()
{
	return 0;
}

bool DirectoryItem::open( const ReqMsg& reqMsg )
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );

	const Directory* pDirectory = 0;

	if ( reqMsgEncoder.hasServiceName() )
	{
		pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory( reqMsgEncoder.getServiceName() );

		if ( !pDirectory && !_ommBaseImpl.getLoginCallbackClient().getLoginRefresh()->singleOpen )
		{
			EmaString temp( "Service name of '" );
			temp.append( reqMsgEncoder.getServiceName() ).append( "' is not found." );

			scheduleItemClosedStatus(reqMsgEncoder, temp);
			return true;
		}
	}
	else
	{
		if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID )
		{
			pDirectory = _ommBaseImpl.getDirectoryCallbackClient().getDirectory( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId );

			if ( !pDirectory && !_ommBaseImpl.getLoginCallbackClient().getLoginRefresh()->singleOpen)
			{
				EmaString temp( "Service id of '" );
				temp.append( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.serviceId ).
				append( "' is not found." );

				scheduleItemClosedStatus(reqMsgEncoder, temp);
				return true;
			}
		}
	}

	_pDirectory = pDirectory;

	return submit( reqMsgEncoder.getRsslRequestMsg() );
}

bool DirectoryItem::modify( const ReqMsg& reqMsg )
{
	return submit( static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() ).getRsslRequestMsg() );
}

bool DirectoryItem::submit( const PostMsg& )
{
	EmaString temp( "Invalid attempt to submit PostMsg on directory stream. " );
	temp.append( "Instance name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp );

	return false;
}

bool DirectoryItem::submit( const GenericMsg& genMsg )
{
	return submit( static_cast<const GenericMsgEncoder&>( genMsg.getEncoder() ).getRsslGenericMsg() );
}

bool DirectoryItem::close()
{
	bool retCode( true );

	RsslCloseMsg rsslCloseMsg;
	rsslClearCloseMsg( &rsslCloseMsg );

	rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	rsslCloseMsg.msgBase.domainType = _domainType;

	retCode = submit( &rsslCloseMsg );

	remove();
	return retCode;
}

void DirectoryItem::remove()
{
	delete this;
}

bool DirectoryItem::submit( RsslGenericMsg* pRsslGenericMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = ( RsslMsg* )pRsslGenericMsg;

	submitMsgOpts.majorVersion = _channel->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = _channel->getRsslChannel()->minorVersion;

	submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;
	submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _channel->getRsslReactor(),
	                                   _channel->getRsslChannel(),
	                                   &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorSubmitMsg() failed in submit( RsslGenericMsg* )" );
			temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to submit GenericMsg on directory stream. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
		.append( ". Error text: " )
		.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	return true;
}

bool DirectoryItem::submit( RsslRequestMsg* pRsslRequestMsg )
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
		pRsslRequestMsg->flags |= ( RSSL_RQMF_HAS_QOS | RSSL_RQMF_HAS_WORST_QOS );
	}

	pRsslRequestMsg->flags |= _ommBaseImpl.getActiveConfig().msgKeyInUpdates ? RSSL_RQMF_MSG_KEY_IN_UPDATES : 0;
	submitMsgOpts.pRsslMsg = ( RsslMsg* )pRsslRequestMsg;

	RsslBuffer serviceNameBuffer;
	if ( _pDirectory )
	{
		serviceNameBuffer.data = ( char* )_pDirectory->getName().c_str();
		serviceNameBuffer.length = _pDirectory->getName().length();
		submitMsgOpts.pServiceName = &serviceNameBuffer;
	}
	else
	{
		submitMsgOpts.pServiceName = 0;
	}

	submitMsgOpts.majorVersion = _channel->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = _channel->getRsslChannel()->minorVersion;

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
	RsslRet ret = rsslReactorSubmitMsg( _channel->getRsslReactor(), _channel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo );
	if ( ret != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in submit( RsslRequestMsg* )" );
			temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to open or modify directory request. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
		.append( ". Error text: " )
		.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	return true;
}

bool DirectoryItem::submit( RsslCloseMsg* pRsslCloseMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = ( RsslMsg* )pRsslCloseMsg;

	submitMsgOpts.majorVersion = _channel->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = _channel->getRsslChannel()->minorVersion;
	if ( !_streamId )
	{
		if ( !submitMsgOpts.pRsslMsg->msgBase.streamId )
			submitMsgOpts.pRsslMsg->msgBase.streamId = _ommBaseImpl.getItemCallbackClient().getNextStreamId();
		_streamId = submitMsgOpts.pRsslMsg->msgBase.streamId;
	}
	else
		submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( _channel->getRsslReactor(), _channel->getRsslChannel(),
	                                   &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in submit( pRsslCloseMsg* )" );
			temp.append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		EmaString text( "Failed to close directory stream. Reason: " );
		text.append( rsslRetCodeToString( ret ) )
		.append( ". Error text: " )
		.append( rsslErrorInfo.rsslError.text );

		_ommBaseImpl.handleIue( text );

		return false;
	}

	return true;
}

void DirectoryItem::scheduleItemClosedStatus(const ReqMsgEncoder& reqMsgEncoder, const EmaString& text)
{
	if (_closedStatusInfo) return;

	_closedStatusInfo = new ClosedStatusInfo(this, reqMsgEncoder, text);

	new TimeOut(_ommBaseImpl, 1000, ItemCallbackClient::sendItemClosedStatus, _closedStatusInfo, true);
}

