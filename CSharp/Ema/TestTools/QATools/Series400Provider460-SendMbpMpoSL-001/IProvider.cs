/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Threading;
using LSEG.Ema.Access;
using LSEG.Ema.Rdm;
using LSEG.Eta.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;


class AppClient : IOmmProviderClient
{
    private bool enableRTT;
    public string OrderNr = "100";
    /*
      holds the last latency value for each client that supports RTT requests
      clientLatencyMap.keySet() holds handles to all consumers that can receive RTT requests
    */
    public Dictionary<long, long> clientLatencyMap = new Dictionary<long, long>();

    //holds all item requests for each client
    public Dictionary<long, List<long>> clientItemHandlesMap = new Dictionary<long, List<long>>();
    public Dictionary<long, List<long>> clientMBPItemHandlesMap = new Dictionary<long, List<long>>();
    public Dictionary<long, List<long>> clientMBOItemHandlesMap = new Dictionary<long, List<long>>();
    public Dictionary<long, List<long>> clientSLItemHandlesMap = new Dictionary<long, List<long>>();

    int i = 0;

    FieldList mbpentryLoad = new FieldList();
    UpdateMsg mbpupdateMsg = new UpdateMsg();
    Map mbpmap = new Map();

    FieldList mbofieldList = new FieldList();
    Map mbomap = new Map();

