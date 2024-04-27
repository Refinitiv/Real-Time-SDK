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
    public EmaBuffer groupId = new EmaBuffer(Encoding.ASCII.GetBytes("10"));

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

    public void ProcessUpdateGroupStatus(OmmProvider provider)
    {
        ElementList serviceGroupId = new ElementList();
        serviceGroupId.AddBuffer(EmaRdm.ENAME_GROUP, groupId);
        serviceGroupId.AddState(EmaRdm.ENAME_STATUS,
                OmmState.StreamStates.CLOSED_RECOVER, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Group Status Msg");

        FilterList filterList = new FilterList();
        filterList.AddEntry(EmaRdm.SERVICE_GROUP_ID, FilterAction.SET, serviceGroupId.Complete());

        Map map = new Map();
        map.AddKeyUInt(1, MapAction.UPDATE, filterList.Complete());

        provider.Submit(new UpdateMsg().DomainType(EmaRdm.MMT_DIRECTORY)
            .Filter(EmaRdm.SERVICE_GROUP_FILTER).Payload(map.Complete()),
            0);	// use 0 item handle to fanout to all subscribers
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

        providerEvent.Provider.Submit(new RefreshMsg().ItemGroup(groupId)
            .Name(reqMsg.Name()).ServiceName(reqMsg.ServiceName()).Solicited(true)
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
            AppClient appClient = new AppClient();
            FieldList fieldList = new FieldList();

            OmmIProviderConfig config = new OmmIProviderConfig();

            provider = new OmmProvider(config, appClient);

            while (appClient.ItemHandle == 0) Thread.Sleep(1000);

            for (int i = 0; i < 10; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().Payload(fieldList.Complete()), appClient.ItemHandle);

                Thread.Sleep(1000);
            }

            appClient.ProcessUpdateGroupStatus(provider);

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
