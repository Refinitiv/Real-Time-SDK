/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assume.assumeTrue;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.file.Files;
import java.nio.file.Paths;
import java.util.ArrayList;
import java.util.List;
import java.util.Objects;
import java.util.Optional;

import com.refinitiv.eta.transport.ConnectOptions;
import org.junit.After;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.rdm.Login.ServerTypes;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginConsumerConnectionStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequestFlags;
import com.refinitiv.eta.valueadd.reactor.ReactorAuthTokenInfo.TokenVersion;
import org.junit.rules.TestName;


public class ReactorWatchlistLDPJunit
{
	final int AUTH_TOKEN_EXPIRATION = 600;
	final int AUTH_V2_TOKEN_EXPIRATION = 6899;
	ReactorAuthTokenInfo _tokenInfo = null;	
	private static final Buffer proxyHost = CodecFactory.createBuffer();
	private static final Buffer proxyPort = CodecFactory.createBuffer();
	private static final Buffer proxyUser = CodecFactory.createBuffer();
	private static final Buffer proxyPassword = CodecFactory.createBuffer();
	private static final Buffer proxyLocalHostname = CodecFactory.createBuffer();
	private static final Buffer proxyKRBConfigFile = CodecFactory.createBuffer();
	private static final Buffer proxyDomain = CodecFactory.createBuffer();

	public ReactorWatchlistLDPJunit() {
		proxyHost.data(System.getProperty("proxyHost"));
		proxyPort.data(System.getProperty("proxyPort"));
		proxyUser.data(System.getProperty("proxyUser"));
		proxyPassword.data(System.getProperty("proxyPassword"));
		proxyLocalHostname.data(System.getProperty("proxyLocalHostname"));
		proxyKRBConfigFile.data(System.getProperty("proxyKRBConfigFile"));
		String proxyDomainStr = Optional
				.ofNullable(System.getProperty("proxyDomain"))
				.orElseGet(() -> proxyHost.toString() + ":" + proxyPort.toString());
		proxyDomain.data(proxyDomainStr);
	}

	abstract class ReactorServiceEndpointEventCallbackTest implements ReactorServiceEndpointEventCallback
	{
		public int _count = 0;
		public ReactorServiceEndpointInfo _endpointInfo = null;
		public String host = "";
		public String port = "";
	}
	
	abstract class ReactorOAuthCredentialEventCallbackTest implements ReactorOAuthCredentialEventCallback
	{
		ReactorOAuthCredential _oauthCredentail = null;
		
		ReactorOAuthCredentialEventCallbackTest(ReactorOAuthCredential oAuthCredential)
		{
			_oauthCredentail = oAuthCredential;
		}
	}
	
	/* This data dictionary is used by JSON converter library. */
	final static DataDictionary dictionary = CodecFactory.createDataDictionary();

	@Rule
	public TestName testName = new TestName();

