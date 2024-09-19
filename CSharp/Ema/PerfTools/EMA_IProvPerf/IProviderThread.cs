/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.PerfTools.Common;
using LSEG.Ema.Rdm;
using System.Collections.Concurrent;
using static LSEG.Ema.Access.OmmProviderConfig;
using static LSEG.Ema.PerfTools.Common.ItemEncoder;

namespace LSEG.Ema.PerfTools.EMA_IProvPerf
{
    public class IProviderThread : ProviderThread
    {
        private EmaIProvPerfConfig config;
        private AutoResetEvent connectionCondition;

        private LatencyRandomArray genMsgLatencyRandomArray;
        internal ConcurrentDictionary<long, ItemInfo> itemHandles;

        private ItemWatchlist generics;
        internal HashSet<ItemInfo> closedItems;
        internal HashSet<long> closedClients;

        private GenericMsg genericMsg;

        private ProviderPerfClient? client;
        private bool connected;

        public override ProviderRoleEnum ProviderRoleKind => ProviderRoleEnum.INTERACTIVE;

        public IProviderThread(EmaIProvPerfConfig config, XmlMsgData msgData) : base(config, msgData)
        {
            this.config = config;
            genMsgLatencyRandomArray = new LatencyRandomArray();
            itemHandles = new ConcurrentDictionary<long, ItemInfo>(3, MSG_STORE_CAPACITY);
            generics = new ItemWatchlist(MSG_STORE_CAPACITY);
            closedItems = new HashSet<ItemInfo>(MSG_STORE_CAPACITY);
            closedClients = new HashSet<long>(100);
            genericMsg = new GenericMsg();
            connectionCondition = new(false);
        }

        public override void Initialize(int threadIndex)
        {
            base.Initialize(threadIndex);
            statsFile.WriteFile("UTC, Requests received, Images sent, Updates sent, Posts reflected, GenMsgs sent, GenMsg Latencies sent, GenMsgs received, GenMsg Latencies received, GenMsg Latency avg (usec), GenMsg Latency std dev (usec), GenMsg Latency max (usec), GenMsg Latency min (usec), CPU usage (%), Memory (MB)\n");
            if (config.genMsgsPerSec != 0 && config.latencyGenMsgRate > 0)
            {
                randomArrayOptions.TotalMsgsPerSec = config.genMsgsPerSec;
                randomArrayOptions.LatencyMsgsPerSec = config.latencyGenMsgRate;

                if (genMsgLatencyRandomArray.Create(randomArrayOptions) != PerfToolsReturnCode.SUCCESS)
                {
                    Console.Error.WriteLine("Error initializing application: Failed to create generic messages latency random array");
                    Environment.Exit(-1);
                }
            }

            // Open latency file if configured.
            if (config.LogLatencyToFile())
            {
                LatencyFile = LogFileHelper.InitFile(config.latencyFilename + providerIndex + ".csv");
                LatencyFile.WriteFile("Message type, Send time, Receive time, Latency (usec)\n");
            }
        }

        protected override void InitializeOmmProvider()
        {
            client = new ProviderPerfClient(config, this, clientLock);

            OmmIProviderConfig providerConfig = new OmmIProviderConfig()
                .ProviderName(providerConfigName!)
                .ServerCertificate(config.CertificateFilename!)
                .ServerPrivateKey(config.PrivateKeyFilename!);
            if (config.UseUserDispatch)
            {
                providerConfig.OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH);
            }

            provider = new OmmProvider(providerConfig, this.client); //Seems that we should create only one provider

            try
            {
                if (config.UseUserDispatch)
                {
                    while (!(connected || shutdown))
                    {
                        provider.Dispatch(1000);
                    }
                }
                else
                {
                    var conditionIsSet = false;
                    while (!(conditionIsSet || shutdown))
                    {
                        conditionIsSet = connectionCondition.WaitOne(1000);
                    }
                }
            }
            catch (Exception)
            {
                shutdown = true;
            }
        }

