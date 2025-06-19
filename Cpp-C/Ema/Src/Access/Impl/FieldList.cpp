/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2020,2022-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "FieldList.h"
#include "ExceptionTranslator.h"
#include "FieldListDecoder.h"
#include "FieldListEncoder.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "OmmInvalidUsageException.h"
#include "StaticDecoder.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

FieldList::FieldList() :
 _toString(),
 _entry(),
 _pDecoder( 0 ),
 _pEncoder( 0 )
{
}

FieldList::~FieldList()
{
	if ( _pEncoder ) 
		g_pool.returnItem( _pEncoder );

	if ( _pDecoder )
		g_pool.returnItem( _pDecoder );
}

FieldList& FieldList::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

DataType::DataTypeEnum FieldList::getDataType() const
{
	return DataType::FieldListEnum;
}

Data::DataCode FieldList::getCode() const
{
	return Data::NoCodeEnum;
}

bool FieldList::hasInfo() const
{
	return _pDecoder->hasInfo();
}

Int16 FieldList::getInfoFieldListNum() const
{
	return _pDecoder->getInfoFieldListNum();
}

Int16 FieldList::getInfoDictionaryId() const
{
	return _pDecoder->getInfoDictionaryId();
}

bool FieldList::forth() const
{
	return !_pDecoder->getNextData();
}

bool FieldList::forth( Int16 fieldId ) const
{
	return !_pDecoder->getNextData( fieldId );
}

bool FieldList::forth( const EmaString& name ) const
{
	return !_pDecoder->getNextData( name );
}

bool FieldList::forth( const Data& data ) const
{
	return !_pDecoder->getNextData( data );
}

void FieldList::reset() const
{
	_pDecoder->reset();
}

const EmaString& FieldList::toString() const
{
		return toString( 0 );
}

const EmaString& FieldList::toString( const refinitiv::ema::rdm::DataDictionary& dictionary ) const
{
	FieldList fieldList;

	if (!dictionary.isEnumTypeDefLoaded() || !dictionary.isFieldDictionaryLoaded())
		return _toString.clear().append("\nDictionary is not loaded.\n");

	if (!_pEncoder)
		_pEncoder = g_pool.getFieldListEncoderItem();

	if (_pEncoder->isComplete())
	{
		RsslBuffer& rsslBuffer = _pEncoder->getRsslBuffer();

		StaticDecoder::setRsslData(&fieldList, &rsslBuffer, RSSL_DT_FIELD_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, dictionary._pImpl->rsslDataDictionary());
		_toString.clear().append(fieldList.toString());

		return _toString;
	}

	return _toString.clear().append("\nUnable to decode not completed FieldList data.\n");
}

const EmaString& FieldList::toString( UInt64 indent ) const
{
	if (!_pDecoder)
		return _toString.clear().append("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n");

	FieldListDecoder tempDecoder;
	tempDecoder.clone( *_pDecoder );

	addIndent( _toString.clear(), indent ).append( "FieldList" );

	if ( tempDecoder.hasInfo() )
		_toString.append( " FieldListNum=\"" ).append( tempDecoder.getInfoFieldListNum() )
			.append( "\" DictionaryId=\"" ).append( tempDecoder.getInfoDictionaryId() ).append( "\"" );

	++indent;

	while ( !tempDecoder.getNextData() )
	{
		addIndent( _toString.append( "\n" ), indent )
			.append( "FieldEntry fid=\"" ).append( tempDecoder.getFieldId() )
			.append( "\" name=\"" ).append( tempDecoder.getName() )
			.append( "\" dataType=\"" ).append( getDTypeAsString( tempDecoder.getLoad().getDataType() ) );

		if ( tempDecoder.getLoad().getDataType() >= DataType::FieldListEnum || tempDecoder.getLoad().getDataType() == DataType::ArrayEnum )
		{
			++indent;
			_toString.append( "\"\n" ).append( tempDecoder.getLoad().toString( indent ) );
			--indent;
			addIndent( _toString, indent ).append( "FieldEntryEnd" );
		}
		else if ( tempDecoder.getLoad().getDataType() == DataType::BufferEnum )
		{
			if ( tempDecoder.getLoad().getCode() == Data::BlankEnum )
				_toString.append( "\" value=\"" ).append( tempDecoder.getLoad().toString() ).append( "\"" );
			else
				_toString.append( "\"\n" ).append( tempDecoder.getLoad().toString() );
		}
		else
			_toString.append( "\" value=\"" ).append( tempDecoder.getLoad().toString() ).append( "\"" );
	}

	--indent;

	addIndent( _toString.append( "\n" ), indent ).append( "FieldListEnd\n" );

	return _toString;
}

