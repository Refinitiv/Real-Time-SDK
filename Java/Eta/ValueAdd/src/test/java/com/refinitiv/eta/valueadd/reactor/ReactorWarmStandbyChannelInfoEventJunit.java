/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import org.junit.*;

import java.util.*;
import java.util.concurrent.locks.ReentrantLock;

import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.*;
import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyServiceBasedChannelInfoEvent.*;
import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

public class ReactorWarmStandbyChannelInfoEventJunit
{
    private final ReactorChannel reactorChannel = mock(ReactorChannel.class);
    private final Watchlist watchlist = mock(Watchlist.class);
    private final WlDirectoryHandler wlDirectoryHandler = mock(WlDirectoryHandler.class);
    private final ReactorChannel activeReactorChannel = mock(ReactorChannel.class);
    private final ReactorChannel standbyReactorChannel = mock(ReactorChannel.class);
    private final ReactorWarmStandbyHandler warmStandByHandlerImpl = mock(ReactorWarmStandbyHandler.class);
    private final ReactorWarmStandbyGroupImpl warmStandbyGroup = mock(ReactorWarmStandbyGroupImpl.class);
    private final Channel activeChannel = mock(Channel.class);
    private final Channel standbyChannel = mock(Channel.class);
    private final WlService wlService1 = mock(WlService.class);
    private final WlService wlService2 = mock(WlService.class);
    private final Service service1 = mock(Service.class);
    private final Service service2 = mock(Service.class);
    private final Service.ServiceInfo info1 = mock(Service.ServiceInfo.class);
    private final Service.ServiceInfo info2 = mock(Service.ServiceInfo.class);
    private final Buffer buffer1 = mock(Buffer.class);
    private final Buffer buffer2 = mock(Buffer.class);
    private final List<ReactorChannel> channelsList = List.of(activeReactorChannel, standbyReactorChannel);
    private final Object userSpecObject = new Object();
    private final TestReactor reactor = new TestReactor();

    @Before
    public void setUp()
    {
        when(activeReactorChannel.channel()).thenReturn(activeChannel);
        when(activeReactorChannel.watchlist()).thenReturn(watchlist);
        when(standbyReactorChannel.channel()).thenReturn(standbyChannel);
        when(standbyReactorChannel.watchlist()).thenReturn(watchlist);

        when(watchlist.directoryHandler()).thenReturn(wlDirectoryHandler);

        when(activeChannel.hostname()).thenReturn("testHost1");
        when(activeChannel.port()).thenReturn(14002);
        when(activeChannel.connectionType()).thenReturn(ConnectionTypes.ENCRYPTED);
        when(activeChannel.encryptedConnectionType()).thenReturn(ConnectionTypes.SOCKET);
        when(activeChannel.protocolType()).thenReturn(2);
        when(activeChannel.userSpecObject()).thenReturn(userSpecObject);
        when(standbyChannel.hostname()).thenReturn("testHost2");
        when(standbyChannel.port()).thenReturn(14003);
        when(standbyChannel.connectionType()).thenReturn(ConnectionTypes.UNIDIR_SHMEM);
        when(standbyChannel.encryptedConnectionType()).thenReturn(-1);
        when(standbyChannel.protocolType()).thenReturn(4);
        when(standbyChannel.userSpecObject()).thenReturn(userSpecObject);

        when(warmStandByHandlerImpl.activeReactorChannel()).thenReturn(activeReactorChannel);
        when(warmStandByHandlerImpl.channelList()).thenReturn(channelsList);
        when(warmStandByHandlerImpl.currentWarmStandbyGroupImpl()).thenReturn(warmStandbyGroup);
        when(warmStandByHandlerImpl.warmStandByHandlerLock()).thenAnswer(invocation -> new ReentrantLock());

        when(wlService1.rdmService()).thenReturn(service1);
        when(service1.info()).thenReturn(info1);
        when(info1.serviceName()).thenReturn(buffer1);
        when(buffer1.toString()).thenReturn("service1");

        when(wlService2.rdmService()).thenReturn(service2);
        when(service2.info()).thenReturn(info2);
        when(info2.serviceName()).thenReturn(buffer2);
        when(buffer2.toString()).thenReturn("service2");

        // ServiceIds
        WlInteger serviceId1 = new WlInteger();
        serviceId1.value(101);
        WlInteger serviceId2 = new WlInteger();
        serviceId2.value(103);
        // Services
        ReactorWSBService service1 = new ReactorWSBService();
        service1.channels = channelsList;
        service1.activeChannel = activeReactorChannel;
        service1.serviceId = serviceId1;
        ReactorWSBService service2 = new ReactorWSBService();
        service2.channels = channelsList;
        service2.activeChannel = standbyReactorChannel;
        service2.serviceId = serviceId2;
        // Per service by ID map
        HashMap<WlInteger, ReactorWSBService> perServiceById = new HashMap<>();
        perServiceById.put(serviceId1, service1);
        perServiceById.put(serviceId2, service2);
        warmStandbyGroup._perServiceById = perServiceById;

        wlDirectoryHandler._serviceCache = new WlServiceCache(watchlist);
        HashMap<WlInteger,WlService> servicesByIdTable = new HashMap<>();
        servicesByIdTable.put(serviceId1, wlService1);
        servicesByIdTable.put(serviceId2, wlService2);
        wlDirectoryHandler._serviceCache._servicesByIdTable = servicesByIdTable;
    }

