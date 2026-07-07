/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.valueadd.reactor.ReactorChannel;
import com.refinitiv.eta.valueadd.reactor.ReactorChannelEvent;
import org.junit.Test;
import org.mockito.ArgumentCaptor;
import org.slf4j.Logger;

import java.lang.reflect.Field;
import java.nio.channels.ClosedSelectorException;
import java.nio.channels.Selector;
import java.util.Collections;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertTrue;
import static org.mockito.ArgumentMatchers.anyLong;
import static org.mockito.Mockito.*;

public class ReactorDispatchLoopTimeoutTests {

    @Test
    public void ommBaseDispatchInfiniteWaitWithoutUserTimeoutUsesSelectorInfiniteSelect() throws Exception {
        Selector selector = mock(Selector.class);
        Logger logger = mock(Logger.class);

        when(selector.selectedKeys()).thenReturn(Collections.emptySet());
        when(selector.select()).thenThrow(new ClosedSelectorException());
        when(logger.isTraceEnabled()).thenReturn(false);

        TestOmmBaseImpl impl = new TestOmmBaseImpl();
        impl.setState(OmmBaseImpl.OmmImplState.RSSLCHANNEL_UP);
        impl.setSelector(selector);
        impl.setLogger(logger);

        assertTrue(impl.rsslReactorDispatchLoop(OmmConsumer.DispatchTimeout.INFINITE_WAIT, 1));

        verify(selector, times(1)).select();
        verify(selector, never()).select(anyLong());
    }

    @Test
    public void ommBaseDispatchInfiniteWaitWithUserTimeoutUsesTimedSelectorSelect() throws Exception {
        Selector selector = mock(Selector.class);
        Logger logger = mock(Logger.class);

        when(selector.selectedKeys()).thenReturn(Collections.emptySet());
        when(selector.select(anyLong())).thenThrow(new ClosedSelectorException());
        when(logger.isTraceEnabled()).thenReturn(false);

        TestOmmBaseImpl impl = new TestOmmBaseImpl();
        impl.setState(OmmBaseImpl.OmmImplState.RSSLCHANNEL_UP);
        impl.setSelector(selector);
        impl.setLogger(logger);
        impl.addFutureUserTimeoutEvent();

        assertTrue(impl.rsslReactorDispatchLoop(OmmConsumer.DispatchTimeout.INFINITE_WAIT, 1));

        verify(selector, never()).select();
        verify(selector, atLeastOnce()).select(anyLong());
    }

    @Test
    public void ommBaseDispatchWaitTimeOneUsesTimedSelectorSelect() throws Exception {
        Selector selector = mock(Selector.class);
        Logger logger = mock(Logger.class);

        when(selector.selectedKeys()).thenReturn(Collections.emptySet());
        when(selector.select(anyLong())).thenThrow(new ClosedSelectorException());
        when(logger.isTraceEnabled()).thenReturn(false);

        TestOmmBaseImpl impl = new TestOmmBaseImpl();
        impl.setState(OmmBaseImpl.OmmImplState.RSSLCHANNEL_UP);
        impl.setSelector(selector);
        impl.setLogger(logger);

        assertTrue(impl.rsslReactorDispatchLoop(1, 1));

        verify(selector, never()).select();
        verify(selector, atLeastOnce()).select(1L);
    }

    @Test
    public void ommBaseDispatchHighWaitTimeUsesTimedSelectorSelect() throws Exception {
        Selector selector = mock(Selector.class);
        Logger logger = mock(Logger.class);

        when(selector.selectedKeys()).thenReturn(Collections.emptySet());
        when(selector.select(anyLong())).thenThrow(new ClosedSelectorException());
        when(logger.isTraceEnabled()).thenReturn(false);

        TestOmmBaseImpl impl = new TestOmmBaseImpl();
        impl.setState(OmmBaseImpl.OmmImplState.RSSLCHANNEL_UP);
        impl.setSelector(selector);
        impl.setLogger(logger);

        long highTimeout = 5_000_000L;
        assertTrue(impl.rsslReactorDispatchLoop(highTimeout, 1));

        verify(selector, never()).select();

        ArgumentCaptor<Long> selectTimeoutCaptor = ArgumentCaptor.forClass(Long.class);
        verify(selector, atLeastOnce()).select(selectTimeoutCaptor.capture());
        List<Long> capturedTimeouts = selectTimeoutCaptor.getAllValues();
        assertFalse(capturedTimeouts.isEmpty());

        long firstTimeout = capturedTimeouts.get(0);
        assertTrue(firstTimeout <= 5000L);
        assertTrue(firstTimeout >= 1L);
    }

