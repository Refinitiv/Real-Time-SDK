/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceState;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.State;
import org.junit.Before;
import org.junit.Test;
import org.mockito.MockitoAnnotations;
import org.mockito.Spy;

import static com.refinitiv.ema.access.FilterEntry.*;
import static com.refinitiv.ema.access.OmmState.*;
import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

public class DirectoryServiceStateTest
{
    @Spy
    DirectoryServiceState directoryServiceState = EmaFactory.Domain.createDirectoryServiceState();

    @Before
    public void setUp()
    {
        MockitoAnnotations.openMocks(this);
    }

    @Test
    public void givenServiceState_whenClear_thenClearAllFields()
    {
        directoryServiceState.acceptingRequests(EmaRdm.SERVICE_NO);
        directoryServiceState.serviceState(EmaRdm.SERVICE_DOWN);
        directoryServiceState.action(FilterAction.CLEAR);
        directoryServiceState.status(StreamState.OPEN, DataState.OK, StatusCode.NONE, "");

        assertTrue(directoryServiceState.checkHasAcceptingRequests());
        assertTrue(directoryServiceState.checkHasStatus());

        directoryServiceState.clear();

        assertFalse(directoryServiceState.checkHasAcceptingRequests());
        assertFalse(directoryServiceState.checkHasStatus());

        assertEquals(EmaRdm.SERVICE_UP, directoryServiceState.serviceState());
        assertEquals(FilterAction.SET, directoryServiceState.action());
        assertEquals(EmaRdm.SERVICE_STATE_ID, directoryServiceState.filterId());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidServiceStateValue_whenSetServiceState_thenThrowException()
    {
        directoryServiceState.serviceState(2);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenAcceptingRequestsNotSet_whenGetAcceptingRequests_thenThrowException()
    {
        directoryServiceState.acceptingRequests();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidActionValue_whenSetAction_thenThrowException()
    {
        directoryServiceState.action(99);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsStatus_whenSetStatus_thenThrowException()
    {
        directoryServiceState.status(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenStatusNotSet_whenGetStatus_thenThrowException()
    {
        directoryServiceState.status();
    }

    @Test
    public void givenStatusFields_whenSetStatusFieldsSeparately_thenGetCorrectStatus()
    {
        assertFalse(directoryServiceState.checkHasStatus());

        directoryServiceState.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");

        assertTrue(directoryServiceState.checkHasStatus());

        OmmState status = directoryServiceState.status();
        assertEquals(StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(DataState.SUSPECT, status.dataState());
        assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test
    public void givenOmmState_whenSetStatusAsOneObject_thenGetCorrectStatus()
    {
        assertFalse(directoryServiceState.checkHasStatus());

        OmmStateImpl statusBefore = new OmmStateImpl();
        State rsslState = CodecFactory.createState();
        rsslState.streamState(StreamState.CLOSED_RECOVER);
        rsslState.dataState(DataState.SUSPECT);
        rsslState.code(StatusCode.SOURCE_UNKNOWN);
        Buffer stateText = CodecFactory.createBuffer();
        stateText.data("Test message");
        rsslState.text(stateText);
        statusBefore.decode(rsslState);

        directoryServiceState.status(statusBefore);

        assertTrue(directoryServiceState.checkHasStatus());

        OmmState status = directoryServiceState.status();
        assertEquals(StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(DataState.SUSPECT, status.dataState());
        assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceState_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryServiceState.copy(null);
    }

    @Test
    public void givenServiceState_whenCopyOtherServiceState_thenReturnTrueAndUpdateFields()
    {
        DirectoryServiceState otherDirectoryServiceState = EmaFactory.Domain.createDirectoryServiceState();
        otherDirectoryServiceState.acceptingRequests(EmaRdm.SERVICE_NO);
        otherDirectoryServiceState.serviceState(EmaRdm.SERVICE_DOWN);
        otherDirectoryServiceState.action(FilterAction.CLEAR);
        otherDirectoryServiceState.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");

        directoryServiceState.copy(otherDirectoryServiceState);
        verify(directoryServiceState, times(1)).clear();

        assertTrue(directoryServiceState.checkHasAcceptingRequests());
        assertEquals(EmaRdm.SERVICE_NO, directoryServiceState.acceptingRequests());

        assertEquals(EmaRdm.SERVICE_DOWN, directoryServiceState.serviceState());
        assertEquals(FilterAction.CLEAR, directoryServiceState.action());
        assertEquals(EmaRdm.SERVICE_STATE_ID, directoryServiceState.filterId());

        assertTrue(directoryServiceState.checkHasStatus());
        OmmState status = directoryServiceState.status();
        assertEquals(StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(DataState.SUSPECT, status.dataState());
        assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test
    public void givenServiceStateWithOptionalFields_whenCopyFromSourceWithoutOptionals_thenClearDestinationOptionals()
    {
        directoryServiceState.acceptingRequests(EmaRdm.SERVICE_NO);
        directoryServiceState.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Existing status");

        DirectoryServiceState otherDirectoryServiceState = EmaFactory.Domain.createDirectoryServiceState();
        otherDirectoryServiceState.serviceState(EmaRdm.SERVICE_DOWN);
        otherDirectoryServiceState.action(FilterAction.CLEAR);

        directoryServiceState.copy(otherDirectoryServiceState);

        assertEquals(EmaRdm.SERVICE_DOWN, directoryServiceState.serviceState());
        assertEquals(FilterAction.CLEAR, directoryServiceState.action());
        assertFalse(directoryServiceState.checkHasAcceptingRequests());
        assertFalse(directoryServiceState.checkHasStatus());
    }

    @Test
    public void givenServiceState_whenCopyIntoSelf_thenPreserveFields()
    {
        directoryServiceState.acceptingRequests(EmaRdm.SERVICE_NO);
        directoryServiceState.serviceState(EmaRdm.SERVICE_DOWN);
        directoryServiceState.action(FilterAction.CLEAR);
        directoryServiceState.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");

        directoryServiceState.copy(directoryServiceState);
        verify(directoryServiceState, never()).clear();

        assertTrue(directoryServiceState.checkHasAcceptingRequests());
        assertEquals(EmaRdm.SERVICE_NO, directoryServiceState.acceptingRequests());
        assertEquals(EmaRdm.SERVICE_DOWN, directoryServiceState.serviceState());
        assertEquals(FilterAction.CLEAR, directoryServiceState.action());

        assertTrue(directoryServiceState.checkHasStatus());
        OmmState status = directoryServiceState.status();
        assertEquals(StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(DataState.SUSPECT, status.dataState());
        assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test
    public void givenInvalidDataState_whenSetStatusFieldsSeparately_thenThrowExceptionAndLeaveStatusAbsent()
    {
        assertFalse(directoryServiceState.checkHasStatus());

        try
        {
            directoryServiceState.status(StreamState.OPEN, 99, StatusCode.NONE, "Test message");
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertFalse(directoryServiceState.checkHasStatus());
        }
    }

    @Test
    public void givenInvalidStreamStateInOmmState_whenSetStatusObject_thenThrowExceptionAndLeaveStatusAbsent()
    {
        assertFalse(directoryServiceState.checkHasStatus());

        OmmState invalidStatus = mock(OmmState.class);
        when(invalidStatus.streamState()).thenReturn(99);
        when(invalidStatus.dataState()).thenReturn(DataState.OK);
        when(invalidStatus.statusCode()).thenReturn(StatusCode.NONE);
        when(invalidStatus.statusText()).thenReturn("Test message");

        try
        {
            directoryServiceState.status(invalidStatus);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertFalse(directoryServiceState.checkHasStatus());
        }
    }

    @Test
    public void givenServiceState_whenDecodeWithNullAsParameter_thenThrowExceptionWithoutChangingState()
    {
        directoryServiceState.serviceState(EmaRdm.SERVICE_DOWN);
        directoryServiceState.acceptingRequests(EmaRdm.SERVICE_NO);
        directoryServiceState.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");

        try
        {
            directoryServiceState.decode(null);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertEquals(EmaRdm.SERVICE_DOWN, directoryServiceState.serviceState());
            assertTrue(directoryServiceState.checkHasAcceptingRequests());
            assertEquals(EmaRdm.SERVICE_NO, directoryServiceState.acceptingRequests());
            assertTrue(directoryServiceState.checkHasStatus());
            assertEquals(StreamState.CLOSED_RECOVER, directoryServiceState.status().streamState());
            assertEquals(DataState.SUSPECT, directoryServiceState.status().dataState());
            assertEquals(StatusCode.SOURCE_UNKNOWN, directoryServiceState.status().statusCode());
            assertEquals("Test message", directoryServiceState.status().statusText());
        }
    }

    @Test
    public void givenServiceState_whenDecodeWithMalformedServiceStateElement_thenClearAndThrowException()
    {
        directoryServiceState.serviceState(EmaRdm.SERVICE_DOWN);
        directoryServiceState.acceptingRequests(EmaRdm.SERVICE_NO);
        directoryServiceState.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Existing status");

        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_SVC_STATE, "badType"));

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        ElementList decElementList = JUnitTestConnect.createElementList();
        JUnitTestConnect.setRsslData(decElementList, encElementList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        try
        {
            directoryServiceState.decode(decElementList);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertEquals(EmaRdm.SERVICE_UP, directoryServiceState.serviceState());
            assertFalse(directoryServiceState.checkHasAcceptingRequests());
            assertFalse(directoryServiceState.checkHasStatus());
            assertEquals(FilterAction.SET, directoryServiceState.action());
        }
    }

    @Test
    public void givenServiceState_whenDecodeWithUnknownElement_thenIgnoreUnknownElement()
    {
        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN));
        encElementList.add(EmaFactory.createElementEntry().ascii("UNKNOWN_ELEMENT", "ignored"));

        DirectoryServiceState otherDirectoryServiceState = EmaFactory.Domain.createDirectoryServiceState();
        otherDirectoryServiceState.decode(createDecodedElementList(encElementList));

        assertEquals(EmaRdm.SERVICE_DOWN, otherDirectoryServiceState.serviceState());
        assertFalse(otherDirectoryServiceState.checkHasAcceptingRequests());
        assertFalse(otherDirectoryServiceState.checkHasStatus());
    }

    @Test
    public void givenServiceState_whenDecodeWithBlankStatus_thenLeaveStatusAbsent()
    {
        directoryServiceState.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Existing status");

        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_SVC_STATE, EmaRdm.SERVICE_DOWN));
        encElementList.add(EmaFactory.createElementEntry().codeState(EmaRdm.ENAME_STATUS));

        directoryServiceState.decode(createDecodedElementList(encElementList));

        assertEquals(EmaRdm.SERVICE_DOWN, directoryServiceState.serviceState());
        assertFalse(directoryServiceState.checkHasAcceptingRequests());
        assertFalse(directoryServiceState.checkHasStatus());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceState_whenDecodeWithoutServiceStateElement_thenThrowException()
    {
        directoryServiceState.serviceState(EmaRdm.SERVICE_DOWN);
        directoryServiceState.acceptingRequests(EmaRdm.SERVICE_NO);
        directoryServiceState.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Existing status");

        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_ACCEPTING_REQS, EmaRdm.SERVICE_NO));

        directoryServiceState.decode(createDecodedElementList(encElementList));
    }

    @Test
    public void givenServiceState_whenEncodeAndThenDecodeIntoOtherServiceState_thenFillOtherServiceStateFields()
    {
        // encode
        directoryServiceState.acceptingRequests(EmaRdm.SERVICE_NO);
        directoryServiceState.serviceState(EmaRdm.SERVICE_DOWN);
        directoryServiceState.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");

        ElementList encElementList = directoryServiceState.encode();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        ElementList decElementList = JUnitTestConnect.createElementList();

        JUnitTestConnect.setRsslData(decElementList, encElementList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        // decode
        DirectoryServiceState otherDirectoryServiceState = EmaFactory.Domain.createDirectoryServiceState();
        otherDirectoryServiceState.decode(decElementList);

        assertTrue(otherDirectoryServiceState.checkHasAcceptingRequests());
        assertEquals(EmaRdm.SERVICE_NO, otherDirectoryServiceState.acceptingRequests());

        assertEquals(EmaRdm.SERVICE_DOWN, otherDirectoryServiceState.serviceState());

        assertTrue(otherDirectoryServiceState.checkHasStatus());
        OmmState status = otherDirectoryServiceState.status();
        assertEquals(StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(DataState.SUSPECT, status.dataState());
        assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test
    public void givenServiceStateStatusWithAnySupportedStreamState_whenEncodeAndDecode_thenPreserveStatus()
    {
        int[] streamStates = new int[] {
                StreamState.OPEN,
                StreamState.NON_STREAMING,
                StreamState.CLOSED_RECOVER,
                StreamState.CLOSED,
                StreamState.CLOSED_REDIRECTED
        };

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        for (int streamState : streamStates)
        {
            DirectoryServiceState sourceServiceState = EmaFactory.Domain.createDirectoryServiceState();
            sourceServiceState.serviceState(EmaRdm.SERVICE_DOWN);
            sourceServiceState.acceptingRequests(EmaRdm.SERVICE_NO);
            sourceServiceState.status(streamState, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                    "StreamState=" + streamState);

            ElementList encElementList = sourceServiceState.encode();
            ElementList decElementList = JUnitTestConnect.createElementList();

            JUnitTestConnect.setRsslData(decElementList, encElementList, Codec.majorVersion(), Codec.minorVersion(),
                    ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

            DirectoryServiceState decodedServiceState = EmaFactory.Domain.createDirectoryServiceState();
            decodedServiceState.decode(decElementList);

            assertTrue(decodedServiceState.checkHasStatus());
            OmmState status = decodedServiceState.status();
            assertEquals(streamState, status.streamState());
            assertEquals(DataState.SUSPECT, status.dataState());
            assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
            assertEquals("StreamState=" + streamState, status.statusText());
        }
    }

    private ElementList createDecodedElementList(ElementList encElementList)
    {
        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        ElementList decElementList = JUnitTestConnect.createElementList();
        JUnitTestConnect.setRsslData(decElementList, encElementList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        return decElementList;
    }
}
