///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2024 LSEG. All rights reserved.                   --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.valueadd.reactor.*;

import org.junit.*;

import java.util.*;

import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

public class ChannelInformationTest {

    private final ReactorChannel reactorChannel = mock(ReactorChannel.class);
    private final ChannelInfo channelInfo = mock(ChannelInfo.class);
    private final Channel channel = mock(Channel.class);
    private final ReactorPreferredHostOptions reactorPreferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
    private final ActiveConfig activeConfig = new OmmConsumerActiveConfig();

    @Before
    public void setUp() {
        reactorPreferredHostOptions.isPreferredHostEnabled(true);
        reactorPreferredHostOptions.detectionTimeSchedule("* * ? * *");
        reactorPreferredHostOptions.detectionTimeInterval(13);
        reactorPreferredHostOptions.connectionListIndex(1);
        reactorPreferredHostOptions.warmStandbyGroupListIndex(1);
        reactorPreferredHostOptions.fallBackWithInWSBGroup(true);

        ChannelConfig cg1 = new ChannelConfig();
        cg1.name = "DummyChannelConfig1";
        ChannelConfig cg2 = new ChannelConfig();
        cg2.name = "DummyChannelConfig2";
        activeConfig.channelConfigSet = Arrays.asList(cg1, cg2);

        WarmStandbyChannelConfig wcg1 = new WarmStandbyChannelConfig();
        wcg1.name = "DummyWsbChannelConfig1";
        WarmStandbyChannelConfig wcg2 = new WarmStandbyChannelConfig();
        wcg2.name = "DummyWsbChannelConfig2";
        activeConfig.configWarmStandbySet = Arrays.asList(wcg1, wcg2);
    }

    @Test
    public void createNewChannelInformationInstanceBasedOnReactorChannel() {

        when(reactorChannel.userSpecObj()).thenReturn(channelInfo);
        when(reactorChannel.hostname()).thenReturn("DummyHostname");
        when(reactorChannel.port()).thenReturn(13);
        when(reactorChannel.channel()).thenReturn(channel);

        when(channelInfo.getActiveConfig()).thenReturn(activeConfig);

        when(channel.connectionType()).thenReturn(ChannelInformation.ConnectionType.HTTP);
        when(channel.protocolType()).thenReturn(ChannelInformation.ProtocolType.JSON);
        when(channel.majorVersion()).thenReturn(2);
        when(channel.minorVersion()).thenReturn(1);
        when(channel.pingTimeout()).thenReturn(10);
        when(channel.state()).thenReturn(ChannelInformation.ChannelState.ACTIVE);
        when(channel.encryptedConnectionType()).thenReturn(1);

        doAnswer(invocation -> {
            ReactorChannelInfo rci = invocation.getArgument(0, ReactorChannelInfo.class);
            rci.preferredHostInfo(reactorPreferredHostOptions);
            return ReactorReturnCodes.SUCCESS;
        }).when(reactorChannel).info(any(ReactorChannelInfo.class), any(ReactorErrorInfo.class));

        ChannelInformation ci = new ChannelInformationImpl(reactorChannel);

        PreferredHostInfo phi = ci.preferredHostInfo();
        assertNotNull(phi);
        assertTrue(phi.isPreferredHostEnabled());
        assertTrue(phi.isFallBackWithInWSBGroup());
        assertEquals("DummyChannelConfig2", phi.getChannelName());
        assertEquals("DummyWsbChannelConfig2", phi.getWsbChannelName());
        assertEquals("* * ? * *", phi.getDetectionTimeSchedule());
        assertEquals(13, phi.getDetectionTimeInterval());

        assertNull(ci.ipAddress());
        assertEquals("DummyHostname", ci.hostname());
        assertEquals(13, ci.port());
        assertEquals("unavailable", ci.componentInformation());
        assertEquals("unavailable", ci.securityProtocol());
        assertEquals(0, ci.maxFragmentSize());
        assertEquals(0, ci.maxOutputBuffers());
        assertEquals(0, ci.guaranteedOutputBuffers());
        assertEquals(0, ci.numInputBuffers());
        assertEquals(0, ci.sysSendBufSize());
        assertEquals(0, ci.sysRecvBufSize());
        assertEquals(0, ci.compressionType());
        assertEquals(0, ci.compressionThreshold());
        assertEquals(ChannelInformation.ConnectionType.HTTP, ci.connectionType());
        assertEquals(ChannelInformation.ProtocolType.JSON, ci.protocolType());
        assertEquals(2, ci.majorVersion());
        assertEquals(1, ci.minorVersion());
        assertEquals(10, ci.pingTimeout());
        assertEquals(ChannelInformation.ChannelState.ACTIVE, ci.channelState());
        assertEquals(1, ci.encryptedConnectionType());
    }

    @Test
    public void createNewChannelInformationInstanceUsingConstructorWithNoArgs() {

        ChannelInformation ci = new ChannelInformationImpl();

        assertNull(ci.preferredHostInfo());
        assertNull(ci.ipAddress());
        assertNull(ci.hostname());
        assertNull(ci.componentInformation());
        assertNull(ci.securityProtocol());

        assertEquals(0, ci.port());
        assertEquals(0, ci.maxFragmentSize());
        assertEquals(0, ci.maxOutputBuffers());
        assertEquals(0, ci.guaranteedOutputBuffers());
        assertEquals(0, ci.numInputBuffers());
        assertEquals(0, ci.sysSendBufSize());
        assertEquals(0, ci.sysRecvBufSize());
        assertEquals(0, ci.compressionType());
        assertEquals(0, ci.compressionThreshold());
        assertEquals(ChannelInformation.ConnectionType.UNIDENTIFIED, ci.connectionType());
        assertEquals(ChannelInformation.ProtocolType.UNKNOWN, ci.protocolType());
        assertEquals(0, ci.majorVersion());
        assertEquals(0, ci.minorVersion());
        assertEquals(0, ci.pingTimeout());
        assertEquals(ChannelInformation.ChannelState.CLOSED, ci.channelState());
        assertEquals(-1, ci.encryptedConnectionType());
    }
}