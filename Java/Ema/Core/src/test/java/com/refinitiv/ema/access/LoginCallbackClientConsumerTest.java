/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;


import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyChangeEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyChannelDetails;
import org.junit.Before;
import org.junit.Test;

import org.mockito.ArgumentCaptor;
import org.mockito.Mock;
import org.mockito.MockitoAnnotations;
import org.slf4j.Logger;

import java.util.ArrayList;

import static com.refinitiv.ema.access.WarmStandbyChannelInformation.*;
import static com.refinitiv.ema.access.OmmState.*;
import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.LOGIN_BASED;
import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.SERVICE_BASED;
import static com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyMode.NONE;
import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

public class LoginCallbackClientConsumerTest
{
    @Mock
    private OmmEventImpl<OmmConsumerClient> eventImpl;
    @Mock
    private OmmConsumerImpl consumer;
    @Mock
    private Logger logger;
    @Mock
    private ReactorWarmStandbyChangeEvent etaEvent;
    @Mock
    private ReactorWarmStandbyChannelDetails currentChannel;
    @Mock
    private ReactorWarmStandbyChannelDetails prevChannel;
    @Mock
    private ChannelInfo currentChannelInfo;
    @Mock
    private ChannelInfo parentChannelInfo;
    @Mock
    private ChannelInfo prevChannelInfo;
    @Mock
    private ChannelInformation channelInformation;
    private final ArgumentCaptor<WarmStandbyChangeEventInfo> eventInfoCaptor = ArgumentCaptor.forClass(WarmStandbyChangeEventInfo.class);

    @Before
    public void setUp()
    {
        MockitoAnnotations.openMocks(this);
        when(consumer.loggerClient()).thenReturn(logger);
        when(consumer.strBuilder()).thenAnswer(invocation -> new StringBuilder());

        when(eventImpl.channelInformation()).thenReturn(channelInformation);
    }

    @Test
    public void givenNullAsEvent_whenReactorWarmStandbyChangeEventCallback_thenReturnFailure()
    {
        LoginCallbackClientConsumer loginCallbackClient = spy(new LoginCallbackClientConsumer(consumer));

        when(loginCallbackClient.loginItemList()).thenReturn(new ArrayList<>());

        int ret = loginCallbackClient.reactorWarmStandbyChangeEventCallback(null);

        assertNotNull(loginCallbackClient);
        assertEquals(ReactorCallbackReturnCodes.FAILURE, ret);
    }

    @Test
    public void givenNullAsLoginItemList_whenReactorWarmStandbyChangeEventCallback_thenReturnSuccess()
    {
        LoginCallbackClientConsumer loginCallbackClient = spy(new LoginCallbackClientConsumer(consumer));

        int ret = loginCallbackClient.reactorWarmStandbyChangeEventCallback(null);

        assertNotNull(loginCallbackClient);
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
    }

    @Test
    public void givenEtaChangeEventWithoutChannels_whenReactorWarmStandbyChangeEventCallback_thenCreateEmaChangeEvent()
    {
        LoginCallbackClientConsumer loginCallbackClient = spy(new LoginCallbackClientConsumer(consumer));
        State state = CodecFactory.createState();
        loginCallbackClient._statusMsg =  new StatusMsgImpl();
        loginCallbackClient._eventImpl = eventImpl;

        when(loginCallbackClient.loginItemList()).thenReturn(new ArrayList<>());
        when(loginCallbackClient.rsslState()).thenReturn(state);
        when(loginCallbackClient._eventImpl.channelInformation()).thenReturn(channelInformation);
        doNothing().when(loginCallbackClient).prepareAndSendStatusMsg(any(ReactorWarmStandbyChangeEvent.class), any(State.class));

        when(channelInformation.sessionChannelName()).thenReturn("sessionA");

        when(etaEvent.warmStandbyMode()).thenReturn(LOGIN_BASED);

        int ret = loginCallbackClient.reactorWarmStandbyChangeEventCallback(etaEvent);
        verify(eventImpl).warmStandbyChangeEventInfo(eventInfoCaptor.capture());

        assertNotNull(loginCallbackClient);
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        WarmStandbyChangeEventInfo eventInfo = eventInfoCaptor.getValue();
        assertNotNull(eventInfo);
        assertEquals(WarmStandbyMode.LOGIN_BASED, eventInfo.warmStandbyMode());
        assertEquals(-1, eventInfo.serviceId());
        assertNull(eventInfo.serviceName());
        assertNull(eventInfo.previousChannelName());
        assertNull(eventInfo.currentChannelName());
        assertNull(eventInfo.wsbGroupName());
        assertEquals("sessionA", eventInfo.sessionChannelName());
        assertEquals(StatusCode.WSB_CHANGE_ACTIVE_COMPLETE, state.code());
        assertEquals(StreamState.OPEN, state.streamState());
        assertEquals(DataState.OK, state.dataState());
        assertNotNull(state.text());
        assertEquals("active channel switched from N/A to N/A", state.text().toString());
    }

