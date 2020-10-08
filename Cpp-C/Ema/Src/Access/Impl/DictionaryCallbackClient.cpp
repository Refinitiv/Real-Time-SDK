/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumerImpl.h"
#include "OmmServerBaseImpl.h"
#include "ChannelCallbackClient.h"
#include "DictionaryCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "Utilities.h"
#include "ReqMsg.h"
#include "ReqMsgEncoder.h"
#include "StaticDecoder.h"
#include "Decoder.h"
#include "OmmNiProviderImpl.h"
#include "OmmIProviderImpl.h"
#include "EmaRdm.h"
#include "OmmInvalidUsageException.h"

#include <new>

#define MAX_DICTIONARY_BUFFER_SIZE 448000
#define DEFAULT_DICTIONARY_RESP_HEADER_SIZE 20480

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

const EmaString DictionaryCallbackClient::_clientName( "DictionaryCallbackClient" );
const EmaString DictionaryCallbackClient::_rwfFldName("RWFFld");
const EmaString DictionaryCallbackClient::_rwfEnumName("RWFEnum");
const EmaString LocalDictionary::_clientName( "LocalDictionary" );
const EmaString ChannelDictionary::_clientName( "ChannelDictionary" );
const EmaString DictionaryItem::_clientName( "DictionaryItem" );
const EmaString NiProviderDictionaryItem::_clientName("NiProviderDictionaryItem");
const EmaString IProviderDictionaryItem::_clientName("IProviderDictionaryItem");

Dictionary::Dictionary() :
	_fldStreamId( 0 ),
	_enumStreamId( 0 )
{
}

Dictionary::~Dictionary()
{
}

Int32 Dictionary::getEnumStreamId() const
{
	return _enumStreamId;
}

Int32 Dictionary::getFldStreamId() const
{
	return _fldStreamId;
}

LocalDictionary* LocalDictionary::create( OmmCommonImpl& ommBaseImpl, BaseConfig& baseConfig )
{
	LocalDictionary* pDictionary = 0;

	try
	{
		pDictionary = new LocalDictionary( ommBaseImpl, baseConfig );
	}
	catch ( std::bad_alloc& ) {}

	if ( !pDictionary )
		ommBaseImpl.handleMee( "Failed to create LocalDictionary" );

	return pDictionary;
}

void LocalDictionary::destroy( LocalDictionary*& pDictionary )
{
	if ( pDictionary )
	{
		delete pDictionary;
		pDictionary = 0;
	}
}

Dictionary::DictionaryType LocalDictionary::getType() const
{
	return FileDictionaryEnum;
}

LocalDictionary::LocalDictionary( OmmCommonImpl& ommCommonImpl, BaseConfig& baseConfig ) :
	_ommCommonImpl(ommCommonImpl),
	_baseConfig(baseConfig),
	_rsslDictionary(),
	_isLoaded( false )
{
	rsslClearDataDictionary( &_rsslDictionary );

	_fldStreamId = 3;
	_enumStreamId = 4;
}

LocalDictionary::~LocalDictionary()
{
	rsslDeleteDataDictionary( &_rsslDictionary );
}

const RsslDataDictionary* LocalDictionary::getRsslDictionary() const
{
	return &_rsslDictionary;
}

bool LocalDictionary::isLoaded() const
{
	return _isLoaded;
}

bool LocalDictionary::load( const EmaString& fldName, const EmaString& enumName, RsslRet& retCode )
{
	char errTxt[256];
	RsslBuffer buffer;
	buffer.data = errTxt;
	buffer.length = 255;

	if ( ( retCode = rsslLoadFieldDictionary( fldName.c_str(), &_rsslDictionary, &buffer ) ) < 0 )
	{
		_isLoaded = false;

		rsslDeleteDataDictionary( &_rsslDictionary );

		if (OmmLoggerClient::ErrorEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString errorText, dir;
			getCurrentDir( dir );
			errorText.set( "Unable to load RDMFieldDictionary from file named " ).append( fldName ).append( CR )
			.append( "Current working directory " ).append( dir ).append( CR )
			.append( "Error text " ).append( errTxt );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
		}
		return false;
	}

	if ( ( retCode = rsslLoadEnumTypeDictionary( enumName.c_str(), &_rsslDictionary, &buffer ) ) < 0 )
	{
		_isLoaded = false;

		rsslDeleteDataDictionary( &_rsslDictionary );

		if (OmmLoggerClient::ErrorEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
		{
			EmaString errorText, dir;
			getCurrentDir( dir );
			errorText.set( "Unable to load EnumTypeDef from file named " ).append(enumName).append( CR )
			.append( "Current working directory " ).append( dir ).append( CR )
			.append( "Error text " ).append( errTxt );
			_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum, errorText);
		}
		return false;
	}

	if (OmmLoggerClient::VerboseEnum >= _baseConfig.loggerConfig.minLoggerSeverity)
	{
		EmaString temp( "Successfully loaded local dictionaries: " );
		temp.append( CR )
		.append( "RDMFieldDictionary file named " ).append( fldName ).append( CR )
		.append( "EnumTypeDef file named " ).append( enumName );
		_ommCommonImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::VerboseEnum, temp);
	}

	_isLoaded = true;

	return true;
}

ChannelDictionary* ChannelDictionary::create( OmmBaseImpl& ommBaseImpl )
{
	ChannelDictionary* pDictionary = 0;

	try
	{
		pDictionary = new ChannelDictionary( ommBaseImpl );
	}
	catch ( std::bad_alloc& ) {}

	if ( !pDictionary )
		ommBaseImpl.handleMee( "Failed to create ChannelDictionary" );

	return pDictionary;
}

void ChannelDictionary::destroy( ChannelDictionary*& pDictionary )
{
	if ( pDictionary )
	{
		delete pDictionary;
		pDictionary = 0;
	}
}

Dictionary::DictionaryType ChannelDictionary::getType() const
{
	return Dictionary::ChannelDictionaryEnum;
}

ChannelDictionary::ChannelDictionary( OmmBaseImpl& ommBaseImpl ) :
	_ommBaseImpl( ommBaseImpl ),
	_pChannel( 0 ),
	_rsslDictionary(),
	_isFldLoaded( false ),
	_isEnumLoaded( false ),
	_pListenerList( 0 ),
	_channelDictLock()
{
}

const RsslDataDictionary* ChannelDictionary::getRsslDictionary() const
{
	return &_rsslDictionary;
}

ChannelDictionary::~ChannelDictionary()
{
	rsslDeleteDataDictionary( &_rsslDictionary );

	if ( _pListenerList )
	{
		delete _pListenerList;
		_pListenerList = 0;
	}
}

Channel* ChannelDictionary::getChannel() const
{
	return _pChannel;
}

ChannelDictionary& ChannelDictionary::setChannel( Channel* pChannel )
{
	_pChannel = pChannel;
	return *this;
}

bool ChannelDictionary::isLoaded() const
{
	return _isEnumLoaded && _isFldLoaded;
}

