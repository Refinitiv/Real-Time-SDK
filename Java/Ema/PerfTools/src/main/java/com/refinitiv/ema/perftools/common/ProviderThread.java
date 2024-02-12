/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022,2024 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.EmaRdm;

import java.util.HashMap;
import java.util.Queue;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.locks.Condition;
import java.util.concurrent.locks.ReentrantLock;

import static com.refinitiv.ema.perftools.common.LogFileHelper.initFile;

public abstract class ProviderThread extends Thread {

    public static final int MSG_STORE_CAPACITY = 1000000;
    public static final int TIM_TRK_3_FID = 3904; // Field TIM_TRK_3 is used to send generic msg latency.
    protected static final int LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
    private static final int TIME_MULTIPLIER = 1000;
    private static final int TIM_TRK_1_FID = 3902; // Field TIM_TRK_1 is used to send update latency.
    private static final int TIM_TRK_2_FID = 3903; // Field TIM_TRK_2 is used to send post latency.
    protected final ProviderThreadStats providerThreadStats; //common
    protected final BaseProviderPerfConfig baseConfig; //
    protected final XmlMsgData xmlMsgData;
    protected final LatencyRandomArray updateLatencyRandomArray;
    protected final LatencyRandomArrayOptions randomArrayOptions;
    protected final ItemWatchList updates;
    protected final Queue<ItemInfo> refreshes;
    protected final RefreshMsg refreshMsg;
    protected final UpdateMsg updateMsg;
    protected final FieldEntry zeroUpdateLatency;
    protected final FieldEntry zeroGenericLatency;
    protected final FieldEntry zeroPostLatency;
    protected final FieldList msgList;
    protected final FieldEntry latencyEntry;
    private final ItemInfoPool itemInfoPool;
    protected int threadIndex;
    protected int providerIndex;
    protected String providerConfigName;
    protected LogFileInfo statsFile;
    protected OmmProvider provider;
    protected long timePerTick;
    protected int currentTicks;
    protected boolean userDispatched;
    protected boolean shutdown;
    protected boolean shutdownAck;
    private ReentrantLock workerLock = new ReentrantLock();
    private Condition condition = workerLock.newCondition();
    private HashMap<Long, PackedMsg> packedMsgHandleList = new HashMap<>();
    private PackedMsg packedMsgPerHandle;

    public ProviderThread(BaseProviderPerfConfig baseConfig, XmlMsgData msgData) {
        this.baseConfig = baseConfig;
        this.xmlMsgData = msgData;
        this.providerThreadStats = new ProviderThreadStats();
        this.updateLatencyRandomArray = new LatencyRandomArray();
        this.randomArrayOptions = new LatencyRandomArrayOptions();
        this.itemInfoPool = new ItemInfoPool(MSG_STORE_CAPACITY);
        this.updates = new ItemWatchList(MSG_STORE_CAPACITY);
        this.refreshes = new ArrayBlockingQueue<>(MSG_STORE_CAPACITY);
        this.refreshMsg = EmaFactory.createRefreshMsg();
        this.updateMsg = EmaFactory.createUpdateMsg();

        this.zeroUpdateLatency = EmaFactory.createFieldEntry();
        this.zeroGenericLatency = EmaFactory.createFieldEntry();
        this.zeroPostLatency = EmaFactory.createFieldEntry();
        this.latencyEntry = EmaFactory.createFieldEntry();
        this.msgList = EmaFactory.createFieldList();
    }

    public void initialize(int threadIndex) {
        timePerTick = 1000000000 / baseConfig.ticksPerSec();

        if (baseConfig.updatesPerSec() != 0 && baseConfig.latencyUpdateRate() > 0) {
            randomArrayOptions.totalMsgsPerSec(baseConfig.updatesPerSec());
            randomArrayOptions.latencyMsgsPerSec(baseConfig.latencyUpdateRate());
            randomArrayOptions.ticksPerSec(baseConfig.ticksPerSec());
            randomArrayOptions.arrayCount(LATENCY_RANDOM_ARRAY_SET_COUNT);

            if (updateLatencyRandomArray.create(randomArrayOptions) != PerfToolsReturnCodes.SUCCESS) {
                System.err.println("Error initializing application: Failed to create updates latency random array");
                System.exit(-1);
            }
        }

        this.threadIndex = threadIndex;
        this.providerIndex = threadIndex + 1;
        this.providerConfigName = defineProviderName();

        // Open stats file.
        this.statsFile = initFile(baseConfig.statsFilename() + this.providerIndex + ".csv");
        this.zeroUpdateLatency.codeUInt(TIM_TRK_1_FID);
        this.zeroPostLatency.codeUInt(TIM_TRK_2_FID);
        this.zeroGenericLatency.codeUInt(TIM_TRK_3_FID);

        for (int i = 0; i < MSG_STORE_CAPACITY; i++) {
            this.itemInfoPool.returnToPool(new ItemInfo());
        }
    }

