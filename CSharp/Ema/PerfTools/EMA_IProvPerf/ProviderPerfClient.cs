/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.PerfTools.Common;
using LSEG.Ema.Rdm;
using System.Collections.Concurrent;
using static LSEG.Ema.PerfTools.Common.ItemEncoder;

namespace LSEG.Ema.PerfTools.EMA_IProvPerf
{
    public class ProviderPerfClient : IOmmProviderClient
    {
        private BaseProviderPerfConfig config;
        private IProviderThread thread;
        private RefreshMsg refresh;
        private ConcurrentDictionary<long, ItemInfo> itemHandles;
        private HashSet<long> clients;
        private object clientLock;

        public ProviderPerfClient(BaseProviderPerfConfig config, IProviderThread thread, object clientLock)
        {
            this.config = config;
            this.thread = thread;
            refresh = new RefreshMsg();
            itemHandles = thread.itemHandles;
            clients = new HashSet<long>(20);
            this.clientLock = clientLock;
        }

        private void HandleClose(int domainType, IOmmProviderEvent providerEvent)
        {
            Monitor.Enter(clientLock);
            try
            {
                switch (domainType)
                {
                    case EmaRdm.MMT_LOGIN:
                        if (clients.Contains(providerEvent.ClientHandle))
                        {
                            clients.Remove(providerEvent.ClientHandle);
                            thread.closedClients.Add(providerEvent.ClientHandle);
                        }
                        break;
                    case EmaRdm.MMT_MARKET_PRICE:
                        if (itemHandles.ContainsKey(providerEvent.Handle))
                        {
                            thread.ProcessClose(providerEvent.Handle);
                        }
                        break;
                }
            }
            finally
            {
                Monitor.Exit(clientLock);
            }
        }

        public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent)
        {
            thread.ProviderThreadStats.StatusCount.Increment();
            if (providerEvent.ChannelInformation().ChannelState != ChannelState.ACTIVE)
            {
                HandleClose(statusMsg.DomainType(), providerEvent);
            }
        }

        public void OnGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent)
        {
            ProviderThreadStats stats = thread.ProviderThreadStats;
            stats.GenMsgRecvCount.Increment();
            if (stats.FirstGenMsgRecvTime == ProviderThreadStats.NOT_DEFINED)
            {
                stats.FirstGenMsgRecvTime = (long)GetTime.GetMicroseconds();
            }
            long timeTrackerValue = ProviderThreadStats.NOT_DEFINED;
            if (genericMsg.Payload().DataType == DataType.DataTypes.FIELD_LIST)
            {
                foreach (FieldEntry fieldEntry in genericMsg.Payload().FieldList())
                {
                    if (fieldEntry.FieldId == TIM_TRK_3_FID)
                    {
                        timeTrackerValue = (long)fieldEntry.UIntValue();
                        break;
                    }
                }
            }
            if (timeTrackerValue != ProviderThreadStats.NOT_DEFINED)
            {
                stats.TimeRecordSubmit(timeTrackerValue, (long)GetTime.GetMicroseconds(), 1);
            }
        }

        public void OnPostMsg(PostMsg postMsg, IOmmProviderEvent providerEvent)
        {
            thread.ProviderThreadStats.PostCount.Increment();

            UpdateMsg updateMsg = postMsg.Payload().UpdateMsg();
            updateMsg.DomainType(postMsg.DomainType());
            updateMsg.PublisherId(postMsg.PublisherIdUserId(), postMsg.PublisherIdUserAddress());
            if (postMsg.HasSeqNum)
            {
                updateMsg.SeqNum(postMsg.SeqNum());
            }
            providerEvent.Provider.Submit(updateMsg, providerEvent.Handle);
        }

        public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
        {
            switch (reqMsg.DomainType())
            {
                case EmaRdm.MMT_LOGIN:
                    refresh.Clear();
                    ElementList attributes = new ElementList();
                    attributes
                        .AddAscii(EmaRdm.ENAME_APP_ID, EmaIProvPerfConfig.APPLICATION_ID)
                        .AddAscii(EmaRdm.ENAME_APP_NAME, EmaIProvPerfConfig.APPLICATION_NAME)
                        .AddAscii(EmaRdm.ENAME_POSITION, this.config.Position)
                        .AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 0)
                        .AddUInt(EmaRdm.ENAME_SUPPORT_BATCH, 1)
                        .AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);
                    refresh
                            .Name(reqMsg.Name())
                            .NameType(EmaRdm.USER_NAME)
                            .DomainType(reqMsg.DomainType())
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted by host " + providerEvent.ChannelInformation().Hostname)
                            .Solicited(true)
                            .Complete(true)
                            .Attrib(attributes.Complete());

                    Monitor.Enter(clientLock);
                    try
                    {
                        clients.Add(providerEvent.ClientHandle);
                        providerEvent.Provider.Submit(refresh, providerEvent.Handle);
                        thread.NotifyOnConnected();
                    }
                    finally
                    {
                        Monitor.Exit(clientLock);
                    }
                    break;
                case EmaRdm.MMT_MARKET_PRICE:
                    thread.ProviderThreadStats.RequestCount.Increment();
                    var itemInfo = thread.CreateItemInfoFromMarketPriceRequest(reqMsg, providerEvent.Handle, providerEvent.ClientHandle);
                    itemHandles.TryAdd(itemInfo.ItemHandle, itemInfo);

                    break;
                default:
                    providerEvent.Provider.Submit(
                           new StatusMsg()
                                    .Name(reqMsg.Name())
                                    .ServiceName(reqMsg.ServiceName())
                                    .DomainType(reqMsg.DomainType())
                                    .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
                            providerEvent.Handle);
                    break;
            }
        }

        public void OnClose(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
        {
            thread.ProviderThreadStats.CloseCount.Increment();
            HandleClose(reqMsg.DomainType(), providerEvent);
        }
    }
}