RsslReactorCallbackRet ChannelDictionary::processCallback( RsslReactor*,
    RsslReactorChannel* pRsslReactorChannel,
    RsslRDMDictionaryMsgEvent* pEvent )
{
	RsslRDMDictionaryMsg* pDictionaryMsg = pEvent->pRDMDictionaryMsg;

	if ( !pDictionaryMsg )
	{
		_ommBaseImpl.closeChannel( pRsslReactorChannel );

		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			RsslErrorInfo* pError = pEvent->baseMsgEvent.pErrorInfo;

			EmaString temp( "Received event without RDMDictionary message" );
			temp.append( CR ).append( "Channel " ).append( CR )
			.append( static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( pError->rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( pError->rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( pError->rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( pError->errorLocation ).append( CR )
			.append( "Error Text " ).append( pError->rsslError.rsslErrorId ? pError->rsslError.text : "" );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		return RSSL_RC_CRET_SUCCESS;
	}

	switch ( pDictionaryMsg->rdmMsgBase.rdmMsgType )
	{
	case RDM_DC_MT_REFRESH:
	{
		RsslRDMDictionaryRefresh* pRefresh = &pDictionaryMsg->refresh;

		RsslState* pState = &pDictionaryMsg->refresh.state;

		if ( pState->streamState != RSSL_STREAM_OPEN )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );

				EmaString temp( "RDMDictionary stream was closed with refresh message" );
				temp.append( CR )
				.append( "Channel " ).append( CR )
				.append( static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->toString() ).append( CR )
				.append( "Reason " ).append( tempState );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}
			break;
		}
		else if ( pState->dataState == RSSL_DATA_SUSPECT )
		{
			if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );

				EmaString temp( "RDMDictionary stream state was changed to suspect with refresh message" );
				temp.append( CR ).append( "Channel " ).append( CR )
				.append( static_cast<Channel*>( pRsslReactorChannel->userSpecPtr )->toString() ).append( CR )
				.append( "Reason " ).append( tempState );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp.trimWhitespace() );
			}
			break;
		}

		if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString name( pRefresh->dictionaryName.data, pRefresh->dictionaryName.length );
			EmaString temp( "Received RDMDictionary refresh message" );
			temp.append( CR )
			.append( "Dictionary name " ).append( name ).append( CR )
			.append( "streamId " ).append( pRefresh->rdmMsgBase.streamId );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
		}

		if ( pRefresh->flags & RDM_DC_RFF_HAS_INFO )
		{
			switch ( pRefresh->type )
			{
			case RDM_DICTIONARY_FIELD_DEFINITIONS :
			{
				if ( !_fldStreamId )
					_fldStreamId = pRefresh->rdmMsgBase.streamId;
				else if ( _fldStreamId != pRefresh->rdmMsgBase.streamId )
				{
					if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString temp( "Received RDMDictionary refresh message with FieldDefinitions but changed streamId" );
						temp.append( CR )
						.append( "Initial streamId " ).append( _fldStreamId ).append( CR )
						.append( "New streamId " ).append( pRefresh->rdmMsgBase.streamId );
						_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
					}
					return RSSL_RC_CRET_SUCCESS;
				}
				break;
			}
			case RDM_DICTIONARY_ENUM_TABLES :
			{
				if ( !_enumStreamId )
					_enumStreamId = pRefresh->rdmMsgBase.streamId;
				else if ( _enumStreamId != pRefresh->rdmMsgBase.streamId )
				{
					if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString temp( "Received RDMDictionary refresh message with EnumTables but changed streamId" );
						temp.append( CR )
						.append( "Initial streamId " ).append( _enumStreamId ).append( CR )
						.append( "New streamId " ).append( pRefresh->rdmMsgBase.streamId );
						_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
					}
					return RSSL_RC_CRET_SUCCESS;
				}
				break;
			}
			default:
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Received RDMDictionary message with unknown dictionary type" );
					temp.append( CR )
					.append( "Dictionary type " ).append( pRefresh->type );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
				return RSSL_RC_CRET_SUCCESS;
			}
			}
		}

		RsslDecodeIterator dIter;
		rsslClearDecodeIterator( &dIter );

		if ( RSSL_RET_SUCCESS != rsslSetDecodeIteratorRWFVersion( &dIter, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion ) )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error: failed to set RWF Version while decoding dictionary" );
				temp.append( CR )
				.append( "Trying to set " )
				.append( pRsslReactorChannel->majorVersion ).append( "." ).append( pRsslReactorChannel->minorVersion );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}
			return RSSL_RC_CRET_SUCCESS;
		}

		if ( RSSL_RET_SUCCESS != rsslSetDecodeIteratorBuffer( &dIter, &pRefresh->dataBody ) )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Internal error: failed to set iterator buffer while decoding dictionary" );
			}
			return RSSL_RC_CRET_SUCCESS;
		}

		if ( _fldStreamId == pRefresh->rdmMsgBase.streamId )
		{
			char errTxt[256];
			RsslBuffer errorText;
			errorText.length = 255;
			errorText.data = errTxt;

			if ( rsslDecodeFieldDictionary( &dIter, &_rsslDictionary, RDM_DICTIONARY_VERBOSE, &errorText ) == RSSL_RET_SUCCESS )
			{
				if ( pRefresh->flags & RDM_DC_RFF_IS_COMPLETE )
				{
					_isFldLoaded = true;

					if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString name( pRefresh->dictionaryName.data, pRefresh->dictionaryName.length );
						EmaString temp( "Received RDMDictionary refresh complete message" );
						temp.append( CR )
						.append( "dictionary name " ).append( name ).append( CR )
						.append( "streamId " ).append( pRefresh->rdmMsgBase.streamId );
						_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
					}
				}
				else
					_isFldLoaded = false;
			}
			else
			{
				_isFldLoaded = false;

				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Internal error: failed to decode FieldDictionary" );
					temp.append( CR )
					.append( "Error text " ).append( errTxt );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
				return RSSL_RC_CRET_SUCCESS;
			}
		}
		else if ( _enumStreamId == pRefresh->rdmMsgBase.streamId )
		{
			char errTxt[256];
			RsslBuffer errorText;
			errorText.length = 255;
			errorText.data = errTxt;

			if ( rsslDecodeEnumTypeDictionary( &dIter, &_rsslDictionary, RDM_DICTIONARY_VERBOSE, &errorText ) == RSSL_RET_SUCCESS )
			{
				if ( pRefresh->flags & RDM_DC_RFF_IS_COMPLETE )
				{
					_isEnumLoaded = true;

					if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString name( pRefresh->dictionaryName.data, pRefresh->dictionaryName.length );
						EmaString temp( "Received RDMDictionary refresh complete message" );
						temp.append( CR )
						.append( "dictionary name " ).append( name ).append( CR )
						.append( "streamId " ).append( pRefresh->rdmMsgBase.streamId );
						_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
					}
				}
				else
					_isEnumLoaded = false;
			}
			else
			{
				_isEnumLoaded = false;

				if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Internal error: failed to decode EnumTable dictionary" );
					temp.append( CR )
					.append( "error text " ).append( errTxt );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
				return RSSL_RC_CRET_SUCCESS;
			}
		}
		else
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received unexpected RDMDictionary refresh message on streamId " );
				temp.append( pRefresh->rdmMsgBase.streamId );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}
			return RSSL_RC_CRET_SUCCESS;
		}

		break;
	}
	case RDM_DC_MT_STATUS:
	{
		if ( pDictionaryMsg->status.flags & RDM_DC_STF_HAS_STATE )
		{
			RsslState* pState = &pDictionaryMsg->status.state;

			if ( pState->streamState != RSSL_STREAM_OPEN )
			{
				if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );

					EmaString temp( "RDMDictionary stream was closed with status message" );
					temp.append( CR )
					.append( "streamId " ).append( pDictionaryMsg->status.rdmMsgBase.streamId ).append( CR )
					.append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
				}

				notifyStatusToListener( pDictionaryMsg->status );
				break;
			}
			else if ( pState->dataState == RSSL_DATA_SUSPECT )
			{
				if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );

					EmaString temp( "RDMDictionary stream state was changed to suspect with status message" );
					temp.append( CR )
					.append( "streamId " ).append( pDictionaryMsg->status.rdmMsgBase.streamId ).append( CR )
					.append( tempState );
					_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
				}

				notifyStatusToListener( pDictionaryMsg->status );
				break;
			}

			if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );

				EmaString temp( "RDMDictionary stream was open with status message" );
				temp.append( CR )
				.append( "streamId " )
				.append( pDictionaryMsg->rdmMsgBase.streamId ).append( CR )
				.append( tempState );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
		}
		else
		{
			if ( OmmLoggerClient::WarningEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received RDMDictionary status message without the state" );
				temp.append( CR )
				.append( "streamId " )
				.append( pDictionaryMsg->status.rdmMsgBase.streamId );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
			}
		}
		break;
	}
	default:
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received unknown RDMDictionary message type" );
			temp.append( CR )
			.append( "message type " ).append( pDictionaryMsg->rdmMsgBase.rdmMsgType ).append( CR )
			.append( "streamId " ).append( pDictionaryMsg->rdmMsgBase.streamId );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
		}
		break;
	}
	}

	return RSSL_RC_CRET_SUCCESS;
}

