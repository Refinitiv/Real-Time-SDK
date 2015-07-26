/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "DictionaryCallbackClient.h"
#include "ChannelCallbackClient.h"
#include "DirectoryCallbackClient.h"
#include "OmmConsumerImpl.h"
#include "Utilities.h"
#include "ReqMsg.h"
#include "ReqMsgEncoder.h"
#include "OmmConsumerClient.h"
#include "OmmConsumerErrorClient.h"
#include "StaticDecoder.h"

#include <new>

#define MAX_DICTIONARY_BUFFER_SIZE 16384

using namespace thomsonreuters::ema::access;

const EmaString DictionaryCallbackClient::_clientName( "DictionaryCallbackClient" );
const EmaString LocalDictionary::_clientName( "LocalDictionary" );
const EmaString ChannelDictionary::_clientName( "ChannelDictionary" );
const EmaString DictionaryItem::_clientName( "DictionaryItem" );

Dictionary::Dictionary() :
_fldStreamId( 0 ),
_enumStreamId( 0 )
{
}

Dictionary::~Dictionary()
{
}

Int32 Dictionary::getEnumStreamId()
{
	return _enumStreamId;
}

Int32 Dictionary::getFldStreamId()
{
	return _fldStreamId;
}

LocalDictionary* LocalDictionary::create( OmmConsumerImpl& ommConsImpl )
{
	LocalDictionary* pDictionary = 0;

	try {
		pDictionary = new LocalDictionary( ommConsImpl );
	}
	catch( std::bad_alloc ) {}

	if ( !pDictionary )
	{
		const char* temp = "Failed to create LocalDictionary";
		if ( OmmLoggerClient::ErrorEnum >= ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		throwMeeException( temp );
	}

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
	return Dictionary::FileDictionaryEnum;
}

LocalDictionary::LocalDictionary( OmmConsumerImpl& ommConsImpl ) :
 _ommConsImpl( ommConsImpl ),
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

bool LocalDictionary::load( const EmaString& fldName, const EmaString& enumName )
{
	char errTxt[256];
	RsslBuffer buffer;
	buffer.data = errTxt;
	buffer.length = 255;

	if ( rsslLoadFieldDictionary( fldName.c_str(), &_rsslDictionary, &buffer ) < 0 )
	{
		_isLoaded = false;

		rsslDeleteDataDictionary( &_rsslDictionary );

		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString errorText, dir;
			getCurrentDir( dir );
			errorText.set( "Unable to load RDMFieldDictionary from file named " ).append( fldName ).append( CR )
				.append( "Current working directory " ).append( dir ).append( CR )
				.append( "Error text " ).append( errTxt );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, errorText );
		}
		return false;
	}
			
	if ( rsslLoadEnumTypeDictionary( enumName.c_str(), &_rsslDictionary, &buffer ) < 0 )
	{
		_isLoaded = false;

		rsslDeleteDataDictionary( &_rsslDictionary );

		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString errorText, dir;
			getCurrentDir( dir );
			errorText.set( "Unable to load EnumTypeDef from file named " ).append( fldName ).append( CR )
				.append( "Current working directory " ).append( dir ).append( CR )
				.append( "Error text " ).append( errTxt );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, errorText );
		}
		return false;
	}

	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		EmaString temp( "Successfully loaded local dictionaries: " );
		temp.append( CR )
			.append( "RDMFieldDictionary file named " ).append( fldName ).append( CR )
			.append( "EnumTypeDef file named " ).append( enumName );
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
	}

	_isLoaded = true;

	return true;
}

