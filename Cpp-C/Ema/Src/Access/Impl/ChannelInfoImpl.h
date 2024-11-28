/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019 LSEG. All rights reserved.                 --
 *|-----------------------------------------------------------------------------
 */

#ifndef __refinitiv_ema_access_ChannelInfoImpl_h
#define __refinitiv_ema_access_ChannelInfoImpl_h

#include "ItemCallbackClient.h"
#include "OmmBaseImplMap.h"

namespace refinitiv {
namespace ema {
namespace access {

class ChannelInfoImpl
{
public:
  static void getChannelInformation(const RsslReactorChannel*, const RsslChannel*, ChannelInformation&);
  static void getChannelInformationImpl(const RsslReactorChannel*, OmmCommonImpl::ImplementationType,
								 ChannelInformation&);
};
}
}
}
#endif
