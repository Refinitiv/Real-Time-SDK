/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryConsumerStatus;
import com.refinitiv.ema.domain.directory.DirectoryConsumerStatusService;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.Codec;
import org.junit.Test;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

import static com.refinitiv.ema.access.MapEntry.*;
import static com.refinitiv.ema.rdm.EmaRdm.*;
import static org.junit.Assert.*;

public class DirectoryConsumerStatusTest
{
    DirectoryConsumerStatus directoryConsumerStatus = EmaFactory.Domain.createDirectoryConsumerStatus();

    @Test
    public void givenServiceLink_whenClear_thenClearAllFields()
    {
        directoryConsumerStatus.sequenceNumber(1);
        directoryConsumerStatus.streamId(2);
        DirectoryConsumerStatusService directoryConsumerStatusService =
                EmaFactory.Domain.createDirectoryConsumerStatusService();
        directoryConsumerStatus.consumerServiceStatusList(Collections.singletonList(directoryConsumerStatusService));

        assertTrue(directoryConsumerStatus.checkHasSequenceNumber());

        directoryConsumerStatus.clear();

        assertFalse(directoryConsumerStatus.checkHasSequenceNumber());

        assertTrue(directoryConsumerStatus.consumerServiceStatusList().isEmpty());
        assertEquals(EmaRdm.ENAME_CONS_STATUS, directoryConsumerStatus.name());
        assertEquals(-1, directoryConsumerStatus.streamId());
    }

