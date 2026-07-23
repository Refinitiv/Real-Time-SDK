/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.reactor.Reactor;
import com.refinitiv.eta.valueadd.reactor.ReactorCallbackReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyChannelInfoCallback;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyChannelInfoEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyLoginBasedChannelInfoEvent;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyServiceBasedChannelInfoEvent;
import org.junit.Before;
import org.junit.Test;
import org.mockito.ArgumentCaptor;

import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import static com.refinitiv.ema.access.WarmStandbyChannelInformation.*;
import static org.junit.Assert.*;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

public class OmmConsumerImplWarmStandbyTest
{
    private OmmConsumerImpl consumerImpl;
    private Reactor reactor;
    private LoginCallbackClientConsumer loginCallbackClient;
    private ChannelInfo activeChannelInfo;
    private ReactorChannel activeReactorChannel;

    @Before
    public void setUp() throws Exception
    {
        OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
        consumerImpl = new OmmConsumerImpl(config, true);

        reactor = mock(Reactor.class);
        loginCallbackClient = mock(LoginCallbackClientConsumer.class);
        activeChannelInfo = mock(ChannelInfo.class);
        activeReactorChannel = mock(ReactorChannel.class);

        when(loginCallbackClient.activeChannelInfo()).thenReturn(activeChannelInfo);
        when(activeChannelInfo.rsslReactorChannel()).thenReturn(activeReactorChannel);

        setField(consumerImpl, "_rsslReactor", reactor);
        setField(consumerImpl, "_rsslErrorInfo", ReactorFactory.createReactorErrorInfo());
        setField(consumerImpl, "_loginCallbackClient", loginCallbackClient);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullServiceNames_whenGetWarmStandbyChannelInformation_thenThrowException()
    {
        consumerImpl.getWarmStandbyChannelInformation(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenMissingReactor_whenGetWarmStandbyChannelInformation_thenThrowException() throws Exception
    {
        setField(consumerImpl, "_rsslReactor", null);

        consumerImpl.getWarmStandbyChannelInformation(Collections.emptyList());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenMissingLoginCallbackClient_whenGetWarmStandbyChannelInformation_thenThrowException() throws Exception
    {
        setField(consumerImpl, "_loginCallbackClient", null);

        consumerImpl.getWarmStandbyChannelInformation(Collections.emptyList());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNoActiveChannel_whenGetWarmStandbyChannelInformation_thenThrowException()
    {
        when(loginCallbackClient.activeChannelInfo()).thenReturn(null);

        consumerImpl.getWarmStandbyChannelInformation(Collections.emptyList());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenActiveChannelWithoutReactorChannel_whenGetWarmStandbyChannelInformation_thenThrowException()
    {
        when(activeChannelInfo.rsslReactorChannel()).thenReturn(null);

        consumerImpl.getWarmStandbyChannelInformation(Collections.emptyList());
    }

    @SuppressWarnings("unchecked")
    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenReactorFailure_whenGetWarmStandbyChannelInformation_thenThrowException()
    {
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        errorInfo.error().text("reactor failed");

        doAnswer(invocation -> {
            ReactorErrorInfo passedErrorInfo = invocation.getArgument(3);
            passedErrorInfo.error().text(errorInfo.error().text());
            return ReactorReturnCodes.FAILURE;
        }).when(reactor).getWarmStandbyChannelInfo(eq(activeReactorChannel), any(ReactorWarmStandbyChannelInfoCallback.class), any(List.class), any(ReactorErrorInfo.class));

        consumerImpl.getWarmStandbyChannelInformation(Collections.singletonList("ELEKTRON_DD"));
    }

    @SuppressWarnings("unchecked")
    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenCallbackWithoutResult_whenGetWarmStandbyChannelInformation_thenThrowException()
    {
        doAnswer(invocation -> ReactorReturnCodes.SUCCESS)
                .when(reactor).getWarmStandbyChannelInfo(eq(activeReactorChannel), any(ReactorWarmStandbyChannelInfoCallback.class), any(List.class), any(ReactorErrorInfo.class));

        consumerImpl.getWarmStandbyChannelInformation(Collections.singletonList("ELEKTRON_DD"));
    }

    @SuppressWarnings({"rawtypes", "unchecked"})
    @Test
    public void givenLoginBasedEvent_whenGetWarmStandbyChannelInformation_thenReturnMappedInformation()
    {
        ReactorWarmStandbyLoginBasedChannelInfoEvent etaEvent = mock(ReactorWarmStandbyLoginBasedChannelInfoEvent.class);
        when(etaEvent.channelsList()).thenReturn(Collections.emptyList());
        when(etaEvent.activeChannel()).thenReturn(null);
        when(etaEvent.reactorChannel()).thenReturn(activeReactorChannel);

        ChannelInfo callbackChannelInfo = mock(ChannelInfo.class);
        ChannelInfo parentChannelInfo = mock(ChannelInfo.class);
        when(parentChannelInfo.name()).thenReturn("wsbGroupA");
        when(callbackChannelInfo.getParentChannel()).thenReturn(parentChannelInfo);
        when(activeReactorChannel.userSpecObj()).thenReturn(callbackChannelInfo);

        List<String> serviceNames = Arrays.asList("SERVICE_A", "SERVICE_B");
        doAnswer(invocation -> {
            ReactorWarmStandbyChannelInfoCallback callback = invocation.getArgument(1);
            int callbackResult = callback.reactorWarmStandbyChannelInfoCallback(etaEvent);
            assertEquals(ReactorCallbackReturnCodes.SUCCESS, callbackResult);
            return ReactorReturnCodes.SUCCESS;
        }).when(reactor).getWarmStandbyChannelInfo(eq(activeReactorChannel), any(ReactorWarmStandbyChannelInfoCallback.class), eq(serviceNames), any(ReactorErrorInfo.class));

        WarmStandbyChannelInformation result = consumerImpl.getWarmStandbyChannelInformation(serviceNames);

        assertNotNull(result);
        assertTrue(result instanceof WarmStandbyLoginBasedChannelInformation);
        assertEquals(WarmStandbyMode.LOGIN_BASED, result.warmStandbyMode());
        assertEquals("wsbGroupA", result.warmStandbyGroupName());

        @SuppressWarnings("rawtypes")
        ArgumentCaptor<List> serviceNamesCaptor = ArgumentCaptor.forClass(List.class);
        verify(reactor).getWarmStandbyChannelInfo(eq(activeReactorChannel), any(ReactorWarmStandbyChannelInfoCallback.class), serviceNamesCaptor.capture(), any(ReactorErrorInfo.class));
        assertEquals(serviceNames, serviceNamesCaptor.getValue());
    }

    @Test
    public void givenServiceBasedEvent_whenGetWarmStandbyChannelInformationWithoutFilter_thenReturnMappedInformation()
    {
        ReactorWarmStandbyServiceBasedChannelInfoEvent etaEvent = mock(ReactorWarmStandbyServiceBasedChannelInfoEvent.class);
        when(etaEvent.perChannelServiceList()).thenReturn(Collections.emptyList());
        when(etaEvent.reactorChannel()).thenReturn(activeReactorChannel);
        when(activeReactorChannel.userSpecObj()).thenReturn(null);

        List<String> serviceNames = Collections.emptyList();
        doAnswer(invocation -> {
            ReactorWarmStandbyChannelInfoCallback callback = invocation.getArgument(1);
            assertEquals(ReactorCallbackReturnCodes.SUCCESS, callback.reactorWarmStandbyChannelInfoCallback(etaEvent));
            return ReactorReturnCodes.SUCCESS;
        }).when(reactor).getWarmStandbyChannelInfo(eq(activeReactorChannel), any(ReactorWarmStandbyChannelInfoCallback.class), eq(serviceNames), any(ReactorErrorInfo.class));

        WarmStandbyChannelInformation result = consumerImpl.getWarmStandbyChannelInformation(serviceNames);

        assertNotNull(result);
        assertTrue(result instanceof WarmStandbyServiceBasedChannelInformation);
        assertEquals(WarmStandbyMode.SERVICE_BASED, result.warmStandbyMode());
        assertNull(result.warmStandbyGroupName());
    }

    @Test
    public void givenNullEtaEvent_whenProcessEtaWsbEvent_thenReturnFailureAndThrowException()
    {
        try
        {
            consumerImpl.processEtaWsbEvent(null, new WarmStandbyChannelInformation[1]);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl exception)
        {
            assertTrue(exception.getMessage().contains("Eta warm standby channel info event is null."));
        }
    }

    @Test
    public void givenNullEmaEventHolder_whenProcessEtaWsbEvent_thenReturnFailureAndThrowException()
    {
        try
        {
            consumerImpl.processEtaWsbEvent(mock(ReactorWarmStandbyLoginBasedChannelInfoEvent.class), null);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl exception)
        {
            assertTrue(exception.getMessage().contains("Ema warm standby channel info event holder is null."));
        }
    }

    @Test
    public void givenLoginBasedEventWithParentChannel_whenProcessEtaWsbEvent_thenPopulateResultAndGroupName()
    {
        ReactorWarmStandbyLoginBasedChannelInfoEvent etaEvent = mock(ReactorWarmStandbyLoginBasedChannelInfoEvent.class);
        when(etaEvent.channelsList()).thenReturn(Collections.emptyList());
        when(etaEvent.activeChannel()).thenReturn(null);

        ReactorChannel reactorChannel = mock(ReactorChannel.class);
        ChannelInfo channelInfo = mock(ChannelInfo.class);
        ChannelInfo parentChannelInfo = mock(ChannelInfo.class);
        when(parentChannelInfo.name()).thenReturn("group-one");
        when(channelInfo.getParentChannel()).thenReturn(parentChannelInfo);
        when(reactorChannel.userSpecObj()).thenReturn(channelInfo);
        when(etaEvent.reactorChannel()).thenReturn(reactorChannel);

        WarmStandbyChannelInformation[] holder = new WarmStandbyChannelInformation[1];
        int result = consumerImpl.processEtaWsbEvent(etaEvent, holder);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, result);
        assertNotNull(holder[0]);
        assertTrue(holder[0] instanceof WarmStandbyLoginBasedChannelInformation);
        assertEquals("group-one", holder[0].warmStandbyGroupName());
    }

    @Test
    public void givenServiceBasedEventWithoutParentChannel_whenProcessEtaWsbEvent_thenPopulateResultWithoutGroupName()
    {
        ReactorWarmStandbyServiceBasedChannelInfoEvent etaEvent = mock(ReactorWarmStandbyServiceBasedChannelInfoEvent.class);
        when(etaEvent.perChannelServiceList()).thenReturn(Collections.emptyList());

        ReactorChannel reactorChannel = mock(ReactorChannel.class);
        ChannelInfo channelInfo = mock(ChannelInfo.class);
        when(channelInfo.getParentChannel()).thenReturn(null);
        when(reactorChannel.userSpecObj()).thenReturn(channelInfo);
        when(etaEvent.reactorChannel()).thenReturn(reactorChannel);

        WarmStandbyChannelInformation[] holder = new WarmStandbyChannelInformation[1];
        int result = consumerImpl.processEtaWsbEvent(etaEvent, holder);

        assertEquals(ReactorCallbackReturnCodes.SUCCESS, result);
        assertNotNull(holder[0]);
        assertTrue(holder[0] instanceof WarmStandbyServiceBasedChannelInformation);
        assertNull(holder[0].warmStandbyGroupName());
    }

    @Test
    public void givenUnknownWarmStandbyEvent_whenProcessEtaWsbEvent_thenThrowException()
    {
        ReactorWarmStandbyChannelInfoEvent etaEvent = mock(ReactorWarmStandbyChannelInfoEvent.class);
        when(etaEvent.warmStandbyMode()).thenReturn(999);

        try
        {
            consumerImpl.processEtaWsbEvent(etaEvent, new WarmStandbyChannelInformation[1]);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl exception)
        {
            assertTrue(exception.getMessage().contains("Unknown warm standby mode received in warm standby channel info event: 999"));
        }
    }

    private void setField(Object target, String fieldName, Object value) throws Exception
    {
        Field field = OmmBaseImpl.class.getDeclaredField(fieldName);
        field.setAccessible(true);
        field.set(target, value);
    }
}

