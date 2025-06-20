/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2015-2016,2018-2020,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

#include "OmmConsumerEvent.h"
#include "ItemCallbackClient.h"
#include "ActiveConfig.h"
#include "ChannelInfoImpl.h"
#include "ChannelStatsImpl.h"
#include "ChannelCallbackClient.h"
#include "OmmConsumerImpl.h"

using namespace refinitiv::ema::access;

OmmConsumerEvent::OmmConsumerEvent(OmmBaseImpl& baseImpl) :
	_handle( 0 ),
	_closure( 0 ),
	_parentHandle( 0 ),
	_channel( 0 ),
	_ommBaseImpl(baseImpl)
{
}

OmmConsumerEvent::~OmmConsumerEvent()
{
}

UInt64 OmmConsumerEvent::getHandle() const
{
	return _handle;
}

void* OmmConsumerEvent::getClosure() const
{
	return _closure;
}

UInt64 OmmConsumerEvent::getParentHandle() const
{
	return _parentHandle;
}

const ChannelInformation& OmmConsumerEvent::getChannelInformation() const 
{
	RsslReactorChannel* rsslReactorChannel = reinterpret_cast<RsslReactorChannel*>( _channel );

	if (rsslReactorChannel == NULL)
	{
		const_cast<ChannelInformation&>(_channelInfo).clear();
		return _channelInfo;
	}
	ChannelInfoImpl::getChannelInformationImpl( rsslReactorChannel, OmmCommonImpl::ConsumerEnum, const_cast<ChannelInformation&>( _channelInfo ) );
	return _channelInfo;
}

void OmmConsumerEvent::getSessionInformation(EmaVector<ChannelInformation>& channelInfoList) const
{
	RsslReactorChannel* rsslReactorChannel = reinterpret_cast<RsslReactorChannel*>(_channel);
	OmmConsumerImpl& consImpl = (OmmConsumerImpl&)_ommBaseImpl;

	// There isn't a session, so do not get any info.
	if (_ommBaseImpl.getConsumerRoutingSession() == NULL)
	{
		channelInfoList.empty();

		return;
	}
	
	consImpl.getSessionInformation(channelInfoList);

	return;
}

const ChannelStatistics& OmmConsumerEvent::getChannelStatistics() const
{
	RsslReactorChannel* rsslReactorChannel = reinterpret_cast<RsslReactorChannel*>( _channel );
	getChannelStats( rsslReactorChannel, const_cast<ChannelStatistics&>( _channelStats ) );
	return _channelStats;
}
