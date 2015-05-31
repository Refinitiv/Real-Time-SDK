/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmRealDecoder_h
#define __thomsonreuters_ema_access_OmmRealDecoder_h

#include "Decoder.h"
#include "OmmReal.h"
#include "EmaBufferInt.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmRealDecoder : public Decoder
{
public :

	OmmRealDecoder();

	virtual ~OmmRealDecoder();

	Int64 getMantissa() const; 

	OmmReal::MagnitudeType getMagnitudeType() const; 

	double toDouble();

	const EmaString& toString();

	Data::DataCode getCode() const;

	Int8 getRsslIsBlank() const;

	UInt8 getRsslHint() const;

	void setRsslData( UInt8 , UInt8 , RsslMsg* , const RsslDataDictionary* );

	void setRsslData( UInt8 , UInt8 , RsslBuffer* , const RsslDataDictionary* , void* );

	void setRsslData( RsslDecodeIterator* , RsslBuffer* );

	const EmaBuffer& getHexBuffer();


private :

	RsslBuffer		_rsslBuffer;

	RsslReal		_rsslReal;

	double			_toDouble;

	EmaString		_toString;

	EmaBufferInt	_hexBuffer;

	bool			_toDoubleSet;

	bool			_toStringSet;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmRealDecoder_h
