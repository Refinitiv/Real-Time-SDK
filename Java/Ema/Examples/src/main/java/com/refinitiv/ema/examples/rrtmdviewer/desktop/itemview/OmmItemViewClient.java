/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.rrtmdviewer.desktop.itemview;

import com.refinitiv.ema.access.*;

public class OmmItemViewClient implements OmmConsumerClient {

    @Override
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmConsumerEvent consumerEvent) {
        handleMsg(EmaFactory.createRefreshMsg(refreshMsg), consumerEvent);
    }

    @Override
    public void onUpdateMsg(UpdateMsg updateMsg, OmmConsumerEvent consumerEvent) {
        handleMsg(EmaFactory.createUpdateMsg(updateMsg), consumerEvent);
    }

    @Override
    public void onStatusMsg(StatusMsg statusMsg, OmmConsumerEvent consumerEvent) {
        handleMsg(EmaFactory.createStatusMsg(statusMsg), consumerEvent);
    }

    @Override
    public void onGenericMsg(GenericMsg genericMsg, OmmConsumerEvent consumerEvent) {
    }

    @Override
    public void onAckMsg(AckMsg ackMsg, OmmConsumerEvent consumerEvent) {
    }

    @Override
    public void onAllMsg(Msg msg, OmmConsumerEvent consumerEvent) {
    }

    private void handleMsg(Msg msg, OmmConsumerEvent consumerEvent) {
        ItemRequestModel request = (ItemRequestModel) consumerEvent.closure();
        final ItemNotificationModel itemNotificationModel = new ItemNotificationModel();
        itemNotificationModel.setHandle(consumerEvent.handle());
        itemNotificationModel.setParentHandle(consumerEvent.parentHandle());
        itemNotificationModel.setMsg(msg);
        itemNotificationModel.setRequest(request);
        try {
            request.getTabViewModel().getNotificationQueue().put(itemNotificationModel);
            request.getTabViewModel().incrementCounter();
        } catch (InterruptedException e) {
        }
    }
}