ChannelDictionary* ChannelDictionary::create( OmmConsumerImpl& ommConsImpl )
{
	ChannelDictionary* pDictionary = 0;

	try {
		pDictionary = new ChannelDictionary( ommConsImpl );
	}
	catch( std::bad_alloc ) {}

	if ( !pDictionary )
	{
		const char* temp = "Failed to create ChannelDictionary";
		if ( OmmLoggerClient::ErrorEnum >= ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( ommConsImpl.hasOmmConnsumerErrorClient() )
			ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
		else
			throwMeeException( temp );
	}

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

ChannelDictionary::ChannelDictionary( OmmConsumerImpl& ommConsImpl ) :
 _ommConsImpl( ommConsImpl ),
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

RsslReactorCallbackRet ChannelDictionary::processCallback( RsslReactor* ,
															RsslReactorChannel* pRsslReactorChannel,
															RsslRDMDictionaryMsgEvent* pEvent )
{
	RsslRDMDictionaryMsg* pDictionaryMsg = pEvent->pRDMDictionaryMsg;

	if ( !pDictionaryMsg )
	{
		_ommConsImpl.closeChannel( pRsslReactorChannel );

		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			RsslErrorInfo* pError = pEvent->baseMsgEvent.pErrorInfo;

			EmaString temp( "Received event without RDMDictionary message");
			temp.append( CR ).append( "Channel " ).append( CR )
				.append( static_cast<Channel*>( pRsslReactorChannel->userSpecPtr)->toString() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( pError->rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( pError->rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( pError->rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( pError->errorLocation ).append( CR )
				.append( "Error Text " ).append( pError->rsslError.text );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
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
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );

				EmaString temp( "RDMDictionary stream was closed with refresh message" );
				temp.append( CR )
					.append( "Channel " ).append( CR )
					.append( static_cast<Channel*>( pRsslReactorChannel->userSpecPtr)->toString() ).append( CR )
					.append( "Reason " ).append( tempState );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
			}
			break;
		}
		else if ( pState->dataState == RSSL_DATA_SUSPECT )
		{
			if ( OmmLoggerClient::WarningEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );

				EmaString temp( "RDMDictionary stream state was changed to suspect with refresh message" );
				temp.append( CR ).append( "Channel " ).append( CR )
					.append( static_cast<Channel*>( pRsslReactorChannel->userSpecPtr)->toString() ).append( CR )
					.append( "Reason " ).append( tempState );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp.trimWhitespace() );
			}
			break;
		}

		if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString name( pRefresh->dictionaryName.data, pRefresh->dictionaryName.length );
			EmaString temp( "Received RDMDictionary refresh message" );
			temp.append( CR )
				.append( "Dictionary name " ).append( name ).append( CR )
				.append( "streamId " ).append( pRefresh->rdmMsgBase.streamId );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
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
						if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
						{
							EmaString temp( "Received RDMDictionary refresh message with FieldDefinitions but changed streamId" );
							temp.append( CR )
								.append( "Initial streamId " ).append( _fldStreamId ).append( CR )
								.append( "New streamId " ).append( pRefresh->rdmMsgBase.streamId );
							_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
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
						if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
						{
							EmaString temp( "Received RDMDictionary refresh message with EnumTables but changed streamId" );
							temp.append( CR )
								.append( "Initial streamId " ).append( _enumStreamId ).append( CR )
								.append( "New streamId " ).append( pRefresh->rdmMsgBase.streamId );
							_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
						}
						return RSSL_RC_CRET_SUCCESS;
					}
					break;
				}
				default: 
				{
					if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString temp( "Received RDMDictionary message with unknown dictionary type" );
						temp.append(CR)
							.append( "Dictionary type " ).append( pRefresh->type );
						_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
					}
					return RSSL_RC_CRET_SUCCESS;
				}
			}
		}

		RsslDecodeIterator dIter;
		rsslClearDecodeIterator( &dIter );
		
		if ( RSSL_RET_SUCCESS != rsslSetDecodeIteratorRWFVersion( &dIter, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion ) )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error: failed to set RWF Version while decoding dictionary" );
				temp.append( CR )
					.append("Trying to set " )
					.append( pRsslReactorChannel->majorVersion ).append( "." ).append( pRsslReactorChannel->minorVersion );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
			}
			return RSSL_RC_CRET_SUCCESS;
		}

		if ( RSSL_RET_SUCCESS != rsslSetDecodeIteratorBuffer( &dIter, &pRefresh->dataBody ) )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Internal error: failed to set iterator buffer while decoding dictionary" );
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

					if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString name( pRefresh->dictionaryName.data, pRefresh->dictionaryName.length );
						EmaString temp( "Received RDMDictionary refresh complete message" );
						temp.append( CR )
							.append( "dictionary name " ).append( name ).append( CR )
							.append( "streamId " ).append( pRefresh->rdmMsgBase.streamId );
						_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
					}
				}
				else
					_isFldLoaded = false;
			}
			else
    		{
				_isFldLoaded = false;

				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Internal error: failed to decode FieldDictionary" );
					temp.append( CR )
						.append("Error text " ).append( errTxt );
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
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

					if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					{
						EmaString name( pRefresh->dictionaryName.data, pRefresh->dictionaryName.length );
						EmaString temp( "Received RDMDictionary refresh complete message" );
						temp.append( CR )
							.append( "dictionary name " ).append( name ).append( CR )
							.append( "streamId " ).append( pRefresh->rdmMsgBase.streamId );
						_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
					}
				}
				else
					_isEnumLoaded = false;
			}
			else
    		{
				_isEnumLoaded = false;

				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Internal error: failed to decode EnumTable dictionary" );
					temp.append( CR )
						.append( "error text " ).append( errTxt );
    				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}
				return RSSL_RC_CRET_SUCCESS;
    		}
		}
		else
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received unexpected RDMDictionary refresh message on streamId " );
				temp.append( pRefresh->rdmMsgBase.streamId );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
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
				if ( OmmLoggerClient::WarningEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );

					EmaString temp( "RDMDictionary stream was closed with status message" );
					temp.append( CR )
						.append( "streamId " ).append( pDictionaryMsg->status.rdmMsgBase.streamId ).append( CR )
						.append( tempState );
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
				}

				notifyStatusToListener( pDictionaryMsg->status );
				break;
			}
			else if ( pState->dataState == RSSL_DATA_SUSPECT )
			{
				if ( OmmLoggerClient::WarningEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString tempState( 0, 256 );
					stateToString( pState, tempState );

					EmaString temp( "RDMDictionary stream state was changed to suspect with status message" );
					temp.append( CR )
						.append( "streamId " ).append( pDictionaryMsg->status.rdmMsgBase.streamId ).append( CR )
						.append( tempState );
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
				}

				notifyStatusToListener( pDictionaryMsg->status );
				break;
			}

			if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString tempState( 0, 256 );
				stateToString( pState, tempState );

				EmaString temp( "RDMDictionary stream was open with status message" );
				temp.append( CR )
					.append( "streamId " )
					.append( pDictionaryMsg->rdmMsgBase.streamId ).append( CR )
					.append( tempState );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp );
			}
		}
		else
		{
			if ( OmmLoggerClient::WarningEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Received RDMDictionary status message without the state" );
				temp.append( CR )
					.append( "streamId " )
					.append( pDictionaryMsg->status.rdmMsgBase.streamId );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::WarningEnum, temp );
			}
		}
		break;
	}
	default:
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Received unknown RDMDictionary message type" );
			temp.append( CR )
				.append( "message type " ).append( pDictionaryMsg->rdmMsgBase.rdmMsgType ).append( CR )
				.append( "streamId " ).append( pDictionaryMsg->rdmMsgBase.streamId );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
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
	msgBuf.length = 512;
	msgBuf.data = new char[msgBuf.length];
	RsslRet ret;

	for( UInt32 index = 0 ; index < _pListenerList->size(); index++ )
	{
		dictItem = _pListenerList->operator[](index);

		if ( dictItem->getStreamId() != status.rdmMsgBase.streamId )
			continue;

		rsslClearEncodeIterator( &encodeIter );
		statusMsg.msgBase.msgClass = RSSL_MC_STATUS;
		statusMsg.msgBase.streamId = dictItem->getStreamId();
		statusMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
		statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		statusMsg.flags = RSSL_STMF_HAS_STATE;
		statusMsg.state.streamState = status.state.streamState;
		statusMsg.state.dataState = status.state.dataState;
		statusMsg.state.code = status.state.code;
		statusMsg.state.text.data = status.state.text.data;
		statusMsg.state.text.length = status.state.text.length;

		if ( ( ret = rsslSetEncodeIteratorBuffer( &encodeIter, &msgBuf ) ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
					"Internal error. Failed to set encode iterator buffer in ChannelDictionary::notifyStatusToListener()" );

			delete[] msgBuf.data;
			_channelDictLock.unlock();
			return;
		}

		if ( ( ret = rsslSetEncodeIteratorRWFVersion( &encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
					"Internal error. Failed to set encode iterator RWF version in ChannelDictionary::notifyStatusToListener()" );

			delete[] msgBuf.data;
			_channelDictLock.unlock();
			return;
		}

		ret = rsslEncodeMsg( &encodeIter, (RsslMsg*)&statusMsg );
		while ( ret == RSSL_RET_BUFFER_TOO_SMALL )
		{
			delete[] msgBuf.data;
			msgBuf.length = msgBuf.length * 2;
			msgBuf.data = new char[msgBuf.length];

			ret = rsslSetEncodeIteratorBuffer( &encodeIter, &msgBuf );
			if ( ret != RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
						_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
						"Internal error. Failed to set encode iterator buffer in ChannelDictionary::notifyStatusToListener()" );

				delete[] msgBuf.data;
				_channelDictLock.unlock();
				return;
			}

			ret = rsslSetEncodeIteratorRWFVersion( &encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION );
			if ( ret != RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
						_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
						"Internal error. Failed to set encode iterator RWF version in ChannelDictionary::notifyStatusToListener()" );

				delete[] msgBuf.data;
				_channelDictLock.unlock();
				return;
			}

			if ( ( ret = rsslEncodeMsg( &encodeIter, (RsslMsg*)&statusMsg ) ) < RSSL_RET_SUCCESS)
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
					"Internal error. Failed to encode in ChannelDictionary::notifyStatusToListener()" );

				delete[] msgBuf.data;
				_channelDictLock.unlock();
				return;
			}
		}

		msgBuf.length = rsslGetEncodedBufferLength(&encodeIter);

		dictItem->getOmmConsumerImpl().getDictionaryCallbackClient().processStatusMsg( &msgBuf, 
			RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, dictItem );

		if ( status.state.streamState != RSSL_STREAM_OPEN )
			_pListenerList->removePosition( index );

		break;
	}

	delete[] msgBuf.data;

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

