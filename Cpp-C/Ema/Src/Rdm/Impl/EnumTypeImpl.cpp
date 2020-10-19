#include "EnumTypeImpl.h"

using namespace refinitiv::ema::rdm;

EnumTypeImpl::EnumTypeImpl()
{

}

EnumTypeImpl::~EnumTypeImpl()
{

}

EnumTypeImpl& EnumTypeImpl::operator=(const EnumTypeImpl& other)
{
	this->_pRsslEnumType = other._pRsslEnumType;
	return *this;
}

RsslEnumType*	EnumTypeImpl::rsslEnumType()
{
	return _pRsslEnumType;
}

void EnumTypeImpl::rsslEnumType(RsslEnumType* rsslEnumType)
{
	_pRsslEnumType = rsslEnumType;
}

refinitiv::ema::access::UInt16 EnumTypeImpl::getValue() const
{
	return _pRsslEnumType->value;
}

const refinitiv::ema::access::EmaString& EnumTypeImpl::getDisplay() const
{
	_stringDispaly.setInt(_pRsslEnumType->display.data, _pRsslEnumType->display.length,
		_pRsslEnumType->display.length > 0 ? true : false);

	return _stringDispaly.toString();
}

const refinitiv::ema::access::EmaString& EnumTypeImpl::getMeaning() const
{
	_stringMeaning.setInt(_pRsslEnumType->meaning.data, _pRsslEnumType->meaning.length,
	_pRsslEnumType->meaning.length > 0 ? true : false);

	return _stringMeaning.toString();
}

const refinitiv::ema::access::EmaString& EnumTypeImpl::toString() const
{
	_stringToString.set(0, 64);

	_stringToString.append("value=").append(getValue()).
		append(" display=\"").append(getDisplay()).
		append("\" meaning=\"").append(getMeaning()).append("\"");

	return _stringToString;
}