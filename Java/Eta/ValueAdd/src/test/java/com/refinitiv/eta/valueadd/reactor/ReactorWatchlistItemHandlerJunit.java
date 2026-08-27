///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2025 LSEG. All rights reserved.
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.ElementNames;
import com.refinitiv.eta.rdm.ViewTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import org.junit.Before;
import org.junit.Test;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import static com.refinitiv.eta.valueadd.reactor.WlItemHandler.compareMsgKeys;
import static org.junit.Assert.*;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.*;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

public class ReactorWatchlistItemHandlerJunit {
    private final Reactor reactor = mock(Reactor.class);
    private final ReactorChannel reactorChannel = mock(ReactorChannel.class);
    private final ConsumerRole consumerRole = mock(ConsumerRole.class);
    private final Watchlist watchlist = mock(Watchlist.class);
    private final WlDirectoryHandler directoryHandler = mock(WlDirectoryHandler.class);
    private final WlService wlService = mock(WlService.class);
    private final ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
    private final ReactorSubmitOptions submitOptions = new ReactorSubmitOptions();
    private final ConsumerWatchlistOptions consumerWatchlistOptions = new ConsumerWatchlistOptions();
    private final Service.ServiceState serviceState = new Service.ServiceState();
    private final Service.ServiceInfo serviceInfo = new Service.ServiceInfo();
    private final Service rdmService = DirectoryMsgFactory.createService();
    private final List<Long> capabilitiesList = new ArrayList<>();
    private final WlStream stream = ReactorFactory.createWlStream();
    private final WlRequest wlRequest = ReactorFactory.createWlRequest();
    private final RequestMsg requestMsg = (RequestMsg) CodecFactory.createMsg();
    private final MsgKey key1 = CodecFactory.createMsgKey();
    private final MsgKey key2 = CodecFactory.createMsgKey();
    private final boolean isReissue = true;

    private WlItemHandler itemHandler;
    private RequestMsg wlRequestMsg;

    private static final int STREAM_ID = 5;

    @Before
    public void init() {
        // Prepare mocks & stabs
        when(reactor.populateErrorInfo(any(ReactorErrorInfo.class), anyInt(), anyString(), anyString())).thenCallRealMethod();

        consumerWatchlistOptions.obeyOpenWindow(false);

        serviceState.serviceState(1);

        capabilitiesList.add((long) DomainTypes.MARKET_PRICE);
        serviceInfo.capabilitiesList(capabilitiesList);

        rdmService.applyHasState();
        rdmService.applyHasInfo();
        rdmService.state(serviceState);
        rdmService.info(serviceInfo);

        when(reactorChannel.majorVersion()).thenReturn(14);
        when(reactorChannel.minorVersion()).thenReturn(1);
        when(reactorChannel.state()).thenReturn(ReactorChannel.State.UP);

        when(consumerRole.watchlistOptions()).thenReturn(consumerWatchlistOptions);

        when(watchlist.role()).thenReturn(consumerRole);
        when(watchlist.reactor()).thenReturn(reactor);
        when(watchlist.reactorChannel()).thenReturn(reactorChannel);
        when(watchlist.directoryHandler()).thenReturn(directoryHandler);
        when(watchlist.watchlistOptions()).thenReturn(consumerWatchlistOptions);

        when(directoryHandler.service(1)).thenReturn(wlService);

        when(wlService.rdmService()).thenReturn(rdmService);

        itemHandler = new  WlItemHandler(watchlist);

        wlRequest.stream(stream);
        wlRequestMsg =  wlRequest.requestMsg();
        wlRequestMsg.msgClass(MsgClasses.REQUEST);
        wlRequestMsg.domainType(DomainTypes.MARKET_PRICE);

        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);

        // Prepare message keys
        key1.clear();
        key2.clear();

        Buffer name1 = CodecFactory.createBuffer();
        name1.data("etaj");
        Buffer name2 = CodecFactory.createBuffer();
        ByteBuffer bb = ByteBuffer.allocate(7);
        byte[] bts = { 1, 2, 0, 10, 57, 0x7F, 4 };
        bb.put(bts);
        name2.data(bb, 0, 7);
        Buffer attrib = CodecFactory.createBuffer();
        ByteBuffer bb1 = ByteBuffer.allocate(4);
        byte[] bts1 = { 4, 8, 5, 10 };
        bb1.put(bts1);
        name2.data(bb1, 0, 4);

        key1.applyHasFilter();
        key1.filter(67);
        key1.applyHasName();
        key1.name(name1);
        key1.applyHasAttrib();
        key1.encodedAttrib(attrib);
        key1.applyHasIdentifier();
        key1.identifier(7);
        key1.applyHasNameType();
        key1.nameType(4);
        key1.applyHasServiceId();
        key1.serviceId(667);
        key1.attribContainerType();