void ChannelDictionary::notifyStatusToListener( const RsslRDMDictionaryStatus& status )
{
	_channelDictLock.lock();

	if ( !_pListenerList || _pListenerList->size() == 0 )
	{
		_channelDictLock.unlock();
		return;
	}

	DictionaryItem* dictItem = 0;
	RsslStatusMsg statusMsg;
	rsslClearStatusMsg( &statusMsg );
	RsslEncodeIterator encodeIter;
	RsslBuffer msgBuf;

	for ( UInt32 index = 0 ; index < _pListenerList->size(); index++ )
	{
		dictItem = _pListenerList->operator[]( index );

		if ( dictItem->getStreamId() != status.rdmMsgBase.streamId )
			continue;

		rsslClearEncodeIterator( &encodeIter );
		statusMsg.msgBase.msgClass = RSSL_MC_STATUS;
		statusMsg.msgBase.streamId = dictItem->getStreamId();
		statusMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
		statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		statusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY;
		statusMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
		statusMsg.msgBase.msgKey.name.data = (char*)dictItem->getName().c_str();
		statusMsg.msgBase.msgKey.name.length = dictItem->getName().length();
		statusMsg.state.streamState = status.state.streamState;
		statusMsg.state.dataState = status.state.dataState;
		statusMsg.state.code = status.state.code;
		statusMsg.state.text.data = status.state.text.data;
		statusMsg.state.text.length = status.state.text.length;

		dictItem->getImpl().getDictionaryCallbackClient().encodeAndNotifyStatusMsg(dictItem, &statusMsg, &msgBuf, DEFAULT_DICTIONARY_RESP_HEADER_SIZE, 
			RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &encodeIter, "ChannelDictionary::notifyStatusToListener");
	
		if ( status.state.streamState != RSSL_STREAM_OPEN )
			_pListenerList->removePosition( index );

		break;
	}

	_channelDictLock.unlock();
}

void ChannelDictionary::acquireLock()
{
	_channelDictLock.lock();
}

void ChannelDictionary::releaseLock()
{
	_channelDictLock.unlock();
}

void ChannelDictionary::addListener( DictionaryItem* item )
{
	if ( _pListenerList == 0 )
	{
		_pListenerList = new EmaVector<DictionaryItem*>( 2 );
	}

	_pListenerList->push_back( item );
}

void ChannelDictionary::removeListener( DictionaryItem* item )
{
	if ( _pListenerList == 0 )
	{
		return;
	}

	_pListenerList->removeValue( item );
}

const EmaVector<DictionaryItem*>* ChannelDictionary::getListenerList() const
{
	return _pListenerList;
}

DictionaryCallbackClient::DictionaryCallbackClient( OmmBaseImpl& ommConsImpl ) :
	_channelDictionaryList(),
	_localDictionary( 0 ),
	_channelDictionary( 0 ),
	_ommBaseImpl( ommConsImpl ),
	_refreshMsg(),
	_statusMsg()
{
	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Created DictionaryCallbackClient" );
	}
}

DictionaryCallbackClient::~DictionaryCallbackClient()
{
	ChannelDictionary* dictionary = _channelDictionaryList.pop_back();
	while ( dictionary )
	{
		if ( _channelDictionary == dictionary )
		{
			dictionary = _channelDictionaryList.pop_back();
			continue;
		}

		ChannelDictionary::destroy( dictionary );
		dictionary = _channelDictionaryList.pop_back();
	}

	LocalDictionary::destroy( _localDictionary );

	ChannelDictionary::destroy( _channelDictionary );

	if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Destroyed DictionaryCallbackClient" );
	}
}

DictionaryCallbackClient* DictionaryCallbackClient::create( OmmBaseImpl& ommBaseImpl )
{
	DictionaryCallbackClient* pClient = 0;

	try
	{
		pClient = new DictionaryCallbackClient( ommBaseImpl );
	}
	catch ( std::bad_alloc& ) {}

	if ( !pClient )
		ommBaseImpl.handleMee( "Failed to create DictionaryCallbackClient" );

	return pClient;
}

void DictionaryCallbackClient::destroy( DictionaryCallbackClient*& pClient )
{
	if ( pClient )
	{
		delete pClient;
		pClient = 0;
	}
}

void DictionaryCallbackClient::initialize()
{
	if ( _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg && _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg )
	{
		if ( ( _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg->get()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) &&
		     ( _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg->get()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID )
		   )
		{
			if ( ( _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg->get()->msgBase.msgKey.serviceId !=
			       _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg->get()->msgBase.msgKey.serviceId ) )
			{
				EmaString temp( "Invalid dictionary configuration was specified through the OmmConsumerConfig::addAdminMsg()" );
				temp.append( CR ).append( "Invalid attempt to download dictionaries from different service IDs." );

				_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
				return;
			}
		}
		else if ( _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg->hasServiceName() &&
		          _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg->hasServiceName() )
		{
			if ( ( _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg->getServiceName() !=
			       _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg->getServiceName() ) )
			{
				EmaString temp( "Invalid dictionary configuration was specified through the OmmConsumerConfig::addAdminMsg()" );
				temp.append( CR ).append( "Invalid attempt to download dictionaries from different service names." );

				_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
				return;
			}
		}
		else
		{
			EmaString temp( "Invalid dictionary configuration was specified through the OmmConsumerConfig::addAdminMsg()" );
			temp.append( CR ).append( "Invalid attempt to download dictionaries from different services." );

			_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
			return;
		}

		_ommBaseImpl.getActiveConfig().dictionaryConfig.dictionaryType = Dictionary::ChannelDictionaryEnum;
	}
	else if ( _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg && !_ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg )
	{
		EmaString temp( "Invalid dictionary configuration was specified through the OmmConsumerConfig::addAdminMsg()" );
		temp.append( CR ).append( "Enumeration type definition request message was not populated." );

		_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}
	else if ( !_ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg && _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg )
	{
		EmaString temp( "Invalid dictionary configuration was specified through the OmmConsumerConfig::addAdminMsg()" );
		temp.append( CR ).append( "RDM Field Dictionary request message was not populated." );

		_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	if (_ommBaseImpl.getActiveConfig().dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum)
		_localDictionary = LocalDictionary::create(_ommBaseImpl, _ommBaseImpl.getActiveConfig());
	else
		_channelDictionary = ChannelDictionary::create(_ommBaseImpl);
}

void DictionaryCallbackClient::loadDictionaryFromFile()
{
	RsslRet retCode;
	if (_localDictionary->load(_ommBaseImpl.getActiveConfig().dictionaryConfig.rdmfieldDictionaryFileName, _ommBaseImpl.getActiveConfig().dictionaryConfig.enumtypeDefFileName, retCode) == false)
	{
		EmaString temp("DictionaryCallbackClient::loadDictionaryFromFile() failed.");
		temp.append(CR).append("Unable to load RDMFieldDictionary from file named ").append(_ommBaseImpl.getActiveConfig().dictionaryConfig.rdmfieldDictionaryFileName)
			.append(CR).append("or load enumtype.def from file named ").append(_ommBaseImpl.getActiveConfig().dictionaryConfig.enumtypeDefFileName);

		throwIueException(temp, retCode);
	}
}

bool DictionaryCallbackClient::downloadDictionary( const Directory& directory )
{
	if ( _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg && _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg )
	{
		if ( _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg->get()->msgBase.msgKey.serviceId == directory.getId() )
		{
			downloadDictionaryFromService( directory );
		}
		else if ( _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg->getServiceName() == directory.getName() )
		{
			downloadDictionaryFromService( directory );
		}

		return true;
	}
	else if ( _localDictionary )
	{
		directory.getChannel()->setDictionary( _localDictionary );
		return true;
	}
	else if ( _ommBaseImpl.getActiveConfig().dictionaryConfig.dictionaryType == Dictionary::ChannelDictionaryEnum )
	{
		if ( directory.getChannel()->getDictionary() )
			return true;
	}

	RsslRequestMsg requestMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );

	rsslClearRequestMsg( &requestMsg );
	requestMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER;
	requestMsg.msgBase.msgKey.filter = RDM_DICTIONARY_NORMAL;
	requestMsg.flags = RSSL_RQMF_STREAMING;

	ChannelDictionary* pDictionary;
		
	if (_channelDictionaryList.size() == 0)
	{
		pDictionary = _channelDictionary; /* Always use the default channel dictionary for this first one in the list */
	}
	else
	{
		pDictionary = ChannelDictionary::create(_ommBaseImpl);
	}

	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = ( RsslMsg* )&requestMsg;
	RsslBuffer serviceName;
	serviceName.data = ( char* )directory.getName().c_str();
	serviceName.length = directory.getName().length();
	submitMsgOpts.pServiceName = &serviceName;
	submitMsgOpts.majorVersion = directory.getChannel()->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = directory.getChannel()->getRsslChannel()->minorVersion;
	submitMsgOpts.requestMsgOptions.pUserSpec = ( void* )pDictionary;

	const EmaVector< EmaString > dictionariesUsed = directory.getInfo().getDictionariesUsed().getDictionaryList();
	const EmaVector< EmaString > dictionariesProvided = directory.getInfo().getDictionariesProvided().getDictionaryList();

	Int32 streamId = 3;

	for ( UInt32 idx = 0; idx < dictionariesUsed.size(); ++idx )
	{
		const EmaString& dictionaryUsage_Name = dictionariesUsed[idx];
		// download the dictionary if it is included to dictionariesProvided list
		if (dictionariesProvided.getPositionOf(dictionaryUsage_Name) < 0)
			continue;

		requestMsg.msgBase.msgKey.name.data = ( char* )dictionariesUsed[idx].c_str();
		requestMsg.msgBase.msgKey.name.length = dictionariesUsed[idx].length();
		requestMsg.msgBase.streamId = streamId++;

		if ( ( ret = rsslReactorSubmitMsg( directory.getChannel()->getRsslReactor(),
		                                   directory.getChannel()->getRsslChannel(),
		                                   &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error: rsslReactorSubmitMsg() failed" );
				temp.append( CR ).append( directory.getChannel()->toString() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

				ChannelDictionary::destroy( pDictionary );
			}
			return false;
		}
		else
		{
			if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Requested Dictionary " );
				temp.append( dictionariesUsed[idx] ).append( CR )
				.append( "from Service " ).append( directory.getName() ).append( CR )
				.append( "on Channel " ).append( CR )
				.append( directory.getChannel()->toString() );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp.trimWhitespace() );
			}
		}
	}

	_channelDictionaryList.push_back( pDictionary );

	if ( _ommBaseImpl.getActiveConfig().dictionaryConfig.dictionaryType == Dictionary::ChannelDictionaryEnum )
		directory.getChannel()->setDictionary( pDictionary );

	return true;
}

