/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.*;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.*;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.List;

import static org.junit.Assert.*;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

public class DirectoryRefreshTest
{
    DirectoryRefresh directoryRefresh = EmaFactory.Domain.createDirectoryRefresh();

    @Test
    public void givenDirectoryRefresh_whenClear_thenClearAllFields()
    {
        setDirectoryRefresh(directoryRefresh);

        assertTrue(directoryRefresh.checkHasSequenceNumber());
        assertTrue(directoryRefresh.checkHasServiceId());

        directoryRefresh.clear();

        assertFalse(directoryRefresh.checkHasSequenceNumber());
        assertFalse(directoryRefresh.checkHasServiceId());
        assertFalse(directoryRefresh.clearCache());
        assertFalse(directoryRefresh.complete());
        assertFalse(directoryRefresh.solicited());
        assertFalse(directoryRefresh.doNotCache());

        assertEquals(0, directoryRefresh.filter());
        assertEquals(-1, directoryRefresh.streamId());
        assertTrue(directoryRefresh.serviceList().isEmpty());
        assertEquals(EmaRdm.MMT_DIRECTORY, directoryRefresh.domainType());
        OmmState status = directoryRefresh.state();
        assertEquals(OmmState.StreamState.OPEN, status.streamState());
        assertEquals(OmmState.DataState.OK, status.dataState());
        assertEquals(OmmState.StatusCode.NONE, status.statusCode());
        assertEquals("", status.statusText());
    }

    @Test
    public void givenDirectoryRefresh_whenUsingFluentApi_thenReturnSameInstance()
    {
        DirectoryRefresh otherDirectoryRefresh = EmaFactory.Domain.createDirectoryRefresh();
        setDirectoryRefresh(otherDirectoryRefresh);
        RefreshMsg decRefreshMsg = decodeRefreshMsg(otherDirectoryRefresh.message());

        DirectoryRefresh returned = directoryRefresh.clear()
                .streamId(7)
                .filter(EmaRdm.SERVICE_INFO_FILTER)
                .serviceId(5)
                .sequenceNumber(1)
                .solicited(true)
                .complete(true)
                .clearCache(true)
                .doNotCache(true)
                .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "OK")
                .serviceList(new ArrayList<>());

