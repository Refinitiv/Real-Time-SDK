/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
*|-----------------------------------------------------------------------------
*/

#include "Ema.h"
#include "LoginStatusImpl.h"
#include <new>

using namespace refinitiv::ema::domain::login;
using namespace refinitiv::ema::access;
using namespace std;

Login::LoginStatus::LoginStatus()
{
	try
	{
		_pLoginStatusImpl = new LoginStatusImpl();
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for LoginStatusImpl in Login::LoginStatus::LoginStatus().");
	}
}

Login::LoginStatus::LoginStatus(const LoginStatus& other)
{
	try
	{
		_pLoginStatusImpl = new LoginStatusImpl(*other._pLoginStatusImpl);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for LoginStatusImpl in Login::LoginStatus::LoginStatus(const LoginStatus&).");
	}
}

Login::LoginStatus::LoginStatus(const StatusMsg& statusMsg)
{
	try
	{
	    _pLoginStatusImpl = new LoginStatusImpl(statusMsg);
	}
	catch (std::bad_alloc&)
	{
		throwMeeException("Failed to allocate memory for LoginStatusImpl in Login::LoginStatus::LoginStatus(const ElementList&).");
	}
}

Login::LoginStatus::~LoginStatus()
{
	if (_pLoginStatusImpl)
	{
		delete _pLoginStatusImpl;
		_pLoginStatusImpl = 0;
	}
}

Login::LoginStatus& Login::LoginStatus::clear()
{
	_pLoginStatusImpl->clear();
	return *this;
}

Login::LoginStatus& Login::LoginStatus::operator=(const LoginStatus& other)
{
	if (this == &other) return *this;

	*_pLoginStatusImpl = *other._pLoginStatusImpl;

	return *this;
}

Login::LoginStatus& Login::LoginStatus::message(const StatusMsg& statusMsg)
{
	_pLoginStatusImpl->message(statusMsg);
	return *this;
}

Login::LoginStatus& Login::LoginStatus::authenticationErrorCode(const UInt64& value)
{
	_pLoginStatusImpl->authenticationErrorCode(value);
	return *this;
}

Login::LoginStatus& Login::LoginStatus::authenticationErrorText(const EmaString& value)
{
	_pLoginStatusImpl->authenticationErrorText(value);
	return *this;
}

Login::LoginStatus& Login::LoginStatus::name(const EmaString& value)
{
	_pLoginStatusImpl->name(value);
	return *this;
}

Login::LoginStatus& Login::LoginStatus::nameType(const UInt32& value)
{
	_pLoginStatusImpl->nameType(value);
	return *this;
}

Login::LoginStatus& Login::LoginStatus::state(const OmmState::StreamState& streamState, const OmmState::DataState dataState, const UInt8& statusCode, const EmaString& statusText)
{
	_pLoginStatusImpl->state(streamState, dataState, statusCode, statusText);
	return *this;
}

bool Login::LoginStatus::hasAuthenticationErrorCode() const
{
	return _pLoginStatusImpl->hasAuthenticationErrorCode();
}

bool Login::LoginStatus::hasAuthenticationErrorText() const
{
	return _pLoginStatusImpl->hasAuthenticationErrorText();
}

bool Login::LoginStatus::hasName() const
{
	return _pLoginStatusImpl->hasName();
}

bool Login::LoginStatus::hasNameType() const
{
	return _pLoginStatusImpl->hasNameType();
}

bool Login::LoginStatus::hasState() const
{
	return _pLoginStatusImpl->hasState();
}

const StatusMsg& Login::LoginStatus::getMessage() const
{
	return _pLoginStatusImpl->getMessage();
}

const UInt64& Login::LoginStatus::getAuthenticationErrorCode() const
{
	return _pLoginStatusImpl->getAuthenticationErrorCode();
}

const EmaString& Login::LoginStatus::getAuthenticationErrorText() const
{
	return _pLoginStatusImpl->getAuthenticationErrorText();
}

const EmaString& Login::LoginStatus::getName() const
{
	return _pLoginStatusImpl->getName();
}

const UInt32& Login::LoginStatus::getNameType() const
{
	return _pLoginStatusImpl->getNameType();
}

const OmmState& Login::LoginStatus::getState() const
{
	return _pLoginStatusImpl->getState();
}

const EmaString& Login::LoginStatus::toString() const
{
	return _pLoginStatusImpl->toString();
}