/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import org.junit.Before;
import org.junit.Test;

import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.codec.UpdateMsgFlags;

import static org.junit.Assert.assertEquals;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.verify;
import static org.mockito.Mockito.verifyNoInteractions;
import static org.mockito.Mockito.when;

public class ServiceIdConverterTest
{
    public static final int HAS_MSG_KEY = UpdateMsgFlags.HAS_MSG_KEY;
    public static final String SERVICE_NAME = "DIRECT_FEED";
    public static final DirectoryServiceStore.ServiceIdInteger SERVICE_ID = new DirectoryServiceStore.ServiceIdInteger().value(42);
    public static final DirectoryServiceStore.ServiceIdInteger INVALID_SERVICE_ID = new DirectoryServiceStore.ServiceIdInteger().value(Integer.MAX_VALUE);
    private final OmmIProviderDirectoryStore mockDirectoryStore = mock(OmmIProviderDirectoryStore.class);
    private final MsgImpl msg = mock(MsgImpl.class);
    private final MsgKey mockMsgKey = mock(MsgKey.class);
    ServiceIdConverter converter = new ServiceIdConverter(mockDirectoryStore);
    private final Msg mockRsslMessage = mock(Msg.class);

    @Before
    public void setUp()
    {
        when(msg.rsslMsg()).thenReturn(mockRsslMessage);
        when(mockRsslMessage.msgKey()).thenReturn(mockMsgKey);
    }

    @Test
    public void shouldSetServiceIdAndReturnErrorCodeNone()
    {
        when(mockDirectoryStore.serviceId(SERVICE_NAME)).thenReturn(SERVICE_ID);
        when(msg.hasServiceName()).thenReturn(true);
        when(msg.serviceName()).thenReturn(SERVICE_NAME);
        ServiceIdConverter.ServiceIdConversionError result = 
                converter.encodeServiceId(msg, HAS_MSG_KEY);

        assertEquals(ServiceIdConverter.ServiceIdConversionError.NONE, result);
        verify(mockMsgKey).serviceId(SERVICE_ID.value());
        verify(mockMsgKey).applyHasServiceId();
        verify(mockRsslMessage).flags(HAS_MSG_KEY);
    }

    @Test
    public void shouldReturnErrorCodeNoneWhenServiceNameAndIdAreMissing()
    {
        ServiceIdConverter.ServiceIdConversionError result =
                converter.encodeServiceId(msg, HAS_MSG_KEY);

        assertEquals(ServiceIdConverter.ServiceIdConversionError.NONE, result);
        verifyNoInteractions(mockMsgKey);
    }

    @Test
    public void shouldReturnErrorWhenServiceIdIsMissingForName()
    {
        when(msg.hasServiceName()).thenReturn(true);
        when(msg.serviceName()).thenReturn(SERVICE_NAME);
        ServiceIdConverter.ServiceIdConversionError result =
                converter.encodeServiceId(msg, HAS_MSG_KEY);
        
        assertEquals(ServiceIdConverter.ServiceIdConversionError.ID_IS_MISSING_FOR_NAME, result); 
        verifyNoInteractions(mockMsgKey);
    }

    @Test
    public void shouldReturnErrorWhenServiceIdIsInvalidForName()
    {
        when(mockDirectoryStore.serviceId(SERVICE_NAME))
                .thenReturn(INVALID_SERVICE_ID);
        when(msg.hasServiceName()).thenReturn(true);
        when(msg.serviceName()).thenReturn(SERVICE_NAME);
        ServiceIdConverter.ServiceIdConversionError result =
                converter.encodeServiceId(msg, HAS_MSG_KEY);

        assertEquals(ServiceIdConverter.ServiceIdConversionError.ID_IS_INVALID_FOR_NAME, result);
        verifyNoInteractions(mockMsgKey);
    }

    @Test
    public void shouldReturnErrorWhenServiceIdHasNoNameInDirectory()
    {
        when(msg.hasServiceId()).thenReturn(true);
        when(msg.serviceId()).thenReturn(SERVICE_ID.value());
        ServiceIdConverter.ServiceIdConversionError result =
                converter.encodeServiceId(msg, HAS_MSG_KEY);

        assertEquals(ServiceIdConverter.ServiceIdConversionError.NAME_IS_MISSING_FOR_ID, result);
        verifyNoInteractions(mockMsgKey);
    }

    @Test
    public void shouldReturnErrorWhenServiceIdIsInvalid()
    {
        when(msg.hasServiceId()).thenReturn(true);
        when(msg.serviceId()).thenReturn(INVALID_SERVICE_ID.value());
        ServiceIdConverter.ServiceIdConversionError result =
                converter.encodeServiceId(msg, HAS_MSG_KEY);

        assertEquals(ServiceIdConverter.ServiceIdConversionError.USER_DEFINED_ID_INVALID, result);
        verifyNoInteractions(mockMsgKey);
    }
}