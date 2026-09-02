/*|-----------------------------------------------------------------------------
*|            This source code is provided under the Apache 2.0 license
*|  and is provided AS IS with no warranty or guarantee of fit for purpose.
*|                See the project's LICENSE.md for details.
*|           Copyright (C) 2026 LSEG. All rights reserved.
*|-----------------------------------------------------------------------------
*/

namespace LSEG.Ema.Example.Traning.Consumer;

using System;
using System.Threading;
using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using LSEG.Eta.Rdm;
using static LSEG.Ema.Access.DataType;

class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Item Name: " + (refreshMsg.HasName ? refreshMsg.Name() : "<not set>"));
        Console.WriteLine("Stream ID: " + refreshMsg.StreamId());
        Console.WriteLine("Service Name: " + (refreshMsg.HasServiceName ? refreshMsg.ServiceName() : "<not set>"));

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (DataTypes.MAP == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().Map());
        else if (DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList(), false);

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Item Name: " + (updateMsg.HasName ? updateMsg.Name() : "<not set>"));
        Console.WriteLine("Stream ID: " + updateMsg.StreamId());
        Console.WriteLine("Service Name: " + (updateMsg.HasServiceName ? updateMsg.ServiceName() : "<not set>"));

        if (DataTypes.MAP == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().Map());
        else if (DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList(), false);

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent @event)
    {
        Console.WriteLine("Item Name: " + (statusMsg.HasName ? statusMsg.Name() : "<not set>"));
        Console.WriteLine("Stream ID: " + statusMsg.StreamId());
        Console.WriteLine("Service Name: " + (statusMsg.HasServiceName ? statusMsg.ServiceName() : "<not set>"));

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    public void OnGenericMsg(GenericMsg genericMsg, IOmmConsumerEvent @event) { }

    public void OnAckMsg(AckMsg ackMsg, IOmmConsumerEvent @event) { }

    public void OnAllMsg(Msg msg, IOmmConsumerEvent @event) { }

    void Decode(FieldList fieldList, bool newLine)
    {
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write(fieldEntry.Name + "\t");

            if (Data.DataCode.BLANK == fieldEntry.Code)
            {
                Console.WriteLine(" blank");
            }
            else
            {
                switch (fieldEntry.LoadType)
                {
                    case DataTypes.REAL:
                        Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
                        break;

                    case DataTypes.DATE:
                        Console.WriteLine(
                            fieldEntry.OmmDateValue().Day + " / " +
                            fieldEntry.OmmDateValue().Month + " / " +
                            fieldEntry.OmmDateValue().Year);
                        break;

                    case DataTypes.TIME:
                        {
                            OmmTime ommTime = fieldEntry.OmmTimeValue();
                            Console.WriteLine($"{ommTime.Hour}:{ommTime.Minute}:{ommTime.Second}:{ommTime.Millisecond}");
                            break;
                        }

                    case DataTypes.INT:
                        Console.WriteLine(fieldEntry.IntValue());
                        break;

                    case DataTypes.UINT:
                        Console.WriteLine(fieldEntry.UIntValue());
                        break;

                    case DataTypes.ASCII:
                        Console.WriteLine(fieldEntry.OmmAsciiValue());
                        break;

                    case DataTypes.RMTES:
                        Console.WriteLine(fieldEntry.OmmRmtesValue());
                        break;

                    case DataTypes.ENUM:
                        Console.WriteLine(fieldEntry.HasEnumDisplay ?
                            fieldEntry.EnumDisplay() :
                            fieldEntry.EnumValue());
                        break;

                    case DataTypes.ERROR:
                        Console.WriteLine("(" +
                            fieldEntry.OmmErrorValue().ErrorCodeAsString() +
                            ")");
                        break;

                    default:
                        Console.WriteLine();
                        break;
                }
            }

            if (newLine)
                Console.WriteLine();
        }
    }

    void Decode(Map map)
    {
        if (DataTypes.FIELD_LIST == map.SummaryData().DataType)
        {
            Console.WriteLine("Summary :");
            Decode(map.SummaryData().FieldList(), true);
            Console.WriteLine();
        }

        bool firstEntry = true;

        foreach (MapEntry mapEntry in map)
        {
            if (firstEntry)
            {
                firstEntry = false;
                Console.WriteLine("Name\tAction");
                Console.WriteLine();
            }

            switch (mapEntry.Key.DataType)
            {
                case DataTypes.BUFFER:
                    Console.WriteLine(mapEntry.Key.Buffer() + "\t" +
                                      mapEntry.MapActionAsString());
                    break;

                case DataTypes.ASCII:
                    Console.WriteLine(mapEntry.Key.Ascii() + "\t" +
                                      mapEntry.MapActionAsString());
                    break;

                case DataTypes.RMTES:
                    Console.WriteLine(mapEntry.Key.Rmtes() + "\t" +
                                      mapEntry.MapActionAsString());
                    break;

                default:
                    break;
            }

            if (DataTypes.FIELD_LIST == mapEntry.LoadType)
            {
                Console.WriteLine("\t");
                Decode(mapEntry.FieldList(), false);
            }
        }
    }
}

