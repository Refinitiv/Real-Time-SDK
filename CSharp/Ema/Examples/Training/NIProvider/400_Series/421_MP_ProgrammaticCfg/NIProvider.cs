/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;

namespace LSEG.Ema.Example.Traning.NIProvider;

public class NIProvider
{
    static Map CreateProgrammaticConfig()
    {
        Map innerMap = new Map();
        Map configMap = new Map();
        ElementList elementList = new ElementList();
        ElementList innerElementList = new ElementList();

        elementList.AddAscii("DefaultNiProvider", "Provider_1");

        innerElementList.AddAscii("Channel", "Channel_10");
        innerElementList.AddAscii("Directory", "Directory_1");
        innerElementList.AddAscii("Logger", "Logger_1");
        innerElementList.AddUInt("XmlTraceToStdout", 0);
        innerElementList.AddUInt("RefreshFirstRequired", 1);
        innerElementList.Complete();

        innerMap.AddKeyAscii("Provider_1", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("NiProviderList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("NiProviderGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerElementList.AddEnum("ChannelType", EmaConfig.ConnectionTypeEnum.SOCKET);
        innerElementList.AddUInt("GuaranteedOutputBuffers", 5000);
        innerElementList.AddUInt("ConnectionPingTimeout", 30000);
        innerElementList.AddAscii("Host", "localhost");
        innerElementList.AddAscii("Port", "14003");
        innerElementList.AddUInt("TcpNodelay", 1);

        innerMap.AddKeyAscii("Channel_10", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("ChannelList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("ChannelGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerMap.AddKeyAscii("Logger_1", MapAction.ADD,
                    new ElementList()
                    .AddEnum("LoggerType", EmaConfig.LoggerTypeEnum.STDOUT)
                    .AddAscii("FileName", "logFile")
                    .AddEnum("LoggerSeverity", EmaConfig.LoggerLevelEnum.INFO).Complete()).Complete();

        elementList.AddMap("LoggerList", innerMap);

        elementList.Complete();
        innerMap.Clear();

        configMap.AddKeyAscii("LoggerGroup", MapAction.ADD, elementList);
        elementList.Clear();

        Map serviceMap = new Map();
        innerElementList.AddUInt("ServiceId", 2);
        innerElementList.AddAscii("Vendor", "company name");
        innerElementList.AddUInt("IsSource", 0);
        innerElementList.AddUInt("AcceptingConsumerStatus", 0);
        innerElementList.AddUInt("SupportsQoSRange", 0);
        innerElementList.AddUInt("SupportsOutOfBandSnapshots", 0);
        innerElementList.AddAscii("ItemList", "#.itemlist");

        OmmArray array = new OmmArray();
        array.AddUInt(EmaConfig.CapabilitiesEnum.MMT_MARKET_PRICE);
        array.AddUInt(EmaConfig.CapabilitiesEnum.MMT_MARKET_BY_PRICE);
        array.AddUInt(200);
        innerElementList.AddArray("Capabilities", array.Complete());
        array.Clear();

        array.AddAscii("Dictionary_1");
        innerElementList.AddArray("DictionariesUsed", array.Complete());
        array.Clear();

        ElementList inner2 = new ElementList();

        Series series = new Series();
        inner2.AddAscii("Timeliness", "Timeliness::RealTime");
        inner2.AddAscii("Rate", "Rate::TickByTick");
        series.AddEntry(inner2.Complete());
        inner2.Clear();

        inner2.AddUInt("Timeliness", 100);
        inner2.AddUInt("Rate", 100);
        series.AddEntry(inner2.Complete());
        inner2.Clear();

        innerElementList.AddSeries("QoS", series.Complete());

        elementList.AddElementList("InfoFilter", innerElementList.Complete());
        innerElementList.Clear();

        innerElementList.AddUInt("ServiceState", 1);
        innerElementList.AddUInt("AcceptingRequests", 1);
        elementList.AddElementList("StateFilter", innerElementList.Complete());
        innerElementList.Clear();

        serviceMap.AddKeyAscii("NI_PUB", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerMap.AddKeyAscii("Directory_1", MapAction.ADD, serviceMap.Complete());
        
        elementList.AddAscii("DefaultDirectory", "Directory_1");
        elementList.AddMap("DirectoryList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("DirectoryGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();

        innerElementList.AddEnum("DictionaryType", EmaConfig.DictionaryTypeEnum.CHANNEL);
        innerMap.AddKeyAscii("Dictionary_1", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("DictionaryList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, elementList.Complete());

        return configMap.Complete();
    }

    public static void Main()
    {
        OmmProvider? provider = null;
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig().Config(CreateProgrammaticConfig());

            provider = new OmmProvider(config.UserName("user"));

            long ibmHandle = 5;
            long triHandle = 6;

            FieldList fieldList = new FieldList();

            fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(fieldList.Complete()).Complete(true), ibmHandle);

            fieldList.Clear();

            fieldList.AddReal(22, 4100, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 4200, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 20, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 40, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(fieldList.Complete()).Complete(true), triHandle);

            Thread.Sleep(1000);

            for (int i = 0; i < 60; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("IBM.N").Payload(fieldList.Complete()), ibmHandle);

                fieldList.Clear();
                fieldList.AddReal(22, 4100 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 21 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("TRI.N").Payload(fieldList.Complete()), triHandle);
                Thread.Sleep(1000);
            }
        }
        catch (Exception excp)
        {
            Console.WriteLine(excp.Message);
        }

        finally
        {
            provider?.Uninitialize();
        }
    }
}