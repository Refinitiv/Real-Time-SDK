/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System.Threading;
using System.Collections.Generic;

using Xunit.Abstractions;

using LSEG.Eta.ValueAdd.Reactor;
using LSEG.Eta.Rdm;
using System;

namespace LSEG.Ema.Access.Tests.OmmIProviderTests;

public class ModifyIOCtlTest : IDisposable
{
    public void Dispose()
    {
        EtaGlobalPoolTestUtil.Clear();
    }

    ITestOutputHelper m_Output;

    public ModifyIOCtlTest(ITestOutputHelper output)
    {
        m_Output = output;
    }

    public class IOCtlSetting
    {
        // the IOCtl code being invoked
        public IOCtlCode Code;
        // value that it receives
        public int Setting;
        // whether an exception is expected
        public bool IsValid = true;
        // expected exception message is expected
        public string? Message;
        // expected error code for the exception
        public int? ErrorCode;
    }

    /// <summary>
    /// Various combinations of IOCtl codes and their new values used for testing.
    /// </summary>
    /// Each entry in this list corresponds to a separate test run.
    public static IEnumerable<object[]> IOCtlSettings => new List<object[]>
    {
        new object[]
        {
            new IOCtlSetting[]
            {
                new() { Code = IOCtlCode.MAX_NUM_BUFFERS, Setting = -10, IsValid = false,
                    Message = "value must be (0 >= value < 2^31", ErrorCode = -1  }
            }
        },

        new object[]
        {
            new IOCtlSetting[]
            {
                new() { Code = IOCtlCode.MAX_NUM_BUFFERS, Setting = 50 }
            }
        },

        new object[]
        {
            new IOCtlSetting[]
            {
                new() { Code = IOCtlCode.MAX_NUM_BUFFERS, Setting = 100 },
                new() { Code = IOCtlCode.NUM_GUARANTEED_BUFFERS, Setting = 100 }
            }
        },

        new object[]
        {
            new IOCtlSetting[]
            {
                new() { Code = IOCtlCode.HIGH_WATER_MARK, Setting = 10 }
            }
        },

        new object[]
        {
            new IOCtlSetting[]
            {
                new() { Code = IOCtlCode.MAX_NUM_BUFFERS, Setting = 10 },
                new() { Code = IOCtlCode.NUM_GUARANTEED_BUFFERS, Setting = 10 },
                new() { Code = IOCtlCode.HIGH_WATER_MARK, Setting = 10 },
                new() { Code = IOCtlCode.SYSTEM_READ_BUFFERS, Setting = 10 },
                new() { Code = IOCtlCode.SYSTEM_WRITE_BUFFERS, Setting = 10 },
                new() { Code = IOCtlCode.COMPRESSION_THRESHOLD, Setting = 10 }
            }
        },

        new object[]
        {
            new IOCtlSetting[]
            {
                // The compression must be enabled between provider and consumer in order
                // to modify the COMPRESSION_THRESHOLD code. The code below is valid as it
                // is greater than the default value for LZ4_COMPRESSION_THRESHOLD.
                new() {
                    Code = IOCtlCode.COMPRESSION_THRESHOLD,
                    Setting = 350,
                    IsValid = true
                }
            }
        }
    };

    /// Test cases within a class are run sequentially, therefore the same port number can
    /// be reused by tests contained by this class
    private const string PROVIDER_PORT = "19101";

    /// <summary>
    /// Applies IOCtl settings to a valid client handle.
    /// </summary>
    /// <remarks>
    /// <para>
    /// This test creates an EMA Provider that receives a connection from a single
    /// Consumer. Connected Client Handle is retrieved in a Request message callback from
    /// the LOGIN message request.</para>
    ///
    /// <para>
    /// IOCtl settings are then applied to that connection thanks to the retrieved Client
    /// Handle.</para>
    ///
    /// <para>
    /// Some of these settings are invalid - exception is expected and must be
    /// thrown. Some of them are valid - no exception is expected to be thrown.</para>
    ///
    /// </remarks>
    /// <param name="settings"></param>
    [Theory]
    [MemberData(nameof(IOCtlSettings))]
    public void ApplyIOCtlToClientHandleTest(IOCtlSetting[] settings)
    {
        OmmIProviderConfig config = new OmmIProviderConfig();
        OmmProvider? provider = null;
        OmmProviderItemClientTest providerClient = new();

        SimpleConsumerTest simpleConsumer = new(m_Output, port: PROVIDER_PORT);
        ReactorOptions reactorOption = new();

        try
        {
            provider = new OmmProvider(config.Port(PROVIDER_PORT)
                .OperationModel(OmmIProviderConfig.OperationModelMode.API_DISPATCH), providerClient);

            long consumerHandle = 0;

            providerClient.ReqMsgHandler = (requestMsg, providerEvent) =>
            {
                switch (requestMsg.DomainType())
                {
                    case (int)DomainType.LOGIN:
                        consumerHandle = providerEvent.Handle;

                        provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                            .DomainType(requestMsg.DomainType())
                            .Solicited(true), providerEvent.Handle);
                        break;
                }
            };

            Assert.NotNull(provider);
            Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

            // Consumer sends Login request which is handled in the callback above to
            // retrieve the connection Item Handle
            simpleConsumer.Initialize(reactorOption, null, true, true);

            // Wait for the provider to accept the client connection.
            Thread.Sleep(3000);

            Assert.Equal(4, simpleConsumer.EventQueue.Count);

            var reactorEvent = simpleConsumer.EventQueue.Dequeue();
            ReactorChannelEvent channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_UP, channelEvent.EventType);