void ChannelDictionary::addListener( DictionaryItem* item)
{
	if ( _pListenerList == 0)
	{
		_pListenerList = new EmaVector<DictionaryItem*>( 2 );
	}

	_pListenerList->push_back( item );
}

void ChannelDictionary::removeListener( DictionaryItem* item)
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

DictionaryCallbackClient::DictionaryCallbackClient( OmmConsumerImpl& ommConsImpl ) :
 _channelDictionaryList(),
 _localDictionary( 0 ),
 _ommConsImpl( ommConsImpl ),
 _event(),
 _refreshMsg(),
 _statusMsg()
{
	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Created DictionaryCallbackClient" );
	}
}

DictionaryCallbackClient::~DictionaryCallbackClient()
{
	ChannelDictionary* dictionary = _channelDictionaryList.pop_back();
	while ( dictionary )
	{
		ChannelDictionary::destroy( dictionary );
		dictionary = _channelDictionaryList.pop_back();
	}

	LocalDictionary::destroy( _localDictionary );

	if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
	{
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, "Destroyed DictionaryCallbackClient" );
	}
}

DictionaryCallbackClient* DictionaryCallbackClient::create( OmmConsumerImpl& ommConsImpl )
{
	DictionaryCallbackClient* pClient = 0;

	try {
		pClient = new DictionaryCallbackClient( ommConsImpl );
	}
	catch ( std::bad_alloc ) {}

	if ( !pClient )
	{
		const char* temp = "Failed to create DictionaryCallbackClient";
		if ( OmmLoggerClient::ErrorEnum >= ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		throwMeeException( temp );
	}

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
	if ( _ommConsImpl.getActiveConfig().pRsslRdmFldRequestMsg && _ommConsImpl.getActiveConfig().pRsslEnumDefRequestMsg )
	{
		if ( _ommConsImpl.getActiveConfig().pRsslRdmFldRequestMsg->msgBase.msgKey.serviceId != 
			_ommConsImpl.getActiveConfig().pRsslEnumDefRequestMsg->msgBase.msgKey.serviceId )
		{
			EmaString temp( "Invalid dictionary configuration was specified through the OmmConsumerConfig::addAdminMsg()" );
			temp.append( CR ).append( "Invalid attempt to download dictionaries from different services." );

			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

			throwIueException( temp );

			return;
		}

		_ommConsImpl.getActiveConfig().dictionaryConfig.dictionaryType = Dictionary::ChannelDictionaryEnum;
		return;
	}
	else if ( _ommConsImpl.getActiveConfig().pRsslRdmFldRequestMsg && !_ommConsImpl.getActiveConfig().pRsslEnumDefRequestMsg )
	{
		EmaString temp( "Invalid dictionary configuration was specified through the OmmConsumerConfig::addAdminMsg()" );
		temp.append( CR ).append( "Enumeration type definition request message was not populated." );

		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		throwIueException( temp );
		return;
	}
	else if ( !_ommConsImpl.getActiveConfig().pRsslRdmFldRequestMsg && _ommConsImpl.getActiveConfig().pRsslEnumDefRequestMsg )
	{
		EmaString temp( "Invalid dictionary configuration was specified through the OmmConsumerConfig::addAdminMsg()" );
		temp.append( CR ).append( "RDM Field Dictionary request message was not populated." );
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		throwIueException( temp );
		return;
	}

	if ( _ommConsImpl.getActiveConfig().dictionaryConfig.dictionaryType == Dictionary::FileDictionaryEnum )
		loadDictionaryFromFile();
}

void DictionaryCallbackClient::loadDictionaryFromFile()
{
	_localDictionary = LocalDictionary::create( _ommConsImpl );

	_localDictionary->load( _ommConsImpl.getActiveConfig().dictionaryConfig.rdmfieldDictionaryFileName, _ommConsImpl.getActiveConfig().dictionaryConfig.enumtypeDefFileName );
}

bool DictionaryCallbackClient::downloadDictionary( const Directory& directory )
{
	if ( _ommConsImpl.getActiveConfig().pRsslRdmFldRequestMsg && _ommConsImpl.getActiveConfig().pRsslEnumDefRequestMsg )
	{
		if ( _ommConsImpl.getActiveConfig().pRsslRdmFldRequestMsg->msgBase.msgKey.serviceId == directory.getId() )
			downloadDictionaryFromService( directory );
		
		return true;
	}
	else if ( _localDictionary )
	{
		directory.getChannel()->setDictionary( _localDictionary );
		return true;
	}
	else if ( _ommConsImpl.getActiveConfig().dictionaryConfig.dictionaryType == Dictionary::ChannelDictionaryEnum )
	{
		if ( directory.getChannel()->getDictionary() )
			return true;
	}

	RsslRequestMsg requestMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;

	rsslClearRequestMsg( &requestMsg );
	requestMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER;
	requestMsg.msgBase.msgKey.filter = RDM_DICTIONARY_NORMAL;
	requestMsg.flags = RSSL_RQMF_STREAMING;

	ChannelDictionary* pDictionary = ChannelDictionary::create( _ommConsImpl );

	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = (RsslMsg*)&requestMsg;
	RsslBuffer serviceName;
	serviceName.data = (char*)directory.getName().c_str();
	serviceName.length = directory.getName().length();
	submitMsgOpts.pServiceName = &serviceName;
	submitMsgOpts.majorVersion = directory.getChannel()->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = directory.getChannel()->getRsslChannel()->minorVersion;
	submitMsgOpts.requestMsgOptions.pUserSpec = (void*)pDictionary;

	const EmaVector< EmaString > dictionariesUsed = directory.getInfo().getDictionariesUsed().getDictionaryList();

	Int32 streamId = 3;

	for ( UInt32 idx = 0; idx < dictionariesUsed.size(); ++idx )
	{
		requestMsg.msgBase.msgKey.name.data = (char*)dictionariesUsed[idx].c_str();
		requestMsg.msgBase.msgKey.name.length = dictionariesUsed[idx].length();
		requestMsg.msgBase.streamId = streamId++;

		if ( ( ret = rsslReactorSubmitMsg( directory.getChannel()->getRsslReactor(),
											directory.getChannel()->getRsslChannel(),
											&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Internal error: rsslReactorSubmitMsg() failed" );
				temp.append( CR ).append( directory.getChannel()->toString() ).append( CR )
					.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
					.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
					.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
					.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
					.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

				ChannelDictionary::destroy( pDictionary );
			}
			return false;
		}
		else
		{
			if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			{
				EmaString temp( "Requested Dictionary " );
				temp.append( dictionariesUsed[idx] ).append( CR )
					.append( "from Service " ).append( directory.getName() ).append( CR )
					.append( "on Channel " ).append( CR )
					.append( directory.getChannel()->toString() );
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp.trimWhitespace() );
			}
		}
	}

	_channelDictionaryList.push_back( pDictionary );

	if ( _ommConsImpl.getActiveConfig().dictionaryConfig.dictionaryType == Dictionary::ChannelDictionaryEnum )
		directory.getChannel()->setDictionary( pDictionary );

	return true;
}

Dictionary* DictionaryCallbackClient::getDefaultDictionary() const
{
	if ( _channelDictionaryList.front() )
		return _channelDictionaryList.front();
	else
		return _localDictionary;
}

bool DictionaryCallbackClient::downloadDictionaryFromService( const Directory& directory )
{
	RsslRequestMsg requestMsg;
	RsslReactorSubmitMsgOptions submitMsgOpts;
	RsslRet ret;
	RsslErrorInfo rsslErrorInfo;

	rsslClearRequestMsg( &requestMsg );
	requestMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER;
	requestMsg.msgBase.msgKey.filter = _ommConsImpl.getActiveConfig().pRsslRdmFldRequestMsg->msgBase.msgKey.filter;

	if ( _ommConsImpl.getActiveConfig().pRsslRdmFldRequestMsg->flags & RSSL_RQMF_STREAMING )
		requestMsg.flags = RSSL_RQMF_STREAMING;

	ChannelDictionary* pDictionary = ChannelDictionary::create( _ommConsImpl );

	rsslClearReactorSubmitMsgOptions( &submitMsgOpts );
	submitMsgOpts.pRsslMsg = (RsslMsg*)&requestMsg;
	RsslBuffer serviceName;
	serviceName.data = (char*)directory.getName().c_str();
	serviceName.length = directory.getName().length();
	submitMsgOpts.pServiceName = &serviceName;
	submitMsgOpts.majorVersion = directory.getChannel()->getRsslChannel()->majorVersion;
	submitMsgOpts.minorVersion = directory.getChannel()->getRsslChannel()->minorVersion;
	submitMsgOpts.requestMsgOptions.pUserSpec = (void*)pDictionary;

	requestMsg.msgBase.msgKey.name = _ommConsImpl.getActiveConfig().pRsslRdmFldRequestMsg->msgBase.msgKey.name;

	requestMsg.msgBase.streamId = 3;

	if ( ( ret = rsslReactorSubmitMsg( directory.getChannel()->getRsslReactor(),
										directory.getChannel()->getRsslChannel(),
										&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: rsslReactorSubmitMsg() failed" );
			temp.append( CR ).append( "Channel" ).append( CR )
				.append( directory.getChannel()->toString() ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );

			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );

			ChannelDictionary::destroy( pDictionary );
		}
		return false;
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString dictionaryName( _ommConsImpl.getActiveConfig().pRsslRdmFldRequestMsg->msgBase.msgKey.name.data,
				_ommConsImpl.getActiveConfig().pRsslRdmFldRequestMsg->msgBase.msgKey.name.length );

			EmaString temp( "Requested Dictionary " );
			temp.append( dictionaryName ).append( CR )
				.append( "from Service " ).append( directory.getName() ).append( CR )
				.append( "on Channel" ).append( CR )
				.append( directory.getChannel()->toString() );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp.trimWhitespace() );
		}
	}

	rsslClearRequestMsg( &requestMsg );
	requestMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
	requestMsg.msgBase.containerType = RSSL_DT_NO_DATA;
	requestMsg.msgBase.msgKey.flags = RSSL_MKF_HAS_NAME | RSSL_MKF_HAS_FILTER;
	requestMsg.msgBase.msgKey.filter = _ommConsImpl.getActiveConfig().pRsslEnumDefRequestMsg->msgBase.msgKey.filter;

	if ( _ommConsImpl.getActiveConfig().pRsslEnumDefRequestMsg->flags & RSSL_RQMF_STREAMING )
		requestMsg.flags = RSSL_RQMF_STREAMING;

	requestMsg.msgBase.msgKey.name = _ommConsImpl.getActiveConfig().pRsslEnumDefRequestMsg->msgBase.msgKey.name;

	requestMsg.msgBase.streamId = 4;

	if ( ( ret = rsslReactorSubmitMsg( directory.getChannel()->getRsslReactor(),
										directory.getChannel()->getRsslChannel(),
										&submitMsgOpts, &rsslErrorInfo ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error. rsslReactorSubmitMsg() failed" );
			temp.append( CR ).append( "Channel " ).append( CR )
			    .append( directory.getChannel()->toString() ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error text " ).append( rsslErrorInfo.rsslError.text );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );

			ChannelDictionary::destroy( pDictionary );
		}
		return false;
	}
	else
	{
		if ( OmmLoggerClient::VerboseEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString dictionaryName( _ommConsImpl.getActiveConfig().pRsslEnumDefRequestMsg->msgBase.msgKey.name.data,
				_ommConsImpl.getActiveConfig().pRsslEnumDefRequestMsg->msgBase.msgKey.name.length );
			EmaString temp( "Requested Dictionary " );
			temp.append( dictionaryName ).append( CR )
				.append( "from Service " ).append( directory.getName() ).append( CR )
				.append( "on Channel " ).append( CR )
				.append( directory.getChannel()->toString() );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::VerboseEnum, temp.trimWhitespace() );
		}
	}

	_channelDictionaryList.push_back( pDictionary );

	if ( _ommConsImpl.getActiveConfig().dictionaryConfig.dictionaryType == Dictionary::ChannelDictionaryEnum )
		directory.getChannel()->setDictionary( pDictionary );

	return true;
}

