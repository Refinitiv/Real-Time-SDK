/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2017-2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.refinitiv.ema.examples.training.iprovider.series300.ex350_Dictionary_Streaming;

import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.UpdateMsg;
import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
    private boolean fldDictComplete = false;
    private boolean enumTypeComplete = false;
    public long itemHandle = 0;
    public long loginHandle = 0;

    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent)
    {
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_LOGIN:
                processLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                processMarketPriceRequest(reqMsg, providerEvent);
                break;
            default:
                processInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
    {
        System.out.println("Received Refresh. Item Handle: " + event.handle());

        System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

        System.out.println("Item State: " + refreshMsg.state());
        // APIQA
        DataDictionary dataDictionary = (DataDictionary)event.closure();
        decode(refreshMsg, refreshMsg.complete(), dataDictionary);
        // END APIQA
        System.out.println();
    }

    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event)
    {
        System.out.println("Received Status. Item Handle: " + event.handle() + " Closure: " + event.closure());

        System.out.println("Item Name: " + (statusMsg.hasName() ? statusMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (statusMsg.hasServiceName() ? statusMsg.serviceName() : "<not set>"));

        if (statusMsg.hasState())
            System.out.println("Item State: " + statusMsg.state());

        System.out.println();
    }

    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent providerEvent)
    {
    }

    public void onPostMsg(PostMsg postMsg, OmmProviderEvent providerEvent)
    {
    }

    public void onReissue(ReqMsg reqMsg, OmmProviderEvent providerEvent)
    {
    }

    public void onClose(ReqMsg reqMsg, OmmProviderEvent providerEvent)
    {
    }

    public void onAllMsg(Msg msg, OmmProviderEvent providerEvent)
    {
    }

    void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        event.provider().submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).nameType(EmaRdm.USER_NAME).complete(true).solicited(true)
                .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted").attrib(EmaFactory.createElementList()), event.handle());

        loginHandle = event.handle();
    }

    void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        if (itemHandle != 0)
        {
            processInvalidItemRequest(reqMsg, event);
            return;
        }

        FieldList fieldList = EmaFactory.createFieldList();

        fieldList.add(EmaFactory.createFieldEntry().ascii(3, reqMsg.name()));
        fieldList.add(EmaFactory.createFieldEntry().enumValue(15, 840));
        fieldList.add(EmaFactory.createFieldEntry().real(21, 3900, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add(EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add(EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add(EmaFactory.createFieldEntry().real(30, 9, OmmReal.MagnitudeType.EXPONENT_0));
        fieldList.add(EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

        event.provider().submit(
                                EmaFactory.createRefreshMsg().serviceName(reqMsg.serviceName()).name(reqMsg.name())
                                        .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").solicited(true).payload(fieldList).complete(true),
                                event.handle());

        itemHandle = event.handle();
    }

    void processInvalidItemRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        event.provider().submit(EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).domainType(reqMsg.domainType())
                .state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_FOUND, "Item not found"), event.handle());
    }

    // APIQA
    void decode(Msg msg, boolean complete, DataDictionary dataDictionary)
    {
        switch (msg.payload().dataType())
        {
            case DataTypes.SERIES:

                if (msg.name().equals("RWFFld"))
                {
                    dataDictionary.decodeFieldDictionary(msg.payload().series(), EmaRdm.DICTIONARY_VERBOSE);

                    if (complete)
                    {
                        fldDictComplete = true;
                    }
                }
                else if (msg.name().equals("RWFEnum"))
                {
                    dataDictionary.decodeEnumTypeDictionary(msg.payload().series(), EmaRdm.DICTIONARY_VERBOSE);
                    if (complete)
                    {
                        enumTypeComplete = true;
                    }
                }

                if (fldDictComplete && enumTypeComplete)
                {
                    System.out.println();
                    System.out.println("\nDictionary download complete");
                    System.out.println("Dictionary Id : " + dataDictionary.dictionaryId());
                    System.out.println("Dictionary field version : " + dataDictionary.fieldVersion());
                    System.out.println("Number of dictionary entries : " + dataDictionary.entries().size());
                    System.out.println(dataDictionary);
                    enumTypeComplete = false;
                    fldDictComplete = false;
                }

                break;
            default:
                break;
        }
    }
}

//END APIQA
public class IProvider
{
    public static void main(String[] args)
    {
        OmmProvider provider = null;
        try
        {
            AppClient appClient = new AppClient();
            FieldList fieldList = EmaFactory.createFieldList();
            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();

            provider = EmaFactory.createOmmProvider(EmaFactory.createOmmIProviderConfig(), appClient);

            while (appClient.loginHandle == 0)
                Thread.sleep(1000);
            // APIQA
            DataDictionary ddict = EmaFactory.createDataDictionary();
            DataDictionary ddict1 = EmaFactory.createDataDictionary();
            DataDictionary ddict2 = EmaFactory.createDataDictionary();
            long rwfFld = provider
                    .registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(EmaRdm.DICTIONARY_VERBOSE).serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_DICTIONARY).interestAfterRefresh(false),
                                    appClient, ddict);
            long rwfEnum = provider.registerClient(EmaFactory.createReqMsg().name("RWFEnum").filter(EmaRdm.DICTIONARY_VERBOSE).serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_DICTIONARY)
                    .interestAfterRefresh(false), appClient, ddict);

            long rwfFld1 = provider
                    .registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(EmaRdm.DICTIONARY_VERBOSE).serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_DICTIONARY).interestAfterRefresh(false),
                                    appClient, ddict1);
            long rwfEnum1 = provider.registerClient(EmaFactory.createReqMsg().name("RWFEnum").filter(EmaRdm.DICTIONARY_VERBOSE).serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_DICTIONARY)
                    .interestAfterRefresh(false), appClient, ddict1);
            long rwfFld2 = provider
                    .registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(EmaRdm.DICTIONARY_VERBOSE).serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_DICTIONARY).interestAfterRefresh(false),
                                    appClient, ddict2);
            long rwfEnum2 = provider.registerClient(EmaFactory.createReqMsg().name("RWFEnum").filter(EmaRdm.DICTIONARY_VERBOSE).serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_DICTIONARY)
                    .interestAfterRefresh(false), appClient, ddict2);
            // END APIQA
            while (appClient.itemHandle == 0)
                Thread.sleep(1000);

            for (int i = 0; i < 60; i++)
            {
                fieldList.clear();
                fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                fieldList.add(EmaFactory.createFieldEntry().real(25, 3994 + i, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + i, OmmReal.MagnitudeType.EXPONENT_0));
                fieldList.add(EmaFactory.createFieldEntry().real(31, 19 + i, OmmReal.MagnitudeType.EXPONENT_0));

                provider.submit(updateMsg.clear().payload(fieldList), appClient.itemHandle);

                Thread.sleep(1000);
            }
        }
        catch (OmmException | InterruptedException excp)
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
