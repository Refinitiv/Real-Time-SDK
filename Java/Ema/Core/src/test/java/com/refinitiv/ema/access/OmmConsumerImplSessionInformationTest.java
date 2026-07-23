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
import com.refinitiv.eta.valueadd.reactor.ReactorChannelType;
import com.refinitiv.eta.valueadd.reactor.ReactorErrorInfo;
import com.refinitiv.eta.valueadd.reactor.ReactorFactory;
import com.refinitiv.eta.valueadd.reactor.ReactorReturnCodes;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyChannelInfoCallback;
import com.refinitiv.eta.valueadd.reactor.ReactorWarmStandbyLoginBasedChannelInfoEvent;
import org.junit.Before;
import org.junit.Test;

import java.lang.reflect.Field;
import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyList;
import static org.mockito.ArgumentMatchers.eq;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.when;

public class OmmConsumerImplSessionInformationTest
{
    private OmmConsumerImpl consumerImpl;
    private Reactor reactor;
    private ConsumerSession<OmmConsumerClient> consumerSession;
    private BaseSessionChannelInfo<OmmConsumerClient> warmStandbySessionChannelInfo;
    private ReactorChannel normalReactorChannel;
    private ReactorChannel warmStandbyReactorChannel;
    private SessionInformation sessionInformation;

