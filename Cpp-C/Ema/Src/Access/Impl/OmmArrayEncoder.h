/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmArrayEncoder_h
#define __refinitiv_ema_access_OmmArrayEncoder_h

#include "Encoder.h"
#include "OmmReal.h"
#include "EmaPool.h"
#include "rtr/rsslArray.h"
#include "OmmState.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmArrayEncoder : public Encoder
{
public :

	OmmArrayEncoder();

	virtual ~OmmArrayEncoder();

	void clear();

	void fixedWidth( UInt16 );

	void addInt( Int64 );

	void addUInt( UInt64 );

	void addReal( Int64 , OmmReal::MagnitudeType );

	void addRealFromDouble( double , OmmReal::MagnitudeType );

	void addFloat( float );

	void addDouble( double );

	void addDate( UInt16 year, UInt8 month, UInt8 day );

	void addTime( UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond );

	void addDateTime( UInt16 year, UInt8 month, UInt8 day,
					   UInt8 hour, UInt8 minute, UInt8 second, UInt16 millisecond, UInt16 microsecond, UInt16 nanosecond );

	void addQos( UInt32 timeliness, UInt32 rate );

	void addState( OmmState::StreamState streamState,
					OmmState::DataState dataState,
					UInt8 statusCode,
					const EmaString& statusText );

	void addEnum( UInt16 value );

	void addBuffer( const EmaBuffer& value );

	void addAscii( const EmaString& value );

	void addUtf8( const EmaBuffer& value );

	void addRmtes( const EmaBuffer& value );

	void addCodeInt();

	void addCodeUInt();

	void addCodeReal();

	void addCodeFloat();

	void addCodeDouble();

	void addCodeDate();

	void addCodeTime();

	void addCodeDateTime();

	void addCodeQos();

	void addCodeState();

	void addCodeEnum();

	void addCodeBuffer();

	void addCodeAscii();

	void addCodeUtf8();

	void addCodeRmtes();

	void complete();

private :

	void initEncode();

	void addPrimitiveEntry( const char* , void* );

	void endEncodingEntry() const;

	RsslArray			_rsslArray;
};

class OmmArrayEncoderPool : public EncoderPool< OmmArrayEncoder >
{
public :

	OmmArrayEncoderPool( unsigned int size = 5 ) : EncoderPool< OmmArrayEncoder >( size ) {};

	virtual ~OmmArrayEncoderPool() {}

private :

	OmmArrayEncoderPool( const OmmArrayEncoderPool& );
	OmmArrayEncoderPool& operator=( const OmmArrayEncoderPool& );
};

}

}

}

#endif // __refinitiv_ema_access_OmmArrayEncoder_h
