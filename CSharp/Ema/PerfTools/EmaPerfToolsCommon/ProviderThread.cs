/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using static LSEG.Ema.Access.OmmProviderConfig;
using static LSEG.Ema.PerfTools.Common.ItemEncoder;

namespace LSEG.Ema.PerfTools.Common
{
    public abstract class ProviderThread
    {
        public const int MSG_STORE_CAPACITY = 1000000;
        protected const int LATENCY_RANDOM_ARRAY_SET_COUNT = 20;
        private const int TIME_MULTIPLIER = 1000;
        public ProviderThreadStats ProviderThreadStats; //common
        protected BaseProviderPerfConfig baseConfig;
        protected XmlMsgData xmlMsgData;
        internal LatencyRandomArray updateLatencyRandomArray;
        protected LatencyRandomArrayOptions randomArrayOptions;
        protected readonly object clientLock;
        protected ItemWatchlist updates;
        protected Queue<ItemInfo> refreshes;
        protected RefreshMsg refreshMsg;
        protected UpdateMsg updateMsg;
        protected FieldEntry? zeroUpdateLatency;
        protected FieldEntry? zeroGenericLatency;
        protected FieldEntry? zeroPostLatency;
        internal FieldList msgList;
        protected FieldEntry? latencyEntry;
        protected ItemInfoPool itemInfoPool;
        protected int threadIndex;
        protected int providerIndex;
        public string? providerConfigName;
        public LogFileInfo? statsFile;
        public OmmProvider provider;
        internal long timePerTick;
        protected int currentTicks;
        internal bool userDispatched;
        public bool shutdown;
        internal object workerLock = new object();
        internal ItemEncoder itemEncoder;
        internal PackedMsg packedMsg;

        public abstract ProviderRoleEnum ProviderRoleKind { get; } // we cannot use provider.ProviderRole because provider field is being initialized in parallel thread

        public LogFileInfo? LatencyFile { get; protected set; }

#pragma warning disable CS8618
        public ProviderThread(BaseProviderPerfConfig baseConfig, XmlMsgData msgData)
#pragma warning restore CS8618
        {
            this.baseConfig = baseConfig;
            xmlMsgData = msgData;
            ProviderThreadStats = new ProviderThreadStats();
            updateLatencyRandomArray = new LatencyRandomArray();
            randomArrayOptions = new LatencyRandomArrayOptions();
            itemInfoPool = new ItemInfoPool(MSG_STORE_CAPACITY, () => new ItemInfo());
            clientLock = new();
            updates = new ItemWatchlist(MSG_STORE_CAPACITY);
            refreshes = new Queue<ItemInfo>(MSG_STORE_CAPACITY);
            refreshMsg = new RefreshMsg();
            updateMsg = new UpdateMsg();
            itemEncoder = new ItemEncoder(msgData);
            msgList = new FieldList();
        }

        public virtual void Initialize(int threadIndex)
        {
            timePerTick = 1000000 / baseConfig.TicksPerSec; // time per tick is in microseconds

            if (baseConfig.UpdatesPerSec != 0 && baseConfig.LatencyUpdateRate > 0)
            {
                randomArrayOptions.TotalMsgsPerSec = baseConfig.UpdatesPerSec;
                randomArrayOptions.LatencyMsgsPerSec = baseConfig.LatencyUpdateRate;
                randomArrayOptions.TicksPerSec = baseConfig.TicksPerSec;
                randomArrayOptions.ArrayCount = LATENCY_RANDOM_ARRAY_SET_COUNT;

                if (updateLatencyRandomArray.Create(randomArrayOptions) != PerfToolsReturnCode.SUCCESS)
                {
                    Console.Error.WriteLine("Error initializing application: Failed to create updates latency random array");
                    Environment.Exit(-1);
                }
            }

            this.threadIndex = threadIndex;
            providerIndex = threadIndex + 1;
            providerConfigName = DefineProviderName();

            // Open stats file.
            statsFile = LogFileHelper.InitFile(baseConfig.StatsFilename + providerIndex + ".csv");
        }

        public void Run()
        {
            InitializeOmmProvider();
            long nextTime = (long)(GetTime.GetMicroseconds() + timePerTick);
            try
            {
                Monitor.Enter(workerLock);
                while (!shutdown)
                {
                    if (GetTime.GetMicroseconds() >= nextTime)
                    {
                        nextTime += timePerTick;
                        SendMsgBurst(nextTime);

                        //One time per tick dispatching if dispatch time was no available
                        if (currentTicks == 0 && baseConfig.UseUserDispatch)
                        {
                            if (!userDispatched)
                            {
                                provider.Dispatch(DispatchTimeout.NO_WAIT);
                            }
                            userDispatched = false;
                        }
                    }
                    else
                    {
                        DispatchApplication(nextTime);
                    }
                }
            }
            catch (OmmException e)
            {
                Console.WriteLine($"ProviderThread_{providerIndex}.Run(). OMMException: {e}\n");
                shutdown = true;
            }
            catch (Exception e)
            {
                Console.WriteLine($"ProviderThread_{providerIndex}.Run(). Exception: {e}\n");
                shutdown = true;
            }
            finally
            {
                Monitor.Exit(workerLock);
                provider.Uninitialize();
            }
        }