    @Override
    public void run() {
        initializeOmmProvider();
        long nextTime = currentTime() + timePerTick;
        try {
            workerLock.lock();
            while (!shutdown()) {
                if (currentTime() >= nextTime) {
                    nextTime += timePerTick;
                    sendMsgBurst(nextTime);

                    //One time per tick execute dispatching if dispatch time was no available
                    if (this.currentTicks == 0 && this.baseConfig.useUserDispatch()) {
                        if (!userDispatched) {
                            this.provider.dispatch(OmmProvider.DispatchTimeout.NO_WAIT);
                        }
                        userDispatched = false;
                    }
                } else {
                    dispatchApplication(nextTime);
                }
            }
        } catch (InterruptedException e) {
            System.out.printf("ProviderThread_%d. run(). InterruptedException.\n", providerIndex);
            shutdown(true);
        } catch (OmmException e) {
            System.out.printf("ProviderThread_%d. run(). OMMException: %s\n", providerIndex, e.getMessage());
            shutdown(true);
        } finally {
            workerLock.unlock();
            this.provider.uninitialize();
            shutdownAck(true);
        }
    }

    private void dispatchApplication(long nextTime) throws InterruptedException {
        long dispatchTime = nextTime - currentTime();
        if (dispatchTime > 0) {
            if (this.baseConfig.useUserDispatch()) {
                dispatchTime = Math.max((int) (dispatchTime / TIME_MULTIPLIER), OmmProvider.DispatchTimeout.NO_WAIT);
                this.provider.dispatch(dispatchTime);
                userDispatched = true;
            } else {
                condition.awaitNanos(dispatchTime);
            }
        }
    }

    protected abstract void initializeOmmProvider();

    /**
     * Send refreshes and updates to open channels.
     * If an operation on channel returns unrecoverable error,
     * the channel is closed.
     */
    private void sendMsgBurst(long nextTime) {
        executeMsgBurst(nextTime);
    }

    protected abstract void executeMsgBurst(long nextTime) throws OmmInvalidUsageException;

    protected final void sendRefreshMessages() {
        int refreshLeft = Math.min(refreshes.size(), this.baseConfig.refreshBurstSize);

        try {
            while (refreshLeft > 0) {
                final ItemInfo itemInfo = refreshes.poll();
                refreshMsg.clear();
                prepareRefreshMsg(itemInfo);
                switch (itemInfo.attributes().domainType()) {
                    case EmaRdm.MMT_MARKET_PRICE:
                        refreshMsg.payload(createMarketPricePayload(xmlMsgData.marketPriceRefreshMsg(), 0, false));
                        break;
                    default:
                        break;
                }

                refreshMsg.complete(true);

                if (sendMsg(refreshMsg, itemInfo)) {
                    providerThreadStats.itemRefreshCount().increment();
                    providerThreadStats.refreshCount().increment();
                    handleRefresh(itemInfo);
                }

                refreshLeft--;
            }
        } catch (OmmInvalidUsageException e) {
            if (e.errorCode() == OmmInvalidUsageException.ErrorCode.NO_BUFFERS) {
                this.providerThreadStats.outOfBuffersCount().add(refreshLeft);
                return;
            }
            throw e;
        }
    }

