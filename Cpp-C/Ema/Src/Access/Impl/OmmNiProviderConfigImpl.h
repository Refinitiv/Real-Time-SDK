/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmNiProviderConfigImpl_h
#define __thomsonreuters_ema_access_OmmNiProviderConfigImpl_h

#include "OmmNiProviderConfig.h"
#include "EmaConfigImpl.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmNiProviderConfigImpl : public EmaConfigImpl
{
public:

	OmmNiProviderConfigImpl( const EmaString & );

	virtual ~OmmNiProviderConfigImpl();

	void clear();

	void providerName( const EmaString& );

	void validateSpecifiedSessionName();

	EmaString getConfiguredName();

	void operationModel( OmmNiProviderConfig::OperationModel );

	void adminControlDirectory( OmmNiProviderConfig::AdminControl );

	OmmNiProviderConfig::OperationModel getOperationModel() const;

	OmmNiProviderConfig::AdminControl getAdminControlDirectory() const;

	RsslReactorOAuthCredential* getReactorOAuthCredential();

private:

	OmmNiProviderConfig::OperationModel		_operationModel;
	OmmNiProviderConfig::AdminControl		_adminControlDirectory;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmNiProviderConfigImpl_h
