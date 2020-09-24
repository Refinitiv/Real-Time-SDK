/*|-----------------------------------------------------------------------------
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __rtsdk_ema_rdm_EnumTypeTableImpl_h
#define __rtsdk_ema_rdm_EnumTypeTableImpl_h

#include "rtr/rsslDataDictionary.h"

#include <EmaVector.h>
#include <EnumType.h>

namespace rtsdk {

namespace ema {

namespace rdm {

class EnumTypeTableImpl
{
public:

	EnumTypeTableImpl();
	virtual ~EnumTypeTableImpl();

	const rtsdk::ema::access::EmaVector<EnumType>& getEnumTypes() const;

	const rtsdk::ema::access::EmaVector<rtsdk::ema::access::Int16>& getFidReferences() const;

	RsslEnumTypeTable* getRsslEnumTypeTable() const;

	void rsslEnumTypeTable(RsslEnumTypeTable*);

	const rtsdk::ema::access::EmaString& toString() const;

private:

	mutable RsslEnumTypeTable* _pEnumTypeTable;

	mutable bool 										_refreshEnumTypeList;

	mutable rtsdk::ema::access::EmaString		_stringToString;

	mutable rtsdk::ema::access::EmaVector<EnumType>*    _pEnumTypeList;

	mutable rtsdk::ema::access::EmaVector<rtsdk::ema::access::Int16>*    _pFidsList;

	EnumTypeTableImpl(RsslEnumTypeTable*);
};

}

}

}

#endif //  __rtsdk_ema_rdm_EnumTypeTableImpl_h

