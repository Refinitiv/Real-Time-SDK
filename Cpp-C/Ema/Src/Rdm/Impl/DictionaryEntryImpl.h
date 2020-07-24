/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_rdm_DictionaryEntryImpl_h
#define __thomsonreuters_ema_rdm_DictionaryEntryImpl_h

#include "rtr/rsslDataDictionary.h"

#include "EmaStringInt.h"
#include "DictionaryEntry.h"
#include "EnumType.h"
#include "EnumTypeTable.h"

namespace thomsonreuters {

namespace ema {

namespace rdm {

class DictionaryEntryImpl
{
public:

	DictionaryEntryImpl(bool isManagedByUser);

	DictionaryEntryImpl(RsslDictionaryEntry* rsslDictionaryEntry);

	virtual ~DictionaryEntryImpl();

	void rsslDictionaryEntry(RsslDictionaryEntry* rsslDictionaryEntry);

	const thomsonreuters::ema::access::EmaString& getAcronym() const;

	const thomsonreuters::ema::access::EmaString& getDDEAcronym() const;

	thomsonreuters::ema::access::Int16 getFid() const;

	thomsonreuters::ema::access::Int16 getRippleToField() const;

	thomsonreuters::ema::access::Int8 getFieldType() const;

	thomsonreuters::ema::access::UInt16 getLength() const;

	thomsonreuters::ema::access::UInt8 getEnumLength() const;

	thomsonreuters::ema::access::UInt8 getRwfType() const;

	thomsonreuters::ema::access::UInt16 getRwfLength() const;

	bool hasEnumType(thomsonreuters::ema::access::UInt16 value) const;

	const EnumType& getEnumEntry(thomsonreuters::ema::access::UInt16 value) const;

	bool hasEnumTypeTable() const;

	const EnumTypeTable& getEnumTypeTable() const;

	RsslDictionaryEntry* getRsslDictionaryEntry();

	const thomsonreuters::ema::access::EmaString& toString() const;

	/** Detects that the instance was created by user or API.
	* @return true when the instance was created by user; otherwise when the instance was created by API.
	*/
	bool isManagedByUser() const;

	operator const char* () const;

private:

	mutable thomsonreuters::ema::access::EmaStringInt	_stringAcronym;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringDDEAcronym;
	mutable thomsonreuters::ema::access::EmaString		_stringToString;

	RsslDictionaryEntry*	_pRsslDictionaryEntry;
	EnumType				_enumType;
	mutable thomsonreuters::ema::access::EmaVector<EnumType>*    _pEnumEntryList;
	EnumTypeTable			_enumTypeTable;
	bool					_isManagedByUser;  // true when the instance is created by user, otherwise the instance is created by API
};

}

}

}

#endif // __thomsonreuters_ema_rdm_DictionaryEntryImpl_h