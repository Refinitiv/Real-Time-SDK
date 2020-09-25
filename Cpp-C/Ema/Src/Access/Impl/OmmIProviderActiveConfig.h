/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
*/

#ifndef __refinitiv_ema_access_OmmIProviderActiveConfig_h
#define __refinitiv_ema_access_OmmIProviderActiveConfig_h

#include "ActiveConfig.h"
#include "OmmIProviderConfig.h"

namespace rtsdk {

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

	bool getRefreshFirstRequired();

	bool getEnforceAckIDValidation();

	void setMaxFieldDictFragmentSize(UInt64);

	void setMaxEnumTypeFragmentSize(UInt64);

	void setRefreshFirstRequired(UInt64);

	void setEnforceAckIDValidation(UInt64);

	EmaString configTrace();
private:

	friend class OmmIProviderImpl;

	OmmIProviderConfig::OperationModel		operationModel;
	OmmIProviderConfig::AdminControl		dictionaryAdminControl;
	OmmIProviderConfig::AdminControl		directoryAdminControl;
	bool									refreshFirstRequired;
	bool									enforceAckIDValidation;
	UInt32									maxFieldDictFragmentSize;
	UInt32									maxEnumTypeFragmentSize;
};

}

}

}

#endif // __refinitiv_ema_access_OmmIProviderActiveConfig_h
