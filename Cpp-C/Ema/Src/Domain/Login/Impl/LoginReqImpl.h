/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#ifndef __refinitiv_ema_domain_LoginReqImpl_h
#define __refinitiv_ema_domain_LoginReqImpl_h

#include "ElementList.h"
#include "EmaBuffer.h"
#include "EmaString.h"
#include "ReqMsg.h"
#include "ExceptionTranslator.h"

namespace refinitiv {

namespace ema {

namespace domain {

namespace login {

class LoginReqImpl
{
public:

	LoginReqImpl();

	LoginReqImpl(const LoginReqImpl&);

	LoginReqImpl(const refinitiv::ema::access::ReqMsg&);

	virtual ~LoginReqImpl();

	LoginReqImpl& clear();

	LoginReqImpl& operator=(const LoginReqImpl&);

	LoginReqImpl& message(const refinitiv::ema::access::ReqMsg&);

	LoginReqImpl& allowSuspectData(bool value);

	LoginReqImpl& downloadConnectionConfig(bool value);

	LoginReqImpl& applicationId(const refinitiv::ema::access::EmaString&);

	LoginReqImpl& applicationName(const refinitiv::ema::access::EmaString&);

	LoginReqImpl& applicationAuthorizationToken(const refinitiv::ema::access::EmaString&);

	LoginReqImpl& instanceId(const refinitiv::ema::access::EmaString&);

	LoginReqImpl& password(const refinitiv::ema::access::EmaString&);

	LoginReqImpl& position(const refinitiv::ema::access::EmaString&);

	LoginReqImpl& providePermissionExpressions(bool value);

	LoginReqImpl& providePermissionProfile(bool value);

	LoginReqImpl& role(refinitiv::ema::access::UInt32 value);

	LoginReqImpl& singleOpen(bool value);

	LoginReqImpl& supportProviderDictionaryDownload(bool value);

	LoginReqImpl& pause(bool value);

	LoginReqImpl& authenticationExtended(const refinitiv::ema::access::EmaBuffer&);

	LoginReqImpl& name(const refinitiv::ema::access::EmaString&);

	LoginReqImpl& nameType(const refinitiv::ema::access::UInt32&);

	bool hasAllowSuspectData() const;

	bool hasDownloadConnectionConfig() const;

	bool hasApplicationId() const;

	bool hasApplicationName() const;

	bool hasApplicationAuthorizationToken() const;

	bool hasInstanceId() const;

	bool hasPassword() const;

	bool hasPosition() const;

	bool hasProvidePermissionExpressions() const;

	bool hasProvidePermissionProfile() const;

	bool hasRole() const;

	bool hasSingleOpen() const;

	bool hasSupportProviderDictionaryDownload() const;

	bool hasPause() const;

	bool hasAuthenticationExtended() const;

	bool hasName() const;

	bool hasNameType() const;

	bool getAllowSuspectData() const;

	bool getDownloadConnectionConfig() const;

	const refinitiv::ema::access::ReqMsg& getMessage();

	const refinitiv::ema::access::EmaString& getApplicationId() const;

	const refinitiv::ema::access::EmaString& getApplicationName() const;

	const refinitiv::ema::access::EmaString& getApplicationAuthorizationToken() const;

	const refinitiv::ema::access::EmaString& getInstanceId() const;

	const refinitiv::ema::access::EmaString& getPassword() const;

	const refinitiv::ema::access::EmaString& getPosition() const;

	bool getProvidePermissionExpressions() const;

	bool getProvidePermissionProfile() const;

	refinitiv::ema::access::UInt32 getRole() const;

	bool getSingleOpen() const;

	bool getSupportProviderDictionaryDownload() const;

	bool getPause() const;

	const refinitiv::ema::access::EmaBuffer& getAuthenticationExtended() const;

	const refinitiv::ema::access::EmaString& getName() const;

	const refinitiv::ema::access::UInt32& getNameType() const;

	const refinitiv::ema::access::EmaString& toString() const;

private:

	void encode(refinitiv::ema::access::ReqMsg&) const;

	void decode(const refinitiv::ema::access::ReqMsg&);

	const refinitiv::ema::access::ReqMsg& message() const;

	bool								    _allowSuspectData;
	bool								    _downloadConnectionConfig;
	bool									_providePermissionProfile;
	bool									_providePermissionExpressions;
	bool						            _singleOpen;
	bool									_supportProviderDictionaryDownload;
	refinitiv::ema::access::UInt32									_role;
	bool									_pause;
	refinitiv::ema::access::EmaString	                            _applicationId;
	refinitiv::ema::access::EmaString	                            _applicationName;
	refinitiv::ema::access::EmaString	                            _applicationAuthToken;
	refinitiv::ema::access::EmaString	                            _instanceId;
	refinitiv::ema::access::EmaString	                            _password;
	refinitiv::ema::access::EmaString	                            _position;
	refinitiv::ema::access::EmaString								_authenticationToken;
	refinitiv::ema::access::EmaBuffer								_authenticationExtended;

	mutable bool		                    _changed;
	bool		                            _allowSuspectDataSet;
	bool		                            _downloadConnectionConfigSet;
	bool		                            _providePermissionProfileSet;
	bool		                            _providePermissionExpressionsSet;
	bool		                            _singleOpenSet;
	bool		                            _supportProviderDictionaryDownloadSet;
	bool		                            _roleSet;
	bool									_pauseSet;
	bool		                            _applicationIdSet;
	bool		                            _applicationNameSet;
	bool		                            _applicationAuthTokenSet;
	bool		                            _instanceIdSet;
	bool		                            _passwordSet;
	bool		                            _positionSet;
	bool									_authenticationExtendedSet;
	bool									_nameSet;
	bool									_nameTypeSet;

	mutable refinitiv::ema::access::ElementList*	                _pElementList;

	refinitiv::ema::access::UInt32									_domainType;
	refinitiv::ema::access::UInt32									_nameType;
	refinitiv::ema::access::EmaString								_name;
	
	refinitiv::ema::access::EmaString						_defaultName;
	refinitiv::ema::access::EmaString						_defaultPosition;

	refinitiv::ema::access::ReqMsg									_reqMsg;

	mutable refinitiv::ema::access::EmaString                       _toString;

	char																defaultUsername[256];
	char																defaultPosition[256];
};

}

}

}

}

#endif // __refinitiv_ema_domain_LoginReqImpl_h