public class Consumer
{
    static void PrintHelp()
    {
        Console.WriteLine(
            "\nOptions:\n" +
        "\t-?                           Shows this usage\n" +
        "\t-slItem                      Specifies Symbol List item name to be requested\r\n" +
        "\t-enhancedSymbolListRequestOn In case specified, Enhanced Symbol List Request feature will be turned on\r\n" +
        "\t-snapshots                   Items from Symbol List will be requested as non-streaming if this parameter is specified (works only together with -enhancedSymbolListRequestOn)\r\n" +
        "\t-mp                          Specifies the list of ordinary Market Price items to be requested (default is empty list)\r\n" +
        "\t-mpPrivate                   If mentioned, ordinary Market Price items are requested as private (if not mentioned items are not private)\r\n" +
        "\t-snapshotMpRequests          If mentioned, ordinary Market Price items are requested as non-streaming (if not mentioned items are streaming)\r\n" +
        "\t-mbp                         Specifies the list of ordinary Market By Price items to be requested (default is empty list)\r\n" +
        "\t-mbpPrivate                  If mentioned, ordinary Market By Price items are requested as private (if not mentioned items are not private)\r\n" +
        "\t-snapshotMbpRequests         If mentioned, ordinary Market By Price items are requested as non-streaming (if not mentioned items are streaming)\r\n" +
        "\t-mbo                         Specifies the list of ordinary Market By Order items to be requested (default is empty list)\r\n" +
        "\t-mboPrivate                  If mentioned, ordinary Market By Order items are requested as private (if not mentioned items are not private)\r\n" +
        "\t-snapshotMboRequests         If mentioned, ordinary Market By Order items are requested as non-streaming (if not mentioned items are streaming)\r\n" +
        "\t-s                           Service name (default is ELEKTRON_DD)\r\n" +
        "\t-consumerName                The desired Consumer configuration entry (default is Consumer_1)\r\n" +
        "Sample command line arguments: " +
        "\"-slItem .BV.N -enhancedSymbolListRequestOn -snapshots " +
        "-mp TRI.N,IBM.N -mpPrivate -snapshotMpRequests " +
        "-mbp ABC.N,DEF.N -mbpPrivate -snapshotMbpRequests " +
        "-mbo XYZ.N,TEST.N -mboPrivate -snapshotMboRequests " +
        "-s DIRECT_FEED\"");
    }

