/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceLink;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.Codec;
import org.junit.Before;
import org.junit.Test;
import org.mockito.MockitoAnnotations;
import org.mockito.Spy;

import static com.refinitiv.ema.access.MapEntry.*;
import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

public class DirectoryServiceLinkTest
{
    @Spy
    DirectoryServiceLink directoryServiceLink = EmaFactory.Domain.createDirectoryServiceLink();

    @Before
    public void setUp()
    {
        MockitoAnnotations.openMocks(this);
    }

    @Test
    public void givenServiceLink_whenClear_thenClearAllFields()
    {
        directoryServiceLink.name("Upstream source");
        directoryServiceLink.text("Test message");
        directoryServiceLink.type(EmaRdm.SERVICE_LINK_BROADCAST);
        directoryServiceLink.linkState(EmaRdm.LinkStates.UP);
        directoryServiceLink.linkCode(EmaRdm.SERVICE_LINK_CODE_OK);
        directoryServiceLink.action(MapAction.DELETE);

        assertTrue(directoryServiceLink.checkHasLinkCode());
        assertTrue(directoryServiceLink.checkHasText());
        assertTrue(directoryServiceLink.checkHasType());

        directoryServiceLink.clear();

        assertFalse(directoryServiceLink.checkHasLinkCode());
        assertFalse(directoryServiceLink.checkHasText());
        assertFalse(directoryServiceLink.checkHasType());

        assertEquals("", directoryServiceLink.name());
        assertEquals(EmaRdm.LinkStates.DOWN, directoryServiceLink.linkState());
        assertEquals(MapAction.ADD, directoryServiceLink.action());
        assertEquals(EmaRdm.SERVICE_LINK_ID, directoryServiceLink.filterId());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenTextNotSet_whenGetText_thenThrowException()
    {
        directoryServiceLink.text();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsText_whenSetText_thenThrowException()
    {
        directoryServiceLink.text(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsName_whenSetName_thenThrowException()
    {
        directoryServiceLink.name(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenTypeNotSet_whenGetType_thenThrowException()
    {
        directoryServiceLink.type();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidType_whenSetType_thenThrowException()
    {
        directoryServiceLink.type(0);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidLinkState_whenSetLinkState_thenThrowException()
    {
        directoryServiceLink.linkState(-1);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenLinkCodeNotSet_whenGetLinkCode_thenThrowException()
    {
        directoryServiceLink.linkCode();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidLinkCode_whenSetLinkCode_thenThrowException()
    {
        directoryServiceLink.linkCode(-1);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidActionValue_whenSetAction_thenThrowException()
    {
        directoryServiceLink.action(99);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceLink_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryServiceLink.copy(null);
    }

    @Test
    public void givenServiceLink_whenCopyOtherServiceLink_thenReturnTrueAndUpdateFields()
    {
        DirectoryServiceLink otherDirectoryServiceLink = EmaFactory.Domain.createDirectoryServiceLink();
        otherDirectoryServiceLink.name("Upstream source");
        otherDirectoryServiceLink.text("Test message");
        otherDirectoryServiceLink.type(EmaRdm.SERVICE_LINK_BROADCAST);
        otherDirectoryServiceLink.linkState(EmaRdm.LinkStates.UP);
        otherDirectoryServiceLink.linkCode(EmaRdm.SERVICE_LINK_CODE_OK);
        otherDirectoryServiceLink.action(MapAction.DELETE);


        directoryServiceLink.copy(otherDirectoryServiceLink);
        verify(directoryServiceLink, times(1)).clear();

        assertTrue(directoryServiceLink.checkHasLinkCode());
        assertTrue(directoryServiceLink.checkHasText());
        assertTrue(directoryServiceLink.checkHasType());

        assertEquals("Test message", directoryServiceLink.text());
        assertEquals("Upstream source", directoryServiceLink.name());
        assertEquals(EmaRdm.LinkStates.UP, directoryServiceLink.linkState());
        assertEquals(EmaRdm.SERVICE_LINK_BROADCAST, directoryServiceLink.type());
        assertEquals(EmaRdm.SERVICE_LINK_CODE_OK, directoryServiceLink.linkCode());
        assertEquals(MapAction.DELETE, directoryServiceLink.action());
        assertEquals(EmaRdm.SERVICE_LINK_ID, directoryServiceLink.filterId());
    }

    @Test
    public void givenServiceLinkWithOptionalFields_whenCopyFromSourceWithoutOptionals_thenClearDestinationOptionals()
    {
        directoryServiceLink.name("Existing source");
        directoryServiceLink.text("Existing text");
        directoryServiceLink.type(EmaRdm.SERVICE_LINK_BROADCAST);
        directoryServiceLink.linkState(EmaRdm.LinkStates.UP);
        directoryServiceLink.linkCode(EmaRdm.SERVICE_LINK_CODE_OK);
        directoryServiceLink.action(MapAction.DELETE);

        DirectoryServiceLink otherDirectoryServiceLink = EmaFactory.Domain.createDirectoryServiceLink();
        otherDirectoryServiceLink.name("Updated source");
        otherDirectoryServiceLink.linkState(EmaRdm.LinkStates.DOWN);
        otherDirectoryServiceLink.action(MapAction.ADD);

        directoryServiceLink.copy(otherDirectoryServiceLink);

        assertEquals("Updated source", directoryServiceLink.name());
        assertEquals(EmaRdm.LinkStates.DOWN, directoryServiceLink.linkState());
        assertEquals(MapAction.ADD, directoryServiceLink.action());
        assertFalse(directoryServiceLink.checkHasText());
        assertFalse(directoryServiceLink.checkHasType());
        assertFalse(directoryServiceLink.checkHasLinkCode());
    }

    @Test
    public void givenServiceLink_whenCopyIntoSelf_thenPreserveFields()
    {
        directoryServiceLink.name("Upstream source");
        directoryServiceLink.text("Test message");
        directoryServiceLink.type(EmaRdm.SERVICE_LINK_BROADCAST);
        directoryServiceLink.linkState(EmaRdm.LinkStates.UP);
        directoryServiceLink.linkCode(EmaRdm.SERVICE_LINK_CODE_OK);
        directoryServiceLink.action(MapAction.DELETE);

        directoryServiceLink.copy(directoryServiceLink);
        verify(directoryServiceLink, never()).clear();

        assertEquals("Upstream source", directoryServiceLink.name());
        assertEquals("Test message", directoryServiceLink.text());
        assertEquals(EmaRdm.SERVICE_LINK_BROADCAST, directoryServiceLink.type());
        assertEquals(EmaRdm.LinkStates.UP, directoryServiceLink.linkState());
        assertEquals(EmaRdm.SERVICE_LINK_CODE_OK, directoryServiceLink.linkCode());
        assertEquals(MapAction.DELETE, directoryServiceLink.action());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceLink_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryServiceLink.decode(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceLink_whenDecodeWithoutLinkState_thenThrowException()
    {
        directoryServiceLink.name("Existing source");
        directoryServiceLink.text("Existing text");
        directoryServiceLink.type(EmaRdm.SERVICE_LINK_BROADCAST);
        directoryServiceLink.linkState(EmaRdm.LinkStates.UP);
        directoryServiceLink.linkCode(EmaRdm.SERVICE_LINK_CODE_OK);
        directoryServiceLink.action(MapAction.DELETE);

        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_TYPE, EmaRdm.SERVICE_LINK_BROADCAST));

        directoryServiceLink.decode(createDecodedElementList(encElementList));
    }

    @Test
    public void givenServiceLink_whenDecodeWithMalformedTypeElement_thenClearAndThrowException()
    {
        directoryServiceLink.name("Existing source");
        directoryServiceLink.text("Existing text");
        directoryServiceLink.type(EmaRdm.SERVICE_LINK_BROADCAST);
        directoryServiceLink.linkState(EmaRdm.LinkStates.UP);
        directoryServiceLink.linkCode(EmaRdm.SERVICE_LINK_CODE_OK);
        directoryServiceLink.action(MapAction.DELETE);

        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_TYPE, "badType"));
        encElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_STATE, EmaRdm.LinkStates.UP));

        try
        {
            directoryServiceLink.decode(createDecodedElementList(encElementList));
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertEquals("", directoryServiceLink.name());
            assertEquals(EmaRdm.LinkStates.DOWN, directoryServiceLink.linkState());
            assertEquals(MapAction.ADD, directoryServiceLink.action());
            assertFalse(directoryServiceLink.checkHasText());
            assertFalse(directoryServiceLink.checkHasType());
            assertFalse(directoryServiceLink.checkHasLinkCode());
        }
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceLink_whenDecodeWithInvalidLinkCodeValue_thenThrowException()
    {
        directoryServiceLink.name("Existing source");
        directoryServiceLink.text("Existing text");
        directoryServiceLink.type(EmaRdm.SERVICE_LINK_BROADCAST);
        directoryServiceLink.linkState(EmaRdm.LinkStates.UP);
        directoryServiceLink.linkCode(EmaRdm.SERVICE_LINK_CODE_OK);
        directoryServiceLink.action(MapAction.DELETE);

        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_STATE, EmaRdm.LinkStates.UP));
        encElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_CODE, 99));

        directoryServiceLink.decode(createDecodedElementList(encElementList));
    }

    @Test
    public void givenServiceLink_whenDecodeWithUnknownElementAndBlankText_thenIgnoreUnknownElementAndLeaveTextAbsent()
    {
        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_LINK_STATE, EmaRdm.LinkStates.UP));
        encElementList.add(EmaFactory.createElementEntry().ascii("UNKNOWN_ELEMENT", "ignored"));
        encElementList.add(EmaFactory.createElementEntry().codeAscii(EmaRdm.ENAME_TEXT));

        directoryServiceLink.decode(createDecodedElementList(encElementList));

        assertEquals(EmaRdm.LinkStates.UP, directoryServiceLink.linkState());
        assertFalse(directoryServiceLink.checkHasText());
        assertFalse(directoryServiceLink.checkHasType());
        assertFalse(directoryServiceLink.checkHasLinkCode());
    }

    @Test
    public void givenServiceLink_whenEncodeAndThenDecodeIntoOtherServiceLink_thenFillOtherServiceLinkFields()
    {
        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        int[] supportedLinkCodes = {
                EmaRdm.SERVICE_LINK_CODE_NONE,
                EmaRdm.SERVICE_LINK_CODE_OK,
                EmaRdm.SERVICE_LINK_CODE_RECOVERY_STARTED,
                EmaRdm.SERVICE_LINK_CODE_RECOVERY_COMPLETED
        };

        for (int supportedLinkCode : supportedLinkCodes)
        {
            directoryServiceLink.clear();
            directoryServiceLink.text("Test message");
            directoryServiceLink.type(EmaRdm.SERVICE_LINK_BROADCAST);
            directoryServiceLink.linkState(EmaRdm.LinkStates.UP);
            directoryServiceLink.linkCode(supportedLinkCode);

            ElementList encElementList = directoryServiceLink.encode();
            ElementList decElementList = JUnitTestConnect.createElementList();

            JUnitTestConnect.setRsslData(decElementList, encElementList, Codec.majorVersion(), Codec.minorVersion(),
                    ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

            DirectoryServiceLink otherDirectoryServiceLink = EmaFactory.Domain.createDirectoryServiceLink();
            otherDirectoryServiceLink.decode(decElementList);

            assertTrue(otherDirectoryServiceLink.checkHasLinkCode());
            assertTrue(otherDirectoryServiceLink.checkHasText());
            assertTrue(otherDirectoryServiceLink.checkHasType());

            assertEquals("Test message", otherDirectoryServiceLink.text());
            assertEquals(EmaRdm.LinkStates.UP, otherDirectoryServiceLink.linkState());
            assertEquals(EmaRdm.SERVICE_LINK_BROADCAST, otherDirectoryServiceLink.type());
            assertEquals(supportedLinkCode, otherDirectoryServiceLink.linkCode());
        }
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