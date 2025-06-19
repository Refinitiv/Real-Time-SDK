/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017,2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_rdm_EnumTypeImpl_h
#define __refinitiv_ema_rdm_EnumTypeImpl_h

#include "rtr/rsslDataDictionary.h"

#include "EnumType.h"
#include "EmaStringInt.h"

namespace refinitiv {

namespace ema {

namespace rdm {

class EnumTypeImpl
{
public:

	EnumTypeImpl();

	EnumTypeImpl& operator=(const EnumTypeImpl&);

	virtual ~EnumTypeImpl();

	refinitiv::ema::access::UInt16 getValue() const;

	const refinitiv::ema::access::EmaString& getDisplay() const;

	const refinitiv::ema::access::EmaString& getMeaning() const;

	void rsslEnumType(RsslEnumType* rsslEnumType);

	RsslEnumType*			rsslEnumType();

	const refinitiv::ema::access::EmaString& toString() const;

private:

	mutable refinitiv::ema::access::EmaStringInt	_stringDispaly;
	mutable refinitiv::ema::access::EmaStringInt	_stringMeaning;
	mutable refinitiv::ema::access::EmaString		_stringToString;

	RsslEnumType*			_pRsslEnumType;

};

}

}

}

#endif // __refinitiv_ema_rdm_EnumTypeImpl_h
