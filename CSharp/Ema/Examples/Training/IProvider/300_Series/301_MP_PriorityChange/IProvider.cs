/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;
    int priorityCount = 0;
    int priorityClass = 0;

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

    public void OnReissue(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
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
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN).Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME)
            .Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
    }

    void ProcessMarketPriceRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (reqMsg.InitialImage())
        {
            FieldList fieldList = new FieldList();
            fieldList.AddAscii(3, reqMsg.Name());
            fieldList.AddEnumValue(15, 840);
            fieldList.AddReal(21, 3900, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(22, 3990, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(25, 3994, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
            fieldList.AddReal(30, 9, OmmReal.MagnitudeTypes.EXPONENT_0);
            fieldList.AddReal(31, 19, OmmReal.MagnitudeTypes.EXPONENT_0);

            providerEvent.Provider.Submit(new RefreshMsg().Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName()).Solicited(true)
                .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Refresh Completed")
                .Payload(fieldList.Complete()).Complete(true),
                providerEvent.Handle);

            ItemHandle = providerEvent.Handle;
        }

        if (reqMsg.HasPriority)
        {
            priorityCount = reqMsg.PriorityCount();
            priorityClass = reqMsg.PriorityClass();

            Console.WriteLine($"Received request message with priority info, priority count = {priorityCount}, priority class = {priorityClass}");
        }
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
            AppClient appClient = new AppClient();
            provider = new OmmProvider(new OmmIProviderConfig().OperationModel(OmmIProviderConfig.OperationModelMode.USER_DISPATCH), appClient);

            while (appClient.ItemHandle == 0) provider.Dispatch(1000);

            FieldList fieldList = new FieldList();
            for (int i = 0; i < 60; i++)
            {
                provider.Dispatch(1000000);

                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(25, 3994 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);
                fieldList.AddReal(31, 19 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().Payload(fieldList.Complete()), appClient.ItemHandle);
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
