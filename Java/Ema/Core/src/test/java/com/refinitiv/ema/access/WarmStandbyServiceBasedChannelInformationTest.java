/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;


import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyChannelDetails;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyServiceBasedChannelInfoEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyServiceBasedChannelInfoEvent.WSBPerChannelServiceInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyServiceBasedChannelInfoEvent.WSBService;
import org.junit.Test;

import java.util.Arrays;
import java.util.Collections;

import static com.refinitiv.ema.access.ChannelInformation.*;
import static com.refinitiv.ema.access.WarmStandbyChannelInformation.*;
import static org.junit.Assert.*;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

public class WarmStandbyServiceBasedChannelInformationTest
{
    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullEvent_whenCreate_thenThrowException()
    {
        WarmStandbyServiceBasedChannelInformation.create(null);
    }

    @Test
    public void givenNullPerChannelServiceList_whenCreate_thenReturnEmptyServiceBasedInformation()
    {
        ReactorWarmStandbyServiceBasedChannelInfoEvent event = mock(ReactorWarmStandbyServiceBasedChannelInfoEvent.class);
        when(event.perChannelServiceList()).thenReturn(null);

        WarmStandbyServiceBasedChannelInformation info = WarmStandbyServiceBasedChannelInformation.create(event);

        assertNotNull(info);
        assertEquals(WarmStandbyMode.SERVICE_BASED, info.warmStandbyMode());
        assertTrue(info.perChannelServiceList().isEmpty());
    }

    @Test
    public void givenEmptyPerChannelServiceList_whenCreate_thenReturnEmptyServiceBasedInformation()
    {
        ReactorWarmStandbyServiceBasedChannelInfoEvent event = mock(ReactorWarmStandbyServiceBasedChannelInfoEvent.class);
        when(event.perChannelServiceList()).thenReturn(Collections.emptyList());

        WarmStandbyServiceBasedChannelInformation info = WarmStandbyServiceBasedChannelInformation.create(event);

        assertNotNull(info);
        assertEquals(WarmStandbyMode.SERVICE_BASED, info.warmStandbyMode());
        assertTrue(info.perChannelServiceList().isEmpty());
    }

    @Test
    public void givenPerChannelEntriesWithNulls_whenCreate_thenPopulateMappedDataAndSkipNulls()
    {
        ReactorWarmStandbyServiceBasedChannelInfoEvent event = mock(ReactorWarmStandbyServiceBasedChannelInfoEvent.class);
        WSBPerChannelServiceInfo firstEntry = mock(WSBPerChannelServiceInfo.class);
        WSBPerChannelServiceInfo secondEntry = mock(WSBPerChannelServiceInfo.class);
        ChannelInfo firstChannelInfo = mock(ChannelInfo.class);
        when(firstChannelInfo.name()).thenReturn("channelA");
        ReactorWarmStandbyChannelDetails firstChannel = mockChannelDetails("hostA", 14002,
                ConnectionTypes.SOCKET, -1, ProtocolType.RWF, firstChannelInfo);
        WSBService activeService = mockService(1, "SERVICE_A", true);
        WSBService standbyService = mockService(2, null, false);

        when(firstEntry.channel()).thenReturn(firstChannel);
        when(firstEntry.serviceList()).thenReturn(Arrays.asList(activeService, null, standbyService));

        when(secondEntry.channel()).thenReturn(null);
        when(secondEntry.serviceList()).thenReturn(null);

        when(event.perChannelServiceList()).thenReturn(Arrays.asList(null, firstEntry, secondEntry));

        WarmStandbyServiceBasedChannelInformation info = WarmStandbyServiceBasedChannelInformation.create(event);

        assertEquals(WarmStandbyMode.SERVICE_BASED, info.warmStandbyMode());
        assertEquals(2, info.perChannelServiceList().size());

        WarmStandbyServiceBasedChannelInformation.WarmStandbyPerChannelServiceInfo mappedFirstEntry = info.perChannelServiceList().get(0);
        assertNotNull(mappedFirstEntry.channel());
        assertEquals("hostA", mappedFirstEntry.channel().hostname());
        assertEquals("channelA", mappedFirstEntry.channel().channelName());
        assertEquals(14002, mappedFirstEntry.channel().port());
        assertEquals(ConnectionType.SOCKET, mappedFirstEntry.channel().connectionType());
        assertEquals(ConnectionType.UNIDENTIFIED, mappedFirstEntry.channel().encryptedConnectionType());
        assertEquals(ProtocolType.RWF, mappedFirstEntry.channel().protocolType());
        assertSame(firstChannelInfo, mappedFirstEntry.channel().userSpecObject());

        assertEquals(2, mappedFirstEntry.serviceList().size());
        WarmStandbyServiceBasedChannelInformation.WarmStandbyService firstService = mappedFirstEntry.serviceList().get(0);
        assertEquals(1, firstService.serviceId());
        assertEquals("SERVICE_A", firstService.serviceName());
        assertTrue(firstService.isActive());

        WarmStandbyServiceBasedChannelInformation.WarmStandbyService secondService = mappedFirstEntry.serviceList().get(1);
        assertEquals(2, secondService.serviceId());
        assertNull(secondService.serviceName());
        assertFalse(secondService.isActive());

        WarmStandbyServiceBasedChannelInformation.WarmStandbyPerChannelServiceInfo mappedSecondEntry = info.perChannelServiceList().get(1);
        assertNull(mappedSecondEntry.channel());
        assertTrue(mappedSecondEntry.serviceList().isEmpty());
    }

