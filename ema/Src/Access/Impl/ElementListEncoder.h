/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_ElementListEncoder_h
#define __thomsonreuters_ema_access_ElementListEncoder_h

#include "Encoder.h"
#include "OmmReal.h"
#include "EmaPool.h"
#include "OmmState.h"
#include "rtr/rsslElementList.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmArray;
class ElementList;
class FieldList;
class FilterList;
class Map;
class Series;
class Vector;
class AckMsg;
class GenericMsg;
class PostMsg;
class ReqMsg;
class RefreshMsg;
class StatusMsg;
class UpdateMsg;
class OmmAnsiPage;
class OmmOpaque;
class OmmXml;

class ElementListEncoder : public Encoder
{
public :

	ElementListEncoder();

	virtual ~ElementListEncoder();

	void clear();

	void info( Int16 elmentListNum );

	void addInt( const EmaString& name, Int64 value );

	void addUInt( const EmaString& name, UInt64 value );

	void addReal( const EmaString& name, Int64 mantissa, OmmReal::MagnitudeType magnitudeType );

	void addRealFromDouble( const EmaString& name, double value, OmmReal::MagnitudeType magnitudeType );

	void addFloat( const EmaString& name, float value );

	void addDouble( const EmaString& name, double value );

	void addDate( const EmaString& name, UInt16 year, UInt8 month, UInt8 day );

	void addTime( const EmaString& name, UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond );

	void addDateTime( const EmaString& name,
							UInt16 year, UInt8 month, UInt8 day,
							UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond );

	void addQos( const EmaString& name, UInt32 timeliness, UInt32 rate );

	void addState( const EmaString& name,
						OmmState::StreamState streamState,
						OmmState::DataState dataState,
						UInt8 statusCode,
						const EmaString& statusText );

	void addEnum( const EmaString& name, UInt16 value );

	void addBuffer( const EmaString& name, const EmaBuffer& value );

	void addAscii( const EmaString& name, const EmaString& value );

	void addUtf8( const EmaString& name, const EmaBuffer& value );

	void addRmtes( const EmaString& name, const EmaBuffer& value );

	void addArray( const EmaString& name, const OmmArray& value );

	void addElementList( const EmaString& name, const ElementList&  value );

	void addFieldList( const EmaString& name, const FieldList& value );

	void addReqMsg( const EmaString& name, const ReqMsg& value );

	void addRefreshMsg( const EmaString& name, const RefreshMsg& value );

	void addStatusMsg( const EmaString& name, const StatusMsg& value );

	void addUpdateMsg( const EmaString& name, const UpdateMsg& value );

	void addPostMsg( const EmaString& name, const PostMsg& value );

	void addAckMsg( const EmaString& name, const AckMsg& value );

	void addGenericMsg( const EmaString& name, const GenericMsg& value );

	void addMap( const EmaString& name, const Map& value );

	void addVector( const EmaString& name, const Vector& value );

	void addSeries( const EmaString& name, const Series& value );

	void addFilterList( const EmaString& name, const FilterList& value );

	void addOpaque( const EmaString& name, const OmmOpaque& value );

	void addXml( const EmaString& name, const OmmXml& value );

	void addAnsiPage( const EmaString& name, const OmmAnsiPage& value );

	void addCodeInt( const EmaString& name );

	void addCodeUInt( const EmaString& name );

	void addCodeReal( const EmaString& name );

	void addCodeFloat( const EmaString& name );

	void addCodeDouble( const EmaString& name );

	void addCodeDate( const EmaString& name );

	void addCodeTime( const EmaString& name );

	void addCodeDateTime( const EmaString& name );

	void addCodeQos( const EmaString& name );

	void addCodeState( const EmaString& name );

	void addCodeEnum( const EmaString& name );

	void addCodeBuffer( const EmaString& name );

	void addCodeAscii( const EmaString& name );

	void addCodeUtf8( const EmaString& name );

	void addCodeRmtes( const EmaString& name );

	void complete();

private :

	void initEncode();

	void addPrimitiveEntry( const EmaString& , RsslDataType , const char* , void* );

	void addEncodedEntry( const EmaString& , RsslDataType , const char* , RsslBuffer& );

	void startEncodingEntry( const EmaString& , RsslDataType , const char* );

	void endEncodingEntry() const;

	RsslElementList				_rsslElementList;

	RsslElementEntry			_rsslElementEntry;
};


class ElementListEncoderPool : public EncoderPool< ElementListEncoder >
{
public :

	ElementListEncoderPool( unsigned int size = 5 ) : EncoderPool< ElementListEncoder >( size ) {};

	virtual ~ElementListEncoderPool() {}

private :

	ElementListEncoderPool( const ElementListEncoderPool& );
	ElementListEncoderPool& operator=( const ElementListEncoderPool& );
};

}

}

}

#endif // __thomsonreuters_ema_access_ElementListEncoder_h
