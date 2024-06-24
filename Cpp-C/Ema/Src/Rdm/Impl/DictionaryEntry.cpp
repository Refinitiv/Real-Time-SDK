/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|          Copyright (C) 2019-2020 LSEG. All rights reserved.               --
 *|-----------------------------------------------------------------------------
 */

#include "DictionaryEntry.h"
#include "DictionaryEntryImpl.h"
#include "ExceptionTranslator.h"
#include "EnumType.h"

#include <new>

using namespace refinitiv::ema::rdm;

DictionaryEntry::DictionaryEntry()
{
	try
	{
		_pImpl = new DictionaryEntryImpl(true);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory in DictionaryEntry::DictionaryEntry()");
	}
}

DictionaryEntry::DictionaryEntry(bool isManagedByUser)
{
	try
	{
		_pImpl = new DictionaryEntryImpl(isManagedByUser);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory in DictionaryEntry::DictionaryEntry()");
	}
}

DictionaryEntry::DictionaryEntry(const DictionaryEntry& other) 
{
	_pImpl = other._pImpl;
}

DictionaryEntry::~DictionaryEntry()
{
	if ( _pImpl )
	{
		delete _pImpl;
		_pImpl = 0;
	}
}

DictionaryEntry& DictionaryEntry::operator=(const DictionaryEntry& other)
{
	_pImpl->rsslDictionaryEntry(other._pImpl->getRsslDictionaryEntry());
	return *this;
}

const refinitiv::ema::access::EmaString& DictionaryEntry::getAcronym() const
{
	return _pImpl->getAcronym();
}

const refinitiv::ema::access::EmaString& DictionaryEntry::getDDEAcronym() const
{
	return _pImpl->getDDEAcronym();
}

refinitiv::ema::access::Int16 DictionaryEntry::getFid() const
{
	return _pImpl->getFid();
}

refinitiv::ema::access::Int16 DictionaryEntry::getRippleToField() const
{
	return _pImpl->getRippleToField();
}


refinitiv::ema::access::Int8 DictionaryEntry::getFieldType() const
{
	return _pImpl->getFieldType();
}

refinitiv::ema::access::UInt16 DictionaryEntry::getLength() const
{
	return _pImpl->getLength();
}

refinitiv::ema::access::UInt8 DictionaryEntry::getEnumLength() const
{
	return _pImpl->getEnumLength();
}


refinitiv::ema::access::UInt8 DictionaryEntry::getRwfType() const
{
	return _pImpl->getRwfType();
}

refinitiv::ema::access::UInt32 DictionaryEntry::getRwfLength() const
{
	return _pImpl->getRwfLength();
}

bool DictionaryEntry::hasEnumType(refinitiv::ema::access::UInt16 value) const
{
	return _pImpl->hasEnumType(value);
}

const EnumType& DictionaryEntry::getEnumType(refinitiv::ema::access::UInt16 value) const
{
	return _pImpl->getEnumEntry(value);
}

bool DictionaryEntry::hasEnumTypeTable() const
{
	return _pImpl->hasEnumTypeTable();
}

const EnumTypeTable& DictionaryEntry::getEnumTypeTable() const
{
	return _pImpl->getEnumTypeTable();
}

const refinitiv::ema::access::EmaString& DictionaryEntry::toString() const
{
	return _pImpl->toString();
}

DictionaryEntry::operator const char* () const
{
	return _pImpl->toString().c_str();
}