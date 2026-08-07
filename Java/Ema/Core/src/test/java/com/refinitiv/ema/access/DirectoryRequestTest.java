/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.ema.domain.directory.DirectoryRequest;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.ema.rdm.EmaRdm;
import com.refinitiv.ema.unittest.TestUtilities;
import com.refinitiv.eta.codec.Codec;
import org.junit.Test;

import static org.junit.Assert.*;

public class DirectoryRequestTest
{
    DirectoryRequest directoryRequest = EmaFactory.Domain.createDirectoryRequest();

    @Test
    public void givenDirectoryRequest_whenClear_thenSetAllFieldsToDefaultValues()
    {
        final int allFiltersSet = EmaRdm.SERVICE_GROUP_FILTER | EmaRdm.SERVICE_LOAD_FILTER | EmaRdm.SERVICE_DATA_FILTER
                | EmaRdm.SERVICE_LINK_FILTER | EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_STATE_FILTER;
        setDirectoryRequest(directoryRequest, true);

        directoryRequest.initialImage(false);

        assertFalse(directoryRequest.checkHasServiceId());
        assertTrue(directoryRequest.checkHasServiceName());
        assertFalse(directoryRequest.initialImage());
        assertTrue(directoryRequest.interestAfterRefresh());

        directoryRequest.clear();

        assertFalse(directoryRequest.checkHasServiceId());
        assertFalse(directoryRequest.checkHasServiceName());
        assertTrue(directoryRequest.initialImage());
        assertTrue(directoryRequest.interestAfterRefresh());

        assertEquals(EmaRdm.MMT_DIRECTORY, directoryRequest.domainType());
        assertEquals(allFiltersSet, directoryRequest.filter());
        assertEquals(-1, directoryRequest.streamId());
    }

