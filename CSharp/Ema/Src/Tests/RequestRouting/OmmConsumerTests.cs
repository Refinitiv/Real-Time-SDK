/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Rdm;
using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;
using Xunit.Abstractions;

namespace LSEG.Ema.Access.Tests.RequestRouting
{
    public class OmmConsumerTests
    {
        readonly ITestOutputHelper m_Output;

        private static readonly string EmaConfigFileLocation = "../../../RequestRouting/EmaConfigTest.xml";

        public OmmConsumerTests(ITestOutputHelper output)
        {
            this.m_Output = output;

        }

        [Fact]
        public void CreateAndClearServiceListTest()
        {
            ServiceList serviceList = new ServiceList("serviceList1");

            serviceList.ConcreteServiceList.Add("FEED_1");
            serviceList.ConcreteServiceList.Add("FEED_2");
            serviceList.ConcreteServiceList.Add("FEED_3");
            Assert.Equal("serviceList1", serviceList.Name);

            String expectedString = "Service list name: serviceList1"
                    + $"{NewLine}Concrete service names:"
                    + $"{NewLine}\tFEED_1"
                    + $"{NewLine}\tFEED_2"
                    + $"{NewLine}\tFEED_3";

            Assert.Equal(expectedString, serviceList.ToString());

            serviceList.Clear();

            expectedString = "Service list name: "
                    + $"{NewLine}Concrete service names:";

            Assert.Equal(expectedString, serviceList.ToString());

            serviceList.Name = "serviceList2";
            serviceList.ConcreteServiceList.Add("FEED_4");
            serviceList.ConcreteServiceList.Add("FEED_5");
            serviceList.ConcreteServiceList.Add("FEED_6");

            expectedString = "Service list name: serviceList2"
                    + $"{NewLine}Concrete service names:"
                    + $"{NewLine}\tFEED_4"
                    + $"{NewLine}\tFEED_5"
                    + $"{NewLine}\tFEED_6";

            Assert.Equal(expectedString, serviceList.ToString());
        }

        [Fact]
        public void AddEmptyOrNullServiceListNameTest()
        {
            ServiceList serviceList = new ServiceList("");

            OmmConsumerConfig consumerConfig = new OmmConsumerConfig();

            OmmInvalidUsageException expectedException = Assert.Throws<OmmInvalidUsageException>(()
                    => { consumerConfig.AddServiceList(serviceList); });

            Assert.StartsWith("The ServiceList's name must be non-empty string value.", expectedException.Message);

#pragma warning disable CS8625 // Cannot convert null literal to non-nullable reference type.
            serviceList.Name = null;
#pragma warning restore CS8625 // Cannot convert null literal to non-nullable reference type.

            expectedException = Assert.Throws<OmmInvalidUsageException>(() => { consumerConfig.AddServiceList(serviceList); });

            Assert.StartsWith("The ServiceList's name must be non-empty string value.", expectedException.Message);
        }

        [Fact]
        public void DuplicateServiceListNameTest()
        {
            ServiceList serviceList = new ServiceList("ServiceGroup");

            serviceList.ConcreteServiceList.Add("A");
            serviceList.ConcreteServiceList.Add("B");

            ServiceList serviceList2 = new ServiceList("ServiceGroup");

            serviceList.ConcreteServiceList.Add("C");
            serviceList.ConcreteServiceList.Add("D");

            OmmConsumerConfig consumerConfig = new OmmConsumerConfig();

            consumerConfig.AddServiceList(serviceList);

            OmmInvalidUsageException expectedException = Assert.Throws<OmmInvalidUsageException>(()
                => { consumerConfig.AddServiceList(serviceList2); });

            Assert.StartsWith("The ServiceGroup name of ServiceList has been added to OmmConsumerConfig.", expectedException.Message);
        }

        [Fact]
        public void SetandGetReqMsgWithServiceNameListTest()
        {
            RequestMsg reqMsg = new();

            string serviceListName = "ServiceList1";

            reqMsg.ServiceListName(serviceListName);

            Assert.True(reqMsg.HasServiceListName);

            Assert.Equal(serviceListName, reqMsg.ServiceListName());

            reqMsg.Clear();

            Assert.False(reqMsg.HasServiceListName);

            OmmInvalidUsageException expectedException = Assert.Throws<OmmInvalidUsageException>(() => { reqMsg.ServiceListName(); });

            Assert.StartsWith("Invalid attempt to call ServiceListName() while Service list name is not set.", expectedException.Message);
        }

        [Fact]
        public void SetServiceListNameAfterSettingServiceNameOrServiceIdTest()
        {
            RequestMsg reqMsg = new();

            string serviceListName = "ServiceList1";

            reqMsg.ServiceName("DIRECT_FEED");

            OmmInvalidUsageException expectedException = Assert.Throws<OmmInvalidUsageException>(() => reqMsg.ServiceListName(serviceListName));

            Assert.Equal("Service name is already set for this RequestMsg.", expectedException.Message);

            reqMsg.Clear();

            reqMsg.ServiceId(1);

            expectedException = Assert.Throws<OmmInvalidUsageException>(() => reqMsg.ServiceListName(serviceListName));

            Assert.Equal("Service Id is already set for this RequestMsg.", expectedException.Message);
        }

        [Fact]
        public void MultiConnectionsLoginReqTimeoutTest()
        {
            OmmConsumerConfig config = new(EmaConfigFileLocation);
            OmmConsumer? consumer = null;
            ConsumerTestOptions options = new ConsumerTestOptions();
            options.GetChannelInformation = true;

            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, options);

            try
            {
                OmmInvalidUsageException expectedException = Assert.Throws<OmmInvalidUsageException>(()
                    =>
                { consumer = new OmmConsumer(config.ConsumerName("Consumer_9"), consumerClient); });

                Assert.StartsWith("login failed (timed out after waiting 6500 milliseconds) for Connection_1, Connection_2", expectedException.Message);

                int queueSize = consumerClient.QueueSize();

                m_Output.WriteLine($"queueSize = {queueSize}");

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.State().ToString());

                ChannelInformation channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal("Connection_1", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.INITIALIZING, channelInfo.ChannelState);

                statusMsg = (StatusMsg)consumerClient.PopMessage();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.State().ToString());

                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_4", channelInfo.ChannelName);
                Assert.Equal("Connection_2", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.INITIALIZING, channelInfo.ChannelState);

