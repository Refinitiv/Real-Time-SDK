/*|-----------------------------------------------------------------------------
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_rdm_EnumTypeTableImpl_h
#define __refinitiv_ema_rdm_EnumTypeTableImpl_h

#include "rtr/rsslDataDictionary.h"

#include <EmaVector.h>
#include <EnumType.h>

namespace refinitiv {

namespace ema {

namespace rdm {

class EnumTypeTableImpl
{
public:

	EnumTypeTableImpl();
	virtual ~EnumTypeTableImpl();

	const refinitiv::ema::access::EmaVector<EnumType>& getEnumTypes() const;

	const refinitiv::ema::access::EmaVector<refinitiv::ema::access::Int16>& getFidReferences() const;

	RsslEnumTypeTable* getRsslEnumTypeTable() const;

	void rsslEnumTypeTable(RsslEnumTypeTable*);

	const refinitiv::ema::access::EmaString& toString() const;

private:

	mutable RsslEnumTypeTable* _pEnumTypeTable;

	mutable bool 										_refreshEnumTypeList;

	mutable refinitiv::ema::access::EmaString		_stringToString;

	mutable refinitiv::ema::access::EmaVector<EnumType>*    _pEnumTypeList;

	mutable refinitiv::ema::access::EmaVector<refinitiv::ema::access::Int16>*    _pFidsList;

	EnumTypeTableImpl(RsslEnumTypeTable*);
};

}

}

}

#endif //  __refinitiv_ema_rdm_EnumTypeTableImpl_h

