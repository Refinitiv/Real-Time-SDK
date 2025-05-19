/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Threading;

using LSEG.Ema.Access.Tests.OmmConsumerTests;
using LSEG.Eta.Codec;
using LSEG.Eta.Common;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;

using Xunit.Abstractions;

namespace LSEG.Ema.Access.Tests.OmmIProviderTests
{
    internal class SimpleConsumerTest : IConsumerCallback
    {
        static readonly int TRANSPORT_BUFFER_SIZE_REQUEST = 1000;

        private ITestOutputHelper m_Output;
        private Thread? m_DispatchingThread;

        private CancellationTokenSource m_ProviderCts = new();
        private CancellationToken m_Token;

        private ConsumerRole m_ConsumerRole = new();

        private Reactor? m_Reactor;
        private ReactorChannel? m_ReactorChannel;

        public Queue<TestReactorEvent> EventQueue { get; private set; } = new Queue<TestReactorEvent>();

        protected bool m_ReactorChannelIsUp;

        private DecodeIterator m_DecodeIterator = new DecodeIterator();

        private EncodeIterator m_EncodeIterator = new EncodeIterator();

        private IRequestMsg m_RequestMsg = new Eta.Codec.Msg();

        private ReactorSubmitOptions m_ReactorSubmitOptions = new();

        private IRequestMsg GetRequestMsg()
        {
            m_RequestMsg.Clear();
            m_RequestMsg.MsgClass = MsgClasses.REQUEST;
            return m_RequestMsg;
        }

        ReactorConnectOptions m_ConnectOptions = new();

        public bool RDMFieldDictComplete { get; private set; }

        public bool RDMEnumDictComplete { get; private set; }

        public DataDictionary DataDictionary { get; private set; } = new DataDictionary();

        public SimpleConsumerTest(ITestOutputHelper output, string port = "19000")
        {
            m_Output = output;

            /* Default host and port */
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = port;
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            connectInfo.ConnectOptions.MajorVersion = Eta.Codec.Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Eta.Codec.Codec.MinorVersion();

            m_ConnectOptions.ConnectionList.Add(connectInfo);
            m_ConnectOptions.SetReconnectAttempLimit(5);
        }

        public bool Initialize(ReactorOptions reactorOptions,
            ReactorConnectOptions? connectOptions = null,
            bool initLoginRequest = false,
            bool initDirectoryRequest = false,
            bool initDictionaryRequest = false)
        {
            m_Reactor = Reactor.CreateReactor(reactorOptions, out var errorInfo);

            if (m_Reactor is null)
            {
                m_Output.WriteLine($"Failed to create Reactor : {errorInfo?.Error.Text}");
                return false;
            }

            m_Token = m_ProviderCts.Token;

            m_DispatchingThread = new Thread(new ThreadStart(Run));
            m_DispatchingThread.Name = "SimpleConsumer";
            m_DispatchingThread.Start();

            ReactorConnectOptions consOptons = connectOptions != null ? connectOptions : m_ConnectOptions;

            // m_ConsumerRole.RdmLoginRequest.UserName.Data("UserName");
            // m_ConsumerRole.RdmLoginRequest.Password.Data("Passwd");

            m_ConsumerRole.ChannelEventCallback = this;
            m_ConsumerRole.DefaultMsgCallback = this;
            m_ConsumerRole.LoginMsgCallback = this;
            m_ConsumerRole.DirectoryMsgCallback = this;
            m_ConsumerRole.DictionaryMsgCallback = this;

            if (initLoginRequest)
                m_ConsumerRole.InitDefaultRDMLoginRequest();

            if (initLoginRequest && initDirectoryRequest)
                m_ConsumerRole.InitDefaultRDMDirectoryRequest();

            if (initDictionaryRequest)
            {
                m_ConsumerRole.InitDefaultRDMFieldDictionaryRequest();
                m_ConsumerRole.InitDefaultRDMEnumDictionaryRequest();

                m_ConsumerRole.DictionaryDownloadMode = DictionaryDownloadMode.FIRST_AVAILABLE;
            }

            if (m_Reactor.Connect(consOptons, m_ConsumerRole, out errorInfo) != ReactorReturnCode.SUCCESS)
            {
                m_Output.WriteLine($"Failed to call Reactor.Connect(): {errorInfo?.Error.Text}");
                return false;
            }

            return true;
        }

