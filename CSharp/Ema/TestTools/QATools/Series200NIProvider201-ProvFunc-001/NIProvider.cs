/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

namespace LSEG.Ema.Example.Traning.NIProvider;

using System;
using System.Text;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

// APIQA
class AppClient : IOmmProviderClient
{
    public DataDictionary dataDictionary = new DataDictionary();
    public bool fldDictComplete = false;
    public bool enumTypeComplete = false;
    public bool dumpDictionary = false;

    public bool IsConnectionUp { get; private set; }

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("Received Refresh. Item Handle: " + providerEvent.Handle + " Closure: " + providerEvent.Closure);
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));
        Console.WriteLine("Item State: " + refreshMsg.State());

        Decode(refreshMsg, refreshMsg.Complete());

        if (refreshMsg.State().StreamState == OmmState.StreamStates.OPEN)
        {
            if (refreshMsg.State().DataState == OmmState.DataStates.OK)
                IsConnectionUp = true;
            else
                IsConnectionUp = false;
        }
        else
            IsConnectionUp = false;
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("Received Status. Item Handle: " + providerEvent.Handle + " Closure: " + providerEvent.Closure);
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
        {
            Console.WriteLine("Item State: " + statusMsg.State());
            if (statusMsg.State().StreamState == OmmState.StreamStates.OPEN)
            {
                IsConnectionUp = (statusMsg.State().DataState == OmmState.DataStates.OK);
            }
            else
                IsConnectionUp = false;
        }
    }

    void Decode(Msg msg, bool complete)
    {
        switch (msg.Payload().DataType)
        {
            case DataType.DataTypes.SERIES:

                if ("RWFFld".Equals(msg.Name()))
                {
                    dataDictionary.DecodeFieldDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_NORMAL);

                    if (complete)
                    {
                        fldDictComplete = true;
                    }
                }
                else if ("RWFEnum".Equals(msg.Name()))
                {
                    dataDictionary.DecodeEnumTypeDictionary(msg.Payload().Series(), EmaRdm.DICTIONARY_NORMAL);

                    if (complete)
                    {
                        enumTypeComplete = true;
                    }
                }

                if (fldDictComplete && enumTypeComplete)
                {
                    Console.WriteLine();
                    Console.WriteLine("Dictionary download complete");
                    Console.WriteLine("Dictionary Id : " + dataDictionary.DictionaryId);
                    Console.WriteLine("Dictionary field version : " + dataDictionary.FieldVersion);
                    Console.WriteLine("Number of dictionary entries : " + dataDictionary.Entries().Count);

                    if (dumpDictionary)
                        Console.WriteLine(dataDictionary);
                }

                break;
            default:
                break;
        }
    }

}
// END APIQA

public class NIProvider
{

