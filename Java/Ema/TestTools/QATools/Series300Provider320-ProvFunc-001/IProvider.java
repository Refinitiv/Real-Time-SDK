///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.thomsonreuters.ema.examples.training.iprovider.series300.example320__Custom__GenericMsg;

import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmIProviderConfig;
import com.thomsonreuters.ema.access.OmmProvider;
import com.thomsonreuters.ema.access.OmmProviderClient;
import com.thomsonreuters.ema.access.OmmProviderEvent;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.Series;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.rdm.DataDictionary;
import com.thomsonreuters.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
    public long itemHandle = 0;
    public DataDictionary dataDictionary = EmaFactory.createDataDictionary();

    private int fragmentationSize = 96000;
    private Series series = EmaFactory.createSeries();
    private RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
    private int currentValue;
    private boolean result;

    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
    {
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_LOGIN:
                processLoginRequest(reqMsg, event);
                break;
            case EmaRdm.MMT_DICTIONARY:
                processDictionaryRequest(reqMsg, event);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                processMarketPriceRequest(reqMsg, event);
                break;
            default:
                processInvalidItemRequest(reqMsg, event);
                break;
        }
    }

    public void onReissue(ReqMsg reqMsg, OmmProviderEvent event)
    {
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_DICTIONARY:
                processDictionaryRequest(reqMsg, event);
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

    public void onClose(ReqMsg reqMsg, OmmProviderEvent event)
    {
    }

    public void onAllMsg(Msg msg, OmmProviderEvent event)
    {
    }

    void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        event.provider().submit(refreshMsg.clear().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).nameType(EmaRdm.USER_NAME).complete(true).solicited(true)
                                        .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"), event.handle());
    }

    void processDictionaryRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        result = false;
        refreshMsg.clear().clearCache(true);

        if (reqMsg.name().equals("RWFFld"))
        {
            currentValue = dataDictionary.minFid();

            while (!result)
            {
                currentValue = dataDictionary.encodeFieldDictionary(series, currentValue, reqMsg.filter(), fragmentationSize);

                result = currentValue == dataDictionary.maxFid() ? true : false;

                event.provider().submit(refreshMsg.name(reqMsg.name()).serviceName(reqMsg.serviceName()).domainType(EmaRdm.MMT_DICTIONARY).filter(reqMsg.filter()).payload(series).complete(result)
                                                .solicited(true), event.handle());

                refreshMsg.clear();
            }
        }
        else if (reqMsg.name().equals("RWFEnum"))
        {
            currentValue = 0;

            while (!result)
            {
                currentValue = dataDictionary.encodeEnumTypeDictionary(series, currentValue, reqMsg.filter(), fragmentationSize);

                result = currentValue == dataDictionary.enumTables().size() ? true : false;

                event.provider().submit(refreshMsg.name(reqMsg.name()).serviceName(reqMsg.serviceName()).domainType(EmaRdm.MMT_DICTIONARY).filter(reqMsg.filter()).payload(series).complete(result)
                                                .solicited(true), event.handle());

                refreshMsg.clear();
            }
        }
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

        event.provider().submit(refreshMsg.clear().name(reqMsg.name()).serviceName(reqMsg.serviceName()).solicited(true)
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
        OmmProvider provider = null;
        try
        {
            AppClient appClient = new AppClient();

            appClient.dataDictionary.loadFieldDictionary("RDMFieldDictionary");
            appClient.dataDictionary.loadEnumTypeDictionary("enumtype.def");

            provider = EmaFactory.createOmmProvider(EmaFactory.createOmmIProviderConfig().adminControlDictionary(OmmIProviderConfig.AdminControl.API_CONTROL), appClient);

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
                if (i == 2)
                {
                    provider.submit(EmaFactory.createStatusMsg().name("RWFFld").serviceId(1).domainType(EmaRdm.MMT_DICTIONARY)
                                            .state(OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Open Suspect Status on dictionary"), 0);
                }
                if (i == 6)
                {
                    provider.submit(EmaFactory.createStatusMsg().name("RWFFld").serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_DICTIONARY)
                                            .state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, "Closed Status on dictionary"), 0);
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