const EmaBuffer& FieldList::getAsHex() const
{
	if ( !_pDecoder )
	{
		EmaString temp( "Attempt to getAsHex() while data is not set." );

		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _pDecoder->getHexBuffer();
}

Decoder& FieldList::getDecoder()
{
	if ( !_pDecoder )
	{
		_entry._pDecoder = _pDecoder = g_pool.getFieldListDecoderItem();
		_entry._pLoad = _pDecoder->getLoadPtr();
	}

	return *_pDecoder;
}

bool FieldList::hasDecoder() const
{
	return _pDecoder ? true : false;
}

const FieldEntry& FieldList::getEntry() const
{
	if ( !_pDecoder || !_pDecoder->decodingStarted() )
	{
		EmaString temp( "Attempt to getEntry() while iteration was NOT started." );

		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _entry;
}

const Encoder& FieldList::getEncoder() const
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	return *_pEncoder;
}

FieldList& FieldList::info( Int16 dictionaryId, Int16 fieldListNum )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->info( dictionaryId, fieldListNum );

	return *this;
}

FieldList& FieldList::addInt( Int16 fieldId, Int64 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addInt( fieldId, value );

	return *this;
}

FieldList& FieldList::addUInt( Int16 fieldId, UInt64 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addUInt( fieldId, value );

	return *this;
}

FieldList& FieldList::addReal( Int16 fieldId, Int64 mantissa, OmmReal::MagnitudeType magnitudeType )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addReal( fieldId, mantissa, magnitudeType );

	return *this;
}

FieldList& FieldList::addRealFromDouble( Int16 fieldId, double value,
								OmmReal::MagnitudeType magnitudeType )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addRealFromDouble( fieldId, value, magnitudeType );

	return *this;
}

FieldList& FieldList::addFloat( Int16 fieldId, float value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addFloat( fieldId, value );

	return *this;
}

FieldList& FieldList::addDouble( Int16 fieldId, double value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addDouble( fieldId, value );

	return *this;
}

FieldList& FieldList::addDate( Int16 fieldId, UInt16 year, UInt8 month, UInt8 day )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addDate( fieldId, year, month, day );

	return *this;
}

FieldList& FieldList::addTime( Int16 fieldId, UInt8 hour, UInt8 minute, UInt8 second,
							  UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addTime( fieldId, hour, minute, second, millisecond, microsecond, nanosecond );

	return *this;
}

FieldList& FieldList::addDateTime( Int16 fieldId,
						UInt16 year, UInt8 month, UInt8 day,
						UInt8 hour, UInt8 minute, UInt8 second,
						UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addDateTime( fieldId, year, month, day, hour, minute, second, millisecond, microsecond, nanosecond );

	return *this;
}

FieldList& FieldList::addQos( Int16 fieldId,
					UInt32 timeliness,
					UInt32 rate )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addQos( fieldId, timeliness, rate );

	return *this;
}

FieldList& FieldList::addState( Int16 fieldId,
					OmmState::StreamState streamState,
					OmmState::DataState dataState,
					UInt8 statusCode,
					const EmaString& statusText )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addState( fieldId, streamState, dataState, statusCode, statusText );

	return *this;
}

FieldList& FieldList::addEnum( Int16 fieldId, UInt16 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addEnum( fieldId, value );

	return *this;
}

FieldList& FieldList::addBuffer( Int16 fieldId, const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addBuffer( fieldId, value );

	return *this;
}

FieldList& FieldList::addAscii( Int16 fieldId, const EmaString& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addAscii( fieldId, value );

	return *this;
}

FieldList& FieldList::addUtf8( Int16 fieldId, const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addUtf8( fieldId, value );

	return *this;
}

