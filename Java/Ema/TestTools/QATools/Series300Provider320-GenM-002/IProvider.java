/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.refinitiv.ema.examples.training.iprovider.series300.ex320_Custom_GenericMsg;

import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmIProviderConfig.OperationModel;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
    public long itemHandle = 0;
    long count = 1;
    public static final int APP_DOMAIN = 200;

    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
    {
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_LOGIN:
                processLoginRequest(reqMsg, event);
                break;
            // APIQA
            case EmaRdm.MMT_MARKET_PRICE:
                processMarketPriceRequest(reqMsg, event);
                break;
            // END APIQA
            case APP_DOMAIN:
                processAppDomainRequest(reqMsg, event);
                break;
            default:
                System.out.println("Received invalid Request msg. Item Handle: " + event.handle() + " Closure: " + event.closure());
                break;
        }
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event)
    {
        switch (genericMsg.domainType())
        {
            // APIQA
            case EmaRdm.MMT_LOGIN:
                processLoginDomainGenericMsg(genericMsg, event);
                break;
            case EmaRdm.MMT_DIRECTORY:
                processDirectoryDomainGenericMsg(genericMsg, event);
                break;
            // END APIQA
            case APP_DOMAIN:
                processAppDomainGenericMsg(genericMsg, event);
                break;
            default:
                System.out.println("Received invalid Generic msg. Item Handle: " + event.handle() + " Closure: " + event.closure());
                break;
        }
    }

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
    {
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event)
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
    }

    public void onAllMsg(Msg msg, OmmProviderEvent event)
    {
    }

    void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        event.provider().submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).nameType(EmaRdm.USER_NAME).complete(true).solicited(true)
                .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"), event.handle());
        System.out.println();
        System.out.println("Clone ReqMsg");
	ReqMsg cloneReqMsg = EmaFactory.createReqMsg(reqMsg);
        System.out.println(cloneReqMsg);
        System.out.println("Clone Req Msg Item Name: " + cloneReqMsg.name());
        System.out.println();
    }

    // APIQA
    void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        if (itemHandle != 0)
        {
            System.out.println("Received invalid Request msg. Item Handle: " + event.handle() + " Closure: " + event.closure());
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

    // END APIQA
    void processAppDomainRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        if (itemHandle != 0)
        {
            System.out.println("Received invalid Request msg. Item Handle: " + event.handle() + " Closure: " + event.closure());
            return;
        }

        FieldList fieldList = EmaFactory.createFieldList();
        fieldList.add(EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add(EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add(EmaFactory.createFieldEntry().real(30, 9, OmmReal.MagnitudeType.EXPONENT_0));
        fieldList.add(EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

        event.provider().submit(EmaFactory.createRefreshMsg().domainType(reqMsg.domainType()).name(reqMsg.name()).serviceName(reqMsg.serviceName()).solicited(true)
                .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").payload(fieldList).complete(true), event.handle());

        itemHandle = event.handle();
    }

    void processAppDomainGenericMsg(GenericMsg genericMsg, OmmProviderEvent event)
    {
        if (itemHandle == 0 || itemHandle != event.handle())
        {
            System.out.println("Received invalid Generic msg. Item Handle: " + event.handle() + " Closure: " + event.closure());
            return;
        }

        System.out.println("Received Generic. Item Handle: " + event.handle() + " Closure: " + event.closure());

        System.out.println();
        System.out.println("Clone GenericMsg");
	GenericMsg cloneGenericMsg = EmaFactory.createGenericMsg(genericMsg);
        System.out.println(cloneGenericMsg);
        System.out.println("Clone Generic Msg Item Name: " + cloneGenericMsg.name());
        System.out.println();
        ElementList elementList = EmaFactory.createElementList();
        elementList.add(EmaFactory.createElementEntry().intValue("valueFromProvider", ++count));
        event.provider().submit(EmaFactory.createGenericMsg().domainType(APP_DOMAIN).name("genericMsg").payload(elementList), event.handle());

        System.out.println();
    }

    // APIQA
    void processLoginDomainGenericMsg(GenericMsg genericMsg, OmmProviderEvent event)
    {
        System.out.println("Received:    GenericMsg. login Handle: " + event.handle() + " Closure: " + event.closure());
        System.out.println(genericMsg);

        System.out.println();
        System.out.println("Clone GenericMsg");
	GenericMsg cloneGenericMsg = EmaFactory.createGenericMsg(genericMsg);
        System.out.println(cloneGenericMsg);
        System.out.println("Clone Generic Msg Item State: " + cloneGenericMsg.name());
        System.out.println();
        ElementList elementList = EmaFactory.createElementList();
        elementList.add(EmaFactory.createElementEntry().intValue("valueFromProvider", 3));
        event.provider().submit(EmaFactory.createGenericMsg().domainType(EmaRdm.MMT_LOGIN).name("genericMsgInGeneral").payload(elementList), event.handle());

        System.out.println();
    }

    void processDirectoryDomainGenericMsg(GenericMsg genericMsg, OmmProviderEvent event)
    {
        System.out.println("Received:    GenericMsg. Dir Handle: " + event.handle() + " Closure: " + event.closure());
        System.out.println(genericMsg);

        ElementList elementList = EmaFactory.createElementList();
        elementList.add(EmaFactory.createElementEntry().intValue("valueFromProvider", 3));
        event.provider().submit(EmaFactory.createGenericMsg().domainType(EmaRdm.MMT_DIRECTORY).name("genericMsgInGeneral").payload(elementList), event.handle());

        System.out.println();
    }
    // END APIQA
}

public class IProvider
{
    public static void main(String[] args)
    {
        OmmProvider provider = null;

        try
        {
            AppClient appClient = new AppClient();
            provider = EmaFactory.createOmmProvider(EmaFactory.createOmmIProviderConfig().operationModel(OperationModel.USER_DISPATCH), appClient);

            while (appClient.itemHandle == 0)
                provider.dispatch(1000);

            FieldList fieldList = EmaFactory.createFieldList();
            long startTime = System.currentTimeMillis();
            int i = 0;
            while (startTime + 60000 > System.currentTimeMillis())
            {
                provider.dispatch(1000);

                fieldList.clear();
                fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i++, OmmReal.MagnitudeType.EXPONENT_0));

                provider.submit(EmaFactory.createUpdateMsg().domainType(AppClient.APP_DOMAIN).payload(fieldList), appClient.itemHandle);

                Thread.sleep(1000);
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
