/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2017. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "DataDictionary.h"
#include "ExceptionTranslator.h"
#include "DataDictionaryImpl.h"

#include <new>

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

DataDictionary::DataDictionary() :
_pImpl(0)
{
	try
	{
		_pImpl = new DataDictionaryImpl(true);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for DataDictionaryImpl in DataDictionary::DataDictionary().");
	}
}

DataDictionary::DataDictionary(bool ownRsslDataDictionary) :
_pImpl(0)
{
	try
	{
		_pImpl = new DataDictionaryImpl(ownRsslDataDictionary);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for DataDictionaryImpl in DataDictionary::DataDictionary(bool).");
	}
}
 
DataDictionary::DataDictionary(const DataDictionary& dataDictionary) :
_pImpl(0)
{
	try
	{
		_pImpl = new DataDictionaryImpl(*dataDictionary._pImpl);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for DataDictionaryImpl in DataDictionary::DataDictionary(const DataDictionary&).");
	}
}

DataDictionary::~DataDictionary()
{
	if ( _pImpl )
	{
		delete _pImpl;
		_pImpl = 0;
	}
}

void DataDictionary::clear()
{
	_pImpl->clear();
}

Int32 DataDictionary::getMinFid() const
{
	return _pImpl->getMinFid();
}

Int32 DataDictionary::getMaxFid() const
{
	return _pImpl->getMaxFid();
}

const EmaVector<DictionaryEntry>& DataDictionary::getEntries() const
{
	return _pImpl->getEntries();
}

Int32 DataDictionary::getDictionaryId() const
{
	return _pImpl->getInfoDictionaryId();
}

const EmaString& DataDictionary::getFieldVersion() const
{
	return _pImpl->getFieldVersion();
}

const EmaString& DataDictionary::getEnumRecordTemplateVersion() const
{
	return _pImpl->getEnumRecordTemplateVersion();
}

const EmaString& DataDictionary::getEnumDisplayTemplateVersion() const
{
	return _pImpl->getEnumDisplayTemplateVersion();
}

const EmaString& DataDictionary::getFieldFilename() const
{
	return _pImpl->getFieldFilename();
}

const EmaString& DataDictionary::getFieldDescription() const
{
	return _pImpl->getFieldDescription();
}

const EmaString& DataDictionary::getFieldBuild() const
{
	return _pImpl->getFieldBuild();
}

const EmaString& DataDictionary::getFieldDate() const
{
	return _pImpl->getFieldDate();
}

const EmaString& DataDictionary::getEnumFilename() const
{
	return _pImpl->getEnumFilename();
}

const EmaString& DataDictionary::getEnumDescription() const
{
	return _pImpl->getEnumDescription();
}

const EmaString& DataDictionary::getEnumDate() const
{
	return _pImpl->getEnumDate();
}

bool DataDictionary::hasEntry(Int16 fieldId) const
{
	return _pImpl->hasEntry(fieldId);
}

const DictionaryEntry& DataDictionary::getEntry(Int16 fieldId) const
{
	return _pImpl->getEntry(fieldId);
}

bool DataDictionary::hasEntry(const thomsonreuters::ema::access::EmaString& fieldName) const
{
	return _pImpl->hasEntry(fieldName);
}

const DictionaryEntry& DataDictionary::getEntry(const thomsonreuters::ema::access::EmaString& fieldName) const
{
	return _pImpl->getEntry(fieldName);
}

bool DataDictionary::hasEnumType(Int16 fieldId, UInt16 value) const
{
	return _pImpl->hasEnumType(fieldId, value);
}

const EnumType& DataDictionary::getEnumType(Int16 fieldId, UInt16 value) const
{
	return _pImpl->getEnumType(fieldId, value);
}

const EmaVector<EnumTypeTable>& DataDictionary::getEnumTables() const
{
	return _pImpl->getEnumTables();
}

void DataDictionary::loadFieldDictionary(const EmaString& filename)
{
	_pImpl->loadFieldDictionary(filename);
}

void DataDictionary::loadEnumTypeDictionary(const EmaString& filename)
{
	_pImpl->loadEnumTypeDictionary(filename);
}

void DataDictionary::encodeFieldDictionary(Series& series, UInt32 verbosity)
{
	_pImpl->encodeFieldDictionary(series, verbosity);
}

bool DataDictionary::encodeFieldDictionary(Series& series, Int32& currentFid, UInt32 verbosity, UInt32 fragmentationSize)
{
	return _pImpl->encodeFieldDictionary(series, currentFid, verbosity, fragmentationSize);
}

void DataDictionary::decodeFieldDictionary(const Series& series, UInt32 verbosity)
{
	_pImpl->decodeFieldDictionary(series, verbosity);
}

void DataDictionary::encodeEnumTypeDictionary(Series& series, UInt32 verbosity)
{
	_pImpl->encodeEnumTypeDictionary(series, verbosity);
}

bool DataDictionary::encodeEnumTypeDictionary(Series& series, Int32& currentCount,
	UInt32 verbosity, UInt32 fragmentationSize)
{
	return _pImpl->encodeEnumTypeDictionary(series, currentCount, verbosity, fragmentationSize);
}

void DataDictionary::decodeEnumTypeDictionary(const Series& series, UInt32 verbosity)
{
	_pImpl->decodeEnumTypeDictionary(series, verbosity);
}

UInt32 DataDictionary::extractDictionaryType(const Series& series)
{
	return _pImpl->extractDictionaryType(series);
}

const EmaString& DataDictionary::toString() const
{
	return _pImpl->toString();
}

DataDictionary::operator const char* () const
{
	return _pImpl->toString().c_str();
}
