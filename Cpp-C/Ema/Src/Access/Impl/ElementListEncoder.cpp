/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "ElementListEncoder.h"
#include "ExceptionTranslator.h"
#include "StaticDecoder.h"
#include "OmmStateDecoder.h"
#include "OmmQosDecoder.h"
#include "OmmRealDecoder.h"
#include "OmmArray.h"
#include "ElementList.h"
#include "FieldList.h"
#include "Map.h"
#include "Vector.h"
#include "Series.h"
#include "FilterList.h"
#include "OmmAnsiPage.h"
#include "OmmOpaque.h"
#include "OmmXml.h"
#include "AckMsg.h"
#include "GenericMsg.h"
#include "PostMsg.h"
#include "ReqMsg.h"
#include "RefreshMsg.h"
#include "UpdateMsg.h"
#include "StatusMsg.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;

extern const EmaString& getMTypeAsString( OmmReal::MagnitudeType mType );

ElementListEncoder::ElementListEncoder() :
 _rsslElementList(),
 _rsslElementEntry(),
 _containerInitialized( false )
{
}

ElementListEncoder::~ElementListEncoder()
{
}

void ElementListEncoder::clear()
{
	Encoder::releaseEncIterator();

	rsslClearElementList( &_rsslElementList );
	rsslClearElementEntry( &_rsslElementEntry );

	_containerInitialized = false;
}

void ElementListEncoder::info( Int16 elementListNum )
{
	if ( !_containerInitialized )
	{
		_rsslElementList.elementListNum = elementListNum;
		rsslElementListApplyHasInfo(&_rsslElementList);
	}
	else
	{
		EmaString temp("Invalid attempt to call info() when container is initialized.");
		throwIueException(temp, OmmInvalidUsageException::InvalidOperationEnum);
	}
}

void ElementListEncoder::initEncode()
{
	RsslRet retCode = rsslEncodeElementListInit( &(_pEncodeIter->_rsslEncIter), &_rsslElementList, 0, 0 );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		retCode = rsslEncodeElementListComplete( &(_pEncodeIter->_rsslEncIter), RSSL_FALSE );

		_pEncodeIter->reallocate();

		retCode = rsslEncodeElementListInit( &(_pEncodeIter->_rsslEncIter), &_rsslElementList, 0, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to initialize ElementList encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}

	_containerInitialized = true;
}

void ElementListEncoder::addPrimitiveEntry( const EmaString& name, RsslDataType rsslDataType,
										 const char* methodName, void* value )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	_rsslElementEntry.encData.data = 0;
	_rsslElementEntry.encData.length = 0;

	_rsslElementEntry.dataType = rsslDataType;

	_rsslElementEntry.name.data = (char*)name.c_str();
	_rsslElementEntry.name.length = name.length();

	RsslRet retCode = rsslEncodeElementEntry( &_pEncodeIter->_rsslEncIter, &_rsslElementEntry, value );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeElementEntry( &_pEncodeIter->_rsslEncIter, &_rsslElementEntry, value );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to " );
		temp.append( methodName ).append( " while encoding ElementList. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void ElementListEncoder::addEncodedEntry( const EmaString& name, RsslDataType rsslDataType, const char* methodName, const RsslBuffer& rsslBuffer )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	_rsslElementEntry.encData = rsslBuffer;

	_rsslElementEntry.dataType = rsslDataType;

	_rsslElementEntry.name.data = (char*)name.c_str();
	_rsslElementEntry.name.length = name.length();

	RsslRet retCode = rsslEncodeElementEntry( &_pEncodeIter->_rsslEncIter, &_rsslElementEntry, 0 );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeElementEntry( &_pEncodeIter->_rsslEncIter, &_rsslElementEntry, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to " );
		temp.append( methodName ).append( " while encoding ElementList. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void ElementListEncoder::startEncodingEntry( const EmaString& name, RsslDataType rsslDataType, const char* methodName )
{
	if ( _containerComplete )
	{
		EmaString temp( "Attempt to add an entry after complete() was called." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
		return;
	}

	_rsslElementEntry.encData.data = 0;
	_rsslElementEntry.encData.length = 0;

	_rsslElementEntry.dataType = rsslDataType;

	_rsslElementEntry.name.data = (char*)name.c_str();
	_rsslElementEntry.name.length = name.length();

	RsslRet retCode = rsslEncodeElementEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslElementEntry, 0 );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeElementEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslElementEntry, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to start encoding entry in ElementList::" );
		temp.append( methodName ).append( ". Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void ElementListEncoder::endEncodingEntry() const
{
	RsslRet retCode = rsslEncodeElementEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeElementEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to end encoding entry in ElementList. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
	}
}

void ElementListEncoder::addInt( const EmaString& name, Int64 value )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_INT, "addInt()", &value );
}

void ElementListEncoder::addUInt( const EmaString& name, UInt64 value )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_UINT, "addUInt()", &value );
}