RsslReactorCallbackRet DictionaryCallbackClient::processCallback( RsslReactor* pRsslReactor,
															RsslReactorChannel* pRsslReactorChannel,
															RsslRDMDictionaryMsgEvent* pEvent )
{

	ChannelDictionary* channelDict = _channelDictionaryList.front();
	bool isChannelDict = false;

	while( channelDict )
	{
		if ( channelDict ==  (ChannelDictionary*)pEvent->baseMsgEvent.pStreamInfo->pUserSpec )
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
		return processCallback( pRsslReactor, pRsslReactorChannel, pEvent, static_cast<DictionaryItem*>(pEvent->baseMsgEvent.pStreamInfo->pUserSpec) );
	}
}

RsslReactorCallbackRet DictionaryCallbackClient::processCallback( RsslReactor* pRsslReactor, RsslReactorChannel* pRsslReactorChannel,
																 RsslRDMDictionaryMsgEvent* pEvent, DictionaryItem* pItem )
{
	if ( !pEvent->pRDMDictionaryMsg )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
			"Internal error. Received RsslRDMDictionaryMsgEvent with no RsslRDMDictionaryMsg in DictionaryCallbackClient::processCallback()" );
		return RSSL_RC_CRET_SUCCESS;
	}

	RsslBuffer rsslMsgBuffer;
	rsslMsgBuffer.length = MAX_DICTIONARY_BUFFER_SIZE;
	rsslMsgBuffer.data = (char*)malloc( sizeof( char ) * rsslMsgBuffer.length );

	if ( !rsslMsgBuffer.data )
	{
		const char* temp = "Failed to allocate memory in DictionaryCallbackClient::processCallback()";
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
			_ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
		else
			throwMeeException( temp );

		free( rsslMsgBuffer.data );
		return RSSL_RC_CRET_SUCCESS;
	}

	RsslEncIterator eIter;
	rsslClearEncodeIterator( &eIter );

	RsslRet retCode = rsslSetEncodeIteratorRWFVersion( &eIter, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
			"Internal error. Failed to set encode iterator version in DictionaryCallbackClient::processCallback()" );
		
		free( rsslMsgBuffer.data );
		return RSSL_RC_CRET_SUCCESS;
	}

	retCode = rsslSetEncodeIteratorBuffer( &eIter, &rsslMsgBuffer );
	if ( retCode != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
			"Internal error. Failed to set encode iterator buffer in DictionaryCallbackClient::processCallback()" );

		free( rsslMsgBuffer.data );
		return RSSL_RC_CRET_SUCCESS;
	}

	RsslErrorInfo rsslErrorInfo;
	char errorBuf[255];
	RsslBuffer errorText = { 255, errorBuf };
	RsslRet ret = RSSL_RET_SUCCESS;
	UInt32 completeFlag  = 0;

	switch ( pEvent->pRDMDictionaryMsg->rdmMsgBase.rdmMsgType )
	{
		case RDM_DC_MT_REFRESH:
			RsslDecodeIterator dIter;
			rsslClearDecodeIterator( &dIter );
			RsslDataDictionary rsslDataDictionary;
			rsslClearDataDictionary( &rsslDataDictionary );
			if ( RSSL_RET_SUCCESS != rsslSetDecodeIteratorRWFVersion( &dIter, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion ) )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Internal error: failed to set RWF Version while decoding dictionary" );
					temp.append( CR )
						.append("Trying to set " )
						.append( pRsslReactorChannel->majorVersion ).append( "." ).append( pRsslReactorChannel->minorVersion );
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}

				free( rsslMsgBuffer.data );
				return RSSL_RC_CRET_SUCCESS;
			}

			if ( RSSL_RET_SUCCESS != rsslSetDecodeIteratorBuffer( &dIter, & pEvent->pRDMDictionaryMsg->refresh.dataBody ) )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, "Internal error: failed to set iterator buffer while decoding dictionary" );
				}

				free( rsslMsgBuffer.data );
				return RSSL_RC_CRET_SUCCESS;
			}

			if ( pItem->getName() == "RWFFld" )
			{
				ret = rsslDecodeFieldDictionary( &dIter, &rsslDataDictionary, (RDMDictionaryVerbosityValues)pItem->getFilter(), &errorText );

				if ( pEvent->pRDMDictionaryMsg->refresh.type == 0 )
					pEvent->pRDMDictionaryMsg->refresh.type = 1;
			}
			else if ( pItem->getName() == "RWFEnum" )
			{
				ret = rsslDecodeEnumTypeDictionary( &dIter, &rsslDataDictionary, (RDMDictionaryVerbosityValues)pItem->getFilter(), &errorText );

				if ( pEvent->pRDMDictionaryMsg->refresh.type == 0 )
					pEvent->pRDMDictionaryMsg->refresh.type = 2;
			}

			if ( ret != RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				{
					EmaString temp( "Internal error: failed to decode data dictionary" );
					temp.append( CR )
						.append("Trying to set " )
						.append( pRsslReactorChannel->majorVersion ).append( "." ).append( pRsslReactorChannel->minorVersion );
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );
				}

				free( rsslMsgBuffer.data );
				return RSSL_RC_CRET_SUCCESS;
			}

			completeFlag = pEvent->pRDMDictionaryMsg->refresh.flags & RDM_DC_RFF_IS_COMPLETE;
			pEvent->pRDMDictionaryMsg->refresh.pDictionary = &rsslDataDictionary;
			pEvent->pRDMDictionaryMsg->refresh.flags &= ~( RDM_DC_RFF_IS_COMPLETE | RDM_DC_RFF_HAS_INFO );

			retCode = rsslEncodeRDMDictionaryMsg( &eIter, pEvent->pRDMDictionaryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );

			while ( retCode == RSSL_RET_BUFFER_TOO_SMALL || retCode == RSSL_RET_DICT_PART_ENCODED )
			{
				if ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
				{
					free( rsslMsgBuffer.data );

					rsslMsgBuffer.length += rsslMsgBuffer.length;
					rsslMsgBuffer.data = (char*)malloc( sizeof( char ) * rsslMsgBuffer.length );
			
					if ( !rsslMsgBuffer.data )
					{
						const char* temp = "Failed to allocate memory in DictionaryCallbackClient::processCallback()";
						if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
							_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

						if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
							_ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
						else
							throwMeeException( temp );

						return RSSL_RC_CRET_SUCCESS;
					}
				}
				else 
				{
					if ( processRefreshMsg( &rsslMsgBuffer, completeFlag,  pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion, pItem ) !=  RSSL_RC_CRET_SUCCESS )
					{
						if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
							_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
							"Internal error. Failed to set encode refresh message in DictionaryCallbackClient::processCallback()" );

						free( rsslMsgBuffer.data );
						return RSSL_RC_CRET_SUCCESS;
					}
				}

				retCode = rsslSetEncodeIteratorBuffer( &eIter, &rsslMsgBuffer );
				if ( retCode != RSSL_RET_SUCCESS )
				{
					if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
							_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
							"Internal error. Failed to set encode iterator buffer in DictionaryCallbackClient::processCallback()" );

							free( rsslMsgBuffer.data );
							return RSSL_RC_CRET_SUCCESS;
				}

				ret = rsslSetEncodeIteratorRWFVersion( &eIter, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion );
				if ( ret != RSSL_RET_SUCCESS )
				{
					if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
							_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
							"Internal error. Failed to set encode iterator RWF version in DictionaryCallbackClient::processCallback()" );

					free( rsslMsgBuffer.data );
					return RSSL_RC_CRET_SUCCESS;
				}

				retCode = rsslEncodeRDMDictionaryMsg( &eIter, pEvent->pRDMDictionaryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );
			}

			if ( retCode == RSSL_RET_SUCCESS )
			{
				if ( processRefreshMsg( &rsslMsgBuffer, completeFlag,  pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion, pItem ) !=  RSSL_RC_CRET_SUCCESS )
					{
						if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
							_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
							"Internal error. Failed to set encode refresh message in DictionaryCallbackClient::processCallback()" );

						free( rsslMsgBuffer.data );
						return RSSL_RC_CRET_SUCCESS;
					}
			}

			break;

		case RDM_DC_MT_STATUS:
			retCode = rsslEncodeRDMDictionaryMsg( &eIter, pEvent->pRDMDictionaryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );
			while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
			{
				free( rsslMsgBuffer.data );

				rsslMsgBuffer.length += rsslMsgBuffer.length;
				rsslMsgBuffer.data = (char*)malloc( sizeof( char ) * rsslMsgBuffer.length );
			
				if ( !rsslMsgBuffer.data )
				{
					const char* temp = "Failed to allocate memory in DictionaryCallbackClient::processCallback()";
					if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
						_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

					if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
						_ommConsImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
					else
						throwMeeException( temp );

					return RSSL_RC_CRET_SUCCESS;
				}
		
				retCode = rsslEncodeRDMDictionaryMsg( &eIter, pEvent->pRDMDictionaryMsg, &rsslMsgBuffer.length, &rsslErrorInfo );
			}
			
			if ( retCode == RSSL_RET_SUCCESS )
			{
				processStatusMsg( &rsslMsgBuffer, pRsslReactorChannel->majorVersion, pRsslReactorChannel->minorVersion, pItem );
			}

		break;
		default :
		{
			if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
				_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
				"Internal error. Received unexpected type of RsslRDMDictionaryMsg in DictionaryCallbackClient::processCallback()" );
			break;
		}
	}

	if ( retCode != RSSL_RET_SUCCESS && retCode != RSSL_RET_DICT_PART_ENCODED)
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		{
			EmaString temp( "Internal error: failed to encode RsslRDMDictionaryMsg in DictionaryCallbackClient::processCallback()" );
			temp.append( CR )
				.append( "RsslReactor " ).append( ptrToStringAsHex( pRsslReactor ) ).append( CR )
				.append( "RsslChannel " ).append( ptrToStringAsHex( rsslErrorInfo.rsslError.channel ) ).append( CR )
				.append( "Error Id " ).append( rsslErrorInfo.rsslError.rsslErrorId ).append( CR )
				.append( "Internal sysError " ).append( rsslErrorInfo.rsslError.sysError ).append( CR )
				.append( "Error Location " ).append( rsslErrorInfo.errorLocation ).append( CR )
				.append( "Error Text " ).append( rsslErrorInfo.rsslError.text );
			_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp.trimWhitespace() );
		}

		free( rsslMsgBuffer.data );
		return RSSL_RC_CRET_SUCCESS;
	}

	free( rsslMsgBuffer.data );
	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet DictionaryCallbackClient::processRefreshMsg( RsslBuffer* msgBuf, UInt32 completeFlag,
																	UInt8 majorVersion, UInt8 minorVersion,
																	DictionaryItem* item )
{
	RsslDecodeIterator rsslDecodeIterator;
	RsslRefreshMsg rsslRefreshMsg;
	RsslRet ret = RSSL_RET_SUCCESS;

	rsslClearDecodeIterator( &rsslDecodeIterator );
	rsslClearRefreshMsg( &rsslRefreshMsg );

	if ( ( ret = rsslSetDecodeIteratorBuffer( &rsslDecodeIterator, msgBuf ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
					"Internal error. Failed to set encode iterator buffer in DictionaryCallbackClient::processRefreshMsg()" );

		return RSSL_RC_CRET_FAILURE;
	}

	if ( ( ret = rsslSetDecodeIteratorRWFVersion( &rsslDecodeIterator, majorVersion, minorVersion ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
					"Internal error. Failed to set encode iterator RWF version in DictionaryCallbackClient::processRefreshMsg()" );

		return RSSL_RC_CRET_FAILURE;
	}

	if ( ( ret = rsslDecodeMsg( &rsslDecodeIterator, (RsslMsg*)&rsslRefreshMsg ) ) != RSSL_RET_SUCCESS )
	{
		if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
					_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
					"Internal error. Failed to decode message in DictionaryCallbackClient::processRefreshMsg()" );

		return RSSL_RC_CRET_FAILURE;
	}

	if ( completeFlag )
	{
		rsslRefreshMsg.flags |= RSSL_RFMF_REFRESH_COMPLETE;
	}
	else
	{
		rsslRefreshMsg.flags &= ~RSSL_RFMF_REFRESH_COMPLETE;
	}

	StaticDecoder::setRsslData( &_refreshMsg, (RsslMsg*)&rsslRefreshMsg, majorVersion, minorVersion, 0 );

	_event._pItem = (Item*)( item );
	
	_event._pItem->getClient().onAllMsg( _refreshMsg, _event );
	_event._pItem->getClient().onRefreshMsg( _refreshMsg, _event );

	if ( _refreshMsg.getState().getStreamState() == OmmState::NonStreamingEnum )
	{
		if ( _refreshMsg.getComplete() )
			_event._pItem->remove();
	}
	else if ( _refreshMsg.getState().getStreamState() != OmmState::OpenEnum )
	{
		_event._pItem->remove();
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet DictionaryCallbackClient::processRefreshMsg( RsslBuffer* msgBuf, UInt8 majorVersion, UInt8 minorVersion, DictionaryItem* item )
{
	StaticDecoder::setRsslData( &_refreshMsg, msgBuf, RSSL_DT_MSG, majorVersion, minorVersion, 0 );

	_event._pItem = (Item*)( item );
	
	_event._pItem->getClient().onAllMsg( _refreshMsg, _event );
	_event._pItem->getClient().onRefreshMsg( _refreshMsg, _event );

	if ( _refreshMsg.getState().getStreamState() == OmmState::NonStreamingEnum )
	{
		if ( _refreshMsg.getComplete() )
			_event._pItem->remove();
	}
	else if ( _refreshMsg.getState().getStreamState() != OmmState::OpenEnum )
	{
		_event._pItem->remove();
	}

	return RSSL_RC_CRET_SUCCESS;
}

RsslReactorCallbackRet DictionaryCallbackClient::processStatusMsg( RsslBuffer* msgBuf, UInt8 majorVersion, UInt8 minorVersion, DictionaryItem* item )
{
	StaticDecoder::setRsslData( &_statusMsg, msgBuf, RSSL_DT_MSG, majorVersion, minorVersion, 0 );

	_event._pItem = (Item*)( item );
	
	_event._pItem->getClient().onAllMsg( _statusMsg, _event );
	_event._pItem->getClient().onStatusMsg( _statusMsg, _event );

	if ( _statusMsg.getState().getStreamState() != OmmState::OpenEnum )
		_event._pItem->remove();

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
	DictionaryItem* dictItem = reinterpret_cast<DictionaryItem*>(item);

	if ( dictItem->isRemoved() )
		return;

	Dictionary* dictionary = dictItem->getOmmConsumerImpl().getDictionaryCallbackClient().getDefaultDictionary();

	RsslBuffer msgBuf;
	bool firstPart = false;
	RsslRet ret = RSSL_RET_SUCCESS;
	RsslRefreshMsg refreshMsg = RSSL_INIT_REFRESH_MSG;
	RsslDecodeIterator decodeIter = RSSL_INIT_DECODE_ITERATOR;

	msgBuf.data = new char[MAX_DICTIONARY_BUFFER_SIZE];
	msgBuf.length = MAX_DICTIONARY_BUFFER_SIZE;

	if ( dictionary->isLoaded() )
	{
		const RsslDataDictionary* rsslDataDictionary = dictionary->getRsslDictionary();
		RsslInt32 dictionaryFid = dictItem->getCurrentFid();

		if ( dictItem->getName() == "RWFFld" )
		{
			if ( dictionaryFid == rsslDataDictionary->minFid )
				firstPart = true;

			ret = DictionaryItem::encodeDataDictionaryResp( msgBuf, dictItem->getName(), dictItem->getFilter(), dictItem->getStreamId() ,
				firstPart, const_cast<RsslDataDictionary*>(rsslDataDictionary), dictionaryFid );
		}
		else if ( dictItem->getName() == "RWFEnum" )
		{
			if ( dictionaryFid== 0 )
				firstPart = true;

			ret = DictionaryItem::encodeDataDictionaryResp( msgBuf, dictItem->getName(), dictItem->getFilter(), dictItem->getStreamId(),
				firstPart, const_cast<RsslDataDictionary*>(rsslDataDictionary), dictionaryFid );
		}

		if ( ( ret == RSSL_RET_SUCCESS ) || ( ret == RSSL_RET_DICT_PART_ENCODED ) )
		{
			dictItem->setCurrentFid( dictionaryFid );
			dictItem->getOmmConsumerImpl().getDictionaryCallbackClient().processRefreshMsg( &msgBuf,
				RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, dictItem );
		}

		if ( ret == RSSL_RET_DICT_PART_ENCODED )
		{
			new TimeOut( dictItem->getOmmConsumerImpl(), 500, &DictionaryCallbackClient::sendInternalMsg, dictItem );
			return;
		}
		
		if ( ret != RSSL_RET_SUCCESS )
		{
			RsslStatusMsg statusMsg = RSSL_INIT_STATUS_MSG;
			char stateText[128];
			RsslEncodeIterator encodeIter;

			/* clear encode iterator */
			rsslClearEncodeIterator(&encodeIter);

			/* set-up message */
			statusMsg.msgBase.msgClass = RSSL_MC_STATUS;
			statusMsg.msgBase.streamId = dictItem->getStreamId();
			statusMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
			statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
			statusMsg.flags = RSSL_STMF_HAS_STATE;
			statusMsg.state.streamState = RSSL_STREAM_CLOSED;
			statusMsg.state.dataState = RSSL_DATA_SUSPECT;
			statusMsg.state.code = RSSL_SC_NONE;
			snprintf(stateText, 128, "Failed to provide data dictionary: Internal error.");
			statusMsg.state.text.data = stateText;
			statusMsg.state.text.length = (RsslUInt32)strlen(stateText) + 1;

			if ( ( ret = rsslSetEncodeIteratorBuffer( &encodeIter, &msgBuf ) ) != RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= dictItem->getOmmConsumerImpl().getActiveConfig().loggerConfig.minLoggerSeverity )
				dictItem->getOmmConsumerImpl().getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
				"Internal error. Failed to set encode iterator buffer in DictionaryCallbackClient::sendInternalMsg()" );
				
				delete[] msgBuf.data;
				return;
			}

			if ( ( ret = rsslSetEncodeIteratorRWFVersion( &encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) ) != RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= dictItem->getOmmConsumerImpl().getActiveConfig().loggerConfig.minLoggerSeverity )
				dictItem->getOmmConsumerImpl().getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
				"Internal error. Failed to set encode iterator RWF version in DictionaryCallbackClient::sendInternalMsg()" );

				delete[] msgBuf.data;
				return;
			}

			if ( ( ret = rsslEncodeMsg( &encodeIter, (RsslMsg*)&statusMsg ) ) != RSSL_RET_SUCCESS )
			{
				if ( OmmLoggerClient::ErrorEnum >= dictItem->getOmmConsumerImpl().getActiveConfig().loggerConfig.minLoggerSeverity )
				dictItem->getOmmConsumerImpl().getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
				"Internal error. Failed to encode msg in DictionaryCallbackClient::sendInternalMsg()" );

				delete[] msgBuf.data;
				return;
			}

			msgBuf.length = rsslGetEncodedBufferLength(&encodeIter);

			dictItem->getOmmConsumerImpl().getDictionaryCallbackClient().processStatusMsg( &msgBuf,
				RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, dictItem );

			delete[] msgBuf.data;
		}
	}
	else
	{
		RsslStatusMsg statusMsg = RSSL_INIT_STATUS_MSG;
		char stateText[128];
		RsslEncodeIterator encodeIter;

		/* clear encode iterator */
		rsslClearEncodeIterator(&encodeIter);

		/* set-up message */
		statusMsg.msgBase.msgClass = RSSL_MC_STATUS;
		statusMsg.msgBase.streamId = dictItem->getStreamId();
		statusMsg.msgBase.domainType = RSSL_DMT_DICTIONARY;
		statusMsg.msgBase.containerType = RSSL_DT_NO_DATA;
		statusMsg.flags = RSSL_STMF_HAS_STATE;
		statusMsg.state.streamState = RSSL_STREAM_CLOSED_RECOVER;
		statusMsg.state.dataState = RSSL_DATA_SUSPECT;
		statusMsg.state.code = RSSL_SC_NONE;
		snprintf(stateText, 128, "Data dictionary is not ready to provide.");
		statusMsg.state.text.data = stateText;
		statusMsg.state.text.length = (RsslUInt32)strlen(stateText) + 1;

		if ( ( ret = rsslSetEncodeIteratorBuffer( &encodeIter, &msgBuf ) ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= dictItem->getOmmConsumerImpl().getActiveConfig().loggerConfig.minLoggerSeverity )
					dictItem->getOmmConsumerImpl().getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
						"Internal error. Failed to set encode iterator buffer in DictionaryCallbackClient::sendInternalMsg()" );
			
			delete[] msgBuf.data;
			return;
		}

		if ( ( ret = rsslSetEncodeIteratorRWFVersion( &encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) ) != RSSL_RET_SUCCESS )
		{
			if ( OmmLoggerClient::ErrorEnum >= dictItem->getOmmConsumerImpl().getActiveConfig().loggerConfig.minLoggerSeverity )
					dictItem->getOmmConsumerImpl().getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
						"Internal error. Failed to set encode iterator RWF version in DictionaryCallbackClient::sendInternalMsg()" );
			
			delete[] msgBuf.data;
			return;
		}

		if ( ( ret = rsslEncodeMsg( &encodeIter, (RsslMsg*)&statusMsg ) ) != RSSL_RET_SUCCESS)
		{
			if ( OmmLoggerClient::ErrorEnum >= dictItem->getOmmConsumerImpl().getActiveConfig().loggerConfig.minLoggerSeverity )
					dictItem->getOmmConsumerImpl().getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum,
						"Internal error. Failed to encode msg in DictionaryCallbackClient::sendInternalMsg()" );
			
			delete[] msgBuf.data;
			return;
		}

		msgBuf.length = rsslGetEncodedBufferLength(&encodeIter);

		dictItem->getOmmConsumerImpl().getDictionaryCallbackClient().processStatusMsg( &msgBuf,
			RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, dictItem );
	
		delete[] msgBuf.data;
		return;
	}
}

