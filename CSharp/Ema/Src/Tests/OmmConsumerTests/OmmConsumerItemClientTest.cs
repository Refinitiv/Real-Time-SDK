/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Access.Tests.OmmConsumerTests
{


    internal delegate void OnRefreshMsgHandler(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent);

    internal delegate void OnUpdateMsgHandler(UpdateMsg refreshMsg, IOmmConsumerEvent consumerEvent);

    internal delegate void OnStatusMsgHandler(StatusMsg refreshMsg, IOmmConsumerEvent consumerEvent);

    internal delegate void OnGenericMsgHandler(GenericMsg refreshMsg, IOmmConsumerEvent consumerEvent);

    internal delegate void OnAckMsgHandler(AckMsg refreshMsg, IOmmConsumerEvent consumerEvent);

    internal class OmmConsumerItemClientTest : IOmmConsumerClient
    {
        OmmConsumer? m_OmmConsumer;
        public long ReceivedHandle { get; private set; }

        public object? Closure { get; private set; }

        public int ReceivedOnAll { get; private set; }

        public int ReceivedOnRefresh { get; private set; }

        public int ReceivedOnUpdate { get; private set; }

        public int ReceivedOnStatus { get; private set; }

        public int ReceivedOnGeneric { get; private set; }

        public int ReceivedOnAck { get; private set; }

        public OnRefreshMsgHandler? RefreshMsgHandler { get; set; }

        public OnUpdateMsgHandler? UpdateMsgHandler { get; set; }

        public OnStatusMsgHandler? StatusMsgHandler { get; set; }

        public OnGenericMsgHandler? GenericMsgHandler { get; set; }

        public OnAckMsgHandler? AckMsgHandler { get; set; }

        public OmmConsumerItemClientTest()
        {
        }

        public OmmConsumerItemClientTest(OmmConsumer ommConsumer)
        {
            m_OmmConsumer = ommConsumer;
        }

        public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
        {
            Assert.True(ReferenceEquals(m_OmmConsumer, consumerEvent.Consumer));
            ReceivedHandle = consumerEvent.Handle;
            Closure = consumerEvent.Closure;
            ReceivedOnRefresh++;

            RefreshMsgHandler?.Invoke(refreshMsg, consumerEvent);
        }

        public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent) 
        {
            if(m_OmmConsumer != null)
                Assert.True(ReferenceEquals(m_OmmConsumer, consumerEvent.Consumer));
            ReceivedHandle = consumerEvent.Handle;
            Closure = consumerEvent.Closure;
            ReceivedOnUpdate++;

            UpdateMsgHandler?.Invoke(updateMsg, consumerEvent);
        }

        public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent) 
        {
            if (m_OmmConsumer != null)
                Assert.True(ReferenceEquals(m_OmmConsumer, consumerEvent.Consumer));
            ReceivedHandle = consumerEvent.Handle;
            Closure = consumerEvent.Closure;
            ReceivedOnStatus++;

            StatusMsgHandler?.Invoke(statusMsg, consumerEvent);
        }

        public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent consumerEvent)
        {
            if (m_OmmConsumer != null)
                Assert.True(ReferenceEquals(m_OmmConsumer, consumerEvent.Consumer));
            ReceivedHandle = consumerEvent.Handle;
            Closure = consumerEvent.Closure;
            ReceivedOnGeneric++;

            GenericMsgHandler?.Invoke(genericMsg, consumerEvent);
        }

        public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent consumerEvent) 
        {
            if (m_OmmConsumer != null)
                Assert.True(ReferenceEquals(m_OmmConsumer, consumerEvent.Consumer));
            ReceivedHandle = consumerEvent.Handle;
            Closure = consumerEvent.Closure;
            ReceivedOnAck++;

            AckMsgHandler?.Invoke(ackMsg, consumerEvent);
        }

        public void OnAllMsg(Msg msg, IOmmConsumerEvent consumerEvent) 
        {
            if (m_OmmConsumer != null)
                Assert.True(ReferenceEquals(m_OmmConsumer, consumerEvent.Consumer));
            ReceivedHandle = consumerEvent.Handle;
            Closure = consumerEvent.Closure;
            ReceivedOnAll++;
        }

        public void ResetCallbackCount()
        {
            ReceivedOnAck = 0;
            ReceivedOnAll = 0;
            ReceivedOnGeneric = 0;
            ReceivedOnRefresh = 0;
            ReceivedOnStatus = 0;
            ReceivedOnUpdate = 0;
        }
    }
}