void ElementListEncoder::addReal( const EmaString& name, Int64 mantissa, OmmReal::MagnitudeType magnitudeType )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	RsslReal real;
	real.isBlank = RSSL_FALSE;
	real.value = mantissa;
	real.hint = magnitudeType;

	addPrimitiveEntry( name, RSSL_DT_REAL, "addReal()", &real );
}

void ElementListEncoder::addRealFromDouble( const EmaString& name, double value,
							OmmReal::MagnitudeType magnitudeType )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	RsslReal real;
	if ( RSSL_RET_SUCCESS != rsslDoubleToReal( &real, &value, magnitudeType ) )
	{
		EmaString temp( "Attempt to addRealFromDouble() with invalid magnitudeType='" );
		temp.append( getMTypeAsString( magnitudeType ) ).append( "'. " );
		throwIueException( temp, OmmInvalidUsageException::InvalidArgumentEnum );
		return;
	}

	addPrimitiveEntry( name, RSSL_DT_REAL, "addRealFromDouble()", &real );
}

void ElementListEncoder::addFloat( const EmaString& name, float value )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_FLOAT, "addFloat()", &value );
}

void ElementListEncoder::addDouble( const EmaString& name, double value )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_DOUBLE, "addDouble()", &value );
}

void ElementListEncoder::addDate( const EmaString& name, UInt16 year, UInt8 month, UInt8 day )
{
	RsslDate date;
	date.year = year;
	date.month = month;
	date.day = day;

	if ( RSSL_FALSE == rsslDateIsValid( &date ) )
	{
		EmaString temp( "Attempt to specify invalid date. Passed in value is='" );
		temp.append( (UInt32)month ).append( " / " ).
			append( (UInt32)day ).append( " / " ).
			append( (UInt32)year ).append( "'." );
		throwOorException( temp );
		return;
	}

	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_DATE, "addDate()", &date );
}

void ElementListEncoder::addTime( const EmaString& name, UInt8 hour, UInt8 minute, UInt8 second,
								 UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond )
{
	RsslTime time;
	time.hour = hour;
	time.minute = minute;
	time.second = second;
	time.millisecond = millisecond;
	time.microsecond = microsecond;
	time.nanosecond = nanosecond;

	if ( RSSL_FALSE == rsslTimeIsValid( &time ) )
	{
		EmaString temp( "Attempt to specify invalid time. Passed in value is='" );
		temp.append( (UInt32)hour ).append( ":" ).
			append( (UInt32)minute ).append( ":" ).
			append( (UInt32)second ).append( "." ).
			append( (UInt32)millisecond ).append( "." ).
			append( (UInt32)microsecond ).append( "." ).
			append( (UInt32)nanosecond ).append( "'." );
		throwOorException( temp );
		return;
	}

	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_TIME, "addTime()", &time );
}

void ElementListEncoder::addDateTime( const EmaString& name,
						UInt16 year, UInt8 month, UInt8 day,
						UInt8 hour, UInt8 minute, UInt8 second,
						UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond )
{
	RsslDateTime dateTime;
	dateTime.date.year = year;
	dateTime.date.month = month;
	dateTime.date.day = day;
	dateTime.time.hour = hour;
	dateTime.time.minute = minute;
	dateTime.time.second = second;
	dateTime.time.millisecond = millisecond;
	dateTime.time.microsecond = microsecond;
	dateTime.time.nanosecond = nanosecond;

	if ( RSSL_FALSE == rsslDateTimeIsValid( &dateTime ) )
	{
		EmaString temp( "Attempt to specify invalid date time. Passed in value is='" );
		temp.append( (UInt32)month ).append( " / " ).
			append( (UInt32)day ).append( " / " ).
			append( (UInt32)year ).append( "  " ).
			append( (UInt32)hour ).append( ":" ).
			append( (UInt32)minute ).append( ":" ).
			append( (UInt32)second ).append( "." ).
			append( (UInt32)millisecond ).append( "." ).
			append( (UInt32)microsecond ).append( "." ).
			append( (UInt32)nanosecond ).append( "'." );
		throwOorException( temp );
		return;
	}

	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_DATETIME, "addDateTime()", &dateTime );
}

