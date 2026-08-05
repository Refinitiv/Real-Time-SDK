/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025,2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.*;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Directory;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.*;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service.ServiceFlags;
import org.junit.Before;
import org.junit.Test;
import org.mockito.ArgumentCaptor;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyString;
import static org.mockito.Mockito.*;

public class WlDirectoryHandlerJunit
{
    private final Reactor reactor = mock(Reactor.class);
    private final Watchlist watchlist = mock(Watchlist.class);

    private final DecodeIterator decodeIter = CodecFactory.createDecodeIterator();
    private final EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

    private static final int DIRECTORY_STREAM_ID = 2;

    @Before
    public void init()
    {
        decodeIter.clear();
        encodeIter.clear();

        // Prepare mocks & stabs
        when(reactor.sendAndHandleDirectoryMsgCallback(anyString(), any(ReactorChannel.class),
                            any(TransportBuffer.class), any(Msg.class), any(DirectoryMsg.class),
                            any(WlRequest.class), any(ReactorErrorInfo.class)))
                .thenReturn(ReactorCallbackReturnCodes.SUCCESS);
        when(reactor.sendAndHandleDefaultMsgCallback(anyString(), any(ReactorChannel.class),
                any(TransportBuffer.class), any(Msg.class), any(WlRequest.class), any(ReactorErrorInfo.class)))
                .thenReturn(ReactorCallbackReturnCodes.SUCCESS);

        when(watchlist.reactor()).thenReturn(reactor);
    }

    @Test
    public void givenGenericMessage_whenReadGenericMsg_thenDefaultMsgCallbackIsInvoked()
    {
        WlDirectoryHandler wlDirectoryHandler = new WlDirectoryHandler(watchlist);

        WlStream wlStream = new WlStream();
        wlStream.watchlist(watchlist);
        WlRequest wlRequest = ReactorFactory.createWlRequest();
        wlRequest.state(WlRequest.State.OPEN);
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        requestMsg.streamId(DIRECTORY_STREAM_ID);
        wlRequest._requestMsg = requestMsg;
        wlStream.userRequestList().add(wlRequest);

        ReactorErrorInfo errorInfo = new ReactorErrorInfo();

        Msg msg = TestUtil.createGenericMessage(DomainTypes.SOURCE, DIRECTORY_STREAM_ID);

        int ret = wlDirectoryHandler.readGenericMsg(wlStream, decodeIter, msg, errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        verify(reactor, times(1))
                .sendAndHandleDefaultMsgCallback(eq("WLDirectoryHandler.readGenericMsg"),
                nullable(ReactorChannel.class), nullable(TransportBuffer.class), any(Msg.class),
                any(WlRequest.class), any(ReactorErrorInfo.class));

    }

    @Test
    public void givenDirectoryConsumerStatusMessage_whenReadGenericMsg_thenDirectoryMsgCallbackIsInvoked()
    {
        WlDirectoryHandler wlDirectoryHandler = new WlDirectoryHandler(watchlist);

        WlStream wlStream = new WlStream();
        wlStream.watchlist(watchlist);
        WlRequest wlRequest = ReactorFactory.createWlRequest();
        wlRequest.state(WlRequest.State.OPEN);
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        requestMsg.streamId(DIRECTORY_STREAM_ID);
        wlRequest._requestMsg = requestMsg;
        wlStream.userRequestList().add(wlRequest);

        ReactorErrorInfo errorInfo = new ReactorErrorInfo();

        Msg msg = TestUtil.createDirectoryCSMessage(encodeIter, decodeIter, DIRECTORY_STREAM_ID);

        int ret = wlDirectoryHandler.readGenericMsg(wlStream, decodeIter, msg, errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        verify(reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WLDirectoryHandler.readGenericMsg"),
                        nullable(ReactorChannel.class), nullable(TransportBuffer.class), any(Msg.class),
                        any(DirectoryMsg.class), any(WlRequest.class), any(ReactorErrorInfo.class));

    }

    @Test
    public void givenRequestFilterAndCachedServices_whenReadRefreshMsgWithReceivedRefresh_thenResultingUpdateUsesRequestFilterAndCurrentCache()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedServiceWithLoad(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString(), 25),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                10,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                false,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedInfoOnlyService(3, "THIRD_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(3, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNotNull(context.handler.service(2));
        assertNotNull(context.handler.service(3));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, null);

        assertEquals(10, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackUpdate.filter());
        assertEquals(2, callbackUpdate.serviceList().size());

        Service service1 = serviceById(callbackUpdate.serviceList(), 1);
        Service service3 = serviceById(callbackUpdate.serviceList(), 3);
        assertNotNull(service1);
        assertNotNull(service3);

        assertTrue(service1.checkHasInfo());
        assertTrue(service1.checkHasState());
        assertFalse(service1.checkHasLoad());

        assertTrue(service3.checkHasInfo());
        assertFalse(service3.checkHasState());
        assertFalse(service3.checkHasLoad());
    }

    @Test
    public void givenServiceIdAndLoadFilter_whenReadRefreshMsgWithReceivedRefreshWithoutLoadFilter_thenResultingUpdateUsesServiceWithNoFilters()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedServiceWithLoad(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString(), 55),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                11,
                Directory.ServiceFilterFlags.LOAD,
                1);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                false,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                updatedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(0, callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertEquals(11, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(0, callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service selectedService = callbackUpdate.serviceList().get(0);
        assertEquals(1, selectedService.serviceId());
        assertFalse(selectedService.checkHasInfo());
        assertFalse(selectedService.checkHasState());
        assertFalse(selectedService.checkHasLoad());
    }

    @Test
    public void givenUnsolicitedClearCacheRefresh_whenReadRefreshMsgWithReceivedRefresh_thenCacheIsRebuiltAndRefreshIsFannedOutAsUpdate()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheDefaultServicesAndClearInvocations(context);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                12,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO,
                true,
                false,
                addedInfoOnlyService(3, "UNSOLICITED_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertNotNull(context.handler.service(3));
        assertEquals(1, context.handler.serviceList().size());

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));
        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(1)).serviceAdded(any(WlService.class));

        UpdateMsg refreshAsUpdateMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate refreshAsUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();
        assertEquals(MsgClasses.UPDATE, refreshAsUpdateMsg.msgClass());
        assertTrue(refreshAsUpdateMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO, refreshAsUpdateMsg.msgKey().filter());
        assertUpdateServiceId(refreshAsUpdateMsg, refreshAsUpdate, null);
        assertTrue(refreshAsUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO, refreshAsUpdate.filter());
        assertEquals(12, refreshAsUpdate.streamId());
        assertEquals(1, refreshAsUpdate.serviceList().size());

