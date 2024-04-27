/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    private bool enableRTT;

    // holds the last latency value for each client that supports RTT requests
    // clientLatencyMap.keySet() holds handles to all consumers that can receive RTT requests
    public Dictionary<long, ulong> clientLatencyMap = new();

    // holds all item requests for each client
    public Dictionary<long, List<long>> clientItemHandlesMap = new();

    public AppClient(bool rtt)
    {
        enableRTT = rtt;
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
                break;
        }
    }

    public void OnGenericMsg(GenericMsg genericMsg, IOmmProviderEvent providerEvent)
    {
        switch (genericMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessRTT(genericMsg, providerEvent);
                break;
            default:
                break;
        }
    }

    public void OnClose(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                clientLatencyMap.Remove(providerEvent.Handle);
                clientItemHandlesMap.Remove(providerEvent.ClientHandle);
                break;

            case EmaRdm.MMT_MARKET_PRICE:
                List<long> list = clientItemHandlesMap[providerEvent.ClientHandle];
                list.Remove(providerEvent.Handle);

                if (list.Count == 0)
                {
                    clientItemHandlesMap.Remove(providerEvent.ClientHandle);
                }
                break;

            default:
                break;
        }
    }

    void ProcessRTT(GenericMsg genericMsg, IOmmProviderEvent providerEvent)
    {
        if (clientLatencyMap.ContainsKey(providerEvent.Handle))
        {
            if (genericMsg.Payload().DataType == DataType.DataTypes.ELEMENT_LIST
                && genericMsg.DomainType() == EmaRdm.MMT_LOGIN)
            {
                Console.WriteLine("Received login RTT message from Consumer " + providerEvent.Handle);
                ElementList data = genericMsg.Payload().ElementList();

                foreach (var elem in data)
                {
                    if (elem.Name.Equals(EmaRdm.ENAME_TICKS))
                    {
                        Console.WriteLine("        RTT Tick value is: " + elem.UIntValue());
                        ulong latency = GetNanoseconds() - elem.UIntValue();
                        clientLatencyMap[providerEvent.Handle] = latency;
                        Console.WriteLine("        Last RTT message latency is: " + latency);
                    }
                }
            }
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
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
                        clientLatencyMap[providerEvent.Handle] = 0L;
                        Console.WriteLine($"Consumer with handle {providerEvent.Handle} supports gathering RTT statistics");
                    }
                }
            }

            ElementList elementList = new ElementList();
            elementList.AddUInt(EmaRdm.ENAME_LATENCY, EmaRdm.LOGIN_RTT_ELEMENT);

            providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name())
                .NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
                .Attrib(elementList.Complete()),
                providerEvent.Handle);

        }
        else
        {
            providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name())
                .NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted")
                .Attrib(new ElementList().Complete()),
                providerEvent.Handle);
        }

    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        FieldList fieldList = new FieldList();
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        providerEvent.Provider.Submit(new RefreshMsg().Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);

        if (clientItemHandlesMap.ContainsKey(providerEvent.ClientHandle))
        {
            clientItemHandlesMap[providerEvent.ClientHandle].Add(providerEvent.Handle);
        }
        else
        {
            List<long> list = new()
            {
                providerEvent.Handle
            };
            clientItemHandlesMap[providerEvent.ClientHandle] = list;
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
            foreach (var handle in clientLatencyMap.Keys)
            {
                elementList.Clear();
                ulong latency = clientLatencyMap[handle];
                if (latency != 0)
                {
                    elementList.AddUInt(EmaRdm.ENAME_LATENCY, latency);
                }
                elementList.AddUInt(EmaRdm.ENAME_TICKS, GetNanoseconds());
                provider.Submit(new GenericMsg().Payload(elementList.Complete()).DomainType(EmaRdm.MMT_LOGIN)
                    .ProviderDriven(true).Complete(true),
                    handle);
            }
        }
    }

    public void SendUpdates(OmmProvider provider, FieldList fieldList)
    {
        foreach (var cl_h in clientItemHandlesMap.Keys)
        {
            List<long> list = clientItemHandlesMap[cl_h];
            foreach (var ih in list)
            {
                provider.Submit(new UpdateMsg().Payload(fieldList), ih);
            }
        }
    }

    public bool StandBy()
    {
        return clientItemHandlesMap.Count == 0;
    }

    private double TicksPerNanosecond = Stopwatch.Frequency / 1_000_000_000.0;

    private ulong GetNanoseconds()
    {
        return (ulong)(Stopwatch.GetTimestamp() / TicksPerNanosecond);
    }
}

public class IProvider
{
    static TimeSpan DELTA = TimeSpan.FromMilliseconds(1000);

    public static void Main(string[] args)
    {

        DateTime nextRequestTime = DateTime.Now + DELTA;
        OmmProvider? provider = null;

        try
        {
            OmmIProviderConfig config = new OmmIProviderConfig();
            AppClient appClient = new AppClient(true);

            provider = new OmmProvider(config.OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH).Port("14002"),
                appClient);
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

                if (DateTime.Now >= nextRequestTime)
                {

                    appClient.SendRTTRequests(provider);
                    fieldList.Clear();
                    fieldList.AddReal(22, 3991 + rnd.Next(50), OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                    fieldList.AddReal(30, 10 + rnd.Next(50), OmmReal.MagnitudeTypes.EXPONENT_0);

                    appClient.SendUpdates(provider, fieldList.Complete());

                    nextRequestTime = DateTime.Now + DELTA;
                }

                Thread.Sleep(50);
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
