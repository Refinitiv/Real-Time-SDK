/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#include "Ema.h"
#include "LoginReqImpl.h"

#include <new>

using namespace thomsonreuters::ema::domain::login;
using namespace thomsonreuters::ema::access;

Login::LoginReq::LoginReq()
{
	try
	{
	     _pLoginReqImpl = new LoginReqImpl();
	}
	catch (std::bad_alloc&) {}
	
    if (!_pLoginReqImpl)
         throwMeeException("Failed to allocate memory for LoginReqImpl in Login::LoginReq::LoginReq().");
}

Login::LoginReq::LoginReq(const LoginReq& LoginReq)
{
	try
	{
	    _pLoginReqImpl = new LoginReqImpl(*LoginReq._pLoginReqImpl);
    }
    catch (std::bad_alloc&) {}

    if (!_pLoginReqImpl)
         throwMeeException("Failed to allocate memory for LoginReqImpl in Login::LoginReq::LoginReq(const LoginReq&).");
}

Login::LoginReq::LoginReq(const ReqMsg& reqMsg)
{
	try
	{
		_pLoginReqImpl = new LoginReqImpl(reqMsg);
	}
	catch (std::bad_alloc&) {}

	if (!_pLoginReqImpl)
		throwMeeException("Failed to allocate memory for LoginReqImpl in Login::LoginReq::LoginReq(const ElementList&).");
}

Login::LoginReq::~LoginReq()
{
	if (_pLoginReqImpl)
	{
		delete _pLoginReqImpl;
		_pLoginReqImpl = 0;
	}
}

Login::LoginReq& Login::LoginReq::clear()
{
	_pLoginReqImpl->clear();
	return *this;
}

Login::LoginReq& Login::LoginReq::operator=(const LoginReq& other)
{
	if (this == &other) return *this;

	*_pLoginReqImpl = *other._pLoginReqImpl;

	return *this;
}

Login::LoginReq& Login::LoginReq::message(const ReqMsg& reqMsg)
{
	_pLoginReqImpl->message(reqMsg);
	return *this;
}

