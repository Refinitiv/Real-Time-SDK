/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access.Tests.OmmConsumerTests;
using LSEG.Ema.Access.Tests.OmmIProviderTests;
using LSEG.Ema.Access.Tests.OmmNiProviderTests;
using LSEG.Ema.Rdm;
using LSEG.Eta.Codec;
using LSEG.Eta.ValueAdd.Reactor;
using System;
using System.Threading;
using Xunit.Abstractions;
using DataDictionary = LSEG.Eta.Codec.DataDictionary;


namespace LSEG.Ema.Access.Tests
{
    public class PackedMsgTest : IDisposable
    {
        public void Dispose()
        {
            EtaGlobalPoolTestUtil.Clear();
        }

        static DataDictionary DataDictionary = new DataDictionary();

        private static void LoadDictionary()
        {
            if (DataDictionary.LoadEnumTypeDictionary("../../../ComplexTypeTests/enumtype.def", out _) < 0)
            {
                Assert.Fail( "Unable to load enum dictionary.");
            }

            if (DataDictionary.LoadFieldDictionary("../../../ComplexTypeTests/RDMFieldDictionary", out _) < 0)
            {
                Assert.Fail( "Unable to load enum dictionary.");
            }
        }

        static PackedMsgTest()
        {
            LoadDictionary();
        }

        ITestOutputHelper m_Output;

        public PackedMsgTest(ITestOutputHelper output)
        {
            m_Output = output;
        }

        [Fact]
        public void PackedMsgConstructTest()
        {
            OmmIProviderConfig config = new();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "18001";

            try
            {
                provider = new OmmProvider(config.Port(port), providerClient);

                PackedMsg packedMsg = new PackedMsg(provider);

                // Checking public interfaces
                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(0, packedMsg.RemainingSize());

                // Checking internal interfaces
                Assert.Null(packedMsg.ReactorChannel);
                Assert.Null(packedMsg.PackedBuffer);
            }
            catch (OmmException exp)
            {
                exception = exp;
            }
            finally
            {
                Assert.Null(exception);
                provider?.Uninitialize();
            }
        }

        [Fact]
        public void PackedMsgInitBufferWithInvalidProviderRoleForIProviderTest()
        {
            OmmIProviderConfig config = new();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "18002";

            try
            {
                provider = new OmmProvider(config.Port(port), providerClient);

                PackedMsg packedMsg = new(provider);

                var ex = Assert.Throws<OmmInvalidUsageException>(() => packedMsg.InitBuffer());

                Assert.Equal("This method is used for Non-Interactive provider only. Setting a client handle is required when using Interactive Provider.", ex.Message);
            }
            catch (OmmException exp)
            {
                exception = exp;
            }
            finally
            {
                /* Checks for unexpected exception */
                Assert.Null(exception);

                provider?.Uninitialize();
            }
        }

        [Fact]
        public void PackedMsgInitBufferWithInvalidProviderRoleForNiProviderTest()
        {
            ProviderSessionOptions providerSessionOpts = new();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new OmmProvider(config.Host(hostString));

                PackedMsg packedMsg = new(provider);

                long clientHandle = 55;

                var ex = Assert.Throws<OmmInvalidUsageException>(() => packedMsg.InitBuffer(clientHandle));

                Assert.Equal("This method is used for Interactive provider only. Setting a client handle is not required when using Non-Interactive Provider.", ex.Message);
            }
            catch (OmmException exp)
            {
                exception = exp;
            }
            finally
            {
                /* Checks for unexpected exception */
                Assert.Null(exception);

                provider?.Uninitialize();
                adhSimulator.UnInitialize();
            }
        }

        [Fact]
        public void SubmitPackedMsgWithoutInitBufferForIProviderTest()
        {
            OmmIProviderConfig config = new();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "18003";

            try
            {
                provider = new OmmProvider(config.Port(port), providerClient);

                PackedMsg packedMsg = new(provider);

                var ex = Assert.Throws<OmmInvalidUsageException>(() => provider.Submit(packedMsg));

                Assert.Equal("Attempt to submit PackedMsg with no channel.", ex.Message);
            }
            catch (OmmException exp)
            {
                exception = exp;
            }
            finally
            {
                /* Checks for unexpected exception */
                Assert.Null(exception);

                provider?.Uninitialize();
            }
        }

