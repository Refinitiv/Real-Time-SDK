/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright Thomson Reuters 2016. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#include "Ema.h"
#include "LoginRefreshImpl.h"

#include <new>

using namespace thomsonreuters::ema::domain::login;
using namespace thomsonreuters::ema::access;

Login::LoginRefresh::LoginRefresh()
{
	try
	{
		_pLoginRefreshImpl = new LoginRefreshImpl();
	}
	catch (std::bad_alloc) {}

	if (!_pLoginRefreshImpl)
		throwMeeException("Failed to allocate memory for LoginRefreshImpl in Login::LoginRefresh::LoginRefresh().");
}

Login::LoginRefresh::LoginRefresh(const LoginRefresh& other)
{
	try
	{
		_pLoginRefreshImpl = new LoginRefreshImpl(*other._pLoginRefreshImpl);
	}
	catch (std::bad_alloc) {}

	if (!_pLoginRefreshImpl)
		throwMeeException("Failed to allocate memory for LoginRefreshImpl in Login::LoginRefresh::LoginRefresh(const LoginRefresh&).");
}

Login::LoginRefresh::LoginRefresh(const RefreshMsg& refreshMsg)
{
	try
	{
	    _pLoginRefreshImpl = new LoginRefreshImpl(refreshMsg);
	}
	catch (std::bad_alloc) {}

	if (!_pLoginRefreshImpl)
		throwMeeException("Failed to allocate memory for LoginRefreshImpl in Login::LoginRefresh::LoginRefresh(const ElementList&).");
}

Login::LoginRefresh::~LoginRefresh()
{
	if (_pLoginRefreshImpl)
	{
		delete _pLoginRefreshImpl;
		_pLoginRefreshImpl = 0;
	}
}