    protected void prepareRefreshMsg(ItemInfo itemInfo) {
        if (itemInfo.attributes().name() != null) {
            refreshMsg.name(itemInfo.attributes().name());
            if(itemInfo.attributes().hasNameType())
            {
            	refreshMsg.nameType(itemInfo.attributes().nameType());
            }
        }
        refreshMsg.domainType(itemInfo.attributes().domainType());
        refreshMsg.solicited((itemInfo.itemFlags() & ItemFlags.IS_SOLICITED) != 0);
        refreshMsg.clearCache((itemInfo.itemFlags() & ItemFlags.IS_SOLICITED) != 0);
        refreshMsg.privateStream((itemInfo.itemFlags() & ItemFlags.IS_PRIVATE) != 0);

        int streamState = (itemInfo.itemFlags() & ItemFlags.IS_STREAMING_REQ) != 0 ? OmmState.StreamState.OPEN : OmmState.StreamState.NON_STREAMING;
        refreshMsg.state(streamState, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed");
        refreshMsg.qos(OmmQos.Timeliness.REALTIME, OmmQos.Rate.TICK_BY_TICK);
    }

    protected abstract void handleRefresh(ItemInfo itemInfo);

    protected abstract boolean sendMsg(Msg msg, ItemInfo itemInfo);

    protected void submitMsg(Msg msg, ItemInfo itemInfo) {
    	Long itemHandle = itemInfo.itemHandle();
    	if (baseConfig.messagePackingCount > 1)
    	{
    		if (!packedMsgHandleList.containsKey(itemHandle))
    		{
    			if (packedMsgPerHandle == null)
    				packedMsgPerHandle = EmaFactory.createPackedMsg(provider);
    			if (provider.providerRole() == OmmProviderConfig.ProviderRole.NON_INTERACTIVE)
				{
    				packedMsgPerHandle.initBuffer(baseConfig.messagePackingBufferSize);
				}
    			else if (provider.providerRole() == OmmProviderConfig.ProviderRole.INTERACTIVE)
    			{
        			packedMsgPerHandle.initBuffer(itemInfo.clientHandle(), baseConfig.messagePackingBufferSize);
    			}
    			packedMsgHandleList.put(itemHandle, packedMsgPerHandle);
    		}
            switch (msg.dataType()) {
            case DataType.DataTypes.REFRESH_MSG:
                provider.submit(refreshMsg, itemHandle);
                break;
    		// Handle packing for update messages only
            case DataType.DataTypes.UPDATE_MSG:
            	PackedMsg packedMsg = packedMsgHandleList.get(itemHandle);
            	if (packedMsg == null)
            		return;	// Channel not up
            	try {
            		packedMsg.addMsg(updateMsg, itemHandle);
            		// Check if Packed Message is at packed message count
            		if (packedMsg.packedMsgCount() == this.baseConfig.messagePackingCount)
            		{
            			provider.submit(packedMsg);
            			this.providerThreadStats.updatePackedMsgCount().increment();
                		packedMsg.initBuffer(itemInfo.clientHandle(), baseConfig.messagePackingBufferSize);
            		}
            	}
            	catch (OmmInvalidUsageException excp)
            	{
            		if (excp.errorCode() == OmmInvalidUsageException.ErrorCode.PACKING_REMAINING_SIZE_TOO_SMALL)
            		{
            			if (packedMsg.packedMsgCount() > 0)
                		{
                			provider.submit(packedMsg);
                			this.providerThreadStats.updatePackedMsgCount().increment();
                			packedMsg.initBuffer(itemInfo.clientHandle(), baseConfig.messagePackingBufferSize);
                		}
                    	try {
                    		packedMsg.addMsg(updateMsg, itemHandle);
                    	}
                    	catch (OmmInvalidUsageException excp2)
                    	{
                    		if (excp.errorCode() == OmmInvalidUsageException.ErrorCode.PACKING_REMAINING_SIZE_TOO_SMALL)
                    		{
                    			// This packet cannot fit into the buffer, even though the buffer is empty
                    			provider.submit(updateMsg, itemHandle);
                    			this.providerThreadStats.updatePackedMsgCount().increment();
                    		}
                    	}
            		}
            	}
                break;
            }
    	}
    	else // Packing is not enabled
    	{
            switch (msg.dataType()) {
            case DataType.DataTypes.REFRESH_MSG:
                provider.submit(refreshMsg, itemHandle);
                break;
            case DataType.DataTypes.UPDATE_MSG:
                provider.submit(updateMsg, itemHandle);
                this.providerThreadStats.updatePackedMsgCount().add(1);
                break;
            }
    	}
    }

    protected FieldList createMarketPricePayload(MarketPriceMsg msg, int timeFieldId, boolean setLatency) {
        if (msg.fieldEntryCount() > 0) {
            msgList.clear();
            for (MarketField field : msg.fieldEntries()) {
                msgList.add(field.fieldEntry());
            }

            if (timeFieldId == 0) {
                msgList.add(zeroUpdateLatency);
                msgList.add(zeroPostLatency);
                msgList.add(zeroGenericLatency);
            } else if (setLatency) {
                latencyEntry.uintValue(timeFieldId, System.nanoTime() / TIME_MULTIPLIER);
                msgList.add(latencyEntry);
            }
            return msgList;
        }
        return null;
    }

    protected final void sendUpdateMessages() {
        int updatesLeft = baseConfig.updatesPerTick();
        int updatesPerTickReminder = baseConfig.updatesPerTickRemainder();
        if (updatesPerTickReminder > currentTicks) {
            ++updatesLeft;
        }
        int latencyUpdateNumber = baseConfig.latencyUpdateRate() > 0 ? updateLatencyRandomArray.next() : -1;
        boolean setLatency = false;
        try {
            for (; updatesLeft > 0; --updatesLeft) {
                final ItemInfo itemInfo = updates.getNext();
                updateMsg.clear();
                this.prepareUpdateMsg(itemInfo);

                switch (itemInfo.attributes().domainType()) {
                    case EmaRdm.MMT_MARKET_PRICE:
                        ItemData itemData = (ItemData) itemInfo.itemData();
                        int index = itemData.getNextIndex();
                        if (itemData.getNextIndex() == xmlMsgData.marketPriceUpdateMsgCount()) {
                            index = 0;
                        }
                        setLatency = baseConfig.latencyUpdateRate() == BaseProviderPerfConfig.ALWAYS_SEND_LATENCY_UPDATE || latencyUpdateNumber == (updatesLeft - 1);
                        updateMsg.payload(createMarketPricePayload(xmlMsgData.marketPriceUpdateMsgs()[index], TIM_TRK_1_FID, setLatency));
                        itemData.setNextIndex(++index);
                        break;
                    default:
                        return;
                }
                if (sendMsg(updateMsg, itemInfo)) {
                    this.providerThreadStats.updateCount().increment();
                }
            }

            if (++currentTicks > this.baseConfig.ticksPerSec()) {
                currentTicks = 0;
            }
        } catch (OmmInvalidUsageException e) {
            if (e.errorCode() == OmmInvalidUsageException.ErrorCode.NO_BUFFERS) {
                this.providerThreadStats.outOfBuffersCount().add(updatesLeft);
                return;
            }
            throw e;
        }
    }

    private String defineProviderName() {
        String providerConfigName = this.baseConfig.providerName();
        if (!this.baseConfig.useCustomName() || this.providerIndex > 1) {
            providerConfigName += this.providerIndex;
        }
        return providerConfigName;
    }

    protected void prepareUpdateMsg(ItemInfo itemInfo) {
        updateMsg.domainType(itemInfo.attributes().domainType());
    }

    public Queue<ItemInfo> refreshes() {
        return refreshes;
    }

    public long currentTime() {
        return System.nanoTime();
    }

    public boolean shutdown() {
        return this.shutdown;
    }

    public void shutdown(boolean shutdown) {
        this.shutdown = shutdown;
    }

    public boolean shutdownAck() {
        return this.shutdownAck;
    }

    public void shutdownAck(boolean shutdownAck) {
        this.shutdownAck = shutdownAck;
    }

    public ProviderThreadStats providerThreadStats() {
        return providerThreadStats;
    }

    public LogFileInfo statsFile() {
        return statsFile;
    }

    public ItemInfoPool itemInfoPool() {
        return itemInfoPool;
    }

    public void clear() {
        providerThreadStats.genMsgLatencyRecords().cleanup();
        if (statsFile().supportedWriting()) {
            statsFile().writer().close();
        }
    }
}
