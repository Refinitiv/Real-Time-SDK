/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceData;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.*;
import org.junit.Before;
import org.junit.Test;
import org.mockito.MockitoAnnotations;
import org.mockito.Spy;

import java.nio.ByteBuffer;
import java.time.LocalDate;
import java.time.LocalDateTime;
import java.time.LocalTime;
import java.util.Iterator;
import java.util.function.Consumer;
import java.util.function.Supplier;

import static com.refinitiv.ema.access.FilterEntry.*;
import static com.refinitiv.ema.access.OmmReal.*;
import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

public class DirectoryServiceDataTest
{
    @Spy
    DirectoryServiceData directoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

    @Before
    public void setUp()
    {
        MockitoAnnotations.openMocks(this);
    }

    @Test
    public void givenServiceData_whenClear_thenClearAllFields()
    {
        Map map = EmaFactory.createMap();
        directoryServiceData.dataAsComplexType(map);
        directoryServiceData.type(EmaRdm.DataTypes.STATUS);
        directoryServiceData.action(FilterAction.UPDATE);

        assertTrue(directoryServiceData.checkHasData());

        directoryServiceData.clear();

        assertFalse(directoryServiceData.checkHasData());

        assertEquals(FilterAction.SET, directoryServiceData.action());
        assertEquals(EmaRdm.SERVICE_DATA_ID, directoryServiceData.filterId());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDataNotSet_whenGetData_thenThrowException()
    {
        directoryServiceData.data();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsData_whenSetDataAsComplexType_thenThrowException()
    {
        directoryServiceData.dataAsComplexType(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNonInternalComplexType_whenSetDataAsComplexType_thenThrowControlledException()
    {
        ComplexType data = mock(ComplexType.class);
        when(data.dataType()).thenReturn(DataType.DataTypes.FIELD_LIST);

        directoryServiceData.dataAsComplexType(data);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsArray_whenSetDataAsArray_thenThrowException()
    {
        directoryServiceData.dataAsArray(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNonInternalArray_whenSetDataAsArray_thenThrowControlledException()
    {
        OmmArray array = mock(OmmArray.class);

        directoryServiceData.dataAsArray(array);
    }

    @Test
    public void givenArray_whenSetDataAsArray_thenStoreDecodedArray()
    {
        OmmArray array = EmaFactory.createOmmArray();
        array.fixedWidth(2);
        array.add(EmaFactory.createOmmArrayEntry().intValue(22));
        array.add(EmaFactory.createOmmArrayEntry().intValue(25));

        directoryServiceData.dataAsArray(array);

        assertEquals(DataType.DataTypes.ARRAY, directoryServiceData.dataType());

        Iterator<OmmArrayEntry> iterator = ((OmmArray) directoryServiceData.data()).iterator();
        assertNotNull(iterator);
        assertTrue(iterator.hasNext());
        assertEquals(22, iterator.next().intValue());
        assertTrue(iterator.hasNext());
        assertEquals(25, iterator.next().intValue());
        assertFalse(iterator.hasNext());
    }

    @Test
    public void givenComplexTypeInputs_whenSetDataAsComplexType_thenStorePayloadWithExpectedType()
    {
        for (ComplexTypeTestCase testCase : complexTypeTestCases())
        {
            directoryServiceData.clear();

            directoryServiceData.dataAsComplexType(testCase.factory.get());

            assertTrue("Expected payload for " + testCase.name, directoryServiceData.checkHasData());
            assertEquals("Unexpected payload data type for " + testCase.name,
                    testCase.expectedDataType, directoryServiceData.dataType());
            assertNotNull(directoryServiceData.data());
            assertEquals("Unexpected stored payload type for " + testCase.name,
                    testCase.expectedDataType, directoryServiceData.data().dataType());
        }
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsData_whenSetDataAsBuffer_thenThrowException()
    {
        directoryServiceData.dataAsBuffer(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsData_whenSetDataAsAscii_thenThrowException()
    {
        directoryServiceData.dataAsAscii(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsData_whenSetDataAsUtf8_thenThrowException()
    {
        directoryServiceData.dataAsUtf8(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsData_whenSetDataAsRmtes_thenThrowException()
    {
        directoryServiceData.dataAsRmtes(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenMagnitudeTypeLessThanMinimum_whenSetDataAsReal_thenThrowException()
    {
        directoryServiceData.dataAsReal(65535, -1);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenMagnitudeTypeGreaterThanMaximum_whenSetDataAsReal_thenThrowException()
    {
        directoryServiceData.dataAsReal(65535, 36);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsData_whenSetDataAsDate_thenThrowException()
    {
        directoryServiceData.dataAsDate(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsData_whenSetDataAsTime_thenThrowException()
    {
        directoryServiceData.dataAsTime(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsData_whenSetDataAsDateTime_thenThrowException()
    {
        directoryServiceData.dataAsDateTime(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDataNotSet_whenGetDataType_thenThrowException()
    {
        directoryServiceData.dataType();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenUnsupportedType_whenSetType_thenThrowException()
    {
        directoryServiceData.type(1024);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidAction_whenSetAction_thenThrowException()
    {
        directoryServiceData.action(0);
    }

    @Test
    public void givenFilterActions_whenSetAction_thenStoreAction()
    {
        directoryServiceData.action(FilterAction.SET);
        assertEquals(FilterAction.SET, directoryServiceData.action());

        directoryServiceData.action(FilterAction.UPDATE);
        assertEquals(FilterAction.UPDATE, directoryServiceData.action());

        directoryServiceData.action(FilterAction.CLEAR);
        assertEquals(FilterAction.CLEAR, directoryServiceData.action());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidStreamState_whenSetDataAsState_thenThrowException()
    {
        directoryServiceData.dataAsState(-1, OmmState.DataState.OK, OmmState.StatusCode.NONE, "Test message");
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidDataState_whenSetDataAsState_thenThrowException()
    {
        directoryServiceData.dataAsState(OmmState.StreamState.OPEN, -1, OmmState.StatusCode.NONE, "Test message");
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidStatusCode_whenSetDataAsState_thenThrowException()
    {
        directoryServiceData.dataAsState(OmmState.StreamState.OPEN, OmmState.DataState.OK, -1, "Test message");
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceData_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryServiceData.copy(null);
    }

    @Test
    public void givenServiceData_whenCopyOtherServiceData_thenReturnTrueAndUpdateFields()
    {
        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();
        // encode
        Map map = EmaFactory.createMap();
        MapEntry mapEntry = EmaFactory.createMapEntry();
        ElementList elementList = EmaFactory.createElementList();
        ElementEntry elementEntry = EmaFactory.createElementEntry();
        elementEntry.uintValue("Test name", 101L);
        elementList.add(elementEntry);
        mapEntry.keyUInt(1, MapEntry.MapAction.ADD, elementList);
        map.add(mapEntry);
        otherDirectoryServiceData
                .dataAsComplexType(map)
                .type(EmaRdm.DataTypes.STATUS)
                .action(FilterAction.UPDATE);

        assertTrue(otherDirectoryServiceData.checkHasData());

        directoryServiceData.copy(otherDirectoryServiceData);
        verify(directoryServiceData, times(1)).clear();

        assertTrue(directoryServiceData.checkHasData());

        assertEquals(DataType.DataTypes.MAP, directoryServiceData.dataType());

        // decode
        map = (Map) directoryServiceData.data();
        Map decMap = JUnitTestConnect.createMap();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        JUnitTestConnect.setRsslData(decMap, map, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        for (MapEntry mEntry : decMap)
        {
            if (mEntry.loadType() == DataType.DataTypes.ELEMENT_LIST)
            {
                assertEquals(1, mEntry.key().uintValue());
                assertEquals(MapEntry.MapAction.ADD, mEntry.action());
                ElementList eList = mEntry.elementList();
                for (ElementEntry eEntry : eList)
                {
                    assertEquals(DataType.DataTypes.UINT, eEntry.loadType());
                    assertEquals(101L, eEntry.uintValue());
                }
            }
        }
        assertEquals(FilterAction.UPDATE, directoryServiceData.action());
        assertEquals(EmaRdm.DataTypes.STATUS, directoryServiceData.type());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceData_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryServiceData.decode(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDecodedElementListWithInvalidType_whenDecode_thenThrowException()
    {
        directoryServiceData.dataAsAscii("Existing value");
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);
        directoryServiceData.action(FilterAction.UPDATE);

        ElementList malformedElementList = EmaFactory.createElementList();
        malformedElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TYPE, 2048));
        malformedElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_DATA, "Test value"));

        directoryServiceData.decode(decodeData(malformedElementList));
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDecodedElementListWithTypeButNoData_whenDecode_thenThrowException()
    {
        directoryServiceData.dataAsAscii("Existing value");
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);
        directoryServiceData.action(FilterAction.UPDATE);

        ElementList malformedElementList = EmaFactory.createElementList();
        malformedElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TYPE, EmaRdm.DataTypes.STATUS));

        directoryServiceData.decode(decodeData(malformedElementList));
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDecodedElementListWithDataButNoType_whenDecode_thenThrowException()
    {
        directoryServiceData.dataAsAscii("Existing value");
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);
        directoryServiceData.action(FilterAction.UPDATE);

        ElementList malformedElementList = EmaFactory.createElementList();
        malformedElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_DATA, "Test value"));

        directoryServiceData.decode(decodeData(malformedElementList));
    }

    @Test
    public void givenServiceDataWithInt_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsInt(15);
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.INT, otherDirectoryServiceData.dataType());
        assertEquals(15, ((OmmInt) otherDirectoryServiceData.data()).intValue());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithAscii_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsAscii("Test string");
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.ASCII, otherDirectoryServiceData.dataType());
        assertEquals("Test string", ((OmmAscii) otherDirectoryServiceData.data()).ascii());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithUtf8_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsUtf8("Test string");
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.UTF8, otherDirectoryServiceData.dataType());
        assertEquals("Test string", ((OmmUtf8) otherDirectoryServiceData.data()).string());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithRmtes_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // prepare RmtesBuffer
        RmtesBuffer inputRmtesBuf = EmaFactory.createRmtesBuffer();
        ByteBuffer inputByteBuf = ByteBuffer.allocate(30);
        // "Waiting for LBM..."
        byte[] inputByte = { 0x57, 0x61, 0x69, 0x74, 0x69, 0x6E, 0x67, 0x20, 0x66, 0x6F, 0x72, 0x20, 0x4C, 0x42, 0x4D,
                0x2E, 0x2E, 0x2E };

        inputByteBuf.put(inputByte);
        inputByteBuf.flip();
        JUnitTestConnect.setRsslData(inputRmtesBuf, inputByteBuf);

        RmtesBuffer outputRmtesBuf = EmaFactory.createRmtesBuffer();
        outputRmtesBuf.apply(inputRmtesBuf);

        // encode
        directoryServiceData.dataAsRmtes(outputRmtesBuf);
        directoryServiceData.type(EmaRdm.DataTypes.STATUS);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();
        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.RMTES, otherDirectoryServiceData.dataType());
        assertEquals("Waiting for LBM...", otherDirectoryServiceData.data().toString());

        assertEquals(EmaRdm.DataTypes.STATUS, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithReal_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsReal(68719476735L, MagnitudeType.EXPONENT_NEG_2);
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.REAL, otherDirectoryServiceData.dataType());
        assertEquals(68719476735L, ((OmmReal) otherDirectoryServiceData.data()).mantissa());
        assertEquals(MagnitudeType.EXPONENT_NEG_2, ((OmmReal) otherDirectoryServiceData.data()).magnitudeType());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithDate1_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsDate(1990, 10,22);
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.DATE, otherDirectoryServiceData.dataType());
        assertEquals(1990, ((OmmDate) otherDirectoryServiceData.data()).year());
        assertEquals(10, ((OmmDate) otherDirectoryServiceData.data()).month());
        assertEquals(22, ((OmmDate) otherDirectoryServiceData.data()).day());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithDate2_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsDate(LocalDate.of(1990, 10, 22));
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.DATE, otherDirectoryServiceData.dataType());
        assertEquals(1990, ((OmmDate) otherDirectoryServiceData.data()).year());
        assertEquals(10, ((OmmDate) otherDirectoryServiceData.data()).month());
        assertEquals(22, ((OmmDate) otherDirectoryServiceData.data()).day());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithTime1_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsTime(6, 5,4,3,2,1);
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.TIME, otherDirectoryServiceData.dataType());
        assertEquals(6, ((OmmTime) otherDirectoryServiceData.data()).hour());
        assertEquals(5, ((OmmTime) otherDirectoryServiceData.data()).minute());
        assertEquals(4, ((OmmTime) otherDirectoryServiceData.data()).second());
        assertEquals(3, ((OmmTime) otherDirectoryServiceData.data()).millisecond());
        assertEquals(2, ((OmmTime) otherDirectoryServiceData.data()).microsecond());
        assertEquals(1, ((OmmTime) otherDirectoryServiceData.data()).nanosecond());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithTime2_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsTime(LocalTime.of(6, 5, 4, 3_002_001));
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.TIME, otherDirectoryServiceData.dataType());
        assertEquals(6, ((OmmTime) otherDirectoryServiceData.data()).hour());
        assertEquals(5, ((OmmTime) otherDirectoryServiceData.data()).minute());
        assertEquals(4, ((OmmTime) otherDirectoryServiceData.data()).second());
        assertEquals(3, ((OmmTime) otherDirectoryServiceData.data()).millisecond());
        assertEquals(2, ((OmmTime) otherDirectoryServiceData.data()).microsecond());
        assertEquals(1, ((OmmTime) otherDirectoryServiceData.data()).nanosecond());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithDateTime1_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsDateTime(1990, 10,22,
                6, 5,4,3,2,1);
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.DATETIME, otherDirectoryServiceData.dataType());
        assertEquals(1990, ((OmmDateTime) otherDirectoryServiceData.data()).year());
        assertEquals(10, ((OmmDateTime) otherDirectoryServiceData.data()).month());
        assertEquals(22, ((OmmDateTime) otherDirectoryServiceData.data()).day());
        assertEquals(6, ((OmmDateTime) otherDirectoryServiceData.data()).hour());
        assertEquals(5, ((OmmDateTime) otherDirectoryServiceData.data()).minute());
        assertEquals(4, ((OmmDateTime) otherDirectoryServiceData.data()).second());
        assertEquals(3, ((OmmDateTime) otherDirectoryServiceData.data()).millisecond());
        assertEquals(2, ((OmmDateTime) otherDirectoryServiceData.data()).microsecond());
        assertEquals(1, ((OmmDateTime) otherDirectoryServiceData.data()).nanosecond());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithDateTime2_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsDateTime(LocalDateTime.of(1990, 10, 22,
                6, 5, 4, 333_222_111));
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.DATETIME, otherDirectoryServiceData.dataType());
        assertEquals(1990, ((OmmDateTime) otherDirectoryServiceData.data()).year());
        assertEquals(10, ((OmmDateTime) otherDirectoryServiceData.data()).month());
        assertEquals(22, ((OmmDateTime) otherDirectoryServiceData.data()).day());
        assertEquals(6, ((OmmDateTime) otherDirectoryServiceData.data()).hour());
        assertEquals(5, ((OmmDateTime) otherDirectoryServiceData.data()).minute());
        assertEquals(4, ((OmmDateTime) otherDirectoryServiceData.data()).second());
        assertEquals(333, ((OmmDateTime) otherDirectoryServiceData.data()).millisecond());
        assertEquals(222, ((OmmDateTime) otherDirectoryServiceData.data()).microsecond());
        assertEquals(111, ((OmmDateTime) otherDirectoryServiceData.data()).nanosecond());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithQos_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsQos(101, 102);
        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.QOS, otherDirectoryServiceData.dataType());
        assertEquals(102, ((OmmQos) otherDirectoryServiceData.data()).rate());
        assertEquals(101, ((OmmQos) otherDirectoryServiceData.data()).timeliness());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithState_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsState(OmmState.StreamState.CLOSED_RECOVER, OmmState.DataState.SUSPECT,
                OmmState.StatusCode.SOURCE_UNKNOWN, "Test message");

        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.STATE, otherDirectoryServiceData.dataType());
        assertEquals(OmmState.StreamState.CLOSED_RECOVER, ((OmmState) otherDirectoryServiceData.data()).streamState());
        assertEquals(OmmState.DataState.SUSPECT, ((OmmState) otherDirectoryServiceData.data()).dataState());
        assertEquals(OmmState.StatusCode.SOURCE_UNKNOWN, ((OmmState) otherDirectoryServiceData.data()).statusCode());
        assertEquals("Test message", ((OmmState) otherDirectoryServiceData.data()).statusText());

        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithEnum_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        directoryServiceData.dataAsEnum(15);
        directoryServiceData.type(EmaRdm.DataTypes.STATUS);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.ENUM, otherDirectoryServiceData.dataType());
        assertEquals(15, ((OmmEnum) otherDirectoryServiceData.data()).enumValue());

        assertEquals(EmaRdm.DataTypes.STATUS, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithArray_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        // encode
        OmmArray array = EmaFactory.createOmmArray();
        array.fixedWidth(2);
        array.add(EmaFactory.createOmmArrayEntry().intValue(22));
        array.add(EmaFactory.createOmmArrayEntry().intValue(25));
        directoryServiceData.dataAsArray(array);
        directoryServiceData.type(EmaRdm.DataTypes.STATUS);

        ElementList encElementList = directoryServiceData.encode();

        // decode
        ElementList decElementList = decodeData(encElementList);

        DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();

        otherDirectoryServiceData.decode(decElementList);

        assertEquals(DataType.DataTypes.ARRAY, otherDirectoryServiceData.dataType());
        Iterator<OmmArrayEntry> iterator = ((OmmArray) otherDirectoryServiceData.data()).iterator();
        assertNotNull(iterator);
        assertTrue(iterator.hasNext());
        assertEquals(22, iterator.next().intValue());
        assertTrue(iterator.hasNext());
        assertEquals(25, iterator.next().intValue());
        assertFalse(iterator.hasNext());

        assertEquals(EmaRdm.DataTypes.STATUS, otherDirectoryServiceData.type());
    }

    @Test
    public void givenServiceDataWithComplexTypeInputs_whenEncodeAndThenDecodeIntoOtherServiceData_thenFillOtherServiceDataFields()
    {
        for (ComplexTypeTestCase testCase : complexTypeTestCases())
        {
            directoryServiceData.clear();

            directoryServiceData.dataAsComplexType(testCase.factory.get());
            directoryServiceData.type(EmaRdm.DataTypes.STATUS);

            ElementList encElementList = directoryServiceData.encode();
            ElementList decElementList = decodeData(encElementList);

            DirectoryServiceData otherDirectoryServiceData = EmaFactory.Domain.createDirectoryServiceData();
            otherDirectoryServiceData.decode(decElementList);

            assertEquals("Unexpected payload data type for " + testCase.name,
                    testCase.expectedDataType, otherDirectoryServiceData.dataType());
            testCase.verifier.accept(materializeComplexTypeData(otherDirectoryServiceData.data(),
                    testCase.expectedDataType));
            assertEquals(EmaRdm.DataTypes.STATUS, otherDirectoryServiceData.type());
        }
    }

    private ComplexTypeTestCase[] complexTypeTestCases()
    {
        return new ComplexTypeTestCase[] {
                new ComplexTypeTestCase("field list", DataType.DataTypes.FIELD_LIST,
                        this::createTestFieldList, this::assertTestFieldList),
                new ComplexTypeTestCase("element list", DataType.DataTypes.ELEMENT_LIST,
                        this::createTestElementList, this::assertTestElementList),
                new ComplexTypeTestCase("map", DataType.DataTypes.MAP,
                        this::createTestMap, this::assertTestMap),
                new ComplexTypeTestCase("filter list", DataType.DataTypes.FILTER_LIST,
                        this::createTestFilterList, this::assertTestFilterList),
                new ComplexTypeTestCase("vector", DataType.DataTypes.VECTOR,
                        this::createTestVector, this::assertTestVector),
                new ComplexTypeTestCase("series", DataType.DataTypes.SERIES,
                        this::createTestSeries, this::assertTestSeries),
                new ComplexTypeTestCase("opaque", DataType.DataTypes.OPAQUE,
                        this::createTestOpaque, this::assertTestOpaque),
                new ComplexTypeTestCase("xml", DataType.DataTypes.XML,
                        this::createTestXml, this::assertTestXml),
                new ComplexTypeTestCase("json", DataType.DataTypes.JSON,
                        this::createTestJson, this::assertTestJson),
                new ComplexTypeTestCase("request message", DataType.DataTypes.REQ_MSG,
                        this::createTestReqMsg, this::assertTestReqMsg),
                new ComplexTypeTestCase("refresh message", DataType.DataTypes.REFRESH_MSG,
                        this::createTestRefreshMsg, this::assertTestRefreshMsg),
                new ComplexTypeTestCase("update message", DataType.DataTypes.UPDATE_MSG,
                        this::createTestUpdateMsg, this::assertTestUpdateMsg)
        };
    }

    private FieldList createTestFieldList()
    {
        FieldList fieldList = EmaFactory.createFieldList();
        fieldList.add(EmaFactory.createFieldEntry().uintValue(1, 5));

        FieldList decodedFieldList = JUnitTestConnect.createFieldList();
        JUnitTestConnect.setRsslData(decodedFieldList, fieldList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl) loadTestDictionary()).rsslDataDictionary(), null);
        return decodedFieldList;
    }

    private void assertTestFieldList(Data data)
    {
        FieldList fieldList = (FieldList) data;
        Iterator<FieldEntry> iterator = fieldList.iterator();

        assertNotNull(iterator);
        assertTrue(iterator.hasNext());
        FieldEntry fieldEntry = iterator.next();
        assertEquals(1, fieldEntry.fieldId());
        assertEquals(DataType.DataTypes.UINT, fieldEntry.loadType());
        assertEquals(5, fieldEntry.uintValue());
        assertFalse(iterator.hasNext());
    }

    private ElementList createTestElementList()
    {
        ElementList elementList = EmaFactory.createElementList();
        elementList.add(EmaFactory.createElementEntry().uintValue("First", 101L));
        elementList.add(EmaFactory.createElementEntry().ascii("Second", "Test value"));
        return elementList;
    }

    private void assertTestElementList(Data data)
    {
        ElementList elementList = (ElementList) data;
        Iterator<ElementEntry> iterator = elementList.iterator();

        assertNotNull(iterator);
        assertTrue(iterator.hasNext());
        ElementEntry firstEntry = iterator.next();
        assertEquals("First", firstEntry.name());
        assertEquals(DataType.DataTypes.UINT, firstEntry.loadType());
        assertEquals(101L, firstEntry.uintValue());

        assertTrue(iterator.hasNext());
        ElementEntry secondEntry = iterator.next();
        assertEquals("Second", secondEntry.name());
        assertEquals(DataType.DataTypes.ASCII, secondEntry.loadType());
        assertEquals("Test value", secondEntry.ascii().ascii());

        assertFalse(iterator.hasNext());
    }

    private Map createTestMap()
    {
        Map map = EmaFactory.createMap();
        ElementList payload = EmaFactory.createElementList();
        payload.add(EmaFactory.createElementEntry().ascii("Key", "Value"));
        map.add(EmaFactory.createMapEntry().keyUInt(7, MapEntry.MapAction.ADD, payload));
        return map;
    }

    private void assertTestMap(Data data)
    {
        Map map = (Map) data;
        Iterator<MapEntry> iterator = map.iterator();

        assertNotNull(iterator);
        assertTrue(iterator.hasNext());
        MapEntry mapEntry = iterator.next();
        assertEquals(7, mapEntry.key().uintValue());
        assertEquals(MapEntry.MapAction.ADD, mapEntry.action());

        ElementList payload = mapEntry.elementList();
        Iterator<ElementEntry> payloadIterator = payload.iterator();
        assertTrue(payloadIterator.hasNext());
        ElementEntry payloadEntry = payloadIterator.next();
        assertEquals("Key", payloadEntry.name());
        assertEquals("Value", payloadEntry.ascii().ascii());
        assertFalse(payloadIterator.hasNext());
        assertFalse(iterator.hasNext());
    }

    private FilterList createTestFilterList()
    {
        FilterList filterList = EmaFactory.createFilterList();
        filterList.add(EmaFactory.createFilterEntry().noData(5, FilterEntry.FilterAction.UPDATE));
        return filterList;
    }

    private void assertTestFilterList(Data data)
    {
        FilterList filterList = (FilterList) data;
        Iterator<FilterEntry> iterator = filterList.iterator();

        assertNotNull(iterator);
        assertTrue(iterator.hasNext());
        FilterEntry filterEntry = iterator.next();
        assertEquals(5, filterEntry.filterId());
        assertEquals(FilterEntry.FilterAction.UPDATE, filterEntry.action());
        assertEquals(DataType.DataTypes.NO_DATA, filterEntry.loadType());
        assertFalse(iterator.hasNext());
    }

    private Vector createTestVector()
    {
        Vector vector = EmaFactory.createVector();
        vector.add(EmaFactory.createVectorEntry().noData(2, VectorEntry.VectorAction.SET));
        return vector;
    }

    private void assertTestVector(Data data)
    {
        Vector vector = (Vector) data;
        Iterator<VectorEntry> iterator = vector.iterator();

        assertNotNull(iterator);
        assertTrue(iterator.hasNext());
        VectorEntry vectorEntry = iterator.next();
        assertEquals(2, vectorEntry.position());
        assertEquals(VectorEntry.VectorAction.SET, vectorEntry.action());
        assertEquals(DataType.DataTypes.NO_DATA, vectorEntry.loadType());
        assertFalse(iterator.hasNext());
    }

    private Series createTestSeries()
    {
        Series series = EmaFactory.createSeries();
        series.add(EmaFactory.createSeriesEntry().noData());
        return series;
    }

    private void assertTestSeries(Data data)
    {
        Series series = (Series) data;
        Iterator<SeriesEntry> iterator = series.iterator();

        assertNotNull(iterator);
        assertTrue(iterator.hasNext());
        SeriesEntry seriesEntry = iterator.next();
        assertEquals(DataType.DataTypes.NO_DATA, seriesEntry.loadType());
        assertFalse(iterator.hasNext());
    }

    private OmmOpaque createTestOpaque()
    {
        OmmOpaque opaque = EmaFactory.createOmmOpaque();
        opaque.string("opaque-value");
        return opaque;
    }

    private void assertTestOpaque(Data data)
    {
        assertEquals("opaque-value", ((OmmOpaque) data).string());
    }

    private OmmXml createTestXml()
    {
        OmmXml xml = EmaFactory.createOmmXml();
        xml.string("<root>value</root>");
        return xml;
    }

    private void assertTestXml(Data data)
    {
        assertEquals("<root>value</root>", ((OmmXml) data).string());
    }

    private OmmJson createTestJson()
    {
        OmmJson json = EmaFactory.createOmmJson();
        json.string("{\"key\":\"value\"}");
        return json;
    }

    private void assertTestJson(Data data)
    {
        assertEquals("{\"key\":\"value\"}", ((OmmJson) data).string());
    }

    private ReqMsg createTestReqMsg()
    {
        return EmaFactory.createReqMsg()
                .serviceId(7)
                .name("TRI.N")
                .domainType(EmaRdm.MMT_MARKET_PRICE)
                .interestAfterRefresh(true);
    }

    private void assertTestReqMsg(Data data)
    {
        ReqMsg reqMsg = (ReqMsg) data;
        assertEquals("TRI.N", reqMsg.name());
        assertEquals(7, reqMsg.serviceId());
        assertEquals(EmaRdm.MMT_MARKET_PRICE, reqMsg.domainType());
        assertTrue(reqMsg.interestAfterRefresh());
    }

    private RefreshMsg createTestRefreshMsg()
    {
        return EmaFactory.createRefreshMsg()
                .streamId(11)
                .serviceId(3)
                .name("REFRESH.RIC")
                .domainType(EmaRdm.MMT_MARKET_PRICE)
                .state(OmmState.StreamState.OPEN, OmmState.DataState.OK, OmmState.StatusCode.NONE, "refresh complete");
    }

    private void assertTestRefreshMsg(Data data)
    {
        RefreshMsg refreshMsg = (RefreshMsg) data;
        assertEquals(11, refreshMsg.streamId());
        assertEquals("REFRESH.RIC", refreshMsg.name());
        assertEquals(3, refreshMsg.serviceId());
        assertEquals(EmaRdm.MMT_MARKET_PRICE, refreshMsg.domainType());
        assertEquals(OmmState.StreamState.OPEN, refreshMsg.state().streamState());
        assertEquals("refresh complete", refreshMsg.state().statusText());
    }

    private UpdateMsg createTestUpdateMsg()
    {
        return EmaFactory.createUpdateMsg()
                .streamId(12)
                .seqNum(99)
                .serviceId(4)
                .name("UPDATE.RIC")
                .domainType(EmaRdm.MMT_MARKET_PRICE);
    }

    private void assertTestUpdateMsg(Data data)
    {
        UpdateMsg updateMsg = (UpdateMsg) data;
        assertEquals(12, updateMsg.streamId());
        assertEquals(99, updateMsg.seqNum());
        assertEquals("UPDATE.RIC", updateMsg.name());
        assertEquals(4, updateMsg.serviceId());
        assertEquals(EmaRdm.MMT_MARKET_PRICE, updateMsg.domainType());
    }

    private Data materializeComplexTypeData(Data data, int dataType)
    {
        DataDictionary dictionary = loadTestDictionary();
        com.refinitiv.eta.codec.DataDictionary rsslDictionary = ((DataDictionaryImpl) dictionary).rsslDataDictionary();

        switch (dataType)
        {
            case DataType.DataTypes.FIELD_LIST:
            {
                FieldList fieldList = JUnitTestConnect.createFieldList();
                JUnitTestConnect.setRsslData(fieldList, data, Codec.majorVersion(), Codec.minorVersion(), rsslDictionary, null);
                return fieldList;
            }
            case DataType.DataTypes.MAP:
            {
                Map map = JUnitTestConnect.createMap();
                JUnitTestConnect.setRsslData(map, data, Codec.majorVersion(), Codec.minorVersion(), rsslDictionary, null);
                return map;
            }
            case DataType.DataTypes.ELEMENT_LIST:
            {
                ElementList elementList = JUnitTestConnect.createElementList();
                JUnitTestConnect.setRsslData(elementList, data, Codec.majorVersion(), Codec.minorVersion(), rsslDictionary, null);
                return elementList;
            }
            case DataType.DataTypes.FILTER_LIST:
            {
                FilterList filterList = JUnitTestConnect.createFilterList();
                JUnitTestConnect.setRsslData(filterList, data, Codec.majorVersion(), Codec.minorVersion(), rsslDictionary, null);
                return filterList;
            }
            case DataType.DataTypes.VECTOR:
            {
                Vector vector = JUnitTestConnect.createVector();
                JUnitTestConnect.setRsslData(vector, data, Codec.majorVersion(), Codec.minorVersion(), rsslDictionary, null);
                return vector;
            }
            case DataType.DataTypes.SERIES:
            {
                Series series = JUnitTestConnect.createSeries();
                JUnitTestConnect.setRsslData(series, data, Codec.majorVersion(), Codec.minorVersion(), rsslDictionary, null);
                return series;
            }
            case DataType.DataTypes.REQ_MSG:
            {
                ReqMsg reqMsg = JUnitTestConnect.createReqMsg();
                JUnitTestConnect.setRsslData(reqMsg, data, Codec.majorVersion(), Codec.minorVersion(), rsslDictionary, null);
                return reqMsg;
            }
            case DataType.DataTypes.REFRESH_MSG:
            {
                RefreshMsg refreshMsg = JUnitTestConnect.createRefreshMsg();
                JUnitTestConnect.setRsslData(refreshMsg, data, Codec.majorVersion(), Codec.minorVersion(), rsslDictionary, null);
                return refreshMsg;
            }
            case DataType.DataTypes.UPDATE_MSG:
            {
                UpdateMsg updateMsg = JUnitTestConnect.createUpdateMsg();
                JUnitTestConnect.setRsslData(updateMsg, data, Codec.majorVersion(), Codec.minorVersion(), rsslDictionary, null);
                return updateMsg;
            }
            default:
                return data;
        }
    }

    private DataDictionary loadTestDictionary()
    {
        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());
        return dictionary;
    }

    private ElementList decodeData(ElementList encElementList)
    {
        DataDictionary dictionary = loadTestDictionary();

        ElementList decElementList = JUnitTestConnect.createElementList();

        JUnitTestConnect.setRsslData(decElementList, encElementList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        return decElementList;
    }

    private static class ComplexTypeTestCase
    {
        private final String name;
        private final int expectedDataType;
        private final Supplier<ComplexType> factory;
        private final Consumer<Data> verifier;

        private ComplexTypeTestCase(String name, int expectedDataType, Supplier<ComplexType> factory,
                                    Consumer<Data> verifier)
        {
            this.name = name;
            this.expectedDataType = expectedDataType;
            this.factory = factory;
            this.verifier = verifier;
        }
    }
}