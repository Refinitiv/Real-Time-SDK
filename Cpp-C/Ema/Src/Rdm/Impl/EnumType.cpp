#include "EnumType.h"
#include "EnumTypeImpl.h"
#include "ExceptionTranslator.h"

#include <new>

using namespace thomsonreuters::ema::rdm;

EnumType::EnumType()
{
	try
	{
		_pImpl = new EnumTypeImpl();
	}
	catch (std::bad_alloc)
	{
		throwMeeException("Failed to allocate memory in EnumType::EnumType()");
	}
}

EnumType& EnumType::operator=(const EnumType& other)
{
	this->_pImpl->rsslEnumType(other._pImpl->rsslEnumType());
	return *this;
}

EnumType::~EnumType()
{
	if ( _pImpl )
	{
		delete _pImpl;
		_pImpl = 0;
	}
}

thomsonreuters::ema::access::UInt16 EnumType::getValue() const
{
	return _pImpl->getValue();
}

const thomsonreuters::ema::access::EmaString& EnumType::getDisplay() const
{
	return _pImpl->getDisplay();
}

const thomsonreuters::ema::access::EmaString& EnumType::getMeaning() const
{
	return _pImpl->getMeaning();
}

const thomsonreuters::ema::access::EmaString& EnumType::toString() const
{
	return _pImpl->toString();
}

EnumType::operator const char* () const
{
	return _pImpl->toString().c_str();
}
