///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------
//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.thomsonreuters.ema.examples.training.iprovider.series300.example350__Dictionary__Streaming;

import com.thomsonreuters.ema.access.EmaFactory;
import com.thomsonreuters.ema.access.FieldList;
//APIQA
import com.thomsonreuters.ema.access.FilterEntry;
import com.thomsonreuters.ema.access.FilterList;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.MapEntry;
//END APIQA
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Msg;
import com.thomsonreuters.ema.access.OmmException;
import com.thomsonreuters.ema.access.OmmProvider;
import com.thomsonreuters.ema.access.OmmProviderClient;
import com.thomsonreuters.ema.access.OmmProviderEvent;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.rdm.DataDictionary;
import com.thomsonreuters.ema.rdm.EmaRdm;

class AppClient implements OmmProviderClient
{
    private DataDictionary dataDictionary = EmaFactory.createDataDictionary();
    private boolean fldDictComplete = false;
    private boolean enumTypeComplete = false;
    private boolean dumpDictionary = false;
    public long itemHandle = 0;
    public long loginHandle = 0;
    // APIQA
    public int filter = EmaRdm.DICTIONARY_NORMAL;

    AppClient(String[] args)
    {
        int idx = 0;

        while (idx < args.length)
        {
            if (args[idx].compareTo("-dumpDictionary") == 0)
            {
                dumpDictionary = true;
            }
            // APIQA
            else if (args[idx].compareTo("-filter") == 0)
            {
                if (++idx == args.length)
                    break;

                if (args[idx].compareToIgnoreCase("INFO") == 0)
                {
                    filter = EmaRdm.DICTIONARY_INFO;
                }
                if (args[idx].compareToIgnoreCase("MINIMAL") == 0)
                {
                    filter = EmaRdm.DICTIONARY_MINIMAL;
                }
                if (args[idx].compareToIgnoreCase("NORMAL") == 0)
                {
                    filter = EmaRdm.DICTIONARY_NORMAL;
                }
                if (args[idx].compareToIgnoreCase("VERBOSE") == 0)
                {
                    filter = EmaRdm.DICTIONARY_VERBOSE;
                }
            }
            // END APIQA
            ++idx;
        }
    }

    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent providerEvent)
    {
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_LOGIN:
                processLoginRequest(reqMsg, providerEvent);
                break;
            case EmaRdm.MMT_MARKET_PRICE:
                processMarketPriceRequest(reqMsg, providerEvent);
                // APIQA
                System.out.println("Send Login Status Close ");
                sendLoginClose(reqMsg, providerEvent);
                // END APIQA
                break;
            default:
                processInvalidItemRequest(reqMsg, providerEvent);
                break;
        }
    }

    public void onRefreshMsg(RefreshMsg refreshMsg, OmmProviderEvent event)
    {
        // System.out.println("Received Refresh. Item Handle: " + event.handle()
        // + " Closure: " + event.closure());
        System.out.println("Received Refresh. Item Handle: " + event.handle());

        System.out.println("Item Name: " + (refreshMsg.hasName() ? refreshMsg.name() : "<not set>"));
        System.out.println("Service Name: " + (refreshMsg.hasServiceName() ? refreshMsg.serviceName() : "<not set>"));

        System.out.println("Item State: " + refreshMsg.state());

        decode(refreshMsg, refreshMsg.complete());

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

    // APIQA
    void sendLoginClose(ReqMsg reqMsg, OmmProviderEvent event)
    {
        event.provider().submit(EmaFactory.createStatusMsg().domainType(EmaRdm.MMT_LOGIN).nameType(EmaRdm.USER_NAME).state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT,
                                                                                                                           OmmState.StatusCode.NOT_AUTHORIZED, "Login denied"),
                                loginHandle);
    }

    // END APIQA
    void updateSourceDirectory(OmmProvider provider)
    {
        ElementList elementList = EmaFactory.createElementList();
        elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN));
        FilterList filterList = EmaFactory.createFilterList();
        filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, elementList));
        Map map = EmaFactory.createMap();
        map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.DELETE, filterList));
        provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).payload(map), 0);
    }

    // END APIQA
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

    void decode(Msg msg, boolean complete)
    {
        switch (msg.payload().dataType())
        {
            case DataTypes.SERIES:

                if (msg.name().equals("RWFFld"))
                {
                    dataDictionary.decodeFieldDictionary(msg.payload().series(), filter);

                    if (complete)
                    {
                        fldDictComplete = true;
                    }
                }
                else if (msg.name().equals("RWFEnum"))
                {
                    dataDictionary.decodeEnumTypeDictionary(msg.payload().series(), filter);

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
                    System.out.println("Number of Fld Table dictionary entries : " + dataDictionary.entries().size());
                    System.out.println("Number of Enum Table dictionary entries : " + dataDictionary.enumTables().size());

                    if (dumpDictionary)
                        System.out.println(dataDictionary);
                    dataDictionary.clear();
                    enumTypeComplete = false;
                    fldDictComplete = false;
                }

                break;
            default:
                break;
        }
    }
}

public class IProvider
{
    public static void main(String[] args)
    {
        OmmProvider provider = null;
        try
        {
            AppClient appClient = new AppClient(args);
            FieldList fieldList = EmaFactory.createFieldList();
            UpdateMsg updateMsg = EmaFactory.createUpdateMsg();

            provider = EmaFactory.createOmmProvider(EmaFactory.createOmmIProviderConfig(), appClient);

            while (appClient.loginHandle == 0)
                Thread.sleep(1000);
            // APIQA
            long rwfFld = provider.registerClient(EmaFactory.createReqMsg().name("RWFFld").filter(appClient.filter).serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_DICTIONARY), appClient);
            long rwfEnum = provider.registerClient(EmaFactory.createReqMsg().name("RWFEnum").filter(appClient.filter).serviceName("DIRECT_FEED").domainType(EmaRdm.MMT_DICTIONARY), appClient);
            // END APIQA
            while (appClient.itemHandle == 0)
                Thread.sleep(1000);
            for (int i = 0; i < 10; i++)
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