DictionaryItem::DictionaryItem( OmmConsumerImpl& ommConsumerImpl, OmmConsumerClient& ommConsumerClient, void* closure ) :
 SingleItem( ommConsumerImpl, ommConsumerClient, closure, 0 ),
 _name(),
 _currentFid( 0 ),
 _filter ( 0 ),
 _isRemoved( false )
{
}

DictionaryItem::~DictionaryItem()
{
}

DictionaryItem* DictionaryItem::create( OmmConsumerImpl& ommConsumerImpl, OmmConsumerClient& ommConsumerClient, void* closure )
{
	DictionaryItem* pItem = 0;

	try {
		pItem = new DictionaryItem( ommConsumerImpl, ommConsumerClient, closure );
	}
	catch( std::bad_alloc ) {}

	if ( !pItem )
	{
		const char* temp = "Failed to create DictionaryItem";
		if ( OmmLoggerClient::ErrorEnum >= ommConsumerImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
			ommConsumerImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

		if ( ommConsumerImpl.hasOmmConnsumerErrorClient() )
			ommConsumerImpl.getOmmConsumerErrorClient().onMemoryExhaustion( temp );
		else
			throwMeeException( temp );
	}

	return pItem;
}

bool DictionaryItem::isRemoved()
{
	return _isRemoved;
}

bool DictionaryItem::open( const ReqMsg& reqMsg )
{
	const ReqMsgEncoder& reqMsgEncoder = static_cast<const ReqMsgEncoder&>( reqMsg.getEncoder() );

	_name = EmaString(reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.data, reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.name.length );

	if ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_FILTER )
		_filter = reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.filter;

	if ( reqMsgEncoder.hasServiceName() || ( reqMsgEncoder.getRsslRequestMsg()->msgBase.msgKey.flags & RSSL_MKF_HAS_SERVICE_ID ) )
	{
		return SingleItem::open( reqMsg );
	}
	else
	{
		if ( _name == "RWFFld" )
		{
			_currentFid = _ommConsImpl.getDictionaryCallbackClient().getDefaultDictionary()->getRsslDictionary()->minFid;
			_streamId = _ommConsImpl.getDictionaryCallbackClient().getDefaultDictionary()->getFldStreamId();
		}
		else if ( _name == "RWFEnum" )
		{
			_currentFid = 0;
			_streamId = _ommConsImpl.getDictionaryCallbackClient().getDefaultDictionary()->getEnumStreamId();
		}

		Dictionary* dictionary = _ommConsImpl.getDictionaryCallbackClient().getDefaultDictionary();

		if ( dictionary )
		{
			if ( dictionary->getType() == Dictionary::ChannelDictionaryEnum )
			{
				ChannelDictionary* channelDict = static_cast<ChannelDictionary*>(dictionary);

				channelDict->acquireLock();

				channelDict->addListener( this );

				channelDict->releaseLock();
			}

			new TimeOut( _ommConsImpl, 500, &DictionaryCallbackClient::sendInternalMsg, this );

			return true;
		}
		else
		{
			return false;
		}
	}
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
	temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

	if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

	if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
		_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( temp );
	else
		throwIueException( temp );

	return false;
}

