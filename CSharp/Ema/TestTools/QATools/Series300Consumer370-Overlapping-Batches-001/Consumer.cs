/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using System;
using System.Collections.Generic;
using System.Threading;
using static LSEG.Ema.Access.DataType;

namespace LSEG.Ema.Example.Traning.Consumer;

internal class AppClient : IOmmConsumerClient
{
    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmConsumerEvent _)
    {
        PrintEvent(_);
        PrintMsgHeader(refreshMsg);

        Console.WriteLine("Item State: " + refreshMsg.State());

        if (DataType.DataTypes.FIELD_LIST == refreshMsg.Payload().DataType)
            Decode(refreshMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnUpdateMsg(UpdateMsg updateMsg, IOmmConsumerEvent _)
    {
        PrintEvent(_);
        PrintMsgHeader(updateMsg);

        if (DataType.DataTypes.FIELD_LIST == updateMsg.Payload().DataType)
            Decode(updateMsg.Payload().FieldList());

        Console.WriteLine();
    }

    public void OnStatusMsg(StatusMsg statusMsg, IOmmConsumerEvent _)
    {
        PrintEvent(_);
        PrintMsgHeader(statusMsg);

        if (statusMsg.HasState)
            Console.WriteLine("Item State: " + statusMsg.State());

        Console.WriteLine();
    }

    void Decode(FieldList fieldList)
    {
        foreach (FieldEntry fieldEntry in fieldList)
        {
            Console.Write("Fid: " + fieldEntry.FieldId + " Name = " + fieldEntry.Name + " DataType: " + DataType.AsString(fieldEntry.Load!.DataType) + " Value: ");

            if (Data.DataCode.BLANK == fieldEntry.Code)
                Console.WriteLine(" blank");
            else
                switch (fieldEntry.LoadType)
                {
                    case DataTypes.REAL:
                        Console.WriteLine(fieldEntry.OmmRealValue().AsDouble());
                        break;
                    case DataTypes.DATE:
                        Console.WriteLine(fieldEntry.OmmDateValue().Day + " / " + fieldEntry.OmmDateValue().Month + " / " + fieldEntry.OmmDateValue().Year);
                        break;
                    case DataTypes.TIME:
                        Console.WriteLine(fieldEntry.OmmTimeValue().Hour + ":" + fieldEntry.OmmTimeValue().Minute + ":" + fieldEntry.OmmTimeValue().Second + ":" + fieldEntry.OmmTimeValue().Millisecond);
                        break;
                    case DataTypes.INT:
                        Console.WriteLine(fieldEntry.IntValue());
                        break;
                    case DataTypes.UINT:
                        Console.WriteLine(fieldEntry.UIntValue());
                        break;
                    case DataTypes.ASCII:
                        Console.WriteLine(fieldEntry.OmmAsciiValue());
                        break;
                    case DataTypes.ENUM:
                        Console.WriteLine(fieldEntry.HasEnumDisplay ? fieldEntry.EnumDisplay() : fieldEntry.EnumValue());
                        break;
                    case DataTypes.RMTES:
                        Console.WriteLine(fieldEntry.OmmRmtesValue());
                        break;
                    case DataTypes.ERROR:
                        Console.WriteLine("(" + fieldEntry.OmmErrorValue().ErrorCodeAsString() + ")");
                        break;
                    default:
                        Console.WriteLine();
                        break;
                }
        }
    }

    void PrintEvent(IOmmConsumerEvent @event)
    {
        Console.WriteLine("DEBUG Item Handle: " + @event.Handle);
        Console.WriteLine("DEBUG Closure: " + (@event.Closure != null ? @event.Closure : "<not set>"));
    }

    void PrintMsgHeader(Access.Msg msg)
    {
        Console.WriteLine("Item Name: " + (msg.HasName ? msg.Name() : "<not set>"));
        Console.WriteLine("Service Name: " + (msg.HasServiceName ? msg.ServiceName() : "<not set>"));
    }
}

public class Consumer
{
    public static void Main(string[] args)
    {
        try
        {
            var cmdLine = ParseCommandLine(args);

            AppClient appClient = new();

            using OmmConsumer consumer = new(new OmmConsumerConfig());

            var ommConsumerInitSleep = TimeSpan.FromSeconds(3);
            Thread.Sleep(ommConsumerInitSleep);
            Console.WriteLine();
            Console.WriteLine($"DEBUG: Done sleep of {ommConsumerInitSleep} during OmmConsumer init");
            Console.WriteLine();

            SendBatchRequest(
                itemList: new[] { "TRI.N", "IBM.N", "ORL.N", "FB.N", "GGL.N", "X.N", },
                viewData: cmdLine.FirstBatchView ? new[] { 22, 25 } : null,
                closure: "1st batch",
                isStreaming: cmdLine.FirstBatchStreaming);
            SendBatchRequest(
                itemList: new[] { "ABC.N", "DEF.N", "GHI.N", "JKL.N", "MNO.N", "X.N", },
                viewData: cmdLine.SecondBatchView
                    ? (cmdLine.SameViewData ? new[] { 22, 25 } : new[] { 11, 32 })
                    : null,
                closure: "2nd batch",
                isStreaming: cmdLine.SecondBatchStreaming);

            Thread.Sleep(cmdLine.RunTime); // API calls OnRefreshMsg(), OnUpdateMsg() and OnStatusMsg()

            #region Local functions

            void SendBatchRequest(IReadOnlyCollection<string> itemList, IReadOnlyCollection<int>? viewData = null, object? closure = null, bool isStreaming = true)
            {
                var batch = new ElementList()
                    .AddArray(EmaRdm.ENAME_BATCH_ITEM_LIST,
                        ConvertToOmmArray(itemList, (arr, item) => arr.AddAscii(item)));
                if (viewData?.Count > 0)
                    batch
                        .AddUInt(EmaRdm.ENAME_VIEW_TYPE, EmaRdm.VT_FIELD_ID_LIST)
                        .AddArray(EmaRdm.ENAME_VIEW_DATA,
                            ConvertToOmmArray(viewData, (arr, datum) => arr.AddInt(datum), fixedWidth: 2));
                batch
                    .Complete();

                var handle = consumer.RegisterClient(
                    new RequestMsg().ServiceName(cmdLine.ServiceName).Payload(batch).InterestAfterRefresh(isStreaming),
                    appClient,
                    closure);
                Console.WriteLine($"DEBUG: batch sent, closure = {closure}, handle = {handle}");
            }

            OmmArray ConvertToOmmArray<T>(IReadOnlyCollection<T> seq, Action<OmmArray, T> add, int? fixedWidth = null)
            {
                var array = new OmmArray();
                if (fixedWidth.HasValue)
                    array.FixedWidth = fixedWidth.Value;
                foreach (var item in seq)
                    add(array, item);
                return array.Complete();
            }

            #endregion
        }
        catch (OmmException ommException)
        {
            Console.WriteLine(ommException.Message);
        }
    }

    private static CmdLine ParseCommandLine(string[] args)
    {
        var spanToParse = args.AsSpan();
        var result = new CmdLine();
        while (spanToParse.Length > 0)
        {
            switch (spanToParse[0])
            {
                case "-batch1-streaming":
                    {
                        spanToParse = ParseParamValue(spanToParse, p => result.FirstBatchStreaming = ParseBatchStreamingOption(p));
                        break;
                    }
                case "-batch2-streaming":
                    {
                        spanToParse = ParseParamValue(spanToParse, p => result.SecondBatchStreaming = ParseBatchStreamingOption(p));
                        break;
                    }
                case "-batch1-view":
                    {
                        spanToParse = ParseParamValue(spanToParse, p => result.FirstBatchView = ParseBatchViewOption(p));
                        break;
                    }
                case "-batch2-view":
                    {
                        spanToParse = ParseParamValue(spanToParse, p => result.SecondBatchView = ParseBatchViewOption(p));
                        break;
                    }
                case "-sv":
                case "-same-view":
                    {
                        result.SameViewData = true;
                        spanToParse = spanToParse.Slice(1);
                        break;
                    }
                case "-s":
                case "-service-name":
                    {
                        spanToParse = ParseParamValue(spanToParse, p => result.ServiceName = p);
                        break;
                    }
                case "-rt":
                case "-runtime":
                case "-run-time":
                    {
                        spanToParse = ParseParamValue(spanToParse, p =>
                            result.RunTime = ParseTimeSpanOption(p));
                        break;
                    }
                case "-?":
                case "-h":
                case "-help":
                    {
                        Console.WriteLine(@"Usage:
  ./Cons370 [options]

Options:
  -batch1-streaming  Sets first batch streaming. Possible values ""snapshot"" and ""stream"".
  -batch2-streaming  Sets second batch streaming. Possible values ""snapshot"" and ""stream"".
  -batch1-view       Sets first batch view. Possible values ""view"" and ""full"".
  -batch2-view       Sets second batch view. Possible values ""view"" and ""full"".
  -same-view,        If passed, sends same view in both batches. 
  -sv
  -service-name,     Sets name of the service to connect to.
  -s
  -runtime,          Sets time after which consumer stops in seconds (fractions allowed) or in format hh:mm:ss.
  -run-time,         Defaults to 1 minute.
  -rt
");
                        Environment.Exit(0);
                        break;
                    }
                default: throw new ApplicationException($"Unknown command line parameter: {spanToParse[0]}");
            }
        }
        return result;

        static Span<string> ParseParamValue(Span<string> span, Action<string> action)
        {
            if (span.Length < 2)
                throw new ApplicationException($"Command line parameter \"{span[0]}\" must have a value");
            action(span[1]);
            return span.Slice(2);
        }

        static TimeSpan ParseTimeSpanOption(string value) =>
            double.TryParse(value, out var seconds)
                ? TimeSpan.FromSeconds(seconds)
                : TimeSpan.Parse(value);

        static bool ParseBatchStreamingOption(string value) =>
            value switch
            {
                "snapshot" => false,
                "stream" => true,
                _ => throw new ApplicationException($"Unknown batch snapshot option: {value}")
            };

        static bool ParseBatchViewOption(string value) =>
            value switch
            {
                "view" => true,
                "full" => false,
                _ => throw new ApplicationException($"Unknown batch view option: {value}")
            };
    }

    private class CmdLine
    {
        public bool FirstBatchStreaming { get; set; }
        public bool SecondBatchStreaming { get; set; }
        public bool FirstBatchView { get; set; } = true;
        public bool SecondBatchView { get; set; } = true;
        public bool SameViewData { get; set; }
        public string ServiceName { get; set; } = "DIRECT_FEED";
        public TimeSpan RunTime { get; set; } = TimeSpan.FromMinutes(1);
    }
}
