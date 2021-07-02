package com.refinitiv.ema.perftools.emajprovperf;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.perftools.common.ItemInfo;
import com.refinitiv.ema.perftools.common.ItemWatchList;
import com.refinitiv.ema.perftools.common.*;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.Map;
import java.util.*;
import java.util.concurrent.ConcurrentHashMap;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

import static com.refinitiv.ema.perftools.common.LogFileHelper.initFile;
import static com.refinitiv.ema.perftools.common.LogFileHelper.writeFile;

public class IProviderThread extends ProviderThread {

    private final IProviderPerfConfig config;
    private final ReentrantLock clientLock;
    private final Condition connectionCondition;

    private final LatencyRandomArray genMsgLatencyRandomArray;
    private final Map<Long, ItemInfo> itemHandles;

    private final ItemWatchList generics;
    private final Set<ItemInfo> closedItems;
    private final Set<Long> closedClients;

    private final GenericMsg genericMsg;

    private LogFileInfo latencyFile;
    private ProviderPerfClient client;
    private boolean connected;

    public IProviderThread(BaseProviderPerfConfig baseConfig, XmlMsgData msgData) {
        super(baseConfig, msgData);
        this.config = (IProviderPerfConfig) baseConfig;
        this.genMsgLatencyRandomArray = new LatencyRandomArray();
        this.itemHandles = new ConcurrentHashMap<>(MSG_STORE_CAPACITY);
        this.generics = new ItemWatchList(MSG_STORE_CAPACITY);
        this.closedItems = new HashSet<>(MSG_STORE_CAPACITY);
        this.closedClients = new HashSet<>(100);
        this.genericMsg = EmaFactory.createGenericMsg();
        this.clientLock = new ReentrantLock();
        this.connectionCondition = clientLock.newCondition();
    }

    @Override
    public void initialize(int threadIndex) {
        super.initialize(threadIndex);
        writeFile(statsFile, "UTC, Requests received, Images sent, Updates sent, Posts reflected, GenMsgs sent, GenMsg Latencies sent, GenMsgs received, GenMsg Latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%%), Memory (MB)\n");
        if (config.genMsgsPerSec() != 0 && config.latencyGenMsgRate() > 0) {
            randomArrayOptions.totalMsgsPerSec(config.genMsgsPerSec());
            randomArrayOptions.latencyMsgsPerSec(config.latencyGenMsgRate());
            randomArrayOptions.ticksPerSec(config.ticksPerSec());
            randomArrayOptions.arrayCount(LATENCY_RANDOM_ARRAY_SET_COUNT);

            if (genMsgLatencyRandomArray.create(randomArrayOptions) != PerfToolsReturnCodes.SUCCESS) {
                System.err.println("Error initializing application: Failed to create generic messages latency random array");
                System.exit(-1);
            }
        }

        // Open latency file if configured.
        if (config.logLatencyToFile()) {
            this.latencyFile = initFile(config.latencyFilename() + providerIndex + ".csv");
            writeFile(latencyFile, "Message type, Send time, Receive time, Latency (usec)\n");
        }
    }

    @Override
    protected void initializeOmmProvider() {
        this.client = new ProviderPerfClient(this.config, this.clientLock);
        final OmmIProviderConfig providerConfig = EmaFactory.createOmmIProviderConfig()
                .providerName(providerConfigName);
        if (this.config.useUserDispatch()) {
            providerConfig.operationModel(OmmIProviderConfig.OperationModel.USER_DISPATCH);
        }

        this.provider = EmaFactory.createOmmProvider(providerConfig, this.client); //Seems that we should create only one provider

        if (this.config.useUserDispatch()) {
            while (!connected) {
                try {
                    this.provider.dispatch(1000);
                } catch (OmmException e) {
                    shutdown = true;
                }
            }
        } else {
            try {
                clientLock.lock();
                while (!connected) {
                    try {
                        System.out.printf("ProviderThread_%d run. Waiting initilization... \n", this.providerIndex);
                        connectionCondition.await(1, TimeUnit.SECONDS);
                    } catch (InterruptedException e) {
                        shutdown = true;
                    }
                }
            } finally {
                clientLock.unlock();
            }
        }
    }

    @Override
    protected void executeMsgBurst(long nextTime) throws OmmInvalidUsageException {
        if (!this.updates.isEmpty()) {
            sendUpdateMessages();
        }
        if (!this.generics.isEmpty()) {
            sendGenericMessages();
        }
        if (!this.closedItems.isEmpty()) {
            processClosedHandles();
        }
        if (!this.closedClients.isEmpty()) {
            processClosedClients();
        }
        if (!this.refreshes.isEmpty()) {
            sendRefreshMessages();
        }
    }

    @Override
    protected void handleRefresh(ItemInfo itemInfo) {
        if ((itemInfo.itemFlags() & ItemFlags.IS_STREAMING_REQ) != 0) {
            updates.add(itemInfo);
            generics.add(itemInfo);
        }
    }

