/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license      --
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
*|                See the project's LICENSE.md for details.                  --
*|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
*|-----------------------------------------------------------------------------
*/

#include "ElementList.h"
#include "EmaString.h"
#include "StatusMsg.h"
#include "ExceptionTranslator.h"
#include "rtr/rsslDataPackage.h"
#include "EmaStringInt.h"
#include "OmmState.h"
#include "OmmStateDecoder.h"

#ifndef __rtsdk_ema_domain_LoginStatusImpl_h
#define __rtsdk_ema_domain_LoginStatusImpl_h

namespace rtsdk {

namespace ema {

namespace domain {

namespace login {

class LoginStatusImpl
{
public:

	LoginStatusImpl();

	LoginStatusImpl(const LoginStatusImpl&);

	LoginStatusImpl(const rtsdk::ema::access::StatusMsg&);

	virtual ~LoginStatusImpl();

	LoginStatusImpl& clear();

	LoginStatusImpl& operator=(const LoginStatusImpl&);

	LoginStatusImpl& message(const rtsdk::ema::access::StatusMsg&);

	LoginStatusImpl& authenticationErrorCode(const rtsdk::ema::access::UInt64& value);

	LoginStatusImpl& authenticationErrorText(const rtsdk::ema::access::EmaString& value);

	LoginStatusImpl& name(const rtsdk::ema::access::EmaString&);

	LoginStatusImpl& nameType(const rtsdk::ema::access::UInt32&);

	LoginStatusImpl& state(const rtsdk::ema::access::OmmState::StreamState&, const rtsdk::ema::access::OmmState::DataState, const rtsdk::ema::access::UInt8&, const rtsdk::ema::access::EmaString&);

	LoginStatusImpl& state(const rtsdk::ema::access::OmmState&);

	bool hasAuthenticationErrorCode() const;

	bool hasAuthenticationErrorText() const;

	bool hasName() const;

	bool hasNameType() const;

	bool hasState() const;

	const rtsdk::ema::access::StatusMsg& getMessage();

	const rtsdk::ema::access::UInt64& getAuthenticationErrorCode() const;

	const rtsdk::ema::access::EmaString& getAuthenticationErrorText() const;

	const rtsdk::ema::access::EmaString& getName() const;

	const rtsdk::ema::access::UInt32& getNameType() const;

	const rtsdk::ema::access::OmmState& getState() const;

	const rtsdk::ema::access::EmaString& toString() const;

private:

	void encode(rtsdk::ema::access::StatusMsg&) const;

	void decode(const rtsdk::ema::access::StatusMsg&);

	const rtsdk::ema::access::StatusMsg& message() const;

	rtsdk::ema::access::UInt64									_authenticationErrorCode;
	rtsdk::ema::access::EmaString								_authenticationErrorText;

	mutable bool			_changed;
	bool					_authenticationErrorCodeSet;
	bool					_authenticationErrorTextSet;
	bool					_stateSet;
	bool					_nameSet;
	bool					_nameTypeSet;

	rtsdk::ema::access::UInt32					_domainType;

	rtsdk::ema::access::UInt32					_nameType;
	rtsdk::ema::access::EmaString				_name;

	RsslState*											_rsslState;
	rtsdk::ema::access::EmaStringInt			_stateText;
	rtsdk::ema::access::OmmState				_state;

	rtsdk::ema::access::StatusMsg				_statusMsg;

	mutable rtsdk::ema::access::ElementList*	_pElementList;

	mutable rtsdk::ema::access::EmaString       _toString;

#ifdef __EMA_COPY_ON_SET__
	rtsdk::ema::access::EmaString				_statusText;
#endif
};
			
}

}

}

}

#endif // __rtsdk_ema_domain_LoginStatusImpl_h

