/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Reactor;
using System.Collections.Generic;

namespace LSEG.Ema.Access
{
    internal class ItemWatchList
    {
        private ItemCallbackClient<IOmmProviderClient> m_ItemCallbackClient;
        private LinkedList<IProviderItem> m_ItemList;
        private MonitorWriteLocker m_ItemListLock = new MonitorWriteLocker(new object());

        public ItemCallbackClient<IOmmProviderClient> ItemCallbackClient { get => m_ItemCallbackClient; }

        public ItemWatchList(ItemCallbackClient<IOmmProviderClient> itemCallbackClient)
        {
            this.m_ItemCallbackClient = itemCallbackClient;
            m_ItemList = new LinkedList<IProviderItem>();
        }

        public void AddItem(IProviderItem providerItem)
        {
            m_ItemListLock.Enter();

            providerItem.ItemListNode = m_ItemList.AddLast(providerItem);

            m_ItemListLock.Exit();
        }

        public void RemoveItem(IProviderItem providerItem)
        {
            m_ItemListLock.Enter();

            if (providerItem.ItemListNode != null)
            {
                m_ItemList.Remove(providerItem.ItemListNode);
                providerItem.ItemListNode = null;
            }

            m_ItemListLock.Exit();
        }

        public void ProcessChannelEvent(ReactorChannelEvent reactorChannelEvent)
        {
            switch(reactorChannelEvent.EventType)
            {
                case ReactorChannelEventType.CHANNEL_DOWN:
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                    NotifyClosedRecoverableStatusMessage();
                    break;
                default:
                    break;
            }
        }

        public void ProcessCloseLogin(ClientSession clientSession)
        {
            m_ItemListLock.Enter();

            try
            {
                foreach (IProviderItem providerItem in m_ItemList)
                {
                    if (ReferenceEquals(providerItem.ClientSession, clientSession))
                    {
                        providerItem.ScheduleItemClosedRecoverableStatus("channel is closed", true);
                    }
                }
            }
            finally
            {
                m_ItemListLock.Exit();
            }
        }

        public void ProcessServiceDelete(ClientSession? clientSession, int serviceId)
        {
            m_ItemListLock.Enter();

            try
            {
                foreach (IProviderItem providerItem in m_ItemList)
                {
                    if (providerItem.RequestWithService())
                    {
                        if (clientSession != null)
                        {
                            if (providerItem.ClientSession == clientSession && providerItem.ServiceId == serviceId)
                            {
                                providerItem.ScheduleItemClosedRecoverableStatus("service is deleted", true);
                            }
                        }
                        else if (providerItem.ServiceId == serviceId)
                        {
                            providerItem.ScheduleItemClosedRecoverableStatus("service is deleted", true);
                        }
                    }
                }
            }
            finally
            {
                m_ItemListLock.Exit();
            }
        }

        internal static IMsg? ProcessRwfMsg(IMsg eventMsg, IProviderItem providerItem)
        {
            switch(eventMsg.MsgClass)
            {
                case MsgClasses.REFRESH:
                    {
                        IRefreshMsg refreshMsg = (IRefreshMsg)eventMsg;

                        if(providerItem.ProcessInitialResp(refreshMsg) == false)
                        {
                            providerItem.ScheduleItemClosedRecoverableStatus("received response is mismatch with the" +
                                " initlal request", true);

                            return null;
                        }

                        if((eventMsg.Flags & RefreshMsgFlags.HAS_MSG_KEY) == 0)
                        {
                            IRefreshMsg modifyRefreshMsg = providerItem.ItemWatchList.ItemCallbackClient.RefreshMsg();

                            CodecReturnCode ret = refreshMsg.Copy(modifyRefreshMsg, CopyMsgFlags.ALL_FLAGS &
                                ~(CopyMsgFlags.KEY | CopyMsgFlags.MSG_BUFFER));

                            if( ret == CodecReturnCode.SUCCESS)
                            {
                                modifyRefreshMsg.ApplyHasMsgKey();
                                MsgKey msgKey = modifyRefreshMsg.MsgKey;

                                ret = providerItem.MsgKey.Copy(msgKey);

                                if (ret != CodecReturnCode.SUCCESS)
                                    return eventMsg;
                            }

                            return modifyRefreshMsg;
                        }

                        break;
                    }
                case MsgClasses.STATUS:
                    {
                        if ((eventMsg.Flags & StatusMsgFlags.HAS_MSG_KEY) == 0)
                        {
                            IStatusMsg statusMsg = (IStatusMsg)eventMsg;

                            IStatusMsg modifyStatusMsg = providerItem.ItemWatchList.ItemCallbackClient.StatusMsg();

                            CodecReturnCode ret = statusMsg.Copy(modifyStatusMsg, CopyMsgFlags.ALL_FLAGS &
                                ~(CopyMsgFlags.KEY | CopyMsgFlags.MSG_BUFFER));

                            if (ret == CodecReturnCode.SUCCESS)
                            {
                                modifyStatusMsg.ApplyHasMsgKey();
                                MsgKey msgKey = modifyStatusMsg.MsgKey;

                                ret = providerItem.MsgKey.Copy(msgKey);

                                if (ret != CodecReturnCode.SUCCESS)
                                    return eventMsg;
                            }

                            return modifyStatusMsg;
                        }
                        break;
                    }
                default:
                    return null;
            }

            return eventMsg;
        }

        private void NotifyClosedRecoverableStatusMessage()
        {
            m_ItemListLock.Enter();
            try
            {
                foreach (IProviderItem item in m_ItemList)
                {
                    item.ScheduleItemClosedRecoverableStatus("channel down", true);
                }
            }
            finally
            {
                m_ItemListLock.Exit();
            }
        }
    }

    internal class ItemTimeOut : ITimeoutClient
    {
        private IProviderItem m_ProviderItem;

        public ItemTimeOut(IProviderItem providerItem)
        {
            m_ProviderItem = providerItem;
        }

        public void HandleTimeoutEvent()
        {
            m_ProviderItem.SendCloseMsg();
            m_ProviderItem.ScheduleItemClosedRecoverableStatus("request is timeout", false);
        }
    }
}