Dictionary* DictionaryCallbackClient::getDefaultDictionary() const
{
	if ( _localDictionary )
	{
		return _localDictionary;
	}
	else
	{
		return _channelDictionary; /* This is the first Channel dictionary in the list */
	}
}

bool DictionaryCallbackClient::downloadDictionaryFromService( const Directory& directory )
{
	RsslRequestMsg requestMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );

	rsslClearRequestMsg( &requestMsg );
	requestMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER;
	requestMsg.msgBase.msgKey.filter = _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg->get()->msgBase.msgKey.filter;

	if ( _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg->get()->flags & RSSL_RQMF_STREAMING )
		requestMsg.flags = RSSL_RQMF_STREAMING;

	ChannelDictionary* pDictionary;
		
	if (_channelDictionaryList.size() == 0)
	{
		pDictionary = _channelDictionary; /* Always use the default channel dictionary for this first one in the list */
	}
	else
	{
		pDictionary = ChannelDictionary::create(_ommBaseImpl);
	}

	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = ( RsslMsg* )&requestMsg;
	RsslBuffer serviceName;
	serviceName.data = ( char* )directory.getName().c_str();
	serviceName.length = directory.getName().length();
	submitMsgOpts.pServiceName = &serviceName;
	submitMsgOpts.majorVersion = directory.getChannel()->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = directory.getChannel()->getRsslChannel()->minorVersion;
	submitMsgOpts.requestMsgOptions.pUserSpec = ( void* )pDictionary;

	requestMsg.msgBase.msgKey.name = _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg->get()->msgBase.msgKey.name;

	requestMsg.msgBase.streamId = 3;

	if ( ( ret = rsslReactorSubmitMsg( directory.getChannel()->getRsslReactor(),
	                                   directory.getChannel()->getRsslChannel(),
	                                   &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed" );
			temp.append( CR ).append( "Channel" ).append( CR )
			.append( directory.getChannel()->toString() ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );

			ChannelDictionary::destroy( pDictionary );
		}
		return false;
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString dictionaryName( _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg->get()->msgBase.msgKey.name.data,
			                          _ommBaseImpl.getActiveConfig().pRsslRdmFldRequestMsg->get()->msgBase.msgKey.name.length );

			EmaString temp( "Requested Dictionary " );
			temp.append( dictionaryName ).append( CR )
			.append( "from Service " ).append( directory.getName() ).append( CR )
			.append( "on Channel" ).append( CR )
			.append( directory.getChannel()->toString() );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp.trimWhitespace() );
		}
	}

	rsslClearRequestMsg( &requestMsg );
	requestMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER;
	requestMsg.msgBase.msgKey.filter = _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg->get()->msgBase.msgKey.filter;

	if ( _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg->get()->flags & RSSL_RQMF_STREAMING )
		requestMsg.flags = RSSL_RQMF_STREAMING;

	requestMsg.msgBase.msgKey.name = _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg->get()->msgBase.msgKey.name;

	requestMsg.msgBase.streamId = 4;

	if ( ( ret = rsslReactorSubmitMsg( directory.getChannel()->getRsslReactor(),
	                                   directory.getChannel()->getRsslChannel(),
	                                   &submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorSubmitMsg() failed" );
			temp.append( CR ).append( "Channel " ).append( CR )
			.append( directory.getChannel()->toString() ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );

			ChannelDictionary::destroy( pDictionary );
		}
		return false;
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString dictionaryName( _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg->get()->msgBase.msgKey.name.data,
			                          _ommBaseImpl.getActiveConfig().pRsslEnumDefRequestMsg->get()->msgBase.msgKey.name.length );
			EmaString temp( "Requested Dictionary " );
			temp.append( dictionaryName ).append( CR )
			.append( "from Service " ).append( directory.getName() ).append( CR )
			.append( "on Channel " ).append( CR )
			.append( directory.getChannel()->toString() );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp.trimWhitespace() );
		}
	}

	_channelDictionaryList.push_back( pDictionary );

	if ( _ommBaseImpl.getActiveConfig().dictionaryConfig.dictionaryType == Dictionary::ChannelDictionaryEnum )
		directory.getChannel()->setDictionary( pDictionary );

	return true;
}

RsslReactorCallbackRet DictionaryCallbackClient::processCallback( RsslReactor* pRsslReactor,
    RsslReactorChannel* pRsslReactorChannel,
    RsslRDMDictionaryMsgEvent* pEvent )
{
	ChannelDictionary* channelDict = _channelDictionaryList.front();
	bool isChannelDict = false;

	while ( channelDict )
	{
		if ( channelDict == ( ChannelDictionary* )pEvent->baseMsgEvent.pStreamInfo->pUserSpec )
		{
			isChannelDict = true;
			break;
		}

		channelDict = channelDict->next();
	}

	if ( isChannelDict )
	{
		return channelDict->processCallback( pRsslReactor, pRsslReactorChannel, pEvent );
	}
	else
	{
		return processCallback( pRsslReactor, pRsslReactorChannel, pEvent, static_cast<DictionaryItem*>( pEvent->baseMsgEvent.pStreamInfo->pUserSpec ) );
	}
}