    @Before
    public void init() {

		System.out.println(">>>>>>>>>>>>>>>>>>>>  " + testName.getMethodName() + " Test <<<<<<<<<<<<<<<<<<<<<<<");

		final String dictionaryFileName = "../../../Java/etc/RDMFieldDictionary";
        final String enumTypeFile = "../../../Java/etc/enumtype.def";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();

        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary(dictionaryFileName, error));
        assertEquals(CodecReturnCodes.SUCCESS,dictionary.loadEnumTypeDictionary(enumTypeFile, error));
    }

	@After
	public void tearDown()
	{
		try { Thread.sleep(JUnitConfigVariables.WAIT_AFTER_TEST); }
		catch (Exception e) { }
	}
	
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

	boolean checkCredentials()
	{
		if (System.getProperty("edpUserName") != null &&
				(System.getProperty("edpPassword") != null ||
				System.getProperty("clientSecret") != null ||
				System.getProperty("jwkFile") != null))
		{
			return true;
		}
		else
		{
			System.out.println("edpUserName and either edpPassword or clientSecret need to be set as VM arguments to run this test.");
			System.out.println("i.e. -DedpUserName=USERNAME -DedpPassword=PASSWORD -DclientSecret=SECRET -DjwkFile=FILELOCATION");
			System.out.println("or with gradle i.e. ./gradlew eta:valueadd:test --tests *LDP* -PvmArgs=\"-DedpUserName=USERNAME -DedpPassword=PASSWORD -DclientSecret=SECRET -DjwkFile=FILELOCATION\"");
			System.out.println("Skipping this test");			
			return false;
		}
	}

	static boolean checkProxy() {
		if (Objects.nonNull(proxyHost.toString()) &&
				Objects.nonNull(proxyPort.toString())) {
			return true;
		}
		return false;
	}

	static boolean checkProxyCredentials() {
		if (Objects.isNull(proxyUser.toString()) ||
				Objects.isNull(proxyPassword.toString())) {
			System.out.println("WARNING: Proxy authenticate credentials haven't been passed.");
			return false;
		}
		return true;
	}

	/* create default consumer connect options */
	static ReactorConnectOptions createDefaultConsumerConnectOptionsForSessionManagment(TestReactorComponent reactor, boolean isWebsocket, String protocolList)
	{
		ReactorConnectOptions rcOpts = ReactorFactory.createReactorConnectOptions();
		ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
		assertNotNull(rcOpts);
		assertEquals(ReactorReturnCodes.PARAMETER_OUT_OF_RANGE, connectInfo.initTimeout(0));
		assertEquals(ReactorReturnCodes.SUCCESS, connectInfo.initTimeout(20));
		connectInfo.connectOptions().connectionType(ConnectionTypes.ENCRYPTED);
		if (isWebsocket) {
			connectInfo.connectOptions().encryptionOptions().connectionType(ConnectionTypes.WEBSOCKET);
			connectInfo.connectOptions().wSocketOpts().protocols(protocolList);
		}
		connectInfo.connectOptions().majorVersion(Codec.majorVersion());
		connectInfo.connectOptions().minorVersion(Codec.minorVersion());

		if(isWebsocket)
		{
			if(Objects.nonNull(System.getProperty("keyfile")))
				connectInfo.connectOptions().encryptionOptions().KeystoreFile(System.getProperty("keyfile"));

			if(Objects.nonNull(System.getProperty("keypasswd")))
				connectInfo.connectOptions().encryptionOptions().KeystorePasswd(System.getProperty("keypasswd"));

			connectInfo.connectOptions().encryptionOptions().KeystoreType("JKS");
			connectInfo.connectOptions().encryptionOptions().SecurityProtocol("TLS");
			connectInfo.connectOptions().encryptionOptions().SecurityProtocolVersions(new String[] {"1.3", "1.2"});
			connectInfo.connectOptions().encryptionOptions().SecurityProvider("SunJSSE");
			connectInfo.connectOptions().encryptionOptions().KeyManagerAlgorithm("SunX509");
			connectInfo.connectOptions().encryptionOptions().TrustManagerAlgorithm("PKIX");
			connectInfo.connectOptions().userSpecObject(reactor);
			
			if (protocolList.contains("tr_json2") || protocolList.contains("rssl.json.v2")) {
				reactor.testReactor().initJsonConverter(dictionary);
            		}
		}
		else
		{
			if(Objects.nonNull(System.getProperty("keyfile")))
				connectInfo.connectOptions().tunnelingInfo().KeystoreFile(System.getProperty("keyfile"));

			if(Objects.nonNull(System.getProperty("keypasswd")))
				connectInfo.connectOptions().tunnelingInfo().KeystorePasswd(System.getProperty("keypasswd"));

			connectInfo.connectOptions().tunnelingInfo().objectName("");
			connectInfo.connectOptions().tunnelingInfo().KeystoreType("JKS");
			connectInfo.connectOptions().tunnelingInfo().SecurityProtocol("TLS");
			connectInfo.connectOptions().tunnelingInfo().SecurityProvider("SunJSSE");
			connectInfo.connectOptions().tunnelingInfo().KeyManagerAlgorithm("SunX509");
			connectInfo.connectOptions().tunnelingInfo().TrustManagerAlgorithm("PKIX");		
			connectInfo.connectOptions().tunnelingInfo().tunnelingType("encrypted");			
			connectInfo.connectOptions().userSpecObject(reactor);
		}

		connectInfo.enableSessionManagement(true);
		setupProxyForConnectOptions(connectInfo.connectOptions());
		rcOpts.connectionList().add(connectInfo);
		
		rcOpts.reconnectAttemptLimit(5);
		rcOpts.reconnectMinDelay(1000);
		rcOpts.reconnectMaxDelay(1000);
		rcOpts.connectionList().get(0).connectOptions().pingTimeout(255);
		rcOpts.connectionList().get(0).initTimeout(10);		
		return rcOpts;
	}
	
	TestReactorEvent getTestEvent(TestReactor testReactor, int loopCount)
	{
		TestReactorEvent event = null;
		while((loopCount > 0) && ((event = testReactor.pollEvent()) == null))
		{
			try {
				Thread.sleep(1000);
				testReactor.dispatch(-1);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			loopCount--;
		}
		
		return event;
	}


	@Test
	public void LDPConnectSpecificLocationTest_Socket() {
		LDPConnectSpecificLocation(false, null);
	}

	@Test
	public void LDPConnectSpecificLocationTest_WebSocket_Rwf() {
		LDPConnectSpecificLocation(true, "rssl.rwf");
	}

	@Test
	public void LDPConnectSpecificLocationTest_WebSocket_Json() {
		LDPConnectSpecificLocation(true, "tr_json2");
	}

	private void LDPConnectSpecificLocation(boolean isWebsocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectSpecificLocationTest <<<<<<<<<<\n");
		/* Test a queryServiceDiscovery */
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		

			ReactorErrorInfo errorInfo = null;		 

			/* Create reactor. */
			consumerReactor = new TestReactor();
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;	  

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocolList);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);
			rcOpts.connectionList().get(0).location("eu");

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);

			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName() == null);	


			TestReactorEvent event = null;
			ReactorChannelEvent chnlEvent = null;
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());			


			verifyAuthTokenEvent(consumerReactor, 10, true, true);

		}
		finally
		{
			consumerReactor.close();
		}		

	}	

	private int verifyAuthTokenEvent(TestReactor consumerReactor,int howLongToWait, boolean success, boolean dispatch)
	{
		TestReactorEvent event = null;
		int count = 0;
		// Consumer receives Auth Service Token Event
		System.out.print("Waiting for auth event: ");
		while (event == null)
		{
			System.out.print(count + " ");
			
			if(dispatch)
			{
				consumerReactor.dispatch(-1);
			}
			
			event = consumerReactor.pollEvent();

			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			count++;
			if (count > howLongToWait)
				break;
		}
		System.out.println("");		

		if (success)
		{
			assertNotNull("Did not receive AUTH_TOKEN_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.AUTH_TOKEN_EVENT, received: " + event.type(), TestReactorEventTypes.AUTH_TOKEN_EVENT, event.type());
			ReactorAuthTokenEvent authTokenEvent = (ReactorAuthTokenEvent)event.reactorEvent();
			System.out.println(authTokenEvent.reactorAuthTokenInfo());				

			_tokenInfo = authTokenEvent.reactorAuthTokenInfo();
			assertEquals(_tokenInfo.expiresIn(), AUTH_TOKEN_EXPIRATION);   	
			return ((_tokenInfo.expiresIn() / 5) * 4);
		}
		else
		{
			if(event.type() == TestReactorEventTypes.CHANNEL_EVENT)
			{
				ReactorChannelEvent chnlEvent = null;				
				assertNotNull("Did not receive CHANNEL_EVENT", event);
				assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
				chnlEvent = (ReactorChannelEvent)event.reactorEvent();
				assertEquals("Expected ReactorChannelEventTypes.WARNING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.WARNING, chnlEvent.eventType());				
			}
			else if(event.type() == TestReactorEventTypes.AUTH_TOKEN_EVENT)
			{
				assertNotNull("Did not receive AUTH_TOKEN_EVENT", event);
				assertEquals("Expected TestReactorEventTypes.AUTH_TOKEN_EVENT, received: " + event.type(), TestReactorEventTypes.AUTH_TOKEN_EVENT, event.type());	
			}
			
			event = consumerReactor.pollEvent();			

			if(event != null)
			{
				if(event.type() == TestReactorEventTypes.CHANNEL_EVENT)
				{
					ReactorChannelEvent chnlEvent = null;				
					assertNotNull("Did not receive CHANNEL_EVENT", event);
					assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
					chnlEvent = (ReactorChannelEvent)event.reactorEvent();
					assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());				
				}
				else if(event.type() == TestReactorEventTypes.AUTH_TOKEN_EVENT)
				{
					assertNotNull("Did not receive AUTH_TOKEN_EVENT", event);
					assertEquals("Expected TestReactorEventTypes.AUTH_TOKEN_EVENT, received: " + event.type(), TestReactorEventTypes.AUTH_TOKEN_EVENT, event.type());
					ReactorAuthTokenEvent authTokenEvent = (ReactorAuthTokenEvent)event.reactorEvent();
					System.out.println(authTokenEvent.reactorAuthTokenInfo());			
	
					System.out.println(authTokenEvent.errorInfo().toString());
					assertTrue(authTokenEvent.errorInfo().toString().contains("Invalid username or password"));		
				}
			}
			
		}
		return -1;
	}
	
	private int verifyAuthV2TokenEvent(TestReactor consumerReactor,int howLongToWait, boolean success, boolean dispatch)
	{
		TestReactorEvent event = null;
		int count = 0;
		// Consumer receives Auth Service Token Event
		System.out.print("Waiting for auth event: ");
		while (event == null)
		{
			System.out.print(count + " ");
			
			if(dispatch)
			{
				consumerReactor.dispatch(-1);
			}
			
			event = consumerReactor.pollEvent();

			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			count++;
			if (count > howLongToWait)
				break;
		}
		System.out.println("");		

		if (success)
		{
			assertNotNull("Did not receive AUTH_TOKEN_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.AUTH_TOKEN_EVENT, received: " + event.type(), TestReactorEventTypes.AUTH_TOKEN_EVENT, event.type());
			ReactorAuthTokenEvent authTokenEvent = (ReactorAuthTokenEvent)event.reactorEvent();
			System.out.println(authTokenEvent.reactorAuthTokenInfo());				

			_tokenInfo = authTokenEvent.reactorAuthTokenInfo();
			assertEquals(_tokenInfo.expiresIn(), AUTH_V2_TOKEN_EXPIRATION);   	
			return ((_tokenInfo.expiresIn() / 5) * 4);
		}
		else
		{
			if(event.type() == TestReactorEventTypes.CHANNEL_EVENT)
			{
				ReactorChannelEvent chnlEvent = null;				
				assertNotNull("Did not receive CHANNEL_EVENT", event);
				assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
				chnlEvent = (ReactorChannelEvent)event.reactorEvent();
				assertEquals("Expected ReactorChannelEventTypes.WARNING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.WARNING, chnlEvent.eventType());				
			}
			else if(event.type() == TestReactorEventTypes.AUTH_TOKEN_EVENT)
			{
				assertNotNull("Did not receive AUTH_TOKEN_EVENT", event);
				assertEquals("Expected TestReactorEventTypes.AUTH_TOKEN_EVENT, received: " + event.type(), TestReactorEventTypes.AUTH_TOKEN_EVENT, event.type());	
			}
			
			event = consumerReactor.pollEvent();			

			if(event != null)
			{
				if(event.type() == TestReactorEventTypes.CHANNEL_EVENT)
				{
					ReactorChannelEvent chnlEvent = null;				
					assertNotNull("Did not receive CHANNEL_EVENT", event);
					assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
					chnlEvent = (ReactorChannelEvent)event.reactorEvent();
					assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());				
				}
				else if(event.type() == TestReactorEventTypes.AUTH_TOKEN_EVENT)
				{
					assertNotNull("Did not receive AUTH_TOKEN_EVENT", event);
					assertEquals("Expected TestReactorEventTypes.AUTH_TOKEN_EVENT, received: " + event.type(), TestReactorEventTypes.AUTH_TOKEN_EVENT, event.type());
					ReactorAuthTokenEvent authTokenEvent = (ReactorAuthTokenEvent)event.reactorEvent();
					System.out.println(authTokenEvent.reactorAuthTokenInfo());			
	
					System.out.println(authTokenEvent.errorInfo().toString());
					assertTrue(authTokenEvent.errorInfo().toString().contains("Invalid username or password"));		
				}
			}
			
		}
		return -1;
	}

	private void setupProxyForReactorRestProxyOptions(ReactorRestProxyOptions reactorRestProxyOptions) {
		if (reactorRestProxyOptions != null && checkProxy()) {
			reactorRestProxyOptions.proxyHostName(proxyHost);
			reactorRestProxyOptions.proxyPort(proxyPort);
			if (checkProxyCredentials()) {
				reactorRestProxyOptions.proxyUserName(proxyUser);
				reactorRestProxyOptions.proxyPassword(proxyPassword);
				reactorRestProxyOptions.proxyDomain(proxyDomain);
				if (Objects.nonNull(proxyLocalHostname.toString())) {
					reactorRestProxyOptions.proxyLocalHostName(proxyLocalHostname);
				} else if (Objects.nonNull(proxyKRBConfigFile.toString())) {
					reactorRestProxyOptions.proxyKrb5ConfigFile(proxyKRBConfigFile);
				}
			}
		}
	}

	private void setupProxyForReactorRenewalOptions(ReactorOAuthCredentialRenewalOptions renewalOptions) {
		if (renewalOptions != null && checkProxy()) {
			renewalOptions.proxyHostName(proxyHost);
			renewalOptions.proxyPort(proxyPort);
			if (checkProxyCredentials()) {
				renewalOptions.proxyUserName(proxyUser);
				renewalOptions.proxyPassword(proxyPassword);
				renewalOptions.proxyDomain(proxyDomain);
				if (Objects.nonNull(proxyLocalHostname.toString())) {
					renewalOptions.proxyLocalHostName(proxyLocalHostname);
				} else if (Objects.nonNull(proxyKRBConfigFile.toString())) {
					renewalOptions.proxyKRB5ConfigFile(proxyKRBConfigFile);
				}
			}
		}
	}

	private void setupProxyForReactorServiceDiscoveryOptions(ReactorServiceDiscoveryOptions serviceDiscoveryOptions) {
		if (serviceDiscoveryOptions != null && checkProxy()) {
			serviceDiscoveryOptions.proxyHostName(proxyHost);
			serviceDiscoveryOptions.proxyPort(proxyPort);
			if (checkProxyCredentials()) {
				serviceDiscoveryOptions.proxyDomain(proxyDomain);
				serviceDiscoveryOptions.proxyUserName(proxyUser);
				serviceDiscoveryOptions.proxyPassword(proxyPassword);
				if (Objects.nonNull(proxyLocalHostname.toString())) {
					serviceDiscoveryOptions.proxyLocalHostName(proxyLocalHostname);
				} else if (Objects.nonNull(proxyKRBConfigFile.toString())) {
					serviceDiscoveryOptions.proxyKRB5ConfigFile(proxyKRBConfigFile);
				}
			}
		}
	}

	private static void setupProxyForConnectOptions(ConnectOptions connectOptions) {
		if (connectOptions != null && checkProxy()) {
			connectOptions.tunnelingInfo().HTTPproxy(true);
			connectOptions.tunnelingInfo().HTTPproxyHostName(proxyHost.toString());
			connectOptions.tunnelingInfo().HTTPproxyPort(Integer.parseInt(proxyPort.toString()));
			if (checkProxyCredentials()) {
				connectOptions.credentialsInfo().HTTPproxyDomain(proxyDomain.toString());
				connectOptions.credentialsInfo().HTTPproxyUsername(proxyUser.toString());
				connectOptions.credentialsInfo().HTTPproxyPasswd(proxyPassword.toString());
				final String proxyLocalHostname = ReactorWatchlistLDPJunit.proxyLocalHostname.toString();
				final String proxyKRBConfigFile = ReactorWatchlistLDPJunit.proxyKRBConfigFile.toString();
				if (Objects.nonNull(proxyLocalHostname)) {
					connectOptions.credentialsInfo().HTTPproxyLocalHostname(proxyLocalHostname);
				} else if (Objects.nonNull(proxyKRBConfigFile)) {
					connectOptions.credentialsInfo().HTTPproxyKRB5configFile(proxyKRBConfigFile);
				}
			}
		}
	}

	@Test
	public void LDPConnectErrorInvalidConnectionTypeTest() {
		LDPConnectErrorInvalidConnectionType(false, null);
	}

	@Test
	public void LDPConnectErrorInvalidConnectionTypeTest_WebSocket_Rwf() {
		LDPConnectErrorInvalidConnectionType(true, "rssl.rwf");
	}

	@Test
	public void LDPConnectErrorInvalidConnectionTypeTest_WebSocket_Json() {
		LDPConnectErrorInvalidConnectionType(false, "tr_json2");
	}

	private void LDPConnectErrorInvalidConnectionType(boolean isWebsocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectErrorInvalidConnectionTypeTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {

			ReactorErrorInfo errorInfo = null;		 

			/* Create reactor. */
			consumerReactor = new TestReactor();
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocolList);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);
			// set invalid connection type
			rcOpts.connectionList().get(0).connectOptions().connectionType(ConnectionTypes.SOCKET);

			assertTrue(consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.PARAMETER_INVALID);

			// fix the connection type and run it again. 
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  
			rcOpts.connectionList().get(0).connectOptions().connectionType(ConnectionTypes.ENCRYPTED);
			
			int ret = consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo);
			
			if (ret != ReactorReturnCodes.SUCCESS)
			{
				System.out.println("Location : " + errorInfo.location());
				System.out.println("Error text: " + errorInfo.error().text());
			}

			assertTrue("Expected SUCCESS", ret == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);			
		}
		finally
		{
			consumerReactor.close();
		}	
	}

	@Test
	public void LDPConnectErrorInvalidLocationTest() {
		LDPConnectErrorInvalidLocation(false, null);
	}

	@Test
	public void LDPConnectErrorInvalidLocationTest_WebSocket_Rwf() {
		LDPConnectErrorInvalidLocation(true, "rssl.rwf");
	}

	@Test
	public void LDPConnectErrorInvalidLocationTest_WebSocket_Json() {
		LDPConnectErrorInvalidLocation(true, "tr_json2");
	}

	private void LDPConnectErrorInvalidLocation(boolean isWebSocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectErrorInvalidLocationTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {

			ReactorErrorInfo errorInfo = null;

			/* Create reactor. */
			consumerReactor = new TestReactor();
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebSocket, protocolList);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);
			rcOpts.connectionList().get(0).location("invalid_location");

			assertTrue(consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.PARAMETER_INVALID); 

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  	

			// fix the location and run it again, also needs to reset the username and password, this time rest client already exists.
			consumerRole.rdmLoginRequest().userName().data(System.getProperty("edpUserName"));
			consumerRole.rdmLoginRequest().password().data(System.getProperty("edpPassword"));
			rcOpts.connectionList().get(0).location("eu");

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS); 
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);		

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			consumerRole.rdmLoginRequest().userName().data(System.getProperty("edpUserName"));
			consumerRole.rdmLoginRequest().password().data(System.getProperty("edpPassword"));
			rcOpts.connectionList().get(0).location("invalid_location");

			assertTrue(consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.PARAMETER_INVALID);			

		}
		finally
		{
			consumerReactor.close();
		}
	}	

	@Test
	public void LDPConnectErrorAddressAndSessionManagmentSpecifiedTest() {
		LDPConnectErrorAddressAndSessionManagmentSpecified(false, null);
	}

	private void LDPConnectErrorAddressAndSessionManagmentSpecified(boolean isWebsocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectErrorAddressAndSessionManagmentSpecifiedTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());	
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {

			ReactorErrorInfo errorInfo = null;

			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpPassword"));
				reactorServiceDiscoveryOptions.password(buf);
			}  	  	
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.clientId(buf);
			}

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					assertTrue(event.errorInfo().code() == ReactorReturnCodes.SUCCESS );
					assertTrue(event.serviceEndpointInfo().size() > 0);

					for (int i = 0; i < event.serviceEndpointInfo().size(); i++)
					{
						assertTrue(event.serviceEndpointInfo().get(i).transport().equals("tcp"));
						assertTrue(event.serviceEndpointInfo().get(i).dataFormatList().get(0).equals("rwf"));
						if (event.serviceEndpointInfo().get(i).locationList().size() == 2)
						{
							_endpointInfo = event.serviceEndpointInfo().get(i);
						}
					}
					System.out.println(event.serviceEndpointInfo());
					_count++;
					return 0;
				}
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);

			/* Create reactor. */
			consumerReactor = new TestReactor();
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocolList);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);
			rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address("FAKE ADDRESS");
			rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName("FAKE SERVICE NAME");			

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);   			

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called once
			assertTrue("callback was not called", callback._count == 1);

			assertTrue(callback._endpointInfo != null);
			rcOpts.connectionList().get(0).connectOptions()
			.unifiedNetworkInfo().address(callback._endpointInfo.endPoint());			
			rcOpts.connectionList().get(0).connectOptions()
			.unifiedNetworkInfo().serviceName(callback._endpointInfo.port());

			// call it again to test when rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			//call it again to test when rest reactor already created, this time fix invalid parameter
			rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocolList);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);
		}
		finally
		{
			consumerReactor.close();
		}
	}	

	@SuppressWarnings("deprecation")
	private void setupConsumer(Consumer consumer, boolean defaultRDMLogin)
	{

		ReactorCallbackHandler callbackHandler = null;
		Selector selector = null;		
		/* Create consumer. */
		callbackHandler = new ReactorCallbackHandler(selector);
		assertEquals(null, callbackHandler.lastChannelEvent());

		ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
		if (defaultRDMLogin)
		{
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
		}	
		consumerRole.channelEventCallback(consumer);
		consumerRole.loginMsgCallback(consumer);
		consumerRole.directoryMsgCallback(consumer);
		consumerRole.dictionaryMsgCallback(consumer);
		consumerRole.defaultMsgCallback(consumer);
		consumerRole.watchlistOptions().enableWatchlist(true);
		consumerRole.watchlistOptions().channelOpenCallback(consumer);
		consumerRole.watchlistOptions().requestTimeout(15000);
		consumerRole.clientId().data(System.getProperty("edpUserName"));
		
		if (defaultRDMLogin)
		{
			consumerRole.rdmLoginRequest().userName().data(System.getProperty("edpUserName"));
			consumerRole.rdmLoginRequest().password().data(System.getProperty("edpPassword"));
			consumerRole.rdmLoginRequest().applyHasPassword();

			// initialize consumer role to default	
			consumerRole.rdmLoginRequest().attrib().applyHasSingleOpen();
			consumerRole.rdmLoginRequest().attrib().singleOpen(1);
			consumerRole.rdmLoginRequest().attrib().applyHasAllowSuspectData();
			consumerRole.rdmLoginRequest().attrib().allowSuspectData(1);			
		}
	}

	@Test
	public void LDPConnectTest() {
		LDPConnect(false, null);
	}

	@Test
	public void LDPConnectTest_WebSocket_Json() {
		LDPConnect(true, "tr_json2");
	}

	private void LDPConnect(boolean isWebsocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		

			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();	

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocolList);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);

			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName() == null);	

			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());			

			int sleep = verifyAuthTokenEvent(consumerReactor, 10, true, true);
			long runtime = System.currentTimeMillis() + ((sleep - 3) * 1000);		

			consumer.testReactor().dispatch(4, 8000);

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        

			verifyAuthTokenRequestAndLoginReissue(consumerReactor, consumer, 1, sleep, runtime, true, false);

		}
		finally
		{
			consumerReactor.close();
		}	
	}	


	@Test
	public void LDPConnectConnectionListTest() {
		LDPConnectConnectionList(false, null);
	}

	@Test
	public void LDPConnectConnectionListTest_WebSocket_Json() {
		LDPConnectConnectionList(true, "tr_json2");
	}

	private void LDPConnectConnectionList(boolean isWebsocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectConnectionListTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();	

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */

			// create first entry in the connection list that will fail
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			assertEquals(ReactorReturnCodes.SUCCESS, connectInfo.initTimeout(10));
			connectInfo.connectOptions().connectionType(ConnectionTypes.SOCKET);			
			connectInfo.connectOptions().majorVersion(Codec.majorVersion());
			connectInfo.connectOptions().minorVersion(Codec.minorVersion());
			connectInfo.connectOptions().unifiedNetworkInfo().address("localhost");
			connectInfo.connectOptions().unifiedNetworkInfo().serviceName("14002");
			connectInfo.connectOptions().userSpecObject(consumer);		
			connectInfo.initTimeout(40);
			connectInfo.enableSessionManagement(false);		
			setupProxyForConnectOptions(connectInfo.connectOptions());

			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocolList);

			ReactorConnectInfo connectInfoSecond = rcOpts.connectionList().remove(0);		

			rcOpts.connectionList().add(connectInfo);
			rcOpts.connectionList().add(connectInfoSecond);

			rcOpts.connectionList().get(1).reactorAuthTokenEventCallback(consumer);				

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			
			for (int j = 0; j < 5; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}	        

			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address().equals("localhost"));
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName().equals("14002"));			
			assertTrue(rcOpts.connectionList().get(1).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(1).connectOptions().unifiedNetworkInfo().serviceName() == null);

			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				

			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			

			for (int j = 0; j < 16; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}

			verifyAuthTokenEvent(consumerReactor, 15, true, true);

			for (int j = 0; j < 6; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}			

			// Consumer receives CHANNEL_UP event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.CHANNEL_UP)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			for (int j = 0; j < 6; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}				

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        
			
			assertTrue("Expected Service Discovery request going out", chnlEvent._reactorChannel.reactorServiceEndpointInfoList().size() > 0);			

		}
		finally
		{
			consumerReactor.close();
		};
	}	
	
	@Test
	public void LDPConnectConnectionList_PreferredHostEnabledTest() {
		LDPConnectConnectionList_PreferredHostEnabled(false, null);
	}

	@Test
	public void LDPConnectConnectionList_PreferredHostEnabledTest_WebSocket_Json() {
		LDPConnectConnectionList_PreferredHostEnabled(true, "tr_json2");
	}

	private void LDPConnectConnectionList_PreferredHostEnabled(boolean isWebsocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectConnectionList_PreferredHostEnabled <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();	

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */

			// create first entry in the connection list that will fail
			ReactorConnectInfo connectInfoNotWorkingConnection = ReactorFactory.createReactorConnectInfo();
			assertEquals(ReactorReturnCodes.SUCCESS, connectInfoNotWorkingConnection.initTimeout(10));
			connectInfoNotWorkingConnection.connectOptions().connectionType(ConnectionTypes.SOCKET);			
			connectInfoNotWorkingConnection.connectOptions().majorVersion(Codec.majorVersion());
			connectInfoNotWorkingConnection.connectOptions().minorVersion(Codec.minorVersion());
			connectInfoNotWorkingConnection.connectOptions().unifiedNetworkInfo().address("localhost");
			connectInfoNotWorkingConnection.connectOptions().unifiedNetworkInfo().serviceName("14002");
			connectInfoNotWorkingConnection.connectOptions().userSpecObject(consumer);		
			connectInfoNotWorkingConnection.initTimeout(40);
			connectInfoNotWorkingConnection.enableSessionManagement(false);		
			setupProxyForConnectOptions(connectInfoNotWorkingConnection.connectOptions());

			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocolList);

			rcOpts.reconnectAttemptLimit(1);
			rcOpts.reconnectMinDelay(1000);
			rcOpts.reconnectMaxDelay(1000);
			rcOpts.connectionList().get(0).connectOptions().pingTimeout(255);
			rcOpts.connectionList().get(0).initTimeout(10);	
			
			ReactorConnectInfo connectInfoWorkingConnection = rcOpts.connectionList().remove(0);		

			rcOpts.connectionList().add(connectInfoNotWorkingConnection);
			rcOpts.connectionList().add(connectInfoWorkingConnection);	
			
			rcOpts._reactorPreferredHostOptions.isPreferredHostEnabled(true);
			rcOpts._reactorPreferredHostOptions.detectionTimeInterval(5);
			rcOpts._reactorPreferredHostOptions.connectionListIndex(0);	

			rcOpts.connectionList().get(1).reactorAuthTokenEventCallback(consumer);		

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			
			for (int j = 0; j < 5; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}	        

			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address().equals("localhost"));
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName().equals("14002"));	
			assertTrue(rcOpts.connectionList().get(1).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(1).connectOptions().unifiedNetworkInfo().serviceName() == null);		

			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				

			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			

			for (int j = 0; j < 16; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}
			
			verifyAuthTokenEvent(consumerReactor, 15, true, true);

			for (int j = 0; j < 6; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}			

			// Consumer receives CHANNEL_UP event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.CHANNEL_UP)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			for (int j = 0; j < 6; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}				

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        
			
			assertTrue("Expected Service Discovery request going out", chnlEvent._reactorChannel.reactorServiceEndpointInfoList().size() > 0);			

		}
		finally
		{
			consumerReactor.close();
		};
	}	
	

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabledTest() {
		LDPConnectWarmStandby_PreferredHostEnabled(false, null);
	}

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabledTest_WebSocket_Json() {
		LDPConnectWarmStandby_PreferredHostEnabled(true, "tr_json2");
	}

	private void LDPConnectWarmStandby_PreferredHostEnabled(boolean isWebsocket, String protocolList)
	{
		// 2 Groups configured
		// 1st group is preferred
		// Ioctl changes Preferred group to 2nd
		// Connect with Service Discovery enabled
		// After DetectionTimeInterval, switchover should occur to 2nd group
		
		System.out.println("\n>>>>>>>>> Running LDPConnectWarmStandby_PreferredHostEnabled <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccountV2();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.reactorOAuthCredential(ReactorFactory.createReactorOAuthCredential());
			Buffer buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("edpUserName"));
			consumerRole.reactorOAuthCredential().clientId(buf);
			buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("clientSecret"));
			consumerRole.reactorOAuthCredential().clientSecret(buf);
			byte[] jwkFile = null;
			try {
				jwkFile = Files.readAllBytes(Paths.get(System.getProperty("jwkFile")));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			String jwkText = new String(jwkFile);
			buf = CodecFactory.createBuffer();
			buf.data(jwkText);
			consumerRole.reactorOAuthCredential().clientJwk(buf);
			consumerRole.reactorOAuthCredential().reactorOAuthCredentialEventCallback(consumer);
			consumer.setReactorOAuthCredentialInfo(consumerRole.reactorOAuthCredential());

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			// Create WSB Groups
			List<Integer> wsbGroup1 = new ArrayList<Integer>();
			List<Integer> wsbGroup2 = new ArrayList<Integer>();
			
			wsbGroup1.add(0);
			wsbGroup1.add(0);
			wsbGroup2.add(0);
			wsbGroup2.add(0);
			int channelPort = -1;

			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);

			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().detectionTimeInterval(10);
			connectOpts.reconnectAttemptLimit(1);
			
			connectOpts = consumer.testReactor().connectWsb_ByPort_SessionManagement_NoStart(opts, connectOpts, consumer,  consumer, protocolList, isWebsocket, dictionary, wsbGroup1, wsbGroup2, channelPort);

			consumer.testReactor().lateStartConnect(connectOpts, consumer, false);

			for (int j = 0; j < 5; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}	        
			
			// check that connection info was not overwritten
			assertTrue(connectOpts.connectionList().size() == 0);
			assertTrue(connectOpts.reactorWarmStandbyGroupList().size() == 2);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	

			
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				

			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        
			
			
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);

		
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			
			
    		// Call ioctl to change preferred group index to 1
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.warmStandbyGroupListIndex(1);
    		ioctlCall.detectionTimeInterval(5);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

			
			for (int j = 0; j < 16; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}
			
			// Consumer receives CHANNEL_READY event for Standby   
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.CHANNEL_READY)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());			


			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			// Verify new auth token
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			if (msgEvent.msg() != null)
				assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        

			for (int j = 0; j < 1; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}				
			
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);

			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			
			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			

			
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			
			
			// Consumer receives CHANNEL_READY event for Standby   
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.CHANNEL_READY)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());			

			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			
			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			
			
		}
		finally
		{
			consumerReactor.close();
		};
	}	

	@Test
	public void LDPConnectWarmStandby_ServiceBased_PreferredHostEnabledTest() {
		LDPConnectWarmStandby_ServiceBased_PreferredHostEnabled(false, null);
	}

	@Test
	public void LDPConnectWarmStandby_ServiceBased_PreferredHostEnabledTest_WebSocket_Json() {
		LDPConnectWarmStandby_ServiceBased_PreferredHostEnabled(true, "tr_json2");
	}

	private void LDPConnectWarmStandby_ServiceBased_PreferredHostEnabled(boolean isWebsocket, String protocolList)
	{
		// 2 Groups configured
		// 1st group is preferred
		// Ioctl changes Preferred group to 2nd
		// Connect with Service Discovery enabled
		// Service Based connection
		// After DetectionTimeInterval, switchover should occur to 2nd group
		
		System.out.println("\n>>>>>>>>> Running LDPConnectWarmStandby_ServiceBased_PreferredHostEnabled <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccountV2();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.reactorOAuthCredential(ReactorFactory.createReactorOAuthCredential());
			Buffer buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("edpUserName"));
			consumerRole.reactorOAuthCredential().clientId(buf);
			buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("clientSecret"));
			consumerRole.reactorOAuthCredential().clientSecret(buf);
			byte[] jwkFile = null;
			try {
				jwkFile = Files.readAllBytes(Paths.get(System.getProperty("jwkFile")));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			String jwkText = new String(jwkFile);
			buf = CodecFactory.createBuffer();
			buf.data(jwkText);
			consumerRole.reactorOAuthCredential().clientJwk(buf);
			consumerRole.reactorOAuthCredential().reactorOAuthCredentialEventCallback(consumer);
			consumer.setReactorOAuthCredentialInfo(consumerRole.reactorOAuthCredential());

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			// Create WSB Groups
			List<Integer> wsbGroup1 = new ArrayList<Integer>();
			List<Integer> wsbGroup2 = new ArrayList<Integer>();
			
			wsbGroup1.add(0);
			wsbGroup1.add(0);
			wsbGroup2.add(0);
			wsbGroup2.add(0);
			int channelPort = -1;

			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.SERVICE_BASED);

			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().detectionTimeInterval(10);
			connectOpts.reconnectAttemptLimit(1);
			
			connectOpts = consumer.testReactor().connectWsb_ByPort_SessionManagement_NoStart(opts, connectOpts, consumer,  consumer, protocolList, isWebsocket, dictionary, wsbGroup1, wsbGroup2, channelPort);

			consumer.testReactor().lateStartConnect(connectOpts, consumer, false);

			for (int j = 0; j < 5; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}	        
			
			// check that connection info was not overwritten
			assertTrue(connectOpts.connectionList().size() == 0);
			assertTrue(connectOpts.reactorWarmStandbyGroupList().size() == 2);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	

			
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				

			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        
			
			
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);

		
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			
			
    		// Call ioctl to change preferred group index to 1
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.warmStandbyGroupListIndex(1);
    		ioctlCall.detectionTimeInterval(5);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

			
			for (int j = 0; j < 16; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}
			
			// Consumer receives CHANNEL_READY event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.CHANNEL_READY)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());			

			

			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			// Verify new auth token
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			if (msgEvent.msg() != null)
				assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        

			for (int j = 0; j < 1; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}				
			
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);

			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			
			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			

			
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			
			
			// Consumer receives CHANNEL_READY event for Standby   
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.CHANNEL_READY)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());			

			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			
			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			
			
		}
		finally
		{
			consumerReactor.close();
		};
	}	

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_SDtoEPTest() {
		LDPConnectWarmStandby_PreferredHostEnabled_SDtoEPTest(false, null);
	}

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_SDtoEPTest_WebSocket_Json() {
		LDPConnectWarmStandby_PreferredHostEnabled_SDtoEPTest(true, "tr_json2");
	}

	private void LDPConnectWarmStandby_PreferredHostEnabled_SDtoEPTest(boolean isWebsocket, String protocolList)
	{
		// 2 Groups configured
		// 1st group is preferred
		// Ioctl changes Preferred group to 2nd
		// Connect with Service Discovery enabled, then switch to one where Endpoint is specified
		// SD (Service Discovery) to EP (Endpoint specified) test
		// After DetectionTimeInterval, switchover should occur to 2nd group
		
		System.out.println("\n>>>>>>>>> Running LDPConnectWarmStandby_PreferredHostEnabled_SDtoEPTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccountV2();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.reactorOAuthCredential(ReactorFactory.createReactorOAuthCredential());
			Buffer buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("edpUserName"));
			consumerRole.reactorOAuthCredential().clientId(buf);
			buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("clientSecret"));
			consumerRole.reactorOAuthCredential().clientSecret(buf);
			byte[] jwkFile = null;
			try {
				jwkFile = Files.readAllBytes(Paths.get(System.getProperty("jwkFile")));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			String jwkText = new String(jwkFile);
			buf = CodecFactory.createBuffer();
			buf.data(jwkText);
			consumerRole.reactorOAuthCredential().clientJwk(buf);
			consumerRole.reactorOAuthCredential().reactorOAuthCredentialEventCallback(consumer);
			consumer.setReactorOAuthCredentialInfo(consumerRole.reactorOAuthCredential());

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			// Create WSB Groups
			List<Integer> wsbGroup1 = new ArrayList<Integer>();
			List<Integer> wsbGroup2 = new ArrayList<Integer>();
			
			wsbGroup1.add(0);
			wsbGroup1.add(0);
			wsbGroup2.add(2);
			wsbGroup2.add(2);
			int channelPort = -1;

			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);

			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().detectionTimeInterval(10);
			connectOpts.reconnectAttemptLimit(1);
			
			connectOpts = consumer.testReactor().connectWsb_ByPort_SessionManagement_NoStart(opts, connectOpts, consumer,  consumer, protocolList, isWebsocket, dictionary, wsbGroup1, wsbGroup2, channelPort);

			consumer.testReactor().lateStartConnect(connectOpts, consumer, false);

			for (int j = 0; j < 7; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}	        
			
			// check that connection info was not overwritten
			assertTrue(connectOpts.connectionList().size() == 0);
			assertTrue(connectOpts.reactorWarmStandbyGroupList().size() == 2);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	
			if (isWebsocket)
			{
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS_WEBSOCKET);	
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS_WEBSOCKET);	
			}
			else
			{
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS);	
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS);		
			}
			
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				

			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        
			
			
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);

		
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			

			// Consumer receives CHANNEL_READY event for Standby   
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.CHANNEL_READY)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());			

			
    		// Call ioctl to change preferred group index to 1
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.warmStandbyGroupListIndex(1);
    		ioctlCall.detectionTimeInterval(5);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

			
			for (int j = 0; j < 16; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}
			

			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			// Verify new auth token
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			if (msgEvent.msg() != null)
				assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        

			for (int j = 0; j < 1; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}				
			
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);

			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			
			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			

			
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			
			
			// Consumer receives CHANNEL_READY event for Standby   
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.CHANNEL_READY)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());			

			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			
			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			
			
		}
		finally
		{
			consumerReactor.close();
		};
	}	

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_EPtoSDTest() {
		LDPConnectWarmStandby_PreferredHostEnabled_EPtoSDTest(false, null);
	}

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_EPtoSDTest_WebSocket_Json() {
		LDPConnectWarmStandby_PreferredHostEnabled_EPtoSDTest(true, "tr_json2");
	}

	private void LDPConnectWarmStandby_PreferredHostEnabled_EPtoSDTest(boolean isWebsocket, String protocolList)
	{
		// 2 Groups configured
		// 1st group is preferred
		// Ioctl changes Preferred group to 2nd
		// Connect with Endpoint specified, then switch to one where Service Discovery enabled
		// EP (Endpoint specified) to SD (Service Discovery) test
		// After DetectionTimeInterval, switchover should occur to 2nd group
		
		System.out.println("\n>>>>>>>>> Running LDPConnectWarmStandby_PreferredHostEnabled_EPtoSDTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccountV2();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.reactorOAuthCredential(ReactorFactory.createReactorOAuthCredential());
			Buffer buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("edpUserName"));
			consumerRole.reactorOAuthCredential().clientId(buf);
			buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("clientSecret"));
			consumerRole.reactorOAuthCredential().clientSecret(buf);
			byte[] jwkFile = null;
			try {
				jwkFile = Files.readAllBytes(Paths.get(System.getProperty("jwkFile")));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			String jwkText = new String(jwkFile);
			buf = CodecFactory.createBuffer();
			buf.data(jwkText);
			consumerRole.reactorOAuthCredential().clientJwk(buf);
			consumerRole.reactorOAuthCredential().reactorOAuthCredentialEventCallback(consumer);
			consumer.setReactorOAuthCredentialInfo(consumerRole.reactorOAuthCredential());

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			// Create WSB Groups
			List<Integer> wsbGroup1 = new ArrayList<Integer>();
			List<Integer> wsbGroup2 = new ArrayList<Integer>();
			
			wsbGroup1.add(2);
			wsbGroup1.add(2);
			wsbGroup2.add(0);
			wsbGroup2.add(0);
			int channelPort = -1;

			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);

			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().detectionTimeInterval(10);
			connectOpts.reconnectAttemptLimit(1);
			
			connectOpts = consumer.testReactor().connectWsb_ByPort_SessionManagement_NoStart(opts, connectOpts, consumer,  consumer, protocolList, isWebsocket, dictionary, wsbGroup1, wsbGroup2, channelPort);

			consumer.testReactor().lateStartConnect(connectOpts, consumer, false);

			for (int j = 0; j < 5; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}	        
			
			// check that connection info was not overwritten
			assertTrue(connectOpts.connectionList().size() == 0);
			assertTrue(connectOpts.reactorWarmStandbyGroupList().size() == 2);	
			if (isWebsocket)
			{
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS_WEBSOCKET);	
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS_WEBSOCKET);		
			}
			else
			{
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS);	
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS);	
			}
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == null);	

			
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				

			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        
			
			
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);

		
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			
			
    		// Call ioctl to change preferred group index to 1
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.warmStandbyGroupListIndex(1);
    		ioctlCall.detectionTimeInterval(5);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

			
			for (int j = 0; j < 16; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}
			

			// Consumer receives CHANNEL_READY event for Standby   
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.CHANNEL_READY)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());			


			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			// Verify new auth token
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			if (msgEvent.msg() != null)
				assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 
			

			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			
			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			


			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        

			for (int j = 0; j < 1; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}				
			
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			
			
			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			
			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			

			
			// Consumer receives CHANNEL_READY event for Standby   
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.CHANNEL_READY)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());			

		}
		finally
		{
			consumerReactor.close();
		};
	}	

	Provider createDefaultProvider(Provider provider, TestReactor providerReactor)
	{
		provider = new Provider(providerReactor);
		ProviderRole providerRole = (ProviderRole)provider.reactorRole();
		providerRole.channelEventCallback(provider);
		providerRole.loginMsgCallback(provider);
		providerRole.directoryMsgCallback(provider);
		providerRole.dictionaryMsgCallback(provider);
		providerRole.defaultMsgCallback(provider);
		
		return provider;
	}
	
	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_NormaltoEPTest() {
		LDPConnectWarmStandby_PreferredHostEnabled_NormaltoEPTest(false, null);
	}

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_NormaltoEPTest_WebSocket_Json() {
		LDPConnectWarmStandby_PreferredHostEnabled_NormaltoEPTest(true, "tr_json2");
	}

	private void LDPConnectWarmStandby_PreferredHostEnabled_NormaltoEPTest(boolean isWebsocket, String protocolList)
	{
		// 2 Groups configured
		// 1st group is preferred
		// Ioctl changes Preferred group to 2nd
		// Connect with a normal Provider, then switch to one where Session Management and an Endpoint is specified
		// Normal connection to EP (Endpoint specified) test
		// After DetectionTimeInterval, switchover should occur to 2nd group
		
		System.out.println("\n>>>>>>>>> Running LDPConnectWarmStandby_PreferredHostEnabled_NormaltoEPTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccountV2();
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		Provider provider = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(1);
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);

			int port1 = provider.bindGetPort(opts);

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.reactorOAuthCredential(ReactorFactory.createReactorOAuthCredential());
			Buffer buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("edpUserName"));
			consumerRole.reactorOAuthCredential().clientId(buf);
			buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("clientSecret"));
			consumerRole.reactorOAuthCredential().clientSecret(buf);
			byte[] jwkFile = null;
			try {
				jwkFile = Files.readAllBytes(Paths.get(System.getProperty("jwkFile")));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			String jwkText = new String(jwkFile);
			buf = CodecFactory.createBuffer();
			buf.data(jwkText);
			consumerRole.reactorOAuthCredential().clientJwk(buf);
			consumerRole.reactorOAuthCredential().reactorOAuthCredentialEventCallback(consumer);
			consumer.setReactorOAuthCredentialInfo(consumerRole.reactorOAuthCredential());

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			// Create WSB Groups
			List<Integer> wsbGroup1 = new ArrayList<Integer>();
			List<Integer> wsbGroup2 = new ArrayList<Integer>();
			
			System.out.println(port1);
			wsbGroup1.add(port1);
			wsbGroup1.add(-1);
			wsbGroup2.add(2);
			wsbGroup2.add(2);
			int channelPort = -1;

			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().detectionTimeInterval(10);
			connectOpts.reconnectAttemptLimit(1);
			
			connectOpts = consumer.testReactor().connectWsb_ByPort_SessionManagement_NoStart(opts, connectOpts, consumer,  consumer, protocolList, isWebsocket, dictionary, wsbGroup1, wsbGroup2, channelPort);

			consumer.testReactor().lateStartConnect(connectOpts, consumer, false);
			
			consumer.testReactor().dispatch(-1);
			
			provider.testReactor().accept(opts, provider, 5000);
			
			/* Provider receives channel-up/channel-ready */
			provider.testReactor().dispatch(2);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());
			
			consumer.testReactor().dispatch(-1);
			
			// check that connection info was not overwritten
			assertTrue(connectOpts.connectionList().size() == 0);
			assertTrue(connectOpts.reactorWarmStandbyGroupList().size() == 2);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == "localhost");	
			//assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == "localhost");	
			if (isWebsocket)
			{
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS_WEBSOCKET);	
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS_WEBSOCKET);	
			}
			else
			{
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS);	
				assertTrue(connectOpts.reactorWarmStandbyGroupList().get(1).standbyServerList().get(0).reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS);		
			}
			
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				

			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			RDMLoginMsgEvent loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			LoginRequest loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

	        loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
	        loginRefresh.applyHasAttrib();
	        loginRefresh.applyHasFeatures();
	        loginRefresh.features().applyHasSupportOptimizedPauseResume();
	        loginRefresh.features().supportOptimizedPauseResume(1);
	        loginRefresh.features().applyHasSupportViewRequests();
	        loginRefresh.features().supportViewRequests(1);
	        loginRefresh.features().applyHasSupportPost();
	        loginRefresh.features().supportOMMPost(1);
	        loginRefresh.features().applyHasSupportStandby();
	        loginRefresh.features().supportStandby(1);
	        loginRefresh.features().applyHasSupportStandbyMode();
	        loginRefresh.features().supportStandbyMode(3);
	        loginRefresh.state().streamState(StreamStates.OPEN);
	        loginRefresh.state().dataState(DataStates.OK);
	        loginRefresh.state().code(StateCodes.NONE);
	        loginRefresh.state().text().data("Login OK");
	        
	        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 1 receives receives consumer connection status and directory request. */
			provider.testReactor().dispatch(2);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);


			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			DirectoryRequest directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			DirectoryRefresh directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

	        directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);

	        directoryRefresh.clear();
	        directoryRefresh.streamId(directoryRequest.streamId());
	        directoryRefresh.filter(directoryRequest.filter());
	        directoryRefresh.applySolicited();
	        directoryRefresh.applyClearCache();
	        directoryRefresh.state().streamState(StreamStates.OPEN);
	        directoryRefresh.state().dataState(DataStates.OK);
	        directoryRefresh.state().code(StateCodes.NONE);
	        directoryRefresh.state().text().data("Source Directory Refresh Complete");

	        Service service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(3);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		chnlEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());

			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			

			consumer.testReactor().dispatch(-1);
			
    		// Call ioctl to change preferred group index to 1
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.warmStandbyGroupListIndex(1);
    		ioctlCall.detectionTimeInterval(5);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peekTail();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

			
			for (int j = 0; j < 16; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}
			
			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			

			// Consumer receives FD_CHANGE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());			


			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			// Consumer receives CHANNEL_DOWN_RECONNECTING event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			
			
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			
			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			// Consumer receives PREFERRED_HOST_COMPLETE event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 
			
			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());			

			// Consumer receives PREFERRED_HOST_COMPLETE event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			


			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			if (msgEvent.msg() != null)
				assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			// Consumer receives CHANNEL_READY event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());			

			// Verify new auth token
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.FD_CHANGE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.FD_CHANGE, chnlEvent.eventType());	        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());	        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());	        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        

		}
		finally
		{
			consumerReactor.close();
			
			provider.close();
			providerReactor.close();
		};
	}	

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoSDTest() {
		LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoSDTest(false, null);
	}

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoSDTest_WebSocket_Json() {
		LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoSDTest(true, "tr_json2");
	}

	private void LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoSDTest(boolean isWebsocket, String protocolList)
	{
		// 1 Group configured
		// 1st group is preferred
		// Connect with a normal Provider, then kill Group 1 and switch to connection list where Service Discovery is configured
		// Normal connection to Service Discovery test
		
		System.out.println("\n>>>>>>>>> Running LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoSDTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccountV2();
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		Provider provider = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(1);
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);

			int port1 = provider.bindGetPort(opts);

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.reactorOAuthCredential(ReactorFactory.createReactorOAuthCredential());
			Buffer buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("edpUserName"));
			consumerRole.reactorOAuthCredential().clientId(buf);
			buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("clientSecret"));
			consumerRole.reactorOAuthCredential().clientSecret(buf);
			byte[] jwkFile = null;
			try {
				jwkFile = Files.readAllBytes(Paths.get(System.getProperty("jwkFile")));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			String jwkText = new String(jwkFile);
			buf = CodecFactory.createBuffer();
			buf.data(jwkText);
			consumerRole.reactorOAuthCredential().clientJwk(buf);
			consumerRole.reactorOAuthCredential().reactorOAuthCredentialEventCallback(consumer);
			consumer.setReactorOAuthCredentialInfo(consumerRole.reactorOAuthCredential());

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			// Create WSB Groups
			List<Integer> wsbGroup1 = new ArrayList<Integer>();
			
			System.out.println(port1);
			wsbGroup1.add(port1);
			int channelPort = 0;

			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			//connectOpts.reactorPreferredHostOptions().detectionTimeInterval(10);
			connectOpts.reconnectAttemptLimit(1);
			
			connectOpts = consumer.testReactor().connectWsb_ByPort_SessionManagement_NoStart(opts, connectOpts, consumer,  consumer, protocolList, isWebsocket, dictionary, wsbGroup1, null, channelPort);

			consumer.testReactor().lateStartConnect(connectOpts, consumer, false);
			
			consumer.testReactor().dispatch(-1);
			
			provider.testReactor().accept(opts, provider, 5000);
			
			/* Provider receives channel-up/channel-ready */
			provider.testReactor().dispatch(2);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());
			
			consumer.testReactor().dispatch(-1);
			
			// check that connection info was not overwritten
			assertTrue(connectOpts.connectionList().size() == 1);
			assertTrue(connectOpts.reactorWarmStandbyGroupList().size() == 1);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == "localhost");	
			assertTrue(connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == null);
			
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				

			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			RDMLoginMsgEvent loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			LoginRequest loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

	        loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
	        loginRefresh.applyHasAttrib();
	        loginRefresh.applyHasFeatures();
	        loginRefresh.features().applyHasSupportOptimizedPauseResume();
	        loginRefresh.features().supportOptimizedPauseResume(1);
	        loginRefresh.features().applyHasSupportViewRequests();
	        loginRefresh.features().supportViewRequests(1);
	        loginRefresh.features().applyHasSupportPost();
	        loginRefresh.features().supportOMMPost(1);
	        loginRefresh.features().applyHasSupportStandby();
	        loginRefresh.features().supportStandby(1);
	        loginRefresh.features().applyHasSupportStandbyMode();
	        loginRefresh.features().supportStandbyMode(3);
	        loginRefresh.state().streamState(StreamStates.OPEN);
	        loginRefresh.state().dataState(DataStates.OK);
	        loginRefresh.state().code(StateCodes.NONE);
	        loginRefresh.state().text().data("Login OK");
	        
	        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 1 receives receives consumer connection status and directory request. */
			provider.testReactor().dispatch(2);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);


			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			DirectoryRequest directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			DirectoryRefresh directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

	        directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);

	        directoryRefresh.clear();
	        directoryRefresh.streamId(directoryRequest.streamId());
	        directoryRefresh.filter(directoryRequest.filter());
	        directoryRefresh.applySolicited();
	        directoryRefresh.applyClearCache();
	        directoryRefresh.state().streamState(StreamStates.OPEN);
	        directoryRefresh.state().dataState(DataStates.OK);
	        directoryRefresh.state().code(StateCodes.NONE);
	        directoryRefresh.state().text().data("Source Directory Refresh Complete");

	        Service service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		chnlEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());

			consumer.testReactor().dispatch(-1);
			
			// Kill Provider 1, switch to Channel List (Endpoint specified)
			provider.close();
			
			for (int j = 0; j < 16; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}

			// Consumer receives CHANNEL_DOWN_RECONNECTING event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			


			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());

			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			

			// Verify new auth token
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			
			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			if (chnlEvent.eventType() == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
			{
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());				

				// Consumer receives CHANNEL_UP event
				event = consumerReactor.pollEvent();
				assertNotNull("Did not receive CHANNEL_EVENT", event);
				assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
				chnlEvent = (ReactorChannelEvent)event.reactorEvent();
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			}
			else	
			{
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			
			}
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			if (msgEvent.msg() != null)
				assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			// Consumer receives CHANNEL_READY event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());			
		}
		finally
		{
			consumerReactor.close();
			
			providerReactor.close();
		};
	}	

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoEPTest() {
		LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoEPTest(false, null);
	}

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoEPTest_WebSocket_Json() {
		LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoEPTest(true, "tr_json2");
	}

	private void LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoEPTest(boolean isWebsocket, String protocolList)
	{
		// 1 Group configured
		// 1st group is preferred
		// Connect with a normal Provider, then kill Group 1 and switch to connection list where Endpoint is configured
		// Normal connection to Endpoint test
		
		System.out.println("\n>>>>>>>>> Running LDPConnectWarmStandby_PreferredHostEnabled_WSBtoChannelList_NormaltoEPTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccountV2();
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		Provider provider = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(1);
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);

			int port1 = provider.bindGetPort(opts);

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.reactorOAuthCredential(ReactorFactory.createReactorOAuthCredential());
			Buffer buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("edpUserName"));
			consumerRole.reactorOAuthCredential().clientId(buf);
			buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("clientSecret"));
			consumerRole.reactorOAuthCredential().clientSecret(buf);
			byte[] jwkFile = null;
			try {
				jwkFile = Files.readAllBytes(Paths.get(System.getProperty("jwkFile")));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			String jwkText = new String(jwkFile);
			buf = CodecFactory.createBuffer();
			buf.data(jwkText);
			consumerRole.reactorOAuthCredential().clientJwk(buf);
			consumerRole.reactorOAuthCredential().reactorOAuthCredentialEventCallback(consumer);
			consumer.setReactorOAuthCredentialInfo(consumerRole.reactorOAuthCredential());

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			// Create WSB Groups
			List<Integer> wsbGroup1 = new ArrayList<Integer>();
			
			System.out.println(port1);
			wsbGroup1.add(port1);
			int channelPort = 2;

			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			//connectOpts.reactorPreferredHostOptions().detectionTimeInterval(10);
			connectOpts.reconnectAttemptLimit(1);
			
			connectOpts = consumer.testReactor().connectWsb_ByPort_SessionManagement_NoStart(opts, connectOpts, consumer,  consumer, protocolList, isWebsocket, dictionary, wsbGroup1, null, channelPort);

			consumer.testReactor().lateStartConnect(connectOpts, consumer, false);
			
			consumer.testReactor().dispatch(-1);
			
			provider.testReactor().accept(opts, provider, 5000);
			
			/* Provider receives channel-up/channel-ready */
			provider.testReactor().dispatch(2);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());
			
			consumer.testReactor().dispatch(-1);
			
			// check that connection info was not overwritten
			assertTrue(connectOpts.connectionList().size() == 1);
			assertTrue(connectOpts.reactorWarmStandbyGroupList().size() == 1);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == "localhost");	
			if (isWebsocket)
				assertTrue(connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS_WEBSOCKET);
			else
				assertTrue(connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS);
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				

			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			RDMLoginMsgEvent loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			LoginRequest loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

	        loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
	        loginRefresh.applyHasAttrib();
	        loginRefresh.applyHasFeatures();
	        loginRefresh.features().applyHasSupportOptimizedPauseResume();
	        loginRefresh.features().supportOptimizedPauseResume(1);
	        loginRefresh.features().applyHasSupportViewRequests();
	        loginRefresh.features().supportViewRequests(1);
	        loginRefresh.features().applyHasSupportPost();
	        loginRefresh.features().supportOMMPost(1);
	        loginRefresh.features().applyHasSupportStandby();
	        loginRefresh.features().supportStandby(1);
	        loginRefresh.features().applyHasSupportStandbyMode();
	        loginRefresh.features().supportStandbyMode(3);
	        loginRefresh.state().streamState(StreamStates.OPEN);
	        loginRefresh.state().dataState(DataStates.OK);
	        loginRefresh.state().code(StateCodes.NONE);
	        loginRefresh.state().text().data("Login OK");
	        
	        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 1 receives receives consumer connection status and directory request. */
			provider.testReactor().dispatch(2);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);


			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			DirectoryRequest directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			DirectoryRefresh directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

	        directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);

	        directoryRefresh.clear();
	        directoryRefresh.streamId(directoryRequest.streamId());
	        directoryRefresh.filter(directoryRequest.filter());
	        directoryRefresh.applySolicited();
	        directoryRefresh.applyClearCache();
	        directoryRefresh.state().streamState(StreamStates.OPEN);
	        directoryRefresh.state().dataState(DataStates.OK);
	        directoryRefresh.state().code(StateCodes.NONE);
	        directoryRefresh.state().text().data("Source Directory Refresh Complete");

	        Service service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		chnlEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());

			consumer.testReactor().dispatch(-1);
			
			// Kill Provider 1, switch to Channel List (Endpoint specified)
			provider.close();
			
			for (int j = 0; j < 16; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}

			// Consumer receives CHANNEL_DOWN_RECONNECTING event        
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			
			if(chnlEvent.eventType() != ReactorChannelEventTypes.FD_CHANGE)
				System.out.println("" + chnlEvent.toString());
			
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			


			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());

			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			

			// Verify new auth token
			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			
			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			if (chnlEvent.eventType() == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
			{
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());				

				// Consumer receives CHANNEL_UP event
				event = consumerReactor.pollEvent();
				assertNotNull("Did not receive CHANNEL_EVENT", event);
				assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
				chnlEvent = (ReactorChannelEvent)event.reactorEvent();
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			}
			else	
			{
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			
			}
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			if (msgEvent.msg() != null)
				assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());	
			
			// Consumer receives CHANNEL_READY event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());			
		}
		finally
		{
			consumerReactor.close();
			
			providerReactor.close();
		};
	}	

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_SDtoNormalTest() {
		LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_SDtoNormalTest(false, null);
	}

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_SDtoNormalTest_WebSocket_Json() {
		LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_SDtoNormalTest(true, "tr_json2");
	}

	private void LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_SDtoNormalTest(boolean isWebsocket, String protocolList)
	{
		// 1 Group configured
		// 1 Connection list configured
		// Group 1 is down, switch to Service Discovery on Connection list.
		// Group 1 is brought up, fallback method should switch to Group 1.
		
		System.out.println("\n>>>>>>>>> Running LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_SDtoNormalTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccountV2();
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		Provider provider = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(1);
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);

			int port1 = provider.bindGetPort(opts);

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.reactorOAuthCredential(ReactorFactory.createReactorOAuthCredential());
			Buffer buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("edpUserName"));
			consumerRole.reactorOAuthCredential().clientId(buf);
			buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("clientSecret"));
			consumerRole.reactorOAuthCredential().clientSecret(buf);
			byte[] jwkFile = null;
			try {
				jwkFile = Files.readAllBytes(Paths.get(System.getProperty("jwkFile")));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			String jwkText = new String(jwkFile);
			buf = CodecFactory.createBuffer();
			buf.data(jwkText);
			consumerRole.reactorOAuthCredential().clientJwk(buf);
			consumerRole.reactorOAuthCredential().reactorOAuthCredentialEventCallback(consumer);
			consumer.setReactorOAuthCredentialInfo(consumerRole.reactorOAuthCredential());

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			// Create WSB Groups
			List<Integer> wsbGroup1 = new ArrayList<Integer>();
			
			System.out.println(port1);
			wsbGroup1.add(port1);
			wsbGroup1.add(-1);
			int channelPort = 0;

			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			//connectOpts.reactorPreferredHostOptions().detectionTimeInterval(10);
			connectOpts.reconnectAttemptLimit(1);
			
			connectOpts = consumer.testReactor().connectWsb_ByPort_SessionManagement_NoStart(opts, connectOpts, consumer,  consumer, protocolList, isWebsocket, dictionary, wsbGroup1, null, channelPort);

			// Kill provider 1 to bring back up later
			provider.close();
			
			consumer.testReactor().lateStartConnect(connectOpts, consumer, false);
			
			consumer.testReactor().dispatch(-1);
			
			

			for (int j = 0; j < 9; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}	        
			
			// check that connection info was not overwritten
			assertTrue(connectOpts.connectionList().size() == 1);
			assertTrue(connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == null);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().size() == 1);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == "localhost");	
			
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				

			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());				

			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());				

			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			
			// Consumer receives extra CHANNEL_DOWN_RECONNECTING or CHANNEL_UP event (depending on timing)
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			if (chnlEvent.eventType() == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
			{
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());				

				// Consumer receives CHANNEL_UP event
				event = consumerReactor.pollEvent();
				assertNotNull("Did not receive CHANNEL_EVENT", event);
				assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
				chnlEvent = (ReactorChannelEvent)event.reactorEvent();
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			}
			else	
			{
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			
			}

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        

			// Start Provider 1
		    providerReactor.close();
		    providerReactor = new TestReactor(true);

			provider = createDefaultProvider(provider, providerReactor);
			provider.bind(opts, port1);
			
			// Call fallback
			consumer.reactorChannel().fallbackPreferredHost(errorInfo);
			
			consumer.testReactor().dispatch(-1);
			
			provider.testReactor().accept(opts, provider, 5000);
			
			/* Provider receives channel-up/channel-ready */
			provider.testReactor().dispatch(2);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());
			
			consumer.testReactor().dispatch(-1);
			
			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());				

			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			

			// Consumer receives Login Status
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			// Consumer receives Directory Update
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());		        

			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			// Consumer receives PREFERRED_HOST_COMPLETE event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			

			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			RDMLoginMsgEvent loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			LoginRequest loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

	        loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
	        loginRefresh.applyHasAttrib();
	        loginRefresh.applyHasFeatures();
	        loginRefresh.features().applyHasSupportOptimizedPauseResume();
	        loginRefresh.features().supportOptimizedPauseResume(1);
	        loginRefresh.features().applyHasSupportViewRequests();
	        loginRefresh.features().supportViewRequests(1);
	        loginRefresh.features().applyHasSupportPost();
	        loginRefresh.features().supportOMMPost(1);
	        loginRefresh.features().applyHasSupportStandby();
	        loginRefresh.features().supportStandby(1);
	        loginRefresh.features().applyHasSupportStandbyMode();
	        loginRefresh.features().supportStandbyMode(3);
	        loginRefresh.state().streamState(StreamStates.OPEN);
	        loginRefresh.state().dataState(DataStates.OK);
	        loginRefresh.state().code(StateCodes.NONE);
	        loginRefresh.state().text().data("Login OK");
	        
	        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 1 receives receives consumer connection status and directory request. */
			provider.testReactor().dispatch(2);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);


			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			DirectoryRequest directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			DirectoryRefresh directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

	        directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);

	        directoryRefresh.clear();
	        directoryRefresh.streamId(directoryRequest.streamId());
	        directoryRefresh.filter(directoryRequest.filter());
	        directoryRefresh.applySolicited();
	        directoryRefresh.applyClearCache();
	        directoryRefresh.state().streamState(StreamStates.OPEN);
	        directoryRefresh.state().dataState(DataStates.OK);
	        directoryRefresh.state().code(StateCodes.NONE);
	        directoryRefresh.state().text().data("Source Directory Refresh Complete");

	        Service service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(3);

	        /* Consumer receives directory update. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		chnlEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());

		}
		finally
		{
			consumerReactor.close();
			
			providerReactor.close();
		};
	}	

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_EPtoNormalTest() {
		LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_EPtoNormalTest(false, null);
	}

	@Test
	public void LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_EPtoNormalTest_WebSocket_Json() {
		LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_EPtoNormalTest(true, "tr_json2");
	}

	private void LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_EPtoNormalTest(boolean isWebsocket, String protocolList)
	{
		// 1 Group configured
		// 1 Connection list configured
		// Group 1 is down, switch to specified endpoint on Connection list.
		// Group 1 is brought up, fallback method should switch to Group 1.
		
		System.out.println("\n>>>>>>>>> Running LDPConnectWarmStandby_PreferredHostEnabled_ChannelListToWSB_EPtoNormalTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccountV2();
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		Provider provider = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(1);
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);

			int port1 = provider.bindGetPort(opts);

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.reactorOAuthCredential(ReactorFactory.createReactorOAuthCredential());
			Buffer buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("edpUserName"));
			consumerRole.reactorOAuthCredential().clientId(buf);
			buf = CodecFactory.createBuffer();
			buf.data(System.getProperty("clientSecret"));
			consumerRole.reactorOAuthCredential().clientSecret(buf);
			byte[] jwkFile = null;
			try {
				jwkFile = Files.readAllBytes(Paths.get(System.getProperty("jwkFile")));
			} catch (IOException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
			String jwkText = new String(jwkFile);
			buf = CodecFactory.createBuffer();
			buf.data(jwkText);
			consumerRole.reactorOAuthCredential().clientJwk(buf);
			consumerRole.reactorOAuthCredential().reactorOAuthCredentialEventCallback(consumer);
			consumer.setReactorOAuthCredentialInfo(consumerRole.reactorOAuthCredential());

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			// Create WSB Groups
			List<Integer> wsbGroup1 = new ArrayList<Integer>();
			
			System.out.println(port1);
			wsbGroup1.add(port1);
			wsbGroup1.add(-1);
			int channelPort = 2;

			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			//connectOpts.reactorPreferredHostOptions().detectionTimeInterval(10);
			connectOpts.reconnectAttemptLimit(1);
			
			connectOpts = consumer.testReactor().connectWsb_ByPort_SessionManagement_NoStart(opts, connectOpts, consumer,  consumer, protocolList, isWebsocket, dictionary, wsbGroup1, null, channelPort);

			// Kill provider 1 to bring back up later
			provider.close();
			
			consumer.testReactor().lateStartConnect(connectOpts, consumer, false);
			
			consumer.testReactor().dispatch(-1);

			for (int j = 0; j < 9; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}	        
			
			// check that connection info was not overwritten
			assertTrue(connectOpts.connectionList().size() == 1);
			if (isWebsocket)
				assertTrue(connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS_WEBSOCKET);	
			else
				assertTrue(connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == consumer.testReactor().LDP_ENDPOINT_ADDRESS);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().size() == 1);	
			assertTrue(connectOpts.reactorWarmStandbyGroupList().get(0).startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address() == "localhost");	
			
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				

			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());				

			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());				

			verifyAuthV2TokenEvent(consumerReactor, 15, true, true);
			
			// Consumer receives extra CHANNEL_DOWN_RECONNECTING or CHANNEL_UP event (depending on timing)
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			if (chnlEvent.eventType() == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
			{
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());				

				// Consumer receives CHANNEL_UP event
				event = consumerReactor.pollEvent();
				assertNotNull("Did not receive CHANNEL_EVENT", event);
				assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
				chnlEvent = (ReactorChannelEvent)event.reactorEvent();
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			}
			else	
			{
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			
			}

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass()); 

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        

			// Start Provider 1
		    providerReactor.close();
		    providerReactor = new TestReactor(true);

			provider = createDefaultProvider(provider, providerReactor);
			provider.bind(opts, port1);
			
			// Call fallback
			consumer.reactorChannel().fallbackPreferredHost(errorInfo);
			
			consumer.testReactor().dispatch(-1);
			
			provider.testReactor().accept(opts, provider, 5000);
			
			/* Provider receives channel-up/channel-ready */
			provider.testReactor().dispatch(2);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());
			
			consumer.testReactor().dispatch(-1);
			
			// Consumer receives PREFERRED_HOST_STARTING_FALLBACK event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK, chnlEvent.eventType());				

			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			

			// Consumer receives Login Status
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass()); 

			// Consumer receives Directory Update
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());		        

			// Consumer receives CHANNEL_UP event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());			

			// Consumer receives PREFERRED_HOST_COMPLETE event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, chnlEvent.eventType());			

			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			RDMLoginMsgEvent loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			LoginRequest loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

	        loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
	        loginRefresh.applyHasAttrib();
	        loginRefresh.applyHasFeatures();
	        loginRefresh.features().applyHasSupportOptimizedPauseResume();
	        loginRefresh.features().supportOptimizedPauseResume(1);
	        loginRefresh.features().applyHasSupportViewRequests();
	        loginRefresh.features().supportViewRequests(1);
	        loginRefresh.features().applyHasSupportPost();
	        loginRefresh.features().supportOMMPost(1);
	        loginRefresh.features().applyHasSupportStandby();
	        loginRefresh.features().supportStandby(1);
	        loginRefresh.features().applyHasSupportStandbyMode();
	        loginRefresh.features().supportStandbyMode(3);
	        loginRefresh.state().streamState(StreamStates.OPEN);
	        loginRefresh.state().dataState(DataStates.OK);
	        loginRefresh.state().code(StateCodes.NONE);
	        loginRefresh.state().text().data("Login OK");
	        
	        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 1 receives receives consumer connection status and directory request. */
			provider.testReactor().dispatch(2);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);


			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			DirectoryRequest directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			DirectoryRefresh directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

	        directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);

	        directoryRefresh.clear();
	        directoryRefresh.streamId(directoryRequest.streamId());
	        directoryRefresh.filter(directoryRequest.filter());
	        directoryRefresh.applySolicited();
	        directoryRefresh.applyClearCache();
	        directoryRefresh.state().streamState(StreamStates.OPEN);
	        directoryRefresh.state().dataState(DataStates.OK);
	        directoryRefresh.state().code(StateCodes.NONE);
	        directoryRefresh.state().text().data("Source Directory Refresh Complete");

	        Service service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(3);

	        /* Consumer receives directory update. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		chnlEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());

		}
		finally
		{
			consumerReactor.close();
			
			providerReactor.close();
		};
	}	

	@Test
	public void LDPConnectConnectionListErrorWrongCredentialsTest() {
		LDPConnectConnectionListErrorWrongCredentials(false, null);
	}

	@Test
	public void LDPConnectConnectionListErrorWrongCredentialsTest_WebSocket_Rwf() {
		LDPConnectConnectionListErrorWrongCredentials(true, "rssl.rwf");
	}

	@Test
	public void LDPConnectConnectionListErrorWrongCredentialsTest_WebSocket_Json() {
		LDPConnectConnectionListErrorWrongCredentials(true, "tr_json2");
	}

	private  void LDPConnectConnectionListErrorWrongCredentials(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectConnectionListErrorWrongCredentialsTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());

		// request service discovery with valid user name / password to prevent being locked out 
		// from Delivery Platform because of too many invalid requests.
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorChannelEvent chnlEvent;			

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();	

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */

			// create first entry in the connection list that will fail
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			connectInfo.connectOptions().connectionType(ConnectionTypes.SOCKET);			
			connectInfo.connectOptions().majorVersion(Codec.majorVersion());
			connectInfo.connectOptions().minorVersion(Codec.minorVersion());
			connectInfo.connectOptions().unifiedNetworkInfo().address("localhost");
			connectInfo.connectOptions().unifiedNetworkInfo().serviceName("14002");
			connectInfo.connectOptions().userSpecObject(consumer);		
			connectInfo.initTimeout(40);
			connectInfo.enableSessionManagement(false);		

			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);

			ReactorConnectInfo connectInfoSecond = rcOpts.connectionList().remove(0);		

			rcOpts.connectionList().add(connectInfo);
			rcOpts.connectionList().add(connectInfoSecond);
			rcOpts.connectionList().get(1).reactorAuthTokenEventCallback(consumer);
			consumerRole.rdmLoginRequest().userName().data("FAKE");
			consumerRole.rdmLoginRequest().password().data("FAKE");
			
			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);

			for (int j = 0; j < 5; j++)
			{
				try {
					Thread.sleep(800);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				consumer.testReactor().dispatch(-1);
			}	        

			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address().equals("localhost"));
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName().equals("14002"));			
			assertTrue(rcOpts.connectionList().get(1).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(1).connectOptions().unifiedNetworkInfo().serviceName() == null);

			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());				


			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());			

			for (int i = 0; i < 1; i++)
			{
				System.out.println(i);
				verifyAuthTokenEvent(consumerReactor, 30, false, true);

				for (int j = 0; j < 12; j++)
				{
					try {
						Thread.sleep(1000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}
					consumer.testReactor().dispatch(-1);
				}
				
				event = consumerReactor.pollEvent();
				assertNotNull("Did not receive CHANNEL_EVENT", event);
				assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
				chnlEvent = (ReactorChannelEvent)event.reactorEvent();
				assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());
				assertTrue(chnlEvent.errorInfo().error().text().contains("Connection refused"));

				event = consumerReactor.pollEvent();
				assertNotNull("Did not receive CHANNEL_EVENT", event);
				assertEquals("Expected TestReactorEventTypes.AUTH_TOKEN_EVENT, received: " + event.type(), TestReactorEventTypes.AUTH_TOKEN_EVENT, event.type());
				ReactorAuthTokenEvent authTokenEvent = (ReactorAuthTokenEvent)event.reactorEvent();
				assertTrue(authTokenEvent.errorInfo().error().text().contains("{\"error_description\":\"Invalid username or password.\",\"error\":\"access_denied\"}"));
			}

		}
		finally
		{
			consumerReactor.close();
		}
	}	


	@Test
	public void LDPConnectConnectionListSecondConnectionDPInvalidCredentialsTest() {
		LDPConnectConnectionListSecondConnectionDPInvalidCredentials(false, null);
	}

	@Test
	public void LDPConnectConnectionListSecondConnectionDPInvalidCredentialsTest_WebSocket_Rwf() {
		LDPConnectConnectionListSecondConnectionDPInvalidCredentials(true, "rssl.rwf");
	}

	@Test
	public void LDPConnectConnectionListSecondConnectionDPInvalidCredentialsTest_WebSocket_Json() {
		LDPConnectConnectionListSecondConnectionDPInvalidCredentials(true, "tr_json2");
	}

	private void LDPConnectConnectionListSecondConnectionDPInvalidCredentials(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectConnectionListSecondConnectionDPInvalidCredentialsTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());

		// request service discovery with valid user name / password to prevent being locked out 
		// from Delivery Platform because of too many invalid requests.
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorChannelEvent chnlEvent;
			
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
       
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			
			consumerRole.rdmLoginRequest().password().data("FAKE");
			consumerRole.rdmLoginRequest().userName().data("FAKE");
			
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */

			// create first entry in the connection list that will fail
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			assertEquals(ReactorReturnCodes.PARAMETER_OUT_OF_RANGE, connectInfo.initTimeout(0));
			assertEquals(ReactorReturnCodes.SUCCESS, connectInfo.initTimeout(10));
			connectInfo.connectOptions().connectionType(ConnectionTypes.SOCKET);			
			connectInfo.connectOptions().majorVersion(Codec.majorVersion());
			connectInfo.connectOptions().minorVersion(Codec.minorVersion());
			connectInfo.connectOptions().unifiedNetworkInfo().address("localhost");
			connectInfo.connectOptions().unifiedNetworkInfo().serviceName("14002");
			connectInfo.connectOptions().userSpecObject(consumer);		
			connectInfo.initTimeout(40);
			connectInfo.enableSessionManagement(false);		

			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);

			ReactorConnectInfo connectInfoSecond = rcOpts.connectionList().remove(0);		

			rcOpts.connectionList().add(connectInfo);
			rcOpts.connectionList().add(connectInfoSecond);

			rcOpts.connectionList().get(1).reactorAuthTokenEventCallback(consumer);					

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address().equals("localhost"));
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName().equals("14002"));
			assertTrue(rcOpts.connectionList().get(1).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(1).connectOptions().unifiedNetworkInfo().serviceName() == null);

			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());

			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = getTestEvent(consumerReactor, 10);
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());
			assertTrue(chnlEvent.errorInfo().error().text().contains("Connection refused"));

			/* Authentication event for the second connection */
			event = getTestEvent(consumerReactor, 10);
			assertNotNull("Did not receive AUTH_TOKEN_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.AUTH_TOKEN_EVENT, received: " + event.type(), TestReactorEventTypes.AUTH_TOKEN_EVENT, event.type());
			ReactorAuthTokenEvent authTokenEvent = (ReactorAuthTokenEvent)event.reactorEvent();
			assertTrue(authTokenEvent.errorInfo().error().text().contains("\"error_description\":\"Invalid username or password.\""));
			
			
			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = getTestEvent(consumerReactor, 10);
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());
			assertTrue(chnlEvent.errorInfo().error().text().contains("Failed REST request for the token service from HTTP status code 400"));
			
			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = getTestEvent(consumerReactor, 10);
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());
			assertTrue(chnlEvent.errorInfo().error().text().contains("Connection refused"));
			
			event = getTestEvent(consumerReactor, 10);
			assertNotNull("Did not receive AUTH_TOKEN_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.AUTH_TOKEN_EVENT, received: " + event.type(), TestReactorEventTypes.AUTH_TOKEN_EVENT, event.type());
			authTokenEvent = (ReactorAuthTokenEvent)event.reactorEvent();
			assertTrue(authTokenEvent.errorInfo().error().text().contains("\"error_description\":\"Invalid username or password.\""));
		}
		finally
		{

			try {
				Thread.sleep(20 * 1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}		

			consumerReactor.close();
		}		
	}	

	@Test
	public void LDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlagTest() {
		LDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlag(false, null);
	}

	@Test
	public void LDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlagTest_WebSocket_Json() {
		LDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlag(true, "tr_json2");
	}

	private void LDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlag(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlagTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		

			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();	

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);					

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);				

			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName() == null);				
			
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());			


			verifyAuthTokenEvent(consumerReactor, 10, true, true);

			try {
				Thread.sleep(2 * 1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}

			consumer.testReactor().dispatch(4, 8000);	        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        

			// Checks for the first login reissue response from the worker thread
			verifyAuthTokenEvent(consumerReactor, 480, true, true);

		}
		finally
		{
			consumerReactor.close();
		}		
	}	

	@Test
	public void LDPConnectWatchlistDisabledAddressAndPortEmptyTest() {
		LDPConnectWatchlistDisabledAddressAndPortEmpty(false, null);
	}

	@Test
	public void LDPConnectWatchlistDisabledAddressAndPortEmptyTest_WebSocket_Rwf() {
		LDPConnectWatchlistDisabledAddressAndPortEmpty(true, "rssl.rwf");
	}

	@Test
	public void LDPConnectWatchlistDisabledAddressAndPortEmptyTest_WebSocket_Json() {
		LDPConnectWatchlistDisabledAddressAndPortEmpty(true, "tr_json2");
	}

	private void LDPConnectWatchlistDisabledAddressAndPortEmpty(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectWatchlistDisabledAddressAndPortEmptyTest <<<<<<<<<<\n");
		/* Test a to see if address and service name will be overwritten by service discovery when user sets them to 
		 * empty strings prior to connection */
		assumeTrue(checkCredentials());

		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;	

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();	

			setupConsumer(consumer, true);
			consumerRole.watchlistOptions().enableWatchlist(false);			

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address("");
			rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName("");			

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);

			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address().equals(""));
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName().equals(""));	
		}
		finally
		{
			consumerReactor.close();
		}		
	}	

	@Test
	public void LDPConnectWatchlistDisabledAddressAndPortSetTest() {
		LDPConnectWatchlistDisabledAddressAndPortSet(false, null);
	}

	@Test
	public void LDPConnectWatchlistDisabledAddressAndPortSetTest_WebSocket_Rwf() {
		LDPConnectWatchlistDisabledAddressAndPortSet(true, "rssl.rwf");
	}

	@Test
	public void LDPConnectWatchlistDisabledAddressAndPortSetTest_WebSocket_Json() {
		LDPConnectWatchlistDisabledAddressAndPortSet(true, "tr_json2");
	}

	private void LDPConnectWatchlistDisabledAddressAndPortSet(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectWatchlistDisabledAddressAndPortSetTest <<<<<<<<<<\n");
		/* Test a to see if address and service name will NOT be overwritten by service discovery when user sets them to 
		 * some string */
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {


			ReactorErrorInfo errorInfo = null;	

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();	

			setupConsumer(consumer, true);
			consumerRole.watchlistOptions().enableWatchlist(false);			

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address("FAKE");
			rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName("FAKE");

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);

			// check if the connection info came from LDP
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address().equals("FAKE"));
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName().equals("FAKE"));
			
		}
		finally
		{
			consumerReactor.close();
		}	
	}	

	@Test
	public void LDPConnectWatchlistDisabledTest() {
		LDPConnectWatchlistDisabled(false, null);
	}

	@Test
	public void LDPConnectWatchlistDisabledTest_WebSocket_json() {
		LDPConnectWatchlistDisabled(true, "tr_json2");
	}

	private void LDPConnectWatchlistDisabled(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectWatchlistDisabledTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			

			/* Create reactor. */
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();	

			setupConsumer(consumer, true);
			consumerRole.watchlistOptions().enableWatchlist(false);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);					

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			
			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName() == null);		

			try {
				Thread.sleep(2 * 1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}

			int sleep = verifyAuthTokenEvent(consumerReactor, 10, true, true);	
			long runtime = System.currentTimeMillis() + ((sleep - 3) * 1000);

			consumer.testReactor().dispatch(-1, 8000);		

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());	        

			verifyAuthTokenRequestAndLoginReissue(consumerReactor, consumer, 2, sleep, runtime, false, false);

		}
		finally
		{
			consumerReactor.close();
		}

	}

	@Test
	public void LDPConnectWatchlistDisabledConnectionListTest() {
		LDPConnectWatchlistDisabledConnectionList(false, null);
	}

	@Test
	public void LDPConnectWatchlistDisabledConnectionListTest_WebSocket_json() {
		LDPConnectWatchlistDisabledConnectionList(true, "tr_json2");
	}

	private void LDPConnectWatchlistDisabledConnectionList(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectWatchlistDisabledConnectionListTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();	
		
		TestReactor consumerReactor = null;
		try {		
	

			ReactorErrorInfo errorInfo = null;		 

			TestReactorEvent event;
			ReactorMsgEvent msgEvent;			
			ReactorChannelEvent chnlEvent;			

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();	

			setupConsumer(consumer, true);
			consumerRole.watchlistOptions().enableWatchlist(false);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			// create first entry in the connection list that will fail
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			assertEquals(ReactorReturnCodes.SUCCESS, connectInfo.initTimeout(10));
			connectInfo.connectOptions().connectionType(ConnectionTypes.SOCKET);			
			connectInfo.connectOptions().majorVersion(Codec.majorVersion());
			connectInfo.connectOptions().minorVersion(Codec.minorVersion());
			connectInfo.connectOptions().unifiedNetworkInfo().address("localhost");
			connectInfo.connectOptions().unifiedNetworkInfo().serviceName("14002");
			connectInfo.connectOptions().userSpecObject(consumer);		
			connectInfo.initTimeout(60);
			connectInfo.enableSessionManagement(false);		
			setupProxyForConnectOptions(connectInfo.connectOptions());

			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);

			ReactorConnectInfo connectInfoSecond = rcOpts.connectionList().remove(0);		

			rcOpts.connectionList().add(connectInfo);
			rcOpts.connectionList().add(connectInfoSecond);

			rcOpts.connectionList().get(1).reactorAuthTokenEventCallback(consumer);				

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			
			consumer.testReactor().dispatch(-1, 16000);

			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address().equals("localhost"));
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName().equals("14002"));			
			assertTrue(rcOpts.connectionList().get(1).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(1).connectOptions().unifiedNetworkInfo().serviceName() == null);

			
			// Consumer receives CHANNEL_DOWN_RECONNECTING event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());

			try {
				Thread.sleep(10 * 1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}		

			int sleep = verifyAuthTokenEvent(consumerReactor, 15, true, true);
			long runtime = System.currentTimeMillis() + ((sleep - 5) * 1000);		        


			try {
				Thread.sleep(15 * 1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());	        


			try {
				Thread.sleep(15 * 1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}			


			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive LOGIN_MSG", event);
			assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive DIRECTORY_MSG", event);
			assertEquals("Expected TestReactorEventTypes.DIRECTORY_MSG, received: " + event.type(), TestReactorEventTypes.DIRECTORY_MSG, event.type());
			msgEvent = (ReactorMsgEvent)event.reactorEvent();
			assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());		        

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_READY, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_READY, chnlEvent.eventType());

			verifyAuthTokenRequestAndLoginReissue(consumerReactor, consumer, 2, sleep, runtime, false, false);
		}
		finally
		{
			consumerReactor.close();
		};		
	}	

	@Test
	public void LDPConnectWatchlistDisabledErrorNoRDMLoginTest() {
		LDPConnectWatchlistDisabledErrorNoRDMLogin(false, null);
	}

	@Test
	public void LDPConnectWatchlistDisabledErrorNoRDMLoginTest_WebSocket_Json() {
		LDPConnectWatchlistDisabledErrorNoRDMLogin(true, "rssl.rwf");
	}

	private void LDPConnectWatchlistDisabledErrorNoRDMLogin(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> LRunning LDPConnectWatchlistDisabledErrorNoRDMLoginTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		

			/* Create reactor. */
			//TestReactor.enableReactorXmlTracing();	        
			consumerReactor = new TestReactor();

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();	

			setupConsumer(consumer, false);
			consumerRole.watchlistOptions().enableWatchlist(false);			

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);					

			// since no RDMLoginRequest there is no user name or password, authentication will fail
			assertTrue(consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.INVALID_USAGE);		
		}
		finally
		{
			consumerReactor.close();
		};		
	}		

	private void verifyAuthTokenRequestAndLoginReissue(TestReactor consumerReactor, Consumer consumer, 
			int numberOfSequences, int sleep, long runtime, boolean loginReissue, boolean refreshFlag)
	{
		for (int i = 0; i < numberOfSequences; i++)
		{
			System.out.println("Sequence number: " + i);
			System.out.print("SLEEP: ");
			// Wait for the token management event
			// to keep the connection alive needs to dispatch every second
			// to send the ping, can't time the 
			for (int j = 0; j < sleep; j++)
			{
				try {
					Thread.sleep(1000);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}

				if (System.currentTimeMillis() > runtime)
					break;
				System.out.print(" " + j);
				consumer.testReactor().dispatch(-1);
			}

			System.out.println("");
			runtime += ((sleep - 3)* 1000);

			if (loginReissue)
			{
				verifyAuthTokenEvent(consumerReactor, 10, true, true);				
			}
			else // no watchlist, so we need to send the login reissue on behalf of the consumer
			{
				//				verifyAuthTokenEvent(consumerReactor, 10, true);				

				ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
				assertNotNull(errorInfo);

				// submit login request and make sure it succeeds
				ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
				LoginRequest loginRequest = consumerRole.rdmLoginRequest();
				loginRequest.userNameType(Login.UserIdTypes.AUTHN_TOKEN);
				loginRequest.userName().data(_tokenInfo.accessToken());
				if (!refreshFlag)
					loginRequest.applyNoRefresh();
				assertNotNull(loginRequest);

				ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
				submitOptions.clear();
				loginRequest.flags(loginRequest.flags() & ~LoginRequestFlags.HAS_PASSWORD);
				loginRequest.applyNoRefresh();
				assertEquals(ReactorReturnCodes.SUCCESS, consumer.reactorChannel().submit(loginRequest, submitOptions, errorInfo));	

				if (refreshFlag)
				{
					try {
						Thread.sleep(2000);
					} catch (InterruptedException e) {
						e.printStackTrace();
					}

					TestReactorEvent event;
					ReactorMsgEvent msgEvent;	            	
					event = consumerReactor.pollEvent();
					assertEquals("Expected TestReactorEventTypes.LOGIN_MSG, received: " + event.type(), TestReactorEventTypes.LOGIN_MSG, event.type());
					msgEvent = (ReactorMsgEvent)event.reactorEvent();
					assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());  	            	
				}
			}
		}
	}
	
	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectUserSpecifiedClientIdBySpecifyingHostAndPortTest(){
		LDPConnectUserSpecifiedClientIdBySpecifyingHostAndPort(false, null);
	}

	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectUserSpecifiedClientIdBySpecifyingHostAndPortTest_WebSocket_Rwf(){
		LDPConnectUserSpecifiedClientIdBySpecifyingHostAndPort(true, "rssl.rwf");
	}

	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectUserSpecifiedClientIdBySpecifyingHostAndPortTest_WebSocket_Json(){
		LDPConnectUserSpecifiedClientIdBySpecifyingHostAndPort(true, "tr_json2");
	}

	@SuppressWarnings("deprecation")
	private void LDPConnectUserSpecifiedClientIdBySpecifyingHostAndPort(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectUserSpecifiedClientIdBySpecifyingHostAndPortTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;

			/* Create reactor. */
			consumerReactor = new TestReactor();
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			consumerRole.clientId().data(System.getProperty("edpUserName"));

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);
			
			/* Get an endpoint from the service discovery */
			
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpPassword"));
				reactorServiceDiscoveryOptions.password(buf);
			}  	  	
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.clientId(buf);
			}

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{				
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					
					for (ReactorServiceEndpointInfo info : event.serviceEndpointInfo())
					{
						if( info.locationList().toString().contains("us-east-1") )
						{
							host = info.endPoint();
							port = info.port();
							break;
						}
					}
					
					return 0;
				}     				
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);
			assertTrue(consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			
			assertTrue(!callback.host.isEmpty() && !callback.port.isEmpty());

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			
			rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address(callback.host);
			rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(callback.port);			
			
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);

			TestReactorEvent event = null;
			ReactorChannelEvent chnlEvent;		
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());

			verifyAuthTokenEvent(consumerReactor, 10, true, true);

		}
		finally
		{
			consumerReactor.close();
		}	
	}		

	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectUserSpecifiedClientIdTest() {
		LDPConnectUserSpecifiedClientId(false, null);
	}

	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectUserSpecifiedClientIdTest_Websocket_Rwf() {
		LDPConnectUserSpecifiedClientId(true, "rssl.rwf");
	}

	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectUserSpecifiedClientIdTest_Websocket_Json() {
		LDPConnectUserSpecifiedClientId(true, "tr_json2");
	}

	private void LDPConnectUserSpecifiedClientId(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectUserSpecifiedClientIdTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;

			/* Create reactor. */
			consumerReactor = new TestReactor();
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			consumerRole.clientId().data(System.getProperty("edpUserName"));

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);
			
			//assertTrue("Expected Service Discovery request going out", consumerReactor._reactor._restClient.endpoint() != null);
			

			TestReactorEvent event = null;
			ReactorChannelEvent chnlEvent;		
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());

			verifyAuthTokenEvent(consumerReactor, 10, true, true);

		}
		finally
		{
			consumerReactor.close();
		}	
	}	

	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectErrorInvalidClientIdTest() {
		LDPConnectErrorInvalidClientId(false, null);
	}

	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectErrorInvalidClientIdTest_WebSocket_Rwf() {
		LDPConnectErrorInvalidClientId(true, "rssl.rwf");
	}

	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectErrorInvalidClientIdTest_WebSocket_Json() {
		LDPConnectErrorInvalidClientId(true, "tr_json2");
	}

	@SuppressWarnings("deprecation")
	private void LDPConnectErrorInvalidClientId(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectErrorInvalidClientIdTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;	

			/* Create reactor. */
			consumerReactor = new TestReactor();
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			consumerRole.clientId().data("FAKE");

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			assertTrue(consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.FAILURE);		
			// unauthorized
			assertTrue(errorInfo.toString().contains("{\"error\":\"invalid_client\"  ,\"error_description\":\"Invalid Application Credential.\" }"));

			// run it again with correct credentials.
			consumerRole.clientId().data(System.getProperty("edpUserName"));

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);			

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);			

			TestReactorEvent event = null;
			ReactorChannelEvent chnlEvent = null;		
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());			

			verifyAuthTokenEvent(consumerReactor, 10, true, true);

		}
		finally
		{
			consumerReactor.close();
		}
	}		

	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectErrorIncorrectCredentialsTest() {
		LDPConnectErrorIncorrectCredentials(false, null);
	}

	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectErrorIncorrectCredentialsTest_WebSocket_Rwf() {
		LDPConnectErrorIncorrectCredentials(true, "rssl.rwf");
	}

	@SuppressWarnings("deprecation")
	@Test
	public void LDPConnectErrorIncorrectCredentialsTest_WebSocket_Json() {
		LDPConnectErrorIncorrectCredentials(true, "tr_json2");
	}

	@SuppressWarnings("deprecation")
	private void LDPConnectErrorIncorrectCredentials(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectErrorIncorrectCredentialsTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());

		// request service discovery with valid user name / password to prevent being locked out 
		// from Delivery Platform because of too many invalid requests.
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		
		

			ReactorErrorInfo errorInfo = null;		

			/* Create reactor. */
			consumerReactor = new TestReactor();
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			consumerRole.rdmLoginRequest().userName().data("FAKE");
			consumerRole.rdmLoginRequest().password().data("FAKE");
			consumerRole.clientId().data("FAKE");
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			assertTrue(consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.FAILURE);
			// unauthorized
			assertTrue(errorInfo.toString().contains("{\"error\":\"invalid_client\"  ,\"error_description\":\"Invalid Application Credential.\" }"));

			// run it again with correct credentials.
			consumerRole.rdmLoginRequest().userName().data(System.getProperty("edpUserName"));
			consumerRole.rdmLoginRequest().password().data(System.getProperty("edpPassword"));
			consumerRole.clientId().data(System.getProperty("edpUserName"));
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);			

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);

			TestReactorEvent event = null;
			ReactorChannelEvent chnlEvent = null;
			// Consumer receives CHANNEL_OPENED event
			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_OPENED, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());			

			verifyAuthTokenEvent(consumerReactor, 10, true, true);	

		}
		finally
		{
			consumerReactor.close();
		}		
	}		 

	@Test
	public void LDPConnectErrorInvalidTokenServiceURLTest() {
		LDPConnectErrorInvalidTokenServiceURL(false, null);
	}

	@Test
	public void LDPConnectErrorInvalidTokenServiceURLTest_WebSocket_Rwf() {
		LDPConnectErrorInvalidTokenServiceURL(true, "rssl.rwf");
	}

	@Test
	public void LDPConnectErrorInvalidTokenServiceURLTest_WebSocket_Json() {
		LDPConnectErrorInvalidTokenServiceURL(true, "tr_json2");
	}

	private void LDPConnectErrorInvalidTokenServiceURL(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectErrorInvalidTokenServiceURLTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;

			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();

			Buffer url = CodecFactory.createBuffer();
			url.data("http://invalidhost/auth/oauth2/beta1/token");
			reactorOptions.tokenServiceURL(url);

			/* Create reactor. */
			consumerReactor = new TestReactor(reactorOptions);
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;	  

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			assertTrue(consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.FAILURE);	

		}
		finally
		{
			consumerReactor.close();
		}
	}		

	@Test
	public void LDPConnectUserSpecifiedTokenServiceUrlTest() {
		LDPConnectUserSpecifiedTokenServiceUrl(false, null);
	}

	@Test
	public void LDPConnectUserSpecifiedTokenServiceUrlTest_WebSocket_Rwf() {
		LDPConnectUserSpecifiedTokenServiceUrl(true, "rssl.rwf");
	}

	@Test
	public void LDPConnectUserSpecifiedTokenServiceUrlTest_WebSocket_Json() {
		LDPConnectUserSpecifiedTokenServiceUrl(true, "tr_json2");
	}

	private void LDPConnectUserSpecifiedTokenServiceUrl(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectUserSpecifiedTokenServiceUrlTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;

			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();

			Buffer url = CodecFactory.createBuffer();
			url.data("https://api.refinitiv.com/auth/oauth2/v1/token");
			reactorOptions.tokenServiceURL(url);

			/* Create reactor. */
			consumerReactor = new TestReactor(reactorOptions);
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;	  

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);			

		}
		finally
		{
			consumerReactor.close();
		}		
	}	

	@Test
	public void LDPConnectUserSpecifiedServiceDiscoveryUrlTest() {
		LDPConnectUserSpecifiedServiceDiscoveryUrl(false, null);
	}

	@Test
	public void LDPConnectUserSpecifiedServiceDiscoveryUrlTest_WebSocket_Rwf() {
		LDPConnectUserSpecifiedServiceDiscoveryUrl(true, "rssl.rwf");
	}

	@Test
	public void LDPConnectUserSpecifiedServiceDiscoveryUrlTest_WebSocket_Json() {
		LDPConnectUserSpecifiedServiceDiscoveryUrl(true, "tr_json2");
	}

	private void LDPConnectUserSpecifiedServiceDiscoveryUrl(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectUserSpecifiedServiceDiscoveryUrlTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();		
		TestReactor consumerReactor = null;
		try {


			ReactorErrorInfo errorInfo = null;

			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();

			Buffer url = CodecFactory.createBuffer();
			url.data("https://api.refinitiv.com/streaming/pricing/v1/");
			reactorOptions.serviceDiscoveryURL(url);

			/* Create reactor. */
			consumerReactor = new TestReactor(reactorOptions);
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;	  

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			setupConsumer(consumer, true);
			
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);			

		}
		finally
		{
			consumerReactor.close();
		};
	}	

	@Test
	public void LDPConnectErrorInvalidServiceDiscoveryURLTest() {
		LDPConnectErrorInvalidServiceDiscoveryURL(false, null);
	}

	@Test
	public void LDPConnectErrorInvalidServiceDiscoveryURLTest_WebSocket_Rwf() {
		LDPConnectErrorInvalidServiceDiscoveryURL(true, "rssl.rwf");
	}

	@Test
	public void LDPConnectErrorInvalidServiceDiscoveryURLTest_WebSocket_Json() {
		LDPConnectErrorInvalidServiceDiscoveryURL(true, "tr_json2");
	}

	private void LDPConnectErrorInvalidServiceDiscoveryURL(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectErrorInvalidServiceDiscoveryURLTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {


			ReactorErrorInfo errorInfo = null;

			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();

			Buffer url = CodecFactory.createBuffer();
			url.data("http://invalidhost/straming/pricing/v1/");
			reactorOptions.serviceDiscoveryURL(url);

			/* Create reactor. */
			consumerReactor = new TestReactor(reactorOptions);
			ReactorCallbackHandler callbackHandler = null;
			Selector selector = null;	  

			/* Create consumer. */
			callbackHandler = new ReactorCallbackHandler(selector);
			assertEquals(null, callbackHandler.lastChannelEvent());

			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			assertTrue(consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.FAILURE);	

		}
		finally
		{
			consumerReactor.close();
		}	
	}			

	@Test
	public void LDPQueryServiceDiscoveryErrorInvalidDataFormatAndTransportTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryErrorInvalidDataFormatAndTransportTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {


			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpPassword"));
				reactorServiceDiscoveryOptions.password(buf);
			}  	  	
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.clientId(buf);
			}

			reactorServiceDiscoveryOptions.transport(11111);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					assertTrue(event.errorInfo().code() == ReactorReturnCodes.PARAMETER_OUT_OF_RANGE );
					System.out.println(event.serviceEndpointInfo());					
					_count++;
					return 0;
				}     				
			};

			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);
			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);

			/* Create reactor. */
			consumerReactor = new TestReactor();


			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			assertTrue(consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.PARAMETER_OUT_OF_RANGE);
			// verify that callback was NOT called
			assertTrue("callback was called", callback._count == 0);
			assertTrue(errorInfo.code() == ReactorReturnCodes.PARAMETER_OUT_OF_RANGE);

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			reactorServiceDiscoveryOptions.dataFormat(23232);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  	     

			assertTrue(consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.PARAMETER_OUT_OF_RANGE);
			// verify that callback was NOT called
			assertTrue("callback was called", callback._count == 0);	
			assertTrue(errorInfo.code() == ReactorReturnCodes.PARAMETER_OUT_OF_RANGE);			
		}
		finally
		{
			consumerReactor.close();
		}
	}		

	@Test
	public void LDPQueryServiceDiscoveryErrorCallbackMissingTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryErrorCallbackMissingTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);		    

			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);
			assertTrue(consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.PARAMETER_INVALID);

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  

			assertTrue(consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.PARAMETER_INVALID);			    

		}
		finally
		{
			consumerReactor.close();
		}	
	}


	@Test
	public void LDPQueryServiceDiscoveryErrorErrorInfoMissingTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryErrorErrorInfoMissingTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		

			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);

			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);
			assertTrue(consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.PARAMETER_INVALID);

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  

			assertTrue(consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.PARAMETER_INVALID);			    

		}
		finally
		{
			consumerReactor.close();
		}		
	}	

	@Test
	public void LDPQueryServiceDiscoveryErrorServiceDiscoveryOptionsMissingTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryErrorServiceDiscoveryOptionsMissingTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {		


			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = null;

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);
			assertTrue(consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.PARAMETER_INVALID);

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  

			assertTrue(consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.PARAMETER_INVALID);	    

		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPQueryServiceDiscoveryMissingClientIdTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryMissingClientIdTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {
			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpPassword"));
				reactorServiceDiscoveryOptions.password(buf);
			}

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					return 0;
				}     				
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);		
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();   
			
			System.out.println("Client ID: " + reactorServiceDiscoveryOptions.clientId());
			
			int ret = consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo);
			
			assertTrue("Expecting ReactorReturnCodes.PARAMETER_INVALID (-5), instead received: " + ret, ret == ReactorReturnCodes.PARAMETER_INVALID);	
			assertTrue(errorInfo.toString().contains("Required parameter clientId is not set"));
		}
		finally
		{
			consumerReactor.close();
		}	
	}

	@Test
	public void LDPQueryServiceDiscoveryErrorMismatchOfTypesTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryErrorMismatchOfTypesTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {

			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpPassword"));
				reactorServiceDiscoveryOptions.password(buf);
			}    	
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.clientId(buf);
			}	

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					System.out.println(event.serviceEndpointInfo());
					System.out.println(event.errorInfo());					
					assertTrue(event.errorInfo().code() == ReactorReturnCodes.FAILURE );
					// since there is a mismatch of the dataformat and protocol the LDP will respond with 404 error Not found.
					assertTrue(event.errorInfo().toString().contains("404"));
					_count++;
					return 0;
				}     				
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called once
			assertTrue("callback was not called", callback._count == 1);

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called twice
			assertTrue("callback was not called", callback._count == 2);

		}
		finally
		{
			consumerReactor.close();
		}
	}	

	@Test
	public void LDPQueryServiceDiscoveryJSONTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryJSONTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {


			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpPassword"));
				reactorServiceDiscoveryOptions.password(buf);
			}  	  	
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.clientId(buf);
			}

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_WEBSOCKET);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_JSON2);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					assertTrue(event.errorInfo().code() == ReactorReturnCodes.SUCCESS );
					assertTrue(event.serviceEndpointInfo().size() > 0);

					for (int i = 0; i < event.serviceEndpointInfo().size(); i++)
					{
						assertTrue(event.serviceEndpointInfo().get(i).transport().equals("websocket"));
						assertTrue(event.serviceEndpointInfo().get(i).dataFormatList().get(0).equals("tr_json2"));
					}
					System.out.println(event.serviceEndpointInfo());					
					_count++;
					return 0;
				}     				
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called once
			assertTrue("callback was not called", callback._count == 1);

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called twice
			assertTrue("callback was not called", callback._count == 2);			

		}
		finally
		{
			consumerReactor.close();
		}	
	}

	@Test
	public void LDPQueryServiceDiscoveryErrorPasswordTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryErrorPasswordTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {
	

			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data("WRONG_USER_NAME");
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data("WRONG_PASSWORD");
				reactorServiceDiscoveryOptions.password(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data("WRONG_CLIENT_ID");
				reactorServiceDiscoveryOptions.clientId(buf);
			}  

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					assertTrue(event.errorInfo().code() == ReactorReturnCodes.FAILURE );
					// since user name and password is incorrect we should see the error message
					assertTrue(event.errorInfo().toString().contains("{\"error\":\"invalid_client\"  ,\"error_description\":\"Invalid Application Credential.\" }"));
					_count++;
					return 0;
				}     				
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called once
			assertTrue("callback was not called", callback._count == 1);

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);    
			// verify that callback was called twice
			assertTrue("callback was not called", callback._count == 2);

		}
		finally
		{
			consumerReactor.close();
		}	
	}

	@Test
	public void LDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnectTest() {
		LDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnect(false, null);
	}

	@Test
	public void LDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnectTest_WebSocket_Rwf() {
		LDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnect(true, "rssl.rwf");
	}

	@Test
	public void LDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnectTest_WebSocket_Json() {
		LDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnect(true, "tr_json2");
	}

	private void LDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnect(boolean isWebsocket, String protocol)
	{
		
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryErrorPasswordAndSuccessConnectTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {
		

			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data("WRONG USER NAME");
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data("WRONG PASSWORD");
				reactorServiceDiscoveryOptions.password(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data("WRONG CLIENT ID");
				reactorServiceDiscoveryOptions.clientId(buf);
			}  

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					assertTrue(event.errorInfo().code() == ReactorReturnCodes.FAILURE );
					// since password is incorrect we should see 400 code
					String tmp = "{\"error\":\"invalid_client\"";
					assertTrue("Message received: " + event.errorInfo().toString() + " Expected: " + tmp, event.errorInfo().toString().contains(tmp));
					//assertTrue("Message received: " + event.errorInfo().toString() ,event.errorInfo().toString().contains("{\"error\":\"access_denied\"  ,\"error_description\":\"Authentication Failed.\" }"));
					System.out.println(event.serviceEndpointInfo());
					System.out.println(event.errorInfo());
					_count++;
					return 0;
				}     				
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called once
			assertTrue("callback was not called", callback._count == 1);

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);    
			// verify that callback was called twice
			assertTrue("callback was not called", callback._count == 2);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);
			
			int ret = consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo);
			
			if ( ret == ReactorReturnCodes.SUCCESS)
			{
				System.out.println("Location: " + errorInfo.location());
				System.out.println("Error text: " + errorInfo.error().text());
			}

			assertTrue("Expected SUCCESS", ret == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);		

			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName() == null);	

		}
		finally
		{
			consumerReactor.close();
		}	
	}	

	@Test
	public void LDPQueryServiceDiscoveryErrorPasswordAndSuccessConnectTest() {
		LDPQueryServiceDiscoveryErrorPasswordAndSuccessConnect(false, null);
	}

	@Test
	public void LDPQueryServiceDiscoveryErrorPasswordAndSuccessConnectTest_WebSocket_Rwf() {
		LDPQueryServiceDiscoveryErrorPasswordAndSuccessConnect(true, "rssl.rwf");
	}

	@Test
	public void LDPQueryServiceDiscoveryErrorPasswordAndSuccessConnectTest_WebSocket_Json() {
		LDPQueryServiceDiscoveryErrorPasswordAndSuccessConnect(true, "tr_json2");
	}

	private void LDPQueryServiceDiscoveryErrorPasswordAndSuccessConnect(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryErrorPasswordAndSuccessConnectTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {

			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data("WRONG PASSWORD");
				reactorServiceDiscoveryOptions.password(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.clientId(buf);
			}

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					assertTrue(event.errorInfo().code() == ReactorReturnCodes.FAILURE );
					// since password is incorrect we should see access_denied message
					assertTrue(event.errorInfo().toString().contains("{\"error\":\"access_denied\"  ,\"error_description\":\"Invalid username or password.\" }"));
					System.out.println(event.serviceEndpointInfo());
					System.out.println(event.errorInfo());
					_count++;
					return 0;
				}     				
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called once
			assertTrue("callback was not called", callback._count == 1);

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);    
			// verify that callback was called twice
			assertTrue("callback was not called", callback._count == 2);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);					

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);		

			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName() == null);	

		}
		finally
		{
			consumerReactor.close();
		}
	}	


	@Test
	public void LDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnectTest() {
		LDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnect(false, null);
	}

	@Test
	public void LDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnectTest_WebSocket_Rwf() {
		LDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnect(true, "rssl.rwf");
	}

	@Test
	public void LDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnectTest_WebSocket_Json() {
		LDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnect(true, "tr_json2");
	}

	private void LDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnect(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnectTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {

			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpPassword"));
				reactorServiceDiscoveryOptions.password(buf);
			}	
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data("INVALID_CLIENT_ID");
				reactorServiceDiscoveryOptions.clientId(buf);
			}

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					assertTrue(event.errorInfo().code() == ReactorReturnCodes.FAILURE );
					// since client id is incorrect we should see invalid client error message
					assertTrue(event.errorInfo().toString().contains("{\"error\":\"invalid_client\"  ,\"error_description\":\"Invalid Application Credential.\" }"));
					System.out.println(event.serviceEndpointInfo());
					System.out.println(event.errorInfo());						
					_count++;
					return 0;
				}
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that the callback was called once
			assertTrue(callback._count == 1);

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);    
			// verify that the callback was called twice
			assertTrue(callback._count == 2);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);					

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue(consumerReactor._countAuthTokenEventCallbackCalls == 1);

			// check that user specified connection info was not overwritten
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address() == null);
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName() == null);	

		}
		finally
		{
			consumerReactor.close();
		}	
	}

	@Test
	public void LDPQueryServiceDiscoveryNoDataFormatProtocolTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryNoDataFormatProtocolTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {

			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpPassword"));
				reactorServiceDiscoveryOptions.password(buf);
			}   	
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.clientId(buf);
			} 	

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					assertTrue(event.errorInfo().code() == ReactorReturnCodes.SUCCESS );
					assertTrue(event.serviceEndpointInfo().size() > 0);

					for (int i = 0; i < event.serviceEndpointInfo().size(); i++)
					{
						assertTrue(event.serviceEndpointInfo().get(i).transport().equals("tcp"));
						assertTrue(event.serviceEndpointInfo().get(i).dataFormatList().get(0).equals("rwf"));
					}
					System.out.println(event.serviceEndpointInfo());				
					_count++;
					return 0;
				}
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			assertTrue("queryServiceDiscovery did not return SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called once
			assertTrue("callback was not called", callback._count == 1);

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  

			assertTrue("queryServiceDiscovery did not return SUCCESS",consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called once
			assertTrue("callback was not called", callback._count == 2);			

		}
		finally
		{
			consumerReactor.close();
		}		
	}	

	@Test
	public void LDPQueryServiceDiscoveryNoTransportProtocolTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryNoTransportProtocolTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {

			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpPassword"));
				reactorServiceDiscoveryOptions.password(buf);
			}  	
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.clientId(buf);
			}

			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					assertTrue(event.errorInfo().code() == ReactorReturnCodes.SUCCESS );
					assertTrue(event.serviceEndpointInfo().size() > 0);

					for (int i = 0; i < event.serviceEndpointInfo().size(); i++)
					{
						assertTrue(event.serviceEndpointInfo().get(i).transport().equals("tcp"));
						assertTrue(event.serviceEndpointInfo().get(i).dataFormatList().get(0).equals("rwf"));
					}
					System.out.println(event.serviceEndpointInfo());			
					_count++;
					return 0;
				}     				
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);     

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called once
			assertTrue("callback was not called", callback._count == 1);			

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);    
			// verify that callback was called twice
			assertTrue("callback was not called", callback._count == 2);

		}
		finally
		{
			consumerReactor.close();
		}	
	}

	private void unlockAccount()
	{
		TestReactor consumerReactor = null;
		try {
			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpPassword"));
				reactorServiceDiscoveryOptions.password(buf);
			}  	
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.clientId(buf);
			}

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					return 0;
				}     				
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);		

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			errorInfo = ReactorFactory.createReactorErrorInfo();   

			int i = 0;
			System.out.println("UNLOCKING ACCOUNT");
			int ret = consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo);
			
			while ( ret != ReactorReturnCodes.SUCCESS || errorInfo.code() != 0)
			{
				System.out.println("================= UNLOCK ACCOUNT: " + (i++) + " ================== \n" + errorInfo.error().toString());	
				errorInfo = ReactorFactory.createReactorErrorInfo();

				try {
					Thread.sleep(1000 * i);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				
				ret = consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo);
			}
			System.out.println("================= ACCOUNT UNLOCKED " + i + " ================== \n" + errorInfo.error().toString());				
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	private void unlockAccountV2()
	{
		TestReactor consumerReactor = null;
		try {
			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.clientId(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("clientSecret"));
				reactorServiceDiscoveryOptions.clientSecret(buf);
			}
			{
				byte[] jwkFile = null;
				try {
					jwkFile = Files.readAllBytes(Paths.get(System.getProperty("jwkFile")));
				} catch (IOException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
				String jwkText = new String(jwkFile);
				Buffer buf = CodecFactory.createBuffer();
				buf.data(jwkText);
				reactorServiceDiscoveryOptions.clientJWK(buf);
			}

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					return 0;
				}     				
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);		

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			errorInfo = ReactorFactory.createReactorErrorInfo();   

			int i = 0;
			System.out.println("UNLOCKING ACCOUNT");
			int ret = consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo);
			
			while ( ret != ReactorReturnCodes.SUCCESS || errorInfo.code() != 0)
			{
				System.out.println("================= UNLOCK ACCOUNT: " + (i++) + " ================== \n" + errorInfo.error().toString());	
				errorInfo = ReactorFactory.createReactorErrorInfo();

				try {
					Thread.sleep(1000 * i);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}
				
				ret = consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo);
			}
			System.out.println("================= ACCOUNT UNLOCKED " + i + " ================== \n" + errorInfo.error().toString());				
		}
		finally
		{
			consumerReactor.close();
		}
	}

	@Test
	public void LDPQueryServiceDiscoveryTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPQueryServiceDiscoveryTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		try {
			ReactorErrorInfo errorInfo = null;		 
			ReactorServiceDiscoveryOptions reactorServiceDiscoveryOptions = ReactorFactory.createReactorServiceDiscoveryOptions();
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.userName(buf);
			}
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpPassword"));
				reactorServiceDiscoveryOptions.password(buf);
			}  	  	
			{
				Buffer buf = CodecFactory.createBuffer();
				buf.data(System.getProperty("edpUserName"));
				reactorServiceDiscoveryOptions.clientId(buf);
			}

			reactorServiceDiscoveryOptions.transport(ReactorDiscoveryTransportProtocol.RD_TP_TCP);
			reactorServiceDiscoveryOptions.dataFormat(ReactorDiscoveryDataFormatProtocol.RD_DP_RWF);

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					assertTrue(event.errorInfo().error().text(), event.errorInfo().code() == ReactorReturnCodes.SUCCESS );
					assertTrue(event.serviceEndpointInfo().size() > 0);

					for (int i = 0; i < event.serviceEndpointInfo().size(); i++)
					{
						assertTrue(event.serviceEndpointInfo().get(i).transport().equals("tcp"));
						assertTrue(event.serviceEndpointInfo().get(i).dataFormatList().get(0).equals("rwf"));
					}
					System.out.println(event.serviceEndpointInfo());				
					_count++;
					return 0;
				}     				
			};

			reactorServiceDiscoveryOptions.reactorServiceEndpointEventCallback(callback);
			setupProxyForReactorServiceDiscoveryOptions(reactorServiceDiscoveryOptions);

			/* Create reactor. */
			consumerReactor = new TestReactor();

			/* Create consumer. */
			Consumer consumer = new Consumer(consumerReactor);

			setupConsumer(consumer, true);

			errorInfo = ReactorFactory.createReactorErrorInfo();
			errorInfo.clear();
			assertNotNull(errorInfo);     

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);
			// verify that callback was called once
			assertTrue("callback was not called", callback._count == 1);

			// do it second time to test with rest reactor already created
			errorInfo = ReactorFactory.createReactorErrorInfo();
			assertNotNull(errorInfo);  

			assertTrue("Expected SUCCESS", consumerReactor._reactor.queryServiceDiscovery(reactorServiceDiscoveryOptions, errorInfo) == ReactorReturnCodes.SUCCESS);    
			// verify that callback was called once
			assertTrue("callback was not called", callback._count == 2);

		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPNoOAuthCredentialForEnablingSessionMgnt()
	{
		System.out.println("\n>>>>>>>>> Running LDPNoOAuthCredentialForEnablingSessionMgnt <<<<<<<<<<\n");
		
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			reactorOptions.tokenReissueRatio(0.1); // Send token reissue every 30 seconds
			
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, false);
			
			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = ReactorFactory.createReactorConnectOptions();
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();

			connectInfo.enableSessionManagement(true);
			setupProxyForConnectOptions(connectInfo.connectOptions());
			rcOpts.connectionList().add(connectInfo);

			assertTrue("Expected PARAMETER_INVALID", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.INVALID_USAGE);
			assertEquals("Expected error text", "There is no user credential available for enabling session management.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPNoUserNameForEnablingSessionMgnt()
	{
		System.out.println("\n>>>>>>>>> Running LDPNoUserNameForEnablingSessionMgnt <<<<<<<<<<\n");
		
		assumeTrue(checkCredentials());
		unlockAccount();
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();

			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, false);
			
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			
			oAuthCredential.password().data(System.getProperty("edpPassword"));
			oAuthCredential.clientId().data(System.getProperty("edpUserName"));
			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = ReactorFactory.createReactorConnectOptions();
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			
			connectInfo.enableSessionManagement(true);
			setupProxyForConnectOptions(connectInfo.connectOptions());
			rcOpts.connectionList().add(connectInfo);

			assertTrue("Expected INVALID_USAGE", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.INVALID_USAGE);
			assertEquals("Expected error text", "Failed to copy OAuth credential for enabling the session management; OAuth user name does not exist.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPNoPasswordForEnablingSessionMgnt()
	{
		System.out.println("\n>>>>>>>>> Running LDPNoPasswordForEnablingSessionMgnt <<<<<<<<<<\n");
		
		assumeTrue(checkCredentials());
		unlockAccount();
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, false);
			
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			
			oAuthCredential.userName().data(System.getProperty("edpUserName"));
			oAuthCredential.clientId().data(System.getProperty("edpUserName"));
			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = ReactorFactory.createReactorConnectOptions();
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			
			connectInfo.enableSessionManagement(true);
			setupProxyForConnectOptions(connectInfo.connectOptions());
			rcOpts.connectionList().add(connectInfo);

			assertTrue("Expected INVALID_USAGE", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.INVALID_USAGE);
			assertEquals("Expected error text", "Failed to copy OAuth credential for enabling the session management; OAuth password does not exist.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@SuppressWarnings("deprecation")
	@Test
	public void LDPNoClientIDForEnablingSessionMgnt()
	{
		System.out.println("\n>>>>>>>>> Running LDPNoClientIDForEnablingSessionMgnt <<<<<<<<<<\n");
		
		assumeTrue(checkCredentials());
		unlockAccount();
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, false);
			
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			
			consumerRole.clientId().clear();
			oAuthCredential.userName().data(System.getProperty("edpUserName"));
			oAuthCredential.password().data(System.getProperty("edpPassword"));

			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = ReactorFactory.createReactorConnectOptions();
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			
			connectInfo.enableSessionManagement(true);
			setupProxyForConnectOptions(connectInfo.connectOptions());
			rcOpts.connectionList().add(connectInfo);

			assertTrue("Expected INVALID_USAGE", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.INVALID_USAGE);
			assertEquals("Expected error text", "Failed to copy OAuth credential for enabling the session management; OAuth client ID does not exist.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_CallbackTest() {
		LDPMultipleOpenConnections_SameUser_Diff_Callback(false, null);
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_CallbackTest_WebSocket_Rwf() {
		LDPMultipleOpenConnections_SameUser_Diff_Callback(true, "rssl.rwf");
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_CallbackTest_WebSocket_Json() {
		LDPMultipleOpenConnections_SameUser_Diff_Callback(true, "tr_json2");
	}

	private void LDPMultipleOpenConnections_SameUser_Diff_Callback(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPMultipleOpenConnections_SameUser_Diff_CallbackTest <<<<<<<<<<\n");
		
		assumeTrue(checkCredentials());
		unlockAccount();
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, true);
			
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			
			oAuthCredential.reactorOAuthCredentialEventCallback(
					new ReactorOAuthCredentialEventCallback()
					{
						@Override
						public int reactorOAuthCredentialEventCallback(
								ReactorOAuthCredentialEvent reactorOAuthCredentialEvent) {
							return 0;
						}
					}
			);
			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			/*
			 * create a Client Connection.
			*/
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			
			connectInfo.enableSessionManagement(true);
			setupProxyForConnectOptions(connectInfo.connectOptions());
			rcOpts.connectionList().add(connectInfo);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.SUCCESS);
			
			/* Register another ReactorOAuthCredentialEventCallback */
			oAuthCredential.reactorOAuthCredentialEventCallback(
					new ReactorOAuthCredentialEventCallback()
					{
						@Override
						public int reactorOAuthCredentialEventCallback(
								ReactorOAuthCredentialEvent reactorOAuthCredentialEvent) {
							return 0;
						}
					}
			);
			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			
			assertTrue("Expected INVALID_USAGE", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.INVALID_USAGE);
			assertEquals("Expected error text", "The ReactorOAuthCredentialEventCallback of ReactorOAuthCredential is not equal for the existing token session.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_NULL_CallbackTest() {
		LDPMultipleOpenConnections_SameUser_Diff_NULL_Callback(false, null);
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_NULL_CallbackTest_WebSocket_Rwf() {
		LDPMultipleOpenConnections_SameUser_Diff_NULL_Callback(true, "rssl.rwf");
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_NULL_CallbackTest_WebSocket_Json() {
		LDPMultipleOpenConnections_SameUser_Diff_NULL_Callback(true, "tr_json2");
	}

	private void LDPMultipleOpenConnections_SameUser_Diff_NULL_Callback(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPMultipleOpenConnections_SameUser_Diff_NULL_CallbackTest <<<<<<<<<<\n");
		
		assumeTrue(checkCredentials());
		unlockAccount();
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, true);
			
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			
			/* Register null callback for the existing token session */
			oAuthCredential.reactorOAuthCredentialEventCallback(null);
			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			/*
			 * create a Client Connection.
			*/
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			
			setupProxyForConnectOptions(connectInfo.connectOptions());
			connectInfo.enableSessionManagement(true);
			rcOpts.connectionList().add(connectInfo);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.SUCCESS);
			
			/* Register another ReactorOAuthCredentialEventCallback */
			oAuthCredential.reactorOAuthCredentialEventCallback(
					new ReactorOAuthCredentialEventCallback()
					{
						@Override
						public int reactorOAuthCredentialEventCallback(
								ReactorOAuthCredentialEvent reactorOAuthCredentialEvent) {
							return 0;
						}
					}
			);
			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			
			assertTrue("Expected INVALID_USAGE", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.INVALID_USAGE);
			assertEquals("Expected error text", "The ReactorOAuthCredentialEventCallback of ReactorOAuthCredential is not equal for the existing token session.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_ClientIDTest() {
		LDPMultipleOpenConnections_SameUser_Diff_ClientID(false, null);
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_ClientIDTest_WebSocket_Rwf() {
		LDPMultipleOpenConnections_SameUser_Diff_ClientID(true, "rssl.rwf");
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_ClientIDTest_WebSocket_Json() {
		LDPMultipleOpenConnections_SameUser_Diff_ClientID(true, "tr_json2");
	}

	private void LDPMultipleOpenConnections_SameUser_Diff_ClientID(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPMultipleOpenConnections_SameUser_Diff_ClientIDTest <<<<<<<<<<\n");
		
		assumeTrue(checkCredentials());
		unlockAccount();
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, true);
			
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			/*
			 * create a Client Connection.
			*/
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			setupProxyForConnectOptions(connectInfo.connectOptions());
			connectInfo.enableSessionManagement(true);
			rcOpts.connectionList().add(connectInfo);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.SUCCESS);
			
			/* Set a different Client ID */
			oAuthCredential.clientId().data("AnotherClientID");
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			assertTrue("Expected INVALID_USAGE", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.INVALID_USAGE);
			assertEquals("Expected error text", "The Client ID of ReactorOAuthCredential is not equal for the existing token session.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_PasswordTest() {
		LDPMultipleOpenConnections_SameUser_Diff_Password(false, null);
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_PasswordTest_WebSocket_Rwf() {
		LDPMultipleOpenConnections_SameUser_Diff_Password(true, "rssl.rwf");
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_PasswordTest_WebSocket_Json() {
		LDPMultipleOpenConnections_SameUser_Diff_Password(true, "tr_json2");
	}

	private void LDPMultipleOpenConnections_SameUser_Diff_Password(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPMultipleOpenConnections_SameUser_Diff_PasswordTest <<<<<<<<<<\n");
		
		assumeTrue(checkCredentials());
		unlockAccount();
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, true);
			
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			/*
			 * create a Client Connection.
			*/
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			
			connectInfo.enableSessionManagement(true);
			setupProxyForConnectOptions(connectInfo.connectOptions());
			rcOpts.connectionList().add(connectInfo);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.SUCCESS);
			
			oAuthCredential.password().data("AnotherPassword");
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			assertTrue("Expected INVALID_USAGE", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.INVALID_USAGE);
			assertEquals("Expected error text", "The password of ReactorOAuthCredential is not equal for the existing token session.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_TokenScopeTest() {
		LDPMultipleOpenConnections_SameUser_Diff_TokenScope(false, null);
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_TokenScopeTest_WebSocket_Rwf() {
		LDPMultipleOpenConnections_SameUser_Diff_TokenScope(true, "rssl.rwf");
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_TokenScopeTest_WebSocket_Json() {
		LDPMultipleOpenConnections_SameUser_Diff_TokenScope(true, "tr_json2");
	}

	private void LDPMultipleOpenConnections_SameUser_Diff_TokenScope(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPMultipleOpenConnections_SameUser_Diff_TokenScopeTest <<<<<<<<<<\n");
		
		assumeTrue(checkCredentials());
		unlockAccount();
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, true);
			
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			/*
			 * create a Client Connection.
			*/
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			
			connectInfo.enableSessionManagement(true);
			setupProxyForConnectOptions(connectInfo.connectOptions());
			rcOpts.connectionList().add(connectInfo);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.SUCCESS);
			
			oAuthCredential.tokenScope().data("refinitiv.api");
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			assertTrue("Expected INVALID_USAGE", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.INVALID_USAGE);
			assertEquals("Expected error text", "The token scope of ReactorOAuthCredential is not equal for the existing token session.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControlTest() {
		LDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControl(false, null);
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControlTest_WebSocket_Rwf() {
		LDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControl(true, "rssl.rwf");
	}

	@Test
	public void LDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControlTest_WebSocket_Json() {
		LDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControl(true, "tr_json2");
	}

	private void LDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControl(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControlTest <<<<<<<<<<\n");
		
		assumeTrue(checkCredentials());
		unlockAccount();
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, true);
			
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			
			oAuthCredential.takeExclusiveSignOnControl(true);
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			/*
			 * create a Client Connection.
			*/
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
			
			connectInfo.enableSessionManagement(true);
			setupProxyForConnectOptions(connectInfo.connectOptions());
			rcOpts.connectionList().add(connectInfo);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.SUCCESS);
			
			oAuthCredential.takeExclusiveSignOnControl(false);
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			assertTrue("Expected INVALID_USAGE", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.INVALID_USAGE);
			assertEquals("Expected error text", "The takeExclusiveSignOnControl of ReactorOAuthCredential is not equal for the existing token session.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPConnectionUsingReactorOAuthCredentialOnlyTest() {
		LDPConnectionUsingReactorOAuthCredentialOnly(false, null);
	}

	@Test
	public void LDPConnectionUsingReactorOAuthCredentialOnlyTest_WebSocket_Json() {
		LDPConnectionUsingReactorOAuthCredentialOnly(true, "tr_json2");
	}

	private void LDPConnectionUsingReactorOAuthCredentialOnly(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectionUsingReactorOAuthCredentialOnlyTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			reactorOptions.tokenReissueRatio(0.1); // Send token reissue every 30 seconds
			
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, false);
			
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.watchlistOptions().enableWatchlist(false); /* Disable the watchlist */
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			
			oAuthCredential.userName().data(System.getProperty("edpUserName"));
			oAuthCredential.password().data(System.getProperty("edpPassword"));
			oAuthCredential.clientId().data(System.getProperty("edpUserName"));
			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);
			
			int ret = consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo);
			
			assertEquals("Checks the number of AuthTokenEvent", 1, consumerReactor._countAuthTokenEventCallbackCalls);
			
			if (ret != ReactorReturnCodes.SUCCESS)
			{
				System.out.println("Location : " + errorInfo.location());
				System.out.println("Error text: " + errorInfo.error().text());
			}

			assertTrue("Expected SUCCESS", ret == ReactorReturnCodes.SUCCESS);
			
			for(int i = 1 ;i < 80; i++)
			{
				consumerReactor.dispatch(-1, 1000);
			}
			
			assertTrue("Checks the number of AuthTokenEvent", consumerReactor._countAuthTokenEventCallbackCalls >= 2);
			
			assertTrue("Expected One token session", consumer.reactorChannel().reactor().numberOfTokenSession() == 1);
			
			assertTrue("Expected SUCCESS", consumer.reactorChannel().close(errorInfo) == ReactorReturnCodes.SUCCESS);
			
			assertTrue("Expected Zero token session", consumer.reactorChannel().reactor().numberOfTokenSession() == 0);
			
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPConnectionUsingReactorOAuthCredentialOnly_WatchListEnabledTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPConnectionUsingReactorOAuthCredentialOnly_WatchListEnabledTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			reactorOptions.tokenReissueRatio(0.1); // Send token reissue every 30 seconds
			
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, false);
			
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.watchlistOptions().enableWatchlist(true);
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			
			oAuthCredential.userName().data(System.getProperty("edpUserName"));
			oAuthCredential.password().data(System.getProperty("edpPassword"));
			oAuthCredential.clientId().data(System.getProperty("edpUserName"));
			
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, false, null);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			
			for(int i = 1 ;i < 80; i++)
			{
				consumerReactor.dispatch(-1, 1000);
			}
			
			assertTrue("Checks the number of AuthTokenEvent", consumerReactor._countAuthTokenEventCallbackCalls >= 2);
			
			assertTrue("Expected One token session", consumer.reactorChannel().reactor().numberOfTokenSession() == 1);
			
			assertTrue("Expected SUCCESS", consumer.reactorChannel().close(errorInfo) == ReactorReturnCodes.SUCCESS);
			
			assertTrue("Expected Zero token session", consumer.reactorChannel().reactor().numberOfTokenSession() == 0);
			
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPMultiConnectionsConnectAndCloseWithSameCredentialTest() {
		LDPMultiConnectionsConnectAndCloseWithSameCredential(false, null);
	}

	@Test
	public void LDPMultiConnectionsConnectAndCloseWithSameCredentialTest_WebSocket_Json() {
		LDPMultiConnectionsConnectAndCloseWithSameCredential(true, "tr_json2");
	}

	private void LDPMultiConnectionsConnectAndCloseWithSameCredential(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPMultiConnectionsConnectAndCloseWithSameCredentialTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			reactorOptions.tokenReissueRatio(0.1); // Send token reissue every 30 seconds
			
			//TestReactor.enableReactorXmlTracing();
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, true);
			
			Consumer consumer2 = new Consumer(consumerReactor);
			setupConsumer(consumer2, true);  

			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			ReactorConnectOptions rcOpts2 = createDefaultConsumerConnectOptionsForSessionManagment(consumer2, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);
			rcOpts2.connectionList().get(0).reactorAuthTokenEventCallback(consumer2);	

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts2, consumer2.reactorRole(), errorInfo) == ReactorReturnCodes.SUCCESS);
			
			for(int i = 1 ;i < 65; i++)
			{
				consumerReactor.dispatch(-1, 1000);
			}
			
			/* This test must receives 2 login request and 4 login reissues*/
			assertTrue("Checks the number of AuthTokenEvent", consumerReactor._countAuthTokenEventCallbackCalls == 4);
			
			assertTrue("Expected SUCCESS", consumer.reactorChannel().close(errorInfo) == ReactorReturnCodes.SUCCESS);
			
			assertTrue("Expected One token session", consumer.reactorChannel().reactor().numberOfTokenSession() == 1);
			
			assertTrue("Expected SUCCESS", consumer2.reactorChannel().close(errorInfo) == ReactorReturnCodes.SUCCESS);
			
			assertTrue("Expected zero token session", consumer.reactorChannel().reactor().numberOfTokenSession() == 0);
			
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnlyTest() {
		LDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnly(false, null);
	}

	@Test
	public void LDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnlyTest_Websocket_Json() {
		LDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnly(true, "tr_json2");
	}

	private void LDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnly(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnlyTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		
		try {
			
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			reactorOptions.tokenReissueRatio(0.1); // Send token reissue every 30 seconds
			
			//TestReactor.enableReactorXmlTracing();
			consumerReactor = new TestReactor(reactorOptions);

			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, true);
			
			Consumer consumer2 = new Consumer(consumerReactor);
			setupConsumer(consumer2, true);
			
			ReactorOAuthCredential reactorOAutchCredential = ReactorFactory.createReactorOAuthCredential();
			Buffer username = CodecFactory.createBuffer();
			username.data(System.getProperty("edpUserName"));
			reactorOAutchCredential.userName(username);
			Buffer password = CodecFactory.createBuffer();
			password.data(System.getProperty("edpPassword"));
			reactorOAutchCredential.password(password);
			reactorOAutchCredential.clientId(username);
			
			ConsumerRole consumerRole1 = (ConsumerRole)consumer.reactorRole();
			consumerRole1.reactorOAuthCredential(reactorOAutchCredential);
			
			ConsumerRole consumerRole2 = (ConsumerRole)consumer2.reactorRole();
			consumerRole2.reactorOAuthCredential(reactorOAutchCredential);
			
			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);
			ReactorConnectOptions rcOpts2 = createDefaultConsumerConnectOptionsForSessionManagment(consumer2, isWebsocket, protocol);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);
			rcOpts2.connectionList().get(0).reactorAuthTokenEventCallback(consumer2);	

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole1, errorInfo) == ReactorReturnCodes.SUCCESS);
			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts2, consumerRole2, errorInfo) == ReactorReturnCodes.SUCCESS);
			
			for(int i = 1 ;i < 65; i++)
			{
				consumerReactor.dispatch(-1, 1000);
			}
			
			/* This test must receives 2 login request and 4 login reissues*/
			assertTrue("Checks the number of AuthTokenEvent", consumerReactor._countAuthTokenEventCallbackCalls == 4);
			
			assertTrue("Expected SUCCESS", consumer.reactorChannel().close(errorInfo) == ReactorReturnCodes.SUCCESS);
			
			assertTrue("Expected One token session", consumer.reactorChannel().reactor().numberOfTokenSession() == 1);
			
			assertTrue("Expected SUCCESS", consumer2.reactorChannel().close(errorInfo) == ReactorReturnCodes.SUCCESS);
			
			assertTrue("Expected zero token session", consumer.reactorChannel().reactor().numberOfTokenSession() == 0);
			
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPSubmitTokenRenewalWithoutTokenSessionAndCallbackTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPSubmitTokenRenewalWithoutTokenSessionAndCallbackTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		
		try
		{
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	
			consumerReactor = new TestReactor();
			
			ReactorOAuthCredentialRenewalOptions renewalOptions = ReactorFactory.createReactorOAuthCredentialRenewalOptions();
			ReactorOAuthCredentialRenewal oAuthCredentialRenewal = ReactorFactory.createReactorOAuthCredentialRenewal();
			
			renewalOptions.renewalModes(ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD);
			
			oAuthCredentialRenewal.userName().data(System.getProperty("edpUserName"));
			oAuthCredentialRenewal.password().data(System.getProperty("edpPassword"));
			oAuthCredentialRenewal.clientId().data(System.getProperty("edpUserName"));
			setupProxyForReactorRenewalOptions(renewalOptions);

			int ret = consumerReactor._reactor.submitOAuthCredentialRenewal(renewalOptions, oAuthCredentialRenewal, errorInfo);
			
			assertTrue("Expected PARAMETER_INVALID", ret == ReactorReturnCodes.PARAMETER_INVALID);
			assertEquals("Checking error text", "ReactorOAuthCredentialRenewalOptions.reactorAuthTokenEventCallback() not provided, aborting.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPSubmitTokenRenewalUsingInvalidCredential_WithoutTokenSessionTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPSubmitTokenRenewalUsingInvalidCredential_WithoutTokenSessionTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		TestReactor consumerReactor = null;
		
		try
		{
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	
			consumerReactor = new TestReactor();
			
			ReactorOAuthCredentialRenewalOptions renewalOptions = ReactorFactory.createReactorOAuthCredentialRenewalOptions();
			ReactorOAuthCredentialRenewal oAuthCredentialRenewal = ReactorFactory.createReactorOAuthCredentialRenewal();
			
			ReactorAuthTokenEventCallback callback = new ReactorAuthTokenEventCallback()
			{
				@Override
				public int reactorAuthTokenEventCallback(ReactorAuthTokenEvent event) {
					return 0;
				}
			};
			
			renewalOptions.reactorAuthTokenEventCallback(callback);
			renewalOptions.renewalModes(ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD);
			setupProxyForReactorRenewalOptions(renewalOptions);

			oAuthCredentialRenewal.userName().data("Fake");
			oAuthCredentialRenewal.password().data("Fake");
			oAuthCredentialRenewal.clientId().data("Fake");
			
			int ret = consumerReactor._reactor.submitOAuthCredentialRenewal(renewalOptions, oAuthCredentialRenewal, errorInfo);
			
			assertTrue("Faield to send the token request", ret == ReactorReturnCodes.FAILURE);
			assertEquals("Checking error text", "Failed to request authentication token information with HTTP error 401. Text: {\"error\":\"invalid_client\"  ,\"error_description\":\"Invalid Application Credential.\" } ", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPSubmitTokenRenewalTakeExclusiveSignOnOff_WithoutTokenSessionTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPSubmitTokenRenewalTakeExclusiveSignOnOff_WithoutTokenSessionTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		
		try
		{
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	
			consumerReactor = new TestReactor();
			
			ReactorOAuthCredentialRenewalOptions renewalOptions = ReactorFactory.createReactorOAuthCredentialRenewalOptions();
			ReactorOAuthCredentialRenewal oAuthCredentialRenewal = ReactorFactory.createReactorOAuthCredentialRenewal();
			setupProxyForReactorRenewalOptions(renewalOptions);

			ReactorAuthTokenEventCallback callback = new ReactorAuthTokenEventCallback()
			{
				@Override
				public int reactorAuthTokenEventCallback(ReactorAuthTokenEvent event) {
					return 0;
				}
			};
			
			renewalOptions.reactorAuthTokenEventCallback(callback);
			renewalOptions.renewalModes(ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD);
			
			oAuthCredentialRenewal.userName().data(System.getProperty("edpUserName"));
			oAuthCredentialRenewal.password().data(System.getProperty("edpPassword"));
			oAuthCredentialRenewal.clientId().data(System.getProperty("edpUserName"));
			oAuthCredentialRenewal.takeExclusiveSignOnControl(false);
			
			int ret = consumerReactor._reactor.submitOAuthCredentialRenewal(renewalOptions, oAuthCredentialRenewal, errorInfo);
			
			assertTrue("Faield to send the token request", ret == ReactorReturnCodes.FAILURE);
			assertEquals("Checking error text", "Failed to request authentication token information with HTTP error 400. Text: {\"error\":\"access_denied\"  ,\"error_description\":\"Session quota is reached.\" } ", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void LDPSubmitTokenRenewalWithoutTokenSessionTest()
	{
		System.out.println("\n>>>>>>>>> Running LDPSubmitTokenRenewalWithoutTokenSessionTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		
		try
		{
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	
			consumerReactor = new TestReactor();
			
			ReactorOAuthCredentialRenewalOptions renewalOptions = ReactorFactory.createReactorOAuthCredentialRenewalOptions();
			ReactorOAuthCredentialRenewal oAuthCredentialRenewal = ReactorFactory.createReactorOAuthCredentialRenewal();
			
			ReactorAuthTokenEventCallback callback = new ReactorAuthTokenEventCallback()
			{
				@Override
				public int reactorAuthTokenEventCallback(ReactorAuthTokenEvent event) {
					
					assertTrue("Expected NO error", event.errorInfo().code() == ReactorReturnCodes.SUCCESS);
					assertTrue("Get an access token", event.reactorAuthTokenInfo().accessToken().length() > 0 );
					assertTrue("Get a refresh token", event.reactorAuthTokenInfo().refreshToken().length() > 0 );
					return 0;
				}
			};
			
			renewalOptions.reactorAuthTokenEventCallback(callback);
			renewalOptions.renewalModes(ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD);
			setupProxyForReactorRenewalOptions(renewalOptions);

			oAuthCredentialRenewal.userName().data(System.getProperty("edpUserName"));
			oAuthCredentialRenewal.password().data(System.getProperty("edpPassword"));
			oAuthCredentialRenewal.clientId().data(System.getProperty("edpUserName"));
			
			assertTrue("Expected SUCCESS", consumerReactor._reactor.submitOAuthCredentialRenewal(renewalOptions, oAuthCredentialRenewal, errorInfo) == ReactorReturnCodes.SUCCESS);
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@SuppressWarnings("deprecation")
	@Test
	public void LDPSubmitCredentialInCallbackTest() {
		LDPSubmitCredentialInCallback(false, null);
	}

	@SuppressWarnings("deprecation")
	@Test
	public void LDPSubmitCredentialInCallbackTest_WebSocket_Json() {
		LDPSubmitCredentialInCallback(true, "tr_json2");
	}

	@SuppressWarnings("deprecation")
	private void LDPSubmitCredentialInCallback(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running LDPSubmitCredentialInCallbackTest <<<<<<<<<<\n");
		assumeTrue(checkCredentials());
		unlockAccount();
		
		TestReactor consumerReactor = null;
		
		try
		{
			ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();	 	

			/* Create reactor. */
			ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
			
			reactorOptions.tokenReissueRatio(0.1); // Send token reissue every 30 seconds
	
			//TestReactor.enableReactorXmlTracing();
			consumerReactor = new TestReactor(reactorOptions);
	
			Consumer consumer = new Consumer(consumerReactor);
			setupConsumer(consumer, true);
			
			ConsumerRole consumerRole = ((ConsumerRole)consumer.reactorRole());
			LoginRequest loginRequest = consumerRole.rdmLoginRequest();
					
			ReactorOAuthCredential oAuthCredential = ReactorFactory.createReactorOAuthCredential();
			oAuthCredential.password().data(loginRequest.password().toString());
			
			ReactorOAuthCredentialEventCallbackTest oAuthCredentialEventCallbackTest = new ReactorOAuthCredentialEventCallbackTest(oAuthCredential)
			{
				@Override
				public int reactorOAuthCredentialEventCallback( ReactorOAuthCredentialEvent reactorOAuthCredentialEvent) {
					
					ReactorOAuthCredentialRenewalOptions renewalOptions = ReactorFactory.createReactorOAuthCredentialRenewalOptions();
					ReactorOAuthCredentialRenewal oAuthCredentialRenewal = ReactorFactory.createReactorOAuthCredentialRenewal();
					
					renewalOptions.renewalModes(ReactorOAuthCredentialRenewalOptions.RenewalModes.PASSWORD);
					oAuthCredentialRenewal.password().data(this._oauthCredentail.password().toString());
					setupProxyForReactorRenewalOptions(renewalOptions);

					assertTrue("Expected SUCCESS", reactorOAuthCredentialEvent.reactor().submitOAuthCredentialRenewal(renewalOptions, oAuthCredentialRenewal, errorInfo) == ReactorReturnCodes.SUCCESS);
					
					return 0;
				}
			};
			
			/*
			 * create a Client Connection.
			 */
			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, false, null);
			rcOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);
			oAuthCredential.reactorOAuthCredentialEventCallback(oAuthCredentialEventCallbackTest);
			consumerRole.reactorOAuthCredential(oAuthCredential);

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			
			RestClient restClient = consumerReactor._reactor._restClient;
			
			for(int i = 1 ;i < 10; i++)
			{
				consumerReactor.dispatch(-1, 1000);
			}
			
			/* Sends the token request using the password type to invalid the existing refresh token */
			{
				RestAuthOptions authOptions = new RestAuthOptions(true);
				RestConnectOptions restConnectOptions = new RestConnectOptions(consumerReactor._reactor.reactorOptions());
				ReactorAuthTokenInfo authTokenInfo = new ReactorAuthTokenInfo();

				ReactorRestProxyOptions restProxyOptions = new ReactorRestProxyOptions();
				setupProxyForReactorRestProxyOptions(restProxyOptions);
				restConnectOptions.applyProxyInfo(restProxyOptions);
				
				authOptions.username(loginRequest.userName().toString());
				authOptions.clientId(consumerRole.clientId().toString());
				authOptions.password(loginRequest.password().toString());
				
				authTokenInfo.tokenVersion(TokenVersion.V1);

				assertTrue("Expected SUCCESS", restClient.getAuthAccessTokenInfo(authOptions, restConnectOptions, authTokenInfo, true, errorInfo) == ReactorReturnCodes.SUCCESS);
			}
			
			for(int i = 1 ;i < 90; i++)
			{
				consumerReactor.dispatch(-1, 1000);
			}

			/* 2 success and 1 fails */
			assertTrue("Checks the number of AuthTokenEvent", consumerReactor._countAuthTokenEventCallbackCalls == 3);

		}
		finally
		{
			consumerReactor.close();
		}
	}
}
