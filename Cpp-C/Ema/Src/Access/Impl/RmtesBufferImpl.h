/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_RmtesBufferImpl_h
#define __refinitiv_ema_access_RmtesBufferImpl_h

#include "EmaBufferInt.h"
#include "EmaBufferU16Int.h"
#include "EmaStringInt.h"
#include "rtr/rsslRmtes.h"

namespace refinitiv {

namespace ema {

namespace access {

class RmtesBuffer;
class OmmRmtesDecoder;

class RmtesBufferImpl
{
public :

	RmtesBufferImpl();
	RmtesBufferImpl( UInt32 );
	RmtesBufferImpl( const char*, UInt32 );
	RmtesBufferImpl( const RmtesBufferImpl& );

	virtual ~RmtesBufferImpl();

	const EmaBuffer& getAsUTF8();

	const EmaBufferU16& getAsUTF16();

	const RsslBuffer& getRsslBuffer();

	const EmaString& toString();

	void apply( const RmtesBufferImpl& );

	void apply( const char* , UInt32 );

	void apply( const EmaBuffer& );

	void clear();

	void setData(const char* buf, UInt32 length);

private :
	
	void reallocateRmtesCacheBuffer( const char* errorText ); 
	
	friend class OmmRmtesDecoder;

	RsslBuffer				_rsslBuffer;
	RsslRmtesCacheBuffer	_rsslCacheBuffer; 

	RsslBuffer				_rsslUTF8Buffer; 
	RsslU16Buffer			_rsslUTF16Buffer; 

	EmaStringInt			_toString;      
	EmaBufferInt			_utf8Buffer; 
	EmaBufferU16Int			_utf16Buffer; 

	bool					_rsslUTF8BufferSet;
	bool					_rsslUTF16BufferSet;

	bool					_applyToCache;
};

}

}

}

#endif // __refinitiv_ema_access_RmtesBufferImpl_h
