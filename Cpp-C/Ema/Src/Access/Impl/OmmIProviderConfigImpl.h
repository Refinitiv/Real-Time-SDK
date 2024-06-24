/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_OmmIProviderConfigImpl_h
#define __refinitiv_ema_access_OmmIProviderConfigImpl_h

#include "OmmIProviderConfig.h"
#include "EmaConfigImpl.h"

namespace refinitiv {

namespace ema {

namespace access {

class OmmIProviderConfigImpl : public EmaConfigServerImpl
{
public:

	OmmIProviderConfigImpl( const EmaString & );

	virtual ~OmmIProviderConfigImpl();

	void clear();

	void providerName( const EmaString& );

	void validateSpecifiedSessionName();

	EmaString getConfiguredName();

	void operationModel( OmmIProviderConfig::OperationModel );

	void adminControlDictionary( OmmIProviderConfig::AdminControl );

	void adminControlDirectory( OmmIProviderConfig::AdminControl );

	OmmIProviderConfig::OperationModel getOperationModel() const;

	OmmIProviderConfig::AdminControl getAdminControlDirectory() const;

	OmmIProviderConfig::AdminControl getAdminControlDictionary() const;

private:

	OmmIProviderConfig::OperationModel		_operationModel;
	OmmIProviderConfig::AdminControl		_adminControlDirectory;
	OmmIProviderConfig::AdminControl		_adminControlDictionary;
};

}

}

}

#endif // __refinitiv_ema_access_OmmIProviderConfigImpl_h
