/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Xunit;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.Codec;
using Buffer = LSEG.Eta.Codec.Buffer;
using LSEG.Eta.Rdm;
using LSEG.Eta.Common;
using System.Threading;

namespace LSEG.Eta.ValuedAdd.Tests
{
    internal class ServerComponentTest : TestComponent
    {
        private const int REFRESH_MSG_SIZE = 1024;
        private const int MAX_FIELD_DICTIONARY_MSG_SIZE = 8192;
        private const int MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE = 12800;

        private const string APPLICATION_NAME = "ReactorTest";
        private const string APPLICATION_ID = "123";
        private const string POSITION = "127.0.0.1";

        private EncodeIterator m_EncodeIterator = new EncodeIterator();

        private ReactorSubmitOptions m_SubmitOptions = new ReactorSubmitOptions();

        public DirectoryRefresh DirectoryRefreshMsg { get; set; } = new DirectoryRefresh();
        public DictionaryRefresh FieldDictionaryRefreshMsg { get; set; } = new DictionaryRefresh();
        public DictionaryRefresh EnumTypeDictionaryRefreshMsg { get; set; } = new DictionaryRefresh();
        public LoginRefresh LoginRefreshMsg { get; set; } = new LoginRefresh();
        public IRefreshMsg MarketPriceRefreshMsg { get; set; } = new Msg();

        public ServerComponentTest()
        {
            CheckDirectoryMsg = msg => { Assert.Equal(DirectoryMsgType.REQUEST, msg.DirectoryMsgType); Assert.NotNull(msg.DirectoryRequest); };
            CheckDictionaryMsg = msg =>
            {
                Assert.True(msg.DictionaryMsgType == DictionaryMsgType.REQUEST || msg.DictionaryMsgType == DictionaryMsgType.CLOSE);
                if (msg.DictionaryMsgType == DictionaryMsgType.REQUEST) Assert.NotNull(msg.DictionaryRequest);
                if (msg.DictionaryMsgType == DictionaryMsgType.CLOSE) Assert.NotNull(msg.DictionaryClose);
            };
            CheckLoginMsg = msg => { Assert.Equal(LoginMsgType.REQUEST, msg.LoginMsgType); Assert.NotNull(msg.LoginRequest); };

            OnDefaultMsgReceived = msgEvent =>
            {
                IMsg msg = msgEvent.Msg;
                switch (msg.MsgClass)
                {
                    case MsgClasses.REQUEST:
                        if (msg.DomainType == (int)DomainType.MARKET_PRICE)
                        {
                            MarketPriceRefreshMsg.Clear();
                            MarketPriceRefreshMsg.DomainType = (int)Rdm.DomainType.MARKET_PRICE;
                            MarketPriceRefreshMsg.MsgClass = MsgClasses.REFRESH;
                            MarketPriceRefreshMsg.ApplyHasMsgKey();
                            MarketPriceRefreshMsg.StreamId = msg.StreamId;
                            MarketPriceRefreshMsg.MsgKey.ApplyHasName();
                            MarketPriceRefreshMsg.MsgKey.Name = msg.MsgKey.Name;
                            MarketPriceRefreshMsg.MsgKey.ApplyHasNameType();
                            MarketPriceRefreshMsg.MsgKey.NameType = InstrumentNameTypes.RIC;
                            MarketPriceRefreshMsg.State.DataState(DataStates.OK);
                            MarketPriceRefreshMsg.State.StreamState(StreamStates.OPEN);
                            MarketPriceRefreshMsg.State.Code(StateCodes.NONE);

                            m_SubmitOptions.Clear();
                            msgEvent.ReactorChannel.Submit((Msg)MarketPriceRefreshMsg, m_SubmitOptions, out ReactorErrorInfo errorInfo);
                        }
                        break;
                    default:
                        break;
                }
            };

            OnLoginMsgReceived = loginEvent =>
            {
                InitTestLoginRefresh(loginEvent.LoginMsg.LoginRequest.StreamId, loginEvent.LoginMsg.LoginRequest.UserName, APPLICATION_NAME, APPLICATION_ID, POSITION);

                ReactorChannel chnl = loginEvent.ReactorChannel;
                ITransportBuffer msgBuf = chnl.GetBuffer(REFRESH_MSG_SIZE, false, out ReactorErrorInfo errorInfo);
                Assert.NotNull(msgBuf);

                m_EncodeIterator.Clear();
                m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, Codec.Codec.MajorVersion(), Codec.Codec.MinorVersion());

                Assert.Equal(CodecReturnCode.SUCCESS, LoginRefreshMsg.Encode(m_EncodeIterator));
                Assert.Equal(ReactorReturnCode.SUCCESS, chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo));
            };

