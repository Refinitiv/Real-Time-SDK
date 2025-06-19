/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.emajprovperf;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.perftools.common.ItemInfo;
import com.refinitiv.ema.perftools.common.*;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.HashSet;
import java.util.Map;
import java.util.Set;
import java.util.concurrent.locks.ReentrantLock;

public class ProviderPerfClient implements OmmProviderClient {

    private final BaseProviderPerfConfig config;
    private final IProviderThread thread;
    private final RefreshMsg refresh;
    private final Map<Long, ItemInfo> itemHandles;
    private final Set<Long> clients;
    private final ReentrantLock clientLock;

    public ProviderPerfClient(BaseProviderPerfConfig config, ReentrantLock clientLock) {
        this.config = config;
        this.thread = (IProviderThread) Thread.currentThread();
        this.refresh = EmaFactory.createRefreshMsg();
        this.itemHandles = this.thread.itemHandles();
        this.clients = new HashSet<>(20);
        this.clientLock = clientLock;
    }

    @Override
    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent providerEvent) {

    }

    @Override
    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent providerEvent) {
        this.thread.providerThreadStats().statusCount().increment();
        if (providerEvent.channelInformation().channelState() != ChannelInformation.ChannelState.ACTIVE) {
            this.handleClose(statusMsg.domainType(), providerEvent);
        }
    }

    @Override
    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent) {
        ProviderThreadStats stats = this.thread.providerThreadStats();
        stats.genMsgRecvCount().increment();
        if (stats.firstGenMsgRecvTime() == ProviderThreadStats.NOT_DEFINED) {
            stats.firstGenMsgRecvTime(System.nanoTime());
        }
        long timeTrackerValue = ProviderThreadStats.NOT_DEFINED;
        if (genericMsg.payload().dataType() == DataType.DataTypes.FIELD_LIST) {
            for (FieldEntry fieldEntry : genericMsg.payload().fieldList()) {
                if (fieldEntry.fieldId() == ProviderThread.TIM_TRK_3_FID) {
                    timeTrackerValue = fieldEntry.uintValue();
                    break;
                }
            }
        }
        if (timeTrackerValue != ProviderThreadStats.NOT_DEFINED) {
            stats.timeRecordSubmit(timeTrackerValue, System.nanoTime() / 1000, 1);
        }
    }

    @Override
    public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent) {
        this.thread.providerThreadStats().postCount().increment();

        final UpdateMsg updateMsg = postMsg.payload().updateMsg();
        updateMsg.domainType(postMsg.domainType());
        updateMsg.publisherId(postMsg.publisherIdUserId(), postMsg.publisherIdUserAddress());
        if (postMsg.hasSeqNum()) {
            updateMsg.seqNum(postMsg.seqNum());
        }
        providerEvent.provider().submit(updateMsg, providerEvent.handle());
    }

    @Override
    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent) {
        switch (reqMsg.domainType()) {
            case EmaRdm.MMT_LOGIN:
                refresh.clear();
                ElementList attributes = EmaFactory.createElementList();
                attributes.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_ID, IProviderPerfConfig.APPLICATION_ID));
                attributes.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_APP_NAME, IProviderPerfConfig.APPLICATION_NAME));
                attributes.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_POSITION, this.config.position()));
                attributes.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SINGLE_OPEN, 0));
                attributes.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_BATCH, 1));
                attributes.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SUPPORT_POST, 1));
                refresh
                        .name(reqMsg.name())
                        .nameType(EmaRdm.USER_NAME)
                        .domainType(reqMsg.domainType())
                        .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted by host " + providerEvent.channelInformation().hostname())
                        .solicited(true)
                        .complete(true)
                        .attrib(attributes);
                try {
                    clientLock.lock();
                    this.clients.add(providerEvent.clientHandle());
                    providerEvent.provider().submit(refresh, providerEvent.handle());
                    this.thread.notifyOnConnected();
                } finally {
                    clientLock.unlock();
                }
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                this.thread.providerThreadStats().requestCount().increment();
                int flags = ItemFlags.IS_SOLICITED;
                if (reqMsg.interestAfterRefresh()) {
                    flags |= ItemFlags.IS_STREAMING_REQ;
                }
                if (reqMsg.privateStream()) {
                    flags |= ItemFlags.IS_PRIVATE;
                }
                final ItemInfo itemInfo = thread.itemInfoPool().getFromPool();
                itemInfo.itemId(reqMsg.streamId());
                itemInfo.itemFlags(flags);
                itemInfo.clientHandle(providerEvent.clientHandle());
                itemInfo.itemHandle(providerEvent.handle());
                itemInfo.active(true);
                itemInfo.attributes().domainType(reqMsg.domainType());
                if (reqMsg.hasName() && reqMsg.hasNameType()) {
                    itemInfo.attributes().name(reqMsg.name());
                    itemInfo.attributes().nameType(reqMsg.nameType());
                }

                if (reqMsg.hasServiceId()) {
                    itemInfo.attributes().serviceId(reqMsg.serviceId());
                }

                if (reqMsg.hasServiceName()) {
                    itemInfo.attributes().serviceName(reqMsg.serviceName());
                }

                this.itemHandles.put(providerEvent.handle(), itemInfo);
                thread.refreshes().add(itemInfo);
                break;
            default:
                providerEvent.provider().submit(
                        EmaFactory.createStatusMsg()
                                .name(reqMsg.name())
                                .serviceName(reqMsg.serviceName())
                                .domainType(reqMsg.domainType())
                                .state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_FOUND, "Item not found"),
                        providerEvent.handle());
                break;
        }
    }

    @Override
    public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent) {

    }

    @Override
    public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent) {
        this.thread.providerThreadStats().closeCount().increment();
        handleClose(reqMsg.domainType(), providerEvent);
    }

    @Override
    public void onAllMsg(Msg msg, OmmProviderEvent providerEvent) {

    }

    private void handleClose(int domainType, OmmProviderEvent providerEvent) {
        try {
            clientLock.lock();
            switch (domainType) {
                case EmaRdm.MMT_LOGIN:
                    if (clients.contains(providerEvent.clientHandle())) {
                        clients.remove(providerEvent.clientHandle());
                        thread.closedClients().add(providerEvent.clientHandle());
                    }
                    break;
                case EmaRdm.MMT_MARKET_PRICE:
                    if (itemHandles.containsKey(providerEvent.handle())) {
                        thread.processClose(providerEvent.handle());
                    }
                    break;
            }
        } finally {
            clientLock.unlock();
        }
    }
}
