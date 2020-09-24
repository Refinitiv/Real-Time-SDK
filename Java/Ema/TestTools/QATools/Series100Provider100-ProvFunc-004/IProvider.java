///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.rtsdk.ema.examples.training.iprovider.series100.ex100_MP_Streaming;

import com.rtsdk.ema.access.ElementList;
import com.rtsdk.ema.access.EmaFactory;
import com.rtsdk.ema.access.FieldList;
import com.rtsdk.ema.access.FilterEntry;
import com.rtsdk.ema.access.FilterList;
import com.rtsdk.ema.access.GenericMsg;
import com.rtsdk.ema.access.Map;
import com.rtsdk.ema.access.MapEntry;
import com.rtsdk.ema.access.Msg;
import com.rtsdk.ema.access.OmmArray;
import com.rtsdk.ema.access.OmmException;
import com.rtsdk.ema.access.OmmIProviderConfig;
import com.rtsdk.ema.access.OmmProvider;
import com.rtsdk.ema.access.OmmProviderClient;
import com.rtsdk.ema.access.OmmProviderEvent;
import com.rtsdk.ema.access.OmmReal;
import com.rtsdk.ema.access.OmmState;
import com.rtsdk.ema.access.PostMsg;
import com.rtsdk.ema.access.RefreshMsg;
import com.rtsdk.ema.access.ReqMsg;
import com.rtsdk.ema.access.StatusMsg;
import com.rtsdk.ema.rdm.EmaRdm;

//APIQA
class AppClient implements OmmProviderClient
{
    public long itemHandle = 0;
    public long loginHandle = 0;
    public boolean acceptLoginReq = true;

    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
    {
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_LOGIN:
                processLoginRequest(reqMsg, event);
                break;
            case EmaRdm.MMT_DIRECTORY:
                processDirectoryRequest(reqMsg, event);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                processMarketPriceRequest(reqMsg, event);
                break;
            default:
                processInvalidItemRequest(reqMsg, event);
                break;
        }
    }

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
    {
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event)
    {
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event)
    {
    }

    public void onPostMsg(PostMsg postMsg, OmmProviderEvent event)
    {
    }

    public void onReissue(ReqMsg reqMsg, OmmProviderEvent event)
    {
    }

    public void onClose(ReqMsg reqMsg, OmmProviderEvent event)
    {
        System.out.println("handle = " + event.handle());
        System.out.println(reqMsg);
        System.out.println("InitialImage " + reqMsg.initialImage());
        System.out.println("InterestAfterRefresh " + reqMsg.interestAfterRefresh());
    }

    public void onAllMsg(Msg msg, OmmProviderEvent event)
    {
    }

    void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        if (!acceptLoginReq)
            return;

        loginHandle = event.handle();
        event.provider().submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).nameType(EmaRdm.USER_NAME).complete(true).solicited(true)
                                        .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"), event.handle());
    }

    void processDirectoryRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        OmmArray capablities = EmaFactory.createOmmArray();
        capablities.add(EmaFactory.createOmmArrayEntry().uintValue(EmaRdm.MMT_MARKET_PRICE));
        capablities.add(EmaFactory.createOmmArrayEntry().uintValue(EmaRdm.MMT_MARKET_BY_PRICE));
        OmmArray dictionaryUsed = EmaFactory.createOmmArray();
        dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii("RWFFld"));
        dictionaryUsed.add(EmaFactory.createOmmArrayEntry().ascii("RWFEnum"));

        ElementList serviceInfoId = EmaFactory.createElementList();

        serviceInfoId.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED"));
        serviceInfoId.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities));
        serviceInfoId.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed));

        ElementList serviceStateId = EmaFactory.createElementList();
        serviceStateId.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));

        FilterList filterList = EmaFactory.createFilterList();
        filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId));
        filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId));

        Map map = EmaFactory.createMap();
        map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.ADD, filterList));

        OmmArray capablities2 = EmaFactory.createOmmArray();
        capablities2.add(EmaFactory.createOmmArrayEntry().uintValue(EmaRdm.MMT_MARKET_PRICE));
        capablities2.add(EmaFactory.createOmmArrayEntry().uintValue(EmaRdm.MMT_MARKET_BY_PRICE));
        OmmArray dictionaryUsed2 = EmaFactory.createOmmArray();
        dictionaryUsed2.add(EmaFactory.createOmmArrayEntry().ascii("RWFFld"));
        dictionaryUsed2.add(EmaFactory.createOmmArrayEntry().ascii("RWFEnum"));

        ElementList serviceInfoId2 = EmaFactory.createElementList();

        serviceInfoId2.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "DIRECT_FEED2"));
        serviceInfoId2.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capablities2));
        serviceInfoId2.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_DICTIONARYS_USED, dictionaryUsed2));

        ElementList serviceStateId2 = EmaFactory.createElementList();
        serviceStateId2.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
        serviceStateId2.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ACCEPTING_REQS, EmaRdm.SERVICE_DOWN));

        FilterList filterList2 = EmaFactory.createFilterList();
        filterList2.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.SET, serviceInfoId2));
        filterList2.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.SET, serviceStateId2));

        map.add(EmaFactory.createMapEntry().keyUInt(2, MapEntry.MapAction.ADD, filterList2));

        RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
        event.provider().submit(refreshMsg.domainType(EmaRdm.MMT_DIRECTORY).clearCache(true).filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER).payload(map).solicited(true)
                                        .complete(true), event.handle());
    }

    void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        if (itemHandle != 0)
        {
            processInvalidItemRequest(reqMsg, event);
            return;
        }

        FieldList fieldList = EmaFactory.createFieldList();
        fieldList.add(EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add(EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add(EmaFactory.createFieldEntry().real(30, 9, OmmReal.MagnitudeType.EXPONENT_0));
        fieldList.add(EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

        event.provider().submit(EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true)
                                        .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").payload(fieldList).complete(true), event.handle());

        itemHandle = event.handle();
    }

    void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        event.provider().submit(EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName())
                                        .state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_FOUND, "Item not found"), event.handle());
    }
}

