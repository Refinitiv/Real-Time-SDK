/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryConsumerStatusService;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.Codec;
import org.junit.Test;

import static com.refinitiv.ema.access.MapEntry.*;
import static com.refinitiv.ema.rdm.EmaRdm.*;
import static org.junit.Assert.*;

public class DirectoryConsumerStatusServiceTest
{
    DirectoryConsumerStatusService directoryConsumerStatusService =
            EmaFactory.Domain.createDirectoryConsumerStatusService();

    @Test
    public void givenServiceLink_whenClear_thenClearAllFields()
    {
        directoryConsumerStatusService.serviceId(101);
        directoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.ACTIVE_WITH_STANDBY);
        directoryConsumerStatusService.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
        directoryConsumerStatusService.action(MapAction.UPDATE);

        assertTrue(directoryConsumerStatusService.checkHasWarmStandbyMode());

        directoryConsumerStatusService.clear();

        assertFalse(directoryConsumerStatusService.checkHasWarmStandbyMode());
        assertFalse(directoryConsumerStatusService.checkHasSourceMirroringMode());

        assertEquals(0, directoryConsumerStatusService.serviceId());
        assertEquals(MapAction.ADD, directoryConsumerStatusService.action());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidSourceMirroringMode_whenSetSourceMirroringMode_thenThrowException()
    {
        directoryConsumerStatusService.sourceMirroringMode(99);
    }

    @Test
    public void givenValidBoundaryServiceIds_whenSetServiceId_thenAcceptValues()
    {
        directoryConsumerStatusService.serviceId(0);
        assertEquals(0, directoryConsumerStatusService.serviceId());

        directoryConsumerStatusService.serviceId(65535);
        assertEquals(65535, directoryConsumerStatusService.serviceId());
    }

    @Test
    public void givenNegativeServiceId_whenSetServiceId_thenThrowException()
    {
        assertInvalidUsageExceptionMessage("Invalid serviceId value of -1",
                () -> directoryConsumerStatusService.serviceId(-1));
    }

    @Test
    public void givenTooLargeServiceId_whenSetServiceId_thenThrowException()
    {
        assertInvalidUsageExceptionMessage("Invalid serviceId value of 65536",
                () -> directoryConsumerStatusService.serviceId(65536));
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenWarmStandbyModeNotSet_whenGetWarmStandbyMode_thenThrowException()
    {
        directoryConsumerStatusService.warmStandbyMode();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidWarmStandbyMode_whenSetWarmStandbyMode_thenThrowException()
    {
        directoryConsumerStatusService.warmStandbyMode(99);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenConsumerStatusService_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryConsumerStatusService.copy(null);
    }

    @Test
    public void givenConsumerStatusService_whenCopyOtherConsumerStatusService_thenReturnTrueAndUpdateFields()
    {
        DirectoryConsumerStatusService otherDirectoryConsumerStatusService =
                EmaFactory.Domain.createDirectoryConsumerStatusService();
        otherDirectoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.ACTIVE_WITH_STANDBY);
        otherDirectoryConsumerStatusService.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
        otherDirectoryConsumerStatusService.serviceId(101);
        otherDirectoryConsumerStatusService.action(MapAction.UPDATE);

        directoryConsumerStatusService.copy(otherDirectoryConsumerStatusService);

        assertTrue(directoryConsumerStatusService.checkHasWarmStandbyMode());

        assertEquals(101, directoryConsumerStatusService.serviceId());
        assertEquals(SourceMirroringMode.ACTIVE_WITH_STANDBY, directoryConsumerStatusService.sourceMirroringMode());
        assertEquals(WarmStandbyDirectoryServiceTypes.ACTIVE, directoryConsumerStatusService.warmStandbyMode());
        assertEquals(MapAction.UPDATE, directoryConsumerStatusService.action());
    }

    @Test
    public void givenSourceWithoutWarmStandby_whenCopy_thenRemoveWarmStandbyFromDestination()
    {
        directoryConsumerStatusService.serviceId(101);
        directoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.ACTIVE_WITH_STANDBY);
        directoryConsumerStatusService.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
        directoryConsumerStatusService.action(MapAction.UPDATE);

        DirectoryConsumerStatusService otherDirectoryConsumerStatusService =
                EmaFactory.Domain.createDirectoryConsumerStatusService();
        otherDirectoryConsumerStatusService.serviceId(202);
        otherDirectoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.STANDBY);
        otherDirectoryConsumerStatusService.action(MapAction.ADD);

        directoryConsumerStatusService.copy(otherDirectoryConsumerStatusService);