RsslReactorCallbackRet DictionaryCallbackClient::processCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel,
    RsslRDMDictionaryMsgEvent* pEvent, DictionaryItem* pItem )
{
	if ( !pEvent->pRDMDictionaryMsg )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
			       "Internal error. Received RsslRDMDictionaryMsgEvent with no RsslRDMDictionaryMsg in DictionaryCallbackClient::processCallback()" );

		return RSSL_RC_CRET_SUCCESS;
	}

	RsslErrorInfo rsslErrorInfo;
	clearRsslErrorInfo( &rsslErrorInfo );
	char errorBuf[255];
	RsslBuffer errorText = { 255, errorBuf };
	RsslRet retCode = RSSL_RET_SUCCESS;
	UInt32 completeFlag  = 0;
	RsslBuffer rsslMsgBuffer;
	rsslClearBuffer(&rsslMsgBuffer);
	RsslEncIterator eIter;
	rsslClearEncodeIterator(&eIter);

	switch ( pEvent->pRDMDictionaryMsg->rdmMsgBase.rdmMsgType )
	{
	case RDM_DC_MT_REFRESH:
	{
		RsslDecodeIterator dIter;
		rsslClearDecodeIterator( &dIter );
		RsslDataDictionary rsslDataDictionary;
		rsslClearDataDictionary( &rsslDataDictionary );
		if ( RSSL_RET_SUCCESS != rsslSetDecodeIteratorRWFVersion( &dIter, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion ) )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error: failed to set RWF Version while decoding dictionary in DictionaryCallbackClient::processCallback()" );
				temp.append( CR )
				.append( "Trying to set " )
				.append( pRsslReactorChannel->majorVersion ).append( "." ).append( pRsslReactorChannel->minorVersion );
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}

			return RSSL_RC_CRET_SUCCESS;
		}

		if ( RSSL_RET_SUCCESS != rsslSetDecodeIteratorBuffer( &dIter, &pEvent->pRDMDictionaryMsg->refresh.dataBody ) )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
					"Internal error: failed to set iterator buffer while decoding dictionary in DictionaryCallbackClient::processCallback()" );
			}

			return RSSL_RC_CRET_SUCCESS;
		}

		if ( pItem->getDictionaryType() == 0 )
		{
			pItem->setDictionaryType( pEvent->pRDMDictionaryMsg->refresh.type );
		}
		else
		{
			pEvent->pRDMDictionaryMsg->refresh.type = pItem->getDictionaryType();
		}

		if ( pEvent->pRDMDictionaryMsg->refresh.type == RDM_DICTIONARY_FIELD_DEFINITIONS )
		{
			retCode = rsslDecodeFieldDictionary(&dIter, &rsslDataDictionary, (RDMDictionaryVerbosityValues)pItem->getFilter(), &errorText);
		}
		else if ( pEvent->pRDMDictionaryMsg->refresh.type == RDM_DICTIONARY_ENUM_TABLES )
		{
			retCode = rsslDecodeEnumTypeDictionary(&dIter, &rsslDataDictionary, (RDMDictionaryVerbosityValues)pItem->getFilter(), &errorText);
		}

		if (retCode != RSSL_RET_SUCCESS)
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error: failed to decode data dictionary in DictionaryCallbackClient::processCallback()" );
				temp.append(CR).append("Reason='").append(errorText.data).append("'");
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}

			return RSSL_RC_CRET_SUCCESS;
		}

		if ( RSSL_RET_SUCCESS != allocateAndSetEncodeIteratorBuffer(&rsslMsgBuffer, pEvent->pRDMDictionaryMsg->refresh.dataBody.length + DEFAULT_DICTIONARY_RESP_HEADER_SIZE,
			pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion, &eIter, "DictionaryCallbackClient::processCallback()"))
		{
			return RSSL_RC_CRET_SUCCESS;
		}

		bool completeFlag = pEvent->pRDMDictionaryMsg->refresh.flags & RDM_DC_RFF_IS_COMPLETE ? true : false;
		pEvent->pRDMDictionaryMsg->refresh.pDictionary = &rsslDataDictionary;
		pEvent->pRDMDictionaryMsg->refresh.flags &= ~( RDM_DC_RFF_IS_COMPLETE | RDM_DC_RFF_HAS_INFO );
		retCode = rsslEncodeRDMDictionaryMsg( &eIter, pEvent->pRDMDictionaryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );

		while ( retCode == RSSL_RET_BUFFER_TOO_SMALL || retCode == RSSL_RET_DICT_PART_ENCODED )
		{
			free( rsslMsgBuffer.data );

			if (RSSL_RET_SUCCESS != allocateAndSetEncodeIteratorBuffer(&rsslMsgBuffer, rsslMsgBuffer.length * 2, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion, &eIter,
				"DictionaryCallbackClient::processCallback()") )
			{
				return RSSL_RC_CRET_SUCCESS;
			}

			retCode = rsslEncodeRDMDictionaryMsg( &eIter, pEvent->pRDMDictionaryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );
		}

		if ( retCode == RSSL_RET_SUCCESS )
		{
			processRefreshMsg(&rsslMsgBuffer, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion, pItem, completeFlag );
		}
	}
	break;

	case RDM_DC_MT_STATUS:

		RsslStatusMsg statusMsg;
		rsslClearStatusMsg(&statusMsg);
		statusMsg.msgBase.streamId = pEvent->pRDMDictionaryMsg->rdmMsgBase.streamId;
		statusMsg.msgBase.domainType = pEvent->pRDMDictionaryMsg->rdmMsgBase.domainType;
		statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		statusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY;
		statusMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_SERVICE_ID;
		statusMsg.msgBase.msgKey.name.data = (char*)pItem->getName().c_str();
		statusMsg.msgBase.msgKey.name.length = pItem->getName().length();
		statusMsg.msgBase.msgKey.serviceId = (RsslUInt16)pItem->getDirectory()->getId();
		statusMsg.state = pEvent->pRDMDictionaryMsg->status.state;

		encodeAndNotifyStatusMsg(pItem, &statusMsg, &rsslMsgBuffer, DEFAULT_DICTIONARY_RESP_HEADER_SIZE, pRsslReactorChannel->majorVersion,
			pRsslReactorChannel->minorVersion, &eIter, "DictionaryCallbackClient::processCallback()");
		
		return RSSL_RC_CRET_SUCCESS;

		break;
	default :
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
						"Internal error. Received unexpected type of RsslRDMDictionaryMsg in DictionaryCallbackClient::processCallback()" );
			break;
		}
	}

	if ( retCode != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: failed to encode RsslRDMDictionaryMsg in DictionaryCallbackClient::processCallback()" );
			temp.append( CR )
			.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
			.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
			.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
			.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
			.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
			.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommBaseImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}
	}

	free( rsslMsgBuffer.data );
	return RSSL_RC_CRET_SUCCESS;
}

int DictionaryCallbackClient::allocateAndSetEncodeIteratorBuffer(RsslBuffer* rsslBuffer, UInt32 allocateBufferSize, UInt8 majorVersion, UInt8 minorVersion, 
	RsslEncodeIterator* rsslEncodeIterator, const char* methodName)
{
	rsslBuffer->length = allocateBufferSize;

	rsslBuffer->data = (char*)malloc(sizeof(char) * rsslBuffer->length);

	if ( !rsslBuffer->data )
	{
		EmaString text("Failed to allocate memory in ");
		text.append(methodName);
		_ommBaseImpl.handleMee(text.c_str());
		return RSSL_RET_FAILURE;
	}

	int retCode = rsslSetEncodeIteratorRWFVersion(rsslEncodeIterator, majorVersion, minorVersion);
	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString text("Internal error. Failed to set encode iterator RWF version in ");
		text.append(methodName);
		if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
			_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
			text.c_str());

		free(rsslBuffer->data);
		return retCode;
	}

	retCode = rsslSetEncodeIteratorBuffer(rsslEncodeIterator, rsslBuffer);
	if ( retCode != RSSL_RET_SUCCESS )
	{
		EmaString text("Internal error. Failed to set encode iterator buffer in ");
		text.append(methodName);
		if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
			_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
			text.c_str());

		free(rsslBuffer->data);
		return retCode;
	}

	return RSSL_RET_SUCCESS;
}

