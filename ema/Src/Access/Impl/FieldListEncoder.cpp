/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "FieldListEncoder.h"
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
#include "StatusMsg.h"
#include "UpdateMsg.h"

using namespace thomsonreuters::ema::access;

extern const EmaString& getMTypeAsString( OmmReal::MagnitudeType mType );

FieldListEncoder::FieldListEncoder() :
 _rsslFieldList(),
 _rsslFieldEntry()
{
	rsslClearFieldList( &_rsslFieldList );
	rsslClearFieldEntry( &_rsslFieldEntry );
}

FieldListEncoder::~FieldListEncoder()
{
}

void FieldListEncoder::clear()
{
	Encoder::releaseEncIterator();

	rsslClearFieldList( &_rsslFieldList );
	rsslClearFieldEntry( &_rsslFieldEntry );
}

void FieldListEncoder::info( Int16 dictionaryId, Int16 fieldListNum )
{
	_rsslFieldList.dictionaryId = dictionaryId;
	_rsslFieldList.fieldListNum = fieldListNum;
	rsslFieldListApplyHasInfo( &_rsslFieldList );
}

void FieldListEncoder::initEncode()
{
	RsslRet retCode = rsslEncodeFieldListInit( &(_pEncodeIter->_rsslEncIter), &_rsslFieldList, 0, 0 );

	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		retCode = rsslEncodeFieldListComplete( &(_pEncodeIter->_rsslEncIter), RSSL_FALSE );

		_pEncodeIter->reallocate();

		retCode = rsslEncodeFieldListInit( &(_pEncodeIter->_rsslEncIter), &_rsslFieldList, 0, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to initialize FieldList encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void FieldListEncoder::addPrimitiveEntry( Int16 fieldId, RsslDataType rsslDataType,
										 const char* methodName, void* value )
{
	_rsslFieldEntry.encData.data = 0;
	_rsslFieldEntry.encData.length = 0;

	_rsslFieldEntry.dataType = rsslDataType;

	_rsslFieldEntry.fieldId = fieldId;

	RsslRet retCode = rsslEncodeFieldEntry( &_pEncodeIter->_rsslEncIter, &_rsslFieldEntry, value );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeFieldEntry( &_pEncodeIter->_rsslEncIter, &_rsslFieldEntry, value );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to " );
		temp.append( methodName ).append( " while encoding FieldList. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void FieldListEncoder::addEncodedEntry( Int16 fieldId, RsslDataType rsslDataType, const char* methodName, RsslBuffer& rsslBuffer )
{
	_rsslFieldEntry.encData = rsslBuffer;

	_rsslFieldEntry.dataType = rsslDataType;

	_rsslFieldEntry.fieldId = fieldId;

	RsslRet retCode = rsslEncodeFieldEntry( &_pEncodeIter->_rsslEncIter, &_rsslFieldEntry, 0 );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeFieldEntry( &_pEncodeIter->_rsslEncIter, &_rsslFieldEntry, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to " );
		temp.append( methodName ).append( " while encoding FieldList. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void FieldListEncoder::startEncodingEntry( Int16 fieldId, RsslDataType rsslDataType, const char* methodName )
{
	_rsslFieldEntry.encData.data = 0;
	_rsslFieldEntry.encData.length = 0;

	_rsslFieldEntry.dataType = rsslDataType;

	_rsslFieldEntry.fieldId = fieldId;

	RsslRet retCode = rsslEncodeFieldEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslFieldEntry, 0 );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeFieldEntryInit( &_pEncodeIter->_rsslEncIter, &_rsslFieldEntry, 0 );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to start encoding entry in FieldList::" );
		temp.append( methodName ).append( ". Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void FieldListEncoder::endEncodingEntry() const
{
	RsslRet retCode = rsslEncodeFieldEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	while ( retCode == RSSL_RET_BUFFER_TOO_SMALL )
	{
		_pEncodeIter->reallocate();

		retCode = rsslEncodeElementEntryComplete( &_pEncodeIter->_rsslEncIter, RSSL_TRUE );
	}

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to end encoding entry in FieldList. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
	}
}

void FieldListEncoder::addInt( Int16 fieldId, Int64 value )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_INT, "addInt()", &value );
}

void FieldListEncoder::addUInt( Int16 fieldId, UInt64 value )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_UINT, "addUInt()", &value );
}

void FieldListEncoder::addReal( Int16 fieldId, Int64 mantissa, OmmReal::MagnitudeType magnitudeType )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	RsslReal real;
	real.isBlank = RSSL_FALSE;
	real.value = mantissa;
	real.hint = magnitudeType;

	addPrimitiveEntry( fieldId, RSSL_DT_REAL, "addReal()", &real );
}

void FieldListEncoder::addRealFromDouble( Int16 fieldId, double value,
							OmmReal::MagnitudeType magnitudeType )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	RsslReal real;
	if ( RSSL_RET_SUCCESS != rsslDoubleToReal( &real, &value, magnitudeType ) )
	{
		EmaString temp( "Attempt to addRealFromDouble() with invalid magnitudeType='" );
		temp.append( getMTypeAsString( magnitudeType) ).append( "'. " );
		throwIueException( temp );
		return;
	}

	addPrimitiveEntry( fieldId, RSSL_DT_REAL, "addRealFromDouble()", &real );
}

void FieldListEncoder::addFloat( Int16 fieldId, float value )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_FLOAT, "addFloat()", &value );
}

