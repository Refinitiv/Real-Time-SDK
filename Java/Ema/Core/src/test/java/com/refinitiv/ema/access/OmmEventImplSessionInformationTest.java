/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import org.junit.Test;
import org.mockito.ArgumentCaptor;

import java.util.Arrays;
import java.util.Collections;
import java.util.List;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertSame;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyList;
import static org.mockito.Mockito.doAnswer;
import static org.mockito.Mockito.doThrow;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;

public class OmmEventImplSessionInformationTest
{
    @Test
    public void givenNullServiceNames_whenSessionInformationWithServices_thenReturnNullAndDoNotCreateSessionInformation()
    {
        OmmEventImpl<OmmConsumerClient> event = new OmmEventImpl<>();
        event._ommBaseImpl = mock(OmmConsumerImpl.class);

        SessionInformation result = event.sessionInformation(null);

        assertNull(result);
        assertNull(event._sessionInformation);
    }

    @SuppressWarnings("unchecked")
    @Test
    public void givenNonConsumerBaseImpl_whenSessionInformationWithServices_thenReturnNull()
    {
        OmmEventImpl<OmmConsumerClient> event = new OmmEventImpl<>();
        event._ommBaseImpl = mock(OmmBaseImpl.class);

        assertNull(event.sessionInformation(Collections.singletonList("SERVICE_A")));
        assertNull(event._sessionInformation);
    }

    @Test
    public void givenConsumerBaseImpl_whenSessionInformationWithServices_thenDelegateReuseAndClearCachedSessionInformation()
    {
        OmmConsumerImpl consumer = mock(OmmConsumerImpl.class);
        OmmEventImpl<OmmConsumerClient> event = new OmmEventImpl<>();
        event._ommBaseImpl = consumer;

        List<String> firstServices = Arrays.asList("SERVICE_A", "SERVICE_B");
        doAnswer(invocation -> {
            SessionInformation passedSessionInformation = invocation.getArgument(0);
            List<String> passedServices = invocation.getArgument(1);
            passedSessionInformation.channelList().add(mock(ChannelInformation.class));
            passedSessionInformation.warmStandbyChannelList().add(mock(WarmStandbyChannelInformation.class));
            assertSame(firstServices, passedServices);
            return null;
        }).doAnswer(invocation -> {
            SessionInformation passedSessionInformation = invocation.getArgument(0);
            List<String> passedServices = invocation.getArgument(1);
            assertTrue(passedSessionInformation.channelList().isEmpty());
            assertTrue(passedSessionInformation.warmStandbyChannelList().isEmpty());
            assertSame(Collections.<String>emptyList(), passedServices);
            passedSessionInformation.channelList().add(mock(ChannelInformation.class));
            return null;
        }).when(consumer).sessionInformation(any(SessionInformation.class), anyList());

        SessionInformation firstResult = event.sessionInformation(firstServices);
        SessionInformation secondResult = event.sessionInformation();

        assertNotNull(firstResult);
        assertSame(firstResult, event._sessionInformation);
        assertSame(firstResult, secondResult);
        assertSame(firstResult, event._sessionInformation);
        assertEquals(1, secondResult.channelList().size());
        assertTrue(secondResult.warmStandbyChannelList().isEmpty());
        verify(consumer).sessionInformation(firstResult, firstServices);
        verify(consumer).sessionInformation(firstResult, Collections.emptyList());
    }

    @Test
    public void givenConsumerThrowsInvalidUsage_whenSessionInformationWithServices_thenReturnNullAndKeepClearedCachedObject()
    {
        OmmConsumerImpl consumer = mock(OmmConsumerImpl.class);
        OmmEventImpl<OmmConsumerClient> event = new OmmEventImpl<>();
        event._ommBaseImpl = consumer;
        event._sessionInformation = EmaFactory.createSessionInformation();
        event._sessionInformation.channelList().add(mock(ChannelInformation.class));
        event._sessionInformation.warmStandbyChannelList().add(mock(WarmStandbyChannelInformation.class));

        doThrow(new OmmInvalidUsageExceptionImpl().message("Exception!", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION))
                .when(consumer).sessionInformation(any(SessionInformation.class), anyList());

        SessionInformation result = event.sessionInformation(Collections.singletonList("SERVICE_A"));

        assertNull(result);
        assertNotNull(event._sessionInformation);
        assertTrue(event._sessionInformation.channelList().isEmpty());
        assertTrue(event._sessionInformation.warmStandbyChannelList().isEmpty());
    }

    @SuppressWarnings("unchecked")
    @Test
    public void givenNoArgOverload_whenSessionInformation_thenDelegateWithSharedEmptyList()
    {
        OmmConsumerImpl consumer = mock(OmmConsumerImpl.class);
        OmmEventImpl<OmmConsumerClient> event = new OmmEventImpl<>();
        event._ommBaseImpl = consumer;

        doAnswer(invocation -> null).when(consumer).sessionInformation(any(SessionInformation.class), anyList());

        SessionInformation result = event.sessionInformation();
        ArgumentCaptor<List<String>> serviceNamesCaptor = (ArgumentCaptor<List<String>>) (ArgumentCaptor<?>) ArgumentCaptor.forClass(List.class);

        verify(consumer).sessionInformation(any(SessionInformation.class), serviceNamesCaptor.capture());
        assertNotNull(result);
        assertTrue(serviceNamesCaptor.getValue().isEmpty());
        assertSame(Collections.emptyList(), serviceNamesCaptor.getValue());
    }
}