void DictionaryCallbackClient::encodeAndNotifyStatusMsg(DictionaryItem* dictionaryItem, RsslStatusMsg* statusMsg, RsslBuffer* msgBuf, UInt32 bufferSize, UInt8 majorVersion, UInt8 minorVersion,
	RsslEncodeIterator* rsslEncodeIterator, const char* methodName)
{
	if (RSSL_RET_SUCCESS != dictionaryItem->getImpl().getDictionaryCallbackClient().allocateAndSetEncodeIteratorBuffer(msgBuf, bufferSize, majorVersion,
		minorVersion, rsslEncodeIterator, methodName))
	{
		return;
	} 

	int ret = rsslEncodeMsg(rsslEncodeIterator, (RsslMsg*)statusMsg);
	while (ret == RSSL_RET_BUFFER_TOO_SMALL)
	{
		free(msgBuf->data);

		if (RSSL_RET_SUCCESS != dictionaryItem->getImpl().getDictionaryCallbackClient().allocateAndSetEncodeIteratorBuffer(msgBuf, msgBuf->length * 2, majorVersion, minorVersion, rsslEncodeIterator,
			methodName))
		{
			return;
		}

		ret = rsslEncodeMsg(rsslEncodeIterator, (RsslMsg*)statusMsg);
	}

	if (ret != RSSL_RET_SUCCESS)
	{
		EmaString text("Internal error. Failed to encode Status message in ");
		text.append(text);
		if (OmmLoggerClient::ErrorEnum >= dictionaryItem->getImpl().getActiveConfig().loggerConfig.minLoggerSeverity)
			dictionaryItem->getImpl().getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
			text.c_str());

		free(msgBuf->data);
		return;
	}

	msgBuf->length = rsslGetEncodedBufferLength(rsslEncodeIterator);

	dictionaryItem->getImpl().getDictionaryCallbackClient().processStatusMsg(msgBuf,
		majorVersion, minorVersion, dictionaryItem);

	free(msgBuf->data);
}