void ElementListEncoder::addQos( const EmaString& name,
						UInt32 timeliness, UInt32 rate )					
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	RsslQos qos;
	OmmQosDecoder::convertToRssl( &qos, timeliness, rate );

	addPrimitiveEntry( name, RSSL_DT_QOS, "addQos()", &qos );
}

void ElementListEncoder::addState( const EmaString& name,
					OmmState::StreamState streamState,
					OmmState::DataState dataState,
					UInt8 statusCode,
					const EmaString& statusText )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	RsslState state;
	state.streamState = streamState;
	state.dataState = dataState;
	state.code = statusCode;
	state.text.data = (char*)statusText.c_str();
	state.text.length = statusText.length();

	addPrimitiveEntry( name, RSSL_DT_STATE, "addState()", &state );
}

void ElementListEncoder::addEnum( const EmaString& name, UInt16 value )	
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_ENUM, "addEnum()", &value );
}

void ElementListEncoder::addBuffer( const EmaString& name, const EmaBuffer& value )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_buf();
	buffer.length = value.length();

	addPrimitiveEntry( name, RSSL_DT_BUFFER, "addBuffer()", &buffer );
}

void ElementListEncoder::addAscii( const EmaString& name, const EmaString& value )	
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_str();
	buffer.length = value.length();

	addPrimitiveEntry( name, RSSL_DT_ASCII_STRING, "addAscii()", &buffer );
}

void ElementListEncoder::addUtf8( const EmaString& name, const EmaBuffer& value )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_buf();
	buffer.length = value.length();

	addPrimitiveEntry( name, RSSL_DT_UTF8_STRING, "addUtf8()", &buffer );
}

void ElementListEncoder::addRmtes( const EmaString& name, const EmaBuffer& value )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_buf();
	buffer.length = value.length();

	addPrimitiveEntry( name, RSSL_DT_RMTES_STRING, "addRmtes()", &buffer );
}

void ElementListEncoder::addArray( const EmaString& name, const OmmArray& array )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	const Encoder& enc = static_cast<const Data&>(array).getEncoder();

	if ( enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( name, RSSL_DT_ARRAY, "addArray()", enc.getRsslBuffer() );
		else
		{
			EmaString temp( "Attempt to addArray() while OmmArray::complete() was not called." );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}
	}
	else if ( static_cast<const Data&>(array).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_ARRAY, "addArray()", static_cast<Data&>( const_cast<OmmArray&>(array)).getDecoder().getRsslBuffer() );
	}
	else
	{
		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( name, RSSL_DT_ARRAY, "addArray()" );
	}
}

void ElementListEncoder::addElementList( const EmaString& name, const ElementList& elementList )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	const Encoder& enc = static_cast<const Data&>(elementList).getEncoder();

	if ( enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( name, RSSL_DT_ELEMENT_LIST, "addElementList()", enc.getRsslBuffer() );
		else
		{
			EmaString temp( "Attempt to addElementList() while ElementList::complete() was not called." );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}
	}
	else if ( static_cast<const Data&>(elementList).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_ELEMENT_LIST, "addElementList()", static_cast<Data&>( const_cast<ElementList&>(elementList)).getDecoder().getRsslBuffer() );
	}
	else
	{
		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( name, RSSL_DT_ELEMENT_LIST, "addElementList()" );
	}
}

