/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryStatus;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.State;
import org.junit.Test;

import java.nio.ByteBuffer;

import static com.refinitiv.ema.access.OmmState.*;
import static org.junit.Assert.*;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

public class DirectoryStatusTest
{
    DirectoryStatus directoryStatus = EmaFactory.Domain.createDirectoryStatus();
    byte[] inputByte = { 0x57, 0x61, 0x69, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x4C, 0x42, 0x4D,
            0x2E, 0x2E, 0x2E };

    @Test
    public void givenDirectoryStatus_whenClear_thenClearAllFields()
    {
        directoryStatus.clearCache(true);
        OmmBuffer buffer = new OmmBufferImpl();
        directoryStatus.permissionData(buffer.buffer());
        directoryStatus.state(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");
        directoryStatus.filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER);
        directoryStatus.serviceId(1);
        directoryStatus.streamId(5);

        assertTrue(directoryStatus.clearCache());
        assertTrue(directoryStatus.checkHasState());
        assertTrue(directoryStatus.checkHasPermissionData());

        directoryStatus.clear();

        assertFalse(directoryStatus.clearCache());
        assertFalse(directoryStatus.checkHasState());
        assertFalse(directoryStatus.checkHasPermissionData());
        assertFalse(directoryStatus.checkHasServiceId());
        assertFalse(directoryStatus.checkHasFilter());

        assertEquals(EmaRdm.MMT_DIRECTORY, directoryStatus.domainType());
        assertEquals(-1, directoryStatus.streamId());
    }