RsslReactorCallbackRet DictionaryCallbackClient::processRefreshMsg( RsslBuffer* msgBuf, UInt8 majorVersion, UInt8 minorVersion, DictionaryItem* dictionaryItem,
	bool completeFlag)
{
	RsslDecodeIterator rsslDecodeIterator;
	RsslRefreshMsg rsslRefreshMsg;
	RsslRet ret = RSSL_RET_SUCCESS;

	rsslClearDecodeIterator(&rsslDecodeIterator);
	rsslClearRefreshMsg(&rsslRefreshMsg);

	if ((ret = rsslSetDecodeIteratorBuffer(&rsslDecodeIterator, msgBuf)) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
			_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
			"Internal error. Failed to set encode iterator buffer in DictionaryCallbackClient::processRefreshMsg()");

		return RSSL_RC_CRET_FAILURE;
	}

	if ((ret = rsslSetDecodeIteratorRWFVersion(&rsslDecodeIterator, majorVersion, minorVersion)) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
			_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
			"Internal error. Failed to set encode iterator RWF version in DictionaryCallbackClient::processRefreshMsg()");

		return RSSL_RC_CRET_FAILURE;
	}

	if ((ret = rsslDecodeMsg(&rsslDecodeIterator, (RsslMsg*)&rsslRefreshMsg)) != RSSL_RET_SUCCESS)
	{
		if (OmmLoggerClient::ErrorEnum >= _ommBaseImpl.getActiveConfig().loggerConfig.minLoggerSeverity)
			_ommBaseImpl.getOmmLoggerClient().log(_clientName, OmmLoggerClient::ErrorEnum,
			"Internal error. Failed to decode message in DictionaryCallbackClient::processRefreshMsg()");

		return RSSL_RC_CRET_FAILURE;
	}

	if (completeFlag)
	{
		rsslRefreshMsg.flags |= RSSL_RFMF_REFRESH_COMPLETE;
	}
	else
	{
		rsslRefreshMsg.flags &= ~RSSL_RFMF_REFRESH_COMPLETE;
	}

	StaticDecoder::setRsslData(&_refreshMsg, (RsslMsg*)&rsslRefreshMsg, majorVersion, minorVersion, 0);

	if ( dictionaryItem->getDirectory() && _refreshMsg.hasServiceId() )
	{
		if ( _refreshMsg.getServiceId() == dictionaryItem->getDirectory()->getId() )
		{
			_refreshMsg.getDecoder().setServiceName(dictionaryItem->getDirectory()->getName().c_str(), dictionaryItem->getDirectory()->getName().length());
		}
	}

	_ommBaseImpl.msgDispatched();

	dictionaryItem->onAllMsg( _refreshMsg );
	dictionaryItem->onRefreshMsg( _refreshMsg );

	if ( _refreshMsg.getState().getStreamState() == OmmState::NonStreamingEnum )
	{
		if ( _refreshMsg.getComplete() )
			dictionaryItem->remove();
	}
	else if ( _refreshMsg.getState().getStreamState() != OmmState::OpenEnum )
	{
		dictionaryItem->remove();
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet DictionaryCallbackClient::processStatusMsg( RsslBuffer* msgBuf, UInt8 majorVersion, UInt8 minorVersion, DictionaryItem* dictionaryItem )
{
	StaticDecoder::setRsslData( &_statusMsg, msgBuf, RSSL_DT_MSG, majorVersion, minorVersion, 0 );

	if ( dictionaryItem->getDirectory() && _statusMsg.hasServiceId() )
	{
		if ( _statusMsg.getServiceId() == dictionaryItem->getDirectory()->getId() )
		{
			_statusMsg.getDecoder().setServiceName( dictionaryItem->getDirectory()->getName().c_str(), dictionaryItem->getDirectory()->getName().length() );
		}
	}

	_ommBaseImpl.msgDispatched();

	dictionaryItem->onAllMsg( _statusMsg );
	dictionaryItem->onStatusMsg( _statusMsg );

	if ( _statusMsg.getState().getStreamState() != OmmState::OpenEnum )
		dictionaryItem->remove();

	return RSSL_RC_CRET_SUCCESS;
}

bool DictionaryCallbackClient::isDictionaryReady() const
{
	bool retCode = true;

	if ( _localDictionary )
		return _localDictionary->isLoaded();
	else
	{
		ChannelDictionary* dictionary = _channelDictionaryList.front();

		if ( !dictionary ) return false;

		while ( dictionary )
		{
			retCode &= dictionary->isLoaded();
			dictionary = dictionary->next();
		}
	}

	return retCode;
}

void DictionaryCallbackClient::sendInternalMsg( void* item )
{
	DictionaryItem* dictItem = reinterpret_cast<DictionaryItem*>( item );

	if ( dictItem->isRemoved() )
		return;

	Dictionary* dictionary = dictItem->getImpl().getDictionaryCallbackClient().getDefaultDictionary();

	RsslBuffer msgBuf;
	bool firstPart = false;
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslRefreshMsg refreshMsg;
	rsslClearRefreshMsg( &refreshMsg );
	RsslDecodeIterator decodeIter;
	rsslClearDecodeIterator( &decodeIter );

	if ( dictionary->isLoaded() )
	{
		msgBuf.length = MAX_DICTIONARY_BUFFER_SIZE;
		msgBuf.data = (char*)malloc(sizeof(char) * msgBuf.length);

		if (!msgBuf.data)
		{
			dictItem->getImpl().handleMee("Failed to allocate memory in DictionaryCallbackClient::sendInternalMsg()");
			return;
		}

		const RsslDataDictionary* rsslDataDictionary = dictionary->getRsslDictionary();
		RsslInt32 dictionaryFid = dictItem->getCurrentFid();

		if ( dictItem->getName() == DictionaryCallbackClient::_rwfFldName )
		{
			if ( dictionaryFid == rsslDataDictionary->minFid )
				firstPart = true;

			ret = DictionaryItem::encodeDataDictionaryResp( *dictItem, msgBuf, dictItem->getName(), dictItem->getFilter(), dictItem->getStreamId() ,
			      firstPart, const_cast<RsslDataDictionary*>( rsslDataDictionary ), dictionaryFid );
		}
		else if ( dictItem->getName() == DictionaryCallbackClient::_rwfEnumName )
		{
			if ( dictionaryFid == 0 )
				firstPart = true;

			ret = DictionaryItem::encodeDataDictionaryResp( *dictItem, msgBuf, dictItem->getName(), dictItem->getFilter(), dictItem->getStreamId(),
			      firstPart, const_cast<RsslDataDictionary*>( rsslDataDictionary ), dictionaryFid );
		}

		if ( ( ret == RSSL_RET_SUCCESS ) || ( ret == RSSL_RET_DICT_PART_ENCODED ) )
		{
			dictItem->setCurrentFid( dictionaryFid );
			dictItem->getImpl().getDictionaryCallbackClient().processRefreshMsg( &msgBuf,
				RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, dictItem, ret == RSSL_RET_SUCCESS ? true : false);
			
			free( msgBuf.data );
		}

		if ( ret == RSSL_RET_DICT_PART_ENCODED )
		{
			new TimeOut( dictItem->getImpl(), 500, &DictionaryCallbackClient::sendInternalMsg, dictItem, true );
			return;
		}

		if (ret != RSSL_RET_SUCCESS)
		{
			RsslStatusMsg statusMsg;
			rsslClearStatusMsg(&statusMsg);
			RsslEncodeIterator encodeIter;
			rsslClearEncodeIterator(&encodeIter);

			statusMsg.msgBase.streamId = dictItem->getStreamId();
			statusMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
			statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
			statusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY;
			statusMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
			statusMsg.msgBase.msgKey.name.data = (char*)dictItem->getName().c_str();
			statusMsg.msgBase.msgKey.name.length = dictItem->getName().length();
			statusMsg.state.streamState = RSSL_STREAM_CLOSED;
			statusMsg.state.dataState = RSSL_DATA_SUSPECT;
			statusMsg.state.code = RSSL_SC_ERROR;
			statusMsg.state.text.data = (char*)"Failed to provide data dictionary: Internal error.";
			statusMsg.state.text.length = (RsslUInt32)strlen(statusMsg.state.text.data) + 1;

			dictItem->getImpl().getDictionaryCallbackClient().encodeAndNotifyStatusMsg(dictItem, &statusMsg, &msgBuf, DEFAULT_DICTIONARY_RESP_HEADER_SIZE,
				RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &encodeIter, "DictionaryCallbackClient::sendInternalMsg()");
		}
	}
	else
	{
		RsslStatusMsg statusMsg;
		rsslClearStatusMsg( &statusMsg );
		RsslEncodeIterator encodeIter;
		rsslClearEncodeIterator( &encodeIter );

		statusMsg.msgBase.streamId = dictItem->getStreamId();
		statusMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
		statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		statusMsg.flags = RSSL_STMF_HAS_STATE | RSSL_STMF_HAS_MSG_KEY;
		statusMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME;
		statusMsg.msgBase.msgKey.name.data = (char*)dictItem->getName().c_str();
		statusMsg.msgBase.msgKey.name.length = dictItem->getName().length();
		statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
		statusMsg.state.dataState = RSSL_DATA_SUSPECT;
		statusMsg.state.code = RSSL_SC_ERROR;
		statusMsg.state.text.data = (char*)"Data dictionary is not ready to provide.";
		statusMsg.state.text.length = ( RsslUInt32 )strlen( statusMsg.state.text.data ) + 1;

		dictItem->getImpl().getDictionaryCallbackClient().encodeAndNotifyStatusMsg(dictItem, &statusMsg, &msgBuf, DEFAULT_DICTIONARY_RESP_HEADER_SIZE,
			RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, &encodeIter, "DictionaryCallbackClient::sendInternalMsg()");
	}
}

DictionaryItem::DictionaryItem( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure ) :
	SingleItem( ommBaseImpl, ommConsClient, closure, 0 ),
	_name(),
	_currentFid( 0 ),
	_filter( 0 ),
	_isRemoved( false ),
	_dictionaryType( 0 )
{
}

DictionaryItem::~DictionaryItem()
{
}

DictionaryItem* DictionaryItem::create( OmmBaseImpl& ommBaseImpl, OmmConsumerClient& ommConsClient, void* closure )
{
	DictionaryItem* pItem = 0;

	try
	{
		pItem = new DictionaryItem( ommBaseImpl, ommConsClient, closure );
	}
	catch ( std::bad_alloc& ) {}

	if ( !pItem )
		ommBaseImpl.handleMee( "Failed to create DictionaryItem" );

	return pItem;
}

bool DictionaryItem::isRemoved()
{
	return _isRemoved;
}

bool DictionaryItem::open( const ReqMsg& reqMsg )
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );

	_name = EmaString( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.data, reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length );

	if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER )
		_filter = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.filter;

	if ( reqMsgEncoder.hasServiceName() || ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) )
	{
		return SingleItem::open( reqMsg );
	}
	else
	{
		Dictionary* dictionary = _ommBaseImpl.getDictionaryCallbackClient().getDefaultDictionary();

		// Unset the channel as the dictionary information is generated from EMA.
		_event._channel = 0;

		if ( dictionary )
		{
			if (_name == DictionaryCallbackClient::_rwfFldName)
			{
				_currentFid = dictionary->getRsslDictionary()->minFid;
				_streamId = dictionary->getFldStreamId();
			}
			else if (_name == DictionaryCallbackClient::_rwfEnumName)
			{
				_currentFid = 0;
				_streamId = dictionary->getEnumStreamId();
			}
			else
			{
				EmaString temp("Invalid ReqMsg's name : ");
				temp.append(_name);
				temp.append("\nReqMsg's name must be \"RWFFld\" or \"RWFEnum\" for MMT_DICTIONARY domain type. ");
				temp.append("Instance name='").append(_ommBaseImpl.getInstanceName()).append("'.");
				_ommBaseImpl.handleIue(temp, OmmInvalidUsageException::InvalidArgumentEnum );
				return false;
			}

			if ( dictionary->getType() == Dictionary::ChannelDictionaryEnum )
			{
				ChannelDictionary* channelDict = static_cast<ChannelDictionary*>( dictionary );

				channelDict->acquireLock();

				channelDict->addListener( this );

				channelDict->releaseLock();
			}

			new TimeOut( _ommBaseImpl, 500, &DictionaryCallbackClient::sendInternalMsg, this, true );

			return true;
		}
		else
		{
			return false;
		}
	}
}

void DictionaryItem::setDictionaryType(UInt64 dictionaryType)
{
	_dictionaryType = dictionaryType;
}

UInt64 DictionaryItem::getDictionaryType()
{
	return _dictionaryType;
}

void DictionaryItem::setCurrentFid( Int32 fid )
{
	_currentFid = fid;
}

const EmaString& DictionaryItem::getName()
{
	return _name;
}

unsigned char DictionaryItem::getFilter()
{
	return _filter;
}

Int32 DictionaryItem::getCurrentFid()
{
	return _currentFid;
}

Int32 DictionaryItem::getStreamId()
{
	return _streamId;
}

