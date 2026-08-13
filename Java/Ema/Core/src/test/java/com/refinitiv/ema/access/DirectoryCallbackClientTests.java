/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import org.junit.Before;
import org.junit.Test;
import org.slf4j.Logger;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.Mockito.mock;
import static org.mockito.Mockito.when;

public class DirectoryCallbackClientTests
{
    private static final int SERVICE_ID = 5;
    private static final String SERVICE_NAME = "DIRECT_FEED";

    private OmmBaseImpl<OmmConsumerClient> baseImpl;
    private DirectoryCallbackClient<OmmConsumerClient> directoryCallbackClient;

    @Before
    @SuppressWarnings("unchecked")
    public void setUp()
    {
        baseImpl = mock(OmmBaseImpl.class);
        Logger logger = mock(Logger.class);

        when(baseImpl.activeConfig()).thenReturn(new OmmConsumerActiveConfig());
        when(baseImpl.objManager()).thenReturn(new EmaObjectManager());
        when(baseImpl.loggerClient()).thenReturn(logger);
        when(logger.isTraceEnabled()).thenReturn(false);

        directoryCallbackClient = new DirectoryCallbackClientConsumer(baseImpl);
    }

    @Test
    public void given_directoryItemWithKnownServiceName_when_specifyServiceNameFromId_then_setsMsgServiceNameFromItem() throws Exception
    {
        RefreshMsgImpl refreshMsg = createRefreshMsgWithServiceId();

        DirectoryItem<OmmConsumerClient> item = new DirectoryItem<>(baseImpl, null, null);
        item.serviceName(SERVICE_NAME);

        invokeSpecifyServiceNameFromId(refreshMsg, item);

        assertTrue(refreshMsg.hasServiceName());
        assertEquals(SERVICE_NAME, refreshMsg.serviceName());
        assertEquals(SERVICE_NAME, item.serviceName());
    }

    @Test
    public void given_cachedDirectoryServiceName_when_specifyServiceNameFromId_then_setsMsgAndItemServiceName() throws Exception
    {
        RefreshMsgImpl refreshMsg = createRefreshMsgWithServiceId();

        DirectoryItem<OmmConsumerClient> item = new DirectoryItem<>(baseImpl, null, null);
        serviceById().put(SERVICE_ID, new Directory<>(SERVICE_NAME));

        invokeSpecifyServiceNameFromId(refreshMsg, item);

        assertTrue(refreshMsg.hasServiceName());
        assertEquals(SERVICE_NAME, refreshMsg.serviceName());
        assertEquals(SERVICE_NAME, item.serviceName());
    }

    @Test
    public void given_cachedDirectoryWithoutServiceName_when_specifyServiceNameFromId_then_leavesServiceNameUnset() throws Exception
    {
        RefreshMsgImpl refreshMsg = createRefreshMsgWithServiceId();

        DirectoryItem<OmmConsumerClient> item = new DirectoryItem<>(baseImpl, null, null);
        serviceById().put(SERVICE_ID, new Directory<>((String) null));

        invokeSpecifyServiceNameFromId(refreshMsg, item);

        assertFalse(refreshMsg.hasServiceName());
        assertTrue(item.serviceName().isEmpty());
    }

    @Test
    public void given_noMatchingSource_when_specifyServiceNameFromId_then_leavesServiceNameUnset() throws Exception
    {
        RefreshMsgImpl refreshMsg = createRefreshMsgWithServiceId();

        DirectoryItem<OmmConsumerClient> item = new DirectoryItem<>(baseImpl, null, null);

        invokeSpecifyServiceNameFromId(refreshMsg, item);

        assertFalse(refreshMsg.hasServiceName());
        assertTrue(item.serviceName().isEmpty());
    }

    private void invokeSpecifyServiceNameFromId(MsgImpl msgImpl, SingleItem<OmmConsumerClient> item) throws Exception
    {
        Method method = DirectoryCallbackClient.class.getDeclaredMethod("specifyServiceNameFromId", MsgImpl.class, SingleItem.class);
        method.setAccessible(true);
        method.invoke(directoryCallbackClient, msgImpl, item);
    }

    @SuppressWarnings("unchecked")
    private java.util.Map<Integer, Directory<OmmConsumerClient>> serviceById() throws Exception
    {
        Field field = DirectoryCallbackClient.class.getDeclaredField("_serviceById");
        field.setAccessible(true);
        return (java.util.Map<Integer, Directory<OmmConsumerClient>>) field.get(directoryCallbackClient);
    }


    private RefreshMsgImpl createRefreshMsgWithServiceId()
    {
        RefreshMsgImpl refreshMsg = new RefreshMsgImpl();
        refreshMsg.serviceId(SERVICE_ID);
        return refreshMsg;
    }
}


