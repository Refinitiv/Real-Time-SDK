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
import com.refinitiv.eta.codec.Codec;
import org.junit.Before;
import org.junit.Test;
import org.mockito.MockitoAnnotations;
import org.mockito.Spy;

import java.nio.ByteBuffer;
import java.nio.charset.StandardCharsets;
import java.util.Arrays;
import java.util.Collections;

import static com.refinitiv.ema.access.FilterEntry.*;
import static com.refinitiv.ema.access.MapEntry.*;
import static com.refinitiv.ema.access.OmmState.*;
import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

public class DirectoryServiceTest
{
    private static final ByteBuffer GROUP_BUFFER = ByteBuffer.wrap("TestGroup".getBytes(StandardCharsets.US_ASCII));

    @Spy
    DirectoryService directoryService = EmaFactory.Domain.createDirectoryService();

    DirectoryServiceInfo directoryServiceInfo = EmaFactory.Domain.createDirectoryServiceInfo();
    DirectoryServiceState directoryServiceState = EmaFactory.Domain.createDirectoryServiceState();
    DirectoryServiceLoad directoryServiceLoad = EmaFactory.Domain.createDirectoryServiceLoad();
    DirectoryServiceData directoryServiceData = EmaFactory.Domain.createDirectoryServiceData();
    DirectoryServiceLinkInfo directoryServiceLinkInfo = EmaFactory.Domain.createDirectoryServiceLinkInfo();
    DirectoryServiceLink directoryServiceLink = EmaFactory.Domain.createDirectoryServiceLink();
    DirectoryServiceGroup directoryServiceGroup = EmaFactory.Domain.createDirectoryServiceGroup();

    @Before
    public void setUp()
    {
        MockitoAnnotations.openMocks(this);

        directoryServiceInfo.serviceName("Test Service");
        directoryServiceInfo.vendor("Test Vendor");
        directoryServiceInfo.itemList("Test Item List");
        directoryServiceInfo.capabilitiesList(Arrays.asList(3L, 7L));

        directoryServiceState.serviceState(EmaRdm.SERVICE_DOWN);
        directoryServiceState.status(StreamState.CLOSED_RECOVER, DataState.SUSPECT, StatusCode.SOURCE_UNKNOWN,
                "Test message");

        directoryServiceLoad.openWindow(5);
        directoryServiceLoad.loadFactor(6);
        directoryServiceLoad.openLimit(7);

        directoryServiceData.type(EmaRdm.DataTypes.HEADLINE);
        directoryServiceData.dataAsAscii("Test ascii");

        directoryServiceLink.name("Test stream");
        directoryServiceLink.linkState(EmaRdm.LinkStates.UP);
        directoryServiceLink.text("Test text");
        directoryServiceLink.type(EmaRdm.SERVICE_LINK_BROADCAST);
        directoryServiceLink.linkCode(EmaRdm.SERVICE_LINK_CODE_OK);
        directoryServiceLinkInfo.linkList(Collections.singletonList(directoryServiceLink));

        directoryServiceGroup.group(GROUP_BUFFER.duplicate());
        directoryServiceGroup.status(StreamState.OPEN, DataState.OK, StatusCode.NONE,
                "Test message");
    }

