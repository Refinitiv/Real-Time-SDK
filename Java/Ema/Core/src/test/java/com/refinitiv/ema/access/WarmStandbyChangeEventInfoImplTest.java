/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyChannelDetails;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyChangeEvent;
import org.junit.Test;

import static com.refinitiv.ema.access.WarmStandbyChannelInformation.*;
import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.LOGIN_BASED;
import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.SERVICE_BASED;
import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.NONE;
import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

public class WarmStandbyChangeEventInfoImplTest
{
    private final ReactorWarmStandbyChangeEvent etaEvent = mock(ReactorWarmStandbyChangeEvent.class);
    private final ReactorWarmStandbyChannelDetails currentChannel = mock(ReactorWarmStandbyChannelDetails.class);
    private final ReactorWarmStandbyChannelDetails prevChannel = mock(ReactorWarmStandbyChannelDetails.class);
    private final ChannelInfo currentChannelInfo = mock(ChannelInfo.class);
    private final ChannelInfo parentChannelInfo = mock(ChannelInfo.class);
    private final ChannelInfo prevChannelInfo = mock(ChannelInfo.class);

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsEvent_whenCreate_thenThrowException()
    {
        WarmStandbyChangeEventInfoImpl.create(null, "sessionA");
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsEvent_whenUpdate_thenThrowException()
    {
        WarmStandbyChangeEventInfo emaEvent = WarmStandbyChangeEventInfoImpl.create(etaEvent, "sessionA");
        ((WarmStandbyChangeEventInfoImpl) emaEvent).update(null, "sessionB");
    }

    @Test
    public void givenEventForServiceMode_whenCreate_thenReturnWarmStandbyChangeEventInfo()
    {
        when(currentChannelInfo.name()).thenReturn("currentChannel");
        when(currentChannelInfo.getParentChannel()).thenReturn(parentChannelInfo);
        when(parentChannelInfo.name()).thenReturn("wsbGroup");

        when(prevChannelInfo.name()).thenReturn("previousChannel");

        when(currentChannel.userSpecObject()).thenReturn(currentChannelInfo);
        when(prevChannel.userSpecObject()).thenReturn(prevChannelInfo);

        when(etaEvent.currentChannel()).thenReturn(currentChannel);
        when(etaEvent.prevChannel()).thenReturn(prevChannel);

        when(etaEvent.warmStandbyMode()).thenReturn(SERVICE_BASED);
        when(etaEvent.serviceId()).thenReturn(123);
        when(etaEvent.serviceName()).thenReturn("SVC");

        WarmStandbyChangeEventInfo emaEvent = WarmStandbyChangeEventInfoImpl.create(etaEvent, "sessionA");

        assertNotNull(emaEvent);
        assertEquals(WarmStandbyMode.SERVICE_BASED, emaEvent.warmStandbyMode());
        assertEquals(123, emaEvent.serviceId());
        assertEquals("SVC", emaEvent.serviceName());
        assertEquals("previousChannel", emaEvent.previousChannelName());
        assertEquals("currentChannel", emaEvent.currentChannelName());
        assertEquals("wsbGroup", emaEvent.wsbGroupName());
        assertEquals("sessionA", emaEvent.sessionChannelName());
    }

    @Test
    public void givenEventForLoginModeWithUserSpecObjectNotChannelInfo_whenCreate_thenReturnEmptyWarmStandbyChangeEventInfo()
    {
        when(currentChannel.userSpecObject()).thenReturn(new Object());
        when(prevChannel.userSpecObject()).thenReturn(new Object());

        when(etaEvent.currentChannel()).thenReturn(currentChannel);
        when(etaEvent.prevChannel()).thenReturn(prevChannel);

        when(etaEvent.warmStandbyMode()).thenReturn(LOGIN_BASED);
        when(etaEvent.serviceId()).thenReturn(5);
        when(etaEvent.serviceName()).thenReturn("SVC");

        WarmStandbyChangeEventInfo emaEvent = WarmStandbyChangeEventInfoImpl.create(etaEvent, null);

        assertNotNull(emaEvent);
        assertEquals(WarmStandbyMode.LOGIN_BASED, emaEvent.warmStandbyMode());
        assertEquals(-1, emaEvent.serviceId());
        assertNull(emaEvent.serviceName());

        assertNull(emaEvent.previousChannelName());
        assertNull(emaEvent.currentChannelName());
        assertNull(emaEvent.wsbGroupName());
        assertNull(emaEvent.sessionChannelName());
    }

    @Test
    public void givenEventWithNullsAsChannels_whenCreate_thenReturnWarmStandbyChangeEventInfo()
    {
        when(etaEvent.currentChannel()).thenReturn(null);
        when(etaEvent.prevChannel()).thenReturn(null);

        when(etaEvent.warmStandbyMode()).thenReturn(NONE);
        when(etaEvent.serviceId()).thenReturn(1);
        when(etaEvent.serviceName()).thenReturn("SVC");

        WarmStandbyChangeEventInfo emaEvent = WarmStandbyChangeEventInfoImpl.create(etaEvent, "sessionB");

        assertNotNull(emaEvent);
        assertEquals(WarmStandbyMode.NONE, emaEvent.warmStandbyMode());
        assertEquals(-1, emaEvent.serviceId());
        assertNull(emaEvent.serviceName());

        assertNull(emaEvent.previousChannelName());
        assertNull(emaEvent.currentChannelName());
        assertNull(emaEvent.wsbGroupName());
        assertEquals("sessionB", emaEvent.sessionChannelName());
    }
}
