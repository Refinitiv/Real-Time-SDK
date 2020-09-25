///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

//APIQA this file is QATools standalone. See qa_readme.txt for details about this tool.

package com.refinitiv.ema.examples.training.iprovider.series100.ex100_MP_Streaming;

import java.nio.ByteBuffer;

import com.refinitiv.ema.access.ElementList;
import com.refinitiv.ema.access.EmaFactory;
import com.refinitiv.ema.access.FieldList;
import com.refinitiv.ema.access.FilterEntry;
import com.refinitiv.ema.access.FilterList;
import com.refinitiv.ema.access.GenericMsg;
import com.refinitiv.ema.access.Map;
import com.refinitiv.ema.access.MapEntry;
import com.refinitiv.ema.access.Msg;
import com.refinitiv.ema.access.OmmArray;
import com.refinitiv.ema.access.OmmException;
import com.refinitiv.ema.access.OmmIProviderConfig;
import com.refinitiv.ema.access.OmmProvider;
import com.refinitiv.ema.access.OmmProviderClient;
import com.refinitiv.ema.access.OmmProviderEvent;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.PostMsg;
import com.refinitiv.ema.access.RefreshMsg;
import com.refinitiv.ema.access.ReqMsg;
import com.refinitiv.ema.access.StatusMsg;
import com.refinitiv.ema.access.Vector;
import com.refinitiv.ema.access.VectorEntry;
import com.refinitiv.ema.rdm.EmaRdm;

//APIQA
class AppClient implements OmmProviderClient
{
    public long itemHandle = 0;
    public boolean submitting = true;

