/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.Consumer;

using System;
using System.Threading;
using LSEG.Ema.Access;

public class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine($"{refreshMsg}\nevent channel info (refresh)\n{consumerEvent.ChannelInformation()}");
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine($"{updateMsg}\nevent channel info (update)\n{consumerEvent.ChannelInformation()}");
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent consumerEvent)
    {
        Console.WriteLine($"{statusMsg}\nevent channel info (status)\n{consumerEvent.ChannelInformation()}");
    }
}

public class Consumer
{
    static Map CreateProgramaticConfig()
    {
        Map innerMap = new Map();
        Map configMap = new Map();
        ElementList elementList = new ElementList();
        ElementList innerElementList = new ElementList();

        elementList.AddAscii("DefaultConsumer", "Consumer_1");

        innerElementList.AddAscii("SessionChannelSet", "Connection_1, Connection_2");
        innerElementList.AddAscii("Dictionary", "Dictionary_1");
        innerElementList.AddUInt("ItemCountHint", 5000);
        innerElementList.AddUInt("ServiceCountHint", 5000);
        innerElementList.AddUInt("ObeyOpenWindow", 0);
        innerElementList.AddUInt("PostAckTimeout", 5000);
        innerElementList.AddUInt("RequestTimeout", 5000);
        innerElementList.AddUInt("MaxOutstandingPosts", 5000);
        innerElementList.AddInt("DispatchTimeoutApiThread", 1);
        innerElementList.AddUInt("MaxDispatchCountApiThread", 500);
        innerElementList.AddUInt("MaxDispatchCountUserThread", 500);
        innerElementList.AddInt("ReconnectAttemptLimit", 10);
        innerElementList.AddInt("ReconnectMinDelay", 2000);
        innerElementList.AddInt("ReconnectMaxDelay", 6000);
        innerElementList.AddUInt("XmlTraceToStdout", 0);
        innerElementList.AddUInt("MsgKeyInUpdates", 1);
        innerElementList.AddUInt("SessionEnhancedItemRecovery", 1);

        innerElementList.Complete();
        innerMap.AddKeyAscii("Consumer_1", MapAction.ADD, innerElementList);
        innerElementList.Clear();

        innerMap.Complete();
        elementList.AddMap("ConsumerList", innerMap);
        innerMap.Clear();

        elementList.Complete();
        configMap.AddKeyAscii("ConsumerGroup", MapAction.ADD, elementList);
        elementList.Clear();

        innerElementList.AddAscii("ChannelSet", "Channel_1, Channel_7");
        innerElementList.AddInt("ReconnectAttemptLimit", 4);
        innerElementList.AddInt("ReconnectMinDelay", 2000);
        innerElementList.AddInt("ReconnectMaxDelay", 6000);

        innerElementList.Complete();
        innerMap.AddKeyAscii("Connection_1", MapAction.ADD, innerElementList);
        innerElementList.Clear();

        innerElementList.AddAscii("ChannelSet", "Channel_10, Channel_11");
        innerElementList.AddInt("ReconnectAttemptLimit", 4);
        innerElementList.AddInt("ReconnectMinDelay", 3000);
        innerElementList.AddInt("ReconnectMaxDelay", 4000);

        innerElementList.Complete();
        innerMap.AddKeyAscii("Connection_2", MapAction.ADD, innerElementList);
        innerElementList.Clear();

        innerMap.Complete();
        elementList.AddMap("SessionChannelList", innerMap);
        innerMap.Clear();

        elementList.Complete();
        configMap.AddKeyAscii("SessionChannelGroup", MapAction.ADD, elementList);
        elementList.Clear();

        innerElementList.AddEnum("ChannelType", EmaConfig.ConnectionTypeEnum.SOCKET);
        innerElementList.AddEnum("CompressionType", EmaConfig.CompressionTypeEnum.ZLIB);
        innerElementList.AddUInt("GuaranteedOutputBuffers", 5000);
        innerElementList.AddUInt("ConnectionPingTimeout", 50000);
        innerElementList.AddAscii("Host", "localhost");
        innerElementList.AddAscii("Port", "14002");
        innerElementList.AddUInt("TcpNodelay", 0);

        innerElementList.Complete();
        innerMap.AddKeyAscii("Channel_1", MapAction.ADD, innerElementList);
        innerElementList.Clear();

        innerElementList.AddEnum("ChannelType", EmaConfig.ConnectionTypeEnum.SOCKET);
        innerElementList.AddEnum("CompressionType", EmaConfig.CompressionTypeEnum.ZLIB);
        innerElementList.AddUInt("GuaranteedOutputBuffers", 5000);
        innerElementList.AddUInt("ConnectionPingTimeout", 50000);
        innerElementList.AddAscii("Host", "localhost");
        innerElementList.AddAscii("Port", "14003");
        innerElementList.AddUInt("TcpNodelay", 0);

        innerElementList.Complete();
        innerMap.AddKeyAscii("Channel_7", MapAction.ADD, innerElementList);
        innerElementList.Clear();

        innerElementList.AddEnum("ChannelType", EmaConfig.ConnectionTypeEnum.SOCKET);
        innerElementList.AddEnum("CompressionType", EmaConfig.CompressionTypeEnum.ZLIB);
        innerElementList.AddUInt("GuaranteedOutputBuffers", 5000);
        innerElementList.AddUInt("ConnectionPingTimeout", 50000);
        innerElementList.AddAscii("Host", "localhost");
        innerElementList.AddAscii("Port", "14004");
        innerElementList.AddUInt("TcpNodelay", 0);

        innerElementList.Complete();
        innerMap.AddKeyAscii("Channel_10", MapAction.ADD, innerElementList);
        innerElementList.Clear();

        innerElementList.AddEnum("ChannelType", EmaConfig.ConnectionTypeEnum.SOCKET);
        innerElementList.AddEnum("CompressionType", EmaConfig.CompressionTypeEnum.ZLIB);
        innerElementList.AddUInt("GuaranteedOutputBuffers", 5000);
        innerElementList.AddUInt("ConnectionPingTimeout", 50000);
        innerElementList.AddAscii("Host", "localhost");
        innerElementList.AddAscii("Port", "14005");
        innerElementList.AddUInt("TcpNodelay", 0);

        innerElementList.Complete();
        innerMap.AddKeyAscii("Channel_11", MapAction.ADD, innerElementList);
        innerElementList.Clear();

        innerMap.Complete();
        elementList.AddMap("ChannelList", innerMap);
        innerMap.Clear();

        elementList.Complete();
        configMap.AddKeyAscii("ChannelGroup", MapAction.ADD, elementList);
        elementList.Clear();

        innerElementList.AddEnum("DictionaryType", EmaConfig.DictionaryTypeEnum.FILE);
        innerElementList.AddAscii("RdmFieldDictionaryFileName", "./RDMFieldDictionary");
        innerElementList.AddAscii("EnumTypeDefFileName", "./enumtype.def");

        innerElementList.Complete();
        innerMap.AddKeyAscii("Dictionary_1", MapAction.ADD, innerElementList);
        innerElementList.Clear();

        innerMap.Complete();
        elementList.AddMap("DictionaryList", innerMap);
        innerMap.Clear();

        elementList.Complete();
        configMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, elementList);
        elementList.Clear();

        configMap.Complete();
        return configMap;
    }

    static void Main()
    {
        OmmConsumer? consumer = null;
        try
        {
            AppClient appClient = new();
            consumer = new OmmConsumer(new OmmConsumerConfig().Config(CreateProgramaticConfig()), appClient); // use programmatic configuration parameters

            consumer.RegisterClient(new RequestMsg().ServiceName("DIRECT_FEED").Name("LSEG.L"), appClient);

            Thread.Sleep(60000);    // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            consumer?.Uninitialize();
        }
    }
}