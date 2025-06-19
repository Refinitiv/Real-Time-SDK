/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import static org.junit.Assert.*;
import static org.mockito.Mockito.when;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.nio.channels.spi.SelectorProvider;
import java.util.Iterator;
import java.util.Set;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import com.refinitiv.eta.valueadd.domainrep.rdm.login.*;
import org.junit.Test;
import org.mockito.Mockito;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.ParseHexFile;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.InstrumentNameTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.test.network.replay.NetworkReplay;
import com.refinitiv.eta.test.network.replay.NetworkReplayFactory;
import com.refinitiv.eta.transport.BindOptions;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelInfo;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.CompressionTypes;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.ReadArgs;
import com.refinitiv.eta.transport.Server;
import com.refinitiv.eta.transport.TestClient;
import com.refinitiv.eta.transport.TestServer;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WriteArgs;
import com.refinitiv.eta.transport.ReadArgsImpl;
import com.refinitiv.eta.transport.WriteFlags;
import com.refinitiv.eta.transport.WritePriorities;
import com.refinitiv.eta.valueadd.common.SelectableBiDirectionalQueue;
import com.refinitiv.eta.valueadd.common.VaNode;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;

public class ReactorJunit
{
    static int _serverPort = 15400;

    final static int CONNECT_TIMEOUT_SECONDS = 10; // 10 seconds
    final static int SELECT_TIME = 100; // 100 millisecond

    final static String LOCAL_ADDRESS = "localhost";
    final static String BASE_RIPC_TEST_DATA_DIR_NAME = "../Core/src/test/resources/com/refinitiv/eta/transport/RipcHandshakeJunit";
    final static String BASE_TEST_DATA_DIR_NAME = "src/test/resources/com/refinitiv/eta/valueadd/ReactorJunit";
    private static final String BASE_PACK_TEST_DATA_DIR_NAME = "../Core/src/test/resources/com/refinitiv/eta/transport/SocketChannelJunit";

    private final int KEY_EXCHANGE = 8;

    /*
     * Inner class to handle default callbacks. It simply stores the event to be
     * retrieved later.
     */
    class ReactorCallbackHandler implements DefaultMsgCallback, ReactorChannelEventCallback,
            RDMLoginMsgCallback, RDMDirectoryMsgCallback, RDMDictionaryMsgCallback
    {
        Selector _selector = null;
        ReactorChannelEvent _lastChannelEvent = null;
        ReactorChannel _lastReactorChannel = null;
        ReactorMsgEvent _lastDefaultMsgEvent = null;
        RDMLoginMsgEvent _lastLoginMsgEvent = null;
        RDMDirectoryMsgEvent _lastDirectoryMsgEvent = null;
        RDMDictionaryMsgEvent _lastDictionaryMsgEvent = null;
        int _channelEventCount = 0;
        int _channelUpEventCount = 0;
        int _channelReadyEventCount = 0;
        int _channelDownEventCount = 0;
        int _defaultMsgEventCount = 0;
        int _loginMsgEventCount = 0;
        int _directoryMsgEventCount = 0;
        int _dictionaryMsgEventCount = 0;
        int _channelFDChangeEventCount = 0;
        int _channelOpenedEventCount = 0;

        // These are the return codes that will be returned from the callbacks.
        // they can be overrided to trigger different reactor behaviors.
        int _channelReturnCode = ReactorCallbackReturnCodes.SUCCESS;
        int _msgReturnCode = ReactorCallbackReturnCodes.SUCCESS;

        ReactorCallbackHandler(Selector selector)
        {
            _selector = selector;
        }

        ReactorChannelEvent lastChannelEvent()
        {
            ReactorChannelEvent event = _lastChannelEvent;
            _lastChannelEvent = null;
            return event;
        }
        
        ReactorChannel lastReactorChannel()
        {
            ReactorChannel reactorChannel = _lastReactorChannel;
            _lastReactorChannel = null;
            return reactorChannel;
        }

        ReactorMsgEvent lastDefaultMsgEvent()
        {
            ReactorMsgEvent event = _lastDefaultMsgEvent;
            _lastDefaultMsgEvent = null;
            return event;
        }

        RDMLoginMsgEvent lastLoginMsgEvent()
        {
            RDMLoginMsgEvent event = _lastLoginMsgEvent;
            _lastLoginMsgEvent = null;
            return event;
        }

        RDMDirectoryMsgEvent lastDirectoryMsgEvent()
        {
            RDMDirectoryMsgEvent event = _lastDirectoryMsgEvent;
            _lastDirectoryMsgEvent = null;
            return event;
        }

        RDMDictionaryMsgEvent lastDictionaryMsgEvent()
        {
            RDMDictionaryMsgEvent event = _lastDictionaryMsgEvent;
            _lastDictionaryMsgEvent = null;
            return event;
        }

        int channelEventCount()
        {
            return _channelEventCount;
        }

        int channelUpEventCount()
        {
            return _channelUpEventCount;
        }

        int channelReadyEventCount()
        {
            return _channelReadyEventCount;
        }

        int channelDownEventCount()
        {
            return _channelDownEventCount;
        }

        int defaultMsgEventCount()
        {
            return _defaultMsgEventCount;
        }

        int loginMsgEventCount()
        {
            return _loginMsgEventCount;
        }

        int directoryMsgEventCount()
        {
            return _directoryMsgEventCount;
        }

        int dictionaryMsgEventCount()
        {
            return _dictionaryMsgEventCount;
        }

        int channelFDChangeEventCount()
        {
            return _channelFDChangeEventCount;
        }

        int channelOpenedEventCount()
        {
            return _channelOpenedEventCount;
        }

        void msgReturnCode(int retCode)
        {
            _msgReturnCode = retCode;
        }

        void channelReturnCode(int retCode)
        {
            _channelReturnCode = retCode;
        }

        @Override
        public int defaultMsgCallback(ReactorMsgEvent event)
        {
            System.out.println("DEBUG: ReactorCallbackHandler.defaultMsgCallback: entered. defaultMsgEventCount="
                            + ++_defaultMsgEventCount + " event=" + event.toString());

            _lastDefaultMsgEvent = new ReactorMsgEvent();
            TestUtil.copyMsgEvent(event, _lastDefaultMsgEvent);

            return ReactorCallbackReturnCodes.SUCCESS;
        }

        @Override
        public int reactorChannelEventCallback(ReactorChannelEvent event)
        {
            System.out.println("DEBUG: ReactorCallbackHandler.reactorChannelEventCallback: entered. channelEventCount="
                            + ++_channelEventCount + " event=" + event.toString());

            _lastChannelEvent = event;
            _lastReactorChannel = event.reactorChannel();

            ReactorChannel rc = event.reactorChannel();
            int eventType = event.eventType();

            switch (eventType)
            {
                case ReactorChannelEventTypes.CHANNEL_UP:
                    _channelUpEventCount++;
                    // register this new reactorChannel for OP_READ with the
                    // selector
                    try
                    {
                        rc.channel().selectableChannel().register(_selector, SelectionKey.OP_READ, rc);
                    }
                    catch (ClosedChannelException e)
                    {
                        assertTrue("Exception occurred, " + e.getLocalizedMessage(), false);
                    }
                    break;
                case ReactorChannelEventTypes.CHANNEL_DOWN:
                case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
                    _channelDownEventCount++;
                    if (rc.selectableChannel() != null)
                    {
                        SelectionKey key = rc.selectableChannel().keyFor(_selector);
                        if (key != null)
                            key.cancel();
                    }
                    break;
                case ReactorChannelEventTypes.CHANNEL_READY:
                    _channelReadyEventCount++;
                    break;
                case ReactorChannelEventTypes.FD_CHANGE:
                    _channelFDChangeEventCount++;
                    break;
                case ReactorChannelEventTypes.CHANNEL_OPENED:
                    _channelOpenedEventCount++;
                    break;
                default:
                    assertTrue("This test currently doesn't handle this event", false);
            }

            return _channelReturnCode;
        }

        @Override
        public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
        {
            System.out.println("DEBUG: ReactorCallbackHandler.rdmLoginMsgCallback: entered. loginMsgEventCount="
                            + ++_loginMsgEventCount + " event=" + event.toString());

            _lastLoginMsgEvent = new RDMLoginMsgEvent();
            TestUtil.copyMsgEvent(event, _lastLoginMsgEvent);

            if (event.rdmLoginMsg() != null)
            {
                _lastLoginMsgEvent.rdmLoginMsg(LoginMsgFactory.createMsg());
                TestUtil.copyLoginMsg(event.rdmLoginMsg(), _lastLoginMsgEvent.rdmLoginMsg());
            }

            return _msgReturnCode;
        }

        @Override
        public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
        {
            System.out.println("DEBUG: ReactorCallbackHandler.rdmDirectoryMsgCallback: entered. directoryMsgEventCount="
                            + ++_directoryMsgEventCount + " event=" + event.toString());

            _lastDirectoryMsgEvent = new RDMDirectoryMsgEvent();
            TestUtil.copyMsgEvent(event, _lastDirectoryMsgEvent);

            if (event.rdmDirectoryMsg() != null)
            {
                _lastDirectoryMsgEvent.rdmDirectoryMsg(DirectoryMsgFactory.createMsg());
                TestUtil.copyDirectoryMsg(event.rdmDirectoryMsg(), _lastDirectoryMsgEvent.rdmDirectoryMsg());
            }

            return _msgReturnCode;
        }

        @Override
        public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
        {
            System.out.println("DEBUG: ReactorCallbackHandler.rdmDictionaryMsgCallback: entered. dictionaryMsgEventCount="
                            + ++_dictionaryMsgEventCount + " event=" + event.toString());

            _lastDictionaryMsgEvent = new RDMDictionaryMsgEvent();
            TestUtil.copyMsgEvent(event, _lastDictionaryMsgEvent);

            if (event.rdmDictionaryMsg() != null)
            {
                _lastDictionaryMsgEvent.rdmDictionaryMsg(DictionaryMsgFactory.createMsg());
                TestUtil.copyDictionaryMsg(event.rdmDictionaryMsg(), _lastDictionaryMsgEvent.rdmDictionaryMsg());
            }

            return _msgReturnCode;
        }
    }

    /*
     * Parses a network replay file and returns a populated NetworkReplay
     * 
     * @param fileName The name of the network replay file to parse
     * 
     * @return A populated NetworkReplay
     * 
     * @throws IOException Thrown if the file could not be parsed
     */
    static NetworkReplay parseReplayFile(String fileName) throws IOException
    {
        NetworkReplay fileReplay = NetworkReplayFactory.create();
        fileReplay.parseFile(fileName);

        return fileReplay;
    }

    /*
     * Wait for notification for 100ms. Call reactor.dispatchAll as notification
     * occurs.
     */
    static void dispatchReactorAll(Selector selector, Reactor reactor)
    {
        dispatchReactor(selector, reactor, 100, true);
    }

    /*
     * Wait for notification for 100ms. Call reactorChannel.dispatch as channels
     * are notified.
     */
    static void dispatchReactor(Selector selector, Reactor reactor)
    {
        dispatchReactor(selector, reactor, 100, false);
    }

    /*
     * Wait for notification until the given timeout has passed. Call
     * reactorChannel.dispatch as channels are notified.
     */
    static void dispatchReactor(Selector selector, Reactor reactor, int timeoutMsec)
    {
        dispatchReactor(selector, reactor, timeoutMsec, false);
    }

    /*
     * Wait for notification until the given timeout has passed. Call
     * reactorChannel.dispatchAll as notification occurs.
     */
    static void dispatchReactorAll(Selector selector, Reactor reactor, int timeoutMsec)
    {
        dispatchReactor(selector, reactor, timeoutMsec, true);
    }

    /*
     * Wait for notification until the given timeout has passed. Dispatch as
     * notification occurs (call reactorChannel.dispatch or
     * reactorChannel.dispatchAll as specified).
     */
    static void dispatchReactor(Selector selector, Reactor reactor, int timeoutMsec, boolean dispatchAll)
    {
        long currentTimeMs = System.nanoTime() / 1000000;
        long stopTimeMs = currentTimeMs + timeoutMsec;
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();

        do
        {
            int ret;
            try
            {
                ret = selector.select(stopTimeMs - currentTimeMs);
            }
            catch (IOException e)
            {
                e.printStackTrace();
                fail("Caught IOException.");
            }
            currentTimeMs = System.nanoTime() / 1000000;

            if (dispatchAll)
            {
                dispatchOptions.clear();
                while ((ret = reactor.dispatchAll(selector.selectedKeys(), dispatchOptions, errorInfo)) > ReactorReturnCodes.SUCCESS)
                    ;

                assertEquals(ReactorReturnCodes.SUCCESS, ret);
            }
            else
            {
                Set<SelectionKey> keys = selector.selectedKeys();
                Iterator<SelectionKey> iter = keys.iterator();
                while (iter.hasNext())
                {
                    ReactorChannel rc = (ReactorChannel)iter.next().attachment();
                    iter.remove();

                    dispatchOptions.clear();
                    while ((ret = rc.dispatch(dispatchOptions, errorInfo)) > ReactorReturnCodes.SUCCESS)
                        ;

                    assertEquals(ReactorReturnCodes.SUCCESS, ret);
                }
            }
        }
        while (currentTimeMs < stopTimeMs);
    }

    @Test
    public void reactorCreateShutdownTest()
    {
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;

        try
        {
            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            ReactorChannel theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            Selector selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            ReactorCallbackHandler callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
            assertEquals(ReactorReturnCodes.PARAMETER_OUT_OF_RANGE, dispatchOptions.maxMessages(0));
            assertEquals(ReactorReturnCodes.SUCCESS, dispatchOptions.maxMessages(1));
            assertEquals(ReactorReturnCodes.FAILURE, theReactorChannel.dispatch(dispatchOptions, errorInfo));
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
    }

    @Test
    public void consumerConnectChannelUpTest()
    {
        final String inputFile = BASE_RIPC_TEST_DATA_DIR_NAME + "/10_input_connectAck_Ripc14.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);
            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the replay msg to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            // dispatch on the reactor's reactorChannel. There should
            // be one "WorkerEvent" to dispatch on. There should
            // be two ReactorChannelEventCallbacks waiting, 1)
            // CHANNEL_UP and 2) CHANNEL_READY.
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());

        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void consumerConnectChannelFailTest()
    {
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;

        try
        {
            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            ReactorChannel theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            Selector selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions("12345");
            ReactorCallbackHandler callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // dispatch on the reactor's reactorChannel. There should
            // be one "WorkerEvent" to dispatch on. There should
            // be one ReactorChannelEventCallbacks waiting,
            // 1) CHANNEL_DOWN.
            // (It may take a second for the connection to fail.)
            ReactorJunit.dispatchReactor(selector, reactor, 3000);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(1, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelDownEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());

        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            errorInfo.clear();
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());
        }
    }

