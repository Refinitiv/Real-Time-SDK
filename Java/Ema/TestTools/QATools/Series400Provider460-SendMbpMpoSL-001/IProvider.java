/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.examples.training.iprovider.series400.ex460_MP_RTT;

import com.refinitiv.ema.access.*;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.rdm.DomainTypes;

import java.nio.ByteBuffer;
import java.util.*;
import java.util.Map;

import com.refinitiv.ema.access.MapEntry;

class AppClient implements OmmProviderClient
{
    private boolean enableRTT;
    public String OrderNr="100";
    /*
      holds the last latency value for each client that supports RTT requests
      clientLatencyMap.keySet() holds handles to all consumers that can receive RTT requests
    */
    public Map<Long, Long> clientLatencyMap = new HashMap<>();

    //holds all item requests for each client
    public Map<Long, List<Long>> clientItemHandlesMap = new HashMap<>();
    public Map<Long, List<Long>> clientMBPItemHandlesMap = new HashMap<>();
    public Map<Long, List<Long>> clientMBOItemHandlesMap = new HashMap<>();
    public Map<Long, List<Long>> clientSLItemHandlesMap = new HashMap<>();

    int i = 0;

    FieldList mbpentryLoad = EmaFactory.createFieldList();
    UpdateMsg mbpupdateMsg = EmaFactory.createUpdateMsg();
    com.refinitiv.ema.access.Map mbpmap = EmaFactory.createMap();

    FieldList mbofieldList = EmaFactory.createFieldList();
    com.refinitiv.ema.access.Map mbomap = EmaFactory.createMap();

