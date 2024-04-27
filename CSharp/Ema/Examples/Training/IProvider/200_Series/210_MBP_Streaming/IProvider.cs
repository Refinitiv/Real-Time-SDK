/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
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
            case EmaRdm.MMT_MARKET_BY_PRICE:
                ProcessMarketByPriceRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
    }

    void ProcessMarketByPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle != 0)
        {
            ProcessInvalidItemRequest(reqMsg, providerEvent);
            return;
        }

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
        entryLoad.AddRmtes(3435, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

        map.AddKeyAscii(OrderNr, MapAction.ADD, entryLoad.Complete());

        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_PRICE)
            .ServiceName(reqMsg.ServiceName()).Name(reqMsg.Name())
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Solicited(true).Payload(map.Complete()).Complete(true),
            providerEvent.Handle);

        ItemHandle = providerEvent.Handle;
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().DomainType(reqMsg.DomainType())
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
            FieldList entryLoad = new FieldList();
            UpdateMsg updateMsg = new UpdateMsg();
            Map map = new Map();

            provider = new OmmProvider(new OmmIProviderConfig().OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH), appClient);

            while (appClient.ItemHandle == 0)
            {
                provider.Dispatch(1000);
                Thread.Sleep(1000);
            }

            for (int i = 0; i < 60; i++)
            {
                provider.Dispatch(1000);

                entryLoad.Clear();
                entryLoad.AddRealFromDouble(3427, 7.76 + i * 0.1, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                entryLoad.AddRealFromDouble(3429, 9600);
                entryLoad.AddEnumValue(3428, 2);
                entryLoad.AddRmtes(3435, new EmaBuffer(Encoding.ASCII.GetBytes("Market Maker")));

                map.Clear();
                map.AddKeyAscii(appClient.OrderNr, MapAction.UPDATE, entryLoad.Complete());

                provider.Submit(updateMsg.Clear().DomainType(EmaRdm.MMT_MARKET_BY_PRICE).Payload(map.Complete()),
                    appClient.ItemHandle);

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
