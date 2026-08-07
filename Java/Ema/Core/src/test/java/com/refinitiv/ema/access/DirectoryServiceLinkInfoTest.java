/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceLink;
import com.refinitiv.ema.domain.directory.DirectoryServiceLinkInfo;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.Codec;
import org.junit.Before;
import org.junit.Test;
import org.mockito.MockitoAnnotations;
import org.mockito.Spy;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;
import java.nio.ByteBuffer;

import static com.refinitiv.ema.access.MapEntry.*;
import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

public class DirectoryServiceLinkInfoTest
{
    @Spy
    DirectoryServiceLinkInfo directoryServiceLinkInfo = EmaFactory.Domain.createDirectoryServiceLinkInfo();
    @Spy
    DirectoryServiceLink directoryServiceLink = EmaFactory.Domain.createDirectoryServiceLink();

    @Before
    public void setUp()
    {
        MockitoAnnotations.openMocks(this);
    }

    @Test
    public void givenServiceLinkInfo_whenClear_thenClearAllFields()
    {
        directoryServiceLinkInfo.linkList(Collections.singletonList(directoryServiceLink));
        directoryServiceLinkInfo.action(MapAction.DELETE);

        directoryServiceLinkInfo.clear();

        assertTrue(directoryServiceLinkInfo.linkList().isEmpty());
        assertEquals(MapAction.ADD, directoryServiceLinkInfo.action());
        assertEquals(EmaRdm.SERVICE_LINK_ID, directoryServiceLinkInfo.filterId());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsLinkList_whenSetLinkList_thenThrowException()
    {
        directoryServiceLinkInfo.linkList(null);
    }

    @Test
    public void givenExternalLinkList_whenSetLinkList_thenInternalListDoesNotRetainListReference()
    {
        List<DirectoryServiceLink> linkList = new ArrayList<>();
        DirectoryServiceLink serviceLink = EmaFactory.Domain.createDirectoryServiceLink();
        serviceLink.name("Upstream 1");
        linkList.add(serviceLink);

        directoryServiceLinkInfo.linkList(linkList);
        linkList.clear();

        assertEquals(1, directoryServiceLinkInfo.linkList().size());
        assertEquals("Upstream 1", directoryServiceLinkInfo.linkList().get(0).name());
    }

    @Test
    public void givenInternalLinkList_whenSetLinkListWithSameInstance_thenLeaveStateUnchanged()
    {
        DirectoryServiceLink serviceLink = EmaFactory.Domain.createDirectoryServiceLink();
        serviceLink.name("Upstream 1");
        directoryServiceLinkInfo.linkList(Collections.singletonList(serviceLink));

        List<DirectoryServiceLink> currentLinks = directoryServiceLinkInfo.linkList();
        directoryServiceLinkInfo.linkList(currentLinks);

        assertSame(currentLinks, directoryServiceLinkInfo.linkList());
        assertEquals(1, directoryServiceLinkInfo.linkList().size());
        assertEquals("Upstream 1", directoryServiceLinkInfo.linkList().get(0).name());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceLinkInfo_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryServiceLinkInfo.copy(null);
    }

    @Test
    public void givenServiceLinkInfo_whenCopyOtherServiceLinkInfo_thenReturnTrueAndUpdateFields()
    {
        DirectoryServiceLinkInfo otherDirectoryServiceLinkInfo = EmaFactory.Domain.createDirectoryServiceLinkInfo();
        otherDirectoryServiceLinkInfo.action(MapAction.UPDATE);

        List<DirectoryServiceLink> linkList = new ArrayList<>();
        directoryServiceLink.action(MapAction.ADD);
        linkList.add(directoryServiceLink);
        DirectoryServiceLink otherDirectoryServiceLink = EmaFactory.Domain.createDirectoryServiceLink();
        otherDirectoryServiceLink.action(MapAction.DELETE);
        linkList.add(otherDirectoryServiceLink);
        otherDirectoryServiceLinkInfo.linkList(linkList);

        directoryServiceLinkInfo.copy(otherDirectoryServiceLinkInfo);
        verify(directoryServiceLinkInfo, times(1)).clear();

        assertEquals(2, directoryServiceLinkInfo.linkList().size());
        assertEquals(MapAction.UPDATE, directoryServiceLinkInfo.action());
    }

    @Test
    public void givenServiceLinkInfo_whenCopyWithSelf_thenReturnTrueWithoutClearingOrChangingState()
    {
        DirectoryServiceLink serviceLink = EmaFactory.Domain.createDirectoryServiceLink();
        serviceLink.name("Upstream 1");
        serviceLink.action(MapAction.UPDATE);
        directoryServiceLinkInfo.linkList(Collections.singletonList(serviceLink));
        directoryServiceLinkInfo.action(MapAction.UPDATE);

        directoryServiceLinkInfo.copy(directoryServiceLinkInfo);
        verify(directoryServiceLinkInfo, never()).clear();

        assertEquals(1, directoryServiceLinkInfo.linkList().size());
        assertEquals("Upstream 1", directoryServiceLinkInfo.linkList().get(0).name());
        assertEquals(MapAction.UPDATE, directoryServiceLinkInfo.action());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceLinkInfo_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryServiceLink.decode(null);
    }

    @Test
    public void givenServiceLinkInfo_whenEncodeAndThenDecodeIntoOtherServiceLinkInfo_thenFillOtherServiceLinkInfoFields()
    {
        // encode
        List<DirectoryServiceLink> linkList = new ArrayList<>();
        directoryServiceLink.action(MapAction.ADD);
        directoryServiceLink.name("link1");
        linkList.add(directoryServiceLink);
        DirectoryServiceLink otherDirectoryServiceLink = EmaFactory.Domain.createDirectoryServiceLink();
        otherDirectoryServiceLink.action(MapAction.DELETE);
        otherDirectoryServiceLink.name("link2");
        linkList.add(otherDirectoryServiceLink);

        directoryServiceLinkInfo.linkList(linkList);

        Map encMap = directoryServiceLinkInfo.encode();

        verify(directoryServiceLink, times(1)).encode();
        assertEquals(2, directoryServiceLinkInfo.linkList().size());

        Map decMap = createDecodedMap(encMap);

        // decode
        DirectoryServiceLinkInfo otherDirectoryServiceLinkInfo = EmaFactory.Domain.createDirectoryServiceLinkInfo();
        otherDirectoryServiceLinkInfo.decode(decMap);

        assertEquals(2, otherDirectoryServiceLinkInfo.linkList().size());
        assertEquals(MapAction.ADD, otherDirectoryServiceLinkInfo.linkList().get(0).action());
        assertEquals("link1", otherDirectoryServiceLinkInfo.linkList().get(0).name());
        assertEquals(MapAction.DELETE, otherDirectoryServiceLinkInfo.linkList().get(1).action());
        assertEquals("link2", otherDirectoryServiceLinkInfo.linkList().get(1).name());
    }

    @Test
    public void givenDeleteEntryWithNoData_whenDecode_thenPopulateLinkNameAndAction()
    {
        Map map = EmaFactory.createMap();
        map.add(EmaFactory.createMapEntry().keyAscii("Upstream 1", MapAction.DELETE));

        directoryServiceLinkInfo.decode(createDecodedMap(map));

        assertEquals(1, directoryServiceLinkInfo.linkList().size());
        assertEquals("Upstream 1", directoryServiceLinkInfo.linkList().get(0).name());
        assertEquals(MapAction.DELETE, directoryServiceLinkInfo.linkList().get(0).action());
    }

    @Test
    public void givenMapWithNonAsciiKey_whenDecode_thenThrowExceptionAndClearState()
    {
        directoryServiceLinkInfo.linkList(Collections.singletonList(directoryServiceLink));
        directoryServiceLinkInfo.action(MapAction.UPDATE);

        Map map = EmaFactory.createMap();
        map.add(EmaFactory.createMapEntry().keyBuffer(ByteBuffer.wrap("Upstream 1".getBytes()), MapAction.ADD));

        try
        {
            directoryServiceLinkInfo.decode(createDecodedMap(map));
            fail("Expected OmmInvalidUsageExceptionImpl to be thrown");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertTrue(directoryServiceLinkInfo.linkList().isEmpty());
            assertEquals(MapAction.ADD, directoryServiceLinkInfo.action());
        }
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenMapWithMalformedLinkPayloadAfterValidEntry_whenDecode_thenThrowException()
    {
        directoryServiceLinkInfo.linkList(Collections.singletonList(directoryServiceLink));
        directoryServiceLinkInfo.action(MapAction.UPDATE);

        Map map = EmaFactory.createMap();
        ElementList elementList = EmaFactory.createElementList();
        elementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_STATE, EmaRdm.LinkStates.UP));
        map.add(EmaFactory.createMapEntry().keyAscii("Upstream 1", MapAction.ADD, elementList));

        ElementList malformedPayload = EmaFactory.createElementList();
        malformedPayload.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_TEXT, "missing link state"));
        map.add(EmaFactory.createMapEntry().keyAscii("Upstream 2", MapAction.ADD, malformedPayload));

        directoryServiceLinkInfo.decode(createDecodedMap(map));
    }

    private Map createDecodedMap(Map map)
    {
        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        Map decodedMap = JUnitTestConnect.createMap();
        JUnitTestConnect.setRsslData(decodedMap, map, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl) dictionary).rsslDataDictionary(), null);

        return decodedMap;
    }
}