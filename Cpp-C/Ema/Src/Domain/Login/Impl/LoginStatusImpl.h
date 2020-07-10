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

#ifndef __thomsonreuters_ema_domain_LoginStatusImpl_h
#define __thomsonreuters_ema_domain_LoginStatusImpl_h

namespace thomsonreuters {

namespace ema {

namespace domain {

namespace login {

class LoginStatusImpl
{
public:

	LoginStatusImpl();

	LoginStatusImpl(const LoginStatusImpl&);

	LoginStatusImpl(const thomsonreuters::ema::access::StatusMsg&);

	virtual ~LoginStatusImpl();

	LoginStatusImpl& clear();

	LoginStatusImpl& operator=(const LoginStatusImpl&);

	LoginStatusImpl& message(const thomsonreuters::ema::access::StatusMsg&);

	LoginStatusImpl& authenticationErrorCode(const thomsonreuters::ema::access::UInt64& value);

	LoginStatusImpl& authenticationErrorText(const thomsonreuters::ema::access::EmaString& value);

	LoginStatusImpl& name(const thomsonreuters::ema::access::EmaString&);

	LoginStatusImpl& nameType(const thomsonreuters::ema::access::UInt32&);

	LoginStatusImpl& state(const thomsonreuters::ema::access::OmmState::StreamState&, const thomsonreuters::ema::access::OmmState::DataState, const thomsonreuters::ema::access::UInt8&, const thomsonreuters::ema::access::EmaString&);

	LoginStatusImpl& state(const thomsonreuters::ema::access::OmmState&);

	bool hasAuthenticationErrorCode() const;

	bool hasAuthenticationErrorText() const;

	bool hasName() const;

	bool hasNameType() const;

	bool hasState() const;

	const thomsonreuters::ema::access::StatusMsg& getMessage();

	const thomsonreuters::ema::access::UInt64& getAuthenticationErrorCode() const;

	const thomsonreuters::ema::access::EmaString& getAuthenticationErrorText() const;

	const thomsonreuters::ema::access::EmaString& getName() const;

	const thomsonreuters::ema::access::UInt32& getNameType() const;

	const thomsonreuters::ema::access::OmmState& getState() const;

	const thomsonreuters::ema::access::EmaString& toString() const;

private:

	void encode(thomsonreuters::ema::access::StatusMsg&) const;

	void decode(const thomsonreuters::ema::access::StatusMsg&);

	const thomsonreuters::ema::access::StatusMsg& message() const;

	thomsonreuters::ema::access::UInt64									_authenticationErrorCode;
	thomsonreuters::ema::access::EmaString								_authenticationErrorText;

	mutable bool			_changed;
	bool					_authenticationErrorCodeSet;
	bool					_authenticationErrorTextSet;
	bool					_stateSet;
	bool					_nameSet;
	bool					_nameTypeSet;

	thomsonreuters::ema::access::UInt32					_domainType;

	thomsonreuters::ema::access::UInt32					_nameType;
	thomsonreuters::ema::access::EmaString				_name;

	RsslState*											_rsslState;
	thomsonreuters::ema::access::EmaStringInt			_stateText;
	thomsonreuters::ema::access::OmmState				_state;

	thomsonreuters::ema::access::StatusMsg				_statusMsg;

	mutable thomsonreuters::ema::access::ElementList*	_pElementList;

	mutable thomsonreuters::ema::access::EmaString       _toString;

#ifdef __EMA_COPY_ON_SET__
	thomsonreuters::ema::access::EmaString				_statusText;
#endif
};
			
}

}

}

}

#endif // __thomsonreuters_ema_domain_LoginStatusImpl_h

