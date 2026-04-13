/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

 
#include "BaseRoutingSession.h"
#include "BaseRoutingChannel.h"
#include "OmmBaseImpl.h"

using namespace refinitiv::ema::access;

const EmaString channelUp("session channel up");
const EmaString channelReconnecting("session channel down reconnecting");
const EmaString channelDown("session channel down");
const EmaString phStartingFallback("Preferred host starting fallback");
const EmaString phFallbackComplete("Preferred host complete");
const EmaString phNoFallback("Preferred host no fallback");


BaseRoutingSession::BaseRoutingSession(OmmBaseImpl& consumerBaseImpl) :
	baseImpl(consumerBaseImpl),
	activeConfig(consumerBaseImpl.getActiveConfig()),
	routingChannelList(),
	aggregatedLoginInfo(),
	sentInitialLoginRefresh(false), 
	_statusMsg(),
	sessionType(RoutingSessionType::NONE)
{
	initialLoginRefreshReceived = false;
	activeChannelCount = 0;
}

BaseRoutingSession::~BaseRoutingSession()
{
	clear();
}

//Closes a channel and removes it from the list.
void BaseRoutingSession::closeChannel(RsslReactorChannel* pRsslReactorChannel)
{
	if (baseImpl._pRsslReactor != NULL)
	{
		for (UInt32 i = 0; i < routingChannelList.size(); i++)
		{
			if (routingChannelList[i]->pReactorChannel == pRsslReactorChannel)
			{
				routingChannelList[i]->closeReactorChannel();
				activeChannelCount--;
				return;
			}
		}
	}
}

void BaseRoutingSession::closeReactorChannels()
{
	if (baseImpl._pRsslReactor != NULL)
	{
		for (UInt32 i = 0; i < routingChannelList.size(); ++i)
		{
			if (routingChannelList[i] != NULL && routingChannelList[i]->pReactorChannel != NULL)
			{
				routingChannelList[i]->closeReactorChannel();
			}
		}

	}
}

void BaseRoutingSession::clear()
{
	closeReactorChannels();

	for (UInt32 i = 0; i < routingChannelList.size(); ++i)
	{
		if (routingChannelList[i] != NULL)
		{
			delete routingChannelList[i];
			routingChannelList[i] = NULL;
		}
	}

	routingChannelList.clear();
}


bool BaseRoutingSession::aggregateLoginRefreshInfo(RsslRDMLoginRefresh* pRefreshMsg)
{
	// Make a shallow copy of the old login refresh so we can crossreference the values below
	// Note that we really don't care about the strings here, so they can be cleared.
	RsslRDMLoginRefresh oldLoginRefresh = *(aggregatedLoginInfo.loginRefreshMsg.getRefreshMsg());

	// Clear the aggregated login info, and iterate through the channel list and re-aggregate everything
	aggregatedLoginInfo.loginRefreshMsg.clear();
	BaseRoutingSessionChannel* pRoutingChannel;
	bool changed = false;


	for (UInt32 i = 0; i < routingChannelList.size(); ++i)
	{
		pRoutingChannel = routingChannelList[i];

		// Do not aggregate if the channel is down, has not received the initial OPEN/OK, or is reconnecting.
		if (pRoutingChannel->channelClosed == false && pRoutingChannel->channelState >= OmmBaseImpl::LoginStreamOpenOkEnum && pRoutingChannel->reconnecting == false)
		{
			changed = aggregatedLoginInfo.loginRefreshMsg.aggregateForRequestRouting(pRoutingChannel->loginInfo.loginRefreshMsg.getRefreshMsg(), this);
		}
	}

	if (pRefreshMsg != NULL)
	{
		changed = aggregatedLoginInfo.loginRefreshMsg.aggregateForRequestRouting(pRefreshMsg, this);
	}

	if (initialLoginRefreshReceived == false)
	{
		initialLoginRefreshReceived = true;
		return true;
	}

	// Check for changes to the old refresh flags, if they have changed here, return true.
	if (changed)
	{
		RsslRDMLoginRefresh* pNewRefresh = aggregatedLoginInfo.loginRefreshMsg.getRefreshMsg();

		if (oldLoginRefresh.providePermissionExpressions != pNewRefresh->providePermissionExpressions)
			return true;

		if (oldLoginRefresh.supportBatchRequests != pNewRefresh->supportBatchRequests)
			return true;

		if (oldLoginRefresh.supportOMMPost != pNewRefresh->supportOMMPost)
			return true;

		if (oldLoginRefresh.supportOptimizedPauseResume != pNewRefresh->supportOptimizedPauseResume)
			return true;

		if (oldLoginRefresh.supportEnhancedSymbolList != pNewRefresh->supportEnhancedSymbolList)
			return true;

		if (oldLoginRefresh.supportViewRequests != pNewRefresh->supportViewRequests)
			return true;

		if ((oldLoginRefresh.flags & RDM_LG_RFF_RTT_SUPPORT) != (pNewRefresh->flags & RDM_LG_RFF_RTT_SUPPORT))
			return true;

		if (baseImpl.getImplType() == OmmCommonImpl::NiProviderEnum && oldLoginRefresh.supportProviderDictionaryDownload != pNewRefresh->supportProviderDictionaryDownload)
			return true;
	}

	return false;
	
}