    @Test
    public void givenEtaChangeEventWithNullAsUserSpecObject_whenReactorWarmStandbyChangeEventCallback_thenCreateEmaChangeEvent()
    {
        LoginCallbackClientConsumer loginCallbackClient = spy(new LoginCallbackClientConsumer(consumer));
        State state = CodecFactory.createState();
        loginCallbackClient._statusMsg =  new StatusMsgImpl();
        loginCallbackClient._eventImpl = eventImpl;

        when(loginCallbackClient.loginItemList()).thenReturn(new ArrayList<>());
        when(loginCallbackClient.rsslState()).thenReturn(state);
        when(loginCallbackClient._eventImpl.channelInformation()).thenReturn(channelInformation);
        doNothing().when(loginCallbackClient).prepareAndSendStatusMsg(any(ReactorWarmStandbyChangeEvent.class), any(State.class));

        when(currentChannel.userSpecObject()).thenReturn(null);
        when(prevChannel.userSpecObject()).thenReturn(null);

        when(etaEvent.currentChannel()).thenReturn(currentChannel);
        when(etaEvent.prevChannel()).thenReturn(prevChannel);

        when(etaEvent.warmStandbyMode()).thenReturn(LOGIN_BASED);

        int ret = loginCallbackClient.reactorWarmStandbyChangeEventCallback(etaEvent);
        verify(eventImpl).warmStandbyChangeEventInfo(eventInfoCaptor.capture());

        assertNotNull(loginCallbackClient);
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        WarmStandbyChangeEventInfo eventInfo = eventInfoCaptor.getValue();
        assertNotNull(eventInfo);
        assertEquals(WarmStandbyMode.LOGIN_BASED, eventInfo.warmStandbyMode());
        assertEquals(-1, eventInfo.serviceId());
        assertNull(eventInfo.serviceName());
        assertNull(eventInfo.previousChannelName());
        assertNull(eventInfo.currentChannelName());
        assertNull(eventInfo.wsbGroupName());
        assertNull(eventInfo.sessionChannelName());
        assertEquals(StatusCode.WSB_CHANGE_ACTIVE_COMPLETE, state.code());
        assertEquals(StreamState.OPEN, state.streamState());
        assertEquals(DataState.OK, state.dataState());
        assertNotNull(state.text());
        assertEquals("active channel switched from N/A to N/A", state.text().toString());
    }

    @Test
    public void givenEtaChangeEventWithIncorrectTypeOfUserSpecObject_whenReactorWarmStandbyChangeEventCallback_thenCreateEmaChangeEvent()
    {
        LoginCallbackClientConsumer loginCallbackClient = spy(new LoginCallbackClientConsumer(consumer));
        State state = CodecFactory.createState();
        loginCallbackClient._statusMsg =  new StatusMsgImpl();
        loginCallbackClient._eventImpl = eventImpl;

        when(loginCallbackClient.loginItemList()).thenReturn(new ArrayList<>());
        when(loginCallbackClient.rsslState()).thenReturn(state);
        when(loginCallbackClient._eventImpl.channelInformation()).thenReturn(channelInformation);
        doNothing().when(loginCallbackClient).prepareAndSendStatusMsg(any(ReactorWarmStandbyChangeEvent.class), any(State.class));

        when(currentChannel.userSpecObject()).thenReturn(new Object());
        when(prevChannel.userSpecObject()).thenReturn(new Object());

        when(etaEvent.currentChannel()).thenReturn(currentChannel);
        when(etaEvent.prevChannel()).thenReturn(prevChannel);

        when(etaEvent.warmStandbyMode()).thenReturn(LOGIN_BASED);

        int ret = loginCallbackClient.reactorWarmStandbyChangeEventCallback(etaEvent);
        verify(eventImpl).warmStandbyChangeEventInfo(eventInfoCaptor.capture());

        assertNotNull(loginCallbackClient);
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        WarmStandbyChangeEventInfo eventInfo = eventInfoCaptor.getValue();
        assertNotNull(eventInfo);
        assertEquals(WarmStandbyMode.LOGIN_BASED, eventInfo.warmStandbyMode());
        assertEquals(-1, eventInfo.serviceId());
        assertNull(eventInfo.serviceName());
        assertNull(eventInfo.previousChannelName());
        assertNull(eventInfo.currentChannelName());
        assertNull(eventInfo.wsbGroupName());
        assertNull(eventInfo.sessionChannelName());
        assertEquals(StatusCode.WSB_CHANGE_ACTIVE_COMPLETE, state.code());
        assertEquals(StreamState.OPEN, state.streamState());
        assertEquals(DataState.OK, state.dataState());
        assertNotNull(state.text());
        assertEquals("active channel switched from N/A to N/A", state.text().toString());
    }