Login::LoginRefresh& Login::LoginRefresh::clear()
{
	_pLoginRefreshImpl->clear();
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::operator=(const LoginRefresh& other)
{
	if (this == &other) return *this;

	*_pLoginRefreshImpl = *other._pLoginRefreshImpl;

	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::message(const RefreshMsg& refreshMsg)
{
	_pLoginRefreshImpl->message(refreshMsg);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::allowSuspectData(bool value)
{
	_pLoginRefreshImpl->allowSuspectData(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::applicationId(const EmaString& value)
{
	_pLoginRefreshImpl->applicationId(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::applicationName(const EmaString& value)
{
	_pLoginRefreshImpl->applicationName(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::position(const EmaString& value)
{
	_pLoginRefreshImpl->position(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::providePermissionExpressions(bool value)
{
	_pLoginRefreshImpl->providePermissionExpressions(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::providePermissionProfile(bool value)
{
	_pLoginRefreshImpl->providePermissionProfile(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::singleOpen(bool value)
{
	_pLoginRefreshImpl->singleOpen(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::supportBatchRequests(UInt32 value)
{
	_pLoginRefreshImpl->supportBatchRequests(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::supportEnhancedSymbolList(UInt32 value)
{
	_pLoginRefreshImpl->supportEnhancedSymbolList(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::supportOMMPost(bool value)
{
	_pLoginRefreshImpl->supportOMMPost(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::supportOptimizedPauseResume(bool value)
{
	_pLoginRefreshImpl->supportOptimizedPauseResume(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::supportProviderDictionaryDownload(bool value)
{
	_pLoginRefreshImpl->supportProviderDictionaryDownload(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::supportViewRequests(bool value)
{
	_pLoginRefreshImpl->supportViewRequests(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::supportStandby(bool value)
{
	_pLoginRefreshImpl->supportStandby(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::solicited(bool value)
{
	_pLoginRefreshImpl->solicited(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::clearCache(bool value)
{
	_pLoginRefreshImpl->clearCache(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::authenticationExtended(const EmaBuffer& value)
{
	_pLoginRefreshImpl->authenticationExtended(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::authenticationTTReissue(const UInt64& value)
{
	_pLoginRefreshImpl->authenticationTTReissue(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::authenticationErrorCode(const UInt64& value)
{
	_pLoginRefreshImpl->authenticationErrorCode(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::authenticationErrorText(const EmaString& value)
{
	_pLoginRefreshImpl->authenticationErrorText(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::name(const EmaString& value)
{
	_pLoginRefreshImpl->name(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::nameType(const UInt32& value)
{
	_pLoginRefreshImpl->nameType(value);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::state(const OmmState::StreamState& streamState, const OmmState::DataState dataState, const UInt8& statusCode, const EmaString& statusText)
{
	_pLoginRefreshImpl->state(streamState, dataState, statusCode, statusText);
	return *this;
}

Login::LoginRefresh& Login::LoginRefresh::seqNum(const UInt32& value)
{
	_pLoginRefreshImpl->seqNum(value);
	return *this;
}

bool Login::LoginRefresh::hasAllowSuspectData() const
{
	return _pLoginRefreshImpl->hasAllowSuspectData();
}

bool Login::LoginRefresh::hasApplicationId() const
{
	return _pLoginRefreshImpl->hasApplicationId();
}

bool Login::LoginRefresh::hasApplicationName() const
{
	return _pLoginRefreshImpl->hasApplicationName();
}

bool Login::LoginRefresh::hasPosition() const
{
	return _pLoginRefreshImpl->hasPosition();
}

bool Login::LoginRefresh::hasProvidePermissionExpressions() const
{
	return _pLoginRefreshImpl->hasProvidePermissionExpressions();
}

bool Login::LoginRefresh::hasProvidePermissionProfile() const
{
	return _pLoginRefreshImpl->hasProvidePermissionProfile();
}

bool Login::LoginRefresh::hasSingleOpen() const
{
	return _pLoginRefreshImpl->hasSingleOpen();
}

bool Login::LoginRefresh::hasSupportBatchRequests() const
{
	return _pLoginRefreshImpl->hasSupportBatchRequests();
}

bool Login::LoginRefresh::hasSupportEnhancedSymbolList() const
{
	return _pLoginRefreshImpl->hasSupportEnhancedSymbolList();
}

bool Login::LoginRefresh::hasSupportOMMPost() const
{
	return _pLoginRefreshImpl->hasSupportOMMPost();
}

bool Login::LoginRefresh::hasSupportOptimizedPauseResume() const
{
	return _pLoginRefreshImpl->hasSupportOptimizedPauseResume();
}

bool Login::LoginRefresh::hasSupportProviderDictionaryDownload() const
{
	return _pLoginRefreshImpl->hasSupportProviderDictionaryDownload();
}

bool Login::LoginRefresh::hasSupportViewRequests() const
{
	return _pLoginRefreshImpl->hasSupportViewRequests();
}

bool Login::LoginRefresh::hasSupportStandby() const
{
	return _pLoginRefreshImpl->hasSupportStandby();
}

bool Login::LoginRefresh::hasSolicited() const
{
	return _pLoginRefreshImpl->hasSolicited();
}

bool Login::LoginRefresh::hasClearCache() const
{
	return _pLoginRefreshImpl->hasClearCache();
}

bool Login::LoginRefresh::hasAuthenticationExtended() const
{
	return _pLoginRefreshImpl->hasAuthenticationExtended();
}

bool Login::LoginRefresh::hasAuthenticationTTReissue() const
{
	return _pLoginRefreshImpl->hasAuthenticationTTReissue();
}

bool Login::LoginRefresh::hasAuthenticationErrorCode() const
{
	return _pLoginRefreshImpl->hasAuthenticationErrorCode();
}

bool Login::LoginRefresh::hasAuthenticationErrorText() const
{
	return _pLoginRefreshImpl->hasAuthenticationErrorText();
}

bool Login::LoginRefresh::hasName() const
{
	return _pLoginRefreshImpl->hasName();
}

bool Login::LoginRefresh::hasNameType() const
{
	return _pLoginRefreshImpl->hasNameType();
}

bool Login::LoginRefresh::hasState() const
{
	return _pLoginRefreshImpl->hasState();
}

bool Login::LoginRefresh::hasSeqNum() const
{
	return _pLoginRefreshImpl->hasSeqNum();
}

const RefreshMsg& Login::LoginRefresh::getMessage() const
{
	return _pLoginRefreshImpl->getMessage();
}

bool Login::LoginRefresh::getAllowSuspectData() const
{
	return _pLoginRefreshImpl->getAllowSuspectData();
}

const EmaString& Login::LoginRefresh::getApplicationId() const
{
	return _pLoginRefreshImpl->getApplicationId();
}

const EmaString& Login::LoginRefresh::getApplicationName() const
{
	return _pLoginRefreshImpl->getApplicationName();
}

const EmaString& Login::LoginRefresh::getPosition() const
{
	return _pLoginRefreshImpl->getPosition();
}

bool Login::LoginRefresh::getProvidePermissionExpressions() const
{
	return _pLoginRefreshImpl->getProvidePermissionExpressions();
}

bool Login::LoginRefresh::getProvidePermissionProfile() const
{
	return _pLoginRefreshImpl->getProvidePermissionProfile();
}

bool Login::LoginRefresh::getSingleOpen() const
{
	return _pLoginRefreshImpl->getSingleOpen();
}

UInt32 Login::LoginRefresh::getSupportBatchRequests() const
{
	return _pLoginRefreshImpl->getSupportBatchRequests();
}

UInt32 Login::LoginRefresh::getSupportEnhancedSymbolList() const
{
	return _pLoginRefreshImpl->getSupportEnhancedSymbolList();
}

bool Login::LoginRefresh::getSupportOMMPost() const
{
	return _pLoginRefreshImpl->getSupportOMMPost();
}

bool Login::LoginRefresh::getSupportOptimizedPauseResume() const
{
	return _pLoginRefreshImpl->getSupportOptimizedPauseResume();
}

bool Login::LoginRefresh::getSupportProviderDictionaryDownload() const
{
	return _pLoginRefreshImpl->getSupportProviderDictionaryDownload();
}

bool Login::LoginRefresh::getSupportViewRequests() const
{
	return _pLoginRefreshImpl->getSupportViewRequests();
}

bool Login::LoginRefresh::getSupportStandby() const
{
	return _pLoginRefreshImpl->getSupportStandby();
}

bool Login::LoginRefresh::getSolicited() const
{
	return _pLoginRefreshImpl->getSolicited();
}

bool Login::LoginRefresh::getClearCache() const
{
	return _pLoginRefreshImpl->getClearCache();
}

const EmaBuffer& Login::LoginRefresh::getAuthenticationExtended() const
{
	return _pLoginRefreshImpl->getAuthenticationExtended();
}

const UInt64& Login::LoginRefresh::getAuthenticationTTReissue() const
{
	return _pLoginRefreshImpl->getAuthenticationTTReissue();
}

const UInt64& Login::LoginRefresh::getAuthenticationErrorCode() const
{
	return _pLoginRefreshImpl->getAuthenticationErrorCode();
}

const EmaString& Login::LoginRefresh::getAuthenticationErrorText() const
{
	return _pLoginRefreshImpl->getAuthenticationErrorText();
}

const EmaString& Login::LoginRefresh::getName() const
{
	return _pLoginRefreshImpl->getName();
}

const UInt32& Login::LoginRefresh::getNameType() const
{
	return _pLoginRefreshImpl->getNameType();
}

const OmmState& Login::LoginRefresh::getState() const
{
	return _pLoginRefreshImpl->getState();
}

const UInt32& Login::LoginRefresh::getSeqNum() const
{
	return _pLoginRefreshImpl->getSeqNum();
}

const EmaString& Login::LoginRefresh::toString() const
{
	return _pLoginRefreshImpl->toString();
}