// This will process and handle sending login status messages when channel events happen
// Note: all channel close operations happen after this call.
void BaseRoutingSession::processChannelEvent(BaseRoutingSessionChannel* pSessionChannel, RsslReactorChannelEvent* pEvent)
{
	EmaVector<Item*>& loginItems = baseImpl.getLoginCallbackClient().getLoginItems();
	LoginCallbackClient& loginClient = baseImpl.getLoginCallbackClient();
	RsslStatusMsg statusMsg;
	RsslBuffer tmpBuffer = RSSL_INIT_BUFFER;
	int reconnectingCount = 0;

	// This will clear the statusMsg structure.
	pSessionChannel->loginInfo.loginRefreshMsg.populate(statusMsg, tmpBuffer);
	statusMsg.flags |= RSSL_STMF_HAS_STATE;

	switch (pEvent->channelEventType)
	{
		case RSSL_RC_CET_CHANNEL_UP:
			// Do not send the login if the initial login refresh has been sent
			if (sentInitialLoginRefresh)
				return;

			// Stream state is OPEN. The data State is either OK or SUSPECT.
			statusMsg.state.streamState = RSSL_STREAM_OPEN;

			if (sentInitialLoginRefresh == true)
				statusMsg.state.dataState = RSSL_DATA_OK;
			else
				statusMsg.state.dataState = RSSL_DATA_SUSPECT;

			statusMsg.state.text.data = (char*)channelUp.c_str();
			statusMsg.state.text.length = channelUp.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);

			// It's possible to get this out of order for NiProviders
			if (pSessionChannel->channelState < OmmBaseImpl::RsslChannelUpEnum)
				pSessionChannel->channelState = OmmBaseImpl::RsslChannelUpEnum;
			pSessionChannel->sentChannelUpStatus = true;

			break;
		case RSSL_RC_CET_CHANNEL_READY:
			// Do not send the login we have already sent a channel up status
			if (pSessionChannel->sentChannelUpStatus == true)
				return;

			// Stream state is OPEN. The data State is either OK or SUSPECT.
			statusMsg.state.streamState = RSSL_STREAM_OPEN;

			if(sentInitialLoginRefresh == true)
				statusMsg.state.dataState = RSSL_DATA_OK;
			else
				statusMsg.state.dataState = RSSL_DATA_SUSPECT;

			statusMsg.state.text.data = (char*)channelUp.c_str();
			statusMsg.state.text.length = channelUp.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);

			// It's possible to get this out of order for NiProviders
			if(pSessionChannel->channelState < OmmBaseImpl::RsslChannelUpEnum)
				pSessionChannel->channelState = OmmBaseImpl::RsslChannelUpEnum;
			pSessionChannel->sentChannelUpStatus = true;

			break;
		case RSSL_RC_CET_CHANNEL_DOWN_RECONNECTING:

			statusMsg.state.streamState = RSSL_STREAM_OPEN;

			// Only send SUSPECT if all channels are reconnecting.  Also, if they are all reconnecting, set sentInitialLoginRefresh to false.
			reconnectingCount = getReconnectingCount();

			// If the ommConsumer is currently initializing, send SUSPECT unless we've sent a login OpenOk to everything.
			if ((!baseImpl.isInitialized() && baseImpl.getState() < OmmBaseImpl::LoginStreamOpenOkEnum) || reconnectingCount == activeChannelCount)
			{	 
				statusMsg.state.dataState = RSSL_DATA_SUSPECT;

				// Only clear this if initialization has succeded.
				if (baseImpl.isInitialized())
				{
					sentInitialLoginRefresh = false;
				}
			}
			else
			{
				statusMsg.state.dataState = RSSL_DATA_OK;
			}

			pSessionChannel->channelState = OmmBaseImpl::RsslChannelDownEnum;
			pSessionChannel->sentChannelUpStatus = false;

			statusMsg.state.text.data = (char*)channelReconnecting.c_str();
			statusMsg.state.text.length = channelReconnecting.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);
			break;
		case RSSL_RC_CET_CHANNEL_DOWN:
			statusMsg.state.streamState = RSSL_STREAM_OPEN;

			// If this is the last channel, then send CLOSED/SUSPECT.
			if (activeChannelCount == 1)
			{
				statusMsg.state.streamState = RSSL_STREAM_CLOSED;
				statusMsg.state.dataState = RSSL_DATA_SUSPECT;
			}
			else
			{
				// Only send SUSPECT if all remaining channels are reconnecting
				// The channel has not been closed yet, so the total active should be one less.
				reconnectingCount = getReconnectingCount();

				// Like DOWN_RECONNECTING above, we need to verify that we're not in initialization and have not given the Login Open/OK to the user yet. Send a SUSPECT state in that case
				// Secondly, check to see if there are any reconnecting channels.  If so, since the current channel has gone down, the count of all possible reconnecting channels is activeChannelCount - 1.
				// If that is equal, send SUSPECT, otherwise send OK.
				if ((!baseImpl.isInitialized() && baseImpl.getState() < OmmBaseImpl::LoginStreamOpenOkEnum) || (reconnectingCount != 0 && reconnectingCount == activeChannelCount - 1))
				{
					statusMsg.state.dataState = RSSL_DATA_SUSPECT;
					// Only clear this if initialization has succeded.
					if (baseImpl.isInitialized())
					{
						sentInitialLoginRefresh = false;
					}
				}
				else
				{
					statusMsg.state.dataState = RSSL_DATA_OK;
				}
			}

			pSessionChannel->channelState = OmmBaseImpl::RsslChannelDownEnum;
			pSessionChannel->sentChannelUpStatus = false;

			statusMsg.state.text.data = (char*)channelDown.c_str();
			statusMsg.state.text.length = channelDown.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);
			break;
		case RSSL_RC_CET_PREFERRED_HOST_COMPLETE:
			// The channel just got a DOWN_RECONNECTING call, so we need to preserve the state here.
			statusMsg.state.streamState = RSSL_STREAM_OPEN;

			// Only send SUSPECT if all channels are reconnecting.  Also, if they are all reconnecting, set sentInitialLoginRefresh to false.
			reconnectingCount = getReconnectingCount();

			if (reconnectingCount == activeChannelCount)
			{
				statusMsg.state.dataState = RSSL_DATA_SUSPECT;
			}
			else
			{
				statusMsg.state.dataState = RSSL_DATA_OK;
			}

			statusMsg.state.text.data = (char*)phFallbackComplete.c_str();
			statusMsg.state.text.length = phFallbackComplete.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);

			// This means that preferred host has ended, so set inPreferredHost to false.
			pSessionChannel->inPreferredHost = false;
			break;
		case RSSL_RC_CET_PREFERRED_HOST_NO_FALLBACK:
			// This can only happen on a channel that's in a good state, so it's always OPEN/OK

			statusMsg.state.streamState = RSSL_STREAM_OPEN;
			statusMsg.state.dataState = RSSL_DATA_OK;

			statusMsg.state.text.data = (char*)phNoFallback.c_str();
			statusMsg.state.text.length = phNoFallback.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);
			break;
		case RSSL_RC_CET_PREFERRED_HOST_STARTING_FALLBACK:
			// This can only happen on a channel that's in a good state, so it's always OPEN/OK

			statusMsg.state.streamState = RSSL_STREAM_OPEN;
			statusMsg.state.dataState = RSSL_DATA_OK;

			statusMsg.state.text.data = (char*)phStartingFallback.c_str();
			statusMsg.state.text.length = phStartingFallback.length();

			loginClient.processStatusMsg((RsslMsg*)&statusMsg, pSessionChannel->pReactorChannel, NULL);

			pSessionChannel->sentChannelUpStatus = true;

			// This means that preferred host has started, so set inPreferredHost to true.
			pSessionChannel->inPreferredHost = true;

			break;
		default:
			break;
	}
}


int BaseRoutingSession::getReconnectingCount()
{
	int reconnectingCount = 0;
	for (UInt32 i = 0; i < routingChannelList.size(); i++)
	{
		if (routingChannelList[i]->channelClosed == false && routingChannelList[i]->reconnecting == true)
		{
			++reconnectingCount;
		}
	}

	return reconnectingCount;
}