    public void onReqMsg(ReqMsg reqMsg, OmmProviderEvent event)
    {
        switch (reqMsg.domainType())
        {
            case EmaRdm.MMT_LOGIN:
                processLoginRequest(reqMsg, event);
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
        System.out.println(reqMsg);

        submitting = false;
    }

    public void onAllMsg(Msg msg, OmmProviderEvent event)
    {
    }

    void processLoginRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        event.provider().submit(EmaFactory.createRefreshMsg().domainType(EmaRdm.MMT_LOGIN).name(reqMsg.name()).nameType(EmaRdm.USER_NAME).complete(true).solicited(true)
                                        .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Login accepted"), event.handle());
    }

    void processMarketPriceRequest(ReqMsg reqMsg, OmmProviderEvent event)
    {
        if (itemHandle != 0)
        {
            event.provider().submit(EmaFactory.createStatusMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName())
                                            .state(OmmState.StreamState.CLOSED, OmmState.DataState.SUSPECT, OmmState.StatusCode.NOT_FOUND, "This test tool support only one item."), event.handle());
            return;
        }

        FieldList fieldList = EmaFactory.createFieldList();
        fieldList.add(EmaFactory.createFieldEntry().intValue(1, 6560));
        fieldList.add(EmaFactory.createFieldEntry().intValue(2, 66));
        fieldList.add(EmaFactory.createFieldEntry().intValue(3855, 52832001));
        fieldList.add(EmaFactory.createFieldEntry().rmtes(296, ByteBuffer.wrap("BOS".getBytes())));
        fieldList.add(EmaFactory.createFieldEntry().time(375, 21, 0));
        fieldList.add(EmaFactory.createFieldEntry().time(1025, 14, 40, 32));
        fieldList.add(EmaFactory.createFieldEntry().real(22, 14400, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add(EmaFactory.createFieldEntry().real(25, 14700, OmmReal.MagnitudeType.EXPONENT_NEG_2));
        fieldList.add(EmaFactory.createFieldEntry().real(30, 9, OmmReal.MagnitudeType.EXPONENT_0));
        fieldList.add(EmaFactory.createFieldEntry().real(31, 19, OmmReal.MagnitudeType.EXPONENT_0));

        event.provider().submit(EmaFactory.createRefreshMsg().name(reqMsg.name()).serviceName(reqMsg.serviceName()).solicited(true).itemGroup(IProvider.groupId)
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
    public class TestFlags
    {
        final static int serviceDown = 0x0000000000000001;
        final static int openWindowsAndLoadFactor = 0x0000000000000002;
        final static int mergedToGroup = 0x0000000000000004;
        final static int groupStatusDataOk = 0x0000000000000008;
        final static int groupStatusDataSuspect = 0x0000000000000010;
        final static int groupStatusClosedRecover = 0x0000000000000020;
        final static int changeDataType = 0x0000000000000040;
        final static int chagneLinkTypeAndStateAndCode = 0x0000000000000080;
        final static int infoAddCapabilities = 0x0000000000000100;
        final static int sequencedMulticast = 0x0000000000000200;
        final static int acceptingRequest = 0x0000000000000400;
        final static int mergedToGroupAndSendGroupStatusClosedRecover = 0x0000000000000800;
        final static int serviceDelete = 0x0000000000001000;
    }

    private static int testFlag = 0;
    public static ByteBuffer groupId = ByteBuffer.wrap("00".getBytes());
    private static int numberOfUpdates = 600;
    private static String link1 = new String("Link1");
    private static String link2 = new String("Link2");

    private static String printUsage()
    {
        StringBuilder text = new StringBuilder();
        text.append("Usage: SourceDirectoryUpdate [UpdateInfo]\n\n").append("\t -serviceDown \n").append("\t -openWindowsAndLoadFactor \n").append("\t -mergedToGroup \n")
                .append("\t -mergedToGroupAndSendGroupStatusClosedRecover \n").append("\t -groupStatusDataOk \n").append("\t -groupStatusDataSuspect \n").append("\t -groupStatusClosedRecover \n")
                .append("\t -changeDataType \n").append("\t -chagneLinkTypeAndStateAndCode \n").append("\t -infoAddCapabilities \n").append("\t -MCastInformation \n")
                .append("\t -sequencedMulticast \n").append("\t -serviceDelete \n").append("\t -acceptingRequest \n");

        return text.toString();
    }

    private static void addSourceDirectory(OmmProvider provider, long handle)
    {
        switch (testFlag)
        {
            case TestFlags.serviceDown:
            {
                // Do nothing for the state filter
            }
                break;
            case TestFlags.openWindowsAndLoadFactor:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_OPEN_LIMIT, 100));
                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_OPEN_WINDOW, 50));
                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LOAD_FACT, 5555));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_LOAD_ID, FilterEntry.FilterAction.SET, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_LOAD_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.mergedToGroup:
            case TestFlags.mergedToGroupAndSendGroupStatusClosedRecover:
            case TestFlags.groupStatusClosedRecover:
            case TestFlags.groupStatusDataSuspect:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_GROUP, groupId));
                elementList.add(EmaFactory.createElementEntry().state(EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, ""));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.SET, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_GROUP_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.groupStatusDataOk:
            { // Send initial state as suspect

                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_GROUP, groupId));
                elementList.add(EmaFactory.createElementEntry().state(EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.SET, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_GROUP_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.changeDataType:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TYPE, 1));
                elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_DATA, "Data"));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_DATA_ID, FilterEntry.FilterAction.SET, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_DATA_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.chagneLinkTypeAndStateAndCode:
            {
                ElementList elementListLink1 = EmaFactory.createElementList();

                elementListLink1.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TYPE, 1));
                elementListLink1.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_STATE, 1));
                elementListLink1.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_CODE, 1));
                elementListLink1.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_TEXT, "Text of Link1"));

                ElementList elementListLink2 = EmaFactory.createElementList();

                elementListLink2.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TYPE, 2));
                elementListLink2.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_STATE, 1));
                elementListLink2.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_CODE, 1));
                elementListLink2.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_TEXT, "Text of Link2"));

                Map linkMap = EmaFactory.createMap();

                linkMap.add(EmaFactory.createMapEntry().keyAscii(link1, MapEntry.MapAction.ADD, elementListLink1));
                linkMap.add(EmaFactory.createMapEntry().keyAscii(link2, MapEntry.MapAction.ADD, elementListLink2));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().map(EmaRdm.SERVICE_LINK_ID, FilterEntry.FilterAction.SET, linkMap));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_LINK_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.sequencedMulticast:
            {
                ElementList mCastEntry = EmaFactory.createElementList();

                mCastEntry.add(EmaFactory.createElementEntry().ascii("MulticastGroup", "224.1.62.2"));
                mCastEntry.add(EmaFactory.createElementEntry().uintValue("Port", 30001));
                mCastEntry.add(EmaFactory.createElementEntry().uintValue("Domain", EmaRdm.MMT_MARKET_PRICE));

                Vector vectorMCastEntry = EmaFactory.createVector();

                vectorMCastEntry.add(EmaFactory.createVectorEntry().elementList(1, VectorEntry.VectorAction.INSERT, mCastEntry));

                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().ascii("ReferenceDataServerHost", "10.0.1.125"));
                elementList.add(EmaFactory.createElementEntry().uintValue("ReferenceDataServerPort", 19000));
                elementList.add(EmaFactory.createElementEntry().ascii("SnapshotServerHost", "10.0.1.125"));
                elementList.add(EmaFactory.createElementEntry().uintValue("SnapshotServerPort", 14002));
                elementList.add(EmaFactory.createElementEntry().ascii("GapRecoveryServerHost", "10.0.1.125"));
                elementList.add(EmaFactory.createElementEntry().uintValue("GapRecoveryServerPort", 14001));
                elementList.add(EmaFactory.createElementEntry().vector("StreamingMulticastChannels", vectorMCastEntry));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(7, FilterEntry.FilterAction.SET, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(64).payload(map), handle);
            }
                break;
        }
    }

    private static void updateSourceDirectory(OmmProvider provider, long handle)
    {
        switch (testFlag)
        {
            case TestFlags.serviceDelete:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.DELETE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).payload(map), handle);
            }
                break;
            case TestFlags.serviceDown:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_STATE_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.acceptingRequest:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_UP));
                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ACCEPTING_REQS, 0));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_STATE_ID, FilterEntry.FilterAction.UPDATE, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_STATE_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.openWindowsAndLoadFactor:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_OPEN_WINDOW, 90));
                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LOAD_FACT, 9999));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_LOAD_ID, FilterEntry.FilterAction.UPDATE, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_LOAD_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.mergedToGroup:
            case TestFlags.mergedToGroupAndSendGroupStatusClosedRecover:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_GROUP, groupId));

                byte[] buffer = new byte[2];
                buffer[0] = 0;
                buffer[1] = 7;

                elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_MERG_TO_GRP, ByteBuffer.wrap(buffer)));

                elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_GROUP, groupId));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.UPDATE, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_GROUP_FILTER).payload(map), handle);

                if (testFlag == TestFlags.mergedToGroupAndSendGroupStatusClosedRecover)
                {
                    elementList.clear();

                    elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_GROUP, ByteBuffer.wrap(buffer)));
                    elementList.add(EmaFactory.createElementEntry().state(EmaRdm.ENAME_STATUS, OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));

                    filterList.clear();

                    filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.UPDATE, elementList));

                    map.clear();

                    map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                    provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_GROUP_FILTER).payload(map), handle);
                }
            }
                break;
            case TestFlags.groupStatusDataOk:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_GROUP, groupId));
                elementList.add(EmaFactory.createElementEntry().state(EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, ""));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.UPDATE, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_GROUP_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.groupStatusDataSuspect:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_GROUP, groupId));
                elementList.add(EmaFactory.createElementEntry().state(EmaRdm.ENAME_STATUS, OmmState.StreamState.OPEN, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.UPDATE, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_GROUP_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.groupStatusClosedRecover:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_GROUP, groupId));
                elementList.add(EmaFactory.createElementEntry().state(EmaRdm.ENAME_STATUS, OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT, OmmState.StatusCode.NONE, ""));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_GROUP_ID, FilterEntry.FilterAction.UPDATE, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_GROUP_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.changeDataType:
            {
                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TYPE, 0));
                elementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_DATA, "UpdatedData"));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_DATA_ID, FilterEntry.FilterAction.UPDATE, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_DATA_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.chagneLinkTypeAndStateAndCode:
            {
                ElementList elementListLink1 = EmaFactory.createElementList();

                elementListLink1.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TYPE, 1));
                elementListLink1.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_STATE, 0));
                elementListLink1.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_CODE, 2));
                elementListLink1.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_TEXT, "Text of Link1"));

                ElementList elementListLink2 = EmaFactory.createElementList();

                elementListLink2.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TYPE, 2));
                elementListLink2.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_STATE, 0));
                elementListLink2.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_CODE, 2));
                elementListLink2.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_TEXT, "Text of Link2"));

                Map linkMap = EmaFactory.createMap();

                linkMap.add(EmaFactory.createMapEntry().keyAscii(link1, MapEntry.MapAction.UPDATE, elementListLink1));
                linkMap.add(EmaFactory.createMapEntry().keyAscii(link2, MapEntry.MapAction.UPDATE, elementListLink2));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().map(EmaRdm.SERVICE_LINK_ID, FilterEntry.FilterAction.UPDATE, linkMap));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_LINK_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.infoAddCapabilities:
            {
                OmmArray capabilites = EmaFactory.createOmmArray();

                capabilites.add(EmaFactory.createOmmArrayEntry().uintValue(EmaRdm.MMT_MARKET_PRICE));
                capabilites.add(EmaFactory.createOmmArrayEntry().uintValue(EmaRdm.MMT_MARKET_BY_PRICE));
                capabilites.add(EmaFactory.createOmmArrayEntry().uintValue(EmaRdm.MMT_DICTIONARY));
                capabilites.add(EmaFactory.createOmmArrayEntry().uintValue(EmaRdm.MMT_MARKET_MAKER));

                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES, capabilites));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(EmaRdm.SERVICE_INFO_ID, FilterEntry.FilterAction.UPDATE, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(EmaRdm.SERVICE_INFO_FILTER).payload(map), handle);
            }
                break;
            case TestFlags.sequencedMulticast:
            {
                ElementList mCastEntry = EmaFactory.createElementList();

                mCastEntry.add(EmaFactory.createElementEntry().ascii("MulticastGroup", "224.1.62.5"));
                mCastEntry.add(EmaFactory.createElementEntry().uintValue("Port", 50001));
                mCastEntry.add(EmaFactory.createElementEntry().uintValue("Domain", EmaRdm.MMT_MARKET_BY_PRICE));

                Vector vectorMCastEntry = EmaFactory.createVector();

                vectorMCastEntry.add(EmaFactory.createVectorEntry().elementList(1, VectorEntry.VectorAction.UPDATE, mCastEntry));

                ElementList elementList = EmaFactory.createElementList();

                elementList.add(EmaFactory.createElementEntry().ascii("ReferenceDataServerHost", "10.0.5.125"));
                elementList.add(EmaFactory.createElementEntry().uintValue("ReferenceDataServerPort", 19005));
                elementList.add(EmaFactory.createElementEntry().ascii("SnapshotServerHost", "10.0.5.125"));
                elementList.add(EmaFactory.createElementEntry().uintValue("SnapshotServerPort", 15002));
                elementList.add(EmaFactory.createElementEntry().ascii("GapRecoveryServerHost", "10.0.5.125"));
                elementList.add(EmaFactory.createElementEntry().uintValue("GapRecoveryServerPort", 15001));
                elementList.add(EmaFactory.createElementEntry().vector("StreamingMulticastChannels", vectorMCastEntry));

                FilterList filterList = EmaFactory.createFilterList();

                filterList.add(EmaFactory.createFilterEntry().elementList(7, FilterEntry.FilterAction.UPDATE, elementList));

                Map map = EmaFactory.createMap();

                map.add(EmaFactory.createMapEntry().keyUInt(1, MapEntry.MapAction.UPDATE, filterList));

                provider.submit(EmaFactory.createUpdateMsg().domainType(EmaRdm.MMT_DIRECTORY).filter(64).payload(map), handle);
            }
                break;
        }
    }

    public static void main(String[] args)
    {
        if (args.length != 1)
        {
            printUsage();
            return;
        }
        else
        {
            if (args[0].equals("-serviceDown"))
            {
                testFlag |= TestFlags.serviceDown;
            }
            else if (args[0].equals("-openWindowsAndLoadFactor"))
            {
                testFlag |= TestFlags.openWindowsAndLoadFactor;
            }
            else if (args[0].equals("-mergedToGroup"))
            {
                testFlag |= TestFlags.mergedToGroup;

                byte[] buffer = new byte[2];
                buffer[0] = 0;
                buffer[1] = 5;

                groupId = ByteBuffer.wrap(buffer);
            }
            else if (args[0].equals("-mergedToGroupAndSendGroupStatusClosedRecover"))
            {
                testFlag |= TestFlags.mergedToGroupAndSendGroupStatusClosedRecover;

                byte[] buffer = new byte[2];
                buffer[0] = 0;
                buffer[1] = 5;

                groupId = ByteBuffer.wrap(buffer);
            }
            else if (args[0].equals("-groupStatusDataOk"))
            {
                testFlag |= TestFlags.groupStatusDataOk;

                byte[] buffer = new byte[2];

                buffer[0] = 0;
                buffer[1] = 5;

                groupId = ByteBuffer.wrap(buffer);
            }
            else if (args[0].equals("-groupStatusDataSuspect"))
            {
                testFlag |= TestFlags.groupStatusDataSuspect;

                byte[] buffer = new byte[2];

                buffer[0] = 0;
                buffer[1] = 5;

                groupId = ByteBuffer.wrap(buffer);
            }
            else if (args[0].equals("-groupStatusClosedRecover"))
            {
                testFlag |= TestFlags.groupStatusClosedRecover;

                byte[] buffer = new byte[2];

                buffer[0] = 0;
                buffer[1] = 5;

                groupId = ByteBuffer.wrap(buffer);
            }
            else if (args[0].equals("-changeDataType"))
            {
                testFlag |= TestFlags.changeDataType;
            }
            else if (args[0].equals("-chagneLinkTypeAndStateAndCode"))
            {
                testFlag |= TestFlags.chagneLinkTypeAndStateAndCode;
            }
            else if (args[0].equals("-infoAddCapabilities"))
            {
                testFlag |= TestFlags.infoAddCapabilities;
            }
            else if (args[0].equals("-sequencedMulticast"))
            {
                testFlag |= TestFlags.sequencedMulticast;
            }
            else if (args[0].equals("-acceptingRequest"))
            {
                testFlag |= TestFlags.acceptingRequest;
            }
            else if (args[0].equals("-serviceDelete"))
            {
                testFlag |= TestFlags.serviceDelete;
            }

            if (testFlag == 0)
            {
                String errorText = new String("Invalid argument " + args[0] + "\n");
                System.out.println(errorText);

                return;
            }
        }

        OmmProvider provider = null;
        try
        {
            AppClient appClient = new AppClient();
            FieldList fieldList = EmaFactory.createFieldList();

            OmmIProviderConfig config = EmaFactory.createOmmIProviderConfig();

            provider = EmaFactory.createOmmProvider(config.operationModel(OmmIProviderConfig.OperationModel.USER_DISPATCH), appClient);

            while (appClient.itemHandle == 0)
                provider.dispatch(1000);

            for (int index = 0; index < numberOfUpdates; index++)
            {
                fieldList.clear();
                fieldList.add(EmaFactory.createFieldEntry().time(1025, 14, 40, 32));
                fieldList.add(EmaFactory.createFieldEntry().intValue(3855, 52832001));
                fieldList.add(EmaFactory.createFieldEntry().real(22, 14400 + (((appClient.itemHandle & 0x1) == 1) ? 1 : 10), OmmReal.MagnitudeType.EXPONENT_NEG_2));
                fieldList.add(EmaFactory.createFieldEntry().real(30, 10 + (((appClient.itemHandle & 0x1) == 1) ? 10 : 20), OmmReal.MagnitudeType.EXPONENT_0));
                fieldList.add(EmaFactory.createFieldEntry().rmtes(296, ByteBuffer.wrap("NAS".getBytes())));

                if (appClient.submitting)
                {
                    provider.submit(EmaFactory.createUpdateMsg().payload(fieldList), appClient.itemHandle);
                }

                if (index == 1)
                {
                    addSourceDirectory(provider, 0);
                }
                else if (index == 3)
                {
                    updateSourceDirectory(provider, 0);
                    provider.dispatch(1000);
                }

                if (index >= 3)
                {
                    Thread.sleep(2000);
                }

                provider.dispatch(1000);
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
//END APIQA
