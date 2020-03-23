/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
*/

#ifndef __thomsonreuters_ema_rdm_DataDictionaryImpl_h
#define __thomsonreuters_ema_rdm_DataDictionaryImpl_h

#include "rtr/rsslDataDictionary.h"

#include "EmaStringInt.h"
#include "DataDictionary.h"
#include "DictionaryEntryImpl.h"
#include "EnumTypeImpl.h"
#include "EnumTypeTableImpl.h"
#include "HashTable.h"
#include "Mutex.h"

namespace thomsonreuters {

namespace ema {

namespace rdm {

class DataDictionaryImpl
{
public:

	DataDictionaryImpl(bool);

	DataDictionaryImpl(const DataDictionaryImpl&);

	virtual ~DataDictionaryImpl();

	thomsonreuters::ema::access::Int32 getMinFid() const;

	thomsonreuters::ema::access::Int32 getMaxFid() const;

	const thomsonreuters::ema::access::EmaVector<DictionaryEntry>& getEntries() const;

	thomsonreuters::ema::access::Int32 getInfoDictionaryId() const;

	const thomsonreuters::ema::access::EmaString& getFieldVersion() const;

	const thomsonreuters::ema::access::EmaString& getEnumRecordTemplateVersion() const;

	const thomsonreuters::ema::access::EmaString& getEnumDisplayTemplateVersion() const;

	const thomsonreuters::ema::access::EmaString& getFieldFilename() const;

	const thomsonreuters::ema::access::EmaString& getFieldDescription() const;

	const thomsonreuters::ema::access::EmaString& getFieldBuild() const;

	const thomsonreuters::ema::access::EmaString& getFieldDate() const;

	const thomsonreuters::ema::access::EmaString& getEnumFilename() const;

	const thomsonreuters::ema::access::EmaString& getEnumDescription() const;

	const thomsonreuters::ema::access::EmaString& getEnumDate() const;

	bool hasEntry(thomsonreuters::ema::access::Int32 fieldId) const;

	const DictionaryEntry& getEntry(thomsonreuters::ema::access::Int32 fieldId) const;

	bool hasEntry(const thomsonreuters::ema::access::EmaString& fieldName) const;

	const DictionaryEntry& getEntry(const thomsonreuters::ema::access::EmaString& fieldName) const;

	bool hasEnumType(thomsonreuters::ema::access::Int32 fieldId, thomsonreuters::ema::access::Int32 value) const;

	const EnumType& getEnumType(thomsonreuters::ema::access::Int32 fieldId, thomsonreuters::ema::access::Int32 value) const;

	const thomsonreuters::ema::access::EmaVector<EnumTypeTable>& getEnumTables() const;

	void clear();

	void loadFieldDictionary(const thomsonreuters::ema::access::EmaString& filename);

	void loadEnumTypeDictionary(const thomsonreuters::ema::access::EmaString& filename);

	void encodeFieldDictionary(thomsonreuters::ema::access::Series& series, thomsonreuters::ema::access::UInt32 verbosity);

	bool encodeFieldDictionary(thomsonreuters::ema::access::Series& series, 
		thomsonreuters::ema::access::Int32& currentFid, thomsonreuters::ema::access::UInt32 verbosity, 
		thomsonreuters::ema::access::UInt32 fragmentationSize);

	void decodeFieldDictionary(const thomsonreuters::ema::access::Series& series, thomsonreuters::ema::access::UInt32 verbosity);

	void encodeEnumTypeDictionary(thomsonreuters::ema::access::Series& series, thomsonreuters::ema::access::UInt32 verbosity);

	bool encodeEnumTypeDictionary(thomsonreuters::ema::access::Series& series, thomsonreuters::ema::access::Int32& currenCount,
		thomsonreuters::ema::access::UInt32 verbosity, thomsonreuters::ema::access::UInt32 fragmentationSize);

	void decodeEnumTypeDictionary(const thomsonreuters::ema::access::Series& series, thomsonreuters::ema::access::UInt32 verbosity);

	thomsonreuters::ema::access::UInt32 extractDictionaryType(const thomsonreuters::ema::access::Series& series);

	void setRsslDataDictionary(const RsslDataDictionary* rsslDataDictionary);

	const thomsonreuters::ema::access::EmaString& toString() const;

private:
	
	typedef thomsonreuters::ema::access::HashTable< thomsonreuters::ema::access::EmaString, thomsonreuters::ema::access::Int16,
		thomsonreuters::ema::access::Hasher<thomsonreuters::ema::access::EmaString>,
		thomsonreuters::ema::access::Equal_To<thomsonreuters::ema::access::EmaString> > FieldNameToIdHash;

	FieldNameToIdHash* fieldNameToIdMap() const;

	static void throwIueForQueryOnly();

	mutable FieldNameToIdHash* _pfieldNameToIdHash;

	mutable RsslDataDictionary* _pRsslDataDictionary;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringInfoFieldVersion;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringInfoEnumRTVersion;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringInfoEnumDTVersion;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringInfoFieldFilename;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringInfoFieldDesc;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringInfoFieldBuild;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringInfoFieldDate;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringInfoEnumFilename;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringInfoEnumDesc;
	mutable thomsonreuters::ema::access::EmaStringInt	_stringInfoEnumDate;
	mutable thomsonreuters::ema::access::EmaString		_stringToString;

	mutable DictionaryEntry		_dictionaryEntry;
	mutable EnumType           _enumType;
	mutable thomsonreuters::ema::access::EmaVector<DictionaryEntry>*	_pDictionaryEntryList;
	mutable thomsonreuters::ema::access::EmaVector<EnumTypeTable>*	_pEnumTypeTableList;
	RsslBuffer _errorText;
	bool _loadedFieldDictionary;
	bool _loadedEnumTypeDef;
	bool _ownRsslDataDictionary;

	mutable thomsonreuters::ema::access::Mutex _dataAccessMutex;
};

}

}

}

#endif // __thomsonreuters_ema_rdm_DataDictionaryImpl_h