                statusMsg = (StatusMsg)consumerClient.PopMessage();

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.State().ToString());

                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_2", channelInfo.ChannelName);
                Assert.Equal("Connection_1", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.INITIALIZING, channelInfo.ChannelState);

                if (queueSize == 4)
                {
                    statusMsg = (StatusMsg)consumerClient.PopMessage();
                    Assert.Equal(1, statusMsg.StreamId());
                    Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                    Assert.Equal("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.State().ToString());

                    channelInfo = consumerClient.PopChannelInfo();
                    Assert.Equal("Channel_5", channelInfo.ChannelName);
                    Assert.Equal("Connection_2", channelInfo.SessionChannelName);
                    Assert.Equal(ChannelState.INITIALIZING, channelInfo.ChannelState);
                }

                if (queueSize == 5)
                {
                    statusMsg = (StatusMsg)consumerClient.PopMessage();
                    Assert.Equal(1, statusMsg.StreamId());
                    Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                    Assert.Equal("Open / Suspect / None / 'session channel closed'", statusMsg.State().ToString());
                }
            }
            catch (Exception excep)
            {
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.Null(consumer);
            }
        }

        [Fact]
        public void MultiConnectionsCloseAConnectionTest()
        {
            OmmIProviderConfig providerConfig = new(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);

            OmmProvider ommProvider = new OmmProvider(providerConfig.Port("19001"), providerClient);

            ConsumerTestOptions options = new();
            options.GetChannelInformation = true;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, options);

            OmmConsumer? consumer = null;

            try
            {
                StatusMsg statusMsg;
                RefreshMsg refreshMsg;
                ChannelInformation channelInfo;

                // The Consumer_9 makes to connections.
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Assert.Equal(4, consumerClient.QueueSize()); // Receives status message for login stream state.
                Assert.Equal(4, consumerClient.ChannelInfoSize());

                statusMsg = (StatusMsg)consumerClient.PopMessage();
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal("Connection_1", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);

                statusMsg = (StatusMsg)consumerClient.PopMessage();
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.State().ToString());

                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_4", channelInfo.ChannelName);
                Assert.Equal("Connection_2", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.INITIALIZING, channelInfo.ChannelState);

                statusMsg = (StatusMsg)consumerClient.PopMessage();
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.State().ToString());

                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_5", channelInfo.ChannelName);
                Assert.Equal("Connection_2", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.INITIALIZING, channelInfo.ChannelState);

                refreshMsg = (RefreshMsg)consumerClient.PopMessage();
                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());

                channelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal("Connection_1", channelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);

                // Ensure that only one connect left
                Assert.Single(ommConsumerImpl.ConsumerSession!.SessionChannelList);

                Assert.Equal("Connection_1", ommConsumerImpl.ConsumerSession.SessionChannelList[0].SessionChannelConfig.Name);
            }
            catch (Exception excep)
            {
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);
                consumer.Uninitialize();
                ommProvider.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForLoginStreamOkTest()
        {
            OmmIProviderConfig config = new(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            OmmProvider provider = new(config.Port("19001"), providerClient);

            OmmProvider provider2 = new(config.Port("19004"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();

            consumerTestOptions.GetChannelInformation = true;

            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, consumerTestOptions);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"), consumerClient);

                OmmConsumerImpl ommConsumerImpl = consumer.m_OmmConsumerImpl!;

                Assert.Equal(3, consumerClient.QueueSize());
                Assert.Equal(3, consumerClient.ChannelInfoSize());

                string firstChannelName = ommConsumerImpl.ConsumerSession!.SessionChannelList[0].SessionChannelConfig.Name;

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                ChannelInformation chanelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_1", chanelInfo.ChannelName);
                Assert.Equal("Connection_1", chanelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, chanelInfo.ChannelState);

                statusMsg = (StatusMsg)consumerClient.PopMessage();
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                chanelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_4", chanelInfo.ChannelName);
                Assert.Equal("Connection_2", chanelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, chanelInfo.ChannelState);

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.True(refreshMsg.HasNameType);
                Assert.Equal(EmaRdm.USER_NAME, refreshMsg.NameType());
                Assert.Equal(DataType.DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);

                chanelInfo = consumerClient.PopChannelInfo();
                Assert.Equal("Channel_4", chanelInfo.ChannelName);
                Assert.Equal("Connection_2", chanelInfo.SessionChannelName);
                Assert.Equal(ChannelState.ACTIVE, chanelInfo.ChannelState);

                ElementList elementList = refreshMsg.Attrib().ElementList();

                foreach (var element in elementList)
                {
                    switch (element.Name)
                    {
                        case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                    }
                }
            }
            catch (OmmException ommException)
            {
                Assert.Fail(ommException.Message);
            }
            finally
            {
                Assert.NotNull(consumer);
                consumer.Uninitialize();
                provider.Uninitialize();
                provider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForLoginStreamViaRegisterClientTest()
        {
            OmmIProviderConfig config = new(EmaConfigFileLocation);
            OmmProvider ommProvider = new OmmProvider(config.Port("19001"),
                new ProviderTestClient(m_Output, new ProviderTestOptions()));
            OmmProvider ommProvider2 = new OmmProvider(config.Port("19004"),
                new ProviderTestClient(m_Output, new ProviderTestOptions()));

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                long loginHandle = consumer.RegisterClient(new RequestMsg().DomainType(EmaRdm.MMT_LOGIN), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one login message

                Msg message = consumerClient.PopMessage();
                Assert.True(message is RefreshMsg);
                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.True(refreshMsg.HasNameType);
                Assert.Equal(EmaRdm.USER_NAME, refreshMsg.NameType());
                Assert.Equal(DataType.DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);

                ElementList elementList = refreshMsg.Attrib().ElementList();
                foreach (var element in elementList)
                {
                    switch (element.Name)
                    {
                        case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                    }
                }

                consumer.Unregister(loginHandle);
            }
            catch (Exception ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                consumer.Uninitialize();
                ommProvider.Uninitialize();
                ommProvider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForLoginStreamDenyOnlyOneSessionTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions1 = new ProviderTestOptions();
            providerTestOptions1.AcceptLoginRequest = true;

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions1);
            OmmProvider ommprovider = new OmmProvider(config.Port("19001"), providerClient);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            providerTestOptions2.AcceptLoginRequest = false;

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"), consumerClient);

                Thread.Sleep(3000);

                Assert.Equal(4, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();
                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                statusMsg = (StatusMsg)consumerClient.PopMessage();
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                statusMsg = (StatusMsg)consumerClient.PopMessage();
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / Not entitled / 'Login denied'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();
                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;
                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.True(refreshMsg.HasNameType);
                Assert.Equal(EmaRdm.USER_NAME, refreshMsg.NameType());
                Assert.Equal(DataType.DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);

                ElementList elementList = refreshMsg.Attrib().ElementList();
                foreach (var element in elementList)
                {
                    switch (element.Name)
                    {
                        case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                    }
                }
            }
            catch (Exception execp)
            {
                Assert.Fail(execp.ToString());
            }
            finally
            {
                Assert.NotNull(consumer);

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForLoginStreamRespAfterOmmConsumerCreationTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions1 = new ProviderTestOptions();
            providerTestOptions1.AcceptLoginRequest = true;
            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions1);
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            providerTestOptions2.AcceptLoginRequest = true;
            providerTestOptions2.SendLoginResponseInMiliSecond = 3000;
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"), consumerClient);

                Assert.Equal(3, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();
                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                statusMsg = (StatusMsg)consumerClient.PopMessage();
                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;
                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.True(refreshMsg.HasNameType);
                Assert.Equal(EmaRdm.USER_NAME, refreshMsg.NameType());
                Assert.Equal(DataType.DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);

                ElementList elementList = refreshMsg.Attrib().ElementList();
                foreach (var element in elementList)
                {
                    switch (element.Name)
                    {
                        case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                    }
                }

                // Wait to receive login response from the second provider.
                Thread.Sleep(3000);

                Assert.Equal(0, consumerClient.QueueSize()); // Ensure that there is no more message

            }
            catch (Exception ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForLoginStreamAndForceLogoutFromProviderTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions1 = new ProviderTestOptions();
            providerTestOptions1.AcceptLoginRequest = true;
            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions1);
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            providerTestOptions2.AcceptLoginRequest = true;
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;

            ConsumerTestOptions options = new ConsumerTestOptions();
            options.GetChannelInformation = true;

            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, options);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"), consumerClient);

                Assert.Equal(3, consumerClient.QueueSize());
                Assert.Equal(3, consumerClient.ChannelInfoSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataType.DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);

                ElementList elementList = refreshMsg.Attrib().ElementList();
                foreach (var element in elementList)
                {
                    switch (element.Name)
                    {
                        case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                Assert.Equal<ulong>(1, element.UIntValue());
                                break;
                            }
                    }
                }

                // Wait to receive login response from the second provider.
                Thread.Sleep(3000);

                providerClient.ForceLogout();

                providerClient2.ForceLogout();

                Thread.Sleep(2000);

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / Not entitled / 'Force logout'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();
                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Closed / Suspect / Not entitled / 'Force logout'", statusMsg.State().ToString());
            }
            catch (Exception ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForDirectoryStreamWithSameServiceNameButDiffrentQoSTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            /* Source directory refresh for the first server */
            OmmArray capablities = new OmmArray();
            capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
            capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
            capablities.Complete();
            OmmArray dictionaryUsed = new OmmArray();
            dictionaryUsed.AddAscii("RWFFld");
            dictionaryUsed.AddAscii("RWFEnum");
            dictionaryUsed.Complete();

            OmmArray qosList = new OmmArray();
            qosList.AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK);
            qosList.AddQos(100, 100);
            qosList.Complete();

            ElementList serviceInfoId = new ElementList();
            serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
            serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
            serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
            serviceInfoId.AddArray(EmaRdm.ENAME_QOS, qosList);
            serviceInfoId.AddAscii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist");
            serviceInfoId.AddUInt(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0);
            serviceInfoId.Complete();

            ElementList serviceStateId = new ElementList();
            serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
            serviceStateId.Complete();

            FilterList filterList = new FilterList();
            filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
            filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
            filterList.Complete();

            Map map = new Map();
            map.AddKeyUInt(1, MapAction.ADD, filterList);
            map.Complete();

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);
            providerTestOptions.SourceDirectoryPayload = map;

            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1")
                    .AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient);

            /* Source directory refresh for the second server with different QoS*/
            qosList.Clear();
            qosList.AddQos(500, 500);
            qosList.Complete();

            serviceInfoId.Clear();
            serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
            serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
            serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
            serviceInfoId.AddArray(EmaRdm.ENAME_QOS, qosList);
            serviceInfoId.AddAscii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist");
            serviceInfoId.AddUInt(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0);
            serviceInfoId.Complete();

            filterList.Clear();
            filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
            filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
            filterList.Complete();

            Map map2 = new Map();
            map2.AddKeyUInt(1, MapAction.ADD, filterList);
            map2.Complete();

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            providerTestOptions2.SourceDirectoryPayload = map2;

            /* The second provider doesn't provide the source directory refresh within the DirectoryRequestTimeout value */
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);

            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1")
                    .AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();
            consumerTestOptions.GetChannelInformation = false;

            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, consumerTestOptions);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"), consumerClient);

                Thread.Sleep(500);

                Assert.Equal(4, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();
                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();
                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();
                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataType.DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);

                message = consumerClient.PopMessage();
                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'session channel closed'", statusMsg.State().ToString());

                RequestMsg reqMsg = new();

                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Map payload = refreshMsg.Payload().Map();
                var mapIt = payload.GetEnumerator();

                Assert.True(mapIt.MoveNext());

                MapEntry mapEntry = mapIt.Current;

                ulong serviceId = 32767;

                Assert.Equal(serviceId, mapEntry.Key.UInt());

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                consumer.Unregister(directoryHandle);

                /* Ensure that the consumer session has only one connection as the second connection is closed due to QoS mismatch */
                Assert.Single(consumer.m_OmmConsumerImpl!.ConsumerSession!.SessionChannelList);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForDirectoryStreamViaRegisterClientSameServiceTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient);

            OmmConsumer? consumer = null;

            ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();

            consumerTestOptions.GetChannelInformation = false;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, consumerTestOptions);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));
                RequestMsg reqMsg = new();

                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);
                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Map payload = refreshMsg.Payload().Map();

                var mapIt = payload.GetEnumerator();
                Assert.True(mapIt.MoveNext());
                MapEntry mapEntry = mapIt.Current;

                ulong serviceId = 32767;
                Assert.Equal(serviceId, mapEntry.Key.UInt());
                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                FilterList filterList = mapEntry.FilterList();
                var filterIt = filterList.GetEnumerator();
                Assert.True(filterIt.MoveNext());
                FilterEntry filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                ElementList elementList = filterEntry.ElementList();
                var elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                ElementEntry elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                Assert.Equal("DIRECT_FEED", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_VENDOR, elementEntry.Name);
                Assert.Equal("company name", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_IS_SOURCE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.UIntValue());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_CAPABILITIES, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                OmmArray ommArray = elementEntry.OmmArrayValue();
                var arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                OmmArrayEntry arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                Assert.Equal<ulong>(6, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(7, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(8, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(5, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(10, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(200, arrayEntry.OmmUIntValue().Value);
                Assert.False(arrayIt.MoveNext());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_QOS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                Assert.Equal("RealTime/TickByTick", arrayEntry.OmmQosValue().ToString());

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("Timeliness: 100/Rate: 100", arrayEntry.OmmQosValue().ToString());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.UIntValue());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ITEM_LIST, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ASCII, elementEntry.LoadType);
                Assert.Equal("#.itemlist", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.UIntValue());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.UIntValue());

                Assert.False(elIt.MoveNext());
                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_GROUP_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_STATE_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_SVC_STATE, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.UIntValue());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.UIntValue());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_STATUS, elementEntry.Name);
                Assert.Equal("Open / Ok / None / ''", elementEntry.OmmStateValue().ToString());
                Assert.False(elIt.MoveNext());

                Assert.False(filterIt.MoveNext());
                Assert.False(mapIt.MoveNext());

                consumer.Unregister(directoryHandle);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                m_Output.WriteLine("Uninitializing...");

                Assert.NotNull(consumer);

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForDirectoryStreamViaRegisterClientSameServiceWithRequestFiltersTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);
            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient);
            ;
            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                // Request info filter only
                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(1), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(1, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Map payload = refreshMsg.Payload().Map();

                var mapIt = payload.GetEnumerator();

                Assert.True(mapIt.MoveNext());

                MapEntry mapEntry = mapIt.Current;

                ulong serviceId = 32767;

                Assert.Equal(serviceId, mapEntry.Key.UInt());

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                FilterList filterList = mapEntry.FilterList();

                var filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());

                FilterEntry filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                ElementList elementList = filterEntry.ElementList();

                var elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                ElementEntry elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                Assert.Equal("DIRECT_FEED", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_VENDOR, elementEntry.Name);
                Assert.Equal("company name", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_IS_SOURCE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_CAPABILITIES, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                OmmArray ommArray = elementEntry.OmmArrayValue();
                var arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                OmmArrayEntry arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                Assert.Equal<ulong>(6, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(7, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(8, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(5, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(10, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(200, arrayEntry.OmmUIntValue().Value);
                Assert.False(arrayIt.MoveNext());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_QOS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                Assert.Equal("RealTime/TickByTick", arrayEntry.OmmQosValue().ToString());

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("Timeliness: 100/Rate: 100", arrayEntry.OmmQosValue().ToString());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ITEM_LIST, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ASCII, elementEntry.LoadType);
                Assert.Equal("#.itemlist", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.False(elIt.MoveNext());

                Assert.False(filterIt.MoveNext());

                long directoryHandle2 = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(3), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(3, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                payload = refreshMsg.Payload().Map();

                mapIt = payload.GetEnumerator();

                Assert.True(mapIt.MoveNext());

                mapEntry = mapIt.Current;

                serviceId = 32767;

                Assert.Equal(serviceId, mapEntry.Key.OmmUInt().Value);

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                filterList = mapEntry.FilterList();

                filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());

                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                Assert.Equal("DIRECT_FEED", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_VENDOR, elementEntry.Name);
                Assert.Equal("company name", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_IS_SOURCE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_CAPABILITIES, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                Assert.Equal<ulong>(6, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(7, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(8, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(5, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(10, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(200, arrayEntry.OmmUIntValue().Value);
                Assert.False(arrayIt.MoveNext());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_QOS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                Assert.Equal("RealTime/TickByTick", arrayEntry.OmmQosValue().ToString());

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("Timeliness: 100/Rate: 100", arrayEntry.OmmQosValue().ToString());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ITEM_LIST, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ASCII, elementEntry.LoadType);
                Assert.Equal("#.itemlist", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.False(elIt.MoveNext());

                Assert.True(filterIt.MoveNext());

                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_STATE_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_SVC_STATE, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.False(filterIt.MoveNext());

                consumer.Unregister(directoryHandle);
                consumer.Unregister(directoryHandle2);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                Console.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForDirectoryStreamViaRegisterClientDiffServiceTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY), consumerClient);

                var consumerSession = consumer.m_OmmConsumerImpl!.ConsumerSession!;

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Map payload = refreshMsg.Payload().Map();

                var mapIt = payload.GetEnumerator();

                Assert.True(mapIt.MoveNext());

                MapEntry mapEntry = mapIt.Current;

                /* Service Id of the first service */
                int serviceId = (int)mapEntry.Key.OmmUInt().Value;

                string serviceName = consumerSession.GetSessionDirectoryById(serviceId)?.ServiceName ?? "";

                Assert.Equal((ulong)serviceId, mapEntry.Key.OmmUInt().Value);

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                FilterList filterList = mapEntry.FilterList();

                var filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());

                FilterEntry filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                ElementList elementList = filterEntry.ElementList();

                var elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                ElementEntry elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                Assert.Equal(serviceName, elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_VENDOR, elementEntry.Name);
                Assert.Equal("company name", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_IS_SOURCE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_CAPABILITIES, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                OmmArray ommArray = elementEntry.OmmArrayValue();
                var arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                OmmArrayEntry arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                Assert.Equal<ulong>(6, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(7, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(8, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(5, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(10, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(200, arrayEntry.OmmUIntValue().Value);
                Assert.False(arrayIt.MoveNext());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_QOS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                Assert.Equal("RealTime/TickByTick", arrayEntry.OmmQosValue().ToString());

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("Timeliness: 100/Rate: 100", arrayEntry.OmmQosValue().ToString());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ITEM_LIST, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ASCII, elementEntry.LoadType);
                Assert.Equal("#.itemlist", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.False(elIt.MoveNext());
                Assert.True(filterIt.MoveNext());

                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_GROUP_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                Assert.False(elIt.MoveNext());
                Assert.True(filterIt.MoveNext());

                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_STATE_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_SVC_STATE, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.False(filterIt.MoveNext());

                Assert.True(mapIt.MoveNext());

                mapEntry = mapIt.Current;

                serviceId = (int)mapEntry.Key.OmmUInt().Value;

                serviceName = consumerSession.GetSessionDirectoryById(serviceId)?.ServiceName ?? "";

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                filterList = mapEntry.FilterList();

                filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());

                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                Assert.Equal(serviceName, elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_VENDOR, elementEntry.Name);
                Assert.Equal("company name", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_IS_SOURCE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_CAPABILITIES, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                Assert.Equal<ulong>(6, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(7, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(8, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(5, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(10, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(200, arrayEntry.OmmUIntValue().Value);
                Assert.False(arrayIt.MoveNext());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_QOS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                Assert.Equal("RealTime/TickByTick", arrayEntry.OmmQosValue().ToString());

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("Timeliness: 100/Rate: 100", arrayEntry.OmmQosValue().ToString());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ITEM_LIST, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ASCII, elementEntry.LoadType);
                Assert.Equal("#.itemlist", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.False(elIt.MoveNext());
                Assert.True(filterIt.MoveNext());

                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_GROUP_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                Assert.False(elIt.MoveNext());
                Assert.True(filterIt.MoveNext());

                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_STATE_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_SVC_STATE, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.False(filterIt.MoveNext());

                consumer.Unregister(directoryHandle);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForDirectoryStreamViaRegisterClientByServiceNameTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceName("DIRECT_FEED_2"), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Map payload = refreshMsg.Payload().Map();

                var mapIt = payload.GetEnumerator();

                Assert.True(mapIt.MoveNext());

                MapEntry mapEntry = mapIt.Current;

                /* This depends on the first source directory message from provider */
                Assert.True(mapEntry.Key.OmmUInt().Value == 32767 || mapEntry.Key.OmmUInt().Value == 32768);

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                FilterList filterList = mapEntry.FilterList();

                var filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());

                FilterEntry filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                ElementList elementList = filterEntry.ElementList();

                var elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                ElementEntry elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                Assert.Equal("DIRECT_FEED_2", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_VENDOR, elementEntry.Name);
                Assert.Equal("company name", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_IS_SOURCE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_CAPABILITIES, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                OmmArray ommArray = elementEntry.OmmArrayValue();
                var arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                OmmArrayEntry arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                Assert.Equal<ulong>(6, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(7, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(8, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(5, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(10, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(200, arrayEntry.OmmUIntValue().Value);
                Assert.False(arrayIt.MoveNext());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_QOS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                Assert.Equal("RealTime/TickByTick", arrayEntry.OmmQosValue().ToString());

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("Timeliness: 100/Rate: 100", arrayEntry.OmmQosValue().ToString());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ITEM_LIST, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ASCII, elementEntry.LoadType);
                Assert.Equal("#.itemlist", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.False(elIt.MoveNext());
                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_GROUP_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                Assert.False(elIt.MoveNext());
                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_STATE_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_SVC_STATE, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.False(filterIt.MoveNext());
                Assert.False(mapIt.MoveNext());

                consumer.Unregister(directoryHandle);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForDirectoryStreamViaRegisterClientByServiceIdTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceId(32767), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Map payload = refreshMsg.Payload().Map();

                var mapIt = payload.GetEnumerator();

                Assert.True(mapIt.MoveNext());

                MapEntry mapEntry = mapIt.Current;

                Assert.True(mapEntry.Key.OmmUInt().Value == 32767);

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                FilterList filterList = mapEntry.FilterList();

                var filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());

                FilterEntry filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                ElementList elementList = filterEntry.ElementList();

                var elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                ElementEntry elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);

                /* This depends on the first source directory message from provider */
                Assert.True(elementEntry.OmmAsciiValue().Value.Equals("DIRECT_FEED_2") || elementEntry.OmmAsciiValue().Value.Equals("DIRECT_FEED"));

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_VENDOR, elementEntry.Name);
                Assert.Equal("company name", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_IS_SOURCE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_CAPABILITIES, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                OmmArray ommArray = elementEntry.OmmArrayValue();
                var arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                OmmArrayEntry arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                Assert.Equal<ulong>(6, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(7, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(8, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(5, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(10, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(200, arrayEntry.OmmUIntValue().Value);
                Assert.False(arrayIt.MoveNext());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_QOS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                Assert.Equal("RealTime/TickByTick", arrayEntry.OmmQosValue().ToString());

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("Timeliness: 100/Rate: 100", arrayEntry.OmmQosValue().ToString());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ITEM_LIST, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ASCII, elementEntry.LoadType);
                Assert.Equal("#.itemlist", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.False(elIt.MoveNext());
                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_GROUP_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                Assert.False(elIt.MoveNext());
                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_STATE_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_SVC_STATE, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.False(filterIt.MoveNext());
                Assert.False(mapIt.MoveNext());

                consumer.Unregister(directoryHandle);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForDirectoryStreamViaRegisterClientWithUnknowServiceNameTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceName("UNKNOWN_SERVICE"), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.False(refreshMsg.HasServiceId);
                Assert.Equal("UNKNOWN_SERVICE", refreshMsg.ServiceName());
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Assert.False(refreshMsg.Payload().Map().GetEnumerator().MoveNext());

                consumer.Unregister(directoryHandle);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForDirectoryStreamViaRegisterClientWithUnknowServiceIdTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceId(55555), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.False(refreshMsg.HasServiceName);
                Assert.Equal(55555, refreshMsg.ServiceId());
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Assert.False(refreshMsg.Payload().Map().GetEnumerator().MoveNext());

                consumer.Unregister(directoryHandle);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForDirectoryStreamViaRegisterClientWithServiceDeletionTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY), consumerClient);

                long directoryHandle2 = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceName("DIRECT_FEED_2"), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Thread.Sleep(2000);

                Assert.Equal(2, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                while (consumerClient.QueueSize() == 0) { }

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);
                refreshMsg = (RefreshMsg)message;

                FilterList filterList = new FilterList();
                Map map = new Map();
                map.AddKeyUInt(1, MapAction.DELETE, filterList.Complete());
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                        Payload(map), 0);   // use 0 item handle to fanout to all subscribers

                map.Clear();
                map.AddKeyUInt(1, MapAction.DELETE, filterList);
                map.Complete();

                ommprovider2.Submit(updateMsg.Clear().DomainType(EmaRdm.MMT_DIRECTORY).
                        Payload(map), 0);   // use 0 item handle to fanout to all subscribers

                while (consumerClient.QueueSize() < 1) { }

                message = consumerClient.PopMessage();

                Assert.True(message is UpdateMsg);

                updateMsg = (UpdateMsg)message;
                Assert.Equal(2, updateMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, updateMsg.DomainType());
                Assert.Equal(DataType.DataTypes.MAP, updateMsg.Payload().DataType);

                Map payload = updateMsg.Payload().Map();

                var mapIt = payload.GetEnumerator();
                Assert.True(mapIt.MoveNext());
                MapEntry mapEntry = mapIt.Current;

                Assert.Equal<ulong>(32767, mapEntry.Key.OmmUInt().Value);

                Assert.Equal(DataType.DataTypes.NO_DATA, mapEntry.LoadType);

                while (consumerClient.QueueSize() < 2) { }

                message = consumerClient.PopMessage();

                Assert.True(message is UpdateMsg);

                updateMsg = (UpdateMsg)message;
                Assert.Equal(2, updateMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, updateMsg.DomainType());
                Assert.Equal(DataType.DataTypes.MAP, updateMsg.Payload().DataType);

                payload = updateMsg.Payload().Map();

                mapIt = payload.GetEnumerator();
                Assert.True(mapIt.MoveNext());
                mapEntry = mapIt.Current;

                Assert.Equal<ulong>(32768, mapEntry.Key.OmmUInt().Value);
                Assert.Equal(DataType.DataTypes.NO_DATA, mapEntry.LoadType);

                Assert.False(mapIt.MoveNext());

                message = consumerClient.PopMessage();

                Assert.True(message is UpdateMsg);

                updateMsg = (UpdateMsg)message;
                Assert.Equal(2, updateMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, updateMsg.DomainType());
                Assert.Equal(DataType.DataTypes.MAP, updateMsg.Payload().DataType);

                payload = updateMsg.Payload().Map();

                mapIt = payload.GetEnumerator();
                Assert.True(mapIt.MoveNext());
                mapEntry = mapIt.Current;

                Assert.Equal<ulong>(32768, mapEntry.Key.OmmUInt().Value);
                Assert.Equal(DataType.DataTypes.NO_DATA, mapEntry.LoadType);
                Assert.False(mapIt.MoveNext());

                consumer.Unregister(directoryHandle);
                consumer.Unregister(directoryHandle2);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForDirectoryStreamViaRegisterClientSameServiceWithStateChangeTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                // Request info filter only
                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(63), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Map payload = refreshMsg.Payload().Map();

                var mapIt = payload.GetEnumerator();
                Assert.True(mapIt.MoveNext());
                MapEntry mapEntry = mapIt.Current;

                ulong serviceId = 32767;

                Assert.Equal(serviceId, mapEntry.Key.OmmUInt().Value);

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                FilterList filterList = mapEntry.FilterList();

                var filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());

                FilterEntry filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                ElementList elementList = filterEntry.ElementList();

                var elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                ElementEntry elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                Assert.Equal("DIRECT_FEED", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_VENDOR, elementEntry.Name);
                Assert.Equal("company name", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_IS_SOURCE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_CAPABILITIES, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                OmmArray ommArray = elementEntry.OmmArrayValue();
                var arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                OmmArrayEntry arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                Assert.Equal<ulong>(6, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(7, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(8, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(5, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(10, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(200, arrayEntry.OmmUIntValue().Value);
                Assert.False(arrayIt.MoveNext());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_QOS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                Assert.Equal("RealTime/TickByTick", arrayEntry.OmmQosValue().ToString());

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("Timeliness: 100/Rate: 100", arrayEntry.OmmQosValue().ToString());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ITEM_LIST, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ASCII, elementEntry.LoadType);
                Assert.Equal("#.itemlist", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.False(elIt.MoveNext());

                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_GROUP_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_STATE_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_SVC_STATE, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.False(filterIt.MoveNext());

                Assert.False(mapIt.MoveNext());

                Thread.Sleep(3000);

                // Change service's state to down from one provider
                ElementList serviceState = new ElementList();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, 0);
                serviceState.Complete();

                FilterList filterListEnc = new FilterList();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, serviceState);
                filterListEnc.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers	

                Thread.Sleep(5000);

                /* No message in the queue as the service state is still up on another provider */
                Assert.Equal(0, consumerClient.QueueSize());

                ommprovider2.Submit(updateMsg.Clear().DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers	

                Thread.Sleep(5000);

                Assert.Equal(1, consumerClient.QueueSize()); // Receives source directory update message to notify that the service state is down.

                message = consumerClient.PopMessage();

                Assert.True(message is UpdateMsg);

                updateMsg = (UpdateMsg)message;

                Assert.Equal(2, updateMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, updateMsg.DomainType());
                Assert.True(updateMsg.HasMsgKey);
                Assert.Equal(63, updateMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, updateMsg.Payload().DataType);

                payload = updateMsg.Payload().Map();

                mapIt = payload.GetEnumerator();

                Assert.True(mapIt.MoveNext());

                mapEntry = mapIt.Current;

                Assert.Equal(serviceId, mapEntry.Key.OmmUInt().Value);

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                filterList = mapEntry.FilterList();

                filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());

                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                Assert.Equal("DIRECT_FEED", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_VENDOR, elementEntry.Name);
                Assert.Equal("company name", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_IS_SOURCE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_CAPABILITIES, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                Assert.Equal<ulong>(6, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(7, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(8, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(5, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(10, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(200, arrayEntry.OmmUIntValue().Value);
                Assert.False(arrayIt.MoveNext());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_QOS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                Assert.Equal("RealTime/TickByTick", arrayEntry.OmmQosValue().ToString());

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("Timeliness: 100/Rate: 100", arrayEntry.OmmQosValue().ToString());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ITEM_LIST, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ASCII, elementEntry.LoadType);
                Assert.Equal("#.itemlist", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.False(elIt.MoveNext());

                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_GROUP_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_STATE_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_SVC_STATE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.False(filterIt.MoveNext());

                Assert.False(mapIt.MoveNext());

                consumer.Unregister(directoryHandle);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForDirectoryStreamViaRegisterClientSameServiceWithAcceptingRequestsChangeTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                // Request info filter only
                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(63), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Map payload = refreshMsg.Payload().Map();

                var mapIt = payload.GetEnumerator();
                Assert.True(mapIt.MoveNext());
                MapEntry mapEntry = mapIt.Current;

                ulong serviceId = 32767;

                Assert.Equal(serviceId, mapEntry.Key.OmmUInt().Value);

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                FilterList filterList = mapEntry.FilterList();

                var filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());

                FilterEntry filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                ElementList elementList = filterEntry.ElementList();

                var elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                ElementEntry elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                Assert.Equal("DIRECT_FEED", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_VENDOR, elementEntry.Name);
                Assert.Equal("company name", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_IS_SOURCE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_CAPABILITIES, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                OmmArray ommArray = elementEntry.OmmArrayValue();
                var arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                OmmArrayEntry arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                Assert.Equal<ulong>(6, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(7, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(8, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(5, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(10, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(200, arrayEntry.OmmUIntValue().Value);
                Assert.False(arrayIt.MoveNext());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_QOS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                Assert.Equal("RealTime/TickByTick", arrayEntry.OmmQosValue().ToString());

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("Timeliness: 100/Rate: 100", arrayEntry.OmmQosValue().ToString());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ITEM_LIST, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ASCII, elementEntry.LoadType);
                Assert.Equal("#.itemlist", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.False(elIt.MoveNext());

                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_GROUP_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_STATE_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_SVC_STATE, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.False(filterIt.MoveNext());

                Assert.False(mapIt.MoveNext());

                Thread.Sleep(3000);

                // Change service's state to down from one provider
                ElementList serviceState = new ElementList();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceState.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 0);
                serviceState.Complete();

                FilterList filterListEnc = new FilterList();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, serviceState);
                filterListEnc.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers	

                Thread.Sleep(5000);

                /* No message in the queue as the service state is still up on another provider */
                Assert.Equal(0, consumerClient.QueueSize());

                ommprovider2.Submit(updateMsg.Clear().DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers	

                Thread.Sleep(5000);

                Assert.Equal(1, consumerClient.QueueSize()); // Receives source directory update message to notify that the service state is down.

                message = consumerClient.PopMessage();

                Assert.True(message is UpdateMsg);

                updateMsg = (UpdateMsg)message;

                Assert.Equal(2, updateMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, updateMsg.DomainType());
                Assert.True(updateMsg.HasMsgKey);
                Assert.Equal(63, updateMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, updateMsg.Payload().DataType);

                payload = updateMsg.Payload().Map();

                mapIt = payload.GetEnumerator();

                Assert.True(mapIt.MoveNext());

                mapEntry = mapIt.Current;

                Assert.Equal(serviceId, mapEntry.Key.OmmUInt().Value);

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                filterList = mapEntry.FilterList();

                filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());

                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                Assert.Equal("DIRECT_FEED", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_VENDOR, elementEntry.Name);
                Assert.Equal("company name", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_IS_SOURCE, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_CAPABILITIES, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.UINT, arrayEntry.LoadType);
                Assert.Equal<ulong>(6, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(7, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(8, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(5, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(10, arrayEntry.OmmUIntValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal<ulong>(200, arrayEntry.OmmUIntValue().Value);
                Assert.False(arrayIt.MoveNext());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_PROVIDED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_DICTIONARYS_USED, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.ASCII, arrayEntry.LoadType);
                Assert.Equal("RWFFld", arrayEntry.OmmAsciiValue().Value);

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("RWFEnum", arrayEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_QOS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ARRAY, elementEntry.LoadType);
                ommArray = elementEntry.OmmArrayValue();
                arrayIt = ommArray.GetEnumerator();
                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;

                Assert.Equal(DataType.DataTypes.QOS, arrayEntry.LoadType);
                Assert.Equal("RealTime/TickByTick", arrayEntry.OmmQosValue().ToString());

                Assert.True(arrayIt.MoveNext());
                arrayEntry = arrayIt.Current;
                Assert.Equal("Timeliness: 100/Rate: 100", arrayEntry.OmmQosValue().ToString());

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_QOS_RANGE, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ITEM_LIST, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.ASCII, elementEntry.LoadType);
                Assert.Equal("#.itemlist", elementEntry.OmmAsciiValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_CONS_STATUS, elementEntry.Name);
                Assert.Equal(DataType.DataTypes.UINT, elementEntry.LoadType);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.False(elIt.MoveNext());

                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_GROUP_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                Assert.True(filterIt.MoveNext());
                filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_STATE_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                elementList = filterEntry.ElementList();

                elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_SVC_STATE, elementEntry.Name);
                Assert.Equal<ulong>(1, elementEntry.OmmUIntValue().Value);

                Assert.True(elIt.MoveNext());
                elementEntry = elIt.Current;
                Assert.Equal(EmaRdm.ENAME_ACCEPTING_REQS, elementEntry.Name);
                Assert.Equal<ulong>(0, elementEntry.OmmUIntValue().Value);

                Assert.False(filterIt.MoveNext());

                Assert.False(mapIt.MoveNext());

                consumer.Unregister(directoryHandle);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsReceiveDirectoryResponseOnlyOneConnectionTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            /* Source directory refresh */
            OmmArray capablities = new OmmArray();
            capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
            capablities.Complete();

            OmmArray dictionaryUsed = new OmmArray();
            dictionaryUsed.AddAscii("RWFFld");
            dictionaryUsed.AddAscii("RWFEnum");
            dictionaryUsed.Complete();

            ElementList serviceInfoId = new ElementList();

            serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
            serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
            serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
            serviceInfoId.Complete();

            ElementList serviceStateId = new ElementList();
            serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
            serviceStateId.Complete();

            FilterList filterList = new FilterList();
            filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
            filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
            filterList.Complete();

            Map map = new Map();
            map.AddKeyUInt(1, MapAction.ADD, filterList);
            map.Complete();

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);
            providerTestOptions.SourceDirectoryPayload = map;


            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1")
                    .AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();

            /* The second provider doesn't provide the source directory refresh within the DirectoryRequestTimeout value */
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1")
                    .AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient2);

            OmmConsumer? consumer = null;

            ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();
            consumerTestOptions.DumpDictionaryRefreshMsg = false;

            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, consumerTestOptions);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                // Request info filter only
                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(63), consumerClient);

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                consumer.Unregister(directoryHandle);
            }
            catch (OmmException excep)
            {
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSingleItemRequestByServiceIdTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendUpdateMessage = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                // Request info filter only
                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(63), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message
                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Map payload = refreshMsg.Payload().Map();
                var mapIt = payload.GetEnumerator();

                Assert.True(mapIt.MoveNext());
                MapEntry mapEntry = mapIt.Current;

                ulong serviceId = mapEntry.Key.UInt();

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                FilterList filterList = mapEntry.FilterList();
                var filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());
                FilterEntry filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                ElementList elementList = filterEntry.ElementList();

                var elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                ElementEntry elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                string serviceName = elementEntry.OmmAsciiValue().Value;

                int direct_feed_serviceId;
                int direct_feed2_serviceId;

                if (serviceName.Equals("DIRECT_FEED"))
                {
                    direct_feed_serviceId = (int)serviceId;
                    direct_feed2_serviceId = direct_feed_serviceId + 1;
                }
                else
                {
                    direct_feed2_serviceId = (int)serviceId;
                    direct_feed_serviceId = direct_feed2_serviceId + 1;
                }

                long itemHandle = consumer.RegisterClient(reqMsg.Clear().ServiceId(direct_feed_serviceId).Name("LSEG.O"), consumerClient);
                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());
                message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);
                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(2, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(direct_feed_serviceId, refreshMsg.ServiceId());

                message = consumerClient.PopMessage();
                UpdateMsg updateMsg = (UpdateMsg)message;

                Assert.Equal("DIRECT_FEED", updateMsg.ServiceName());
                Assert.Equal("LSEG.O", updateMsg.Name());
                Assert.Equal(direct_feed_serviceId, updateMsg.ServiceId());

                long itemHandle2 = consumer.RegisterClient(reqMsg.Clear().ServiceId(direct_feed2_serviceId).Name("LSEG.O"), consumerClient);
                Thread.Sleep(2000);

                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();
                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED_2", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(2, consumerClient.QueueSize());
                message = consumerClient.PopMessage();
                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED_2", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(direct_feed2_serviceId, refreshMsg.ServiceId());

                message = consumerClient.PopMessage();

                updateMsg = (UpdateMsg)message;

                Assert.Equal("DIRECT_FEED_2", updateMsg.ServiceName());
                Assert.Equal("LSEG.O", updateMsg.Name());
                Assert.Equal(direct_feed2_serviceId, updateMsg.ServiceId());

                consumer.Unregister(directoryHandle);
                consumer.Unregister(itemHandle);
                consumer.Unregister(itemHandle2);
            }
            catch (OmmException excep)
            {
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForLoadingDictionaryFromNetworkAndSubscribeDictioanryStreamTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1")
                    .AdminControlDictionary(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1")
                    .AdminControlDictionary(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient);

            OmmConsumer? consumer = null;

            ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();

            consumerTestOptions.DumpDictionaryRefreshMsg = false;

            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, consumerTestOptions);

            try
            {
                /* Consumer_11 to download dictionary from network when the first service available. */
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_11"));

                /* Provider receives two dictionary request from OmmConsumer */
                Assert.Equal(2, providerClient.QueueSize());

                Msg message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("RWFFld", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());
                Assert.Equal(EmaRdm.DICTIONARY_NORMAL, requestMsg.Filter());

                message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("RWFEnum", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());
                Assert.Equal(EmaRdm.DICTIONARY_NORMAL, requestMsg.Filter());

                RequestMsg reqMsg = new();

                long rwfFldHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFFld").
                        Filter(EmaRdm.DICTIONARY_NORMAL), consumerClient);

                long rwfEnumHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFEnum").
                        Filter(EmaRdm.DICTIONARY_NORMAL), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(3, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                /* Receives first partial refresh from EMA */
                Assert.Equal("RWFFld", refreshMsg.Name());
                Assert.False(refreshMsg.Complete());
                Assert.True(refreshMsg.ClearCache());

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                /* Receives complete refresh from EMA */
                Assert.Equal("RWFEnum", refreshMsg.Name());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.ClearCache());

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                /* Receives second partial refresh from EMA */
                Assert.Equal("RWFFld", refreshMsg.Name());
                Assert.True(refreshMsg.Complete());
                Assert.False(refreshMsg.ClearCache());

                consumer.Unregister(rwfFldHandle);
                consumer.Unregister(rwfEnumHandle);
            }
            catch (OmmException excep)
            {
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForLoadingDictionaryFromNetworkAndSubscribeDictioanryStreamByServiceNameTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1")
                    .AdminControlDictionary(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1")
                    .AdminControlDictionary(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient);

            OmmConsumer? consumer = null;

            ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();

            consumerTestOptions.DumpDictionaryRefreshMsg = false;

            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, consumerTestOptions);

            try
            {
                /* Consumer_11 to download dictionary from network when the first service available. */
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_11"));

                /* Provider receives two dictionary request from OmmConsumer */
                Assert.Equal(2, providerClient.QueueSize());

                Msg message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("RWFFld", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());
                Assert.Equal(EmaRdm.DICTIONARY_NORMAL, requestMsg.Filter());

                message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("RWFEnum", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());
                Assert.Equal(EmaRdm.DICTIONARY_NORMAL, requestMsg.Filter());

                RequestMsg reqMsg = new();

                long rwfFldHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFFld").ServiceName("DIRECT_FEED").
                        Filter(EmaRdm.DICTIONARY_NORMAL), consumerClient);

                long rwfEnumHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DICTIONARY).Name("RWFEnum").ServiceName("DIRECT_FEED").
                        Filter(EmaRdm.DICTIONARY_NORMAL), consumerClient);

                Thread.Sleep(5000);

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                /* Receives complete refresh from EMA */
                Assert.Equal("RWFFld", refreshMsg.Name());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.ClearCache());

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                /* Receives second partial refresh from EMA */
                Assert.Equal("RWFEnum", refreshMsg.Name());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.ClearCache());

                consumer.Unregister(rwfFldHandle);
                consumer.Unregister(rwfEnumHandle);
            }
            catch (OmmException excep)
            {
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
                consumer.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsWithSymbollistRequestTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendUpdateMessage = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED_2").DomainType(EmaRdm.MMT_SYMBOL_LIST).Name(".AV.N"), consumerClient);

                Thread.Sleep(2000);

                var consumerSession = consumer.m_OmmConsumerImpl!.ConsumerSession;

                int serviceId = consumerSession!.GetSessionDirectoryByName("DIRECT_FEED_2")!.Service!.ServiceId;

                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(1, providerClient2.QueueSize());

                Msg message = providerClient2.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED_2", requestMsg.ServiceName());
                Assert.Equal(".AV.N", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());
                Assert.Equal(EmaRdm.MMT_SYMBOL_LIST, requestMsg.DomainType());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED_2", refreshMsg.ServiceName());
                Assert.Equal(".AV.N", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(EmaRdm.MMT_SYMBOL_LIST, refreshMsg.DomainType());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Map map = refreshMsg.Payload().Map();

                Assert.True(map.HasTotalCountHint);
                Assert.Equal(3, map.TotalCountHint());
                Assert.Equal(DataType.DataTypes.FIELD_LIST, map.SummaryData().DataType);

                var it = map.GetEnumerator();

                Assert.True(it.MoveNext());
                MapEntry entry = it.Current;
                Assert.Equal("6974 656D 41                                    itemA", entry.Key.Buffer().ToString());

                Assert.True(it.MoveNext());
                entry = it.Current;
                Assert.Equal("6974 656D 42                                    itemB", entry.Key.Buffer().ToString());

                Assert.True(it.MoveNext());
                entry = it.Current;
                Assert.Equal("6974 656D 43                                    itemC", entry.Key.Buffer().ToString());

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSingleItemRequestByServiceNameTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendUpdateMessage = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                // Request info filter only
                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).Filter(63), consumerClient);

                /* Waits until OmmConsumer receives the refresh message */
                while (consumerClient.QueueSize() == 0) { }

                Assert.Equal(1, consumerClient.QueueSize()); // Ensure that the callback receives only one directory message
                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(63, refreshMsg.Filter());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);

                Map payload = refreshMsg.Payload().Map();
                var mapIt = payload.GetEnumerator();

                Assert.True(mapIt.MoveNext());
                MapEntry mapEntry = mapIt.Current;

                ulong serviceId = mapEntry.Key.UInt();

                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                FilterList filterList = mapEntry.FilterList();
                var filterIt = filterList.GetEnumerator();

                Assert.True(filterIt.MoveNext());
                FilterEntry filterEntry = filterIt.Current;

                Assert.Equal(FilterAction.SET, filterEntry.Action);
                Assert.Equal(EmaRdm.SERVICE_INFO_FILTER, filterEntry.FilterId);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, filterEntry.LoadType);

                // Checks INFO filter
                ElementList elementList = filterEntry.ElementList();

                var elIt = elementList.GetEnumerator();
                Assert.True(elIt.MoveNext());
                ElementEntry elementEntry = elIt.Current;

                Assert.Equal(EmaRdm.ENAME_NAME, elementEntry.Name);
                string serviceName = elementEntry.OmmAsciiValue().Value;

                int direct_feed_serviceId;
                int direct_feed2_serviceId;

                if (serviceName.Equals("DIRECT_FEED"))
                {
                    direct_feed_serviceId = (int)serviceId;
                    direct_feed2_serviceId = direct_feed_serviceId + 1;
                }
                else
                {
                    direct_feed2_serviceId = (int)serviceId;
                    direct_feed_serviceId = direct_feed2_serviceId + 1;
                }

                long itemHandle = consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);
                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());
                message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);
                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(2, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(direct_feed_serviceId, refreshMsg.ServiceId());

                message = consumerClient.PopMessage();
                UpdateMsg updateMsg = (UpdateMsg)message;

                Assert.Equal("DIRECT_FEED", updateMsg.ServiceName());
                Assert.Equal("LSEG.O", updateMsg.Name());
                Assert.Equal(direct_feed_serviceId, updateMsg.ServiceId());

                long itemHandle2 = consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED_2").Name("LSEG.O"), consumerClient);
                Thread.Sleep(2000);

                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();
                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED_2", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(2, consumerClient.QueueSize());
                message = consumerClient.PopMessage();
                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED_2", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(direct_feed2_serviceId, refreshMsg.ServiceId());

                message = consumerClient.PopMessage();

                updateMsg = (UpdateMsg)message;

                Assert.Equal("DIRECT_FEED_2", updateMsg.ServiceName());
                Assert.Equal("LSEG.O", updateMsg.Name());
                Assert.Equal(direct_feed2_serviceId, updateMsg.ServiceId());

                consumer.Unregister(directoryHandle);
                consumer.Unregister(itemHandle);
                consumer.Unregister(itemHandle2);
            }
            catch (OmmException excep)
            {
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsBatchItemRequestByServiceNameTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendUpdateMessage = false;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_12"));

                consumerClient.Consumer(consumer);

                ElementList batch = new ElementList();
                OmmArray array = new OmmArray();

                array.AddAscii("itemA");
                array.AddAscii("itemB");
                array.AddAscii("itemC");
                array.Complete();

                batch.AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array);
                batch.Complete();

                RequestMsg reqMsg = new();

                consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Payload(batch), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(3, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal(3, requestMsg.StreamId());
                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemA", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal(4, requestMsg.StreamId());
                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemB", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal(5, requestMsg.StreamId());
                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemC", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(4, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                Assert.True(message is StatusMsg);

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(5, statusMsg.StreamId());
                Assert.False(statusMsg.HasName);
                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal(32767, statusMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Stream closed for batch", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(6, refreshMsg.StreamId());
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("itemA", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                Assert.Equal(7, refreshMsg.StreamId());
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("itemB", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                Assert.Equal(8, refreshMsg.StreamId());
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("itemC", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                /* Closes the first provider to force recovering items */
                ommprovider.Uninitialize();

                Thread.Sleep(3000);

                Assert.Equal(3, providerClient2.QueueSize());

                Thread.Sleep(1000);

                message = providerClient2.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal(3, requestMsg.StreamId());
                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemA", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                message = providerClient2.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal(4, requestMsg.StreamId());
                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemB", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                message = providerClient2.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal(5, requestMsg.StreamId());
                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemC", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Assert.Equal(6, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;
                Assert.Equal(6, statusMsg.StreamId());
                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("itemA", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("channel down.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;
                Assert.Equal(7, statusMsg.StreamId());
                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("itemB", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("channel down.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;
                Assert.Equal(8, statusMsg.StreamId());
                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("itemC", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("channel down.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                Assert.Equal(6, refreshMsg.StreamId());
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("itemA", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                Assert.Equal(7, refreshMsg.StreamId());
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("itemB", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                Assert.Equal(8, refreshMsg.StreamId());
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("itemC", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                consumerClient.UnregisterAllHandles();

                Thread.Sleep(1000);

            }
            catch (OmmException excep)
            {
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsBatchItemRequestByServiceListTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendUpdateMessage = false;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).AddServiceList(serviceList).ConsumerName("Consumer_9"));

                ElementList batch = new ElementList();
                OmmArray array = new OmmArray();

                array.AddAscii("itemA");
                array.AddAscii("itemB");
                array.AddAscii("itemC");
                array.Complete();

                batch.AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST, array);
                batch.Complete();

                RequestMsg reqMsg = new RequestMsg();

                consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Payload(batch), consumerClient);

                Thread.Sleep(2000);

                ProviderTestClient providerClient = providerClient1.QueueSize() > 0 ? providerClient1 : providerClient2;

                Assert.Equal(3, providerClient.QueueSize());

                Msg message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal(3, requestMsg.StreamId());
                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemA", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal(4, requestMsg.StreamId());
                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemB", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                message = providerClient.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal(5, requestMsg.StreamId());
                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemC", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(4, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                Assert.True(message is StatusMsg);

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.False(statusMsg.HasName);
                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal(32767, statusMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Stream closed for batch", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("itemA", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("itemB", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("itemC", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
            }
            catch (OmmException exp)
            {
                m_Output.WriteLine(exp.StackTrace);
                Assert.Fail(exp.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void SingleItemRecoverFromRequestTimeoutTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendItemResponse = false;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                /* The item request timeout is set to 5 seconds for Consumer_9 */
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle1 = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(9000);

                /* Ensure that the provider receives a request message but it doesn't send a response back. */
                Assert.Equal(2, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();
                RequestMsg recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                Assert.Equal(2, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(5, statusMsg.StreamId());
                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Request timeout", statusMsg.State().StatusText);

                Thread.Sleep(2000);
                Assert.Equal(1, providerClient1.QueueSize());

                /* Provider receives a close message */
                message = providerClient1.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                /* The second provider receives the request message */
                Assert.Equal(1, providerClient2.QueueSize());
                message = providerClient2.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                /* Receives the first refresh message from the second provider */
                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                consumer.Unregister(itemHandle1);
            }
            catch (OmmException excp)
            {
                m_Output.WriteLine(excp.StackTrace);
                Assert.Fail(excp.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithRequestedServiceDownWithOpenSuspectStatusTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            providerTestOptions.ItemGroupId = new EmaBuffer(Encoding.ASCII.GetBytes("10"));

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(3000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.True(providerTestOptions.ItemGroupId.Equals(refreshMsg.ItemGroup()));
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* The first provider sends a source directory update message to change service state to DOWN with service's status */
                ElementList serviceState = new ElementList();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN);
                serviceState.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1);
                serviceState.AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.OPEN, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE);
                serviceState.Complete();

                FilterList filterListEnc = new FilterList();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceState);
                filterListEnc.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers


                Thread.Sleep(2000); // Wait until consumer receives the item Open/Suspect status message from the VA Watchlist

                /* The second provider receives the request message */
                Assert.Equal(1, providerClient2.QueueSize());
                message = providerClient2.PopMessage();
                RequestMsg recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);

                /* Receives refresh message from the second provider */
                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithRequestedConnectionDownWithEnableSessionEnhancedItemRecoveryTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            providerTestOptions.ItemGroupId = new EmaBuffer(Encoding.ASCII.GetBytes("10"));

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_12"));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(3000);

                /* Checks provider that receives the item request. */
                Msg message;
                RequestMsg? requestMsg = null;

                message = providerClient1.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.True(providerTestOptions.ItemGroupId.Equals(refreshMsg.ItemGroup()));
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Force channel down on provider */
                RequestMsg recvReqMsg;
                ommprovider.Uninitialize();

                Thread.Sleep(2000);

                /* The second provider receives the request message */
                Assert.Equal(1, providerClient2.QueueSize());
                message = providerClient2.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("channel down.", statusMsg.State().StatusText);

                /* Receives refresh message from the second provider */
                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            providerTestOptions.ItemGroupId = new EmaBuffer(Encoding.ASCII.GetBytes("55"));

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                long itemHandle = consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(1000);

                /* Checks provider that receives the item request. */
                Msg message;
                RequestMsg? requestMsg = null;

                message = providerClient1.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;

                if (!requestMsg.HasServiceName)
                {
                    m_Output.WriteLine(requestMsg.ToString());
                }

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName()); // Failed to get service name
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.True(providerTestOptions.ItemGroupId.Equals(refreshMsg.ItemGroup()));
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Force channel down on the first provider */
                RequestMsg recvReqMsg;
                ommprovider.Uninitialize();

                Thread.Sleep(1000);

                ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

                int count = 0;
                while (count <= 5 && providerClient1.QueueSize() != 2)
                {
                    Thread.Sleep(1200);
                    count++;
                }

                /* The first provider receives the request message */
                Assert.Equal(2, providerClient1.QueueSize());

                message = providerClient1.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(1, recvReqMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, recvReqMsg.DomainType());

                message = providerClient1.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(4, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);

                /* Receives refresh message from the first provider */
                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.True(providerTestOptions.ItemGroupId.Equals(refreshMsg.ItemGroup()));
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryAndChannelIsClosedTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            providerTestOptions.ItemGroupId = new EmaBuffer(Encoding.ASCII.GetBytes("55"));

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(1000);

                /* Checks provider that receives the item request. */
                Msg message;
                RequestMsg? requestMsg = null;

                message = providerClient1.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.True(providerTestOptions.ItemGroupId.Equals(refreshMsg.ItemGroup()));
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Force channel down on the first provider */
                RequestMsg recvReqMsg;
                ommprovider.Uninitialize();

                /* Wait until the subscribed channel is closed */
                Thread.Sleep(11000);

                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);

                /* Receives refresh message from the second provider */
                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemReceivedCloseSuspectFromProviderTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            providerTestOptions.CloseItemRequest = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();

            providerTestOptions2.CloseItemRequest = true;

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(3000);

                /* Ensure that the provider receives a request message but it closes the item stream. */
                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();
                RequestMsg recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                Thread.Sleep(1000);

                /* Ensure that the first provider doesn't receive a close message */
                Assert.Equal(0, providerClient1.QueueSize());

                /* Ensure that the second provider receive a request message but it closes the item stream */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                Assert.Equal(2, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(5, statusMsg.StreamId());
                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, statusMsg.State().StatusCode);
                Assert.Equal("Unauthorized access to the item.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal(5, statusMsg.StreamId());
                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, statusMsg.State().StatusCode);
                Assert.Equal("Unauthorized access to the item.", statusMsg.State().StatusText);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithUnknownServiceNameAndServiceIdTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle1 = consumer.RegisterClient(reqMsg.ServiceName("UNKNOWN_SERVICE").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                /* Ensure that the provider doesn't receive any request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("UNKNOWN_SERVICE", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                consumer.Unregister(itemHandle1); // Users cancel this request.

                long itemHandle2 = consumer.RegisterClient(reqMsg.Clear().ServiceId(0).Name("LSEG.O"), consumerClient);

                /* Ensure that the provider doesn't receive any request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal(0, statusMsg.ServiceId());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Service id of '0' is not found.", statusMsg.State().StatusText);

                consumer.Unregister(itemHandle2); // Users cancel this request.
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithNonExistenceServiceNameAndAddTheServiceTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle1 = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED2").Name("LSEG.O"), consumerClient);

                long itemHandle2 = consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED2").Name("LSEG.L"), consumerClient);

                Thread.Sleep(2000);

                /* Ensure that the provider doesn't receive any request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(2, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED2", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED2", statusMsg.ServiceName());
                Assert.Equal("LSEG.L", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                /* Provider send source directory update message to add the DIRECT_FEED2 service */
                OmmArray capablities = new OmmArray();
                capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
                capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
                capablities.Complete();

                OmmArray dictionaryUsed = new OmmArray();
                dictionaryUsed.AddAscii("RWFFld");
                dictionaryUsed.AddAscii("RWFEnum");
                dictionaryUsed.Complete();

                ElementList serviceInfoId = new ElementList();

                serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED2");
                serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
                serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
                serviceInfoId.Complete();

                ElementList serviceStateId = new ElementList();
                serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceStateId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
                filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
                filterList.Complete();

                Map map = new Map();
                map.AddKeyUInt(2, MapAction.ADD, filterList);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                                        Payload(map), 0);

                Thread.Sleep(2000);

                /* Provider receives a request once the service is available */
                Assert.Equal(2, providerClient1.QueueSize());

                message = providerClient1.PopMessage();
                RequestMsg recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(2, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED2", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                message = providerClient1.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(4, recvReqMsg.StreamId());
                Assert.Equal(2, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED2", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.L", recvReqMsg.Name());

                Thread.Sleep(2000);

                Assert.Equal(2, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED2", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(32768, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED2", refreshMsg.ServiceName());
                Assert.Equal("LSEG.L", refreshMsg.Name());
                Assert.Equal(32768, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                consumer.Unregister(itemHandle1);
                consumer.Unregister(itemHandle2);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemAndRecoveringTheItemWithNoMatchingServiceStatusTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                var consumerSession = consumer.m_OmmConsumerImpl!.ConsumerSession;

                int serviceId = consumerSession!.GetSessionDirectoryByName("DIRECT_FEED")!.Service!.ServiceId;

                RequestMsg reqMsg = new RequestMsg();

                long itemHandle1 = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(1000);

                /* Ensure that the provider receives the request message */
                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal(3, requestMsg.StreamId());
                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* The first server changes the service state to down and sends item closed recoverable status */
                ElementList serviceState = new ElementList();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN);
                serviceState.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 0);
                serviceState.AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT,
                        OmmState.StatusCodes.NONE, "Item temporary closed");
                serviceState.Complete();

                FilterList filterListEnc = new FilterList();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, serviceState);
                filterListEnc.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers

                Thread.Sleep(2000);

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal(serviceId, statusMsg.ServiceId());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Item temporary closed", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();
                statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal(serviceId, statusMsg.ServiceId());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);


                /* The second provider send source directory update message to add the DIRECT_FEED service */
                OmmArray capablities = new OmmArray();
                capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
                capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
                capablities.Complete();

                OmmArray dictionaryUsed = new OmmArray();
                dictionaryUsed.AddAscii("RWFFld");
                dictionaryUsed.AddAscii("RWFEnum");
                dictionaryUsed.Complete();

                OmmArray qosList = new OmmArray();
                qosList.AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK);
                qosList.AddQos(100, 100);
                qosList.Complete();

                ElementList serviceInfoId = new ElementList();
                serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
                serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
                serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
                serviceInfoId.AddArray(EmaRdm.ENAME_QOS, qosList);
                serviceInfoId.AddAscii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist");
                serviceInfoId.AddUInt(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0);
                serviceInfoId.Complete();

                ElementList serviceStateId = new ElementList();
                serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceStateId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
                filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
                filterList.Complete();

                map = new Map();
                map.AddKeyUInt(2, MapAction.ADD, filterList);
                map.Complete();

                updateMsg = new UpdateMsg();
                ommprovider2.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                                        Payload(map), 0);

                Thread.Sleep(2000);

                /* The second provider receives a request once the service is available */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();
                RequestMsg recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(2, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                consumer.Unregister(itemHandle1);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingTheSameItemNameAndServiceNameTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new RequestMsg();

                int serviceId = 32767;

                long itemHandle1 = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                long itemHandle2 = consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                /* The second provider doesn't receive any request messages */
                Assert.Equal(0, providerClient2.QueueSize());

                /* The first provider receives the two request messages */
                Assert.Equal(2, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());
                Assert.Equal(1, requestMsg.PriorityCount());

                message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());
                Assert.Equal(2, requestMsg.PriorityCount());

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                consumer.Unregister(itemHandle1);
                consumer.Unregister(itemHandle2);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingUnsupportedCapabilitiesByServiceNameAndServiceIdTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                int domainType = 100;

                long itemHandle1 = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").DomainType(domainType).Name("LSEG.O"),
                        consumerClient);

                Thread.Sleep(1000);

                /* Ensure that the provider doesn't receive any request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(domainType, statusMsg.DomainType());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Capability not supported", statusMsg.State().StatusText);

                Thread.Sleep(1000);

                consumer.Unregister(itemHandle1);

                long itemHandle2 = consumer.RegisterClient(reqMsg.Clear().ServiceId(32767).DomainType(domainType).Name("LSEG.O"),
                        consumerClient);

                Thread.Sleep(1000);

                /* Ensure that the provider doesn't receive any request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal(32767, statusMsg.ServiceId());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(domainType, statusMsg.DomainType());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Capability not supported", statusMsg.State().StatusText);

                Thread.Sleep(1000);

                consumer.Unregister(itemHandle2);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingWithSupportedCapabilitiesOnAnotherServerTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            /* Source directory for this the first provider */
            OmmArray capablities = new OmmArray();
            capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
            capablities.Complete();

            OmmArray dictionaryUsed = new OmmArray();
            dictionaryUsed.AddAscii("RWFFld");
            dictionaryUsed.AddAscii("RWFEnum");
            dictionaryUsed.Complete();

            ElementList serviceInfoId = new ElementList();

            serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
            serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
            serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
            serviceInfoId.Complete();

            ElementList serviceStateId = new ElementList();
            serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
            serviceStateId.Complete();

            FilterList filterList = new FilterList();
            filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
            filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
            filterList.Complete();

            Map map = new Map();
            map.AddKeyUInt(1, MapAction.ADD, filterList);
            map.Complete();

            providerTestOptions.SourceDirectoryPayload = map;

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1")
                    .AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient1);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);

            /* Source directory for this the first provider */
            OmmArray capablities2 = new OmmArray();
            capablities2.AddUInt(EmaRdm.MMT_MARKET_PRICE);
            capablities2.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
            capablities2.Complete();

            OmmArray dictionaryUsed2 = new OmmArray();
            dictionaryUsed2.AddAscii("RWFFld");
            dictionaryUsed2.AddAscii("RWFEnum");
            dictionaryUsed2.Complete();

            ElementList serviceInfoId2 = new ElementList();
            serviceInfoId2.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
            serviceInfoId2.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities2);
            serviceInfoId2.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed2);
            serviceInfoId2.Complete();

            ElementList serviceStateId2 = new ElementList();
            serviceStateId2.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
            serviceStateId2.Complete();

            FilterList filterList2 = new FilterList();
            filterList2.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId2);
            filterList2.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId2);
            filterList2.Complete();

            Map map2 = new Map();
            map2.AddKeyUInt(1, MapAction.ADD, filterList2);
            map2.Complete();

            providerTestOptions2.SourceDirectoryPayload = map2;

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1")
                    .AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                // The second provider provides the MARKET_PRICE domain 
                long itemHandle1 = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").DomainType(EmaRdm.MMT_MARKET_PRICE).Name("LSEG.O"),
                        consumerClient);

                /* Ensure that the first server doesn't receive the request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Thread.Sleep(500);

                /* The second receives the request message as it provides the MARKET_PRICE domain type */
                Assert.Equal(1, providerClient2.QueueSize());

                Msg message = providerClient2.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                Thread.Sleep(1000);

                consumer.Unregister(itemHandle1);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingUnsupportedQosTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle1 = consumer.RegisterClient(reqMsg.Qos(OmmQos.Timelinesses.INEXACT_DELAYED, OmmQos.Rates.JUST_IN_TIME_CONFLATED)
                    .ServiceName("DIRECT_FEED").DomainType(EmaRdm.MMT_MARKET_PRICE).Name("LSEG.O"),
                        consumerClient);

                Thread.Sleep(1000);

                /* Ensure that the provider doesn't receive any request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Service does not provide a matching QoS", statusMsg.State().StatusText);

                Thread.Sleep(1000);

                consumer.Unregister(itemHandle1);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();

                /* Ensure that there is no item status as the item is unregistered */
                Assert.Equal(0, consumerClient.QueueSize());

                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingUnsupportedQosByServiceNameAndServiceIdTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle1 = consumer.RegisterClient(reqMsg.Qos(OmmQos.Timelinesses.INEXACT_DELAYED, OmmQos.Rates.JUST_IN_TIME_CONFLATED).ServiceName("DIRECT_FEED").DomainType(EmaRdm.MMT_MARKET_PRICE).Name("LSEG.O"),
                        consumerClient);

                Thread.Sleep(1000);

                /* Ensure that the provider doesn't receive any request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Service does not provide a matching QoS", statusMsg.State().StatusText);

                Thread.Sleep(1000);

                consumer.Unregister(itemHandle1);

                itemHandle1 = consumer.RegisterClient(reqMsg.Clear().Qos(OmmQos.Timelinesses.INEXACT_DELAYED, OmmQos.Rates.JUST_IN_TIME_CONFLATED).ServiceId(32767)
                    .DomainType(EmaRdm.MMT_MARKET_PRICE).Name("LSEG.O"), consumerClient);

                Thread.Sleep(1000);

                /* Ensure that the provider doesn't receive any request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal(32767, statusMsg.ServiceId());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Service does not provide a matching QoS", statusMsg.State().StatusText);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSingleItemRequestWithGroupCloseRecoverableTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.ItemGroupId = new EmaBuffer(Encoding.ASCII.GetBytes("10"));

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new RequestMsg();

                int serviceId = 32767;

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.True(providerTestOptions.ItemGroupId.Equals(refreshMsg.ItemGroup()));
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Send item group recoverable status from the first provider */
                ElementList serviceGroupId = new ElementList();
                serviceGroupId.AddBuffer(EmaRdm.ENAME_GROUP, providerTestOptions.ItemGroupId);
                serviceGroupId.AddState(EmaRdm.ENAME_STATUS,
                        OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Group Status Msg");
                serviceGroupId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_GROUP_ID, FilterAction.SET, serviceGroupId);
                filterList.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterList);
                map.Complete();

                ommprovider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_GROUP_FILTER).
                        Payload(map), 0);   // use 0 item handle to fanout to all subscribers

                Thread.Sleep(2000); // Wait until consumer receives the group status message.

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(serviceId, statusMsg.ServiceId());
                Assert.True(providerTestOptions.ItemGroupId.Equals(statusMsg.ItemGroup()));
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Group Status Msg", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.True(providerTestOptions.ItemGroupId.Equals(refreshMsg.ItemGroup()));
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSingleItemRequestWithItemCloseRecoverableTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                int serviceId = 32767;

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Send item recoverable status from the first provider */

                long providerItemHandle = providerClient1.RetriveItemHandle("LSEG.O");

                ommprovider.Submit(new StatusMsg().DomainType(EmaRdm.MMT_MARKET_PRICE)
                        .State(OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Item temporary closed")
                        , providerItemHandle);

                Thread.Sleep(2000); // Wait until consumer receives the item closed recoverable status message.

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(serviceId, statusMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSingleItemRequestUnsubscribeItemWhenTheItemIsBeingRecoveredTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                int serviceId = 32767;

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Send source directory update to stop accepting item requests for both providers */
                ElementList serviceState = new ElementList();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceState.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 0);
                serviceState.Complete();

                FilterList filterListEnc = new FilterList();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, serviceState);
                filterListEnc.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers

                serviceState.Clear();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceState.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 0);
                serviceState.Complete();

                filterListEnc.Clear();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, serviceState);
                filterListEnc.Complete();

                map.Clear();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                ommprovider2.Submit(updateMsg.Clear().DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers

                Thread.Sleep(2000);

                long providerItemHandle = providerClient1.RetriveItemHandle("LSEG.O");

                ommprovider.Submit(new StatusMsg().DomainType(EmaRdm.MMT_MARKET_PRICE)
                        .State(OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Item temporary closed")
                        , providerItemHandle);

                Thread.Sleep(2000); // Wait until consumer receives the item closed recoverable status message.

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(serviceId, statusMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Item temporary closed", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(serviceId, statusMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                /* Unsubscribe item when it is being recovered. */
                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsItemClosedRecoverableAndWaitForServiceToAcceptRequestsTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                int serviceId = 32767;

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Send source directory update to stop accepting item requests for both providers */
                ElementList serviceState = new ElementList();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceState.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 0);
                serviceState.Complete();

                FilterList filterListEnc = new FilterList();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, serviceState);
                filterListEnc.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers

                serviceState.Clear();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceState.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 0);
                serviceState.Complete();

                filterListEnc.Clear();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, serviceState);
                filterListEnc.Complete();

                map.Clear();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                ommprovider2.Submit(updateMsg.Clear().DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers

                Thread.Sleep(1000);

                long providerItemHandle = providerClient1.RetriveItemHandle("LSEG.O");

                ommprovider.Submit(new StatusMsg().DomainType(EmaRdm.MMT_MARKET_PRICE)
                        .State(OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Item temporary closed")
                        , providerItemHandle);

                Thread.Sleep(1000); // Wait until consumer receives the item closed recoverable status message.

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(serviceId, statusMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Item temporary closed", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(serviceId, statusMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                Thread.Sleep(1000);

                /* Send source directory update for the second provider to accept requests */
                serviceState.Clear();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceState.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1);
                serviceState.Complete();

                filterListEnc.Clear();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, serviceState);
                filterListEnc.Complete();

                map.Clear();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                ommprovider2.Submit(updateMsg.Clear().DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers

                Thread.Sleep(3000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.Message);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void OmmConsumerItemRequestsWithServiceListNamesTest() /* The request routing feature is not enabled for this test. */
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendUpdateMessage = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1").AddServiceList(serviceList));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal(1, refreshMsg.ServiceId());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                message = consumerClient.PopMessage();

                UpdateMsg updateMsg = (UpdateMsg)message;

                Assert.Equal("DIRECT_FEED", updateMsg.ServiceName());
                Assert.Equal(1, updateMsg.ServiceId());
                Assert.Equal("LSEG.O", updateMsg.Name());

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsItemRequestsWithServiceListNamesTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendUpdateMessage = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                ServiceList serviceList2 = new ServiceList("SVG2");

                serviceList2.ConcreteServiceList.Add("DIRECT_FEED_2");
                serviceList2.ConcreteServiceList.Add("DIRECT_FEED");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList)
                        .AddServiceList(serviceList2));

                var consumerSession = consumer.m_OmmConsumerImpl!.ConsumerSession;

                int serviceId = consumerSession!.ServiceListDict!["SVG1"].ServiceId;
                int serviceId2 = consumerSession!.ServiceListDict!["SVG2"].ServiceId;

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                message = consumerClient.PopMessage();

                UpdateMsg updateMsg = (UpdateMsg)message;

                Assert.Equal("SVG1", updateMsg.ServiceName());
                Assert.Equal(serviceId, updateMsg.ServiceId());
                Assert.Equal("LSEG.O", updateMsg.Name());

                long itemHandle2 = consumer.RegisterClient(reqMsg.ServiceListName("SVG2").Name("LSEG2.O"), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED_2", requestMsg.ServiceName());
                Assert.Equal("LSEG2.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG2", refreshMsg.ServiceName());
                Assert.Equal(serviceId2, refreshMsg.ServiceId());
                Assert.Equal("LSEG2.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                message = consumerClient.PopMessage();

                updateMsg = (UpdateMsg)message;

                Assert.Equal("SVG2", updateMsg.ServiceName());
                Assert.Equal(serviceId2, updateMsg.ServiceId());
                Assert.Equal("LSEG2.O", updateMsg.Name());

                consumer.Unregister(itemHandle);
                consumer.Unregister(itemHandle2);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                Console.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsItemRequestsWithServiceListNamesWithAUnknownServiceInTheListTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("UNKNOWN_SERVICE");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));

                var consumerSession = consumer.m_OmmConsumerImpl!.ConsumerSession;

                int serviceId = consumerSession!.ServiceListDict!["SVG1"].ServiceId;

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsItemRequestsWithServiceListNameButConcreteServicesAreNotAvaliableThenConcreateServiceIsAddedTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("UNKNOWN_SERVICE");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                /* Provider send source directory update message to add the DIRECT_FEED2 service */
                OmmArray capablities = new OmmArray();
                capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
                capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
                capablities.Complete();

                OmmArray dictionaryUsed = new OmmArray();
                dictionaryUsed.AddAscii("RWFFld");
                dictionaryUsed.AddAscii("RWFEnum");
                dictionaryUsed.Complete();

                ElementList serviceInfoId = new ElementList();

                serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED2");
                serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
                serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
                serviceInfoId.Complete();

                ElementList serviceStateId = new ElementList();
                serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceStateId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
                filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
                filterList.Complete();

                Map map = new Map();
                map.AddKeyUInt(2, MapAction.ADD, filterList);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                                        Payload(map), 0);

                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED2", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(2, requestMsg.ServiceId());

                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;
                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionItemClosedReacoverableStatusWithServiceListTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));

                var consumerSession = consumer.m_OmmConsumerImpl!.ConsumerSession;
                int serviceId = consumerSession!.ServiceListDict!["SVG1"].ServiceId;

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Send item recoverable status from the first provider */
                long providerItemHandle = providerClient1.RetriveItemHandle("LSEG.O");

                ElementList serviceState = new ElementList();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN);
                serviceState.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1);
                serviceState.AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.OPEN, OmmState.DataStates.SUSPECT,
                    OmmState.StatusCodes.NONE);
                serviceState.Complete();

                FilterList filterListEnc = new FilterList();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.UPDATE, serviceState);
                filterListEnc.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers

                ommprovider.Submit(new StatusMsg().DomainType(EmaRdm.MMT_MARKET_PRICE)
                        .State(OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Item temporary closed")
                        , providerItemHandle);


                Thread.Sleep(2000); // Wait until consumer receives the item closed recoverable status message.

                /* The second provider receive a request message for the DIRECT_FEED_2 */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED_2", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                Thread.Sleep(2000);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionGroupClosedReacoverableStatusWithServiceListTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.ItemGroupId = new EmaBuffer(Encoding.ASCII.GetBytes("10"));

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));
                var consumerSession = consumer.m_OmmConsumerImpl!.ConsumerSession;
                int serviceId = consumerSession!.ServiceListDict!["SVG1"].ServiceId;

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Send item recoverable status from the first provider */
                long providerItemHandle = providerClient1.RetriveItemHandle("LSEG.O");

                ElementList serviceGroupId = new ElementList();
                serviceGroupId.AddBuffer(EmaRdm.ENAME_GROUP, providerTestOptions.ItemGroupId);
                serviceGroupId.AddState(EmaRdm.ENAME_STATUS,
                        OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Group Status Msg");
                serviceGroupId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_GROUP_ID, FilterAction.SET, serviceGroupId);
                filterList.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterList);
                map.Complete();

                ommprovider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_GROUP_FILTER).
                        Payload(map), 0);   // use 0 item handle to fanout to all subscribers

                Thread.Sleep(2000); // Wait until consumer receives the item closed recoverable status message.

                /* The second provider receive a request message for the DIRECT_FEED_2 */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED_2", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal(serviceId, statusMsg.ServiceId());
                Assert.True(providerTestOptions.ItemGroupId.Equals(statusMsg.ItemGroup()));
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.True(providerTestOptions.ItemGroupId.Equals(refreshMsg.ItemGroup()));
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                Thread.Sleep(2000);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionGroupOpenSuspectStatusWithServiceListTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.ItemGroupId = new EmaBuffer(Encoding.ASCII.GetBytes("10"));

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));
                var consumerSession = consumer.m_OmmConsumerImpl!.ConsumerSession;
                int serviceId = consumerSession!.ServiceListDict!["SVG1"].ServiceId;

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Send item recoverable status from the first provider */
                long providerItemHandle = providerClient1.RetriveItemHandle("LSEG.O");

                ElementList serviceGroupId = new ElementList();
                serviceGroupId.AddBuffer(EmaRdm.ENAME_GROUP, providerTestOptions.ItemGroupId);
                serviceGroupId.AddState(EmaRdm.ENAME_STATUS,
                        OmmState.StreamStates.OPEN, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Group Status Msg");
                serviceGroupId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_GROUP_ID, FilterAction.SET, serviceGroupId);
                filterList.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterList);
                map.Complete();

                ommprovider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_GROUP_FILTER).
                        Payload(map), 0);   // use 0 item handle to fanout to all subscribers

                Thread.Sleep(2000); // Wait until consumer receives the item closed recoverable status message.

                /* The second provider receive a request message for the DIRECT_FEED_2 */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED_2", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal(serviceId, statusMsg.ServiceId());
                Assert.True(providerTestOptions.ItemGroupId.Equals(statusMsg.ItemGroup()));
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Group Status Msg", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.True(providerTestOptions.ItemGroupId.Equals(refreshMsg.ItemGroup()));
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                Thread.Sleep(2000);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.Message);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsOnStreamPostingWithServiceNameAndIdTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendRefreshAttrib = true;
            providerTestOptions.SupportOMMPosting = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();


                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Submit a PostMsg to the first item stream */
                PostMsg postMsg = new PostMsg();
                UpdateMsg nestedUpdateMsg = new UpdateMsg();
                FieldList nestedFieldList = new FieldList();

                nestedFieldList.AddReal(22, 34, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddReal(25, 35, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddTime(18, 11, 29, 30);
                nestedFieldList.AddEnumValue(37, 3);
                nestedFieldList.Complete();

                nestedUpdateMsg.Payload(nestedFieldList);

                int postId = 0;

                consumer.Submit(postMsg.PostId(++postId).ServiceName("DIRECT_FEED")
                                                            .Name("IBM.N").SolicitAck(false).Complete(true)
                                                            .Payload(nestedUpdateMsg), itemHandle);

                Thread.Sleep(1000);

                /* Checks to ensure that the provider receives the PostMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is PostMsg);

                PostMsg recvPostMsg = (PostMsg)message;

                Assert.Equal("DIRECT_FEED", recvPostMsg.ServiceName());
                Assert.Equal("IBM.N", recvPostMsg.Name());
                Assert.Equal(1, recvPostMsg.PostId());
                Assert.True(recvPostMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvPostMsg.Payload().DataType);

                UpdateMsg updateMsg = recvPostMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                /* Ensure there is no more message from provider as the Ack flag is not set */
                Assert.Equal(0, consumerClient.QueueSize());

                /* Submit another PostMsg which requires Ack to the second provider. */
                consumer.Submit(postMsg.Clear().PostId(++postId).ServiceId(32767)
                        .Name("IBM.N").SolicitAck(true).Complete(true)
                        .Payload(nestedUpdateMsg), itemHandle);

                Thread.Sleep(1000);

                /* Checks to ensure that the provider receives the PostMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is PostMsg);

                recvPostMsg = (PostMsg)message;

                Assert.Equal("DIRECT_FEED", recvPostMsg.ServiceName());
                Assert.Equal("IBM.N", recvPostMsg.Name());
                Assert.Equal(2, recvPostMsg.PostId());
                Assert.True(recvPostMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvPostMsg.Payload().DataType);

                updateMsg = recvPostMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                Thread.Sleep(2000);
                /* Ensure that the client side receives ACK message from provider */
                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                AckMsg ackMessage = (AckMsg)message;

                Assert.Equal("DIRECT_FEED", ackMessage.ServiceName());
                Assert.Equal(32767, ackMessage.ServiceId());
                Assert.Equal("IBM.N", ackMessage.Name());
                Assert.Equal(2, ackMessage.AckId());

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsOnStreamPostingWithUnKnownServiceNameAndIdTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendRefreshAttrib = true;
            providerTestOptions.SupportOMMPosting = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();


                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Submit a PostMsg to the first item stream */
                PostMsg postMsg = new PostMsg();
                UpdateMsg nestedUpdateMsg = new UpdateMsg();
                FieldList nestedFieldList = new FieldList();

                nestedFieldList.AddReal(22, 34, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddReal(25, 35, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddTime(18, 11, 29, 30);
                nestedFieldList.AddEnumValue(37, 3);
                nestedFieldList.Complete();

                nestedUpdateMsg.Payload(nestedFieldList);

                /* This is invalid usage as the DIRECT_FEED_2 service name doesn't exist on the provider of this item stream but the second provider. */
                OmmInvalidUsageException expectedException = Assert.Throws<OmmInvalidUsageException>(() =>
                {
                    consumer.Submit(postMsg.PostId(1).ServiceName("DIRECT_FEED_2")
                                                            .Name("IBM.N").SolicitAck(false).Complete(true)
                                                            .Payload(nestedUpdateMsg), itemHandle);
                });

                Assert.StartsWith("Failed to submit PostMsg on item stream. Reason: INVALID_USAGE. Error text: " +
                    "Message submitted with unknown service name DIRECT_FEED_2", expectedException.Message);

                int serviceId = consumer.m_OmmConsumerImpl!.ConsumerSession!.GetSessionDirectoryByName("DIRECT_FEED_2")!.Service!.ServiceId;

                /* This is invalid usage as the service Id doesn't exist on the provider of this item stream but the second provider */
                expectedException = Assert.Throws<OmmInvalidUsageException>(() =>
                    consumer.Submit(postMsg.Clear().PostId(2).ServiceId(serviceId)
                    .Name("IBM.N").SolicitAck(true).Complete(true)
                    .Payload(nestedUpdateMsg), itemHandle));

                Assert.StartsWith("Failed to submit PostMsg on item stream. Reason: INVALID_USAGE. Error text: "
                    + "Message submitted with unknown service Id 32768", expectedException.Message);

                /* This is invalid usage as the UNKNOWN_FEED service name doesn't exist in any providers */
                expectedException = Assert.Throws<OmmInvalidUsageException>(() => consumer.Submit(postMsg.PostId(1).ServiceName("UNKNOWN_FEED")
                                                        .Name("IBM.N").SolicitAck(false).Complete(true)
                                                        .Payload(nestedUpdateMsg), itemHandle));

                Assert.StartsWith("Failed to submit PostMsg on item stream. Reason: INVALID_USAGE. Error text: "
                    + "Message submitted with unknown service name UNKNOWN_FEED", expectedException.Message);


                /* This is invalid usage as the 65535 service Id doesn't exist in any providers */
                expectedException = Assert.Throws<OmmInvalidUsageException>(() => consumer.Submit(postMsg.Clear().PostId(2).ServiceId(65535)
                    .Name("IBM.N").SolicitAck(true).Complete(true)
                    .Payload(nestedUpdateMsg), itemHandle));

                Assert.StartsWith("Failed to submit PostMsg on item stream. Reason: INVALID_USAGE. Error text: "
                    + "Message submitted with unknown service Id 65535", expectedException.Message);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsOffStreamPostingWithServiceNameAndIdTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendRefreshAttrib = true;
            providerTestOptions.SupportOMMPosting = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();


                long loginHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_LOGIN), consumerClient);

                Thread.Sleep(1000);

                /* This is login refresh message message */
                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Submit a PostMsg to the first item stream */
                PostMsg postMsg = new PostMsg();
                UpdateMsg nestedUpdateMsg = new UpdateMsg();
                FieldList nestedFieldList = new FieldList();

                nestedFieldList.AddReal(22, 34, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddReal(25, 35, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddTime(18, 11, 29, 30);
                nestedFieldList.AddEnumValue(37, 3);
                nestedFieldList.Complete();

                nestedUpdateMsg.Payload(nestedFieldList);

                int postId = 0;

                consumer.Submit(postMsg.PostId(++postId).ServiceName("DIRECT_FEED")
                                                            .Name("IBM.N").SolicitAck(false).Complete(true)
                                                            .Payload(nestedUpdateMsg), loginHandle);

                Thread.Sleep(1000);

                /* Checks to ensure that the provider receives the PostMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is PostMsg);

                PostMsg recvPostMsg = (PostMsg)message;

                Assert.Equal("DIRECT_FEED", recvPostMsg.ServiceName());
                Assert.Equal("IBM.N", recvPostMsg.Name());
                Assert.Equal(1, recvPostMsg.PostId());
                Assert.True(recvPostMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvPostMsg.Payload().DataType);

                UpdateMsg updateMsg = recvPostMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                /* Ensure there is no more message from provider as the Ack flag is not set */
                Assert.Equal(0, consumerClient.QueueSize());

                /* Submit another PostMsg which requires Ack to the second provider. */
                consumer.Submit(postMsg.Clear().PostId(++postId).ServiceId(32767)
                        .Name("IBM.N").SolicitAck(true).Complete(true)
                        .Payload(nestedUpdateMsg), loginHandle);

                Thread.Sleep(2000);

                /* Checks to ensure that the provider receives the PostMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is PostMsg);

                recvPostMsg = (PostMsg)message;

                Assert.Equal("DIRECT_FEED", recvPostMsg.ServiceName());
                Assert.Equal("IBM.N", recvPostMsg.Name());
                Assert.Equal(2, recvPostMsg.PostId());
                Assert.True(recvPostMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvPostMsg.Payload().DataType);

                updateMsg = recvPostMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                Thread.Sleep(2000);
                /* Ensure that the client side receives ACK message from provider */
                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                AckMsg ackMessage = (AckMsg)message;

                Assert.Equal("DIRECT_FEED", ackMessage.ServiceName());
                Assert.Equal(32767, ackMessage.ServiceId());
                Assert.Equal("IBM.N", ackMessage.Name());
                Assert.Equal(2, ackMessage.AckId());

                consumer.Unregister(loginHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSubmittingGenericMsgWithoutSpecifyingServiceIdTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendGenericMessage = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Submit a GenericMsg to the first item stream */
                GenericMsg genericMsg = new GenericMsg();
                UpdateMsg nestedUpdateMsg = new UpdateMsg();
                FieldList nestedFieldList = new FieldList();

                nestedFieldList.AddReal(22, 34, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddReal(25, 35, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddTime(18, 11, 29, 30);
                nestedFieldList.AddEnumValue(37, 3);
                nestedFieldList.Complete();

                nestedUpdateMsg.Payload(nestedFieldList);

                consumer.Submit(genericMsg.Name("genericMsg").DomainType(200).Complete(true).Payload(nestedUpdateMsg), itemHandle);

                Thread.Sleep(1000);

                /* Checks to ensure that the provider receives the GenericMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is GenericMsg);

                GenericMsg recvGenericMsg = (GenericMsg)message;

                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.False(recvGenericMsg.HasServiceId);
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.Payload().DataType);

                UpdateMsg updateMsg = recvGenericMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                /* Ensure there is no more message from provider */
                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.False(recvGenericMsg.HasServiceId);
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSubmittingGenericMsgWithServiceIdAndUnknownServiceIdTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendGenericMessage = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O"), consumerClient);

                Thread.Sleep(1000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Submit a GenericMsg to the first item stream */
                GenericMsg genericMsg = new GenericMsg();
                UpdateMsg nestedUpdateMsg = new UpdateMsg();
                FieldList nestedFieldList = new FieldList();

                nestedFieldList.AddReal(22, 34, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddReal(25, 35, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddTime(18, 11, 29, 30);
                nestedFieldList.AddEnumValue(37, 3);
                nestedFieldList.Complete();

                nestedUpdateMsg.Payload(nestedFieldList);

                consumer.Submit(genericMsg.Name("genericMsg").DomainType(200).ServiceId(32767).Complete(true).Payload(nestedUpdateMsg), itemHandle);

                Thread.Sleep(3000);

                /* Checks to ensure that the provider receives the GenericMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is GenericMsg);

                GenericMsg recvGenericMsg = (GenericMsg)message;

                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.HasServiceId);
                Assert.Equal(1, recvGenericMsg.ServiceId());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.Payload().DataType);

                UpdateMsg updateMsg = recvGenericMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                /* Ensure that Consumer receives a generic message from provider */
                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.HasServiceId);
                Assert.Equal(32767, recvGenericMsg.ServiceId());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                /* Submit unknown service Id which there is no translation */
                consumer.Submit(genericMsg.Name("genericMsg2").DomainType(205).ServiceId(555)
                        .Complete(true).Payload(nestedUpdateMsg), itemHandle);

                Thread.Sleep(1000);

                /* Checks to ensure that the provider receives the GenericMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is GenericMsg);

                recvGenericMsg = (GenericMsg)message;

                Assert.Equal("genericMsg2", recvGenericMsg.Name());
                Assert.Equal(205, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.HasServiceId);
                Assert.Equal(555, recvGenericMsg.ServiceId());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.Payload().DataType);

                updateMsg = recvGenericMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                /* Ensure that Consumer receives a generic message from provider */
                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal("genericMsg2", recvGenericMsg.Name());
                Assert.Equal(205, recvGenericMsg.DomainType());
                Assert.False(recvGenericMsg.HasServiceId);
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSubmittingGenericMsgWithoutSpecifyingServiceIdViaLoginDomainTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendGenericMessage = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name 
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();


                long loginHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_LOGIN), consumerClient);

                Thread.Sleep(1000);

                /* This is login refresh message message */
                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Submit a GenericMsg to the first item stream */
                GenericMsg genericMsg = new GenericMsg();
                UpdateMsg nestedUpdateMsg = new UpdateMsg();
                FieldList nestedFieldList = new FieldList();

                nestedFieldList.AddReal(22, 34, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddReal(25, 35, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddTime(18, 11, 29, 30);
                nestedFieldList.AddEnumValue(37, 3);
                nestedFieldList.Complete();

                // nestedUpdateMsg.Payload(nestedFieldList);
                /* The service Id is required in order to decode the GenericMsg's payload which includes FieldList properly */
                consumer.Submit(genericMsg.Name("genericMsg").DomainType(200).Complete(true).Payload(nestedUpdateMsg), loginHandle);

                Thread.Sleep(1000);

                /* Checks to ensure that the first provider receives the GenericMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is GenericMsg);

                GenericMsg recvGenericMsg = (GenericMsg)message;

                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.Payload().DataType);

                UpdateMsg updateMsg = recvGenericMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.NO_DATA, updateMsg.Payload().DataType);

                /* Checks to ensure that the second provider receives the GenericMsg */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();

                Assert.True(message is GenericMsg);

                recvGenericMsg = (GenericMsg)message;

                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.Payload().DataType);

                updateMsg = recvGenericMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.NO_DATA, updateMsg.Payload().DataType);

                /* Ensure that receives two generic messages from the two providers.*/
                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.False(recvGenericMsg.HasServiceId);
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.False(recvGenericMsg.HasServiceId);
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                consumer.Unregister(loginHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSubmittingGenericMsgWithServiceIdAndUnknownServiceIdViaLoginDomainTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendGenericMessage = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name 
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long loginHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_LOGIN), consumerClient);

                Thread.Sleep(1000);

                /* This is login refresh message message */
                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Submit a GenericMsg to the first item stream */
                GenericMsg genericMsg = new GenericMsg();
                UpdateMsg nestedUpdateMsg = new UpdateMsg();
                FieldList nestedFieldList = new FieldList();

                nestedFieldList.AddReal(22, 34, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddReal(25, 35, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddTime(18, 11, 29, 30);
                nestedFieldList.AddEnumValue(37, 3);
                nestedFieldList.Complete();

                nestedUpdateMsg.Payload(nestedFieldList);
                /* The service Id is required in order to decode the GenericMsg's payload which includes FieldList properly */
                consumer.Submit(genericMsg.Name("genericMsg").DomainType(200).Complete(true).ServiceId(32767).Payload(nestedUpdateMsg), loginHandle);

                Thread.Sleep(1000);

                /* Checks to ensure that the first provider receives the GenericMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is GenericMsg);

                GenericMsg recvGenericMsg = (GenericMsg)message;

                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.Payload().DataType);

                UpdateMsg updateMsg = recvGenericMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                /* Checks to ensure that the second provider receives the GenericMsg */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();

                Assert.True(message is GenericMsg);

                recvGenericMsg = (GenericMsg)message;

                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.ERROR, recvGenericMsg.Payload().DataType);

                /* Ensure that receives two generic message from the two providers.*/
                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.False(recvGenericMsg.HasServiceId);
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.False(recvGenericMsg.HasServiceId);
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                /* Submit unknown service Id which there is no translation */
                consumer.Submit(genericMsg.Name("genericMsg2").DomainType(205).ServiceId(555)
                        .Complete(true).Payload(nestedUpdateMsg), loginHandle);

                Thread.Sleep(1000);

                /* Checks to ensure that the first provider receives the GenericMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is GenericMsg);

                recvGenericMsg = (GenericMsg)message;

                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg2", recvGenericMsg.Name());
                Assert.Equal(205, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.ERROR, recvGenericMsg.Payload().DataType);

                /* Checks to ensure that the second provider receives the GenericMsg */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();

                Assert.True(message is GenericMsg);

                recvGenericMsg = (GenericMsg)message;

                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg2", recvGenericMsg.Name());
                Assert.Equal(205, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.ERROR, recvGenericMsg.Payload().DataType);

                /* Ensure that receives two generic message from the two providers.*/
                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg2", recvGenericMsg.Name());
                Assert.Equal(205, recvGenericMsg.DomainType());
                Assert.False(recvGenericMsg.HasServiceId);
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg2", recvGenericMsg.Name());
                Assert.Equal(205, recvGenericMsg.DomainType());
                Assert.False(recvGenericMsg.HasServiceId);
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                consumer.Unregister(loginHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSingleItemRequestWithUnmatchedQoSThenServiceProvideTheRequestedQoSTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                var consumerSession = consumer.m_OmmConsumerImpl!.ConsumerSession;

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O").Qos(55, 55), consumerClient);

                Thread.Sleep(2000);

                /* Ensure that the provider doesn't receive the request message as the requested QoS doesn't match */
                Assert.Equal(0, providerClient1.QueueSize());

                /* Receive a StatusMsg from EMA */
                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Service does not provide a matching QoS", statusMsg.State().StatusText);

                /* Provider sends source directory update to delete the DIRECT_FEED service name. */
                Map map = new Map();
                map.AddKeyUInt(1, MapAction.DELETE, new FilterList());
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                        Payload(map), 0);

                Thread.Sleep(1000);

                /* Provider sends source directory update message to add the DIRECT_FEED service */
                OmmArray capablities = new OmmArray();
                capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
                capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
                capablities.Complete();

                OmmArray dictionaryUsed = new OmmArray();
                dictionaryUsed.AddAscii("RWFFld");
                dictionaryUsed.AddAscii("RWFEnum");
                dictionaryUsed.Complete();

                OmmArray qosList = new OmmArray();
                qosList.AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK);
                qosList.AddQos(55, 55);
                qosList.Complete();

                ElementList serviceInfoId = new ElementList();
                serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
                serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
                serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
                serviceInfoId.AddArray(EmaRdm.ENAME_QOS, qosList);
                serviceInfoId.AddAscii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist");
                serviceInfoId.AddUInt(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0);
                serviceInfoId.Complete();

                ElementList serviceStateId = new ElementList();
                serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceStateId.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1);
                serviceStateId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
                filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
                filterList.Complete();

                map.Clear();
                map.AddKeyUInt(1, MapAction.ADD, filterList);
                map.Complete();

                updateMsg.Clear();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                                        Payload(map), 0);

                Thread.Sleep(2000);

                /* Provider receives the request message */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                RequestMsg recvReq = (RequestMsg)message;

                Assert.Equal(3, recvReq.StreamId());
                Assert.Equal("DIRECT_FEED", recvReq.ServiceName());
                Assert.Equal("LSEG.O", recvReq.Name());
                Assert.Equal(1, recvReq.ServiceId());

                /* Receives a refresh from the provider */
                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                /* Receives Refresh message from the active server of the second connection */
                RefreshMsg refreshMsg = (RefreshMsg)message;
                Assert.Equal(5, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_MARKET_PRICE, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(consumerSession!.GetSessionDirectoryByName("DIRECT_FEED")!.Service!.ServiceId, refreshMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(DataType.DataTypes.FIELD_LIST, refreshMsg.Payload().DataType);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSingleItemRequestServiceListWithUnmatchedQoSThenServiceProvideTheRequestedQoSTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O").Qos(55, 55), consumerClient);

                Thread.Sleep(2000);

                /* Ensure that the provider doesn't receive the request message as the requested QoS doesn't match */
                Assert.Equal(0, providerClient1.QueueSize());

                /* Receive a StatusMsg from EMA */
                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Service does not provide a matching QoS", statusMsg.State().StatusText);

                /* Provider sends source directory update to delete the DIRECT_FEED service name. */
                Map map = new Map();
                map.AddKeyUInt(1, MapAction.DELETE, new FilterList());
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                        Payload(map), 0);

                Thread.Sleep(1000);

                /* Provider sends source directory update message to add the DIRECT_FEED service */
                OmmArray capablities = new OmmArray();
                capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
                capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
                capablities.Complete();

                OmmArray dictionaryUsed = new OmmArray();
                dictionaryUsed.AddAscii("RWFFld");
                dictionaryUsed.AddAscii("RWFEnum");
                dictionaryUsed.Complete();

                OmmArray qosList = new OmmArray();
                qosList.AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK);
                qosList.AddQos(55, 55);
                qosList.Complete();

                ElementList serviceInfoId = new ElementList();
                serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
                serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
                serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
                serviceInfoId.AddArray(EmaRdm.ENAME_QOS, qosList);
                serviceInfoId.AddAscii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist");
                serviceInfoId.AddUInt(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0);
                serviceInfoId.Complete();

                ElementList serviceStateId = new ElementList();
                serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceStateId.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1);
                serviceStateId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
                filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
                filterList.Complete();

                map.Clear();
                map.AddKeyUInt(1, MapAction.ADD, filterList);
                map.Complete();

                updateMsg.Clear();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                                        Payload(map), 0);

                Thread.Sleep(2000);

                /* Provider receives the request message */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                RequestMsg recvReq = (RequestMsg)message;

                Assert.Equal(3, recvReq.StreamId());
                Assert.Equal("DIRECT_FEED", recvReq.ServiceName());
                Assert.Equal("LSEG.O", recvReq.Name());
                Assert.Equal(1, recvReq.ServiceId());

                /* Receives a refresh from the provider */
                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;
                Assert.Equal(5, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_MARKET_PRICE, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(DataType.DataTypes.FIELD_LIST, refreshMsg.Payload().DataType);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }
        [Fact]
        public void MultiConnectionsSingleItemRequestWithUnmatchedCapabilityThenServiceProvideTheRequestedCapabilityTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                var consumerSession = consumer.m_OmmConsumerImpl!.ConsumerSession;

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O").DomainType(55), consumerClient);

                Thread.Sleep(3000);

                /* Ensure that the provider doesn't receive the request message as the requested capability doesn't match */
                Assert.Equal(0, providerClient1.QueueSize());

                /* Receive a StatusMsg from EMA */
                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(55, statusMsg.DomainType());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Capability not supported", statusMsg.State().StatusText);

                /* Provider sends source directory update to delete the DIRECT_FEED service name. */
                Map map = new Map();
                map.AddKeyUInt(1, MapAction.DELETE, new FilterList());
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                        Payload(map), 0);

                Thread.Sleep(1000);

                /* Provider sends source directory update message to add the DIRECT_FEED service */
                OmmArray capablities = new OmmArray();
                capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
                capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
                capablities.AddUInt(55);
                capablities.Complete();

                OmmArray dictionaryUsed = new OmmArray();
                dictionaryUsed.AddAscii("RWFFld");
                dictionaryUsed.AddAscii("RWFEnum");
                dictionaryUsed.Complete();

                OmmArray qosList = new OmmArray();
                qosList.AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK);
                qosList.AddQos(55, 55);
                qosList.Complete();

                ElementList serviceInfoId = new ElementList();
                serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
                serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
                serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
                serviceInfoId.AddArray(EmaRdm.ENAME_QOS, qosList);
                serviceInfoId.AddAscii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist");
                serviceInfoId.AddUInt(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0);
                serviceInfoId.Complete();

                ElementList serviceStateId = new ElementList();
                serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceStateId.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1);
                serviceStateId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
                filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
                filterList.Complete();

                map.Clear();
                map.AddKeyUInt(1, MapAction.ADD, filterList);
                map.Complete();

                updateMsg.Clear();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                                        Payload(map), 0);

                Thread.Sleep(2000);

                /* Provider receives the request message */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                RequestMsg recvReq = (RequestMsg)message;

                Assert.Equal(3, recvReq.StreamId());
                Assert.Equal("DIRECT_FEED", recvReq.ServiceName());
                Assert.Equal("LSEG.O", recvReq.Name());
                Assert.Equal(55, recvReq.DomainType());
                Assert.Equal(1, recvReq.ServiceId());

                /* Receives a refresh from the provider */
                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;
                Assert.Equal(5, refreshMsg.StreamId());
                Assert.Equal(55, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(consumerSession!.GetSessionDirectoryByName("DIRECT_FEED")!.Service!.ServiceId, refreshMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(DataType.DataTypes.FIELD_LIST, refreshMsg.Payload().DataType);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsSingleItemRequestServiceListWithUnmatchedCapabilityThenServiceProvideTheRequestedCapabilityTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));

                var consumerSession = consumer.m_OmmConsumerImpl!.ConsumerSession;

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O").DomainType(55), consumerClient);

                Thread.Sleep(2000);

                /* Ensure that the provider doesn't receive the request message as the requested capability doesn't match */
                Assert.Equal(0, providerClient1.QueueSize());

                /* Receive a StatusMsg from EMA */
                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(55, statusMsg.DomainType());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Capability not supported", statusMsg.State().StatusText);

                /* Provider sends source directory update to delete the DIRECT_FEED service name. */
                Map map = new Map();
                map.AddKeyUInt(1, MapAction.DELETE, new FilterList());
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                        Payload(map), 0);

                Thread.Sleep(1000);

                /* Provider sends source directory update message to add the DIRECT_FEED service */
                OmmArray capablities = new OmmArray();
                capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
                capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
                capablities.AddUInt(55);
                capablities.Complete();

                OmmArray dictionaryUsed = new OmmArray();
                dictionaryUsed.AddAscii("RWFFld");
                dictionaryUsed.AddAscii("RWFEnum");
                dictionaryUsed.Complete();

                OmmArray qosList = new OmmArray();
                qosList.AddQos(OmmQos.Timelinesses.REALTIME, OmmQos.Rates.TICK_BY_TICK);
                qosList.AddQos(55, 55);
                qosList.Complete();

                ElementList serviceInfoId = new ElementList();
                serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
                serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
                serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
                serviceInfoId.AddArray(EmaRdm.ENAME_QOS, qosList);
                serviceInfoId.AddAscii(EmaRdm.ENAME_ITEM_LIST, "#.itemlist");
                serviceInfoId.AddUInt(EmaRdm.ENAME_SUPPS_QOS_RANGE, 0);
                serviceInfoId.Complete();

                ElementList serviceStateId = new ElementList();
                serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceStateId.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1);
                serviceStateId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
                filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
                filterList.Complete();

                map.Clear();
                map.AddKeyUInt(1, MapAction.ADD, filterList);
                map.Complete();

                updateMsg.Clear();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                                        Payload(map), 0);

                Thread.Sleep(2000);

                /* Provider receives the request message */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                RequestMsg recvReq = (RequestMsg)message;

                Assert.Equal(3, recvReq.StreamId());
                Assert.Equal("DIRECT_FEED", recvReq.ServiceName());
                Assert.Equal("LSEG.O", recvReq.Name());
                Assert.Equal(55, recvReq.DomainType());
                Assert.Equal(1, recvReq.ServiceId());

                /* Receives a refresh from the provider */
                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;
                Assert.Equal(5, refreshMsg.StreamId());
                Assert.Equal(55, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(DataType.DataTypes.FIELD_LIST, refreshMsg.Payload().DataType);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void SingleItemRecoverFromRequestTimeoutWithPrivateStreamTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendItemResponse = false;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                /* The item request timeout is set to 3 seconds for Consumer_9 */
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle1 = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O").PrivateStream(true), consumerClient);

                Thread.Sleep(7000);

                /* Ensure that the provider receives a request message but it doesn't send a response back. */
                Assert.Equal(3, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();
                RequestMsg recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                message = providerClient1.PopMessage();
                recvReqMsg = (RequestMsg)message;

                /* This is close message from consumer */
                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                message = providerClient1.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(5, statusMsg.StreamId());
                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED_RECOVER, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Request timeout", statusMsg.State().StatusText);

                /* The second provider doesn't receive any request message */
                Assert.Equal(0, providerClient2.QueueSize());

                consumer.Unregister(itemHandle1);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithRequestedServiceDownWithOpenSuspectStatusWithPrivateStreamTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O").PrivateStream(true), consumerClient);

                Thread.Sleep(3000);

                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* The first provider sends a source directory update message to change service state to DOWN with service's status */
                ElementList serviceState = new ElementList();
                serviceState.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN);
                serviceState.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, 1);
                serviceState.AddState(EmaRdm.ENAME_STATUS, OmmState.StreamStates.OPEN, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE);
                serviceState.Complete();

                FilterList filterListEnc = new FilterList();
                filterListEnc.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceState);
                filterListEnc.Complete();

                Map map = new Map();
                map.AddKeyUInt(1, MapAction.UPDATE, filterListEnc);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                        Filter(EmaRdm.SERVICE_STATE_FILTER).
                        Payload(map), 0);   // use 0 item handle to fan-out to all subscribers


                Thread.Sleep(2000); // Wait until consumer receives the item Open/Suspect status message from the VA Watchlist

                /* Ensure that second provider doesn't receive the request message */
                Assert.Equal(0, providerClient2.QueueSize());
                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryWithPrivateStreamTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O").PrivateStream(true), consumerClient);

                Thread.Sleep(1000);

                /* Checks provider that receives the item request. */
                Msg message;
                RequestMsg? requestMsg = null;

                message = providerClient1.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Force channel down on the first provider */
                RequestMsg recvReqMsg;
                ommprovider.Uninitialize();

                Thread.Sleep(1000);

                ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

                Thread.Sleep(2000);

                /* Ensures that the first provider receives the login request message  */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(1, recvReqMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, recvReqMsg.DomainType());

                Thread.Sleep(3000);

                /* Ensures that the first provider doesn't receive the request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED_RECOVER, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("channel down.", statusMsg.State().StatusText);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithRequestedConnectionDownWithEnableSessionEnhancedItemRecoveryWithPrivateStreamTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_12"));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("LSEG.O").PrivateStream(true), consumerClient);

                Thread.Sleep(3000);

                /* Checks provider that receives the item request. */
                Msg message;
                RequestMsg? requestMsg = null;

                message = providerClient1.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(2000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Force channel down on provider */
                ommprovider.Uninitialize();

                Thread.Sleep(2000);

                /* Ensure that the second provider doesn't receive the request message */
                Assert.Equal(0, providerClient2.QueueSize());

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED_RECOVER, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("channel down.", statusMsg.State().StatusText);

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithUnknownServiceNameAndServiceIdThenCloseConsumerSessionTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("FEED_1");
                serviceList.ConcreteServiceList.Add("FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));

                RequestMsg reqMsg = new();

                consumer.RegisterClient(reqMsg.ServiceName("UNKNOWN_SERVICE").Name("itemA"), consumerClient);

                consumer.RegisterClient(reqMsg.Clear().ServiceName("UNKNOWN_SERVICE").Name("itemB"), consumerClient);

                consumer.RegisterClient(reqMsg.Clear().ServiceListName("SVG1").Name("itemC"), consumerClient);

                Thread.Sleep(2000);

                /* Ensure that the provider doesn't receive any request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(3, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("UNKNOWN_SERVICE", statusMsg.ServiceName());
                Assert.Equal("itemA", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("UNKNOWN_SERVICE", statusMsg.ServiceName());
                Assert.Equal("itemB", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("itemC", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                // Uninitialized OmmConsumer to get item closed status
                m_Output.WriteLine("Uninitializing...");
                consumer.Uninitialize();

                Assert.Equal(3, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("UNKNOWN_SERVICE", statusMsg.ServiceName());
                Assert.Equal("itemA", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Consumer session is closed.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("UNKNOWN_SERVICE", statusMsg.ServiceName());
                Assert.Equal("itemB", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Consumer session is closed.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("itemC", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Consumer session is closed.", statusMsg.State().StatusText);

            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithRequestedConnectionDownWithDisableSessionEnhancedItemRecoveryThenCloseConsumerSessionTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));

                RequestMsg reqMsg = new();

                consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED").Name("itemA"), consumerClient);

                consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("itemB"), consumerClient);

                consumer.RegisterClient(reqMsg.Clear().ServiceListName("SVG1").Name("itemC"), consumerClient);

                Thread.Sleep(2000);

                /* Checks provider that receives the item request. */
                Msg message;
                RequestMsg? requestMsg = null;

                Assert.Equal(3, providerClient1.QueueSize());

                message = providerClient1.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemA", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                message = providerClient1.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemB", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                message = providerClient1.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("itemC", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(1000);

                Assert.Equal(3, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("itemA", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal("itemB", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("itemC", refreshMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Force channel down on the first provider */
                ommprovider.Uninitialize();

                Thread.Sleep(2000);

                m_Output.WriteLine("Uninitializing...");
                consumer.Uninitialize();

                StatusMsg statusMsg;

                Assert.Equal(6, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("itemA", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("channel down.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("itemB", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("channel down.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();
                statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("itemC", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("channel down.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("itemA", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Consumer session is closed.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();
                statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.Equal("itemB", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Consumer session is closed.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();
                statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("itemC", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("Consumer session is closed.", statusMsg.State().StatusText);

            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingTheSameItemNameAndServiceListNameTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));

                RequestMsg reqMsg = new();

                int serviceId = 32767;

                long itemHandle1 = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O"), consumerClient);

                long itemHandle2 = consumer.RegisterClient(reqMsg.Clear().ServiceListName("SVG1").Name("LSEG.O"), consumerClient);

                Thread.Sleep(3000);

                /* The second provider doesn't receive any request messages */
                Assert.Equal(0, providerClient2.QueueSize());

                /* The first provider receives the two request messages */
                Assert.Equal(2, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                RequestMsg requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());
                Assert.Equal(1, requestMsg.PriorityCount());

                message = providerClient1.PopMessage();

                Assert.True(message is RequestMsg);

                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());
                Assert.Equal(2, requestMsg.PriorityCount());

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Complete());

                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Complete());

                consumer.Unregister(itemHandle1);
                consumer.Unregister(itemHandle2);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithRequestedServiceListConnectionDownWithDisableSessionEnhancedItemRecoveryTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).AddServiceList(serviceList).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O"), consumerClient);

                Thread.Sleep(1000);

                /* Checks provider that receives the item request. */
                Msg message;
                RequestMsg? requestMsg = null;

                message = providerClient1.PopMessage();
                Assert.True(message is RequestMsg);
                requestMsg = (RequestMsg)message;

                Assert.Equal("DIRECT_FEED", requestMsg.ServiceName());
                Assert.Equal("LSEG.O", requestMsg.Name());
                Assert.Equal(1, requestMsg.ServiceId());

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Force channel down on the first provider */
                RequestMsg recvReqMsg;
                ommprovider.Uninitialize();

                Thread.Sleep(1000);

                ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

                Thread.Sleep(3000);

                /* The first provider receives the request message */
                Assert.Equal(1, providerClient1.QueueSize());

                /* Ensure that the second provider doesn't receive any request message */
                Assert.Equal(0, providerClient2.QueueSize());

                message = providerClient1.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(1, recvReqMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, recvReqMsg.DomainType());

                Thread.Sleep(4000);
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(4, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.O", recvReqMsg.Name());

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);

                /* Receives refresh message from the first provider */
                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                refreshMsg = (RefreshMsg)message;

                Assert.Equal("SVG1", refreshMsg.ServiceName());
                Assert.Equal("LSEG.O", refreshMsg.Name());
                Assert.Equal(32767, refreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());

                consumer.Unregister(itemHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void GetSessionInformationFromOmmConsumerTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendUpdateMessage = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9_1"), consumerClient);

                ChannelInformation channelInformation = new ChannelInformation();

                OmmInvalidUsageException expectedException = Assert.Throws<OmmInvalidUsageException>(()
                                        => { consumer.ChannelInformation(channelInformation); });

                Assert.Equal("The request routing feature do not support the ChannelInformation method. The SessionChannelInfo() must be used instead.", expectedException.Message);

                List<ChannelInformation> channelList = new List<ChannelInformation>();
                consumer.SessionChannelInfo(channelList);

                Assert.Equal(2, channelList.Count);
                ChannelInformation channelInfo = channelList[0];
                ChannelInformation channelInfo2 = channelList[1];

                Assert.Equal("Channel_1", channelInfo.ChannelName);
                Assert.Equal("Connection_3", channelInfo.SessionChannelName);
                Assert.Equal("not available for OmmConsumer connections", channelInfo.IpAddress);
                Assert.Equal(19001, channelInfo.Port);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);

                Assert.Equal("Channel_4", channelInfo2.ChannelName);
                Assert.Equal("Connection_4", channelInfo2.SessionChannelName);
                Assert.Equal(19004, channelInfo2.Port);
                Assert.Equal(ChannelState.ACTIVE, channelInfo2.ChannelState);
                Assert.Equal("not available for OmmConsumer connections", channelInfo2.IpAddress);

                // Shutdown the first provider to remove the first session channel.
                ommprovider.Uninitialize();

                // Wait to remove the session channel
                Thread.Sleep(5000);

                // Gets channel info list again.
                consumer.SessionChannelInfo(channelList);

                Assert.Single(channelList!);

                channelInfo = channelList[0];
                Assert.Equal("Channel_4", channelInfo.ChannelName);
                Assert.Equal("Connection_4", channelInfo.SessionChannelName);
                Assert.Equal(19004, channelInfo.Port);
                Assert.Equal(ChannelState.ACTIVE, channelInfo.ChannelState);
                Assert.Equal("not available for OmmConsumer connections", channelInfo.IpAddress);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithServiceListReceivedCloseSuspectFromProviderTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.CloseItemRequest = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("DIRECT_FEED");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED_2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));

                RequestMsg reqMsg = new();

                consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.L"), consumerClient);

                Thread.Sleep(2000);

                /* Ensure that the provider receives a request message but it closes the item stream. */
                Assert.Equal(1, providerClient1.QueueSize());

                Msg message = providerClient1.PopMessage();
                RequestMsg recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.L", recvReqMsg.Name());

                Thread.Sleep(1000);

                /* Ensure that the first provider doesn't receive a close message */
                Assert.Equal(0, providerClient1.QueueSize());

                /* Ensure that the second provider receive a request message but it closes the item stream */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();
                recvReqMsg = (RequestMsg)message;

                Assert.Equal(3, recvReqMsg.StreamId());
                Assert.Equal(1, recvReqMsg.ServiceId());
                Assert.Equal("DIRECT_FEED_2", recvReqMsg.ServiceName());
                Assert.Equal("LSEG.L", recvReqMsg.Name());

                Assert.Equal(2, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(5, statusMsg.StreamId());
                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("LSEG.L", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, statusMsg.State().StatusCode);
                Assert.Equal("Unauthorized access to the item.", statusMsg.State().StatusText);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal(5, statusMsg.StreamId());
                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("LSEG.L", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.CLOSED, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NOT_AUTHORIZED, statusMsg.State().StatusCode);
                Assert.Equal("Unauthorized access to the item.", statusMsg.State().StatusText);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsOffStreamPostingWithServiceNameAndServiceDownTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendRefreshAttrib = true;
            providerTestOptions.SupportOMMPosting = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name for the first provider of the first connection.
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            providerTestOptions2.SendRefreshAttrib = true;
            providerTestOptions2.SupportOMMPosting = true;

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);

            // Provider_1 provides the DIRECT_FEED service name for the first provider of the second connection.
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            // Provider_1 provides the DIRECT_FEED service name for the second provider of the first connection.
            ProviderTestClient providerClient3 = new ProviderTestClient(m_Output, providerTestOptions);
            OmmProvider ommprovider3 = new OmmProvider(config.Port("19002").ProviderName("Provider_1"), providerClient3);

            OmmConsumer? consumer = null;
            ConsumerTestOptions consumerTestOptions = new ConsumerTestOptions();
            consumerTestOptions.SubmitPostOnLoginRefresh = true;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, consumerTestOptions);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                consumerClient.Consumer(consumer);


                long loginHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_LOGIN), consumerClient);

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                /* This is login refresh message message */
                Msg message = consumerClient.PopMessage();

                Thread.Sleep(1000);

                /* Checks to ensure that the first provider of the first connection receives the PostMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is PostMsg);

                PostMsg recvPostMsg = (PostMsg)message;

                Assert.Equal(1, recvPostMsg.StreamId());
                Assert.Equal("DIRECT_FEED", recvPostMsg.ServiceName());
                Assert.Equal("IBM.N", recvPostMsg.Name());
                Assert.Equal(1, recvPostMsg.PostId());
                Assert.True(recvPostMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvPostMsg.Payload().DataType);

                UpdateMsg updateMsg = recvPostMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                /* Checks to ensure that the first provider of the second connection receives the PostMsg */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();

                Assert.True(message is PostMsg);

                recvPostMsg = (PostMsg)message;

                Assert.Equal(1, recvPostMsg.StreamId());
                Assert.Equal("DIRECT_FEED", recvPostMsg.ServiceName());
                Assert.Equal("IBM.N", recvPostMsg.Name());
                Assert.Equal(1, recvPostMsg.PostId());
                Assert.True(recvPostMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvPostMsg.Payload().DataType);

                updateMsg = recvPostMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                /* Shutdown the first provider */
                ommprovider.Uninitialize();

                Thread.Sleep(5000);

                /* Checks to ensure that the second provider doesn't receive the post message as the source directory is not ready yet. */
                Assert.Equal(0, providerClient3.QueueSize());

                /* Checks to ensure that only the first provider of the second connection receives the PostMsg */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();

                Assert.True(message is PostMsg);

                recvPostMsg = (PostMsg)message;

                Assert.Equal(1, recvPostMsg.StreamId());
                Assert.Equal("DIRECT_FEED", recvPostMsg.ServiceName());
                Assert.Equal("IBM.N", recvPostMsg.Name());
                Assert.Equal(2, recvPostMsg.PostId());
                Assert.True(recvPostMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvPostMsg.Payload().DataType);

                updateMsg = recvPostMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                consumer.Unregister(loginHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
                ommprovider3.Uninitialize();
            }
        }

        [Fact]
        public void RequestingSingleItemWithNonExistenceServiceNameAndUnregisterRequestThenAddTheServiceTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.CloseItemRequest = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceName("DIRECT_FEED2").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                /* Ensure that the provider doesn't receive any request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("DIRECT_FEED2", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                /* Cancel the request to remove the item */
                consumer.Unregister(itemHandle);

                /* Provider send source directory update message to add the DIRECT_FEED2 service */
                OmmArray capablities = new OmmArray();
                capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
                capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
                capablities.Complete();

                OmmArray dictionaryUsed = new OmmArray();
                dictionaryUsed.AddAscii("RWFFld");
                dictionaryUsed.AddAscii("RWFEnum");
                dictionaryUsed.Complete();

                ElementList serviceInfoId = new ElementList();

                serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED2");
                serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
                serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
                serviceInfoId.Complete();

                ElementList serviceStateId = new ElementList();
                serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceStateId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
                filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
                filterList.Complete();

                Map map = new Map();
                map.AddKeyUInt(2, MapAction.ADD, filterList);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                                        Payload(map), 0);

                Thread.Sleep(2000);

                /* There is no recover to the provider as the request is canceled */
                Assert.Equal(0, providerClient1.QueueSize());
                Assert.Equal(0, consumerClient.QueueSize());
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsItemRequestsWithServiceListNameButConcreteServicesAreNotAvaliableUnregisterRequestThenConcreateServiceIsAddedTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.CloseItemRequest = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                ServiceList serviceList = new ServiceList("SVG1");

                serviceList.ConcreteServiceList.Add("UNKNOWN_SERVICE");
                serviceList.ConcreteServiceList.Add("DIRECT_FEED2");

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9").AddServiceList(serviceList));

                RequestMsg reqMsg = new();

                long itemHandle = consumer.RegisterClient(reqMsg.ServiceListName("SVG1").Name("LSEG.O"), consumerClient);

                Thread.Sleep(2000);

                /* Ensure that the provider doesn't receive any request message */
                Assert.Equal(0, providerClient1.QueueSize());

                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal("SVG1", statusMsg.ServiceName());
                Assert.Equal("LSEG.O", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("No matching service present.", statusMsg.State().StatusText);

                /* Cancel the request to remove the item */
                consumer.Unregister(itemHandle);

                /* Provider send source directory update message to add the DIRECT_FEED2 service */
                OmmArray capablities = new OmmArray();
                capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
                capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);
                capablities.Complete();

                OmmArray dictionaryUsed = new OmmArray();
                dictionaryUsed.AddAscii("RWFFld");
                dictionaryUsed.AddAscii("RWFEnum");
                dictionaryUsed.Complete();

                ElementList serviceInfoId = new ElementList();

                serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED2");
                serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities);
                serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed);
                serviceInfoId.Complete();

                ElementList serviceStateId = new ElementList();
                serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
                serviceStateId.Complete();

                FilterList filterList = new FilterList();
                filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId);
                filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId);
                filterList.Complete();

                Map map = new Map();
                map.AddKeyUInt(2, MapAction.ADD, filterList);
                map.Complete();

                UpdateMsg updateMsg = new UpdateMsg();
                ommprovider.Submit(updateMsg.DomainType(EmaRdm.MMT_DIRECTORY).
                                                        Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).
                                                        Payload(map), 0);

                Thread.Sleep(2000);

                /* There is no recover to the provider as the request is canceled */
                Assert.Equal(0, providerClient1.QueueSize());
                Assert.Equal(0, consumerClient.QueueSize());
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsReissueOnLoginStreamTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendRefreshAttrib = true;
            providerTestOptions.SupportOptimizedPauseAndResume = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_1"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                LSEG.Ema.Domain.Login.LoginReq loginReq = new();

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9")
                    .AddAdminMsg(loginReq.Message()));

                RequestMsg reqMsg = new();

                consumerClient.Consumer(consumer);


                long loginHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_LOGIN), consumerClient);

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                /* This is login refresh message message */
                Msg message = consumerClient.PopMessage();

                // Submit login reissue to the providers.

                consumer.Reissue(loginReq.Pause(true).Message(), loginHandle);

                Thread.Sleep(1000);

                /* Ensure that the providers receive the reissue message on the login stream;*/
                Assert.Equal(1, providerClient1.QueueSize());

                /* Checks login reissue from the first provider */
                message = providerClient1.PopMessage();
                RequestMsg requestMsg = (RequestMsg)message;
                Assert.Equal(EmaRdm.MMT_LOGIN, requestMsg.DomainType());
                Assert.True(requestMsg.Pause());

                Assert.Equal(1, providerClient2.QueueSize());

                /* Checks login reissue from the second provider */
                message = providerClient2.PopMessage();
                requestMsg = (RequestMsg)message;
                Assert.Equal(EmaRdm.MMT_LOGIN, requestMsg.DomainType());
                Assert.True(requestMsg.Pause());

                consumer.Unregister(loginHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void SingleConnectionsReissueOnLoginStreamTest() /* The request routing is disable for this case */
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendRefreshAttrib = true;
            providerTestOptions.SupportOptimizedPauseAndResume = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                LSEG.Ema.Domain.Login.LoginReq loginReq = new();

                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_1")
                    .AddAdminMsg(loginReq.Message()));

                RequestMsg reqMsg = new();

                consumerClient.Consumer(consumer);


                long loginHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_LOGIN), consumerClient);

                Thread.Sleep(1000);

                Assert.Equal(1, consumerClient.QueueSize());

                /* This is login refresh message message */
                Msg message = consumerClient.PopMessage();

                // Submit login reissue to the providers.
                consumer.Reissue(loginReq.Pause(true).Message(), loginHandle);

                Thread.Sleep(1000);

                /* Ensure that the providers receive the reissue message on the login stream;*/
                Assert.Equal(1, providerClient1.QueueSize());

                /* Checks login reissue from the first provider */
                message = providerClient1.PopMessage();
                RequestMsg requestMsg = (RequestMsg)message;
                Assert.Equal(EmaRdm.MMT_LOGIN, requestMsg.DomainType());
                Assert.True(requestMsg.Pause());

                consumer.Unregister(loginHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsProviderSubmittingGenericMsgWithServiceIdAndUnknownServiceIdViaLoginDomainTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendGenericMessage = true;

            ProviderTestClient providerClient1 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient1);

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_3 provides the DIRECT_FEED_2 service name 
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                long loginHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_LOGIN), consumerClient);

                Thread.Sleep(1000);

                /* This is login refresh message message */
                Assert.Equal(1, consumerClient.QueueSize());

                Msg message = consumerClient.PopMessage();

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal(OmmState.StreamStates.OPEN, refreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, refreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, refreshMsg.State().StatusCode);

                /* Submit a GenericMsg to the first item stream */
                GenericMsg genericMsg = new GenericMsg();
                UpdateMsg nestedUpdateMsg = new UpdateMsg();
                FieldList nestedFieldList = new FieldList();

                nestedFieldList.AddReal(22, 34, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddReal(25, 35, OmmReal.MagnitudeTypes.EXPONENT_POS_1);
                nestedFieldList.AddTime(18, 11, 29, 30);
                nestedFieldList.AddEnumValue(37, 3);
                nestedFieldList.Complete();

                nestedUpdateMsg.Payload(nestedFieldList);

                /* Set the service Id for the GenericMsg from provider */
                providerTestOptions.SubmitGenericMsgWithServiceId = 1;

                /* The service Id is required in order to decode the GenericMsg's payload which includes FieldList properly */
                consumer.Submit(genericMsg.Name("genericMsg").DomainType(200).Complete(true).ServiceId(32767).Payload(nestedUpdateMsg), loginHandle);

                Thread.Sleep(1000);

                /* Checks to ensure that the first provider receives the GenericMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is GenericMsg);

                GenericMsg recvGenericMsg = (GenericMsg)message;

                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.UPDATE_MSG, recvGenericMsg.Payload().DataType);

                UpdateMsg updateMsg = recvGenericMsg.Payload().UpdateMsg();
                Assert.Equal(DataType.DataTypes.FIELD_LIST, updateMsg.Payload().DataType);

                /* Checks to ensure that the second provider receives the GenericMsg */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();

                Assert.True(message is GenericMsg);

                recvGenericMsg = (GenericMsg)message;

                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.ERROR, recvGenericMsg.Payload().DataType);

                /* Ensure that receives two generic message from the two providers.*/
                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.HasServiceId);

                int serviceId = recvGenericMsg.ServiceId();
                int nextServiceId = 0;

                if (serviceId == 32767)
                {
                    nextServiceId = 32768;
                }
                else if (serviceId == 32768)
                {
                    nextServiceId = 32767;
                }
                else
                {
                    Assert.Fail($"Found unexpected service Id {serviceId}");
                }

                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg", recvGenericMsg.Name());
                Assert.Equal(200, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.HasServiceId);
                Assert.Equal(nextServiceId, recvGenericMsg.ServiceId());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);


                /* Don't set the service Id for the GenericMsg from provider */
                providerTestOptions.SubmitGenericMsgWithServiceId = -1;

                /* Submit unknown service Id which there is no translation */
                consumer.Submit(genericMsg.Name("genericMsg2").DomainType(205).ServiceId(555)
                        .Complete(true).Payload(nestedUpdateMsg), loginHandle);

                Thread.Sleep(1000);

                /* Checks to ensure that the first provider receives the GenericMsg */
                Assert.Equal(1, providerClient1.QueueSize());

                message = providerClient1.PopMessage();

                Assert.True(message is GenericMsg);

                recvGenericMsg = (GenericMsg)message;

                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg2", recvGenericMsg.Name());
                Assert.Equal(205, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.ERROR, recvGenericMsg.Payload().DataType);

                /* Checks to ensure that the second provider receives the GenericMsg */
                Assert.Equal(1, providerClient2.QueueSize());

                message = providerClient2.PopMessage();

                Assert.True(message is GenericMsg);

                recvGenericMsg = (GenericMsg)message;

                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg2", recvGenericMsg.Name());
                Assert.Equal(205, recvGenericMsg.DomainType());
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.ERROR, recvGenericMsg.Payload().DataType);

                /* Ensure that receives two generic message from the two providers.*/
                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg2", recvGenericMsg.Name());
                Assert.Equal(205, recvGenericMsg.DomainType());
                Assert.False(recvGenericMsg.HasServiceId);
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                message = consumerClient.PopMessage();

                recvGenericMsg = (GenericMsg)message;
                Assert.Equal(1, recvGenericMsg.StreamId());
                Assert.Equal("genericMsg2", recvGenericMsg.Name());
                Assert.Equal(205, recvGenericMsg.DomainType());
                Assert.False(recvGenericMsg.HasServiceId);
                Assert.True(recvGenericMsg.Complete());
                Assert.Equal(DataType.DataTypes.NO_DATA, recvGenericMsg.Payload().DataType);

                consumer.Unregister(loginHandle);
            }
            catch (OmmException excep)
            {
                m_Output.WriteLine(excep.StackTrace);
                Assert.Fail(excep.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionItemRecoveryWithDirectoryStreamViaRegisterClientConnectionDownAndUpTest()
        {
            OmmIProviderConfig config = new(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);

            // Provider_1 provides the DIRECT_FEED service name
            OmmProvider ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

            // Provider_3 provides the DIRECT_FEED_2 service name
            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004").ProviderName("Provider_3"), providerClient);

            OmmConsumer? consumer = null;
            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output);

            try
            {
                consumer = new OmmConsumer(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                RequestMsg reqMsg = new();

                /* Get the service ID for the DIRECT_FEED service name */
                int serviceId = consumer.m_OmmConsumerImpl!.ConsumerSession!.GetSessionDirectoryByName("DIRECT_FEED")!.Service!.ServiceId;

                long directoryHandle = consumer.RegisterClient(reqMsg.DomainType(EmaRdm.MMT_DIRECTORY).ServiceName("DIRECT_FEED"), consumerClient);

                Thread.Sleep(500);

                long itemHandle = consumer.RegisterClient(reqMsg.Clear().ServiceName("DIRECT_FEED").Name("LSEG.L"), consumerClient);

                Thread.Sleep(1500);

                Assert.Equal(2, consumerClient.QueueSize()); // Ensure that the consumer receives one source directory and one item refresh message.

                Msg message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(2, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / ''", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.False(refreshMsg.HasServiceId);
                Assert.True(refreshMsg.HasServiceName);
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal(DataType.DataTypes.MAP, refreshMsg.Payload().DataType);
                Map payload = refreshMsg.Payload().Map();
                
                var mapIt = payload.GetEnumerator();
                Assert.True(mapIt.MoveNext());
                MapEntry mapEntry =mapIt.Current;
                Assert.Equal((ulong)serviceId, mapEntry.Key.UInt());
                Assert.Equal(MapAction.ADD, mapEntry.Action);

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                /* Checks item refresh message */
                Assert.Equal(5, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_MARKET_PRICE, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.True(refreshMsg.HasServiceId);
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.True(refreshMsg.HasServiceName);
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal(DataType.DataTypes.FIELD_LIST, refreshMsg.Payload().DataType);

                /* Bring down the connection for the DIRECT_FEED service name */
                ommprovider.Uninitialize();

                int count = 0;
                while (count < 5 && consumerClient.QueueSize() != 2)
                {
                    Thread.Sleep(500);
                    ++count;
                }

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();
                Assert.True(message is UpdateMsg);

                UpdateMsg updateMsg = (UpdateMsg)message;
                Assert.Equal(2, updateMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, updateMsg.DomainType());
                Assert.Equal(DataType.DataTypes.MAP, updateMsg.Payload().DataType);

                payload = updateMsg.Payload().Map();

                mapIt = payload.GetEnumerator();
                Assert.True(mapIt.MoveNext());
                mapEntry = mapIt.Current;

                Assert.Equal((ulong)serviceId, mapEntry.Key.UInt());
                Assert.Equal(MapAction.DELETE, mapEntry.Action);
                Assert.Equal(DataType.DataTypes.NO_DATA, mapEntry.LoadType);

                message = consumerClient.PopMessage();
                Assert.True(message is StatusMsg);

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.True(statusMsg.HasServiceName);
                Assert.Equal("DIRECT_FEED", statusMsg.ServiceName());
                Assert.True(statusMsg.HasServiceId);
                Assert.Equal(serviceId, statusMsg.ServiceId());
                Assert.Equal("LSEG.L", statusMsg.Name());
                Assert.Equal(OmmState.StreamStates.OPEN, statusMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.SUSPECT, statusMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, statusMsg.State().StatusCode);
                Assert.Equal("channel down.", statusMsg.State().StatusText);

                /* Bring the provider up again */
                ommprovider = new OmmProvider(config.Port("19001").ProviderName("Provider_1"), providerClient);

                /* Waits for channel up and the consumer receives the source directory update */
                count = 0;
                while (count < 10 && consumerClient.QueueSize() != 2)
                {
                    Thread.Sleep(1000);
                    ++count;
                }

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                Assert.True(message is UpdateMsg);

                updateMsg = (UpdateMsg)message;
                Assert.Equal(2, updateMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_DIRECTORY, updateMsg.DomainType());
                Assert.Equal(DataType.DataTypes.MAP, updateMsg.Payload().DataType);
                Assert.True(updateMsg.HasMsgKey);
                Assert.False(updateMsg.HasServiceId);
                Assert.True(updateMsg.HasServiceName);
                Assert.Equal("DIRECT_FEED", updateMsg.ServiceName());
                Assert.Equal(DataType.DataTypes.MAP, updateMsg.Payload().DataType);
                payload = updateMsg.Payload().Map();

                mapIt = payload.GetEnumerator();
                Assert.True(mapIt.MoveNext());
                mapEntry = mapIt.Current;
                Assert.Equal((ulong)serviceId, mapEntry.Key.UInt());
                Assert.Equal(MapAction.ADD, mapEntry.Action);
                Assert.Equal(DataType.DataTypes.FILTER_LIST, mapEntry.LoadType);

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                /* Checks item refresh message */
                Assert.Equal(5, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_MARKET_PRICE, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Refresh Completed'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.True(refreshMsg.HasServiceId);
                Assert.Equal(serviceId, refreshMsg.ServiceId());
                Assert.True(refreshMsg.HasServiceName);
                Assert.Equal("DIRECT_FEED", refreshMsg.ServiceName());
                Assert.Equal(DataType.DataTypes.FIELD_LIST, refreshMsg.Payload().DataType);

                consumer.Unregister(directoryHandle);
                consumer.Unregister(itemHandle);
            }
            catch (OmmException ex)
            {
                Assert.Fail(ex.Message);
            }
            finally
            {
                Assert.NotNull(consumer);

                m_Output.WriteLine("Uninitializing...");

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsForLoginStreamWithReloginToServersTest()
        {
            OmmIProviderConfig config = new(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();
            providerTestOptions.SendRefreshAttrib = true;

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);

            OmmProvider ommprovider = new (config.Port("19001"), providerClient);

            ProviderTestOptions providerTestOptions2 = new ProviderTestOptions();
            providerTestOptions2.SendRefreshAttrib = true;
            providerTestOptions2.SupportOMMPosting = true;

            ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions2);

            OmmProvider ommprovider2 = new (config.Port("19004"), providerClient2);

            OmmConsumer? consumer = null;
            ConsumerTestOptions options = new ConsumerTestOptions();

            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, options);

            try
            {
                consumer = new (new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"), consumerClient);

                Assert.Equal(3, consumerClient.QueueSize()); // Ensure that the callback receives only one login message

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataType.DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);

                ElementList elementList = refreshMsg.Attrib().ElementList();

                bool foundOmmPosting = false;

                foreach (ElementEntry element in elementList)
                {
                    switch (element.Name)
                    {
                        case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_POST:
                            {
                                foundOmmPosting = true;
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                    }
                }

                Assert.False(foundOmmPosting);

                // Bring down the Channel_1 of Connection_1
                ommprovider.Uninitialize();

                ProviderTestOptions providerTestOptions3 = new ProviderTestOptions();
                providerTestOptions3.SendRefreshAttrib = true;
                providerTestOptions3.SupportOMMPosting = true;

                ProviderTestClient providerClient3 = new ProviderTestClient(m_Output, providerTestOptions3);

                // Bring up the Channel_2 of Connection_1
                ommprovider = new OmmProvider(config.Port("19002"), providerClient3);

                /* Waits until the Channel_1 of Connection_1 is closed */
                Thread.Sleep(3000);

                Assert.Equal(3, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'session channel down reconnecting'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataType.DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);

                elementList = refreshMsg.Attrib().ElementList();

                foundOmmPosting = false;

                foreach (ElementEntry element in elementList)
                {
                    switch (element.Name)
                    {
                        case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_POST:
                            {
                                foundOmmPosting = true;
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                    }
                }

                /* Ensure that OmmPosting is found */
                Assert.True(foundOmmPosting);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'session channel up'", statusMsg.State().ToString());
            }
            catch (OmmException)
		    {
                Assert.False(true);
            }
            finally
            {
                Assert.NotNull(consumer);

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();

            }
        }

        [Fact]
        public void MultiConnectionsForLoginStreamForConnectionRecoveryTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);

            OmmProvider ommprovider = new OmmProvider(config.Port("19001"), providerClient);

            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004"), providerClient);

            OmmConsumer? consumer = null;
            ConsumerTestOptions options = new ConsumerTestOptions();

            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, options);

            try
            {
                consumer = new (new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"), consumerClient);

                Assert.Equal(3, consumerClient.QueueSize()); // Ensure that the callback receives only one login message

                Msg message = consumerClient.PopMessage();

                StatusMsg statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel up'", statusMsg.State().ToString());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                RefreshMsg refreshMsg = (RefreshMsg)message;

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataType.DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);

                ElementList elementList = refreshMsg.Attrib().ElementList();

                bool foundOmmPosting = false;

                foreach (ElementEntry element in elementList)
                {
                    switch (element.Name)
                    {
                        case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_POST:
                            {
                                foundOmmPosting = true;
                                Assert.Equal((ulong)0, element.UIntValue());
                                break;
                            }
                    }
                }

                Assert.False(foundOmmPosting);

                // Bring down the Channel_1 of Connection_1
                ommprovider.Uninitialize();

                /* Waits until the Connect_1 is closed */
                Thread.Sleep(11000);

                Assert.Equal(3, consumerClient.QueueSize());

                for (int i = 0; i < 2; i++)
                {
                    message = consumerClient.PopMessage();

                    statusMsg = (StatusMsg)message;

                    Assert.Equal(1, statusMsg.StreamId());
                    Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                    Assert.Equal("Open / Ok / None / 'session channel down reconnecting'", statusMsg.State().ToString());
                }

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'session channel closed'", statusMsg.State().ToString());

                // Bring down the Connection_2
                ommprovider2.Uninitialize();

                Thread.Sleep(3000);

                Assert.Equal(1, consumerClient.QueueSize());
                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Suspect / None / 'session channel down reconnecting'", statusMsg.State().ToString());

                /* Change the login response to support OMM posting */
                providerTestOptions.SendRefreshAttrib = true;
                providerTestOptions.SupportOMMPosting = true;
                ProviderTestClient providerClient2 = new ProviderTestClient(m_Output, providerTestOptions);

                // Bring up Channel_5 of Connection_2
                ommprovider2 = new OmmProvider(config.Port("19005"), providerClient2);

                Thread.Sleep(3000);

                Assert.Equal(2, consumerClient.QueueSize());

                message = consumerClient.PopMessage();

                Assert.True(message is RefreshMsg);

                refreshMsg = (RefreshMsg)message;

                Assert.Equal(1, refreshMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, refreshMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'Login accepted'", refreshMsg.State().ToString());
                Assert.True(refreshMsg.Solicited());
                Assert.True(refreshMsg.Complete());
                Assert.True(refreshMsg.HasMsgKey);
                Assert.Equal(DataType.DataTypes.NO_DATA, refreshMsg.Payload().DataType);
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, refreshMsg.Attrib().DataType);

                elementList = refreshMsg.Attrib().ElementList();

                foundOmmPosting = false;

                foreach (ElementEntry element in elementList)
                {
                    switch (element.Name)
                    {
                        case EmaRdm.ENAME_SINGLE_OPEN:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_ALLOW_SUSPECT_DATA:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_BATCH:
                            {
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                        case EmaRdm.ENAME_SUPPORT_POST:
                            {
                                foundOmmPosting = true;
                                Assert.Equal((ulong)1, element.UIntValue());
                                break;
                            }
                    }
                }

                /* Ensure that OmmPosting is found */
                Assert.True(foundOmmPosting);

                message = consumerClient.PopMessage();

                statusMsg = (StatusMsg)message;

                Assert.Equal(1, statusMsg.StreamId());
                Assert.Equal(EmaRdm.MMT_LOGIN, statusMsg.DomainType());
                Assert.Equal("Open / Ok / None / 'session channel up'", statusMsg.State().ToString());

            }
            catch (OmmException)
		    {
                Assert.False(true);
            }
            finally
            {
                Assert.NotNull(consumer);

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }

        [Fact]
        public void MultiConnectionsModifyIOCtlForGuaranteedOutputBuffersTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig(EmaConfigFileLocation);

            ProviderTestOptions providerTestOptions = new ProviderTestOptions();

            ProviderTestClient providerClient = new ProviderTestClient(m_Output, providerTestOptions);

            OmmProvider ommprovider = new OmmProvider(config.Port("19001"), providerClient);

            OmmProvider ommprovider2 = new OmmProvider(config.Port("19004"), providerClient);

            OmmConsumer? consumer = null;
            ConsumerTestOptions options = new ConsumerTestOptions();

            ConsumerTestClient consumerClient = new ConsumerTestClient(m_Output, options);

            try
            {
                consumer = new(new OmmConsumerConfig(EmaConfigFileLocation).ConsumerName("Consumer_9"));

                List<ChannelInformation> channelInformationList = new List<ChannelInformation>();

                consumer.SessionChannelInfo(channelInformationList);

                Assert.Equal(2, channelInformationList.Count);

                // Checks with the configured value from EMA configuration file.
                Assert.Equal(5000, channelInformationList[0].GuaranteedOutputBuffers);
                Assert.Equal(5000, channelInformationList[1].GuaranteedOutputBuffers);

                int expectedGuranteedOutputBuffers = 7000;

                consumer.ModifyIOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, expectedGuranteedOutputBuffers);

                consumer.SessionChannelInfo(channelInformationList);

                Assert.Equal(2, channelInformationList.Count);

                Assert.Equal(expectedGuranteedOutputBuffers, channelInformationList[0].GuaranteedOutputBuffers);
                Assert.Equal(expectedGuranteedOutputBuffers, channelInformationList[1].GuaranteedOutputBuffers);
            }
            catch (OmmException)
            {
                Assert.False(true);
            }
            finally
            {
                Assert.NotNull(consumer);

                consumer.Uninitialize();
                ommprovider.Uninitialize();
                ommprovider2.Uninitialize();
            }
        }
    }
}