    @Test
    public void givenDirectoryConsumerStatus_whenUsingFluentApi_thenReturnSameInstance()
    {
        DirectoryConsumerStatus otherDirectoryConsumerStatus = EmaFactory.Domain.createDirectoryConsumerStatus();
        setDirectoryConsumerStatusService(otherDirectoryConsumerStatus);

        GenericMsg encGenericMsg = otherDirectoryConsumerStatus.message();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        GenericMsg genericMsg = JUnitTestConnect.createGenericMsg();
        JUnitTestConnect.setRsslData(genericMsg, encGenericMsg, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        DirectoryConsumerStatus returned = directoryConsumerStatus.clear()
                .streamId(5)
                .sequenceNumber(101)
                .consumerServiceStatusList(new ArrayList<>());

        assertSame(directoryConsumerStatus, returned);
        assertSame(directoryConsumerStatus, directoryConsumerStatus.copy(otherDirectoryConsumerStatus));
        assertSame(directoryConsumerStatus, directoryConsumerStatus.message(genericMsg));
        checkDirectoryConsumerStatus(directoryConsumerStatus, false);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenSequenceNumberNotSet_whenGetSequenceNumber_thenThrowException()
    {
        directoryConsumerStatus.sequenceNumber();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsConsumerServiceStatusList_whenSetConsumerServiceStatusList_thenThrowException()
    {
        directoryConsumerStatus.consumerServiceStatusList(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenConsumerStatus_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryConsumerStatus.copy(null);
    }

    @Test
    public void givenConsumerStatus_whenCopyOtherConsumerStatus_thenUpdateFields()
    {
        DirectoryConsumerStatus otherDirectoryConsumerStatus = EmaFactory.Domain.createDirectoryConsumerStatus();
        DirectoryConsumerStatusService firstDirectoryConsumerStatusService = 
                setDirectoryConsumerStatusService(otherDirectoryConsumerStatus);

        directoryConsumerStatus.copy(otherDirectoryConsumerStatus);

        assertNotSame(firstDirectoryConsumerStatusService, directoryConsumerStatus.consumerServiceStatusList().get(0));

        checkDirectoryConsumerStatus(directoryConsumerStatus, true);
    }

    @Test
    public void givenLiveConsumerServiceStatusList_whenSetBackOnObject_thenLeaveEntriesUnchanged()
    {
        setDirectoryConsumerStatusService(directoryConsumerStatus);

        List<DirectoryConsumerStatusService> liveList = directoryConsumerStatus.consumerServiceStatusList();

        directoryConsumerStatus.consumerServiceStatusList(liveList);

        assertSame(liveList, directoryConsumerStatus.consumerServiceStatusList());
        checkDirectoryConsumerStatus(directoryConsumerStatus, true);
    }

    @Test
    public void givenConsumerStatus_whenCopySelf_thenLeaveFieldsAndReferencesUnchanged()
    {
        setDirectoryConsumerStatusService(directoryConsumerStatus);

        DirectoryConsumerStatusService firstServiceBeforeCopy = directoryConsumerStatus.consumerServiceStatusList().get(0);

        directoryConsumerStatus.copy(directoryConsumerStatus);

        assertSame(firstServiceBeforeCopy, directoryConsumerStatus.consumerServiceStatusList().get(0));
        checkDirectoryConsumerStatus(directoryConsumerStatus, true);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenConsumerStatus_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryConsumerStatus.message(null);
    }

    @Test
    public void givenGenericMsgWithWrongDomain_whenDecode_thenThrowException()
    {
        GenericMsg genericMsg = EmaFactory.createGenericMsg();
        genericMsg.domainType(EmaRdm.MMT_LOGIN);

        assertInvalidUsageExceptionMessage("Domain type must be Directory.", () -> directoryConsumerStatus.message(genericMsg));
    }

    @Test
    public void givenGenericMsgWithoutName_whenDecode_thenThrowException()
    {
        GenericMsg genericMsg = EmaFactory.createGenericMsg();
        genericMsg.domainType(EmaRdm.MMT_DIRECTORY);
        genericMsg.payload(EmaFactory.createMap());

        assertInvalidUsageExceptionMessage("Message name is absent.", () -> directoryConsumerStatus.message(genericMsg));
    }

    @Test
    public void givenGenericMsgWithWrongName_whenDecode_thenThrowException()
    {
        GenericMsg genericMsg = EmaFactory.createGenericMsg();
        genericMsg.domainType(EmaRdm.MMT_DIRECTORY);
        genericMsg.name("WrongName");
        genericMsg.payload(EmaFactory.createMap());

        assertInvalidUsageExceptionMessage("Message name is incorrect.", () -> directoryConsumerStatus.message(genericMsg));
    }

    @Test
    public void givenGenericMsgWithNonMapPayload_whenDecode_thenThrowException()
    {
        GenericMsg genericMsg = EmaFactory.createGenericMsg();
        genericMsg.domainType(EmaRdm.MMT_DIRECTORY);
        genericMsg.name(EmaRdm.ENAME_CONS_STATUS);
        genericMsg.payload(EmaFactory.createElementList());

        assertInvalidUsageExceptionMessage("Payload data type should be Map.", () -> directoryConsumerStatus.message(genericMsg));
    }

    @Test
    public void givenConsumerStatus_whenEncodeAndThenDecodeIntoOtherConsumerStatus_thenFillOtherConsumerStatusFields()
    {
        // encode
        setDirectoryConsumerStatusService(directoryConsumerStatus);

        GenericMsg encGenericMsg = directoryConsumerStatus.message();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        GenericMsg decGenericMsg = JUnitTestConnect.createGenericMsg();

        JUnitTestConnect.setRsslData(decGenericMsg, encGenericMsg, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        // decode
        DirectoryConsumerStatus otherDirectoryConsumerStatus = EmaFactory.Domain.createDirectoryConsumerStatus();
        otherDirectoryConsumerStatus.message(decGenericMsg);

        checkDirectoryConsumerStatus(otherDirectoryConsumerStatus, false);
    }

    private DirectoryConsumerStatusService setDirectoryConsumerStatusService(DirectoryConsumerStatus directoryConsumerStatus)
    {
        directoryConsumerStatus.sequenceNumber(101);
        directoryConsumerStatus.streamId(5);

        List<DirectoryConsumerStatusService> serviceList = new ArrayList<>();
        DirectoryConsumerStatusService directoryConsumerStatusService =
                EmaFactory.Domain.createDirectoryConsumerStatusService();
        DirectoryConsumerStatusService firstDirectoryConsumerStatusService = directoryConsumerStatusService;
        serviceList.add(directoryConsumerStatusService);
        directoryConsumerStatusService.serviceId(3);
        directoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.ACTIVE_WITH_STANDBY);
        directoryConsumerStatusService.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
        directoryConsumerStatusService.action(MapAction.UPDATE);

        directoryConsumerStatusService = EmaFactory.Domain.createDirectoryConsumerStatusService();
        serviceList.add(directoryConsumerStatusService);
        directoryConsumerStatusService.serviceId(5);
        directoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.ACTIVE_NO_STANDBY);
        directoryConsumerStatusService.action(MapAction.ADD);

        directoryConsumerStatusService = EmaFactory.Domain.createDirectoryConsumerStatusService();
        serviceList.add(directoryConsumerStatusService);
        directoryConsumerStatusService.serviceId(7);
        directoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.ACTIVE_NO_STANDBY);
        directoryConsumerStatusService.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
        directoryConsumerStatusService.action(MapAction.DELETE);

        directoryConsumerStatus.consumerServiceStatusList(serviceList);

        return firstDirectoryConsumerStatusService;
    }

    private void checkDirectoryConsumerStatus(DirectoryConsumerStatus directoryConsumerStatus, boolean isCopied)
    {
        assertTrue(directoryConsumerStatus.checkHasSequenceNumber());
        assertEquals(101, directoryConsumerStatus.sequenceNumber());
        assertEquals(5, directoryConsumerStatus.streamId());

        assertEquals(3, directoryConsumerStatus.consumerServiceStatusList().size());

        DirectoryConsumerStatusService directoryConsumerStatusService =
                directoryConsumerStatus.consumerServiceStatusList().get(0);
        assertEquals(3, directoryConsumerStatusService.serviceId());
        assertTrue(directoryConsumerStatusService.checkHasSourceMirroringMode());
        assertEquals(SourceMirroringMode.ACTIVE_WITH_STANDBY, directoryConsumerStatusService.sourceMirroringMode());
        assertTrue(directoryConsumerStatusService.checkHasWarmStandbyMode());
        assertEquals(WarmStandbyDirectoryServiceTypes.ACTIVE, directoryConsumerStatusService.warmStandbyMode());
        assertEquals(MapAction.UPDATE, directoryConsumerStatusService.action());

        directoryConsumerStatusService = directoryConsumerStatus.consumerServiceStatusList().get(1);
        assertEquals(5, directoryConsumerStatusService.serviceId());
        assertTrue(directoryConsumerStatusService.checkHasSourceMirroringMode());
        assertEquals(SourceMirroringMode.ACTIVE_NO_STANDBY, directoryConsumerStatusService.sourceMirroringMode());
        assertFalse(directoryConsumerStatusService.checkHasWarmStandbyMode());
        assertEquals(MapAction.ADD, directoryConsumerStatusService.action());

        directoryConsumerStatusService = directoryConsumerStatus.consumerServiceStatusList().get(2);
        assertEquals(7, directoryConsumerStatusService.serviceId());
        if (isCopied)
        {
            assertTrue(directoryConsumerStatusService.checkHasSourceMirroringMode());
            assertEquals(SourceMirroringMode.ACTIVE_NO_STANDBY, directoryConsumerStatusService.sourceMirroringMode());
            assertTrue(directoryConsumerStatusService.checkHasWarmStandbyMode());
            assertEquals(WarmStandbyDirectoryServiceTypes.ACTIVE, directoryConsumerStatusService.warmStandbyMode());
        }
        else
        {
            assertFalse(directoryConsumerStatusService.checkHasWarmStandbyMode());
            assertFalse(directoryConsumerStatusService.checkHasSourceMirroringMode());
        }
        assertEquals(MapAction.DELETE, directoryConsumerStatusService.action());
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