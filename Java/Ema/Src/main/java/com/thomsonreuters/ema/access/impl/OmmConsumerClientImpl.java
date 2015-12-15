///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmConsumerClient;
import com.thomsonreuters.ema.access.OmmConsumerEvent;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;


class OmmConsumerClientImpl implements OmmConsumerClient
{
	@Override
	public void onRefreshMsg(RefreshMsg RefreshMsg,	OmmConsumerEvent consumerEvent) {}

	@Override
	public void onUpdateMsg(UpdateMsg UpdateMsg, OmmConsumerEvent consumerEvent) {}

	@Override
	public void onStatusMsg(StatusMsg StatusMsg, OmmConsumerEvent consumerEvent) {}

	@Override
	public void onGenericMsg(GenericMsg GenericMsg,	OmmConsumerEvent consumerEvent) {}

	@Override
	public void onAckMsg(AckMsg AckMsg, OmmConsumerEvent consumerEvent) {}

	@Override
	public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent) {}
}