FieldList& FieldList::addRmtes( Int16 fieldId, const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addRmtes( fieldId, value );

	return *this;
}

FieldList& FieldList::addArray( Int16 fieldId, const OmmArray& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addArray( fieldId, value );

	return *this;
}

FieldList& FieldList::addElementList( Int16 fieldId, const ElementList& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addElementList( fieldId, value );

	return *this;
}

FieldList& FieldList::addFieldList( Int16 fieldId, const FieldList& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addFieldList( fieldId, value );

	return *this;
}

FieldList& FieldList::addReqMsg( Int16 fieldId, const ReqMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addReqMsg( fieldId, value );

	return *this;
}

FieldList& FieldList::addRefreshMsg( Int16 fieldId, const RefreshMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addRefreshMsg( fieldId, value );

	return *this;
}

FieldList& FieldList::addUpdateMsg( Int16 fieldId, const UpdateMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addUpdateMsg( fieldId, value );

	return *this;
}

FieldList& FieldList::addStatusMsg( Int16 fieldId, const StatusMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addStatusMsg( fieldId, value );

	return *this;
}

FieldList& FieldList::addPostMsg( Int16 fieldId, const PostMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addPostMsg( fieldId, value );

	return *this;
}

FieldList& FieldList::addAckMsg( Int16 fieldId, const AckMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addAckMsg( fieldId, value );

	return *this;
}

FieldList& FieldList::addGenericMsg( Int16 fieldId, const GenericMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addGenericMsg( fieldId, value );

	return *this;
}

FieldList& FieldList::addMap( Int16 fieldId, const Map& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addMap( fieldId, value );

	return *this;
}

FieldList& FieldList::addVector( Int16 fieldId, const Vector& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addVector( fieldId, value );

	return *this;
}

FieldList& FieldList::addSeries( Int16 fieldId, const Series& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addSeries( fieldId, value );

	return *this;
}

FieldList& FieldList::addFilterList( Int16 fieldId, const FilterList& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addFilterList( fieldId, value );

	return *this;
}

FieldList& FieldList::addOpaque( Int16 fieldId, const OmmOpaque& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addOpaque( fieldId, value );

	return *this;
}

FieldList& FieldList::addXml( Int16 fieldId, const OmmXml& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addXml( fieldId, value );

	return *this;
}

FieldList& FieldList::addJson( Int16 fieldId, const OmmJson& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addJson( fieldId, value );

	return *this;
}

FieldList& FieldList::addAnsiPage( Int16 fieldId, const OmmAnsiPage& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addAnsiPage( fieldId, value );

	return *this;
}

FieldList& FieldList::addCodeInt( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeInt( fieldId );

	return *this;
}

FieldList& FieldList::addCodeUInt( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeUInt( fieldId );

	return *this;
}

FieldList& FieldList::addCodeReal( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeReal( fieldId );

	return *this;
}

FieldList& FieldList::addCodeFloat( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeFloat( fieldId );

	return *this;
}

FieldList& FieldList::addCodeDouble( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeDouble( fieldId );

	return *this;
}

FieldList& FieldList::addCodeDate( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeDate( fieldId );

	return *this;
}

FieldList& FieldList::addCodeTime( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeTime( fieldId );

	return *this;
}

FieldList& FieldList::addCodeDateTime( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeDateTime( fieldId );

	return *this;
}

FieldList& FieldList::addCodeQos( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeQos( fieldId );

	return *this;
}

FieldList& FieldList::addCodeState( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeState( fieldId );

	return *this;
}

FieldList& FieldList::addCodeEnum( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeEnum( fieldId );

	return *this;
}

FieldList& FieldList::addCodeBuffer( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeBuffer( fieldId );

	return *this;
}

FieldList& FieldList::addCodeAscii( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeAscii( fieldId );

	return *this;
}

FieldList& FieldList::addCodeUtf8( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeUtf8( fieldId );

	return *this;
}

FieldList& FieldList::addCodeRmtes( Int16 fieldId )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->addCodeRmtes( fieldId );

	return *this;
}

const FieldList& FieldList::complete()
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getFieldListEncoderItem();

	_pEncoder->complete();

	return *this;
}

bool FieldList::hasEncoder() const
{
	return _pEncoder ? true : false;
}
