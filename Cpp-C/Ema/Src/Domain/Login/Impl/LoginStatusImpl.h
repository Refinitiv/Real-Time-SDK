/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.
*|                See the project's LICENSE.md for details.
*|           Copyright (C) 2019 LSEG. All rights reserved.                 --
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

#ifndef __refinitiv_ema_domain_LoginStatusImpl_h
#define __refinitiv_ema_domain_LoginStatusImpl_h

namespace refinitiv {

namespace ema {

namespace domain {

namespace login {

class LoginStatusImpl
{
public:

	LoginStatusImpl();

	LoginStatusImpl(const LoginStatusImpl&);

	LoginStatusImpl(const refinitiv::ema::access::StatusMsg&);

	virtual ~LoginStatusImpl();

	LoginStatusImpl& clear();

	LoginStatusImpl& operator=(const LoginStatusImpl&);

	LoginStatusImpl& message(const refinitiv::ema::access::StatusMsg&);

	LoginStatusImpl& authenticationErrorCode(const refinitiv::ema::access::UInt64& value);

	LoginStatusImpl& authenticationErrorText(const refinitiv::ema::access::EmaString& value);

	LoginStatusImpl& name(const refinitiv::ema::access::EmaString&);

	LoginStatusImpl& nameType(const refinitiv::ema::access::UInt32&);

	LoginStatusImpl& state(const refinitiv::ema::access::OmmState::StreamState&, const refinitiv::ema::access::OmmState::DataState, const refinitiv::ema::access::UInt8&, const refinitiv::ema::access::EmaString&);

	LoginStatusImpl& state(const refinitiv::ema::access::OmmState&);

	bool hasAuthenticationErrorCode() const;

	bool hasAuthenticationErrorText() const;

	bool hasName() const;

	bool hasNameType() const;

	bool hasState() const;

	const refinitiv::ema::access::StatusMsg& getMessage();

	const refinitiv::ema::access::UInt64& getAuthenticationErrorCode() const;

	const refinitiv::ema::access::EmaString& getAuthenticationErrorText() const;

	const refinitiv::ema::access::EmaString& getName() const;

	const refinitiv::ema::access::UInt32& getNameType() const;

	const refinitiv::ema::access::OmmState& getState() const;

	const refinitiv::ema::access::EmaString& toString() const;

private:

	void encode(refinitiv::ema::access::StatusMsg&) const;

	void decode(const refinitiv::ema::access::StatusMsg&);

	const refinitiv::ema::access::StatusMsg& message() const;

	refinitiv::ema::access::UInt64									_authenticationErrorCode;
	refinitiv::ema::access::EmaString								_authenticationErrorText;

	mutable bool			_changed;
	bool					_authenticationErrorCodeSet;
	bool					_authenticationErrorTextSet;
	bool					_stateSet;
	bool					_nameSet;
	bool					_nameTypeSet;

	refinitiv::ema::access::UInt32					_domainType;

	refinitiv::ema::access::UInt32					_nameType;
	refinitiv::ema::access::EmaString				_name;

	RsslState*											_rsslState;
	refinitiv::ema::access::EmaStringInt			_stateText;
	refinitiv::ema::access::OmmState				_state;

	refinitiv::ema::access::StatusMsg				_statusMsg;

	mutable refinitiv::ema::access::ElementList*	_pElementList;

	mutable refinitiv::ema::access::EmaString       _toString;

#ifdef __EMA_COPY_ON_SET__
	refinitiv::ema::access::EmaString				_statusText;
#endif
};
			
}

}

}

}

#endif // __refinitiv_ema_domain_LoginStatusImpl_h

