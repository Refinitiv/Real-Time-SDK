///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015 - 2018. All rights reserved.     --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.reactor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;

import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.spi.SelectorProvider;
import java.util.HashMap;
import java.util.Set;

import org.junit.Test;

import com.thomsonreuters.upa.codec.AckMsg;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.codec.NakCodes;
import com.thomsonreuters.upa.codec.PostMsg;
import com.thomsonreuters.upa.codec.PostMsgFlags;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.rdm.Dictionary;
import com.thomsonreuters.upa.rdm.Directory;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.InstrumentNameTypes;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.test.network.replay.NetworkReplay;
import com.thomsonreuters.upa.transport.TestServer;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.MsgBaseImpl;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsg;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginStatus;

public class ReactorWatchlistJunit
{

	/** this is just a dummy class only for testing */
	public class MarketPriceRequestDummmy extends MsgBaseImpl
	{
	    /**
	     * Instantiates a new market price request.
	     */
	    public MarketPriceRequestDummmy()
	    {
	        this(DomainTypes.MARKET_PRICE);
	    }

		public MarketPriceRequestDummmy(int marketPrice) {
			// TODO Auto-generated constructor stub
		}

		@Override
		public int domainType() {
			// TODO Auto-generated method stub
			return 0;
		}

		@Override
		public int encode(EncodeIterator eIter) {
			// TODO Auto-generated method stub
			return 0;
		}

		@Override
		public int decode(DecodeIterator dIter, Msg msg) {
			// TODO Auto-generated method stub
			return 0;
		}
	}	
	
    static final int HEX_LINE_SIZE = 16;

    static int _serverPort = 17300;

    final static int CONNECT_TIMEOUT_SECONDS = 10; // 10 seconds
    final static int SELECT_TIME = 100; // 100 millisecond

    final static String LOCAL_ADDRESS = "localhost";
    final static String BASE_RIPC_TEST_DATA_DIR_NAME = "../Core/src/test/resources/com/thomsonreuters/upa/transport/RipcHandshakeJunit";
    final static String BASE_TEST_DATA_DIR_NAME = "src/test/resources/com/thomsonreuters/upa/valueadd/ReactorJunit";

    private final int KEY_EXCHANGE = 8;

    /*
     * Inner class to handle default callbacks. It simply stores the event to be
     * retrieved later.
     */
    class ReactorCallbackHandler implements DefaultMsgCallback,
            ReactorChannelEventCallback, RDMLoginMsgCallback,
            RDMDirectoryMsgCallback, RDMDictionaryMsgCallback
    {
        Selector _selector = null;
        ReactorChannelEvent _lastChannelEvent = null;
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
                            + ++_defaultMsgEventCount
                            + " event="
                            + event.toString());

            _lastDefaultMsgEvent = new ReactorMsgEvent();
            TestUtil.copyMsgEvent(event, _lastDefaultMsgEvent);