        public override void ExecuteMsgBurst(long nextTime)
        {
            if (closedItems.Count != 0)
            {
                ProcessClosedHandles();
            }
            if (closedClients.Count != 0)
            {
                ProcessClosedClients();
            }
            if (updates.Count() != 0)
            {
                SendUpdateMessages();
            }
            if (generics.Count() != 0)
            {
                SendGenericMessages();
            }
            do
            {
                if (refreshes.Count != 0)
                {
                    SendRefreshMessages();
                }
                else
                {
                    break;
                }
            } while (GetTime.GetMicroseconds() < nextTime);
        }

        protected override void HandleRefresh(ItemInfo itemInfo)
        {
            if ((itemInfo.ItemFlags & (int)ItemFlags.IS_STREAMING_REQ) != 0)
            {
                updates.Add(itemInfo);
                generics.Add(itemInfo);
            }
        }

        protected override bool SendMsg(Msg msg, ItemInfo itemInfo)
        {
            try
            {
                if (IsActiveStream(itemInfo))
                {
                    SubmitMsg(msg, itemInfo);
                    return true;
                }
            }
            catch(OmmInvalidHandleException)
            {
                closedClients.Add(itemInfo.ClientHandle);
                ProcessClosedClients();
                return false;
            }
            catch (OmmInvalidUsageException e)
            {
                if (e.ErrorCode == OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT)
                {
                    closedClients.Add(itemInfo.ClientHandle);
                    ProcessClosedClients();
                    return false;
                }
                throw;
            }
            return false;
        }

        internal void NotifyOnConnected()
        {
            connected = true;
            if (!config.UseUserDispatch)
            {
                connectionCondition.Set();
            }
        }

        internal ItemInfo CreateItemInfoFromMarketPriceRequest(RequestMsg reqMsg, long itemHandle, long clientHandle)
        {
            var flags = ItemFlags.IS_SOLICITED;
            if (reqMsg.InterestAfterRefresh())
            {
                flags |= ItemFlags.IS_STREAMING_REQ;
            }
            if (reqMsg.PrivateStream())
            {
                flags |= ItemFlags.IS_PRIVATE;
            }

            ItemInfo itemInfo = itemInfoPool.GetFromPool()!;
            itemInfo.ItemId = reqMsg.StreamId();
            itemInfo.ItemFlags = (int)flags;
            itemInfo.ClientHandle = clientHandle;
            itemInfo.ItemHandle = itemHandle;
            itemInfo.Active = true;
            itemInfo.Attributes.DomainType = reqMsg.DomainType();
            if (reqMsg.HasName && reqMsg.HasNameType)
            {
                itemInfo.Attributes.Name = reqMsg.Name();
                itemInfo.Attributes.NameType = reqMsg.NameType();
            }

            if (reqMsg.HasServiceId)
            {
                itemInfo.Attributes.ServiceId = reqMsg.ServiceId();
            }

            if (reqMsg.HasServiceName)
            {
                itemInfo.Attributes.ServiceName = reqMsg.ServiceName();
            }

            refreshes.Enqueue(itemInfo);

            return itemInfo;
        }

        private bool IsActiveStream(ItemInfo itemInfo)
        {
            Monitor.Enter(clientLock);
            try
            {
                return itemHandles.TryGetValue(itemInfo.ItemHandle, out var foundItemInfo)
                        && foundItemInfo.Active
                        && !closedItems.Contains(itemInfo)
                        && !closedClients.Contains(itemInfo.ClientHandle);
            }
            finally
            {
                Monitor.Exit(clientLock);
            }
        }

