/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017,2019-2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "EnumTypeTableImpl.h"
#include "EnumTypeImpl.h"

using namespace refinitiv::ema::access;
using namespace refinitiv::ema::rdm;

EnumTypeTableImpl::EnumTypeTableImpl() :
_pEnumTypeList(0),
_pFidsList(0)
{
}

EnumTypeTableImpl::EnumTypeTableImpl(RsslEnumTypeTable* rsslEnumTypeTable) :
_pEnumTypeTable(rsslEnumTypeTable),
_pEnumTypeList(0),
_pFidsList(0),
_refreshEnumTypeList(false)
{
}

EnumTypeTableImpl::~EnumTypeTableImpl()
{
	if ( _pEnumTypeList )
	{
		delete _pEnumTypeList;	
		_pEnumTypeList = 0;
	}

	if ( _pFidsList )
	{
   		delete _pFidsList;
		_pFidsList = 0;
	}
}

RsslEnumTypeTable* EnumTypeTableImpl::getRsslEnumTypeTable() const
{
	return _pEnumTypeTable;
}

void EnumTypeTableImpl::rsslEnumTypeTable(RsslEnumTypeTable* enumTypeTable)
{
	_refreshEnumTypeList = true;

	_pEnumTypeTable = enumTypeTable;
}

const refinitiv::ema::access::EmaVector<EnumType>& EnumTypeTableImpl::getEnumTypes() const
{
	if (_pEnumTypeList == 0)
	{
		_pEnumTypeList = new EmaVector<EnumType>(_pEnumTypeTable->maxValue);
	}
	else if (_refreshEnumTypeList )
	{
		_pEnumTypeList->clear();
	}

	RsslEnumType*	rsslEnumType = 0;
	EnumType	enumType;

	for (UInt32 index = 0; index <= _pEnumTypeTable->maxValue; index++)
	{
		rsslEnumType = *(_pEnumTypeTable->enumTypes + index);

		if (rsslEnumType)
		{
			enumType._pImpl->rsslEnumType(rsslEnumType);
			_pEnumTypeList->push_back(enumType);
		}
	}

	_refreshEnumTypeList = false;

	return *_pEnumTypeList;
}

const refinitiv::ema::access::EmaVector<refinitiv::ema::access::Int16>& EnumTypeTableImpl::getFidReferences() const
{
	if (_pFidsList == 0)
	{
		_pFidsList = new EmaVector<Int16>(_pEnumTypeTable->fidReferenceCount);
	}
	else
	{
		_pFidsList->clear();
	}

	for (UInt32 index = 0; index < _pEnumTypeTable->fidReferenceCount; index++)
	{
		_pFidsList->push_back(_pEnumTypeTable->fidReferences[index]);
	}

	return *_pFidsList;
}

const refinitiv::ema::access::EmaString& EnumTypeTableImpl::toString() const
{
	_stringToString.set(0, 256);

	RsslEnumType* rsslEnumType = 0;
	EnumType enumType;

	for (UInt32 index = 0; index < _pEnumTypeTable->fidReferenceCount; index++)
	{
		_stringToString.append("(Referenced by Fid ").append(_pEnumTypeTable->fidReferences[index]).append(")\n");
	}

	for (UInt32 index = 0; index <= _pEnumTypeTable->maxValue; index++)
	{
		rsslEnumType = *(_pEnumTypeTable->enumTypes + index);

		if (rsslEnumType)
		{
			enumType._pImpl->rsslEnumType(rsslEnumType);

			_stringToString.append(enumType.toString()).append("\n");
		}
	}
	
	return _stringToString;
}