            return ReactorCallbackReturnCodes.SUCCESS;
        }

        @Override
        public int reactorChannelEventCallback(ReactorChannelEvent event)
        {
            System.out.println("DEBUG: ReactorCallbackHandler.reactorChannelEventCallback: entered. channelEventCount="
                            + ++_channelEventCount
                            + " event="
                            + event.toString());

            _lastChannelEvent = event;

            ReactorChannel rc = event.reactorChannel();
            int eventType = event.eventType();

            switch (eventType)
            {
            case ReactorChannelEventTypes.CHANNEL_UP:
                _channelUpEventCount++;
                // register this new reactorChannel for OP_READ with the selector
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
                            + ++_loginMsgEventCount
                            + " event="
                            + event.toString());

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
                            + ++_directoryMsgEventCount
                            + " event="
                            + event.toString());

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
                            + ++_dictionaryMsgEventCount
                            + " event="
                            + event.toString());

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

    /* create watchlist consumer role */
    ConsumerRole createWatchlistConsumerRole(ReactorCallbackHandler callbackHandler)
    {
         ConsumerRole consumerRole = ReactorFactory.createConsumerRole();
         assertNotNull(consumerRole);

         assertNotNull(callbackHandler);
         consumerRole.defaultMsgCallback(callbackHandler);
         consumerRole.channelEventCallback(callbackHandler);
         consumerRole.loginMsgCallback(callbackHandler);
         consumerRole.directoryMsgCallback(callbackHandler);
         consumerRole.dictionaryMsgCallback(callbackHandler);

         consumerRole.watchlistOptions().enableWatchlist(true);
         consumerRole.watchlistOptions().obeyOpenWindow(true);
         consumerRole.watchlistOptions().channelOpenCallback(callbackHandler);

         return consumerRole;
    }

    @Test
    public void watchlistConnectTest()
    {
        final String inputFile = BASE_RIPC_TEST_DATA_DIR_NAME
                    + "/10_input_connectAck_Ripc14.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            
            // attempt to submit buffer with watchlist enabled, it should fail with INVALID_USAGE
            TransportBuffer msgBuf = event.reactorChannel().channel().getBuffer(1000, false, errorInfo.error());
            assertNotNull(msgBuf);
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals("DIRECT_FEED", submitOptions.serviceName());
            assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(msgBuf, submitOptions, errorInfo));
            
            // attempt to submit market price RDM message, it should fail with INVALID_USAGE
            MarketPriceRequestDummmy marketPriceRequest = new MarketPriceRequestDummmy();
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(marketPriceRequest, submitOptions, errorInfo));
            
            // fake that channel is down and make sure submit still works
            event.reactorChannel().state(ReactorChannel.State.DOWN_RECONNECTING);

            // submit login request and make sure it succeeds
            assertNull(consumerRole.rdmLoginRequest());
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
            assertNotNull(loginRequest);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequest, submitOptions, errorInfo));

            
            // now set channel state back to UP
            event.reactorChannel().state(ReactorChannel.State.UP);
            
            // trigger channel up event and dispatch
            event.reactorChannel().watchlist().loginHandler().channelUp(errorInfo);
            ReactorJunit.dispatchReactor(selector, reactor);
            
            // make sure login request gets sent to provider
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistLoginTest()
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
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            
            // submit login request and make sure selector is triggered and dispatch succeeds
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals("DIRECT_FEED", submitOptions.serviceName());
            assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
            assertNull(consumerRole.rdmLoginRequest());
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
            assertNotNull(loginRequest);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequest, submitOptions, errorInfo));
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
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
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            assertTrue(loginMsgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(loginMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertTrue(msgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(msgEvent.streamInfo().userSpecObject().equals("Unit Test"));

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistDefaultLoginTest()
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
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistLoginRequestTimeoutTest()
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
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            consumerRole.watchlistOptions().requestTimeout(5000);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then wait for request timeout
             * to expire and for login request to be resent
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            
            // wait for login request timer to expire
            ReactorJunit.dispatchReactor(selector, reactor, 6000);

            /*
             * the reactor will send a close and then re-send out our default LoginRequest. wait for
             * testServer to read the close and LoginRequest
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verify2Messages(testServer.buffer(), MsgClasses.CLOSE, DomainTypes.LOGIN, MsgClasses.REQUEST, DomainTypes.LOGIN);

            // wait for login request timer to expire
            ReactorJunit.dispatchReactor(selector, reactor, 6000);
            
            /*
             * the reactor will re-send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verify2Messages(testServer.buffer(), MsgClasses.CLOSE, DomainTypes.LOGIN, MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(3, callbackHandler.loginMsgEventCount()); // 1 for LoginRefresh, 2 for LoginStatus messages for timeouts
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistLoginStatusTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/020_Provider_LoginStatus_LoginRefresh.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginStatus
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginStatus to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginStatus, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            verifyLoginStatus(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /* Should receive CHANNEL_DOWN event. */
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelDownEventCount());

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }


    @Test
    public void watchlistLoginStatusClosedTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/080_Provider_LoginStatusClosed.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            consumerRole.watchlistOptions().requestTimeout(5000);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginStatus Closed
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginStatus to the Reactor.
            testServer.writeMessageToSocket(replay.read());


            /*
             * call dispatch which should read the LoginStatus Closed, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);

            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            verifyLoginStatusClosed(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            ReactorJunit.dispatchReactor(selector, reactor, 6000);
            
            // make sure any timeouts do not send additional messages
            assertEquals(false, testServer.check(TestServer.State.READABLE));

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistLoginRefreshCloseRecoverTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/030_Provider_LoginRefreshCloseRecover_LoginRefresh.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginRefresh CloseRecover
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginStatus to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh CloseRecover, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /* Should receive CHANNEL_DOWN event. */
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelDownEventCount());
            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistLoginPostingTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/010_Provider_LoginRefresh_DirRefresh_MPRefresh_2MPUpdates.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
        PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
        
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // set-up post message for testing
            consumerRole.watchlistOptions().postAckTimeout(5000);
            postMsg.clear();
            postMsg.msgClass(MsgClasses.POST);
            postMsg.streamId(consumerRole.rdmLoginRequest().streamId());
            postMsg.domainType(DomainTypes.MARKET_PRICE);
            postMsg.containerType(DataTypes.NO_DATA);
            postMsg.applyAck();
            postMsg.applyPostComplete();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            Watchlist watchlist = event.reactorChannel().watchlist();
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);

            // attempt to submit post message here
            // it should fail since login stream is not yet open
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj("Unit Test");
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));
            
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());


            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);

            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // attempt to submit post message again here
            // it should fail since post message has no MsgKey
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            // submit with same post id twice, should fail on second submit
            postMsg.applyHasMsgKey();
            postMsg.msgKey().applyHasName();
            postMsg.msgKey().name().data("MsgKeyName");
            postMsg.applyHasPostId();
            postMsg.postId(1);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));
            
            // now try same post id with same sequence numbers, this should fail on second submit
            postMsg.flags(postMsg.flags() & ~PostMsgFlags.POST_COMPLETE);
            postMsg.postId(2);
            postMsg.applyHasSeqNum();
            postMsg.seqNum(1);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            // now try same post id with different sequence numbers, this should pass
            postMsg.applyHasPartNum();
            postMsg.postId(3);
            postMsg.seqNum(1);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));
            postMsg.seqNum(2);
            postMsg.applyPostComplete();
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out the PostMsg. wait for
             * testServer to read the PostMsg
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.POST, DomainTypes.MARKET_PRICE);
            
            // wait for timeout
            Thread.sleep(15000);

            // dispatch the next event
            ReactorJunit.dispatchReactor(selector, reactor);
            
            // make sure _postTimeoutInfoList and _postIdToMsgTable is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postTimeoutInfoList.size());
            assertEquals(0, watchlist.loginHandler().wlStream()._postIdToMsgTable.size()); 

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
        }
        catch (Exception e)
        {
            System.out.println("exception occurred: " + e.getLocalizedMessage());
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistLoginPostingAckNakTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/050_Provider_LoginRefresh_OffStreamPostAck.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
        PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
        
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // set-up post message for testing
            consumerRole.watchlistOptions().postAckTimeout(5000);
            postMsg.clear();
            postMsg.msgClass(MsgClasses.POST);
            postMsg.streamId(consumerRole.rdmLoginRequest().streamId());
            postMsg.domainType(DomainTypes.MARKET_PRICE);
            postMsg.containerType(DataTypes.NO_DATA);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            Watchlist watchlist = event.reactorChannel().watchlist();
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj("Unit Test");

            // submit PostMsg requiring ACK
            postMsg.applyHasMsgKey();
            postMsg.msgKey().applyHasName();
            postMsg.msgKey().name().data("MsgKeyName");
            postMsg.applyHasPostId();
            postMsg.postId(1);
            postMsg.applyHasSeqNum();
            postMsg.seqNum(1);
            postMsg.applyPostComplete();
            postMsg.applyAck();
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out the PostMsg. wait for
             * testServer to read the PostMsg
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.POST, DomainTypes.MARKET_PRICE);
            
            // wait for post ACK time to expire

            // dispatch the next events, turn off event RAISE
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
            ReactorJunit.dispatchReactor(selector, reactor, 6000);
            
            // make sure _postTimeoutInfoList is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postTimeoutInfoList.size());
            // make sure _postIdToMsgTable is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postIdToMsgTable.size());
            
            // make sure NAK was received in default callback
            assertEquals(3, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertNotNull(msgEvent.msg());
            assertEquals(MsgClasses.ACK, msgEvent.msg().msgClass());
            AckMsg ackMsg = (AckMsg)msgEvent.msg();
            assertTrue(ackMsg.checkHasNakCode());
            assertEquals(NakCodes.NO_RESPONSE, ackMsg.nakCode());

            // submit PostMsg requiring ACK again
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out the PostMsg. wait for
             * testServer to read the PostMsg
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.POST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Ack to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            // dispatch the next events
            ReactorJunit.dispatchReactor(selector, reactor);
            
            // make sure _postIdToMsgTable is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postIdToMsgTable.size());
            
            // make sure ACK was received in callback
            assertEquals(4, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertNotNull(msgEvent.msg());
            assertEquals(MsgClasses.ACK, msgEvent.msg().msgClass());
            ackMsg = (AckMsg)msgEvent.msg();
            assertFalse(ackMsg.checkHasNakCode());
            assertEquals(postMsg.streamId(), ackMsg.streamId());
            assertEquals(postMsg.domainType(), ackMsg.domainType());

            // wait for timeout
            Thread.sleep(10000);

            // dispatch the next events
            ReactorJunit.dispatchReactor(selector, reactor);

            // make sure _postTimeoutInfoList is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postTimeoutInfoList.size());

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistLoginCredentialsUpdateTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/070_Provider_LoginRefresh_LogonRefreshCredentialUpdate.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.rdmLoginRequest().applyHasInstanceId();
            consumerRole.rdmLoginRequest().instanceId().data("instanceid");
            consumerRole.rdmLoginRequest().applyHasPassword();
            consumerRole.rdmLoginRequest().password().data("password");
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
            // have the TestServer send the replay msg to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag
    
            // dispatch the first event. Dispatch should return 0
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            Watchlist watchlist = event.reactorChannel().watchlist();
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
           
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // reissue login request with user name change - this should fail
            LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
            assertNotNull(loginRequest);
            loginRequest.userName().data("NEW NAME");
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(loginRequest, submitOptions, errorInfo));
            
            // reissue login request with user token change - this should pass
            watchlist.loginHandler()._loginRequest.userNameType(Login.UserIdTypes.TOKEN);
            loginRequest.userNameType(Login.UserIdTypes.TOKEN);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequest, submitOptions, errorInfo));

            /*
             * the reactor will send out another LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessageWithToken(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN, loginRequest.userName());
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(2, callbackHandler.loginMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount());
            loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // reissue different instance id - this should fail
            LoginRequest loginRequestInstanceIdChange = (LoginRequest)LoginMsgFactory.createMsg();
            loginRequestInstanceIdChange.rdmMsgType(LoginMsgType.REQUEST);
            consumerRole.rdmLoginRequest().copy(loginRequestInstanceIdChange);
            loginRequestInstanceIdChange.instanceId().data("DifferentInstanceId");
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(loginRequestInstanceIdChange, submitOptions, errorInfo));
            
            // reissue different password - this should fail
            LoginRequest loginRequestPasswordChange = (LoginRequest)LoginMsgFactory.createMsg();
            loginRequestPasswordChange.rdmMsgType(LoginMsgType.REQUEST);
            consumerRole.rdmLoginRequest().copy(loginRequestPasswordChange);
            loginRequestPasswordChange.password().data("DifferentPassword");
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(loginRequestPasswordChange, submitOptions, errorInfo));

            // reissue login request with same instance id - this should pass
            watchlist.loginHandler()._loginRequest.instanceId().data("DifferentInstanceId");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequestInstanceIdChange, submitOptions, errorInfo));

            // reissue login request with same password - this should pass
            watchlist.loginHandler()._loginRequest.password().data("DifferentPassword");
            loginRequestPasswordChange.instanceId().data("DifferentInstanceId"); // need this since cached instance id was just changed above
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequestPasswordChange, submitOptions, errorInfo));

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    /* verifies 2 message read back to back in same byte buffer has correct MsgClass and DomainType*/
    void verify2Messages(ByteBuffer byteBuffer, int msgClass1, int domainType1, int msgClass2, int domainType2)
    {
        System.out.println("ReactorTest.verify2Messages: checking message for msgClass1="
                        + MsgClasses.toString(msgClass1)
                        + " domain1="
                        + DomainTypes.toString(domainType1)
                        + " and msgClass2="
                        + MsgClasses.toString(msgClass2)
                        + " domain2="
                        + DomainTypes.toString(domainType2)
                        + "\n"
                        + Transport.toHexString(byteBuffer, 0, byteBuffer.position()));

        int firstMsgLen = ReactorJunit.stripRipcHeader(byteBuffer);

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
        
        // verify first message
        assertEquals(msgClass1, msg.msgClass());
        assertEquals(domainType1, msg.domainType());
        
        // verify second message
        
        // jump to next message in byte buffer
        byteBuffer.position(firstMsgLen - 3);
        assertEquals("verify2Messages only supports Normal RIPC messages (Flag=0x02)", 2, byteBuffer.get(byteBuffer.position() + 2));
         byteBuffer.position(byteBuffer.position() + 3);
         byteBuffer.compact();
         byteBuffer.flip();
         
         // reset buffer to adjusted byte buffer
         buffer.clear();
         buffer.data(byteBuffer);
         dIter.clear();

         // set-up decode iterator and decode
         assertEquals(CodecReturnCodes.SUCCESS, dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion()));
         msg.clear();
         assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));
         
         // verify message
         assertEquals(msgClass2, msg.msgClass());
         assertEquals(domainType2, msg.domainType());
    }

    /* verifies 4 message read back to back in same byte buffer has correct MsgClass and DomainType */
    void verify4Messages(ByteBuffer byteBuffer, int msgClass1, int domainType1, String itemName1, int msgClass2, int domainType2, String itemName2, int msgClass3, int domainType3, String itemName3, int msgClass4, int domainType4, String itemName4)
    {
        System.out.println("ReactorTest.verify4Messages: checking message for msgClass1="
                        + MsgClasses.toString(msgClass1)
                        + " domain1="
                        + DomainTypes.toString(domainType1)
                        + " and msgClass2="
                        + MsgClasses.toString(msgClass2)
                        + " domain2="
                        + DomainTypes.toString(domainType2)
                        + " and msgClass3="
                        + MsgClasses.toString(msgClass3)
                        + " domain3="
                        + DomainTypes.toString(domainType3)
                        + " and msgClass4="
                        + MsgClasses.toString(msgClass4)
                        + " domain4="
                        + DomainTypes.toString(domainType4)
                        + "\n"
                        + Transport.toHexString(byteBuffer, 0, byteBuffer.position()));

        int totalLenReceived = byteBuffer.position();
        int lenProcessed = 0;
        int firstMsgLen = ReactorJunit.stripRipcHeader(byteBuffer);

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
        
        // verify first message
        assertEquals(msgClass1, msg.msgClass());
        assertEquals(domainType1, msg.domainType());
        if (msg.msgKey() != null && msg.msgKey().name() != null)
        {
            assertTrue(itemName1.equals(msg.msgKey().name().toString()));
        }
        
        // increment lenProcessed
        lenProcessed += firstMsgLen;
        
        // verify second message
        
        // jump to next message in byte buffer
        byteBuffer.position(firstMsgLen - 3);
        int secondMsgLen = byteBuffer.getShort();
        assertEquals("verify4Messages only supports Normal RIPC messages (Flag=0x02)", 2, byteBuffer.get());
         byteBuffer.compact();
         byteBuffer.flip();
         
         // reset buffer to adjusted byte buffer
         buffer.clear();
         buffer.data(byteBuffer);
         dIter.clear();

         // set-up decode iterator and decode
         assertEquals(CodecReturnCodes.SUCCESS, dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion()));
         msg.clear();
         assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));
         
         // verify message
         assertEquals(msgClass2, msg.msgClass());
         assertEquals(domainType2, msg.domainType());
         if (msg.msgKey() != null && msg.msgKey().name() != null)
         {
             assertTrue(itemName2.equals(msg.msgKey().name().toString()));
         }
         
         // increment lenProcessed
         lenProcessed += secondMsgLen;
         
         // verify third message
         
         // jump to next message in byte buffer
         byteBuffer.position(secondMsgLen - 3);
         int thirdMsgLen = byteBuffer.getShort();
         assertEquals("verify4Messages only supports Normal RIPC messages (Flag=0x02)", 2, byteBuffer.get());
          byteBuffer.compact();
          byteBuffer.flip();
          
          // reset buffer to adjusted byte buffer
          buffer.clear();
          buffer.data(byteBuffer);
          dIter.clear();

          // set-up decode iterator and decode
          assertEquals(CodecReturnCodes.SUCCESS, dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion()));
          msg.clear();
          assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));
          
          // verify message
          assertEquals(msgClass3, msg.msgClass());
          assertEquals(domainType3, msg.domainType());
          if (msg.msgKey() != null && msg.msgKey().name() != null)
          {
              assertTrue(itemName3.equals(msg.msgKey().name().toString()));
          }
          
          // increment lenProcessed
          lenProcessed += thirdMsgLen;
          
          // verify fourth message
          
          // jump to next message in byte buffer
          byteBuffer.position(thirdMsgLen - 3);
          int fourthMsgLen = byteBuffer.getShort();
          assertEquals("verify4Messages only supports Normal RIPC messages (Flag=0x02)", 2, byteBuffer.get());
          byteBuffer.compact();
          byteBuffer.flip();
           
          // reset buffer to adjusted byte buffer
          buffer.clear();
          buffer.data(byteBuffer);
          dIter.clear();

          // set-up decode iterator and decode
          assertEquals(CodecReturnCodes.SUCCESS, dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion()));
          msg.clear();
          assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));
           
          // verify message
          assertEquals(msgClass4, msg.msgClass());
          assertEquals(domainType4, msg.domainType());
          if (msg.msgKey() != null && msg.msgKey().name() != null)
          {
              assertTrue(itemName4.equals(msg.msgKey().name().toString()));
          }
          
          // increment lenProcessed
          lenProcessed += fourthMsgLen;
          
          // there should be no more message
          assertEquals(lenProcessed, totalLenReceived);    
    }

    
    /* verify byte buffer is a login request with pause flag set */
    void verifyMessageWithPause(ByteBuffer byteBuffer, int msgClass, int domainType)
    {
        System.out.println("ReactorTest.verifyMessageWithPause: checking message for msgClass="
                        + MsgClasses.toString(msgClass)
                        + " domain="
                        + DomainTypes.toString(domainType)
                        + "\n"
                        + Transport.toHexString(byteBuffer, 0, byteBuffer.position()));

        ReactorJunit.stripRipcHeader(byteBuffer);

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
        assertTrue(loginRequest.checkPause());
    }
    
    /* verify byte buffer is a login request with resume (pause flag not set) */
    void verifyMessageWithResume(ByteBuffer byteBuffer, int msgClass, int domainType)
    {
        System.out.println("ReactorTest.verifyMessageWithResume: checking message for msgClass="
                        + MsgClasses.toString(msgClass)
                        + " domain="
                        + DomainTypes.toString(domainType)
                        + "\n"
                        + Transport.toHexString(byteBuffer, 0, byteBuffer.position()));

        ReactorJunit.stripRipcHeader(byteBuffer);

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
        assertTrue(((RequestMsg)msg).checkStreaming());
        LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
        loginRequest.rdmMsgType(LoginMsgType.REQUEST);
        loginRequest.decode(dIter, msg);
        assertFalse(loginRequest.checkPause());
    }

    /* verify byte buffer has correct MsgClass, DomainType and priority */
    void verifyRequestWithPriority(ByteBuffer byteBuffer, int msgClass, int domainType, int priorityClass, int priorityCount)
    {
        System.out.println("ReactorTest.verifyRequestWithPriority: checking message for msgClass="
                        + MsgClasses.toString(msgClass)
                        + " domain="
                        + DomainTypes.toString(domainType)
                        + "\n"
                        + Transport.toHexString(byteBuffer, 0, byteBuffer.position()));

        ReactorJunit.stripRipcHeader(byteBuffer);

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
        assertTrue(((RequestMsg)msg).checkHasPriority());
        assertEquals(priorityClass, ((RequestMsg)msg).priority().priorityClass());
        assertEquals(priorityCount, ((RequestMsg)msg).priority().count());
    }
    
    /* verify byte buffer has correct MsgClass, DomainType and filter */
    void verifyRequestWithFilter(ByteBuffer byteBuffer, int msgClass, int domainType, long filter)
    {
        System.out.println("ReactorTest.verifyRequestWithFilter: checking message for msgClass="
                        + MsgClasses.toString(msgClass)
                        + " domain="
                        + DomainTypes.toString(domainType)
                        + "\n"
                        + Transport.toHexString(byteBuffer, 0, byteBuffer.position()));

        ReactorJunit.stripRipcHeader(byteBuffer);

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
        assertTrue(((RequestMsg)msg).msgKey().checkHasFilter());
        assertEquals(filter, ((RequestMsg)msg).msgKey().filter());
    }
    
    /* verify login message is a status */
    void verifyLoginStatus(LoginMsg loginMsg)
    {
        assertEquals(LoginMsgType.STATUS, loginMsg.rdmMsgType());
    }

    /* verify login message is a status with stream state closed */
    void verifyLoginStatusClosed(LoginMsg loginMsg)
    {
        assertEquals(LoginMsgType.STATUS, loginMsg.rdmMsgType());
        
        LoginStatus loginStatus = (LoginStatus)loginMsg;
        assertTrue(loginStatus.checkHasState());
        assertEquals(StreamStates.CLOSED, loginStatus.state().streamState());
        
    }
    
    @Test
    public void watchlistItemAggregationKeyTest()
    {
        ConsumerRole consumerRole = ReactorFactory.createConsumerRole();
        assertNotNull(consumerRole);
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        Buffer buffer = CodecFactory.createBuffer();
        buffer.data(ByteBuffer.allocate(1024));
        EncodeIterator eIter = CodecFactory.createEncodeIterator();
        eIter.clear();
        eIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        consumerRole.rdmLoginRequest().encode(eIter);
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        dIter.clear();
        dIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
        Msg msg = CodecFactory.createMsg();
        msg.decode(dIter);
        
        MsgKey msgKey = msg.msgKey();
        WlItemAggregationKey itemAggregationKey1 = ReactorFactory.createWlItemAggregationKey();
        WlItemAggregationKey itemAggregationKey2 = ReactorFactory.createWlItemAggregationKey();
        itemAggregationKey1.clear();
        itemAggregationKey2.clear();
        assertTrue(itemAggregationKey1.equals(itemAggregationKey2));
        assertEquals(itemAggregationKey1.hashCode(), itemAggregationKey2.hashCode());
        msgKey.copy(itemAggregationKey1.msgKey());
        msgKey.copy(itemAggregationKey2.msgKey());
        assertTrue(itemAggregationKey1.msgKey().equals(itemAggregationKey2.msgKey()));
        assertEquals(itemAggregationKey1.msgKey().hashCode(), itemAggregationKey2.msgKey().hashCode());
        assertTrue(itemAggregationKey1.equals(itemAggregationKey2));
        assertEquals(itemAggregationKey1.hashCode(), itemAggregationKey2.hashCode());
        itemAggregationKey1.domainType(DomainTypes.MARKET_PRICE);
        assertFalse(itemAggregationKey1.equals(itemAggregationKey2));
        if (itemAggregationKey1.hashCode() == itemAggregationKey2.hashCode())
        {
            assertTrue(false);
        }
        itemAggregationKey2.domainType(DomainTypes.MARKET_PRICE);
        assertTrue(itemAggregationKey1.equals(itemAggregationKey2));
        assertEquals(itemAggregationKey1.hashCode(), itemAggregationKey2.hashCode());
        itemAggregationKey1.qos().timeliness(QosTimeliness.DELAYED);
        assertFalse(itemAggregationKey1.equals(itemAggregationKey2));
        if (itemAggregationKey1.hashCode() == itemAggregationKey2.hashCode())
        {
            assertTrue(false);
        }
        itemAggregationKey1.qos().rate(QosRates.JIT_CONFLATED);
        assertFalse(itemAggregationKey1.equals(itemAggregationKey2));
        if (itemAggregationKey1.hashCode() == itemAggregationKey2.hashCode())
        {
            assertTrue(false);
        }
        HashMap<WlItemAggregationKey,String> testMap = new HashMap<WlItemAggregationKey,String>();
        testMap.put(itemAggregationKey1, "Test1");
        assertTrue(testMap.containsKey(itemAggregationKey1)); // key 1 lookup should pass
        assertFalse(testMap.containsKey(itemAggregationKey2)); // key 2 lookup should fail since it's not the same as key 1
        itemAggregationKey2.qos().timeliness(QosTimeliness.DELAYED); // change key 2 info to be same as key 1
        itemAggregationKey2.qos().rate(QosRates.JIT_CONFLATED);
        assertTrue(itemAggregationKey1.equals(itemAggregationKey2));
        assertEquals(itemAggregationKey1.hashCode(), itemAggregationKey2.hashCode());
        assertTrue(testMap.containsKey(itemAggregationKey2)); // key 2 lookup should pass since it's now the same as key 1
        testMap.remove(itemAggregationKey2); // remove this entry from table
        assertFalse(testMap.containsKey(itemAggregationKey1)); // key removed so lookup now fails
        assertFalse(testMap.containsKey(itemAggregationKey2)); // key removed so lookup now fails
        itemAggregationKey1.qos().timeInfo(10); // change key 1 info
        itemAggregationKey1.qos().rateInfo(20);
        testMap.put(itemAggregationKey1, "Test1"); // put new key in with key change
        assertTrue(testMap.containsKey(itemAggregationKey1)); // now key 1 lookup should pass
        assertFalse(testMap.containsKey(itemAggregationKey2)); // key 2 lookup fails since it hasn't been changed
        itemAggregationKey2.qos().timeInfo(10); // change key 2 info to be same as key 1
        itemAggregationKey2.qos().rateInfo(20);
        assertTrue(testMap.containsKey(itemAggregationKey2)); // key 2 lookup should pass since it's now the same as key 1
        assertTrue(testMap.get(itemAggregationKey2).equals("Test1")); // key 2 should find value associated with key 1
    }
    
    @Test
    public void watchlistServiceCacheAddUpdateDeleteTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/090_Provider_ServiceCacheAdd.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            Watchlist watchlist = event.reactorChannel().watchlist();
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // check service cache for proper entries
            // the following serviceIds/serviceNames should have been added
            // 460/IDN_RDF
            // 7191/ELEKTRON_DD
            // 7001/QPROV1
            // 7002/QPROV2
            // 37397/NI_PUB
            
            // verify Service can be retrieved by both service id and name 
            assertNotNull(watchlist.directoryHandler().service(460));
            assertEquals(watchlist.directoryHandler().service(460).rdmService().action(), MapEntryActions.ADD);
            assertNotNull(watchlist.directoryHandler().service("IDN_RDF"));
            assertEquals(watchlist.directoryHandler().service("IDN_RDF").rdmService().action(), MapEntryActions.ADD);
            assertNotNull(watchlist.directoryHandler().service(7191));
            assertEquals(watchlist.directoryHandler().service(7191).rdmService().action(), MapEntryActions.ADD);
            assertNotNull(watchlist.directoryHandler().service("ELEKTRON_DD"));
            assertEquals(watchlist.directoryHandler().service("ELEKTRON_DD").rdmService().action(), MapEntryActions.ADD);
            assertNotNull(watchlist.directoryHandler().service(7001));
            assertEquals(watchlist.directoryHandler().service(7001).rdmService().action(), MapEntryActions.ADD);
            assertNotNull(watchlist.directoryHandler().service("QPROV1"));
            assertEquals(watchlist.directoryHandler().service("QPROV1").rdmService().action(), MapEntryActions.ADD);
            assertNotNull(watchlist.directoryHandler().service(7002));
            assertEquals(watchlist.directoryHandler().service(7002).rdmService().action(), MapEntryActions.ADD);
            assertNotNull(watchlist.directoryHandler().service("QPROV2"));
            assertEquals(watchlist.directoryHandler().service("QPROV2").rdmService().action(), MapEntryActions.ADD);
            assertNotNull(watchlist.directoryHandler().service(37397));
            assertEquals(watchlist.directoryHandler().service(37397).rdmService().action(), MapEntryActions.ADD);
            assertNotNull(watchlist.directoryHandler().service("NI_PUB"));
            assertEquals(watchlist.directoryHandler().service("NI_PUB").rdmService().action(), MapEntryActions.ADD);
            
            // verify service name can be retrieved by service id and vice-versa
            assertTrue(watchlist.directoryHandler().serviceName(460).equals("IDN_RDF"));
            assertEquals(watchlist.directoryHandler().serviceId("IDN_RDF"), 460);
            assertTrue(watchlist.directoryHandler().serviceName(7191).equals("ELEKTRON_DD"));
            assertEquals(watchlist.directoryHandler().serviceId("ELEKTRON_DD"), 7191);
            assertTrue(watchlist.directoryHandler().serviceName(7001).equals("QPROV1"));
            assertEquals(watchlist.directoryHandler().serviceId("QPROV1"), 7001);
            assertTrue(watchlist.directoryHandler().serviceName(7002).equals("QPROV2"));
            assertEquals(watchlist.directoryHandler().serviceId("QPROV2"), 7002);
            assertTrue(watchlist.directoryHandler().serviceName(37397).equals("NI_PUB"));
            assertEquals(watchlist.directoryHandler().serviceId("NI_PUB"), 37397);
            
            // now have the TestServer send unsolicited DirectoryRefresh with DELETE of "37397/NI_PUB" to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the unsolicited DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(2, callbackHandler.directoryMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount());
            directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // check that "37397/NI_PUB" has been deleted from the service cache
            assertNull(watchlist.directoryHandler().service(37397));
            assertNull(watchlist.directoryHandler().service("NI_PUB"));
            assertNull(watchlist.directoryHandler().serviceName(37397));
            assertEquals(watchlist.directoryHandler().serviceId("NI_PUB"), ReactorReturnCodes.PARAMETER_INVALID);

            // now have the TestServer send unsolicited DirectoryRefresh with UPDATE of "460/IDN_RDF" to the Reactor.
            // copy the service before the update
            Service serviceByIdBeforeUpdate = DirectoryMsgFactory.createService();
            watchlist.directoryHandler().service(460).rdmService().copy(serviceByIdBeforeUpdate);
            Service serviceByNameBeforeUpdate = DirectoryMsgFactory.createService();
            watchlist.directoryHandler().service("IDN_RDF").rdmService().copy(serviceByNameBeforeUpdate);
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the unsolicited DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(3, callbackHandler.directoryMsgEventCount());
            assertEquals(4, callbackHandler.defaultMsgEventCount());
            directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // check that "460/IDN_RDF" has been updated
            // updates include ServiceState, OpenLimit and Capabilities
            Service serviceByIdAfterUpdate = watchlist.directoryHandler().service(460).rdmService();
            assertNotNull(serviceByIdAfterUpdate);
            Service serviceByNameAfterUpdate = watchlist.directoryHandler().service("IDN_RDF").rdmService();
            assertNotNull(serviceByNameAfterUpdate);
            assertEquals(serviceByIdAfterUpdate, serviceByNameAfterUpdate);
            assertEquals(serviceByIdAfterUpdate.action(), MapEntryActions.UPDATE);
            assertFalse(serviceByIdAfterUpdate.state().acceptingRequests() == serviceByIdBeforeUpdate.state().acceptingRequests());
            assertFalse(serviceByIdAfterUpdate.state().serviceState() == serviceByIdBeforeUpdate.state().serviceState());
            assertFalse(serviceByIdAfterUpdate.load().openLimit() == serviceByIdBeforeUpdate.load().openLimit());
            assertFalse(serviceByIdAfterUpdate.info().capabilitiesList().size() == serviceByIdBeforeUpdate.info().capabilitiesList().size());
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }
    
    @Test
    public void watchlistOpenCallbackItemSubmitTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/100_Provider_WatchlistItemSubmit.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            consumerRole.watchlistOptions().requestTimeout(5000);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            
            // submit some items before login and directory stream is open
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyMsgKeyInUpdates();
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            requestMsg.streamId(6);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            requestMsg.streamId(7);
            requestMsg.msgKey().name().data("IBM");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            
            // attempt to submit another item with NO_REFRESH flag set
            // this should fail
            requestMsg.streamId(8);
            requestMsg.msgKey().name().data("WFM");
            requestMsg.applyNoRefresh();
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));            
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(4, callbackHandler.defaultMsgEventCount()); // 1 for login plus 3 for submitted item requests
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(5, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory plus 3 for submitted item requests
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the watchlist will send out 2 item requests (TRI and IBM) even though 2 request were made for TRI.
             * wait for testServer to read the item requests
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);            

            // dispatch all timeout events
            // dispatch the next event
            ReactorJunit.dispatchReactor(selector, reactor, 6000);
            
            /*
             * the reactor will re-send out our item requests. wait for
             * testServer to read the item requests, then send the Item Refresh
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verify4Messages(testServer.buffer(), MsgClasses.CLOSE, DomainTypes.MARKET_PRICE, "TRI",
                                                 MsgClasses.CLOSE, DomainTypes.MARKET_PRICE, "IBM",
                                                 MsgClasses.REQUEST, DomainTypes.MARKET_PRICE, "TRI",
                                                 MsgClasses.REQUEST, DomainTypes.MARKET_PRICE, "IBM");
            
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*/
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(10, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 3 for submitted item requests, 3 for timeout status messages
                                                                     // plus 2 for Item Refresh (2 for Item Refresh due to fanout for 2 TRIs requested)
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // now have the TestServer send another TRI Item Refresh and two TRI updates to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            testServer.writeMessageToSocket(replay.read());
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(14, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 3 for submitted item requests, 3 for timeout status messages
                                                                      // plus 2 for Item Refresh (2 for Item Refresh due to fanout for 2 TRIs requested)
                                                                      // 4 for the 2 TRI item updates (second TRI refresh is ignored)
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // verify that update has msgKey in it
            UpdateMsg updateMsg = (UpdateMsg)msgEvent.msg();
            assertTrue(updateMsg.checkHasMsgKey());
            
            // close login stream twice and make sure no errors
            CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
            closeMsg.msgClass(MsgClasses.CLOSE);
            closeMsg.streamId(1);
            closeMsg.domainType(DomainTypes.LOGIN);
            assertEquals(ReactorReturnCodes.SUCCESS, msgEvent.reactorChannel().submit(closeMsg, submitOptions, errorInfo));
            assertEquals(ReactorReturnCodes.SUCCESS, msgEvent.reactorChannel().submit(closeMsg, submitOptions, errorInfo));            
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }
    
    @Test
    public void watchlistItemReissueTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/120_Provider_WatchlistItemReissue.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            
            // submit some items before login and directory stream is open
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            requestMsg.streamId(6);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            requestMsg.streamId(7);
            requestMsg.msgKey().name().data("IBM");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());


            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(4, callbackHandler.defaultMsgEventCount()); // 1 for login plus 3 for submitted item requests
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(5, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory plus 3 for submitted item requests
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the watchlist will send out 2 item requests (TRI and IBM) even though 2 request were made for TRI.
             * wait for testServer to read the item requests
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(7, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 3 for submitted item requests
                                                                     // 2 for Item Refresh due to fanout for 2 TRIs requested
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // now have the TestServer send a TRI update to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the Item Update, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(9, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 3 for submitted item requests
                                                                     // 2 for Item Refresh due to fanout for 2 TRIs requested
                                                                     // 2 for the TRI item update
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // now reissue second TRI request with STREAMING flag turning off
            // this should fail since reissue is not allowed with a STREAMING flag change
            requestMsg.flags(requestMsg.flags() & ~RequestMsgFlags.STREAMING);
            requestMsg.msgKey().name().data("TRI");
            requestMsg.streamId(6);
            assertEquals(ReactorReturnCodes.FAILURE, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            // now reissue second TRI request with STREAMING flag back on
            // this should be successful
            requestMsg.applyStreaming();
            requestMsg.msgKey().name().data("TRI");
            requestMsg.streamId(6);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            // now have the TestServer send a TRI update to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should only read one Item Update since the second
             * is in the middle of reissue,
             * invoke callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(11, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 3 for submitted item requests
                                                                      // 2 for Item Refresh due to fanout for 2 TRIs requested
                                                                      // 2 for first TRI item updates
                                                                      // 2 for second TRI item update, still two updates as the reissue does not change anything on the item, hence no refresh is pending            
            // now have the TestServer send a TRI refresh for the reissue to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh for the reissue,
             * invoke callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            
            assertEquals(11, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 3 for submitted item requests
                                                                      // 2 for Item Refresh due to fanout for 2 TRIs requested
                                                                      // 2 for first TRI item updates
                                                                      // 2 for second TRI item update
                                                                      // refresh was not callback to the client since that re-issue is an no-op
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // now have the TestServer send a TRI update to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh for the reissue,
             * invoke callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(13, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 3 for submitted item requests
                                                                      // 2 for Item Refresh due to fanout for 2 TRIs requested
                                                                      // 2 for first TRI item updates
                                                                      // 2 for second TRI item update
                                                                      // 2 for last TRI item update
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistItemSubmitSnapshotTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/105_Provider_WatchlistItemSubmitSnapshot.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            
            // submit two of the same items before login and directory stream is open
            // submit first TRI as snapshot and second as streaming
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj("TEST_userSpecObj");
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            requestMsg.applyStreaming();
            requestMsg.streamId(6);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount()); // 1 for login plus 2 for submitted item requests
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(4, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory plus 2 for submitted item requests
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the watchlist will send out 1 TRI item request as a snapshot request.
             * wait for testServer to read the item requests
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(6, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 2 for submitted item requests plus 2 for Item Refreshes
                                                                     // 1 refresh for snapshot, one for streaming
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertEquals(6, msgEvent.msg().streamId());
            assertTrue(msgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(msgEvent.streamInfo().userSpecObject().equals("TEST_userSpecObj"));

            // now have the TestServer send two TRI updates to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(8, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 2 for submitted item requests plus 2 for Item Refresh
                                                                     // 1 for Item Refresh while waiting for snapshot response to be received
                                                                     // 1 for Item Refresh after waiting for snapshot response to be received
                                                                     // 2 for the 2 TRI item updates
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // now try to re-submit snapshot request as streaming, this should be successful
            requestMsg.streamId(5);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistItemSubmitMultiPartTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/110_Provider_WatchlistItemSubmitLevel2.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertEquals(1, callbackHandler.channelReadyEventCount());
            event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            
            // submit a market by price request
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_BY_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasServiceId();
            requestMsg.msgKey().serviceId(1);
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /*
             * the watchlist will send out the market by price item request.
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_BY_PRICE);
            // have the TestServer send the first part of the multi-part Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the first part of multi-part Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(3, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 1 for first part of multi-part refresh
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // send out the second market by price request
            requestMsg.streamId(6);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            
            /*
             * the watchlist will wait to send out the second market by price item request
             * since the first market by price item request is in the middle of a multi-part refresh
             * so wait should time out here
             */
            try
            {
                testServer.waitWithException(TestServer.State.READABLE);
                assertTrue(false);
            }
            catch(Exception e) {}
            
            // send remainder of multi-part refresh
            // have the TestServer send the remainder of the multi-part Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(5, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory,
                                                                     // 1 for first part of multi-part refresh,
                                                                     // 2 for remainder of multi-part refresh
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            /*
             * the watchlist will send out the second market by price item request since the remainder of multi-part refresh has been received.
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_BY_PRICE);

            // now have the TestServer send the second set of multi-part market by price refreshes to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(8, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory,
                                                                     // 2 for first part of multi-part refreshes,
                                                                     // 4 for remainder of multi-part refreshes
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // now have the TestServer send two market by price updates to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(12, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory,
                                                                     // 2 for first part of multi-part refreshes,
                                                                     // 4 for remainder of multi-part refreshes
                                                                     // 4 for updates
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistDictionarySubmitTest()
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
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());


            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // now submit field dictionary request to the watchlist
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            String myUserSpecObject1 = "myUserSpecObject1";
            submitOptions.requestMsgOptions().userSpecObj(myUserSpecObject1);
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.DICTIONARY);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("RWFFld");
            requestMsg.msgKey().applyHasFilter();
            requestMsg.msgKey().filter(Dictionary.VerbosityValues.MINIMAL);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /*
             * the watchlist will send out dictionary request
             * wait for testServer to read the dictionary request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyRequestWithFilter(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.DICTIONARY, Dictionary.VerbosityValues.MINIMAL);
            // have the TestServer send the Dictionary Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Dictionary Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDictionaryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.dictionaryMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            // verify userSpecObject
            RDMDictionaryMsgEvent dictionaryMsgEvent = callbackHandler.lastDictionaryMsgEvent();
            assertEquals(dictionaryMsgEvent.streamInfo().userSpecObject(), myUserSpecObject1);
            ReactorJunit.verifyDictionaryMessage(dictionaryMsgEvent.rdmDictionaryMsg());
            
            // now submit enum type dictionary request to the watchlist
            String myUserSpecObject2 = "myUserSpecObject2";
            submitOptions.requestMsgOptions().userSpecObj(myUserSpecObject2);
            requestMsg.streamId(7);
            requestMsg.msgKey().name().data("RWFEnum");
            requestMsg.msgKey().filter(Dictionary.VerbosityValues.VERBOSE);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /*
             * the watchlist will send out dictionary request
             * wait for testServer to read the dictionary request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyRequestWithFilter(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.DICTIONARY, Dictionary.VerbosityValues.VERBOSE);
            // have the TestServer send the Dictionary Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Dictionary Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDictionaryMsgCallback and defaultMsgCallback was called.
            assertEquals(2, callbackHandler.dictionaryMsgEventCount());
            assertEquals(4, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            // verify userSpecObject
            dictionaryMsgEvent = callbackHandler.lastDictionaryMsgEvent();
            assertEquals(dictionaryMsgEvent.streamInfo().userSpecObject(), myUserSpecObject2);
            ReactorJunit.verifyDictionaryMessage(dictionaryMsgEvent.rdmDictionaryMsg());
            
            // change service name of field dictionary request and attempt to reissue
            // this should fail
            submitOptions.serviceName("123456");
            requestMsg.streamId(5);
            requestMsg.msgKey().name().data("RWFFld");
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            
            // change filter of field dictionary request and attempt to reissue
            submitOptions.serviceName("DIRECT_FEED");
            requestMsg.msgKey().filter(Dictionary.VerbosityValues.NORMAL);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /*
             * the watchlist will send out dictionary request
             * wait for testServer to read the dictionary request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyRequestWithFilter(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.DICTIONARY, Dictionary.VerbosityValues.NORMAL);

            // submit dictionary request with invalid name to the watchlist
            // this should pass since provider may not advertise all names in service's dictionary list
            requestMsg.streamId(8);
            requestMsg.msgKey().name().data("XYZ");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistItemPriorityTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/130_Provider_WatchlistItemPriority.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            
            // submit an item before login and directory stream is open
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            requestMsg.applyHasPriority();
            requestMsg.priority().priorityClass(5);
            requestMsg.priority().count(2);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount()); // 1 for login plus 1 for submitted item request
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory plus 1 for submitted item request
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the watchlist will send out an item request for TRI.
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyRequestWithPriority(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE, 5, 2);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(4, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 1 for submitted item request
                                                                     // 1 for Item Refresh
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // now send a second TRI request with a different stream id and same priority
            requestMsg.streamId(6);
            requestMsg.priority().priorityClass(5);
            requestMsg.priority().count(2);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /*
             * the watchlist will send out an item request for TRI.
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyRequestWithPriority(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE, 5, 4);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(5, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 1 for submitted item request
                                                                     // 2 for Item Refreshes
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // now send a third TRI request with a different stream id and a higher priority class
            requestMsg.streamId(7);
            requestMsg.priority().priorityClass(6);
            requestMsg.priority().count(2);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /*
             * the watchlist will send out an item request for TRI.
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyRequestWithPriority(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE, 6, 6);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(6, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 1 for submitted item request
                                                                     // 3 for Item Refreshes
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // close one of the streams (this should alter the priority count of subsequent requests)
            CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
            closeMsg.msgClass(MsgClasses.CLOSE);
            closeMsg.streamId(6);
            closeMsg.domainType(DomainTypes.MARKET_PRICE);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(closeMsg, submitOptions, errorInfo));

            /*
             * the watchlist will send out an item reissue for TRI since item was closed.
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyRequestWithPriority(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE, 6, 4); // 14 = 2 + 2 + 2 - 2 (-2 is for close)
            
            // now send a fourth TRI request with a different stream id and a higher priority count
            requestMsg.streamId(8);
            requestMsg.priority().priorityClass(6);
            requestMsg.priority().count(10);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            
            /* Dispatch reactor in case we need to restart internal flushing. */
            ReactorJunit.dispatchReactor(selector, reactor);
            

			ReactorJunit.dispatchReactor(selector, reactor); /* Read FLUSH_DONE event internally. */

            /*
             * the watchlist will send out an item request for TRI.
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            verifyRequestWithPriority(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE, 6, 14); // 14 = 2 + 2 + 2 - 2 + 10 (-2 is for close)
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistItemQosRangeTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/140_Provider_WatchlistItemQosRange.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            
            // submit an item before login and directory stream is open
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            requestMsg.applyHasQos();
            requestMsg.qos().timeliness(QosTimeliness.DELAYED);
            requestMsg.qos().rate(QosRates.TICK_BY_TICK);
            requestMsg.applyHasWorstQos();
            requestMsg.worstQos().timeliness(QosTimeliness.DELAYED_UNKNOWN);
            requestMsg.worstQos().rate(QosRates.JIT_CONFLATED);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
    
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount()); // 1 for login plus 1 for submitted item request
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory plus 1 for submitted item request
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the watchlist will send out an item request for TRI.
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(4, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 1 for submitted item request
                                                                     // 1 for Item Refresh
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // now send a second TRI request with a different stream id and different qos range that's within range
            requestMsg.streamId(6);
            requestMsg.qos().timeliness(QosTimeliness.DELAYED);
            requestMsg.qos().rate(QosRates.JIT_CONFLATED);
            requestMsg.worstQos().timeliness(QosTimeliness.DELAYED_UNKNOWN);
            requestMsg.worstQos().rate(QosRates.JIT_CONFLATED);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /*
             * the watchlist will send out an item request for TRI.
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(5, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 1 for submitted item request
                                                                     // 2 for Item Refreshes
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // now send a third TRI request with a different stream id and different qos range that's out of range
            requestMsg.streamId(7);
            requestMsg.qos().timeliness(QosTimeliness.REALTIME);
            requestMsg.qos().rate(QosRates.TICK_BY_TICK);
            requestMsg.worstQos().timeliness(QosTimeliness.REALTIME);
            requestMsg.worstQos().rate(QosRates.JIT_CONFLATED);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /* the watchlist won't send out the third TRI request since the qos range is out of range */
            try
            {
                testServer.waitWithException(TestServer.State.READABLE);
                assertTrue(false);
            }
            catch(Exception e) {}
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }
    
    @Test
    public void watchlistItemOpenWindowTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/150_Provider_WatchlistItemOpenWindow.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
            // have the TestServer send the replay msg to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            // read the extra RIPC 14 handshake message
            assertTrue(testServer.readMessageFromSocket() > 0);
            assertEquals(KEY_EXCHANGE, testServer.buffer().get(2)); // verify
                                                                    // KEY_EXCHANGE
                                                                    // flag
    
            // dispatch the first event. Dispatch should return 0
            ReactorJunit.dispatchReactor(selector, reactor);

            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // now submit 3 item requests to the watchlist
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasServiceId();
            requestMsg.msgKey().serviceId(1);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            submitOptions.clear();
            submitOptions.requestMsgOptions().userSpecObj("REQUEST TRI");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            requestMsg.streamId(6);
            requestMsg.msgKey().name().data("IBM");
            requestMsg.flags(requestMsg.flags() & ~RequestMsgFlags.STREAMING);
            submitOptions.clear();
            submitOptions.requestMsgOptions().userSpecObj("REQUEST IBM");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            requestMsg.streamId(7);
            requestMsg.msgKey().name().data("WFM");
            requestMsg.applyStreaming();
            submitOptions.clear();
            submitOptions.requestMsgOptions().userSpecObj("REQUEST WFM");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            
            // change service id of third item request and attempt to reissue
            // this should fail
            requestMsg.msgKey().serviceId(2);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /*
             * the watchlist will send out one item request since the open window is 1
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * Call dispatch which should read the Item Refresh, invoke
             * callback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(3, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertTrue(msgEvent.streamInfo().userSpecObject().equals("REQUEST TRI"));

            /*
             * the watchlist will send out the second item request since the open window is 1
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(4, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertTrue(msgEvent.streamInfo().userSpecObject().equals("REQUEST IBM"));
            
            /*
             * the watchlist will send out the third item request since the open window is 1
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(5, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertTrue(msgEvent.streamInfo().userSpecObject().equals("REQUEST WFM"));
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }
    
    @Test
    public void watchlistItemPostingTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/150_Provider_WatchlistItemOpenWindow.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
        PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
        
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            // dispatch the first event. Dispatch should return 0
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the ReactorChannelEventCallback was called.
            assertEquals(2, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            Watchlist watchlist = event.reactorChannel().watchlist();
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // submit an item request
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasServiceId();
            requestMsg.msgKey().serviceId(1);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            
            /*
             * the watchlist will send out one item request since the open window is 1
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            
            // submit post message before item stream is open, this should fail
            consumerRole.watchlistOptions().postAckTimeout(5000);
            postMsg.clear();
            postMsg.msgClass(MsgClasses.POST);
            postMsg.streamId(5);
            postMsg.domainType(DomainTypes.MARKET_PRICE);
            postMsg.containerType(DataTypes.NO_DATA);
            postMsg.applyHasPostId();
            postMsg.postId(1);
            postMsg.applyAck();
            postMsg.applyPostComplete();
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * Call dispatch which should read the Item Refresh, invoke
             * callback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(3, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // now that item stream is open, submit with same post id twice, should fail on second submit
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            // now try same post id with same sequence numbers, this should fail on second submit
            postMsg.flags(postMsg.flags() & ~PostMsgFlags.POST_COMPLETE);
            postMsg.postId(2);
            postMsg.applyHasSeqNum();
            postMsg.seqNum(1);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            // now try same post id with different sequence numbers, this should pass
            postMsg.applyHasPartNum();
            postMsg.postId(3);
            postMsg.seqNum(1);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));
            postMsg.seqNum(2);
            postMsg.applyPostComplete();
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out the PostMsg. wait for
             * testServer to read the PostMsg
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.POST, DomainTypes.MARKET_PRICE);
            
            // wait for timeout
            Thread.sleep(15000);

            // dispatch the next event
            ReactorJunit.dispatchReactor(selector, reactor);
            
            // make sure _postTimeoutInfoList and _postIdToMsgTable is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postTimeoutInfoList.size());
            assertEquals(0, watchlist.loginHandler().wlStream()._postIdToMsgTable.size());
            
            // now set maxOutstandingPosts to 0, post submit should fail
            watchlist.watchlistOptions().maxOutstandingPosts(0);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
        }
        catch (Exception e)
        {
            System.out.println("exception occurred: " + e.getLocalizedMessage());
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistItemPostingAckNakMultiPartTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/160_Provider_ItemRefresh_OnStreamPostAck.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
        PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
        
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            Watchlist watchlist = event.reactorChannel().watchlist();
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // submit an item request
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasServiceId();
            requestMsg.msgKey().serviceId(1);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            
            /*
             * the watchlist will send out one item request.
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * Call dispatch which should read the Item Refresh, invoke
             * callback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(3, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // submit PostMsg requiring ACK
            consumerRole.watchlistOptions().postAckTimeout(5000);
            postMsg.clear();
            postMsg.msgClass(MsgClasses.POST);
            postMsg.streamId(5);
            postMsg.domainType(DomainTypes.MARKET_PRICE);
            postMsg.containerType(DataTypes.NO_DATA);
            postMsg.applyHasPostId();
            postMsg.postId(1);
            postMsg.applyHasSeqNum();
            postMsg.seqNum(1);
            postMsg.applyPostComplete();
            postMsg.applyAck();
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out the PostMsg. wait for
             * testServer to read the PostMsg
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.POST, DomainTypes.MARKET_PRICE);
            
            // wait for post ACK time to expire
            ReactorJunit.dispatchReactor(selector, reactor, 16000);
            
            // make sure _postTimeoutInfoList is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postTimeoutInfoList.size());
            // make sure _postIdToMsgTable is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postIdToMsgTable.size());
            
            // make sure NAK was received in callback
            assertEquals(4, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertNotNull(msgEvent.msg());
            assertEquals(MsgClasses.ACK, msgEvent.msg().msgClass());
            AckMsg ackMsg = (AckMsg)msgEvent.msg();
            assertTrue(ackMsg.checkHasNakCode());
            assertEquals(NakCodes.NO_RESPONSE, ackMsg.nakCode());

            // submit PostMsg requiring ACK again
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out the PostMsg. wait for
             * testServer to read the PostMsg
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.POST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Ack to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            // dispatch the next events
            ReactorJunit.dispatchReactor(selector, reactor);
            
            // make sure _postIdToMsgTable is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postIdToMsgTable.size());
            
            // make sure ACK was received in callback
            assertEquals(5, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertNotNull(msgEvent.msg());
            assertEquals(MsgClasses.ACK, msgEvent.msg().msgClass());
            ackMsg = (AckMsg)msgEvent.msg();
            assertFalse(ackMsg.checkHasNakCode());
            assertEquals(postMsg.streamId(), ackMsg.streamId());
            assertEquals(postMsg.domainType(), ackMsg.domainType());

            // submit PostMsg requiring ACK again with second part of multi-part refresh
            postMsg.seqNum(2);
            postMsg.applyPostComplete();
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out the PostMsg. wait for
             * testServer to read the PostMsg
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.POST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Ack to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            // dispatch the next events
            ReactorJunit.dispatchReactor(selector, reactor);

            // make sure _postTimeoutInfoList is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postTimeoutInfoList.size());

            // make sure ACK was received in callback
            assertEquals(6, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertNotNull(msgEvent.msg());
            assertEquals(MsgClasses.ACK, msgEvent.msg().msgClass());
            ackMsg = (AckMsg)msgEvent.msg();
            assertFalse(ackMsg.checkHasNakCode());
            assertEquals(postMsg.streamId(), ackMsg.streamId());
            assertEquals(postMsg.domainType(), ackMsg.domainType());

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistItemPostingAckNakSinglePartTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/161_Provider_ItemRefresh_OnStreamPostAckSinglePart.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
        PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
        
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
            Watchlist watchlist = event.reactorChannel().watchlist();
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // submit an item request
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasServiceId();
            requestMsg.msgKey().serviceId(1);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            
            /*
             * the watchlist will send out one item request.
             * wait for testServer to read the item request
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Item Refresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * Call dispatch which should read the Item Refresh, invoke
             * callback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(3, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // submit PostMsg requiring ACK
            consumerRole.watchlistOptions().postAckTimeout(5000);
            postMsg.clear();
            postMsg.msgClass(MsgClasses.POST);
            postMsg.streamId(5);
            postMsg.domainType(DomainTypes.MARKET_PRICE);
            postMsg.containerType(DataTypes.NO_DATA);
            postMsg.applyHasPostId();
            postMsg.postId(1);
            postMsg.applyPostComplete();
            postMsg.applyAck();
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out the PostMsg. wait for
             * testServer to read the PostMsg
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.POST, DomainTypes.MARKET_PRICE);
            
            // wait for post ACK time to expire
            Thread.sleep(15000);

            // dispatch the next events
            ReactorJunit.dispatchReactor(selector, reactor);
            
            // make sure _postTimeoutInfoList is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postTimeoutInfoList.size());
            // make sure _postIdToMsgTable is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postIdToMsgTable.size());
            
            // make sure NAK was received in callback
            assertEquals(4, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertNotNull(msgEvent.msg());
            assertEquals(MsgClasses.ACK, msgEvent.msg().msgClass());
            AckMsg ackMsg = (AckMsg)msgEvent.msg();
            assertTrue(ackMsg.checkHasNakCode());
            assertEquals(NakCodes.NO_RESPONSE, ackMsg.nakCode());

            // submit PostMsg requiring ACK again
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(postMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out the PostMsg. wait for
             * testServer to read the PostMsg
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.POST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the Ack to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            // dispatch the next events
            ReactorJunit.dispatchReactor(selector, reactor);
            
            // make sure _postIdToMsgTable is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postIdToMsgTable.size());
            
            // make sure ACK was received in callback
            assertEquals(5, callbackHandler.defaultMsgEventCount());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertNotNull(msgEvent.msg());
            assertEquals(MsgClasses.ACK, msgEvent.msg().msgClass());
            ackMsg = (AckMsg)msgEvent.msg();
            assertFalse(ackMsg.checkHasNakCode());
            assertEquals(postMsg.streamId(), ackMsg.streamId());
            assertEquals(postMsg.domainType(), ackMsg.domainType());

            // wait for timeout
            Thread.sleep(10000);

            // dispatch the next events
            ReactorJunit.dispatchReactor(selector, reactor);

            // make sure _postTimeoutInfoList is empty
            assertEquals(0, watchlist.loginHandler().wlStream()._postTimeoutInfoList.size());

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistItemRecoveryDueToLoginStreamCloseRecoverTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/160_Provider_ItemRecoveryDueToLoginStream.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            consumerRole.watchlistOptions().requestTimeout(5000);

            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
                        
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
    
            // submit an item before login and directory stream is open
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyMsgKeyInUpdates();
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount()); 
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
 
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount());  // extra for itemRequest
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount()); // watchlist callback
            event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            
            /*
             * testServer then read item requests and then send the Item Response
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the ItemRefresh and ItemUpdate to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(5, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 1 for submitted item requests
                                                                     // 1 for Item Refresh due to fanout 
                                                                     // 1 for the TRI item update fanout
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // verify that update has msgKey in it
            UpdateMsg updateMsg = (UpdateMsg)msgEvent.msg();
            assertTrue(updateMsg.checkHasMsgKey());
 
            // have the TestServer send the LoginStatus to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginStatus Closed, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);

            /* Should receive CHANNEL_DOWN event. */
            assertEquals(4, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelDownEventCount());
            ReactorChannelEvent event1 = callbackHandler.lastChannelEvent();
            assertNotNull(event1);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event1.eventType());
            
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(2, callbackHandler.loginMsgEventCount());
            assertEquals(2, callbackHandler.directoryMsgEventCount());
            assertEquals(8, callbackHandler.defaultMsgEventCount());   // one more loginStatus, one directory update, one more itemStatus callback 
            RDMLoginMsgEvent loginMsgEvent1 = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent1);
            
            RDMDirectoryMsgEvent directoryMsgEvent1 = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent1);
            assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent1.rdmDirectoryMsg().rdmMsgType());
            
            ReactorMsgEvent msgEvent1 = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent1);
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }
                
    @Test
    public void watchlistItemRecoveryDueToDirectoryStreamCloseTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                    + "/170_Provider_ItemRecoveryDueToDirectoryStream.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
            
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
            
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            consumerRole.watchlistOptions().requestTimeout(5000);
            
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
        
            // submit an item before login and directory stream is open
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyMsgKeyInUpdates();
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount()); 
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount());  // extra for itemRequest
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            /*
             * testServer then read item requests and then send the Item Response
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the ItemRefresh and ItemUpdate to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(5, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 1 for submitted item requests
            // 1 for Item Refresh due to fanout 
            // 1 for the TRI item update fanout
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // verify that update has msgKey in it
            UpdateMsg updateMsg = (UpdateMsg)msgEvent.msg();
            assertTrue(updateMsg.checkHasMsgKey());
            
            // have the TestServer send the DirectoryStatus to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the DirectoryStatus Closed, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(2, callbackHandler.directoryMsgEventCount());
            assertEquals(7, callbackHandler.defaultMsgEventCount());   // one more loginStatus, directory update, and itemStatus callback 
            
            // Directory update deleting service.
            RDMDirectoryMsgEvent directoryMsgEvent1 = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent1);                       
            assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent1.rdmDirectoryMsg().rdmMsgType());
            assertEquals(1, ((DirectoryUpdate)directoryMsgEvent1.rdmDirectoryMsg()).serviceList().size());
            assertEquals(MapEntryActions.DELETE, ((DirectoryUpdate)directoryMsgEvent1.rdmDirectoryMsg()).serviceList().get(0).action());
            
            // Item status.
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertEquals(DomainTypes.MARKET_PRICE, msgEvent.msg().domainType());
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            StatusMsg statusMsg = (StatusMsg)msgEvent.msg();
            assertTrue(statusMsg.checkHasState());
            assertEquals(StreamStates.OPEN, statusMsg.state().streamState());
            assertEquals(DataStates.SUSPECT, statusMsg.state().dataState());
            
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
            
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }                   
    }
    
    
    @Test
    public void watchlistItemRecoveryDueToItemStreamCloseTest()
    {
        // Open/Suspect 
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/180_Provider_ItemRecoveryDueToItemStream.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest(); 
            consumerRole.initDefaultRDMDirectoryRequest();
            LoginRequest loginRequest = consumerRole.rdmLoginRequest();
            loginRequest.attrib().applyHasSingleOpen();
            loginRequest.attrib().singleOpen(1);
            loginRequest.attrib().applyHasAllowSuspectData();
            loginRequest.attrib().allowSuspectData(-1);

            consumerRole.watchlistOptions().requestTimeout(5000);

            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
                        
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
    
            // submit an item before login and directory stream is open
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyMsgKeyInUpdates();
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
           
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount()); 
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
 
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount());  // extra for itemRequest
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            /*
             * testServer then read item requests and then send the Item Response
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the ItemRefresh and ItemUpdate to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(5, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 1 for submitted item requests
                                                                     // 1 for Item Refresh due to fanout 
                                                                     // 1 for the TRI item update fanout
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // verify that update has msgKey in it
            UpdateMsg updateMsg = (UpdateMsg)msgEvent.msg();
            assertTrue(updateMsg.checkHasMsgKey());

            // have the TestServer send the Item Suspect to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the LoginStatus Closed, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(6, callbackHandler.defaultMsgEventCount());   // one more loginStatus, one more itemStatus callback 
            ReactorMsgEvent defaultMsgEvent1 = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(defaultMsgEvent1);           
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }
        
            
    @Test
    public void watchlistItemRecoveryAndCloseAgainTest()
    {
        // Open/Suspect 
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/190_Provider_ItemRecoveryAndCloseAgain.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            consumerRole.initDefaultRDMLoginRequest(); 
            consumerRole.initDefaultRDMDirectoryRequest();
 
            consumerRole.watchlistOptions().requestTimeout(5000);

            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
                        
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_UP, event.eventType());
    
            // submit an item before login and directory stream is open
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.applyMsgKeyInUpdates();
            requestMsg.applyStreaming();
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.containerType(DataTypes.NO_DATA);
            requestMsg.msgKey().applyHasNameType();
            requestMsg.msgKey().nameType(InstrumentNameTypes.RIC);
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
            // have the TestServer send the LoginRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            
            /*
             * call dispatch which should read the LoginRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount()); 
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
 
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount());  // extra for itemRequest
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            /*
             * testServer then read item requests and then send the Item Response
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.MARKET_PRICE);
            // have the TestServer send the ItemRefresh and ItemUpdate to the Reactor.
            testServer.writeMessageToSocket(replay.read());
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the Item Refresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the defaultMsgCallback was called.
            assertEquals(5, callbackHandler.defaultMsgEventCount()); // 1 for login, 1 for directory, 1 for submitted item requests
                                                                     // 1 for Item Refresh due to fanout 
                                                                     // 1 for the TRI item update fanout
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            // verify that update has msgKey in it
            UpdateMsg updateMsg = (UpdateMsg)msgEvent.msg();
            assertTrue(updateMsg.checkHasMsgKey());
 
            // have the TestServer send the Item Close/Recover to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the ItemStatus Closed, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(6, callbackHandler.defaultMsgEventCount());   // one more loginStatus, one more itemStatus callback 
            ReactorMsgEvent defaultMsgEvent1 = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(defaultMsgEvent1);         
            
            // submit a close request on the item 
            CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
            closeMsg.clear();
            closeMsg.msgClass(MsgClasses.CLOSE);
            closeMsg.streamId(5);
            closeMsg.domainType(DomainTypes.MARKET_PRICE);
            closeMsg.containerType(DataTypes.NO_DATA);     
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(closeMsg, submitOptions, errorInfo));
            
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistUserDirectoryRequestTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/200_Provider_DirectoryRequestClose.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            
            // submit login request and make sure selector is triggered and dispatch succeeds
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals("DIRECT_FEED", submitOptions.serviceName());
            assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
            assertNull(consumerRole.rdmLoginRequest());
            consumerRole.initDefaultRDMLoginRequest();

            LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
            assertNotNull(loginRequest);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequest, submitOptions, errorInfo));
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
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
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            assertTrue(loginMsgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(loginMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertTrue(msgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(msgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            


            // Send Normal Directory Request to succeed
            DirectoryRequest directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
            directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
            directoryRequest.streamId(2);
            directoryRequest.filter(Directory.ServiceFilterFlags.INFO |
                    Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP);
            directoryRequest.applyStreaming();
            assertNotNull(directoryRequest);
            // remove serviceName from submitOptions so all services will be returned
            ReactorSubmitOptions directoryRequestSubmitOptions = ReactorFactory.createReactorSubmitOptions();
            directoryRequestSubmitOptions.clear();
            directoryRequestSubmitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(directoryRequest, directoryRequestSubmitOptions, errorInfo));
            
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            assertNull(directoryMsgEvent.streamInfo().serviceName());
            assertTrue(directoryMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            DirectoryRefresh returnDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent._directoryMsg;
            RefreshMsg returnRefreshMsg = (RefreshMsg)directoryMsgEvent.msg();
            
            // Check RDM message contents
            assertEquals(2, returnDirectoryRefresh.streamId());
            assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP, returnDirectoryRefresh.filter());
            for (int i = 0; i < returnDirectoryRefresh.serviceList().size(); ++i)
            {
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasData());
            	assertTrue(returnDirectoryRefresh.serviceList().get(i).checkHasInfo());
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasLink());
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasLoad());
            	assertTrue(returnDirectoryRefresh.serviceList().get(i).checkHasState());
            }
            assertEquals(0, returnDirectoryRefresh.serviceId());
            assertEquals("IDN_RDF", returnDirectoryRefresh.serviceList().get(0).info().serviceName().toString());
            assertEquals("ELEKTRON_DD", returnDirectoryRefresh.serviceList().get(1).info().serviceName().toString());
            assertEquals("QPROV1", returnDirectoryRefresh.serviceList().get(2).info().serviceName().toString());
            assertEquals("QPROV2", returnDirectoryRefresh.serviceList().get(3).info().serviceName().toString());
            assertEquals("NI_PUB", returnDirectoryRefresh.serviceList().get(4).info().serviceName().toString());
            
            // Check Codec message contents
            assertEquals(2, returnRefreshMsg.streamId());
            assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP, returnRefreshMsg.msgKey().filter());
            assertEquals(0, returnRefreshMsg.msgKey().serviceId());
            
            
            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
            
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }
    
    @Test
    public void watchlistDefaultDirectoryRequestTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/200_Provider_DirectoryRequestClose.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            
            // submit login request and make sure selector is triggered and dispatch succeeds
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals("DIRECT_FEED", submitOptions.serviceName());
            assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
            assertNull(consumerRole.rdmLoginRequest());
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
            assertNotNull(loginRequest);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequest, submitOptions, errorInfo));
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
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
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            assertTrue(loginMsgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(loginMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertTrue(msgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(msgEvent.streamInfo().userSpecObject().equals("Unit Test"));

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // Stream info is not set because there has been no submit on the Directory Handler
            assertNull(directoryMsgEvent.streamInfo().serviceName());
            assertNull(directoryMsgEvent.streamInfo().userSpecObject());
            
            DirectoryRefresh returnDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent._directoryMsg;
            RefreshMsg returnRefreshMsg = (RefreshMsg)directoryMsgEvent.msg();
            
            // Check RDM message contents
            assertEquals(2, returnDirectoryRefresh.streamId());
            assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP, returnDirectoryRefresh.filter());
            assertEquals(0, returnDirectoryRefresh.serviceId());
            assertEquals("IDN_RDF", returnDirectoryRefresh.serviceList().get(0).info().serviceName().toString());
            assertEquals("ELEKTRON_DD", returnDirectoryRefresh.serviceList().get(1).info().serviceName().toString());
            assertEquals("QPROV1", returnDirectoryRefresh.serviceList().get(2).info().serviceName().toString());
            assertEquals("QPROV2", returnDirectoryRefresh.serviceList().get(3).info().serviceName().toString());
            assertEquals("NI_PUB", returnDirectoryRefresh.serviceList().get(4).info().serviceName().toString());
            
            // Check Codec message contents
            assertEquals(2, returnRefreshMsg.streamId());
            assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP, returnRefreshMsg.msgKey().filter());
            assertEquals(0, returnRefreshMsg.msgKey().serviceId());
            
            
            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
            
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistUserDirectoryRequestReissueTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/200_Provider_DirectoryRequestClose.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
        
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            
            // submit login request and make sure selector is triggered and dispatch succeeds
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals("DIRECT_FEED", submitOptions.serviceName());
            assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
            assertNull(consumerRole.rdmLoginRequest());
            consumerRole.initDefaultRDMLoginRequest();

            LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
            assertNotNull(loginRequest);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequest, submitOptions, errorInfo));
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
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
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            assertTrue(loginMsgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(loginMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertTrue(msgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(msgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            


            // Send Normal Directory Request to succeed
            DirectoryRequest directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
            directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
            directoryRequest.streamId(2);
            directoryRequest.filter(Directory.ServiceFilterFlags.INFO |
                    Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP);
            directoryRequest.applyStreaming();
            assertNotNull(directoryRequest);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(directoryRequest, submitOptions, errorInfo));
            
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            assertTrue(directoryMsgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(directoryMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            DirectoryRefresh returnDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent._directoryMsg;
            RefreshMsg returnRefreshMsg = (RefreshMsg)directoryMsgEvent.msg();
            
            // Check RDM message contents
            assertEquals(2, returnDirectoryRefresh.streamId());
            assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP, returnDirectoryRefresh.filter());
            for (int i = 0; i < returnDirectoryRefresh.serviceList().size(); ++i)
            {
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasData());
            	assertTrue(returnDirectoryRefresh.serviceList().get(i).checkHasInfo());
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasLink());
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasLoad());
            	assertTrue(returnDirectoryRefresh.serviceList().get(i).checkHasState());
            }
            assertEquals(0, returnDirectoryRefresh.serviceId());
            
            // Check Codec message contents
            assertEquals(2, returnRefreshMsg.streamId());
            assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP, returnRefreshMsg.msgKey().filter());
            assertEquals(0, returnRefreshMsg.msgKey().serviceId());
            
            // Send Normal Directory Request Reissue to succeed
            directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
            directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
            directoryRequest.streamId(2);
            directoryRequest.filter(Directory.ServiceFilterFlags.DATA);
            directoryRequest.applyStreaming();
            assertNotNull(directoryRequest);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(directoryRequest, submitOptions, errorInfo));
            
            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(2, callbackHandler.directoryMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount());
            directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            assertTrue(directoryMsgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(directoryMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            returnDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent._directoryMsg;
            returnRefreshMsg = (RefreshMsg)directoryMsgEvent.msg();
            
            // Check RDM message contents
            assertEquals(2, returnDirectoryRefresh.streamId());
            assertEquals(Directory.ServiceFilterFlags.DATA, returnDirectoryRefresh.filter());
            for (int i = 0; i < returnDirectoryRefresh.serviceList().size(); ++i)
            {
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasData());
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasInfo());
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasLink());
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasLoad());
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasState());
            }
            assertEquals(0, returnDirectoryRefresh.serviceId());
            
            // Check Codec message contents
            assertEquals(2, returnRefreshMsg.streamId());
            assertEquals(Directory.ServiceFilterFlags.DATA, returnRefreshMsg.msgKey().filter());
            assertEquals(0, returnRefreshMsg.msgKey().serviceId());
            
            
            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
            
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }
    
    @Test
    public void watchlistUserDirectoryRequestBeforeLoginTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/200_Provider_DirectoryRequestClose.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
        int updCnt;
        Set<SelectionKey> keys;
        
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            
            // submit login request and make sure selector is triggered and dispatch succeeds
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals("DIRECT_FEED", submitOptions.serviceName());
            assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
            assertNull(consumerRole.rdmLoginRequest());
            consumerRole.initDefaultRDMLoginRequest();
            
            // Send Normal Directory Request to succeed before login request is sent.
            DirectoryRequest directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
            directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
            directoryRequest.streamId(2);
            directoryRequest.filter(Directory.ServiceFilterFlags.INFO |
                    Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP | Directory.ServiceFilterFlags.DATA);
            directoryRequest.applyStreaming();
            assertNotNull(directoryRequest);

            // do not set serviceName because we want to test that all services are received
            ReactorSubmitOptions directoryRequestSubmitOptions = ReactorFactory.createReactorSubmitOptions();
            directoryRequestSubmitOptions.clear();
            directoryRequestSubmitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(directoryRequest, directoryRequestSubmitOptions, errorInfo));

            /*
             * go back to the select loop, the consumerReactorChannel should
             * not trigger because the request has not yet been sent
             */
            updCnt = selector.select(1000); // 1 second
            assertFalse(updCnt > 0);
            keys = selector.selectedKeys();
            assertNotNull(keys);
            assertEquals(0, keys.size());


            LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
            assertNotNull(loginRequest);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequest, submitOptions, errorInfo));
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
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
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            assertEquals(0, callbackHandler.directoryMsgEventCount()); // DirectoryHandler has not yet sent the request message
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            assertTrue(loginMsgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(loginMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertTrue(msgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(msgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent  directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            assertTrue(directoryMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            DirectoryRefresh returnDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent._directoryMsg;
            RefreshMsg returnRefreshMsg = (RefreshMsg)directoryMsgEvent.msg();
            
            // Check RDM message contents
            assertEquals(2, returnDirectoryRefresh.streamId());
            assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP | Directory.ServiceFilterFlags.DATA, returnDirectoryRefresh.filter());
            for (int i = 0; i < returnDirectoryRefresh.serviceList().size(); ++i)
            {
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasData());
            	assertTrue(returnDirectoryRefresh.serviceList().get(i).checkHasInfo());
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasLink());
            	assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasLoad());
            	assertTrue(returnDirectoryRefresh.serviceList().get(i).checkHasState());
            	assertEquals(0, returnDirectoryRefresh.serviceList().get(i).groupStateList().size());
            }
            assertEquals(0, returnDirectoryRefresh.serviceId());
            assertEquals("IDN_RDF", returnDirectoryRefresh.serviceList().get(0).info().serviceName().toString());
            assertEquals("ELEKTRON_DD", returnDirectoryRefresh.serviceList().get(1).info().serviceName().toString());
            assertEquals("QPROV1", returnDirectoryRefresh.serviceList().get(2).info().serviceName().toString());
            assertEquals("QPROV2", returnDirectoryRefresh.serviceList().get(3).info().serviceName().toString());
            assertEquals("NI_PUB", returnDirectoryRefresh.serviceList().get(4).info().serviceName().toString());
            
            // Check Codec message contents
            assertEquals(2, returnRefreshMsg.streamId());
            assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP | Directory.ServiceFilterFlags.DATA, returnRefreshMsg.msgKey().filter());
            assertEquals(0, returnRefreshMsg.msgKey().serviceId());
            
            
            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
            
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }
    
    @Test
    public void watchlistDirectoryRequestServiceSuccessAndFailTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/200_Provider_DirectoryRequestClose.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
        
    
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            
            // submit login request and make sure selector is triggered and dispatch succeeds
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals("DIRECT_FEED", submitOptions.serviceName());
            assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
            assertNull(consumerRole.rdmLoginRequest());
            consumerRole.initDefaultRDMLoginRequest();

            LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
            assertNotNull(loginRequest);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequest, submitOptions, errorInfo));
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
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
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            assertTrue(loginMsgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(loginMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertTrue(msgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(msgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            


            
            // Send Normal Directory Request to succeed
            DirectoryRequest directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
            directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
            directoryRequest.streamId(2);
            directoryRequest.filter(Directory.ServiceFilterFlags.INFO |
                    Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP);
            directoryRequest.applyStreaming();
            assertNotNull(directoryRequest);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(directoryRequest, submitOptions, errorInfo));
           
            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            
            assertTrue(directoryMsgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(directoryMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            DirectoryRefresh returnDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent._directoryMsg;
            assertEquals(2, returnDirectoryRefresh.streamId());
            assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP, returnDirectoryRefresh.filter());
            assertEquals(0, returnDirectoryRefresh.serviceId());
            
            // Send failing directory request because of changing Service Name
            directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
            directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
            directoryRequest.streamId(2);
            directoryRequest.filter(Directory.ServiceFilterFlags.DATA);
            directoryRequest.applyHasServiceId();
            directoryRequest.serviceId(0);
            submitOptions.serviceName("TRI");
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(directoryRequest, submitOptions, errorInfo));
            
            // Send failing directory request because of changing service Id
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            requestMsg.domainType(DomainTypes.SOURCE);
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.msgKey().applyHasServiceId();
            requestMsg.msgKey().applyHasFilter();
            requestMsg.streamId(2);
            requestMsg.msgKey().filter(Directory.ServiceFilterFlags.GROUP | Directory.ServiceFilterFlags.LINK);
            requestMsg.msgKey().serviceId(10);
            submitOptions.serviceName("DIRECT_FEED");
            assertEquals(ReactorReturnCodes.INVALID_USAGE, event.reactorChannel().submit(requestMsg, submitOptions, errorInfo));

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
            
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }
    
    @Test
    public void watchlistDirectoryDownTest()
    {
        final String inputFile = BASE_TEST_DATA_DIR_NAME
                + "/200_Provider_DirectoryRequestClose.txt";
        Reactor reactor = null;
        ReactorErrorInfo errorInfo = null;
        ReactorChannel theReactorChannel = null;
        Selector selector = null;
        ReactorCallbackHandler callbackHandler = null;
        TestServer testServer = null;
            
        try
        {
            NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
            reactor = ReactorJunit.createReactor(errorInfo);
    
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
            ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
            callbackHandler = new ReactorCallbackHandler(selector);
            assertEquals(null, callbackHandler.lastChannelEvent());
            ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);
            
            // if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
            assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));
            
            // if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
            consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));
    
            // wait for the TestServer to accept a connection.
            testServer.waitForAcceptable();
            testServer.acceptSocket();

            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyConnectReq(testServer.buffer());
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
            assertEquals(3, callbackHandler.channelEventCount());
            assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
            assertEquals(1, callbackHandler.channelUpEventCount());
            assertEquals(1, callbackHandler.channelReadyEventCount());
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());
            
            // submit login request and make sure selector is triggered and dispatch succeeds
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            submitOptions.serviceName("DIRECT_FEED");
            submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
            assertEquals("DIRECT_FEED", submitOptions.serviceName());
            assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
            assertNull(consumerRole.rdmLoginRequest());
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
            assertNotNull(loginRequest);
            assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequest, submitOptions, errorInfo));
            
            /*
             * the reactor will send out our default LoginRequest. wait for
             * testServer to read the LoginRequest, then send the LoginResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
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
            // verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.loginMsgEventCount());
            assertEquals(1, callbackHandler.defaultMsgEventCount());
            RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
            assertNotNull(loginMsgEvent);
            assertTrue(loginMsgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(loginMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
            ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
            ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            assertTrue(msgEvent.streamInfo().serviceName().equals("DIRECT_FEED"));
            assertTrue(msgEvent.streamInfo().userSpecObject().equals("Unit Test"));

            /*
             * the reactor will send out our default DirectoryRequest. wait for
             * testServer to read the DirectoryRequest, then send the DirectoryResponse
             */
            testServer.waitForReadable();
            // wait and read one message.
            assertTrue(testServer.readMessageFromSocket() > 0);
            ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
            // have the TestServer send the DirectoryRefresh to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the DirectoryRefresh, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(1, callbackHandler.directoryMsgEventCount());
            assertEquals(2, callbackHandler.defaultMsgEventCount());
            RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent);
            ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);

            // Get Directory Close
            
            // have the TestServer send the Directory Close to the Reactor.
            testServer.writeMessageToSocket(replay.read());

            /*
             * call dispatch which should read the Directory Close, invoke
             * callback. Have callback return RAISE, then Reactor should call
             * defaultMsgCallback.
             */
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
            ReactorJunit.dispatchReactor(selector, reactor);
            // verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
            assertEquals(2, callbackHandler.directoryMsgEventCount());
            assertEquals(3, callbackHandler.defaultMsgEventCount());
            directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
            assertNotNull(directoryMsgEvent.rdmDirectoryMsg());	// Null because we get a status message back w/ close
            assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
            msgEvent = callbackHandler.lastDefaultMsgEvent();
            assertNotNull(msgEvent);
            for (int i = 0; i < (((DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg()).serviceList().size()); ++i)
            {
            	assertEquals(MapEntryActions.DELETE, ((DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg()).serviceList().get(i).action());
            }

            // reset the msgReturnCode to SUCCESS.
            callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);
            
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
            errorInfo.clear();
            assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
            assertEquals(true, reactor.isShutdown());
    
            ReactorChannelEvent event = callbackHandler.lastChannelEvent();
            assertNotNull(event);
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
        }
    }

    @Test
    public void watchlistUserDirectoryRequestTestSpecificDirectory()
    {
    	final String inputFile = BASE_TEST_DATA_DIR_NAME
    			+ "/200_Provider_DirectoryRequestClose.txt";
    	Reactor reactor = null;
    	ReactorErrorInfo errorInfo = null;
    	ReactorChannel theReactorChannel = null;
    	Selector selector = null;
    	ReactorCallbackHandler callbackHandler = null;
    	TestServer testServer = null;

    	try
    	{
    		NetworkReplay replay = ReactorJunit.parseReplayFile(inputFile);
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
    		reactor = ReactorJunit.createReactor(errorInfo);

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
    		ReactorConnectOptions rcOpts = ReactorJunit.createDefaultConsumerConnectOptions(String.valueOf(serverPort));
    		callbackHandler = new ReactorCallbackHandler(selector);
    		assertEquals(null, callbackHandler.lastChannelEvent());
    		ConsumerRole consumerRole = createWatchlistConsumerRole(callbackHandler);

    		// if watchlist enabled, connect cannot succeed with dictionaryDownloadMode = DictionaryDownloadModes.FIRST_AVAILABLE
    		consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.FIRST_AVAILABLE);
    		assertEquals(ReactorReturnCodes.INVALID_USAGE, reactor.connect(rcOpts, consumerRole, errorInfo));

    		// if watchlist enabled, connect can only succeed with dictionaryDownloadMode = DictionaryDownloadModes.NONE
    		consumerRole.dictionaryDownloadMode(DictionaryDownloadModes.NONE);
    		assertEquals(ReactorReturnCodes.SUCCESS, reactor.connect(rcOpts, consumerRole, errorInfo));

    		// wait for the TestServer to accept a connection.
    		testServer.waitForAcceptable();
    		testServer.acceptSocket();

    		// wait and read one message.
    		assertTrue(testServer.readMessageFromSocket() > 0);
    		ReactorJunit.verifyConnectReq(testServer.buffer());
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
    		assertEquals(3, callbackHandler.channelEventCount());
    		assertEquals(1, callbackHandler.channelOpenedEventCount()); // watchlist callback
    		assertEquals(1, callbackHandler.channelUpEventCount());
    		assertEquals(1, callbackHandler.channelReadyEventCount());
    		ReactorChannelEvent event = callbackHandler.lastChannelEvent();
    		assertNotNull(event);
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, event.eventType());

    		// submit login request and make sure selector is triggered and dispatch succeeds
    		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
    		submitOptions.serviceName("NI_PUB");
    		submitOptions.requestMsgOptions().userSpecObj(new String("Unit Test"));
    		assertEquals("NI_PUB", submitOptions.serviceName());
    		assertNotNull(submitOptions.requestMsgOptions().userSpecObj());
    		assertNull(consumerRole.rdmLoginRequest());
    		consumerRole.initDefaultRDMLoginRequest();

    		LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
    		assertNotNull(loginRequest);
    		assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(loginRequest, submitOptions, errorInfo));

    		/*
    		 * the reactor will send out our default LoginRequest. wait for
    		 * testServer to read the LoginRequest, then send the LoginResponse
    		 */
    		testServer.waitForReadable();
    		// wait and read one message.
    		assertTrue(testServer.readMessageFromSocket() > 0);
    		ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.LOGIN);
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
    		// verify that the RDMLoginMsgCallback and defaultMsgCallback was called.
    		assertEquals(1, callbackHandler.loginMsgEventCount());
    		assertEquals(1, callbackHandler.defaultMsgEventCount());
    		RDMLoginMsgEvent loginMsgEvent = callbackHandler.lastLoginMsgEvent();
    		assertNotNull(loginMsgEvent);
    		assertTrue(loginMsgEvent.streamInfo().serviceName().equals("NI_PUB"));
    		assertTrue(loginMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
    		ReactorJunit.verifyLoginMessage(loginMsgEvent.rdmLoginMsg());
    		ReactorMsgEvent msgEvent = callbackHandler.lastDefaultMsgEvent();
    		assertNotNull(msgEvent);
    		assertTrue(msgEvent.streamInfo().serviceName().equals("NI_PUB"));
    		assertTrue(msgEvent.streamInfo().userSpecObject().equals("Unit Test"));



    		// Send Normal Directory Request to succeed
    		DirectoryRequest directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
    		directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
    		directoryRequest.streamId(2);
    		directoryRequest.filter(Directory.ServiceFilterFlags.INFO |
    				Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP);
    		directoryRequest.applyStreaming();
    		assertNotNull(directoryRequest);
    		    		
    		assertEquals(ReactorReturnCodes.SUCCESS, event.reactorChannel().submit(directoryRequest, submitOptions, errorInfo));

    		/*
    		 * the reactor will send out our default DirectoryRequest. wait for
    		 * testServer to read the DirectoryRequest, then send the DirectoryResponse
    		 */
    		testServer.waitForReadable();
    		// wait and read one message.
    		assertTrue(testServer.readMessageFromSocket() > 0);
    		ReactorJunit.verifyMessage(testServer.buffer(), MsgClasses.REQUEST, DomainTypes.SOURCE);
    		// have the TestServer send the DirectoryRefresh to the Reactor.
    		testServer.writeMessageToSocket(replay.read());

    		/*
    		 * call dispatch which should read the DirectoryRefresh, invoke
    		 * callback. Have callback return RAISE, then Reactor should call
    		 * defaultMsgCallback.
    		 */
    		callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.RAISE);
    		ReactorJunit.dispatchReactor(selector, reactor);
    		// verify that the RDMDirectoryMsgCallback and defaultMsgCallback was called.
    		assertEquals(1, callbackHandler.directoryMsgEventCount());
    		assertEquals(2, callbackHandler.defaultMsgEventCount());
    		RDMDirectoryMsgEvent directoryMsgEvent = callbackHandler.lastDirectoryMsgEvent();
    		assertNotNull(directoryMsgEvent);
    		ReactorJunit.verifyDirectoryMessage(directoryMsgEvent.rdmDirectoryMsg());
    		msgEvent = callbackHandler.lastDefaultMsgEvent();
    		assertNotNull(msgEvent);

    		assertTrue(msgEvent.streamInfo().serviceName().equals("NI_PUB"));
    		assertTrue(directoryMsgEvent.streamInfo().userSpecObject().equals("Unit Test"));
    		DirectoryRefresh returnDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent._directoryMsg;
    		RefreshMsg returnRefreshMsg = (RefreshMsg)directoryMsgEvent.msg();

    		// Check RDM message contents
    		assertEquals(2, returnDirectoryRefresh.streamId());
    		assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP, returnDirectoryRefresh.filter());
    		assertEquals(returnDirectoryRefresh.serviceList().size(), 1);
    		for (int i = 0; i < returnDirectoryRefresh.serviceList().size(); ++i)
    		{
    			assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasData());
    			assertTrue(returnDirectoryRefresh.serviceList().get(i).checkHasInfo());
    			assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasLink());
    			assertFalse(returnDirectoryRefresh.serviceList().get(i).checkHasLoad());
    			assertTrue(returnDirectoryRefresh.serviceList().get(i).checkHasState());
    		}
    		assertEquals(0, returnDirectoryRefresh.serviceId());
    		assertEquals("NI_PUB", returnDirectoryRefresh.serviceList().get(0).info().serviceName().toString());
   
    		// Check Codec message contents
    		assertEquals(2, returnRefreshMsg.streamId());
    		assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP, returnRefreshMsg.msgKey().filter());
    		assertEquals(0, returnRefreshMsg.msgKey().serviceId());


    		// reset the msgReturnCode to SUCCESS.
    		callbackHandler.msgReturnCode(ReactorCallbackReturnCodes.SUCCESS);

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
    		errorInfo.clear();
    		assertEquals(ReactorReturnCodes.SUCCESS, reactor.shutdown(errorInfo));
    		assertEquals(true, reactor.isShutdown());

    		ReactorChannelEvent event = callbackHandler.lastChannelEvent();
    		assertNotNull(event);
    		assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, event.eventType());
    	}
    }
}