void FieldListEncoder::addDouble( Int16 fieldId, double value )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_DOUBLE, "addDouble()", &value );
}

void FieldListEncoder::addDate( Int16 fieldId, UInt16 year, UInt8 month, UInt8 day )
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

	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_DATE, "addDate()", &date );
}

void FieldListEncoder::addTime( Int16 fieldId, UInt8 hour, UInt8 minute, UInt8 second,
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
			append( (UInt32)second ).append( "." ).
			append( (UInt32)second ).append( "." ).
			append( (UInt32)nanosecond ).append( "'." );
		throwOorException( temp );
		return;
	}

	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_TIME, "addTime()", &time );
}

void FieldListEncoder::addDateTime( Int16 fieldId,
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
			append( (UInt32)second ).append( "." ).
			append( (UInt32)second ).append( "." ).
			append( (UInt32)nanosecond ).append( "'." );
		throwOorException( temp );
		return;
	}

	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_DATETIME, "addDateTime()", &dateTime );
}

void FieldListEncoder::addQos( Int16 fieldId,
					UInt32 timeliness, UInt32 rate )					
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	RsslQos qos;
	OmmQosDecoder::convertToRssl( &qos, timeliness, rate );

	addPrimitiveEntry( fieldId, RSSL_DT_QOS, "addQos()", &qos );
}

void FieldListEncoder::addState( Int16 fieldId,
					OmmState::StreamState streamState,
					OmmState::DataState dataState,
					UInt8 statusCode,
					const EmaString& statusText )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	RsslState state;
	state.streamState = streamState;
	state.dataState = dataState;
	state.code = statusCode;
	state.text.data = (char*)statusText.c_str();
	state.text.length = statusText.length();

	addPrimitiveEntry( fieldId, RSSL_DT_STATE, "addState()", &state );
}

void FieldListEncoder::addEnum( Int16 fieldId, UInt16 value )	
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_ENUM, "addEnum()", &value );
}

void FieldListEncoder::addBuffer( Int16 fieldId, const EmaBuffer& value )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_buf();
	buffer.length = value.length();

	addPrimitiveEntry( fieldId, RSSL_DT_BUFFER, "addBuffer()", &buffer );
}

void FieldListEncoder::addAscii( Int16 fieldId, const EmaString& value )	
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_str();
	buffer.length = value.length();

	addPrimitiveEntry( fieldId, RSSL_DT_ASCII_STRING, "addAscii()", &buffer );
}

void FieldListEncoder::addUtf8( Int16 fieldId, const EmaBuffer& value )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_buf();
	buffer.length = value.length();

	addPrimitiveEntry( fieldId, RSSL_DT_UTF8_STRING, "addUtf8()", &buffer );
}

void FieldListEncoder::addRmtes( Int16 fieldId, const EmaBuffer& value )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	RsslBuffer buffer;
	buffer.data = (char*)value.c_buf();
	buffer.length = value.length();

	addPrimitiveEntry( fieldId, RSSL_DT_RMTES_STRING, "addRmtes()", &buffer );
}