    @Before
    @SuppressWarnings("unchecked")
    public void setUp() throws Exception
    {
        OmmConsumerConfig config = EmaFactory.createOmmConsumerConfig();
        consumerImpl = new OmmConsumerImpl(config, true);

        reactor = mock(Reactor.class);
        consumerSession = mock(ConsumerSession.class);
        when(consumerSession.ommBaseImpl()).thenReturn(consumerImpl);
        SessionChannelInfo<OmmConsumerClient> normalSessionChannelInfo = new SessionChannelInfo<>(new ConsumerSessionChannelConfig("default-session"), consumerSession);
        warmStandbySessionChannelInfo = mock(BaseSessionChannelInfo.class);
        normalReactorChannel = mock(ReactorChannel.class);
        warmStandbyReactorChannel = mock(ReactorChannel.class);
        sessionInformation = EmaFactory.createSessionInformation();

        when(consumerSession.sessionChannelList()).thenReturn(Arrays.asList(normalSessionChannelInfo, warmStandbySessionChannelInfo));
        normalSessionChannelInfo.reactorChannel(normalReactorChannel);
        when(warmStandbySessionChannelInfo.reactorChannel()).thenReturn(warmStandbyReactorChannel);
        when(normalReactorChannel.reactorChannelType()).thenReturn(ReactorChannelType.NORMAL);
        when(warmStandbyReactorChannel.reactorChannelType()).thenReturn(ReactorChannelType.WARM_STANDBY);

        setField(consumerImpl, "_consumerSession", consumerSession);
        setField(consumerImpl, "_rsslReactor", reactor);
        setField(consumerImpl, "_rsslErrorInfo", ReactorFactory.createReactorErrorInfo());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullSessionInformation_whenSessionInformationWithServices_thenThrowException()
    {
        consumerImpl.sessionInformation(null, Collections.emptyList());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullServiceNames_whenSessionInformationWithServices_thenThrowException()
    {
        consumerImpl.sessionInformation(sessionInformation, null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenMissingConsumerSession_whenSessionInformationWithServices_thenThrowException() throws Exception
    {
        setField(consumerImpl, "_consumerSession", null);

        consumerImpl.sessionInformation(sessionInformation, Collections.emptyList());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenMissingReactor_whenSessionInformationWithServices_thenThrowException() throws Exception
    {
        setField(consumerImpl, "_rsslReactor", null);

        consumerImpl.sessionInformation(sessionInformation, Collections.emptyList());
    }

    @Test
    public void givenNormalAndWarmStandbyChannels_whenSessionInformationWithServices_thenPopulateAndClearLists()
    {
        SessionChannelInfo<OmmConsumerClient> normalSessionInfo = new SessionChannelInfo<>(new ConsumerSessionChannelConfig("session-normal"), consumerSession);
        normalSessionInfo.reactorChannel(normalReactorChannel);
        when(consumerSession.sessionChannelList()).thenReturn(Arrays.asList(normalSessionInfo, warmStandbySessionChannelInfo));

        ChannelInfo normalChannelInfo = new ChannelInfo("channel-A", reactor);
        SocketChannelConfig normalChannelConfig = new SocketChannelConfig();
        normalChannelConfig.name = "channel-A";
        normalChannelInfo._channelConfig = normalChannelConfig;
        normalChannelInfo.sessionChannelInfo(normalSessionInfo);
        when(normalReactorChannel.userSpecObj()).thenReturn(normalChannelInfo);
        when(normalReactorChannel.port()).thenReturn(14002);

        ConsumerSessionChannelConfig sessionChannelConfig = new ConsumerSessionChannelConfig("session-A");
        when(warmStandbySessionChannelInfo.sessionChannelConfig()).thenReturn(sessionChannelConfig);

        ReactorWarmStandbyLoginBasedChannelInfoEvent etaEvent = mock(ReactorWarmStandbyLoginBasedChannelInfoEvent.class);
        when(etaEvent.channelsList()).thenReturn(Collections.emptyList());
        when(etaEvent.activeChannel()).thenReturn(null);
        when(etaEvent.reactorChannel()).thenReturn(warmStandbyReactorChannel);

        List<String> serviceNames = Arrays.asList("SERVICE_A", "SERVICE_B");
        doAnswer(invocation -> {
            ReactorWarmStandbyChannelInfoCallback callback = invocation.getArgument(1);
            assertEquals(ReactorCallbackReturnCodes.SUCCESS, callback.reactorWarmStandbyChannelInfoCallback(etaEvent));
            return ReactorReturnCodes.SUCCESS;
        }).when(reactor).getWarmStandbyChannelInfo(eq(warmStandbyReactorChannel), any(ReactorWarmStandbyChannelInfoCallback.class), anyList(), any(ReactorErrorInfo.class));

        sessionInformation.channelList().add(mock(ChannelInformation.class));
        sessionInformation.warmStandbyChannelList().add(mock(WarmStandbyChannelInformation.class));

        consumerImpl.sessionInformation(sessionInformation, serviceNames);

        assertEquals(1, sessionInformation.channelList().size());
        assertTrue(sessionInformation.channelList().get(0) instanceof ChannelInformationImpl);
        assertEquals("channel-A", sessionInformation.channelList().get(0).channelName());
        assertEquals("session-normal", sessionInformation.channelList().get(0).sessionChannelName());
        assertEquals(1, sessionInformation.warmStandbyChannelList().size());
        WarmStandbyChannelInformation warmStandbyChannelInformation = sessionInformation.warmStandbyChannelList().get(0);
        assertTrue(warmStandbyChannelInformation instanceof WarmStandbyLoginBasedChannelInformation);
        assertEquals("session-A", warmStandbyChannelInformation.sessionChannelName());
        verify(reactor).getWarmStandbyChannelInfo(eq(warmStandbyReactorChannel), any(ReactorWarmStandbyChannelInfoCallback.class), eq(serviceNames), any(ReactorErrorInfo.class));
    }

    @Test
    public void givenReactorFailure_whenSessionInformationWithServices_thenClearAndThrowException()
    {
        when(consumerSession.sessionChannelList()).thenReturn(Collections.singletonList(warmStandbySessionChannelInfo));
        when(warmStandbyReactorChannel.reactorChannelType()).thenReturn(ReactorChannelType.WARM_STANDBY);

        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        errorInfo.error().text("warm standby lookup failed");

        doAnswer(invocation -> {
            ReactorErrorInfo passedErrorInfo = invocation.getArgument(3);
            passedErrorInfo.error().text(errorInfo.error().text());
            return ReactorReturnCodes.FAILURE;
        }).when(reactor).getWarmStandbyChannelInfo(eq(warmStandbyReactorChannel), any(ReactorWarmStandbyChannelInfoCallback.class), anyList(), any(ReactorErrorInfo.class));

        sessionInformation.channelList().add(mock(ChannelInformation.class));
        sessionInformation.warmStandbyChannelList().add(mock(WarmStandbyChannelInformation.class));

        try
        {
            consumerImpl.sessionInformation(sessionInformation, Collections.singletonList("SERVICE_A"));
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl exception)
        {
            assertTrue(exception.getMessage().contains("Failed to get warm standby channel info from reactor. Error text: warm standby lookup failed"));
            assertTrue(sessionInformation.channelList().isEmpty());
            assertTrue(sessionInformation.warmStandbyChannelList().isEmpty());
        }
    }

    @Test
    public void givenCallbackWithoutResult_whenSessionInformationWithServices_thenClearAndThrowException()
    {
        when(consumerSession.sessionChannelList()).thenReturn(Collections.singletonList(warmStandbySessionChannelInfo));
        doAnswer(invocation -> ReactorReturnCodes.SUCCESS)
                .when(reactor).getWarmStandbyChannelInfo(eq(warmStandbyReactorChannel), any(ReactorWarmStandbyChannelInfoCallback.class), anyList(), any(ReactorErrorInfo.class));

        sessionInformation.channelList().add(mock(ChannelInformation.class));
        sessionInformation.warmStandbyChannelList().add(mock(WarmStandbyChannelInformation.class));

        try
        {
            consumerImpl.sessionInformation(sessionInformation, Collections.singletonList("SERVICE_A"));
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl exception)
        {
            assertTrue(exception.getMessage().contains("Failed to get warm standby channel info: callback reported an error."));
            assertTrue(sessionInformation.channelList().isEmpty());
            assertTrue(sessionInformation.warmStandbyChannelList().isEmpty());
        }
    }

    @Test
    public void givenOneArgumentOverload_whenSessionInformation_thenDelegateWithEmptyServiceList()
    {
        SessionChannelInfo<OmmConsumerClient> normalSessionInfo = new SessionChannelInfo<>(new ConsumerSessionChannelConfig("session-A"), consumerSession);
        normalSessionInfo.reactorChannel(normalReactorChannel);
        when(consumerSession.sessionChannelList()).thenReturn(Collections.singletonList(normalSessionInfo));

        ChannelInfo normalChannelInfo = new ChannelInfo("channel-A", reactor);
        SocketChannelConfig normalChannelConfig = new SocketChannelConfig();
        normalChannelConfig.name = "channel-A";
        normalChannelInfo._channelConfig = normalChannelConfig;
        normalChannelInfo.sessionChannelInfo(normalSessionInfo);

        when(normalReactorChannel.userSpecObj()).thenReturn(normalChannelInfo);
        when(normalReactorChannel.port()).thenReturn(14008);

        SessionInformation providedSessionInformation = EmaFactory.createSessionInformation();
        providedSessionInformation.channelList().add(mock(ChannelInformation.class));
        providedSessionInformation.warmStandbyChannelList().add(mock(WarmStandbyChannelInformation.class));

        consumerImpl.sessionInformation(providedSessionInformation);

        assertEquals(1, providedSessionInformation.channelList().size());
        assertEquals("channel-A", providedSessionInformation.channelList().get(0).channelName());
        assertEquals("session-A", providedSessionInformation.channelList().get(0).sessionChannelName());
        assertTrue(providedSessionInformation.warmStandbyChannelList().isEmpty());
    }

    private void setField(Object target, String fieldName, Object value) throws Exception
    {
        Field field = OmmBaseImpl.class.getDeclaredField(fieldName);
        field.setAccessible(true);
        field.set(target, value);
    }
}