    @Test
    public void givenEntryWithEmptyServiceList_whenCreate_thenKeepEntryWithEmptyServices()
    {
        ReactorWarmStandbyServiceBasedChannelInfoEvent event = mock(ReactorWarmStandbyServiceBasedChannelInfoEvent.class);
        WSBPerChannelServiceInfo entry = mock(WSBPerChannelServiceInfo.class);
        ReactorWarmStandbyChannelDetails channel = mockChannelDetails("hostB", 15000,
                ConnectionTypes.HTTP, -1, ProtocolType.JSON, null);

        when(entry.channel()).thenReturn(channel);
        when(entry.serviceList()).thenReturn(Collections.emptyList());
        when(event.perChannelServiceList()).thenReturn(Collections.singletonList(entry));

        WarmStandbyServiceBasedChannelInformation info = WarmStandbyServiceBasedChannelInformation.create(event);

        assertEquals(1, info.perChannelServiceList().size());
        WarmStandbyServiceBasedChannelInformation.WarmStandbyPerChannelServiceInfo mappedEntry = info.perChannelServiceList().get(0);
        assertNotNull(mappedEntry.channel());
        assertEquals("hostB", mappedEntry.channel().hostname());
        assertNull(mappedEntry.channel().channelName());
        assertTrue(mappedEntry.serviceList().isEmpty());
    }

    @Test
    public void givenInformation_whenToString_thenIncludeModeGroupAndPerChannelDetails()
    {
        ReactorWarmStandbyServiceBasedChannelInfoEvent event = mock(ReactorWarmStandbyServiceBasedChannelInfoEvent.class);
        WSBPerChannelServiceInfo entry = mock(WSBPerChannelServiceInfo.class);
        ChannelInfo channelInfo = mock(ChannelInfo.class);
        when(channelInfo.name()).thenReturn("channelC");
        ReactorWarmStandbyChannelDetails channel = mockChannelDetails("hostC", 16000,
                ConnectionTypes.ENCRYPTED, ConnectionTypes.WEBSOCKET, ProtocolType.RWF, channelInfo);
        WSBService service = mockService(7, "SERVICE_C", true);

        when(entry.channel()).thenReturn(channel);
        when(entry.serviceList()).thenReturn(Collections.singletonList(service));
        when(event.perChannelServiceList()).thenReturn(Collections.singletonList(entry));

        WarmStandbyServiceBasedChannelInformation info = WarmStandbyServiceBasedChannelInformation.create(event);
        info.warmStandbyGroupName("wsbGroupB");

        String text = info.toString();

        assertTrue(text.contains("WarmStandbyServiceBasedChannelInformation:"));
        assertTrue(text.contains("warmStandbyMode: SERVICE_BASED"));
        assertTrue(text.contains("warmStandbyGroupName: wsbGroupB"));
        assertTrue(text.contains("perChannelServiceList:"));
        assertTrue(text.contains("channelName='channelC'"));
        assertTrue(text.contains("hostName='hostC'"));
        assertTrue(text.contains("connectionType='encrypted'"));
        assertTrue(text.contains("encryptedConnectionType='webSocket'"));
        assertTrue(text.contains("serviceId=7"));
        assertTrue(text.contains("serviceName='SERVICE_C'"));
    }

    private ReactorWarmStandbyChannelDetails mockChannelDetails(String hostname, int port, int connectionType,
                                                                int encryptedConnectionType, int protocolType,
                                                                Object userSpecObject)
    {
        ReactorWarmStandbyChannelDetails details = mock(ReactorWarmStandbyChannelDetails.class);
        when(details.hostname()).thenReturn(hostname);
        when(details.port()).thenReturn(port);
        when(details.connectionType()).thenReturn(connectionType);
        when(details.encryptedConnectionType()).thenReturn(encryptedConnectionType);
        when(details.protocolType()).thenReturn(protocolType);
        when(details.userSpecObject()).thenReturn(userSpecObject);
        return details;
    }

    private WSBService mockService(int serviceId, String serviceName, boolean isActive)
    {
        WSBService service = mock(WSBService.class);
        when(service.serviceId()).thenReturn(serviceId);
        when(service.serviceName()).thenReturn(serviceName);
        when(service.isActive()).thenReturn(isActive);
        return service;
    }
}
