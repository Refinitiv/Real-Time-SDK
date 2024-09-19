/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022-2023 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Transports;
using System.Collections;
using System.Collections.Generic;

namespace LSEG.Eta.Example.Common
{
    public class ItemRequestInfoList : IEnumerable<ItemRequestInfo>
    {
        private static readonly int MAX_ITEM_REQ_INFO_LIST_SIZE = ProviderDirectoryHandler.OPEN_LIMIT * ProviderSession.NUM_CLIENT_SESSIONS;
        private List<ItemRequestInfo> m_ItemRequestInfoList = new List<ItemRequestInfo>(MAX_ITEM_REQ_INFO_LIST_SIZE);

        /// <summary>
        /// Instantiates a new item request info list.
        /// </summary>
        public ItemRequestInfoList()
        {
            for (int i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
            {
                m_ItemRequestInfoList.Add(new ItemRequestInfo());
            }
        }

        /// <summary>
        /// Finds an item request info associated with a channel and stream.
        /// </summary>
        /// <param name="chnl">The channel</param>
        /// <param name="streamId">The stream ID</param>
        /// <returns>The item request info</returns>
        public ItemRequestInfo? Get(IChannel chnl, int streamId)
        {
            foreach (ItemRequestInfo itemReqInfo in m_ItemRequestInfoList)
            {
                if (itemReqInfo.Channel == chnl && itemReqInfo.StreamId == streamId && itemReqInfo.IsInUse)
                    return itemReqInfo;
            }
            return null;
        }

        /// <summary>
        /// Initializes item request information list.
        /// </summary>
        public void Init()
        {
            foreach (ItemRequestInfo itemRequestInfo in m_ItemRequestInfoList)
            {
                itemRequestInfo.Clear();
            }
        }

        public IEnumerator<ItemRequestInfo> GetEnumerator()
        {
            return m_ItemRequestInfoList.GetEnumerator();
        }

        IEnumerator IEnumerable.GetEnumerator()
        {
            return GetEnumerator();
        }
    }
}