    @Test
    public void givenDirectoryStatus_whenUsingFluentApi_thenReturnSameInstance()
    {
        DirectoryStatus otherDirectoryStatus = EmaFactory.Domain.createDirectoryStatus();
        setDirectoryStatus(otherDirectoryStatus);

        StatusMsg encStatusMsg = otherDirectoryStatus.message();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        StatusMsg decStatusMsg = JUnitTestConnect.createStatusMsg();
        JUnitTestConnect.setRsslData(decStatusMsg, encStatusMsg, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        DirectoryStatus returned = directoryStatus.clear()
                .streamId(5)
                .filter(EmaRdm.SERVICE_INFO_FILTER)
                .serviceId(3)
                .clearCache(true)
                .permissionData(ByteBuffer.wrap(new byte[] { 0x01, 0x02 }))
                .state(StreamState.OPEN, DataState.OK, StatusCode.NONE, "OK");

        assertSame(directoryStatus, returned);
        assertSame(directoryStatus, directoryStatus.copy(otherDirectoryStatus));
        assertSame(directoryStatus, directoryStatus.message(decStatusMsg));
        checkDirectoryStatus(directoryStatus);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsState_whenSetState_thenThrowException()
    {
        directoryStatus.state(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenStateNotSet_whenGetState_thenThrowException()
    {
        directoryStatus.state();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsPermissionData_whenSetPermissionData_thenThrowException()
    {
        directoryStatus.permissionData(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenPermissionDataNotSet_whenGetPermissionData_thenThrowException()
    {
        directoryStatus.permissionData();
    }

    @Test
    public void givenStatusFields_whenSetStatusFieldsSeparately_thenGetCorrectStatus()
    {
        assertFalse(directoryStatus.checkHasState());

        directoryStatus.state(StreamState.CLOSED, DataState.NO_CHANGE, StatusCode.NONE,
                "Test message");

        assertTrue(directoryStatus.checkHasState());

        OmmState status = directoryStatus.state();
        assertEquals(StreamState.CLOSED, status.streamState());
        assertEquals(DataState.NO_CHANGE, status.dataState());
        assertEquals(StatusCode.NONE, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test
    public void givenOmmState_whenSetStatusAsOneObject_thenGetCorrectStatus()
    {
        assertFalse(directoryStatus.checkHasState());

        OmmStateImpl statusBefore = new OmmStateImpl();
        State rsslState = CodecFactory.createState();
        rsslState.streamState(StreamState.CLOSED);
        rsslState.dataState(DataState.OK);
        rsslState.code(StatusCode.ERROR);
        Buffer stateText = CodecFactory.createBuffer();
        stateText.data("Test message");
        rsslState.text(stateText);
        statusBefore.decode(rsslState);

        directoryStatus.state(statusBefore);

        assertTrue(directoryStatus.checkHasState());

        OmmState status = directoryStatus.state();
        assertEquals(StreamState.CLOSED, status.streamState());
        assertEquals(DataState.OK, status.dataState());
        assertEquals(StatusCode.ERROR, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test
    public void givenNullStatusText_whenSetStateFieldsSeparately_thenThrowExceptionAndLeaveStateAbsent()
    {
        assertFalse(directoryStatus.checkHasState());

        try
        {
            directoryStatus.state(StreamState.OPEN, DataState.OK, StatusCode.NONE, null);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertFalse(directoryStatus.checkHasState());
        }
    }

    @Test
    public void givenInvalidDataState_whenSetStateFieldsSeparately_thenThrowExceptionAndLeaveStateAbsent()
    {
        assertFalse(directoryStatus.checkHasState());

        try
        {
            directoryStatus.state(StreamState.OPEN, 99, StatusCode.NONE, "Test message");
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertFalse(directoryStatus.checkHasState());
        }
    }

    @Test
    public void givenInvalidStreamStateInOmmState_whenSetStateObject_thenThrowExceptionAndLeaveStateAbsent()
    {
        assertFalse(directoryStatus.checkHasState());

        OmmState invalidState = mock(OmmState.class);
        when(invalidState.streamState()).thenReturn(99);
        when(invalidState.dataState()).thenReturn(DataState.OK);
        when(invalidState.statusCode()).thenReturn(StatusCode.NONE);
        when(invalidState.statusText()).thenReturn("Test message");

        try
        {
            directoryStatus.state(invalidState);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertFalse(directoryStatus.checkHasState());
        }
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDirectoryStatus_whenCopyWithNullAsParameter_thenReturnFalse()
    {
        directoryStatus.copy(null);
    }

    @Test
    public void givenDirectoryStatus_whenCopyOtherDirectoryStatus_thenReturnTrueAndUpdateFields()
    {
        DirectoryStatus otherDirectoryStatus = EmaFactory.Domain.createDirectoryStatus();
        setDirectoryStatus(otherDirectoryStatus);

        directoryStatus.copy(otherDirectoryStatus);

        checkDirectoryStatus(directoryStatus);
    }

    @Test
    public void givenDirectoryStatusWithOptionalFields_whenCopyFromSourceWithoutOptionals_thenClearDestinationOptionals()
    {
        setDirectoryStatus(directoryStatus);

        DirectoryStatus otherDirectoryStatus = EmaFactory.Domain.createDirectoryStatus();
        otherDirectoryStatus.streamId(17);
        otherDirectoryStatus.filter(EmaRdm.SERVICE_GROUP_FILTER);

        directoryStatus.copy(otherDirectoryStatus);

        assertEquals(17, directoryStatus.streamId());
        assertEquals(EmaRdm.SERVICE_GROUP_FILTER, directoryStatus.filter());
        assertFalse(directoryStatus.clearCache());
        assertFalse(directoryStatus.checkHasPermissionData());
        assertFalse(directoryStatus.checkHasState());
    }

    @Test
    public void givenDirectoryStatus_whenCopyIntoSelf_thenPreserveFields()
    {
        setDirectoryStatus(directoryStatus);

        directoryStatus.copy(directoryStatus);

        checkDirectoryStatus(directoryStatus);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDirectoryStatus_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryStatus.message(null);
    }

    @Test
    public void givenDirectoryStatus_whenDecodeWithWrongDomain_thenThrowExceptionWithoutChangingState()
    {
        setDirectoryStatus(directoryStatus);

        StatusMsg wrongDomainMsg = EmaFactory.createStatusMsg();
        wrongDomainMsg.domainType(EmaRdm.MMT_LOGIN);
        wrongDomainMsg.streamId(13);

        try
        {
            directoryStatus.message(wrongDomainMsg);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            checkDirectoryStatus(directoryStatus);
        }
    }

    @Test
    public void givenDirectoryStatusWithOptionalFields_whenDecodeSparseStatusMsg_thenClearDestinationOptionals()
    {
        setDirectoryStatus(directoryStatus);

        StatusMsg sparseStatusMsg = EmaFactory.createStatusMsg();
        sparseStatusMsg.domainType(EmaRdm.MMT_DIRECTORY);
        sparseStatusMsg.streamId(11);

        directoryStatus.message(sparseStatusMsg);

        assertEquals(11, directoryStatus.streamId());
        assertFalse(directoryStatus.checkHasFilter());
        assertFalse(directoryStatus.clearCache());
        assertFalse(directoryStatus.checkHasServiceId());
        assertFalse(directoryStatus.checkHasPermissionData());
        assertFalse(directoryStatus.checkHasState());
    }

    @Test
    public void givenDirectoryStatus_whenEncodeAndThenDecodeIntoOtherDirectoryStatus_thenFillOtherDirectoryStatusFields()
    {
        // encode
        setDirectoryStatus(directoryStatus);
        StatusMsg encStatusMsg = directoryStatus.message();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        StatusMsg decStatusMsg = JUnitTestConnect.createStatusMsg();

        JUnitTestConnect.setRsslData(decStatusMsg, encStatusMsg, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        // decode
        DirectoryStatus otherDirectoryStatus = EmaFactory.Domain.createDirectoryStatus();
        otherDirectoryStatus.message(decStatusMsg);

        checkDirectoryStatus(otherDirectoryStatus);
    }

    @Test
    public void givenPermissionDataSourceBufferMutated_whenPermissionDataWasSet_thenStoredPermissionDataRemainsUnchanged()
    {
        byte[] sourceBytes = { 0x01, 0x02, 0x03 };
        ByteBuffer sourceBuffer = ByteBuffer.wrap(sourceBytes);

        directoryStatus.permissionData(sourceBuffer);
        sourceBytes[0] = 0x05;

        assertTrue(directoryStatus.checkHasPermissionData());
        assertArrayEquals(new byte[] { 0x01, 0x02, 0x03 }, directoryStatus.permissionData().array());
    }

    @Test
    public void givenReturnedPermissionDataBufferPositionChanged_whenEncode_thenEncodedPermissionDataRemainsUnchanged()
    {
        directoryStatus.permissionData(ByteBuffer.wrap(new byte[] { 0x01, 0x02, 0x03 }));

        ByteBuffer returnedPermissionData = directoryStatus.permissionData();
        returnedPermissionData.get();

        StatusMsg statusMsg = directoryStatus.message();

        assertTrue(statusMsg.hasPermissionData());
        assertArrayEquals(new byte[] { 0x01, 0x02, 0x03 }, statusMsg.permissionData().array());
    }

    private void setDirectoryStatus(DirectoryStatus directoryStatus)
    {
        directoryStatus.filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER);
        directoryStatus.serviceId(3);
        directoryStatus.streamId(5);
        directoryStatus.clearCache(true);
        ByteBuffer inputByteBuf = ByteBuffer.wrap(inputByte);
        directoryStatus.permissionData(inputByteBuf);
        directoryStatus.state(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");
    }

    private void checkDirectoryStatus(DirectoryStatus directoryStatus)
    {
        assertTrue(directoryStatus.clearCache());
        assertEquals(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER, directoryStatus.filter());
        assertEquals(5, directoryStatus.streamId());
        assertEquals(3, directoryStatus.serviceId());
        assertTrue(directoryStatus.checkHasPermissionData());
        assertArrayEquals(inputByte, directoryStatus.permissionData().array());

        assertTrue(directoryStatus.checkHasState());
        OmmState status = directoryStatus.state();
        assertEquals(StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(DataState.SUSPECT, status.dataState());
        assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());
    }
}