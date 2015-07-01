/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmXmlDecoder_h
#define __thomsonreuters_ema_access_OmmXmlDecoder_h

#include "Decoder.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmXmlDecoder : public Decoder
{
public :

	OmmXmlDecoder();

	virtual ~OmmXmlDecoder();

	void setRsslData( UInt8 majVer, UInt8 minVer, RsslMsg* , const RsslDataDictionary* );

	void setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* , const RsslDataDictionary* , void* );

	void setRsslData( RsslDecodeIterator* , RsslBuffer* );

	Data::DataCode getCode() const;

	const EmaString& toString();

	const EmaString& getString();

	const EmaBuffer& getBuffer();
	
private :

	RsslBuffer				_rsslBuffer;

	EmaStringInt			_toString;

	EmaStringInt			_getString;

	EmaBufferInt			_getBuffer;

	Data::DataCode			_dataCode;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmXmlDecoder_h