    @Test
    public void ommServerDispatchInfiniteWaitWithoutUserTimeoutUsesSelectorInfiniteSelect() throws Exception {
        Selector selector = mock(Selector.class);
        Logger logger = mock(Logger.class);

        when(selector.selectedKeys()).thenReturn(Collections.emptySet());
        when(selector.select()).thenThrow(new ClosedSelectorException());
        when(logger.isTraceEnabled()).thenReturn(false);

        TestOmmServerBaseImpl impl = new TestOmmServerBaseImpl();
        impl.setState(OmmServerBaseImpl.OmmImplState.REACTOR_INITIALIZED);
        impl.setSelector(selector);
        impl.setLogger(logger);

        assertTrue(impl.rsslReactorDispatchLoop(OmmConsumer.DispatchTimeout.INFINITE_WAIT, 1));

        verify(selector, times(1)).select();
        verify(selector, never()).select(anyLong());
    }

    @Test
    public void ommServerDispatchWaitTimeOneUsesTimedSelectorSelect() throws Exception {
        Selector selector = mock(Selector.class);
        Logger logger = mock(Logger.class);

        when(selector.selectedKeys()).thenReturn(Collections.emptySet());
        when(selector.selectNow()).thenThrow(new ClosedSelectorException());
        when(logger.isTraceEnabled()).thenReturn(false);

        TestOmmServerBaseImpl impl = new TestOmmServerBaseImpl();
        impl.setState(OmmServerBaseImpl.OmmImplState.REACTOR_INITIALIZED);
        impl.setSelector(selector);
        impl.setLogger(logger);

        assertTrue(impl.rsslReactorDispatchLoop(1, 1));

        verify(selector, atLeastOnce()).selectNow();
        verify(selector, never()).select();
        verify(selector, never()).select(anyLong());
    }

    @Test
    public void ommServerDispatchHighWaitTimeUsesTimedSelectorSelect() throws Exception {
        Selector selector = mock(Selector.class);
        Logger logger = mock(Logger.class);

        when(selector.selectedKeys()).thenReturn(Collections.emptySet());
        when(selector.select(anyLong())).thenThrow(new ClosedSelectorException());
        when(logger.isTraceEnabled()).thenReturn(false);

        TestOmmServerBaseImpl impl = new TestOmmServerBaseImpl();
        impl.setState(OmmServerBaseImpl.OmmImplState.REACTOR_INITIALIZED);
        impl.setSelector(selector);
        impl.setLogger(logger);

        long highTimeout = 5_000_000L;
        assertTrue(impl.rsslReactorDispatchLoop(highTimeout, 1));

        verify(selector, never()).select();

        ArgumentCaptor<Long> selectTimeoutCaptor = ArgumentCaptor.forClass(Long.class);
        verify(selector, atLeastOnce()).select(selectTimeoutCaptor.capture());
        List<Long> capturedTimeouts = selectTimeoutCaptor.getAllValues();
        assertFalse(capturedTimeouts.isEmpty());

        long firstTimeout = capturedTimeouts.get(0);
        assertTrue(firstTimeout <= 5000L);
        assertTrue(firstTimeout >= 1L);
    }

    private static final class TestOmmBaseImpl extends OmmBaseImpl<OmmConsumerClient> implements TimeoutClient {

        private final ReentrantLock lock = new ReentrantLock();
        private final Logger testLogger = mock(Logger.class);
        private final StringBuilder builder = new StringBuilder();