            reactorEvent = simpleConsumer.EventQueue.Dequeue();
            Assert.Equal(OmmConsumerTests.TestReactorEventType.LOGIN_MSG, reactorEvent.EventType);

            reactorEvent = simpleConsumer.EventQueue.Dequeue();
            Assert.Equal(OmmConsumerTests.TestReactorEventType.DIRECTORY_MSG, reactorEvent.EventType);

            reactorEvent = simpleConsumer.EventQueue.Dequeue();
            channelEvent = (ReactorChannelEvent)reactorEvent.ReactorEvent;
            Assert.Equal(ReactorChannelEventType.CHANNEL_READY, channelEvent.EventType);

            Assert.Equal(1, providerClient.ReceivedOnReqMsg);

            OmmServerBaseImpl serverBaseImpl = (OmmServerBaseImpl)provider.m_OmmProviderImpl!;
            Assert.Single(serverBaseImpl.ConnectedChannelList);

            // modify IOCtl codes as defined in test parameters
            foreach (IOCtlSetting setting in settings)
            {
                m_Output.WriteLine($"BEGIN Modify IOCtl: {setting.Code}, new value: {setting.Setting} valid? {setting.IsValid}");

                // consumer handle must be obtained by this moment, and it is not zero
                Assert.NotEqual(0, consumerHandle);

                // some settings are known to be invalid and are expected to raise an exception
                if (setting.IsValid)
                {
                    // IProvider applies IOCtl only when a handle is specified,
                    // even for the Provider itself (when no items are involved).
                    provider!.ModifyIOCtl(setting.Code, setting.Setting, consumerHandle);
                }
                else
                {
                    OmmInvalidUsageException ex = Assert.Throws<OmmInvalidUsageException>(()
                        => provider!.ModifyIOCtl(setting.Code, setting.Setting, consumerHandle));

                    Assert.NotEmpty(ex.Message);
                    if (setting.Message is not null)
                        Assert.Contains(setting.Message, ex.Message);

                    if (setting.ErrorCode is not null)
                        Assert.Equal(setting.ErrorCode, ex.ErrorCode);
                }
                m_Output.WriteLine($"DONE Modify IOCtl: {setting.Code}, new value: {setting.Setting} valid? {setting.IsValid}");
            }
        }
        finally
        {
            simpleConsumer.UnInitialize();
            provider?.Uninitialize();
        }
    }

    /// Modify IOCtl for the Provider itself, not to the items.
    [Fact]
    public void ApplyIOCtlToProviderTest()
    {
        OmmIProviderConfig config = new OmmIProviderConfig();
        OmmProvider? provider = null;
        OmmProviderItemClientTest providerClient = new();

        SimpleConsumerTest simpleConsumer = new(m_Output, port: PROVIDER_PORT);
        ReactorOptions reactorOption = new();

        try
        {
            provider = new OmmProvider(config.Port(PROVIDER_PORT), providerClient);

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
                        service.AddEntry(23, FilterAction.SET, new ElementList().AddInt("Name", 1).MarkForClear().Complete());
                        payload.AddKeyUInt(5, MapAction.ADD, service.MarkForClear().Complete());

                        provider.Submit(new RefreshMsg().StreamId(requestMsg.StreamId())
                            .DomainType(requestMsg.DomainType())
                            .Solicited(true).MarkForClear().Payload(payload), providerEvent.Handle);
                        break;

                    default:
                        break;
                }
            };

            Assert.NotNull(provider);
            Assert.Contains("DefaultEmaIProvider", provider.ProviderName);

            // Set invalid value (less than 0). Ensure that an exception is thrown
            Assert.Throws<OmmInvalidUsageException>(()
                => provider!.ModifyIOCtl(IOCtlCode.SERVER_NUM_POOL_BUFFERS, -100, 0));

            provider!.ModifyIOCtl(IOCtlCode.SERVER_NUM_POOL_BUFFERS, 100, 0);

            simpleConsumer.Initialize(reactorOption, null, true, true);

            Thread.Sleep(3000); // Wait for the provider to accept the client connection.

            // consumer receives events
            Assert.Equal(4, simpleConsumer.EventQueue.Count);
        }
        finally
        {
            simpleConsumer.UnInitialize();
            provider?.Uninitialize();
        }
    }
}