    @Test
    public void givenReactorChannel_whenCreate_thenCreateReactorWarmStandbyLoginBasedChannelInfoEvent()
    {
        reactorChannel.warmStandByHandlerImpl = warmStandByHandlerImpl;

        ReactorWarmStandbyLoginBasedChannelInfoEvent wsbChannelInfoEvent = ReactorWarmStandbyLoginBasedChannelInfoEvent
                .create(reactorChannel);

        assertNotNull(wsbChannelInfoEvent);
        assertEquals(LOGIN_BASED, wsbChannelInfoEvent.warmStandbyMode());

        assertEquals("testHost1", wsbChannelInfoEvent.activeChannel().hostname());
        assertEquals(14002, wsbChannelInfoEvent.activeChannel().port());
        assertEquals(ConnectionTypes.ENCRYPTED, wsbChannelInfoEvent.activeChannel().connectionType());
        assertEquals(ConnectionTypes.SOCKET, wsbChannelInfoEvent.activeChannel().encryptedConnectionType());
        assertEquals(2, wsbChannelInfoEvent.activeChannel().protocolType());
        assertEquals(userSpecObject, wsbChannelInfoEvent.activeChannel().userSpecObject());

        assertEquals(2, wsbChannelInfoEvent.channelsList().size());

        List<ReactorWarmStandbyChannelDetails>  channelList = wsbChannelInfoEvent.channelsList();
        channelList.sort(Comparator.comparing(ReactorWarmStandbyChannelDetails::hostname));

        assertEquals("testHost1", channelList.get(0).hostname());
        assertEquals(14002, channelList.get(0).port());
        assertEquals(ConnectionTypes.ENCRYPTED, channelList.get(0).connectionType());
        assertEquals(ConnectionTypes.SOCKET, channelList.get(0).encryptedConnectionType());
        assertEquals(2, channelList.get(0).protocolType());
        assertEquals(userSpecObject, channelList.get(0).userSpecObject());
        assertEquals("testHost2", channelList.get(1).hostname());
        assertEquals(14003, channelList.get(1).port());
        assertEquals(ConnectionTypes.UNIDIR_SHMEM, channelList.get(1).connectionType());
        assertEquals(-1, channelList.get(1).encryptedConnectionType());
        assertEquals(4, channelList.get(1).protocolType());
        assertEquals(userSpecObject, channelList.get(1).userSpecObject());
    }

