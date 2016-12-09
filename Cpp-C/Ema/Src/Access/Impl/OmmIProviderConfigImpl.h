/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmIProviderConfigImpl_h
#define __thomsonreuters_ema_access_OmmIProviderConfigImpl_h

#include "OmmIProviderConfig.h"
#include "EmaConfigImpl.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmIProviderConfigImpl : public EmaConfigServerImpl
{
public:

	OmmIProviderConfigImpl();

	virtual ~OmmIProviderConfigImpl();

	void clear();

	void providerName( const EmaString& );

	EmaString getConfiguredName();

	void operationModel( OmmIProviderConfig::OperationModel );

	void adminControlDictionary( OmmIProviderConfig::AdminControl );

	void adminControlDirectory( OmmIProviderConfig::AdminControl );

	OmmIProviderConfig::OperationModel getOperationModel() const;

	OmmIProviderConfig::AdminControl getAdminControlDirectory() const;

	OmmIProviderConfig::AdminControl getAdminControlDictionary() const;

	bool getDictionaryName( const EmaString& , EmaString& ) const;

	bool getDirectoryName( const EmaString& , EmaString& ) const;

private:

	OmmIProviderConfig::OperationModel		_operationModel;
	OmmIProviderConfig::AdminControl		_adminControlDirectory;
	OmmIProviderConfig::AdminControl		_adminControlDictionary;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmIProviderConfigImpl_h
