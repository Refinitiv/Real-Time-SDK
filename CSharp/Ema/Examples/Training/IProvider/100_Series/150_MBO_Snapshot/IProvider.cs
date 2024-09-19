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
            case EmaRdm.MMT_DIRECTORY:
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

        String OrderNr = "100";
        map.AddKeyAscii(OrderNr, MapAction.ADD, entryLoad.Complete());

        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_ORDER)
            .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName()).Solicited(true)
            .State(OmmState.StreamStates.NON_STREAMING, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Payload(map.Complete()).Complete(true),
            providerEvent.Handle);
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

            OmmIProviderConfig config = new OmmIProviderConfig();

            provider = new OmmProvider(config.Port("14002"), appClient);

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