    @Test
    public void givenNullAsReactorChannel_whenCreate_thenCreateReactorWarmStandbyLoginBasedChannelInfoEvent()
    {
        ReactorWarmStandbyLoginBasedChannelInfoEvent wsbChannelInfoEvent = ReactorWarmStandbyLoginBasedChannelInfoEvent
                .create(null);

        assertNotNull(wsbChannelInfoEvent);
        assertEquals(LOGIN_BASED, wsbChannelInfoEvent.warmStandbyMode());
        assertNull(wsbChannelInfoEvent.activeChannel());
        assertTrue(wsbChannelInfoEvent.channelsList().isEmpty());
    }

    @Test
    public void givenReactorChannelWithNullAsReactorWarmStandbyHandler_whenCreate_thenCreateReactorWarmStandbyLoginBasedChannelInfoEvent()
    {
        ReactorWarmStandbyLoginBasedChannelInfoEvent wsbChannelInfoEvent = ReactorWarmStandbyLoginBasedChannelInfoEvent
                .create(reactorChannel);

        assertNotNull(wsbChannelInfoEvent);
        assertEquals(LOGIN_BASED, wsbChannelInfoEvent.warmStandbyMode());
        assertNull(wsbChannelInfoEvent.activeChannel());
        assertTrue(wsbChannelInfoEvent.channelsList().isEmpty());
    }

    @Test
    public void givenReactorChannelWithoutServiceNamesList_whenCreate_thenCreateReactorWarmStandbyServiceBasedChannelInfoEvent()
    {
        reactorChannel.warmStandByHandlerImpl = warmStandByHandlerImpl;

        ReactorWarmStandbyServiceBasedChannelInfoEvent wsbChannelInfoEvent = create(reactorChannel, null);

        assertNotNull(wsbChannelInfoEvent);
        assertEquals(SERVICE_BASED, wsbChannelInfoEvent.warmStandbyMode());

        assertEquals(2, wsbChannelInfoEvent.perChannelServiceList().size());

        List<WSBPerChannelServiceInfo>  perChannelServiceList = wsbChannelInfoEvent.perChannelServiceList();
        perChannelServiceList.sort(Comparator.comparing(info -> info.channel().hostname()));

        WSBPerChannelServiceInfo wsbChannelInfo = perChannelServiceList.get(0);
        assertNotNull(wsbChannelInfo);
        assertEquals("testHost1", wsbChannelInfo.channel().hostname());
        assertEquals(14002, wsbChannelInfo.channel().port());
        assertEquals(ConnectionTypes.ENCRYPTED, wsbChannelInfo.channel().connectionType());
        assertEquals(ConnectionTypes.SOCKET, wsbChannelInfo.channel().encryptedConnectionType());
        assertEquals(2, wsbChannelInfo.channel().protocolType());
        assertEquals(userSpecObject, wsbChannelInfo.channel().userSpecObject());
        assertEquals(2, wsbChannelInfo.serviceList().size());
        assertEquals(101, wsbChannelInfo.serviceList().get(0).serviceId());
        assertEquals("service1", wsbChannelInfo.serviceList().get(0).serviceName());
        assertTrue(wsbChannelInfo.serviceList().get(0).isActive());
        assertEquals(103, wsbChannelInfo.serviceList().get(1).serviceId());
        assertEquals("service2", wsbChannelInfo.serviceList().get(1).serviceName());
        assertFalse(wsbChannelInfo.serviceList().get(1).isActive());

        wsbChannelInfo = perChannelServiceList.get(1);
        assertNotNull(wsbChannelInfo);
        assertEquals("testHost2", wsbChannelInfo.channel().hostname());
        assertEquals(14003, wsbChannelInfo.channel().port());
        assertEquals(ConnectionTypes.UNIDIR_SHMEM, wsbChannelInfo.channel().connectionType());
        assertEquals(-1, wsbChannelInfo.channel().encryptedConnectionType());
        assertEquals(4, wsbChannelInfo.channel().protocolType());
        assertEquals(userSpecObject, wsbChannelInfo.channel().userSpecObject());
        assertEquals(2, wsbChannelInfo.serviceList().size());
        assertEquals(101, wsbChannelInfo.serviceList().get(0).serviceId());
        assertEquals("service1", wsbChannelInfo.serviceList().get(0).serviceName());
        assertFalse(wsbChannelInfo.serviceList().get(0).isActive());
        assertEquals(103, wsbChannelInfo.serviceList().get(1).serviceId());
        assertEquals("service2", wsbChannelInfo.serviceList().get(1).serviceName());
        assertTrue(wsbChannelInfo.serviceList().get(1).isActive());
    }

