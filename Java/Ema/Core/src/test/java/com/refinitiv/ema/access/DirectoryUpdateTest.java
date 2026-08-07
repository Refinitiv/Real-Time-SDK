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

public class DirectoryUpdateTest 
{
    DirectoryUpdate directoryUpdate = EmaFactory.Domain.createDirectoryUpdate();

    @Test
    public void givenDirectoryUpdate_whenClear_thenClearAllFields()
    {
        setDirectoryUpdate(directoryUpdate);

        assertTrue(directoryUpdate.checkHasSequenceNumber());

        directoryUpdate.clear();

        assertFalse(directoryUpdate.checkHasSequenceNumber());
        assertFalse(directoryUpdate.doNotCache());
        assertFalse(directoryUpdate.doNotConflate());
        assertFalse(directoryUpdate.checkHasFilter());

        assertTrue(directoryUpdate.serviceList().isEmpty());
        assertEquals(EmaRdm.MMT_DIRECTORY, directoryUpdate.domainType());
        assertEquals(-1, directoryUpdate.streamId());
    }

    @Test
    public void givenDirectoryUpdate_whenUsingFluentApi_thenReturnSameInstance()
    {
        DirectoryUpdate otherDirectoryUpdate = EmaFactory.Domain.createDirectoryUpdate();
        setDirectoryUpdate(otherDirectoryUpdate);
        UpdateMsg decUpdateMsg = decodeUpdateMsg(otherDirectoryUpdate.message());

        DirectoryUpdate returned = directoryUpdate.clear()
                .streamId(5)
                .filter(EmaRdm.SERVICE_INFO_FILTER)
                .sequenceNumber(1)
                .doNotCache(true)
                .doNotConflate(true)
                .serviceList(new ArrayList<>());

        assertSame(directoryUpdate, returned);
        assertSame(directoryUpdate, directoryUpdate.copy(otherDirectoryUpdate));
        assertSame(directoryUpdate, directoryUpdate.message(decUpdateMsg));
        checkDirectoryUpdate(directoryUpdate, false);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenSequenceNumberNotSet_whenGetSequenceNumber_thenThrowException()
    {
        directoryUpdate.sequenceNumber();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenFilterNotSet_whenGetFilter_thenThrowException()
    {
        directoryUpdate.filter();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsServiceList_whenSetServiceList_thenThrowException()
    {
        directoryUpdate.serviceList(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDirectoryUpdate_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryUpdate.copy(null);
    }

    @Test
    public void givenDirectoryUpdate_whenCopyOtherDirectoryUpdate_thenUpdateFields()
    {
        DirectoryUpdate otherDirectoryUpdate = EmaFactory.Domain.createDirectoryUpdate();
        DirectoryService firstDirectoryService = setDirectoryUpdate(otherDirectoryUpdate);

        directoryUpdate.copy(otherDirectoryUpdate);

        assertNotSame(firstDirectoryService, directoryUpdate.serviceList().get(0));

        checkDirectoryUpdate(directoryUpdate, true);
    }

    @Test
    public void givenDirectoryUpdate_whenCopySelf_thenLeaveFieldsAndReferencesUnchanged()
    {
        DirectoryService firstDirectoryService = setDirectoryUpdate(directoryUpdate);

        directoryUpdate.copy(directoryUpdate);

        assertSame(firstDirectoryService, directoryUpdate.serviceList().get(0));
        checkDirectoryUpdate(directoryUpdate, true);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDirectoryUpdate_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryUpdate.message(null);
    }

    @Test
    public void givenUpdateMsgWithWrongDomain_whenDecode_thenThrowException()
    {
        UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
        updateMsg.domainType(EmaRdm.MMT_LOGIN);
        updateMsg.payload(EmaFactory.createMap());

        assertInvalidUsageExceptionMessage("Domain type must be Directory.", () -> directoryUpdate.message(updateMsg));
    }

    @Test
    public void givenUpdateMsgWithNonMapPayload_whenDecode_thenThrowException()
    {
        UpdateMsg updateMsg = EmaFactory.createUpdateMsg();
        updateMsg.domainType(EmaRdm.MMT_DIRECTORY);
        updateMsg.payload(EmaFactory.createElementList());

        assertInvalidUsageExceptionMessage("Payload data type should be Map.", () -> directoryUpdate.message(updateMsg));
    }

    @Test
    public void givenDirectoryUpdateWithAllOptionalFields_whenEncodeToMessage_thenPopulateUpdateMsg()
    {
        setDirectoryUpdate(directoryUpdate);

        UpdateMsg updateMsg = directoryUpdate.message();

        assertEquals(EmaRdm.MMT_DIRECTORY, updateMsg.domainType());
        assertEquals(5, updateMsg.streamId());
        assertTrue(updateMsg.hasFilter());
        assertEquals(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER, updateMsg.filter());
        assertTrue(updateMsg.hasSeqNum());
        assertEquals(1, updateMsg.seqNum());
        assertTrue(updateMsg.doNotCache());
        assertTrue(updateMsg.doNotConflate());
    }

    @Test
    public void givenDirectoryUpdateWithoutOptionalFields_whenEncodeToMessage_thenLeaveOptionalMessageFieldsAbsent()
    {
        directoryUpdate.streamId(7);

        UpdateMsg updateMsg = directoryUpdate.message();

        assertEquals(EmaRdm.MMT_DIRECTORY, updateMsg.domainType());
        assertEquals(7, updateMsg.streamId());
        assertFalse(updateMsg.hasFilter());
        assertFalse(updateMsg.hasSeqNum());
        assertFalse(updateMsg.doNotCache());
        assertFalse(updateMsg.doNotConflate());
    }

    @Test
    public void givenPrePopulatedDirectoryUpdate_whenDecodeMinimalUpdateMsg_thenClearAbsentOptionalFieldsAndServices()
    {
        setDirectoryUpdate(directoryUpdate);

        DirectoryUpdate sourceDirectoryUpdate = EmaFactory.Domain.createDirectoryUpdate();
        sourceDirectoryUpdate.streamId(11);

        UpdateMsg updateMsg = decodeUpdateMsg(sourceDirectoryUpdate.message());

        directoryUpdate.message(updateMsg);

        assertEquals(11, directoryUpdate.streamId());
        assertFalse(directoryUpdate.checkHasFilter());
        assertFalse(directoryUpdate.checkHasSequenceNumber());
        assertFalse(directoryUpdate.doNotCache());
        assertFalse(directoryUpdate.doNotConflate());
        assertTrue(directoryUpdate.serviceList().isEmpty());
    }

    @Test
    public void givenPrePopulatedDirectoryUpdate_whenDecodeUpdateWithSubsetOfOptionals_thenOnlyThoseOptionalsRemainPresent()
    {
        setDirectoryUpdate(directoryUpdate);

        DirectoryUpdate sourceDirectoryUpdate = EmaFactory.Domain.createDirectoryUpdate();
        sourceDirectoryUpdate.streamId(13);
        sourceDirectoryUpdate.filter(EmaRdm.SERVICE_GROUP_FILTER);
        sourceDirectoryUpdate.doNotConflate(true);

        UpdateMsg updateMsg = decodeUpdateMsg(sourceDirectoryUpdate.message());

        directoryUpdate.message(updateMsg);

        assertEquals(13, directoryUpdate.streamId());
        assertTrue(directoryUpdate.checkHasFilter());
        assertEquals(EmaRdm.SERVICE_GROUP_FILTER, directoryUpdate.filter());
        assertFalse(directoryUpdate.checkHasSequenceNumber());
        assertFalse(directoryUpdate.doNotCache());
        assertTrue(directoryUpdate.doNotConflate());
        assertTrue(directoryUpdate.serviceList().isEmpty());
    }

    @Test
    public void givenDirectoryUpdate_whenEncodeAndThenDecodeIntoOtherDirectoryUpdate_thenFillOtherDirectoryUpdateFields()
    {
        // encode
        setDirectoryUpdate(directoryUpdate);

        UpdateMsg encUpdateMsg = directoryUpdate.message();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        UpdateMsg decUpdateMsg = JUnitTestConnect.createUpdateMsg();

        JUnitTestConnect.setRsslData(decUpdateMsg, encUpdateMsg, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        // decode
        DirectoryUpdate otherDirectoryUpdate = EmaFactory.Domain.createDirectoryUpdate();
        otherDirectoryUpdate.message(decUpdateMsg);

        checkDirectoryUpdate(otherDirectoryUpdate, false);
    }

    private void checkDirectoryUpdate(DirectoryUpdate directoryUpdate, boolean isCopied)
    {
        assertTrue(directoryUpdate.checkHasSequenceNumber());
        assertTrue(directoryUpdate.checkHasFilter());
        assertTrue(directoryUpdate.doNotConflate());
        assertTrue(directoryUpdate.doNotCache());

        assertEquals(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER, directoryUpdate.filter());
        assertEquals(5, directoryUpdate.streamId());
        assertEquals(EmaRdm.MMT_DIRECTORY, directoryUpdate.domainType());

        assertEquals(3, directoryUpdate.serviceList().size());

        DirectoryService directoryService = directoryUpdate.serviceList().get(0);
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

        directoryService = directoryUpdate.serviceList().get(1);
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

        directoryService = directoryUpdate.serviceList().get(2);
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

    private DirectoryService setDirectoryUpdate(DirectoryUpdate directoryUpdate)
    {
        directoryUpdate.filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER);
        directoryUpdate.streamId(5);
        directoryUpdate.sequenceNumber(1);
        directoryUpdate.doNotCache(true);
        directoryUpdate.doNotConflate(true);

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

        directoryUpdate.serviceList(directoryServiceList);

        return firstDirectoryService;
    }

    private UpdateMsg decodeUpdateMsg(UpdateMsg encUpdateMsg)
    {
        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        UpdateMsg decUpdateMsg = JUnitTestConnect.createUpdateMsg();
        JUnitTestConnect.setRsslData(decUpdateMsg, encUpdateMsg, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl) dictionary).rsslDataDictionary(), null);

        return decUpdateMsg;
    }

    private void assertInvalidUsageExceptionMessage(String expectedMessageFragment, Runnable action)
    {
        try
        {
            action.run();
            fail("Expected OmmInvalidUsageExceptionImpl containing: " + expectedMessageFragment);
        }
        catch (OmmInvalidUsageExceptionImpl exception)
        {
            assertTrue(exception.getMessage().contains(expectedMessageFragment));
        }
    }
}