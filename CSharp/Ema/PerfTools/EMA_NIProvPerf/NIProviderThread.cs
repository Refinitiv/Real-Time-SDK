/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.PerfTools.Common;
using static LSEG.Ema.Access.OmmNiProviderConfig;
using static LSEG.Ema.Access.OmmProviderConfig;

namespace LSEG.Ema.PerfTools.EMA_NiProvPerf
{
    public class NIProviderThread : ProviderThread
    {
        private EmaNiProviderPerfConfig config;
        private NiProviderPerfClient? client;

        public override ProviderRoleEnum ProviderRoleKind => ProviderRoleEnum.NON_INTERACTIVE;

        public NIProviderThread(EmaNiProviderPerfConfig config, XmlMsgData msgData) : base(config, msgData)
        {
            this.config = config;
        }

        public override void Initialize(int threadIndex)
        {
            base.Initialize(threadIndex);
            statsFile.WriteFile("UTC, Requests received, Images sent, Updates sent, CPU usage (%), Memory (MB)\n");
        }

        protected override void InitializeOmmProvider()
        {
            client = new NiProviderPerfClient(this);
            OmmNiProviderConfig providerConfig = new OmmNiProviderConfig().ProviderName(providerConfigName!);
            if (baseConfig.UseUserDispatch)
            {
                providerConfig.OperationModel(OperationModelMode.USER_DISPATCH);
            }

            provider = new OmmProvider(providerConfig, client); //Seems that we should create only one provider

            while (!client.IsConnectionUp())
            {
                try
                {
                    Thread.Sleep(200);
                }
                catch
                {
                    shutdown = true;
                }
            }

            InitializeItemPublishing();
        }

        public override void ExecuteMsgBurst(long nextTime)
        {
            if (client!.IsConnectionUp())
            {
                try
                {
                    if (updates.Count() != 0)
                    {
                        SendUpdateMessages();
                    }
                    do
                    {
                        if (refreshes.Count() != 0)
                        {
                            SendRefreshMessages();
                        }
                        else
                        {
                            break;
                        }
                    } while ((long)GetTime.GetMicroseconds() < nextTime);
                }
                catch (OmmInvalidUsageException)
                {
                    CloseChannel();
                    throw;
                }
            }
        }

        protected override void HandleRefresh(ItemInfo itemInfo)
        {
            updates.Add(itemInfo);
        }

        protected override void PrepareRefreshMsg(ItemInfo itemInfo)
        {
            base.PrepareRefreshMsg(itemInfo);
            if (config.UseServiceId)
            {
                refreshMsg.ServiceId(itemInfo.Attributes.ServiceId);
            }
            refreshMsg.ServiceName(itemInfo.Attributes.ServiceName!);
        }

        protected override bool SendMsg(Msg msg, ItemInfo itemInfo)
        {
            try
            {
                SubmitMsg(msg, itemInfo);
            }
            catch (OmmInvalidUsageException e)
            {
                if (e.ErrorCode == OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT)
                {
                    return false;
                }
                throw;
            }
            return true;
        }

        private void InitializeItemPublishing()
        {
            if (config.ItemRequestCount > 0)
            {
                int itemListUniqueIndex;
                int itemListCount;
                int itemListCountRemainder;

                /* Figure out which items this thread should publish. */

                /* Calculate unique index for each thread. Each thread publishes a common
                 * and unique set of items. Unique index is so each thread has a unique
                 * index into the shared item list. Unique items for this provider are after
                 * the items assigned to providers with a lower index.
                 */
                itemListUniqueIndex = config.CommonItemCount;
                itemListUniqueIndex += ((config.ItemRequestCount - config.CommonItemCount) / config.ThreadCount) * (threadIndex);

                itemListCount = config.ItemRequestCount / config.ThreadCount;
                itemListCountRemainder = config.ItemRequestCount % config.ThreadCount;

                if (threadIndex < itemListCountRemainder)
                {
                    /* This provider publishes an extra item */
                    itemListCount += 1;

                    /* Shift index by one for each provider before this one, since they publish extra items too. */
                    itemListUniqueIndex += threadIndex;
                }
                else
                {
                    /* Shift index by one for each provider that publishes an extra item. */
                    itemListUniqueIndex += itemListCountRemainder;
                }

                if (AddPublishingItems(itemListUniqueIndex, itemListCount - config.CommonItemCount) != (int)PerfToolsReturnCode.SUCCESS)
                {
                    CloseChannel();
                }
                else
                {
                    Console.WriteLine("Created publishing list");
                }
            }
        }

        private PerfToolsReturnCode AddPublishingItems(int itemListUniqueIndex, int uniqueItemCount)
        {
            string xmlItemInfoFile = config.ItemFilename!;
            int commonItemCount = config.CommonItemCount;
            XmlItemInfoList xmlItemInfoList = new XmlItemInfoList(itemListUniqueIndex + uniqueItemCount);
            if (xmlItemInfoList.ParseFile(xmlItemInfoFile) == PerfToolsReturnCode.FAILURE)
            {
                Console.Error.WriteLine("Failed to load item list from file '" + xmlItemInfoFile + "'.");
                return PerfToolsReturnCode.FAILURE;
            }

            int itemListIndex = 0;
            for (int i = 0; i < commonItemCount + uniqueItemCount; ++i)
            {
                if (itemListIndex == commonItemCount && itemListIndex < itemListUniqueIndex)
                {
                    itemListIndex = itemListUniqueIndex;
                }
                ItemInfo? itemInfo = itemInfoPool.GetFromPool();
                if (itemInfo == null)
                {
                    return PerfToolsReturnCode.FAILURE;
                }
                itemInfo.Attributes.Name = xmlItemInfoList.ItemInfoList[itemListIndex].Name;
                itemInfo.Attributes.DomainType = xmlItemInfoList.ItemInfoList[itemListIndex].DomainType;
                itemInfo.Attributes.ServiceId = config.ServiceId;
                itemInfo.Attributes.ServiceName = config.ServiceName;
                itemInfo.ItemFlags = (int)ItemFlags.IS_STREAMING_REQ;
                itemInfo.ItemHandle = ++itemListIndex;
                refreshes.Enqueue(itemInfo);
            }

            return PerfToolsReturnCode.SUCCESS;
        }

        public void CloseChannel()
        {
            provider.Submit(new StatusMsg().State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Stream Closed"), providerIndex);
            shutdown = true;
        }
    }
}