    @Test
    public void givenReactorChannelWithServiceNamesList_whenCreate_thenCreateReactorWarmStandbyServiceBasedChannelInfoEvent()
    {
        reactorChannel.warmStandByHandlerImpl = warmStandByHandlerImpl;

        ReactorWarmStandbyServiceBasedChannelInfoEvent wsbChannelInfoEvent = create(reactorChannel, Set.of("service1"));

        assertNotNull(wsbChannelInfoEvent);
        assertEquals(SERVICE_BASED, wsbChannelInfoEvent.warmStandbyMode());

        assertEquals(2, wsbChannelInfoEvent.perChannelServiceList().size());

        List<WSBPerChannelServiceInfo>  perChannelServiceList = wsbChannelInfoEvent.perChannelServiceList();
        perChannelServiceList.sort(Comparator.comparing(info -> info.channel().hostname()));

        WSBPerChannelServiceInfo wsbChannelInfo = perChannelServiceList.get(0);
        assertNotNull(wsbChannelInfo);
        assertEquals("testHost1", wsbChannelInfo.channel().hostname());
        assertEquals(14002, wsbChannelInfo.channel().port());
        assertEquals(ConnectionTypes.ENCRYPTED, wsbChannelInfo.channel().connectionType());
        assertEquals(ConnectionTypes.SOCKET, wsbChannelInfo.channel().encryptedConnectionType());
        assertEquals(2, wsbChannelInfo.channel().protocolType());
        assertEquals(userSpecObject, wsbChannelInfo.channel().userSpecObject());
        assertEquals(1, wsbChannelInfo.serviceList().size());
        assertEquals(101, wsbChannelInfo.serviceList().get(0).serviceId());
        assertEquals("service1", wsbChannelInfo.serviceList().get(0).serviceName());
        assertTrue(wsbChannelInfo.serviceList().get(0).isActive());

        wsbChannelInfo = perChannelServiceList.get(1);
        assertNotNull(wsbChannelInfo);
        assertEquals("testHost2", wsbChannelInfo.channel().hostname());
        assertEquals(14003, wsbChannelInfo.channel().port());
        assertEquals(ConnectionTypes.UNIDIR_SHMEM, wsbChannelInfo.channel().connectionType());
        assertEquals(-1, wsbChannelInfo.channel().encryptedConnectionType());
        assertEquals(4, wsbChannelInfo.channel().protocolType());
        assertEquals(userSpecObject, wsbChannelInfo.channel().userSpecObject());
        assertEquals(1, wsbChannelInfo.serviceList().size());
        assertEquals(101, wsbChannelInfo.serviceList().get(0).serviceId());
        assertEquals("service1", wsbChannelInfo.serviceList().get(0).serviceName());
        assertFalse(wsbChannelInfo.serviceList().get(0).isActive());
    }

    @Test
    public void givenNullAsReactorChannel_whenCreate_thenCreateReactorWarmStandbyServiceBasedChannelInfoEvent()
    {
        reactorChannel.warmStandByHandlerImpl = warmStandByHandlerImpl;

        ReactorWarmStandbyServiceBasedChannelInfoEvent wsbChannelInfoEvent = create(null, null);

        assertNotNull(wsbChannelInfoEvent);
        assertEquals(SERVICE_BASED, wsbChannelInfoEvent.warmStandbyMode());

        assertTrue(wsbChannelInfoEvent.perChannelServiceList().isEmpty());
    }