        internal void SendGenericMessages()
        {
            //Determine generic messages to send out. Spread the remainder out over the first ticks
            int genMsgsLeft = config.genMsgsPerTick;
            int genMsgsPerTickRemainder = config.genMsgsPerTickRemainder;
            if (genMsgsPerTickRemainder > currentTicks)
            {
                ++genMsgsLeft;
            }

            int latencyGenMsgNumber = (config.latencyGenMsgRate > 0) ? genMsgLatencyRandomArray.Next() : -1;
            bool setLatency = false;

            try
            {
                for (; genMsgsLeft > 0; --genMsgsLeft)
                {
                    ItemInfo? itemInfo = generics.GetNext();

                    if (itemInfo != null && IsActiveStream(itemInfo))
                    {
                        // When appropriate, provide a latency timestamp for the generic messages.
                        genericMsg.Clear();
                        genericMsg.DomainType(itemInfo.Attributes.DomainType);

                        switch (itemInfo.Attributes.DomainType)
                        {
                            case EmaRdm.MMT_MARKET_PRICE:
                                ItemData itemData = itemInfo.ItemData;
                                int index = itemData.NextIndex;
                                if (itemData.NextIndex == xmlMsgData.UpdateCount)
                                {
                                    index = 0;
                                }
                                setLatency = config.latencyGenMsgRate == BaseProviderPerfConfig.ALWAYS_SEND_LATENCY_GENMSG || latencyGenMsgNumber == (genMsgsLeft - 1);
                                var payload = CreateMarketPricePayload(xmlMsgData.MpGenMsgs![index], TIM_TRK_3_FID, setLatency);
                                if (payload != null)
                                {
                                    genericMsg.Payload(payload);
                                }
                                itemData.NextIndex = ++index;
                                break;
                            default:
                                return;
                        }

                        if (SendMsg(genericMsg, itemInfo))
                        {
                            if (setLatency)
                            {
                                ProviderThreadStats.LatencyGenMsgSentCount.Increment();
                            }
                            ProviderThreadStats.GenMsgSentCount.Increment();
                        }
                    }
                }
            }
            catch (OmmInvalidUsageException e)
            {
                if (e.ErrorCode == OmmInvalidUsageException.ErrorCodes.NO_BUFFERS)
                {
                    ProviderThreadStats.OutOfBuffersCount.Add(genMsgsLeft);
                    return;
                }
                throw;
            }


            if (++currentTicks > config.TicksPerSec)
            {
                currentTicks = 0;
            }
        }

        public void ProcessClose(long handle)
        {
            itemHandles.TryGetValue(handle, out ItemInfo? itemInfo);
            if (itemInfo != null && itemInfo.Active)
            {
                closedItems.Add(itemInfo);
                itemInfo.Active = false;
            }
        }

        private void ProcessClosedHandles()
        {
            Monitor.Enter(clientLock);
            try
            {
                foreach (var item in closedItems)
                {
                    itemHandles.Remove(item.ItemHandle, out _);
                    itemInfoPool.ReturnToPool(item);
                }

                refreshes.RemoveIf(closedItems.Contains);
                generics.RemoveIf(closedItems.Contains);
                updates.RemoveIf(closedItems.Contains);

                closedItems.Clear();
            }
            finally
            {
                Monitor.Exit(clientLock);
            }
        }

        private void ProcessClosedClients()
        {
            Monitor.Enter(clientLock);
            try
            {
                if(refreshes.Count > 0)
                    refreshes.RemoveIf(x => closedClients.Contains(x.ClientHandle));

                if(updates.Count() > 0)
                    updates.RemoveIf(x => closedClients.Contains(x.ClientHandle));

                if(generics.Count() > 0)
                    generics.RemoveIf(x => closedClients.Contains(x.ClientHandle));

                var itemHandleEntriesToRemove = itemHandles
                    .Where(x => closedClients.Contains(x.Value.ClientHandle))
                    .ToList();
                foreach (var itemEntry in itemHandleEntriesToRemove)
                {
                    itemHandles.Remove(itemEntry.Key, out _);
                    itemInfoPool.ReturnToPool(itemEntry.Value);
                }

                closedClients.Clear();
            }
            finally
            {
                Monitor.Exit(clientLock);
            }
        }
    }
}