        private void DispatchApplication(long nextTime)
        {
            long dispatchTime = nextTime - (long)GetTime.GetMicroseconds();
            if (dispatchTime > 0)
            {
                if (baseConfig.UseUserDispatch)
                {
                    dispatchTime = Math.Max((int)(dispatchTime / TIME_MULTIPLIER), DispatchTimeout.NO_WAIT);
                    provider.Dispatch((int)dispatchTime);
                    userDispatched = true;
                }
                else
                {
                    int sleepTime = ((int)dispatchTime / 1000); // convert to millisecond
                    if(sleepTime > 0)
                        Thread.Sleep((int)dispatchTime);
                }
            }
        }

        protected abstract void InitializeOmmProvider();

        /// <summary>
        /// Send refreshes and updates to open channels. 
        /// If an operation on channel returns unrecoverable error, the channel is closed.
        /// </summary>
        /// <param name="nextTime"></param>
        private void SendMsgBurst(long nextTime)
        {
            ExecuteMsgBurst(nextTime);
        }

        public abstract void ExecuteMsgBurst(long nextTime);

        protected void SendRefreshMessages()
        {
            int refreshLeft = Math.Min(refreshes.Count, baseConfig.RefreshBurstSize);

            try
            {
                while (refreshLeft > 0)
                {
                    ItemInfo itemInfo = refreshes.Dequeue();

                    if(itemInfo == null)
                    {
                        return;
                    }

                    refreshMsg.Clear();
                    PrepareRefreshMsg(itemInfo);
                    switch (itemInfo.Attributes.DomainType)
                    {
                        case EmaRdm.MMT_MARKET_PRICE:
                            FieldList? fieldList = CreateMarketPricePayload(xmlMsgData.MarketPriceRefreshMsg!, 0, false);
                            if(fieldList != null)
                                refreshMsg.Payload(fieldList);
                            break;
                        default:
                            break;
                    }

                    refreshMsg.Complete(true);

                    if (SendMsg(refreshMsg, itemInfo))
                    {
                        ProviderThreadStats.ItemRefreshCount.Increment();
                        ProviderThreadStats.RefreshCount.Increment();
                        HandleRefresh(itemInfo);
                    }

                    refreshLeft--;
                }
            }
            catch (OmmInvalidUsageException e)
            {
                if (e.ErrorCode == OmmInvalidUsageException.ErrorCodes.NO_BUFFERS)
                {
                    ProviderThreadStats.OutOfBuffersCount.Add(refreshLeft);
                    return;
                }
                throw;
            }
        }

        protected virtual void PrepareRefreshMsg(ItemInfo itemInfo)
        {
            if (itemInfo.Attributes.Name != null)
            {
                refreshMsg.Name(itemInfo.Attributes.Name);
                if (itemInfo.Attributes.HasNameType)
                {
                    refreshMsg.NameType(itemInfo.Attributes.NameType);
                }
            }
            refreshMsg.DomainType(itemInfo.Attributes.DomainType);
            refreshMsg.Solicited((itemInfo.ItemFlags & (int)ItemFlags.IS_SOLICITED) != 0);
            refreshMsg.ClearCache((itemInfo.ItemFlags & (int)ItemFlags.IS_SOLICITED) != 0);
            refreshMsg.PrivateStream((itemInfo.ItemFlags & (int)ItemFlags.IS_PRIVATE) != 0);

            int streamState = (itemInfo.ItemFlags & (int)ItemFlags.IS_STREAMING_REQ) != 0 ? OmmState.StreamStates.OPEN : OmmState.StreamStates.NON_STREAMING;
            refreshMsg.State(streamState, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed");
            refreshMsg.Qos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK);
        }

        protected abstract void HandleRefresh(ItemInfo itemInfo);

        protected abstract bool SendMsg(Msg msg, ItemInfo itemInfo);