bool DictionaryItem::submit( const PostMsg& )
{
	EmaString temp( "Invalid attempt to submit PostMsg on dictionary stream. " );
	temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

	if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

	if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
		_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( temp );
	else
		throwIueException( temp );

	return false;
}

bool DictionaryItem::submit( const GenericMsg& )
{
	EmaString temp( "Invalid attempt to submit GenericMsg on dictionary stream. " );
	temp.append( "OmmConsumer name='" ).append( _ommConsImpl .getConsumerName() ).append( "'." );

	if ( OmmLoggerClient::ErrorEnum >= _ommConsImpl.getActiveConfig().loggerConfig.minLoggerSeverity )
		_ommConsImpl.getOmmLoggerClient().log( _clientName, OmmLoggerClient::ErrorEnum, temp );

	if ( _ommConsImpl.hasOmmConnsumerErrorClient() )
		_ommConsImpl.getOmmConsumerErrorClient().onInvalidUsage( temp );
	else
		throwIueException( temp );

	return false;
}

void DictionaryItem::remove()
{
	if ( !_isRemoved )
	{
		_isRemoved = true;

		new TimeOut( getOmmConsumerImpl(), 2000, &DictionaryItem::ScheduleRemove, this );
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

			Dictionary* dictionary = _ommConsImpl.getDictionaryCallbackClient().getDefaultDictionary();

			if ( dictionary->getType() == Dictionary::ChannelDictionaryEnum )
			{
				ChannelDictionary* channelDict = static_cast<ChannelDictionary*>(dictionary);

				channelDict->acquireLock();

				channelDict->removeListener( this );

				channelDict->releaseLock();
			}

			new TimeOut( this->getOmmConsumerImpl(), 2000, &DictionaryItem::ScheduleRemove, this );
		}
	}

	return true;
}