    @Test
    public void givenDirectoryRequest_whenUsingFluentApi_thenReturnSameInstance()
    {
        DirectoryRequest otherDirectoryRequest = EmaFactory.Domain.createDirectoryRequest();
        setDirectoryRequest(otherDirectoryRequest, false);
        ReqMsg decRequestMsg = decodeRequestMsg(otherDirectoryRequest.message());

        DirectoryRequest returned = directoryRequest.clear()
                .streamId(7)
                .filter(EmaRdm.SERVICE_GROUP_FILTER)
                .serviceName("Fluent service")
                .initialImage(false)
                .interestAfterRefresh(true);

        assertSame(directoryRequest, returned);
        assertSame(directoryRequest, directoryRequest.copy(otherDirectoryRequest));
        assertSame(directoryRequest, directoryRequest.message(decRequestMsg));
        checkDirectoryRequest(directoryRequest, false);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenNullAsServiceName_whenSetServiceName_thenThrowException()
    {
        directoryRequest.serviceName(null);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceNameNotSet_whenGetServiceName_thenThrowException()
    {
        directoryRequest.serviceName();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceIdNotSet_whenGetServiceId_thenThrowException()
    {
        directoryRequest.serviceId();
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenDirectoryRequest_whenCopyWithNullAsParameter_thenThrowException()
    {
        directoryRequest.copy(null);
    }

    @Test
    public void givenDirectoryRequest_whenCopyOtherDirectoryRequest_thenUpdateFields()
    {
        DirectoryRequest otherDirectoryRequest = EmaFactory.Domain.createDirectoryRequest();
        setDirectoryRequest(otherDirectoryRequest, true);

        directoryRequest.copy(otherDirectoryRequest);

        checkDirectoryRequest(directoryRequest, true);
    }

    @Test
    public void givenDirectoryRequestWithServiceId_whenCopyOtherDirectoryRequest_thenUpdateFields()
    {
        DirectoryRequest otherDirectoryRequest = EmaFactory.Domain.createDirectoryRequest();
        setDirectoryRequest(otherDirectoryRequest, false);

        directoryRequest.copy(otherDirectoryRequest);

        checkDirectoryRequest(directoryRequest, false);
    }

    @Test
    public void givenDirectoryRequest_whenCopyIntoSelf_thenPreserveFields()
    {
        setDirectoryRequest(directoryRequest, true);

        directoryRequest.copy(directoryRequest);

        checkDirectoryRequest(directoryRequest, true);
    }

    @Test(expected = OmmInvalidUsageExceptionImpl.class)
    public void givenServiceLink_whenDecodeWithNullAsParameter_thenThrowException()
    {
        directoryRequest.message(null);
    }

    @Test
    public void givenReqMsgWithWrongDomain_whenDecode_thenThrowExceptionWithoutChangingState()
    {
        setDirectoryRequest(directoryRequest, true);

        ReqMsg wrongDomainMsg = EmaFactory.createReqMsg();
        wrongDomainMsg.domainType(EmaRdm.MMT_LOGIN);
        wrongDomainMsg.streamId(13);

        try
        {
            directoryRequest.message(wrongDomainMsg);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            checkDirectoryRequest(directoryRequest, true);
        }
    }

    @Test
    public void givenServiceNameAlreadySet_whenSetServiceId_thenThrowExceptionAndPreserveServiceName()
    {
        directoryRequest.serviceName("Test service name");

        try
        {
            directoryRequest.serviceId(101);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertTrue(directoryRequest.checkHasServiceName());
            assertFalse(directoryRequest.checkHasServiceId());
            assertEquals("Test service name", directoryRequest.serviceName());
        }
    }

    @Test
    public void givenServiceIdAlreadySet_whenSetServiceName_thenThrowExceptionAndPreserveServiceId()
    {
        directoryRequest.serviceId(101);

        try
        {
            directoryRequest.serviceName("Test service name");
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertTrue(directoryRequest.checkHasServiceId());
            assertFalse(directoryRequest.checkHasServiceName());
            assertEquals(101, directoryRequest.serviceId());
        }
    }

    @Test
    public void givenBoundaryServiceIds_whenSetServiceId_thenAcceptValidValues()
    {
        directoryRequest.serviceId(0);
        assertEquals(0, directoryRequest.serviceId());

        directoryRequest.clear();

        directoryRequest.serviceId(65535);
        assertEquals(65535, directoryRequest.serviceId());
    }

    @Test
    public void givenOutOfRangeServiceId_whenSetServiceId_thenThrowExceptionAndLeaveSelectorAbsent()
    {
        try
        {
            directoryRequest.serviceId(-1);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertFalse(directoryRequest.checkHasServiceId());
        }

        try
        {
            directoryRequest.serviceId(65536);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertFalse(directoryRequest.checkHasServiceId());
        }
    }

    @Test
    public void givenDirectoryRequest_whenEncodeAndThenDecodeIntoOtherDirectoryRequest_thenFillOtherDirectoryRequestFields()
    {
        // encode
        setDirectoryRequest(directoryRequest, false);
        ReqMsg encRequestMsg = directoryRequest.message();

        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        ReqMsg decRequestMsg = JUnitTestConnect.createReqMsg();

        JUnitTestConnect.setRsslData(decRequestMsg, encRequestMsg, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        // decode
        DirectoryRequest otherDirectoryRequest = EmaFactory.Domain.createDirectoryRequest();
        otherDirectoryRequest.message(decRequestMsg);

        checkDirectoryRequest(otherDirectoryRequest, false);
    }

    @Test
    public void givenDirectoryRequestWithServiceName_whenCreateReqMsgViewAndDecodeIntoOtherDirectoryRequest_thenFillOtherDirectoryRequestFields()
    {
        setDirectoryRequest(directoryRequest, true);

        ReqMsg encRequestMsg = directoryRequest.message();
        assertTrue(encRequestMsg.hasServiceName());
        assertEquals("Test service name", encRequestMsg.serviceName());
        assertFalse(encRequestMsg.hasServiceId());

        DirectoryRequest otherDirectoryRequest = EmaFactory.Domain.createDirectoryRequest();
        otherDirectoryRequest.message(encRequestMsg);

        checkDirectoryRequest(otherDirectoryRequest, true);
    }

    @Test
    public void givenDirectoryRequestWithoutSelectorAndFlagsDisabled_whenEncodeAndDecode_thenPreserveAllServicesRequest()
    {
        directoryRequest.streamId(7);
        directoryRequest.filter(EmaRdm.SERVICE_GROUP_FILTER | EmaRdm.SERVICE_STATE_FILTER);
        directoryRequest.initialImage(false);
        directoryRequest.interestAfterRefresh(false);

        ReqMsg encRequestMsg = directoryRequest.message();
        ReqMsg decRequestMsg = decodeRequestMsg(encRequestMsg);

        DirectoryRequest otherDirectoryRequest = EmaFactory.Domain.createDirectoryRequest();
        otherDirectoryRequest.message(decRequestMsg);

        assertFalse(otherDirectoryRequest.checkHasServiceId());
        assertFalse(otherDirectoryRequest.checkHasServiceName());
        assertFalse(otherDirectoryRequest.initialImage());
        assertFalse(otherDirectoryRequest.interestAfterRefresh());
        assertEquals(EmaRdm.SERVICE_GROUP_FILTER | EmaRdm.SERVICE_STATE_FILTER, otherDirectoryRequest.filter());
        assertEquals(7, otherDirectoryRequest.streamId());
    }

    @Test
    public void givenClearedDirectoryRequest_whenCreateReqMsgView_thenInterestAfterRefreshDefaultsToTrue()
    {
        ReqMsg requestMsg = directoryRequest.clear().message();

        assertTrue(directoryRequest.initialImage());
        assertTrue(directoryRequest.interestAfterRefresh());
        assertTrue(requestMsg.initialImage());
        assertTrue(requestMsg.interestAfterRefresh());
    }

    @Test
    public void givenReqMsgWithBothServiceSelectors_whenDecode_thenThrowException()
    {
        ReqMsg reqMsg = EmaFactory.createReqMsg();
        reqMsg.domainType(EmaRdm.MMT_DIRECTORY);
        reqMsg.streamId(5);
        reqMsg.serviceId(101);
        reqMsg.serviceName("Test service name");

        try
        {
            directoryRequest.message(reqMsg);
            fail("Expected OmmInvalidUsageExceptionImpl");
        }
        catch (OmmInvalidUsageExceptionImpl ex)
        {
            assertTrue(directoryRequest.checkHasServiceId());
            assertFalse(directoryRequest.checkHasServiceName());
            assertEquals(101, directoryRequest.serviceId());
        }
    }

    private ReqMsg decodeRequestMsg(ReqMsg encRequestMsg)
    {
        DataDictionary dictionary = EmaFactory.createDataDictionary();
        dictionary.loadFieldDictionary(TestUtilities.getFieldDictionaryFileName());
        dictionary.loadEnumTypeDictionary(TestUtilities.getEnumTableFileName());

        ReqMsg decRequestMsg = JUnitTestConnect.createReqMsg();

        JUnitTestConnect.setRsslData(decRequestMsg, encRequestMsg, Codec.majorVersion(), Codec.minorVersion(),
                ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);

        return decRequestMsg;
    }

    private void setDirectoryRequest(DirectoryRequest directoryRequest, boolean withServiceName)
    {
        directoryRequest.streamId(5);
        if (withServiceName)
        {
            directoryRequest.serviceName("Test service name");
        }
        else
        {
            directoryRequest.serviceId(101);
        }
        directoryRequest.filter(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_DATA_FILTER);
        directoryRequest.interestAfterRefresh(true);
    }

    private void checkDirectoryRequest(DirectoryRequest directoryRequest, boolean withServiceName)
    {
        assertTrue(directoryRequest.initialImage());
        assertTrue(directoryRequest.interestAfterRefresh());

        if (withServiceName)
        {
            assertTrue(directoryRequest.checkHasServiceName());
            assertEquals("Test service name", directoryRequest.serviceName());
        }
        else
        {
            assertTrue(directoryRequest.checkHasServiceId());
            assertEquals(101, directoryRequest.serviceId());
        }

        assertEquals(EmaRdm.SERVICE_INFO_FILTER | EmaRdm.SERVICE_DATA_FILTER, directoryRequest.filter());
        assertEquals(5, directoryRequest.streamId());
    }
}