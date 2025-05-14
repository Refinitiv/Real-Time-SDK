/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using System;
using System.Collections.Generic;
using Xunit.Abstractions;

namespace LSEG.Ema.Access.Tests.RequestRouting
{
    internal class ConsumerTestClient : IOmmConsumerClient
    {
        private readonly Queue<Msg> m_MessageQueue = new(30);

        private readonly Queue<ChannelInformation> m_ChannelInfoQueue = new();

        private readonly Queue<List<ChannelInformation>> m_SessionChannelInfoQueue = new();
        private MonitorWriteLocker AccessLock { get; set; } = new MonitorWriteLocker(new object());

        private ConsumerTestOptions m_ConsumerTestOptoins;

        private readonly HashSet<long> m_Handles = new();

        private long m_PostId = 0;

        private OmmConsumer? m_Consumer;

        readonly ITestOutputHelper m_Output;

        public ConsumerTestClient(ITestOutputHelper output)
        {
            m_ConsumerTestOptoins = new ConsumerTestOptions();
            m_Output = output;
        }

        public ConsumerTestClient(ITestOutputHelper output, ConsumerTestOptions consumerTestOptoins)
        {
            m_ConsumerTestOptoins = consumerTestOptoins;
            m_Output = output;
        }

        public void Consumer(OmmConsumer consumer)
        {
            m_Consumer = consumer;
        }