            OnDirectoryMsgReceived = directoryEvent =>
            {
                DirectoryMsg directoryMsg = directoryEvent.DirectoryMsg;
                switch (directoryMsg.DirectoryMsgType)
                {
                    case DirectoryMsgType.REQUEST:
                        DirectoryRequest request = directoryMsg.DirectoryRequest;
                        InitTestDirectoryRefresh(request.StreamId, request.Filter, request.ServiceId, "DIRECT_FEED");
                        ReactorChannel channel = directoryEvent.ReactorChannel;
                        ITransportBuffer msgBuf = channel.GetBuffer(REFRESH_MSG_SIZE, false, out ReactorErrorInfo error);
                        Assert.NotNull(msgBuf);
                        m_EncodeIterator.Clear();
                        Assert.Equal(CodecReturnCode.SUCCESS, m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, channel.MajorVersion, channel.MinorVersion));
                        Assert.Equal(CodecReturnCode.SUCCESS, DirectoryRefreshMsg.Encode(m_EncodeIterator));
                        channel.Submit(msgBuf, m_SubmitOptions, out error);
                        break;
                    default:
                        break;
                }
            };

            OnDictionaryMsgReceived = dictionaryEvent =>
            {
                DictionaryMsg dictionaryMsg = dictionaryEvent.DictionaryMsg;
                ReactorChannel chnl = dictionaryEvent.ReactorChannel;
                switch (dictionaryMsg.DictionaryMsgType)
                {
                    case DictionaryMsgType.REQUEST:
                        DictionaryRequest dictionaryRequest = dictionaryMsg.DictionaryRequest;
                        switch (dictionaryRequest.DictionaryName.ToString())
                        {
                            case "RWFFld":
                                InitTestFieldDictionaryRefreshMsg(dictionaryRequest.StreamId, dictionaryRequest.ServiceId, dictionaryRequest.Verbosity);
                                SubmitDictionaryResponse(chnl, FieldDictionaryRefreshMsg, MAX_FIELD_DICTIONARY_MSG_SIZE);
                                break;
                            case "RWFEnum":
                                InitTestEnumTypeDictionaryRefreshMsg(dictionaryRequest.StreamId, dictionaryRequest.ServiceId, dictionaryRequest.Verbosity);
                                SubmitDictionaryResponse(chnl, EnumTypeDictionaryRefreshMsg, MAX_ENUM_TYPE_DICTIONARY_MSG_SIZE);
                                break;
                        }
                        break;
                    default:
                        break;
                }
            };
        }

        public virtual void InitTestLoginRefresh(int streamId, Buffer userName, string applicationId, string applicationName, string position)
        {
            LoginRefreshMsg.Clear();
            LoginRefreshMsg.StreamId = streamId;
            LoginRefreshMsg.HasUserName = true;
            LoginRefreshMsg.UserName = userName;

            LoginRefreshMsg.HasAttrib = true;
            LoginRefreshMsg.HasUserNameType = true;
            LoginRefreshMsg.UserNameType = Login.UserIdTypes.NAME;

            LoginRefreshMsg.State.Code(StateCodes.NONE);
            LoginRefreshMsg.State.DataState(DataStates.OK);
            LoginRefreshMsg.State.StreamState(StreamStates.OPEN);
            LoginRefreshMsg.State.Text().Data("Login accepted by host localhost");

            LoginRefreshMsg.Solicited = true;

            LoginRefreshMsg.HasAttrib = true;
            LoginRefreshMsg.LoginAttrib.HasApplicationId = true;
            LoginRefreshMsg.LoginAttrib.ApplicationId.Data(applicationId);
            LoginRefreshMsg.LoginAttrib.HasApplicationName = true;
            LoginRefreshMsg.LoginAttrib.ApplicationName.Data(applicationName);

            if (position != null)
            {
                LoginRefreshMsg.LoginAttrib.HasPosition = true;
                LoginRefreshMsg.LoginAttrib.Position.Data(position);
            }

            LoginRefreshMsg.LoginAttrib.HasSingleOpen = true;
            LoginRefreshMsg.LoginAttrib.SingleOpen = 0;

            LoginRefreshMsg.HasFeatures = true;
            LoginRefreshMsg.SupportedFeatures.HasSupportBatchRequests = true;
            LoginRefreshMsg.SupportedFeatures.SupportBatchRequests = 1;

            LoginRefreshMsg.SupportedFeatures.HasSupportPost = true;
            LoginRefreshMsg.SupportedFeatures.SupportOMMPost = 1;
        }

        public virtual void InitTestDirectoryRefresh(int streamId, long filter, int serviceId, string serviceName)
        {
            DirectoryRefreshMsg.Clear();
            DirectoryRefreshMsg.StreamId = streamId;
            DirectoryRefreshMsg.ClearCache = true;
            DirectoryRefreshMsg.Solicited = true;
            DirectoryRefreshMsg.State.Clear();
            DirectoryRefreshMsg.State.StreamState(StreamStates.OPEN);
            DirectoryRefreshMsg.State.DataState(DataStates.OK);
            DirectoryRefreshMsg.State.Code(StateCodes.NONE);
            DirectoryRefreshMsg.State.Text().Data("Source Directory Refresh Completed");
            DirectoryRefreshMsg.Filter = filter;
            Service service = new Service();
            service.Clear();
            service.Action = MapEntryActions.ADD;

            // set the service Id (map key)
            service.ServiceId = serviceId;

            if ((filter & Directory.ServiceFilterFlags.INFO) != 0)
            {
                service.HasInfo = true;
                service.Info.Action = FilterEntryActions.SET;

                service.Info.HasVendor = true;
                service.Info.Vendor.Data("Refinitiv");

                service.Info.ServiceName.Data(serviceName);

                service.Info.HasSupportQosRange = true;
                service.Info.SupportsQosRange = 0;

                service.Info.CapabilitiesList.Add((long)Rdm.DomainType.MARKET_PRICE);
                service.Info.CapabilitiesList.Add((long)Rdm.DomainType.MARKET_BY_ORDER);
                service.Info.CapabilitiesList.Add((long)Rdm.DomainType.MARKET_BY_PRICE);
                service.Info.CapabilitiesList.Add((long)Rdm.DomainType.DICTIONARY);
                service.Info.CapabilitiesList.Add((long)Rdm.DomainType.SYMBOL_LIST);

                service.Info.HasQos = true;
                Qos qos = new Qos();
                qos.Rate(QosRates.TICK_BY_TICK);
                qos.Timeliness(QosTimeliness.REALTIME);
                service.Info.QosList.Add(qos);

                service.Info.HasDictionariesProvided = true;
                service.Info.DictionariesProvidedList.Add("RWFFld");
                service.Info.DictionariesProvidedList.Add("RWFEnum");

                service.Info.HasIsSource = true;
                service.Info.IsSource = 1;

                service.Info.HasItemList = true;
                service.Info.ItemList.Data("_ETA_ITEM_LIST");

                service.Info.HasAcceptingConsStatus = true;
                service.Info.AcceptConsumerStatus = 1;

                service.Info.HasSupportOOBSnapshots = true;
                service.Info.SupportsOOBSnapshots = 1;
            }

            if ((filter & Directory.ServiceFilterFlags.STATE) != 0)
            {
                service.HasState = true;
                service.State.Action = FilterEntryActions.SET;

                // service state
                service.State.ServiceStateVal = 1;

                // accepting requests
                service.State.HasAcceptingRequests = true;
                service.State.AcceptingRequests = 1;
            }

            if ((filter & Directory.ServiceFilterFlags.LOAD) != 0)
            {
                service.HasLoad = true;
                service.Load.Action = FilterEntryActions.SET;

                service.Load.HasOpenLimit = true;
                service.Load.OpenLimit = 5;
            }

            if ((filter & Directory.ServiceFilterFlags.LINK) != 0)
            {
                service.HasLink = true;
                service.Link.Action = FilterEntryActions.SET;

                ServiceLink serviceLink = new ServiceLink();

                serviceLink.Name.Data("DIRECT_FEED");

                serviceLink.HasType = true;
                serviceLink.Type = Directory.LinkTypes.INTERACTIVE;

                serviceLink.HasText = true;
                serviceLink.Text.Data("Link state is up");
                service.Link.LinkList.Add(serviceLink);
            }

            DirectoryRefreshMsg.ServiceList.Add(service);
        }

        public virtual void InitTestFieldDictionaryRefreshMsg(int fdStreamId, int serviceId, long verbosity)
        {
            LoadFieldDictionary(TestUtil.FIELD_TYPE_DICTIONARY_SHORT);
            
            FieldDictionaryRefreshMsg.Clear();

            FieldDictionaryRefreshMsg.StreamId = fdStreamId;
            FieldDictionaryRefreshMsg.DictionaryType = Rdm.Dictionary.Types.FIELD_DEFINITIONS;
            FieldDictionaryRefreshMsg.DataDictionary = Dictionary;
            FieldDictionaryRefreshMsg.State.StreamState(StreamStates.OPEN);
            FieldDictionaryRefreshMsg.State.DataState(DataStates.OK);
            FieldDictionaryRefreshMsg.State.Code(StateCodes.NONE);
            FieldDictionaryRefreshMsg.Verbosity = verbosity;
            FieldDictionaryRefreshMsg.ServiceId = serviceId;
            FieldDictionaryRefreshMsg.DictionaryName.Data("RWFFld");
            FieldDictionaryRefreshMsg.Solicited = true;
            FieldDictionaryRefreshMsg.State.Text().Data($"Field Dictionary Refresh");
        }

        public virtual void InitTestEnumTypeDictionaryRefreshMsg(int etStreamId, int serviceId, long verbosity)
        {
            LoadEnumTypeDictionary(TestUtil.ENUM_TYPE_DICTIONARY_SHORT);

            EnumTypeDictionaryRefreshMsg.Clear();

            EnumTypeDictionaryRefreshMsg.StreamId = etStreamId;
            EnumTypeDictionaryRefreshMsg.DictionaryType = Rdm.Dictionary.Types.ENUM_TABLES;
            EnumTypeDictionaryRefreshMsg.DataDictionary = Dictionary;
            EnumTypeDictionaryRefreshMsg.ServiceId = serviceId;
            EnumTypeDictionaryRefreshMsg.Verbosity = verbosity;
            EnumTypeDictionaryRefreshMsg.Solicited = true;
            EnumTypeDictionaryRefreshMsg.RefreshComplete = true;
            EnumTypeDictionaryRefreshMsg.ClearCache = true;
            EnumTypeDictionaryRefreshMsg.DictionaryName.Data("RWFEnum");

            EnumTypeDictionaryRefreshMsg.State.StreamState(StreamStates.OPEN);
            EnumTypeDictionaryRefreshMsg.State.DataState(DataStates.OK);
            EnumTypeDictionaryRefreshMsg.State.Code(StateCodes.NONE);
            EnumTypeDictionaryRefreshMsg.State.Text().Data("Enum Type Dictionary Refresh");
        }

        private void SubmitDictionaryResponse(ReactorChannel chnl, DictionaryRefresh response, int maxMsgSize)
        {
            bool firstMultiPart = true;
            while (true)
            {
                // get a buffer for the dictionary response
                ITransportBuffer msgBuf = chnl.GetBuffer(maxMsgSize, false, out ReactorErrorInfo errorInfo);
                Assert.NotNull(msgBuf);

                msgBuf.Data.Limit = maxMsgSize;

                // clear encode iterator
                m_EncodeIterator.Clear();
                CodecReturnCode ret = m_EncodeIterator.SetBufferAndRWFVersion(msgBuf, chnl.MajorVersion, chnl.MinorVersion);
                Assert.True(ret == CodecReturnCode.SUCCESS);

                if (firstMultiPart)
                {
                    response.ClearCache = true;
                    firstMultiPart = false;
                }
                else
                {
                    response.Flags = DictionaryRefreshFlags.SOLICITED;
                }

                ret = response.Encode(m_EncodeIterator);
                Assert.Equal(ReactorReturnCode.SUCCESS, chnl.Submit(msgBuf, m_SubmitOptions, out errorInfo));

                if (ret == CodecReturnCode.SUCCESS)
                {
                    break;
                }

                try
                {
                    Thread.Sleep(1);
                }
                catch (Exception e)
                {
                    TestUtil.Fail(e.Message);
                }
            }
        }
    }
}
