/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2020 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
*/

#ifndef __refinitiv_ema_rdm_DataDictionaryImpl_h
#define __refinitiv_ema_rdm_DataDictionaryImpl_h

#include "rtr/rsslDataDictionary.h"

#include "EmaStringInt.h"
#include "DataDictionary.h"
#include "DictionaryEntryImpl.h"
#include "EnumTypeImpl.h"
#include "EnumTypeTableImpl.h"
#include "HashTable.h"
#include "Mutex.h"
#include "rtr/rtratomic.h"

namespace refinitiv {

namespace ema {

namespace rdm {

class DataDictionaryImpl
{
public:

	DataDictionaryImpl(bool);

	DataDictionaryImpl(const DataDictionaryImpl&);

	virtual ~DataDictionaryImpl();

	refinitiv::ema::access::Int32 getMinFid() const;

	refinitiv::ema::access::Int32 getMaxFid() const;

	const refinitiv::ema::access::EmaVector<DictionaryEntry>& getEntries() const;

	refinitiv::ema::access::Int32 getInfoDictionaryId() const;

	const refinitiv::ema::access::EmaString& getFieldVersion() const;

	const refinitiv::ema::access::EmaString& getEnumRecordTemplateVersion() const;

	const refinitiv::ema::access::EmaString& getEnumDisplayTemplateVersion() const;

	const refinitiv::ema::access::EmaString& getFieldFilename() const;

	const refinitiv::ema::access::EmaString& getFieldDescription() const;

	const refinitiv::ema::access::EmaString& getFieldBuild() const;

	const refinitiv::ema::access::EmaString& getFieldDate() const;

	const refinitiv::ema::access::EmaString& getEnumFilename() const;

	const refinitiv::ema::access::EmaString& getEnumDescription() const;

	const refinitiv::ema::access::EmaString& getEnumDate() const;

	bool hasEntry(refinitiv::ema::access::Int32 fieldId) const;

	void getEntry(refinitiv::ema::access::Int32 fieldId, DictionaryEntry& entry) const;

	const DictionaryEntry& getEntry(refinitiv::ema::access::Int32 fieldId) const;

	bool hasEntry(const refinitiv::ema::access::EmaString& fieldName) const;

	void getEntry(const refinitiv::ema::access::EmaString& fieldName, DictionaryEntry& entry) const;

	const DictionaryEntry& getEntry(const refinitiv::ema::access::EmaString& fieldName) const;

	bool hasEnumType(refinitiv::ema::access::Int32 fieldId, refinitiv::ema::access::Int32 value) const;

	const EnumType& getEnumType(refinitiv::ema::access::Int32 fieldId, refinitiv::ema::access::Int32 value) const;

	const refinitiv::ema::access::EmaVector<EnumTypeTable>& getEnumTables() const;

	void clear();

	void loadFieldDictionary(const refinitiv::ema::access::EmaString& filename);

	void loadEnumTypeDictionary(const refinitiv::ema::access::EmaString& filename);

	void encodeFieldDictionary(refinitiv::ema::access::Series& series, refinitiv::ema::access::UInt32 verbosity);

	bool encodeFieldDictionary(refinitiv::ema::access::Series& series, 
		refinitiv::ema::access::Int32& currentFid, refinitiv::ema::access::UInt32 verbosity, 
		refinitiv::ema::access::UInt32 fragmentationSize);

	void decodeFieldDictionary(const refinitiv::ema::access::Series& series, refinitiv::ema::access::UInt32 verbosity);

	void encodeEnumTypeDictionary(refinitiv::ema::access::Series& series, refinitiv::ema::access::UInt32 verbosity);

	bool encodeEnumTypeDictionary(refinitiv::ema::access::Series& series, refinitiv::ema::access::Int32& currenCount,
		refinitiv::ema::access::UInt32 verbosity, refinitiv::ema::access::UInt32 fragmentationSize);

	void decodeEnumTypeDictionary(const refinitiv::ema::access::Series& series, refinitiv::ema::access::UInt32 verbosity);

	refinitiv::ema::access::UInt32 extractDictionaryType(const refinitiv::ema::access::Series& series);

	void setRsslDataDictionary(const RsslDataDictionary* rsslDataDictionary);

	bool isFieldDictionaryLoaded();

	bool isEnumTypeDefLoaded();

	RsslDataDictionary* rsslDataDictionary();

	void incDataDictionaryRefCount();

	void decDataDictionaryRefCount();

	rtr_atomic_val getDataDictionaryRefCount();

	const refinitiv::ema::access::EmaString& toString() const;

private:
	
	typedef refinitiv::ema::access::HashTable< refinitiv::ema::access::EmaString, refinitiv::ema::access::Int16,
		refinitiv::ema::access::Hasher<refinitiv::ema::access::EmaString>,
		refinitiv::ema::access::Equal_To<refinitiv::ema::access::EmaString> > FieldNameToIdHash;

	FieldNameToIdHash* fieldNameToIdMap() const;

	static void throwIueForQueryOnly();

	void getEntryInt(refinitiv::ema::access::Int32 fieldId, DictionaryEntry& entry) const;

	void getEntryInt(const refinitiv::ema::access::EmaString& fieldName, DictionaryEntry& entry) const;

	mutable FieldNameToIdHash* _pfieldNameToIdHash;

	mutable RsslDataDictionary* _pRsslDataDictionary;
	mutable refinitiv::ema::access::EmaStringInt	_stringInfoFieldVersion;
	mutable refinitiv::ema::access::EmaStringInt	_stringInfoEnumRTVersion;
	mutable refinitiv::ema::access::EmaStringInt	_stringInfoEnumDTVersion;
	mutable refinitiv::ema::access::EmaStringInt	_stringInfoFieldFilename;
	mutable refinitiv::ema::access::EmaStringInt	_stringInfoFieldDesc;
	mutable refinitiv::ema::access::EmaStringInt	_stringInfoFieldBuild;
	mutable refinitiv::ema::access::EmaStringInt	_stringInfoFieldDate;
	mutable refinitiv::ema::access::EmaStringInt	_stringInfoEnumFilename;
	mutable refinitiv::ema::access::EmaStringInt	_stringInfoEnumDesc;
	mutable refinitiv::ema::access::EmaStringInt	_stringInfoEnumDate;
	mutable refinitiv::ema::access::EmaString		_stringToString;

	mutable DictionaryEntry		_dictionaryEntry;
	mutable EnumType           _enumType;
	mutable refinitiv::ema::access::EmaVector<DictionaryEntry>*	_pDictionaryEntryList;
	mutable refinitiv::ema::access::EmaVector<EnumTypeTable>*	_pEnumTypeTableList;
	RsslBuffer _errorText;
	bool _loadedFieldDictionary;
	bool _loadedEnumTypeDef;
	bool _ownRsslDataDictionary;
	rtr_atomic_val _dataDictionaryRefConuter;

	mutable refinitiv::ema::access::Mutex _dataAccessMutex;
};

}

}

}

#endif // __refinitiv_ema_rdm_DataDictionaryImpl_h