    @Test
    public void givenUnknownEtaChangeEvent_whenReactorWarmStandbyChangeEventCallback_thenCreateEmaChangeEvent()
    {
        LoginCallbackClientConsumer loginCallbackClient = spy(new LoginCallbackClientConsumer(consumer));
        State state = CodecFactory.createState();
        loginCallbackClient._statusMsg =  new StatusMsgImpl();
        loginCallbackClient._eventImpl = eventImpl;

        when(loginCallbackClient.loginItemList()).thenReturn(new ArrayList<>());
        when(loginCallbackClient.rsslState()).thenReturn(state);
        when(loginCallbackClient._eventImpl.channelInformation()).thenReturn(channelInformation);
        doNothing().when(loginCallbackClient).prepareAndSendStatusMsg(any(ReactorWarmStandbyChangeEvent.class), any(State.class));

        when(channelInformation.sessionChannelName()).thenReturn("sessionA");

        when(currentChannelInfo.name()).thenReturn("currentChannel");
        when(currentChannelInfo.getParentChannel()).thenReturn(parentChannelInfo);
        when(parentChannelInfo.name()).thenReturn("wsbGroup");

        when(prevChannelInfo.name()).thenReturn("previousChannel");

        when(currentChannel.userSpecObject()).thenReturn(currentChannelInfo);
        when(prevChannel.userSpecObject()).thenReturn(prevChannelInfo);

        when(etaEvent.currentChannel()).thenReturn(currentChannel);
        when(etaEvent.prevChannel()).thenReturn(prevChannel);

        when(etaEvent.warmStandbyMode()).thenReturn(NONE);

        int ret = loginCallbackClient.reactorWarmStandbyChangeEventCallback(etaEvent);
        verify(eventImpl).warmStandbyChangeEventInfo(eventInfoCaptor.capture());

        assertNotNull(loginCallbackClient);
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        WarmStandbyChangeEventInfo eventInfo = eventInfoCaptor.getValue();
        assertNotNull(eventInfo);
        assertEquals(WarmStandbyMode.NONE, eventInfo.warmStandbyMode());
        assertEquals(-1, eventInfo.serviceId());
        assertNull(eventInfo.serviceName());
        assertEquals("previousChannel", eventInfo.previousChannelName());
        assertEquals("currentChannel", eventInfo.currentChannelName());
        assertEquals("wsbGroup", eventInfo.wsbGroupName());
        assertEquals("sessionA", eventInfo.sessionChannelName());
        assertEquals(StatusCode.WSB_CHANGE_ACTIVE_COMPLETE, state.code());
        assertEquals(StreamState.OPEN, state.streamState());
        assertEquals(DataState.OK, state.dataState());
        assertNotNull(state.text());
        assertEquals("Unknown warm standby mode", state.text().toString());
    }

