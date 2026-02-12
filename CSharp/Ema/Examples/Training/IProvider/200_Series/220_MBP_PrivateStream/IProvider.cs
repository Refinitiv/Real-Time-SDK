/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.Md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public string OrderNr = "100";
    public Dictionary<long, bool> ItemHandles = new Dictionary<long, bool>();

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
        Map map = new Map();
        FieldList summary = new FieldList();
        FieldList entryLoad = new FieldList();

        summary.Clear();
        summary.AddRealFromDouble(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        summary.AddRealFromDouble(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        summary.AddRealFromDouble(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        summary.AddRealFromDouble(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        map.SummaryData(summary.Complete());

        entryLoad.Clear();
        entryLoad.AddRealFromDouble(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        entryLoad.AddRealFromDouble(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        entryLoad.AddRealFromDouble(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        entryLoad.AddRealFromDouble(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        map.AddKeyAscii(OrderNr, MapAction.ADD, entryLoad.Complete());

        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_MARKET_BY_PRICE)
            .ServiceName(reqMsg.ServiceName()).Name(reqMsg.Name())
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Solicited(true).Payload(map.Complete()).PrivateStream(reqMsg.PrivateStream()).Complete(true),
            providerEvent.Handle);

        ItemHandles.Add(providerEvent.Handle, reqMsg.PrivateStream());
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
        try
        {
            AppClient appClient = new AppClient();
            FieldList summary = new FieldList();
            FieldList entryLoad = new FieldList();
            UpdateMsg updateMsg = new UpdateMsg();
            List<long> removeHandles = new List<long>();
            Map map = new Map();

            using OmmProvider provider = new OmmProvider(new OmmIProviderConfig().OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH), appClient);

            while (appClient.ItemHandles.Count == 0)
            {
                provider.Dispatch(1000);
                Thread.Sleep(1000);
            }

            for (int i = 0; i < 60; i++)
            {
                provider.Dispatch(1000);

                summary.Clear();
                summary.AddRealFromDouble(22, 3990 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                summary.AddRealFromDouble(25, 3994 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                summary.AddRealFromDouble(30, 9 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                summary.AddRealFromDouble(31, 19 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                map.Clear();
                map.SummaryData(summary.Complete());

                entryLoad.Clear();
                entryLoad.AddRealFromDouble(22, 3990 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                entryLoad.AddRealFromDouble(25, 3994 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                entryLoad.AddRealFromDouble(30, 9 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                entryLoad.AddRealFromDouble(31, 19 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                map.AddKeyAscii(appClient.OrderNr, MapAction.UPDATE, entryLoad.Complete());
                map.Complete();
                removeHandles.Clear();

                foreach (long handle in appClient.ItemHandles.Keys)
                {
                    try
                    {
                        provider.Submit(updateMsg.Clear().DomainType(EmaRdm.MMT_MARKET_BY_PRICE).Payload(map),
                            handle);
                    }
                    catch (OmmException excp)
                    {
                        Console.WriteLine(excp.Message);
                        removeHandles.Add(handle);
                        continue;
                    }
                }
                while (removeHandles.Count > 0)
                {
                    appClient.ItemHandles.Remove(removeHandles[0]);
                    removeHandles.Remove(removeHandles[0]);
                }

                

                Thread.Sleep(1000);
            }
        }
        catch (OmmException excp)
        {
            Console.WriteLine(excp.Message);
        }
    }
}
