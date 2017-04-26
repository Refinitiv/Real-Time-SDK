/*|-----------------------------------------------------------------------------
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_rdm_EnumTypeTableImpl_h
#define __thomsonreuters_ema_rdm_EnumTypeTableImpl_h

#include "rtr/rsslDataDictionary.h"

#include <EmaVector.h>
#include <EnumType.h>

namespace thomsonreuters {

namespace ema {

namespace rdm {

class EnumTypeTableImpl
{
public:

	EnumTypeTableImpl();

	const thomsonreuters::ema::access::EmaVector<EnumType>& getEnumTypes() const;

	const thomsonreuters::ema::access::EmaVector<thomsonreuters::ema::access::Int16>& getFidReferences() const;

	RsslEnumTypeTable* getRsslEnumTypeTable() const;

	void rsslEnumTypeTable(RsslEnumTypeTable*);

	const thomsonreuters::ema::access::EmaString& toString() const;

private:

	mutable RsslEnumTypeTable* _pEnumTypeTable;

	mutable bool 										_refreshEnumTypeList;

	mutable thomsonreuters::ema::access::EmaString		_stringToString;

	mutable thomsonreuters::ema::access::EmaVector<EnumType>*    _pEnumTypeList;

	mutable thomsonreuters::ema::access::EmaVector<thomsonreuters::ema::access::Int16>*    _pFidsList;

	EnumTypeTableImpl(RsslEnumTypeTable*);
	virtual ~EnumTypeTableImpl();
};

}

}

}

#endif //  __thomsonreuters_ema_rdm_EnumTypeTableImpl_h

