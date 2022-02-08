/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmConsumerConfigImpl_h
#define __refinitiv_ema_access_OmmConsumerConfigImpl_h

#ifdef WIN32
#include "direct.h"
#endif

#include "OmmConsumerConfig.h"
#include "EmaConfigImpl.h"
#include "ExceptionTranslator.h"
#include "ProgrammaticConfigure.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmConsumerConfigImpl : public EmaConfigImpl
{
public:

	OmmConsumerConfigImpl(const EmaString &);

	virtual ~OmmConsumerConfigImpl();

	void consumerName( const EmaString& );

	void validateSpecifiedSessionName();

	EmaString getConfiguredName();

	void operationModel( OmmConsumerConfig::OperationModel );

	OmmConsumerConfig::OperationModel operationModel() const;

	bool getDictionaryName( const EmaString& , EmaString& ) const;

	bool getDirectoryName( const EmaString& , EmaString& ) const;

	RsslReactorOAuthCredential* getReactorOAuthCredential();

	void setOmmRestLoggingClient(OmmRestLoggingClient*, void*);

	OmmRestLoggingClient* getOmmRestLoggingClient() const;

	void* getRestLoggingClosure() const;

private:

	OmmConsumerConfig::OperationModel		_operationModel;
	RsslReactorOAuthCredential				_reactorOAuthCredential;
	OmmRestLoggingClient*					_pOmmRestLoggingClient;
	void*									_pRestLoggingClosure;
};

}

}

}

#endif // __refinitiv_ema_access_OmmConsumerConfigImpl_h
