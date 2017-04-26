#include "EnumTypeTableImpl.h"
#include "EnumTypeImpl.h"

using namespace thomsonreuters::ema::access;
using namespace thomsonreuters::ema::rdm;

EnumTypeTableImpl::EnumTypeTableImpl() :
_pEnumTypeList(0),
_pFidsList(0)
{
}

EnumTypeTableImpl::EnumTypeTableImpl(RsslEnumTypeTable* rsslEnumTypeTable) :
_pEnumTypeTable(rsslEnumTypeTable),
_pEnumTypeList(0),
_pFidsList(0)
{
}

EnumTypeTableImpl::~EnumTypeTableImpl()
{
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

const thomsonreuters::ema::access::EmaVector<EnumType>& EnumTypeTableImpl::getEnumTypes() const
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

const thomsonreuters::ema::access::EmaVector<thomsonreuters::ema::access::Int16>& EnumTypeTableImpl::getFidReferences() const
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

const thomsonreuters::ema::access::EmaString& EnumTypeTableImpl::toString() const
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