        Service newService = refreshAsUpdate.serviceList().get(0);
        assertEquals(3, newService.serviceId());
        assertEquals(MapEntryActions.ADD, newService.action());
        assertTrue(newService.checkHasInfo());
        assertFalse(newService.checkHasState());
        assertFalse(newService.checkHasLoad());
    }

    @Test
    public void givenAllServicesRequestAndMixedUnsolicitedClearCacheRefresh_whenReadRefreshMsgWithReceivedRefresh_thenOnlyAddedServicesAreFannedOutAsUpdate()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                17,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                true,
                false,
                updatedStateOnlyService(1, Provider.defaultService().info().serviceName().toString()),
                addedInfoOnlyService(3, "UNSOLICITED_ADDED_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertNotNull(context.handler.service(3));
        assertEquals(2, context.handler.serviceList().size());

        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(2)).serviceAdded(any(WlService.class));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, null);

        assertEquals(17, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO,
                callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service addedService = callbackUpdate.serviceList().get(0);
        assertEquals(3, addedService.serviceId());
        assertEquals(MapEntryActions.ADD, addedService.action());
        assertTrue(addedService.checkHasInfo());
        assertFalse(addedService.checkHasState());
    }

    @Test
    public void givenUnsolicitedClearCacheRefresh_whenReadRefreshMsgWithReceivedRefresh_thenHandleCloseUpdateIsGeneratedBeforeRefreshAsUpdate()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();
        List<String> callbackLocations = new ArrayList<>();
        List<Msg> callbackMsgs = new ArrayList<>();
        List<DirectoryMsg> callbackDirectoryMsgs = new ArrayList<>();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        doAnswer(invocation -> {
            callbackLocations.add(invocation.getArgument(0));

            Msg copiedMsg = CodecFactory.createMsg();
            ((Msg) invocation.getArgument(3)).copy(copiedMsg, CopyMsgFlags.ALL_FLAGS);
            callbackMsgs.add(copiedMsg);

            DirectoryMsg copiedDirectoryMsg = DirectoryMsgFactory.createMsg();
            TestUtil.copyDirectoryMsg(invocation.getArgument(4), copiedDirectoryMsg);
            callbackDirectoryMsgs.add(copiedDirectoryMsg);

            return ReactorCallbackReturnCodes.SUCCESS;
        }).when(context.reactor).sendAndHandleDirectoryMsgCallback(anyString(), any(ReactorChannel.class),
                nullable(TransportBuffer.class), any(Msg.class), any(DirectoryMsg.class),
                any(WlRequest.class), any(ReactorErrorInfo.class));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                16,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO,
                true,
                false,
                addedInfoOnlyService(3, "UNSOLICITED_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());

        assertEquals(2, callbackLocations.size());
        assertEquals("WlDirectoryHandler.deleteAllServices", callbackLocations.get(0));
        assertEquals("WlDirectoryHandler.readRefreshMsg", callbackLocations.get(1));

        UpdateMsg handleCloseMsg = (UpdateMsg) callbackMsgs.get(0);
        DirectoryUpdate handleCloseUpdate = (DirectoryUpdate) callbackDirectoryMsgs.get(0);

        assertEquals(MsgClasses.UPDATE, handleCloseMsg.msgClass());
        assertFalse(handleCloseMsg.checkHasMsgKey());
        assertEquals(16, handleCloseUpdate.streamId());
        assertFalse(handleCloseUpdate.checkHasFilter());
        assertFalse(handleCloseUpdate.checkHasServiceId());
        assertEquals(2, handleCloseUpdate.serviceList().size());
        assertEquals(1, handleCloseUpdate.serviceList().get(0).serviceId());
        assertEquals(MapEntryActions.DELETE, handleCloseUpdate.serviceList().get(0).action());
        assertEquals(2, handleCloseUpdate.serviceList().get(1).serviceId());
        assertEquals(MapEntryActions.DELETE, handleCloseUpdate.serviceList().get(1).action());

        UpdateMsg refreshAsUpdateMsg = (UpdateMsg) callbackMsgs.get(1);
        DirectoryUpdate refreshAsUpdate = (DirectoryUpdate) callbackDirectoryMsgs.get(1);

        assertEquals(MsgClasses.UPDATE, refreshAsUpdateMsg.msgClass());
        assertTrue(refreshAsUpdateMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO, refreshAsUpdateMsg.msgKey().filter());
        assertUpdateServiceId(refreshAsUpdateMsg, refreshAsUpdate, null);
        assertTrue(refreshAsUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO, refreshAsUpdate.filter());
        assertEquals(16, refreshAsUpdate.streamId());
        assertEquals(1, refreshAsUpdate.serviceList().size());
        assertEquals(3, refreshAsUpdate.serviceList().get(0).serviceId());
    }

    @Test
    public void givenRequestWithServiceIdAndUnsolicitedClearCache_whenReadRefreshMsgWithReceivedRefresh_thenCacheIsRebuiltAndRefreshIsFannedOutAsUpdate()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                14,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                1);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.STATE,
                true,
                false,
                addedStateOnlyService(1, Provider.defaultService().info().serviceName().toString()));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertNull(context.handler.service(Provider.defaultService().info().serviceName().toString()));
        assertEquals(1, context.handler.serviceList().size());

        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(1)).serviceAdded(any(WlService.class));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.STATE, callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.STATE, callbackUpdate.filter());
        assertEquals(14, callbackUpdate.streamId());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service rebuiltService = callbackUpdate.serviceList().get(0);
        assertEquals(1, rebuiltService.serviceId());
        assertEquals(MapEntryActions.ADD, rebuiltService.action());
        assertFalse(rebuiltService.checkHasInfo());
        assertTrue(rebuiltService.checkHasState());
        assertFalse(rebuiltService.checkHasLoad());
    }

    @Test
    public void givenRequestWithServiceIdAndUnsolicitedClearCacheUpdateForRequestedService_whenReadRefreshMsgWithoutReceivedRefresh_thenOnlyHandleCloseIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);
        List<String> callbackLocations = new ArrayList<>();
        List<Msg> callbackMsgs = new ArrayList<>();
        List<DirectoryMsg> callbackDirectoryMsgs = new ArrayList<>();

        captureDirectoryCallbacks(context, callbackLocations, callbackMsgs, callbackDirectoryMsgs);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                29,
                Directory.ServiceFilterFlags.STATE,
                1);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.STATE,
                true,
                false,
                updatedStateOnlyService(1, Provider.defaultService().info().serviceName().toString()));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertEquals(1, context.handler.serviceList().size());
        assertEquals(MapEntryActions.UPDATE, context.handler.service(1).rdmService().action());

        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(1)).serviceAdded(any(WlService.class));

        assertEquals(1, callbackLocations.size());
        assertEquals("WlDirectoryHandler.deleteAllServices", callbackLocations.get(0));

        UpdateMsg handleCloseMsg = (UpdateMsg) callbackMsgs.get(0);
        DirectoryUpdate handleCloseUpdate = (DirectoryUpdate) callbackDirectoryMsgs.get(0);

        assertEquals(MsgClasses.UPDATE, handleCloseMsg.msgClass());
        assertEquals(29, handleCloseUpdate.streamId());
        assertTrue(handleCloseUpdate.checkHasServiceId());
        assertEquals(1, handleCloseUpdate.serviceId());
        assertEquals(1, handleCloseUpdate.serviceList().size());
        assertEquals(1, handleCloseUpdate.serviceList().get(0).serviceId());
        assertEquals(MapEntryActions.DELETE, handleCloseUpdate.serviceList().get(0).action());
    }

    @Test
    public void givenRequestWithServiceIdAndUnsolicitedClearCacheUpdateForRequestedService_whenReadRefreshMsgWithReceivedRefresh_thenOnlyHandleCloseIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();
        List<String> callbackLocations = new ArrayList<>();
        List<Msg> callbackMsgs = new ArrayList<>();
        List<DirectoryMsg> callbackDirectoryMsgs = new ArrayList<>();

        captureDirectoryCallbacks(context, callbackLocations, callbackMsgs, callbackDirectoryMsgs);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                18,
                Directory.ServiceFilterFlags.STATE,
                1);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.STATE,
                true,
                false,
                updatedStateOnlyService(1, Provider.defaultService().info().serviceName().toString()));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertEquals(1, context.handler.serviceList().size());
        assertEquals(MapEntryActions.UPDATE, context.handler.service(1).rdmService().action());

        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(1)).serviceAdded(any(WlService.class));

        assertEquals(1, callbackLocations.size());
        assertEquals("WlDirectoryHandler.deleteAllServices", callbackLocations.get(0));

        UpdateMsg handleCloseMsg = (UpdateMsg) callbackMsgs.get(0);
        DirectoryUpdate handleCloseUpdate = (DirectoryUpdate) callbackDirectoryMsgs.get(0);

        assertEquals(MsgClasses.UPDATE, handleCloseMsg.msgClass());
        assertEquals(18, handleCloseUpdate.streamId());
        assertTrue(handleCloseUpdate.checkHasServiceId());
        assertEquals(1, handleCloseUpdate.serviceId());
        assertEquals(1, handleCloseUpdate.serviceList().size());
        assertEquals(1, handleCloseUpdate.serviceList().get(0).serviceId());
        assertEquals(MapEntryActions.DELETE, handleCloseUpdate.serviceList().get(0).action());
    }

    @Test
    public void givenRequestWithUnknownServiceId_whenHandleClose_thenNoDeleteUpdateIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();
        List<String> callbackLocations = new ArrayList<>();
        List<Msg> callbackMsgs = new ArrayList<>();
        List<DirectoryMsg> callbackDirectoryMsgs = new ArrayList<>();

        captureDirectoryCallbacks(context, callbackLocations, callbackMsgs, callbackDirectoryMsgs);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()));

        WlInteger streamTableKey = ReactorFactory.createWlInteger();
        streamTableKey.value(context.handler._stream.streamId());
        context.handler._stream.tableKey(streamTableKey);
        context.watchlist.streamIdtoWlStreamTable().put(streamTableKey, context.handler._stream);

        createAndRegisterDirectoryRequest(context, 31, Directory.ServiceFilterFlags.STATE, 999);

        context.handler.handleClose(context.handler._stream, context.errorInfo);

        assertTrue(callbackLocations.isEmpty());
        assertTrue(callbackMsgs.isEmpty());
        assertTrue(callbackDirectoryMsgs.isEmpty());
        assertTrue(context.handler._stream.userRequestList().isEmpty());
        assertTrue(context.handler.serviceList().isEmpty());
        assertEquals(StreamStates.CLOSED, context.handler._stream.state().streamState());
    }

    @Test
    public void givenRequestWithUnknownServiceId_whenDeleteAllServices_thenNoDeleteUpdateIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();
        List<String> callbackLocations = new ArrayList<>();
        List<Msg> callbackMsgs = new ArrayList<>();
        List<DirectoryMsg> callbackDirectoryMsgs = new ArrayList<>();

        captureDirectoryCallbacks(context, callbackLocations, callbackMsgs, callbackDirectoryMsgs);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                32,
                Directory.ServiceFilterFlags.STATE,
                999);
        wlRequest.state(WlRequest.State.OPEN);

        context.handler.deleteAllServices(context.handler._stream, false, context.errorInfo);

        assertTrue(callbackLocations.isEmpty());
        assertTrue(callbackMsgs.isEmpty());
        assertTrue(callbackDirectoryMsgs.isEmpty());
        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(StreamStates.CLOSED_RECOVER, context.handler._stream.state().streamState());
        assertEquals(DataStates.SUSPECT, context.handler._stream.state().dataState());
        assertEquals(1, context.handler._stream.userRequestList().size());
        assertTrue(context.handler.serviceList().isEmpty());
    }

    @Test
    public void givenRequestWithUnknownServiceName_whenDeleteAllServices_thenNoUpdateIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();
        List<String> callbackLocations = new ArrayList<>();
        List<Msg> callbackMsgs = new ArrayList<>();
        List<DirectoryMsg> callbackDirectoryMsgs = new ArrayList<>();

        captureDirectoryCallbacks(context, callbackLocations, callbackMsgs, callbackDirectoryMsgs);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                33,
                Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);
        wlRequest.streamInfo().serviceName("UNKNOWN_SERVICE");

        context.handler.deleteAllServices(context.handler._stream, false, context.errorInfo);

        assertTrue(callbackLocations.isEmpty());
        assertTrue(callbackMsgs.isEmpty());
        assertTrue(callbackDirectoryMsgs.isEmpty());
        assertEquals(WlRequest.State.PENDING_REFRESH, wlRequest.state());
        assertEquals(StreamStates.CLOSED_RECOVER, context.handler._stream.state().streamState());
        assertEquals(DataStates.SUSPECT, context.handler._stream.state().dataState());
        assertEquals(1, context.handler._stream.userRequestList().size());
        assertTrue(context.handler.serviceList().isEmpty());
    }

    @Test
    public void givenRequestWithUnknownServiceName_whenHandleClose_thenNoUpdateIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();
        List<String> callbackLocations = new ArrayList<>();
        List<Msg> callbackMsgs = new ArrayList<>();
        List<DirectoryMsg> callbackDirectoryMsgs = new ArrayList<>();

        captureDirectoryCallbacks(context, callbackLocations, callbackMsgs, callbackDirectoryMsgs);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()));

        WlInteger streamTableKey = ReactorFactory.createWlInteger();
        streamTableKey.value(context.handler._stream.streamId());
        context.handler._stream.tableKey(streamTableKey);
        context.watchlist.streamIdtoWlStreamTable().put(streamTableKey, context.handler._stream);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                34,
                Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.streamInfo().serviceName("UNKNOWN_SERVICE");

        context.handler.handleClose(context.handler._stream, context.errorInfo);

        assertTrue(callbackLocations.isEmpty());
        assertTrue(callbackMsgs.isEmpty());
        assertTrue(callbackDirectoryMsgs.isEmpty());
        assertTrue(context.handler._stream.userRequestList().isEmpty());
        assertTrue(context.handler.serviceList().isEmpty());
        assertEquals(StreamStates.CLOSED, context.handler._stream.state().streamState());
    }

    @Test
    public void givenRequestWithServiceNameAndUnsolicitedClearCache_whenReadRefreshMsgWithReceivedRefresh_thenCacheIsRebuiltAndNoRefreshUpdateIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        "TEST_SERVICE"),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                14,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                1);
        wlRequest.state(WlRequest.State.OPEN);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.STATE,
                true,
                false,
                addedStateOnlyService(1, Provider.defaultService().info().serviceName().toString()));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertNull(context.handler.service(Provider.defaultService().info().serviceName().toString()));
        assertEquals(1, context.handler.serviceList().size());

        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(1)).serviceAdded(any(WlService.class));

        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.deleteAllServices"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), any(Msg.class),
                        any(DirectoryMsg.class), same(wlRequest), any(ReactorErrorInfo.class));
        verify(context.reactor, never())
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), any(Msg.class),
                        any(DirectoryMsg.class), same(wlRequest), same(context.errorInfo));
    }

    @Test
    public void givenUnsolicitedClearCacheRefreshWithStateOnlyExistingServiceAndUpdatedStatus_whenReadRefreshMsgWithReceivedRefresh_thenServiceStateAndStatusArePropagated()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                15,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.STATE,
                true,
                false,
                addedStateOnlyServiceWithStatus(1, Provider.defaultService().info().serviceName().toString(),
                        0, DataStates.SUSPECT, StreamStates.CLOSED_RECOVER, 1));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());

        WlService cachedService = context.handler.service(1);
        assertNotNull(cachedService);
        assertNull(context.handler.service(2));
        assertNull(context.handler.service(Provider.defaultService().info().serviceName().toString()));
        assertEquals(1, context.handler.serviceList().size());
        assertFalse(cachedService.rdmService().checkHasInfo());
        assertTrue(cachedService.rdmService().checkHasState());
        assertEquals(0, cachedService.rdmService().state().serviceState());
        assertEquals(DataStates.SUSPECT, cachedService.rdmService().state().status().dataState());
        assertEquals(StreamStates.CLOSED_RECOVER, cachedService.rdmService().state().status().streamState());
        assertEquals(1, cachedService.rdmService().state().acceptingRequests());

        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(1)).serviceAdded(any(WlService.class));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.STATE, callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, null);

        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.STATE, callbackUpdate.filter());
        assertEquals(15, callbackUpdate.streamId());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service rebuiltService = callbackUpdate.serviceList().get(0);
        assertEquals(1, rebuiltService.serviceId());
        assertEquals(MapEntryActions.ADD, rebuiltService.action());
        assertFalse(rebuiltService.checkHasInfo());
        assertTrue(rebuiltService.checkHasState());
        assertEquals(0, rebuiltService.state().serviceState());
        assertEquals(DataStates.SUSPECT, rebuiltService.state().status().dataState());
        assertEquals(StreamStates.CLOSED_RECOVER, rebuiltService.state().status().streamState());
        assertEquals(1, rebuiltService.state().acceptingRequests());
        assertFalse(rebuiltService.checkHasLoad());
    }

    @Test
    public void givenRequestWithoutExplicitFilter_whenReadRefreshMsgWithReceivedRefresh_thenResultingUpdateFilterCorrespondsPayload()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedServiceWithLoad(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString(), 40),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequestWithoutFilter(context, 13, null);
        assertFalse(wlRequest.requestMsg().msgKey().checkHasFilter());

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                false,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedInfoOnlyService(3, "NO_REQUEST_FILTER_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(3, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNotNull(context.handler.service(2));
        assertNotNull(context.handler.service(3));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertTrue(callbackMsg.msgKey().checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, null);

        assertEquals(13, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                callbackUpdate.filter());
        assertEquals(2, callbackUpdate.serviceList().size());

        Service service1 = serviceById(callbackUpdate.serviceList(), 1);
        Service service3 = serviceById(callbackUpdate.serviceList(), 3);
        assertNotNull(service1);
        assertNotNull(service3);

        assertTrue(service1.checkHasInfo());
        assertTrue(service1.checkHasState());
        assertTrue(service1.checkHasLoad());
        assertEquals(40, service1.load().openWindow());

        assertTrue(service3.checkHasInfo());
        assertFalse(service3.checkHasState());
        assertFalse(service3.checkHasLoad());
    }

    @Test
    public void givenRequestFilterAndCachedServices_whenReadRefreshMsgWithoutReceivedRefresh_thenResultingRefreshUsesRequestFilterAndCurrentCache()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedServiceWithLoad(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString(), 25),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                21,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                false,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedInfoOnlyService(3, "THIRD_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(3, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNotNull(context.handler.service(2));
        assertNotNull(context.handler.service(3));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        RefreshMsg callbackMsg = (RefreshMsg) msgCaptor.getValue();
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, null);

        assertEquals(21, callbackRefresh.streamId());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackRefresh.filter());
        assertTrue(callbackRefresh.checkSolicited());
        assertFalse(callbackRefresh.checkClearCache());
        assertEquals(2, callbackRefresh.serviceList().size());

        Service service1 = serviceById(callbackRefresh.serviceList(), 1);
        Service service3 = serviceById(callbackRefresh.serviceList(), 3);
        assertNotNull(service1);
        assertNotNull(service3);

        assertTrue(service1.checkHasInfo());
        assertTrue(service1.checkHasState());
        assertFalse(service1.checkHasLoad());

        assertTrue(service3.checkHasInfo());
        assertFalse(service3.checkHasState());
        assertFalse(service3.checkHasLoad());
    }

    @Test
    public void givenServiceIdAndLoadFilter_whenReadRefreshMsgWithoutReceivedRefreshWithoutLoadFilter_thenResultingRefreshUsesServiceWithNoFilters()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedServiceWithLoad(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString(), 55),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                22,
                Directory.ServiceFilterFlags.LOAD,
                1);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                false,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                updatedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        RefreshMsg callbackMsg = (RefreshMsg) msgCaptor.getValue();
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(0, callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, 1);

        assertEquals(22, callbackRefresh.streamId());
        assertEquals(0, callbackRefresh.filter());
        assertEquals(1, callbackRefresh.serviceList().size());

        Service selectedService = callbackRefresh.serviceList().get(0);
        assertEquals(1, selectedService.serviceId());
        assertFalse(selectedService.checkHasInfo());
        assertFalse(selectedService.checkHasState());
        assertFalse(selectedService.checkHasLoad());
    }

    @Test
    public void givenRefreshContainingDeletedService_whenReadRefreshMsgWithoutReceivedRefresh_thenDeletedServiceIsRemovedFromCacheAndFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                24,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);

        DirectoryRefresh receivedRefresh = createRefreshWithUpdatedService1AndDeletedService2(context.handler._stream.streamId());

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertNull(context.handler.service(Provider.defaultService2().info().serviceName().toString()));

        verify(context.itemHandler, times(1)).serviceUpdated(any(WlService.class), anyBoolean());
        verify(context.itemHandler, times(1)).serviceDeleted(any(WlService.class), eq(false));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readRefreshMsg", wlRequest);
        RefreshMsg callbackMsg = (RefreshMsg) callback.msg;
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) callback.directoryMsg;

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, null);

        assertEquals(24, callbackRefresh.streamId());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackRefresh.filter());
        assertTrue(callbackRefresh.checkSolicited());
        assertFalse(callbackRefresh.checkClearCache());
        assertEquals(2, callbackRefresh.serviceList().size());

        Service remainingService = callbackRefresh.serviceList().get(0);
        Service deletedService = callbackRefresh.serviceList().get(1);
        assertEquals(MapEntryActions.UPDATE, remainingService.action());
        assertEquals(1, remainingService.serviceId());
        assertTrue(remainingService.checkHasInfo());
        assertTrue(remainingService.checkHasState());
        assertEquals(MapEntryActions.DELETE, deletedService.action());
        assertEquals(2, deletedService.serviceId());
    }

    @Test
    public void givenRefreshContainingDeletedService_whenReadRefreshMsgWithReceivedRefresh_thenDeletedServiceIsRemovedFromCacheAndFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheDefaultServicesAndClearInvocations(context);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                25,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);

        DirectoryRefresh receivedRefresh = createRefreshWithUpdatedService1AndDeletedService2(context.handler._stream.streamId());

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertNull(context.handler.service(Provider.defaultService2().info().serviceName().toString()));

        verify(context.itemHandler, times(1)).serviceUpdated(any(WlService.class), anyBoolean());
        verify(context.itemHandler, times(1)).serviceDeleted(any(WlService.class), eq(false));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readRefreshMsg", wlRequest);
        UpdateMsg callbackMsg = (UpdateMsg) callback.msg;
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) callback.directoryMsg;

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, null);

        assertEquals(25, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackUpdate.filter());
        assertEquals(2, callbackUpdate.serviceList().size());

        Service remainingService = callbackUpdate.serviceList().get(0);
        Service deletedService = callbackUpdate.serviceList().get(1);
        assertEquals(MapEntryActions.UPDATE, remainingService.action());
        assertEquals(1, remainingService.serviceId());
        assertTrue(remainingService.checkHasInfo());
        assertTrue(remainingService.checkHasState());
        assertEquals(MapEntryActions.DELETE, deletedService.action());
        assertEquals(2, deletedService.serviceId());
    }

    @Test
    public void givenRequestWithServiceId_whenReadRefreshMsgContainsDeletedOtherService_thenDeletedServiceIsNotFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheDefaultServicesAndClearInvocations(context);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                26,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                1);

        DirectoryRefresh receivedRefresh = createRefreshWithUpdatedService1AndDeletedService2(context.handler._stream.streamId());

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readRefreshMsg", wlRequest);
        RefreshMsg callbackMsg = (RefreshMsg) callback.msg;
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) callback.directoryMsg;

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, 1);

        assertEquals(26, callbackRefresh.streamId());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackRefresh.filter());
        assertEquals(1, callbackRefresh.serviceList().size());

        Service updatedService = callbackRefresh.serviceList().get(0);
        assertEquals(MapEntryActions.UPDATE, updatedService.action());
        assertEquals(1, updatedService.serviceId());
        assertNull(serviceById(callbackRefresh.serviceList(), 2));
    }

    @Test
    public void givenRequestWithServiceId_whenReadRefreshMsgContainsDeletedRequestedService_thenDeletedServiceIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheDefaultServicesAndClearInvocations(context);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                27,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                2);

        DirectoryRefresh receivedRefresh = createRefreshWithUpdatedService1AndDeletedService2(context.handler._stream.streamId());

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readRefreshMsg", wlRequest);
        RefreshMsg callbackMsg = (RefreshMsg) callback.msg;
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) callback.directoryMsg;

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(0, callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, 2);

        assertEquals(27, callbackRefresh.streamId());
        assertEquals(0, callbackRefresh.filter());
        assertEquals(1, callbackRefresh.serviceList().size());

        Service deletedService = callbackRefresh.serviceList().get(0);
        assertEquals(MapEntryActions.DELETE, deletedService.action());
        assertEquals(2, deletedService.serviceId());
        assertNull(serviceById(callbackRefresh.serviceList(), 1));
    }

    @Test
    public void givenRequestWithServiceName_whenReadRefreshMsgContainsDeletedOtherService_thenDeletedServiceIsNotFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1, "TEST_SERVICE"),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                28,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                false,
                updatedService(Provider.defaultService(), 1, "TEST_SERVICE"),
                buildServiceFromTemplate(Provider.defaultService2(), MapEntryActions.DELETE, 2,
                        Provider.defaultService2().info().serviceName().toString()));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        RefreshMsg callbackMsg = (RefreshMsg) msgCaptor.getValue();
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, 1);

        assertEquals(28, callbackRefresh.streamId());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackRefresh.filter());
        assertEquals(1, callbackRefresh.serviceList().size());

        Service updatedService = callbackRefresh.serviceList().get(0);
        assertEquals(MapEntryActions.UPDATE, updatedService.action());
        assertEquals(1, updatedService.serviceId());
        assertNull(serviceById(callbackRefresh.serviceList(), 2));
    }

    @Test
    public void givenRequestWithServiceName_whenReadRefreshMsgContainsDeletedRequestedService_thenDeletedServiceIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1, "TEST_SERVICE"),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                29,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryRefresh receivedRefresh = createRefreshWithUpdatedService2AndDeletedTestService(context.handler._stream.streamId());

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(1, context.handler.serviceList().size());
        assertNull(context.handler.service(1));
        assertNull(context.handler.service("TEST_SERVICE"));
        assertNotNull(context.handler.service(2));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readRefreshMsg", wlRequest);
        RefreshMsg callbackMsg = (RefreshMsg) callback.msg;
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) callback.directoryMsg;

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(0, callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, 1);

        assertEquals(29, callbackRefresh.streamId());
        assertEquals(0, callbackRefresh.filter());
        assertEquals(1, callbackRefresh.serviceList().size());

        Service deletedService = callbackRefresh.serviceList().get(0);
        assertEquals(MapEntryActions.DELETE, deletedService.action());
        assertEquals(1, deletedService.serviceId());
        assertNull(serviceById(callbackRefresh.serviceList(), 2));
    }

    @Test
    public void givenRequestWithServiceId_whenReadRefreshMsgWithReceivedRefreshContainsDeletedOtherService_thenDeletedServiceIsNotFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheDefaultServicesAndClearInvocations(context);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                30,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                1);

        DirectoryRefresh receivedRefresh = createRefreshWithUpdatedService1AndDeletedService2(context.handler._stream.streamId());

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readRefreshMsg", wlRequest);
        UpdateMsg callbackMsg = (UpdateMsg) callback.msg;
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) callback.directoryMsg;

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertEquals(30, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service updatedService = callbackUpdate.serviceList().get(0);
        assertEquals(MapEntryActions.UPDATE, updatedService.action());
        assertEquals(1, updatedService.serviceId());
        assertNull(serviceById(callbackUpdate.serviceList(), 2));
    }

    @Test
    public void givenRequestWithServiceId_whenReadRefreshMsgWithReceivedRefreshContainsDeletedRequestedService_thenDeletedServiceIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheDefaultServicesAndClearInvocations(context);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                31,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                2);

        DirectoryRefresh receivedRefresh = createRefreshWithUpdatedService1AndDeletedService2(context.handler._stream.streamId());

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readRefreshMsg", wlRequest);
        UpdateMsg callbackMsg = (UpdateMsg) callback.msg;
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) callback.directoryMsg;

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(0, callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 2);

        assertEquals(31, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(0, callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service deletedService = callbackUpdate.serviceList().get(0);
        assertEquals(MapEntryActions.DELETE, deletedService.action());
        assertEquals(2, deletedService.serviceId());
        assertNull(serviceById(callbackUpdate.serviceList(), 1));
    }

    @Test
    public void givenRequestWithServiceName_whenReadRefreshMsgWithReceivedRefreshContainsDeletedOtherService_thenDeletedServiceIsNotFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1, "TEST_SERVICE"),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                32,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                false,
                updatedService(Provider.defaultService(), 1, "TEST_SERVICE"),
                buildServiceFromTemplate(Provider.defaultService2(), MapEntryActions.DELETE, 2,
                        Provider.defaultService2().info().serviceName().toString()));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertEquals(32, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service updatedService = callbackUpdate.serviceList().get(0);
        assertEquals(MapEntryActions.UPDATE, updatedService.action());
        assertEquals(1, updatedService.serviceId());
        assertNull(serviceById(callbackUpdate.serviceList(), 2));
    }

    @Test
    public void givenRequestWithServiceName_whenReadRefreshMsgWithReceivedRefreshContainsDeletedRequestedService_thenDeletedServiceIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1, "TEST_SERVICE"),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                33,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryRefresh receivedRefresh = createRefreshWithUpdatedService2AndDeletedTestService(context.handler._stream.streamId());

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(1, context.handler.serviceList().size());
        assertNull(context.handler.service(1));
        assertNull(context.handler.service("TEST_SERVICE"));
        assertNotNull(context.handler.service(2));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readRefreshMsg", wlRequest);
        UpdateMsg callbackMsg = (UpdateMsg) callback.msg;
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) callback.directoryMsg;

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(0, callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertEquals(33, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(0, callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service deletedService = callbackUpdate.serviceList().get(0);
        assertEquals(MapEntryActions.DELETE, deletedService.action());
        assertEquals(1, deletedService.serviceId());
        assertNull(serviceById(callbackUpdate.serviceList(), 2));
    }

    @Test
    public void givenRequestWithServiceName_whenReadRefreshMsgAddsRequestedService_thenRefreshUsesResolvedServiceId()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                34,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                false,
                addedService(Provider.defaultService(), 1, "TEST_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNotNull(context.handler.service(1));
        assertNotNull(context.handler.service("TEST_SERVICE"));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readRefreshMsg", wlRequest);
        RefreshMsg callbackMsg = (RefreshMsg) callback.msg;
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) callback.directoryMsg;

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, 1);

        assertEquals(34, callbackRefresh.streamId());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackRefresh.filter());
        assertEquals(1, callbackRefresh.serviceList().size());

        Service addedService = callbackRefresh.serviceList().get(0);
        assertEquals(MapEntryActions.ADD, addedService.action());
        assertEquals(1, addedService.serviceId());
        assertTrue(addedService.checkHasInfo());
        assertTrue(addedService.checkHasState());
    }

    @Test
    public void givenRequestWithServiceName_whenReadRefreshMsgWithReceivedRefreshAddsRequestedService_thenUpdateUsesResolvedServiceId()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                35,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                false,
                addedService(Provider.defaultService(), 1, "TEST_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNotNull(context.handler.service(1));
        assertNotNull(context.handler.service("TEST_SERVICE"));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readRefreshMsg", wlRequest);
        UpdateMsg callbackMsg = (UpdateMsg) callback.msg;
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) callback.directoryMsg;

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertEquals(35, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service addedService = callbackUpdate.serviceList().get(0);
        assertEquals(MapEntryActions.ADD, addedService.action());
        assertEquals(1, addedService.serviceId());
        assertTrue(addedService.checkHasInfo());
        assertTrue(addedService.checkHasState());
    }

    @Test
    public void givenUnsolicitedClearCacheRefresh_whenReadRefreshMsgWithoutReceivedRefresh_thenCacheIsRebuiltAndRefreshIsFannedOutAsRefresh()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                23,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO,
                true,
                false,
                addedInfoOnlyService(3, "UNSOLICITED_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertNotNull(context.handler.service(3));
        assertEquals(1, context.handler.serviceList().size());

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));
        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(1)).serviceAdded(any(WlService.class));

        RefreshMsg callbackMsg = (RefreshMsg) msgCaptor.getValue();
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) directoryMsgCaptor.getValue();
        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO, callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, null);
        assertTrue(callbackRefresh.checkSolicited());
        assertFalse(callbackRefresh.checkClearCache());
        assertEquals(Directory.ServiceFilterFlags.INFO, callbackRefresh.filter());
        assertEquals(23, callbackRefresh.streamId());
        assertEquals(1, callbackRefresh.serviceList().size());

        Service newService = callbackRefresh.serviceList().get(0);
        assertEquals(3, newService.serviceId());
        assertEquals(MapEntryActions.ADD, newService.action());
        assertTrue(newService.checkHasInfo());
        assertFalse(newService.checkHasState());
        assertFalse(newService.checkHasLoad());
    }

    @Test
    public void givenAllServicesRequestAndMixedUnsolicitedClearCacheRefresh_whenReadRefreshMsgWithoutReceivedRefresh_thenOnlyAddedServicesAreFannedOutAsRefresh()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                30,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                true,
                false,
                updatedStateOnlyService(1, Provider.defaultService().info().serviceName().toString()),
                addedInfoOnlyService(3, "UNSOLICITED_ADDED_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertNotNull(context.handler.service(3));
        assertEquals(2, context.handler.serviceList().size());

        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(2)).serviceAdded(any(WlService.class));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        RefreshMsg callbackMsg = (RefreshMsg) msgCaptor.getValue();
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO,
                callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, null);

        assertEquals(30, callbackRefresh.streamId());
        assertEquals(Directory.ServiceFilterFlags.INFO,
                callbackRefresh.filter());
        assertEquals(1, callbackRefresh.serviceList().size());

        Service addedService = callbackRefresh.serviceList().get(0);
        assertEquals(3, addedService.serviceId());
        assertEquals(MapEntryActions.ADD, addedService.action());
        assertTrue(addedService.checkHasInfo());
        assertFalse(addedService.checkHasState());
    }

    @Test
    public void givenUnsolicitedClearCacheRefresh_whenReadRefreshMsgWithoutReceivedRefresh_thenHandleCloseUpdateIsGeneratedBeforeRefresh()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);
        List<String> callbackLocations = new ArrayList<>();
        List<Msg> callbackMsgs = new ArrayList<>();
        List<DirectoryMsg> callbackDirectoryMsgs = new ArrayList<>();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        doAnswer(invocation -> {
            callbackLocations.add(invocation.getArgument(0));

            Msg copiedMsg = CodecFactory.createMsg();
            ((Msg) invocation.getArgument(3)).copy(copiedMsg, CopyMsgFlags.ALL_FLAGS);
            callbackMsgs.add(copiedMsg);

            DirectoryMsg copiedDirectoryMsg = DirectoryMsgFactory.createMsg();
            TestUtil.copyDirectoryMsg(invocation.getArgument(4), copiedDirectoryMsg);
            callbackDirectoryMsgs.add(copiedDirectoryMsg);

            return ReactorCallbackReturnCodes.SUCCESS;
        }).when(context.reactor).sendAndHandleDirectoryMsgCallback(anyString(), any(ReactorChannel.class),
                nullable(TransportBuffer.class), any(Msg.class), any(DirectoryMsg.class),
                any(WlRequest.class), any(ReactorErrorInfo.class));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                24,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO,
                true,
                false,
                addedInfoOnlyService(3, "UNSOLICITED_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());

        assertEquals(2, callbackLocations.size());
        assertEquals("WlDirectoryHandler.deleteAllServices", callbackLocations.get(0));
        assertEquals("WlDirectoryHandler.readRefreshMsg", callbackLocations.get(1));

        UpdateMsg handleCloseMsg = (UpdateMsg) callbackMsgs.get(0);
        DirectoryUpdate handleCloseUpdate = (DirectoryUpdate) callbackDirectoryMsgs.get(0);

        assertEquals(MsgClasses.UPDATE, handleCloseMsg.msgClass());
        assertFalse(handleCloseMsg.checkHasMsgKey());
        assertEquals(24, handleCloseUpdate.streamId());
        assertFalse(handleCloseUpdate.checkHasFilter());
        assertFalse(handleCloseUpdate.checkHasServiceId());
        assertEquals(2, handleCloseUpdate.serviceList().size());
        assertEquals(1, handleCloseUpdate.serviceList().get(0).serviceId());
        assertEquals(MapEntryActions.DELETE, handleCloseUpdate.serviceList().get(0).action());
        assertEquals(2, handleCloseUpdate.serviceList().get(1).serviceId());
        assertEquals(MapEntryActions.DELETE, handleCloseUpdate.serviceList().get(1).action());

        RefreshMsg callbackMsg = (RefreshMsg) callbackMsgs.get(1);
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) callbackDirectoryMsgs.get(1);

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO, callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, null);
        assertTrue(callbackRefresh.checkSolicited());
        assertFalse(callbackRefresh.checkClearCache());
        assertEquals(Directory.ServiceFilterFlags.INFO, callbackRefresh.filter());
        assertEquals(24, callbackRefresh.streamId());
        assertEquals(1, callbackRefresh.serviceList().size());
        assertEquals(3, callbackRefresh.serviceList().get(0).serviceId());
    }

    @Test
    public void givenRequestWithServiceIdAndUnsolicitedClearCache_whenReadRefreshMsgWithoutReceivedRefresh_thenCacheIsRebuiltAndRefreshIsFannedOutAsRefresh()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                25,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                1);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.STATE,
                true,
                false,
                addedStateOnlyService(1, Provider.defaultService().info().serviceName().toString()));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertNull(context.handler.service(Provider.defaultService().info().serviceName().toString()));
        assertEquals(1, context.handler.serviceList().size());

        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(1)).serviceAdded(any(WlService.class));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        RefreshMsg callbackMsg = (RefreshMsg) msgCaptor.getValue();
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.STATE, callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, 1);

        assertEquals(Directory.ServiceFilterFlags.STATE, callbackRefresh.filter());
        assertEquals(25, callbackRefresh.streamId());
        assertEquals(1, callbackRefresh.serviceList().size());

        Service rebuiltService = callbackRefresh.serviceList().get(0);
        assertEquals(1, rebuiltService.serviceId());
        assertEquals(MapEntryActions.ADD, rebuiltService.action());
        assertFalse(rebuiltService.checkHasInfo());
        assertTrue(rebuiltService.checkHasState());
        assertFalse(rebuiltService.checkHasLoad());
    }

    @Test
    public void givenRequestWithServiceNameAndUnsolicitedClearCache_whenReadRefreshMsgWithoutReceivedRefresh_thenCacheIsRebuiltAndNoRefreshIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        "TEST_SERVICE"),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                26,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                1);
        wlRequest.state(WlRequest.State.OPEN);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.STATE,
                true,
                false,
                addedStateOnlyService(1, Provider.defaultService().info().serviceName().toString()));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertNull(context.handler.service(Provider.defaultService().info().serviceName().toString()));
        assertEquals(1, context.handler.serviceList().size());

        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(1)).serviceAdded(any(WlService.class));

        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.deleteAllServices"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), any(Msg.class),
                        any(DirectoryMsg.class), same(wlRequest), any(ReactorErrorInfo.class));
        verify(context.reactor, never())
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), any(Msg.class),
                        any(DirectoryMsg.class), same(wlRequest), same(context.errorInfo));
    }

    @Test
    public void givenUnsolicitedClearCacheRefreshWithStateOnlyExistingServiceAndUpdatedStatus_whenReadRefreshMsgWithoutReceivedRefresh_thenServiceStateAndStatusArePropagated()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                27,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.STATE,
                true,
                false,
                addedStateOnlyServiceWithStatus(1, Provider.defaultService().info().serviceName().toString(),
                        0, DataStates.SUSPECT, StreamStates.CLOSED_RECOVER, 1));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());

        WlService cachedService = context.handler.service(1);
        assertNotNull(cachedService);
        assertNull(context.handler.service(2));
        assertNull(context.handler.service(Provider.defaultService().info().serviceName().toString()));
        assertEquals(1, context.handler.serviceList().size());
        assertFalse(cachedService.rdmService().checkHasInfo());
        assertTrue(cachedService.rdmService().checkHasState());
        assertEquals(0, cachedService.rdmService().state().serviceState());
        assertEquals(DataStates.SUSPECT, cachedService.rdmService().state().status().dataState());
        assertEquals(StreamStates.CLOSED_RECOVER, cachedService.rdmService().state().status().streamState());
        assertEquals(1, cachedService.rdmService().state().acceptingRequests());

        verify(context.itemHandler, times(2)).serviceDeleted(any(WlService.class), eq(false));
        verify(context.itemHandler, times(1)).serviceAdded(any(WlService.class));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        RefreshMsg callbackMsg = (RefreshMsg) msgCaptor.getValue();
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.STATE, callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, null);

        assertEquals(Directory.ServiceFilterFlags.STATE, callbackRefresh.filter());
        assertEquals(27, callbackRefresh.streamId());
        assertEquals(1, callbackRefresh.serviceList().size());

        Service rebuiltService = callbackRefresh.serviceList().get(0);
        assertEquals(1, rebuiltService.serviceId());
        assertEquals(MapEntryActions.ADD, rebuiltService.action());
        assertFalse(rebuiltService.checkHasInfo());
        assertTrue(rebuiltService.checkHasState());
        assertEquals(0, rebuiltService.state().serviceState());
        assertEquals(DataStates.SUSPECT, rebuiltService.state().status().dataState());
        assertEquals(StreamStates.CLOSED_RECOVER, rebuiltService.state().status().streamState());
        assertEquals(1, rebuiltService.state().acceptingRequests());
        assertFalse(rebuiltService.checkHasLoad());
    }

    @Test
    public void givenRequestWithoutExplicitFilter_whenReadRefreshMsgWithoutReceivedRefresh_thenResultingRefreshFilterCorrespondsPayload()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext(false);

        cacheServices(context.handler,
                addedServiceWithLoad(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString(), 40),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequestWithoutFilter(context, 28, null);
        assertFalse(wlRequest.requestMsg().msgKey().checkHasFilter());

        DirectoryRefresh receivedRefresh = createDirectoryRefresh(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                false,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedInfoOnlyService(3, "NO_REQUEST_FILTER_SERVICE"));

        Msg refreshMsg = encodeDirectoryMsg(receivedRefresh, decodeIter);

        int ret = context.handler.readRefreshMsg(context.handler._stream, decodeIter, refreshMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(WlRequest.State.OPEN, wlRequest.state());
        assertEquals(3, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNotNull(context.handler.service(2));
        assertNotNull(context.handler.service(3));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readRefreshMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        RefreshMsg callbackMsg = (RefreshMsg) msgCaptor.getValue();
        DirectoryRefresh callbackRefresh = (DirectoryRefresh) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.REFRESH, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertTrue(callbackMsg.msgKey().checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                callbackMsg.msgKey().filter());
        assertRefreshServiceId(callbackMsg, callbackRefresh, null);

        assertEquals(28, callbackRefresh.streamId());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                callbackRefresh.filter());
        assertEquals(2, callbackRefresh.serviceList().size());

        Service service1 = serviceById(callbackRefresh.serviceList(), 1);
        Service service3 = serviceById(callbackRefresh.serviceList(), 3);
        assertNotNull(service1);
        assertNotNull(service3);

        assertTrue(service1.checkHasInfo());
        assertTrue(service1.checkHasState());
        assertTrue(service1.checkHasLoad());
        assertEquals(40, service1.load().openWindow());

        assertTrue(service3.checkHasInfo());
        assertFalse(service3.checkHasState());
        assertFalse(service3.checkHasLoad());
    }

    @Test
    public void givenRequestFilterAndUpdatedServices_whenReadUpdateMsg_thenResultingUpdateUsesRequestFilterAndChangedServices()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedServiceWithLoad(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString(), 25),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                17,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedInfoOnlyService(3, "UPDATED_SERVICE"));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(3, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNotNull(context.handler.service(2));
        assertNotNull(context.handler.service(3));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readUpdateMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, null);

        assertEquals(17, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackUpdate.filter());
        assertEquals(2, callbackUpdate.serviceList().size());

        Service service1 = serviceById(callbackUpdate.serviceList(), 1);
        Service service3 = serviceById(callbackUpdate.serviceList(), 3);
        assertNotNull(service1);
        assertNotNull(service3);
        assertNull(serviceById(callbackUpdate.serviceList(), 2));

        assertEquals(MapEntryActions.UPDATE, service1.action());
        assertTrue(service1.checkHasInfo());
        assertTrue(service1.checkHasState());
        assertFalse(service1.checkHasLoad());

        assertEquals(MapEntryActions.ADD, service3.action());
        assertTrue(service3.checkHasInfo());
        assertFalse(service3.checkHasState());
        assertFalse(service3.checkHasLoad());
    }

    @Test
    public void givenServiceIdAndLoadFilter_whenReadUpdateMsgWithoutLoadFilter_thenResultingUpdateHasEmptyPayloadAndZeroFilter()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedServiceWithLoad(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString(), 55),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                18,
                Directory.ServiceFilterFlags.LOAD,
                1);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                updatedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertNotNull(context.handler.service(1));
        assertNotNull(context.handler.service(2));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readUpdateMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(0, callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertEquals(18, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(0, callbackUpdate.filter());
        assertTrue(callbackUpdate.serviceList().isEmpty());
    }

    @Test
    public void givenRequestWithoutExplicitFilter_whenReadUpdateMsg_thenResultingUpdateUsesReceivedUpdateFilterAndPayload()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedServiceWithLoad(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString(), 40),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequestWithoutFilter(context, 19, null);
        wlRequest.state(WlRequest.State.OPEN);
        assertFalse(wlRequest.requestMsg().msgKey().checkHasFilter());

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedServiceWithLoad(Provider.defaultService(), 3, "NO_REQUEST_FILTER_SERVICE", 60));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(3, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNotNull(context.handler.service(2));
        assertNotNull(context.handler.service(3));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readUpdateMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, null);

        assertEquals(19, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.LOAD,
                callbackUpdate.filter());
        assertEquals(2, callbackUpdate.serviceList().size());

        Service service1 = serviceById(callbackUpdate.serviceList(), 1);
        Service service3 = serviceById(callbackUpdate.serviceList(), 3);
        assertNotNull(service1);
        assertNotNull(service3);

        assertEquals(MapEntryActions.UPDATE, service1.action());
        assertTrue(service1.checkHasInfo());
        assertTrue(service1.checkHasState());
        assertFalse(service1.checkHasLoad());

        assertEquals(MapEntryActions.ADD, service3.action());
        assertTrue(service3.checkHasInfo());
        assertTrue(service3.checkHasState());
        assertTrue(service3.checkHasLoad());
        assertEquals(60, service3.load().openWindow());
    }

    @Test
    public void givenUpdateContainingDeletedService_whenReadUpdateMsg_thenDeletedServiceIsRemovedFromCacheAndFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                20,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                buildServiceFromTemplate(Provider.defaultService2(), MapEntryActions.DELETE, 2,
                        Provider.defaultService2().info().serviceName().toString()));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));
        assertNull(context.handler.service(Provider.defaultService2().info().serviceName().toString()));

        verify(context.itemHandler, times(1)).serviceUpdated(any(WlService.class), anyBoolean());
        verify(context.itemHandler, times(1)).serviceDeleted(any(WlService.class), eq(false));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readUpdateMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, null);

        assertEquals(20, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackUpdate.filter());
        assertEquals(2, callbackUpdate.serviceList().size());

        Service updatedService = serviceById(callbackUpdate.serviceList(), 1);
        Service deletedService = serviceById(callbackUpdate.serviceList(), 2);
        assertNotNull(updatedService);
        assertNotNull(deletedService);

        assertEquals(MapEntryActions.UPDATE, updatedService.action());
        assertTrue(updatedService.checkHasInfo());
        assertTrue(updatedService.checkHasState());

        assertEquals(MapEntryActions.DELETE, deletedService.action());
        assertEquals(2, deletedService.serviceId());
    }

    @Test
    public void givenRequestWithServiceId_whenReadUpdateMsgContainsDeletedRequestedService_thenDeletedServiceIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheDefaultServicesAndClearInvocations(context);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                21,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                2);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                buildServiceFromTemplate(Provider.defaultService2(), MapEntryActions.DELETE, 2,
                        Provider.defaultService2().info().serviceName().toString()));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readUpdateMsg", wlRequest);
        UpdateMsg callbackMsg = (UpdateMsg) callback.msg;
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) callback.directoryMsg;

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(0, callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 2);

        assertEquals(21, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(0, callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service deletedService = callbackUpdate.serviceList().get(0);
        assertEquals(MapEntryActions.DELETE, deletedService.action());
        assertEquals(2, deletedService.serviceId());
        assertNull(serviceById(callbackUpdate.serviceList(), 1));
    }

    @Test
    public void givenRequestWithServiceName_whenReadUpdateMsgContainsDeletedRequestedService_thenDeletedServiceIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1, "TEST_SERVICE"),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                22,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                updatedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()),
                buildServiceFromTemplate(Provider.defaultService(), MapEntryActions.DELETE, 1, "TEST_SERVICE"));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(1, context.handler.serviceList().size());
        assertNull(context.handler.service(1));
        assertNull(context.handler.service("TEST_SERVICE"));
        assertNotNull(context.handler.service(2));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readUpdateMsg", wlRequest);
        UpdateMsg callbackMsg = (UpdateMsg) callback.msg;
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) callback.directoryMsg;

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(0, callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertEquals(22, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(0, callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service deletedService = callbackUpdate.serviceList().get(0);
        assertEquals(MapEntryActions.DELETE, deletedService.action());
        assertEquals(1, deletedService.serviceId());
        assertNull(serviceById(callbackUpdate.serviceList(), 2));
    }

    @Test
    public void givenRequestWithServiceId_whenReadUpdateMsgContainsDeletedOtherService_thenDeletedServiceIsNotFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheDefaultServicesAndClearInvocations(context);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                23,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                1);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                buildServiceFromTemplate(Provider.defaultService2(), MapEntryActions.DELETE, 2,
                        Provider.defaultService2().info().serviceName().toString()));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readUpdateMsg", wlRequest);
        UpdateMsg callbackMsg = (UpdateMsg) callback.msg;
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) callback.directoryMsg;

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertEquals(23, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service updatedService = callbackUpdate.serviceList().get(0);
        assertEquals(MapEntryActions.UPDATE, updatedService.action());
        assertEquals(1, updatedService.serviceId());
        assertNull(serviceById(callbackUpdate.serviceList(), 2));
    }

    @Test
    public void givenRequestWithServiceName_whenReadUpdateMsgContainsDeletedOtherService_thenDeletedServiceIsNotFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1, "TEST_SERVICE"),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                24,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                updatedService(Provider.defaultService(), 1, "TEST_SERVICE"),
                buildServiceFromTemplate(Provider.defaultService2(), MapEntryActions.DELETE, 2,
                        Provider.defaultService2().info().serviceName().toString()));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readUpdateMsg", wlRequest);
        UpdateMsg callbackMsg = (UpdateMsg) callback.msg;
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) callback.directoryMsg;

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertEquals(24, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service updatedService = callbackUpdate.serviceList().get(0);
        assertEquals(MapEntryActions.UPDATE, updatedService.action());
        assertEquals(1, updatedService.serviceId());
        assertNull(serviceById(callbackUpdate.serviceList(), 2));
    }

    @Test
    public void givenRequestWithServiceName_whenReadUpdateMsgAddsRequestedService_thenUpdateUsesResolvedServiceId()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                25,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                addedService(Provider.defaultService(), 1, "TEST_SERVICE"));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertNotNull(context.handler.service(1));
        assertNotNull(context.handler.service("TEST_SERVICE"));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readUpdateMsg", wlRequest);
        UpdateMsg callbackMsg = (UpdateMsg) callback.msg;
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) callback.directoryMsg;

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertEquals(25, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service addedService = callbackUpdate.serviceList().get(0);
        assertEquals(MapEntryActions.ADD, addedService.action());
        assertEquals(1, addedService.serviceId());
        assertTrue(addedService.checkHasInfo());
        assertTrue(addedService.checkHasState());
    }

    @Test
    public void givenRequestWithoutServiceNameOrServiceId_whenReadUpdateMsgContainsDeletedService_thenDeletedServiceIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheDefaultServicesAndClearInvocations(context);

        WlRequest wlRequest = createAndRegisterDirectoryRequestWithoutFilter(context, 25, null);
        wlRequest.state(WlRequest.State.OPEN);

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                buildServiceFromTemplate(Provider.defaultService2(), MapEntryActions.DELETE, 2,
                        Provider.defaultService2().info().serviceName().toString()));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertEquals(1, context.handler.serviceList().size());
        assertNotNull(context.handler.service(1));
        assertNull(context.handler.service(2));

        CapturedDirectoryCallback callback = captureSingleDirectoryCallback(context,
                "WlDirectoryHandler.readUpdateMsg", wlRequest);
        UpdateMsg callbackMsg = (UpdateMsg) callback.msg;
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) callback.directoryMsg;

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, null);

        assertEquals(25, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                callbackUpdate.filter());
        assertEquals(2, callbackUpdate.serviceList().size());

        Service updatedService = serviceById(callbackUpdate.serviceList(), 1);
        Service deletedService = serviceById(callbackUpdate.serviceList(), 2);
        assertNotNull(updatedService);
        assertNotNull(deletedService);

        assertEquals(MapEntryActions.UPDATE, updatedService.action());
        assertEquals(1, updatedService.serviceId());
        assertEquals(MapEntryActions.DELETE, deletedService.action());
        assertEquals(2, deletedService.serviceId());
    }

    @Test
    public void givenRequestWithServiceNameAndUpdatedServiceLosesInfo_whenReadUpdateMsg_thenStateOnlyUpdateIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1, "TEST_SERVICE"),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                20,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);
        wlRequest.streamInfo().serviceName("TEST_SERVICE");

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.STATE,
                updatedStateOnlyService(1, Provider.defaultService().info().serviceName().toString()));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertNotNull(context.handler.service(1));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readUpdateMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(Directory.ServiceFilterFlags.STATE, callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, 1);

        assertEquals(20, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.STATE, callbackUpdate.filter());
        assertEquals(1, callbackUpdate.serviceList().size());

        Service updatedService = callbackUpdate.serviceList().get(0);
        assertEquals(MapEntryActions.UPDATE, updatedService.action());
        assertEquals(1, updatedService.serviceId());
        assertFalse(updatedService.checkHasInfo());
        assertTrue(updatedService.checkHasState());
        assertFalse(updatedService.checkHasLoad());
    }

    @Test
    public void givenRequestWithUnknownServiceName_whenReadUpdateMsg_thenEmptyUpdateWithZeroFilterIsFannedOut()
    {
        DirectoryHandlerTestContext context = new DirectoryHandlerTestContext();

        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));

        WlRequest wlRequest = createAndRegisterDirectoryRequest(context,
                21,
                Directory.ServiceFilterFlags.STATE,
                null);
        wlRequest.state(WlRequest.State.OPEN);
        wlRequest.streamInfo().serviceName("UNKNOWN_SERVICE");

        DirectoryUpdate receivedUpdate = createDirectoryUpdate(
                context.handler._stream.streamId(),
                Directory.ServiceFilterFlags.STATE,
                updatedStateOnlyService(1, Provider.defaultService().info().serviceName().toString()),
                updatedStateOnlyService(2, Provider.defaultService2().info().serviceName().toString()));

        Msg updateMsg = encodeDirectoryMsg(receivedUpdate, decodeIter);

        int ret = context.handler.readUpdateMsg(context.handler._stream, decodeIter, updateMsg, context.errorInfo);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        assertNotNull(context.handler.service(1));
        assertNotNull(context.handler.service(2));

        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq("WlDirectoryHandler.readUpdateMsg"),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));

        UpdateMsg callbackMsg = (UpdateMsg) msgCaptor.getValue();
        DirectoryUpdate callbackUpdate = (DirectoryUpdate) directoryMsgCaptor.getValue();

        assertEquals(MsgClasses.UPDATE, callbackMsg.msgClass());
        assertTrue(callbackMsg.checkHasMsgKey());
        assertEquals(0, callbackMsg.msgKey().filter());
        assertUpdateServiceId(callbackMsg, callbackUpdate, null);

        assertEquals(21, callbackUpdate.streamId());
        assertTrue(callbackUpdate.checkHasFilter());
        assertEquals(0, callbackUpdate.filter());
        assertTrue(callbackUpdate.serviceList().isEmpty());
    }

    private void assertRefreshServiceId(RefreshMsg callbackMsg, DirectoryRefresh callbackRefresh, Integer expectedServiceId)
    {
        if (expectedServiceId == null)
        {
            assertFalse(callbackMsg.msgKey().checkHasServiceId());
            assertFalse(callbackRefresh.checkHasServiceId());
        }
        else
        {
            assertTrue(callbackMsg.msgKey().checkHasServiceId());
            assertEquals(expectedServiceId.intValue(), callbackMsg.msgKey().serviceId());
            assertTrue(callbackRefresh.checkHasServiceId());
            assertEquals(expectedServiceId.intValue(), callbackRefresh.serviceId());
        }
    }

    private void assertUpdateServiceId(UpdateMsg callbackMsg, DirectoryUpdate callbackUpdate, Integer expectedServiceId)
    {
        if (expectedServiceId == null)
        {
            assertFalse(callbackMsg.msgKey().checkHasServiceId());
            assertFalse(callbackUpdate.checkHasServiceId());
        }
        else
        {
            assertTrue(callbackMsg.msgKey().checkHasServiceId());
            assertEquals(expectedServiceId.intValue(), callbackMsg.msgKey().serviceId());
            assertTrue(callbackUpdate.checkHasServiceId());
            assertEquals(expectedServiceId.intValue(), callbackUpdate.serviceId());
        }
    }

    private void cacheServices(WlDirectoryHandler handler, Service... services)
    {
        assertEquals(ReactorReturnCodes.SUCCESS,
                handler._serviceCache.processServiceList(Arrays.asList(services), CodecFactory.createMsg(), new ReactorErrorInfo()));
    }

    private WlRequest createAndRegisterDirectoryRequest(DirectoryHandlerTestContext context, int streamId,
                                                        long filter, Integer serviceId)
    {
        return createAndRegisterDirectoryRequest(context, streamId, serviceId, true, filter);
    }

    private WlRequest createAndRegisterDirectoryRequestWithoutFilter(DirectoryHandlerTestContext context, int streamId,
                                                                     Integer serviceId)
    {
        return createAndRegisterDirectoryRequest(context, streamId, serviceId, false, 0);
    }

    private WlRequest createAndRegisterDirectoryRequest(DirectoryHandlerTestContext context, int streamId,
                                                        Integer serviceId, boolean hasFilter, long filter)
    {
        WlRequest wlRequest = ReactorFactory.createWlRequest();
        wlRequest.state(WlRequest.State.PENDING_REFRESH);
        wlRequest.stream(context.handler._stream);

        RequestMsg requestMsg = (RequestMsg) CodecFactory.createMsg();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.domainType(DomainTypes.SOURCE);
        requestMsg.streamId(streamId);
        requestMsg.applyStreaming();
        if (hasFilter)
        {
            requestMsg.msgKey().applyHasFilter();
            requestMsg.msgKey().filter(filter);
        }
        if (serviceId != null)
        {
            requestMsg.msgKey().applyHasServiceId();
            requestMsg.msgKey().serviceId(serviceId);
        }

        wlRequest._requestMsg = requestMsg;
        context.handler._stream.userRequestList().add(wlRequest);

        WlInteger tableKey = ReactorFactory.createWlInteger();
        tableKey.value(streamId);
        wlRequest.tableKey(tableKey);
        context.watchlist.streamIdtoWlRequestTable().put(tableKey, wlRequest);

        return wlRequest;
    }

    private DirectoryRefresh createDirectoryRefresh(int streamId, long filter, boolean clearCache, Service... services)
    {
        return createDirectoryRefresh(streamId, filter, clearCache, true, services);
    }

    private DirectoryRefresh createDirectoryRefresh(int streamId, long filter, boolean clearCache, boolean solicited,
                                                    Service... services)
    {
        DirectoryRefresh directoryRefresh = (DirectoryRefresh) DirectoryMsgFactory.createMsg();
        directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);
        directoryRefresh.streamId(streamId);
        directoryRefresh.filter(filter);
        if (solicited) {
            directoryRefresh.applySolicited();
        }
        if (clearCache) {
            directoryRefresh.applyClearCache();
        }
        directoryRefresh.state().streamState(StreamStates.OPEN);
        directoryRefresh.state().dataState(DataStates.OK);
        directoryRefresh.state().code(StateCodes.NONE);
        directoryRefresh.state().text().data("Directory refresh complete");
        directoryRefresh.serviceList().addAll(Arrays.asList(services));
        return directoryRefresh;
    }

    private DirectoryUpdate createDirectoryUpdate(int streamId, long filter, Service... services)
    {
        DirectoryUpdate directoryUpdate = (DirectoryUpdate) DirectoryMsgFactory.createMsg();
        directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdate.streamId(streamId);
        directoryUpdate.applyHasFilter();
        directoryUpdate.filter(filter);
        directoryUpdate.serviceList().addAll(Arrays.asList(services));
        return directoryUpdate;
    }

    private Msg encodeDirectoryMsg(DirectoryMsg directoryMsg, DecodeIterator targetDecodeIterator)
    {
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(4096));
        EncodeIterator localEncodeIterator = CodecFactory.createEncodeIterator();
        Msg msg = CodecFactory.createMsg();

        targetDecodeIterator.clear();
        localEncodeIterator.clear();
        localEncodeIterator.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertTrue(directoryMsg.encode(localEncodeIterator) >= CodecReturnCodes.SUCCESS);

        targetDecodeIterator.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(targetDecodeIterator));

        return msg;
    }

    private void captureDirectoryCallbacks(DirectoryHandlerTestContext context, List<String> callbackLocations,
                                           List<Msg> callbackMsgs, List<DirectoryMsg> callbackDirectoryMsgs)
    {
        doAnswer(invocation -> {
            callbackLocations.add(invocation.getArgument(0));

            Msg copiedMsg = CodecFactory.createMsg();
            ((Msg) invocation.getArgument(3)).copy(copiedMsg, CopyMsgFlags.ALL_FLAGS);
            callbackMsgs.add(copiedMsg);

            DirectoryMsg copiedDirectoryMsg = DirectoryMsgFactory.createMsg();
            TestUtil.copyDirectoryMsg(invocation.getArgument(4), copiedDirectoryMsg);
            callbackDirectoryMsgs.add(copiedDirectoryMsg);

            return ReactorCallbackReturnCodes.SUCCESS;
        }).when(context.reactor).sendAndHandleDirectoryMsgCallback(anyString(), any(ReactorChannel.class),
                nullable(TransportBuffer.class), any(Msg.class), any(DirectoryMsg.class),
                any(WlRequest.class), any(ReactorErrorInfo.class));
    }

    private void cacheDefaultServicesAndClearInvocations(DirectoryHandlerTestContext context)
    {
        cacheServices(context.handler,
                addedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                addedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()));
        clearInvocations(context.itemHandler);
    }

    private DirectoryRefresh createRefreshWithUpdatedService1AndDeletedService2(int streamId)
    {
        return createDirectoryRefresh(
                streamId,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                false,
                updatedService(Provider.defaultService(), 1,
                        Provider.defaultService().info().serviceName().toString()),
                buildServiceFromTemplate(Provider.defaultService2(), MapEntryActions.DELETE, 2,
                        Provider.defaultService2().info().serviceName().toString()));
    }

    private DirectoryRefresh createRefreshWithUpdatedService2AndDeletedTestService(int streamId)
    {
        return createDirectoryRefresh(
                streamId,
                Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE,
                false,
                updatedService(Provider.defaultService2(), 2,
                        Provider.defaultService2().info().serviceName().toString()),
                buildServiceFromTemplate(Provider.defaultService(), MapEntryActions.DELETE, 1, "TEST_SERVICE"));
    }

    private CapturedDirectoryCallback captureSingleDirectoryCallback(DirectoryHandlerTestContext context,
                                                                    String location, WlRequest wlRequest)
    {
        ArgumentCaptor<Msg> msgCaptor = ArgumentCaptor.forClass(Msg.class);
        ArgumentCaptor<DirectoryMsg> directoryMsgCaptor = ArgumentCaptor.forClass(DirectoryMsg.class);
        verify(context.reactor, times(1))
                .sendAndHandleDirectoryMsgCallback(eq(location),
                        same(context.reactorChannel), nullable(TransportBuffer.class), msgCaptor.capture(),
                        directoryMsgCaptor.capture(), same(wlRequest), same(context.errorInfo));
        return new CapturedDirectoryCallback(msgCaptor.getValue(), directoryMsgCaptor.getValue());
    }

    private Service buildServiceFromTemplate(Service template, int action, int serviceId, String serviceName)
    {
        Service service = DirectoryMsgFactory.createService();
        template.copy(service);
        service.action(action);
        service.serviceId(serviceId);
        if (service.checkHasInfo())
        {
            service.info().serviceName().data(serviceName);
        }
        return service;
    }

    private Service addedService(Service template, int serviceId, String serviceName)
    {
        return buildServiceFromTemplate(template, MapEntryActions.ADD, serviceId, serviceName);
    }

    private Service updatedService(Service template, int serviceId, String serviceName)
    {
        return buildServiceFromTemplate(template, MapEntryActions.UPDATE, serviceId, serviceName);
    }

    private Service addedServiceWithLoad(Service template, int serviceId, String serviceName, int openWindow)
    {
        Service service = addedService(template, serviceId, serviceName);
        service.applyHasLoad();
        service.load().applyHasOpenWindow();
        service.load().openWindow(openWindow);
        return service;
    }

    private Service addedInfoOnlyService(int serviceId, String serviceName)
    {
        Service service = addedService(Provider.defaultService(), serviceId, serviceName);
        service.flags(service.flags() & ~ServiceFlags.HAS_STATE);
        return service;
    }

    private Service addedStateOnlyService(int serviceId, String serviceName)
    {
        Service service = addedService(Provider.defaultService(), serviceId, serviceName);
        service.flags(service.flags() & ~ServiceFlags.HAS_INFO);
        return service;
    }

    private Service addedStateOnlyServiceWithStatus(int serviceId, String serviceName, int serviceState,
                                                    int dataState, int streamState, int acceptingRequests)
    {
        Service service = addedStateOnlyService(serviceId, serviceName);
        service.state().action(FilterEntryActions.SET);
        service.state().applyHasStatus();
        service.state().status().dataState(dataState);
        service.state().status().streamState(streamState);
        service.state().applyHasAcceptingRequests();
        service.state().acceptingRequests(acceptingRequests);
        service.state().serviceState(serviceState);
        return service;
    }

    private Service updatedStateOnlyService(int serviceId, String serviceName)
    {
        Service service = updatedService(Provider.defaultService(), serviceId, serviceName);
        service.flags(service.flags() & ~ServiceFlags.HAS_INFO);
        return service;
    }

    private Service serviceById(List<Service> services, int serviceId)
    {
        for (Service service : services)
        {
            if (service.serviceId() == serviceId)
            {
                return service;
            }
        }
        return null;
    }

    private static class CapturedDirectoryCallback
    {
        private final Msg msg;
        private final DirectoryMsg directoryMsg;

        private CapturedDirectoryCallback(Msg msg, DirectoryMsg directoryMsg)
        {
            this.msg = msg;
            this.directoryMsg = directoryMsg;
        }
    }

    private static class DirectoryHandlerTestContext
    {
        private final Reactor reactor = mock(Reactor.class);
        private final ReactorChannel reactorChannel = mock(ReactorChannel.class);
        private final WlItemHandler itemHandler = mock(WlItemHandler.class);
        private final Watchlist watchlist;
        private final WlDirectoryHandler handler;
        private final ReactorErrorInfo errorInfo = new ReactorErrorInfo();

        private DirectoryHandlerTestContext()
        {
            this(true);
        }

        private DirectoryHandlerTestContext(boolean receivedRefresh)
        {
            ConsumerRole consumerRole = ReactorFactory.createConsumerRole();

            when(reactorChannel.reactor()).thenReturn(reactor);
            when(reactor.reactorHandlesWarmStandby(reactorChannel)).thenReturn(false);
            when(reactor.sendAndHandleDirectoryMsgCallback(anyString(), any(ReactorChannel.class),
                    any(TransportBuffer.class), any(Msg.class), any(DirectoryMsg.class),
                    any(WlRequest.class), any(ReactorErrorInfo.class)))
                    .thenReturn(ReactorCallbackReturnCodes.SUCCESS);
            when(reactor.sendAndHandleDefaultMsgCallback(anyString(), any(ReactorChannel.class),
                    any(TransportBuffer.class), any(Msg.class), any(WlRequest.class), any(ReactorErrorInfo.class)))
                    .thenReturn(ReactorCallbackReturnCodes.SUCCESS);
            when(itemHandler.serviceAdded(any(WlService.class))).thenReturn(ReactorReturnCodes.SUCCESS);
            when(itemHandler.serviceUpdated(any(WlService.class), anyBoolean())).thenReturn(ReactorReturnCodes.SUCCESS);
            when(itemHandler.serviceDeleted(any(WlService.class), anyBoolean())).thenReturn(ReactorReturnCodes.SUCCESS);

            watchlist = new Watchlist(reactorChannel, consumerRole);
            watchlist._itemHandler = itemHandler;
            handler = watchlist.directoryHandler();
            handler._receivedRefresh = receivedRefresh;
            handler._stream.state().streamState(StreamStates.OPEN);
            handler._stream.state().dataState(DataStates.OK);
        }
    }
}