    @Test
    public void givenReactorChannelWithNullAsReactorWarmStandbyHandler_whenCreate_thenCreateReactorWarmStandbyServiceBasedChannelInfoEvent()
    {
        ReactorWarmStandbyServiceBasedChannelInfoEvent wsbChannelInfoEvent = create(reactorChannel, null);

        assertNotNull(wsbChannelInfoEvent);
        assertEquals(SERVICE_BASED, wsbChannelInfoEvent.warmStandbyMode());

        assertTrue(wsbChannelInfoEvent.perChannelServiceList().isEmpty());
    }

    @Test
    public void givenNullAsErrorInfo_whenGetWarmStandbyChannelInfo_thenReturnFailure()
    {
        List<String> serviceNamesList = new ArrayList<>();

        int ret = reactor._reactor.getWarmStandbyChannelInfo(reactorChannel,
                (event) -> ReactorCallbackReturnCodes.SUCCESS, serviceNamesList, null);

        assertEquals(ReactorReturnCodes.FAILURE, ret);
    }

    @Test
    public void givenNullAsReactorChannel_whenGetWarmStandbyChannelInfo_thenReturnFailure()
    {
        List<String> serviceNamesList = new ArrayList<>();
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();

        int ret = reactor._reactor.getWarmStandbyChannelInfo(null,
                (event) -> ReactorCallbackReturnCodes.SUCCESS, serviceNamesList, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, ret);
        assertEquals("Reactor.getWarmStandbyChannelInfo", errorInfo.location());
        assertEquals("ReactorChannel must be set to receive the WarmStandby Channel Info.",
                errorInfo.error().text());
    }

    @Test
    public void givenNullAsCallback_whenGetWarmStandbyChannelInfo_thenReturnFailure()
    {
        List<String> serviceNamesList = new ArrayList<>();
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();

        int ret = reactor._reactor.getWarmStandbyChannelInfo(reactorChannel, null, serviceNamesList, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, ret);
        assertEquals("Reactor.getWarmStandbyChannelInfo", errorInfo.location());
        assertEquals("ReactorWarmStandbyChannelInfoCallback must be set to receive the WarmStandby Channel Info.",
                errorInfo.error().text());
    }

    @Test
    public void givenNullAsServiceNames_whenGetWarmStandbyChannelInfo_thenReturnFailure()
    {
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();

        int ret = reactor._reactor.getWarmStandbyChannelInfo(reactorChannel,
                (event) -> ReactorCallbackReturnCodes.SUCCESS, null, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, ret);
        assertEquals("Reactor.getWarmStandbyChannelInfo", errorInfo.location());
        assertEquals("serviceNames cannot be null.", errorInfo.error().text());
    }

    @Test
    public void givenReactorIsShutdown_whenGetWarmStandbyChannelInfo_thenReturnFailure()
    {
        List<String> serviceNamesList = new ArrayList<>();
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        reactor._reactor._reactorActive = false;

        int ret = reactor._reactor.getWarmStandbyChannelInfo(reactorChannel,
                (event) -> ReactorCallbackReturnCodes.SUCCESS, serviceNamesList, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, ret);
        assertEquals("Reactor.getWarmStandbyChannelInfo", errorInfo.location());
        assertEquals("Reactor is shutdown.", errorInfo.error().text());
    }

    @Test
    public void givenReactorChannelIsClosed_whenGetWarmStandbyChannelInfo_thenReturnFailure()
    {
        List<String> serviceNamesList = new ArrayList<>();
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        when(reactorChannel.state()).thenReturn(ReactorChannel.State.CLOSED);

        int ret = reactor._reactor.getWarmStandbyChannelInfo(reactorChannel,
                (event) -> ReactorCallbackReturnCodes.SUCCESS, serviceNamesList, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, ret);
        assertEquals("Reactor.getWarmStandbyChannelInfo", errorInfo.location());
        assertEquals("ReactorChannel is not active.", errorInfo.error().text());
    }

