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

namespace thomsonreuters {

namespace ema {

namespace access {

class OmmNiProviderActiveConfig : public ActiveConfig
{
public:

	OmmNiProviderActiveConfig();
  virtual ~OmmNiProviderActiveConfig() {}
	EmaString niProviderName;
        EmaVector< ChannelConfig* > & getConfigChannelSet() { return channelConfigSet; }

private:
	static UInt64 lastId;
        EmaVector< ChannelConfig* > channelConfigSet;
};

}

}

}

#endif // __thomsonreuters_ema_access_OmmConsumerActiveConfig_h

