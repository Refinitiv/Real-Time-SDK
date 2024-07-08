/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2024 LSEG. All rights reserved.             --
 *|-----------------------------------------------------------------------------
 */

#include "ElementList.h"
#include "OmmBuffer.h"
#include "EmaString.h"
#include "ExceptionTranslator.h"
#include "ElementListDecoder.h"
#include "ElementListEncoder.h"
#include "Utilities.h"
#include "GlobalPool.h"
#include "OmmInvalidUsageException.h"
#include "StaticDecoder.h"

using namespace refinitiv::ema::access;

extern const EmaString& getDTypeAsString( DataType::DataTypeEnum dType );

ElementList::ElementList() :
 _toString(),
 _entry(),
 _pDecoder( 0 ),
 _pEncoder( 0 )
{
}

ElementList::~ElementList()
{
	if ( _pEncoder )
		g_pool.returnItem( _pEncoder );

	if ( _pDecoder )
		g_pool.returnItem( _pDecoder );
}

ElementList& ElementList::clear()
{
	if ( _pEncoder )
		_pEncoder->clear();

	return *this;
}

DataType::DataTypeEnum ElementList::getDataType() const
{
	return DataType::ElementListEnum;
}

Data::DataCode ElementList::getCode() const
{
	return Data::NoCodeEnum;
}

bool ElementList::hasInfo() const
{
	return _pDecoder->hasInfo();
}

Int16 ElementList::getInfoElementListNum() const
{
	return _pDecoder->getInfoElementListNum();
}

bool ElementList::forth() const
{
	return !_pDecoder->getNextData();
}

bool ElementList::forth( const EmaString& name ) const
{
	return !_pDecoder->getNextData( name );
}

bool ElementList::forth( const Data& data ) const
{
	return !_pDecoder->getNextData( data );
}

void ElementList::reset() const
{
	_pDecoder->reset();
}

const EmaString& ElementList::toString() const
{
	return toString( 0 );
}

const EmaString& ElementList::toString( const refinitiv::ema::rdm::DataDictionary& dictionary ) const
{
	ElementList elementList;

	if (!dictionary.isEnumTypeDefLoaded() || !dictionary.isFieldDictionaryLoaded())
		return _toString.clear().append("\nDictionary is not loaded.\n");

	if (!_pEncoder)
		_pEncoder = g_pool.getElementListEncoderItem();

	if (_pEncoder->isComplete())
	{
		RsslBuffer& rsslBuffer = _pEncoder->getRsslBuffer();

		StaticDecoder::setRsslData(&elementList, &rsslBuffer, RSSL_DT_ELEMENT_LIST, RSSL_RWF_MAJOR_VERSION, RSSL_RWF_MINOR_VERSION, dictionary._pImpl->rsslDataDictionary());
		_toString.clear().append(elementList.toString());

		return _toString;
	}

	return _toString.clear().append("\nUnable to decode not completed ElementList data.\n");
}

const EmaString& ElementList::toString( UInt64 indent ) const
{
	if (!_pDecoder)
		return _toString.clear().append("\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n");

	ElementListDecoder tempDecoder;
	addIndent(_toString.clear(), indent).append("ElementList");

	tempDecoder.clone(*_pDecoder);

	if (tempDecoder.hasInfo())
		_toString.append(" ElementListNum=\"").append(tempDecoder.getInfoElementListNum()).append("\"");

	++indent;

	while (!tempDecoder.getNextData())
	{
		addIndent(_toString.append("\n"), indent)
			.append("ElementEntry name=\"").append(tempDecoder.getName())
			.append("\" dataType=\"").append(getDTypeAsString(tempDecoder.getLoad().getDataType()));

		if (tempDecoder.getLoad().getDataType() >= DataType::FieldListEnum || tempDecoder.getLoad().getDataType() == DataType::ArrayEnum)
		{
			++indent;
			_toString.append("\"\n").append(tempDecoder.getLoad().toString(indent));
			--indent;
			addIndent(_toString, indent).append("ElementEntryEnd");
		}
		else if (tempDecoder.getLoad().getDataType() == DataType::BufferEnum)
		{
			if (tempDecoder.getLoad().getCode() == Data::BlankEnum)
				_toString.append("\" value=\"").append(tempDecoder.getLoad().toString()).append("\"");
			else
				_toString.append("\"\n").append(tempDecoder.getLoad().toString());
		}
		else
			_toString.append("\" value=\"").append(tempDecoder.getLoad().toString()).append("\"");
	}

	--indent;

	addIndent( _toString.append( "\n" ), indent ).append( "ElementListEnd\n" );

	return _toString;
}