        public void UnInitialize()
        {
            if (m_ReactorChannel != null)
            {
                m_Reactor?.CloseChannel(m_ReactorChannel, out _);
            }

            Thread.Sleep(1000);

            if (m_DispatchingThread?.IsAlive == true)
            {
                m_ProviderCts.Cancel();

                m_DispatchingThread?.Join();
            }
            m_ProviderCts.Dispose();

            m_Reactor?.Shutdown(out _);
        }

        private void Run()
        {
            ReactorDispatchOptions dispatchOptions = new ReactorDispatchOptions();

            while (!m_Token.IsCancellationRequested
                   && m_Reactor != null)
            {
                if (m_Reactor.Dispatch(dispatchOptions, out var errorInfo) == ReactorReturnCode.FAILURE)
                {
                    Assert.Fail($"Reactor.Dispatch failed with {errorInfo}");
                    break;
                }
            }
        }

        public ReactorCallbackReturnCode DefaultMsgCallback(ReactorMsgEvent msgEvent)
        {
            EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.MSG, msgEvent));
            return ReactorCallbackReturnCode.SUCCESS;
        }

        internal int DictionaryStreamId = 0;
        internal int DictionaryServiceId = 0;

        public ReactorCallbackReturnCode RdmDictionaryMsgCallback(RDMDictionaryMsgEvent msgEvent)
        {
            switch (msgEvent.DictionaryMsg!.DictionaryMsgType)
            {
                case DictionaryMsgType.REFRESH:
                    {
                        DictionaryRefresh dictionaryRefresh = msgEvent.DictionaryMsg!.DictionaryRefresh!;

                        DictionaryStreamId = dictionaryRefresh.StreamId;
                        DictionaryServiceId = dictionaryRefresh.ServiceId;

                        m_DecodeIterator.Clear();

                        Assert.Equal(CodecReturnCode.SUCCESS,
                            m_DecodeIterator.SetBufferAndRWFVersion(dictionaryRefresh!.DataBody,
                                msgEvent.ReactorChannel!.MajorVersion, msgEvent.ReactorChannel.MinorVersion));

                        if (dictionaryRefresh!.DictionaryName.ToString().Equals("RWFFld"))
                        {
                            RDMFieldDictComplete = dictionaryRefresh.RefreshComplete;

                            Assert.Equal(CodecReturnCode.SUCCESS,
                                DataDictionary.DecodeFieldDictionary(m_DecodeIterator, (int)msgEvent.Msg!.MsgKey.Filter, out var decodeError));

                            Assert.Null(decodeError);
                        }
                        else if (dictionaryRefresh!.DictionaryName.ToString().Equals("RWFEnum"))
                        {
                            RDMEnumDictComplete = dictionaryRefresh.RefreshComplete;

                            Assert.Equal(CodecReturnCode.SUCCESS,
                                DataDictionary.DecodeEnumTypeDictionary(m_DecodeIterator, (int)msgEvent.Msg!.MsgKey.Filter, out _));
                        }
                    }
                    break;

                case DictionaryMsgType.STATUS:
                    Console.WriteLine("HIT! DictionaryMsgType.STATUS");
                    // give unit-tests an opportunity to examine received dictionary status message
                    EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.DICTIONARY_MSG, msgEvent));

                    break;

            }

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmDirectoryMsgCallback(RDMDirectoryMsgEvent directoryMsgEvent)
        {
            EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.DIRECTORY_MSG, directoryMsgEvent));
            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode RdmLoginMsgCallback(RDMLoginMsgEvent loginEvent)
        {
            EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.LOGIN_MSG, loginEvent));
            return ReactorCallbackReturnCode.SUCCESS;
        }

        public ReactorCallbackReturnCode ReactorChannelEventCallback(ReactorChannelEvent evt)
        {
            switch (evt.EventType)
            {
                case ReactorChannelEventType.CHANNEL_OPENED:
                    Assert.False(m_ReactorChannelIsUp);
                    break;

                case ReactorChannelEventType.CHANNEL_UP:
                    m_ReactorChannelIsUp = true;
                    break;

                case ReactorChannelEventType.CHANNEL_DOWN:
                case ReactorChannelEventType.CHANNEL_DOWN_RECONNECTING:
                    if (m_ReactorChannelIsUp)
                    {
                        /* If channel was up, the Socket should be present. */
                        Assert.NotNull(m_ReactorChannel!.Socket);
                    }

                    m_ReactorChannelIsUp = false;

                    m_ReactorChannel = null;

                    break;

                case ReactorChannelEventType.CHANNEL_READY:
                case ReactorChannelEventType.FD_CHANGE:
                case ReactorChannelEventType.WARNING:
                    Assert.NotNull(m_ReactorChannel);
                    break;

                default:
                    EmaTestUtils.Fail("Unhandled ReactorChannelEventType.");
                    break;

            }

            if (m_ReactorChannel != null)
            {
                Assert.Equal(m_ReactorChannel, evt.ReactorChannel);
            }
            else
                m_ReactorChannel = evt.ReactorChannel;

            EventQueue.Enqueue(new TestReactorEvent(TestReactorEventType.CHANNEL_EVENT, evt));

            return ReactorCallbackReturnCode.SUCCESS;
        }

        public void SubmitLoginRequest()
        {

            //m_ReactorChannel.Submit(loginReq, new ReactorSubmitOptions(), out _);
        }

        public bool SubmitItemRequest(int streamId, int serviceId, string itemName, int domainType, bool streaming = true, bool privateStream = false,
            bool keyInUpdate = false)
        {
            if (m_ReactorChannel == null)
                return false;

            IRequestMsg requestMsg = GetRequestMsg();
            Eta.Codec.Buffer nameBuffer = new Eta.Codec.Buffer();
            nameBuffer.Data(itemName);

            requestMsg.StreamId = streamId;
            requestMsg.DomainType = domainType;
            requestMsg.MsgKey.ApplyHasServiceId();
            requestMsg.MsgKey.ServiceId = serviceId;
            requestMsg.MsgKey.ApplyHasName();
            requestMsg.MsgKey.Name = nameBuffer;
            requestMsg.MsgKey.ApplyHasNameType();
            requestMsg.MsgKey.NameType = InstrumentNameTypes.RIC;

            if (streaming)
                requestMsg.Flags |= RequestMsgFlags.STREAMING;

            if (privateStream)
                requestMsg.Flags |= RequestMsgFlags.PRIVATE_STREAM;

            if (keyInUpdate)
                requestMsg.Flags |= RequestMsgFlags.MSG_KEY_IN_UPDATES;

            requestMsg.ApplyHasQos();
            requestMsg.Qos.Rate(QosRates.TICK_BY_TICK);
            requestMsg.Qos.Timeliness(QosTimeliness.REALTIME);

            ITransportBuffer? msgBuf = m_ReactorChannel.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out var errorInfo);

            if (msgBuf == null)
            {
                m_Output.WriteLine($"ReactorChannel.GetBuffer() return failure with error text {errorInfo?.Error.Text}");
                return false;
            }

            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, m_ReactorChannel.MajorVersion, m_ReactorChannel.MinorVersion);


            CodecReturnCode ret = requestMsg.EncodeInit(m_EncodeIterator, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return false;
            }

            ret = requestMsg.EncodeComplete(m_EncodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return false;
            }

            m_ReactorSubmitOptions.Clear();
            m_ReactorChannel.Submit(msgBuf, m_ReactorSubmitOptions, out errorInfo);

            return true;
        }

        public bool SubmitItemCloseRequest(int streamId, int domainType)
        {
            if (m_ReactorChannel == null)
                return false;

            ICloseMsg closeMsg = new Eta.Codec.Msg();
            closeMsg.MsgClass = MsgClasses.CLOSE;

            closeMsg.StreamId = streamId;
            closeMsg.DomainType = domainType;
            ITransportBuffer? msgBuf = m_ReactorChannel.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out var errorInfo);

            if (msgBuf == null)
            {
                m_Output.WriteLine($"ReactorChannel.GetBuffer() return failure with error text {errorInfo?.Error.Text}");
                return false;
            }

            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, m_ReactorChannel.MajorVersion, m_ReactorChannel.MinorVersion);


            CodecReturnCode ret = closeMsg.EncodeInit(m_EncodeIterator, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return false;
            }

            ret = closeMsg.EncodeComplete(m_EncodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return false;
            }

            m_ReactorSubmitOptions.Clear();
            m_ReactorChannel.Submit(msgBuf, m_ReactorSubmitOptions, out errorInfo);

            return true;
        }

        public bool SubmitGenericMsgOnStream(int streamId, int serviceId, string itemName, int domainType)
        {
            if (m_ReactorChannel == null)
                return false;

            IGenericMsg genericMsg = new Eta.Codec.Msg();
            genericMsg.MsgClass = MsgClasses.GENERIC;
            Eta.Codec.Buffer nameBuffer = new Eta.Codec.Buffer();
            nameBuffer.Data(itemName);

            genericMsg.StreamId = streamId;
            genericMsg.DomainType = domainType;
            genericMsg.ApplyHasMsgKey();
            genericMsg.MsgKey.ApplyHasServiceId();
            genericMsg.MsgKey.ServiceId = serviceId;
            genericMsg.MsgKey.ApplyHasName();
            genericMsg.MsgKey.Name = nameBuffer;
            genericMsg.MsgKey.ApplyHasNameType();
            genericMsg.MsgKey.NameType = InstrumentNameTypes.RIC;

            ITransportBuffer? msgBuf = m_ReactorChannel.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out var errorInfo);

            if (msgBuf == null)
            {
                m_Output.WriteLine($"ReactorChannel.GetBuffer() return failure with error text {errorInfo?.Error.Text}");
                return false;
            }

            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, m_ReactorChannel.MajorVersion, m_ReactorChannel.MinorVersion);


            CodecReturnCode ret = genericMsg.EncodeInit(m_EncodeIterator, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return false;
            }

            ret = genericMsg.EncodeComplete(m_EncodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return false;
            }

            m_ReactorSubmitOptions.Clear();
            m_ReactorChannel.Submit(msgBuf, m_ReactorSubmitOptions, out errorInfo);

            return true;
        }

        public bool SubmitPostMsgOnStream(int streamId, int serviceId, string itemName, int domainType, long postID, bool applyAck = false, bool postComplete = true,
            int partNum = 1, int postUserRights = -1)
        {
            if (m_ReactorChannel == null)
                return false;

            IPostMsg postMsg = new Eta.Codec.Msg();
            postMsg.MsgClass = MsgClasses.POST;
            Eta.Codec.Buffer nameBuffer = new Eta.Codec.Buffer();
            nameBuffer.Data(itemName);

            postMsg.StreamId = streamId;
            postMsg.DomainType = domainType;
            postMsg.ApplyHasMsgKey();
            postMsg.MsgKey.ApplyHasServiceId();
            postMsg.MsgKey.ServiceId = serviceId;
            postMsg.MsgKey.ApplyHasName();
            postMsg.MsgKey.Name = nameBuffer;
            postMsg.MsgKey.ApplyHasNameType();
            postMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
            postMsg.ApplyHasPostId();
            postMsg.PostId = postID;

            if(applyAck)
            {
                postMsg.ApplyAck();
            }

            if (postComplete)
            {
                postMsg.ApplyPostComplete();
            }
            else
            {
                postMsg.PartNum = partNum;
            }

            if(postUserRights > 0)
            {
                postMsg.ApplyHasPostUserRights();
                postMsg.PostUserRights = postUserRights;
            }

            ITransportBuffer? msgBuf = m_ReactorChannel.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out var errorInfo);

            if (msgBuf == null)
            {
                m_Output.WriteLine($"ReactorChannel.GetBuffer() return failure with error text {errorInfo?.Error.Text}");
                return false;
            }

            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, m_ReactorChannel.MajorVersion, m_ReactorChannel.MinorVersion);


            CodecReturnCode ret = postMsg.EncodeInit(m_EncodeIterator, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return false;
            }

            ret = postMsg.EncodeComplete(m_EncodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return false;
            }

            m_ReactorSubmitOptions.Clear();
            m_ReactorChannel.Submit(msgBuf, m_ReactorSubmitOptions, out errorInfo);

            return true;
        }

        public bool SubmitItemStatusMsg(int streamId, int serviceId, string itemName, int domainType)
        {
            if (m_ReactorChannel == null)
                return false;

            IStatusMsg statusMsg = new Eta.Codec.Msg();
            statusMsg.MsgClass = MsgClasses.STATUS;
            Eta.Codec.Buffer nameBuffer = new Eta.Codec.Buffer();
            nameBuffer.Data(itemName);

            statusMsg.StreamId = streamId;
            statusMsg.DomainType = domainType;
            statusMsg.ApplyHasMsgKey();
            statusMsg.MsgKey.ApplyHasServiceId();
            statusMsg.MsgKey.ServiceId = serviceId;
            statusMsg.MsgKey.ApplyHasName();
            statusMsg.MsgKey.Name = nameBuffer;
            statusMsg.MsgKey.ApplyHasNameType();
            statusMsg.MsgKey.NameType = InstrumentNameTypes.RIC;


            ITransportBuffer? msgBuf = m_ReactorChannel.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out var errorInfo);

            if (msgBuf == null)
            {
                m_Output.WriteLine($"ReactorChannel.GetBuffer() return failure with error text {errorInfo?.Error.Text}");
                return false;
            }

            m_EncodeIterator.Clear();
            m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, m_ReactorChannel.MajorVersion, m_ReactorChannel.MinorVersion);
            

            CodecReturnCode ret = statusMsg.EncodeInit(m_EncodeIterator, 0);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return false;
            }

            ret = statusMsg.EncodeComplete(m_EncodeIterator, true);
            if (ret < CodecReturnCode.SUCCESS)
            {
                return false;
            }

            m_ReactorSubmitOptions.Clear();
            m_ReactorChannel.Submit(msgBuf, m_ReactorSubmitOptions, out errorInfo);
            
            return true;
        }

        public void SubmitDictionaryRequest(int streamId, int serviceId, string serviceName, string dictName)
        {
            Assert.NotNull(m_ReactorChannel);

            var dictRequest = new DictionaryRequest();

            Eta.Codec.Buffer nameBuffer = new Eta.Codec.Buffer();
            nameBuffer.Data(dictName);

            dictRequest.DictionaryName = nameBuffer;
            dictRequest.StreamId = streamId;

            // dictRequest.ServiceId = serviceName;
            dictRequest.ServiceId = serviceId;

            SubmitMessage(dictRequest);
        }

        internal void SubmitDictionaryClose(int streamId)
        {
            Assert.NotNull(m_ReactorChannel);

            var dictClose = new DictionaryClose();

            dictClose.StreamId = streamId;

            SubmitMessage(dictClose);
        }

        internal void SubmitDictionaryStatus(int streamId)
        {
            Assert.NotNull(m_ReactorChannel);

            var dictStatus = new DictionaryStatus();

            dictStatus.StreamId = streamId;

            SubmitMessage(dictStatus);
        }

        internal void SubmitDictionaryRefresh(int streamId, int serviceId, string dictionaryName,
            Eta.Codec.DataDictionary dataDictionary)
        {
            Assert.NotNull(m_ReactorChannel);

            // todo: use a generic IRefresh message from ETA layer to avoid dealing
            //       with the valid Payload -- just specify DOMAIN and other required fields
            IRefreshMsg dictRefresh = (IRefreshMsg)new Eta.Codec.Msg();

            dictRefresh.Clear();

            dictRefresh.MsgClass = MsgClasses.REFRESH;
            dictRefresh.DomainType = (int)DomainType.DICTIONARY;
            dictRefresh.StreamId = streamId;

            dictRefresh.ApplyHasMsgKey();
            dictRefresh.MsgKey.ApplyHasServiceId();
            dictRefresh.MsgKey.ServiceId = serviceId;

            dictRefresh.State.Code(StateCodes.FAILOVER_COMPLETED);
            dictRefresh.State.DataState(DataStates.SUSPECT);
            dictRefresh.State.StreamState(StreamStates.OPEN);

            dictRefresh.Flags = (int) DictionaryRefreshFlags.CLEAR_CACHE
                | (int)DictionaryRefreshFlags.HAS_INFO
                | (int)DictionaryRefreshFlags.HAS_SEQ_NUM
                // | DictionaryRefreshFlags.IS_COMPLETE
                | (int)DictionaryRefreshFlags.SOLICITED;

            //dictRefresh.DictionaryName.Data(dictionaryName);
            //dictRefresh.DictionaryType = Types.ENUM_TABLES;

            //dictRefresh.DataDictionary = dataDictionary;

            SubmitMessage((Eta.Codec.Msg)dictRefresh);
        }

        private void SubmitMessage(MsgBase msg)
        {
            ITransportBuffer? msgBuf = m_ReactorChannel!.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out var errorInfo);

            Assert.NotNull(msgBuf);

            m_EncodeIterator.Clear();

            Assert.Equal(CodecReturnCode.SUCCESS,
                m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, m_ReactorChannel.MajorVersion, m_ReactorChannel.MinorVersion));

            Assert.Equal(CodecReturnCode.SUCCESS,
                msg.Encode(m_EncodeIterator));

            m_ReactorSubmitOptions.Clear();

            Assert.Equal(ReactorReturnCode.SUCCESS,
                m_ReactorChannel.Submit(msgBuf!, m_ReactorSubmitOptions, out var submitError));
        }

        private void SubmitMessage(Eta.Codec.Msg msg)
        {
            ITransportBuffer? msgBuf = m_ReactorChannel!.GetBuffer(TRANSPORT_BUFFER_SIZE_REQUEST, false, out var errorInfo);

            Assert.NotNull(msgBuf);

            m_EncodeIterator.Clear();

            Assert.Equal(CodecReturnCode.SUCCESS,
                m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, m_ReactorChannel.MajorVersion, m_ReactorChannel.MinorVersion));

            Assert.False(msg.EncodeInit(m_EncodeIterator, 0) < CodecReturnCode.SUCCESS);
            //Assert.False(msg.EncodeKeyAttribComplete(m_EncodeIterator, true) < CodecReturnCode.SUCCESS);
            Assert.False(msg.EncodeComplete(m_EncodeIterator, true) < CodecReturnCode.SUCCESS);

            m_ReactorSubmitOptions.Clear();

            Assert.Equal(ReactorReturnCode.SUCCESS,
                m_ReactorChannel.Submit(msgBuf!, m_ReactorSubmitOptions, out var submitError));
        }
    }
}
