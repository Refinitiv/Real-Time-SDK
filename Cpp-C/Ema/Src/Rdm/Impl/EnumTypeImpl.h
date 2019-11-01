/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2017. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_rdm_EnumTypeImpl_h
#define __thomsonreuters_ema_rdm_EnumTypeImpl_h

#include "rtr/rsslDataDictionary.h"

#include "EnumType.h"
#include "EmaStringInt.h"

namespace thomsonreuters {

namespace ema {

namespace rdm {

class EnumTypeImpl
{
public:

	EnumTypeImpl();

	EnumTypeImpl& operator=(const EnumTypeImpl&);

	virtual ~EnumTypeImpl();

	thomsonreuters::ema::access::UInt16 getValue() const;

	const thomsonreuters::ema::access::EmaString& getDisplay() const;

	const thomsonreuters::ema::access::EmaString& getMeaning() const;

	void rsslEnumType(RsslEnumType* rsslEnumType);

	RsslEnumType*			rsslEnumType();

	const thomsonreuters::ema::access::EmaString& toString() const;

private:

	mutable thomsonreuters::ema::access::EmaStringInt	_stringDispaly;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringMeaning;
	mutable thomsonreuters::ema::access::EmaString		_stringToString;

	RsslEnumType*			_pRsslEnumType;

};

}

}

}

#endif // __thomsonreuters_ema_rdm_EnumTypeImpl_h
