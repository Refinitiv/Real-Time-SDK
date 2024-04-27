/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using System.Collections.Generic;

using Xunit.Abstractions;

using LSEG.Ema.Rdm;
using LSEG.Eta.ValueAdd.Reactor;


namespace LSEG.Ema.Access.Tests.OmmConsumerTests;

public class ModifyIOCtlTest
{
    ITestOutputHelper output;

    public ModifyIOCtlTest(ITestOutputHelper output)
    {
        this.output = output;
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
                new IOCtlSetting() { Code = IOCtlCode.MAX_NUM_BUFFERS, Setting = -10, IsValid = false,
                    Message = "value must be (0 >= value < 2^31", ErrorCode =  OmmInvalidUsageException.ErrorCodes.FAILURE },
                new IOCtlSetting() { Code = IOCtlCode.NUM_GUARANTEED_BUFFERS, Setting = -100, IsValid = false,
                    Message = "value must be (0 >= value < 2^31", ErrorCode =  OmmInvalidUsageException.ErrorCodes.FAILURE }
            }
        },

        new object[]
        {
            new IOCtlSetting[]
            {
                new IOCtlSetting() { Code = IOCtlCode.MAX_NUM_BUFFERS, Setting = 100 }
            }
        },

        new object[]
        {
            new IOCtlSetting[]
            {
                new IOCtlSetting() { Code = IOCtlCode.MAX_NUM_BUFFERS, Setting = 200 },
                new IOCtlSetting() { Code = IOCtlCode.NUM_GUARANTEED_BUFFERS, Setting = 200 }
            }
        },

        new object[]
        {
            new IOCtlSetting[]
            {
                new IOCtlSetting() {Code = IOCtlCode.HIGH_WATER_MARK, Setting = 10 }
            }
        },

        new object[]
        {
            new IOCtlSetting[]
            {
                new IOCtlSetting() { Code = IOCtlCode.MAX_NUM_BUFFERS, Setting = 10 },
                new IOCtlSetting() { Code = IOCtlCode.NUM_GUARANTEED_BUFFERS, Setting = 10 },
                new IOCtlSetting() { Code = IOCtlCode.HIGH_WATER_MARK, Setting = 10 },
                new IOCtlSetting() { Code = IOCtlCode.SYSTEM_READ_BUFFERS, Setting = 10 },
                new IOCtlSetting() { Code = IOCtlCode.SYSTEM_WRITE_BUFFERS, Setting = 10 },
                new IOCtlSetting() { Code = IOCtlCode.COMPRESSION_THRESHOLD, Setting = 350 }
            }
        },

        new object[]
        {
            new IOCtlSetting[]
            {
                // The compression must be enabled between provider and consumer in order
                // to modify the COMPRESSION_THRESHOLD code. The code below is invalid as it
                // is less than the default value for LZ4_COMPRESSION_THRESHOLD.
                new IOCtlSetting() { Code = IOCtlCode.COMPRESSION_THRESHOLD, Setting = 10, IsValid = false,
                    Message = "Failed to modify I/O option = COMPRESSION_THRESHOLD. Reason: FAILURE. Error text: Channel.IOCtl failed, error: value must be equal to or greater than",
                    ErrorCode = OmmInvalidUsageException.ErrorCodes.FAILURE
                }
            }
        }
    };

    /// <summary>
    /// Submits GenericMsg after modifying IOCtl codes as provided in the <paramref name="settings"/>.
    /// </summary>
    /// <param name="settings">Array of <see cref="IOCtlSetting"/> entries defining IOCtl modifications</param>
    [Theory]
    [MemberData(nameof(IOCtlSettings))]
    public void ModifyIOCtlBeforeGenericMsg_Test(IOCtlSetting[] settings)
    {
        ProviderSessionOptions providerSessionOpts = new ProviderSessionOptions()
        {
            SendDictionaryResp = true,
            CompressionType = Eta.Transports.CompressionType.LZ4
        };

        providerSessionOpts.SendMarketDataItemUpdate = false;
        ProviderTest providerTest = new ProviderTest(providerSessionOpts, output);
        ReactorOptions reactorOptions = new ReactorOptions();
        providerTest.Initialize(reactorOptions);

        string hostString = $"localhost:{providerTest.ServerPort}";
        output.WriteLine($"Connect with {hostString}");

        OmmConsumerConfig config = new OmmConsumerConfig();
        OmmConsumer? consumer = null;

        OmmConsumerItemClientTest consumerClient;

        config.Host(hostString);

        config.OperationModel(OmmConsumerConfig.OperationModelMode.USER_DISPATCH);

        consumer = new OmmConsumer(config.Host(hostString));

        output.WriteLine("Finished creating an OmmConsumer");

        // modify IOCtl codes as defined in test parameters
        foreach (IOCtlSetting setting in settings)
        {
            output.WriteLine($"Modify IOCtl: {setting.Code}, new value: {setting.Setting}");
            // some settings are known to be invalid and are expected to raise an exception
            if (setting.IsValid)
            {
                consumer.ModifyIOCtl(setting.Code, setting.Setting);
            }
            else
            {
                OmmInvalidUsageException ex = Assert.Throws<OmmInvalidUsageException>(
                    () => consumer.ModifyIOCtl(setting.Code, setting.Setting));

                Assert.NotEmpty(ex.Message);
                if (setting.Message is not null)
                    Assert.Contains(setting.Message, ex.Message);

                if (setting.ErrorCode is not null)
                    Assert.Equal(setting.ErrorCode, ex.ErrorCode);
            }
        }

        // submit ordinary Generic Message as usually after modifying the IOCtl codes

        consumerClient = new OmmConsumerItemClientTest(consumer);

        string itemName = "ItemName";

        RequestMsg requestMsg = new();
        requestMsg.DomainType(EmaRdm.MMT_MARKET_PRICE)
            .ServiceName(ProviderTest.DefaultService.Info.ServiceName.ToString())
            .Name(itemName)
            .InterestAfterRefresh(true);

        long handle = consumer.RegisterClient(requestMsg, consumerClient, this);

        Assert.True(handle > 0);

        /* Checks the expected RefreshMsg from the market data request */
        consumerClient.RefreshMsgHandler = (refreshMsg, consEvent) =>
        {
            Assert.Equal(handle, consEvent.Handle);

            OmmConsumerTest.CheckRefreshPayload(providerTest, 0, refreshMsg, itemName);
        };


        consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);

        Assert.Equal(1, consumerClient.ReceivedOnAll);
        Assert.Equal(1, consumerClient.ReceivedOnRefresh);
        Assert.Equal(0, consumerClient.ReceivedOnStatus);
        Assert.Equal(0, consumerClient.ReceivedOnUpdate);

        // Submit generic message to the provider side.
        {
            ElementList elementList = new();
            elementList.AddInt("element1", 555)
                .AddUInt("element2", 666)
                .Complete();
            consumer.Submit(new GenericMsg().DomainType(200).Name("genericMsg").Payload(elementList),
                handle);

            consumerClient.GenericMsgHandler = (genericMsg, consEvent) =>
            {
                Assert.Equal(handle, consEvent.Handle);
                Assert.Same(this, consEvent.Closure);

                Assert.Equal(200, genericMsg.DomainType());
                Assert.Equal("genericMsg", genericMsg.Name());

                // Check generic message from provider side
                Assert.Equal(DataType.DataTypes.ELEMENT_LIST, genericMsg.Payload().DataType);

                ElementList elementList = genericMsg.Payload().ElementList();

                var elementListIt = elementList.GetEnumerator();

                Assert.True(elementListIt.MoveNext());
                var elementEntry = elementListIt.Current;

                Assert.Equal("element1", elementEntry.Name);
                Assert.Equal(555, elementEntry.IntValue());

                Assert.True(elementListIt.MoveNext());
                elementEntry = elementListIt.Current;

                Assert.Equal("element2", elementEntry.Name);
                Assert.Equal((ulong)666, elementEntry.UIntValue());

                Assert.False(elementListIt.MoveNext());
            };

            // Waiting for provider to send generic message back.
            consumer.Dispatch(DispatchTimeout.INFINITE_WAIT);

            Assert.Equal(2, consumerClient.ReceivedOnAll);
            Assert.Equal(1, consumerClient.ReceivedOnGeneric);
        }

        consumer.Unregister(handle);

        consumer?.Uninitialize();
        providerTest.UnInitialize();
    }
}