        protected void SubmitMsg(Msg msg, ItemInfo itemInfo)
        {
            long itemHandle = itemInfo.ItemHandle;

            if (baseConfig.MessagePackingCount > 1)
            {
                if (packedMsg == null)
                {
                    packedMsg = new (provider);

                    if(provider.ProviderRole == OmmProviderConfig.ProviderRoleEnum.NON_INTERACTIVE)
                    {
                        packedMsg.InitBuffer(baseConfig.MessagePackingBufferSize);
                    }
                    else
                    {
                        packedMsg.InitBuffer(itemInfo.ClientHandle, baseConfig.MessagePackingBufferSize);
                    }
                }

                switch (msg.DataType)
                {
                    case DataType.DataTypes.REFRESH_MSG:
                        provider.Submit(refreshMsg, itemHandle);
                        break;
                    case DataType.DataTypes.UPDATE_MSG:
                        packedMsg.AddMsg(updateMsg, itemHandle);

                        if (packedMsg.PackedMsgCount() == baseConfig.MessagePackingCount)
                        {
                            provider.Submit(packedMsg);

                            ProviderThreadStats.UpdatePackedMsgCount.Increment();

                            if (provider.ProviderRole == OmmProviderConfig.ProviderRoleEnum.NON_INTERACTIVE)
                            {
                                packedMsg.InitBuffer(baseConfig.MessagePackingBufferSize);
                            }
                            else
                            {
                                packedMsg.InitBuffer(itemInfo.ClientHandle, baseConfig.MessagePackingBufferSize);
                            }
                        }
                        break;
                }
            }
            else
            {
                switch (msg.DataType)
                {
                    case DataType.DataTypes.REFRESH_MSG:
                        provider.Submit(refreshMsg, itemHandle);
                        break;
                    case DataType.DataTypes.UPDATE_MSG:
                        provider.Submit(updateMsg, itemHandle);
                        break;
                }
            }
        }

        protected FieldList? CreateMarketPricePayload(MarketPriceMsg msg, int timeFieldId, bool setLatency)
        {
            if (msg.FieldEntryCount > 0)
            {
                msgList.Clear();
                itemEncoder.CreatePayload(msg, (long)GetTime.GetMicroseconds(), timeFieldId, msgList, setLatency);
                return msgList;
            }
            return null;
        }

        protected void SendUpdateMessages()
        {
            int updatesLeft = baseConfig.UpdatesPerTick;
            int updatesPerTickReminder = baseConfig.UpdatesPerTickRemainder;
            if (updatesPerTickReminder > currentTicks)
            {
                ++updatesLeft;
            }
            int latencyUpdateNumber = baseConfig.LatencyUpdateRate > 0 ? updateLatencyRandomArray.Next() : -1;
            bool setLatency;
            try
            {
                for (; updatesLeft > 0; --updatesLeft)
                {
                    ItemInfo? itemInfo = updates.GetNext();
                    if (itemInfo == null)
                    {
                        continue;
                    }
                    updateMsg.Clear();
                    PrepareUpdateMsg(itemInfo);

                    switch (itemInfo.Attributes.DomainType)
                    {
                        case EmaRdm.MMT_MARKET_PRICE:
                            ItemData itemData = itemInfo.ItemData;
                            int index = itemData.NextIndex;
                            if (itemData.NextIndex == xmlMsgData.UpdateCount)
                            {
                                index = 0;
                            }
                            setLatency = baseConfig.LatencyUpdateRate == BaseProviderPerfConfig.ALWAYS_SEND_LATENCY_UPDATE || latencyUpdateNumber == (updatesLeft - 1);
                            FieldList? fieldList = CreateMarketPricePayload(xmlMsgData.MpUpdateMsgs![index], TIM_TRK_1_FID, setLatency);
                            if (fieldList != null)
                                updateMsg.Payload(fieldList);
                            itemData.NextIndex = ++index;
                            break;
                        default:
                            return;
                    }
                    if (SendMsg(updateMsg, itemInfo))
                    {
                        ProviderThreadStats.UpdateCount.Increment();
                    }
                }

                if (++currentTicks > baseConfig.TicksPerSec)
                {
                    currentTicks = 0;
                }
            }
            catch (OmmInvalidUsageException e)
            {
                if (e.ErrorCode == OmmInvalidUsageException.ErrorCodes.NO_BUFFERS)
                {
                    ProviderThreadStats.OutOfBuffersCount.Add(updatesLeft);
                    return;
                }
                throw;
            }
        }

        private string DefineProviderName()
        {
            string providerConfigName = baseConfig.ProviderName!;
            if (!baseConfig.UseCustomName || providerIndex > 1)
            {
                providerConfigName += providerIndex;
            }
            return providerConfigName;
        }

        protected void PrepareUpdateMsg(ItemInfo itemInfo)
        {
            updateMsg.DomainType(itemInfo.Attributes.DomainType);
        }

        public void Clear()
        {
            ProviderThreadStats.GenMsgLatencyRecords.Cleanup();
            statsFile.Close();
            LatencyFile.Close();
        }
    }
}
