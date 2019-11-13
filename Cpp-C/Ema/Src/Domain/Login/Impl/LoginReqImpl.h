/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#ifndef __thomsonreuters_ema_domain_LoginReqImpl_h
#define __thomsonreuters_ema_domain_LoginReqImpl_h

#include "ElementList.h"
#include "EmaBuffer.h"
#include "EmaString.h"
#include "ReqMsg.h"
#include "ExceptionTranslator.h"

namespace thomsonreuters {

namespace ema {

namespace domain {

namespace login {

class LoginReqImpl
{
public:

	LoginReqImpl();

	LoginReqImpl(const LoginReqImpl&);

	LoginReqImpl(const thomsonreuters::ema::access::ReqMsg&);

	virtual ~LoginReqImpl();

	LoginReqImpl& clear();

	LoginReqImpl& operator=(const LoginReqImpl&);

	LoginReqImpl& message(const thomsonreuters::ema::access::ReqMsg&);

	LoginReqImpl& allowSuspectData(bool value);

	LoginReqImpl& downloadConnectionConfig(bool value);

	LoginReqImpl& applicationId(const thomsonreuters::ema::access::EmaString&);

	LoginReqImpl& applicationName(const thomsonreuters::ema::access::EmaString&);

	LoginReqImpl& applicationAuthorizationToken(const thomsonreuters::ema::access::EmaString&);

	LoginReqImpl& instanceId(const thomsonreuters::ema::access::EmaString&);

	LoginReqImpl& password(const thomsonreuters::ema::access::EmaString&);

	LoginReqImpl& position(const thomsonreuters::ema::access::EmaString&);

	LoginReqImpl& providePermissionExpressions(bool value);

	LoginReqImpl& providePermissionProfile(bool value);

	LoginReqImpl& role(thomsonreuters::ema::access::UInt32 value);

	LoginReqImpl& singleOpen(bool value);

	LoginReqImpl& supportProviderDictionaryDownload(bool value);

	LoginReqImpl& pause(bool value);

	LoginReqImpl& authenticationExtended(const thomsonreuters::ema::access::EmaBuffer&);

	LoginReqImpl& name(const thomsonreuters::ema::access::EmaString&);

	LoginReqImpl& nameType(const thomsonreuters::ema::access::UInt32&);

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

	const thomsonreuters::ema::access::ReqMsg& getMessage();

	const thomsonreuters::ema::access::EmaString& getApplicationId() const;

	const thomsonreuters::ema::access::EmaString& getApplicationName() const;

	const thomsonreuters::ema::access::EmaString& getApplicationAuthorizationToken() const;

	const thomsonreuters::ema::access::EmaString& getInstanceId() const;

	const thomsonreuters::ema::access::EmaString& getPassword() const;

	const thomsonreuters::ema::access::EmaString& getPosition() const;

	bool getProvidePermissionExpressions() const;

	bool getProvidePermissionProfile() const;

	thomsonreuters::ema::access::UInt32 getRole() const;

	bool getSingleOpen() const;

	bool getSupportProviderDictionaryDownload() const;

	bool getPause() const;

	const thomsonreuters::ema::access::EmaBuffer& getAuthenticationExtended() const;

	const thomsonreuters::ema::access::EmaString& getName() const;

	const thomsonreuters::ema::access::UInt32& getNameType() const;

	const thomsonreuters::ema::access::EmaString& toString() const;

private:

	void encode(thomsonreuters::ema::access::ReqMsg&) const;

	void decode(const thomsonreuters::ema::access::ReqMsg&);

	const thomsonreuters::ema::access::ReqMsg& message() const;

	bool								    _allowSuspectData;
	bool								    _downloadConnectionConfig;
	bool									_providePermissionProfile;
	bool									_providePermissionExpressions;
	bool						            _singleOpen;
	bool									_supportProviderDictionaryDownload;
	thomsonreuters::ema::access::UInt32									_role;
	bool									_pause;
	thomsonreuters::ema::access::EmaString	                            _applicationId;
	thomsonreuters::ema::access::EmaString	                            _applicationName;
	thomsonreuters::ema::access::EmaString	                            _applicationAuthToken;
	thomsonreuters::ema::access::EmaString	                            _instanceId;
	thomsonreuters::ema::access::EmaString	                            _password;
	thomsonreuters::ema::access::EmaString	                            _position;
	thomsonreuters::ema::access::EmaString								_authenticationToken;
	thomsonreuters::ema::access::EmaBuffer								_authenticationExtended;

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

	mutable thomsonreuters::ema::access::ElementList*	                _pElementList;

	thomsonreuters::ema::access::UInt32									_domainType;
	thomsonreuters::ema::access::UInt32									_nameType;
	thomsonreuters::ema::access::EmaString								_name;
	
	thomsonreuters::ema::access::EmaString						_defaultName;
	thomsonreuters::ema::access::EmaString						_defaultPosition;

	thomsonreuters::ema::access::ReqMsg									_reqMsg;

	mutable thomsonreuters::ema::access::EmaString                       _toString;

	char																defaultUsername[256];
	char																defaultPosition[256];
};

}

}

}

}

#endif // __thomsonreuters_ema_domain_LoginReqImpl_h