void ElementListEncoder::addFieldList( const EmaString& name, const FieldList& fieldList )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	const Encoder& enc = static_cast<const Data&>(fieldList).getEncoder();

	if ( enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( name, RSSL_DT_FIELD_LIST, "addFieldList()", enc.getRsslBuffer() );
		else
		{
			EmaString temp( "Attempt to addFieldList() while FieldList::complete() was not called." );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}
	}
	else if ( static_cast<const Data&>(fieldList).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_FIELD_LIST, "addFieldList()", static_cast<Data&>( const_cast<FieldList&>(fieldList)).getDecoder().getRsslBuffer() );
	}
	else
	{
		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( name, RSSL_DT_FIELD_LIST, "addFieldList()" );
	}
}

void ElementListEncoder::addReqMsg( const EmaString& name, const ReqMsg& reqMsg )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(reqMsg).hasEncoder() && static_cast<const Data&>(reqMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addReqMsg()", static_cast<const Data&>(reqMsg).getEncoder().getRsslBuffer() );
	}
	else if ( static_cast<const Data&>(reqMsg).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addReqMsg()", static_cast<Data&>( const_cast<ReqMsg&>(reqMsg)).getDecoder().getRsslBuffer() );
	}
	else
	{
		EmaString temp( "Attempt to pass an empty message to addReqMsg() while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void ElementListEncoder::addRefreshMsg( const EmaString& name, const RefreshMsg& refreshMsg )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(refreshMsg).hasEncoder() && static_cast<const Data&>(refreshMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addRefreshMsg()", static_cast<const Data&>(refreshMsg).getEncoder().getRsslBuffer() );
	}
	else if ( static_cast<const Data&>(refreshMsg).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addRefreshMsg()", static_cast<Data&>( const_cast<RefreshMsg&>(refreshMsg)).getDecoder().getRsslBuffer() );
	}
	else
	{
		EmaString temp( "Attempt to pass an empty message to addRefreshMsg() while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void ElementListEncoder::addStatusMsg( const EmaString& name, const StatusMsg& statusMsg )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(statusMsg).hasEncoder() && static_cast<const Data&>(statusMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addStatusMsg()", static_cast<const Data&>(statusMsg).getEncoder().getRsslBuffer() );
	}
	else if ( static_cast<const Data&>(statusMsg).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addStatusMsg()", static_cast<Data&>( const_cast<StatusMsg&>(statusMsg)).getDecoder().getRsslBuffer() );
	}
	else
	{
		EmaString temp( "Attempt to pass an empty message to addStatusMsg() while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void ElementListEncoder::addUpdateMsg( const EmaString& name, const UpdateMsg& updateMsg )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(updateMsg).hasEncoder() && static_cast<const Data&>(updateMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addUpdateMsg()", static_cast<const Data&>(updateMsg).getEncoder().getRsslBuffer() );
	}
	else if ( static_cast<const Data&>(updateMsg).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addUpdateMsg()", static_cast<Data&>( const_cast<UpdateMsg&>(updateMsg)).getDecoder().getRsslBuffer() );
	}
	else
	{
		EmaString temp( "Attempt to pass an empty message to addUpdateMsg() while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void ElementListEncoder::addPostMsg( const EmaString& name, const PostMsg& postMsg )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(postMsg).hasEncoder() && static_cast<const Data&>(postMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addPostMsg()", static_cast<const Data&>(postMsg).getEncoder().getRsslBuffer() );
	}
	else if ( static_cast<const Data&>(postMsg).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addPostMsg()", static_cast<Data&>( const_cast<PostMsg&>(postMsg)).getDecoder().getRsslBuffer() );
	}
	else
	{
		EmaString temp( "Attempt to pass an empty message to addPostMsg() while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void ElementListEncoder::addAckMsg( const EmaString& name, const AckMsg& ackMsg )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(ackMsg).hasEncoder() && static_cast<const Data&>(ackMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addAckMsg()", static_cast<const Data&>(ackMsg).getEncoder().getRsslBuffer() );
	}
	else if ( static_cast<const Data&>(ackMsg).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addAckMsg()", static_cast<Data&>( const_cast<AckMsg&>(ackMsg)).getDecoder().getRsslBuffer() );
	}
	else
	{
		EmaString temp( "Attempt to pass an empty message to addAckMsg() while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void ElementListEncoder::addGenericMsg( const EmaString& name, const GenericMsg& genMsg )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(genMsg).hasEncoder() && static_cast<const Data&>(genMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addGenericMsg()", static_cast<const Data&>(genMsg).getEncoder().getRsslBuffer() );
	}
	else if ( static_cast<const Data&>(genMsg).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_MSG, "addGenericMsg()", static_cast<Data&>( const_cast<GenericMsg&>(genMsg)).getDecoder().getRsslBuffer() );
	}
	else
	{
		EmaString temp( "Attempt to pass an empty message to addGenericMsg() while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void ElementListEncoder::addMap( const EmaString& name, const Map& map )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	const Encoder& enc = static_cast<const Data&>(map).getEncoder();

	if ( enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( name, RSSL_DT_MAP, "addMap()", enc.getRsslBuffer() );
		else
		{
			EmaString temp( "Attempt to addMap() while Map::complete() was not called." );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}
	}
	else if ( static_cast<const Data&>(map).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_MAP, "addMap()", static_cast<Data&>( const_cast<Map&>(map)).getDecoder().getRsslBuffer() );
	}
	else
	{
		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( name, RSSL_DT_MAP, "addMap()" );
	}
}