public class IProvider
{
    public static void main(String[] args)
    {
        boolean sendCloseStatus = false;
        boolean sendLoginCloseStatus = false;

        AppClient appClient = new AppClient();

        if (args.length == 1)
        {
            if (args[0].equals("-sendCloseStatus"))
            {
                sendCloseStatus = true;
            }
            else if (args[0].equals("-sendLoginCloseStatus"))
            {
                sendLoginCloseStatus = true;
            }
            else if (args[0].equals("-notAcceptLoginReq"))
            {
                appClient.acceptLoginReq = false;
            }
        }

        OmmProvider provider = null;
        try
        {
            provider = EmaFactory.createOmmProvider(EmaFactory.createOmmIProviderConfig().adminControlDirectory(OmmIProviderConfig.AdminControl.USER_CONTROL), appClient);

            while (appClient.itemHandle == 0)
                Thread.sleep(1000);

            FieldList fieldList = EmaFactory.createFieldList();
            for (int i = 0; i < 60; i++)
            {
                fieldList.clear();
                fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));

                provider.submit(EmaFactory.createUpdateMsg().payload(fieldList), appClient.itemHandle);

                Thread.sleep(1000);

                if (sendCloseStatus)
                {
                    provider.submit(EmaFactory.createStatusMsg().state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Close item from provider."),
                                    appClient.itemHandle);
                    break;

                }
                else if (sendLoginCloseStatus)
                {
                    provider.submit(EmaFactory.createStatusMsg().domainType(EmaRdm.MMT_LOGIN)
                                            .state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Close login from provider."), appClient.loginHandle);
                    break;
                }

            }
        }
        catch (InterruptedException | OmmException excp)
        {
            System.out.println(excp.getMessage());
        }
        finally
        {
            if (provider != null)
                provider.uninitialize();
        }
    }
}
//END APIQA