    @Test
    public void givenLoginBasedEtaChangeEvent_whenReactorWarmStandbyChangeEventCallback_thenCreateEmaChangeEvent()
    {
        LoginCallbackClientConsumer loginCallbackClient = spy(new LoginCallbackClientConsumer(consumer));
        State state = CodecFactory.createState();
        loginCallbackClient._statusMsg =  new StatusMsgImpl();
        loginCallbackClient._eventImpl = eventImpl;

        when(loginCallbackClient.loginItemList()).thenReturn(new ArrayList<>());
        when(loginCallbackClient.rsslState()).thenReturn(state);
        when(loginCallbackClient._eventImpl.channelInformation()).thenReturn(channelInformation);
        doNothing().when(loginCallbackClient).prepareAndSendStatusMsg(any(ReactorWarmStandbyChangeEvent.class), any(State.class));

        when(channelInformation.sessionChannelName()).thenReturn("sessionA");

        when(currentChannelInfo.name()).thenReturn("currentChannel");
        when(currentChannelInfo.getParentChannel()).thenReturn(parentChannelInfo);
        when(parentChannelInfo.name()).thenReturn("wsbGroup");

        when(prevChannelInfo.name()).thenReturn("previousChannel");

        when(currentChannel.userSpecObject()).thenReturn(currentChannelInfo);
        when(prevChannel.userSpecObject()).thenReturn(prevChannelInfo);

        when(etaEvent.currentChannel()).thenReturn(currentChannel);
        when(etaEvent.prevChannel()).thenReturn(prevChannel);

        when(etaEvent.warmStandbyMode()).thenReturn(LOGIN_BASED);

        int ret = loginCallbackClient.reactorWarmStandbyChangeEventCallback(etaEvent);
        verify(eventImpl).warmStandbyChangeEventInfo(eventInfoCaptor.capture());

        assertNotNull(loginCallbackClient);
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        WarmStandbyChangeEventInfo eventInfo = eventInfoCaptor.getValue();
        assertNotNull(eventInfo);
        assertEquals(WarmStandbyMode.LOGIN_BASED, eventInfo.warmStandbyMode());
        assertEquals(-1, eventInfo.serviceId());
        assertNull(eventInfo.serviceName());
        assertEquals("previousChannel", eventInfo.previousChannelName());
        assertEquals("currentChannel", eventInfo.currentChannelName());
        assertEquals("wsbGroup", eventInfo.wsbGroupName());
        assertEquals("sessionA", eventInfo.sessionChannelName());
        assertEquals(StatusCode.WSB_CHANGE_ACTIVE_COMPLETE, state.code());
        assertEquals(StreamState.OPEN, state.streamState());
        assertEquals(DataState.OK, state.dataState());
        assertNotNull(state.text());
        assertEquals("active channel switched from previousChannel to currentChannel", state.text().toString());
    }

    @Test
    public void givenServiceBasedEtaChangeEvent_whenReactorWarmStandbyChangeEventCallback_thenCreateEmaChangeEvent()
    {
        LoginCallbackClientConsumer loginCallbackClient = spy(new LoginCallbackClientConsumer(consumer));
        State state = CodecFactory.createState();
        loginCallbackClient._statusMsg =  new StatusMsgImpl();
        loginCallbackClient._eventImpl = eventImpl;

        when(loginCallbackClient.loginItemList()).thenReturn(new ArrayList<>());
        when(loginCallbackClient.rsslState()).thenReturn(state);
        when(loginCallbackClient._eventImpl.channelInformation()).thenReturn(channelInformation);
        doNothing().when(loginCallbackClient).prepareAndSendStatusMsg(any(ReactorWarmStandbyChangeEvent.class), any(State.class));

        when(channelInformation.sessionChannelName()).thenReturn("sessionA");

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

        int ret = loginCallbackClient.reactorWarmStandbyChangeEventCallback(etaEvent);
        verify(eventImpl).warmStandbyChangeEventInfo(eventInfoCaptor.capture());

        assertNotNull(loginCallbackClient);
        assertEquals(ReactorCallbackReturnCodes.SUCCESS, ret);
        WarmStandbyChangeEventInfo eventInfo = eventInfoCaptor.getValue();
        assertNotNull(eventInfo);
        assertEquals(WarmStandbyMode.SERVICE_BASED, eventInfo.warmStandbyMode());
        assertEquals(123, eventInfo.serviceId());
        assertEquals("SVC", eventInfo.serviceName());
        assertEquals("previousChannel", eventInfo.previousChannelName());
        assertEquals("currentChannel", eventInfo.currentChannelName());
        assertEquals("wsbGroup", eventInfo.wsbGroupName());
        assertEquals("sessionA", eventInfo.sessionChannelName());
        assertEquals(StatusCode.WSB_CHANGE_ACTIVE_COMPLETE, state.code());
        assertEquals(StreamState.OPEN, state.streamState());
        assertEquals(DataState.OK, state.dataState());
        assertNotNull(state.text());
        assertEquals("service SVC switched from previousChannel to currentChannel", state.text().toString());
    }
}
