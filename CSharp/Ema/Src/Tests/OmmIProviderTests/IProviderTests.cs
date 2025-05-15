/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access.Tests.OmmConsumerTests;
using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.Rdm;
using LSEG.Eta.ValueAdd.Rdm;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Collections.Generic;
using System.Threading;
using Xunit.Abstractions;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Ema.Access.Tests.OmmIProviderTests
{
    public class IProviderTests : IDisposable
    {
        public void Dispose()
        {
            EtaGlobalPoolTestUtil.Clear();
        }

        ITestOutputHelper m_Output;

        public IProviderTests(ITestOutputHelper output)
        {
            m_Output = output;
        }

        [Fact]
        public void InitializeAndUninitializeTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            try
            {
                provider = new OmmProvider(config.Port("19000"), providerClient);

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Fact]
        public void SecondUninitializeTest()
        {
            // Arrange
            OmmIProviderConfig config = new();
            OmmProviderItemClientTest providerClient = new();
            OmmProvider provider = new(config.Port("19000"), providerClient);
            string providerNameInitial = provider.ProviderName;
            OmmProviderConfig.ProviderRoleEnum providerRoleInitial = provider.ProviderRole;

            provider.Uninitialize();

            // Act/Assert
            provider.Uninitialize();
            // No exception should be at this point

            Assert.Equal(providerNameInitial, provider.ProviderName);
            Assert.Equal(providerRoleInitial, provider.ProviderRole);
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void ProviderAcceptAClientConnection(bool userDispatch)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "19001";

            SimpleConsumerTest simpleConsumer = new(m_Output, port);
            ReactorOptions reactorOption = new();

            try
            {
                provider = new OmmProvider(config.Port(port).OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                    : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption);

                if (userDispatch)
                {
                    for (int i = 0; i < 10; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(10000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(2, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();

                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;

                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;

                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                Assert.Single(serverBaseImpl.ConnectedChannelList);

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                    Thread.Sleep(5000); // Removes the client connection

                Assert.Empty(serverBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            Assert.Null(exception);
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void ProviderBasicLoginHandlerTest(bool userDispatch)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "19002";

            SimpleConsumerTest simpleConsumer = new(m_Output, port);
            ReactorOptions reactorOption = new();

            try
            {
                provider = new OmmProvider(config.Port(port).OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                    : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);
                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                        .DomainType(requestMsg.DomainType())
                        .Solicited(true), providerEvent.Handle);
                };

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, null, true);

                if (userDispatch)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(3000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(3, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                Assert.Equal(1, providerClient.ReceivedOnReqMsg);

                OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;
                Assert.Single(serverBaseImpl.ConnectedChannelList);

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(5000); // Removes the client connection

                Assert.Empty(serverBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }
            finally
            {
                Assert.NotNull(provider);
                provider?.Uninitialize();
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void ProviderBasicDirectoryHandlerTest(bool userDispatch)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "19003";

            SimpleConsumerTest simpleConsumer = new(m_Output, port);
            ReactorOptions reactorOption = new();

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(port).OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                    : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);
                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:
                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType())
                                .Solicited(true), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:
                            var payload = new Map();
                            var service = new FilterList();
                            service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, new ElementList().AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED").MarkForClear().Complete());
                            payload.AddKeyUInt(5, MapAction.ADD, service.MarkForClear().Complete()).MarkForClear().Complete();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType())
                                .Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                            break;
                        default:
                            break;
                    }

                };

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, null, true, true);

                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                Assert.Equal(2, providerClient.ReceivedOnReqMsg);

                OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;
                Assert.Single(serverBaseImpl.ConnectedChannelList);

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    for (int i = 0; i < 3; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(serverBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }
            finally
            {
                Assert.NotNull(provider);
                provider?.Uninitialize();
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void ProviderSingleMarketItemTest(bool userDispatch)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19004";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            /* Expected test parameters */
            ulong serviceID = 1;
            string serviceName = "DIRECT_FEED";
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int streamID = 5;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                    .OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                        : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:
                            var payload = new Map();
                            var service = new FilterList();
                            service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName)
                                                 .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                        .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                        .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .MarkForClear().Complete())
                            .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                                .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1).MarkForClear().Complete())
                            .MarkForClear().Complete();

                            payload.AddKeyUInt(serviceID, MapAction.ADD, service).MarkForClear().Complete();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                                .MarkForClear().Complete(true).Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                            break;
                        case (int)DomainType.MARKET_PRICE:

                            /* Checks the requset message from the consumer side. */
                            Assert.Equal(streamID, requestMsg.StreamId());
                            Assert.Equal(domainType, requestMsg.DomainType());
                            Assert.True(requestMsg.HasServiceId);
                            Assert.Equal((int)serviceID, requestMsg.ServiceId());
                            Assert.True(requestMsg.HasServiceName);
                            Assert.Equal(serviceName, requestMsg.ServiceName());
                            Assert.True(requestMsg.HasName);
                            Assert.Equal(itemName, requestMsg.Name());

                            /* Send refresh message to the consumer */
                            provider.Submit(new RefreshMsg().ServiceName(serviceName).Name(itemName).DomainType(domainType)
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Solicited Refresh Completed")
                            .MarkForClear().Payload(new FieldList()
                            .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                            .AddUInt(3, 55)
                            .AddAscii(967, "ascii value")
                            .AddTime(375, 4, 15, 59, 100, 200, 300)
                            .MarkForClear().Complete())
                            .MarkForClear().Complete(true).ClearCache(true).Solicited(true), providerEvent.Handle);

                            break;
                        default:
                            break;
                    }

                };

                SimpleConsumerTest simpleConsumer = new(m_Output);
                ReactorOptions reactorOption = new();

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, connectOptions, true, true);

                if (userDispatch)
                {
                    for (int i = 0; i < 10; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(10000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                // Checks login response from the provider side
                {
                    RDMLoginMsgEvent loginMsgEvent = (RDMLoginMsgEvent)reactorEvent.ReactorEvent;

                    Assert.NotNull(loginMsgEvent.Msg);

                    /* Checks the complete flag */
                    Assert.True((loginMsgEvent.Msg!.Flags & RefreshMsgFlags.REFRESH_COMPLETE) != 0);

                    Assert.NotNull(loginMsgEvent.LoginMsg);
                    Assert.Equal(LoginMsgType.REFRESH, loginMsgEvent.LoginMsg!.LoginMsgType);

                    Assert.NotNull(loginMsgEvent.LoginMsg.LoginRefresh);

                    var loginRefresh = loginMsgEvent.LoginMsg.LoginRefresh!;

                    Assert.Equal(StreamStates.OPEN, loginRefresh.State.StreamState());
                    Assert.Equal(DataStates.OK, loginRefresh.State.DataState());
                    Assert.Equal(StateCodes.NONE, loginRefresh.State.Code());
                    Assert.Equal("Login accepted", loginRefresh.State.Text().ToString());

                    Assert.True(loginRefresh.Solicited);

                    Assert.True(loginRefresh.HasAttrib);

                    Assert.True(loginRefresh.LoginAttrib.HasApplicationId);
                    Assert.Equal("555", loginRefresh.LoginAttrib.ApplicationId.ToString());

                    Assert.True(loginRefresh.LoginAttrib.HasApplicationName);
                    Assert.Equal("ProviderSingleMarketItemTest", loginRefresh.LoginAttrib.ApplicationName.ToString());

                    Assert.True(loginRefresh.LoginAttrib.HasPosition);
                    Assert.Equal("localhost", loginRefresh.LoginAttrib.Position.ToString());

                    Assert.True(loginRefresh.LoginAttrib.HasSingleOpen);
                    Assert.Equal(1, loginRefresh.LoginAttrib.SingleOpen);

                    Assert.True(loginRefresh.LoginAttrib.HasAllowSuspectData);
                    Assert.Equal(1, loginRefresh.LoginAttrib.AllowSuspectData);

                    Assert.True(loginRefresh.HasFeatures);
                    Assert.True(loginRefresh.SupportedFeatures.HasSupportPost);
                    Assert.Equal(1, loginRefresh.SupportedFeatures.SupportOMMPost);
                }

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                // Checks directory response from the provider side
                {
                    RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)reactorEvent.ReactorEvent;

                    Assert.NotNull(directoryMsgEvent.Msg);

                    /* Checks the complete flag */
                    Assert.True((directoryMsgEvent.Msg!.Flags & RefreshMsgFlags.REFRESH_COMPLETE) != 0);

                    Assert.NotNull(directoryMsgEvent.DirectoryMsg);
                    Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg!.DirectoryMsgType);

                    Assert.NotNull(directoryMsgEvent.DirectoryMsg.DirectoryRefresh);

                    var directoryRefresh = directoryMsgEvent.DirectoryMsg.DirectoryRefresh!;

                    Assert.Equal(StreamStates.OPEN, directoryRefresh.State.StreamState());
                    Assert.Equal(DataStates.OK, directoryRefresh.State.DataState());
                    Assert.Equal(StateCodes.NONE, directoryRefresh.State.Code());
                    Assert.Equal("", directoryRefresh.State.Text().ToString());

                    Assert.True(directoryRefresh.Solicited);

                    Assert.Equal(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER, directoryRefresh.Filter);

                    Assert.False(directoryRefresh.HasServiceId);
                    Assert.Single(directoryRefresh.ServiceList);

                    var service = directoryRefresh.ServiceList[0];

                    Assert.Equal(1, service.ServiceId);

                    Assert.True(service.HasInfo);

                    /* Checks service Info */
                    {
                        var serviceInfo = service.Info;
                        Assert.Equal("DIRECT_FEED", serviceInfo.ServiceName.ToString());
                        Assert.Equal(2, serviceInfo.CapabilitiesList.Count);
                        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, serviceInfo.CapabilitiesList[0]);
                        Assert.Equal(EmaRdm.MMT_MARKET_BY_PRICE, serviceInfo.CapabilitiesList[1]);

                        Assert.Equal(2, serviceInfo.DictionariesUsedList.Count);
                        Assert.Equal("RWFFld", serviceInfo.DictionariesUsedList[0]);
                        Assert.Equal("RWFEnum", serviceInfo.DictionariesUsedList[1]);
                    }

                    Assert.True(service.HasState);
                    /* Checks service state */
                    {
                        var serviceState = service.State;
                        Assert.Equal(EmaRdm.SERVICE_UP, serviceState.ServiceStateVal);
                    }

                }

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                Assert.Equal(2, providerClient.ReceivedOnReqMsg);

                OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;
                Assert.Single(serverBaseImpl.ConnectedChannelList);

                simpleConsumer.SubmitItemRequest(streamID, (int)serviceID, itemName, domainType);

                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                /* Checks response from item request */
                Assert.Single(simpleConsumer.EventQueue);
                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                {
                    ReactorMsgEvent reactorMsgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;

                    Assert.NotNull(reactorMsgEvent.Msg);
                    Assert.Equal(MsgClasses.REFRESH, reactorMsgEvent.Msg!.MsgClass);

                    IRefreshMsg refreshMsg = (IRefreshMsg)reactorMsgEvent.Msg!;

                    Assert.Equal(streamID, refreshMsg.StreamId);
                    Assert.True(refreshMsg.CheckClearCache());
                    Assert.True(refreshMsg.CheckRefreshComplete());

                    Assert.Equal(EmaRdm.MMT_MARKET_PRICE, refreshMsg.DomainType);

                    Assert.True(refreshMsg.MsgKey.CheckHasServiceId());
                    Assert.Equal((int)serviceID, refreshMsg.MsgKey.ServiceId);

                    Assert.True(refreshMsg.MsgKey.CheckHasName());
                    Assert.Equal(itemName, refreshMsg.MsgKey.Name.ToString());

                    Assert.Equal(DataTypes.FIELD_LIST, refreshMsg.ContainerType);

                    DecodeIterator decodeIterator = new DecodeIterator();

                    decodeIterator.SetBufferAndRWFVersion(refreshMsg.EncodedDataBody, reactorMsgEvent.ReactorChannel!.MajorVersion,
                        reactorMsgEvent.ReactorChannel.MinorVersion);

                    Eta.Codec.FieldList fieldList = new();
                    Assert.Equal(CodecReturnCode.SUCCESS, fieldList.Decode(decodeIterator, null));

                    Eta.Codec.FieldEntry fieldEntry = new();

                    Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(decodeIterator));
                    Assert.Equal(22, fieldEntry.FieldId);
                    Real realValue = new();
                    Assert.Equal(CodecReturnCode.SUCCESS, realValue.Decode(decodeIterator));
                    Assert.Equal(3990, realValue.ToLong());
                    Assert.Equal(OmmReal.MagnitudeTypes.EXPONENT_NEG_2, realValue.Hint);

                    Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(decodeIterator));
                    Assert.Equal(3, fieldEntry.FieldId);
                    UInt uInt = new();
                    Assert.Equal(CodecReturnCode.SUCCESS, uInt.Decode(decodeIterator));
                    Assert.Equal<ulong>(55, uInt.ToULong());

                    Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(decodeIterator));
                    Assert.Equal(967, fieldEntry.FieldId);
                    Buffer asciiBuffer = new();
                    Assert.Equal(CodecReturnCode.SUCCESS, asciiBuffer.Decode(decodeIterator));
                    Assert.Equal("ascii value", asciiBuffer.ToString());

                    Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(decodeIterator));
                    Assert.Equal(375, fieldEntry.FieldId);
                    Time time = new Time();
                    Assert.Equal(CodecReturnCode.SUCCESS, time.Decode(decodeIterator));
                    Assert.Equal(4, time.Hour());
                    Assert.Equal(15, time.Minute());
                    Assert.Equal(59, time.Second());
                    Assert.Equal(100, time.Millisecond());
                    Assert.Equal(200, time.Microsecond());
                    Assert.Equal(300, time.Nanosecond());

                    Assert.Equal(CodecReturnCode.END_OF_CONTAINER, fieldEntry.Decode(decodeIterator));
                }

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(serverBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();

        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void ProviderInvalidServiceIdSingleMarketItemTest(bool userDispatch)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19005";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;

            /* Expected test parameters */
            ulong serviceID = 1;
            string serviceName = "DIRECT_FEED";
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int streamID = 5;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                   .OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                       : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;


                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:
                            var payload = new Map();
                            var service = new FilterList();
                            service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName)
                                                 .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                        .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                        .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .MarkForClear().Complete())
                            .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP).MarkForClear().Complete())
                            .MarkForClear().Complete();

                            payload.AddKeyUInt(serviceID, MapAction.ADD, service).MarkForClear().Complete();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                                .MarkForClear().Complete(true).Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                            break;
                        case (int)DomainType.MARKET_PRICE:

                            /* Don't receive the item request as EMA rejects this request from invalid service ID */
                            Assert.Fail("Should not receive the item request message.");

                            break;
                        default:
                            break;
                    }

                };

                SimpleConsumerTest simpleConsumer = new(m_Output);
                ReactorOptions reactorOption = new();

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, connectOptions, true, true);


                if (userDispatch)
                {
                    for (int i = 0; i < 10; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(10000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                Assert.Equal(2, providerClient.ReceivedOnReqMsg);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                int invalidServiceID = 555;

                /* submit with invalid service ID */
                simpleConsumer.SubmitItemRequest(streamID, invalidServiceID, itemName, domainType, true, userDispatch ? true : false);

                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                /* Checks response from item request */
                Assert.Single(simpleConsumer.EventQueue);
                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                {
                    ReactorMsgEvent reactorMsgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;

                    Assert.NotNull(reactorMsgEvent.Msg);
                    Assert.Equal(MsgClasses.STATUS, reactorMsgEvent.Msg!.MsgClass);

                    IStatusMsg statusMsg = (IStatusMsg)reactorMsgEvent.Msg!;
                    Assert.Equal(streamID, statusMsg.StreamId);

                    Assert.Equal(EmaRdm.MMT_MARKET_PRICE, statusMsg.DomainType);
                    Assert.Equal(DataTypes.NO_DATA, statusMsg.ContainerType);

                    if (userDispatch)
                        Assert.True(statusMsg.CheckPrivateStream());
                    else
                        Assert.False(statusMsg.CheckPrivateStream());

                    Assert.True(statusMsg.CheckHasState());
                    Assert.Equal(StreamStates.CLOSED_RECOVER, statusMsg.State.StreamState());
                    Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());
                    Assert.Equal(StateCodes.USAGE_ERROR, statusMsg.State.Code());
                    Assert.Equal($"Request Message rejected - the service Id = {invalidServiceID} does not exist in the source directory.",
                        statusMsg.State.Text().ToString());

                    Assert.False(statusMsg.CheckHasMsgKey());

                    /* Ensure that the application doesn't receive the OnClose callback method. */
                    Assert.Equal(0, providerClient.ReceivedOnClose);

                    /* Ensure that the application doesn't receive the OnResMsg callback method for item request */
                    Assert.Equal(2, providerClient.ReceivedOnReqMsg);
                }

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void ProviderSendItemRequestWithoutLoggedInSingleMarketItemTest(bool userDispatch)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19006";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;

            /* Expected test parameters */
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int streamID = 5;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                   .OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                       : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.MARKET_PRICE:

                            /* Don't receive the item request as EMA rejects this request without being logged in. */
                            Assert.Fail("Should not receive the item request message.");

                            break;
                        default:
                            break;
                    }

                };

                SimpleConsumerTest simpleConsumer = new(m_Output);
                ReactorOptions reactorOption = new();

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, connectOptions, false);

                if (userDispatch)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(2000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(2, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                /* submit with a request without logging in */
                simpleConsumer.SubmitItemRequest(streamID, 5, itemName, domainType, true, userDispatch ? true : false);

                if (userDispatch)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(2000); // Wait for the provider to accept the client connection.
                }

                /* Checks response from item request */
                Assert.Single(simpleConsumer.EventQueue);
                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                {
                    ReactorMsgEvent reactorMsgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;

                    Assert.NotNull(reactorMsgEvent.Msg);
                    Assert.Equal(MsgClasses.STATUS, reactorMsgEvent.Msg!.MsgClass);

                    IStatusMsg statusMsg = (IStatusMsg)reactorMsgEvent.Msg!;
                    Assert.Equal(streamID, statusMsg.StreamId);

                    Assert.Equal(EmaRdm.MMT_MARKET_PRICE, statusMsg.DomainType);
                    Assert.Equal(DataTypes.NO_DATA, statusMsg.ContainerType);

                    if (userDispatch)
                        Assert.True(statusMsg.CheckPrivateStream());
                    else
                        Assert.False(statusMsg.CheckPrivateStream());

                    Assert.True(statusMsg.CheckHasState());
                    Assert.Equal(StreamStates.CLOSED_RECOVER, statusMsg.State.StreamState());
                    Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());
                    Assert.Equal(StateCodes.USAGE_ERROR, statusMsg.State.Code());
                    Assert.Equal($"Message rejected - there is no logged in user for this session.",
                        statusMsg.State.Text().ToString());

                    Assert.False(statusMsg.CheckHasMsgKey());

                    /* Ensure that the application doesn't receive the OnClose callback method. */
                    Assert.Equal(0, providerClient.ReceivedOnClose);

                    /* Ensure that the application doesn't receive the OnResMsg callback method for item request */
                    Assert.Equal(0, providerClient.ReceivedOnReqMsg);
                }

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Theory]
        [InlineData(false, true)]
        [InlineData(true, false)]
        public void ProviderSendItemRequestWithoutAcceptingRequestsMarketItemTest(bool userDispatch, bool acceptWithoutAcceptingReqs)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19007";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;

            /* Expected test parameters */
            ulong serviceID = 1;
            string serviceName = "DIRECT_FEED";
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int streamID = 5;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                   .OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                       : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                ommServerBaseImpl.MarketItemHandler.AcceptMessageWithoutAcceptingRequests = acceptWithoutAcceptingReqs;

                if (acceptWithoutAcceptingReqs == false)
                    ommServerBaseImpl.MarketItemHandler.IsDirectoryApiControl = true;

                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:
                            var payload = new Map();
                            var service = new FilterList();
                            service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName)
                                                 .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                        .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                        .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .MarkForClear().Complete())
                            .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                                .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 0).MarkForClear().Complete()) /* Don't accept the item requests */
                            .MarkForClear().Complete();

                            payload.AddKeyUInt(serviceID, MapAction.ADD, service).MarkForClear().Complete();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                                .MarkForClear().Complete(true).Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                            break;
                        case (int)DomainType.MARKET_PRICE:

                            if (acceptWithoutAcceptingReqs == false)
                            {
                                /* Don't receive the item request as EMA rejects this request from invalid service ID */
                                Assert.Fail("Should not receive the item request message.");
                            }

                            break;
                        default:
                            break;
                    }

                };

                SimpleConsumerTest simpleConsumer = new(m_Output);
                ReactorOptions reactorOption = new();

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, connectOptions, true, true);


                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                /* Checks that the provider receives both login and directory requests*/
                Assert.Equal(2, providerClient.ReceivedOnReqMsg);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                /* submit with a request without logging in */
                simpleConsumer.SubmitItemRequest(streamID, (int)serviceID, itemName, domainType, true, userDispatch ? true : false);

                if (userDispatch)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(2000); // Wait for the provider to accept the client connection.
                }

                if (acceptWithoutAcceptingReqs == false)
                {
                    /* Checks response from item request */
                    Assert.Single(simpleConsumer.EventQueue);
                    reactorEvent = simpleConsumer.EventQueue.Dequeue();
                    {
                        ReactorMsgEvent reactorMsgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;

                        Assert.NotNull(reactorMsgEvent.Msg);
                        Assert.Equal(MsgClasses.STATUS, reactorMsgEvent.Msg!.MsgClass);

                        IStatusMsg statusMsg = (IStatusMsg)reactorMsgEvent.Msg!;
                        Assert.Equal(streamID, statusMsg.StreamId);

                        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, statusMsg.DomainType);
                        Assert.Equal(DataTypes.NO_DATA, statusMsg.ContainerType);

                        if (userDispatch)
                            Assert.True(statusMsg.CheckPrivateStream());
                        else
                            Assert.False(statusMsg.CheckPrivateStream());

                        Assert.True(statusMsg.CheckHasState());
                        Assert.Equal(StreamStates.CLOSED_RECOVER, statusMsg.State.StreamState());
                        Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());
                        Assert.Equal(StateCodes.USAGE_ERROR, statusMsg.State.Code());
                        Assert.Equal($"Request message rejected - the service Id = {serviceID} does not accept any requests.",
                          statusMsg.State.Text().ToString());

                        Assert.False(statusMsg.CheckHasMsgKey());

                        /* Ensure that the application doesn't receive the OnClose callback method. */
                        Assert.Equal(0, providerClient.ReceivedOnClose);
                    }
                }
                else
                {
                    /* Receives the item request from the consumer side */
                    Assert.Equal(3, providerClient.ReceivedOnReqMsg);
                }

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Theory]
        [InlineData(false, true)]
        [InlineData(true, false)]
        public void ProviderSendItemRequestQosOutOfServiceRangeMarketItemTest(bool userDispatch, bool acceptOutOfRangeQos)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19008";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;

            /* Expected test parameters */
            ulong serviceID = 1;
            string serviceName = "DIRECT_FEED";
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int streamID = 5;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                   .OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                       : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                ommServerBaseImpl.MarketItemHandler.AcceptMessageWithoutQosInRange = acceptOutOfRangeQos;

                ((OmmIProviderImpl)ommServerBaseImpl).StoreUserSubmittedSource = true;

                if (acceptOutOfRangeQos == false)
                {
                    ommServerBaseImpl.MarketItemHandler.IsDirectoryApiControl = true;
                }

                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:
                            var payload = new Map();
                            var service = new FilterList();
                            service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName)
                                                 .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                        .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                        .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_QOS, new OmmArray()
                                                        .AddQos(OmmQos.Timelinesses.INEXACT_DELAYED, OmmQos.Rates.JUST_IN_TIME_CONFLATED).MarkForClear().Complete())
                                                 .MarkForClear().Complete())
                            .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                                .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1).MarkForClear().Complete())
                            .MarkForClear().Complete();

                            payload.AddKeyUInt(serviceID, MapAction.ADD, service).MarkForClear().Complete();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                                .MarkForClear().Complete(true).Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                            break;
                        case (int)DomainType.MARKET_PRICE:

                            if (acceptOutOfRangeQos == false)
                            {
                                /* Don't receive the item request as EMA rejects this request from out of range QoS */
                                Assert.Fail("Should not receive the item request message.");
                            }

                            break;
                        default:
                            break;
                    }

                };

                SimpleConsumerTest simpleConsumer = new(m_Output);
                ReactorOptions reactorOption = new();

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, connectOptions, true, true);


                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                /* Checks that the provider receives both login and directory requests*/
                Assert.Equal(2, providerClient.ReceivedOnReqMsg);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                /* submit with a request without logging in */
                simpleConsumer.SubmitItemRequest(streamID, (int)serviceID, itemName, domainType, true, userDispatch ? true : false);

                if (userDispatch)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(2000); // Wait for the provider to accept the client connection.
                }

                if (acceptOutOfRangeQos == false)
                {
                    /* Checks response from item request */
                    Assert.Single(simpleConsumer.EventQueue);
                    reactorEvent = simpleConsumer.EventQueue.Dequeue();
                    {
                        ReactorMsgEvent reactorMsgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;

                        Assert.NotNull(reactorMsgEvent.Msg);
                        Assert.Equal(MsgClasses.STATUS, reactorMsgEvent.Msg!.MsgClass);

                        IStatusMsg statusMsg = (IStatusMsg)reactorMsgEvent.Msg!;
                        Assert.Equal(streamID, statusMsg.StreamId);

                        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, statusMsg.DomainType);
                        Assert.Equal(DataTypes.NO_DATA, statusMsg.ContainerType);

                        if (userDispatch)
                            Assert.True(statusMsg.CheckPrivateStream());
                        else
                            Assert.False(statusMsg.CheckPrivateStream());

                        Assert.True(statusMsg.CheckHasState());
                        Assert.Equal(StreamStates.CLOSED_RECOVER, statusMsg.State.StreamState());
                        Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());
                        Assert.Equal(StateCodes.USAGE_ERROR, statusMsg.State.Code());
                        Assert.Equal($"Request message rejected - the service Id = {serviceID} does not support the specified QoS(Qos: Realtime/TickByTick/Static - timeInfo: 0 - rateInfo: 0).",
                          statusMsg.State.Text().ToString());

                        Assert.False(statusMsg.CheckHasMsgKey());

                        /* Ensure that the application doesn't receive the OnClose callback method. */
                        Assert.Equal(0, providerClient.ReceivedOnClose);
                    }
                }
                else
                {
                    /* Receives the item request message from out of range QoS */
                    Assert.Equal(3, providerClient.ReceivedOnReqMsg);
                }

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Theory]
        [InlineData(false, true)]
        [InlineData(true, false)]
        public void ProviderSendItemRequestSameKeyDiffStreamMarketItemTest(bool userDispatch, bool acceptSameKeyDiffStream)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19009";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;

            /* Expected test parameters */
            ulong serviceID = 1;
            string serviceName = "DIRECT_FEED";
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int streamID = 5;

            int numOfItemRequests = 0;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                   .OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                       : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                ommServerBaseImpl.MarketItemHandler.AcceptMessageSameKeyButDiffStream = acceptSameKeyDiffStream;

                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:
                            var payload = new Map();
                            var service = new FilterList();
                            service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName)
                                                 .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                        .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                        .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_QOS, new OmmArray()
                                                        .AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK).MarkForClear().Complete())
                                                 .MarkForClear().Complete())
                            .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                                .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1).MarkForClear().Complete())
                            .MarkForClear().Complete();

                            payload.AddKeyUInt(serviceID, MapAction.ADD, service).MarkForClear().Complete();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                                .MarkForClear().Complete(true).Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                            break;
                        case (int)DomainType.MARKET_PRICE:

                            numOfItemRequests++;

                            if (acceptSameKeyDiffStream == false && numOfItemRequests == 2)
                            {
                                /* Don't receive the item request as EMA rejects this request from out of range QoS */
                                Assert.Fail("Should not receive the item request message.");
                            }

                            break;
                        default:
                            break;
                    }

                };

                SimpleConsumerTest simpleConsumer = new(m_Output);
                ReactorOptions reactorOption = new();

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, connectOptions, true, true);


                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                /* Checks that the provider receives both login and directory requests*/
                Assert.Equal(2, providerClient.ReceivedOnReqMsg);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                simpleConsumer.SubmitItemRequest(streamID, (int)serviceID, itemName, domainType, true, userDispatch ? true : false);

                simpleConsumer.SubmitItemRequest(streamID + 1, (int)serviceID, itemName, domainType, true, userDispatch ? true : false);

                if (userDispatch)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(4000); // Wait for the provider to accept the client connection.
                }

                if (acceptSameKeyDiffStream == false)
                {
                    /* Checks response from item request */
                    Assert.Single(simpleConsumer.EventQueue);
                    reactorEvent = simpleConsumer.EventQueue.Dequeue();
                    {
                        ReactorMsgEvent reactorMsgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;

                        Assert.NotNull(reactorMsgEvent.Msg);
                        Assert.Equal(MsgClasses.STATUS, reactorMsgEvent.Msg!.MsgClass);

                        IStatusMsg statusMsg = (IStatusMsg)reactorMsgEvent.Msg!;
                        Assert.Equal(streamID + 1, statusMsg.StreamId);

                        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, statusMsg.DomainType);
                        Assert.Equal(DataTypes.NO_DATA, statusMsg.ContainerType);

                        if (userDispatch)
                            Assert.True(statusMsg.CheckPrivateStream());
                        else
                            Assert.False(statusMsg.CheckPrivateStream());

                        Assert.True(statusMsg.CheckHasState());
                        Assert.Equal(StreamStates.CLOSED_RECOVER, statusMsg.State.StreamState());
                        Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());
                        Assert.Equal(StateCodes.USAGE_ERROR, statusMsg.State.Code());
                        Assert.Equal($"Request Message rejected - Item already open with exact same message key on another stream.",
                          statusMsg.State.Text().ToString());

                        Assert.False(statusMsg.CheckHasMsgKey());

                        /* Ensure that the application doesn't receive the OnClose callback method. */
                        Assert.Equal(0, providerClient.ReceivedOnClose);

                        /* Ensure that the provider receives the first item request */
                        Assert.Equal(3, providerClient.ReceivedOnReqMsg);
                    }
                }
                else
                {
                    /* Ensure that the provider receives the first and second item requests */
                    Assert.Equal(4, providerClient.ReceivedOnReqMsg);

                    Assert.Equal(2, numOfItemRequests);
                }

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Theory]
        [InlineData(false, true)]
        [InlineData(true, false)]
        public void ProviderSendItemReissueWithServiceNameChangeMarketItemTest(bool userDispatch, bool acceptServiceNameChange)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19010";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;

            /* Expected test parameters */
            ulong serviceID = 1;
            ulong serviceID2 = 2;
            string serviceName = "DIRECT_FEED";
            string serviceName2 = "DIRECT_FEED2";
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int streamID = 5;
            long itemHandle = 0;
            long itemHandle2 = 0;
            int numOfItemRequests = 0;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                   .OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                       : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                ommServerBaseImpl.MarketItemHandler.AcceptMessageThatChangesService = acceptServiceNameChange;

                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:
                            var payload = new Map();
                            var service = new FilterList();
                            service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName)
                                                 .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                        .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                        .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_QOS, new OmmArray()
                                                        .AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK).MarkForClear().Complete())
                                                 .MarkForClear().Complete())
                            .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                                .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1).MarkForClear().Complete())
                            .MarkForClear().Complete();

                            var service2 = new FilterList();
                            service2.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName2)
                                                 .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                        .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                        .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_QOS, new OmmArray()
                                                        .AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK).MarkForClear().Complete())
                                                 .MarkForClear().Complete())
                            .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                                .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1).MarkForClear().Complete())
                            .MarkForClear().Complete();

                            payload.AddKeyUInt(serviceID, MapAction.ADD, service)
                                .AddKeyUInt(serviceID2, MapAction.ADD, service2).MarkForClear().Complete();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                                .MarkForClear().Complete(true).Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                            break;
                        case (int)DomainType.MARKET_PRICE:

                            numOfItemRequests++;

                            if (numOfItemRequests == 1)
                            {
                                itemHandle = providerEvent.Handle;
                            }
                            else
                            {
                                itemHandle2 = providerEvent.Handle;
                            }

                            if (acceptServiceNameChange == false && numOfItemRequests == 2)
                            {
                                /* Don't receive the item request as EMA rejects this request from out of range QoS */
                                Assert.Fail("Should not receive the item request message.");
                            }

                            break;
                        default:
                            break;
                    }

                };

                providerClient.CloseHandler = (requestMsg, providerEvent) =>
                {
                    /* Checks for the close message of the item domain */
                    if (requestMsg.DomainType() > EmaRdm.MMT_DIRECTORY)
                    {
                        Assert.Equal(itemHandle, providerEvent.Handle);

                        /* Checks the close message of the first item stream */
                        Assert.Equal(streamID, requestMsg.StreamId());
                        Assert.Equal(domainType, requestMsg.DomainType());
                        Assert.True(requestMsg.HasServiceId);
                        Assert.Equal((int)serviceID, requestMsg.ServiceId());
                        Assert.True(requestMsg.HasServiceName);
                        Assert.Equal(serviceName, requestMsg.ServiceName());
                        Assert.True(requestMsg.HasName);
                        Assert.Equal(itemName, requestMsg.Name());
                        Assert.False(requestMsg.InterestAfterRefresh());
                    }
                };

                providerClient.ReissueMsgHandler = (requestMsg, providerEvent) =>
                {
                    /* Checks for the close message of the item domain */
                    if (requestMsg.DomainType() > EmaRdm.MMT_DIRECTORY)
                    {
                        Assert.Equal(itemHandle, providerEvent.Handle);

                        /* Checks the close message of the first item stream */
                        Assert.Equal(streamID, requestMsg.StreamId());
                        Assert.Equal(domainType, requestMsg.DomainType());
                        Assert.True(requestMsg.HasServiceId);
                        Assert.Equal((int)serviceID2, requestMsg.ServiceId());
                        Assert.True(requestMsg.HasServiceName);
                        Assert.Equal(serviceName2, requestMsg.ServiceName());
                        Assert.True(requestMsg.HasName);
                        Assert.Equal(itemName, requestMsg.Name());
                        Assert.True(requestMsg.InterestAfterRefresh());
                    }
                };

                SimpleConsumerTest simpleConsumer = new(m_Output);
                ReactorOptions reactorOption = new();

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, connectOptions, true, true);


                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                /* Checks that the provider receives both login and directory requests*/
                Assert.Equal(2, providerClient.ReceivedOnReqMsg);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                simpleConsumer.SubmitItemRequest(streamID, (int)serviceID, itemName, domainType, true, userDispatch ? true : false);

                /* Change the stream ID with the same item stream. */
                simpleConsumer.SubmitItemRequest(streamID, (int)serviceID2, itemName, domainType, true, userDispatch ? true : false);

                if (userDispatch)
                {
                    for (int i = 0; i < 4; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(4000); // Wait for the provider to accept the client connection.
                }

                if (acceptServiceNameChange == false)
                {
                    /* Checks response from item request */
                    Assert.Single(simpleConsumer.EventQueue);
                    reactorEvent = simpleConsumer.EventQueue.Dequeue();
                    {
                        ReactorMsgEvent reactorMsgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;

                        Assert.NotNull(reactorMsgEvent.Msg);
                        Assert.Equal(MsgClasses.STATUS, reactorMsgEvent.Msg!.MsgClass);

                        IStatusMsg statusMsg = (IStatusMsg)reactorMsgEvent.Msg!;
                        Assert.Equal(streamID, statusMsg.StreamId);

                        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, statusMsg.DomainType);
                        Assert.Equal(DataTypes.NO_DATA, statusMsg.ContainerType);

                        if (userDispatch)
                            Assert.True(statusMsg.CheckPrivateStream());
                        else
                            Assert.False(statusMsg.CheckPrivateStream());

                        Assert.True(statusMsg.CheckHasState());
                        Assert.Equal(StreamStates.CLOSED_RECOVER, statusMsg.State.StreamState());
                        Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());
                        Assert.Equal(StateCodes.USAGE_ERROR, statusMsg.State.Code());
                        Assert.Equal($"Request Message rejected - Attempt to reissue the service Id from {serviceID} to {serviceID2} while" +
                            $" this is not supported.",
                          statusMsg.State.Text().ToString());

                        Assert.False(statusMsg.CheckHasMsgKey());

                        /* Ensure that the provider receives the first item request */
                        Assert.Equal(3, providerClient.ReceivedOnReqMsg);

                        Assert.Equal(1, numOfItemRequests);

                        /* Ensure that the application receive the OnClose callback method. */
                        Assert.Equal(1, providerClient.ReceivedOnClose);
                    }
                }
                else
                {
                    /* Ensure that the provider receives the first and second item requests */
                    Assert.Equal(3, providerClient.ReceivedOnReqMsg);

                    Assert.Equal(1, numOfItemRequests);

                    Assert.Equal(0, providerClient.ReceivedOnClose);

                    /* Ensure that the application receive the OnReissueMsg callback method. */
                    Assert.Equal(1, providerClient.ReceivedOnReissueMsg);
                }

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void ProviderSendCloseMarketItemTest(bool userDispatch)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19011";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;

            /* Expected test parameters */
            ulong serviceID = 1;
            string serviceName = "DIRECT_FEED";
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int streamID = 5;
            long itemHandle = 0;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                   .OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                       : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:
                            var payload = new Map();
                            var service = new FilterList();
                            service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName)
                                                 .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                        .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                        .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_QOS, new OmmArray()
                                                        .AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK).MarkForClear().Complete())
                                                 .MarkForClear().Complete())
                            .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                                .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1).MarkForClear().Complete())
                            .MarkForClear().Complete();

                            payload.AddKeyUInt(serviceID, MapAction.ADD, service).MarkForClear().Complete();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                                .MarkForClear().Complete(true).Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                            break;
                        case (int)DomainType.MARKET_PRICE:

                            itemHandle = providerEvent.Handle;

                            Assert.Equal(streamID, requestMsg.StreamId());
                            Assert.Equal(domainType, requestMsg.DomainType());
                            Assert.True(requestMsg.HasServiceId);
                            Assert.Equal((int)serviceID, requestMsg.ServiceId());
                            Assert.True(requestMsg.HasServiceName);
                            Assert.Equal(serviceName, requestMsg.ServiceName());
                            Assert.True(requestMsg.HasName);
                            Assert.Equal(itemName, requestMsg.Name());

                            /* Send refresh message to the consumer */
                            provider.Submit(new RefreshMsg().ServiceName(serviceName).Name(itemName).DomainType(domainType)
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Solicited Refresh Completed")
                            .MarkForClear().Payload(new FieldList()
                            .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                            .AddUInt(3, 55)
                            .AddAscii(967, "ascii value")
                            .AddTime(375, 4, 15, 59, 100, 200, 300)
                            .MarkForClear().Complete())
                            .MarkForClear().Complete(true).ClearCache(true).Solicited(true), providerEvent.Handle);

                            break;
                        default:
                            break;
                    }

                };

                providerClient.CloseHandler = (requestMsg, providerEvent) =>
                {
                    /* Checks for the close message of the item domain */
                    if (requestMsg.DomainType() > EmaRdm.MMT_DIRECTORY)
                    {
                        Assert.Equal(itemHandle, providerEvent.Handle);

                        /* Checks the close message of the item stream */
                        Assert.Equal(streamID, requestMsg.StreamId());
                        Assert.Equal(domainType, requestMsg.DomainType());
                        Assert.True(requestMsg.HasServiceId);
                        Assert.Equal((int)serviceID, requestMsg.ServiceId());
                        Assert.True(requestMsg.HasServiceName);
                        Assert.Equal(serviceName, requestMsg.ServiceName());
                        Assert.True(requestMsg.HasName);
                        Assert.Equal(itemName, requestMsg.Name());
                        Assert.False(requestMsg.InterestAfterRefresh());
                    }
                };

                SimpleConsumerTest simpleConsumer = new(m_Output);
                ReactorOptions reactorOption = new();

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, connectOptions, true, true);


                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                /* Checks that the provider receives both login and directory requests*/
                Assert.Equal(2, providerClient.ReceivedOnReqMsg);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                simpleConsumer.SubmitItemRequest(streamID, (int)serviceID, itemName, domainType, true, userDispatch);

                if (userDispatch)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(2000); // Wait for the provider to accept the client connection.
                }

                /* Ensure that the provider receives the first item request */
                Assert.Equal(3, providerClient.ReceivedOnReqMsg);

                simpleConsumer.SubmitItemCloseRequest(streamID, domainType);

                if (userDispatch)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(2000); // Wait for the provider to accept the client connection.
                }

                /* Checks to ensure that the provider receives the item close message */
                Assert.Equal(1, providerClient.ReceivedOnClose);

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void ProviderSendGenericMsgOnMarketItemTest(bool userDispatch)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19012";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;

            /* Expected test parameters */
            ulong serviceID = 1;
            string serviceName = "DIRECT_FEED";
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int genMsgDomainType = 200;
            int genServiceId = 5;
            string genItemName = "ITEM_NAME";
            int streamID = 5;
            long itemHandle = 0;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                   .OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                       : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:
                            var payload = new Map();
                            var service = new FilterList();
                            service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName)
                                                 .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                        .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                        .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_QOS, new OmmArray()
                                                        .AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK).MarkForClear().Complete())
                                                 .MarkForClear().Complete())
                            .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                                .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1).MarkForClear().Complete())
                            .MarkForClear().Complete();

                            payload.AddKeyUInt(serviceID, MapAction.ADD, service).MarkForClear().Complete();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                                .MarkForClear().Complete(true).Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                            break;
                        case (int)DomainType.MARKET_PRICE:

                            itemHandle = providerEvent.Handle;

                            Assert.Equal(streamID, requestMsg.StreamId());
                            Assert.Equal(domainType, requestMsg.DomainType());
                            Assert.True(requestMsg.HasServiceId);
                            Assert.Equal((int)serviceID, requestMsg.ServiceId());
                            Assert.True(requestMsg.HasServiceName);
                            Assert.Equal(serviceName, requestMsg.ServiceName());
                            Assert.True(requestMsg.HasName);
                            Assert.Equal(itemName, requestMsg.Name());

                            /* Send refresh message to the consumer */
                            provider.Submit(new RefreshMsg().ServiceName(serviceName).Name(itemName).DomainType(domainType)
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Solicited Refresh Completed")
                            .MarkForClear().Payload(new FieldList()
                            .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                            .AddUInt(3, 55)
                            .AddAscii(967, "ascii value")
                            .AddTime(375, 4, 15, 59, 100, 200, 300)
                            .MarkForClear().Complete())
                            .MarkForClear().Complete(true).ClearCache(true).Solicited(true), providerEvent.Handle);

                            break;
                        default:
                            break;
                    }

                };

                providerClient.GenericMsgHandler = (genericMsg, providerEvent) =>
                {
                    Assert.Equal(itemHandle, providerEvent.Handle);

                    /* Checks the generic message on the item stream */
                    Assert.Equal(streamID, genericMsg.StreamId());
                    Assert.Equal(genMsgDomainType, genericMsg.DomainType());
                    Assert.True(genericMsg.HasServiceId);
                    Assert.Equal(genServiceId, genericMsg.ServiceId());
                    Assert.True(genericMsg.HasName);
                    Assert.Equal(genItemName, genericMsg.Name());
                    Assert.True(genericMsg.HasNameType);
                    Assert.Equal(EmaRdm.INSTRUMENT_NAME_RIC, genericMsg.NameType());
                };

                SimpleConsumerTest simpleConsumer = new(m_Output);
                ReactorOptions reactorOption = new();

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, connectOptions, true, true);


                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                /* Checks that the provider receives both login and directory requests*/
                Assert.Equal(2, providerClient.ReceivedOnReqMsg);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                simpleConsumer.SubmitItemRequest(streamID, (int)serviceID, itemName, domainType, true, userDispatch);

                if (userDispatch)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(2000); // Wait for the provider to accept the client connection.
                }

                /* Ensure that the provider receives the first item request */
                Assert.Equal(3, providerClient.ReceivedOnReqMsg);

                simpleConsumer.SubmitGenericMsgOnStream(streamID, genServiceId, genItemName, genMsgDomainType);

                if (userDispatch)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(2000); // Wait for the provider to accept the client connection.
                }

                /* Checks to ensure that the provider receives the item close message */
                Assert.Equal(1, providerClient.ReceivedOnGeneric);

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Theory]
        [InlineData(false, false)]
        [InlineData(true, true)]
        public void ProviderSendPostMsgOnMarketItemTest(bool userDispatch, bool enforceAckID)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19013";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;

            /* Expected test parameters */
            ulong serviceID = 1;
            string serviceName = "DIRECT_FEED";
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int postMsgDomainType = 200;
            int postServiceId = 5;
            string postItemName = "ITEM_NAME";
            long postID = 5;
            int streamID = 5;
            long itemHandle = 0;
            long invalidAckID = 999;
            string? submitInvalidAckIDMsg = null;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                   .OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                       : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                ommServerBaseImpl.MarketItemHandler.EnforceAckIDValidation = enforceAckID;

                OmmIProviderImpl ommIProviderImpl = (OmmIProviderImpl)ommServerBaseImpl;

                ommIProviderImpl.EnforceAckIDValidation = enforceAckID;

                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:
                            var payload = new Map();
                            var service = new FilterList();
                            service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName)
                                                 .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                        .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                        .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_QOS, new OmmArray()
                                                        .AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK).MarkForClear().Complete())
                                                 .MarkForClear().Complete())
                            .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                                .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1).MarkForClear().Complete())
                            .MarkForClear().Complete();

                            payload.AddKeyUInt(serviceID, MapAction.ADD, service).MarkForClear().Complete();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                                .MarkForClear().Complete(true).Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                            break;
                        case (int)DomainType.MARKET_PRICE:

                            itemHandle = providerEvent.Handle;

                            Assert.Equal(streamID, requestMsg.StreamId());
                            Assert.Equal(domainType, requestMsg.DomainType());
                            Assert.True(requestMsg.HasServiceId);
                            Assert.Equal((int)serviceID, requestMsg.ServiceId());
                            Assert.True(requestMsg.HasServiceName);
                            Assert.Equal(serviceName, requestMsg.ServiceName());
                            Assert.True(requestMsg.HasName);
                            Assert.Equal(itemName, requestMsg.Name());

                            /* Send refresh message to the consumer */
                            provider.Submit(new RefreshMsg().ServiceName(serviceName).Name(itemName).DomainType(domainType)
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Solicited Refresh Completed")
                            .MarkForClear().Payload(new FieldList()
                            .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                            .AddUInt(3, 55)
                            .AddAscii(967, "ascii value")
                            .AddTime(375, 4, 15, 59, 100, 200, 300)
                            .MarkForClear().Complete())
                            .MarkForClear().Complete(true).ClearCache(true).Solicited(true), providerEvent.Handle);

                            break;
                        default:
                            break;
                    }

                };

                providerClient.PostMsgHandler = (postMsg, providerEvent) =>
                {
                    Assert.Equal(itemHandle, providerEvent.Handle);

                    /* Checks the generic message on the item stream */
                    Assert.Equal(streamID, postMsg.StreamId());
                    Assert.Equal(postMsgDomainType, postMsg.DomainType());
                    Assert.True(postMsg.HasServiceId);
                    Assert.Equal(postServiceId, postMsg.ServiceId());
                    Assert.True(postMsg.HasName);
                    Assert.Equal(postItemName, postMsg.Name());
                    Assert.True(postMsg.HasNameType);
                    Assert.Equal(EmaRdm.INSTRUMENT_NAME_RIC, postMsg.NameType());
                    Assert.Equal(postID, postMsg.PostId());

                    try
                    {
                        /* Submit with invalid ack ID 999 in order to get exception */
                        providerEvent.Provider.Submit(new AckMsg().AckId(enforceAckID ? invalidAckID : postID).NackCode(NakCodes.NONE).Text("Acknowledge"), providerEvent.Handle);
                    }
                    catch (Exception ex)
                    {
                        submitInvalidAckIDMsg = ex.Message;
                    }
                };

                SimpleConsumerTest simpleConsumer = new(m_Output);
                ReactorOptions reactorOption = new();

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, connectOptions, true, true);


                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                /* Checks that the provider receives both login and directory requests*/
                Assert.Equal(2, providerClient.ReceivedOnReqMsg);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                simpleConsumer.SubmitItemRequest(streamID, (int)serviceID, itemName, domainType, true, userDispatch);

                if (userDispatch)
                {
                    for (int i = 0; i < 2; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(2000); // Wait for the provider to accept the client connection.
                }

                /* Ensure that the provider receives the first item request */
                Assert.Equal(3, providerClient.ReceivedOnReqMsg);

                /* Consumer receives the item response message */
                Assert.Single(simpleConsumer.EventQueue);
                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.MSG, reactorEvent.EventType);

                simpleConsumer.SubmitPostMsgOnStream(streamID, postServiceId, postItemName, postMsgDomainType, postID, true);

                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                /* Checks to ensure that the provider receives the item close message */
                Assert.Equal(1, providerClient.ReceivedOnPostMsg);

                if (enforceAckID)
                {
                    Assert.NotNull(submitInvalidAckIDMsg);
                    Assert.Equal($"Attempt to submit AckMsg with non existent AckId = {invalidAckID}. Handle = {itemHandle}", submitInvalidAckIDMsg);
                }
                else
                {
                    /* Check that consumer side receives the ACK message */
                    Assert.Single(simpleConsumer.EventQueue);
                }

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void ProviderFanoutMultiConnectionsSingleMarketItemTest(bool userDispatch)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string serverPort = "19014";

            /* Expected test parameters */
            ulong serviceID = 1;
            string serviceName = "DIRECT_FEED";
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int streamID = 5;
            int numDictRequest = 0;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(serverPort).OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                        : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:

                            numDictRequest++;

                            if (numDictRequest == 2)
                            {
                                var payload = new Map();
                                var service = new FilterList();
                                service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                    new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName)
                                                     .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                            .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                            .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                     .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                            .AddAscii("RWFFld")
                                                            .AddAscii("RWFEnum").MarkForClear().Complete())
                                                     .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                            .AddAscii("RWFFld")
                                                            .AddAscii("RWFEnum").MarkForClear().Complete())
                                                     .MarkForClear().Complete())
                                .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                    new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                                    .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1).MarkForClear().Complete())
                                .MarkForClear().Complete();

                                payload.AddKeyUInt(serviceID, MapAction.ADD, service).MarkForClear().Complete();

                                provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                    .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                                    .MarkForClear().Complete(true).Solicited(true).MarkForClear().Payload(payload), 0); // Submit to all consumers
                            }
                            break;
                        case (int)DomainType.MARKET_PRICE:

                            /* Checks the requset message from the consumer side. */
                            Assert.Equal(streamID, requestMsg.StreamId());
                            Assert.Equal(domainType, requestMsg.DomainType());
                            Assert.True(requestMsg.HasServiceId);
                            Assert.Equal((int)serviceID, requestMsg.ServiceId());
                            Assert.True(requestMsg.HasServiceName);
                            Assert.Equal(serviceName, requestMsg.ServiceName());
                            Assert.True(requestMsg.HasName);
                            Assert.Equal(itemName, requestMsg.Name());

                            /* Send refresh message to the consumer */
                            provider.Submit(new RefreshMsg().ServiceName(serviceName).Name(itemName).DomainType(domainType)
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Solicited Refresh Completed")
                            .MarkForClear().Payload(new FieldList()
                            .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                            .AddUInt(3, 55)
                            .AddAscii(967, "ascii value")
                            .AddTime(375, 4, 15, 59, 100, 200, 300)
                            .MarkForClear().Complete())
                            .MarkForClear().Complete(true).ClearCache(true).Solicited(true), providerEvent.Handle);

                            break;
                        default:
                            break;
                    }

                };

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                SimpleConsumerTest[] simpleConsumerArray = { new(m_Output, serverPort), new(m_Output, serverPort) };

                simpleConsumerArray[0].Initialize(new ReactorOptions(), null, true, true);

                simpleConsumerArray[1].Initialize(new ReactorOptions(), null, true, true);

                if (userDispatch)
                {
                    for (int i = 0; i < 10; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(10000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, providerClient.ReceivedOnReqMsg);
                Assert.Equal(2, serverBaseImpl.ConnectedChannelList.Count);

                foreach (var simpleConsumer in simpleConsumerArray)
                {
                    Assert.Equal(4, simpleConsumer.EventQueue.Count);

                    TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                    ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                    Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                    reactorEvent = simpleConsumer.EventQueue.Dequeue();
                    Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                    reactorEvent = simpleConsumer.EventQueue.Dequeue();
                    Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                    // Checks directory response from the provider side
                    {
                        RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)reactorEvent.ReactorEvent;

                        Assert.NotNull(directoryMsgEvent.Msg);

                        /* Checks the complete flag */
                        Assert.True((directoryMsgEvent.Msg!.Flags & RefreshMsgFlags.REFRESH_COMPLETE) != 0);

                        Assert.NotNull(directoryMsgEvent.DirectoryMsg);
                        Assert.Equal(DirectoryMsgType.REFRESH, directoryMsgEvent.DirectoryMsg!.DirectoryMsgType);

                        Assert.NotNull(directoryMsgEvent.DirectoryMsg.DirectoryRefresh);

                        var directoryRefresh = directoryMsgEvent.DirectoryMsg.DirectoryRefresh!;

                        Assert.Equal(StreamStates.OPEN, directoryRefresh.State.StreamState());
                        Assert.Equal(DataStates.OK, directoryRefresh.State.DataState());
                        Assert.Equal(StateCodes.NONE, directoryRefresh.State.Code());
                        Assert.Equal("", directoryRefresh.State.Text().ToString());

                        Assert.True(directoryRefresh.Solicited);

                        Assert.Equal(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER, directoryRefresh.Filter);

                        Assert.False(directoryRefresh.HasServiceId);
                        Assert.Single(directoryRefresh.ServiceList);

                        var service = directoryRefresh.ServiceList[0];

                        Assert.Equal(1, service.ServiceId);

                        Assert.True(service.HasInfo);

                        /* Checks service Info */
                        {
                            var serviceInfo = service.Info;
                            Assert.Equal("DIRECT_FEED", serviceInfo.ServiceName.ToString());
                            Assert.Equal(2, serviceInfo.CapabilitiesList.Count);
                            Assert.Equal(EmaRdm.MMT_MARKET_PRICE, serviceInfo.CapabilitiesList[0]);
                            Assert.Equal(EmaRdm.MMT_MARKET_BY_PRICE, serviceInfo.CapabilitiesList[1]);

                            Assert.Equal(2, serviceInfo.DictionariesUsedList.Count);
                            Assert.Equal("RWFFld", serviceInfo.DictionariesUsedList[0]);
                            Assert.Equal("RWFEnum", serviceInfo.DictionariesUsedList[1]);
                        }

                        Assert.True(service.HasState);
                        /* Checks service state */
                        {
                            var serviceState = service.State;
                            Assert.Equal(EmaRdm.SERVICE_UP, serviceState.ServiceStateVal);
                        }

                    }

                    reactorEvent = simpleConsumer.EventQueue.Dequeue();
                    channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                    Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                    simpleConsumer.SubmitItemRequest(streamID, (int)serviceID, itemName, domainType);
                }

                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                foreach (var simpleConsumer in simpleConsumerArray)
                {
                    /* Checks response from item request */
                    Assert.Single(simpleConsumer.EventQueue);
                    TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                    {
                        ReactorMsgEvent reactorMsgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;

                        Assert.NotNull(reactorMsgEvent.Msg);
                        Assert.Equal(MsgClasses.REFRESH, reactorMsgEvent.Msg!.MsgClass);

                        IRefreshMsg refreshMsg = (IRefreshMsg)reactorMsgEvent.Msg!;

                        Assert.Equal(streamID, refreshMsg.StreamId);
                        Assert.True(refreshMsg.CheckClearCache());
                        Assert.True(refreshMsg.CheckRefreshComplete());

                        Assert.Equal(EmaRdm.MMT_MARKET_PRICE, refreshMsg.DomainType);

                        Assert.True(refreshMsg.MsgKey.CheckHasServiceId());
                        Assert.Equal((int)serviceID, refreshMsg.MsgKey.ServiceId);

                        Assert.True(refreshMsg.MsgKey.CheckHasName());
                        Assert.Equal(itemName, refreshMsg.MsgKey.Name.ToString());

                        Assert.Equal(DataTypes.FIELD_LIST, refreshMsg.ContainerType);

                        DecodeIterator decodeIterator = new DecodeIterator();

                        decodeIterator.SetBufferAndRWFVersion(refreshMsg.EncodedDataBody, reactorMsgEvent.ReactorChannel!.MajorVersion,
                            reactorMsgEvent.ReactorChannel.MinorVersion);

                        Eta.Codec.FieldList fieldList = new();
                        Assert.Equal(CodecReturnCode.SUCCESS, fieldList.Decode(decodeIterator, null));

                        Eta.Codec.FieldEntry fieldEntry = new();

                        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(decodeIterator));
                        Assert.Equal(22, fieldEntry.FieldId);
                        Real realValue = new();
                        Assert.Equal(CodecReturnCode.SUCCESS, realValue.Decode(decodeIterator));
                        Assert.Equal(3990, realValue.ToLong());
                        Assert.Equal(OmmReal.MagnitudeTypes.EXPONENT_NEG_2, realValue.Hint);

                        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(decodeIterator));
                        Assert.Equal(3, fieldEntry.FieldId);
                        UInt uInt = new();
                        Assert.Equal(CodecReturnCode.SUCCESS, uInt.Decode(decodeIterator));
                        Assert.Equal<ulong>(55, uInt.ToULong());

                        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(decodeIterator));
                        Assert.Equal(967, fieldEntry.FieldId);
                        Buffer asciiBuffer = new();
                        Assert.Equal(CodecReturnCode.SUCCESS, asciiBuffer.Decode(decodeIterator));
                        Assert.Equal("ascii value", asciiBuffer.ToString());

                        Assert.Equal(CodecReturnCode.SUCCESS, fieldEntry.Decode(decodeIterator));
                        Assert.Equal(375, fieldEntry.FieldId);
                        Time time = new Time();
                        Assert.Equal(CodecReturnCode.SUCCESS, time.Decode(decodeIterator));
                        Assert.Equal(4, time.Hour());
                        Assert.Equal(15, time.Minute());
                        Assert.Equal(59, time.Second());
                        Assert.Equal(100, time.Millisecond());
                        Assert.Equal(200, time.Microsecond());
                        Assert.Equal(300, time.Nanosecond());

                        Assert.Equal(CodecReturnCode.END_OF_CONTAINER, fieldEntry.Decode(decodeIterator));
                    }

                    simpleConsumer.UnInitialize();
                }

                if (userDispatch)
                {
                    for (int i = 0; i < 8; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                    Thread.Sleep(8000); // Removes the client connections

                Assert.Empty(serverBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();

        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void ProviderSendStatusMsgOnMarketItemTest(bool userDispatch)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19015";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;

            /* Expected test parameters */
            ulong serviceID = 1;
            string serviceName = "DIRECT_FEED";
            string itemName = "LSEG.L";
            int domainType = EmaRdm.MMT_MARKET_PRICE;
            int streamID = 5;
            long itemHandle = 0;

            try
            {
                provider = new OmmProvider(config.AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL)
                    .Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName)
                   .OperationModel(userDispatch ? OmmIProviderConfig.OperationModelMode.USER_DISPATCH
                       : OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);
                            break;
                        case (int)DomainType.SOURCE:
                            var payload = new Map();
                            var service = new FilterList();
                            service.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET,
                                new ElementList().AddAscii(EmaRdm.ENAME_NAME, serviceName)
                                                 .AddArray(EmaRdm.ENAME_CAPABILITIES, new OmmArray()
                                                        .AddUInt(EmaRdm.MMT_MARKET_PRICE)
                                                        .AddUInt(EmaRdm.MMT_MARKET_BY_PRICE).MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_USED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_DICTIONARYS_PROVIDED, new OmmArray()
                                                        .AddAscii("RWFFld")
                                                        .AddAscii("RWFEnum").MarkForClear().Complete())
                                                 .AddArray(EmaRdm.ENAME_QOS, new OmmArray()
                                                        .AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK).MarkForClear().Complete())
                                                 .MarkForClear().Complete())
                            .AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET,
                                new ElementList().AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP)
                                .AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1).MarkForClear().Complete())
                            .MarkForClear().Complete();

                            payload.AddKeyUInt(serviceID, MapAction.ADD, service).MarkForClear().Complete();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
                                .MarkForClear().Complete(true).Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                            break;
                        case (int)DomainType.MARKET_PRICE:

                            itemHandle = providerEvent.Handle;

                            Assert.Fail("Request message for the market price doamin is not expected.");

                            break;
                        default:
                            break;
                    }

                };

                SimpleConsumerTest simpleConsumer = new(m_Output);
                ReactorOptions reactorOption = new();

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                simpleConsumer.Initialize(reactorOption, connectOptions, true, true);


                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                TestReactorEvent reactorEvent = simpleConsumer.EventQueue.Dequeue();
                ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                Assert.Equal(TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
                Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

                /* Checks that the provider receives both login and directory requests*/
                Assert.Equal(2, providerClient.ReceivedOnReqMsg);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                simpleConsumer.SubmitItemStatusMsg(streamID, (int)serviceID, itemName, domainType);

                if (userDispatch)
                {
                    for (int i = 0; i < 5; i++)
                    {
                        provider.Dispatch(1000000);
                    }
                }
                else
                {
                    Thread.Sleep(5000); // Wait for the provider to accept the client connection.
                }

                /* Checks response from item request */
                Assert.Single(simpleConsumer.EventQueue);
                reactorEvent = simpleConsumer.EventQueue.Dequeue();
                {
                    ReactorMsgEvent reactorMsgEvent = (ReactorMsgEvent)reactorEvent.ReactorEvent;

                    Assert.NotNull(reactorMsgEvent.Msg);
                    Assert.Equal(MsgClasses.STATUS, reactorMsgEvent.Msg!.MsgClass);

                    IStatusMsg statusMsg = (IStatusMsg)reactorMsgEvent.Msg!;
                    Assert.Equal(streamID, statusMsg.StreamId);

                    Assert.Equal(EmaRdm.MMT_MARKET_PRICE, statusMsg.DomainType);
                    Assert.Equal(DataTypes.NO_DATA, statusMsg.ContainerType);


                    Assert.False(statusMsg.CheckPrivateStream());

                    Assert.True(statusMsg.CheckHasState());
                    Assert.Equal(StreamStates.CLOSED_RECOVER, statusMsg.State.StreamState());
                    Assert.Equal(DataStates.SUSPECT, statusMsg.State.DataState());
                    Assert.Equal(StateCodes.USAGE_ERROR, statusMsg.State.Code());
                    Assert.Equal($"Rejected unhandled message type STATUS",
                      statusMsg.State.Text().ToString());

                    Assert.False(statusMsg.CheckHasMsgKey());

                    /* Ensure that the application doesn't receive the OnClose callback method. */
                    Assert.Equal(0, providerClient.ReceivedOnClose);
                }

                /* Ensure that the provider receives the first item request */
                Assert.Equal(0, providerClient.ReceivedOnStatus);

                simpleConsumer.UnInitialize();

                if (userDispatch)
                {
                    provider.Dispatch(DispatchTimeout.INFINITE_WAIT);
                }
                else
                    Thread.Sleep(3000); // Removes the client connection

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        class OmmProviderErrorClientTest : IOmmProviderErrorClient
        {
            public List<OmmInvalidHandleException> InvalidHandleExceptionList { get; private set; } = new();

            public List<OmmInvalidUsageException> InvalidUsageExceptionList { get; private set; } = new();

            public void OnInvalidHandle(long handle, string text)
            {
                InvalidHandleExceptionList.Add(new OmmInvalidHandleException(handle, text));
            }

            public void OnInvalidUsage(string text, int errorCode)
            {
                InvalidUsageExceptionList.Add(new OmmInvalidUsageException(text, errorCode));
            }
        }

        class OmmProviderClientTest : IOmmProviderClient
        {
            void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent) { }

            void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent) { }
        };

        [Fact]
        public void ProviderNotifyWithProviderErrorClientTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            connectInfo.ConnectOptions.UnifiedNetworkInfo.Address = "localhost";
            connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName = "19016";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;
            OmmProviderErrorClientTest providerErrorClient = new OmmProviderErrorClientTest();

            long invalidHandle = 100;

            try
            {
                provider = new OmmProvider(config.Port(connectInfo.ConnectOptions.UnifiedNetworkInfo.ServiceName), providerClient, providerErrorClient, null);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                long handle = provider.RegisterClient(null!, null!);

                Assert.Equal(0, handle);

                provider.Submit(new RefreshMsg(), invalidHandle);

                provider.Submit(new UpdateMsg(), invalidHandle);

                provider.Submit(new StatusMsg(), invalidHandle);

                provider.Submit(new GenericMsg(), invalidHandle);

                provider.Submit(new AckMsg(), invalidHandle);

                handle = provider.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_MARKET_PRICE), new OmmProviderClientTest());

                Assert.Equal(0, handle);

                provider.Reissue(new RequestMsg().DomainType(EmaRdm.MMT_MARKET_PRICE), invalidHandle);

                /* Checks OmmInvalidUsageException */
                Assert.True(providerErrorClient.InvalidUsageExceptionList.Count == 3);
                Assert.Equal("A derived class of IOmmProviderClient is not set",
                    providerErrorClient.InvalidUsageExceptionList[0].Message);
                Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT,
                    providerErrorClient.InvalidUsageExceptionList[0].ErrorCode);

                Assert.Equal("OMM Interactive provider supports registering DICTIONARY domain type only.",
                    providerErrorClient.InvalidUsageExceptionList[1].Message);
                Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT,
                    providerErrorClient.InvalidUsageExceptionList[1].ErrorCode);

                Assert.Equal("OMM Interactive provider supports reissuing DICTIONARY domain type only.",
                    providerErrorClient.InvalidUsageExceptionList[2].Message);
                Assert.Equal(OmmInvalidUsageException.ErrorCodes.INVALID_ARGUMENT,
                    providerErrorClient.InvalidUsageExceptionList[2].ErrorCode);

                /* Checks OmmInvalidHandleException */
                Assert.True(providerErrorClient.InvalidHandleExceptionList.Count == 5);
                Assert.Equal(invalidHandle, providerErrorClient.InvalidHandleExceptionList[0].Handle);
                Assert.Equal("Attempt to submit RefreshMsg with non existent Handle = 100.",
                    providerErrorClient.InvalidHandleExceptionList[0].Message);

                Assert.Equal(invalidHandle, providerErrorClient.InvalidHandleExceptionList[1].Handle);
                Assert.Equal("Attempt to submit UpdateMsg with non existent Handle = 100.",
                    providerErrorClient.InvalidHandleExceptionList[1].Message);

                Assert.Equal(invalidHandle, providerErrorClient.InvalidHandleExceptionList[2].Handle);
                Assert.Equal("Attempt to submit StatusMsg with non existent Handle = 100.",
                    providerErrorClient.InvalidHandleExceptionList[2].Message);

                Assert.Equal(invalidHandle, providerErrorClient.InvalidHandleExceptionList[3].Handle);
                Assert.Equal("Attempt to submit GenericMsg with non existent Handle = 100.",
                    providerErrorClient.InvalidHandleExceptionList[3].Message);

                Assert.Equal(invalidHandle, providerErrorClient.InvalidHandleExceptionList[4].Handle);
                Assert.Equal("Attempt to submit AckMsg with non existent Handle = 100.",
                    providerErrorClient.InvalidHandleExceptionList[4].Message);


                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }

        [Fact]
        public void ProviderCloseLoginAndLogOutRequestFromMultipleOmmConsumerTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            /* Setup connection options */
            ReactorConnectOptions connectOptions = new();
            ReactorConnectInfo connectInfo = new();
            string serverAddress = "localhost";
            string serverPort = "19017";

            connectOptions.ConnectionList.Add(connectInfo);
            connectInfo.ConnectOptions.MajorVersion = Codec.MajorVersion();
            connectInfo.ConnectOptions.MinorVersion = Codec.MinorVersion();

            OmmServerBaseImpl ommServerBaseImpl;
            int loginInCount = 0;
            int loginOutCount = 0;
            List<ChannelInformation> channelInfoList = new List<ChannelInformation>();

            try
            {
                provider = new OmmProvider(
                    config.Port(serverPort)
                   .OperationModel(OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

                ommServerBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;

                string expecedLoginUser = "";

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            ElementList attrib = new();
                            attrib.AddAscii(EmaRdm.ENAME_APP_ID, "555");
                            attrib.AddAscii(EmaRdm.ENAME_APP_NAME, "ProviderSingleMarketItemTest");
                            attrib.AddAscii(EmaRdm.ENAME_POSITION, "localhost");
                            attrib.AddUInt(EmaRdm.ENAME_SINGLE_OPEN, 1);
                            attrib.AddUInt(EmaRdm.ENAME_ALLOW_SUSPECT_DATA, 1);
                            attrib.AddUInt(EmaRdm.ENAME_SUPPORT_POST, 1);

                            expecedLoginUser = requestMsg.Name();

                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType()).Name(requestMsg.Name()).NameType(Rdm.EmaRdm.USER_NAME).Attrib(attrib.MarkForClear().Complete())
                                .MarkForClear().Complete(true).Solicited(true).State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK,
                                OmmState.StatusCodes.NONE, "Login accepted"), providerEvent.Handle);

                            loginInCount++;

                            break;
                        default:
                            break;
                    }

                };

                providerClient.CloseHandler = (closeMsg, providerEvent) =>
                {
                    switch (closeMsg.DomainType())
                    {
                        case (int)DomainType.LOGIN:

                            loginOutCount++;

                            break;
                    };
                };

                OmmConsumerConfig consumerConfig = new();
                consumerConfig.Host($"{serverAddress}:{serverPort}");

                OmmConsumer ommConsumer = new OmmConsumer(consumerConfig);

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                provider?.ConnectedClientChannelInfo(channelInfoList);
                Assert.Single(channelInfoList);

                var channelInformation = channelInfoList[0];

                Assert.Contains("emacsharp", channelInformation.ComponentInfo);

                ommConsumer.Uninitialize();

                Thread.Sleep(2000); // Wait for the provider to close the client connection.

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);
                provider?.ConnectedClientChannelInfo(channelInfoList);

                Assert.Empty(channelInfoList);

                Assert.Equal(1, loginInCount);
                Assert.Equal(loginOutCount, loginInCount);

                ommConsumer = new OmmConsumer(consumerConfig);

                Assert.Single(ommServerBaseImpl.ConnectedChannelList);

                provider?.ConnectedClientChannelInfo(channelInfoList);
                Assert.Single(channelInfoList);

                channelInformation = channelInfoList[0];

                Assert.Contains("emacsharp", channelInformation.ComponentInfo);

                ommConsumer.Uninitialize();

                Thread.Sleep(2000); // Wait for the provider to close the client connection.

                Assert.Empty(ommServerBaseImpl.ConnectedChannelList);

                Assert.Equal(2, loginInCount);
                Assert.Equal(loginOutCount, loginInCount);

                provider?.ConnectedClientChannelInfo(channelInfoList);

                Assert.Empty(channelInfoList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }

            if (exception != null)
            {
                Assert.Fail("Caught OmmException: " + exception.Message);
            }
            Assert.NotNull(provider);
            provider?.Uninitialize();
        }
    }
}
