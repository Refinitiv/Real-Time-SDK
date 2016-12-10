/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2016. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#ifndef __thomsonreuters_ema_access_OmmIProviderActiveConfig_h
#define __thomsonreuters_ema_access_OmmIProviderActiveConfig_h

#include "ActiveConfig.h"
#include "OmmIProviderConfig.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmIProviderImpl;

class OmmIProviderActiveConfig : public ActiveServerConfig
{
public:
	OmmIProviderActiveConfig();
	virtual ~OmmIProviderActiveConfig();

	void clear();

	OmmIProviderConfig::AdminControl getDictionaryAdminControl();

	OmmIProviderConfig::AdminControl getDirectoryAdminControl();

	UInt32 getMaxFieldDictFragmentSize();

	UInt32 getMaxEnumTypeFragmentSize();

private:

	friend class OmmIProviderImpl;

	OmmIProviderConfig::OperationModel		operationModel;
	OmmIProviderConfig::AdminControl		dictionaryAdminControl;
	OmmIProviderConfig::AdminControl		directoryAdminControl;
	bool									refreshFirstRequired;
	UInt32									maxFieldDictFragmentSize;
	UInt32									maxEnumTypeFragmentSize;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmIProviderActiveConfig_h