    public AppClient(boolean rtt) {
        enableRTT = rtt;
    }

    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
    {
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_LOGIN :
                processLoginRequest(reqMsg, event);
                break;
            case EmaRdm.MMT_MARKET_PRICE :
                processMarketPriceRequest(reqMsg, event);
                break;
            case EmaRdm.MMT_MARKET_BY_PRICE:
                processMarketByPriceRequest(reqMsg, event);
                break;
            case EmaRdm.MMT_MARKET_BY_ORDER:
                processMarketByOrderRequest(reqMsg, event);
                break;
            case EmaRdm.MMT_SYMBOL_LIST:
                processSymbolListRequest(reqMsg, event);
                break;
            default :
                break;
        }
    }

    public void onRefreshMsg(RefreshMsg refreshMsg,	OmmProviderEvent event){}
    public void onStatusMsg(StatusMsg statusMsg, OmmProviderEvent event){}

    public void onGenericMsg(GenericMsg genericMsg, OmmProviderEvent event){
        switch (genericMsg.domainType()) {
            case EmaRdm.MMT_LOGIN:
                processRTT(genericMsg, event);
                break;
            default:
        }
    }
    public void onPostMsg(PostMsg postMsg, OmmProviderEvent event){}
    public void onReissue(ReqMsg reqMsg, OmmProviderEvent event){}
    public void onClose(ReqMsg reqMsg, OmmProviderEvent event){
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_LOGIN :
                clientLatencyMap.remove(event.handle());
                clientItemHandlesMap.remove(event.clientHandle());
                break;
            case EmaRdm.MMT_MARKET_PRICE :
                List<Long> list = clientItemHandlesMap.get(event.clientHandle());
                list.remove(event.handle());
                if (list.isEmpty()) {
                    clientItemHandlesMap.remove(event.clientHandle());
                }
                break;
            case EmaRdm.MMT_MARKET_BY_PRICE :
                List<Long> mbplist = clientMBPItemHandlesMap.get(event.clientHandle());
                mbplist.remove(event.handle());
                if (mbplist.isEmpty()) {
                    clientMBPItemHandlesMap.remove(event.clientHandle());
                }
                break;
            case EmaRdm.MMT_MARKET_BY_ORDER :
                List<Long> mbolist = clientMBOItemHandlesMap.get(event.clientHandle());
                mbolist.remove(event.handle());
                if (mbolist.isEmpty()) {
                    clientMBOItemHandlesMap.remove(event.clientHandle());
                }
                break;
            case EmaRdm.MMT_SYMBOL_LIST :
                List<Long> sllist = clientSLItemHandlesMap.get(event.clientHandle());
                sllist.remove(event.handle());
                if (sllist.isEmpty()) {
                    clientSLItemHandlesMap.remove(event.clientHandle());
                }
                break;
            default :
                break;
        }
    }
    public void onAllMsg(Msg msg, OmmProviderEvent event){}

    void processRTT(GenericMsg genericMsg, OmmProviderEvent event) {
        if (clientLatencyMap.containsKey(event.handle())) {
            if (genericMsg.payload().dataType() == DataTypes.ELEMENT_LIST && genericMsg.domainType() == DomainTypes.LOGIN) {
                System.out.println("Received login RTT message from Consumer " + event.handle());
                ElementList data = genericMsg.payload().elementList();
                for ( ElementEntry elem : data) {
                    if (elem.name().equals(EmaRdm.ENAME_TICKS)) {
                        System.out.println("        RTT Tick value is: " + elem.uintValue());
                        long latency = System.nanoTime() - elem.uintValue();
                        clientLatencyMap.put(event.handle(), latency);
                        System.out.println("        Last RTT message latency is: " + latency);
                    }
                }
            }
        }
    }

    void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        if (enableRTT) {
            if (reqMsg.attrib().dataType() == DataTypes.ELEMENT_LIST) {
                ElementList reqAttributes = reqMsg.attrib().elementList();
                for ( ElementEntry reqAttrib : reqAttributes ) {
                    if (reqAttrib.name().equals(EmaRdm.ENAME_LATENCY)) {
                        clientLatencyMap.put(event.handle(), 0L);
                        System.out.println("Consumer with handle " + event.handle() + " supports gathering RTT statistics");
                    }
                }
            }
            ElementList elementList = EmaFactory.createElementList();
            elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LATENCY, EmaRdm.LOGIN_RTT_ELEMENT));
            event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
                            nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
                            state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted").
                            attrib(elementList),
                    event.handle() );

        } else {
            event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).
                            nameType(EmaRdm.USER_NAME).complete(true).solicited(true).
                            state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"),
                    event.handle() );
        }

    }

    void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        FieldList fieldList = EmaFactory.createFieldList();
        fieldList.add( EmaFactory.createFieldEntry().real(22, 3990, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add( EmaFactory.createFieldEntry().real(25, 3994, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add( EmaFactory.createFieldEntry().real(30, 9,  OmmReal.MagnitudeType.EXPONENT_0));
        fieldList.add( EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

        event.provider().submit( EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true)
                        .privateStream(reqMsg.privateStream())
                                .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
                        payload(fieldList).complete(true),
                event.handle() );

        if (clientItemHandlesMap.containsKey(event.clientHandle())) {
            clientItemHandlesMap.get(event.clientHandle()).add(event.handle());
        } else {
            LinkedList<Long> list = new LinkedList<>();
            list.add(event.handle());
            clientItemHandlesMap.put(event.clientHandle(), list);
        }
    }

    void processMarketByPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        com.refinitiv.ema.access.Map map = EmaFactory.createMap();
        FieldList summary = EmaFactory.createFieldList();
        FieldList entryLoad = EmaFactory.createFieldList();

        summary.add(EmaFactory.createFieldEntry().enumValue(15, 840));
        summary.add(EmaFactory.createFieldEntry().enumValue(53, 1));
        summary.add(EmaFactory.createFieldEntry().enumValue(3423, 1));
        summary.add(EmaFactory.createFieldEntry().enumValue(1709, 2));

        map.summaryData(summary);

        entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        entryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
        entryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
        entryLoad.add(EmaFactory.createFieldEntry().rmtes(3435, ByteBuffer.wrap("Market Maker".getBytes())));

        map.add(EmaFactory.createMapEntry().keyAscii(OrderNr, MapEntry.MapAction.ADD, entryLoad));

        event.provider().submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_PRICE).serviceName(reqMsg.serviceName()).
                name(reqMsg.name()).state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed")
                .privateStream(reqMsg.privateStream())
                        .solicited(true).payload(map).complete(true), event.handle());

        if (clientMBPItemHandlesMap.containsKey(event.clientHandle())) {
            clientMBPItemHandlesMap.get(event.clientHandle()).add(event.handle());
        } else {
            LinkedList<Long> list = new LinkedList<>();
            list.add(event.handle());
            clientMBPItemHandlesMap.put(event.clientHandle(), list);
        }
    }

    void processMarketByOrderRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        FieldList mapSummaryData = EmaFactory.createFieldList();
        mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(15,  840));
        mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(53,  1));
        mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(3423,  1));
        mapSummaryData.add(EmaFactory.createFieldEntry().enumValue(1709,  2));

        FieldList entryData = EmaFactory.createFieldList();
        entryData.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        entryData.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
        entryData.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
        entryData.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));

        com.refinitiv.ema.access.Map map = EmaFactory.createMap();
        map.summaryData(mapSummaryData);

        map.add(EmaFactory.createMapEntry().keyAscii(OrderNr, MapEntry.MapAction.ADD, entryData));

        event.provider().submit( EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER)
                        .name(reqMsg.name()).serviceName(reqMsg.serviceName())
                        .solicited(true).privateStream(reqMsg.privateStream())
                        .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
                        payload(map).complete(true),
                event.handle() );

        if (clientMBOItemHandlesMap.containsKey(event.clientHandle())) {
            clientMBOItemHandlesMap.get(event.clientHandle()).add(event.handle());
        } else {
            LinkedList<Long> list = new LinkedList<>();
            list.add(event.handle());
            clientMBOItemHandlesMap.put(event.clientHandle(), list);
        }
    }

    void processSymbolListRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        com.refinitiv.ema.access.Map mapEnc = EmaFactory.createMap();
        FieldList fieldList1 = EmaFactory.createFieldList();
        String a;
        for (int i = 0; i < 5 ; ++i) {


            a = "A"+i;

            mapEnc.add(EmaFactory.createMapEntry().keyAscii(a, MapEntry.MapAction.ADD, fieldList1));
        }

        event.provider().submit( EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceId(reqMsg.serviceId()).solicited(true).
                        state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Refresh Completed").
                        payload(mapEnc).complete(true),
                event.handle() );

        if (clientSLItemHandlesMap.containsKey(event.clientHandle())) {
            clientSLItemHandlesMap.get(event.clientHandle()).add(event.handle());
        } else {
            LinkedList<Long> list = new LinkedList<>();
            list.add(event.handle());
            clientSLItemHandlesMap.put(event.clientHandle(), list);
        }
    }

    public void sendRTTRequests(OmmProvider provider) {
        if (!enableRTT) {
            System.out.println("This provider does not support RTT");
        } else {
            ElementList elementList = EmaFactory.createElementList();
            for (Long handle : clientLatencyMap.keySet()) {
                elementList.clear();
                Long latency = clientLatencyMap.get(handle);
                if (latency != 0) {
                    elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LATENCY, latency));
                }
                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TICKS, System.nanoTime()));
                provider.submit(EmaFactory.createGenericMsg().payload(elementList).domainType(DomainTypes.LOGIN).providerDriven(true).complete(true), handle);
            }
        }
    }

    public void sendUpdates(OmmProvider provider, FieldList fieldList) {
        for (Long cl_h : clientItemHandlesMap.keySet()) {
            List<Long> list = clientItemHandlesMap.get(cl_h);
            for (Long ih : list)  {
                provider.submit( EmaFactory.createUpdateMsg().payload( fieldList ), ih );
            }
        }
    }

    public void sendMBPUpdates(OmmProvider provider) {
        for (Long cl_h : clientMBPItemHandlesMap.keySet()) {
            List<Long> list = clientMBPItemHandlesMap.get(cl_h);
            for (Long ih : list)  {
                mbpentryLoad.clear();
                mbpentryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76 + i++ * 0.1, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                mbpentryLoad.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
                mbpentryLoad.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
                mbpentryLoad.add(EmaFactory.createFieldEntry().rmtes(3435, ByteBuffer.wrap("Market Maker".getBytes())));

                mbpmap.clear();
                mbpmap.add(EmaFactory.createMapEntry().keyAscii(OrderNr, MapEntry.MapAction.UPDATE, mbpentryLoad));

                provider.submit(mbpupdateMsg.clear().domainType(EmaRdm.MMT_MARKET_BY_PRICE).payload(mbpmap), ih );
            }
        }
    }

    public void sendMBOUpdates(OmmProvider provider) {
        for (Long cl_h : clientMBPItemHandlesMap.keySet()) {
            List<Long> list = clientMBPItemHandlesMap.get(cl_h);
            for (Long ih : list)  {
                mbofieldList.add(EmaFactory.createFieldEntry().realFromDouble(3427, 7.76 + i++ * 0.1, OmmReal.MagnitudeType.EXPONENT_NEG_2));
                mbofieldList.add(EmaFactory.createFieldEntry().realFromDouble(3429, 9600));
                mbofieldList.add(EmaFactory.createFieldEntry().enumValue(3428, 2));
                mbofieldList.add(EmaFactory.createFieldEntry().rmtes(212, ByteBuffer.wrap("Market Maker".getBytes())));

                mbomap.add(EmaFactory.createMapEntry().keyAscii(OrderNr, MapEntry.MapAction.ADD, mbofieldList));

                provider.submit( EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_MARKET_BY_ORDER).payload( mbomap ), ih );

                mbomap.clear();
                mbofieldList.clear();
            }
        }
    }

    public boolean standBy() {
        return clientItemHandlesMap.size() == 0 && clientMBPItemHandlesMap.size() == 0 && clientMBOItemHandlesMap.size() == 0;
    }
}

