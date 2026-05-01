/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "ConsumerRoutingChannel.h"
#include "ConsumerRoutingSession.h"



using namespace refinitiv::ema::access;
 
size_t ConsumerRoutingSessionChannel::UInt16rHasher::operator()(const UInt16& value) const
{
	return value;
}

bool ConsumerRoutingSessionChannel::UInt16Equal_To::operator()(const UInt16& x, const UInt16& y) const
{
	return x == y ? true : false;
}

size_t ConsumerRoutingSessionChannel::EmaStringPtrHasher::operator()(const EmaStringPtr& value) const
{
	size_t result = 0;
	size_t magic = 8388593;

	const char* s = value->c_str();
	UInt32 n = value->length();
	while (n--)
		result = ((result % magic) << 8) + (size_t)*s++;
	return result;
}

bool ConsumerRoutingSessionChannel::EmaStringPtrEqual_To::operator()(const EmaStringPtr& x, const EmaStringPtr& y) const
{
	return *x == *y;
}

void ConsumerRoutingSessionChannelConfig::clearWSBChannelSet()
{
	if (configWarmStandbySet.size() == 0)
		return;
	for (unsigned int i = 0; i < configWarmStandbySet.size(); ++i)
	{
		if (configWarmStandbySet[i] != NULL)
		{
			delete configWarmStandbySet[i];
			configWarmStandbySet[i] = NULL;
		}
	}

	configWarmStandbySet.clear();
}

void ConsumerRoutingSessionChannelConfig::clearChannelSetForWSB()
{
	if (configChannelSetForWSB.size() == 0)
		return;
	for (unsigned int i = 0; i < configChannelSetForWSB.size(); ++i)
	{
		if (configChannelSetForWSB[i] != NULL)
		{
			delete configChannelSetForWSB[i];
			configChannelSetForWSB[i] = NULL;
		}
	}

	configChannelSetForWSB.clear();
}


// Default values are taken directly from ActiveConfig.h
ConsumerRoutingSessionChannelConfig::ConsumerRoutingSessionChannelConfig(const EmaString& channelName, ActiveConfig& activeConfig) :
	BaseRoutingSessionChannelConfig(channelName, activeConfig),
	enablePreferredHostOptions(false),
	phDetectionTimeSchedule(activeConfig.phDetectionTimeSchedule),
	phDetectionTimeInterval(activeConfig.phDetectionTimeInterval),
	preferredChannelName(),
	preferredWSBChannelName(),
	phFallBackWithInWSBGroup(activeConfig.phFallBackWithInWSBGroup)
{
	pRoutingChannel = NULL;

	rsslClearReactorConnectOptions(&connectOpts);

	useActiveConfigLogger = true;
}

ConsumerRoutingSessionChannelConfig::~ConsumerRoutingSessionChannelConfig()
{
	clear();
}

void ConsumerRoutingSessionChannelConfig::clear()
{
	BaseRoutingSessionChannelConfig::clear();
	enablePreferredHostOptions = false;
	phDetectionTimeSchedule.clear();
	phDetectionTimeInterval = 0;
	preferredChannelName.clear();
	preferredWSBChannelName.clear();
	phFallBackWithInWSBGroup = false;


	clearWSBChannelSet();
	clearChannelSetForWSB();
}

ConsumerRoutingSessionChannel::ConsumerRoutingSessionChannel(OmmBaseImpl& consumerBaseImpl, const EmaString& sessionChannelName, ConsumerRoutingSessionChannelConfig& sessionChannelConfig) :
	BaseRoutingSessionChannel(consumerBaseImpl, sessionChannelName, static_cast<BaseRoutingSessionChannelConfig&>(sessionChannelConfig)),
	serviceById(),
	serviceByName(),
	serviceList(),
	routedRequestList(consumerBaseImpl)
{
}

ConsumerRoutingSessionChannel::~ConsumerRoutingSessionChannel()
{
	clear();
}

// Inherits from BaseRoutingSessionchannel::clear()
void ConsumerRoutingSessionChannel::clear()
{
	BaseRoutingSessionChannel::clear();
	pRoutingSession = NULL;

	serviceById.clear();
	serviceByName.clear();

	Directory* pDirectory = serviceList.pop_front();
	while(pDirectory != NULL)
	{
		Directory::destroy(pDirectory);
		pDirectory = serviceList.pop_front();
	}

	serviceList.clear();
}