    @Test
    public void givenService_whenClear_thenClearAllFields()
    {
        directoryService.serviceId(101);
        setDirectoryService(directoryService);
        directoryService.action(MapAction.DELETE);

        assertTrue(directoryService.checkHasInfo());
        assertTrue(directoryService.checkHasState());
        assertTrue(directoryService.checkHasLoad());
        assertTrue(directoryService.checkHasData());
        assertTrue(directoryService.checkHasLink());

        directoryService.clear();

        assertFalse(directoryService.checkHasInfo());
        assertFalse(directoryService.checkHasState());
        assertFalse(directoryService.checkHasLoad());
        assertFalse(directoryService.checkHasData());
        assertFalse(directoryService.checkHasLink());

        assertEquals(-1, directoryService.serviceId());
        assertTrue(directoryService.groupStateList().isEmpty());
        assertEquals(MapAction.ADD, directoryService.action());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsInfo_whenSetInfo_thenThrowException()
    {
        directoryService.info(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInfoNotSet_whenGetInfo_thenThrowException()
    {
        directoryService.info();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsState_whenSetState_thenThrowException()
    {
        directoryService.state(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenStateNotSet_whenGetState_thenThrowException()
    {
        directoryService.state();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsLoad_whenSetLoad_thenThrowException()
    {
        directoryService.load(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenLoadNotSet_whenGetLoad_thenThrowException()
    {
        directoryService.load();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsData_whenSetData_thenThrowException()
    {
        directoryService.data(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDataNotSet_whenGetData_thenThrowException()
    {
        directoryService.data();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsLink_whenSetLink_thenThrowException()
    {
        directoryService.link(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenLinkNotSet_whenGetLink_thenThrowException()
    {
        directoryService.link();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsGroupStateList_whenSetGroupStateList_thenThrowException()
    {
        directoryService.groupStateList(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidActionValue_whenSetAction_thenThrowException()
    {
        directoryService.action(99);
    }

    @Test
    public void givenService_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryService.serviceId(101);
        directoryService.action(MapAction.DELETE);
        setDirectoryService(directoryService);

        try
        {
            directoryService.copy(null);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            // expected
        }

        assertEquals(101, directoryService.serviceId());
        assertEquals(MapAction.DELETE, directoryService.action());
        checkDirectoryService(directoryService);
    }

    @Test
    public void givenService_whenCopyWithOtherService_thenUpdateFields()
    {
        DirectoryService otherDirectoryService = EmaFactory.Domain.createDirectoryService();
        otherDirectoryService.serviceId(103);
        setDirectoryService(otherDirectoryService);
        otherDirectoryService.action(MapAction.ADD);

        directoryService.copy(otherDirectoryService);
        verify(directoryService, times(1)).clear();

        assertEquals(MapAction.ADD, directoryService.action());
        assertEquals(103, directoryService.serviceId());

        checkDirectoryService(directoryService);

        DirectoryServiceLink directoryServiceLink = directoryService.link().linkList().get(0);
        assertEquals("Test stream", directoryServiceLink.name());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDirectoryService_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryService.decode(null);
    }

    @Test
    public void givenClearFilters_whenEncodeAndDecode_thenPreserveClearEntries()
    {
        DirectoryServiceInfo clearInfo = EmaFactory.Domain.createDirectoryServiceInfo();
        clearInfo.action(FilterAction.CLEAR);

        DirectoryServiceLinkInfo clearLink = EmaFactory.Domain.createDirectoryServiceLinkInfo();
        clearLink.action(MapAction.DELETE);

        directoryService.info(clearInfo);
        directoryService.link(clearLink);

        FilterList encFilterList = directoryService.encode();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        FilterList decFilterList = JUnitTestConnect.createFilterList();
        JUnitTestConnect.setRsslData(decFilterList, encFilterList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        boolean infoFilterFound = false;
        boolean linkFilterFound = false;
        for (FilterEntry filterEntry : decFilterList)
        {
            if (filterEntry.filterId() == EmaRdm.SERVICE_INFO_ID)
            {
                infoFilterFound = true;
                assertEquals(FilterAction.CLEAR, filterEntry.action());
                assertEquals(DataType.DataTypes.NO_DATA, filterEntry.loadType());
            }
            else if (filterEntry.filterId() == EmaRdm.SERVICE_LINK_ID)
            {
                linkFilterFound = true;
                assertEquals(FilterAction.CLEAR, filterEntry.action());
                assertEquals(DataType.DataTypes.NO_DATA, filterEntry.loadType());
            }
        }

        assertTrue(infoFilterFound);
        assertTrue(linkFilterFound);

        DirectoryService otherDirectoryService = EmaFactory.Domain.createDirectoryService();
        otherDirectoryService.decode(decFilterList);

        assertTrue(otherDirectoryService.checkHasInfo());
        assertEquals(FilterAction.CLEAR, otherDirectoryService.info().action());
        assertTrue(otherDirectoryService.checkHasLink());
        assertEquals(MapAction.DELETE, otherDirectoryService.link().action());
    }

    @Test
    public void givenService_whenEncodeAndThenDecodeIntoOtherService_thenFillOtherServiceFields()
    {
        // encode
        setDirectoryService(directoryService);

        FilterList encFilterList = directoryService.encode();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        FilterList decFilterList = JUnitTestConnect.createFilterList();

        JUnitTestConnect.setRsslData(decFilterList, encFilterList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        // decode
        DirectoryService otherDirectoryService = EmaFactory.Domain.createDirectoryService();
        otherDirectoryService.decode(decFilterList);

        checkDirectoryService(otherDirectoryService);
    }

    @Test
    public void givenServiceMetadata_whenDecode_thenPreserveServiceIdAndAction()
    {
        setDirectoryService(directoryService);
        FilterList encFilterList = directoryService.encode();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        FilterList decFilterList = JUnitTestConnect.createFilterList();
        JUnitTestConnect.setRsslData(decFilterList, encFilterList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        DirectoryService otherDirectoryService = EmaFactory.Domain.createDirectoryService();
        otherDirectoryService.serviceId(555);
        otherDirectoryService.action(MapAction.UPDATE);

        otherDirectoryService.decode(decFilterList);

        assertEquals(555, otherDirectoryService.serviceId());
        assertEquals(MapAction.UPDATE, otherDirectoryService.action());
        checkDirectoryService(otherDirectoryService);
    }

    @Test
    public void givenServiceWithMetadata_whenDecodeEmptyFilterList_thenClearFiltersAndPreserveMetadata()
    {
        directoryService.serviceId(777);
        directoryService.action(MapAction.DELETE);
        setDirectoryService(directoryService);

        FilterList emptyFilterList = EmaFactory.createFilterList();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        FilterList decodedEmptyFilterList = JUnitTestConnect.createFilterList();
        JUnitTestConnect.setRsslData(decodedEmptyFilterList, emptyFilterList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        directoryService.decode(decodedEmptyFilterList);

        assertEquals(777, directoryService.serviceId());
        assertEquals(MapAction.DELETE, directoryService.action());
        assertFalse(directoryService.checkHasInfo());
        assertFalse(directoryService.checkHasState());
        assertFalse(directoryService.checkHasLoad());
        assertFalse(directoryService.checkHasData());
        assertFalse(directoryService.checkHasLink());
        assertTrue(directoryService.groupStateList().isEmpty());
    }

    @Test
    public void givenService_whenCopyWithPartialSource_thenReplaceCurrentState()
    {
        setDirectoryService(directoryService);
        directoryService.serviceId(200);
        directoryService.action(MapAction.DELETE);

        DirectoryService sourceService = EmaFactory.Domain.createDirectoryService();
        sourceService.serviceId(300);
        sourceService.action(MapAction.UPDATE);

        DirectoryServiceInfo sourceInfo = EmaFactory.Domain.createDirectoryServiceInfo();
        sourceInfo.serviceName("Source Service");
        sourceInfo.capabilitiesList(Collections.singletonList(9L));
        sourceService.info(sourceInfo);

        directoryService.copy(sourceService);

        assertEquals(300, directoryService.serviceId());
        assertEquals(MapAction.UPDATE, directoryService.action());
        assertTrue(directoryService.checkHasInfo());
        assertEquals("Source Service", directoryService.info().serviceName());
        assertFalse(directoryService.checkHasState());
        assertFalse(directoryService.checkHasLoad());
        assertFalse(directoryService.checkHasData());
        assertFalse(directoryService.checkHasLink());
        assertTrue(directoryService.groupStateList().isEmpty());
    }

    private static void checkDirectoryService(DirectoryService otherDirectoryService)
    {
        assertTrue(otherDirectoryService.checkHasInfo());
        assertTrue(otherDirectoryService.checkHasState());
        assertTrue(otherDirectoryService.checkHasLoad());
        assertTrue(otherDirectoryService.checkHasData());
        assertTrue(otherDirectoryService.checkHasLink());

        assertEquals("Test Service", otherDirectoryService.info().serviceName());
        assertTrue(otherDirectoryService.info().checkHasVendor());
        assertEquals("Test Vendor", otherDirectoryService.info().vendor());
        assertTrue(otherDirectoryService.info().checkHasItemList());
        assertEquals("Test Item List", otherDirectoryService.info().itemList());
        assertEquals(2, otherDirectoryService.info().capabilitiesList().size());

        assertEquals(EmaRdm.SERVICE_DOWN, otherDirectoryService.state().serviceState());
        assertTrue(otherDirectoryService.state().checkHasStatus());
        OmmState status = otherDirectoryService.state().status();
        assertEquals(StreamState.CLOSED_RECOVER, status.streamState());
        assertEquals(DataState.SUSPECT, status.dataState());
        assertEquals(StatusCode.SOURCE_UNKNOWN, status.statusCode());
        assertEquals("Test message", status.statusText());

        assertTrue(otherDirectoryService.load().checkHasOpenWindow());
        assertEquals(5, otherDirectoryService.load().openWindow());
        assertTrue(otherDirectoryService.load().checkHasLoadFactor());
        assertEquals(6, otherDirectoryService.load().loadFactor());
        assertTrue(otherDirectoryService.load().checkHasOpenLimit());
        assertEquals(7, otherDirectoryService.load().openLimit());

        assertTrue(otherDirectoryService.data().checkHasData());
        assertEquals(DataType.DataTypes.ASCII, otherDirectoryService.data().dataType());
        assertEquals("Test ascii", ((OmmAscii) otherDirectoryService.data().data()).ascii());
        assertEquals(EmaRdm.DataTypes.HEADLINE, otherDirectoryService.data().type());

        assertEquals(1, otherDirectoryService.link().linkList().size());
        DirectoryServiceLink directoryServiceLink = otherDirectoryService.link().linkList().get(0);
        assertEquals(EmaRdm.LinkStates.UP, directoryServiceLink.linkState());
        assertTrue(directoryServiceLink.checkHasText());
        assertEquals("Test text", directoryServiceLink.text());
        assertTrue(directoryServiceLink.checkHasType());
        assertEquals(EmaRdm.SERVICE_LINK_BROADCAST, directoryServiceLink.type());
        assertTrue(directoryServiceLink.checkHasLinkCode());
        assertEquals(EmaRdm.SERVICE_LINK_CODE_OK, directoryServiceLink.linkCode());

        assertEquals(1, otherDirectoryService.groupStateList().size());
        assertEquals(GROUP_BUFFER.duplicate(), otherDirectoryService.groupStateList().get(0).group().buffer());
        assertTrue(otherDirectoryService.groupStateList().get(0).checkHasStatus());
        status = otherDirectoryService.groupStateList().get(0).status();
        assertEquals(StreamState.OPEN, status.streamState());
        assertEquals(DataState.OK, status.dataState());
        assertEquals(StatusCode.NONE, status.statusCode());
        assertEquals("Test message", status.statusText());
    }

    private void setDirectoryService(DirectoryService directoryService)
    {
        directoryService.info(directoryServiceInfo);
        directoryService.state(directoryServiceState);
        directoryService.load(directoryServiceLoad);
        directoryService.data(directoryServiceData);
        directoryService.link(directoryServiceLinkInfo);
        directoryService.groupStateList(Collections.singletonList(directoryServiceGroup));
    }
}