Login::LoginReq& Login::LoginReq::allowSuspectData(bool value)
{
	_pLoginReqImpl->allowSuspectData(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::downloadConnectionConfig(bool value)
{
	_pLoginReqImpl->downloadConnectionConfig(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::applicationId(const EmaString& value)
{
	_pLoginReqImpl->applicationId(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::applicationName(const EmaString& value)
{
	_pLoginReqImpl->applicationName(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::applicationAuthorizationToken(const EmaString& value)
{
	_pLoginReqImpl->applicationAuthorizationToken(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::instanceId(const EmaString& value)
{
	_pLoginReqImpl->instanceId(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::password(const EmaString& value)
{
	_pLoginReqImpl->password(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::position(const EmaString& value)
{
	_pLoginReqImpl->position(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::providePermissionExpressions(bool value)
{
	_pLoginReqImpl->providePermissionExpressions(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::providePermissionProfile(bool value)
{
	_pLoginReqImpl->providePermissionProfile(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::role(UInt32 value)
{
	_pLoginReqImpl->role(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::singleOpen(bool value)
{
	_pLoginReqImpl->singleOpen(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::supportProviderDictionaryDownload(bool value)
{
	_pLoginReqImpl->supportProviderDictionaryDownload(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::pause(bool value)
{
	_pLoginReqImpl->pause(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::authenticationExtended(const EmaBuffer& value)
{
	_pLoginReqImpl->authenticationExtended(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::name(const EmaString& value)
{
	_pLoginReqImpl->name(value);
	return *this;
}

Login::LoginReq& Login::LoginReq::nameType(const UInt32& value)
{
	_pLoginReqImpl->nameType(value);
	return *this;
}

bool Login::LoginReq::hasAllowSuspectData() const
{
	return _pLoginReqImpl->hasAllowSuspectData();
}

bool Login::LoginReq::hasDownloadConnectionConfig() const
{
	return _pLoginReqImpl->hasDownloadConnectionConfig();
}

bool Login::LoginReq::hasApplicationId() const
{
	return _pLoginReqImpl->hasApplicationId();
}

bool Login::LoginReq::hasApplicationName() const
{
	return _pLoginReqImpl->hasApplicationName();
}

bool Login::LoginReq::hasApplicationAuthorizationToken() const
{
	return _pLoginReqImpl->hasApplicationAuthorizationToken();
}

bool Login::LoginReq::hasInstanceId() const
{
	return _pLoginReqImpl->hasInstanceId();
}

bool Login::LoginReq::hasPassword() const
{
	return _pLoginReqImpl->hasPassword();
}

bool Login::LoginReq::hasPosition() const
{
	return _pLoginReqImpl->hasPosition();
}

bool Login::LoginReq::hasProvidePermissionExpressions() const
{
	return _pLoginReqImpl->hasProvidePermissionExpressions();
}

bool Login::LoginReq::hasProvidePermissionProfile() const
{
	return _pLoginReqImpl->hasProvidePermissionProfile();
}

bool Login::LoginReq::hasRole() const
{
	return _pLoginReqImpl->hasRole();
}

bool Login::LoginReq::hasSingleOpen() const
{
	return _pLoginReqImpl->hasSingleOpen();
}

bool Login::LoginReq::hasSupportProviderDictionaryDownload() const
{
	return _pLoginReqImpl->hasSupportProviderDictionaryDownload();
}

bool Login::LoginReq::hasPause() const
{
	return _pLoginReqImpl->hasPause();
}

bool Login::LoginReq::hasAuthenticationExtended() const
{
	return _pLoginReqImpl->hasAuthenticationExtended();
}

bool Login::LoginReq::hasName() const
{
	return _pLoginReqImpl->hasName();
}

bool Login::LoginReq::hasNameType() const
{
	return _pLoginReqImpl->hasNameType();
}

const ReqMsg& Login::LoginReq::getMessage() const
{
	return _pLoginReqImpl->getMessage();
}

bool Login::LoginReq::getAllowSuspectData() const
{
	return _pLoginReqImpl->getAllowSuspectData();
}

bool Login::LoginReq::getDownloadConnectionConfig() const
{
	return _pLoginReqImpl->getDownloadConnectionConfig();
}

const EmaString& Login::LoginReq::getApplicationId() const
{
	return _pLoginReqImpl->getApplicationId();
}

const EmaString& Login::LoginReq::getApplicationName() const
{
	return _pLoginReqImpl->getApplicationName();
}

const EmaString& Login::LoginReq::getApplicationAuthorizationToken() const
{
	return _pLoginReqImpl->getApplicationAuthorizationToken();
}

const EmaString& Login::LoginReq::getInstanceId() const
{
	return _pLoginReqImpl->getInstanceId();
}

const EmaString& Login::LoginReq::getPassword() const
{
	return _pLoginReqImpl->getPassword();
}

const EmaString& Login::LoginReq::getPosition() const
{
	return _pLoginReqImpl->getPosition();
}

bool Login::LoginReq::getProvidePermissionExpressions() const
{
	return _pLoginReqImpl->getProvidePermissionExpressions();
}

bool Login::LoginReq::getProvidePermissionProfile() const
{
	return _pLoginReqImpl->getProvidePermissionProfile();
}

UInt32 Login::LoginReq::getRole() const
{
	return _pLoginReqImpl->getRole();
}

bool Login::LoginReq::getSingleOpen() const
{
	return _pLoginReqImpl->getSingleOpen();
}

bool Login::LoginReq::getSupportProviderDictionaryDownload() const
{
	return _pLoginReqImpl->getSupportProviderDictionaryDownload();
}

bool Login::LoginReq::getPause() const
{
	return _pLoginReqImpl->getPause();
}

const EmaBuffer& Login::LoginReq::getAuthenticationExtended() const
{
	return _pLoginReqImpl->getAuthenticationExtended();
}

const EmaString& Login::LoginReq::getName() const
{
	return _pLoginReqImpl->getName();
}

const UInt32& Login::LoginReq::getNameType() const
{
	return _pLoginReqImpl->getNameType();
}

const EmaString& Login::LoginReq::toString() const
{
	return _pLoginReqImpl->toString();
}