void DictionaryItem::ScheduleRemove( void* item )
{
	delete (DictionaryItem*)item;
}

RsslRet DictionaryItem::encodeDataDictionaryResp( RsslBuffer& msgBuf, const EmaString& name, unsigned char filter, RsslInt32 streamId,
												 bool firstMultiRefresh, RsslDataDictionary* rsslDataDictionary, Int32& currentFid )
{
	RsslRet ret = RSSL_RET_SUCCESS;
	char errTxt[256];
	RsslBuffer errorText = {255, (char*)errTxt};
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

	refreshMsg.msgBase.msgKey.name.data = (char*)name.c_str();
	refreshMsg.msgBase.msgKey.name.length = name.length();

	refreshMsg.msgBase.streamId = streamId;

	while ( true )
	{
		if ( ret == RSSL_RET_BUFFER_TOO_SMALL )
		{
			delete[] msgBuf.data;
			msgBuf.data = new char[msgBuf.length * 2 ];
			msgBuf.length = msgBuf.length * 2;
		}

		if( (ret = rsslSetEncodeIteratorBuffer( &encodeIter, &msgBuf ) ) < RSSL_RET_SUCCESS )
		{
			return ret;
		}

		if ( ( ret = rsslSetEncodeIteratorRWFVersion( &encodeIter, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION ) ) < RSSL_RET_SUCCESS )
		{
			return ret;
		}

		if ( ( ret = rsslEncodeMsgInit( &encodeIter, (RsslMsg*)&refreshMsg, 0 ) ) < RSSL_RET_SUCCESS )
		{
			return ret;
		}

		if ( name == "RWFFld" )
		{
			ret = rsslEncodeFieldDictionary( &encodeIter, rsslDataDictionary, &currentFid, (RDMDictionaryVerbosityValues)filter, &errorText );
		}
		else
		{
			ret = rsslEncodeEnumTypeDictionaryAsMultiPart( &encodeIter, rsslDataDictionary, &currentFid, (RDMDictionaryVerbosityValues)filter, &errorText );
		}

		if ( ret  != RSSL_RET_SUCCESS )
		{
			if ( ret == RSSL_RET_BUFFER_TOO_SMALL )
			{
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
