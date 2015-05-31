/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ChannelCallbackClient.h"
#include "DictionaryCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "OmmConsumerClient.h"
#include "OmmConsumerErrorClient.h"
#include "OmmConsumerEvent.h"
#include "OmmConsumerImpl.h"
#include "ReqMsg.h"
#include "ReqMsgEncoder.h"
#include "StaticDecoder.h"
#include "Utilities.h"

#include <new>

using namespace thomsonreuters::ema::access;

const EmaString DirectoryCallbackClient::_clientName( "DirectoryCallbackClient" );

CapabilityList::CapabilityList() :
 _toString(),
 _toStringSet( false )
{
	clear();
}

CapabilityList::~CapabilityList()
{
}

CapabilityList::CapabilityList( const CapabilityList& other ) :
 _toString(),
 _toStringSet( false )
{
	copyAll( other );
}

CapabilityList& CapabilityList::operator=( const CapabilityList& other )
{
	if ( this != &other )
		copyAll( other );

	return *this;
}

CapabilityList& CapabilityList::clear()
{
	_toStringSet = false;

	for ( UInt32 idx = 0; idx < _maxCapability; ++idx )
		_list[idx] = false;

	return *this;
}

void CapabilityList::copyAll( const CapabilityList& other )
{
	_toStringSet = false;

	for ( UInt32 idx = 0; idx < _maxCapability; ++idx )
		_list[idx] = other._list[idx];
}

bool CapabilityList::hasCapability( UInt16 capability )
{ 
	return _list[capability];
}

CapabilityList& CapabilityList::addCapability( UInt16 capability )
{
	_list[capability] = true;
	_toStringSet = false;
	return *this;
}

const EmaString& CapabilityList::toString() const
{
	if ( !_toStringSet )
	{
		_toStringSet = true;

		_toString.set( "'" );

		for ( UInt32 idx = 0; idx < _maxCapability; ++idx )
			if ( _list[idx] ) _toString.append( idx ).append( " " );

		_toString.append( "' " );
	}

	return _toString;
}

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
 _toStringSet( false ),
 _capabilities()
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
	_capabilities = other._capabilities;
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
	_capabilities.clear();
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

const CapabilityList& Info::getCapabilities() const
{
	return _capabilities;
}