    @Test
    public void reactorConsumerWithDefaultLoginAndDirectory()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(1, callbackHandler.channelEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback. When that returns, reactor will send the
             * DirectoryRequest.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was
            // called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            verifyMessage(loginMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);
            verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            verifyMessage(msgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);

            /*
             * wait for testServer to read the DirectoryRequest, then send the
             * DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryResponse, invoke
             * rdmDirectoryMsgCallback, then send CHANNEL_READY
             * reactorChannelEventCallback.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount()); // no
                                                                     // change
                                                                     // expected
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            verifyMessage(directoryMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.SOURCE);
            verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            // make sure CHANNEL_READY event received
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());

        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorConsumerWithDefaultLoginDirectoryAndDictionary()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            // change dictionary download mode to first available
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(DictionaryDownloadModes.FIRST_AVAILABLE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(1, callbackHandler.channelEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback. When that returns, reactor will send the
             * DirectoryRequest.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was
            // called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            verifyMessage(loginMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);
            verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            verifyMessage(msgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);

            /*
             * wait for testServer to read the DirectoryRequest, then send the
             * DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryResponse, invoke
             * rdmDirectoryMsgCallback, then send CHANNEL_READY
             * reactorChannelEventCallback.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount()); // no
                                                                     // change
                                                                     // expected
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            verifyMessage(directoryMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.SOURCE);
            verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());

            // make sure no CHANNEL_READY event received yet
            assertEquals(1, callbackHandler.channelEventCount());
            assertEquals(0, callbackHandler.channelReadyEventCount());

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);

            /*
             * wait for testServer to read the DictionaryRequest, then send the
             * field DictionaryResponse
             */
            // this processes both field dictionary request and enum type
            // dictionary request
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.DICTIONARY);
            // have the TestServer send the field DictionaryRefresh to the
            // Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DictionaryResponse, invoke
             * rdmDictionaryMsgCallback, then send CHANNEL_READY
             * reactorChannelEventCallback.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDictionaryMsgCallback was called.
            assertEquals(1, callbackHandler.dictionaryMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount()); // no
                                                                     // change
                                                                     // expected
            RDMDictionaryMsgEvent dictionaryMsgEvent = callbackHandler.lastDictionaryMsgEvent();
            assertNotNull(dictionaryMsgEvent);
            verifyMessage(dictionaryMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.DICTIONARY);
            verifyDictionaryMessage(dictionaryMsgEvent.rdmDictionaryMsg());

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);

            // have the TestServer send the enum type DictionaryRefresh to the
            // Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DictionaryResponse, invoke
             * rdmDictionaryMsgCallback, then send CHANNEL_READY
             * reactorChannelEventCallback.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDictionaryMsgCallback was called.
            assertEquals(2, callbackHandler.dictionaryMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount()); // no
                                                                     // change
                                                                     // expected
            dictionaryMsgEvent = callbackHandler.lastDictionaryMsgEvent();
            assertNotNull(dictionaryMsgEvent);
            verifyMessage(dictionaryMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.DICTIONARY);
            verifyDictionaryMessage(dictionaryMsgEvent.rdmDictionaryMsg());

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);

            /*
             * wait for testServer to read the DictionaryClose
             */
            // this processes both field dictionary close and enum type
            // dictionary close
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.CLOSE, DomainTypes.DICTIONARY);

            // make sure CHANNEL_READY event received
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());

        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorDefaultConsumerRtt() {
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/210_Provider_LoginRefresh_RttNotification.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.rdmLoginRequest().attrib().applyHasSupportRoundTripLatencyMonitoring();
            // make sure dictionary download mode is none
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());

            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify KEY_EXCHANGE flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(1, callbackHandler.channelEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was
            // called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            verifyMessage(loginMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);
            verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            verifyMessage(msgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);


            testServer.waitForReadable();
            // wait and read one message.
            testServer.readMessageFromSocket();

            // Send RTT message to Reactor
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
            testServer.writeMessageToSocket(replay.read());
            ReactorJunit.dispatchReactor(selector, reactor);

            // verify that the LoginMsgCallback was called.
            assertEquals(2, callbackHandler.loginMsgEventCount());

            // verify RTT message
            RDMLoginMsgEvent rttLoginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(rttLoginMsgEvent);
            verifyMessage(rttLoginMsgEvent.transportBuffer(), MsgClasses.GENERIC, DomainTypes.LOGIN);
            verifyLoginRttMessage(rttLoginMsgEvent.rdmLoginMsg());
            LoginRTT loginRttOnReactor = (LoginRTT) rttLoginMsgEvent.rdmLoginMsg();

            // wait for returning RTT message from Consumer
            testServer.waitForReadable();
            assertTrue(testServer.readMessageFromSocket() > 0);
            //verify that it is GENERIC message.
            verifyMessage(testServer.buffer(), MsgClasses.GENERIC, DomainTypes.LOGIN);
        } catch (Exception e) {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        } finally {
            testServer.shutDown();

            if (theReactorChannel != null) {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorDefaultConsumerDisabledRtt() {
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/210_Provider_LoginRefresh_RttNotification.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            // make sure dictionary download mode is none
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());

            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify KEY_EXCHANGE flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(1, callbackHandler.channelEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was
            // called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            verifyMessage(loginMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);
            verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            verifyMessage(msgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);


            testServer.waitForReadable();
            // wait and read one message.
            testServer.readMessageFromSocket();

            // Send RTT message to Reactor
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
            int writtenBytes = testServer.writeMessageToSocket(replay.read());
            ReactorJunit.dispatchReactor(selector, reactor);

            // verify that the LoginMsgCallback hadn't been called for RTT.
            assertEquals(1, callbackHandler.loginMsgEventCount());

            // verify that obtained data is not returned back RTT message
            testServer.waitForReadable();
            assertTrue(testServer.readMessageFromSocket() < writtenBytes);
        } catch (Exception e) {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        } finally {
            testServer.shutDown();

            if (theReactorChannel != null) {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    public class ReadNullChannel implements Channel
    {
        Channel _channel;

        public ReadNullChannel(Channel channel)
        {
            _channel = channel;
        }

        @Override
        public int info(ChannelInfo info, Error error)
        {
            return 0;
        }

        @Override
        public int ioctl(int code, Object value, Error error)
        {
            return 0;
        }

        @Override
        public int ioctl(int code, int value, Error error)
        {
            return 0;
        }

        @Override
        public int bufferUsage(Error error)
        {
            return 0;
        }

        @Override
        public int init(InProgInfo inProg, Error error)
        {
            return 0;
        }

        @Override
        public int close(Error error)
        {
            return 0;
        }

        @Override
        public TransportBuffer read(ReadArgs readArgs, Error error)
        {
            // fake out read() returning null here
            ((ReadArgsImpl)readArgs).readRetVal(TransportReturnCodes.FAILURE);
            return null;
        }

        @Override
        public TransportBuffer getBuffer(int size, boolean packedBuffer, Error error)
        {
            return null;
        }

        @Override
        public int releaseBuffer(TransportBuffer buffer, Error error)
        {
            return 0;
        }

        @Override
        public int packBuffer(TransportBuffer buffer, Error error)
        {
            return 0;
        }

        @Override
        public int write(TransportBuffer buffer, WriteArgs writeArgs, Error error)
        {
            return 0;
        }

        @Override
        public int flush(Error error)
        {
            return 0;
        }

        @Override
        public int ping(Error error)
        {
            return 0;
        }

        @Override
        public int majorVersion()
        {
            return 0;
        }

        @Override
        public int minorVersion()
        {
            return 0;
        }

        @Override
        public int protocolType()
        {
            return 0;
        }

        @Override
        public int state()
        {
            return 0;
        }

        @Override
        public SocketChannel scktChannel()
        {
            return (SocketChannel)_channel.selectableChannel();
        }

        @Override
        public SocketChannel oldScktChannel()
        {
            return (SocketChannel)_channel.oldSelectableChannel();
        }

        @Override
        public SelectableChannel selectableChannel()
        {
            return _channel.selectableChannel();
        }

        @Override
        public SelectableChannel oldSelectableChannel()
        {
            return _channel.oldSelectableChannel();
        }

        @Override
        public int pingTimeout()
        {
            return 0;
        }

        @Override
        public Object userSpecObject()
        {
            return null;
        }

        @Override
        public boolean blocking()
        {
            return false;
        }

        @Override
        public int reconnectClient(Error error)
        {
            return 0;
        }

        @Override
        public int connectionType()
        {
            return 0;
        }

		@Override
		public String hostname() {
			// TODO Auto-generated method stub
			return null;
		}
    }

    @Test
    public void reactorDispatchWithChannelDown()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(1, callbackHandler.channelEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
            ReactorChannel consumerReactorChannel = callbackHandler.lastReactorChannel();

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            // create ReadNullChannel for testing
            Channel readNullChannel = new ReadNullChannel(consumerReactorChannel.channel());

            // set reactor channel's channel to readNullChannel
            // this will cause next dispatch to read null
            consumerReactorChannel.selectableChannelFromChannel(readNullChannel);

            /*
             * Call dispatch where the channel read should fail. CHANNEL_DOWN
             * callback should occur.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelDownEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            errorInfo.clear();
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());
        }
    }

    public class ReadFDChangeChannel implements Channel
    {
        boolean oldScktChannelCalled;
        boolean scktChannelCalled;
        int readRetVal;
        boolean resetRetValAfterRead;

        @Override
        public int info(ChannelInfo info, Error error)
        {
            return 0;
        }

        @Override
        public int ioctl(int code, Object value, Error error)
        {
            return 0;
        }

        @Override
        public int ioctl(int code, int value, Error error)
        {
            return 0;
        }

        @Override
        public int bufferUsage(Error error)
        {
            return 0;
        }

        @Override
        public int init(InProgInfo inProg, Error error)
        {
            return 0;
        }

        @Override
        public int close(Error error)
        {
            return 0;
        }

        @Override
        public TransportBuffer read(ReadArgs readArgs, Error error)
        {
            // fake out read() returning FD_CHANGE here
            ((ReadArgsImpl)readArgs).readRetVal(readRetVal);
            if (resetRetValAfterRead)
                readRetVal = TransportReturnCodes.SUCCESS;
            return null;
        }

        @Override
        public TransportBuffer getBuffer(int size, boolean packedBuffer, Error error)
        {
            return null;
        }

        @Override
        public int releaseBuffer(TransportBuffer buffer, Error error)
        {
            return 0;
        }

        @Override
        public int packBuffer(TransportBuffer buffer, Error error)
        {
            return 0;
        }

        @Override
        public int write(TransportBuffer buffer, WriteArgs writeArgs, Error error)
        {
            return 0;
        }

        @Override
        public int flush(Error error)
        {
            return 0;
        }

        @Override
        public int ping(Error error)
        {
            return 0;
        }

        @Override
        public int majorVersion()
        {
            return 0;
        }

        @Override
        public int minorVersion()
        {
            return 0;
        }

        @Override
        public int protocolType()
        {
            return 0;
        }

        @Override
        public int state()
        {
            return 0;
        }

        @Override
        public SocketChannel scktChannel()
        {
            scktChannelCalled = true;
            SocketChannel scktChnl = null;
            try
            {
                scktChnl = java.nio.channels.SocketChannel.open();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }

            return scktChnl;
        }

        @Override
        public SocketChannel oldScktChannel()
        {
            oldScktChannelCalled = true;
            SocketChannel scktChnl = null;
            try
            {
                scktChnl = java.nio.channels.SocketChannel.open();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }

            return scktChnl;
        }

        @Override
        public SelectableChannel selectableChannel()
        {
            scktChannelCalled = true;
            SocketChannel scktChnl = null;
            try
            {
                scktChnl = java.nio.channels.SocketChannel.open();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }

            return scktChnl;
        }

        @Override
        public SelectableChannel oldSelectableChannel()
        {
            oldScktChannelCalled = true;
            SocketChannel scktChnl = null;
            try
            {
                scktChnl = java.nio.channels.SocketChannel.open();
            }
            catch (IOException e)
            {
                e.printStackTrace();
            }

            return scktChnl;
        }

        @Override
        public int pingTimeout()
        {
            return 0;
        }

        @Override
        public Object userSpecObject()
        {
            return null;
        }

        @Override
        public boolean blocking()
        {
            return false;
        }

        @Override
        public int reconnectClient(Error error)
        {
            return 0;
        }

        @Override
        public int connectionType()
        {
            return 0;
        }

		@Override
		public String hostname() {
			// TODO Auto-generated method stub
			return null;
		}
    }

    @Test
    public void reactorDispatchWithFDChange()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(1, callbackHandler.channelEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
            ReactorChannel consumerReactorChannel = callbackHandler.lastReactorChannel();


            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            // create ReadFDChangeChannel for testing
            ReadFDChangeChannel readFDChangeChannel = new ReadFDChangeChannel();

            // set reactor channel's channel to readFDChangeChannel
            // this will cause next dispatch to read FD_CHANGE
            consumerReactorChannel.selectableChannelFromChannel(readFDChangeChannel);

            /*
             * Call dispatch where read should return FD_CHANGE. FD_CHANGE
             * callback should occur.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            readFDChangeChannel.readRetVal = TransportReturnCodes.READ_FD_CHANGE;
            readFDChangeChannel.resetRetValAfterRead = true;
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelFDChangeEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.FD_CHANGE, event.eventType());
            Thread.sleep(1000);
            assertTrue(readFDChangeChannel.oldScktChannelCalled);
            assertTrue(readFDChangeChannel.scktChannelCalled);
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            errorInfo.clear();
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());
        }
    }

    public class WriteFlushFailChannel implements Channel
    {
        int writeRetVal = TransportReturnCodes.SUCCESS;
        int flushRetVal = TransportReturnCodes.SUCCESS;
        boolean resetRetValAfterFlush = false;
        int flushCount = 0;
        int releaseBufferCount = 0;
        boolean noBuffers = false;
        boolean smallBuffer = false;
        int _state = ChannelState.ACTIVE;
        private Channel _channel;

        // inner class for TransportBuffer
        class TestTransportBuffer implements TransportBuffer
        {
            final int smallBufSize = 1;
            final int bufSize = 1000;
            ByteBuffer byteBuffer = ByteBuffer.allocate(bufSize);
            ByteBuffer smallByteBuffer = ByteBuffer.allocate(smallBufSize);

            @Override
            public ByteBuffer data()
            {
                if (smallBuffer == false)
                {
                    return byteBuffer;
                }
                else
                {
                    smallBuffer = false;
                    return smallByteBuffer;
                }
            }

            @Override
            public int length()
            {
                if (smallBuffer == false)
                {
                    return bufSize;
                }
                else
                {
                    smallBuffer = false;
                    return smallBufSize;
                }
            }

            @Override
            public int copy(ByteBuffer destBuffer)
            {
                return 0;
            }

            @Override
            public int capacity()
            {
                if (smallBuffer == false)
                {
                    return bufSize;
                }
                else
                {
                    smallBuffer = false;
                    return smallBufSize;
                }
            }

            @Override
            public int dataStartPosition()
            {

                return 0;
            }
        }

        public WriteFlushFailChannel(Channel channel)
        {
            _channel = channel;
        }

        @Override
        public int info(ChannelInfo info, Error error)
        {
            return 0;
        }

        @Override
        public int ioctl(int code, Object value, Error error)
        {
            return 0;
        }

        @Override
        public int ioctl(int code, int value, Error error)
        {
            return 0;
        }

        @Override
        public int bufferUsage(Error error)
        {
            return 0;
        }

        @Override
        public int init(InProgInfo inProg, Error error)
        {
            return 0;
        }

        @Override
        public int close(Error error)
        {
            return 0;
        }

        @Override
        public TransportBuffer read(ReadArgs readArgs, Error error)
        {
            ((ReadArgsImpl)readArgs).readRetVal(TransportReturnCodes.SUCCESS);
            return null;
        }

        @Override
        public TransportBuffer getBuffer(int size, boolean packedBuffer, Error error)
        {
            if (noBuffers == false)
            {
                return new TestTransportBuffer();
            }
            else
            {
                return null;
            }
        }

        @Override
        public int releaseBuffer(TransportBuffer buffer, Error error)
        {
            releaseBufferCount++;
            return TransportReturnCodes.SUCCESS;
        }

        @Override
        public int packBuffer(TransportBuffer buffer, Error error)
        {
            return 0;
        }

        @Override
        public int write(TransportBuffer buffer, WriteArgs writeArgs, Error error)
        {
            return writeRetVal;
        }

        @Override
        public int flush(Error error)
        {
            int ret = flushRetVal;
            if (resetRetValAfterFlush)
            {
                flushRetVal = TransportReturnCodes.SUCCESS;
                resetRetValAfterFlush = false;
            }
            flushCount++;
            return ret;
        }

        @Override
        public int ping(Error error)
        {
            return 0;
        }

        @Override
        public int majorVersion()
        {
            return 14;
        }

        @Override
        public int minorVersion()
        {
            return 0;
        }

        @Override
        public int protocolType()
        {
            return 0;
        }

        @Override
        public int state()
        {
            return _state;
        }

        @Override
        public SocketChannel scktChannel()
        {

            return (SocketChannel)_channel.selectableChannel();
        }

        @Override
        public SocketChannel oldScktChannel()
        {
            return (SocketChannel)_channel.oldSelectableChannel();
        }

        @Override
        public SelectableChannel selectableChannel()
        {
            return _channel.selectableChannel();
        }

        @Override
        public SelectableChannel oldSelectableChannel()
        {
            return _channel.oldSelectableChannel();
        }

        @Override
        public int pingTimeout()
        {
            return 0;
        }

        @Override
        public Object userSpecObject()
        {
            return null;
        }

        @Override
        public boolean blocking()
        {
            return false;
        }

        @Override
        public int reconnectClient(Error error)
        {
            return 0;
        }

        @Override
        public int connectionType()
        {
            return 0;
        }

		@Override
		public String hostname() {
			// TODO Auto-generated method stub
			return null;
		}
    }

    @Test
    public void reactorSubmitWithTransportBuffer()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            // make sure login and directory requests are null
            assertEquals(null, consumerRole._loginRequest);
            assertEquals(null, consumerRole._directoryRequest);
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
            
            ReactorChannel reactorChannel = callbackHandler.lastReactorChannel();

            Channel writeFlushFailChannel = new WriteFlushFailChannel(reactorChannel.channel());

            // submit a login request message
            LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
            loginRequest.rdmMsgType(LoginMsgType.REQUEST);
            /* get a buffer for the login request */
            Error error = TransportFactory.createError();
            TransportBuffer msgBuf = reactorChannel.channel().getBuffer(1000, false, error);
            assertNotNull(msgBuf);

            loginRequest.clear();
            loginRequest.initDefaultRequest(1);

            EncodeIterator encIter = CodecFactory.createEncodeIterator();
            encIter.clear();
            encIter.setBufferAndRWFVersion(msgBuf, reactorChannel.channel().majorVersion(), reactorChannel.channel().minorVersion());

            int ret = loginRequest.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);

            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals("DIRECT_FEED", submitOptions.serviceName());
            assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msgBuf, submitOptions, errorInfo));
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            /*
             * Wait for testServer to read the LoginRequest.
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);

            // exercise various internal return codes for submit and make sure
            // flush is called
            reactorChannel.selectableChannelFromChannel(writeFlushFailChannel);

            // set write return value to positive number
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = TransportReturnCodes.SUCCESS;
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msgBuf, submitOptions, errorInfo));
            Thread.sleep(1000);
            assertEquals(1, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            // set write return value to WRITE_CALL_AGAIN and flush return value
            // to positive
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = TransportReturnCodes.WRITE_CALL_AGAIN;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).resetRetValAfterFlush = true;
            assertEquals(ReactorReturnCodes.WRITE_CALL_AGAIN, reactorChannel.submit(msgBuf, submitOptions, errorInfo));
            Thread.sleep(1000);
            /*
             * Worker thread will flush twice -- first attempt gets 1 returned,
             * second attempt gets 0.
             */
            assertEquals(3, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            // set write return value to positive number and flush return value
            // to positive
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).resetRetValAfterFlush = true;
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msgBuf, submitOptions, errorInfo));
            Thread.sleep(1000);

            /*
             * Worker thread will flush twice -- first attempt gets 1 returned,
             * second attempt gets 0.
             */
            assertEquals(5, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);

            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorSubmitWithCodecMsg()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);
            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            // make sure login and directory requests are null
            assertEquals(null, consumerRole._loginRequest);
            assertEquals(null, consumerRole._directoryRequest);
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
            
            ReactorChannel reactorChannel = callbackHandler.lastReactorChannel();

            Channel writeFlushFailChannel = new WriteFlushFailChannel(reactorChannel.channel());

            // submit a login request message
            RequestMsg msg = (RequestMsg)CodecFactory.createMsg();

            /* set-up message */
            msg.msgClass(MsgClasses.REQUEST);
            msg.streamId(1);
            msg.domainType(DomainTypes.LOGIN);
            msg.containerType(DataTypes.ELEMENT_LIST);

            /* set msgKey members */
            msg.msgKey().applyHasNameType();
            msg.msgKey().applyHasName();
            msg.msgKey().applyHasIdentifier();
            msg.msgKey().applyHasAttrib();

            msg.msgKey().name().data("TRI.N");
            msg.msgKey().nameType(InstrumentNameTypes.RIC);
            msg.msgKey().identifier(0x7fff);

            Buffer encodedAttrib = CodecFactory.createBuffer();
            encodedAttrib.data("ENCODED ATTRIB");
            msg.msgKey().attribContainerType(DataTypes.OPAQUE);
            msg.msgKey().encodedAttrib(encodedAttrib);

            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals("DIRECT_FEED", submitOptions.serviceName());
            assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msg, submitOptions, errorInfo));
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            /*
             * Wait for testServer to read the LoginRequest.
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);

            // exercise various internal return codes for submit and make sure
            // flush is called
            reactorChannel.selectableChannelFromChannel(writeFlushFailChannel);

            // set write return value to positive number
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = TransportReturnCodes.SUCCESS;
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msg, submitOptions, errorInfo));
            Thread.sleep(1000);
            assertEquals(1, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            // set write return value to WRITE_CALL_AGAIN and flush return value
            // to positive
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = TransportReturnCodes.WRITE_CALL_AGAIN;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).resetRetValAfterFlush = true;
            assertEquals(ReactorReturnCodes.WRITE_CALL_AGAIN, reactorChannel.submit(msg, submitOptions, errorInfo));
            Thread.sleep(1000);
            /*
             * Worker thread will flush twice -- first attempt gets 1 returned,
             * second gets 0.
             */
            assertEquals(3, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);

            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            // set write return value to positive number and flush return value
            // to positive
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).resetRetValAfterFlush = true;
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msg, submitOptions, errorInfo));
            Thread.sleep(1000);
            /*
             * Worker thread will flush twice -- first attempt gets 1 returned,
             * second gets 0.
             */
            assertEquals(5, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);

            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorSubmitWithRdmMsg()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            // make sure login and directory requests are null
            assertEquals(null, consumerRole._loginRequest);
            assertEquals(null, consumerRole._directoryRequest);
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
            
            ReactorChannel reactorChannel = callbackHandler.lastReactorChannel();

            Channel writeFlushFailChannel = new WriteFlushFailChannel(reactorChannel.channel());

            // submit a login request message
            LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
            loginRequest.rdmMsgType(LoginMsgType.REQUEST);
            loginRequest.initDefaultRequest(1);

            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals("DIRECT_FEED", submitOptions.serviceName());
            assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(loginRequest, submitOptions, errorInfo));
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            /*
             * Wait for testServer to read the LoginRequest.
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);

            // exercise various internal return codes for submit and make sure
            // flush is called
            reactorChannel.selectableChannelFromChannel(writeFlushFailChannel);

            // set write return value to positive number
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = TransportReturnCodes.SUCCESS;
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(loginRequest, submitOptions, errorInfo));
            Thread.sleep(1000);
            assertEquals(1, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);

            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            // set write return value to WRITE_CALL_AGAIN and flush return value
            // to positive
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = TransportReturnCodes.WRITE_CALL_AGAIN;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).resetRetValAfterFlush = true;
            assertEquals(ReactorReturnCodes.WRITE_CALL_AGAIN, reactorChannel.submit(loginRequest, submitOptions, errorInfo));
            Thread.sleep(1000);
            /*
             * Worker thread will flush twice -- first attempt gets 1 returned,
             * second gets 0.
             */
            assertEquals(3, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);

            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            // set write return value to positive number and flush return value
            // to positive
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).resetRetValAfterFlush = true;
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(loginRequest, submitOptions, errorInfo));
            Thread.sleep(1000);
            /*
             * Worker thread will flush twice -- first attempt gets 1 returned,
             * second gets 0.
             */
            assertEquals(5, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);

            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorSubmitWithCodecMsgNoBuffers()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            // make sure login and directory requests are null
            assertEquals(null, consumerRole._loginRequest);
            assertEquals(null, consumerRole._directoryRequest);
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
            
            ReactorChannel reactorChannel = callbackHandler.lastReactorChannel();

            Channel writeFlushFailChannel = new WriteFlushFailChannel(reactorChannel.channel());

            // submit a login request message
            RequestMsg msg = (RequestMsg)CodecFactory.createMsg();

            /* set-up message */
            msg.msgClass(MsgClasses.REQUEST);
            msg.streamId(1);
            msg.domainType(DomainTypes.LOGIN);
            msg.containerType(DataTypes.ELEMENT_LIST);

            /* set msgKey members */
            msg.msgKey().applyHasNameType();
            msg.msgKey().applyHasName();
            msg.msgKey().applyHasIdentifier();
            msg.msgKey().applyHasAttrib();

            msg.msgKey().name().data("TRI.N");
            msg.msgKey().nameType(InstrumentNameTypes.RIC);
            msg.msgKey().identifier(0x7fff);

            Buffer encodedAttrib = CodecFactory.createBuffer();
            encodedAttrib.data("ENCODED ATTRIB");
            msg.msgKey().attribContainerType(DataTypes.OPAQUE);
            msg.msgKey().encodedAttrib(encodedAttrib);

            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msg, submitOptions, errorInfo));
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            /*
             * Wait for testServer to read the LoginRequest.
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);

            reactorChannel.selectableChannelFromChannel(writeFlushFailChannel);

            // make channel getBuffer() return null, flush should get called
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = TransportReturnCodes.SUCCESS;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = TransportReturnCodes.SUCCESS;
            ((WriteFlushFailChannel)writeFlushFailChannel).noBuffers = true;
            assertEquals(ReactorReturnCodes.NO_BUFFERS, reactorChannel.submit(msg, submitOptions, errorInfo));
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            Thread.sleep(1000);
            assertEquals(0, ((WriteFlushFailChannel)writeFlushFailChannel).releaseBufferCount);
            assertEquals(1, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            errorInfo.clear();
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorSubmitWithRdmMsgNoBuffers()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            // make sure login and directory requests are null
            assertEquals(null, consumerRole._loginRequest);
            assertEquals(null, consumerRole._directoryRequest);
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
            
            ReactorChannel reactorChannel = callbackHandler.lastReactorChannel();

            Channel writeFlushFailChannel = new WriteFlushFailChannel(reactorChannel.channel());

            // submit a login request message
            LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
            loginRequest.rdmMsgType(LoginMsgType.REQUEST);
            loginRequest.initDefaultRequest(1);

            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(loginRequest, submitOptions, errorInfo));
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            /*
             * Wait for testServer to read the LoginRequest.
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);

            reactorChannel.selectableChannelFromChannel(writeFlushFailChannel);

            // make channel getBuffer() return null, flush should get called
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = TransportReturnCodes.SUCCESS;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = TransportReturnCodes.SUCCESS;
            ((WriteFlushFailChannel)writeFlushFailChannel).noBuffers = true;
            assertEquals(ReactorReturnCodes.NO_BUFFERS, reactorChannel.submit(loginRequest, submitOptions, errorInfo));
            Thread.sleep(1000);
            assertEquals(0, ((WriteFlushFailChannel)writeFlushFailChannel).releaseBufferCount);
            assertEquals(1, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);

            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            errorInfo.clear();
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorSubmitWithCodecMsgResizeBuffer()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            // make sure login and directory requests are null
            assertEquals(null, consumerRole._loginRequest);
            assertEquals(null, consumerRole._directoryRequest);
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
            
            ReactorChannel reactorChannel = callbackHandler.lastReactorChannel();

            Channel writeFlushFailChannel = new WriteFlushFailChannel(reactorChannel.channel());

            // submit a login request message
            RequestMsg msg = (RequestMsg)CodecFactory.createMsg();

            /* set-up message */
            msg.msgClass(MsgClasses.REQUEST);
            msg.streamId(1);
            msg.domainType(DomainTypes.LOGIN);
            msg.containerType(DataTypes.ELEMENT_LIST);

            /* set msgKey members */
            msg.msgKey().applyHasNameType();
            msg.msgKey().applyHasName();
            msg.msgKey().applyHasIdentifier();
            msg.msgKey().applyHasAttrib();

            msg.msgKey().name().data("TRI.N");
            msg.msgKey().nameType(InstrumentNameTypes.RIC);
            msg.msgKey().identifier(0x7fff);

            Buffer encodedAttrib = CodecFactory.createBuffer();
            encodedAttrib.data("ENCODED ATTRIB");
            msg.msgKey().attribContainerType(DataTypes.OPAQUE);
            msg.msgKey().encodedAttrib(encodedAttrib);

            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msg, submitOptions, errorInfo));
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            /*
             * Wait for testServer to read the LoginRequest.
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);

            reactorChannel.selectableChannelFromChannel(writeFlushFailChannel);

            // make channel getBuffer() return buffer size of 1 and then normal
            // size
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = TransportReturnCodes.SUCCESS;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = TransportReturnCodes.SUCCESS;
            ((WriteFlushFailChannel)writeFlushFailChannel).smallBuffer = true;
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msg, submitOptions, errorInfo));
            Thread.sleep(1000);
            assertEquals(1, ((WriteFlushFailChannel)writeFlushFailChannel).releaseBufferCount);
            assertEquals(0, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            errorInfo.clear();
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorSubmitWithRdmMsgResizeBuffer()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            // make sure login and directory requests are null
            assertEquals(null, consumerRole._loginRequest);
            assertEquals(null, consumerRole._directoryRequest);
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
            
            ReactorChannel reactorChannel = callbackHandler.lastReactorChannel();

            Channel writeFlushFailChannel = new WriteFlushFailChannel(reactorChannel.channel());

            // submit a login request message
            LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
            loginRequest.rdmMsgType(LoginMsgType.REQUEST);
            loginRequest.initDefaultRequest(1);

            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(loginRequest, submitOptions, errorInfo));

            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            /*
             * Wait for testServer to read the LoginRequest.
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);

            reactorChannel.selectableChannelFromChannel(writeFlushFailChannel);

            // make channel getBuffer() return buffer size of 1 and then normal
            // size
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = TransportReturnCodes.SUCCESS;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = TransportReturnCodes.SUCCESS;
            ((WriteFlushFailChannel)writeFlushFailChannel).smallBuffer = true;
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(loginRequest, submitOptions, errorInfo));
            Thread.sleep(1000);
            assertEquals(1, ((WriteFlushFailChannel)writeFlushFailChannel).releaseBufferCount);
            assertEquals(0, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            errorInfo.clear();
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorSubmitWithWriteFail()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            // make sure login and directory requests are null
            assertEquals(null, consumerRole._loginRequest);
            assertEquals(null, consumerRole._directoryRequest);
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
            
            ReactorChannel reactorChannel = callbackHandler.lastReactorChannel();

            Channel writeFlushFailChannel = new WriteFlushFailChannel(reactorChannel.channel());

            // submit a login request message
            LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
            loginRequest.rdmMsgType(LoginMsgType.REQUEST);
            /* get a buffer for the login request */
            Error error = TransportFactory.createError();
            TransportBuffer msgBuf = reactorChannel.channel().getBuffer(1000, false, error);
            assertNotNull(msgBuf);

            loginRequest.clear();
            loginRequest.initDefaultRequest(1);

            EncodeIterator encIter = CodecFactory.createEncodeIterator();
            encIter.clear();
            encIter.setBufferAndRWFVersion(msgBuf, reactorChannel.channel().majorVersion(), reactorChannel.channel().minorVersion());

            int ret = loginRequest.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msgBuf, submitOptions, errorInfo));

            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            /*
             * Wait for testServer to read the LoginRequest.
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);

            // exercise various internal return codes for submit and make sure
            // flush is called
            reactorChannel.selectableChannelFromChannel(writeFlushFailChannel);

            // set write return value to failure
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = TransportReturnCodes.FAILURE;
            assertEquals(ReactorReturnCodes.FAILURE, reactorChannel.submit(msgBuf, submitOptions, errorInfo));
            Thread.sleep(1000);
            assertEquals(0, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            errorInfo.clear();
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());
        }
    }

    @Test
    public void reactorSubmitWithFlushFail()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            // make sure login and directory requests are null
            assertEquals(null, consumerRole._loginRequest);
            assertEquals(null, consumerRole._directoryRequest);
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            ReactorChannel reactorChannel = callbackHandler.lastReactorChannel();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

            Channel writeFlushFailChannel = new WriteFlushFailChannel(reactorChannel.channel());

            // submit a login request message
            LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
            loginRequest.rdmMsgType(LoginMsgType.REQUEST);
            /* get a buffer for the login request */
            Error error = TransportFactory.createError();
            TransportBuffer msgBuf = reactorChannel.channel().getBuffer(1000, false, error);
            assertNotNull(msgBuf);

            loginRequest.clear();
            loginRequest.initDefaultRequest(1);

            EncodeIterator encIter = CodecFactory.createEncodeIterator();
            encIter.clear();
            encIter.setBufferAndRWFVersion(msgBuf, reactorChannel.channel().majorVersion(), reactorChannel.channel().minorVersion());

            int ret = loginRequest.encode(encIter);
            assertEquals(CodecReturnCodes.SUCCESS, ret);
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msgBuf, submitOptions, errorInfo));
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            /*
             * Wait for testServer to read the LoginRequest.
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);

            // exercise various internal return codes for submit and make sure
            // flush is called
            reactorChannel.selectableChannelFromChannel(writeFlushFailChannel);

            // set write return value to positive number and flush return value
            // to failure
            ((WriteFlushFailChannel)writeFlushFailChannel).writeRetVal = 1;
            ((WriteFlushFailChannel)writeFlushFailChannel).flushRetVal = TransportReturnCodes.FAILURE;
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(msgBuf, submitOptions, errorInfo));
            Thread.sleep(1000);
            assertEquals(1, ((WriteFlushFailChannel)writeFlushFailChannel).flushCount);

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_DOWN.
             */
            ReactorJunit.dispatchReactor(selector, reactor);

            // verify that the ReactorChannelEventCallback was called.
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelDownEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            errorInfo.clear();
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());
        }
    }

    @Test
    public void reactorProviderAccept()
    {
        /*
         * This file contains just the RIPC ConnectReq handshake message.
         */
        final String ripcConnectReqFile = "../Core/src/test/resources/com/refinitiv/eta/transport/RipcHandshakeJunit/60_input_connectReq_Ripc14.txt";
        Server server = null;
        Error error = TransportFactory.createError();
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;

        try
        {
            NetworkReplay replay = parseReplayFile(ripcConnectReqFile);

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);
            assertEquals(false, reactor.isShutdown());

            /* bind server */
            BindOptions bindOpts = createDefaultBindOptions();
            server = Transport.bind(bindOpts, error);
            assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
                    + error.sysError(), server);

            /* register server for ACCEPT */
            selector = SelectorProvider.provider().openSelector();
            server.selectableChannel().register(selector, SelectionKey.OP_ACCEPT, server);

            /* create test client */
            TestClient testClient = new TestClient(14005);
            testClient.connect();

            // wait for the server to signal us that the client is connecting
            int updCnt = selector.select(10000); // 10 seconds
            assertTrue(updCnt > 0);
            Set<SelectionKey> keys = selector.selectedKeys();
            assertNotNull(keys);
            assertEquals(1, keys.size());
            Iterator<SelectionKey> iter = keys.iterator();
            assertTrue(iter.hasNext());
            SelectionKey key = iter.next();
            assertNotNull(key);
            iter.remove();
            Server serverFromKey = (Server)key.attachment();
            assertEquals(server, serverFromKey);

            /* register with the reactor's reactorChannel */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create ReactorAcceptOptions, ProviderRole and call accept
             */
            ReactorAcceptOptions raOpts = createDefaultProviderAcceptOptions();
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ProviderRole providerRole = createDefaultProviderRole(callbackHandler);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.accept(server, raOpts, providerRole, errorInfo));

            // have our test client send a ConnectReq and clientKey to the
            // server
            testClient.writeMessageToSocket(replay.read());
            Thread.sleep(1000);
            testClient.writeMessageToSocket(replay.read());

            // dispatch on the reactor's reactorChannel. There should
            // be one "WorkerEvent" to dispatch on. There should
            // be two ReactorChannelEventCallbacks waiting, 1)
            // CHANNEL_UP and 2) CHANNEL_READY.
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());

            testClient.shutDown();
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            if (server != null)
            {
                assertEquals(TransportReturnCodes.SUCCESS, server.close(error));
            }

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            errorInfo.clear();
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());
        }

    }

    @Test
    public void reactorNIProviderConnect()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            NIProviderRole niProviderRole = createDefaultNIProviderRole(callbackHandler);
            niProviderRole.initDefaultRDMLoginRequest();
            niProviderRole.initDefaultRDMDirectoryRefresh("RMDS_PUB", 1);
            reactor.connect(rcOpts, niProviderRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(1, callbackHandler.channelEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback. When that returns, reactor will send the
             * DirectoryRequest.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was
            // called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            verifyMessage(loginMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);
            verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            verifyMessage(msgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);

            /*
             * wait for testServer to read the DirectoryRefresh
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REFRESH, DomainTypes.SOURCE);

            /*
             * call dispatch which should send the CHANNEL_READY
             * reactorChannelEventCallback
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // make sure CHANNEL_READY event received
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorConsumerDispatchAllKeySet()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            // wait for the reactor to signal us that the channel is ready.
            int updCnt = selector.select(10000); // 10 seconds
            assertTrue(updCnt > 0);
            Set<SelectionKey> keys = selector.selectedKeys();
            assertNotNull(keys);
            assertEquals(1, keys.size());

            /*
             * Call dispatchAll on the reactor. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
            assertEquals(ReactorReturnCodes.PARAMETER_OUT_OF_RANGE, dispatchOptions.maxMessages(0));
            assertEquals(ReactorReturnCodes.SUCCESS, dispatchOptions.maxMessages(1));
            // dispatchAll the first event. Dispatch should return 0, meaning
            // that there is no more to read.
            assertEquals(0, reactor.dispatchAll(keys, dispatchOptions, errorInfo));
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(1, callbackHandler.channelEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * Call dispatchAll which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback. When that returns, reactor will send the
             * DirectoryRequest.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            dispatchOptions.clear();
            dispatchOptions.maxMessages(1);
            ReactorJunit.dispatchReactorAll(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was
            // called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            verifyMessage(loginMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);
            verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            verifyMessage(msgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);

            /*
             * wait for testServer to read the DirectoryRequest, then send the
             * DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * Call dispatchAll which should read the DirectoryResponse, invoke
             * rdmDirectoryMsgCallback, then send CHANNEL_READY
             * reactorChannelEventCallback.
             */
            dispatchOptions.clear();
            dispatchOptions.maxMessages(1);
            ReactorJunit.dispatchReactorAll(selector, reactor);
            // verify that the RDMDirectoryMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount()); // no
                                                                     // change
                                                                     // expected
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            verifyMessage(directoryMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.SOURCE);
            verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            // make sure CHANNEL_READY event received
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());

        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorConsumerDispatchAllKeySetMultipleChannels()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        ExecutorService executorService1 = null;
        ExecutorService executorService2 = null;

        try
        {
            NetworkReplay replay1 = parseReplayFile(inputFile);
            NetworkReplay replay2 = parseReplayFile(inputFile);

            /*
             * create 3 testServers which will send RIPC ConnectAck.
             */
            int serverPort1 = ++_serverPort;
            TestServer testServer1 = new TestServer(serverPort1);
            executorService1 = Executors.newSingleThreadExecutor();
            executorService1.execute(testServer1);
            int serverPort2 = ++_serverPort;
            TestServer testServer2 = new TestServer(serverPort2);
            executorService2 = Executors.newSingleThreadExecutor();
            executorService2.execute(testServer2);

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create 3 Client Connections.
             */
            ReactorConnectOptions rcOpts1 = createDefaultConsumerConnectOptions(String.valueOf(serverPort1));
            ReactorConnectOptions rcOpts2 = createDefaultConsumerConnectOptions(String.valueOf(serverPort2));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts1, consumerRole, errorInfo);
            reactor.connect(rcOpts2, consumerRole, errorInfo);

            Thread.sleep(1000);

            // wait for the TestServer to accept a connection.
            testServer1.wait(TestServer.State.ACCEPTED);
            testServer2.wait(TestServer.State.ACCEPTED);

            Thread.sleep(1000);

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer1.readMessageFromSocket() > 0);
            verifyConnectReq(testServer1.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer1.writeMessageToSocket(replay1.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer1.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer1.buffer().get(2)); // verify
                                                                     // KEY_EXCHANGE
                                                                     // flag
            assertTrue(testServer2.readMessageFromSocket() > 0);
            verifyConnectReq(testServer2.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer2.writeMessageToSocket(replay2.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer2.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer2.buffer().get(2)); // verify
                                                                     // KEY_EXCHANGE
                                                                     // flag

            Thread.sleep(1000); // wait for all connections

            /*
             * Call dispatchAll on the reactor. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactorAll(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer1.wait(TestServer.State.READABLE);
            // wait and read one message.
            assertTrue(testServer1.readMessageFromSocket() > 0);
            verifyMessage(testServer1.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer1.writeMessageToSocket(replay1.read());

            testServer2.wait(TestServer.State.READABLE);
            // wait and read one message.
            assertTrue(testServer2.readMessageFromSocket() > 0);
            verifyMessage(testServer2.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer2.writeMessageToSocket(replay2.read());

            Thread.sleep(1000); // wait for all messages

            /*
             * Call dispatchAll which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback. When that returns, reactor will send the
             * DirectoryRequest.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactorAll(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was
            // called.
            assertEquals(2, callbackHandler.loginMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            verifyMessage(loginMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);
            verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            verifyMessage(msgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);

            /*
             * wait for testServer to read the DirectoryRequest, then send the
             * DirectoryResponse
             */
            testServer1.wait(TestServer.State.READABLE);
            // wait and read one message.
            assertTrue(testServer1.readMessageFromSocket() > 0);
            verifyMessage(testServer1.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer1.writeMessageToSocket(replay1.read());

            testServer2.wait(TestServer.State.READABLE);
            // wait and read one message.
            assertTrue(testServer2.readMessageFromSocket() > 0);
            verifyMessage(testServer2.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer2.writeMessageToSocket(replay2.read());

            Thread.sleep(1000); // wait for all messages

            /*
             * Call dispatchAll which should read the DirectoryResponse, invoke
             * rdmDirectoryMsgCallback, then send CHANNEL_READY
             * reactorChannelEventCallback.
             */
            ReactorJunit.dispatchReactorAll(selector, reactor);
            // verify that the RDMDirectoryMsgCallback was called.
            assertEquals(2, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount()); // no
                                                                     // change
                                                                     // expected
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            verifyMessage(directoryMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.SOURCE);
            verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            // make sure CHANNEL_READY event received
            assertEquals(4, callbackHandler.channelEventCount());
            assertEquals(2, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());

            testServer1.shutDown();
            testServer2.shutDown();
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());

            executorService1.shutdownNow();
            executorService2.shutdownNow();
        }
    }

    @Test
    public void reactorConsumerDispatchAllNoKeySet()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(SelectorProvider.provider().openSelector());
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            Thread.sleep(1000);

            /*
             * Call dispatchAll on the reactor. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
            assertEquals(0, reactor.dispatchAll(null, dispatchOptions, errorInfo));
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(1, callbackHandler.channelEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            Thread.sleep(1000);

            /*
             * Call dispatchAll which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback. When that returns, reactor will send the
             * DirectoryRequest.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            dispatchOptions.clear();
            assertEquals(0, reactor.dispatchAll(null, dispatchOptions, errorInfo));
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was
            // called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            verifyMessage(loginMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);
            verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            verifyMessage(msgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);

            /*
             * wait for testServer to read the DirectoryRequest, then send the
             * DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            Thread.sleep(1000);

            /*
             * Call dispatchAll which should read the DirectoryResponse, invoke
             * rdmDirectoryMsgCallback, then send CHANNEL_READY
             * reactorChannelEventCallback.
             */
            dispatchOptions.clear();
            assertEquals(0, reactor.dispatchAll(null, dispatchOptions, errorInfo));
            // verify that the RDMDirectoryMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount()); // no
                                                                     // change
                                                                     // expected
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            verifyMessage(directoryMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.SOURCE);
            verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            // make sure CHANNEL_READY event received
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());

            // dispatchAll the next event. Dispatch should return 0, meaning
            // that
            // there is no more to read.
            assertEquals(0, reactor.dispatchAll(null, dispatchOptions, errorInfo));
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorConsumerDispatchNoKeySetMultipleChannels()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorCallbackHandler callbackHandler = null;
        ExecutorService executorService1 = null;
        ExecutorService executorService2 = null;

        try
        {
            NetworkReplay replay1 = parseReplayFile(inputFile);
            NetworkReplay replay2 = parseReplayFile(inputFile);

            /*
             * create 3 testServers which will send RIPC ConnectAck.
             */
            int serverPort1 = ++_serverPort;
            TestServer testServer1 = new TestServer(serverPort1);
            executorService1 = Executors.newSingleThreadExecutor();
            executorService1.execute(testServer1);
            int serverPort2 = ++_serverPort;
            TestServer testServer2 = new TestServer(serverPort2);
            executorService2 = Executors.newSingleThreadExecutor();
            executorService2.execute(testServer2);

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create 3 Client Connections.
             */
            ReactorConnectOptions rcOpts1 = createDefaultConsumerConnectOptions(String.valueOf(serverPort1));
            ReactorConnectOptions rcOpts2 = createDefaultConsumerConnectOptions(String.valueOf(serverPort2));
            callbackHandler = new ReactorCallbackHandler(SelectorProvider.provider().openSelector());
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts1, consumerRole, errorInfo);
            reactor.connect(rcOpts2, consumerRole, errorInfo);

            Thread.sleep(1000);

            // wait for the TestServer to accept a connection.
            testServer1.wait(TestServer.State.ACCEPTED);
            testServer2.wait(TestServer.State.ACCEPTED);

            Thread.sleep(1000);

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer1.readMessageFromSocket() > 0);
            verifyConnectReq(testServer1.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer1.writeMessageToSocket(replay1.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer1.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer1.buffer().get(2)); // verify
                                                                     // KEY_EXCHANGE
                                                                     // flag
            assertTrue(testServer2.readMessageFromSocket() > 0);
            verifyConnectReq(testServer2.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer2.writeMessageToSocket(replay2.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer2.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer2.buffer().get(2)); // verify
                                                                     // KEY_EXCHANGE
                                                                     // flag

            Thread.sleep(1000);

            /*
             * Call dispatchAll on the reactor. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
            assertEquals(ReactorReturnCodes.PARAMETER_OUT_OF_RANGE, dispatchOptions.maxMessages(0));
            assertEquals(ReactorReturnCodes.SUCCESS, dispatchOptions.maxMessages(1));
            // Call dispatchAll on the Reactor.
            // process first worker event
            dispatchOptions.clear();
            // confirm all events are dispatched
            assertEquals(0, reactor.dispatchAll(null, dispatchOptions, errorInfo));
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer1.wait(TestServer.State.READABLE);
            // wait and read one message.
            assertTrue(testServer1.readMessageFromSocket() > 0);
            verifyMessage(testServer1.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer1.writeMessageToSocket(replay1.read());

            testServer2.wait(TestServer.State.READABLE);
            // wait and read one message.
            assertTrue(testServer2.readMessageFromSocket() > 0);
            verifyMessage(testServer2.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer2.writeMessageToSocket(replay2.read());

            Thread.sleep(1000);

            /*
             * Call dispatchAll which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback. When that returns, reactor will send the
             * DirectoryRequest.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            dispatchOptions.clear();
            dispatchOptions.maxMessages(10);
            assertEquals(0, reactor.dispatchAll(null, dispatchOptions, errorInfo));
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was
            // called.
            assertEquals(2, callbackHandler.loginMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            verifyMessage(loginMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);
            verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            verifyMessage(msgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.LOGIN);

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);

            /*
             * wait for testServer to read the DirectoryRequest, then send the
             * DirectoryResponse
             */
            testServer1.wait(TestServer.State.READABLE);
            // wait and read one message.
            assertTrue(testServer1.readMessageFromSocket() > 0);
            verifyMessage(testServer1.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer1.writeMessageToSocket(replay1.read());

            testServer2.wait(TestServer.State.READABLE);
            // wait and read one message.
            assertTrue(testServer2.readMessageFromSocket() > 0);
            verifyMessage(testServer2.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer2.writeMessageToSocket(replay2.read());

            Thread.sleep(1000);

            /*
             * Call dispatchAll which should read the DirectoryResponse, invoke
             * rdmDirectoryMsgCallback, then send CHANNEL_READY
             * reactorChannelEventCallback.
             */
            dispatchOptions.clear();
            // process channel events
            assertEquals(0, reactor.dispatchAll(null, dispatchOptions, errorInfo));
            // verify that the RDMDirectoryMsgCallback was called.
            assertEquals(2, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount()); // no
                                                                     // change
                                                                     // expected
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            verifyMessage(directoryMsgEvent.transportBuffer(), MsgClasses.REFRESH, DomainTypes.SOURCE);
            verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            // make sure CHANNEL_READY event received
            assertEquals(4, callbackHandler.channelEventCount());
            assertEquals(2, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());

            // dispatchAll the next event. Dispatch should return 0, meaning
            // that
            // there is no more to read.
            assertEquals(0, reactor.dispatchAll(null, dispatchOptions, errorInfo));

            testServer1.shutDown();
            testServer2.shutDown();
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());

            executorService1.shutdownNow();
            executorService2.shutdownNow();
        }
    }

    @Test
    public void writeToWorkerThreadFailTest()
    {
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;

        try
        {
            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            ReactorChannel theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            Selector selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions("13456");
            ReactorCallbackHandler callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);

            // save original worker queue
            SelectableBiDirectionalQueue oldWorkerQueue = reactor._workerQueue;
            // create a mock SelectableBiDirectionalQueue
            SelectableBiDirectionalQueue workerQueue = Mockito.mock(SelectableBiDirectionalQueue.class);
            // the first call to write() will fail
            when(workerQueue.write(Mockito.any(VaNode.class))).thenReturn(false);
            // set reactor worker queue to mock worker queue
            reactor._workerQueue = workerQueue;
            assertEquals(ReactorReturnCodes.FAILURE, reactor.connect(rcOpts, consumerRole, errorInfo));
            assertTrue("sendWorkerEvent() failed".equals(errorInfo._error.text()));

            // verify that the ReactorChannelEventCallback was called.
            assertEquals(1, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelDownEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());

            // reset reactor worker queue back to original worker queue
            reactor._workerQueue = oldWorkerQueue;
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            errorInfo.clear();
            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());
        }
    }

    @Test
    public void reactorConnectionRecoveryTest()
    {
        final String inputFile = BASE_RIPC_TEST_DATA_DIR_NAME + "/10_input_connectAck_Ripc14.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);
            NetworkReplay replayBackup = parseReplayFile(inputFile);
            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            TestServer testServer = new TestServer(serverPort);
            testServer.setupServerSocket();
            int serverPortBackup = ++_serverPort;
            TestServer testServerBackup = new TestServer(serverPortBackup);
            testServerBackup.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createConnectOptionsWithBackup(String.valueOf(serverPort), String.valueOf(serverPortBackup));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the replay msg to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            // dispatch on the reactor's reactorChannel. There should
            // be one "WorkerEvent" to dispatch on. There should
            // be two ReactorChannelEventCallbacks waiting, 1)
            // CHANNEL_UP and 2) CHANNEL_READY.
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            
            ReactorChannel reactorChannel = callbackHandler.lastReactorChannel();
            
            assertTrue(reactorChannel.userSpecObj().equals("userSpecObject: " + serverPort));

            testServer.shutDown();

            // wait for the reactor to signal us that the channel is down
            int updCnt = selector.select(10000); // 10 seconds

            assertTrue(updCnt > 0);
            Set<SelectionKey> keys = selector.selectedKeys();
            assertNotNull(keys);
            assertEquals(1, keys.size());
            Iterator<SelectionKey> iter = keys.iterator();
            assertTrue(iter.hasNext());
            SelectionKey key = iter.next();
            assertNotNull(key);
            iter.remove();

            Thread.sleep(45000);

            // dispatch channel down event
            ReactorDispatchOptions dispatchOptions = ReactorFactory.createReactorDispatchOptions();
            ReactorChannel rc = reactor.reactorChannel();
            assertEquals(0, rc.dispatch(dispatchOptions, errorInfo));
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelDownEventCount());
            event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, event.eventType());

            // wait for the Backup TestServer to accept a connection.
            testServerBackup.waitForAcceptable();
            testServerBackup.acceptSocket();

            // wait and read one message.
            assertTrue(testServerBackup.readMessageFromSocket() > 0);
            verifyConnectReq(testServerBackup.buffer());
            // have the TestServer send the replay msg to the Reactor.
            testServerBackup.writeMessageToSocket(replayBackup.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServerBackup.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServerBackup.buffer().get(2)); // verify
                                                                          // KEY_EXCHANGE
                                                                          // flag

            // dispatch on the reactor's reactorChannel. There should
            // be one "WorkerEvent" to dispatch on. There should
            // be two ReactorChannelEventCallbacks waiting, 1)
            // CHANNEL_UP and 2) CHANNEL_READY.
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(5, callbackHandler.channelEventCount());
            assertEquals(2, callbackHandler.channelUpEventCount());
            assertEquals(2, callbackHandler.channelReadyEventCount());
            event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            assertTrue(reactorChannel.userSpecObj().equals("userSpecObject: " + serverPortBackup));

            testServerBackup.shutDown();
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorPackedSubmitTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestServer testServer = null;

        try
        {
            NetworkReplay replay = parseReplayFile(inputFile);

            /*
             * create a testServer which will send RIPC ConnectAck.
             */
            int serverPort = ++_serverPort;
            testServer = new TestServer(serverPort);
            testServer.setupServerSocket();

            /*
             * create ReactorErrorInfo.
             */
            errorInfo = ReactorFactory.createReactorErrorInfo();
            assertNotNull(errorInfo);

            /*
             * create a Reactor.
             */
            reactor = createReactor(errorInfo);

            assertEquals(false, reactor.isShutdown());

            /*
             * create a selector and register with the reactor's reactorChannel.
             */
            theReactorChannel = reactor.reactorChannel();
            assertNotNull(theReactorChannel);

            selector = SelectorProvider.provider().openSelector();
            theReactorChannel.selectableChannel().register(selector, SelectionKey.OP_READ, theReactorChannel);

            /*
             * create a Client Connection.
             */
            ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createDefaultConsumerRole(callbackHandler);
            // make sure login and directory requests are null
            assertEquals(null, consumerRole._loginRequest);
            assertEquals(null, consumerRole._directoryRequest);
            // make sure dictionary download mode is none
            assertEquals(DictionaryDownloadModes.NONE, consumerRole.dictionaryDownloadMode());
            reactor.connect(rcOpts, consumerRole, errorInfo);

            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // have the TestServer read a message (the RIPC ConnectReq)
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyConnectReq(testServer.buffer());
            // have the TestServer send the ConnectAck to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag

            /*
             * dispatch on the reactor's reactorChannel. There should be one
             * "WorkerEvent" to dispatch on. There should be one
             * ReactorChannelEventCallback waiting, 1) CHANNEL_UP.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent channelEvent = callbackHandler.lastChannelEvent();
            assertNotNull(channelEvent);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

            // disable xml tracing
            
            ReactorChannel reactorChannel = callbackHandler.lastReactorChannel();

            // get 500 byte buffer
            TransportBuffer sendbuffer = reactorChannel.getBuffer(500, true, errorInfo);
            assertEquals(500, sendbuffer.length());

            ByteBuffer bb = sendbuffer.data();
            bb.put("packed test #1".getBytes());
            assertTrue(reactorChannel.packBuffer(sendbuffer, errorInfo) > 0);

            bb.put("packed test #2".getBytes());
            assertTrue(reactorChannel.packBuffer(sendbuffer, errorInfo) > 0);

            bb.put("final packing test #3".getBytes());

            submitOptions.writeArgs().priority(WritePriorities.HIGH);
            submitOptions.writeArgs().flags(WriteFlags.DIRECT_SOCKET_WRITE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(sendbuffer, submitOptions, errorInfo));
            assertEquals(58, submitOptions.writeArgs().uncompressedBytesWritten());
            assertEquals(58, submitOptions.writeArgs().bytesWritten());
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */
            /*
             * Wait for testServer to read the packed message.
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);

            // should equal output from ETAC
            byte[] expected = ParseHexFile.parse(BASE_PACK_TEST_DATA_DIR_NAME + "/Packed1.txt");
            assertNotNull(expected);
            byte[] bytes = new byte[testServer.buffer().position()];
            for (int i = 0; i < testServer.buffer().position(); i++)
            {
                bytes[i] = testServer.buffer().get(i);
            }
            assertArrayEquals(expected, bytes);

            sendbuffer = reactorChannel.getBuffer(500, true, errorInfo);

            bb = sendbuffer.data();
            bb.put("packing2 test #1".getBytes());
            assertTrue(reactorChannel.packBuffer(sendbuffer, errorInfo) > 0);

            bb.put("packing2 test #2".getBytes());
            assertTrue(reactorChannel.packBuffer(sendbuffer, errorInfo) > 0);

            bb.put("packing3 final test #3".getBytes());
            assertTrue(reactorChannel.packBuffer(sendbuffer, errorInfo) > 0);

            submitOptions.writeArgs().priority(WritePriorities.HIGH);
            submitOptions.writeArgs().flags(WriteFlags.DIRECT_SOCKET_WRITE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel.submit(sendbuffer, submitOptions, errorInfo));
            assertEquals(63, submitOptions.writeArgs().uncompressedBytesWritten());
            assertEquals(63, submitOptions.writeArgs().bytesWritten());
            ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            /*
             * Wait for testServer to read the packed message.
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);

            // should equal output from ETAC
            expected = ParseHexFile.parse(BASE_PACK_TEST_DATA_DIR_NAME + "/Packed2.txt");
            assertNotNull(expected);
            bytes = new byte[testServer.buffer().position()];
            for (int i = 0; i < testServer.buffer().position(); i++)
            {
                bytes[i] = testServer.buffer().get(i);
            }
            assertArrayEquals(expected, bytes);
        }
        catch (Exception e)
        {
            assertTrue("exception occurred" + e.getLocalizedMessage(), false);
        }
        finally
        {
            testServer.shutDown();

            if (theReactorChannel != null)
            {
                SelectionKey key = theReactorChannel.selectableChannel().keyFor(selector);
                key.cancel();
            }

            assertNotNull(reactor);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertNotNull(errorInfo);
            assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
            assertEquals(true, reactor.isShutdown());

            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void reactorReconnectTest_reconnectDelay()
    {
        /* Test that reconnection works and follows the expected delay timer. */

        TestReactorEvent event;
        ReactorChannelEvent channelEvent;
        RDMLoginMsgEvent loginMsgEvent;

        final int reconnectMinDelay = 1000, reconnectMaxDelay = 4000;
        long expectedReconnectDelayTimeMs = reconnectMinDelay;
        long startTimeNano, deviationTimeMs;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);

        ConsumerProviderSessionOptions sessionOpts = new ConsumerProviderSessionOptions();
        sessionOpts.reconnectAttemptLimit(-1);
        sessionOpts.reconnectMinDelay(reconnectMinDelay);
        sessionOpts.reconnectMaxDelay(reconnectMaxDelay);

        consumer.testReactor().connect(sessionOpts, consumer, TestReactorComponent.nextReservedServerPort());
        for (int i = 0; i < 3; ++i)
        {

            /*
             * Calculate delay time (first failure is immediate, others will be
             * delayed accordingly)
             */
            if (i == 0)
                expectedReconnectDelayTimeMs = 0;
            else
            {
                expectedReconnectDelayTimeMs = reconnectMinDelay;
                for (int j = 1; j < i; ++j)
                    expectedReconnectDelayTimeMs *= 2;

                if (expectedReconnectDelayTimeMs > reconnectMaxDelay)
                    expectedReconnectDelayTimeMs = reconnectMaxDelay;
            }

            /* Consumer receives channel-down event, at the expected time. */
            startTimeNano = System.nanoTime();
            consumerReactor.dispatch(1, reconnectMaxDelay * 2);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            channelEvent = (ReactorChannelEvent)event.reactorEvent();
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());

            /*
             * Test that the delay was as expected. It takes time to discover
             * that the server port is closed (so being lenient on the upper
             * bound), but should take at least as long as the reconnect delay.
             */
            deviationTimeMs = (event.nanoTime() - startTimeNano) / 1000000 - expectedReconnectDelayTimeMs;
            assertTrue("Reconnection delay was " + (-deviationTimeMs) + "ms too soon.", deviationTimeMs >= -150);
            assertTrue("Reconnection delay was " + (-deviationTimeMs) + "ms too long.", deviationTimeMs <= 3000);
        }

        provider.bindForReconnectTest(sessionOpts);
        provider.testReactor().accept(sessionOpts, provider);

        /* Provider receives channel-up/channel-ready */
        provider.testReactor().dispatch(2);

        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

        /* Consumer receives channel-up */
        consumerReactor.dispatch(1);
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

        /* Provider receives login request. */
        providerReactor.dispatch(1);
        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
        loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
        assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());

        provider.close();

        /*
         * Test fails unless select is called because server still looks open to
         * client. Suspect it has something to do with the channel array or
         * cancelled key map but haven't pinned down anything for sure. For now,
         * will dispatch the reactor to ensure no events are received, but the
         * select call is enough to stop the problem.
         */
        // try { providerReactor._selector.select(); } catch
        // (java.io.IOException e) { assertTrue(false); }
        providerReactor.dispatch(0);

        for (int i = 0; i < 3; ++i)
        {

            /*
             * Calculate delay time (first failure is immediate, others will be
             * delayed accordingly)
             */
            if (i == 0)
                expectedReconnectDelayTimeMs = 0;
            else
            {
                expectedReconnectDelayTimeMs = reconnectMinDelay;
                for (int j = 1; j < i; ++j)
                    expectedReconnectDelayTimeMs *= 2;

                if (expectedReconnectDelayTimeMs > reconnectMaxDelay)
                    expectedReconnectDelayTimeMs = reconnectMaxDelay;
            }

            /* Consumer receives channel-down event, at the expected time. */
            startTimeNano = System.nanoTime();
            consumerReactor.dispatch(1, reconnectMaxDelay * 2);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            channelEvent = (ReactorChannelEvent)event.reactorEvent();
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());

            /*
             * Test that the delay was as expected. It takes time to discover
             * that the server port is closed (so being lenient on the upper
             * bound), but should take at least as long as the reconnect delay.
             */
            deviationTimeMs = (event.nanoTime() - startTimeNano) / 1000000 - expectedReconnectDelayTimeMs;
            assertTrue("Reconnection delay was " + (-deviationTimeMs) + "ms too soon.", deviationTimeMs >= -100);
            assertTrue("Reconnection delay was " + (-deviationTimeMs) + "ms too long.", deviationTimeMs <= 3000);
        }

        consumer.close();
        consumerReactor.close();
        providerReactor.close();

    }

    @Test
    public void reactorReconnectTest_reconnectLimit()
    {
        /* Test that reconnection works and stops when the limit is reached. */
        TestReactorEvent event;
        ReactorChannelEvent channelEvent;

        final int reconnectMinDelay = 1000, reconnectMaxDelay = 4000;
        final int reconnectAttemptLimit = 3;
        long expectedReconnectDelayTimeMs = reconnectMinDelay;
        long startTimeNano, deviationTimeMs;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);

        ConsumerProviderSessionOptions sessionOpts = new ConsumerProviderSessionOptions();
        sessionOpts.reconnectAttemptLimit(3);
        sessionOpts.reconnectMinDelay(reconnectMinDelay);
        sessionOpts.reconnectMaxDelay(reconnectMaxDelay);

        consumer.testReactor().connect(sessionOpts, consumer, TestReactorComponent.nextReservedServerPort());
        for (int i = 0; i < reconnectAttemptLimit + 1; ++i)
        {
            /*
             * Calculate delay time (first failure is immediate, others will be
             * delayed accordingly)
             */
            if (i == 0)
                expectedReconnectDelayTimeMs = 0;
            else
            {
                expectedReconnectDelayTimeMs = reconnectMinDelay;
                for (int j = 1; j < i; ++j)
                    expectedReconnectDelayTimeMs *= 2;

                if (expectedReconnectDelayTimeMs > reconnectMaxDelay)
                    expectedReconnectDelayTimeMs = reconnectMaxDelay;
            }

            /* Consumer receives channel-down event, at the expected time. */
            startTimeNano = System.nanoTime();
            consumerReactor.dispatch(1, reconnectMaxDelay * 2);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            channelEvent = (ReactorChannelEvent)event.reactorEvent();

            /*
             * We get one CHANNEL_DOWN_RECONNECTING for the initial failure,
             * then two more for each of the first two reconnect attempts. When
             * the third reconnection attempt fails, we get CHANNEL_DOWN.
             */
            if (i < reconnectAttemptLimit)
                assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
            else
                assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());

            /*
             * Test that the delay was as expected. It takes time to discover
             * that the server port is closed (so being lenient on the upper
             * bound), but should take at least as long as the reconnect delay.
             */
            deviationTimeMs = (event.nanoTime() - startTimeNano) / 1000000 - expectedReconnectDelayTimeMs;
            assertTrue("Reconnection delay was " + (-deviationTimeMs) + "ms too soon.", deviationTimeMs >= -100);
            assertTrue("Reconnection delay was " + (-deviationTimeMs) + "ms too long.", deviationTimeMs <= 3000);
        }

        consumer.close();
        consumerReactor.close();
    }

    /*
     * Consumer that delays returning from a CHANNEL_DOWN_RECONNECTING event.
     * Used by reactorReconnectTest_delayOnChannelCallback.
     */
    class SleepOnChannelReconnectConsumer extends Consumer
    {
        public SleepOnChannelReconnectConsumer(TestReactor testReactor)
        {
            super(testReactor);
        }

        @Override
        public int reactorChannelEventCallback(ReactorChannelEvent event)
        {
            if (event.eventType() == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
            {
                /* Sleep for half a second before handling the event. */
                do
                {
                    try
                    {
                        Thread.sleep(1500);
                        /*
                         * Since the channel was active, the reactorChannel
                         * should still have a selectableChannel and key.
                         */
                        assertNotNull(_reactorChannel.selectableChannel());
                        assertNotNull(_reactorChannel.selectableChannel().keyFor(_testReactor._selector));
                        break;
                    }
                    catch (InterruptedException e)
                    {
                        /*
                         * Interrupted while sleeping; ignore and sleep again.
                         */}

                }
                while (true);
            }
            return super.reactorChannelEventCallback(event);
        }
    }

    @Test
    public void reactorReconnectTest_sleepInReconnectCallback()
    {
        /*
         * Test a previously-existing timing condition between the
         * reactorChannelEventCallback and connection recovery. Uses the
         * SleepOnChannelReconnectConsumer to delay returning from the callback,
         * to make the channel key can still be safely found and canceled before
         * any reconnection is attempted.
         */

        TestReactorEvent event;
        ReactorChannelEvent channelEvent;
        RDMLoginMsgEvent loginMsgEvent;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new SleepOnChannelReconnectConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);

        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);

        ConsumerProviderSessionOptions sessionOpts = new ConsumerProviderSessionOptions();
        sessionOpts.reconnectAttemptLimit(-1);
        sessionOpts.reconnectMinDelay(1000);
        sessionOpts.reconnectMaxDelay(1000);

        provider.bind(sessionOpts);
        consumer.testReactor().connect(sessionOpts, consumer, provider.serverPort());
        provider.testReactor().accept(sessionOpts, provider);

        /* Provider receives channel-up/channel-ready */
        provider.testReactor().dispatch(2);

        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

        /* Consumer receives channel-up */
        consumerReactor.dispatch(1);
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

        /* Provider receives login request. */
        providerReactor.dispatch(1);
        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
        loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
        assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());

        provider.closeChannel();

        /*
         * Consumer receives channel-down-reconnecting event. This is where the
         * consumer will wait half a second before handling the event.
         */

        long startTimeNano = System.nanoTime();
        consumerReactor.dispatch(1);
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());

        /*
         * Self check -- consumer sleeps before the TestReactorEvent is created,
         * so verify that at least about half a second has passed.
         */
        assertTrue((event.nanoTime() - startTimeNano) >= 1400000000L);

        /* Provider accepts new connection. */
        provider.testReactor().accept(sessionOpts, provider);

        /* Provider receives channel-up/channel-ready */
        provider.testReactor().dispatch(2);

        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

        /* Consumer receives channel-up */
        consumerReactor.dispatch(1);
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

        /* Provider receives login request. */
        providerReactor.dispatch(1);
        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
        loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
        assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());

        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();

    }

    @Test
    public void reactorReconnectTest_pingTimeout()
    {
        /* Test that reconnection works in response to a ping timeout. */
        TestReactorEvent event;
        ReactorChannelEvent channelEvent;
        ReadEvent readEvent;
        LoginMsg loginMsg;

        final int reconnectMinDelay = 1000, reconnectMaxDelay = 4000;
        final int pingTimeout = 10;
        long startTimeNano, deviationTimeMs;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);

        /* Create provider. */
        CoreComponent provider = new CoreComponent();

        ConsumerProviderSessionOptions sessionOpts = new ConsumerProviderSessionOptions();
        sessionOpts.reconnectAttemptLimit(-1);
        sessionOpts.reconnectMinDelay(reconnectMinDelay);
        sessionOpts.reconnectMaxDelay(reconnectMaxDelay);
        sessionOpts.pingTimeout(pingTimeout);

        provider.bind(sessionOpts);
        consumer.testReactor().connect(sessionOpts, consumer, provider.serverPort());
        provider.acceptAndInitChannel(sessionOpts.connectionType());

        /* Consumer receives channel up. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
        assertEquals(pingTimeout, consumer.reactorChannel().channel().pingTimeout());

        /* Provider receives login request. */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        loginMsg = readEvent.loginMsg();
        assertEquals(loginMsg.rdmMsgType(), LoginMsgType.REQUEST);

        /*
         * Core provider is active but won't send pings. Wait for the ping
         * timeout.
         */
        startTimeNano = System.nanoTime();
        consumerReactor.dispatch(1, pingTimeout * 1000 * 2);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
        deviationTimeMs = Math.abs((event.nanoTime() - startTimeNano) / 1000000 - pingTimeout * 1000);
        assertTrue("Ping timeout was off by " + deviationTimeMs + "ms.", deviationTimeMs <= 2000);

        /* Provider read fails. */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        assertTrue(readEvent.lastReadRet() < 0);
        provider.closeChannel();

        /* Provider receives reconnection. */
        provider.acceptAndInitChannel(sessionOpts.connectionType());

        /* Consumer receives channel up. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

        /* Provider receives login request. */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        loginMsg = readEvent.loginMsg();
        assertEquals(loginMsg.rdmMsgType(), LoginMsgType.REQUEST);

        consumer.close();
        provider.close();
        consumerReactor.close();
    }

    @Test
    public void reactorReconnectTest_initializationTimeout()
    {
        /*
         * Test that reconnection works in response to an initialization
         * timeout.
         */
        TestReactorEvent event;
        ReactorChannelEvent channelEvent;
        ReadEvent readEvent;
        LoginMsg loginMsg;

        final int reconnectMinDelay = 1000, reconnectMaxDelay = 4000;
        final int consumerInitializationTimeout = 10;
        long startTimeNano, deviationTimeMs;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);

        /* Create provider. */
        CoreComponent provider = new CoreComponent();

        ConsumerProviderSessionOptions sessionOpts = new ConsumerProviderSessionOptions();
        sessionOpts.reconnectAttemptLimit(-1);
        sessionOpts.reconnectMinDelay(reconnectMinDelay);
        sessionOpts.reconnectMaxDelay(reconnectMaxDelay);
        sessionOpts.consumerChannelInitTimeout(consumerInitializationTimeout);

        /*
         * Connect consumer & provider. Provider does not initialize the
         * channel.
         */
        provider.bind(sessionOpts);
        consumer.testReactor().connect(sessionOpts, consumer, provider.serverPort());
        provider.accept(sessionOpts.connectionType());

        /* Consumer receives channel-down-reconnecting event. */
        startTimeNano = System.nanoTime();
        consumerReactor.dispatch(1, consumerInitializationTimeout * 1000 * 2);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
        deviationTimeMs = Math.abs((event.nanoTime() - startTimeNano) / 1000000 - consumerInitializationTimeout * 1000);
        assertTrue("Initialization timeout was off by " + deviationTimeMs + "ms.", deviationTimeMs <= 2000);

        /* Provider read fails. */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        assertTrue(readEvent.lastReadRet() < 0);
        provider.closeChannel();

        /* Provider receives & accepts reconnection. */
        provider.acceptAndInitChannel(sessionOpts.connectionType());

        /* Consumer receives channel up. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

        /* Provider receives login request. */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        loginMsg = readEvent.loginMsg();
        assertEquals(loginMsg.rdmMsgType(), LoginMsgType.REQUEST);

        consumer.close();
        provider.close();
        consumerReactor.close();
    }

    @Test
    public void reactorConnectOptionsCopyTest()
    {
        /* Test the ReactorConnectOptions.copy() method. */
        ReactorConnectOptions srcReactorOpts = ReactorFactory.createReactorConnectOptions();
        ReactorConnectOptions destReactorOpts = ReactorFactory.createReactorConnectOptions();
        ReactorConnectInfo srcInfo1 = ReactorFactory.createReactorConnectInfo();
        ReactorConnectInfo srcInfo2 = ReactorFactory.createReactorConnectInfo();

        srcInfo1.connectOptions().componentVersion("HARTNELL");
        srcInfo1.connectOptions().connectionType(ConnectionTypes.SEQUENCED_MCAST);
        srcInfo1.connectOptions().compressionType(CompressionTypes.LZ4);
        srcInfo1.connectOptions().blocking(true);
        srcInfo1.connectOptions().pingTimeout(55);
        srcInfo1.connectOptions().guaranteedOutputBuffers(66);
        srcInfo1.connectOptions().numInputBuffers(77);
        srcInfo1.connectOptions().majorVersion(88);
        srcInfo1.connectOptions().minorVersion(99);
        srcInfo1.connectOptions().protocolType(128);
        srcInfo1.connectOptions().userSpecObject(this);
        srcInfo1.connectOptions().unifiedNetworkInfo().address("TROUGHTON");
        srcInfo1.connectOptions().unifiedNetworkInfo().serviceName("5555");
        srcInfo1.connectOptions().unifiedNetworkInfo().interfaceName("PERTWEE");
        srcInfo1.connectOptions().unifiedNetworkInfo().unicastServiceName("TOM.BAKER");
        srcInfo1.connectOptions().segmentedNetworkInfo().recvAddress("DAVISON");
        srcInfo1.connectOptions().segmentedNetworkInfo().recvServiceName("6666");
        srcInfo1.connectOptions().segmentedNetworkInfo().unicastServiceName("7777");
        srcInfo1.connectOptions().segmentedNetworkInfo().interfaceName("COLIN.BAKER");
        srcInfo1.connectOptions().segmentedNetworkInfo().sendAddress("MCCOY");
        srcInfo1.connectOptions().segmentedNetworkInfo().sendServiceName("MCGANN");
        srcInfo1.connectOptions().tunnelingInfo().tunnelingType("encrypted");
        srcInfo1.connectOptions().tunnelingInfo().HTTPproxy(true);
        srcInfo1.connectOptions().tunnelingInfo().HTTPproxyHostName("HURT");
        srcInfo1.connectOptions().tunnelingInfo().HTTPproxyPort(8888);
        srcInfo1.connectOptions().tunnelingInfo().objectName("ECCLESTON");
        srcInfo1.connectOptions().tunnelingInfo().KeystoreType("TENNANT");
        srcInfo1.connectOptions().tunnelingInfo().KeystoreFile("SMITH");
        srcInfo1.connectOptions().tunnelingInfo().KeystorePasswd("CAPALDI");
        srcInfo1.connectOptions().tunnelingInfo().SecurityProtocol("STEWART");
        srcInfo1.connectOptions().tunnelingInfo().SecurityProvider("FRAKES");
        srcInfo1.connectOptions().tunnelingInfo().KeyManagerAlgorithm("DORN");
        srcInfo1.connectOptions().tunnelingInfo().TrustManagerAlgorithm("SIRTIS");
        srcInfo1.connectOptions().credentialsInfo().HTTPproxyUsername("SPINER");
        srcInfo1.connectOptions().credentialsInfo().HTTPproxyPasswd("BURTON");
        srcInfo1.connectOptions().credentialsInfo().HTTPproxyDomain("CRUSHER");
        srcInfo1.connectOptions().credentialsInfo().HTTPproxyLocalHostname("WHEATON");
        srcInfo1.connectOptions().credentialsInfo().HTTPproxyKRB5configFile("SCHULTZ");
        srcInfo1.connectOptions().tcpOpts().tcpNoDelay(true);
        srcInfo1.connectOptions().multicastOpts().disconnectOnGaps(true);
        srcInfo1.connectOptions().multicastOpts().packetTTL(99);
        srcInfo1.connectOptions().multicastOpts().tcpControlPort("9999");
        srcInfo1.connectOptions().multicastOpts().portRoamRange(12);
        srcInfo1.connectOptions().shmemOpts().maxReaderLag(11111);
        srcInfo1.connectOptions().channelReadLocking(true);
        srcInfo1.connectOptions().channelWriteLocking(true);
        srcInfo1.connectOptions().sysSendBufSize(22222);
        srcInfo1.connectOptions().sysRecvBufSize(33333);
        srcInfo1.connectOptions().seqMCastOpts().maxMsgSize(44444);
        srcInfo1.connectOptions().seqMCastOpts().instanceId(55555);
        srcInfo1.initTimeout(66666);

        srcInfo2.connectOptions().componentVersion("MULGREW");
        srcInfo2.connectOptions().connectionType(ConnectionTypes.UNIDIR_SHMEM);
        srcInfo2.connectOptions().compressionType(CompressionTypes.ZLIB);
        srcInfo2.connectOptions().blocking(false);
        srcInfo2.connectOptions().pingTimeout(66);
        srcInfo2.connectOptions().guaranteedOutputBuffers(77);
        srcInfo2.connectOptions().numInputBuffers(88);
        srcInfo2.connectOptions().majorVersion(99);
        srcInfo2.connectOptions().minorVersion(111);
        srcInfo2.connectOptions().protocolType(129);
        srcInfo2.connectOptions().userSpecObject(this);
        srcInfo2.connectOptions().unifiedNetworkInfo().address("RUSS");
        srcInfo2.connectOptions().unifiedNetworkInfo().serviceName("6666");
        srcInfo2.connectOptions().unifiedNetworkInfo().interfaceName("DAWSON");
        srcInfo2.connectOptions().unifiedNetworkInfo().unicastServiceName("PICARDO");
        srcInfo2.connectOptions().segmentedNetworkInfo().recvAddress("BELTRAN");
        srcInfo2.connectOptions().segmentedNetworkInfo().recvServiceName("7777");
        srcInfo2.connectOptions().segmentedNetworkInfo().unicastServiceName("8888");
        srcInfo2.connectOptions().segmentedNetworkInfo().interfaceName("WANG");
        srcInfo2.connectOptions().segmentedNetworkInfo().sendAddress("PHILIPS");
        srcInfo2.connectOptions().segmentedNetworkInfo().sendServiceName("MCNEIL");
        srcInfo2.connectOptions().tunnelingInfo().tunnelingType("http");
        srcInfo2.connectOptions().tunnelingInfo().HTTPproxy(false);
        srcInfo2.connectOptions().tunnelingInfo().HTTPproxyHostName("RYAN");
        srcInfo2.connectOptions().tunnelingInfo().HTTPproxyPort(9999);
        srcInfo2.connectOptions().tunnelingInfo().objectName("SIDDIG");
        srcInfo2.connectOptions().tunnelingInfo().KeystoreType("VISITOR");
        srcInfo2.connectOptions().tunnelingInfo().KeystoreFile("AUBERJONOIS");
        srcInfo2.connectOptions().tunnelingInfo().KeystorePasswd("SHIMERMAN");
        srcInfo2.connectOptions().tunnelingInfo().SecurityProtocol("FARRELL");
        srcInfo2.connectOptions().tunnelingInfo().SecurityProvider("MEANEY");
        srcInfo2.connectOptions().tunnelingInfo().KeyManagerAlgorithm("LOFTON");
        srcInfo2.connectOptions().tunnelingInfo().TrustManagerAlgorithm("BOER");
        srcInfo2.connectOptions().credentialsInfo().HTTPproxyUsername("SHATNER");
        srcInfo2.connectOptions().credentialsInfo().HTTPproxyPasswd("KELLEY");
        srcInfo2.connectOptions().credentialsInfo().HTTPproxyDomain("NICHOLS");
        srcInfo2.connectOptions().credentialsInfo().HTTPproxyLocalHostname("DOOHAN");
        srcInfo2.connectOptions().credentialsInfo().HTTPproxyKRB5configFile("TAKEI");
        srcInfo2.connectOptions().tcpOpts().tcpNoDelay(false);
        srcInfo2.connectOptions().multicastOpts().disconnectOnGaps(false);
        srcInfo2.connectOptions().multicastOpts().packetTTL(1111);
        srcInfo2.connectOptions().multicastOpts().tcpControlPort("11111");
        srcInfo2.connectOptions().multicastOpts().portRoamRange(13);
        srcInfo2.connectOptions().shmemOpts().maxReaderLag(22222);
        srcInfo2.connectOptions().channelReadLocking(false);
        srcInfo2.connectOptions().channelWriteLocking(false);
        srcInfo2.connectOptions().sysSendBufSize(33333);
        srcInfo2.connectOptions().sysRecvBufSize(44444);
        srcInfo2.connectOptions().seqMCastOpts().maxMsgSize(55555);
        srcInfo2.connectOptions().seqMCastOpts().instanceId(66666);
        srcInfo2.initTimeout(77777);

        srcReactorOpts.connectionList().add(srcInfo1);
        srcReactorOpts.connectionList().add(srcInfo2);
        srcReactorOpts.reconnectAttemptLimit(10);
        srcReactorOpts.reconnectMinDelay(1234);
        srcReactorOpts.reconnectMaxDelay(4321);

        /* Copy options */
        srcReactorOpts.copy(destReactorOpts);

        assertEquals(2, srcReactorOpts.connectionList().size());
        assertEquals(2, destReactorOpts.connectionList().size());

        /* Ensure source and destination options have the correct values. */
        for (int i = 0; i < 2; ++i)
        {
            ReactorConnectOptions reactorConnectOptions = ((i == 0) ? destReactorOpts
                    : srcReactorOpts);
            ConnectOptions testOpts = reactorConnectOptions.connectionList().get(0).connectOptions();

            assertEquals(10, reactorConnectOptions.reconnectAttemptLimit());
            assertEquals(1234, reactorConnectOptions.reconnectMinDelay());
            assertEquals(4321, reactorConnectOptions.reconnectMaxDelay());

            assertEquals(66666, reactorConnectOptions.connectionList().get(0).initTimeout());
            assertTrue(testOpts.componentVersion().toString().equals("HARTNELL"));
            assertEquals(ConnectionTypes.SEQUENCED_MCAST, testOpts.connectionType());
            assertEquals(CompressionTypes.LZ4, testOpts.compressionType());
            assertEquals(true, testOpts.blocking());
            assertEquals(55, testOpts.pingTimeout());
            assertEquals(66, testOpts.guaranteedOutputBuffers());
            assertEquals(77, testOpts.numInputBuffers());
            assertEquals(88, testOpts.majorVersion());
            assertEquals(99, testOpts.minorVersion());
            assertEquals(128, testOpts.protocolType());
            assertEquals(this, testOpts.userSpecObject());
            assertTrue(testOpts.unifiedNetworkInfo().address().equals("TROUGHTON"));
            assertTrue(testOpts.unifiedNetworkInfo().serviceName().equals("5555"));
            assertTrue(testOpts.unifiedNetworkInfo().interfaceName().equals("PERTWEE"));
            assertTrue(testOpts.unifiedNetworkInfo().unicastServiceName().equals("TOM.BAKER"));
            assertTrue(testOpts.segmentedNetworkInfo().recvAddress().equals("DAVISON"));
            assertTrue(testOpts.segmentedNetworkInfo().recvServiceName().equals("6666"));
            assertTrue(testOpts.segmentedNetworkInfo().unicastServiceName().equals("7777"));
            assertTrue(testOpts.segmentedNetworkInfo().interfaceName().equals("COLIN.BAKER"));
            assertTrue(testOpts.segmentedNetworkInfo().sendAddress().equals("MCCOY"));
            assertTrue(testOpts.segmentedNetworkInfo().sendServiceName().equals("MCGANN"));
            assertTrue(testOpts.tunnelingInfo().tunnelingType().equals("encrypted"));
            assertEquals(true, testOpts.tunnelingInfo().HTTPproxy());
            assertTrue(testOpts.tunnelingInfo().HTTPproxyHostName().equals("HURT"));
            assertEquals(8888, testOpts.tunnelingInfo().HTTPproxyPort());
            assertTrue(testOpts.tunnelingInfo().objectName().equals("ECCLESTON"));
            assertTrue(testOpts.tunnelingInfo().KeystoreType().equals("TENNANT"));
            assertTrue(testOpts.tunnelingInfo().KeystoreFile().equals("SMITH"));
            assertTrue(testOpts.tunnelingInfo().KeystorePasswd().equals("CAPALDI"));
            assertTrue(testOpts.tunnelingInfo().SecurityProtocol().equals("STEWART"));
            assertTrue(testOpts.tunnelingInfo().SecurityProvider().equals("FRAKES"));
            assertTrue(testOpts.tunnelingInfo().KeyManagerAlgorithm().equals("DORN"));
            assertTrue(testOpts.tunnelingInfo().TrustManagerAlgorithm().equals("SIRTIS"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyUsername().equals("SPINER"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyPasswd().equals("BURTON"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyDomain().equals("CRUSHER"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyLocalHostname().equals("WHEATON"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyKRB5configFile().equals("SCHULTZ"));
            assertEquals(true, testOpts.tcpOpts().tcpNoDelay());
            assertEquals(true, testOpts.multicastOpts().disconnectOnGaps());
            assertEquals(99, testOpts.multicastOpts().packetTTL());
            assertTrue(testOpts.multicastOpts().tcpControlPort().equals("9999"));
            assertEquals(12, testOpts.multicastOpts().portRoamRange());
            assertEquals(11111, testOpts.shmemOpts().maxReaderLag());
            assertEquals(true, testOpts.channelReadLocking());
            assertEquals(true, testOpts.channelWriteLocking());
            assertEquals(22222, testOpts.sysSendBufSize());
            assertEquals(33333, testOpts.sysRecvBufSize());
            assertEquals(44444, testOpts.seqMCastOpts().maxMsgSize());
            assertEquals(55555, testOpts.seqMCastOpts().instanceId());

            testOpts = reactorConnectOptions.connectionList().get(1).connectOptions();

            assertEquals(77777, reactorConnectOptions.connectionList().get(1).initTimeout());
            assertTrue(testOpts.componentVersion().toString().equals("MULGREW"));
            assertEquals(ConnectionTypes.UNIDIR_SHMEM, testOpts.connectionType());
            assertEquals(CompressionTypes.ZLIB, testOpts.compressionType());
            assertEquals(false, testOpts.blocking());
            assertEquals(66, testOpts.pingTimeout());
            assertEquals(77, testOpts.guaranteedOutputBuffers());
            assertEquals(88, testOpts.numInputBuffers());
            assertEquals(99, testOpts.majorVersion());
            assertEquals(111, testOpts.minorVersion());
            assertEquals(129, testOpts.protocolType());
            assertEquals(this, testOpts.userSpecObject());
            assertTrue(testOpts.unifiedNetworkInfo().address().equals("RUSS"));
            assertTrue(testOpts.unifiedNetworkInfo().serviceName().equals("6666"));
            assertTrue(testOpts.unifiedNetworkInfo().interfaceName().equals("DAWSON"));
            assertTrue(testOpts.unifiedNetworkInfo().unicastServiceName().equals("PICARDO"));
            assertTrue(testOpts.segmentedNetworkInfo().recvAddress().equals("BELTRAN"));
            assertTrue(testOpts.segmentedNetworkInfo().recvServiceName().equals("7777"));
            assertTrue(testOpts.segmentedNetworkInfo().unicastServiceName().equals("8888"));
            assertTrue(testOpts.segmentedNetworkInfo().interfaceName().equals("WANG"));
            assertTrue(testOpts.segmentedNetworkInfo().sendAddress().equals("PHILIPS"));
            assertTrue(testOpts.segmentedNetworkInfo().sendServiceName().equals("MCNEIL"));
            assertTrue(testOpts.tunnelingInfo().tunnelingType().equals("http"));
            assertEquals(false, testOpts.tunnelingInfo().HTTPproxy());
            assertTrue(testOpts.tunnelingInfo().HTTPproxyHostName().equals("RYAN"));
            assertEquals(9999, testOpts.tunnelingInfo().HTTPproxyPort());
            assertTrue(testOpts.tunnelingInfo().objectName().equals("SIDDIG"));
            assertTrue(testOpts.tunnelingInfo().KeystoreType().equals("VISITOR"));
            assertTrue(testOpts.tunnelingInfo().KeystoreFile().equals("AUBERJONOIS"));
            assertTrue(testOpts.tunnelingInfo().KeystorePasswd().equals("SHIMERMAN"));
            assertTrue(testOpts.tunnelingInfo().SecurityProtocol().equals("FARRELL"));
            assertTrue(testOpts.tunnelingInfo().SecurityProvider().equals("MEANEY"));
            assertTrue(testOpts.tunnelingInfo().KeyManagerAlgorithm().equals("LOFTON"));
            assertTrue(testOpts.tunnelingInfo().TrustManagerAlgorithm().equals("BOER"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyUsername().equals("SHATNER"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyPasswd().equals("KELLEY"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyDomain().equals("NICHOLS"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyLocalHostname().equals("DOOHAN"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyKRB5configFile().equals("TAKEI"));
            assertEquals(false, testOpts.tcpOpts().tcpNoDelay());
            assertEquals(false, testOpts.multicastOpts().disconnectOnGaps());
            assertEquals(1111, testOpts.multicastOpts().packetTTL());
            assertTrue(testOpts.multicastOpts().tcpControlPort().equals("11111"));
            assertEquals(13, testOpts.multicastOpts().portRoamRange());
            assertEquals(22222, testOpts.shmemOpts().maxReaderLag());
            assertEquals(false, testOpts.channelReadLocking());
            assertEquals(false, testOpts.channelWriteLocking());
            assertEquals(33333, testOpts.sysSendBufSize());
            assertEquals(44444, testOpts.sysRecvBufSize());
            assertEquals(55555, testOpts.seqMCastOpts().maxMsgSize());
            assertEquals(66666, testOpts.seqMCastOpts().instanceId());
        }
    }

    /* create a reactor */
    static Reactor createReactor(ReactorErrorInfo errorInfo)
    {
        ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
        assertNotNull(reactorOptions);
        assertEquals(ReactorReturnCodes.PARAMETER_INVALID, reactorOptions.userSpecObj(null));
        String userSpecObject = "test";
        assertEquals(ReactorReturnCodes.SUCCESS, reactorOptions.userSpecObj(userSpecObject));

        Reactor reactor = ReactorFactory.createReactor(reactorOptions, errorInfo);
        assertNotNull(reactor);
        assertEquals(ReactorReturnCodes.SUCCESS, errorInfo.code());
        assertEquals(userSpecObject, reactor.userSpecObj());
        return reactor;
    }

    /* create default consumer connect options */
    static ReactorConnectOptions createDefaultConsumerConnectOptions(String serviceName)
    {
        ReactorConnectOptions rcOpts = ReactorFactory.createReactorConnectOptions();
        ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
        assertNotNull(rcOpts);
        assertEquals(ReactorReturnCodes.PARAMETER_OUT_OF_RANGE, connectInfo.initTimeout(0));
        assertEquals(ReactorReturnCodes.SUCCESS, connectInfo.initTimeout(10));
        connectInfo.connectOptions().connectionType(ConnectionTypes.SOCKET);
        connectInfo.connectOptions().majorVersion(Codec.majorVersion());
        connectInfo.connectOptions().minorVersion(Codec.minorVersion());
        connectInfo.connectOptions().unifiedNetworkInfo().serviceName(serviceName);
        connectInfo.connectOptions().unifiedNetworkInfo().address(LOCAL_ADDRESS);
        rcOpts.connectionList().add(connectInfo);
        return rcOpts;
    }

    /* create default consumer connect options with backup */
    ReactorConnectOptions createConnectOptionsWithBackup(String serviceName, String serviceNameBackup)
    {
        ReactorConnectOptions rcOpts = ReactorFactory.createReactorConnectOptions();
        assertNotNull(rcOpts);
        ReactorConnectInfo connectInfo1 = ReactorFactory.createReactorConnectInfo();
        assertEquals(ReactorReturnCodes.PARAMETER_OUT_OF_RANGE, connectInfo1.initTimeout(0));
        assertEquals(ReactorReturnCodes.SUCCESS, connectInfo1.initTimeout(10));
        connectInfo1.connectOptions().connectionType(ConnectionTypes.SOCKET);
        connectInfo1.connectOptions().majorVersion(Codec.majorVersion());
        connectInfo1.connectOptions().minorVersion(Codec.minorVersion());
        connectInfo1.connectOptions().unifiedNetworkInfo().serviceName(serviceName);
        connectInfo1.connectOptions().unifiedNetworkInfo().address(LOCAL_ADDRESS);
        connectInfo1.connectOptions().userSpecObject("userSpecObject: " + serviceName);
        rcOpts.connectionList().add(connectInfo1);
        ReactorConnectInfo connectInfo2 = ReactorFactory.createReactorConnectInfo();
        assertEquals(ReactorReturnCodes.PARAMETER_OUT_OF_RANGE, connectInfo2.initTimeout(0));
        assertEquals(ReactorReturnCodes.SUCCESS, connectInfo2.initTimeout(10));
        connectInfo2.connectOptions().connectionType(ConnectionTypes.SOCKET);
        connectInfo2.connectOptions().majorVersion(Codec.majorVersion());
        connectInfo2.connectOptions().minorVersion(Codec.minorVersion());
        connectInfo2.connectOptions().unifiedNetworkInfo().serviceName(serviceNameBackup);
        connectInfo2.connectOptions().unifiedNetworkInfo().address(LOCAL_ADDRESS);
        connectInfo2.connectOptions().userSpecObject("userSpecObject: " + serviceNameBackup);
        rcOpts.connectionList().add(connectInfo2);
        rcOpts.reconnectAttemptLimit(-1);
        rcOpts.reconnectMinDelay(1000);
        rcOpts.reconnectMaxDelay(1000);
        return rcOpts;
    }

    /* create default provider accept options */
    ReactorAcceptOptions createDefaultProviderAcceptOptions()
    {
        ReactorAcceptOptions raOpts = ReactorFactory.createReactorAcceptOptions();
        assertNotNull(raOpts);
        assertEquals(ReactorReturnCodes.PARAMETER_OUT_OF_RANGE, raOpts.initTimeout(0));
        assertEquals(ReactorReturnCodes.SUCCESS, raOpts.initTimeout(10));
        return raOpts;
    }

    /* create default consumer role */
    ConsumerRole createDefaultConsumerRole(ReactorCallbackHandler callbackHandler)
    {
        ConsumerRole consumerRole = ReactorFactory.createConsumerRole();
        assertNotNull(consumerRole);

        assertNotNull(callbackHandler);
        consumerRole.defaultMsgCallback(callbackHandler);
        consumerRole.channelEventCallback(callbackHandler);
        consumerRole.loginMsgCallback(callbackHandler);
        consumerRole.directoryMsgCallback(callbackHandler);
        consumerRole.dictionaryMsgCallback(callbackHandler);

        return consumerRole;
    }

    /* create default provider role */
    ProviderRole createDefaultProviderRole(ReactorCallbackHandler callbackHandler)
    {
        ProviderRole providerRole = ReactorFactory.createProviderRole();
        assertNotNull(providerRole);

        assertNotNull(callbackHandler);
        providerRole.defaultMsgCallback(callbackHandler);
        providerRole.channelEventCallback(callbackHandler);
        providerRole.loginMsgCallback(callbackHandler);
        providerRole.directoryMsgCallback(callbackHandler);
        providerRole.dictionaryMsgCallback(callbackHandler);

        return providerRole;
    }

    /* create default niprovider role */
    NIProviderRole createDefaultNIProviderRole(ReactorCallbackHandler callbackHandler)
    {
        NIProviderRole niProviderRole = ReactorFactory.createNIProviderRole();
        assertNotNull(niProviderRole);

        assertNotNull(callbackHandler);
        niProviderRole.defaultMsgCallback(callbackHandler);
        niProviderRole.channelEventCallback(callbackHandler);
        niProviderRole.loginMsgCallback(callbackHandler);

        return niProviderRole;
    }

    /* create default bind options */
    private BindOptions createDefaultBindOptions()
    {
        BindOptions bindOpts = TransportFactory.createBindOptions();
        bindOpts.userSpecObject("TEST SERVER");
        bindOpts.serviceName("14005");
        bindOpts.majorVersion(Codec.majorVersion());
        bindOpts.minorVersion(Codec.minorVersion());
        bindOpts.protocolType(Codec.protocolType());
        return bindOpts;
    }

    /* verify the byte buffer is a connect request */
    static void verifyConnectReq(ByteBuffer buffer)
    {
        // peak into the buffer and verify that it's a connectReq msg.
        assertEquals(0, buffer.get(2));
        int version = buffer.getInt(3);
        System.out.println("ReactorTest.verifyConnectReq: rcv'ed a ConnectReq, version=" + version);
    }

    /* verify byte buffer has correct MsgClass and DomainType */
    static void verifyMessage(ByteBuffer byteBuffer, int msgClass, int domainType)
    {
        System.out.println("ReactorTest.verifyMessage: checking message for msgClass="
                + MsgClasses.toString(msgClass) + " domain=" + DomainTypes.toString(domainType)
                + "\n" + Transport.toHexString(byteBuffer, 0, byteBuffer.position()));

        stripRipcHeader(byteBuffer);

        Buffer buffer = CodecFactory.createBuffer();
        assertNotNull(buffer);
        buffer.data(byteBuffer);
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        assertNotNull(dIter);
        dIter.clear();

        assertEquals(CodecReturnCodes.SUCCESS, dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion()));
        Msg msg = CodecFactory.createMsg();
        assertNotNull(msg);
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));
        assertEquals(msgClass, msg.msgClass());
        assertEquals(domainType, msg.domainType());
    }

    /* verify transport buffer has correct MsgClass and DomainType */
    static void verifyMessage(TransportBuffer transportBuffer, int msgClass, int domainType)
    {
        System.out.println("ReactorTest.verifyMessage: checking message for msgClass="
                + MsgClasses.toString(msgClass) + " domain=" + DomainTypes.toString(domainType)
                + "\n");

        int savePos = transportBuffer.data().position();
        transportBuffer.data().position(savePos);

        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        assertNotNull(dIter);
        dIter.clear();

        assertEquals(CodecReturnCodes.SUCCESS, dIter.setBufferAndRWFVersion(transportBuffer, Codec.majorVersion(), Codec.minorVersion()));
        Msg msg = CodecFactory.createMsg();
        assertNotNull(msg);
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));
        assertEquals(msgClass, msg.msgClass());
        assertEquals(domainType, msg.domainType());
    }

    /* verify byte buffer is a login request with specified token */
    static void verifyMessageWithToken(ByteBuffer byteBuffer, int msgClass, int domainType, Buffer token)
    {
        System.out.println("ReactorTest.verifyMessageWithToken: checking message for msgClass="
                + MsgClasses.toString(msgClass) + " domain=" + DomainTypes.toString(domainType)
                + "\n");

        stripRipcHeader(byteBuffer);

        Buffer buffer = CodecFactory.createBuffer();
        assertNotNull(buffer);
        buffer.data(byteBuffer);
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        assertNotNull(dIter);
        dIter.clear();

        assertEquals(CodecReturnCodes.SUCCESS, dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion()));
        Msg msg = CodecFactory.createMsg();
        assertNotNull(msg);
        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));
        assertEquals(msgClass, msg.msgClass());
        assertEquals(domainType, msg.domainType());
        LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
        loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        loginRequest.decode(dIter, msg);
        assertTrue(loginRequest.checkHasUserNameType());
        assertEquals(Login.UserIdTypes.TOKEN, loginRequest.userNameType());
        assertTrue(token.equals(loginRequest.userName()));
    }

    /* verify login message is a refresh */
    static void verifyLoginMessage(LoginMsg loginMsg)
    {
        assertEquals(LoginMsgType.REFRESH, loginMsg.rdmMsgType());
    }

    static void verifyLoginRttMessage(LoginMsg loginMsg) {
        assertEquals(LoginMsgType.RTT, loginMsg.rdmMsgType());
    }

    /* verify directory message is a refresh */
    static void verifyDirectoryMessage(DirectoryMsg directoryMsg)
    {
        assertEquals(DirectoryMsgType.REFRESH, directoryMsg.rdmMsgType());
    }

    /* verify dictionary message is a refresh */
    static void verifyDictionaryMessage(DictionaryMsg dictionaryMsg)
    {
        assertEquals(DictionaryMsgType.REFRESH, dictionaryMsg.rdmMsgType());
    }

    /* strip off the ripc header from the byte buffer */
    static int stripRipcHeader(ByteBuffer byteBuffer)
    {
        // look at the flags and verify that this is a normal message.
        assertEquals("stripcRipcHeader only supports Normal RIPC messages (Flag=0x02)", 2, byteBuffer.get(2));

        // get length of first message in case there are two
        int len = byteBuffer.getShort(0);

        // remove the first three bytes.
        byteBuffer.limit(byteBuffer.position());
        byteBuffer.position(3);
        byteBuffer.compact();
        byteBuffer.flip();

        return len;
    }
}
