
/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019, 2025 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_LoginRdmReqMsgImpl_h
#define __refinitiv_ema_access_LoginRdmReqMsgImpl_h


#include "Common.h"
#include "EmaString.h"
#include "EmaBuffer.h"
#include "OmmLoginCredentialConsumerClient.h"
#include "EmaRdm.h"
#include "rtr/rsslRDMLoginMsg.h"
#include "rtr/rsslReactor.h"

namespace refinitiv {

namespace ema {

namespace access {

class EmaConfigImpl;

class LoginRdmReqMsgImpl
{
public :
	LoginRdmReqMsgImpl();

	LoginRdmReqMsgImpl(OmmLoginCredentialConsumerClient&);

	LoginRdmReqMsgImpl(OmmLoginCredentialConsumerClient&, void* closure);

	LoginRdmReqMsgImpl(const LoginRdmReqMsgImpl&);

	~LoginRdmReqMsgImpl();

	LoginRdmReqMsgImpl& clear();

	LoginRdmReqMsgImpl& set(EmaConfigImpl*, RsslRequestMsg* );

	LoginRdmReqMsgImpl& overlay(RsslRDMLoginRequest* pRequest);

	RsslRDMLoginRequest* get();

	LoginRdmReqMsgImpl& clearPassword();

	LoginRdmReqMsgImpl& arrayIndex(UInt8);

	const EmaString& channelList();

	const EmaString& getUserName();

	const EmaString& getPassword();

	void* getClosure();

	bool hasLoginClient();

	UInt8 getArrayIndex();

	OmmLoginCredentialConsumerClient& getClient();

	const EmaString& toString();

	LoginRdmReqMsgImpl& username( const EmaString& );

	LoginRdmReqMsgImpl& position( const EmaString& );

	LoginRdmReqMsgImpl& password( const EmaString& );

	LoginRdmReqMsgImpl& applicationId( const EmaString& );

	LoginRdmReqMsgImpl& applicationName( const EmaString& );

	LoginRdmReqMsgImpl& applicationAuthorizationToken(const EmaString&);

	LoginRdmReqMsgImpl& authenticationExtended(const EmaBuffer&);

	LoginRdmReqMsgImpl& instanceId( const EmaString& );

	LoginRdmReqMsgImpl& setRole( RDMLoginRoleTypes );

	LoginRdmReqMsgImpl& setChannelList(const EmaString&);

private :

	EmaString				_username;
	EmaString				_password;
	EmaString				_authenticationToken;
	EmaBuffer				_authenticationExtended;
	EmaString				_applicationAuthorizationToken;
	EmaString				_position;
	EmaString				_applicationId;
	EmaString				_applicationName;
	EmaString				_instanceId;
	RsslRDMLoginRequest		_rsslRdmLoginRequest;
	EmaString				_defaultApplicationName;
	EmaString				_channelList;
	OmmLoginCredentialConsumerClient& _client;
	RsslBool				_hasLoginClient;
	void*					_closure;
	UInt8					_arrayIndex;

	bool					_toStringSet;
	EmaString				_toString;
	

};

}

}

}


#endif // __refinitiv_ema_access_OmmConsumerConfigImpl_h