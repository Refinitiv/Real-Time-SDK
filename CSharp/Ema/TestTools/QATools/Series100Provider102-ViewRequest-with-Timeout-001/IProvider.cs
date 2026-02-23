/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Linq;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public int count = 0;

    private readonly CmdLine m_CmdLine;

    public AppClient(CmdLine cmdLine)
    {
        m_CmdLine = cmdLine;
    }

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                ProcessMarketPriceRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
            .Complete(true),
            providerEvent.Handle);
    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (m_CmdLine.NoFirstItemResponse && reqMsg.Name() == "X.N" && count == 0)
        {
            Thread.Sleep(m_CmdLine.ItemResponseDelay);
            Console.WriteLine("DEBUG -------------; not send response\n\n");
            count++;
        }
        else
        {
            FieldList fieldList = new();
            if (reqMsg.Payload().DataType == DataType.DataTypes.ELEMENT_LIST)
            {
                var el = reqMsg.Payload().ElementList();
                var fieldsArray = el
                    .Where(x => x.Name == EmaRdm.ENAME_VIEW_DATA && x.Code != Data.DataCode.BLANK && x.LoadType == DataType.DataTypes.ARRAY)
                    .SelectMany(x => x.OmmArrayValue())
                    .Select(x => (int)x.OmmIntValue().Value)
                    .Select(x => x switch
                    {
                        22 => (fieldId: x, mantissa: 3990, magnitudeType: OmmReal.MagnitudeTypes.EXPONENT_NEG_2),
                        25 => (fieldId: x, mantissa: 3994, magnitudeType: OmmReal.MagnitudeTypes.EXPONENT_NEG_2),
                        30 => (fieldId: x, mantissa: 9, magnitudeType: OmmReal.MagnitudeTypes.EXPONENT_0),
                        31 => (fieldId: x, mantissa: 19, magnitudeType: OmmReal.MagnitudeTypes.EXPONENT_0),
                        _ => (fieldId: x, mantissa: 1, magnitudeType: OmmReal.MagnitudeTypes.EXPONENT_0)
                    });
                foreach (var fieldWithValue in fieldsArray)
                {
                    fieldList.AddReal(fieldWithValue.fieldId, fieldWithValue.mantissa, fieldWithValue.magnitudeType);
                }
            }
            else
                fieldList
                    .AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2)
                    .AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0)
                    .AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

            providerEvent.Provider.Submit(
                new RefreshMsg()
                    .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName()).Solicited(true)
                    .State(OmmState.StreamStates.NON_STREAMING, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
                    .Payload(fieldList.Complete()).Complete(true),
                providerEvent.Handle);
        }
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg()
            .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }
}

public class CmdLine
{
    public bool NoFirstItemResponse { get; set; }
    public TimeSpan ItemResponseDelay { get; set; } = TimeSpan.FromSeconds(1);
    public TimeSpan RunTime { get; set; } = TimeSpan.FromMinutes(1);
}

public class IProvider
{
    public static void Main(string[] args)
    {
        OmmProvider? provider = null;
        try
        {
            var cmdLine = ParseCommandLine(args);

            AppClient appClient = new AppClient(cmdLine);

            OmmIProviderConfig config = new OmmIProviderConfig();

            provider = new OmmProvider(config.Port("14002"), appClient);

            Thread.Sleep(cmdLine.RunTime);
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

    private static CmdLine ParseCommandLine(string[] args)
    {
        var spanToParse = args.AsSpan();
        var result = new CmdLine();
        while (spanToParse.Length > 0)
        {
            switch (spanToParse[0])
            {
                case "-item-response-delay":
                    {
                        spanToParse = ParseParamValue(spanToParse, p =>
                            result.ItemResponseDelay = ParseTimeSpanOption(p));
                        break;
                    }
                case "-no-first-item-response":
                    {
                        result.NoFirstItemResponse = true;
                        spanToParse = spanToParse.Slice(1);
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
  ./IProv102 [options]

Options:
  -item-response-delay     Sets delay before no response in seconds (fractions allowed) or in format hh:mm:ss.
  -no-first-item-response  If passed, then first item request will not be responded.
  -runtime,                Sets time after which provider stops in seconds (fractions allowed) or in format hh:mm:ss.
  -run-time,               Defaults to 1 minute.
  -rt
");
                        Environment.Exit(0);
                        break;
                    }
                default: throw new ApplicationException($"Unknown command line parameter: {spanToParse[0]}");
            };
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
    }
}