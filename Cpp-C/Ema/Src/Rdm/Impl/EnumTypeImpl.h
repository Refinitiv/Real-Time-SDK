/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_rdm_EnumTypeImpl_h
#define __rtsdk_ema_rdm_EnumTypeImpl_h

#include "rtr/rsslDataDictionary.h"

#include "EnumType.h"
#include "EmaStringInt.h"

namespace rtsdk {

namespace ema {

namespace rdm {

class EnumTypeImpl
{
public:

	EnumTypeImpl();

	EnumTypeImpl& operator=(const EnumTypeImpl&);

	virtual ~EnumTypeImpl();

	rtsdk::ema::access::UInt16 getValue() const;

	const rtsdk::ema::access::EmaString& getDisplay() const;

	const rtsdk::ema::access::EmaString& getMeaning() const;

	void rsslEnumType(RsslEnumType* rsslEnumType);

	RsslEnumType*			rsslEnumType();

	const rtsdk::ema::access::EmaString& toString() const;

private:

	mutable rtsdk::ema::access::EmaStringInt	_stringDispaly;
	mutable rtsdk::ema::access::EmaStringInt	_stringMeaning;
	mutable rtsdk::ema::access::EmaString		_stringToString;

	RsslEnumType*			_pRsslEnumType;

};

}

}

}

#endif // __rtsdk_ema_rdm_EnumTypeImpl_h
