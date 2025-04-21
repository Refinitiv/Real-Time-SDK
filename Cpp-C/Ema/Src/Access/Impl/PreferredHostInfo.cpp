/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
 *|-----------------------------------------------------------------------------
 */

#include "PreferredHostInfo.h"
#include "ChannelCallbackClient.h"
#include "ConsumerRoutingChannel.h"

using namespace refinitiv::ema::access;

PreferredHostInfo::PreferredHostInfo() :
	_enablePreferredHostOptions(false),
	_phDetectionTimeSchedule(""),
	_phDetectionTimeInterval(0),
	_preferredChannelName(""),
	_preferredWSBChannelName(""),
	_phFallBackWithInWSBGroup(false),
	_remainingDetectionTime(0),
	_isChannelPreferred(false),
	_toString("")
{
}

PreferredHostInfo::~PreferredHostInfo()
{
}

PreferredHostInfo& PreferredHostInfo::clear(){
	_enablePreferredHostOptions = false;
	_phDetectionTimeSchedule.clear();
	_phDetectionTimeInterval = 0;
	_preferredChannelName.clear();
	_preferredWSBChannelName.clear();
	_phFallBackWithInWSBGroup = false;
	_remainingDetectionTime = 0;
	_isChannelPreferred = false;
	_toString.clear();

	return *this;
}

const EmaString& PreferredHostInfo::toString() const
{
	_toString.append("\n\tph preferred host option: ").append(_enablePreferredHostOptions ? "enabled" : "disabled")
		.append("\n\tph detection time schedule: ").append(_phDetectionTimeSchedule)
		.append("\n\tph detection time interval: ").append(_phDetectionTimeInterval)
		.append("\n\tph channel name: ").append(_preferredChannelName)
		.append("\n\tph wsb channel name: ").append(_preferredWSBChannelName)
		.append("\n\tph fall back with in WSB group: ").append(_phFallBackWithInWSBGroup ? "enabled" : "disabled")
		.append("\n\tph is channel preferred: ").append(_isChannelPreferred ? "preferred" : "non-preferred")
		.append("\n\tph remaining detection time: ").append(_remainingDetectionTime);

	return _toString;
}

PreferredHostInfo& PreferredHostInfo::enablePreferredHostOptions(bool enablePreferredHostOptions) {
	_enablePreferredHostOptions = enablePreferredHostOptions;
	return *this;
}

PreferredHostInfo& PreferredHostInfo::phDetectionTimeSchedule(EmaString phDetectionTimeSchedule) {
	_phDetectionTimeSchedule = phDetectionTimeSchedule;
	return *this;
}

PreferredHostInfo& PreferredHostInfo::phDetectionTimeInterval(UInt32 phDetectionTimeInterval) {
	_phDetectionTimeInterval = phDetectionTimeInterval;
	return *this;
}

PreferredHostInfo& PreferredHostInfo::preferredChannelName(UInt32 channelIndex, const void* pChannel) {

	if (!pChannel) {
		_preferredChannelName.append("Channel data is not set for channel set preferred host.");
		return *this;
	}

	OmmBaseImpl* pBaseImpl = ((Channel*)pChannel)->getBaseImpl();
	if (pBaseImpl->getConsumerRoutingSession() == NULL)
	{
		const ActiveConfig& activeConfig = pBaseImpl->getActiveConfig();

		if (!activeConfig.configChannelSet.empty())
			_preferredChannelName = activeConfig.configChannelSet[channelIndex]->name;
	}
	else
	{
		ConsumerRoutingSessionChannel* pSessionChannel = ((Channel*)pChannel)->getConsumerRoutingChannel();

		if (!pSessionChannel->routingChannelConfig.configChannelSet.empty())
		{
			_preferredChannelName = pSessionChannel->routingChannelConfig.configChannelSet[channelIndex]->name;
		}
	}

	return *this;
}

PreferredHostInfo& PreferredHostInfo::preferredWSBChannelName(UInt32 wsbChannelIndex, const void* pChannel) {

	if (!pChannel) {
		_preferredWSBChannelName.append("Channel data is not set for warm standby channels preferred host.");
		return *this;
	}

	OmmBaseImpl* pBaseImpl = ((Channel*)pChannel)->getBaseImpl();
	if (pBaseImpl->getConsumerRoutingSession() == NULL)
	{
		const ActiveConfig& activeConfig = pBaseImpl->getActiveConfig();

	if (!activeConfig.configWarmStandbySet.empty())
			_preferredWSBChannelName = activeConfig.configWarmStandbySet[wsbChannelIndex]->name;
	}
	else
	{
		ConsumerRoutingSessionChannel* pSessionChannel = ((Channel*)pChannel)->getConsumerRoutingChannel();

		if (!pSessionChannel->routingChannelConfig.configWarmStandbySet.empty())
		{
			_preferredWSBChannelName = pSessionChannel->routingChannelConfig.configWarmStandbySet[wsbChannelIndex]->name;
		}
	}

	return *this;
}

PreferredHostInfo& PreferredHostInfo::phFallBackWithInWSBGroup(bool phFallBackWithInWSBGroup) {

	_phFallBackWithInWSBGroup = phFallBackWithInWSBGroup;
	return *this;
}

PreferredHostInfo& PreferredHostInfo::remainingDetectionTime(UInt32 remainingDetectionTime) {

	_remainingDetectionTime = remainingDetectionTime;
	return *this;
}

PreferredHostInfo& PreferredHostInfo::isChannelPreferred(bool isChannelPreferred) {

	_isChannelPreferred = isChannelPreferred;
	return *this;
}
