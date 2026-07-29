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
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyLoginBasedChannelInfoEvent;
import org.junit.Test;

import java.util.Arrays;
import java.util.Collections;

import static org.junit.Assert.*;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

import static com.refinitiv.ema.access.ChannelInformation.*;
import static com.refinitiv.ema.access.WarmStandbyChannelInformation.*;

public class WarmStandbyLoginBasedChannelInformationTest
{
    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullEvent_whenCreate_thenThrowException()
    {
        WarmStandbyLoginBasedChannelInformation.create(null);
    }

    @Test
    public void givenNullChannelsList_whenCreate_thenReturnEmptyLoginBasedInformation()
    {
        ReactorWarmStandbyLoginBasedChannelInfoEvent event = mock(ReactorWarmStandbyLoginBasedChannelInfoEvent.class);
        when(event.channelsList()).thenReturn(null);

        WarmStandbyLoginBasedChannelInformation info = WarmStandbyLoginBasedChannelInformation.create(event);

        assertNotNull(info);
        assertEquals(WarmStandbyMode.LOGIN_BASED, info.warmStandbyMode());
        assertEquals(-1, info.activeChannelIndex());
        assertTrue(info.channelsList().isEmpty());
    }

    @Test
    public void givenEmptyChannelsList_whenCreate_thenReturnEmptyLoginBasedInformation()
    {
        ReactorWarmStandbyLoginBasedChannelInfoEvent event = mock(ReactorWarmStandbyLoginBasedChannelInfoEvent.class);
        when(event.channelsList()).thenReturn(Collections.emptyList());

        WarmStandbyLoginBasedChannelInformation info = WarmStandbyLoginBasedChannelInformation.create(event);

        assertNotNull(info);
        assertEquals(WarmStandbyMode.LOGIN_BASED, info.warmStandbyMode());
        assertEquals(-1, info.activeChannelIndex());
        assertTrue(info.channelsList().isEmpty());
    }

    @Test
    public void givenChannelsWithActiveChannelAndNullEntry_whenCreate_thenPopulateListAndSetActiveIndex()
    {
        ReactorWarmStandbyLoginBasedChannelInfoEvent event = mock(ReactorWarmStandbyLoginBasedChannelInfoEvent.class);
        ChannelInfo activeChannelInfo = mock(ChannelInfo.class);
        when(activeChannelInfo.name()).thenReturn("activeChannelName");
        ReactorWarmStandbyChannelDetails activeChannel = mockChannelDetails("activeHost", 14002,
                ConnectionTypes.SOCKET, -1, ProtocolType.RWF, activeChannelInfo);
        ReactorWarmStandbyChannelDetails standbyChannel = mockChannelDetails("standbyHost", 14003,
                ConnectionTypes.ENCRYPTED, ConnectionTypes.HTTP, ProtocolType.JSON, null);

        when(event.channelsList()).thenReturn(Arrays.asList(null, activeChannel, standbyChannel));
        when(event.activeChannel()).thenReturn(activeChannel);

        WarmStandbyLoginBasedChannelInformation info = WarmStandbyLoginBasedChannelInformation.create(event);

        assertEquals(WarmStandbyMode.LOGIN_BASED, info.warmStandbyMode());
        assertEquals(0, info.activeChannelIndex());
        assertEquals(2, info.channelsList().size());

        WarmStandbyChannelDetails firstChannel = info.channelsList().get(0);
        assertEquals("activeHost", firstChannel.hostname());
        assertEquals("activeChannelName", firstChannel.channelName());
        assertEquals(14002, firstChannel.port());
        assertEquals(ConnectionType.SOCKET, firstChannel.connectionType());
        assertEquals(-1, firstChannel.encryptedConnectionType());
        assertEquals(ProtocolType.RWF, firstChannel.protocolType());
        assertSame(activeChannelInfo, firstChannel.userSpecObject());

        WarmStandbyChannelDetails secondChannel = info.channelsList().get(1);
        assertEquals("standbyHost", secondChannel.hostname());
        assertNull(secondChannel.channelName());
        assertEquals(14003, secondChannel.port());
        assertEquals(ConnectionType.ENCRYPTED, secondChannel.connectionType());
        assertEquals(ConnectionType.HTTP, secondChannel.encryptedConnectionType());
        assertEquals(ProtocolType.JSON, secondChannel.protocolType());
        assertNull(secondChannel.userSpecObject());
    }

