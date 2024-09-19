/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2024 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;
    public string OrderNr = "100";

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_BY_ORDER:
                ProcessMarketByOrderRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
            .Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
    }

    void ProcessMarketByOrderRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle != 0)
        {
            ProcessInvalidItemRequest(reqMsg, providerEvent);
            return;
        }

        FieldList mapSummaryData = new FieldList();
        mapSummaryData.AddEnumValue(15, 840);
        mapSummaryData.AddEnumValue(53, 1);
        mapSummaryData.AddEnumValue(3423, 1);
        mapSummaryData.AddEnumValue(1709, 2);

        FieldList entryData = new FieldList();
        entryData.AddRealFromDouble(3427, 7.76, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        entryData.AddRealFromDouble(3429, 9600);
        entryData.AddEnumValue(3428, 2);
        entryData.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

        Map map = new Map();
        map.SummaryData(mapSummaryData.Complete());

        map.AddKeyAscii(OrderNr, Access.MapAction.ADD, entryData.Complete());

        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER)
            .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName()).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Payload(map.Complete()).Complete(true),
            providerEvent.Handle);

        ItemHandle = providerEvent.Handle;
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg()
            .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
            .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NOT_FOUND, "Item not found"),
            providerEvent.Handle);
    }
}

public class IProvider
{
    public static void Main(string[] args)
    {
        OmmProvider? provider = null;
        try
        {
            AppClient appClient = new AppClient();
            FieldList fieldList = new FieldList();
            Map map = new Map();

            OmmIProviderConfig config = new OmmIProviderConfig();

            provider = new OmmProvider(config.Port("14002"), appClient);

            while (appClient.ItemHandle == 0)
            {
                Thread.Sleep(1000);
            }

            for (int i = 0; i < 60; i++)
            {
                fieldList.AddRealFromDouble(3427, 7.76 + i * 0.1, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddRealFromDouble(3429, 9600);
                fieldList.AddEnumValue(3428, 2);
                fieldList.AddRmtes(212, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

                map.AddKeyAscii(appClient.OrderNr, Access.MapAction.ADD, fieldList.Complete());

                provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER).Payload(map.Complete()), appClient.ItemHandle);

                map.Clear();
                fieldList.Clear();

                Thread.Sleep(1000);
            }

            Thread.Sleep(60_000);
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
