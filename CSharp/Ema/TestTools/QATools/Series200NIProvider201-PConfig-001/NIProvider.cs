/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.NIProvider;

using System;
using System.Threading;

using LSEG.Ema.Access;
using static LSEG.Ema.Access.EmaConfig;

public class NIProvider
{
    static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n"
            + "  -?\tShows this usage\n"
            + "  if the application will attempt to make an encrypted \n "
            + "           connection, ChannelType must need to be set to ChannelType::RSSL_ENCRYPTED \n"
            + "            in EMA configuration file.\n"
            + "  -ph Proxy host name.\n"
            + "  -pp Proxy port number.\n"
            + "  -plogin User name on proxy server.\n"
            + "  -ppasswd Password on proxy server.\n"
            + "\n");
    }

    static bool ReadCommandlineArgs(string[] args, OmmNiProviderConfig config)
    {
        try
        {
            int argsCount = 0;

            while (argsCount < args.Length)
            {
                if ("-?".Equals(args[argsCount]))
                {
                    PrintHelp();
                    return false;
                }
                else if ("-ph".Equals(args[argsCount]))
                {
                    config.ProxyHost(args[++argsCount]);
                    ++argsCount;
                }
                else if ("-pp".Equals(args[argsCount]))
                {
                    config.ProxyPort(argsCount < (args.Length - 1) ? args[++argsCount] : string.Empty);
                    ++argsCount;
                }
                else if ("-plogin".Equals(args[argsCount]))
                {
                    config.ProxyUserName(argsCount < (args.Length - 1) ? args[++argsCount] : string.Empty);
                    ++argsCount;
                }
                else if ("-ppasswd".Equals(args[argsCount]))
                {
                    config.ProxyPassword(argsCount < (args.Length - 1) ? args[++argsCount] : string.Empty);
                    ++argsCount;
                }
                else // unrecognized command line argument
                {
                    PrintHelp();
                    return false;
                }
            }
        }
        catch (Exception)
        {
            PrintHelp();
            return false;
        }

        return true;
    }

    static Map CreateProgrammaticConfig()
    {
        Map innerMap = new Map();
        Map configMap = new Map();
        ElementList elementList = new ElementList();
        ElementList innerElementList = new ElementList();

        elementList.AddAscii("DefaultNiProvider", "Provider_4");

        innerElementList.AddAscii("Channel", "Channel_13");
        innerElementList.AddAscii("Directory", "Directory_1");
        innerElementList.AddUInt("XmlTraceToStdout", 1);
        innerElementList.AddUInt("RefreshFirstRequired", 1);

        innerMap.AddKeyAscii("Provider_4", MapAction.ADD, innerElementList.Complete());

        innerElementList.Clear();

        elementList.AddMap("NiProviderList", innerMap.Complete());

        innerMap.Clear();

        configMap.AddKeyAscii("NiProviderGroup", MapAction.ADD, elementList.Complete());

        elementList.Clear();

        innerElementList.AddEnum("ChannelType", ConnectionTypeEnum.ENCRYPTED);
        innerElementList.AddEnum("CompressionType", CompressionTypeEnum.NONE);
        innerElementList.AddUInt("GuaranteedOutputBuffers", 5000);
        innerElementList.AddUInt("ConnectionPingTimeout", 50000);
        innerElementList.AddAscii("Host", "localhost");
        innerElementList.AddAscii("Port", "14002");
        innerElementList.AddAscii("ObjectName", "P_ObjectName");
        innerElementList.AddUInt("TcpNodelay", 1);
        innerElementList.AddAscii("ProxyHost", "proxyHostToConnectTo");
        innerElementList.AddAscii("ProxyPort", "proxyPortToConnectTo");

        innerMap.AddKeyAscii("Channel_13", MapAction.ADD, innerElementList.Complete());

        innerElementList.Clear();

        elementList.AddMap("ChannelList", innerMap.Complete());

        innerMap.Clear();

        configMap.AddKeyAscii("ChannelGroup", MapAction.ADD, elementList.Complete());

        elementList.Clear();

        Map serviceMap = new Map();
        innerElementList.AddUInt("ServiceId", 1);
        innerElementList.AddAscii("Vendor", "company name");
        innerElementList.AddUInt("IsSource", 0);
        innerElementList.AddUInt("AcceptingConsumerStatus", 0);
        innerElementList.AddUInt("SupportsQoSRange", 0);
        innerElementList.AddUInt("SupportsOutOfBandSnapshots", 0);
        innerElementList.AddAscii("ItemList", "#.itemlist");

        OmmArray array = new OmmArray();
        array.AddUInt(CapabilitiesEnum.MMT_MARKET_PRICE);
        array.AddUInt(CapabilitiesEnum.MMT_MARKET_BY_PRICE);
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

        serviceMap.AddKeyAscii("TEST_NI_PUB", MapAction.ADD, elementList.Complete());

        elementList.Clear();

        innerMap.AddKeyAscii("Directory_1", MapAction.ADD, serviceMap.Complete());

        elementList.AddAscii("DefaultDirectory", "Directory_1");
        elementList.AddMap("DirectoryList", innerMap.Complete());

        configMap.AddKeyAscii("DirectoryGroup", MapAction.ADD, elementList.Complete());
        elementList.Clear();
        innerMap.Clear();

        innerElementList.AddEnum("DictionaryType", DictionaryTypeEnum.CHANNEL);
        innerMap.AddKeyAscii("Dictionary_1", MapAction.ADD, innerElementList.Complete());
        innerElementList.Clear();

        elementList.AddMap("DictionaryList", innerMap.Complete());
        innerMap.Clear();

        configMap.AddKeyAscii("DictionaryGroup", MapAction.ADD, elementList.Complete());

        return configMap.Complete();
    }

    public static void Main(string[] args)
    {
        OmmProvider? provider = null;
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            if (!ReadCommandlineArgs(args, config))
                return;

            provider = new OmmProvider(config.Config(CreateProgrammaticConfig()).ProviderName("Provider_4").UserName("user"));

            long ibmHandle = 5;
            long triHandle = 6;

            FieldList fieldList = new FieldList();

            fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("TEST_NI_PUB").Name("IBM.N")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .Payload(fieldList.Complete()).Complete(true),
                ibmHandle);

            fieldList.Clear();

            fieldList.AddReal(22, 4100, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 4200, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 20, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 40, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("TEST_NI_PUB").Name("TRI.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(fieldList.Complete()).Complete(true), triHandle);

            Thread.Sleep(1000);

            for (int i = 0; i < 60; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().ServiceName("TEST_NI_PUB").Name("IBM.N").Payload(fieldList.Complete()),
                    ibmHandle);

                fieldList.Clear();
                fieldList.AddReal(22, 4100 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 21 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().ServiceName("TEST_NI_PUB").Name("TRI.N").Payload(fieldList.Complete()),
                    triHandle);

                Thread.Sleep(1000);
            }
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
        finally
        {
            provider?.Uninitialize();
        }
    }
}
