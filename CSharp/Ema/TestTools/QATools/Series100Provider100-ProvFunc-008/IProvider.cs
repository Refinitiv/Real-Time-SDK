/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Collections.Generic;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;

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
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (ItemHandle != 0)
        {
            ProcessInvalidItemRequest(reqMsg, providerEvent);
            return;
        }

        FieldList fieldList = new FieldList();
        fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
        fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
        fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

        providerEvent.Provider.Submit(new RefreshMsg().Name(reqMsg.Name()).ServiceId(reqMsg.ServiceId()).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
            .Payload(fieldList.Complete()).Complete(true),
            providerEvent.Handle);

        ItemHandle = providerEvent.Handle;
    }

    void ProcessInvalidItemRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        providerEvent.Provider.Submit(new StatusMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName())
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
            //APIQA
            // Provider only publishes for 2 seconds
            for (int i = 0; i < 1_000_000; i++)
            {
                AppClient appClient = new AppClient();
                FieldList fieldList = new FieldList();

                OmmIProviderConfig config = new OmmIProviderConfig();

                Console.WriteLine("!!! createOmmProvider() " + i + " !!!");
                provider = new OmmProvider(config.Port("14002"), appClient);

                while (appClient.ItemHandle == 0)
                {
                    Thread.Sleep(1000);
                }

                for (int j = 0; j < 2; j++)
                {
                    fieldList.Clear();
                    fieldList.AddReal(22, 3991 + j, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                    fieldList.AddReal(30, 10 + j, OmmReal.MagnitudeTypes.EXPONENT_0);

                    provider.Submit(new UpdateMsg().Payload(fieldList.Complete()), appClient.ItemHandle);

                    Thread.Sleep(1000);
                }
                Console.WriteLine("!!! provider.uninitialize() " + i + " !!!");
                provider.Uninitialize();
            }
            //END APIQA
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