    public static void Main(string[] args)
    {
        int argsCount = 0;

        string itemName = ".AV.N";
        string serviceName = "ELEKTRON_DD";
        string consumerName = "Consumer_1";

        string[]? mpRequests = null;
        string[]? mbpRequests = null;
        string[]? mboRequests = null;

        bool enhancedSymbolListRequestOn = false;
        bool snapshots = false;

        bool privateMpRequests = false;
        bool snapshotMpRequests = false;

        bool privateMbpRequests = false;
        bool snapshotMbpRequests = false;

        bool privateMboRequests = false;
        bool snapshotMboRequests = false;

        string reqString = null;
        string reqMbpString = null;
        string reqMboString = null;

        while (argsCount < args.Length)
        {
            if ("-?".Equals(args[argsCount]))
            {
                PrintHelp();
                return;
            }
            else if ("-slItem".Equals(args[argsCount]))
            {
                itemName = argsCount < args.Length - 1
                    ? args[++argsCount]
                    : ".AV.N";
            }
            else if ("-enhancedSymbolListRequestOn".Equals(args[argsCount]))
            {
                enhancedSymbolListRequestOn = true;
            }
            else if ("-snapshots".Equals(args[argsCount]))
            {
                snapshots = true;
            }
            else if ("-mp".Equals(args[argsCount]))
            {
                reqString = argsCount < (args.Length - 1) ? args[++argsCount] : "TRI.N";
                mpRequests = reqString.Split(',');
            }
            else if ("-mpPrivate".Equals(args[argsCount]))
            {
                privateMpRequests = true;
            }
            else if ("-snapshotMpRequests".Equals(args[argsCount]))
            {
                snapshotMpRequests = true;
            }
            else if ("-mbp".Equals(args[argsCount]))
            {
                reqMbpString = argsCount < (args.Length - 1) ? args[++argsCount] : "TRI.N";
                mbpRequests = reqMbpString.Split(',');
            }
            else if ("-mbpPrivate".Equals(args[argsCount]))
            {
                privateMbpRequests = true;
            }
            else if ("-snapshotMbpRequests".Equals(args[argsCount]))
            {
                snapshotMbpRequests = true;
            }
            else if ("-mbo".Equals(args[argsCount]))
            {
                reqMboString = argsCount < (args.Length - 1) ? args[++argsCount] : "TRI.N";
                mboRequests = reqMboString.Split(',');
            }
            else if ("-mboPrivate".Equals(args[argsCount]))
            {
                privateMboRequests = true;
            }
            else if ("-snapshotMboRequests".Equals(args[argsCount]))
            {
                snapshotMboRequests = true;
            }
            else if ("-s".Equals(args[argsCount]))
            {
                serviceName = args[++argsCount];
            }
            else if ("-consumerName".Equals(args[argsCount]))
            {
                consumerName = argsCount < (args.Length - 1) ? args[++argsCount] : "Consumer_1"; ;
            }
            else // unrecognized command line argument
            {
                Console.WriteLine("Unrecognized parameter: " + args[argsCount]);
                PrintHelp();
                return;
            }

            ++argsCount;
        }

        Console.WriteLine("App settings: \n\t-slItem " + itemName
                + ", \n\tEnhanced SymbolL List Feature on: " + enhancedSymbolListRequestOn
                + ", \n\t-snapshots: " + snapshots
                + ", \n\t-mp: " + reqString
                + ", \n\t-mpPrivate: " + privateMpRequests
                + ", \n\t-snapshotMpRequests: " + snapshotMpRequests
                + ", \n\t-mbp: " + reqMbpString
                + ", \n\t-mbpPrivate: " + privateMbpRequests
                + ", \n\t-snapshotMbpRequests: " + snapshotMbpRequests
                + ", \n\t-mbo: " + reqMboString
                + ", \n\t-mboPrivate: " + privateMboRequests
                + ", \n\t-snapshotMboRequests: " + snapshotMboRequests
                + ", \n\t-serviceName: " + serviceName
                + ", \n\t-consumerName: " + consumerName);

        try
        {
            AppClient appClient = new();

            using OmmConsumer consumer =
                new(new OmmConsumerConfig()
                    .ConsumerName(consumerName)
                    .UserName("user"));

            if (mpRequests != null && mpRequests.Length > 0)
            {
                foreach (string ric in mpRequests)
                {
                    RequestMsg request = new RequestMsg()
                        .DomainType(EmaRdm.MMT_MARKET_PRICE)
                        .ServiceName(serviceName)
                        .Name(ric);

                    if (snapshotMpRequests)
                        request.InterestAfterRefresh(false);

                    if (privateMpRequests)
                        request.PrivateStream(true);

                    consumer.RegisterClient(request, appClient, 0);
                }
            }

            if (mbpRequests != null && mbpRequests.Length > 0)
            {
                foreach (string ric in mbpRequests)
                {
                    RequestMsg request = new RequestMsg()
                        .DomainType(EmaRdm.MMT_MARKET_BY_PRICE)
                        .ServiceName(serviceName)
                        .Name(ric);

                    if (snapshotMbpRequests)
                        request.InterestAfterRefresh(false);

                    if (privateMbpRequests)
                        request.PrivateStream(true);

                    consumer.RegisterClient(request, appClient, 0);
                }
            }

            if (mboRequests != null && mboRequests.Length > 0)
            {
                foreach (string ric in mboRequests)
                {
                    RequestMsg request = new RequestMsg()
                        .DomainType(EmaRdm.MMT_MARKET_BY_ORDER)
                        .ServiceName(serviceName)
                        .Name(ric);

                    if (snapshotMboRequests)
                        request.InterestAfterRefresh(false);

                    if (privateMboRequests)
                        request.PrivateStream(true);

                    consumer.RegisterClient(request, appClient, 0);
                }
            }

            RequestMsg slrequest = new RequestMsg().DomainType(EmaRdm.MMT_SYMBOL_LIST).ServiceName(serviceName).Name(itemName);

            if (enhancedSymbolListRequestOn)
            {
                ElementList payload = new ElementList();
                ElementList eePayload = new ElementList();

                eePayload.AddUInt(":DataStreams", (ulong)(snapshots
                    ? SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS
                        : SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS));

                eePayload.Complete();

                payload.AddElementList(":SymbolListBehaviors", eePayload);
                payload.Complete();

                slrequest.Payload(payload);
            }

            consumer.RegisterClient(slrequest, appClient, 0);


            Thread.Sleep(60000);
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
    }
}
