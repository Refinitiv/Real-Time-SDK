/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright Thomson Reuters 2015. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

#ifndef __thomsonreuters_ema_access_OmmNiProviderActiveConfig_h
#define __thomsonreuters_ema_access_OmmNiProviderActiveConfig_h

#include "ActiveConfig.h"
#include "OmmProviderConfig.h"
#include "rtr/rsslState.h"
#include "rtr/rsslQos.h"

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmNiProviderActiveConfig : public ActiveConfig
{
public:

	OmmNiProviderActiveConfig();

	virtual ~OmmNiProviderActiveConfig();

	void clear();

	OmmNiProviderConfig::AdminControl getDirectoryAdminControl();
	bool getMergeSourceDirectoryStreams();
	bool getRecoverUserSubmitSourceDirectory();
	bool getRemoveItemsOnDisconnect();
	bool getRefreshFirstRequired();
	void setMergeSourceDirectoryStreams(UInt64);
	void setRecoverUserSubmitSourceDirectory(UInt64);
	void setRemoveItemsOnDisconnect(UInt64);
	void setRefreshFirstRequired(UInt64);
	EmaString configTrace();

private:

	friend class OmmNiProviderImpl;

	OmmNiProviderConfig::OperationModel		operationModel;
	OmmNiProviderConfig::AdminControl		directoryAdminControl;
	bool									refreshFirstRequired;
	bool									mergeSourceDirectoryStreams;
	bool									recoverUserSubmitSourceDirectory;
	bool									removeItemsOnDisconnect;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmNiProviderActiveConfig_h