Info& Info::addCapability( UInt16 capability )
{
	_capabilities.addCapability( capability );
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

const EmaString& Info::toString() const
{
	if ( !_toStringSet )
	{
		_toStringSet = true;

		_toString.clear();

		// todo ... need impl
	}

	return _toString;
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

Directory* Directory::create( OmmConsumerImpl& ommConsImpl )
{
	Directory* pDirectory = 0;

	try {
		pDirectory = new Directory();
	}
	catch( std::bad_alloc ) {}

	if ( !pDirectory )
	{
		const char* temp = "Failed to create Directory.";
		if ( ommConsImpl.hasOmmConnsumerErrorClient() )
			ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
		else
			throwMeeException( temp );
	}

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
 _toStringSet( false )
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

const EmaString& Directory::toString() const
{
	if ( !_toStringSet )
	{
		_toStringSet = true;

		// todo .... need impl
		_toString.clear();
	}

	return _toString;
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

DirectoryCallbackClient::DirectoryCallbackClient( OmmConsumerImpl& ommConsImpl ) :
 _directoryByIdHt( ommConsImpl.getActiveConfig().serviceCountHint ),
 _directoryByNameHt( ommConsImpl.getActiveConfig().serviceCountHint ),
 _ommConsImpl( ommConsImpl ),
 _event(),
 _refreshMsg(),
 _updateMsg(),
 _statusMsg(),
 _genericMsg()
{    
	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Created DirectoryCallbackClient" );
	}
}

DirectoryCallbackClient::~DirectoryCallbackClient()
{
	Directory* directory = _directoryList.front();

	while ( directory )
	{
		removeDirectory ( directory );
		directory = _directoryList.front();
	}

	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Destroyed DirectoryCallbackClient" );
	}
}

DirectoryCallbackClient* DirectoryCallbackClient::create( OmmConsumerImpl& ommConsImpl )
{
	DirectoryCallbackClient* pClient = 0;

	try {
		pClient = new DirectoryCallbackClient( ommConsImpl );
	}
	catch ( std::bad_alloc ) {}

	if ( !pClient )
	{
		const char* temp = "Failed to create DirectoryCallbackClient";
		if ( OmmLoggerClient::ErrorEnum >= ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( ommConsImpl.hasOmmConnsumerErrorClient() )
			ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
		else
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

	if ( !_ommConsImpl.getActiveConfig().pRsslDirectoryRequestMsg )
	{
		_directoryRequest.filter = (RDM_DIRECTORY_SERVICE_INFO_FILTER
			| RDM_DIRECTORY_SERVICE_STATE_FILTER
			| RDM_DIRECTORY_SERVICE_GROUP_FILTER
			| RDM_DIRECTORY_SERVICE_LOAD_FILTER
			| RDM_DIRECTORY_SERVICE_DATA_FILTER
			| RDM_DIRECTORY_SERVICE_LINK_FILTER);

		_directoryRequest.flags = RDM_DR_RQF_STREAMING;
	}
	else
	{
		if ( _ommConsImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER )
			_directoryRequest.filter = _ommConsImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.filter;
		else
		{			
			if ( _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity <= OmmLoggerClient::WarningEnum )
			{
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum,
					    "Configured source directory request message contains no filter. Will request all filters" );
			}

			_directoryRequest.filter = (RDM_DIRECTORY_SERVICE_INFO_FILTER
				| RDM_DIRECTORY_SERVICE_STATE_FILTER
				| RDM_DIRECTORY_SERVICE_GROUP_FILTER
				| RDM_DIRECTORY_SERVICE_LOAD_FILTER
				| RDM_DIRECTORY_SERVICE_DATA_FILTER
				| RDM_DIRECTORY_SERVICE_LINK_FILTER);
		}

		if ( _ommConsImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID )
		{
			_directoryRequest.serviceId = _ommConsImpl.getActiveConfig().pRsslDirectoryRequestMsg->msgBase.msgKey.serviceId;
			_directoryRequest.flags |= RDM_DR_RQF_HAS_SERVICE_ID;
		}

		if ( !( _ommConsImpl.getActiveConfig().pRsslDirectoryRequestMsg->flags & RSSL_RQMF_STREAMING ) )
		{
			if ( _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity <= OmmLoggerClient::WarningEnum )
			{
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum,
					"Configured source directory request message contains no streaming flag. Will request streaming" );
			}
		}

		_directoryRequest.flags = RDM_DR_RQF_STREAMING;
	}

	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
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

		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
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
		_ommConsImpl.closeChannel( pRsslReactorChannel );

		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			RsslErrorInfo* pError = pEvent->baseMsgEvent.pErrorInfo;

			EmaString temp( "Received event without RDMDirectory message" );
			temp.append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pError->rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( pError->rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( pError->rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( pError->errorLocation ).append( CR )
				.append( "Error Text " ).append( pError->rsslError.text );

			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	if ( pEvent && pEvent->baseMsgEvent.pStreamInfo && pEvent->baseMsgEvent.pStreamInfo->pUserSpec )
	{
		SingleItem* pItem = (SingleItem*)pEvent->baseMsgEvent.pStreamInfo->pUserSpec;

		return processCallback( pRsslReactor, pRsslReactorChannel, pEvent, pItem );
	}

	switch ( pDirectoryMsg->rdmMsgBase.rdmMsgType )
	{
	case RDM_DR_MT_REFRESH:
	{
		RsslState* pState = &pDirectoryMsg->refresh.state;

		if ( pState->streamState != RSSL_STREAM_OPEN )
		{
			_ommConsImpl.closeChannel( pRsslReactorChannel );

			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );

				EmaString temp( "RDMDirectory stream was closed with refresh message " );
				temp.append( tempState );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}

			processDirectoryPayload( pDirectoryMsg->refresh.serviceCount, pDirectoryMsg->refresh.serviceList, pRsslReactorChannel->userSpecPtr );

			break;
		}
		else if ( pState->dataState == RSSL_DATA_SUSPECT )
		{
			if ( OmmLoggerClient::WarningEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );

				EmaString temp( "RDMDirectory stream state was changed to suspect with refresh message " );
				temp.append( tempState );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
			}

			_ommConsImpl.setState( OmmConsumerImpl::DirectoryStreamOpenSuspectEnum );

			processDirectoryPayload( pDirectoryMsg->refresh.serviceCount, pDirectoryMsg->refresh.serviceList, pRsslReactorChannel->userSpecPtr );
			break;
		}

		_ommConsImpl.setState( OmmConsumerImpl::DirectoryStreamOpenOkEnum );

		if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString tempState( 0, 256 );
			stateToString( pState, tempState );

			EmaString temp( "RDMDirectory stream was open with refresh message " );
			temp.append( tempState );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
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
				_ommConsImpl.closeChannel( pRsslReactorChannel );

				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );

					EmaString temp( "RDMDirectory stream was closed with status message " );
					temp.append( tempState );
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}

				return RSSL_RC_CRET_SUCCESS;
			}
			else if ( pState->dataState == RSSL_DATA_SUSPECT )
			{
				if ( OmmLoggerClient::WarningEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );

					EmaString temp( "RDMDirectory stream state was changed to suspect with status message " );
					temp.append( tempState );
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
				}

				_ommConsImpl.setState( OmmConsumerImpl::DirectoryStreamOpenSuspectEnum );
				break;
			}

			if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );

				EmaString temp( "RDMDirectory stream was open with status message " );
				temp.append( tempState );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}

			_ommConsImpl.setState( OmmConsumerImpl::DirectoryStreamOpenOkEnum );
		}
		else
		{
			if ( OmmLoggerClient::WarningEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, "Received RDMDirectory status message without the state" );
			}
		}
		break;
	}
	case RDM_DR_MT_UPDATE:
	{
		if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Received RDMDirectory update message" );
		}

		processDirectoryPayload( pDirectoryMsg->update.serviceCount, pDirectoryMsg->update.serviceList,pRsslReactorChannel->userSpecPtr );
		break;
	}
	default:
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received unknown RDMDirectory message type" );
			temp.append( CR )
				.append( "message type value " )
				.append( pDirectoryMsg->rdmMsgBase.rdmMsgType );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}
		break;
	}
	}
	return RSSL_RC_CRET_SUCCESS;
}