        assertFalse(directoryConsumerStatusService.checkHasWarmStandbyMode());
        assertEquals(202, directoryConsumerStatusService.serviceId());
        assertEquals(SourceMirroringMode.STANDBY, directoryConsumerStatusService.sourceMirroringMode());
        assertEquals(MapAction.ADD, directoryConsumerStatusService.action());
    }

    @Test
    public void givenConsumerStatusService_whenCopySelf_thenLeaveFieldsUnchanged()
    {
        directoryConsumerStatusService.serviceId(101);
        directoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.ACTIVE_WITH_STANDBY);
        directoryConsumerStatusService.warmStandbyMode(WarmStandbyDirectoryServiceTypes.STANDBY);
        directoryConsumerStatusService.action(MapAction.UPDATE);

        directoryConsumerStatusService.copy(directoryConsumerStatusService);

        assertTrue(directoryConsumerStatusService.checkHasWarmStandbyMode());
        assertEquals(101, directoryConsumerStatusService.serviceId());
        assertEquals(SourceMirroringMode.ACTIVE_WITH_STANDBY, directoryConsumerStatusService.sourceMirroringMode());
        assertEquals(WarmStandbyDirectoryServiceTypes.STANDBY, directoryConsumerStatusService.warmStandbyMode());
        assertEquals(MapAction.UPDATE, directoryConsumerStatusService.action());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenConsumerStatusService_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryConsumerStatusService.decode(null);
    }

    @Test
    public void givenConsumerStatusService_whenEncodeAndThenDecodeIntoOtherConsumerStatusService_thenFillOtherConsumerStatusServiceFields()
    {
        // encode
        directoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.ACTIVE_WITH_STANDBY);
        directoryConsumerStatusService.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
        ElementList encElementList = directoryConsumerStatusService.encode();

        ElementList decElementList = createDecodedElementList(encElementList);

        // decode
        DirectoryConsumerStatusService otherDirectoryConsumerStatusService =
                EmaFactory.Domain.createDirectoryConsumerStatusService();
        otherDirectoryConsumerStatusService.decode(decElementList);

        assertTrue(otherDirectoryConsumerStatusService.checkHasWarmStandbyMode());

        assertEquals(SourceMirroringMode.ACTIVE_WITH_STANDBY, otherDirectoryConsumerStatusService.sourceMirroringMode());
        assertEquals(WarmStandbyDirectoryServiceTypes.ACTIVE, otherDirectoryConsumerStatusService.warmStandbyMode());
    }

    @Test
    public void givenConsumerStatusServiceWithWarmStandbyOnly_whenEncodeAndDecode_thenSourceMirroringRemainsAbsent()
    {
        directoryConsumerStatusService.warmStandbyMode(WarmStandbyDirectoryServiceTypes.STANDBY);

        ElementList encElementList = directoryConsumerStatusService.encode();
        assertEquals(1, encElementList.size());

        ElementList decElementList = createDecodedElementList(encElementList);

        DirectoryConsumerStatusService otherDirectoryConsumerStatusService =
                EmaFactory.Domain.createDirectoryConsumerStatusService();
        otherDirectoryConsumerStatusService.decode(decElementList);

        assertFalse(otherDirectoryConsumerStatusService.checkHasSourceMirroringMode());
        assertTrue(otherDirectoryConsumerStatusService.checkHasWarmStandbyMode());
        assertEquals(WarmStandbyDirectoryServiceTypes.STANDBY, otherDirectoryConsumerStatusService.warmStandbyMode());
    }

    @Test
    public void givenConsumerStatusServiceWithoutWarmStandby_whenEncodeAndDecode_thenWarmStandbyRemainsAbsent()
    {
        directoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.STANDBY);
        ElementList decElementList = createDecodedElementList(directoryConsumerStatusService.encode());

        DirectoryConsumerStatusService otherDirectoryConsumerStatusService =
                EmaFactory.Domain.createDirectoryConsumerStatusService();
        otherDirectoryConsumerStatusService.decode(decElementList);

        assertFalse(otherDirectoryConsumerStatusService.checkHasWarmStandbyMode());
        assertEquals(SourceMirroringMode.STANDBY, otherDirectoryConsumerStatusService.sourceMirroringMode());
    }

    @Test
    public void givenDifferentPayloadFieldCombinations_whenEncode_thenEncodeExpectedNumberOfElements()
    {
        directoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.STANDBY);
        assertEquals(1, directoryConsumerStatusService.encode().size());

        directoryConsumerStatusService.clear();
        directoryConsumerStatusService.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
        assertEquals(1, directoryConsumerStatusService.encode().size());

        directoryConsumerStatusService.clear();
        directoryConsumerStatusService.sourceMirroringMode(SourceMirroringMode.ACTIVE_WITH_STANDBY);
        directoryConsumerStatusService.warmStandbyMode(WarmStandbyDirectoryServiceTypes.ACTIVE);
        assertEquals(2, directoryConsumerStatusService.encode().size());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenElementListWithoutSourceMirroringModeAndWarmStandbyMode_whenDecode_thenThrowException()
    {
        ElementList encElementList = EmaFactory.createElementList();

        ElementList decElementList = createDecodedElementList(encElementList);

        directoryConsumerStatusService.decode(decElementList);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenEmptyPayload_whenEncode_thenThrowException()
    {
        directoryConsumerStatusService.encode();
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