    public AppClient(bool rtt)
    {
        enableRTT = rtt;
    }

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent ev)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, ev);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                ProcessMarketPriceRequest(reqMsg, ev);
                break;
            case EmaRdm.MMT_MARKET_BY_PRICE:
                ProcessMarketByPriceRequest(reqMsg, ev);
                break;
            case EmaRdm.MMT_MARKET_BY_ORDER:
                ProcessMarketByOrderRequest(reqMsg, ev);
                break;
            case EmaRdm.MMT_SYMBOL_LIST:
                ProcessSymbolListRequest(reqMsg, ev);
                break;
            default:
                break;
        }
    }

    public void OnRefreshMsg(RefreshMsg refreshMsg, IOmmProviderEvent ev) { }
    public void OnStatusMsg(StatusMsg statusMsg, IOmmProviderEvent ev) { }

    public void OnGenericMsg(GenericMsg genericMsg, IOmmProviderEvent ev)
    {
        switch (genericMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessRTT(genericMsg, ev);
                break;
            default:
                break;
        }
    }

    public void OnPostMsg(PostMsg postMsg, IOmmProviderEvent ev) { }
    public void OnReissue(RequestMsg reqMsg, IOmmProviderEvent ev) { }
    public void OnClose(RequestMsg reqMsg, IOmmProviderEvent ev)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                clientLatencyMap.Remove(ev.Handle);
                clientItemHandlesMap.Remove(ev.ClientHandle);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                List<long> list = clientItemHandlesMap[ev.ClientHandle];
                list.Remove(ev.Handle);
                if (list.Count == 0)
                {
                    clientItemHandlesMap.Remove(ev.ClientHandle);
                }
                break;
            case EmaRdm.MMT_MARKET_BY_PRICE:
                List<long> mbplist = clientMBPItemHandlesMap[ev.ClientHandle];
                mbplist.Remove(ev.Handle);
                if (mbplist.Count == 0)
                {
                    clientMBPItemHandlesMap.Remove(ev.ClientHandle);
                }
                break;
            case EmaRdm.MMT_MARKET_BY_ORDER:
                List<long> mbolist = clientMBOItemHandlesMap[ev.ClientHandle];
                mbolist.Remove(ev.Handle);
                if (mbolist.Count == 0)
                {
                    clientMBOItemHandlesMap.Remove(ev.ClientHandle);
                }
                break;
            case EmaRdm.MMT_SYMBOL_LIST:
                List<long> sllist = clientSLItemHandlesMap[ev.ClientHandle];
                sllist.Remove(ev.Handle);
                if (sllist.Count == 0)
                {
                    clientSLItemHandlesMap.Remove(ev.ClientHandle);
                }
                break;
            default:
                break;
        }
    }

    public void OnAllMsg(Msg msg, IOmmProviderEvent ev) { }

    void ProcessRTT(GenericMsg genericMsg, IOmmProviderEvent ev)
    {
        if (clientLatencyMap.ContainsKey(ev.Handle))
        {
            if (genericMsg.Payload().DataType == DataType.DataTypes.ELEMENT_LIST && genericMsg.DomainType() == (int)DomainType.LOGIN)
            {
                Console.WriteLine("Received login RTT message from Consumer " + ev.Handle);
                ElementList data = genericMsg.Payload().ElementList();
                foreach (ElementEntry elem in data)
                {
                    if (elem.Name.Equals(EmaRdm.ENAME_TICKS))
                    {
                        Console.WriteLine("        RTT Tick value is: " + elem.UIntValue());
                        long latency = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds() - (long)elem.UIntValue();
                        clientLatencyMap.Add(ev.Handle, latency);
                        Console.WriteLine("        Last RTT message latency is: " + latency);
                    }
                }
            }
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent ev)
    {
        if (enableRTT)
        {
            if (reqMsg.Attrib().DataType == DataType.DataTypes.ELEMENT_LIST)
            {
                ElementList reqAttributes = reqMsg.Attrib().ElementList();
                foreach (ElementEntry reqAttrib in reqAttributes)
                {
                    if (reqAttrib.Name.Equals(EmaRdm.ENAME_LATENCY))
                    {
                        clientLatencyMap.Add(ev.Handle, 0L);
                        Console.WriteLine("Consumer with handle " + ev.Handle + " supports gathering RTT statistics");
                    }
                }
            }
            ElementList elementList = new ElementList();
            elementList.AddUInt(EmaRdm.ENAME_LATENCY, EmaRdm.LOGIN_RTT_ELEMENT);
            elementList.Complete();
            ev.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name())
                .NameType(EmaRdm.USER_NAME).Complete(true)
                .Solicited(true)
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
                .Attrib(elementList), ev.Handle);
        }
        else
        {
            ev.Provider
                .Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
                .Complete(true)
                .Solicited(true)
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
                    ev.Handle);
        }

    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent ev)
    {
        FieldList fieldList = new FieldList();
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        fieldList.Complete();

        ev.Provider.Submit(new RefreshMsg().Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).Solicited(true)
                        .PrivateStream(reqMsg.PrivateStream())
                        .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
                        .Payload(fieldList)
                        .Complete(true), ev.Handle);



        if (clientItemHandlesMap.ContainsKey(ev.ClientHandle))
        {
            clientItemHandlesMap[ev.ClientHandle].Add(ev.Handle);
        }
        else
        {
            List<long> list = new List<long>();
            list.Add(ev.Handle);
            clientItemHandlesMap.Add(ev.ClientHandle, list);
        }
    }

    void ProcessMarketByPriceRequest(RequestMsg reqMsg, IOmmProviderEvent ev)
    {
        Map map = new Map();
        FieldList summary = new FieldList();
        FieldList entryLoad = new FieldList();

        summary.AddEnumValue(15, 840);
        summary.AddEnumValue(53, 1);
        summary.AddEnumValue(3423, 1);
        summary.AddEnumValue(1709, 2);

        summary.Complete();

        map.SummaryData(summary);

        entryLoad.AddRealFromDouble(3429, 9600);
        entryLoad.AddEnumValue(3428, 2);
        var buf = new EmaBuffer();
        buf.Append((byte)'M').Append((byte)'a').Append((byte)'r').Append((byte)'k').Append((byte)'e').Append((byte)'t');
        entryLoad.AddRmtes(3435, buf);
        entryLoad.Complete();

        map.AddKeyAscii(OrderNr, MapAction.ADD, entryLoad);
        map.Complete();

        ev.Provider.Submit(new RefreshMsg()
            .DomainType(EmaRdm.MMT_MARKET_BY_PRICE)
            .ServiceName(reqMsg.ServiceName())
            .Name(reqMsg.Name())
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .PrivateStream(reqMsg.PrivateStream())
            .Solicited(true)
            .Payload(map)
            .Complete(true), ev.Handle);

        if (clientMBPItemHandlesMap.ContainsKey(ev.ClientHandle))
        {
            clientMBPItemHandlesMap[ev.ClientHandle].Add(ev.Handle);
        }
        else
        {
            List<long> list = new List<long>();
            list.Add(ev.Handle);
            clientMBPItemHandlesMap.Add(ev.ClientHandle, list);
        }
    }

    void ProcessMarketByOrderRequest(RequestMsg reqMsg, IOmmProviderEvent ev)
    {
        FieldList mapSummaryData = new FieldList();
        mapSummaryData.AddEnumValue(15, 840);
        mapSummaryData.AddEnumValue(53, 1);
        mapSummaryData.AddEnumValue(3423, 1);
        mapSummaryData.AddEnumValue(1709, 2);
        mapSummaryData.Complete();

        FieldList entryData = new FieldList();
        entryData.AddRealFromDouble(3427, 7.76, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        entryData.AddRealFromDouble(3429, 9600);
        entryData.AddEnumValue(3428, 2);
        var buf = new EmaBuffer();
        buf.Append((byte)'M').Append((byte)'a').Append((byte)'r').Append((byte)'k').Append((byte)'e').Append((byte)'t');
        entryData.AddRmtes(212, buf);
        entryData.Complete();

        Map map = new Map();
        map.SummaryData(mapSummaryData);

        map.AddKeyAscii(OrderNr, MapAction.ADD, entryData);
        map.Complete();

        ev.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER)
                        .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
                        .Solicited(true).PrivateStream(reqMsg.PrivateStream())
                        .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
                        .Payload(map)
                        .Complete(true),
                        ev.Handle);

        if (clientMBOItemHandlesMap.ContainsKey(ev.ClientHandle))
        {
            clientMBOItemHandlesMap[ev.ClientHandle].Add(ev.Handle);
        }
        else
        {
            List<long> list = new List<long>();
            list.Add(ev.Handle);
            clientMBOItemHandlesMap.Add(ev.ClientHandle, list);
        }
    }

    void ProcessSymbolListRequest(RequestMsg reqMsg, IOmmProviderEvent ev)
    {
        Map mapEnc = new Map();
        FieldList fieldList1 = new FieldList();
        fieldList1.Complete();
        string a;
        for (int i = 0; i < 5; ++i)
        {
            a = "A" + i;
            mapEnc.AddKeyAscii(a, MapAction.ADD, fieldList1);
        }
        mapEnc.Complete();

        ev.Provider.Submit(new RefreshMsg().Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Payload(mapEnc)
            .Complete(true), ev.Handle);

        if (clientSLItemHandlesMap.ContainsKey(ev.ClientHandle))
        {
            clientSLItemHandlesMap[ev.ClientHandle].Add(ev.Handle);
        }
        else
        {
            List<long> list = new List<long>();
            list.Add(ev.Handle);
            clientSLItemHandlesMap.Add(ev.ClientHandle, list);
        }
    }

    public void SendRTTRequests(OmmProvider provider)
    {
        if (!enableRTT)
        {
            Console.WriteLine("This provider does not support RTT");
        }
        else
        {
            ElementList elementList = new ElementList();
            foreach (long handle in clientLatencyMap.Keys)
            {
                elementList.Clear();
                long latency = clientLatencyMap[handle];
                if (latency != 0)
                {
                    elementList.AddUInt(EmaRdm.ENAME_LATENCY, (ulong)latency);
                }
                elementList.AddUInt(EmaRdm.ENAME_TICKS, (ulong)DateTimeOffset.UtcNow.ToUnixTimeMilliseconds());
                elementList.Complete();
                provider.Submit(new GenericMsg().Payload(elementList).DomainType(EmaRdm.MMT_LOGIN).ProviderDriven(true).Complete(true), handle);
            }
        }
    }

    public void SendUpdates(OmmProvider provider, FieldList fieldList)
    {
        foreach (long cl_h in clientItemHandlesMap.Keys)
        {
            List<long> list = clientItemHandlesMap[cl_h];
            foreach (long ih in list)
            {
                provider.Submit(new UpdateMsg().Payload(fieldList), ih);
            }
        }
    }

    public void SendMBPUpdates(OmmProvider provider)
    {
        foreach (long cl_h in clientMBPItemHandlesMap.Keys)
        {
            List<long> list = clientMBPItemHandlesMap[cl_h];
            foreach (long ih in list)
            {
                mbpentryLoad.Clear();
                mbpentryLoad.AddRealFromDouble(3427, 7.76 + i++ * 0.1, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                mbpentryLoad.AddRealFromDouble(3429, 9600);
                mbpentryLoad.AddEnumValue(3428, 2);
                var buf = new EmaBuffer();
                buf.Append((byte)'M').Append((byte)'a').Append((byte)'r').Append((byte)'k').Append((byte)'e').Append((byte)'t');
                mbpentryLoad.AddRmtes(3435, buf);
                mbpentryLoad.Complete();

                mbpmap.Clear();
                mbpmap.AddKeyAscii(OrderNr, MapAction.UPDATE, mbpentryLoad);
                mbpmap.Complete();

                provider.Submit(mbpupdateMsg.Clear().DomainType(EmaRdm.MMT_MARKET_BY_PRICE).Payload(mbpmap), ih);
            }
        }
    }

    public void SendMBOUpdates(OmmProvider provider)
    {
        foreach (long cl_h in clientMBPItemHandlesMap.Keys)
        {
            List<long> list = clientMBPItemHandlesMap[cl_h];
            foreach (long ih in list)
            {
                mbofieldList.AddRealFromDouble(3427, 7.76 + i++ * 0.1, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                mbofieldList.AddRealFromDouble(3429, 9600);
                mbofieldList.AddEnumValue(3428, 2);
                var buf = new EmaBuffer();
                buf.Append((byte)'M').Append((byte)'a').Append((byte)'r').Append((byte)'k').Append((byte)'e').Append((byte)'t');
                mbofieldList.AddRmtes(212, buf);
                mbofieldList.Complete();

                mbomap.AddKeyAscii(OrderNr, MapAction.ADD, mbofieldList);
                mbomap.Complete();

                provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).Payload(mbomap), ih);

                mbomap.Clear();
                mbofieldList.Clear();
            }
        }
    }

    public bool StandBy()
    {
        return clientItemHandlesMap.Count == 0 && clientMBPItemHandlesMap.Count == 0 && clientMBOItemHandlesMap.Count == 0;
    }
}