    @Test
    public void givenWarmStandbyDisabled_whenGetWarmStandbyChannelInfo_thenReturnFailure()
    {
        List<String> serviceNamesList = new ArrayList<>();
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        when(reactorChannel.state()).thenReturn(ReactorChannel.State.UP);

        int ret = reactor._reactor.getWarmStandbyChannelInfo(reactorChannel,
                (event) -> ReactorCallbackReturnCodes.SUCCESS, serviceNamesList, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, ret);
        assertEquals("Reactor.getWarmStandbyChannelInfo", errorInfo.location());
        assertEquals("WarmStandby is not enabled or not initialized for this Channel.",
                errorInfo.error().text());
    }

    @Test
    public void givenCallbackReturnsFailure_whenGetWarmStandbyChannelInfo_thenReturnFailure()
    {
        List<String> serviceNamesList = new ArrayList<>();
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        reactorChannel.warmStandByHandlerImpl = warmStandByHandlerImpl;
        when(reactorChannel.state()).thenReturn(ReactorChannel.State.UP);
        when(warmStandbyGroup.warmStandbyMode()).thenReturn(LOGIN_BASED);

        int ret = reactor._reactor.getWarmStandbyChannelInfo(reactorChannel,
                (event) -> ReactorCallbackReturnCodes.FAILURE, serviceNamesList, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, ret);
        assertEquals("Reactor.getWarmStandbyChannelInfo", errorInfo.location());
        assertEquals("ReactorCallbackReturnCodes.FAILURE was returned from reactorWarmStandbyChannelInfoCallback.",
                errorInfo.error().text());
    }

    @Test
    public void givenIncorrectWarmStandbyMode_whenGetWarmStandbyChannelInfo_thenReturnFailure()
    {
        List<String> serviceNamesList = new ArrayList<>();
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        reactorChannel.warmStandByHandlerImpl = warmStandByHandlerImpl;
        when(reactorChannel.state()).thenReturn(ReactorChannel.State.UP);
        when(warmStandbyGroup.warmStandbyMode()).thenReturn(NONE);

        int ret = reactor._reactor.getWarmStandbyChannelInfo(reactorChannel,
                (event) -> ReactorCallbackReturnCodes.SUCCESS, serviceNamesList, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, ret);
        assertEquals("Reactor.getWarmStandbyChannelInfo", errorInfo.location());
        assertEquals("Failed to create the WarmStandby Channel Info event.",
                errorInfo.error().text());
    }

    @Test
    public void givenCallbackThrowException_whenGetWarmStandbyChannelInfo_thenReturnFailure()
    {
        List<String> serviceNamesList = new ArrayList<>();
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        reactorChannel.warmStandByHandlerImpl = warmStandByHandlerImpl;
        when(reactorChannel.state()).thenReturn(ReactorChannel.State.UP);
        when(warmStandbyGroup.warmStandbyMode()).thenReturn(LOGIN_BASED);

        int ret = reactor._reactor.getWarmStandbyChannelInfo(reactorChannel,
                (event) -> {throw new RuntimeException("Callback failed");}, serviceNamesList, errorInfo);

        assertEquals(ReactorReturnCodes.FAILURE, ret);
        assertEquals("Reactor.getWarmStandbyChannelInfo", errorInfo.location());
        assertEquals("Exception in getWarmStandbyChannelInfo: Callback failed",
                errorInfo.error().text());
    }

    @Test
    public void givenCallbackReturnsSuccess_whenGetWarmStandbyChannelInfo_thenReturnSuccess()
    {
        List<String> serviceNamesList = new ArrayList<>();
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        reactorChannel.warmStandByHandlerImpl = warmStandByHandlerImpl;
        when(reactorChannel.state()).thenReturn(ReactorChannel.State.UP);
        when(warmStandbyGroup.warmStandbyMode()).thenReturn(LOGIN_BASED);

        int ret = reactor._reactor.getWarmStandbyChannelInfo(reactorChannel,
                (event) -> ReactorCallbackReturnCodes.SUCCESS, serviceNamesList, errorInfo);

        assertEquals(ReactorReturnCodes.SUCCESS, ret);
        assertNull(errorInfo.error().text());
    }
}