        [Fact]
        public void SubmitPackedMsgWithoutInitBufferForNiProviderTest()
        {
            ProviderSessionOptions providerSessionOpts = new();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new OmmProvider(config.Host(hostString));

                PackedMsg packedMsg = new(provider);

                var ex = Assert.Throws<OmmInvalidUsageException>(() => provider.Submit(packedMsg));

                Assert.Equal("Attempt to submit PackedMsg with non init transport buffer.", ex.Message);
            }
            catch (OmmException exp)
            {
                exception = exp;
            }
            finally
            {
                /* Checks for unexpected exception */
                Assert.Null(exception);

                provider?.Uninitialize();
                adhSimulator.UnInitialize();
            }
        }

        [Fact]
        public void PackedMsgInitBufferForIProviderWithInvalidClientHandleTest()
        {
            OmmIProviderConfig config = new();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "18004";

            try
            {
                provider = new OmmProvider(config.Port(port), providerClient);

                PackedMsg packedMsg = new(provider);

                long invalidClientHandle = 555;

                var ex = Assert.Throws<OmmInvalidUsageException>(() => packedMsg.InitBuffer(invalidClientHandle));

                Assert.Equal("Client handle is not valid.", ex.Message);
            }
            catch (OmmException exp)
            {
                exception = exp;
            }
            finally
            {
                /* Checks for unexpected exception */
                Assert.Null(exception);

                provider?.Uninitialize();
            }
        }

        [Fact]
        public void PackedMsgAddMsgForIProviderWithInvalidItemHandleTest()
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "18005";

            SimpleConsumerTest simpleConsumer = new(m_Output, port);
            ReactorOptions reactorOption = new();
            long clientHandle = 0;

            try
            {
                provider = new OmmProvider(config.Port(port).OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH), providerClient);

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case EmaRdm.MMT_LOGIN:
                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType())
                                .Solicited(true), providerEvent.Handle);

                            clientHandle = providerEvent.ClientHandle;

                            break;
                    }
                };

                OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl;

                simpleConsumer.Initialize(reactorOption, null, true);

                for (int i = 0; i < 5; i++)
                {
                    provider.Dispatch(1000000);
                }

                /* Ensure SimpleConsumer receive CHANNEL_UP, CHANNEL_READY, LOGIN events */
                Assert.Equal(3, simpleConsumer.EventQueue.Count);

                Assert.Single(serverBaseImpl.ConnectedChannelList);

                PackedMsg packedMsg = new(provider);

                packedMsg.InitBuffer(clientHandle);

                long invalidItemHandle = 555;

                var ex = Assert.Throws<OmmInvalidUsageException>(() => packedMsg.AddMsg(new RefreshMsg(), invalidItemHandle));

                Assert.Equal("Incorrect item handle for incoming message.", ex.Message);
            }
            catch (OmmException exp)
            {
                exception = exp;
            }
            finally
            { /* Checks for unexpected exception */
                Assert.Null(exception);

                provider?.Uninitialize();
            }
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void PackedMsgInitBufferForNiProviderTest(bool defaultSize)
        {
            ProviderSessionOptions providerSessionOpts = new();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new();
            OmmException? exception = null;
            OmmProvider? provider = null;

            int bufferSize = defaultSize ? 6000 : 6140;

            try
            {
                provider = new(config.Host(hostString));
                m_Output.WriteLine("Created OmmProvider successfully");

                PackedMsg packedMsg = new(provider);

                if (defaultSize)
                {
                    packedMsg.InitBuffer();
                }
                else
                {
                    packedMsg.InitBuffer(bufferSize);
                }

                // Checking public interfaces
                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(bufferSize, packedMsg.RemainingSize());

                // Checking internal interfaces
                Assert.NotNull(packedMsg.ReactorChannel);
                Assert.NotNull(packedMsg.PackedBuffer);

                packedMsg.Clear();

                // Checking public interfaces
                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(0, packedMsg.RemainingSize());

                // Checking internal interfaces
                Assert.Null(packedMsg.ReactorChannel);
                Assert.Null(packedMsg.PackedBuffer);

            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }
            finally
            {
                /* Checks for unexpected exception */
                Assert.Null(exception);

                provider?.Uninitialize();
                adhSimulator.UnInitialize();
            }
        }

        [Theory]
        [InlineData(6145)]
        [InlineData(8000)]
        public void PackedMsgInitBufferInvalidBufferSizeForNiProviderTest(int bufferSize)
        {
            ProviderSessionOptions providerSessionOpts = new();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new(config.Host(hostString));
                m_Output.WriteLine("Created OmmProvider successfully");

                PackedMsg packedMsg = new(provider);

                var ex = Assert.Throws<OmmInvalidUsageException>(() => packedMsg.InitBuffer(bufferSize));

                Assert.Equal($"Failed to get packed buffer in InitBuffer().{NewLine}Channel Rssl Channel{NewLine}\tscktChannel:" +
                    $" 127.0.0.1:{adhSimulator.ServerPort}{NewLine}\tconnected: True{NewLine}\tstate: ACTIVE{NewLine}\tconnectionType: SOCKET{NewLine}\tclientIP: {NewLine}\tclientHostname:" +
                    $" {NewLine}\tpingTimeout: 60{NewLine}\tmajorVersion: 14{NewLine}\tminorVersion: 1{NewLine}\tprotocolType: RWF{NewLine}\tuserSpecObject:" +
                    $" LSEG.Ema.Access.ChannelInfo{NewLine}{NewLine}Error Id: SUCCESS{NewLine}Internal SysError: 0{NewLine}Error Text:" +
                    $" Packing buffer must fit in maxFragmentSize{NewLine}", ex.Message);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }
            finally
            {
                /* Checks for unexpected exception */
                Assert.Null(exception);

                provider?.Uninitialize();
                adhSimulator.UnInitialize();
            }
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void PackedMsgIntoBufferAndSubmitWithInvalidServiceForNiProviderTest(bool setServiceName)
        {
            ProviderSessionOptions providerSessionOpts = new();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new(config.Host(hostString));
                m_Output.WriteLine("Created OmmProvider successfully");

                PackedMsg packedMsg = new(provider);

                packedMsg.InitBuffer();

                // Checking public interfaces
                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(6000, packedMsg.RemainingSize());

                RefreshMsg refreshMsg = new();
                FieldList fieldList = new();

                string invalid_service_name = "INVALID_NI_SERVICE";
                int invalid_service_id = 777;
                long itemHandle = 5;

                if (setServiceName)
                {
                    refreshMsg.ServiceName(invalid_service_name).Name("IBM.N").MarkForClear().Complete(true).ClearCache(true).State(OmmState.StreamStates.OPEN,
                        OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Unsolicited Refresh Completed");

                    var ex = Assert.Throws<OmmInvalidUsageException>(() => packedMsg.AddMsg(refreshMsg, itemHandle));

                    Assert.Equal($"Attempt to add RefreshMsg with service name of {invalid_service_name} that was not included in the SourceDirectory. Dropping this RefreshMsg.", ex.Message);
                }
                else
                {
                    refreshMsg.ServiceId(invalid_service_id).Name("IBM.N").MarkForClear().Complete(true).ClearCache(true).State(OmmState.StreamStates.OPEN,
                        OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Unsolicited Refresh Completed");

                    var ex = Assert.Throws<OmmInvalidUsageException>(() => packedMsg.AddMsg(refreshMsg, itemHandle));

                    Assert.Equal($"Attempt to add RefreshMsg with service Id of {invalid_service_id} that was not included in the SourceDirectory. Dropping this RefreshMsg.", ex.Message);
                }
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }
            finally
            {
                /* Checks for unexpected exception */
                Assert.Null(exception);

                provider?.Uninitialize();
                adhSimulator.UnInitialize();
            }
        }

        [Theory]
        [InlineData(1)]
        [InlineData(10)]
        public void PackedMsgInitBufferAndAddMsgWithSmallBufferForNiProviderTest(int bufferSize)
        {
            ProviderSessionOptions providerSessionOpts = new();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new();
            OmmException? exception = null;
            OmmProvider? provider = null;

            try
            {
                provider = new(config.Host(hostString));
                m_Output.WriteLine("Created OmmProvider successfully");

                PackedMsg packedMsg = new(provider);

                packedMsg.InitBuffer(bufferSize);

                // Checking public interfaces
                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(bufferSize, packedMsg.RemainingSize());

                RefreshMsg refreshMsg = new();
                FieldList fieldList = new();
                fieldList.AddReal(22, 3390, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(30, 50, OmmReal.MagnitudeTypes.EXPONENT_0);

                refreshMsg.ServiceId(0).Name("IBM.N").MarkForClear().Complete(true).ClearCache(true).State(OmmState.StreamStates.OPEN,
                         OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Unsolicited Refresh Completed").MarkForClear().Payload(fieldList.MarkForClear().Complete());

                long itemHandle = 5;

                var ex = Assert.Throws<OmmInvalidUsageException>(() => packedMsg.AddMsg(refreshMsg, itemHandle));

                Assert.Equal("Failed IMsg.Encode() with error: Failure: The buffer provided does not have sufficient space to perform the operation.", ex.Message);
                Assert.Equal(OmmInvalidUsageException.ErrorCodes.BUFFER_TOO_SMALL, ex.ErrorCode);

                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(bufferSize, packedMsg.RemainingSize());
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }
            finally
            {
                /* Checks for unexpected exception */
                Assert.Null(exception);

                provider?.Uninitialize();
                adhSimulator.UnInitialize();
            }
        }

        [Theory]
        [InlineData(false, false)]
        [InlineData(true, true)]
        public void PackedMsgIntoBufferAndSubmitForNiProviderTest(bool defaultSize, bool setServiceName)
        {
            ProviderSessionOptions providerSessionOpts = new();
            providerSessionOpts.SendLoginReject = false;
            ADHSimulator adhSimulator = new(providerSessionOpts, m_Output);
            ReactorOptions reactorOptions = new();
            adhSimulator.Initialize(reactorOptions);

            string hostString = $"localhost:{adhSimulator.ServerPort}";

            m_Output.WriteLine($"Connect with {hostString}");

            OmmNiProviderConfig config = new();
            OmmException? exception = null;
            OmmProvider? provider = null;

            int bufferSize = defaultSize ? 6000 : 3000;

            try
            {
                provider = new(config.Host(hostString));
                m_Output.WriteLine("Created OmmProvider successfully");

                PackedMsg packedMsg = new(provider);

                if (defaultSize)
                {
                    packedMsg.InitBuffer();
                }
                else
                {
                    packedMsg.InitBuffer(bufferSize);
                }

                // Checking public interfaces
                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(bufferSize, packedMsg.RemainingSize());

                RefreshMsg refreshMsg = new();
                FieldList fieldList = new();
                fieldList.AddReal(22, 3390, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(30, 50, OmmReal.MagnitudeTypes.EXPONENT_0);

                if (setServiceName)
                {
                    refreshMsg.ServiceName("NI_PUB").Name("IBM.N").MarkForClear().Complete(true).ClearCache(true).State(OmmState.StreamStates.OPEN,
                        OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Unsolicited Refresh Completed").MarkForClear().Payload(fieldList.MarkForClear().Complete());
                }
                else
                {
                    refreshMsg.ServiceId(0).Name("IBM.N").MarkForClear().Complete(true).ClearCache(true).State(OmmState.StreamStates.OPEN,
                        OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Unsolicited Refresh Completed").MarkForClear().Payload(fieldList.MarkForClear().Complete());
                }

                long itemHandle = 5;

                packedMsg.AddMsg(refreshMsg, itemHandle);

                Assert.Equal(1, packedMsg.PackedMsgCount());

                int expectedRemaining = defaultSize ? 5922 : 2922;

                Assert.Equal(expectedRemaining, packedMsg.RemainingSize());

                UpdateMsg updateMsg = new();
                fieldList.Clear();
                fieldList.AddReal(22, 3490, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                   .AddReal(25, 3998, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                   .AddReal(30, 60, OmmReal.MagnitudeTypes.EXPONENT_0);

                if (setServiceName)
                {
                    updateMsg.ServiceName("NI_PUB").Name("IBM.N").MarkForClear().Payload(fieldList.MarkForClear().Complete());
                }
                else
                {
                    updateMsg.ServiceId(0).Name("IBM.N").MarkForClear().Payload(fieldList.MarkForClear().Complete());
                }

                packedMsg.AddMsg(updateMsg, itemHandle);

                Assert.Equal(2, packedMsg.PackedMsgCount());

                expectedRemaining = defaultSize ? 5879 : 2879;
                Assert.Equal(expectedRemaining, packedMsg.RemainingSize());

                adhSimulator.EventQueue.Clear(); /* Clears all previous events in the event queue */

                provider.Submit(packedMsg);

                /* Make sure that the ADHSimulator receives the messages. */
                Thread.Sleep(1000);

                Assert.Equal(2, adhSimulator.EventQueue.Count);

                TestReactorEvent testReactorEvent = adhSimulator.EventQueue.Dequeue();

                Assert.Equal(TestReactorEventType.MSG, testReactorEvent.EventType);
                ReactorMsgEvent msgEvent = (ReactorMsgEvent)testReactorEvent.ReactorEvent;

                /* Check with the first message */
                RefreshMsg decodeRefreshMsg = new();

                decodeRefreshMsg.Decode(msgEvent.Msg!, Codec.MajorVersion(), Codec.MinorVersion(),
                    adhSimulator.DictionaryHandler.DataDictionary);

                int expectedStreamId = -1;

                Assert.Equal(expectedStreamId, decodeRefreshMsg.StreamId());
                Assert.True(decodeRefreshMsg.HasName);
                Assert.Equal("IBM.N", decodeRefreshMsg.Name());
                Assert.True(decodeRefreshMsg.HasServiceId);
                Assert.Equal(0, decodeRefreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, decodeRefreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, decodeRefreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, decodeRefreshMsg.State().StatusCode);
                Assert.True(decodeRefreshMsg.MarkForClear().Complete());
                Assert.True(decodeRefreshMsg.ClearCache());
                Assert.False(decodeRefreshMsg.Solicited());
                Assert.Equal(DataType.DataTypes.FIELD_LIST, decodeRefreshMsg.MarkForClear().Payload().DataType);

                var decodeFieldList = decodeRefreshMsg.MarkForClear().Payload().FieldList();

                var iterator = decodeFieldList.GetEnumerator();
                Assert.True(iterator.MoveNext());
                FieldEntry entry = iterator.Current;
                Assert.Equal(22, entry.FieldId);
                Assert.Equal(33.90, entry.OmmRealValue().AsDouble());

                Assert.True(iterator.MoveNext());
                entry = iterator.Current;
                Assert.Equal(25, entry.FieldId);
                Assert.Equal(39.94, entry.OmmRealValue().AsDouble());

                Assert.True(iterator.MoveNext());
                entry = iterator.Current;
                Assert.Equal(30, entry.FieldId);
                Assert.Equal(50, entry.OmmRealValue().AsDouble());

                Assert.False(iterator.MoveNext());

                /* Check with the second message */
                testReactorEvent = adhSimulator.EventQueue.Dequeue();

                Assert.Equal(TestReactorEventType.MSG, testReactorEvent.EventType);
                msgEvent = (ReactorMsgEvent)testReactorEvent.ReactorEvent;

                UpdateMsg decodeUpdateMsg = new();

                decodeUpdateMsg.Decode(msgEvent.Msg!, Codec.MajorVersion(), Codec.MinorVersion(),
                    adhSimulator.DictionaryHandler.DataDictionary);

                Assert.Equal(expectedStreamId, decodeUpdateMsg.StreamId());
                Assert.True(decodeUpdateMsg.HasName);
                Assert.Equal("IBM.N", decodeUpdateMsg.Name());
                Assert.True(decodeUpdateMsg.HasServiceId);
                Assert.Equal(0, decodeUpdateMsg.ServiceId());
                Assert.Equal(DataType.DataTypes.FIELD_LIST, decodeUpdateMsg.MarkForClear().Payload().DataType);

                decodeFieldList = decodeUpdateMsg.MarkForClear().Payload().FieldList();

                iterator = decodeFieldList.GetEnumerator();
                Assert.True(iterator.MoveNext());
                entry = iterator.Current;
                Assert.Equal(22, entry.FieldId);
                Assert.Equal(34.90, entry.OmmRealValue().AsDouble());

                Assert.True(iterator.MoveNext());
                entry = iterator.Current;
                Assert.Equal(25, entry.FieldId);
                Assert.Equal(39.98, entry.OmmRealValue().AsDouble());

                Assert.True(iterator.MoveNext());
                entry = iterator.Current;
                Assert.Equal(30, entry.FieldId);
                Assert.Equal(60, entry.OmmRealValue().AsDouble());

                Assert.False(iterator.MoveNext());

            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }
            finally
            {
                /* Checks for unexpected exception */
                Assert.Null(exception);

                provider?.Uninitialize();
                adhSimulator.UnInitialize();
            }
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void PackedMsgInitBufferForIProviderTest(bool defaultSize)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "18006";

            SimpleConsumerTest simpleConsumer = new(m_Output, port);
            ReactorOptions reactorOption = new();
            long clientHandle = 0;

            try
            {
                provider = new OmmProvider(config.Port(port).OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH), providerClient);

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case EmaRdm.MMT_LOGIN:
                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType())
                                .Solicited(true), providerEvent.Handle);

                            clientHandle = providerEvent.ClientHandle;

                            break;
                    }
                };

                OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl;

                simpleConsumer.Initialize(reactorOption, null, true);

                for (int i = 0; i < 5; i++)
                {
                    provider.Dispatch(1000000);
                }

                /* Ensure SimpleConsumer receive CHANNEL_UP, CHANNEL_READY, LOGIN events */
                Assert.Equal(3, simpleConsumer.EventQueue.Count);

                Assert.Single(serverBaseImpl.ConnectedChannelList);

                PackedMsg packedMsg = new(provider);

                int bufferSize = defaultSize ? 6000 : 6140;

                if (defaultSize)
                {
                    packedMsg.InitBuffer(clientHandle);
                }
                else
                {
                    packedMsg.InitBuffer(clientHandle, bufferSize);
                }

                // Checking public interfaces
                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(bufferSize, packedMsg.RemainingSize());

                // Checking internal interfaces
                Assert.NotNull(packedMsg.ReactorChannel);
                Assert.NotNull(packedMsg.PackedBuffer);

                packedMsg.Clear();

                // Checking public interfaces
                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(0, packedMsg.RemainingSize());

                // Checking internal interfaces
                Assert.Null(packedMsg.ReactorChannel);
                Assert.Null(packedMsg.PackedBuffer);

                simpleConsumer.UnInitialize();

                for (int i = 0; i < 10; i++)
                {
                    provider.Dispatch(1000000);
                }

                Assert.Empty(serverBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }
            finally
            {
                Assert.Null(exception);
                Assert.NotNull(provider);
                provider?.Uninitialize();
            }
        }

        [Theory]
        [InlineData(false)]
        [InlineData(true)]
        public void PackedMsgInitBufferAndSubmitWithInvalidServiceForIProviderTest(bool setServiceName)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "18007";

            SimpleConsumerTest simpleConsumer = new(m_Output, port);
            ReactorOptions reactorOption = new();
            long clientHandle = 0;
            long itemHandle = 0;

            try
            {
                provider = new OmmProvider(config.Port(port).OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH)
                    .AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient);

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case EmaRdm.MMT_LOGIN:
                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType())
                                .Solicited(true), providerEvent.Handle);

                            itemHandle = providerEvent.Handle;
                            clientHandle = providerEvent.ClientHandle;

                            break;
                        case EmaRdm.MMT_DIRECTORY:
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

                OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl;

                simpleConsumer.Initialize(reactorOption, null, true, true);

                for (int i = 0; i < 5; i++)
                {
                    provider.Dispatch(1000000);
                }

                /* Ensure SimpleConsumer receive CHANNEL_UP, CHANNEL_READY, LOGIN and DIRECTORY event */
                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                Assert.Single(serverBaseImpl.ConnectedChannelList);

                PackedMsg packedMsg = new(provider);

                packedMsg.InitBuffer(clientHandle);

                // Checking public interfaces
                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(6000, packedMsg.RemainingSize());

                RefreshMsg refreshMsg = new();

                string invalid_service_name = "INVALID_SERVICE";
                int invalid_service_id = 555;

                if (setServiceName)
                {
                    refreshMsg.ServiceName(invalid_service_name).Name("IBM.N").MarkForClear().Complete(true).ClearCache(true).State(OmmState.StreamStates.OPEN,
                        OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Unsolicited Refresh Completed");

                    var ex = Assert.Throws<OmmInvalidUsageException>(() => packedMsg.AddMsg(refreshMsg, itemHandle));

                    Assert.Equal($"Attempt to add RefreshMsg with service name of {invalid_service_name} that was not included in the SourceDirectory. Dropping this RefreshMsg.", ex.Message);
                }
                else
                {
                    refreshMsg.ServiceId(invalid_service_id).Name("IBM.N").MarkForClear().Complete(true).ClearCache(true).State(OmmState.StreamStates.OPEN,
                        OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Unsolicited Refresh Completed");

                    var ex = Assert.Throws<OmmInvalidUsageException>(() => packedMsg.AddMsg(refreshMsg, itemHandle));

                    Assert.Equal($"Attempt to add RefreshMsg with service Id of {invalid_service_id} that was not included in the SourceDirectory. Dropping this RefreshMsg.", ex.Message);
                }

                simpleConsumer.UnInitialize();

                for (int i = 0; i < 5; i++)
                {
                    provider.Dispatch(1000000);
                }

                Assert.Empty(serverBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }
            finally
            {
                Assert.Null(exception);
                Assert.NotNull(provider);
                provider?.Uninitialize();
            }
        }

        [Theory]
        [InlineData(1)]
        [InlineData(5)]
        public void PackedMsgInitBufferAndAddMsgWithSmallBufferForIProviderTest(int bufferSize)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "18008";

            SimpleConsumerTest simpleConsumer = new(m_Output, port);
            ReactorOptions reactorOption = new();
            long clientHandle = 0;
            long itemHandle = 0;

            try
            {
                provider = new OmmProvider(config.Port(port).OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH)
                    .AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient);

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case EmaRdm.MMT_LOGIN:
                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType())
                                .Solicited(true), providerEvent.Handle);

                            itemHandle = providerEvent.Handle;
                            clientHandle = providerEvent.ClientHandle;

                            break;
                        case EmaRdm.MMT_DIRECTORY:
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

                OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl;

                simpleConsumer.Initialize(reactorOption, null, true, true);

                for (int i = 0; i < 5; i++)
                {
                    provider.Dispatch(1000000);
                }

                /* Ensure SimpleConsumer receive CHANNEL_UP, CHANNEL_READY, LOGIN and DIRECTORY event */
                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                Assert.Single(serverBaseImpl.ConnectedChannelList);

                PackedMsg packedMsg = new(provider);

                packedMsg.InitBuffer(clientHandle, bufferSize);

                // Checking public interfaces
                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(bufferSize, packedMsg.RemainingSize());

                RefreshMsg refreshMsg = new();
                FieldList fieldList = new();
                fieldList.AddReal(22, 3390, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(30, 50, OmmReal.MagnitudeTypes.EXPONENT_0);

                refreshMsg.ServiceId(5).Name("IBM.N").MarkForClear().Complete(true).ClearCache(true).State(OmmState.StreamStates.OPEN,
                                        OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Unsolicited Refresh Completed").MarkForClear().Payload(fieldList.MarkForClear().Complete());

                var ex = Assert.Throws<OmmInvalidUsageException>(() => packedMsg.AddMsg(refreshMsg, itemHandle));

                Assert.Equal("Failed IMsg.Encode() with error: Failure: The buffer provided does not have sufficient space to perform the operation.", ex.Message);
                Assert.Equal(OmmInvalidUsageException.ErrorCodes.BUFFER_TOO_SMALL, ex.ErrorCode);

                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(bufferSize, packedMsg.RemainingSize());

                simpleConsumer.UnInitialize();

                for (int i = 0; i < 10; i++)
                {
                    provider.Dispatch(1000000);
                }

                Assert.Empty(serverBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }
            finally
            {
                Assert.Null(exception);
                Assert.NotNull(provider);
                provider?.Uninitialize();
            }
        }

        [Theory]
        [InlineData(false, true)]
        [InlineData(true, false)]
        public void PackedMsgInitBufferAndSubmitForIProviderTest(bool defaultSize, bool setServiceName)
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            OmmException? exception = null;
            OmmProvider? provider = null;
            OmmProviderItemClientTest providerClient = new();

            string port = "18010";

            SimpleConsumerTest simpleConsumer = new(m_Output, port);
            ReactorOptions reactorOption = new();
            long clientHandle = 0;
            long itemHandle = 0;

            try
            {
                provider = new OmmProvider(config.Port(port).OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH)
                    .AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL), providerClient);

                Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

                providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
                {
                    switch (requestMsg.DomainType())
                    {
                        case EmaRdm.MMT_LOGIN:
                            provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                                .DomainType(requestMsg.DomainType())
                                .Solicited(true), providerEvent.Handle);

                            itemHandle = providerEvent.Handle;
                            clientHandle = providerEvent.ClientHandle;

                            break;
                        case EmaRdm.MMT_DIRECTORY:
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

                OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl;

                simpleConsumer.Initialize(reactorOption, null, true, true);

                for (int i = 0; i < 5; i++)
                {
                    provider.Dispatch(1000000);
                }

                /* Ensure SimpleConsumer receive CHANNEL_UP, CHANNEL_READY, LOGIN and DIRECTORY event */
                Assert.Equal(4, simpleConsumer.EventQueue.Count);

                Assert.Single(serverBaseImpl.ConnectedChannelList);

                PackedMsg packedMsg = new(provider);

                int bufferSize = defaultSize ? 6000 : 3000;

                if (defaultSize)
                {
                    packedMsg.InitBuffer(clientHandle);
                }
                else
                {
                    packedMsg.InitBuffer(clientHandle, bufferSize);
                }

                // Checking public interfaces
                Assert.Equal(0, packedMsg.PackedMsgCount());
                Assert.Equal(bufferSize, packedMsg.RemainingSize());

                RefreshMsg refreshMsg = new();
                FieldList fieldList = new();
                fieldList.AddReal(22, 3390, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(30, 50, OmmReal.MagnitudeTypes.EXPONENT_0);

                if (setServiceName)
                {
                    refreshMsg.ServiceName("DIRECT_FEED").Name("IBM.N").MarkForClear().Complete(true).ClearCache(true).State(OmmState.StreamStates.OPEN,
                        OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Unsolicited Refresh Completed").MarkForClear().Payload(fieldList.MarkForClear().Complete());
                }
                else
                {
                    refreshMsg.ServiceId(5).Name("IBM.N").MarkForClear().Complete(true).ClearCache(true).State(OmmState.StreamStates.OPEN,
                        OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Unsolicited Refresh Completed").MarkForClear().Payload(fieldList.MarkForClear().Complete());
                }

                packedMsg.AddMsg(refreshMsg, itemHandle);

                Assert.Equal(1, packedMsg.PackedMsgCount());

                int expectedRemaining = defaultSize ? 5922 : 2922;

                Assert.Equal(expectedRemaining, packedMsg.RemainingSize());

                UpdateMsg updateMsg = new();
                fieldList.Clear();
                fieldList.AddReal(22, 3490, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                   .AddReal(25, 3998, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                   .AddReal(30, 60, OmmReal.MagnitudeTypes.EXPONENT_0);

                if (setServiceName)
                {
                    updateMsg.ServiceName("DIRECT_FEED").Name("IBM.N").MarkForClear().Payload(fieldList.MarkForClear().Complete());
                }
                else
                {
                    updateMsg.ServiceId(5).Name("IBM.N").MarkForClear().Payload(fieldList.MarkForClear().Complete());
                }

                packedMsg.AddMsg(updateMsg, itemHandle);

                Assert.Equal(2, packedMsg.PackedMsgCount());

                expectedRemaining = defaultSize ? 5879 : 2879;
                Assert.Equal(expectedRemaining, packedMsg.RemainingSize());

                simpleConsumer.EventQueue.Clear(); /* Clears all previous events in the event queue */

                provider.Submit(packedMsg);

                /* Make sure that the ADHSimulator receives the messages. */
                Thread.Sleep(1000);

                Assert.Equal(2, simpleConsumer.EventQueue.Count);

                TestReactorEvent testReactorEvent = simpleConsumer.EventQueue.Dequeue();

                Assert.Equal(TestReactorEventType.MSG, testReactorEvent.EventType);
                ReactorMsgEvent msgEvent = (ReactorMsgEvent)testReactorEvent.ReactorEvent;

                /* Check with the first message */
                RefreshMsg decodeRefreshMsg = new();

                decodeRefreshMsg.Decode(msgEvent.Msg!, Codec.MajorVersion(), Codec.MinorVersion(),
                    DataDictionary);

                int expectedStreamId = 1;

                Assert.Equal(expectedStreamId, decodeRefreshMsg.StreamId());
                Assert.True(decodeRefreshMsg.HasName);
                Assert.Equal("IBM.N", decodeRefreshMsg.Name());
                Assert.True(decodeRefreshMsg.HasServiceId);
                Assert.Equal(5, decodeRefreshMsg.ServiceId());
                Assert.Equal(OmmState.StreamStates.OPEN, decodeRefreshMsg.State().StreamState);
                Assert.Equal(OmmState.DataStates.OK, decodeRefreshMsg.State().DataState);
                Assert.Equal(OmmState.StatusCodes.NONE, decodeRefreshMsg.State().StatusCode);
                Assert.True(decodeRefreshMsg.MarkForClear().Complete());
                Assert.True(decodeRefreshMsg.ClearCache());
                Assert.False(decodeRefreshMsg.Solicited());
                Assert.Equal(DataType.DataTypes.FIELD_LIST, decodeRefreshMsg.MarkForClear().Payload().DataType);

                var decodeFieldList = decodeRefreshMsg.MarkForClear().Payload().FieldList();

                var iterator = decodeFieldList.GetEnumerator();
                Assert.True(iterator.MoveNext());
                FieldEntry entry = iterator.Current;
                Assert.Equal(22, entry.FieldId);
                Assert.Equal(33.90, entry.OmmRealValue().AsDouble());

                Assert.True(iterator.MoveNext());
                entry = iterator.Current;
                Assert.Equal(25, entry.FieldId);
                Assert.Equal(39.94, entry.OmmRealValue().AsDouble());

                Assert.True(iterator.MoveNext());
                entry = iterator.Current;
                Assert.Equal(30, entry.FieldId);
                Assert.Equal(50, entry.OmmRealValue().AsDouble());

                Assert.False(iterator.MoveNext());

                /* Check with the second message */
                testReactorEvent = simpleConsumer.EventQueue.Dequeue();

                Assert.Equal(TestReactorEventType.MSG, testReactorEvent.EventType);
                msgEvent = (ReactorMsgEvent)testReactorEvent.ReactorEvent;

                UpdateMsg decodeUpdateMsg = new();

                decodeUpdateMsg.Decode(msgEvent.Msg!, Codec.MajorVersion(), Codec.MinorVersion(),
                    DataDictionary);

                Assert.Equal(expectedStreamId, decodeUpdateMsg.StreamId());
                Assert.True(decodeUpdateMsg.HasName);
                Assert.Equal("IBM.N", decodeUpdateMsg.Name());
                Assert.True(decodeUpdateMsg.HasServiceId);
                Assert.Equal(5, decodeUpdateMsg.ServiceId());
                Assert.Equal(DataType.DataTypes.FIELD_LIST, decodeUpdateMsg.MarkForClear().Payload().DataType);

                decodeFieldList = decodeUpdateMsg.MarkForClear().Payload().FieldList();

                iterator = decodeFieldList.GetEnumerator();
                Assert.True(iterator.MoveNext());
                entry = iterator.Current;
                Assert.Equal(22, entry.FieldId);
                Assert.Equal(34.90, entry.OmmRealValue().AsDouble());

                Assert.True(iterator.MoveNext());
                entry = iterator.Current;
                Assert.Equal(25, entry.FieldId);
                Assert.Equal(39.98, entry.OmmRealValue().AsDouble());

                Assert.True(iterator.MoveNext());
                entry = iterator.Current;
                Assert.Equal(30, entry.FieldId);
                Assert.Equal(60, entry.OmmRealValue().AsDouble());

                Assert.False(iterator.MoveNext());

                simpleConsumer.UnInitialize();

                for (int i = 0; i < 5; i++)
                {
                    provider.Dispatch(1000000);
                }

                Assert.Empty(serverBaseImpl.ConnectedChannelList);
            }
            catch (OmmException ommException)
            {
                exception = ommException;
            }
            finally
            {
                Assert.Null(exception);
                Assert.NotNull(provider);
                provider?.Uninitialize();
            }
        }
    }
}