void ElementListEncoder::addVector( const EmaString& name, const Vector& vector )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	const Encoder& enc = static_cast<const Data&>(vector).getEncoder();

	if ( enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( name, RSSL_DT_VECTOR, "addVector()", enc.getRsslBuffer() );
		else
		{
			EmaString temp( "Attempt to addVector() while Vector::complete() was not called." );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}
	}
	else if ( static_cast<const Data&>(vector).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_VECTOR, "addVector()", static_cast<Data&>( const_cast<Vector&>(vector)).getDecoder().getRsslBuffer() );
	}
	else
	{
		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( name, RSSL_DT_VECTOR, "addVector()" );
	}
}

void ElementListEncoder::addSeries( const EmaString& name, const Series& series )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	const Encoder& enc = static_cast<const Data&>(series).getEncoder();

	if ( enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( name, RSSL_DT_SERIES, "addSeries()", enc.getRsslBuffer() );
		else
		{
			EmaString temp( "Attempt to addSeries() while Series::complete() was not called." );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}
	}
	else if ( static_cast<const Data&>(series).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_SERIES, "addSeries()", static_cast<Data&>( const_cast<Series&>(series)).getDecoder().getRsslBuffer() );
	}
	else
	{
		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( name, RSSL_DT_SERIES, "addSeries()" );
	}
}

void ElementListEncoder::addFilterList( const EmaString& name, const FilterList& filterList )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	const Encoder& enc = static_cast<const Data&>(filterList).getEncoder();

	if ( enc.ownsIterator() )
	{
		if ( enc.isComplete() )
			addEncodedEntry( name, RSSL_DT_FILTER_LIST, "addFilterList()", enc.getRsslBuffer() );
		else
		{
			EmaString temp( "Attempt to addFilterList() while FilterList::complete() was not called." );
			throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
			return;
		}
	}
	else if ( static_cast<const Data&>(filterList).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_FILTER_LIST, "addFilterList()", static_cast<Data&>( const_cast<FilterList&>(filterList)).getDecoder().getRsslBuffer() );
	}
	else
	{
		passEncIterator( const_cast<Encoder&>( enc ) );
		startEncodingEntry( name, RSSL_DT_FILTER_LIST, "addFilterList()" );
	}
}