void DirectoryCallbackClient::processDirectoryPayload( UInt32 count, RsslRDMService* pServiceList, void* userSpecPtr )
{
	if ( !pServiceList && count )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Received RDMDirectory message indicating a number of services but without a service list" );
		}
		return;
	}

	if ( !userSpecPtr )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Internal error: no RsslReactorChannel->userSpecPtr" );
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
				pDirectory = Directory::create( _ommConsImpl );
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

				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Received RsslRDMService with Add action but no Service Info" );
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
			}
			else 
			{
				pDirectory->setName( pServiceList[jdx].info.serviceName.data, pServiceList[jdx].info.serviceName.length );

				pDirectory->setId( pServiceList[jdx].serviceId );

				pDirectory->setChannel( (Channel*)userSpecPtr );

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
				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Received Update action for unknown RsslRDMService with service id " );
					temp.append( pServiceList[jdx].serviceId );
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
				break;
			}
			else if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received Update action for RsslRDMService" );
				temp.append( CR )
					.append( "Service name " ).append( (*pDirectoryPtr)->getName() ).append( CR )
					.append( "Service id " ).append( pServiceList[ jdx ].serviceId );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}

			if ( pServiceList[jdx].flags & RDM_SVCF_HAS_INFO )
			{
				Info info;

				info.setName( pServiceList[jdx].info.serviceName.data, pServiceList[jdx].info.serviceName.length );

				if ( info.getName() != (*pDirectoryPtr)->getName() )
				{
					if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString temp( "Received Update action for RsslRDMService" );
						temp.append( CR )
							.append( "Service name " ).append( (*pDirectoryPtr)->getName() ).append( CR )
							.append( "Service id " ).append( pServiceList[jdx].serviceId ).append( CR )
							.append( "attempting to change service name to " ).append( info.getName() );
						_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
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

				(*pDirectoryPtr)->setInfo( info );
			}

			if ( pServiceList[jdx].flags & RDM_SVCF_HAS_STATE )
			{
				State state;

				state.setServiceState( pServiceList[jdx].state.serviceState );

				if ( pServiceList[jdx].state.flags & RDM_SVC_STF_HAS_ACCEPTING_REQS )
					state.setAcceptingRequests( pServiceList[jdx].state.acceptingRequests );

				(*pDirectoryPtr)->setState( state );
			}

			break;
		}
		case RSSL_MPEA_DELETE_ENTRY :
		{
			DirectoryPtr* pDirectoryPtr = _directoryByIdHt.find( pServiceList[jdx].serviceId );

			if ( !pDirectoryPtr )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Received Delete action for unknown RsslRDMService with service id " );
					temp.append( pServiceList[jdx].serviceId );
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
				break;
			}
			else if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received Delete action for RsslRDMService" );
				temp.append( CR )
					.append( "Service name " ).append( (*pDirectoryPtr)->getName() ).append( CR )
					.append( "Service id " ).append( pServiceList[jdx].serviceId );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}

			(*pDirectoryPtr)->markDeleted();

			break;
		}
		default :
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received unknown action for RsslRDMService. Action value " );
				temp.append( pServiceList[jdx].action );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
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
	return x == y;
}

