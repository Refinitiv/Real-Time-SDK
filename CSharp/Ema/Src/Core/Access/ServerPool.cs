/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.ValueAdd.Common;

namespace LSEG.Ema.Access
{
    internal class ServerPool
    {
        internal VaPool m_ClientSessionPool = new VaPool(false);
        internal VaPool m_ItemInfoPool = new VaPool(false);
        long m_ClientHandle = 0;
        long m_ItemHandle = 0;
        MonitorWriteLocker ClientHandlerLock { get; set; } = new MonitorWriteLocker(new object());
        OmmServerBaseImpl m_OmmServerBaseImpl;

        public ServerPool(OmmServerBaseImpl ommServerBaseImpl)
        {
            m_OmmServerBaseImpl = ommServerBaseImpl;
        }

        public void Initialize(uint initClientSession, uint initItemInfo)
        {
            for (uint index = 0; index < initClientSession; index++)
            {
                m_ClientSessionPool.Add(new ClientSession(m_OmmServerBaseImpl));
            }

            ulong numOfItemInfo = (ulong)initClientSession * initItemInfo;

            for (ulong index = 0; index < numOfItemInfo; index++)
            {
                m_ItemInfoPool.Add(new ItemInfo(m_OmmServerBaseImpl));
            }
        }

        public ClientSession GetClientSession(OmmServerBaseImpl serverBaseImpl)
        {
            ClientHandlerLock.Enter();
            ClientSession? clientSession = (ClientSession?)m_ClientSessionPool.Poll();

            try
            {
                if(clientSession == null)
                {
                    clientSession = new ClientSession(serverBaseImpl);
                    m_ClientSessionPool.UpdatePool(clientSession);
                }
                else
                {
                    clientSession.Clear();
                }

                return clientSession;

            }
            finally
            {
                ClientHandlerLock.Exit();
            }
        }

        public ItemInfo GetItemInfo()
        {
            ItemInfo? itemInfo = (ItemInfo?)m_ItemInfoPool.Poll();
            if(itemInfo == null)
            {
                itemInfo = new ItemInfo(m_OmmServerBaseImpl);
                m_ItemInfoPool.UpdatePool(itemInfo);
            }
            else
            {
                itemInfo.Clear();
            }

            return itemInfo;
        }

        public long GetClientHandle()
        {
            ClientHandlerLock.Enter();
            try
            {
                return ++m_ClientHandle;
            }
            finally
            {
                ClientHandlerLock.Exit();
            }
        }

        public long GetItemHandle()
        {
            return ++m_ItemHandle;
        }
    }
}