        @Override
        public void handleInvalidUsage(String text, int errorCode) {}

        @Override
        public void handleInvalidHandle(long handle, String text) {}

        @Override
        public void handleJsonConverterError(ReactorChannel reactorChannel, int errorCode, String text) {}

        @Override
        public Logger loggerClient() { return testLogger; }

        @Override
        public String formatLogMessage(String clientName, String temp, int level) { return temp; }

        @Override
        public EmaObjectManager objManager() { return _objManager; }

        @Override
        public String instanceName() { return "TestInstance"; }

        @Override
        public StringBuilder strBuilder() { return builder; }

        @Override
        public void eventReceived() {}

        @Override
        Logger createLoggerClient() { return mock(Logger.class); }

        @Override
        void readCustomConfig(EmaConfigImpl config) {}

        @Override
        boolean hasErrorClient() { return false; }

        @Override
        void notifyErrorClient(OmmException ommException) {}

        @Override
        void onDispatchError(String text, int errorCode) {}

        @Override
        ConfigManager.ConfigAttributes getAttributes(EmaConfigImpl config) { return null; }

        @Override
        Object getAttributeValue(EmaConfigImpl config, int AttributeKey) { return null; }

        @Override
        public void channelInformation(ChannelInformation ci) {}

        @Override
        public long nextLongId() { return 1L; }

        @Override
        public int implType() { return OmmCommonImpl.ImplementationType.CONSUMER; }

        @Override
        void handleAdminDomains(EmaConfigImpl config) {}

        @Override
        public void handleTimeoutEvent() {}

        @Override
        public ReentrantLock userLock() { return lock; }

        void setState(int state) { ommImplState(state); }

        void addFutureUserTimeoutEvent() { _timeoutEventQueue.add(new TimeoutEvent(5_000_000L, this)); }

        void setSelector(Selector selector) throws Exception { setPrivateField(OmmBaseImpl.class, "_selector", selector); }

        void setLogger(Logger logger) throws Exception { setPrivateField(OmmBaseImpl.class, "_loggerClient", logger); }

        private void setPrivateField(Class<?> type, String fieldName, Object value) throws Exception {
            Field field = type.getDeclaredField(fieldName);
            field.setAccessible(true);
            field.set(this, value);
        }
    }

    private static final class TestOmmServerBaseImpl extends OmmServerBaseImpl {

        TestOmmServerBaseImpl() {
            super(mock(OmmProviderClient.class), null);
        }

        @Override
        OmmProvider provider() { return null; }

        @Override
        Logger createLoggerClient() { return mock(Logger.class); }

        @Override
        ConfigManager.ConfigAttributes getAttributes(EmaConfigServerImpl config) { return null; }

        @Override
        Object getAttributeValue(EmaConfigServerImpl config, int AttributeKey) { return null; }

        @Override
        void readCustomConfig(EmaConfigServerImpl config) {}

        @Override
        DirectoryServiceStore directoryServiceStore() { return null; }

        @Override
        void processChannelEvent(ReactorChannelEvent reactorChannelEvent) {}

        @Override
        public String formatLogMessage(String clientName, String temp, int level) { return temp; }

        @Override
        public EmaObjectManager objManager() { return _objManager; }

        @Override
        public String instanceName() { return "TestServerInstance"; }

        @Override
        public int implType() { return OmmCommonImpl.ImplementationType.IPROVIDER; }

        @Override
        public long nextLongId() { return 1L; }

        @Override
        public void channelInformation(ChannelInformation ci) {}

        void setState(int state) { _state = state; }

        void setSelector(Selector selector) throws Exception { setPrivateField("_selector", selector); }

        void setLogger(Logger logger) throws Exception { setPrivateField("_loggerClient", logger); }

        private void setPrivateField(String fieldName, Object value) throws Exception {
            Field field = OmmServerBaseImpl.class.getDeclaredField(fieldName);
            field.setAccessible(true);
            field.set(this, value);
        }
    }
}