public class IProvider
{
    static long DELTA = 1000;
    static string providerName = null;
    static bool rtt = false;

    public static void PrintHelp()
    {
        Console.WriteLine("\nOptions:\n" + "  -?\tShows this usage\n" + "  -n  \tProvider name from xml config\n + \"  -rtt  \tSpecifies whether the povider supports rtt\n"
                + "\n");

        Environment.Exit(-1);
    }
    public static bool ReadCommandlineArgs(string[] argv)
    {
        int count = argv.Length;
        int idx = 0;

        while (idx < count)
        {
            if ("-?".Equals(argv[idx]))
            {
                PrintHelp();
                return false;
            }
            else if ("-n".Equals(argv[idx]))
            {
                if (++idx >= count)
                {
                    PrintHelp();
                    return false;
                }
                providerName = argv[idx];
                ++idx;
            }
            else if ("-rtt".Equals(argv[idx]))
            {
                rtt = true;
                ++idx;
            }
            else
            {
                Console.WriteLine("Found some other arg: " + argv[idx]);
                PrintHelp();
                return false;
            }
        }
        return true;
    }

    public static void Main(string[] args)
    {

        long nextRequestTime = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds() + DELTA;
        OmmProvider provider = null;

        ReadCommandlineArgs(args);

        try
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            AppClient appClient = new AppClient(rtt);

            if (providerName != null) config.ProviderName(providerName);

            provider = new OmmProvider(config.OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH), appClient);
            FieldList fieldList = new FieldList();

            Random rnd = new Random();
            while (appClient.StandBy())
            {
                provider.Dispatch(500);
                Thread.Sleep(500);
            }

            for (int i = 0; i < 6000; i++)
            {
                provider.Dispatch(50);

                if (DateTimeOffset.UtcNow.ToUnixTimeMilliseconds() >= nextRequestTime)
                {

                    appClient.SendRTTRequests(provider);
                    fieldList.Clear();
                    fieldList.AddReal(22, 3991 + rnd.NextInt64(), OmmReal.MagnitudeTypes.EXPONENT_NEG_2);

                    appClient.SendUpdates(provider, fieldList);

                    appClient.SendMBOUpdates(provider);
                    appClient.SendMBPUpdates(provider);

                    nextRequestTime = DateTimeOffset.UtcNow.ToUnixTimeMilliseconds() + DELTA;
                }

                Thread.Sleep(100);
            }
        }
        catch (Exception excp)
        {
            Console.WriteLine("Usage: ");
            Console.WriteLine(excp.Message);
        }
        finally
        {
            if (provider != null) provider.Uninitialize();
        }
    }
}