        key1.copy(key2);
    }

    @Test
    public void compareMsgKeys_EqualKeys_ReturnTrue()
    {
        assertTrue(compareMsgKeys(key1, key2));
        assertTrue(compareMsgKeys(key2, key1));
    }

    @Test
    public void compareMsgKeys_KeysWithDifferentNames_ReturnFalse()
    {
        Buffer name1 = CodecFactory.createBuffer();
        name1.data("emaj");
        key1.name(name1);

        assertFalse(compareMsgKeys(key1, key2));
        assertFalse(compareMsgKeys(key2, key1));
    }

    @Test
    public void compareMsgKeys_OnlyOneKeyContainsName_ReturnTrue()
    {
        Buffer name1 = CodecFactory.createBuffer();
        name1.data("emaj");
        key1.name(name1);
        key1.flags(key1.flags() & ~MsgKeyFlags.HAS_NAME);

        assertTrue(compareMsgKeys(key1, key2));
        assertTrue(compareMsgKeys(key2, key1));
    }

    @Test
    public void compareMsgKeys_KeysWithoutNames_ReturnTrue()
    {
        Buffer name1 = CodecFactory.createBuffer();
        name1.data("emaj");
        key1.name(name1);
        key1.flags(key1.flags() & ~MsgKeyFlags.HAS_NAME);
        key2.flags(key2.flags() & ~MsgKeyFlags.HAS_NAME);

        assertTrue(compareMsgKeys(key1, key2));
    }

    @Test
    public void compareMsgKeys_KeysWithDifferentServiceId_ReturnTrue()
    {
        key1.serviceId(1);
        key2.serviceId(2);

        assertTrue(compareMsgKeys(key1, key2));
        assertTrue(compareMsgKeys(key2, key1));
    }

    @Test
    public void submitRequest_ReissueDifferentDomainTypes_ReturnFailure() {
        requestMsg.domainType(DomainTypes.SYMBOL_LIST);

        // Call target method
        int returnValue = itemHandler.submitRequest(wlRequest, requestMsg, isReissue, submitOptions, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, returnValue);
        assertEquals("WlItemHandler.handleReissue", errorInfo.location());
        assertEquals("Domain type does not match existing request.", errorInfo.error().text());
    }

    @Test
    public void submitRequest_ReissueRequestMessageHasBatch_ReturnFailure() {
        requestMsg.applyHasBatch();

        // Call target method
        int returnValue = itemHandler.submitRequest(wlRequest, requestMsg, isReissue, submitOptions, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, returnValue);
        assertEquals("WlItemHandler.handleReissue", errorInfo.location());
        assertEquals("Request reissue may not contain batch flag.", errorInfo.error().text());
    }

    @Test
    public void submitRequest_ReissueRequestMessageAddPrivateStreamFlag_ReturnFailure() {
        requestMsg.applyPrivateStream();

        // Call target method
        int returnValue = itemHandler.submitRequest(wlRequest, requestMsg, isReissue, submitOptions, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, returnValue);
        assertEquals("WlItemHandler.handleReissue", errorInfo.location());
        assertEquals("Request reissue may not add private stream flag.", errorInfo.error().text());
    }

    @Test
    public void submitRequest_ReissueDifferentMessageKeys_ReturnFailure() {
        Buffer name1 = CodecFactory.createBuffer();
        name1.data("emaj");
        key1.name(name1);

        MsgKey wlRequestMsgKey = wlRequestMsg.msgKey();
        key1.copy(wlRequestMsgKey);

        // Create request message
        MsgKey requestMsgKey = wlRequestMsg.msgKey();
        key2.copy(requestMsgKey);

        // Call target method
        int returnValue = itemHandler.submitRequest(wlRequest, requestMsg, isReissue, submitOptions, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, returnValue);
        assertEquals("WlItemHandler.handleReissue", errorInfo.location());
        assertEquals("Message key does not match existing request.", errorInfo.error().text());
    }

    @Test
    public void submitRequest_ReissueOnlyOneRequestMsgHasQos_ReturnFailure() {
        wlRequestMsg.applyHasQos();
        Qos wlRequestMsgQos = wlRequestMsg.qos();
        wlRequestMsgQos.rate(QosRates.TIME_CONFLATED);
        wlRequestMsgQos.rateInfo(65532);
        wlRequestMsgQos.timeliness(QosTimeliness.DELAYED);

        // Call target method
        int returnValue = itemHandler.submitRequest(wlRequest, requestMsg, isReissue, submitOptions, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, returnValue);
        assertEquals("WlItemHandler.handleReissue", errorInfo.location());
        assertEquals("QoS does not match existing request.", errorInfo.error().text());
    }

    @Test
    public void submitRequest_ReissueDifferentQos_ReturnFailure() {
        wlRequestMsg.applyHasQos();
        Qos wlRequestMsgQos = wlRequestMsg.qos();
        wlRequestMsgQos.rate(QosRates.TIME_CONFLATED);
        wlRequestMsgQos.rateInfo(65532);
        wlRequestMsgQos.timeliness(QosTimeliness.DELAYED);

        requestMsg.applyHasQos();
        Qos requestMsgQos = wlRequestMsg.qos();
        requestMsgQos.rate(QosRates.TIME_CONFLATED);
        requestMsgQos.rateInfo(65534);
        requestMsgQos.timeliness(QosTimeliness.DELAYED);

        // Call target method
        int returnValue = itemHandler.submitRequest(wlRequest, requestMsg, isReissue, submitOptions, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, returnValue);
        assertEquals("WlItemHandler.handleReissue", errorInfo.location());
        assertEquals("QoS does not match existing request.", errorInfo.error().text());
    }

    @Test
    public void submitRequest_ReissueOnlyOneRequestMsgHasWorstQos_ReturnFailure() {
        wlRequestMsg.applyHasWorstQos();
        Qos wlRequestMsgQos = wlRequestMsg.worstQos();
        wlRequestMsgQos.rate(QosRates.TIME_CONFLATED);
        wlRequestMsgQos.rateInfo(65532);
        wlRequestMsgQos.timeliness(QosTimeliness.DELAYED);

        // Call target method
        int returnValue = itemHandler.submitRequest(wlRequest, requestMsg, isReissue, submitOptions, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, returnValue);
        assertEquals("WlItemHandler.handleReissue", errorInfo.location());
        assertEquals("Worst QoS does not match existing request.", errorInfo.error().text());
    }

    @Test
    public void submitRequest_ReissueDifferentWorstQos_ReturnFailure() {
        wlRequestMsg.applyHasWorstQos();
        Qos wlRequestMsgQos = wlRequestMsg.worstQos();
        wlRequestMsgQos.rate(QosRates.TIME_CONFLATED);
        wlRequestMsgQos.rateInfo(65532);
        wlRequestMsgQos.timeliness(QosTimeliness.DELAYED);

        requestMsg.applyHasWorstQos();
        Qos requestMsgQos = wlRequestMsg.worstQos();
        requestMsgQos.rate(QosRates.TIME_CONFLATED);
        requestMsgQos.rateInfo(65534);
        requestMsgQos.timeliness(QosTimeliness.DELAYED);

        // Call target method
        int returnValue = itemHandler.submitRequest(wlRequest, requestMsg, isReissue, submitOptions, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, returnValue);
        assertEquals("WlItemHandler.handleReissue", errorInfo.location());
        assertEquals("Worst QoS does not match existing request.", errorInfo.error().text());
    }

    @Test
    public void handleRequest_TwoSnapshotRequestsWithSameView_BothAddedIntoUserRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(false, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Create second request message
        prepareRequestMsg(false, viewFieldList, requestMsg);
        
        /* The Watchlist component creates an unique  WlRequest for each  user request.*/
        WlRequest wlRequest2 = ReactorFactory.createWlRequest();

        // Call target method
        itemHandler.handleRequest(wlRequest2, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(2, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_TwoSnapshotRequestsWithDifferentViews_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare first view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(false, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Prepare second view
        prepareView(viewFieldList, 0, 6);

        // Create second request message
        prepareRequestMsg(false, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_FirstStreamingRequestSecondSnapshotRequestWithSameView_BothAddedIntoUserRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(true, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Create second request message
        prepareRequestMsg(false, viewFieldList, requestMsg); 
        
        /* The Watchlist component creates an unique  WlRequest for each  user request.*/
        WlRequest wlRequest2 = ReactorFactory.createWlRequest();

        // Call target method
        itemHandler.handleRequest(wlRequest2, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(2, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_FirstStreamingRequestSecondSnapshotRequestWithDifferentViews_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare first view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(true, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Prepare second view
        prepareView(viewFieldList, 0, 6);

        // Create second request message
        prepareRequestMsg(false, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_FirstSnapshotRequestSecondStreamingRequestWithSameView_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(false, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Create second request message
        prepareRequestMsg(true, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_FirstSnapshotRequestSecondStreamingRequestWithDifferentViews_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare first view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(false, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Prepare second view
        prepareView(viewFieldList, 0, 6);

        // Create second request message
        prepareRequestMsg(true, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_TwoStreamingRequestsWithSameView_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(true, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Create second request message
        prepareRequestMsg(true, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    @Test
    public void handleRequest_TwoStreamingRequestsWithDifferentViews_FirstAddedIntoUserRequestListSecondAddedIntoWaitingRequestList() {
        WlItemHandler itemHandler = new  WlItemHandler(watchlist);

        // Prepare first view
        List<Integer> viewFieldList = new ArrayList<>();
        prepareView(viewFieldList, 22, 25);

        // Create first request message
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        prepareRequestMsg(true, viewFieldList, requestMsg);

        WlRequest wlRequest = ReactorFactory.createWlRequest();
        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(0, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());

        wlRequest.stream()._requestPending = true;
        wlRequest.stream()._refreshState = WlStream.RefreshStates.REFRESH_VIEW_PENDING;

        // Prepare second view
        prepareView(viewFieldList, 0, 6);

        // Create second request message
        prepareRequestMsg(true, viewFieldList, requestMsg);

        // Call target method
        itemHandler.handleRequest(wlRequest, requestMsg, submitOptions, false, errorInfo);

        assertEquals(WlRequest.State.PENDING_REQUEST, wlRequest.state());
        assertEquals(1, wlRequest.stream().waitingRequestList().size());
        assertEquals(1, wlRequest.stream().userRequestList().size());
    }

    private void prepareView(List<Integer> viewFieldList, int... values) {
        if(viewFieldList == null) {
            return;
        }

        viewFieldList.clear();
        for(int value : values) {
            viewFieldList.add(value);
        }
    }

    private void prepareRequestMsg(boolean isStreaming, List<Integer> viewFieldList, RequestMsg requestMsg) {
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        if(isStreaming) {
            requestMsg.applyStreaming();
        }
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().applyHasServiceId();
        requestMsg.applyHasView();
        requestMsg.msgKey().name().data("TRI.N");
        requestMsg.msgKey().serviceId(1);
        encodeViewFieldIdList(reactorChannel, viewFieldList, requestMsg);
    }

   private static void encodeViewFieldIdList(ReactorChannel rc, List<Integer> fieldIdList, RequestMsg msg)
    {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        Int tempInt = CodecFactory.createInt();
        UInt tempUInt = CodecFactory.createUInt();
        Array viewArray = CodecFactory.createArray();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        encodeIter.setBufferAndRWFVersion(buf, rc.majorVersion(), rc.minorVersion());
        ElementList elementList = CodecFactory.createElementList();
        ElementEntry elementEntry = CodecFactory.createElementEntry();
        ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

        elementList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(encodeIter, null, 0));

        elementEntry.clear();
        elementEntry.name(ElementNames.VIEW_TYPE);
        elementEntry.dataType(DataTypes.UINT);

        tempUInt.value(ViewTypes.FIELD_ID_LIST);

        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(encodeIter, tempUInt));

        elementEntry.clear();
        elementEntry.name(ElementNames.VIEW_DATA);
        elementEntry.dataType(DataTypes.ARRAY);
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeInit(encodeIter, 0));
        viewArray.primitiveType(DataTypes.INT);
        viewArray.itemLength(2);

        assertEquals(CodecReturnCodes.SUCCESS, viewArray.encodeInit(encodeIter));

        for (Integer viewField : fieldIdList)
        {
            arrayEntry.clear();
            tempInt.value(viewField);
            assertEquals(CodecReturnCodes.SUCCESS, arrayEntry.encode(encodeIter, tempInt));
        }
        assertEquals(CodecReturnCodes.SUCCESS, viewArray.encodeComplete(encodeIter, true));
        assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeComplete(encodeIter, true));
        assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(encodeIter, true));

        msg.containerType(DataTypes.ELEMENT_LIST);
        msg.encodedDataBody(buf);
    }

    @Test
    public void handleReissue_wlRequestStreamIsNull_ReturnSuccess() {
        WlItemHandler wlItemHandler = new WlItemHandler(watchlist);

        WlStream wlStream = new WlStream();
        wlStream.watchlist(watchlist);
        WlRequest wlRequest = ReactorFactory.createWlRequest();
        wlRequest.state(WlRequest.State.OPEN);
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        requestMsg.streamId(STREAM_ID);
        requestMsg.msgClass(MsgClasses.REQUEST);
        wlRequest._requestMsg = requestMsg;
        wlStream.userRequestList().add(wlRequest);

        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();

        assertNull(wlRequest.stream());

        // It emulates situation when consumer sends item request one more time after receiving
        // status message with stream closed recoverable state
        int ret = wlItemHandler.handleReissue(wlRequest, requestMsg, submitOptions, errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
    }
}
