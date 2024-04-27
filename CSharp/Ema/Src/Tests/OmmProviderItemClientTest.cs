/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access.Tests
{
    internal delegate void OnRefreshMsgHandler(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent);

    internal delegate void OnStatusMsgHandler(StatusMsg statusMsg, IOmmProviderEvent providerEvent);

    internal delegate void OnGenericMsgHandler(GenericMsg genericMsg, IOmmProviderEvent providerEvent);

    internal delegate void OnPostMsgHandler(PostMsg refreshMsg, IOmmProviderEvent providerEvent);

    internal delegate void OnReqMsgHandler(RequestMsg requestMsg, IOmmProviderEvent providerEvent);

    internal delegate void OnReissueMsgHandler(RequestMsg requestMsg, IOmmProviderEvent providerEvent);

    internal delegate void OnCloseHandler(RequestMsg requestMsg, IOmmProviderEvent providerEvent);


    internal class OmmProviderItemClientTest : IOmmProviderClient
    {
        OmmProvider? m_OmmProvider;

        public long ReceivedHandle { get; private set; }

        public object? Closure { get; private set; }

        public int ReceivedOnAll { get; private set; }

        public int ReceivedOnRefresh { get; private set; }

        public int ReceivedOnStatus { get; private set; }

        public int ReceivedOnGeneric { get; private set; }

        public int ReceivedOnReqMsg { get; private set; }

        public int ReceivedOnPostMsg { get; private set; }

        public int ReceivedOnReissueMsg { get; private set; }

        public int ReceivedOnClose { get; private set; }

        public OnRefreshMsgHandler? RefreshMsgHandler { get; set; }

        public OnStatusMsgHandler? StatusMsgHandler { get; set; }

        public OnGenericMsgHandler? GenericMsgHandler { get; set; }

        public OnPostMsgHandler? PostMsgHandler { get; set; }

        public OnReqMsgHandler? ReqMsgHandler { get; set; }

        public OnReissueMsgHandler? ReissueMsgHandler { get; set; }

        public OnCloseHandler? CloseHandler { get; set; }

        public OmmProviderItemClientTest()
        {
        }

        public OmmProviderItemClientTest(OmmProvider ommProvider)
        {
            m_OmmProvider = ommProvider;
        }

        public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent) 
        {
            if(m_OmmProvider != null)
                Assert.True(ReferenceEquals(m_OmmProvider, providerEvent.Provider));

            ReceivedHandle = providerEvent.Handle;
            Closure = providerEvent.Closure;
            ReceivedOnRefresh++;

            RefreshMsgHandler?.Invoke(refreshMsg, providerEvent);
        }

        public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent)
        {
            if (m_OmmProvider != null)
                Assert.True(ReferenceEquals(m_OmmProvider, providerEvent.Provider));

            ReceivedHandle = providerEvent.Handle;
            Closure = providerEvent.Closure;
            ReceivedOnStatus++;

            StatusMsgHandler?.Invoke(statusMsg, providerEvent);
        }
     
        public void OnGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent) 
        {
            if (m_OmmProvider != null)
                Assert.True(ReferenceEquals(m_OmmProvider, providerEvent.Provider));

            ReceivedHandle = providerEvent.Handle;
            Closure = providerEvent.Closure;
            ReceivedOnGeneric++;

            GenericMsgHandler?.Invoke(genericMsg, providerEvent);
        }

        public void OnAllMsg(Msg msg, IOmmProviderEvent providerEvent)
        {
            if (m_OmmProvider != null)
                Assert.True(ReferenceEquals(m_OmmProvider, providerEvent.Provider));

            ReceivedHandle = providerEvent.Handle;
            Closure = providerEvent.Closure;
            ReceivedOnAll++;
        }

        public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
        {
            if (m_OmmProvider != null)
                Assert.True(ReferenceEquals(m_OmmProvider, providerEvent.Provider));

            ReceivedHandle = providerEvent.Handle;
            Closure = providerEvent.Closure;
            ReceivedOnReqMsg++;

            ReqMsgHandler?.Invoke(reqMsg, providerEvent);
        }

        public void OnPostMsg(PostMsg postMsg, IOmmProviderEvent providerEvent)
        {
            if (m_OmmProvider != null)
                Assert.True(ReferenceEquals(m_OmmProvider, providerEvent.Provider));

            ReceivedHandle = providerEvent.Handle;
            Closure = providerEvent.Closure;
            ReceivedOnPostMsg++;

            PostMsgHandler?.Invoke(postMsg, providerEvent);
        }

        public void OnReissue(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
        {
            if (m_OmmProvider != null)
                Assert.True(ReferenceEquals(m_OmmProvider, providerEvent.Provider));

            ReceivedHandle = providerEvent.Handle;
            Closure = providerEvent.Closure;
            ReceivedOnReissueMsg++;

            ReissueMsgHandler?.Invoke(reqMsg, providerEvent);
        }

        public void OnClose(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
        {
            if (m_OmmProvider != null)
                Assert.True(ReferenceEquals(m_OmmProvider, providerEvent.Provider));

            ReceivedHandle = providerEvent.Handle;
            Closure = providerEvent.Closure;
            ReceivedOnClose++;

            CloseHandler?.Invoke(reqMsg, providerEvent);
        }

        public void ResetCallbackCount()
        {
            ReceivedOnAll = 0;
            ReceivedOnGeneric = 0;
            ReceivedOnRefresh = 0;
            ReceivedOnStatus = 0;
            ReceivedOnPostMsg = 0;
            ReceivedOnReqMsg = 0;
            ReceivedOnReissueMsg = 0;
            ReceivedOnClose = 0;
        }

        public int MessagesCount => ReceivedOnAll
            + ReceivedOnGeneric
            + ReceivedOnRefresh
            + ReceivedOnStatus
            + ReceivedOnPostMsg
            + ReceivedOnReqMsg
            + ReceivedOnReissueMsg
            + ReceivedOnClose;

    }
}