public class IProvider
{
    static long DELTA = 1000;
    static String providerName = null;
    static boolean rtt = false;

    public static void printHelp()
    {
        System.out.println("\nOptions:\n" + "  -?\tShows this usage\n" + "  -n  \tProvider name from xml config\n + \"  -rtt  \tSpecifies whether the povider supports rtt\n"
                + "\n");

        System.exit(-1);
    }
    public static boolean readCommandlineArgs(String[] argv)
    {
        int count = argv.length;
        int idx = 0;

        while (idx < count)
        {
            if ("-?".equals(argv[idx]))
            {
                printHelp();
                return false;
            }
            else if ("-n".equals(argv[idx]))
            {
                if (++idx >= count)
                {
                    printHelp();
                    return false;
                }
                providerName = argv[idx];
                ++idx;
            }
            else if ("-rtt".equals(argv[idx]))
            {
                rtt = true;
                ++idx;
            }
            else
            {
                System.out.println("Found some other arg: " + argv[idx]);
                printHelp();
                return false;
            }
        }
        return true;
    }

    public static void main(String[] args)
    {

        long nextRequestTime = System.currentTimeMillis() + DELTA;
        OmmProvider provider = null;

        readCommandlineArgs(args);

        try
        {
            OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();
            AppClient appClient = new AppClient(rtt);

            if (providerName != null) config.providerName(providerName);

            provider = EmaFactory.createOmmProvider(config.operationModel(OmmIProviderConfig.OperationModel.USER_DISPATCH), appClient);
            FieldList fieldList = EmaFactory.createFieldList();

            Random rnd = new Random();
            while (appClient.standBy()) {
                provider.dispatch(500);
                Thread.sleep(500);
            }

            for (int i = 0; i < 6000; i++)
            {
                provider.dispatch(50);

                if (System.currentTimeMillis() >= nextRequestTime) {

                    appClient.sendRTTRequests(provider);
                    fieldList.clear();
                    fieldList.add(EmaFactory.createFieldEntry().real(22, 3991 + rnd.nextInt(50), OmmReal.MagnitudeType.EXPONENT_NEG_2));
                    fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + rnd.nextInt(50), OmmReal.MagnitudeType.EXPONENT_0));

                    appClient.sendUpdates(provider, fieldList);

                    appClient.sendMBOUpdates(provider);
                    appClient.sendMBPUpdates(provider);

                    nextRequestTime = System.currentTimeMillis() + DELTA;
                }

                Thread.sleep(50);
            }
        }
        catch (Exception excp)
        {
            System.out.println("Usage: ");
            System.out.println(excp.getMessage());
        }
        finally
        {
            if (provider != null) provider.uninitialize();
        }
    }
}