const EmaBuffer& ElementList::getAsHex() const
{
	return _pDecoder->getHexBuffer();
}

Decoder& ElementList::getDecoder()
{
	if ( !_pDecoder )
	{
		_entry._pDecoder = _pDecoder = g_pool.getElementListDecoderItem();
		_entry._pLoad = _pDecoder->getLoadPtr();
	}

	return *_pDecoder;
}

bool ElementList::hasDecoder() const
{
	return _pDecoder ? true : false;
}

const ElementEntry& ElementList::getEntry() const
{
	if ( !_pDecoder || !_pDecoder->decodingStarted() )
	{
		EmaString temp( "Attempt to getEntry() while iteration was NOT started." );

		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _entry;
}

const Encoder& ElementList::getEncoder() const
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	return *_pEncoder;
}

ElementList& ElementList::info( Int16 elmentListNum )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->info( elmentListNum );

	return *this;
}

ElementList& ElementList::addInt( const EmaString& name, Int64 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addInt( name, value );

	return *this;
}

ElementList& ElementList::addUInt( const EmaString& name, UInt64 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addUInt( name, value );

	return *this;
}

ElementList& ElementList::addReal( const EmaString& name, Int64 mantissa, OmmReal::MagnitudeType magnitudeType )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addReal( name, mantissa, magnitudeType );

	return *this;
}

ElementList& ElementList::addRealFromDouble( const EmaString& name, double value,
								OmmReal::MagnitudeType magnitudeType )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addRealFromDouble( name, value, magnitudeType );

	return *this;
}

ElementList& ElementList::addFloat( const EmaString& name, float value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addFloat( name, value );

	return *this;
}

ElementList& ElementList::addDouble( const EmaString& name, double value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addDouble( name, value );

	return *this;
}

ElementList& ElementList::addDate( const EmaString& name, UInt16 year, UInt8 month, UInt8 day )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addDate( name, year, month, day );

	return *this;
}

ElementList& ElementList::addTime( const EmaString& name, UInt8 hour, UInt8 minute, UInt8 second,
								  UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addTime( name, hour, minute, second, millisecond, microsecond, nanosecond );

	return *this;
}

ElementList& ElementList::addDateTime( const EmaString& name,
						UInt16 year, UInt8 month, UInt8 day,
						UInt8 hour, UInt8 minute, UInt8 second,
						UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addDateTime( name, year, month, day, hour, minute, second, millisecond, microsecond, nanosecond );

	return *this;
}

ElementList& ElementList::addQos( const EmaString& name,
					UInt32 timeliness,
					UInt32 rate )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addQos( name, timeliness, rate );

	return *this;
}

ElementList& ElementList::addState( const EmaString& name,
					OmmState::StreamState streamState,
					OmmState::DataState dataState,
					UInt8 statusCode,
					const EmaString& statusText )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addState( name, streamState, dataState, statusCode, statusText );

	return *this;
}

ElementList& ElementList::addEnum( const EmaString& name, UInt16 value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addEnum( name, value );

	return *this;
}

ElementList& ElementList::addBuffer( const EmaString& name, const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addBuffer( name, value );

	return *this;
}

ElementList& ElementList::addAscii( const EmaString& name, const EmaString& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addAscii( name, value );

	return *this;
}

ElementList& ElementList::addUtf8( const EmaString& name, const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addUtf8( name, value );

	return *this;
}

