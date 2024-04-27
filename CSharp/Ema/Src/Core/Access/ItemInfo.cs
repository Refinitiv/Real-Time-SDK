/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Common;
using System;
using System.Collections.Concurrent;
using System.Collections.Generic;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access
{
    /// <summary>
    /// This class is used to represents individual streams
    /// </summary>
    internal class ItemInfo : VaNode
    {
        ItemInfoFlags m_Flags;
        Buffer m_ItemGroup = new Buffer();
        long m_Handle;
        int m_StreamId;
        MsgKey m_MsgKey;
        Dictionary<long, int>? m_postIdsCount;

        [System.Flags]
        internal enum ItemInfoFlags
        {
            NONE = 0,
            STREAMING = 0x01,
            PRIVATE_STREAM = 0x02,
            ITEM_GROUP = 0x04
        }

        public ItemInfo(OmmServerBaseImpl ommServerBaseImpl)
        {
            SentRefresh = false;
            m_Flags = ItemInfoFlags.NONE;
            ClientSession = null;
            m_Handle = ommServerBaseImpl.ServerPool.GetItemHandle();
            m_MsgKey = new MsgKey();
        }

        public void RequestMsg(IRequestMsg requestMsg)
        {
            requestMsg.MsgKey.Copy(m_MsgKey);

            m_StreamId = requestMsg.StreamId;
            DomainType = requestMsg.DomainType;

            if (requestMsg.CheckStreaming())
            {
                m_Flags |= ItemInfoFlags.STREAMING;
            }

            if (requestMsg.CheckPrivateStream())
            {
                m_Flags |= ItemInfoFlags.PRIVATE_STREAM;
            }
        }

        public MsgKey MsgKey {get => m_MsgKey; }

        public long Handle { get => m_Handle; }

        public int StreamId { get => m_StreamId; set { m_StreamId = value; } }

        public bool SentRefresh { get; set; }

        public int ServiceId { get => m_MsgKey.ServiceId; }

        public int DomainType { get; set; }

        public ItemInfoFlags Flags { get => m_Flags; set { m_Flags = value; } }

        public bool IsPrivateStream { get => (m_Flags & ItemInfoFlags.PRIVATE_STREAM) != 0; }

        public bool IsStreaming { get => (m_Flags & ItemInfoFlags.STREAMING) != 0; }

        public bool HasItemGroup { get => (m_Flags & ItemInfoFlags.ITEM_GROUP) != 0; }

        public Buffer ItemGroup
        {
            get
            {
                return m_ItemGroup;
            }

            set
            {
                m_Flags |= ItemInfoFlags.ITEM_GROUP;
                m_ItemGroup = value;
            }
        }

        public ClientSession? ClientSession { get; set; }

        public void AddPostId(long postId)
        {
            if (m_postIdsCount == null)
            {
                m_postIdsCount = new Dictionary<long, int>();
            }
            if (!m_postIdsCount.TryGetValue(postId, out int oldCount))
            {
                m_postIdsCount[postId] = 1;
            }
            else
            {
                m_postIdsCount[postId] = oldCount + 1;
            }
        }

        public bool RemovePostId(long id)
        {
            if (m_postIdsCount == null)
            {
                return false;
            }

            if (!m_postIdsCount.TryGetValue(id, out var count))
            {
                return false;
            }
            else if (count > 1)
            {
                m_postIdsCount[id] = count - 1;
            }
            else
            {
                m_postIdsCount.Remove(id, out _);
            }
            return true;
        }

        public void Clear()
        {
            SentRefresh = false;
            m_Flags = ItemInfoFlags.NONE;
            ClientSession = null;
            m_StreamId = 0;
            DomainType = 0;
            m_MsgKey.Clear();
        }

        public override int GetHashCode()
        {
            if (m_MsgKey.CheckHasName())
            {
                return m_MsgKey.Name.GetHashCode();
            }
            else
            {
                return 0;
            }
        }

        public override bool Equals(object? other)
        {
            if (other == this)
                return true;

            ItemInfo? itemInfo = other as ItemInfo;

            if (itemInfo == null)
                return false;

            if (DomainType != itemInfo.DomainType)
                return false;

            if (IsPrivateStream != itemInfo.IsPrivateStream)
                return false;

            return m_MsgKey.Equals(itemInfo.m_MsgKey);
        }

        public override void ReturnToPool()
        {
            if (m_postIdsCount != null)
            {
                m_postIdsCount.Clear();
            }

            ClientSession = null;
            base.ReturnToPool();
        }
    }
}
