/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|          Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
 *|-----------------------------------------------------------------------------
*/

#ifndef __rtsdk_ema_rdm_DataDictionaryImpl_h
#define __rtsdk_ema_rdm_DataDictionaryImpl_h

#include "rtr/rsslDataDictionary.h"

#include "EmaStringInt.h"
#include "DataDictionary.h"
#include "DictionaryEntryImpl.h"
#include "EnumTypeImpl.h"
#include "EnumTypeTableImpl.h"
#include "HashTable.h"
#include "Mutex.h"

namespace rtsdk {

namespace ema {

namespace rdm {

class DataDictionaryImpl
{
public:

	DataDictionaryImpl(bool);

	DataDictionaryImpl(const DataDictionaryImpl&);

	virtual ~DataDictionaryImpl();

	rtsdk::ema::access::Int32 getMinFid() const;

	rtsdk::ema::access::Int32 getMaxFid() const;

	const rtsdk::ema::access::EmaVector<DictionaryEntry>& getEntries() const;

	rtsdk::ema::access::Int32 getInfoDictionaryId() const;

	const rtsdk::ema::access::EmaString& getFieldVersion() const;

	const rtsdk::ema::access::EmaString& getEnumRecordTemplateVersion() const;

	const rtsdk::ema::access::EmaString& getEnumDisplayTemplateVersion() const;

	const rtsdk::ema::access::EmaString& getFieldFilename() const;

	const rtsdk::ema::access::EmaString& getFieldDescription() const;

	const rtsdk::ema::access::EmaString& getFieldBuild() const;

	const rtsdk::ema::access::EmaString& getFieldDate() const;

	const rtsdk::ema::access::EmaString& getEnumFilename() const;

	const rtsdk::ema::access::EmaString& getEnumDescription() const;

	const rtsdk::ema::access::EmaString& getEnumDate() const;

	bool hasEntry(rtsdk::ema::access::Int32 fieldId) const;

	void getEntry(rtsdk::ema::access::Int32 fieldId, DictionaryEntry& entry) const;

	const DictionaryEntry& getEntry(rtsdk::ema::access::Int32 fieldId) const;

	bool hasEntry(const rtsdk::ema::access::EmaString& fieldName) const;

	void getEntry(const rtsdk::ema::access::EmaString& fieldName, DictionaryEntry& entry) const;

	const DictionaryEntry& getEntry(const rtsdk::ema::access::EmaString& fieldName) const;

	bool hasEnumType(rtsdk::ema::access::Int32 fieldId, rtsdk::ema::access::Int32 value) const;

	const EnumType& getEnumType(rtsdk::ema::access::Int32 fieldId, rtsdk::ema::access::Int32 value) const;

	const rtsdk::ema::access::EmaVector<EnumTypeTable>& getEnumTables() const;

	void clear();

	void loadFieldDictionary(const rtsdk::ema::access::EmaString& filename);

	void loadEnumTypeDictionary(const rtsdk::ema::access::EmaString& filename);

	void encodeFieldDictionary(rtsdk::ema::access::Series& series, rtsdk::ema::access::UInt32 verbosity);

	bool encodeFieldDictionary(rtsdk::ema::access::Series& series, 
		rtsdk::ema::access::Int32& currentFid, rtsdk::ema::access::UInt32 verbosity, 
		rtsdk::ema::access::UInt32 fragmentationSize);

	void decodeFieldDictionary(const rtsdk::ema::access::Series& series, rtsdk::ema::access::UInt32 verbosity);

	void encodeEnumTypeDictionary(rtsdk::ema::access::Series& series, rtsdk::ema::access::UInt32 verbosity);

	bool encodeEnumTypeDictionary(rtsdk::ema::access::Series& series, rtsdk::ema::access::Int32& currenCount,
		rtsdk::ema::access::UInt32 verbosity, rtsdk::ema::access::UInt32 fragmentationSize);

	void decodeEnumTypeDictionary(const rtsdk::ema::access::Series& series, rtsdk::ema::access::UInt32 verbosity);

	rtsdk::ema::access::UInt32 extractDictionaryType(const rtsdk::ema::access::Series& series);

	void setRsslDataDictionary(const RsslDataDictionary* rsslDataDictionary);

	const rtsdk::ema::access::EmaString& toString() const;

private:
	
	typedef rtsdk::ema::access::HashTable< rtsdk::ema::access::EmaString, rtsdk::ema::access::Int16,
		rtsdk::ema::access::Hasher<rtsdk::ema::access::EmaString>,
		rtsdk::ema::access::Equal_To<rtsdk::ema::access::EmaString> > FieldNameToIdHash;

	FieldNameToIdHash* fieldNameToIdMap() const;

	static void throwIueForQueryOnly();

	void getEntryInt(rtsdk::ema::access::Int32 fieldId, DictionaryEntry& entry) const;

	void getEntryInt(const rtsdk::ema::access::EmaString& fieldName, DictionaryEntry& entry) const;

	mutable FieldNameToIdHash* _pfieldNameToIdHash;

	mutable RsslDataDictionary* _pRsslDataDictionary;
	mutable rtsdk::ema::access::EmaStringInt	_stringInfoFieldVersion;
	mutable rtsdk::ema::access::EmaStringInt	_stringInfoEnumRTVersion;
	mutable rtsdk::ema::access::EmaStringInt	_stringInfoEnumDTVersion;
	mutable rtsdk::ema::access::EmaStringInt	_stringInfoFieldFilename;
	mutable rtsdk::ema::access::EmaStringInt	_stringInfoFieldDesc;
	mutable rtsdk::ema::access::EmaStringInt	_stringInfoFieldBuild;
	mutable rtsdk::ema::access::EmaStringInt	_stringInfoFieldDate;
	mutable rtsdk::ema::access::EmaStringInt	_stringInfoEnumFilename;
	mutable rtsdk::ema::access::EmaStringInt	_stringInfoEnumDesc;
	mutable rtsdk::ema::access::EmaStringInt	_stringInfoEnumDate;
	mutable rtsdk::ema::access::EmaString		_stringToString;

	mutable DictionaryEntry		_dictionaryEntry;
	mutable EnumType           _enumType;
	mutable rtsdk::ema::access::EmaVector<DictionaryEntry>*	_pDictionaryEntryList;
	mutable rtsdk::ema::access::EmaVector<EnumTypeTable>*	_pEnumTypeTableList;
	RsslBuffer _errorText;
	bool _loadedFieldDictionary;
	bool _loadedEnumTypeDef;
	bool _ownRsslDataDictionary;

	mutable rtsdk::ema::access::Mutex _dataAccessMutex;
};

}

}

}

#endif // __rtsdk_ema_rdm_DataDictionaryImpl_h