size_t DirectoryCallbackClient::EmaStringPtrHasher::operator()( const EmaStringPtr& value ) const
{
	size_t result = 0;
	size_t magic = 8388593;

	const char *s = value->c_str();
	UInt32 n = value->length();
	while (n--)
		result = ((result % magic) << 8) + (size_t) *s++;
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

	if ( pDirectory->getState().getAcceptingRequests() == 1 &&
		pDirectory->getState().getServiceState() == 1 )
		_ommConsImpl.getDictionaryCallbackClient().downloadDictionary( *pDirectory );
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

RsslReactorCallbackRet DirectoryCallbackClient::processCallback( RsslReactor* pRsslReactor,
													RsslReactorChannel* pRsslReactorChannel,
													RsslRDMDirectoryMsgEvent* pEvent,
													SingleItem* pItem )
{
	RsslBuffer rsslMsgBuffer;
	rsslMsgBuffer.length = 4096;
	rsslMsgBuffer.data = (char*)malloc( sizeof( char ) * rsslMsgBuffer.length );

	if ( !rsslMsgBuffer.data )
	{
		const char* temp = "Failed to allocate memory in DirectoryCallbackClient::processCallback()";
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
			_ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
		else
			throwMeeException( temp );

		return RSSL_RC_CRET_SUCCESS;
	}

	RsslEncIterator eIter;
	rsslClearEncodeIterator( &eIter );

	RsslRet retCode = rsslSetEncodeIteratorRWFVersion( &eIter, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		free( rsslMsgBuffer.data );

		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
			"Internal error. Failed to set encode iterator version in DirectoryCallbackClient::processCallback()" );
		return RSSL_RC_CRET_SUCCESS;
	}

	retCode = rsslSetEncodeIteratorBuffer( &eIter, &rsslMsgBuffer );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		free( rsslMsgBuffer.data );

		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
			"Internal error. Failed to set encode iterator buffer in DirectoryCallbackClient::processCallback()" );
		return RSSL_RC_CRET_SUCCESS;
	}

	RsslErrorInfo rsslErrorInfo;
	retCode = rsslEncodeRDMDirectoryMsg( &eIter, pEvent->pRDMDirectoryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		free( rsslMsgBuffer.data );

		rsslMsgBuffer.length += rsslMsgBuffer.length;
		rsslMsgBuffer.data = (char*)malloc( sizeof( char ) * rsslMsgBuffer.length );

		if ( !rsslMsgBuffer.data )
		{
			const char* temp = "Failed to allocate memory in DirectoryCallbackClient::processCallback()";
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

			if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
				_ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
			else
				throwMeeException( temp );

			return RSSL_RC_CRET_SUCCESS;
		}

		retCode = rsslEncodeRDMDirectoryMsg( &eIter, pEvent->pRDMDirectoryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );
	}

	if ( retCode != RSSL_RET_SUCCESS )
	{
		free( rsslMsgBuffer.data );

		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: failed to encode RsslRDMDirectoryMsg in DirectoryCallbackClient::processCallback()" );
			temp.append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
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

			_event._pItem = pItem;
			_event._pItem->getClient().onAllMsg( _refreshMsg, _event );
			_event._pItem->getClient().onRefreshMsg( _refreshMsg, _event );
		}
		break;
	case RDM_DR_MT_UPDATE :
		{
			StaticDecoder::setRsslData( &_updateMsg, &rsslMsgBuffer, RSSL_DT_MSG,
										pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
										0 );

			_event._pItem = pItem;
			_event._pItem->getClient().onAllMsg( _updateMsg, _event );
			_event._pItem->getClient().onUpdateMsg( _updateMsg, _event );
		}
		break;
	case RDM_DR_MT_STATUS :
		{
			StaticDecoder::setRsslData( &_statusMsg, &rsslMsgBuffer, RSSL_DT_MSG,
										pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
										0 );

			_event._pItem = pItem;
			_event._pItem->getClient().onAllMsg( _statusMsg, _event );
			_event._pItem->getClient().onStatusMsg( _statusMsg, _event );
		}
		break;
	case RDM_DR_MT_CONSUMER_STATUS :
		{
			StaticDecoder::setRsslData( &_genericMsg, &rsslMsgBuffer, RSSL_DT_MSG,
										pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion,
										0 );

			_event._pItem = pItem;
			_event._pItem->getClient().onAllMsg( _genericMsg, _event );
			_event._pItem->getClient().onGenericMsg( _genericMsg, _event );
		}
		break;
	default :
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
				"Internal error. Received unexpected type of RsslRDMDirectoryMsg in DirectoryCallbackClient::processCallback()" );
			break;
		}
	}

	free( rsslMsgBuffer.data );

	return RSSL_RC_CRET_SUCCESS;
}