        assertSame(directoryRefresh, returned);
        assertSame(directoryRefresh, directoryRefresh.copy(otherDirectoryRefresh));
        assertSame(directoryRefresh, directoryRefresh.message(decRefreshMsg));
        checkDirectoryRefresh(directoryRefresh, false);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenSequenceNumberNotSet_whenGetSequenceNumber_thenThrowException()
    {
        directoryRefresh.sequenceNumber();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceIdNotSet_whenGetServiceId_thenThrowException()
    {
        directoryRefresh.serviceId();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsServiceList_whenSetServiceList_thenThrowException()
    {
        directoryRefresh.serviceList(null);
    }

    @Test
    public void givenStateFields_whenSetStateFieldsSeparately_thenGetCorrectState()
    {
        directoryRefresh.state(OmmState.StreamState.CLOSED, OmmState.DataState.NO_CHANGE, OmmState.StatusCode.NONE,
                "Test message");

        OmmState status = directoryRefresh.state();
        assertEquals(OmmState.StreamState.CLOSED, status.streamState());
        assertEquals(OmmState.DataState.NO_CHANGE, status.dataState());
        assertEquals(OmmState.StatusCode.NONE, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test
    public void givenOmmState_whenSetStateAsOneObject_thenGetCorrectState()
    {
        OmmStateImpl statusBefore = new OmmStateImpl();
        State rsslState = CodecFactory.createState();
        rsslState.streamState(OmmState.StreamState.CLOSED);
        rsslState.dataState(OmmState.DataState.OK);
        rsslState.code(OmmState.StatusCode.ERROR);
        Buffer stateText = CodecFactory.createBuffer();
        stateText.data("Test message");
        rsslState.text(stateText);
        statusBefore.decode(rsslState);

        directoryRefresh.state(statusBefore);

        OmmState status = directoryRefresh.state();
        assertEquals(OmmState.StreamState.CLOSED, status.streamState());
        assertEquals(OmmState.DataState.OK, status.dataState());
        assertEquals(OmmState.StatusCode.ERROR, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsState_whenSetState_thenThrowException()
    {
        directoryRefresh.state(null);
    }

    @Test
    public void givenNullStatusText_whenSetStateFieldsSeparately_thenThrowExceptionAndLeaveStateUnchanged()
    {
        OmmState statusBefore = directoryRefresh.state();

        try
        {
            directoryRefresh.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, null);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            OmmState statusAfter = directoryRefresh.state();
            assertEquals(statusBefore.streamState(), statusAfter.streamState());
            assertEquals(statusBefore.dataState(), statusAfter.dataState());
            assertEquals(statusBefore.statusCode(), statusAfter.statusCode());
            assertEquals(statusBefore.statusText(), statusAfter.statusText());
        }
    }

    @Test
    public void givenInvalidDataState_whenSetStateFieldsSeparately_thenThrowExceptionAndLeaveStateUnchanged()
    {
        OmmState statusBefore = directoryRefresh.state();

        try
        {
            directoryRefresh.state(OmmState.StreamState.OPEN, 99, OmmState.StatusCode.NONE, "Test message");
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            OmmState statusAfter = directoryRefresh.state();
            assertEquals(statusBefore.streamState(), statusAfter.streamState());
            assertEquals(statusBefore.dataState(), statusAfter.dataState());
            assertEquals(statusBefore.statusCode(), statusAfter.statusCode());
            assertEquals(statusBefore.statusText(), statusAfter.statusText());
        }
    }

    @Test
    public void givenInvalidStreamStateInOmmState_whenSetStateObject_thenThrowExceptionAndLeaveStateUnchanged()
    {
        OmmState statusBefore = directoryRefresh.state();

        OmmState invalidState = mock(OmmState.class);
        when(invalidState.streamState()).thenReturn(99);
        when(invalidState.dataState()).thenReturn(OmmState.DataState.OK);
        when(invalidState.statusCode()).thenReturn(OmmState.StatusCode.NONE);
        when(invalidState.statusText()).thenReturn("Test message");

        try
        {
            directoryRefresh.state(invalidState);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            OmmState statusAfter = directoryRefresh.state();
            assertEquals(statusBefore.streamState(), statusAfter.streamState());
            assertEquals(statusBefore.dataState(), statusAfter.dataState());
            assertEquals(statusBefore.statusCode(), statusAfter.statusCode());
            assertEquals(statusBefore.statusText(), statusAfter.statusText());
        }
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDirectoryRefresh_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryRefresh.copy(null);
    }

    @Test
    public void givenDirectoryRefresh_whenCopyOtherDirectoryRefresh_thenUpdateFields()
    {
        DirectoryRefresh otherDirectoryRefresh = EmaFactory.Domain.createDirectoryRefresh();
        DirectoryService firstDirectoryService = setDirectoryRefresh(otherDirectoryRefresh);

        directoryRefresh.copy(otherDirectoryRefresh);

        assertNotSame(firstDirectoryService, directoryRefresh.serviceList().get(0));

        checkDirectoryRefresh(directoryRefresh, true);
    }

    @Test
    public void givenDirectoryRefresh_whenCopySelf_thenLeaveFieldsAndReferencesUnchanged()
    {
        DirectoryService firstDirectoryService = setDirectoryRefresh(directoryRefresh);

        directoryRefresh.copy(directoryRefresh);

        assertSame(firstDirectoryService, directoryRefresh.serviceList().get(0));
        checkDirectoryRefresh(directoryRefresh, true);
    }

    @Test
    public void givenDirectoryRefreshWithOptionalFields_whenCopyFromSourceWithoutOptionals_thenClearDestinationOptionals()
    {
        setDirectoryRefresh(directoryRefresh);

        DirectoryRefresh otherDirectoryRefresh = EmaFactory.Domain.createDirectoryRefresh();
        otherDirectoryRefresh.streamId(17);
        otherDirectoryRefresh.filter(EmaRdm.SERVICE_GROUP_FILTER);

        directoryRefresh.copy(otherDirectoryRefresh);

        assertEquals(17, directoryRefresh.streamId());
        assertEquals(EmaRdm.SERVICE_GROUP_FILTER, directoryRefresh.filter());
        assertFalse(directoryRefresh.checkHasSequenceNumber());
        assertFalse(directoryRefresh.checkHasServiceId());
        assertFalse(directoryRefresh.clearCache());
        assertFalse(directoryRefresh.complete());
        assertFalse(directoryRefresh.solicited());
        assertFalse(directoryRefresh.doNotCache());
        assertTrue(directoryRefresh.serviceList().isEmpty());

        OmmState status = directoryRefresh.state();
        assertEquals(OmmState.StreamState.OPEN, status.streamState());
        assertEquals(OmmState.DataState.OK, status.dataState());
        assertEquals(OmmState.StatusCode.NONE, status.statusCode());
        assertEquals("", status.statusText());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDirectoryRefresh_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryRefresh.message(null);
    }

    @Test
    public void givenRefreshMsgWithWrongDomain_whenDecode_thenThrowExceptionWithoutChangingState()
    {
        setDirectoryRefresh(directoryRefresh);

        RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
        refreshMsg.domainType(EmaRdm.MMT_LOGIN);
        refreshMsg.payload(EmaFactory.createMap());

        try
        {
            directoryRefresh.message(refreshMsg);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            checkDirectoryRefresh(directoryRefresh, true);
        }
    }

    @Test
    public void givenRefreshMsgWithNonMapPayload_whenDecode_thenThrowExceptionWithoutChangingState()
    {
        setDirectoryRefresh(directoryRefresh);

        RefreshMsg refreshMsg = EmaFactory.createRefreshMsg();
        refreshMsg.domainType(EmaRdm.MMT_DIRECTORY);
        refreshMsg.payload(EmaFactory.createElementList());

        try
        {
            directoryRefresh.message(refreshMsg);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            checkDirectoryRefresh(directoryRefresh, true);
        }
    }

    @Test
    public void givenDirectoryRefreshWithoutOptionalFields_whenEncodeToMessage_thenLeaveOptionalMessageFieldsAbsent()
    {
        directoryRefresh.streamId(7);
        directoryRefresh.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "OK");

        RefreshMsg refreshMsg = directoryRefresh.message();

        assertEquals(EmaRdm.MMT_DIRECTORY, refreshMsg.domainType());
        assertEquals(7, refreshMsg.streamId());
        assertFalse(refreshMsg.hasSeqNum());
        assertFalse(refreshMsg.hasServiceId());
        assertFalse(refreshMsg.solicited());
        assertFalse(refreshMsg.complete());
        assertFalse(refreshMsg.clearCache());
        assertFalse(refreshMsg.doNotCache());
    }

    @Test
    public void givenDirectoryRefreshWithDefaultFieldsValues_whenEncodeToMessage_thenReturnCorrectRefreshMsg()
    {
        RefreshMsg refreshMsg = directoryRefresh.message();

        assertEquals(EmaRdm.MMT_DIRECTORY, refreshMsg.domainType());
        assertEquals(-1, refreshMsg.streamId());

        assertFalse(refreshMsg.hasSeqNum());
        assertFalse(refreshMsg.hasServiceId());
        assertFalse(refreshMsg.solicited());
        assertFalse(refreshMsg.complete());
        assertFalse(refreshMsg.clearCache());
        assertFalse(refreshMsg.doNotCache());

        OmmState status = directoryRefresh.state();
        assertEquals(OmmState.StreamState.OPEN, status.streamState());
        assertEquals(OmmState.DataState.OK, status.dataState());
        assertEquals(OmmState.StatusCode.NONE, status.statusCode());
        assertEquals("", status.statusText());

    }

    @Test
    public void givenDirectoryRefreshWithServiceId_whenEncodeToMessage_thenIncludeServiceId()
    {
        directoryRefresh.streamId(7);
        directoryRefresh.serviceId(5);
        directoryRefresh.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "OK");

        RefreshMsg refreshMsg = directoryRefresh.message();

        assertTrue(refreshMsg.hasServiceId());
        assertEquals(5, refreshMsg.serviceId());
    }

    @Test
    public void givenPrePopulatedDirectoryRefresh_whenDecodeMinimalRefreshMsg_thenClearAbsentOptionalFieldsAndServices()
    {
        setDirectoryRefresh(directoryRefresh);

        DirectoryRefresh sourceDirectoryRefresh = EmaFactory.Domain.createDirectoryRefresh();
        sourceDirectoryRefresh.streamId(11);
        sourceDirectoryRefresh.filter(EmaRdm.SERVICE_GROUP_FILTER);
        sourceDirectoryRefresh.state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "OK");

        RefreshMsg refreshMsg = decodeRefreshMsg(sourceDirectoryRefresh.message());

        directoryRefresh.message(refreshMsg);

        assertEquals(11, directoryRefresh.streamId());
        assertEquals(EmaRdm.SERVICE_GROUP_FILTER, directoryRefresh.filter());
        assertFalse(directoryRefresh.checkHasSequenceNumber());
        assertFalse(directoryRefresh.checkHasServiceId());
        assertFalse(directoryRefresh.clearCache());
        assertFalse(directoryRefresh.complete());
        assertFalse(directoryRefresh.solicited());
        assertFalse(directoryRefresh.doNotCache());
        assertTrue(directoryRefresh.serviceList().isEmpty());

        OmmState status = directoryRefresh.state();
        assertEquals(OmmState.StreamState.OPEN, status.streamState());
        assertEquals(OmmState.DataState.OK, status.dataState());
        assertEquals(OmmState.StatusCode.NONE, status.statusCode());
        assertEquals("OK", status.statusText());
    }

    @Test
    public void givenDirectoryRefresh_whenEncodeAndThenDecodeIntoOtherDirectoryRefresh_thenFillOtherDirectoryRefreshFields()
    {
        // encode
        setDirectoryRefresh(directoryRefresh);
        RefreshMsg encRefreshMsg = directoryRefresh.message();

        RefreshMsg decRefreshMsg = decodeRefreshMsg(encRefreshMsg);

        // decode
        DirectoryRefresh otherDirectoryRefresh = EmaFactory.Domain.createDirectoryRefresh();
        otherDirectoryRefresh.message(decRefreshMsg);

        checkDirectoryRefresh(otherDirectoryRefresh, false);
    }

    private void checkDirectoryRefresh(DirectoryRefresh directoryRefresh, boolean isCopied)
    {
        assertTrue(directoryRefresh.checkHasSequenceNumber());
        assertTrue(directoryRefresh.checkHasServiceId());
        assertTrue(directoryRefresh.clearCache());
        assertTrue(directoryRefresh.complete());
        assertTrue(directoryRefresh.solicited());
        assertTrue(directoryRefresh.doNotCache());

        assertEquals(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER, directoryRefresh.filter());
        assertEquals(EmaRdm.MMT_DIRECTORY, directoryRefresh.domainType());
        assertEquals(7, directoryRefresh.streamId());
        assertEquals(5, directoryRefresh.serviceId());

        OmmState status = directoryRefresh.state();
        assertEquals(OmmState.StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(OmmState.DataState.SUSPECT, status.dataState());
        assertEquals(OmmState.StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());

        assertEquals(3, directoryRefresh.serviceList().size());

        DirectoryService directoryService = directoryRefresh.serviceList().get(0);
        assertEquals(101, directoryService.serviceId());
        assertEquals(MapEntry.MapAction.ADD, directoryService.action());
        assertTrue(directoryService.checkHasInfo());
        assertFalse(directoryService.checkHasLink());
        assertFalse(directoryService.checkHasData());
        assertFalse(directoryService.checkHasState());
        assertFalse(directoryService.checkHasLoad());
        DirectoryServiceInfo directoryServiceInfo = directoryService.info();
        assertEquals("Test Service 1", directoryServiceInfo.serviceName());
        assertTrue(directoryServiceInfo.checkHasVendor());
        assertEquals("Test Vendor 1", directoryServiceInfo.vendor());
        assertTrue(directoryServiceInfo.checkHasItemList());
        assertEquals("Test Item List 1", directoryServiceInfo.itemList());
        assertEquals(Arrays.asList(1L, 2L), directoryServiceInfo.capabilitiesList());

        directoryService = directoryRefresh.serviceList().get(1);
        assertEquals(102, directoryService.serviceId());
        assertEquals(MapEntry.MapAction.UPDATE, directoryService.action());
        assertTrue(directoryService.checkHasInfo());
        assertFalse(directoryService.checkHasLink());
        assertFalse(directoryService.checkHasData());
        assertFalse(directoryService.checkHasState());
        assertFalse(directoryService.checkHasLoad());
        directoryServiceInfo = directoryService.info();
        assertEquals("Test Service 2", directoryServiceInfo.serviceName());
        assertTrue(directoryServiceInfo.checkHasVendor());
        assertEquals("Test Vendor 2", directoryServiceInfo.vendor());
        assertTrue(directoryServiceInfo.checkHasItemList());
        assertEquals("Test Item List 2", directoryServiceInfo.itemList());
        assertEquals(Arrays.asList(3L, 4L), directoryServiceInfo.capabilitiesList());

        directoryService = directoryRefresh.serviceList().get(2);
        assertEquals(103, directoryService.serviceId());
        assertEquals(MapEntry.MapAction.DELETE, directoryService.action());
        assertFalse(directoryService.checkHasLink());
        assertFalse(directoryService.checkHasData());
        assertFalse(directoryService.checkHasState());
        assertFalse(directoryService.checkHasLoad());
        if (isCopied)
        {
            assertTrue(directoryService.checkHasInfo());
            directoryServiceInfo = directoryService.info();
            assertEquals("Test Service 3", directoryServiceInfo.serviceName());
            assertTrue(directoryServiceInfo.checkHasVendor());
            assertEquals("Test Vendor 3", directoryServiceInfo.vendor());
            assertTrue(directoryServiceInfo.checkHasItemList());
            assertEquals("Test Item List 3", directoryServiceInfo.itemList());
            assertEquals(Arrays.asList(5L, 6L), directoryServiceInfo.capabilitiesList());
        }
        else
        {
            assertFalse(directoryService.checkHasInfo());
        }
    }

    private DirectoryService setDirectoryRefresh(DirectoryRefresh directoryRefresh)
    {
        directoryRefresh.filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER);
        directoryRefresh.streamId(7);
        directoryRefresh.serviceId(5);
        directoryRefresh.sequenceNumber(1);
        directoryRefresh.solicited(true);
        directoryRefresh.complete(true);
        directoryRefresh.clearCache(true);
        directoryRefresh.doNotCache(true);
        directoryRefresh.state(OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT,
                OmmState.StatusCode.SOURCE_UNKNOWN, "Test message");

        List<DirectoryService> directoryServiceList = new ArrayList<>();
        DirectoryService directoryService = EmaFactory.Domain.createDirectoryService();
        DirectoryService firstDirectoryService = directoryService;
        directoryService.serviceId(101);
        directoryService.action(MapEntry.MapAction.ADD);
        DirectoryServiceInfo directoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo();
        directoryServiceInfo.serviceName("Test Service 1");
        directoryServiceInfo.vendor("Test Vendor 1");
        directoryServiceInfo.itemList("Test Item List 1");
        directoryServiceInfo.capabilitiesList(Arrays.asList(1L, 2L));
        directoryService.info(directoryServiceInfo);
        directoryServiceList.add(directoryService);

        directoryService = EmaFactory.Domain.createDirectoryService();
        directoryService.serviceId(102);
        directoryService.action(MapEntry.MapAction.UPDATE);
        directoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo();
        directoryServiceInfo.serviceName("Test Service 2");
        directoryServiceInfo.vendor("Test Vendor 2");
        directoryServiceInfo.itemList("Test Item List 2");
        directoryServiceInfo.capabilitiesList(Arrays.asList(3L, 4L));
        directoryService.info(directoryServiceInfo);
        directoryServiceList.add(directoryService);

        directoryService = EmaFactory.Domain.createDirectoryService();
        directoryService.serviceId(103);
        directoryService.action(MapEntry.MapAction.DELETE);
        directoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo();
        directoryServiceInfo.serviceName("Test Service 3");
        directoryServiceInfo.vendor("Test Vendor 3");
        directoryServiceInfo.itemList("Test Item List 3");
        directoryServiceInfo.capabilitiesList(Arrays.asList(5L, 6L));
        directoryService.info(directoryServiceInfo);
        directoryServiceList.add(directoryService);

        directoryRefresh.serviceList(directoryServiceList);

        return firstDirectoryService;
    }

    private RefreshMsg decodeRefreshMsg(RefreshMsg encRefreshMsg)
    {
        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        RefreshMsg decRefreshMsg = JUnitTestConnect.createRefreshMsg();
        JUnitTestConnect.setRsslData(decRefreshMsg, encRefreshMsg, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl) dictionary).rsslDataDictionary(), null);

        return decRefreshMsg;
    }
}