void ElementListEncoder::addOpaque( const EmaString& name, const OmmOpaque& ommOpaque )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	const Encoder& enc = static_cast<const Data&>(ommOpaque).getEncoder();

	if ( enc.ownsIterator() )
	{
		addEncodedEntry( name, RSSL_DT_OPAQUE, "addOpaque()", enc.getRsslBuffer());
	}
	else if ( static_cast<const Data&>(ommOpaque).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_OPAQUE, "addOpaque()", static_cast<Data&>( const_cast<OmmOpaque&>(ommOpaque)).getDecoder().getRsslBuffer() );
	}
	else
	{
		EmaString temp( "Attempt to pass an empty OmmOpaque to addOpaque() while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void ElementListEncoder::addXml( const EmaString& name, const OmmXml& ommXml )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	const Encoder& enc = static_cast<const Data&>(ommXml).getEncoder();

	if ( enc.ownsIterator() )
	{
		addEncodedEntry( name, RSSL_DT_XML, "addXml()", enc.getRsslBuffer());
	}
	else if ( static_cast<const Data&>(ommXml).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_XML, "addXml()", static_cast<Data&>( const_cast<OmmXml&>(ommXml)).getDecoder().getRsslBuffer() );
	}
	else
	{
		EmaString temp( "Attempt to pass an empty OmmXml to addXml() while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void ElementListEncoder::addAnsiPage( const EmaString& name, const OmmAnsiPage& ommAnsiPage )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	const Encoder& enc = static_cast<const Data&>(ommAnsiPage).getEncoder();

	if ( enc.ownsIterator() )
	{
		addEncodedEntry( name, RSSL_DT_ANSI_PAGE, "addAnsiPage()", enc.getRsslBuffer());
	}
	else if ( static_cast<const Data&>(ommAnsiPage).hasDecoder() )
	{
		addEncodedEntry( name, RSSL_DT_ANSI_PAGE, "addAnsiPage()", static_cast<Data&>( const_cast<OmmAnsiPage&>(ommAnsiPage)).getDecoder().getRsslBuffer() );
	}
	else
	{
		EmaString temp( "Attempt to pass an empty OmmAnsiPage to addAnsiPage() while it is not supported." );
		throwIueException( temp, OmmInvalidUsageException::InvalidOperationEnum );
	}
}

void ElementListEncoder::addCodeInt( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_INT, "addCodeInt()", 0 );
}

void ElementListEncoder::addCodeUInt( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_UINT, "addCodeUInt()", 0 );
}

void ElementListEncoder::addCodeReal( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_REAL, "addCodeReal()", 0 );
}

void ElementListEncoder::addCodeFloat( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_FLOAT, "addCodeFloat()", 0 );
}

void ElementListEncoder::addCodeDouble( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_DOUBLE, "addCodeDouble()", 0 );
}

void ElementListEncoder::addCodeDate( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_DATE, "addCodeDate()", 0 );
}

void ElementListEncoder::addCodeTime( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_TIME, "addCodeTime()", 0 );
}

void ElementListEncoder::addCodeDateTime( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_DATETIME, "addCodeDateTime()", 0 );
}

void ElementListEncoder::addCodeQos( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_QOS, "addCodeQos()", 0 );
}

void ElementListEncoder::addCodeState( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_STATE, "addCodeState()", 0 );
}

void ElementListEncoder::addCodeEnum( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_ENUM, "addCodeEnum()", 0 );
}

void ElementListEncoder::addCodeBuffer( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_BUFFER, "addCodeBuffer()", 0 );
}

void ElementListEncoder::addCodeAscii( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_ASCII_STRING, "addCodeAscii()", 0 );
}

void ElementListEncoder::addCodeUtf8( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_UTF8_STRING, "addCodeUtf8()", 0 );
}

void ElementListEncoder::addCodeRmtes( const EmaString& name )
{
	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		rsslElementListApplyHasStandardData( &_rsslElementList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( name, RSSL_DT_RMTES_STRING, "addCodeRmtes()", 0 );
}

void ElementListEncoder::add( const EmaString& name )
{
	if (rsslElementListCheckHasStandardData(&_rsslElementList) == RSSL_FALSE)
	{
		rsslElementListApplyHasStandardData(&_rsslElementList);

		acquireEncIterator();

		initEncode();
	}

	RsslBuffer rsslBuffer;
	rsslClearBuffer(&rsslBuffer);

	addEncodedEntry(name, RSSL_DT_NO_DATA, "add()", rsslBuffer);
}

void ElementListEncoder::complete()
{
	if ( _containerComplete ) return;

	if ( rsslElementListCheckHasStandardData( &_rsslElementList ) == RSSL_FALSE )
	{
		acquireEncIterator();

		initEncode();
	}

	RsslRet retCode = rsslEncodeElementListComplete( &(_pEncodeIter->_rsslEncIter), RSSL_TRUE );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to complete ElementList encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp, retCode );
		return;
	}

	_pEncodeIter->setEncodedLength( rsslGetEncodedBufferLength( &(_pEncodeIter->_rsslEncIter) ) );

	if ( !ownsIterator() && _iteratorOwner )
		_iteratorOwner->endEncodingEntry();

	_containerComplete = true;
}