        public int QueueSize()
        {
            AccessLock.Enter();
            try
            {
                return m_MessageQueue.Count;
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public Msg PopMessage()
        {
            AccessLock.Enter();
            try
            {
                return m_MessageQueue.Dequeue();
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public int ChannelInfoSize()
        {
            AccessLock.Enter();
            try
            {
                return m_ChannelInfoQueue.Count;
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public ChannelInformation PopChannelInfo()
        {
            AccessLock.Enter();
            try
            {
                return m_ChannelInfoQueue.Dequeue();
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public int SessionChannelInfoSize()
        {
            AccessLock.Enter();
            try
            {
                return m_SessionChannelInfoQueue.Count;
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public List<ChannelInformation> PopSessionChannelInfo()
        {
            AccessLock.Enter();
            try
            {
                return m_SessionChannelInfoQueue.Dequeue();
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        private void AddChannelAndSessionInfo(IOmmConsumerEvent consumerEvent)
        {
            if (m_ConsumerTestOptoins.GetChannelInformation)
            {
                ChannelInformation eventChannelInfo = consumerEvent.ChannelInformation();
                ChannelInformation channelInfo = new()
                {
                    ChannelState = eventChannelInfo.ChannelState,
                    ConnectionType = eventChannelInfo.ConnectionType,
                    ProtocolType = eventChannelInfo.ProtocolType,
                    PingTimeout = eventChannelInfo.PingTimeout,
                    MajorVersion = eventChannelInfo.MajorVersion,
                    MinorVersion = eventChannelInfo.MinorVersion,
                    IpAddress = eventChannelInfo.IpAddress,
                    Hostname = eventChannelInfo.Hostname,
                    ComponentInfo = eventChannelInfo.ComponentInfo,
                    Port = eventChannelInfo.Port,
                    MaxFragmentSize = eventChannelInfo.MaxFragmentSize,
                    MaxOutputBuffers = eventChannelInfo.MaxOutputBuffers,
                    GuaranteedOutputBuffers = eventChannelInfo.GuaranteedOutputBuffers,
                    NumInputBuffers = eventChannelInfo.NumInputBuffers,
                    SysSendBufSize = eventChannelInfo.SysSendBufSize,
                    SysRecvBufSize = eventChannelInfo.SysRecvBufSize,
                    CompressionType = eventChannelInfo.CompressionType,
                    CompressionThreshold = eventChannelInfo.CompressionThreshold,
                    EncryptedConnectionType = eventChannelInfo.EncryptedConnectionType,
                    EncryptionProtocol = eventChannelInfo.EncryptionProtocol,
                    ChannelName = eventChannelInfo.ChannelName,
                    SessionChannelName = eventChannelInfo.SessionChannelName
                };

                m_ChannelInfoQueue.Enqueue(channelInfo);

                m_Output.WriteLine(consumerEvent.ChannelInformation().ToString());
            }

            if (m_ConsumerTestOptoins.GetSessionChannelInfo)
            {
                List<ChannelInformation> sessionInfo = new ();

                consumerEvent.SessionChannelInfo(sessionInfo);

                m_SessionChannelInfoQueue.Enqueue(sessionInfo);
            }
        }

        public void UnregisterAllHandles()
        {
            foreach(var handle in m_Handles)
            {
                m_Consumer?.Unregister(handle);
            }

            m_Handles.Clear();
        }

        public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent) 
        {
            AccessLock.Enter();

            try
            {
                m_Handles.Add(consumerEvent.Handle);

                RefreshMsg cloneMsg = new RefreshMsg(refreshMsg);

                if (refreshMsg.DomainType() == EmaRdm.MMT_DICTIONARY)
                {
                    if (m_ConsumerTestOptoins.DumpDictionaryRefreshMsg)
                        m_Output.WriteLine(cloneMsg.ToString());
                }
                else
                {
                    if (m_ConsumerTestOptoins.SubmitPostOnLoginRefresh)
                    {
                        if (refreshMsg.DomainType() == EmaRdm.MMT_LOGIN &&
                                refreshMsg.State().StreamState == OmmState.StreamStates.OPEN &&
                                refreshMsg.State().DataState == OmmState.DataStates.OK)
                        {
                            /* Submit a PostMsg to the login stream */
                            PostMsg postMsg = new ();
                            UpdateMsg nestedUpdateMsg = new ();
                            FieldList nestedFieldList = new ();

                            nestedFieldList.AddReal(22, 34, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                            nestedFieldList.AddReal(25, 35, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                            nestedFieldList.AddTime(18, 11, 29, 30);
                            nestedFieldList.AddEnumValue(37, 3);
                            nestedFieldList.Complete();

                            nestedUpdateMsg.Payload(nestedFieldList);

                            m_Consumer?.Submit(postMsg.PostId(++m_PostId).ServiceName("DIRECT_FEED")
                                                                        .Name("IBM.N").SolicitAck(false).Complete(true)
                                                                        .Payload(nestedUpdateMsg), consumerEvent.Handle);
                        }
                    }

                    m_Output.WriteLine(cloneMsg.ToString());
                }

                m_MessageQueue.Enqueue(cloneMsg);

                AddChannelAndSessionInfo(consumerEvent);
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
        {
            AccessLock.Enter();

            try
            {
                UpdateMsg cloneMsg =new (updateMsg);

                m_Output.WriteLine(cloneMsg.ToString());

                m_MessageQueue.Enqueue(cloneMsg);

                AddChannelAndSessionInfo(consumerEvent);
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent) 
        {
            AccessLock.Enter();

            try
            {
                StatusMsg cloneMsg = new(statusMsg);

                m_Output.WriteLine(cloneMsg.ToString());

                m_MessageQueue.Enqueue(cloneMsg);

                AddChannelAndSessionInfo(consumerEvent);
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent consumerEvent) 
        {
            AccessLock.Enter();

            try
            {
                AckMsg cloneMsg = new(ackMsg);

                m_Output.WriteLine(cloneMsg.ToString());

                m_MessageQueue.Enqueue(cloneMsg);

                AddChannelAndSessionInfo(consumerEvent);
            }
            finally
            {
                AccessLock.Exit();
            }
        }

        public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent consumerEvent)
        {
            AccessLock.Enter();

            try
            {
                GenericMsg cloneMsg = new(genericMsg);

                m_Output.WriteLine(cloneMsg.ToString());

                m_MessageQueue.Enqueue(cloneMsg);

                AddChannelAndSessionInfo(consumerEvent);
            }
            finally
            {
                AccessLock.Exit();
            }
        }
    }
}