    @Override
    protected boolean sendMsg(Msg msg, ItemInfo itemInfo) {
        try {
            if (isActiveStream(itemInfo)) {
                submitMsg(msg, itemInfo.itemHandle());
                return true;
            }
        } catch (OmmInvalidUsageException e) {
            if (e.errorCode() == OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT) {
                closedClients.add(itemInfo.clientHandle());
                this.processClosedHandles();
                return false;
            }
            throw e;
        }
        return false;
    }

    private void sendGenericMessages() {
        //Determine generic messages to send out. Spread the remainder out over the first ticks
        int genMsgsLeft = config.genMsgsPerTick();
        int genMsgsPerTickRemainder = config.genMsgsPerTickRemainder();
        if (genMsgsPerTickRemainder > currentTicks) {
            ++genMsgsLeft;
        }

        int latencyGenMsgNumber = (config.latencyGenMsgRate() > 0) ? genMsgLatencyRandomArray.next() : -1;
        boolean setLatency = false;

        try {
            for (; genMsgsLeft > 0; --genMsgsLeft) {
                long latencyStartTime;
                ItemInfo itemInfo = generics.getNext();

                if (this.isActiveStream(itemInfo)) {
                    // When appropriate, provide a latency timestamp for the generic messages.
                    genericMsg.clear();
                    genericMsg.domainType(itemInfo.attributes().domainType());

                    switch (itemInfo.attributes().domainType()) {
                        case EmaRdm.MMT_MARKET_PRICE:
                            ItemData itemData = (ItemData) itemInfo.itemData();
                            int index = itemData.getNextIndex();
                            if (itemData.getNextIndex() == xmlMsgData.marketPriceUpdateMsgCount()) {
                                index = 0;
                            }
                            setLatency = config.latencyGenMsgRate() == BaseProviderPerfConfig.ALWAYS_SEND_LATENCY_GENMSG || latencyGenMsgNumber == (genMsgsLeft - 1);
                            genericMsg.payload(createMarketPricePayload(xmlMsgData.marketPriceGenMsgs()[index], TIM_TRK_3_FID, setLatency));
                            itemData.setNextIndex(++index);
                            break;
                        default:
                            return;
                    }

                    if (sendMsg(genericMsg, itemInfo)) {
                        if (setLatency) {
                            providerThreadStats.latencyGenMsgSentCount().increment();
                        }
                        providerThreadStats.genMsgSentCount().increment();
                    }
                }
            }
        } catch (OmmInvalidUsageException e) {
            if (e.errorCode() == OmmInvalidUsageException.ErrorCode.NO_BUFFERS) {
                this.providerThreadStats.outOfBuffersCount().add(genMsgsLeft);
                return;
            }
            throw e;
        }


        if (++currentTicks > this.config.ticksPerSec()) {
            currentTicks = 0;
        }
    }

    @Override
    protected void submitMsg(Msg msg, Long handle) {
        if (msg.dataType() == DataType.DataTypes.GENERIC_MSG) {
            this.provider.submit(genericMsg, handle);
        } else {
            super.submitMsg(msg, handle);
        }
    }

    private boolean isActiveStream(ItemInfo itemInfo) {
        try {
            clientLock.lock();
            return itemHandles.containsKey(itemInfo.itemHandle())
                    && itemHandles.get(itemInfo.itemHandle()).active()
                    && !closedClients.contains(itemInfo.clientHandle());
        } finally {
            clientLock.unlock();
        }
    }

    public void processClose(Long handle) {
        ItemInfo itemInfo = this.itemHandles.get(handle);
        if (Objects.nonNull(itemInfo) && itemInfo.active()) {
            closedItems.add(itemInfo);
            itemInfo.active(false);
        }
    }

    private void processClosedHandles() {
        try {
            clientLock.lock();
            this.itemHandles.entrySet().removeIf(e -> closedItems.contains(e.getValue()));
            this.refreshes.removeIf(closedItems::contains);
            this.generics.removeItems(this.closedItems);
            this.updates.removeItems(this.closedItems);
            this.itemInfoPool().returnToPool(this.closedItems);
            this.closedItems.clear();
        } finally {
            clientLock.unlock();
        }
    }

    private void processClosedClients() {
        try {
            clientLock.lock();
            refreshes.removeIf((e) -> closedClients.contains(e.clientHandle()));
            updates.removeItemsForClients(closedClients);
            generics.removeItemsForClients(closedClients);
            Iterator<Map.Entry<Long, ItemInfo>> entries = itemHandles.entrySet().iterator();
            while (entries.hasNext()) {
                Map.Entry<Long, ItemInfo> infoEntry = entries.next();
                if (closedClients.contains(infoEntry.getValue().clientHandle())) {
                    entries.remove();
                    itemInfoPool().returnToPool(infoEntry.getValue());
                }
            }
            closedClients.clear();
        } finally {
            clientLock.unlock();
        }
    }


    public void notifyOnConnected() {
        this.connected = true;
        if (clientLock.isLocked()) {
            connectionCondition.signalAll();
        }
    }

    public Map<Long, ItemInfo> itemHandles() {
        return itemHandles;
    }

    public Set<Long> closedClients() {
        return closedClients;
    }

    public LogFileInfo latencyFile() {
        return latencyFile;
    }

    @Override
    public void clear() {
        super.clear();
        if (latencyFile.supportedWriting()) {
            latencyFile.writer().close();
        }
    }
}
