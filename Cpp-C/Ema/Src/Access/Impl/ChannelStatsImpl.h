/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ChannelStatsImpl_h
#define __refinitiv_ema_access_ChannelStatsImpl_h

#include "ItemCallbackClient.h"
#include "OmmBaseImplMap.h"

namespace refinitiv {
namespace ema {
namespace access {

  void getChannelStats(const RsslReactorChannel*, ChannelStatistics&);
}
}
}
#endif
