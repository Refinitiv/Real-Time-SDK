/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

using LSEG.Ema.Rdm;
using LSEG.Eta.ValueAdd.Common;
using LSEG.Eta.ValueAdd.Reactor;

using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    internal class ClientSession : VaNode
    {
        public long ClientHandle;
        internal ReactorChannel? m_ReactorChannel;
        internal Dictionary<long, ItemInfo> ItemInfoByStreamIdMap;
        internal Dictionary<long, Dictionary<Buffer, List<ItemInfo>>> ServiceGroupIdToItemInfoMap;
        internal HashSet<ItemInfo>? ItemInfoByItemInfoSet = null;
        internal bool IsLogin;
        internal OmmServerBaseImpl ServerBaseImpl;
        internal bool RemovingInCloseAll;
        public bool IsADHSession = false;
        internal long LoginHandle;
        public int MajorVersion { get; private set; }
        public int MinorVersion { get; private set; }
        public LinkedListNode<ReactorChannel>? Node { get; internal set; }
        public bool IsActiveClientSession { get; private set; } = false;

        public ClientSession(OmmServerBaseImpl ommServerBaseImpl)
        {
            ServerBaseImpl = ommServerBaseImpl;

            int itemCountHint = Utilities.Convert_ulong_int(ommServerBaseImpl.ConfigImpl.IProviderConfig.ItemCountHint);

            ItemInfoByStreamIdMap = new Dictionary<long, ItemInfo>(itemCountHint);
            ServiceGroupIdToItemInfoMap = new Dictionary<long, Dictionary<Buffer, List<ItemInfo>>>();

            if (!ommServerBaseImpl.ConfigImpl.IProviderConfig.AcceptMessageSameKeyButDiffStream)
            {
                ItemInfoByItemInfoSet = new HashSet<ItemInfo>(itemCountHint);
            }

            ClientHandle = ommServerBaseImpl.ServerPool.GetClientHandle();
            m_ReactorChannel = null;
            IsLogin = false;
            RemovingInCloseAll = false;
        }

        internal ReactorChannel Channel()
        {
            return m_ReactorChannel!;
        }

        internal void Channel(ReactorChannel rsslReactorChannel)
        {
            m_ReactorChannel = rsslReactorChannel;

            MajorVersion = rsslReactorChannel.MajorVersion;
            MinorVersion = rsslReactorChannel.MinorVersion;

            IsActiveClientSession = true;
        }

        internal void AddItemInfo(ItemInfo itemInfo)
        {
            ItemInfoByStreamIdMap[itemInfo.StreamId] = itemInfo;
        }

        internal void RemoveItemInfo(ItemInfo itemInfo)
        {
            if (RemovingInCloseAll == false)
            {
                ItemInfoByStreamIdMap.Remove(itemInfo.StreamId);
                if (ItemInfoByItemInfoSet != null)
                {
                    if (itemInfo.DomainType > EmaRdm.MMT_DICTIONARY)
                    {
                        ItemInfoByItemInfoSet.Remove(itemInfo);
                    }
                }
            }
        }

        internal ItemInfo? GetItemInfo(long streamId)
        {
            ServerBaseImpl.GetUserLocker().Enter();
            ItemInfoByStreamIdMap.TryGetValue(streamId, out ItemInfo? itemInfo);
            ServerBaseImpl.GetUserLocker().Exit();

            return itemInfo;
        }

        internal bool CheckingExistingReq(ItemInfo itemInfo)
        {
            if (ItemInfoByItemInfoSet == null)
            {
                return false;
            }

            bool found = ItemInfoByItemInfoSet.Contains(itemInfo);
            ServerBaseImpl.GetUserLocker().Enter();
            if (!found)
            {
                ItemInfoByItemInfoSet.Add(itemInfo);
            }
            ServerBaseImpl.GetUserLocker().Exit();

            return found;
        }

        internal ICollection<ItemInfo> ItemInfoList()
        {
            return ItemInfoByStreamIdMap.Values;
        }

        internal void CloseAllItemInfo()
        {
            if (ServerBaseImpl != null)
            {
                RemovingInCloseAll = true;

                var iter = ItemInfoByStreamIdMap.Values.GetEnumerator();

                ItemInfo itemInfo;

                while (iter.MoveNext())
                {
                    itemInfo = iter.Current;

                    switch (itemInfo.DomainType)
                    {
                        case EmaRdm.MMT_DICTIONARY:
                            ServerBaseImpl.DictionaryHandler.ItemInfoList.Remove(itemInfo);
                            break;
                        case EmaRdm.MMT_DIRECTORY:
                            ServerBaseImpl.DirectoryHandler.ItemInfoList.Remove(itemInfo);
                            break;
                        case EmaRdm.MMT_LOGIN:
                            ServerBaseImpl.LoginHandler.ItemInfoList.Remove(itemInfo);
                            LoginHandle = 0;
                            break;
                        default:
                            break;
                    }

                    ServerBaseImpl.RemoveItemInfo(itemInfo, false);
                }

                ItemInfoByStreamIdMap.Clear();

                var groupIdToItemInfoIt = ServiceGroupIdToItemInfoMap.Values.GetEnumerator();

                while (groupIdToItemInfoIt.MoveNext())
                {
                    Dictionary<Buffer, List<ItemInfo>> groupIdToItemInfo = groupIdToItemInfoIt.Current;
                    var itemInfoListIt = groupIdToItemInfo.Values.GetEnumerator();

                    while (itemInfoListIt.MoveNext())
                    {
                        itemInfoListIt.Current.Clear();
                    }

                    groupIdToItemInfo.Clear();
                }

                ServiceGroupIdToItemInfoMap.Clear();

                if (ItemInfoByItemInfoSet != null)
                {
                    ItemInfoByItemInfoSet.Clear();
                }

                RemovingInCloseAll = false;
            }

            IsActiveClientSession = false;
        }

        public void SetLoginHandle(long loginHandle)
        {
            LoginHandle = loginHandle;
        }

        public void ResetLoginHandle()
        {
            LoginHandle = 0;
        }

        internal void Clear()
        {
            LoginHandle = 0;
            ItemInfoByStreamIdMap.Clear();
            ServiceGroupIdToItemInfoMap.Clear();

            if (ItemInfoByItemInfoSet != null)
            {
                ItemInfoByItemInfoSet.Clear();
            }

            m_ReactorChannel = null;
            IsLogin = false;
            RemovingInCloseAll = false;

            IsActiveClientSession = false;
        }
        
        public override void ReturnToPool()
        {
            m_ReactorChannel = null;
            base.ReturnToPool();
        }

        public override string ToString()
        {
            return "\tClient handle = " + ClientHandle;
        }
    }
}
