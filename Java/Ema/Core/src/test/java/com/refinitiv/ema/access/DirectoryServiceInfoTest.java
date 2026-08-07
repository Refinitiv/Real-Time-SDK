/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryQos;
import com.refinitiv.ema.domain.directory.DirectoryServiceInfo;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.*;
import org.junit.Before;
import org.junit.Test;
import org.mockito.MockitoAnnotations;
import org.mockito.Spy;

import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;

import static com.refinitiv.ema.access.FilterEntry.*;
import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

public class DirectoryServiceInfoTest
{
    @Spy
    DirectoryServiceInfo directoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo();

    @Before
    public void setUp()
    {
        MockitoAnnotations.openMocks(this);
    }

    @Test
    public void givenServiceInfo_whenClear_thenClearAllFields()
    {
        directoryServiceInfo.itemList("Test Item List");
        directoryServiceInfo.vendor("Test Vendor");
        directoryServiceInfo.serviceName("Test Service Name");
        directoryServiceInfo.isSource(false);
        directoryServiceInfo.supportsQosRange(true);
        directoryServiceInfo.supportsOutOfBandSnapshots(false);
        directoryServiceInfo.acceptingConsumerStatus(false);
        directoryServiceInfo.capabilitiesList(Arrays.asList(1L, 2L));
        directoryServiceInfo.dictionariesProvidedList(new ArrayList<>());
        directoryServiceInfo.dictionariesUsedList(new ArrayList<>());
        directoryServiceInfo.qosList(new ArrayList<>());
        directoryServiceInfo.action(FilterAction.CLEAR);

        assertTrue(directoryServiceInfo.checkHasItemList());
        assertTrue(directoryServiceInfo.checkHasVendor());
        assertTrue(directoryServiceInfo.checkHasIsSource());
        assertTrue(directoryServiceInfo.checkHasSupportsQosRange());
        assertTrue(directoryServiceInfo.checkHasSupportsOutOfBandSnapshots());
        assertTrue(directoryServiceInfo.checkHasAcceptingConsumerStatus());
        assertTrue(directoryServiceInfo.checkHasDictionariesProvided());
        assertTrue(directoryServiceInfo.checkHasDictionariesUsed());
        assertTrue(directoryServiceInfo.checkHasQos());

        directoryServiceInfo.clear();

        assertFalse(directoryServiceInfo.checkHasItemList());
        assertFalse(directoryServiceInfo.checkHasVendor());
        assertFalse(directoryServiceInfo.checkHasIsSource());
        assertFalse(directoryServiceInfo.checkHasSupportsQosRange());
        assertFalse(directoryServiceInfo.checkHasSupportsOutOfBandSnapshots());
        assertFalse(directoryServiceInfo.checkHasAcceptingConsumerStatus());
        assertFalse(directoryServiceInfo.checkHasDictionariesProvided());
        assertFalse(directoryServiceInfo.checkHasDictionariesUsed());
        assertFalse(directoryServiceInfo.checkHasQos());

        assertTrue(directoryServiceInfo.capabilitiesList().isEmpty());
        assertEquals("", directoryServiceInfo.serviceName());
        assertEquals(FilterAction.SET, directoryServiceInfo.action());
        assertEquals(EmaRdm.SERVICE_INFO_ID, directoryServiceInfo.filterId());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenItemListNotSet_whenGetItemList_thenThrowException()
    {
        directoryServiceInfo.itemList();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsItemList_whenSetItemList_thenThrowException()
    {
        directoryServiceInfo.itemList(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenVendorNotSet_whenGetVendor_thenThrowException()
    {
        directoryServiceInfo.vendor();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsVendor_whenSetVendor_thenThrowException()
    {
        directoryServiceInfo.vendor(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsServiceName_whenSetServiceName_thenThrowException()
    {
        directoryServiceInfo.serviceName(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenIsSourceNotSet_whenGetIsSource_thenThrowException()
    {
        directoryServiceInfo.isSource();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenSupportsQosRangeNotSet_whenGetSupportsQosRange_thenThrowException()
    {
        directoryServiceInfo.supportsQosRange();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenSupportsOutOfBandSnapshotsNotSet_whenGetSupportsOutOfBandSnapshots_thenThrowException()
    {
        directoryServiceInfo.supportsOutOfBandSnapshots();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenAcceptingConsumerStatusNotSet_whenGetAcceptingConsumerStatus_thenThrowException()
    {
        directoryServiceInfo.acceptingConsumerStatus();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDictionariesProvidedListNotSet_whenGetDictionariesProvidedList_thenThrowException()
    {
        directoryServiceInfo.dictionariesProvidedList();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsDictionariesProvidedList_whenSetDictionariesProvidedList_thenThrowException()
    {
        directoryServiceInfo.dictionariesProvidedList(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDictionariesUsedListNotSet_whenGetDictionariesUsedList_thenThrowException()
    {
        directoryServiceInfo.dictionariesUsedList();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsDictionariesUsedList_whenSetDictionariesUsedList_thenThrowException()
    {
        directoryServiceInfo.dictionariesUsedList(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenQosListNotSet_whenGetQosList_thenThrowException()
    {
        directoryServiceInfo.qosList();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsQosList_whenSetQosList_thenThrowException()
    {
        directoryServiceInfo.qosList(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsDirectoryQosList_whenSetQosListFromDirectoryQos_thenThrowException()
    {
        directoryServiceInfo.qosListFromDirectoryQos(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidActionValue_whenSetAction_thenThrowException()
    {
        directoryServiceInfo.action(99);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceInfo_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryServiceInfo.copy(null);
    }

    @Test
    public void givenServiceInfo_whenCopyOtherServiceInfo_thenReturnTrueAndUpdateFields()
    {
        DirectoryServiceInfo otherDirectoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo()
                .itemList("Test Item List")
                .vendor("Test Vendor")
                .serviceName("Test Service Name")
                .isSource(true)
                .supportsQosRange(false)
                .supportsOutOfBandSnapshots(true)
                .acceptingConsumerStatus(true)
                .capabilitiesList(Collections.singletonList(101L))
                .dictionariesProvidedList(Collections.singletonList("Provided 1"))
                .dictionariesUsedList(Arrays.asList("Used 1", "Used 2"))
                .qosListFromDirectoryQos(Collections.singletonList(createDirectoryQos(200, 100)))
                .action(FilterAction.UPDATE);

        directoryServiceInfo.copy(otherDirectoryServiceInfo);
        verify(directoryServiceInfo, times(1)).clear();

        assertTrue(directoryServiceInfo.checkHasItemList());
        assertTrue(directoryServiceInfo.checkHasVendor());
        assertTrue(directoryServiceInfo.checkHasIsSource());
        assertTrue(directoryServiceInfo.checkHasSupportsQosRange());
        assertTrue(directoryServiceInfo.checkHasSupportsOutOfBandSnapshots());
        assertTrue(directoryServiceInfo.checkHasAcceptingConsumerStatus());
        assertTrue(directoryServiceInfo.checkHasDictionariesProvided());
        assertTrue(directoryServiceInfo.checkHasDictionariesUsed());
        assertTrue(directoryServiceInfo.checkHasQos());

        assertEquals("Test Item List", directoryServiceInfo.itemList());
        assertEquals("Test Vendor", directoryServiceInfo.vendor());
        assertEquals("Test Service Name", directoryServiceInfo.serviceName());
        assertTrue(directoryServiceInfo.isSource());
        assertTrue(directoryServiceInfo.supportsOutOfBandSnapshots());
        assertTrue(directoryServiceInfo.acceptingConsumerStatus());
        assertFalse(directoryServiceInfo.supportsQosRange());

        assertEquals(1, directoryServiceInfo.capabilitiesList().size());
        assertEquals(101, (long) directoryServiceInfo.capabilitiesList().get(0));

        assertEquals(1, directoryServiceInfo.dictionariesProvidedList().size());
        assertEquals("Provided 1", directoryServiceInfo.dictionariesProvidedList().get(0));

        assertEquals(2, directoryServiceInfo.dictionariesUsedList().size());
        assertEquals("Used 1", directoryServiceInfo.dictionariesUsedList().get(0));
        assertEquals("Used 2", directoryServiceInfo.dictionariesUsedList().get(1));

        assertEquals(1, directoryServiceInfo.qosList().size());
        assertEquals(100, directoryServiceInfo.qosList().get(0).rate());
        assertEquals(200, directoryServiceInfo.qosList().get(0).timeliness());

        assertEquals(FilterAction.UPDATE, directoryServiceInfo.action());
        assertEquals(EmaRdm.SERVICE_INFO_ID, directoryServiceInfo.filterId());
    }

    @Test
    public void givenServiceInfoWithOptionalFields_whenCopyFromSourceWithoutOptionals_thenClearDestinationOptionals()
    {
        directoryServiceInfo.itemList("Existing Item List");
        directoryServiceInfo.vendor("Existing Vendor");
        directoryServiceInfo.serviceName("Existing Service Name");
        directoryServiceInfo.isSource(false);
        directoryServiceInfo.supportsQosRange(true);
        directoryServiceInfo.supportsOutOfBandSnapshots(false);
        directoryServiceInfo.acceptingConsumerStatus(false);
        directoryServiceInfo.capabilitiesList(Arrays.asList(7L, 8L));
        directoryServiceInfo.dictionariesProvidedList(Collections.singletonList("Existing Provided"));
        directoryServiceInfo.dictionariesUsedList(Collections.singletonList("Existing Used"));
        directoryServiceInfo.qosList(Collections.singletonList(new OmmQosImpl()));
        directoryServiceInfo.action(FilterAction.UPDATE);

        DirectoryServiceInfo otherDirectoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo();
        otherDirectoryServiceInfo.serviceName("Updated Service Name");
        otherDirectoryServiceInfo.capabilitiesList(Collections.singletonList(202L));
        otherDirectoryServiceInfo.action(FilterAction.CLEAR);

        directoryServiceInfo.copy(otherDirectoryServiceInfo);

        assertEquals("Updated Service Name", directoryServiceInfo.serviceName());
        assertEquals(FilterAction.CLEAR, directoryServiceInfo.action());
        assertEquals(1, directoryServiceInfo.capabilitiesList().size());
        assertEquals(202L, (long) directoryServiceInfo.capabilitiesList().get(0));
        assertFalse(directoryServiceInfo.checkHasItemList());
        assertFalse(directoryServiceInfo.checkHasVendor());
        assertFalse(directoryServiceInfo.checkHasIsSource());
        assertFalse(directoryServiceInfo.checkHasSupportsQosRange());
        assertFalse(directoryServiceInfo.checkHasSupportsOutOfBandSnapshots());
        assertFalse(directoryServiceInfo.checkHasAcceptingConsumerStatus());
        assertFalse(directoryServiceInfo.checkHasDictionariesProvided());
        assertFalse(directoryServiceInfo.checkHasDictionariesUsed());
        assertFalse(directoryServiceInfo.checkHasQos());
    }

    @Test
    public void givenServiceInfo_whenCopyIntoSelf_thenPreserveFields()
    {
        directoryServiceInfo.itemList("Test Item List");
        directoryServiceInfo.vendor("Test Vendor");
        directoryServiceInfo.serviceName("Test Service Name");
        directoryServiceInfo.isSource(true);
        directoryServiceInfo.supportsQosRange(false);
        directoryServiceInfo.supportsOutOfBandSnapshots(true);
        directoryServiceInfo.acceptingConsumerStatus(true);
        directoryServiceInfo.capabilitiesList(Collections.singletonList(101L));
        directoryServiceInfo.dictionariesProvidedList(Collections.singletonList("Provided 1"));
        directoryServiceInfo.dictionariesUsedList(Arrays.asList("Used 1", "Used 2"));
        directoryServiceInfo.qosList(Collections.singletonList(new OmmQosImpl()));
        directoryServiceInfo.action(FilterAction.UPDATE);

        directoryServiceInfo.copy(directoryServiceInfo);

        assertTrue(directoryServiceInfo.checkHasItemList());
        assertTrue(directoryServiceInfo.checkHasVendor());
        assertTrue(directoryServiceInfo.checkHasIsSource());
        assertTrue(directoryServiceInfo.checkHasSupportsQosRange());
        assertTrue(directoryServiceInfo.checkHasSupportsOutOfBandSnapshots());
        assertTrue(directoryServiceInfo.checkHasAcceptingConsumerStatus());
        assertTrue(directoryServiceInfo.checkHasDictionariesProvided());
        assertTrue(directoryServiceInfo.checkHasDictionariesUsed());
        assertTrue(directoryServiceInfo.checkHasQos());
        assertEquals("Test Item List", directoryServiceInfo.itemList());
        assertEquals("Test Vendor", directoryServiceInfo.vendor());
        assertEquals("Test Service Name", directoryServiceInfo.serviceName());
        assertTrue(directoryServiceInfo.isSource());
        assertFalse(directoryServiceInfo.supportsQosRange());
        assertTrue(directoryServiceInfo.supportsOutOfBandSnapshots());
        assertTrue(directoryServiceInfo.acceptingConsumerStatus());
        assertEquals(Collections.singletonList(101L), directoryServiceInfo.capabilitiesList());
        assertEquals(Collections.singletonList("Provided 1"), directoryServiceInfo.dictionariesProvidedList());
        assertEquals(Arrays.asList("Used 1", "Used 2"), directoryServiceInfo.dictionariesUsedList());
        assertEquals(1, directoryServiceInfo.qosList().size());
        assertEquals(FilterAction.UPDATE, directoryServiceInfo.action());
    }

    @Test
    public void givenDirectoryQosList_whenSetQosListFromDirectoryQos_thenPopulateQosField()
    {
        directoryServiceInfo.qosListFromDirectoryQos(Collections.singletonList(
                createDirectoryQos(10, 15)));

        assertTrue(directoryServiceInfo.checkHasQos());
        assertEquals(1, directoryServiceInfo.qosList().size());
        assertEquals(10, directoryServiceInfo.qosList().get(0).timeliness());
        assertEquals(15, directoryServiceInfo.qosList().get(0).rate());
    }

    @Test
    public void givenEmptyDirectoryQosList_whenSetQosListFromDirectoryQos_thenMarkQosPresentAndEmpty()
    {
        directoryServiceInfo.qosListFromDirectoryQos(new ArrayList<>());

        assertTrue(directoryServiceInfo.checkHasQos());
        assertTrue(directoryServiceInfo.qosList().isEmpty());
    }

    @Test
    public void givenExistingQos_whenSetQosListFromDirectoryQos_thenReplacePreviousEntriesAndPreserveOrder()
    {
        directoryServiceInfo
                .qosList(Collections.singletonList(createQos(1, 2)))
                .qosListFromDirectoryQos(Arrays.asList(
                        createDirectoryQos(10, 15),
                        createDirectoryQos(OmmQos.Timeliness.INEXACT_DELAYED,
                                OmmQos.Rate.JUST_IN_TIME_CONFLATED)));

        assertEquals(2, directoryServiceInfo.qosList().size());
        assertEquals(10, directoryServiceInfo.qosList().get(0).timeliness());
        assertEquals(15, directoryServiceInfo.qosList().get(0).rate());
        assertEquals(OmmQos.Timeliness.INEXACT_DELAYED, directoryServiceInfo.qosList().get(1).timeliness());
        assertEquals(OmmQos.Rate.JUST_IN_TIME_CONFLATED, directoryServiceInfo.qosList().get(1).rate());
    }

    @Test
    public void givenDirectoryQosSourceMutatedAfterSet_whenUsingQosListFromDirectoryQos_thenStoredQosRemainsDetached()
    {
        DirectoryQos sourceQos = createDirectoryQos(10, 15);

        directoryServiceInfo.qosListFromDirectoryQos(Collections.singletonList(sourceQos));

        sourceQos.timeliness(OmmQos.Timeliness.REALTIME).rate(OmmQos.Rate.TICK_BY_TICK);

        assertEquals(1, directoryServiceInfo.qosList().size());
        assertEquals(10, directoryServiceInfo.qosList().get(0).timeliness());
        assertEquals(15, directoryServiceInfo.qosList().get(0).rate());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceInfo_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryServiceInfo.decode(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceInfo_whenDecodeWithoutNameElement_thenThrowException()
    {
        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES,
                createCapabilitiesArray(1L)));
        encElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_VENDOR, "Ignored"));

        directoryServiceInfo.decode(createDecodedElementList(encElementList));
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceInfo_whenDecodeWithBlankNameElement_thenThrowException()
    {
        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().codeAscii(EmaRdm.ENAME_NAME));
        encElementList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES,
                createCapabilitiesArray(1L)));

        directoryServiceInfo.decode(createDecodedElementList(encElementList));
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceInfo_whenDecodeWithoutCapabilitiesElement_thenThrowException()
    {
        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "Service Name"));
        encElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_VENDOR, "Ignored"));

        directoryServiceInfo.decode(createDecodedElementList(encElementList));
    }

    @Test
    public void givenServiceInfo_whenDecodeWithUnknownElementAndBlankOptionals_thenIgnoreUnknownAndLeaveOptionalsAbsent()
    {
        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "Service Name"));
        encElementList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES,
                createCapabilitiesArray(1L)));
        encElementList.add(EmaFactory.createElementEntry().ascii("UNKNOWN_ELEMENT", "ignored"));
        encElementList.add(EmaFactory.createElementEntry().codeAscii(EmaRdm.ENAME_VENDOR));
        encElementList.add(EmaFactory.createElementEntry().codeAscii(EmaRdm.ENAME_ITEM_LIST));

        directoryServiceInfo.decode(createDecodedElementList(encElementList));

        assertEquals("Service Name", directoryServiceInfo.serviceName());
        assertEquals(Collections.singletonList(1L), directoryServiceInfo.capabilitiesList());
        assertFalse(directoryServiceInfo.checkHasVendor());
        assertFalse(directoryServiceInfo.checkHasItemList());
        assertFalse(directoryServiceInfo.checkHasIsSource());
        assertFalse(directoryServiceInfo.checkHasSupportsQosRange());
        assertFalse(directoryServiceInfo.checkHasSupportsOutOfBandSnapshots());
        assertFalse(directoryServiceInfo.checkHasAcceptingConsumerStatus());
        assertFalse(directoryServiceInfo.checkHasDictionariesProvided());
        assertFalse(directoryServiceInfo.checkHasDictionariesUsed());
        assertFalse(directoryServiceInfo.checkHasQos());
    }

    @Test
    public void givenServiceInfo_whenDecodeWithInvalidBooleanElementValues_thenThrowException()
    {
        assertDecodeInvalidBooleanElementThrows(EmaRdm.ENAME_IS_SOURCE);
        assertDecodeInvalidBooleanElementThrows(EmaRdm.ENAME_SUPPS_QOS_RANGE);
        assertDecodeInvalidBooleanElementThrows(EmaRdm.ENAME_SUPPS_OOB_SNAPSHOTS);
        assertDecodeInvalidBooleanElementThrows(EmaRdm.ENAME_ACCEPTING_CONS_STATUS);
    }

    @Test
    public void givenServiceInfo_whenSetEmptyOptionalLists_thenMarkListsPresentAndEmpty()
    {
        directoryServiceInfo.dictionariesProvidedList(new ArrayList<>());
        directoryServiceInfo.dictionariesUsedList(new ArrayList<>());
        directoryServiceInfo.qosList(new ArrayList<>());

        assertTrue(directoryServiceInfo.checkHasDictionariesProvided());
        assertTrue(directoryServiceInfo.dictionariesProvidedList().isEmpty());
        assertTrue(directoryServiceInfo.checkHasDictionariesUsed());
        assertTrue(directoryServiceInfo.dictionariesUsedList().isEmpty());
        assertTrue(directoryServiceInfo.checkHasQos());
        assertTrue(directoryServiceInfo.qosList().isEmpty());
    }

    @Test
    public void givenServiceInfoWithRequiredFieldsOnly_whenEncodeAndDecode_thenLeaveOptionalsAbsent()
    {
        directoryServiceInfo.serviceName("Required Only Service");
        directoryServiceInfo.capabilitiesList(Arrays.asList(5L, 8L));

        DirectoryServiceInfo otherDirectoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo();
        otherDirectoryServiceInfo.decode(createDecodedElementList(directoryServiceInfo.encode()));

        assertEquals("Required Only Service", otherDirectoryServiceInfo.serviceName());
        assertEquals(Arrays.asList(5L, 8L), otherDirectoryServiceInfo.capabilitiesList());
        assertFalse(otherDirectoryServiceInfo.checkHasItemList());
        assertFalse(otherDirectoryServiceInfo.checkHasVendor());
        assertFalse(otherDirectoryServiceInfo.checkHasIsSource());
        assertFalse(otherDirectoryServiceInfo.checkHasSupportsQosRange());
        assertFalse(otherDirectoryServiceInfo.checkHasSupportsOutOfBandSnapshots());
        assertFalse(otherDirectoryServiceInfo.checkHasAcceptingConsumerStatus());
        assertFalse(otherDirectoryServiceInfo.checkHasDictionariesProvided());
        assertFalse(otherDirectoryServiceInfo.checkHasDictionariesUsed());
        assertFalse(otherDirectoryServiceInfo.checkHasQos());
    }

    @Test
    public void givenServiceInfo_whenCopy_thenDetachMutableListsAndQosInstancesFromSource()
    {
        DirectoryServiceInfo sourceDirectoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo()
                .serviceName("Source Service Name")
                .capabilitiesList(new ArrayList<>(Arrays.asList(1L, 2L)))
                .dictionariesProvidedList(new ArrayList<>(Collections.singletonList("Provided 1")))
                .dictionariesUsedList(new ArrayList<>(Collections.singletonList("Used 1")))
                .qosList(new ArrayList<>(Collections.singletonList(createQos(7, 9))));

        directoryServiceInfo.copy(sourceDirectoryServiceInfo);

        sourceDirectoryServiceInfo.capabilitiesList().add(999L);
        sourceDirectoryServiceInfo.dictionariesProvidedList().add("Provided 2");
        sourceDirectoryServiceInfo.dictionariesUsedList().clear();
        OmmQos sourceQos = sourceDirectoryServiceInfo.qosList().get(0);
        sourceDirectoryServiceInfo.qosList().clear();

        assertEquals(Arrays.asList(1L, 2L), directoryServiceInfo.capabilitiesList());
        assertEquals(Collections.singletonList("Provided 1"), directoryServiceInfo.dictionariesProvidedList());
        assertEquals(Collections.singletonList("Used 1"), directoryServiceInfo.dictionariesUsedList());
        assertEquals(1, directoryServiceInfo.qosList().size());
        assertEquals(7, directoryServiceInfo.qosList().get(0).timeliness());
        assertEquals(9, directoryServiceInfo.qosList().get(0).rate());
        assertNotSame(sourceQos, directoryServiceInfo.qosList().get(0));
    }

    @Test
    public void givenServiceInfo_whenEncodeAndThenDecodeIntoOtherServiceInfo_thenFillOtherServiceInfoFields()
    {
        // encode
        directoryServiceInfo.itemList("Test Item List");
        directoryServiceInfo.vendor("Test Vendor");
        directoryServiceInfo.serviceName("Test Service Name");
        directoryServiceInfo.isSource(true);
        directoryServiceInfo.supportsQosRange(false);
        directoryServiceInfo.supportsOutOfBandSnapshots(false);
        directoryServiceInfo.acceptingConsumerStatus(true);
        directoryServiceInfo.capabilitiesList(Collections.singletonList(303L));
        directoryServiceInfo.dictionariesProvidedList(Collections.singletonList("Provided 1"));
        directoryServiceInfo.dictionariesUsedList(Arrays.asList("Used 1", "Used 2"));
        directoryServiceInfo.qosList(Collections.singletonList(new OmmQosImpl()));

        ElementList encElementList = directoryServiceInfo.encode();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        ElementList decElementList = JUnitTestConnect.createElementList();

        JUnitTestConnect.setRsslData(decElementList, encElementList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        // decode
        DirectoryServiceInfo otherDirectoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo();
        otherDirectoryServiceInfo.decode(decElementList);

        assertTrue(otherDirectoryServiceInfo.checkHasItemList());
        assertTrue(otherDirectoryServiceInfo.checkHasVendor());
        assertTrue(otherDirectoryServiceInfo.checkHasIsSource());
        assertTrue(otherDirectoryServiceInfo.checkHasSupportsQosRange());
        assertTrue(otherDirectoryServiceInfo.checkHasSupportsOutOfBandSnapshots());
        assertTrue(otherDirectoryServiceInfo.checkHasAcceptingConsumerStatus());
        assertTrue(otherDirectoryServiceInfo.checkHasDictionariesProvided());
        assertTrue(otherDirectoryServiceInfo.checkHasDictionariesUsed());
        assertTrue(otherDirectoryServiceInfo.checkHasQos());

        assertEquals("Test Item List", otherDirectoryServiceInfo.itemList());
        assertEquals("Test Vendor", otherDirectoryServiceInfo.vendor());
        assertEquals("Test Service Name", otherDirectoryServiceInfo.serviceName());
        assertTrue(otherDirectoryServiceInfo.isSource());
        assertFalse(otherDirectoryServiceInfo.supportsOutOfBandSnapshots());
        assertTrue(otherDirectoryServiceInfo.acceptingConsumerStatus());
        assertFalse(otherDirectoryServiceInfo.supportsQosRange());

        assertEquals(1, otherDirectoryServiceInfo.capabilitiesList().size());
        assertEquals(303, (long) otherDirectoryServiceInfo.capabilitiesList().get(0));

        assertEquals(1, otherDirectoryServiceInfo.dictionariesProvidedList().size());
        assertEquals("Provided 1", otherDirectoryServiceInfo.dictionariesProvidedList().get(0));

        assertEquals(2, otherDirectoryServiceInfo.dictionariesUsedList().size());
        assertEquals("Used 1", otherDirectoryServiceInfo.dictionariesUsedList().get(0));
        assertEquals("Used 2", otherDirectoryServiceInfo.dictionariesUsedList().get(1));

        assertEquals(1, otherDirectoryServiceInfo.qosList().size());
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

    private void assertDecodeInvalidBooleanElementThrows(String elementName)
    {
        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_NAME, "Service Name"));
        encElementList.add(EmaFactory.createElementEntry().array(EmaRdm.ENAME_CAPABILITIES,
                createCapabilitiesArray(1L)));
        encElementList.add(EmaFactory.createElementEntry().uintValue(elementName, 2));

        try
        {
            directoryServiceInfo.decode(createDecodedElementList(encElementList));
            fail("Expected OmmInvalidUsageExceptionImpl for " + elementName);
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            // expected
        }
    }

    private DirectoryQos createDirectoryQos(int timeliness, int rate)
    {
        return EmaFactory.Domain.createDirectoryQos()
                .timeliness(timeliness)
                .rate(rate);
    }

    private OmmQos createQos(int timeliness, int rate)
    {
        Qos etaQos = CodecFactory.createQos();
        Utilities.toRsslQos(rate, timeliness, etaQos);

        OmmQosImpl qos = new OmmQosImpl();
        qos.decode(etaQos);

        return qos;
    }

    private OmmArray createCapabilitiesArray(long... capabilities)
    {
        OmmArray capabilityArray = EmaFactory.createOmmArray();
        for (long capability : capabilities)
        {
            capabilityArray.add(EmaFactory.createOmmArrayEntry().uintValue(capability));
        }

        return capabilityArray;
    }
}