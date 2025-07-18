/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmAnsiPageDecoder_h
#define __refinitiv_ema_access_OmmAnsiPageDecoder_h

#include "Decoder.h"
#include "EmaStringInt.h"
#include "EmaBufferInt.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmAnsiPageDecoder : public Decoder
{
public :

	OmmAnsiPageDecoder();

	virtual ~OmmAnsiPageDecoder();

	bool setRsslData( UInt8 majVer, UInt8 minVer, RsslMsg* , const RsslDataDictionary* );

	bool setRsslData( UInt8 majVer, UInt8 minVer, RsslBuffer* , const RsslDataDictionary* , void* );

	bool setRsslData( RsslDecodeIterator* , RsslBuffer* );

	Data::DataCode getCode() const;
	
	const EmaString& toString();
	
	const EmaString& getString();

	const EmaBuffer& getBuffer();

	const RsslBuffer& getRsslBuffer() const;

	OmmError::ErrorCode getErrorCode() const;

private :

	RsslBuffer				_rsslBuffer;

	EmaStringInt			_toString;

	EmaStringInt			_getString;

	EmaBufferInt			_getBuffer;

	Data::DataCode			_dataCode;

	OmmError::ErrorCode		_errorCode;
};

}

}

}

#endif // __refinitiv_ema_access_OmmAnsiPageDecoder_h
