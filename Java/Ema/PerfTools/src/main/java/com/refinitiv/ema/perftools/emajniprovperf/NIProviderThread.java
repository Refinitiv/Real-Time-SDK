/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.emajniprovperf;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.perftools.common.ItemInfo;
import com.refinitiv.ema.perftools.common.*;

import static com.refinitiv.ema.perftools.common.LogFileHelper.initFile;
import static com.refinitiv.ema.perftools.common.LogFileHelper.writeFile;

public class NIProviderThread extends ProviderThread {
    private final NIProviderPerfConfig config;
    private NIProviderPerfClient client;


    public NIProviderThread(NIProviderPerfConfig config, XmlMsgData msgData) {
        super(config, msgData);
        this.config = config;
    }

    @Override
    public void initialize(int threadIndex) {
        super.initialize(threadIndex);
        writeFile(statsFile, "UTC, Requests received, Images sent, Updates sent, CPU usage (%%), Memory (MB)\n");
    }

    @Override
    protected void initializeOmmProvider() {
        this.client = new NIProviderPerfClient();
        final OmmNiProviderConfig providerConfig = EmaFactory.createOmmNiProviderConfig()
                .providerName(providerConfigName)
                .tunnelingKeyStoreFile(this.config.keyFile())
                .tunnelingKeyStorePasswd(this.config.keyPassw())
                .tunnelingSecurityProvider(this.config.securityProvider());
        if (this.baseConfig.useUserDispatch()) {
            providerConfig.operationModel(OmmIProviderConfig.OperationModel.USER_DISPATCH);
        }

        this.provider = EmaFactory.createOmmProvider(providerConfig, this.client); //Seems that we should create only one provider

        while (!client.isConnectionUp()) {
            try {
                Thread.sleep(200);
            } catch (InterruptedException e) {
                shutdown(true);
            }
        }

        initializeItemPublishing();
    }

    @Override
    protected void executeMsgBurst(long nextTime) throws OmmInvalidUsageException {
        if (client.isConnectionUp()) {
            try {
                if (!updates.isEmpty()) {
                    sendUpdateMessages();
                }
                do {
                    if (!this.refreshes.isEmpty()) {
                        sendRefreshMessages();
                    } else {
                        break;
                    }
                } while (currentTime() < nextTime);
            } catch (OmmInvalidUsageException e) {
                closeChannel();
            }
        }
    }

    @Override
    protected void handleRefresh(ItemInfo itemInfo) {
        this.updates.add(itemInfo);
    }

    @Override
    protected void prepareRefreshMsg(ItemInfo itemInfo) {
        super.prepareRefreshMsg(itemInfo);
        if (this.config.useServiceId()) {
            this.refreshMsg.serviceId(itemInfo.attributes().serviceId());
        }
        this.refreshMsg.serviceName(itemInfo.attributes().serviceName());
    }

    @Override
    protected boolean sendMsg(Msg msg, ItemInfo itemInfo) {
        try {
            submitMsg(msg, itemInfo);
        } catch (OmmInvalidUsageException e) {
            if (e.errorCode() == OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT) {
                return false;
            }
            throw e;
        }
        return true;
    }

    private void initializeItemPublishing() {
        if (config.itemPublishCount() > 0) {
            int itemListUniqueIndex;
            int itemListCount;
            int itemListCountRemainder;

            /* Figure out which items this thread should publish. */

            /* Calculate unique index for each thread. Each thread publishes a common
             * and unique set of items. Unique index is so each thread has a unique
             * index into the shared item list. Unique items for this provider are after
             * the items assigned to providers with a lower index.
             */
            itemListUniqueIndex = config.commonItemCount();
            itemListUniqueIndex += ((config.itemPublishCount() - config.commonItemCount())
                    / config.threadCount()) * (threadIndex);

            itemListCount = config.itemPublishCount() / config.threadCount();
            itemListCountRemainder = config.itemPublishCount() % config.threadCount();

            if (threadIndex < itemListCountRemainder) {
                /* This provider publishes an extra item */
                itemListCount += 1;

                /* Shift index by one for each provider before this one, since they publish extra items too. */
                itemListUniqueIndex += threadIndex;
            } else {
                /* Shift index by one for each provider that publishes an extra item. */
                itemListUniqueIndex += itemListCountRemainder;
            }

            if (addPublishingItems(itemListUniqueIndex, itemListCount - config.commonItemCount()) != PerfToolsReturnCodes.SUCCESS) {
                closeChannel();
            } else {
                System.out.println("Created publishing list");
            }
        }
    }

    private int addPublishingItems(int itemListUniqueIndex, int uniqueItemCount) {
        final String xmlItemInfoFile = this.config.itemFilename();
        final int commonItemCount = this.config.commonItemCount();
        XmlItemInfoList xmlItemInfoList = new XmlItemInfoList(itemListUniqueIndex + uniqueItemCount);
        if (xmlItemInfoList.parseFile(xmlItemInfoFile) == PerfToolsReturnCodes.FAILURE) {
            System.err.println("Failed to load item list from file '" + xmlItemInfoFile + "'.");
            return PerfToolsReturnCodes.FAILURE;
        }

        int itemListIndex = 0;
        for (int i = 0; i < commonItemCount + uniqueItemCount; ++i) {
            if (itemListIndex == commonItemCount && itemListIndex < itemListUniqueIndex) {
                itemListIndex = itemListUniqueIndex;
            }
            ItemInfo itemInfo = itemInfoPool().getFromPool();
            if (itemInfo == null) {
                return PerfToolsReturnCodes.FAILURE;
            }
            itemInfo.attributes().name(xmlItemInfoList.itemInfoList()[itemListIndex].name());
            itemInfo.attributes().domainType(xmlItemInfoList.itemInfoList()[itemListIndex].domainType());
            itemInfo.attributes().serviceId(config.serviceId());
            itemInfo.attributes().serviceName(config.serviceName());
            itemInfo.itemFlags(ItemFlags.IS_STREAMING_REQ);
            itemInfo.itemHandle(++itemListIndex);
            this.refreshes().add(itemInfo);
        }

        return PerfToolsReturnCodes.SUCCESS;
    }

    public void closeChannel() {
        provider.submit(EmaFactory.createStatusMsg().state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT,
                OmmState.StatusCode.NONE, "Stream Closed"), providerIndex);
        shutdown(true);
        shutdownAck(true);
    }
}