    static void PrintHelp()
    {

        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n"
            + "  if the application will attempt to make an encrypted \n "
            + "           connection, ChannelType must need to be set to ChannelType::RSSL_ENCRYPTED \n"
            + "            in EMA configuration file.\n"
            + "  -ph Proxy host name.\n"
            + "  -pp Proxy port number.\n"
            + "  -plogin User name on proxy server.\n"
            + "  -ppasswd Password on proxy server.\n"
            //APIQA
            + "  -objectname objectName set.\n"
            + "  -reqDict enable to run the test with dictionary streaming.\n"
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
                //APIQA
                else if ("-reqDict".Equals(args[argsCount]))
                {
                    ++argsCount;
                }
                //END APIQA
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


    public static void Main(string[] args)
    {
        AppClient appClient = new AppClient();
        bool sendRefreshMsg = false;
        bool reqDict = false;

        OmmProvider? provider = null;
        try
        {
            OmmNiProviderConfig config = new OmmNiProviderConfig();

            if (!ReadCommandlineArgs(args, config))
                return;

            provider = new OmmProvider(config.OperationModel(OmmNiProviderConfig.OperationModelMode.USER_DISPATCH)
                .UserName("user").ProviderName("Provider_4"),
                appClient);

            provider.Dispatch(1_000_000);

            int idx = 0;

            while (idx < args.Length)
            {
                if ("-reqDict".Equals(args[idx]))
                {
                    reqDict = true;
                    appClient.dumpDictionary = true;
                }

                ++idx;
            }

            if (reqDict)
            {
                provider.RegisterClient(new RequestMsg().Name("RWFFld").Filter(EmaRdm.DICTIONARY_NORMAL)
                    .ServiceName("NI_PUB").DomainType(EmaRdm.MMT_DICTIONARY), appClient);

                provider.RegisterClient(new RequestMsg().Name("RWFEnum").Filter(EmaRdm.DICTIONARY_NORMAL)
                    .ServiceName("NI_PUB").DomainType(EmaRdm.MMT_DICTIONARY), appClient);

            }
            long ibmHandle = 5;
            long triHandle = 6;
            //APIQA
            long aaoHandle = 7;
            long aggHandle = 8;

            Map map = new Map();
            FieldList summary = new FieldList();
            FieldList entryLoad = new FieldList();

            summary.AddEnumValue(15, 840);
            summary.AddEnumValue(53, 1);
            summary.AddEnumValue(3423, 1);
            summary.AddEnumValue(1709, 2);

            map.SummaryData(summary.Complete());

            entryLoad.AddRealFromDouble(3427, 7.76, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            entryLoad.AddRealFromDouble(3429, 9600);
            entryLoad.AddEnumValue(3428, 2);
            entryLoad.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

            map.AddKeyAscii("100", MapAction.ADD, entryLoad.Complete());

            provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("NI_PUB").Name("AAO.V")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .Payload(map.Complete()).Complete(true),
                aaoHandle);

            summary.Clear();

            summary.AddEnumValue(15, 840);
            summary.AddEnumValue(53, 1);
            summary.AddEnumValue(3423, 1);
            summary.AddEnumValue(1709, 2);

            map.Clear();
            map.SummaryData(summary.Complete());

            entryLoad.Clear();

            entryLoad.AddRealFromDouble(3427, 9.92, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            entryLoad.AddRealFromDouble(3429, 1200);
            entryLoad.AddEnumValue(3428, 2);
            entryLoad.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

            map.AddKeyAscii("222", MapAction.ADD, entryLoad.Complete());

            provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("NI_PUB").Name("AGG.V")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .Payload(map.Complete()).Complete(true),
                aggHandle);
            //END APIQA

            FieldList fieldList = new FieldList();

            fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                .Payload(fieldList.Complete()).Complete(true),
                ibmHandle);

            fieldList.Clear();

            fieldList.AddReal(22, 4100, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 4200, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 20, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 40, OmmReal.MagnitudeTypes.EXPONENT_0);

            provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                    .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                    .Payload(fieldList.Complete()).Complete(true),
                    triHandle);

            //Thread.sleep(1000);
            provider.Dispatch(1_000_000);

            for (int i = 0; i < 60; i++)
            {
                if (appClient.IsConnectionUp)
                {
                    if (sendRefreshMsg)
                    {
                        summary.Clear();
                        summary.AddEnumValue(15, 840);
                        summary.AddEnumValue(53, 1);
                        summary.AddEnumValue(3423, 1);
                        summary.AddEnumValue(1709, 2);

                        map.Clear();

                        map.SummaryData(summary.Complete());

                        entryLoad.Clear();

                        entryLoad.AddRealFromDouble(3427, 7.76, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        entryLoad.AddRealFromDouble(3429, 9600);
                        entryLoad.AddEnumValue(3428, 2);
                        entryLoad.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

                        map.AddKeyAscii("100", MapAction.ADD, entryLoad.Complete());

                        provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("NI_PUB").Name("AAO.V")
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                            .Payload(map.Complete()).Complete(true),
                            aaoHandle);

                        summary.Clear();

                        summary.AddEnumValue(15, 840);
                        summary.AddEnumValue(53, 1);
                        summary.AddEnumValue(3423, 1);
                        summary.AddEnumValue(1709, 2);

                        map.Clear();

                        map.SummaryData(summary.Complete());

                        entryLoad.Clear();

                        entryLoad.AddRealFromDouble(3427, 9.92, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        entryLoad.AddRealFromDouble(3429, 1200);
                        entryLoad.AddEnumValue(3428, 2);
                        entryLoad.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

                        map.AddKeyAscii("222", MapAction.ADD, entryLoad.Complete());

                        provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("NI_PUB").Name("AGG.V")
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                            .Payload(map.Complete()).Complete(true),
                            aggHandle);

                        fieldList.Clear();

                        fieldList.AddReal(22, 14400, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(25, 14700, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

                        provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("IBM.N")
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                            .Payload(fieldList.Complete()).Complete(true),
                            ibmHandle);

                        fieldList.Clear();

                        fieldList.AddReal(22, 4100, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(25, 4200, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 20, OmmReal.MagnitudeTypes.EXPONENT_0);
                        fieldList.AddReal(31, 40, OmmReal.MagnitudeTypes.EXPONENT_0);

                        provider.Submit(new RefreshMsg().ServiceName("NI_PUB").Name("TRI.N")
                            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "UnSolicited Refresh Completed")
                            .Payload(fieldList.Complete()).Complete(true), triHandle);
                        sendRefreshMsg = false;
                    }
                    else
                    {
                        fieldList.Clear();

                        fieldList.AddReal(22, 14400 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                        provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("IBM.N").Payload(fieldList.Complete()), ibmHandle);

                        fieldList.Clear();

                        fieldList.AddReal(22, 4100 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        fieldList.AddReal(30, 21 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                        provider.Submit(new UpdateMsg().ServiceName("NI_PUB").Name("TRI.N").Payload(fieldList.Complete()), triHandle);
                        //APIQA
                        entryLoad.Clear();

                        entryLoad.AddRealFromDouble(3427, 7.76 + i * 0.1, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        entryLoad.AddRealFromDouble(3429, 9600);
                        entryLoad.AddEnumValue(3428, 2);
                        entryLoad.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

                        map.Clear();

                        map.AddKeyAscii("100", MapAction.UPDATE, entryLoad.Complete());

                        provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("NI_PUB").Name("AAO.V")
                            .Payload(map.Complete()),
                            aaoHandle);

                        entryLoad.Clear();

                        entryLoad.AddRealFromDouble(3427, 9.92 + i * 0.1, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                        entryLoad.AddRealFromDouble(3429, 1200);
                        entryLoad.AddEnumValue(3428, 2);
                        entryLoad.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

                        map.Clear();

                        map.AddKeyAscii("222", MapAction.UPDATE, entryLoad.Complete());

                        provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).ServiceName("NI_PUB").Name("AGG.V")
                            .Payload(map.Complete()),
                            aggHandle);
                        //END APIQA
                    }
                }
                else
                {
                    sendRefreshMsg = true;
                }
                provider.Dispatch(1_000_000);
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