    @Test
    public void givenActiveChannelNotInList_whenCreate_thenLeaveActiveIndexUnset()
    {
        ReactorWarmStandbyLoginBasedChannelInfoEvent event = mock(ReactorWarmStandbyLoginBasedChannelInfoEvent.class);
        ReactorWarmStandbyChannelDetails listedChannel = mockChannelDetails("listedHost", 15000,
                ConnectionTypes.ENCRYPTED, ConnectionTypes.SOCKET, ProtocolType.RWF, new Object());
        ReactorWarmStandbyChannelDetails activeChannel = mockChannelDetails("activeHost", 15001,
                ConnectionTypes.SOCKET, -1, ProtocolType.JSON, new Object());

        when(event.channelsList()).thenReturn(Collections.singletonList(listedChannel));
        when(event.activeChannel()).thenReturn(activeChannel);

        WarmStandbyLoginBasedChannelInformation info = WarmStandbyLoginBasedChannelInformation.create(event);

        assertEquals(-1, info.activeChannelIndex());
        assertEquals(1, info.channelsList().size());
        assertEquals("listedHost", info.channelsList().get(0).hostname());
    }

    @Test
    public void givenInformation_whenToString_thenIncludeModeGroupActiveMarkerAndChannels()
    {
        ReactorWarmStandbyLoginBasedChannelInfoEvent event = mock(ReactorWarmStandbyLoginBasedChannelInfoEvent.class);
        ChannelInfo activeChannelInfo = mock(ChannelInfo.class);
        when(activeChannelInfo.name()).thenReturn("channelA");
        ReactorWarmStandbyChannelDetails standbyChannel = mockChannelDetails("standbyHostA", 15999,
                ConnectionTypes.ENCRYPTED, ConnectionTypes.HTTP, ProtocolType.RWF, null);
        ReactorWarmStandbyChannelDetails activeChannel = mockChannelDetails("activeHost", 16000,
                ConnectionTypes.SOCKET, -1, ProtocolType.JSON, activeChannelInfo);
        ReactorWarmStandbyChannelDetails standbyChannelB = mockChannelDetails("standbyHostB", 16001,
                ConnectionTypes.SOCKET, -1, ProtocolType.RWF, new Object());

        when(event.channelsList()).thenReturn(Arrays.asList(standbyChannel, activeChannel, standbyChannelB));
        when(event.activeChannel()).thenReturn(activeChannel);

        WarmStandbyLoginBasedChannelInformation info = WarmStandbyLoginBasedChannelInformation.create(event);
        info.warmStandbyGroupName("wsbGroupA");

        String text = info.toString();

        assertTrue(text.contains("WarmStandbyLoginBasedChannelInformation:"));
        assertTrue(text.contains("warmStandbyMode: LOGIN_BASED"));
        assertTrue(text.contains("warmStandbyGroupName: wsbGroupA"));
        assertTrue(text.contains("channelsList:"));
        assertTrue(text.contains("active: {channelName='channelA'"));
        assertTrue(text.contains("standby: {channelName='N/A', hostName='standbyHostA'"));
        assertTrue(text.contains("standby: {channelName='N/A', hostName='standbyHostB'"));
        assertTrue(text.contains("channelName='channelA'"));
        assertTrue(text.contains("connectionType='socket'"));
        assertTrue(text.contains("encryptedConnectionType='N/A'"));
        assertTrue(text.contains("connectionType='encrypted'"));
        assertTrue(text.contains("encryptedConnectionType='http'"));
        assertTrue(text.contains("hostName='activeHost'"));
        assertFalse(text.contains("active: {channelName='N/A', hostName='standbyHostA'"));
        assertFalse(text.contains("active: {channelName='N/A', hostName='standbyHostB'"));
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
}