ElementList& ElementList::addRmtes( const EmaString& name, const EmaBuffer& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addRmtes( name, value );

	return *this;
}

ElementList& ElementList::addArray( const EmaString& name, const OmmArray& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addArray( name, value );

	return *this;
}

ElementList& ElementList::addElementList( const EmaString& name, const ElementList& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addElementList( name, value );

	return *this;
}

ElementList& ElementList::addFieldList( const EmaString& name, const FieldList& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addFieldList( name, value );

	return *this;
}

ElementList& ElementList::addReqMsg( const EmaString& name, const ReqMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addReqMsg( name, value );

	return *this;
}

ElementList& ElementList::addRefreshMsg( const EmaString& name, const RefreshMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addRefreshMsg( name, value );

	return *this;
}

ElementList& ElementList::addUpdateMsg( const EmaString& name, const UpdateMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addUpdateMsg( name, value );

	return *this;
}

ElementList& ElementList::addStatusMsg( const EmaString& name, const StatusMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addStatusMsg( name, value );

	return *this;
}

ElementList& ElementList::addPostMsg( const EmaString& name, const PostMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addPostMsg( name, value );

	return *this;
}

ElementList& ElementList::addAckMsg( const EmaString& name, const AckMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addAckMsg( name, value );

	return *this;
}

ElementList& ElementList::addGenericMsg( const EmaString& name, const GenericMsg& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addGenericMsg( name, value );

	return *this;
}

ElementList& ElementList::addMap( const EmaString& name, const Map& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addMap( name, value );

	return *this;
}

ElementList& ElementList::addVector( const EmaString& name, const Vector& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addVector( name, value );

	return *this;
}

ElementList& ElementList::addSeries( const EmaString& name, const Series& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addSeries( name, value );

	return *this;
}

ElementList& ElementList::addFilterList( const EmaString& name, const FilterList& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addFilterList( name, value );

	return *this;
}

ElementList& ElementList::addOpaque( const EmaString& name, const OmmOpaque& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addOpaque( name, value );

	return *this;
}

ElementList& ElementList::addXml( const EmaString& name, const OmmXml& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addXml( name, value );

	return *this;
}

ElementList& ElementList::addAnsiPage( const EmaString& name, const OmmAnsiPage& value )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addAnsiPage( name, value );

	return *this;
}

ElementList& ElementList::addCodeInt( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeInt( name );

	return *this;
}

ElementList& ElementList::addCodeUInt( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeUInt( name );

	return *this;
}

ElementList& ElementList::addCodeReal( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeReal( name );

	return *this;
}

ElementList& ElementList::addCodeFloat( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeFloat( name );

	return *this;
}

ElementList& ElementList::addCodeDouble( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeDouble( name );

	return *this;
}

ElementList& ElementList::addCodeDate( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeDate( name );

	return *this;
}

ElementList& ElementList::addCodeTime( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeTime( name );

	return *this;
}

ElementList& ElementList::addCodeDateTime( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeDateTime( name );

	return *this;
}

ElementList& ElementList::addCodeQos( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeQos( name );

	return *this;
}

ElementList& ElementList::addCodeState( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeState( name );

	return *this;
}

ElementList& ElementList::addCodeEnum( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeEnum( name );

	return *this;
}

ElementList& ElementList::addCodeBuffer( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeBuffer( name );

	return *this;
}

ElementList& ElementList::addCodeAscii( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeAscii( name );

	return *this;
}

ElementList& ElementList::addCodeUtf8( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeUtf8( name );

	return *this;
}

ElementList& ElementList::addCodeRmtes( const EmaString& name )
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->addCodeRmtes( name );

	return *this;
}

ElementList& ElementList::add( const EmaString& name )
{
	if (!_pEncoder)
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->add( name );

	return *this;
}

const ElementList& ElementList::complete()
{
	if ( !_pEncoder )
		_pEncoder = g_pool.getElementListEncoderItem();

	_pEncoder->complete();

	return *this;
}

bool ElementList::hasEncoder() const
{
	return _pEncoder ? true : false;
}