void FieldListEncoder::addArray( Int16 fieldId, const OmmArray& array )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(array).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called
		addEncodedEntry( fieldId, RSSL_DT_ARRAY, "addArray()", static_cast<const Data&>(array).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const Data&>(array).getEncoder() ) );
		startEncodingEntry( fieldId, RSSL_DT_ARRAY, "addArray()" );
	}
}

void FieldListEncoder::addElementList( Int16 fieldId, const ElementList& elementList )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(elementList).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called
		addEncodedEntry( fieldId, RSSL_DT_ELEMENT_LIST, "addElementList()", static_cast<const Data&>(elementList).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const Data&>(elementList).getEncoder() ) );
		startEncodingEntry( fieldId, RSSL_DT_ELEMENT_LIST, "addElementList()" );
	}
}

void FieldListEncoder::addFieldList( Int16 fieldId, const FieldList& fieldList )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(fieldList).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called		
		addEncodedEntry( fieldId, RSSL_DT_FIELD_LIST, "addFieldList()", static_cast<const Data&>(fieldList).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const Data&>(fieldList).getEncoder() ) );
		startEncodingEntry( fieldId, RSSL_DT_FIELD_LIST, "addFieldList()" );
	}
}

void FieldListEncoder::addReqMsg( Int16 fieldId, const ReqMsg& reqMsg )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(reqMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( fieldId, RSSL_DT_MSG, "addReqMsg()", static_cast<const Data&>(reqMsg).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... exception?
	}
}

void FieldListEncoder::addRefreshMsg( Int16 fieldId, const RefreshMsg& refreshMsg )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(refreshMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( fieldId, RSSL_DT_MSG, "addRefreshMsg()", static_cast<const Data&>(refreshMsg).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... exception?
	}
}

void FieldListEncoder::addStatusMsg( Int16 fieldId, const StatusMsg& statusMsg )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(statusMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( fieldId, RSSL_DT_MSG, "addStatusMsg()", static_cast<const Data&>(statusMsg).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... exception?
	}
}

void FieldListEncoder::addUpdateMsg( Int16 fieldId, const UpdateMsg& updateMsg )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(updateMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( fieldId, RSSL_DT_MSG, "addUpdateMsg()", static_cast<const Data&>(updateMsg).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... exception?
	}
}

void FieldListEncoder::addPostMsg( Int16 fieldId, const PostMsg& postMsg )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(postMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( fieldId, RSSL_DT_MSG, "addPostMsg()", static_cast<const Data&>(postMsg).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... exception?
	}
}

void FieldListEncoder::addAckMsg( Int16 fieldId, const AckMsg& ackMsg )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(ackMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( fieldId, RSSL_DT_MSG, "addAckMsg()", static_cast<const Data&>(ackMsg).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... exception?
	}
}

void FieldListEncoder::addGenericMsg( Int16 fieldId, const GenericMsg& genMsg )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(genMsg).getEncoder().ownsIterator() )
	{
		addEncodedEntry( fieldId, RSSL_DT_MSG, "addGenericMsg()", static_cast<const Data&>(genMsg).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... exception?
	}
}

void FieldListEncoder::addMap( Int16 fieldId, const Map& map )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(map).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called
		addEncodedEntry( fieldId, RSSL_DT_MAP, "addMap()", static_cast<const Data&>(map).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const Data&>(map).getEncoder() ) );
		startEncodingEntry( fieldId, RSSL_DT_MAP, "addMap()" );
	}
}

void FieldListEncoder::addVector( Int16 fieldId, const Vector& vector )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(vector).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called
		addEncodedEntry( fieldId, RSSL_DT_VECTOR, "addVector()", static_cast<const Data&>(vector).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const Data&>(vector).getEncoder() ) );
		startEncodingEntry( fieldId, RSSL_DT_VECTOR, "addVector()" );
	}
}

void FieldListEncoder::addSeries( Int16 fieldId, const Series& series )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(series).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called
		addEncodedEntry( fieldId, RSSL_DT_SERIES, "addSeries()", static_cast<const Data&>(series).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const Data&>(series).getEncoder() ) );
		startEncodingEntry( fieldId, RSSL_DT_SERIES, "addSeries()" );
	}
}