const EmaString DirectoryItem::_clientName( "DirectoryCallbackClient" );

DirectoryItem::DirectoryItem( OmmConsumerImpl& ommConsImpl,  OmmConsumerClient& ommConsClient, void* closure, const Channel * channel ) :
	Item( ommConsImpl, ommConsClient, closure ), channel( channel )
{
	_itemType = DirectoryItemEnum;
}

DirectoryItem* DirectoryItem::create( OmmConsumerImpl& ommConsImpl, OmmConsumerClient& ommConsClient, void* closure, const Channel * channel )
{
	DirectoryItem * pItem;
	try {
		pItem = new DirectoryItem( ommConsImpl, ommConsClient, closure, channel );
	}
	catch( std::bad_alloc ) {}

	if ( !pItem )
	{
		const char* temp = "Failed to create DirectoryItem";
		if ( OmmLoggerClient::ErrorEnum >= ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( ommConsImpl.hasOmmConnsumerErrorClient() )
			ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
		else
			throwMeeException( temp );
	}

	return pItem;
}

bool DirectoryItem::open( const ReqMsg& reqMsg )
{	
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );
	hasServiceName = reqMsgEncoder.hasServiceName();
	return submit( reqMsgEncoder.getRsslRequestMsg() );
}

bool DirectoryItem::modify( const ReqMsg& )
{
	return false;
}

bool DirectoryItem::submit( const PostMsg& )
{
	return false;
}

bool DirectoryItem::submit( const GenericMsg& )
{
	return false;
}

bool DirectoryItem::close()
{
	bool retCode(true);

	if ( hasServiceName )
	{
		RsslCloseMsg rsslCloseMsg;
		rsslClearCloseMsg( &rsslCloseMsg );

		rsslCloseMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		rsslCloseMsg.msgBase.domainType = _domainType;

		retCode = submit( &rsslCloseMsg );
	}

	remove();
	return retCode;
}

void DirectoryItem::remove()
{
	delete this;
}

bool DirectoryItem::submit( RsslGenericMsg* )
{
	return false;
}

bool DirectoryItem::submit( RsslRequestMsg* pRsslRequestMsg)
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

	submitMsgOpts.pServiceName = 0;

	submitMsgOpts.majorVersion = channel->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = channel->getRsslChannel()->minorVersion;

	submitMsgOpts.requestMsgOptions.pUserSpec = (void*)this;

	if ( !_streamId )
	{
		if ( !submitMsgOpts.pRsslMsg->msgBase.streamId )
			submitMsgOpts.pRsslMsg->msgBase.streamId = const_cast< Channel* >(channel)->getNextStreamId();
		_streamId = submitMsgOpts.pRsslMsg->msgBase.streamId;
	}
	else
		submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	if ( !_domainType )
		_domainType = submitMsgOpts.pRsslMsg->msgBase.domainType;
	else
		submitMsgOpts.pRsslMsg->msgBase.domainType = _domainType;

	RsslErrorInfo rsslErrorInfo;
	RsslRet ret = rsslReactorSubmitMsg( channel->getRsslReactor(), channel->getRsslChannel(), &submitMsgOpts, &rsslErrorInfo );
	if ( ret != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in submit( RsslRequestMsg* )" );
			temp.append( CR )
				.append( "Channel " ).append( channel->toString() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );

			return false;
		}
	}

	return true;
}

bool DirectoryItem::submit( RsslCloseMsg* pRsslCloseMsg )
{
	RsslReactorSubmitMsgOptions submitMsgOpts;
	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );

	submitMsgOpts.pRsslMsg = (RsslMsg*)pRsslCloseMsg;

	submitMsgOpts.majorVersion = channel->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = channel->getRsslChannel()->minorVersion;
	if ( !_streamId )
	{
		if ( !submitMsgOpts.pRsslMsg->msgBase.streamId )
			submitMsgOpts.pRsslMsg->msgBase.streamId = const_cast< Channel* >(channel)->getNextStreamId();
		_streamId = submitMsgOpts.pRsslMsg->msgBase.streamId;
	}
	else
		submitMsgOpts.pRsslMsg->msgBase.streamId = _streamId;

	RsslErrorInfo rsslErrorInfo;
	RsslRet ret;
	if ( ( ret = rsslReactorSubmitMsg( channel->getRsslReactor(), channel->getRsslChannel(),
									   &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed in submit( pRsslCloseMsg* )" );
			temp.append( CR )
				.append( "Channel" ).append( channel->toString() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return false;
	}

	return true;
}
