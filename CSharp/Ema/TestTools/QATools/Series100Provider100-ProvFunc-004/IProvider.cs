/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.Md for details.                  --
 *|           Copyright (C) 2024 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

using System;
using System.Threading;

using LSEG.Ema.Access;
using LSEG.Ema.Rdm;

namespace LSEG.Ema.Example.Traning.IProvider;

//APIQA
class AppClient : IOmmProviderClient
{
    public long ItemHandle = 0;
    public long LoginHandle = 0;
    public bool acceptLoginReq = true;

    public void OnReqMsg(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        switch (reqMsg.DomainType())
        {
            case EmaRdm.MMT_LOGIN:
                ProcessLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_DIRECTORY:
                ProcessDirectoryRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                ProcessMarketPriceRequest(reqMsg, providerEvent);
                break;
            default:
                ProcessInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    public void OnClose(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        Console.WriteLine("handle = " + providerEvent.Handle);
        Console.WriteLine(reqMsg);
        Console.WriteLine("InitialImage " + reqMsg.InitialImage());
        Console.WriteLine("InterestAfterRefresh " + reqMsg.InterestAfterRefresh());
    }

    void ProcessLoginRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        if (!acceptLoginReq)
            return;

        LoginHandle = providerEvent.Handle;
        providerEvent.Provider.Submit(new RefreshMsg().DomainType(EmaRdm.MMT_LOGIN)
            .Name(reqMsg.Name()).NameType(EmaRdm.USER_NAME).Complete(true).Solicited(true)
            .State(OmmState.StreamStates.OPEN, OmmState.DataStates.OK, OmmState.StatusCodes.NONE, "Login accepted"),
            providerEvent.Handle);
    }

    void ProcessDirectoryRequest(RequestMsg reqMsg, IOmmProviderEvent providerEvent)
    {
        OmmArray capablities = new OmmArray();
        capablities.AddUInt(EmaRdm.MMT_MARKET_PRICE);
        capablities.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);

        OmmArray dictionaryUsed = new OmmArray();
        dictionaryUsed.AddAscii("RWFFld");
        dictionaryUsed.AddAscii("RWFEnum");

        ElementList serviceInfoId = new ElementList();
        serviceInfoId.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED");
        serviceInfoId.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities.Complete());
        serviceInfoId.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed.Complete());

        ElementList serviceStateId = new ElementList();
        serviceStateId.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);

        FilterList filterList = new FilterList();
        filterList.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId.Complete());
        filterList.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId.Complete());

        Map map = new Map();
        map.AddKeyUInt(1, MapAction.ADD, filterList.Complete());

        OmmArray capablities2 = new OmmArray();
        capablities2.AddUInt(EmaRdm.MMT_MARKET_PRICE);
        capablities2.AddUInt(EmaRdm.MMT_MARKET_BY_PRICE);

        OmmArray dictionaryUsed2 = new OmmArray();
        dictionaryUsed2.AddAscii("RWFFld");
        dictionaryUsed2.AddAscii("RWFEnum");

        ElementList serviceInfoId2 = new ElementList();

        serviceInfoId2.AddAscii(EmaRdm.ENAME_NAME, "DIRECT_FEED2");
        serviceInfoId2.AddArray(EmaRdm.ENAME_CAPABILITIES, capablities2.Complete());
        serviceInfoId2.AddArray(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed2.Complete());

        ElementList serviceStateId2 = new ElementList();
        serviceStateId2.AddUInt(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP);
        serviceStateId2.AddUInt(EmaRdm.ENAME_ACCEPTING_REQS, EmaRdm.SERVICE_DOWN);

        FilterList filterList2 = new FilterList();
        filterList2.AddEntry(EmaRdm.SERVICE_INFO_ID, FilterAction.SET, serviceInfoId2.Complete());
        filterList2.AddEntry(EmaRdm.SERVICE_STATE_ID, FilterAction.SET, serviceStateId2.Complete());

        map.AddKeyUInt(2, MapAction.ADD, filterList2.Complete());

        RefreshMsg refreshMsg = new RefreshMsg();
        providerEvent.Provider.Submit(refreshMsg.DomainType(EmaRdm.MMT_DIRECTORY)
            .ClearCache(true).Filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER)
            .Payload(map.Complete()).Solicited(true).Complete(true),
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
        bool sendCloseStatus = false;
        bool sendLoginCloseStatus = false;

        AppClient appClient = new AppClient();

        if (args.Length == 1)
        {
            if (args[0].Equals("-sendCloseStatus"))
            {
                sendCloseStatus = true;
            }
            else if (args[0].Equals("-sendLoginCloseStatus"))
            {
                sendLoginCloseStatus = true;
            }
            else if (args[0].Equals("-notAcceptLoginReq"))
            {
                appClient.acceptLoginReq = false;
            }
        }

        OmmProvider? provider = null;
        try
        {
            provider = new OmmProvider(new OmmIProviderConfig().AdminControlDirectory(OmmIProviderConfig.AdminControlMode.USER_CONTROL), appClient);

            while (appClient.ItemHandle == 0)
            {
                Thread.Sleep(1000);
            }

            FieldList fieldList = new FieldList();
            for (int i = 0; i < 60; i++)
            {
                fieldList.Clear();
                fieldList.AddReal(22, 3991 + i, OmmReal.MagnitudeTypes.EXPONENT_NEG_2);
                fieldList.AddReal(30, 10 + i, OmmReal.MagnitudeTypes.EXPONENT_0);

                provider.Submit(new UpdateMsg().Payload(fieldList.Complete()), appClient.ItemHandle);

                Thread.Sleep(1000);

                if (sendCloseStatus)
                {
                    provider.Submit(new StatusMsg()
                        .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Close item from provider."),
                        appClient.ItemHandle);
                    break;

                }
                else if (sendLoginCloseStatus)
                {
                    provider.Submit(new StatusMsg().DomainType(EmaRdm.MMT_LOGIN)
                        .State(OmmState.StreamStates.CLOSED, OmmState.DataStates.SUSPECT, OmmState.StatusCodes.NONE, "Close login from provider."),
                        appClient.LoginHandle);
                    break;
                }

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
//END APIQA
