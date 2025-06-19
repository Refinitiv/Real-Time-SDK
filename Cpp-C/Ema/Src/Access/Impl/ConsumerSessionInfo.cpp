/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "Access/Include/ConsumerSessionInfo.h"

using namespace refinitiv::ema::access;

ConsumerSessionInfo::ConsumerSessionInfo()
{
}

ConsumerSessionInfo::~ConsumerSessionInfo()
{
}

const ChannelInformation& ConsumerSessionInfo::getChannelInformation() const
{
	return _channelInfo;
}
