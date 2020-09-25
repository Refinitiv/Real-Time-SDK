/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#include "DictionaryEntryImpl.h"
#include "EnumTypeImpl.h"
#include "EnumTypeTableImpl.h"
#include "ExceptionTranslator.h"
#include "OmmInvalidUsageException.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

DictionaryEntryImpl::DictionaryEntryImpl(bool isManagedByUser) : _isManagedByUser(isManagedByUser)
{
}

DictionaryEntryImpl::DictionaryEntryImpl(RsslDictionaryEntry* rsslDictionaryEntry)
{
	_pRsslDictionaryEntry = rsslDictionaryEntry;
}

DictionaryEntryImpl::~DictionaryEntryImpl()
{
}

void DictionaryEntryImpl::rsslDictionaryEntry(RsslDictionaryEntry* rsslDictionaryEntry)
{
	_pRsslDictionaryEntry = rsslDictionaryEntry;
}

const refinitiv::ema::access::EmaString& DictionaryEntryImpl::getAcronym() const
{
	_stringAcronym.setInt(_pRsslDictionaryEntry->acronym.data, _pRsslDictionaryEntry->acronym.length,
		_pRsslDictionaryEntry->acronym.length > 0 ? true : false);

	return _stringAcronym.toString();
}

const refinitiv::ema::access::EmaString& DictionaryEntryImpl::getDDEAcronym() const
{
	_stringDDEAcronym.setInt(_pRsslDictionaryEntry->ddeAcronym.data, _pRsslDictionaryEntry->ddeAcronym.length,
		_pRsslDictionaryEntry->ddeAcronym.length > 0 ? true : false);

	return _stringDDEAcronym.toString();
}

refinitiv::ema::access::Int16 DictionaryEntryImpl::getFid() const
{
	return _pRsslDictionaryEntry->fid;
}

refinitiv::ema::access::Int16 DictionaryEntryImpl::getRippleToField() const
{
	return _pRsslDictionaryEntry->rippleToField;
}


refinitiv::ema::access::Int8 DictionaryEntryImpl::getFieldType() const
{
	return _pRsslDictionaryEntry->fieldType;
}

refinitiv::ema::access::UInt16 DictionaryEntryImpl::getLength() const
{
	return _pRsslDictionaryEntry->length;
}


refinitiv::ema::access::UInt8 DictionaryEntryImpl::getEnumLength() const
{
	return _pRsslDictionaryEntry->enumLength;
}


refinitiv::ema::access::UInt8 DictionaryEntryImpl::getRwfType() const
{
	return _pRsslDictionaryEntry->rwfType;
}

refinitiv::ema::access::UInt16 DictionaryEntryImpl::getRwfLength() const
{
	return _pRsslDictionaryEntry->rwfLength;
}

bool DictionaryEntryImpl::hasEnumType(refinitiv::ema::access::UInt16 value) const
{
	return (_pRsslDictionaryEntry->pEnumTypeTable && value <= _pRsslDictionaryEntry->pEnumTypeTable->maxValue) ? true : false;
}

const EnumType& DictionaryEntryImpl::getEnumEntry(refinitiv::ema::access::UInt16 value) const
{
	RsslEnumType* rsslEnumType = ( _pRsslDictionaryEntry->pEnumTypeTable && value <= _pRsslDictionaryEntry->pEnumTypeTable->maxValue) ?
		_pRsslDictionaryEntry->pEnumTypeTable->enumTypes[value] : 0;

	if (rsslEnumType)
	{
		_enumType._pImpl->rsslEnumType(rsslEnumType);
	}
	else
	{
		EmaString errorText("The enum value ");
		errorText.append(value).append(" for the Field ID ");
		errorText.append(_pRsslDictionaryEntry->fid).append(" does not exist in enumerated type definitions");
		throwIueException( errorText, OmmInvalidUsageException::InvalidArgumentEnum );
	}

	return _enumType;
}

bool DictionaryEntryImpl::hasEnumTypeTable() const
{
	return (_pRsslDictionaryEntry->pEnumTypeTable) ? true : false;
}

const EnumTypeTable& DictionaryEntryImpl::getEnumTypeTable() const
{
	if ( _pRsslDictionaryEntry->pEnumTypeTable )
	{
		_enumTypeTable._pImpl->rsslEnumTypeTable(_pRsslDictionaryEntry->pEnumTypeTable);
	}
	else
	{
		EmaString errorText("The EnumTypeTable does not exist for the Field ID ");
		errorText.append(_pRsslDictionaryEntry->fid);
		throwIueException( errorText, OmmInvalidUsageException::InvalidOperationEnum );
	}

	return _enumTypeTable;
}

RsslDictionaryEntry* DictionaryEntryImpl::getRsslDictionaryEntry()
{
	return _pRsslDictionaryEntry;
}

const refinitiv::ema::access::EmaString& DictionaryEntryImpl::toString() const
{
	_stringToString.set(0, 512);

	_stringToString.append("Fid=").append(getFid()).append(" '").append(getAcronym()).
		append("' '").append(getDDEAcronym()).
		append("' Type=").append(getFieldType()).
		append(" RippleTo=").append(getRippleToField()).append(" Len=").append(getLength()).
		append(" EnumLen=").append(getEnumLength()).
		append(" RwfType=").append(getRwfType()).append(" RwfLen=").append(getRwfLength());

	EnumTypeTable	enumTypeTable;

	if ( hasEnumTypeTable() )
	{
		_stringToString.append("\n\nEnum Type Table:\n");

		enumTypeTable._pImpl->rsslEnumTypeTable(_pRsslDictionaryEntry->pEnumTypeTable);

		_stringToString.append(enumTypeTable.toString());
	}

	return _stringToString;
}

bool DictionaryEntryImpl::isManagedByUser() const
{
	return _isManagedByUser;
}