bool DictionaryItem::modify( const ReqMsg& reqMsg )
{
	EmaString temp( "Invalid attempt to modify dictionary stream. " );
	temp.append( "User name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );

	return false;
}

bool DictionaryItem::submit( const PostMsg& )
{
	EmaString temp( "Invalid attempt to submit PostMsg on dictionary stream. " );
	temp.append( "User name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );

	return false;
}

bool DictionaryItem::submit( const GenericMsg& )
{
	EmaString temp( "Invalid attempt to submit GenericMsg on dictionary stream. " );
	temp.append( "User name='" ).append( _ommBaseImpl .getInstanceName() ).append( "'." );

	_ommBaseImpl.handleIue( temp, OmmInvalidUsageException::InvalidOperationEnum );

	return false;
}

void DictionaryItem::remove()
{
	if ( !_isRemoved )
	{
		_isRemoved = true;

		new TimeOut( getImpl(), 2000, &DictionaryItem::ScheduleRemove, this, true );
	}
}

bool DictionaryItem::close()
{
	if ( _streamId > 4 )
	{
		SingleItem::close();
	}
	else
	{
		if ( _isRemoved == false )
		{
			_isRemoved = true;

			Dictionary* dictionary = _ommBaseImpl.getDictionaryCallbackClient().getDefaultDictionary();

			if ( dictionary->getType() == Dictionary::ChannelDictionaryEnum )
			{
				ChannelDictionary* channelDict = static_cast<ChannelDictionary*>( dictionary );

				channelDict->acquireLock();

				channelDict->removeListener( this );

				channelDict->releaseLock();
			}

			new TimeOut( this->getImpl(), 2000, &DictionaryItem::ScheduleRemove, this, true );
		}
	}

	return true;
}

void DictionaryItem::ScheduleRemove( void* item )
{
	delete( DictionaryItem* )item;
}

RsslRet DictionaryItem::encodeDataDictionaryResp( DictionaryItem& dictionaryItem, RsslBuffer& msgBuf, const EmaString& name, unsigned char filter, RsslInt32 streamId,
    bool firstMultiRefresh, RsslDataDictionary* rsslDataDictionary, Int32& currentFid )
{
	RsslRet ret = RSSL_RET_SUCCESS;
	char errTxt[256];
	RsslBuffer errorText = {255, ( char* )errTxt};
	RsslEncodeIterator encodeIter;
	RsslBool dictionaryComplete = RSSL_FALSE;
	RsslRefreshMsg refreshMsg;

	rsslClearRefreshMsg( &refreshMsg );
	rsslClearEncodeIterator( &encodeIter );

	refreshMsg.msgBase.msgClass = RSSL_MC_REFRESH;
	refreshMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	refreshMsg.msgBase.containerType = RSSL_DT_SERIES;
	refreshMsg.state.streamState = RSSL_STREAM_OPEN;
	refreshMsg.state.code = RSSL_SC_NONE;
	refreshMsg.msgBase.msgKey.filter = filter;

	refreshMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER;
	refreshMsg.flags = RSSL_RFMF_HAS_MSG_KEY | RSSL_RFMF_SOLICITED;

	if ( firstMultiRefresh )
		refreshMsg.flags |= RSSL_RFMF_CLEAR_CACHE;

	refreshMsg.msgBase.msgKey.name.data = ( char* )name.c_str();
	refreshMsg.msgBase.msgKey.name.length = name.length();

	refreshMsg.msgBase.streamId = streamId;

	while ( true )
	{
		if ( ( ret = rsslSetEncodeIteratorBuffer( &encodeIter, &msgBuf ) ) < RSSL_RET_SUCCESS )
		{
			return ret;
		}

		if ( ( ret = rsslSetEncodeIteratorRWFVersion( &encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) ) < RSSL_RET_SUCCESS )
		{
			return ret;
		}

		if ( ( ret = rsslEncodeMsgInit( &encodeIter, ( RsslMsg* )&refreshMsg, 0 ) ) < RSSL_RET_SUCCESS )
		{
			return ret;
		}

		if ( name == DictionaryCallbackClient::_rwfFldName )
		{
			ret = rsslEncodeFieldDictionary( &encodeIter, rsslDataDictionary, &currentFid, ( RDMDictionaryVerbosityValues )filter, &errorText );
		}
		else if ( name == DictionaryCallbackClient::_rwfEnumName )
		{
			ret = rsslEncodeEnumTypeDictionaryAsMultiPart( &encodeIter, rsslDataDictionary, &currentFid, ( RDMDictionaryVerbosityValues )filter, &errorText );
		}

		if ( ret  != RSSL_RET_SUCCESS )
		{
			if ( ret == RSSL_RET_BUFFER_TOO_SMALL )
			{
				free( msgBuf.data );

				try
				{
					msgBuf.length = msgBuf.length * 2;
					msgBuf.data = (char*)malloc(sizeof(char) * msgBuf.length);
				}
				catch (std::bad_alloc&)
				{
					dictionaryItem.getImpl().handleMee("Failed to allocate memory in DictionaryCallbackClient::encodeDataDictionaryResp()");
					return RSSL_RET_FAILURE;
				}

				continue;
			}
			else if ( ret != RSSL_RET_DICT_PART_ENCODED )
			{
				return ret;
			}
		}
		else
		{
			dictionaryComplete = RSSL_TRUE;
			rsslSetRefreshCompleteFlag( &encodeIter );
		}

		if ( ( ret = rsslEncodeMsgComplete( &encodeIter, RSSL_TRUE ) ) < RSSL_RET_SUCCESS )
			return ret;

		break;
	}

	msgBuf.length = rsslGetEncodedBufferLength( &encodeIter );

	return dictionaryComplete ? RSSL_RET_SUCCESS : RSSL_RET_DICT_PART_ENCODED;
}

NiProviderDictionaryItem::NiProviderDictionaryItem(OmmBaseImpl& ommBaseImpl, OmmProviderClient& ommProvClient, ItemWatchList* pItemWatchList, void* closure) :
	NiProviderSingleItem(ommBaseImpl, ommProvClient, pItemWatchList, closure, 0)
{
}

NiProviderDictionaryItem::~NiProviderDictionaryItem()
{
}

NiProviderDictionaryItem* NiProviderDictionaryItem::create(OmmBaseImpl& ommBaseImpl, OmmProviderClient& ommProvClient, void* closure)
{
	NiProviderDictionaryItem* pItem = 0;

	try
	{
		pItem = new NiProviderDictionaryItem(ommBaseImpl, ommProvClient, &static_cast<OmmNiProviderImpl&>(ommBaseImpl).getItemWatchList() ,closure);
	}
	catch (std::bad_alloc&) {}

	if (!pItem)
		ommBaseImpl.handleMee("Failed to create NiProviderDictionaryItem");

	return pItem;
}

bool NiProviderDictionaryItem::modify( const ReqMsg& reqMsg )
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder());

	reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType = MMT_DICTIONARY;

	return ProviderItem::modify( reqMsg );
}

IProviderDictionaryItem::IProviderDictionaryItem(OmmServerBaseImpl& ommServerBaseImpl, OmmProviderClient& ommProvClient, ItemWatchList* pItemWatchList, void* closure) :
	IProviderSingleItem( ommServerBaseImpl, ommProvClient, pItemWatchList, closure, 0 )
{
}

IProviderDictionaryItem::~IProviderDictionaryItem()
{
}

IProviderDictionaryItem* IProviderDictionaryItem::create(OmmServerBaseImpl& ommServerBaseImpl, OmmProviderClient& ommProvClient, void* closure)
{
	IProviderDictionaryItem* pItem = 0;

	try
	{
		pItem = new IProviderDictionaryItem( ommServerBaseImpl, ommProvClient, &static_cast<OmmIProviderImpl&>(ommServerBaseImpl).getItemWatchList(), closure );
	}
	catch (std::bad_alloc&) {}

	if (!pItem)
		ommServerBaseImpl.handleMee("Failed to create IProviderDictionaryItem");

	return pItem;
}

bool IProviderDictionaryItem::modify(const ReqMsg& reqMsg)
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>(reqMsg.getEncoder());

	reqMsgEncoder.getRsslRequestMsg()->msgBase.domainType = MMT_DICTIONARY;

	return ProviderItem::modify( reqMsg );
}
