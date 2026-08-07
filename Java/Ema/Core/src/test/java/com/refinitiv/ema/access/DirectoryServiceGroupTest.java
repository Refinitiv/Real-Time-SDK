/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceGroup;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.State;
import org.junit.Test;

import static com.refinitiv.ema.access.FilterEntry.*;
import static com.refinitiv.ema.access.OmmState.*;
import static org.junit.Assert.*;

public class DirectoryServiceGroupTest
{
    DirectoryServiceGroup directoryServiceGroup = EmaFactory.Domain.createDirectoryServiceGroup();
    private final Buffer buffer = CodecFactory.createBuffer();

    @Test
    public void givenServiceGroup_whenClear_thenClearAllFields()
    {
        buffer.data("1.26.102");
        directoryServiceGroup.group(buffer.data());
        buffer.clear();
        buffer.data("1.26.110");
        directoryServiceGroup.mergedToGroup(buffer.data());
        directoryServiceGroup.action(FilterAction.CLEAR);
        directoryServiceGroup.status(StreamState.OPEN, DataState.OK, StatusCode.NONE, "");

        assertTrue(directoryServiceGroup.checkHasMergedToGroup());
        assertTrue(directoryServiceGroup.checkHasStatus());

        directoryServiceGroup.clear();

        assertFalse(directoryServiceGroup.checkHasMergedToGroup());
        assertFalse(directoryServiceGroup.checkHasStatus());

        assertEquals("", directoryServiceGroup.group().toString());
        assertEquals(FilterAction.SET, directoryServiceGroup.action());
        assertEquals(EmaRdm.SERVICE_GROUP_ID, directoryServiceGroup.filterId());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenStatusNotSet_whenGetStatus_thenThrowException()
    {
        directoryServiceGroup.status();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsStatus_whenSetStatus_thenThrowException()
    {
        directoryServiceGroup.status(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenMergedToGroupNotSet_whenGetMergedToGroup_thenThrowException()
    {
        directoryServiceGroup.mergedToGroup();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsMergedToGroup_whenSetMergedToGroup_thenThrowException()
    {
        directoryServiceGroup.mergedToGroup(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsGroup_whenSetGroup_thenThrowException()
    {
        directoryServiceGroup.group(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidActionValue_whenSetAction_thenThrowException()
    {
        directoryServiceGroup.action(99);
    }

    @Test
    public void givenStatusFields_whenSetStatusFieldsSeparately_thenGetCorrectStatus()
    {
        assertFalse(directoryServiceGroup.checkHasStatus());

        directoryServiceGroup.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");

        assertTrue(directoryServiceGroup.checkHasStatus());

        OmmState status = directoryServiceGroup.status();
        assertEquals(StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(DataState.SUSPECT, status.dataState());
        assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test
    public void givenOmmState_whenSetStatusAsOneObject_thenGetCorrectStatus()
    {
        assertFalse(directoryServiceGroup.checkHasStatus());

        OmmStateImpl statusBefore = new OmmStateImpl();
        State rsslState = CodecFactory.createState();
        rsslState.streamState(StreamState.CLOSED_RECOVER);
        rsslState.dataState(DataState.SUSPECT);
        rsslState.code(StatusCode.SOURCE_UNKNOWN);
        Buffer stateText = CodecFactory.createBuffer();
        stateText.data("Test message");
        rsslState.text(stateText);
        statusBefore.decode(rsslState);

        directoryServiceGroup.status(statusBefore);

        assertTrue(directoryServiceGroup.checkHasStatus());

        OmmState status = directoryServiceGroup.status();
        assertEquals(StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(DataState.SUSPECT, status.dataState());
        assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceGroup_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryServiceGroup.copy(null);
    }

    @Test
    public void givenServiceGroup_whenCopyOtherServiceGroup_thenReturnTrueAndUpdateFields()
    {
        DirectoryServiceGroup otherDirectoryServiceGroup = EmaFactory.Domain.createDirectoryServiceGroup();
        buffer.data("1.26.102");
        otherDirectoryServiceGroup.group(buffer.data());
        buffer.clear();
        buffer.data("1.26.110");
        otherDirectoryServiceGroup.mergedToGroup(buffer.data());
        otherDirectoryServiceGroup.action(FilterAction.CLEAR);
        otherDirectoryServiceGroup.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");

        directoryServiceGroup.copy(otherDirectoryServiceGroup);

        assertTrue(directoryServiceGroup.checkHasMergedToGroup());

        buffer.clear();
        buffer.data("1.26.102");
        assertEquals(buffer.data(), directoryServiceGroup.group().buffer());
        buffer.clear();
        buffer.data("1.26.110");
        assertEquals(buffer.data(), directoryServiceGroup.mergedToGroup().buffer());
        assertEquals(FilterAction.CLEAR, directoryServiceGroup.action());
        assertEquals(EmaRdm.SERVICE_GROUP_ID, directoryServiceGroup.filterId());

        assertTrue(directoryServiceGroup.checkHasStatus());
        OmmState status = directoryServiceGroup.status();
        assertEquals(StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(DataState.SUSPECT, status.dataState());
        assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    @Test
    public void givenServiceGroupWithOptionalFields_whenCopyFromSourceWithoutOptionals_thenClearDestinationOptionals()
    {
        buffer.data("1.26.200");
        directoryServiceGroup.group(buffer.data());
        buffer.clear();
        buffer.data("1.26.210");
        directoryServiceGroup.mergedToGroup(buffer.data());
        directoryServiceGroup.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Existing status");

        DirectoryServiceGroup otherDirectoryServiceGroup = EmaFactory.Domain.createDirectoryServiceGroup();
        buffer.clear();
        buffer.data("1.26.102");
        otherDirectoryServiceGroup.group(buffer.data());
        otherDirectoryServiceGroup.action(FilterAction.CLEAR);

        directoryServiceGroup.copy(otherDirectoryServiceGroup);

        assertEquals(FilterAction.CLEAR, directoryServiceGroup.action());
        buffer.clear();
        buffer.data("1.26.102");
        assertEquals(buffer.data(), directoryServiceGroup.group().buffer());
        assertFalse(directoryServiceGroup.checkHasMergedToGroup());
        assertFalse(directoryServiceGroup.checkHasStatus());
    }

    @Test
    public void givenServiceGroup_whenCopyIntoSelf_thenPreserveFields()
    {
        buffer.data("1.26.102");
        directoryServiceGroup.group(buffer.data());
        buffer.clear();
        buffer.data("1.26.110");
        directoryServiceGroup.mergedToGroup(buffer.data());
        directoryServiceGroup.action(FilterAction.CLEAR);
        directoryServiceGroup.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");

        directoryServiceGroup.copy(directoryServiceGroup);

        assertTrue(directoryServiceGroup.checkHasMergedToGroup());
        assertTrue(directoryServiceGroup.checkHasStatus());
        assertEquals(FilterAction.CLEAR, directoryServiceGroup.action());
        buffer.clear();
        buffer.data("1.26.102");
        assertEquals(buffer.data(), directoryServiceGroup.group().buffer());
        buffer.clear();
        buffer.data("1.26.110");
        assertEquals(buffer.data(), directoryServiceGroup.mergedToGroup().buffer());
        assertEquals("Test message", directoryServiceGroup.status().statusText());
    }

    @Test
    public void givenInvalidDataState_whenSetStatusFieldsSeparately_thenThrowExceptionAndLeaveStatusAbsent()
    {
        assertFalse(directoryServiceGroup.checkHasStatus());

        try
        {
            directoryServiceGroup.status(StreamState.OPEN, 99, StatusCode.NONE, "Test message");
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertFalse(directoryServiceGroup.checkHasStatus());
        }
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceGroup_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryServiceGroup.decode(null);
    }

    @Test
    public void givenServiceGroup_whenDecodeWithMalformedGroupElement_thenClearAndThrowException()
    {
        buffer.data("1.26.200");
        directoryServiceGroup.group(buffer.data());
        buffer.clear();
        buffer.data("1.26.210");
        directoryServiceGroup.mergedToGroup(buffer.data());
        directoryServiceGroup.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Existing status");

        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_GROUP, "badType"));

        try
        {
            directoryServiceGroup.decode(createDecodedElementList(encElementList));
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertEquals(FilterAction.SET, directoryServiceGroup.action());
            assertEquals("", directoryServiceGroup.group().toString());
            assertFalse(directoryServiceGroup.checkHasMergedToGroup());
            assertFalse(directoryServiceGroup.checkHasStatus());
        }
    }

    @Test
    public void givenServiceGroup_whenDecodeWithUnknownElement_thenIgnoreUnknownElement()
    {
        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_GROUP,
                java.nio.ByteBuffer.wrap("1.26.102".getBytes())));
        encElementList.add(EmaFactory.createElementEntry().ascii("UNKNOWN_ELEMENT", "ignored"));

        DirectoryServiceGroup otherDirectoryServiceGroup = EmaFactory.Domain.createDirectoryServiceGroup();
        otherDirectoryServiceGroup.decode(createDecodedElementList(encElementList));

        buffer.data("1.26.102");
        assertEquals(buffer.data(), otherDirectoryServiceGroup.group().buffer());
        assertFalse(otherDirectoryServiceGroup.checkHasMergedToGroup());
        assertFalse(otherDirectoryServiceGroup.checkHasStatus());
    }

    @Test
    public void givenServiceGroup_whenDecodeWithBlankMergedToGroupAndStatus_thenLeaveOptionalsAbsent()
    {
        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().buffer(EmaRdm.ENAME_GROUP,
                java.nio.ByteBuffer.wrap("1.26.102".getBytes())));
        encElementList.add(EmaFactory.createElementEntry().codeBuffer(EmaRdm.ENAME_MERG_TO_GRP));
        encElementList.add(EmaFactory.createElementEntry().codeState(EmaRdm.ENAME_STATUS));

        directoryServiceGroup.decode(createDecodedElementList(encElementList));

        buffer.data("1.26.102");
        assertEquals(buffer.data(), directoryServiceGroup.group().buffer());
        assertFalse(directoryServiceGroup.checkHasMergedToGroup());
        assertFalse(directoryServiceGroup.checkHasStatus());
    }

    @Test
    public void givenServiceGroup_whenDecodeWithoutGroupElement_thenClearAndThrowException()
    {
        buffer.data("1.26.200");
        directoryServiceGroup.group(buffer.data());
        buffer.clear();
        buffer.data("1.26.210");
        directoryServiceGroup.mergedToGroup(buffer.data());
        directoryServiceGroup.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Existing status");

        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().codeState(EmaRdm.ENAME_STATUS));

        try
        {
            directoryServiceGroup.decode(createDecodedElementList(encElementList));
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertEquals(FilterAction.SET, directoryServiceGroup.action());
            assertEquals("", directoryServiceGroup.group().toString());
            assertFalse(directoryServiceGroup.checkHasMergedToGroup());
            assertFalse(directoryServiceGroup.checkHasStatus());
        }
    }

    @Test
    public void givenServiceGroup_whenEncodeAndThenDecodeIntoOtherServiceGroup_thenFillOtherServiceGroupFields()
    {
        // encode
        buffer.data("1.26.102");
        directoryServiceGroup.group(buffer.data());
        buffer.clear();
        buffer.data("1.26.110");
        directoryServiceGroup.mergedToGroup(buffer.data());
        directoryServiceGroup.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");

        ElementList encElementList = directoryServiceGroup.encode();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        ElementList decElementList = JUnitTestConnect.createElementList();

        JUnitTestConnect.setRsslData(decElementList, encElementList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        // decode
        DirectoryServiceGroup otherDirectoryServiceGroup = EmaFactory.Domain.createDirectoryServiceGroup();
        otherDirectoryServiceGroup.decode(decElementList);

        assertTrue(directoryServiceGroup.checkHasMergedToGroup());
        buffer.clear();
        buffer.data("1.26.102");
        assertEquals(buffer.data(), directoryServiceGroup.group().buffer());
        buffer.clear();
        buffer.data("1.26.110");
        assertEquals(buffer.data(), directoryServiceGroup.mergedToGroup().buffer());

        assertTrue(otherDirectoryServiceGroup.checkHasStatus());
        OmmState status = otherDirectoryServiceGroup.status();
        assertEquals(StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(DataState.SUSPECT, status.dataState());
        assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    private ElementList createDecodedElementList(ElementList encElementList)
    {
        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        ElementList decElementList = JUnitTestConnect.createElementList();
        JUnitTestConnect.setRsslData(decElementList, encElementList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl) dictionary).rsslDataDictionary(), null);

        return decElementList;
    }
}