void FieldListEncoder::addFilterList( Int16 fieldId, const FilterList& filterList )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(filterList).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called
		addEncodedEntry( fieldId, RSSL_DT_FILTER_LIST, "addFilterList()", static_cast<const Data&>(filterList).getEncoder().getRsslBuffer() );
	}
	else
	{
		// todo ... check if clear was called
		passEncIterator( const_cast<Encoder&>( static_cast<const Data&>(filterList).getEncoder() ) );
		startEncodingEntry( fieldId, RSSL_DT_FILTER_LIST, "addFilterList()" );
	}
}

void FieldListEncoder::addOpaque( Int16 fieldId, const OmmOpaque& ommOpaque )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(ommOpaque).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called
		addEncodedEntry( fieldId, RSSL_DT_OPAQUE, "addOpaque()", static_cast<const Data&>(ommOpaque).getEncoder().getRsslBuffer() );
	}
}

void FieldListEncoder::addXml( Int16 fieldId, const OmmXml& ommXml )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(ommXml).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called
		addEncodedEntry( fieldId, RSSL_DT_XML, "addXml()", static_cast<const Data&>(ommXml).getEncoder().getRsslBuffer() );
	}
}

void FieldListEncoder::addAnsiPage( Int16 fieldId, const OmmAnsiPage& ommAnsiPage )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	if ( static_cast<const Data&>(ommAnsiPage).getEncoder().ownsIterator() )
	{
		// todo ... check if complete was called
		addEncodedEntry( fieldId, RSSL_DT_ANSI_PAGE, "addAnsiPage()", static_cast<const Data&>(ommAnsiPage).getEncoder().getRsslBuffer() );
	}
}

void FieldListEncoder::addCodeInt( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_INT, "addCodeInt()", 0 );
}

void FieldListEncoder::addCodeUInt( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_UINT, "addCodeUInt()", 0 );
}

void FieldListEncoder::addCodeReal( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_REAL, "addCodeReal()", 0 );
}

void FieldListEncoder::addCodeFloat( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_FLOAT, "addCodeFloat()", 0 );
}

void FieldListEncoder::addCodeDouble( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_DOUBLE, "addCodeDouble()", 0 );
}

void FieldListEncoder::addCodeDate( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_DATE, "addCodeDate()", 0 );
}

void FieldListEncoder::addCodeTime( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_TIME, "addCodeTime()", 0 );
}

void FieldListEncoder::addCodeDateTime( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_DATETIME, "addCodeDateTime()", 0 );
}

void FieldListEncoder::addCodeQos( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_QOS, "addCodeQos()", 0 );
}

void FieldListEncoder::addCodeState( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_STATE, "addCodeState()", 0 );
}

void FieldListEncoder::addCodeEnum( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_ENUM, "addCodeEnum()", 0 );
}

void FieldListEncoder::addCodeBuffer( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_BUFFER, "addCodeBuffer()", 0 );
}

void FieldListEncoder::addCodeAscii( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_ASCII_STRING, "addCodeAscii()", 0 );
}

void FieldListEncoder::addCodeUtf8( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_UTF8_STRING, "addCodeUtf8()", 0 );
}

void FieldListEncoder::addCodeRmtes( Int16 fieldId )
{
	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		rsslFieldListApplyHasStandardData( &_rsslFieldList );

		acquireEncIterator();

		initEncode();
	}

	addPrimitiveEntry( fieldId, RSSL_DT_RMTES_STRING, "addCodeRmtes()", 0 );
}

void FieldListEncoder::complete()
{
	if ( !hasEncIterator() )
	{
		EmaString temp( "Cannot complete an empty FieldList" );
		throwIueException( temp );
		return;
	}

	if ( rsslFieldListCheckHasStandardData( &_rsslFieldList ) == RSSL_FALSE )
	{
		// todo ... empty element list?
	}

	RsslRet retCode = rsslEncodeFieldListComplete( &(_pEncodeIter->_rsslEncIter), RSSL_TRUE );

	if ( retCode < RSSL_RET_SUCCESS )
	{
		EmaString temp( "Failed to complete FieldList encoding. Reason='" );
		temp.append( rsslRetCodeToString( retCode ) ).append( "'. " );
		throwIueException( temp );
		return;
	}

	_pEncodeIter->setEncodedLength( rsslGetEncodedBufferLength( &(_pEncodeIter->_rsslEncIter) ) );

	if ( !ownsIterator() && _iteratorOwner )
		_iteratorOwner->endEncodingEntry();

	_containerComplete = true;
}
