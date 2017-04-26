#include "EnumTypeTable.h"
#include "EnumTypeTableImpl.h"

using namespace thomsonreuters::ema::rdm;

EnumTypeTable::EnumTypeTable()
{
	_pImpl = new EnumTypeTableImpl();
}


EnumTypeTable::~EnumTypeTable()
{
}

EnumTypeTable& EnumTypeTable::operator=(const EnumTypeTable& other)
{
	_pImpl->rsslEnumTypeTable(other._pImpl->getRsslEnumTypeTable());

	return *this;
}

const thomsonreuters::ema::access::EmaVector<EnumType>& EnumTypeTable::getEnumTypes() const
{
	return _pImpl->getEnumTypes();
}

const thomsonreuters::ema::access::EmaVector<thomsonreuters::ema::access::Int16>& EnumTypeTable::getFidReferences() const
{
	return _pImpl->getFidReferences();
}

const thomsonreuters::ema::access::EmaString& EnumTypeTable::toString() const
{
	return _pImpl->toString();
}

EnumTypeTable::operator const char* () const
{
	return _pImpl->toString().c_str();
}