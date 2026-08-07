/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryServiceLoad;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.Codec;
import org.junit.Before;
import org.junit.Test;
import org.mockito.MockitoAnnotations;
import org.mockito.Spy;

import static com.refinitiv.ema.access.FilterEntry.*;
import static org.junit.Assert.*;
import static org.mockito.Mockito.*;

public class DirectoryServiceLoadTest
{
    @Spy
    DirectoryServiceLoad directoryServiceLoad = EmaFactory.Domain.createDirectoryServiceLoad();

    @Before
    public void setUp()
    {
        MockitoAnnotations.openMocks(this);
    }

    @Test
    public void givenServiceLoad_whenClear_thenClearAllFields()
    {
        directoryServiceLoad.openLimit(101);
        directoryServiceLoad.openWindow(202);
        directoryServiceLoad.loadFactor(303);
        directoryServiceLoad.action(FilterAction.CLEAR);

        assertTrue(directoryServiceLoad.checkHasOpenLimit());
        assertTrue(directoryServiceLoad.checkHasOpenWindow());
        assertTrue(directoryServiceLoad.checkHasLoadFactor());

        directoryServiceLoad.clear();

        assertFalse(directoryServiceLoad.checkHasOpenLimit());
        assertFalse(directoryServiceLoad.checkHasOpenWindow());
        assertFalse(directoryServiceLoad.checkHasLoadFactor());

        assertEquals(FilterAction.SET, directoryServiceLoad.action());
        assertEquals(EmaRdm.SERVICE_LOAD_ID, directoryServiceLoad.filterId());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenOpenLimitNotSet_whenGetOpenLimit_thenThrowException()
    {
        directoryServiceLoad.openLimit();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenOpenLimitLessThanLowerLimit_whenSetOpenLimit_thenThrowException()
    {
        directoryServiceLoad.openLimit(-1);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenOpenLimitGreaterThanUpperLimit_whenSetOpenLimit_thenThrowException()
    {
        directoryServiceLoad.openLimit(4294967296L);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenOpenWindowNotSet_whenGetOpenWindow_thenThrowException()
    {
        directoryServiceLoad.openWindow();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenOpenWindowLessThanLowerLimit_whenSetOpenLimit_thenThrowException()
    {
        directoryServiceLoad.openWindow(-1);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenOpenWindowGreaterThanUpperLimit_whenSetOpenLimit_thenThrowException()
    {
        directoryServiceLoad.openWindow(4294967296L);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenLoadFactorNotSet_whenGetLoadFactor_thenThrowException()
    {
        directoryServiceLoad.loadFactor();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenLoadFactorLessThanLowerLimit_whenSetOpenLimit_thenThrowException()
    {
        directoryServiceLoad.loadFactor(-1);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenLoadFactorGreaterThanUpperLimit_whenSetOpenLimit_thenThrowException()
    {
        directoryServiceLoad.loadFactor(65536);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenInvalidActionValue_whenSetAction_thenThrowException()
    {
        directoryServiceLoad.action(99);
    }

    @Test
    public void givenServiceLoad_whenCopyOtherServiceLoad_thenReturnTrueAndUpdateFields()
    {
        DirectoryServiceLoad otherDirectoryServiceLoad = EmaFactory.Domain.createDirectoryServiceLoad();
        otherDirectoryServiceLoad.openLimit(101);
        otherDirectoryServiceLoad.openWindow(202);
        otherDirectoryServiceLoad.loadFactor(303);
        otherDirectoryServiceLoad.action(FilterAction.CLEAR);

        directoryServiceLoad.copy(otherDirectoryServiceLoad);
        verify(directoryServiceLoad, times(1)).clear();

        assertTrue(directoryServiceLoad.checkHasOpenLimit());
        assertEquals(101, directoryServiceLoad.openLimit());

        assertTrue(directoryServiceLoad.checkHasOpenWindow());
        assertEquals(202, directoryServiceLoad.openWindow());

        assertTrue(directoryServiceLoad.checkHasLoadFactor());
        assertEquals(303, directoryServiceLoad.loadFactor());

        assertEquals(FilterAction.CLEAR, directoryServiceLoad.action());
        assertEquals(EmaRdm.SERVICE_LOAD_ID, directoryServiceLoad.filterId());
    }

    @Test
    public void givenServiceLoad_whenCopyIntoSelf_thenPreserveFields()
    {
        directoryServiceLoad.openLimit(101);
        directoryServiceLoad.openWindow(202);
        directoryServiceLoad.loadFactor(303);
        directoryServiceLoad.action(FilterAction.CLEAR);

        directoryServiceLoad.copy(directoryServiceLoad);
        verify(directoryServiceLoad, never()).clear();

        assertTrue(directoryServiceLoad.checkHasOpenLimit());
        assertEquals(101, directoryServiceLoad.openLimit());

        assertTrue(directoryServiceLoad.checkHasOpenWindow());
        assertEquals(202, directoryServiceLoad.openWindow());

        assertTrue(directoryServiceLoad.checkHasLoadFactor());
        assertEquals(303, directoryServiceLoad.loadFactor());

        assertEquals(FilterAction.CLEAR, directoryServiceLoad.action());
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceLoad_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryServiceLoad.copy(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceLoad_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryServiceLoad.decode(null);
    }

    @Test
    public void givenServiceLoad_whenDecodeWithMalformedLoadElement_thenClearAndThrowException()
    {
        directoryServiceLoad.openLimit(101);
        directoryServiceLoad.openWindow(202);
        directoryServiceLoad.loadFactor(303);
        directoryServiceLoad.action(FilterAction.CLEAR);

        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().ascii(EmaRdm.ENAME_OPEN_LIMIT, "badType"));

        try
        {
            directoryServiceLoad.decode(createDecodedElementList(encElementList));
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertFalse(directoryServiceLoad.checkHasOpenLimit());
            assertFalse(directoryServiceLoad.checkHasOpenWindow());
            assertFalse(directoryServiceLoad.checkHasLoadFactor());
            assertEquals(FilterAction.SET, directoryServiceLoad.action());
        }
    }

    @Test
    public void givenServiceLoad_whenDecodeWithUnknownElement_thenIgnoreUnknownElement()
    {
        ElementList encElementList = EmaFactory.createElementList();
        encElementList.add(EmaFactory.createElementEntry().uintValue(EmaRdm.ENAME_OPEN_LIMIT, 101));
        encElementList.add(EmaFactory.createElementEntry().ascii("UNKNOWN_ELEMENT", "ignored"));

        DirectoryServiceLoad otherDirectoryServiceLoad = EmaFactory.Domain.createDirectoryServiceLoad();
        otherDirectoryServiceLoad.decode(createDecodedElementList(encElementList));

        assertTrue(otherDirectoryServiceLoad.checkHasOpenLimit());
        assertEquals(101, otherDirectoryServiceLoad.openLimit());
        assertFalse(otherDirectoryServiceLoad.checkHasOpenWindow());
        assertFalse(otherDirectoryServiceLoad.checkHasLoadFactor());
    }

    @Test
    public void givenServiceLoad_whenEncodeAndThenDecodeIntoOtherServiceLoad_thenFillOtherServiceLoadFields()
    {
        // encode
        directoryServiceLoad.openLimit(101);
        directoryServiceLoad.openWindow(202);
        directoryServiceLoad.loadFactor(303);
        directoryServiceLoad.action(FilterAction.CLEAR);

        ElementList encElementList = directoryServiceLoad.encode();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        ElementList decElementList = JUnitTestConnect.createElementList();

        JUnitTestConnect.setRsslData(decElementList, encElementList, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        // decode
        DirectoryServiceLoad otherDirectoryServiceLoad = EmaFactory.Domain.createDirectoryServiceLoad();
        otherDirectoryServiceLoad.decode(decElementList);

        assertTrue(otherDirectoryServiceLoad.checkHasOpenLimit());
        assertEquals(101, otherDirectoryServiceLoad.openLimit());

        assertTrue(otherDirectoryServiceLoad.checkHasOpenWindow());
        assertEquals(202, otherDirectoryServiceLoad.openWindow());

        assertTrue(otherDirectoryServiceLoad.checkHasLoadFactor());
        assertEquals(303, otherDirectoryServiceLoad.loadFactor());
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