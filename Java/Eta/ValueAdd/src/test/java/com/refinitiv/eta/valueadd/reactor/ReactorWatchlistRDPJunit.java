///*|-----------------------------------------------------------------------------
//*|            This source code is provided under the Apache 2.0 license      --
//*|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
//*|                See the project's LICENSE.md for details.                  --
//*|           Copyright (C) 2019-2020 Refinitiv. All rights reserved.          --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.valueadd.reactor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assume.assumeTrue;

import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Objects;
import java.util.Optional;

import com.refinitiv.eta.transport.ConnectOptions;
import org.junit.Before;
import org.junit.Test;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.rdm.Login;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequestFlags;


public class ReactorWatchlistRDPJunit
{
	final int AUTH_TOKEN_EXPIRATION = 300;
	ReactorAuthTokenInfo _tokenInfo = null;	
	private static final Buffer proxyHost = CodecFactory.createBuffer();
	private static final Buffer proxyPort = CodecFactory.createBuffer();
	private static final Buffer proxyUser = CodecFactory.createBuffer();
	private static final Buffer proxyPassword = CodecFactory.createBuffer();
	private static final Buffer proxyLocalHostname = CodecFactory.createBuffer();
	private static final Buffer proxyKRBConfigFile = CodecFactory.createBuffer();
	private static final Buffer proxyDomain = CodecFactory.createBuffer();

	public ReactorWatchlistRDPJunit() {
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

    @Before
    public void init() {

        final String dictionaryFileName = "../../../Java/etc/RDMFieldDictionary";
        final String enumTypeFile = "../../../Java/etc/enumtype.def";
        com.refinitiv.eta.transport.Error error = TransportFactory.createError();
        dictionary.clear();

        assertEquals(CodecReturnCodes.SUCCESS, dictionary.loadFieldDictionary(dictionaryFileName, error));
        assertEquals(CodecReturnCodes.SUCCESS,dictionary.loadEnumTypeDictionary(enumTypeFile, error));
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
				System.getProperty("edpPassword") != null && 
				System.getProperty("keyfile") != null && 
				System.getProperty("keypasswd") != null ) 	
		{
			return true;
		}
		else
		{
			System.out.println("edpUserName edpPassword keyfile and keypasswd need to be set as VM arguments to run this test.");
			System.out.println("i.e. -DedpUserName=USERNAME -DedpPassword=PASSWORD -Dkeyfile=*.jks -Dkeypasswd=PASSWORD");
			System.out.println("or with gradle i.e. ./gradlew eta:valueadd:test --tests *RDP* -PvmArgs=\"-DedpUserName=USERNAME -DedpPassword=PASSWORD -Dkeyfile=keystore.jks -Dkeypasswd=PASSWORD\"");
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
			connectInfo.connectOptions().encryptionOptions().KeystoreFile(System.getProperty("keyfile"));
			connectInfo.connectOptions().encryptionOptions().KeystorePasswd(System.getProperty("keypasswd"));
			connectInfo.connectOptions().encryptionOptions().KeystoreType("JKS");
			connectInfo.connectOptions().encryptionOptions().SecurityProtocol("TLS");
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
			connectInfo.connectOptions().tunnelingInfo().KeystoreFile(System.getProperty("keyfile"));
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
	public void RDPConnectSpecificLocationTest_Socket() {
		RDPConnectSpecificLocation(false, null);
	}

	@Test
	public void RDPConnectSpecificLocationTest_WebSocket_Rwf() {
		RDPConnectSpecificLocation(true, "rssl.rwf");
	}

	@Test
	public void RDPConnectSpecificLocationTest_WebSocket_Json() {
		RDPConnectSpecificLocation(true, "tr_json2");
	}

	private void RDPConnectSpecificLocation(boolean isWebsocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectSpecificLocationTest <<<<<<<<<<\n");
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

	private void setupProxyForReactorRenewalOptions(ReactorOAuthCredentialRenewalOptions renewalOptions) {
		if (checkProxy()) {
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
		if (checkProxy()) {
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
		if (checkProxy()) {
			connectOptions.tunnelingInfo().HTTPproxy(true);
			connectOptions.tunnelingInfo().HTTPproxyHostName(proxyHost.toString());
			connectOptions.tunnelingInfo().HTTPproxyPort(Integer.parseInt(proxyPort.toString()));
			if (checkProxyCredentials()) {
				connectOptions.credentialsInfo().HTTPproxyDomain(proxyDomain.toString());
				connectOptions.credentialsInfo().HTTPproxyUsername(proxyUser.toString());
				connectOptions.credentialsInfo().HTTPproxyPasswd(proxyPassword.toString());
				final String proxyLocalHostname = ReactorWatchlistRDPJunit.proxyLocalHostname.toString();
				final String proxyKRBConfigFile = ReactorWatchlistRDPJunit.proxyKRBConfigFile.toString();
				if (Objects.nonNull(proxyLocalHostname)) {
					connectOptions.credentialsInfo().HTTPproxyLocalHostname(proxyLocalHostname);
				} else if (Objects.nonNull(proxyKRBConfigFile)) {
					connectOptions.credentialsInfo().HTTPproxyKRB5configFile(proxyKRBConfigFile);
				}
			}
		}
	}

	@Test
	public void RDPConnectErrorInvalidConnectionTypeTest() {
		RDPConnectErrorInvalidConnectionType(false, null);
	}

	@Test
	public void RDPConnectErrorInvalidConnectionTypeTest_WebSocket_Rwf() {
		RDPConnectErrorInvalidConnectionType(true, "rssl.rwf");
	}

	@Test
	public void RDPConnectErrorInvalidConnectionTypeTest_WebSocket_Json() {
		RDPConnectErrorInvalidConnectionType(false, "tr_json2");
	}

	private void RDPConnectErrorInvalidConnectionType(boolean isWebsocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectErrorInvalidConnectionTypeTest <<<<<<<<<<\n");	
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
	public void RDPConnectErrorInvalidLocationTest() {
		RDPConnectErrorInvalidLocation(false, null);
	}

	@Test
	public void RDPConnectErrorInvalidLocationTest_WebSocket_Rwf() {
		RDPConnectErrorInvalidLocation(true, "rssl.rwf");
	}

	@Test
	public void RDPConnectErrorInvalidLocationTest_WebSocket_Json() {
		RDPConnectErrorInvalidLocation(true, "tr_json2");
	}

	private void RDPConnectErrorInvalidLocation(boolean isWebSocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectErrorInvalidLocationTest <<<<<<<<<<\n");	
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
	public void RDPConnectErrorAddressAndSessionManagmentSpecifiedTest() {
		RDPConnectErrorAddressAndSessionManagmentSpecified(false, null);
	}

	private void RDPConnectErrorAddressAndSessionManagmentSpecified(boolean isWebsocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectErrorAddressAndSessionManagmentSpecifiedTest <<<<<<<<<<\n");		
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
	public void RDPConnectTest() {
		RDPConnect(false, null);
	}

	@Test
	public void RDPConnectTest_WebSocket_Json() {
		RDPConnect(true, "tr_json2");
	}

	private void RDPConnect(boolean isWebsocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectTest <<<<<<<<<<\n");	
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
	public void RDPConnectConnectionListTest() {
		RDPConnectConnectionList(false, null);
	}

	@Test
	public void RDPConnectConnectionListTest_WebSocket_Json() {
		RDPConnectConnectionList(true, "tr_json2");
	}

	private void RDPConnectConnectionList(boolean isWebsocket, String protocolList)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectConnectionListTest <<<<<<<<<<\n");	
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

			verifyAuthTokenEvent(consumerReactor, 240, true, true);
			
			assertTrue("Expected Service Discovery request going out", chnlEvent._reactorChannel.reactorServiceEndpointInfoList().size() > 0);			

		}
		finally
		{
			consumerReactor.close();
		};
	}	

	@Test
	public void RDPConnectConnectionListErrorWrongCredentialsTest() {
		RDPConnectConnectionListErrorWrongCredentials(false, null);
	}

	@Test
	public void RDPConnectConnectionListErrorWrongCredentialsTest_WebSocket_Rwf() {
		RDPConnectConnectionListErrorWrongCredentials(true, "rssl.rwf");
	}

	@Test
	public void RDPConnectConnectionListErrorWrongCredentialsTest_WebSocket_Json() {
		RDPConnectConnectionListErrorWrongCredentials(true, "tr_json2");
	}

	private  void RDPConnectConnectionListErrorWrongCredentials(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectConnectionListErrorWrongCredentialsTest <<<<<<<<<<\n");	
		assumeTrue(checkCredentials());

		// request service discovery with valid user name / password to prevent being locked out 
		// from Refinitiv Data Platform because of too many invalid requests.
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
			setupProxyForConnectOptions(connectInfo.connectOptions());

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
				assertTrue(chnlEvent.errorInfo().error().text().contains("Connection refused: no further information"));

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
	public void RDPConnectConnectionListSecondConnectionRDPInvalidCredentialsTest() {
		RDPConnectConnectionListSecondConnectionRDPInvalidCredentials(false, null);
	}

	@Test
	public void RDPConnectConnectionListSecondConnectionRDPInvalidCredentialsTest_WebSocket_Rwf() {
		RDPConnectConnectionListSecondConnectionRDPInvalidCredentials(true, "rssl.rwf");
	}

	@Test
	public void RDPConnectConnectionListSecondConnectionRDPInvalidCredentialsTest_WebSocket_Json() {
		RDPConnectConnectionListSecondConnectionRDPInvalidCredentials(true, "tr_json2");
	}

	private void RDPConnectConnectionListSecondConnectionRDPInvalidCredentials(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectConnectionListSecondConnectionRDPInvalidCredentialsTest <<<<<<<<<<\n");	
		assumeTrue(checkCredentials());

		// request service discovery with valid user name / password to prevent being locked out 
		// from Refinitiv Data Platform because of too many invalid requests.
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
			setupProxyForConnectOptions(connectInfo.connectOptions());

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
			assertTrue(chnlEvent.errorInfo().error().text().contains("Connection refused: no further information"));
			
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
			assertTrue(chnlEvent.errorInfo().error().text().contains("Connection refused: no further information"));
			
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
	public void RDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlagTest() {
		RDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlag(false, null);
	}

	@Test
	public void RDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlagTest_WebSocket_Json() {
		RDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlag(true, "tr_json2");
	}

	private void RDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlag(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectWatchlistDisabledLoginReIssueSendWithRefreshFlagTest <<<<<<<<<<\n");	
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
			verifyAuthTokenEvent(consumerReactor, 240, true, true);
			
			// Checks for the second login reissue response from the worker thread
			verifyAuthTokenEvent(consumerReactor, 240, true, true);	

		}
		finally
		{
			consumerReactor.close();
		}		
	}	

	@Test
	public void RDPConnectWatchlistDisabledAddressAndPortEmptyTest() {
		RDPConnectWatchlistDisabledAddressAndPortEmpty(false, null);
	}

	@Test
	public void RDPConnectWatchlistDisabledAddressAndPortEmptyTest_WebSocket_Rwf() {
		RDPConnectWatchlistDisabledAddressAndPortEmpty(true, "rssl.rwf");
	}

	@Test
	public void RDPConnectWatchlistDisabledAddressAndPortEmptyTest_WebSocket_Json() {
		RDPConnectWatchlistDisabledAddressAndPortEmpty(true, "tr_json2");
	}

	private void RDPConnectWatchlistDisabledAddressAndPortEmpty(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectWatchlistDisabledAddressAndPortEmptyTest <<<<<<<<<<\n");			
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
	public void RDPConnectWatchlistDisabledAddressAndPortSetTest() {
		RDPConnectWatchlistDisabledAddressAndPortSet(false, null);
	}

	@Test
	public void RDPConnectWatchlistDisabledAddressAndPortSetTest_WebSocket_Rwf() {
		RDPConnectWatchlistDisabledAddressAndPortSet(true, "rssl.rwf");
	}

	@Test
	public void RDPConnectWatchlistDisabledAddressAndPortSetTest_WebSocket_Json() {
		RDPConnectWatchlistDisabledAddressAndPortSet(true, "tr_json2");
	}

	private void RDPConnectWatchlistDisabledAddressAndPortSet(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectWatchlistDisabledAddressAndPortSetTest <<<<<<<<<<\n");			
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

			// check if the connection info came from RDP
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address().equals("FAKE"));
			assertTrue(rcOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName().equals("FAKE"));
			
		}
		finally
		{
			consumerReactor.close();
		}	
	}	

	@Test
	public void RDPConnectWatchlistDisabledTest() {
		RDPConnectWatchlistDisabled(false, null);
	}

	@Test
	public void RDPConnectWatchlistDisabledTest_WebSocket_json() {
		RDPConnectWatchlistDisabled(true, "tr_json2");
	}

	private void RDPConnectWatchlistDisabled(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectWatchlistDisabledTest <<<<<<<<<<\n");	
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
	public void RDPConnectWatchlistDisabledConnectionListTest() {
		RDPConnectWatchlistDisabledConnectionList(false, null);
	}

	@Test
	public void RDPConnectWatchlistDisabledConnectionListTest_WebSocket_json() {
		RDPConnectWatchlistDisabledConnectionList(true, "tr_json2");
	}

	private void RDPConnectWatchlistDisabledConnectionList(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectWatchlistDisabledConnectionListTest <<<<<<<<<<\n");	
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
			connectInfo.initTimeout(40);
			connectInfo.enableSessionManagement(false);		
			setupProxyForConnectOptions(connectInfo.connectOptions());

			ReactorConnectOptions rcOpts = createDefaultConsumerConnectOptionsForSessionManagment(consumer, isWebsocket, protocol);

			ReactorConnectInfo connectInfoSecond = rcOpts.connectionList().remove(0);		

			rcOpts.connectionList().add(connectInfo);
			rcOpts.connectionList().add(connectInfoSecond);

			rcOpts.connectionList().get(1).reactorAuthTokenEventCallback(consumer);				

			assertTrue("Expected SUCCESS", consumerReactor._reactor.connect(rcOpts, consumerRole, errorInfo) == ReactorReturnCodes.SUCCESS);
			
			consumer.testReactor().dispatch(-1, 8000);			

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
				Thread.sleep(5 * 1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}		

			int sleep = verifyAuthTokenEvent(consumerReactor, 10, true, true);
			long runtime = System.currentTimeMillis() + ((sleep - 5) * 1000);		        


			try {
				Thread.sleep(10 * 1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}			

			event = consumerReactor.pollEvent();
			assertNotNull("Did not receive CHANNEL_EVENT", event);
			assertEquals("Expected TestReactorEventTypes.CHANNEL_EVENT, received: " + event.type(), TestReactorEventTypes.CHANNEL_EVENT, event.type());
			chnlEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals("Expected ReactorChannelEventTypes.CHANNEL_UP, received: " + chnlEvent.eventType(), ReactorChannelEventTypes.CHANNEL_UP, chnlEvent.eventType());	        


			try {
				Thread.sleep(10 * 1000);
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
	public void RDPConnectWatchlistDisabledErrorNoRDMLoginTest() {
		RDPConnectWatchlistDisabledErrorNoRDMLogin(false, null);
	}

	@Test
	public void RDPConnectWatchlistDisabledErrorNoRDMLoginTest_WebSocket_Json() {
		RDPConnectWatchlistDisabledErrorNoRDMLogin(true, "rssl.rwf");
	}

	private void RDPConnectWatchlistDisabledErrorNoRDMLogin(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectWatchlistDisabledErrorNoRDMLoginTest <<<<<<<<<<\n");	
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
	public void RDPConnectUserSpecifiedClientIdBySpecifyingHostAndPortTest(){
		RDPConnectUserSpecifiedClientIdBySpecifyingHostAndPort(false, null);
	}

	@SuppressWarnings("deprecation")
	@Test
	public void RDPConnectUserSpecifiedClientIdBySpecifyingHostAndPortTest_WebSocket_Rwf(){
		RDPConnectUserSpecifiedClientIdBySpecifyingHostAndPort(true, "rssl.rwf");
	}

	@SuppressWarnings("deprecation")
	@Test
	public void RDPConnectUserSpecifiedClientIdBySpecifyingHostAndPortTest_WebSocket_Json(){
		RDPConnectUserSpecifiedClientIdBySpecifyingHostAndPort(true, "tr_json2");
	}

	@SuppressWarnings("deprecation")
	private void RDPConnectUserSpecifiedClientIdBySpecifyingHostAndPort(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectUserSpecifiedClientIdBySpecifyingHostAndPortTest <<<<<<<<<<\n");	
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

			ReactorServiceEndpointEventCallbackTest callback = new ReactorServiceEndpointEventCallbackTest()
			{				
				@Override
				public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event) {
					
					for (ReactorServiceEndpointInfo info : event.serviceEndpointInfo())
					{
						if( info.locationList().toString().contains("us-east") )
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
	public void RDPConnectUserSpecifiedClientIdTest() {
		RDPConnectUserSpecifiedClientId(false, null);
	}

	@SuppressWarnings("deprecation")
	@Test
	public void RDPConnectUserSpecifiedClientIdTest_Websocket_Rwf() {
		RDPConnectUserSpecifiedClientId(true, "rssl.rwf");
	}

	@SuppressWarnings("deprecation")
	@Test
	public void RDPConnectUserSpecifiedClientIdTest_Websocket_Json() {
		RDPConnectUserSpecifiedClientId(true, "tr_json2");
	}

	private void RDPConnectUserSpecifiedClientId(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectUserSpecifiedClientIdTest <<<<<<<<<<\n");	
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
	public void RDPConnectErrorInvalidClientIdTest() {
		RDPConnectErrorInvalidClientId(false, null);
	}

	@SuppressWarnings("deprecation")
	@Test
	public void RDPConnectErrorInvalidClientIdTest_WebSocket_Rwf() {
		RDPConnectErrorInvalidClientId(true, "rssl.rwf");
	}

	@SuppressWarnings("deprecation")
	@Test
	public void RDPConnectErrorInvalidClientIdTest_WebSocket_Json() {
		RDPConnectErrorInvalidClientId(true, "tr_json2");
	}

	@SuppressWarnings("deprecation")
	private void RDPConnectErrorInvalidClientId(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectErrorInvalidClientIdTest <<<<<<<<<<\n");	
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
	public void RDPConnectErrorIncorrectCredentialsTest() {
		RDPConnectErrorIncorrectCredentials(false, null);
	}

	@SuppressWarnings("deprecation")
	@Test
	public void RDPConnectErrorIncorrectCredentialsTest_WebSocket_Rwf() {
		RDPConnectErrorIncorrectCredentials(true, "rssl.rwf");
	}

	@SuppressWarnings("deprecation")
	@Test
	public void RDPConnectErrorIncorrectCredentialsTest_WebSocket_Json() {
		RDPConnectErrorIncorrectCredentials(true, "tr_json2");
	}

	@SuppressWarnings("deprecation")
	private void RDPConnectErrorIncorrectCredentials(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectErrorIncorrectCredentialsTest <<<<<<<<<<\n");	
		assumeTrue(checkCredentials());

		// request service discovery with valid user name / password to prevent being locked out 
		// from Refinitiv Data Platform because of too many invalid requests.
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
	public void RDPConnectErrorInvalidTokenServiceURLTest() {
		RDPConnectErrorInvalidTokenServiceURL(false, null);
	}

	@Test
	public void RDPConnectErrorInvalidTokenServiceURLTest_WebSocket_Rwf() {
		RDPConnectErrorInvalidTokenServiceURL(true, "rssl.rwf");
	}

	@Test
	public void RDPConnectErrorInvalidTokenServiceURLTest_WebSocket_Json() {
		RDPConnectErrorInvalidTokenServiceURL(true, "tr_json2");
	}

	private void RDPConnectErrorInvalidTokenServiceURL(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectErrorInvalidTokenServiceURLTest <<<<<<<<<<\n");	
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
	public void RDPConnectUserSpecifiedTokenServiceUrlTest() {
		RDPConnectUserSpecifiedTokenServiceUrl(false, null);
	}

	@Test
	public void RDPConnectUserSpecifiedTokenServiceUrlTest_WebSocket_Rwf() {
		RDPConnectUserSpecifiedTokenServiceUrl(true, "rssl.rwf");
	}

	@Test
	public void RDPConnectUserSpecifiedTokenServiceUrlTest_WebSocket_Json() {
		RDPConnectUserSpecifiedTokenServiceUrl(true, "tr_json2");
	}

	private void RDPConnectUserSpecifiedTokenServiceUrl(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectUserSpecifiedTokenServiceUrlTest <<<<<<<<<<\n");	
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
	public void RDPConnectUserSpecifiedServiceDiscoveryUrlTest() {
		RDPConnectUserSpecifiedServiceDiscoveryUrl(false, null);
	}

	@Test
	public void RDPConnectUserSpecifiedServiceDiscoveryUrlTest_WebSocket_Rwf() {
		RDPConnectUserSpecifiedServiceDiscoveryUrl(true, "rssl.rwf");
	}

	@Test
	public void RDPConnectUserSpecifiedServiceDiscoveryUrlTest_WebSocket_Json() {
		RDPConnectUserSpecifiedServiceDiscoveryUrl(true, "tr_json2");
	}

	private void RDPConnectUserSpecifiedServiceDiscoveryUrl(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectUserSpecifiedServiceDiscoveryUrlTest <<<<<<<<<<\n");	
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
	public void RDPConnectErrorInvalidServiceDiscoveryURLTest() {
		RDPConnectErrorInvalidServiceDiscoveryURL(false, null);
	}

	@Test
	public void RDPConnectErrorInvalidServiceDiscoveryURLTest_WebSocket_Rwf() {
		RDPConnectErrorInvalidServiceDiscoveryURL(true, "rssl.rwf");
	}

	@Test
	public void RDPConnectErrorInvalidServiceDiscoveryURLTest_WebSocket_Json() {
		RDPConnectErrorInvalidServiceDiscoveryURL(true, "tr_json2");
	}

	private void RDPConnectErrorInvalidServiceDiscoveryURL(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectErrorInvalidServiceDiscoveryURLTest <<<<<<<<<<\n");	
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
	public void RDPQueryServiceDiscoveryErrorInvalidDataFormatAndTransportTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryErrorInvalidDataFormatAndTransportTest <<<<<<<<<<\n");	
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
	public void RDPQueryServiceDiscoveryErrorCallbackMissingTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryErrorCallbackMissingTest <<<<<<<<<<\n");			
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
	public void RDPQueryServiceDiscoveryErrorErrorInfoMissingTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryErrorErrorInfoMissingTest <<<<<<<<<<\n");	
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
	public void RDPQueryServiceDiscoveryErrorServiceDiscoveryOptionsMissingTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryErrorServiceDiscoveryOptionsMissingTest <<<<<<<<<<\n");	
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
	public void RDPQueryServiceDiscoveryMissingClientIdTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryMissingClientIdTest <<<<<<<<<<\n");	
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
	public void RDPQueryServiceDiscoveryErrorMismatchOfTypesTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryErrorMismatchOfTypesTest <<<<<<<<<<\n");	
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
					// since there is a mismatch of the dataformat and protocol the RDP will respond with 404 error Not found.
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
	public void RDPQueryServiceDiscoveryJSONTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryJSONTest <<<<<<<<<<\n");	
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
	public void RDPQueryServiceDiscoveryErrorPasswordTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryErrorPasswordTest <<<<<<<<<<\n");	
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
	public void RDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnectTest() {
		RDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnect(false, null);
	}

	@Test
	public void RDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnectTest_WebSocket_Rwf() {
		RDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnect(true, "rssl.rwf");
	}

	@Test
	public void RDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnectTest_WebSocket_Json() {
		RDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnect(true, "tr_json2");
	}

	private void RDPQueryServiceDiscoveryErrorUserNamePasswordAndSuccessConnect(boolean isWebsocket, String protocol)
	{
		
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryErrorPasswordAndSuccessConnectTest <<<<<<<<<<\n");	
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
					String tmp = "{\"error\":\"invalid_client\"  ,\"error_description\":\"Invalid Application Credential.\" }";
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
	public void RDPQueryServiceDiscoveryErrorPasswordAndSuccessConnectTest() {
		RDPQueryServiceDiscoveryErrorPasswordAndSuccessConnect(false, null);
	}

	@Test
	public void RDPQueryServiceDiscoveryErrorPasswordAndSuccessConnectTest_WebSocket_Rwf() {
		RDPQueryServiceDiscoveryErrorPasswordAndSuccessConnect(true, "rssl.rwf");
	}

	@Test
	public void RDPQueryServiceDiscoveryErrorPasswordAndSuccessConnectTest_WebSocket_Json() {
		RDPQueryServiceDiscoveryErrorPasswordAndSuccessConnect(true, "tr_json2");
	}

	private void RDPQueryServiceDiscoveryErrorPasswordAndSuccessConnect(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryErrorPasswordAndSuccessConnectTest <<<<<<<<<<\n");	
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
	public void RDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnectTest() {
		RDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnect(false, null);
	}

	@Test
	public void RDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnectTest_WebSocket_Rwf() {
		RDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnect(true, "rssl.rwf");
	}

	@Test
	public void RDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnectTest_WebSocket_Json() {
		RDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnect(true, "tr_json2");
	}

	private void RDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnect(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryErrorInvalidClientIdAndSuccessConnectTest <<<<<<<<<<\n");	
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
	public void RDPQueryServiceDiscoveryNoDataFormatProtocolTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryNoDataFormatProtocolTest <<<<<<<<<<\n");	
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
	public void RDPQueryServiceDiscoveryNoTransportProtocolTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryNoTransportProtocolTest <<<<<<<<<<\n");
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

	@Test
	public void RDPQueryServiceDiscoveryTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPQueryServiceDiscoveryTest <<<<<<<<<<\n");	
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
	public void RDPNoOAuthCredentialForEnablingSessionMgnt()
	{
		System.out.println("\n>>>>>>>>> Running RDPNoOAuthCredentialForEnablingSessionMgnt <<<<<<<<<<\n");	
		
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
	public void RDPNoUserNameForEnablingSessionMgnt()
	{
		System.out.println("\n>>>>>>>>> Running RDPNoUserNameForEnablingSessionMgnt <<<<<<<<<<\n");	
		
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
	public void RDPNoPasswordForEnablingSessionMgnt()
	{
		System.out.println("\n>>>>>>>>> Running RDPNoPasswordForEnablingSessionMgnt <<<<<<<<<<\n");	
		
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
	public void RDPNoClientIDForEnablingSessionMgnt()
	{
		System.out.println("\n>>>>>>>>> Running RDPNoClientIDForEnablingSessionMgnt <<<<<<<<<<\n");	
		
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
	public void RDPMultipleOpenConnections_SameUser_Diff_CallbackTest() {
		RDPMultipleOpenConnections_SameUser_Diff_Callback(false, null);
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_CallbackTest_WebSocket_Rwf() {
		RDPMultipleOpenConnections_SameUser_Diff_Callback(true, "rssl.rwf");
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_CallbackTest_WebSocket_Json() {
		RDPMultipleOpenConnections_SameUser_Diff_Callback(true, "tr_json2");
	}

	private void RDPMultipleOpenConnections_SameUser_Diff_Callback(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPMultipleOpenConnections_SameUser_Diff_CallbackTest <<<<<<<<<<\n");	
		
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
	public void RDPMultipleOpenConnections_SameUser_Diff_NULL_CallbackTest() {
		RDPMultipleOpenConnections_SameUser_Diff_NULL_Callback(false, null);
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_NULL_CallbackTest_WebSocket_Rwf() {
		RDPMultipleOpenConnections_SameUser_Diff_NULL_Callback(true, "rssl.rwf");
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_NULL_CallbackTest_WebSocket_Json() {
		RDPMultipleOpenConnections_SameUser_Diff_NULL_Callback(true, "tr_json2");
	}

	private void RDPMultipleOpenConnections_SameUser_Diff_NULL_Callback(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPMultipleOpenConnections_SameUser_Diff_NULL_CallbackTest <<<<<<<<<<\n");	
		
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
	public void RDPMultipleOpenConnections_SameUser_Diff_ClientIDTest() {
		RDPMultipleOpenConnections_SameUser_Diff_ClientID(false, null);
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_ClientIDTest_WebSocket_Rwf() {
		RDPMultipleOpenConnections_SameUser_Diff_ClientID(true, "rssl.rwf");
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_ClientIDTest_WebSocket_Json() {
		RDPMultipleOpenConnections_SameUser_Diff_ClientID(true, "tr_json2");
	}

	private void RDPMultipleOpenConnections_SameUser_Diff_ClientID(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPMultipleOpenConnections_SameUser_Diff_ClientIDTest <<<<<<<<<<\n");	
		
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
	public void RDPMultipleOpenConnections_SameUser_Diff_ClientSecretTest() {
		RDPMultipleOpenConnections_SameUser_Diff_ClientSecret(false, null);
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_ClientSecretTest_WebSocket_Rwf() {
		RDPMultipleOpenConnections_SameUser_Diff_ClientSecret(true, "rssl.rwf");
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_ClientSecretTest_WebSocket_Json() {
		RDPMultipleOpenConnections_SameUser_Diff_ClientSecret(true, "tr_json2");
	}

	private void RDPMultipleOpenConnections_SameUser_Diff_ClientSecret(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPMultipleOpenConnections_SameUser_Diff_ClientSecretTest <<<<<<<<<<\n");	
		
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
			
			oAuthCredential.clientSecret().data("ClientSecret1");
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
			
			oAuthCredential.clientSecret().data("ClientSecret2");
			consumerRole.reactorOAuthCredential(oAuthCredential);
			
			assertTrue("Expected INVALID_USAGE", consumerReactor._reactor.connect(rcOpts, consumer.reactorRole(), errorInfo) == ReactorReturnCodes.INVALID_USAGE);
			assertEquals("Expected error text", "The Client secret of ReactorOAuthCredential is not equal for the existing token session.", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_PasswordTest() {
		RDPMultipleOpenConnections_SameUser_Diff_Password(false, null);
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_PasswordTest_WebSocket_Rwf() {
		RDPMultipleOpenConnections_SameUser_Diff_Password(true, "rssl.rwf");
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_PasswordTest_WebSocket_Json() {
		RDPMultipleOpenConnections_SameUser_Diff_Password(true, "tr_json2");
	}

	private void RDPMultipleOpenConnections_SameUser_Diff_Password(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPMultipleOpenConnections_SameUser_Diff_PasswordTest <<<<<<<<<<\n");	
		
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
	public void RDPMultipleOpenConnections_SameUser_Diff_TokenScopeTest() {
		RDPMultipleOpenConnections_SameUser_Diff_TokenScope(false, null);
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_TokenScopeTest_WebSocket_Rwf() {
		RDPMultipleOpenConnections_SameUser_Diff_TokenScope(true, "rssl.rwf");
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_TokenScopeTest_WebSocket_Json() {
		RDPMultipleOpenConnections_SameUser_Diff_TokenScope(true, "tr_json2");
	}

	private void RDPMultipleOpenConnections_SameUser_Diff_TokenScope(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPMultipleOpenConnections_SameUser_Diff_TokenScopeTest <<<<<<<<<<\n");	
		
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
	public void RDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControlTest() {
		RDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControl(false, null);
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControlTest_WebSocket_Rwf() {
		RDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControl(true, "rssl.rwf");
	}

	@Test
	public void RDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControlTest_WebSocket_Json() {
		RDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControl(true, "tr_json2");
	}

	private void RDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControl(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPMultipleOpenConnections_SameUser_Diff_TakeExclusiveSignOnControlTest <<<<<<<<<<\n");	
		
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
	public void RDPConnectionUsingReactorOAuthCredentialOnlyTest() {
		RDPConnectionUsingReactorOAuthCredentialOnly(false, null);
	}

	@Test
	public void RDPConnectionUsingReactorOAuthCredentialOnlyTest_WebSocket_Json() {
		RDPConnectionUsingReactorOAuthCredentialOnly(true, "tr_json2");
	}

	private void RDPConnectionUsingReactorOAuthCredentialOnly(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectionUsingReactorOAuthCredentialOnlyTest <<<<<<<<<<\n");	
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
	public void RDPConnectionUsingReactorOAuthCredentialOnly_WatchListEnabledTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPConnectionUsingReactorOAuthCredentialOnly_WatchListEnabledTest <<<<<<<<<<\n");	
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
	public void RDPMultiConnectionsConnectAndCloseWithSameCredentialTest() {
		RDPMultiConnectionsConnectAndCloseWithSameCredential(false, null);
	}

	@Test
	public void RDPMultiConnectionsConnectAndCloseWithSameCredentialTest_WebSocket_Json() {
		RDPMultiConnectionsConnectAndCloseWithSameCredential(true, "tr_json2");
	}

	private void RDPMultiConnectionsConnectAndCloseWithSameCredential(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPMultiConnectionsConnectAndCloseWithSameCredentialTest <<<<<<<<<<\n");	
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
			assertTrue("Checks the number of AuthTokenEvent", consumerReactor._countAuthTokenEventCallbackCalls == 6);
			
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
	public void RDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnlyTest() {
		RDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnly(false, null);
	}

	@Test
	public void RDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnlyTest_Websocket_Json() {
		RDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnly(true, "tr_json2");
	}

	private void RDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnly(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPMultiConnectionsConnectAndCloseWithSameCredential_Using_ReactorOAuthCredentialOnlyTest <<<<<<<<<<\n");	
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
			setupConsumer(consumer, false);
			
			Consumer consumer2 = new Consumer(consumerReactor);
			setupConsumer(consumer2, false);
			
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
			assertTrue("Checks the number of AuthTokenEvent", consumerReactor._countAuthTokenEventCallbackCalls == 6);
			
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
	public void RDPSubmitTokenRenewalWithoutTokenSessionAndCallbackTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPSubmitTokenRenewalWithoutTokenSessionAndCallbackTest <<<<<<<<<<\n");	
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
	public void RDPSubmitTokenRenewalUsingInvalidCredential_WithoutTokenSessionTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPSubmitTokenRenewalUsingInvalidCredential_WithoutTokenSessionTest <<<<<<<<<<\n");	
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
			assertEquals("Checking error text", "Failed to request authentication token information. Text: {\"error\":\"invalid_client\"  ,\"error_description\":\"Invalid Application Credential.\" } ", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void RDPSubmitTokenRenewalTakeExclusiveSignOnOff_WithoutTokenSessionTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPSubmitTokenRenewalTakeExclusiveSignOnOff_WithoutTokenSessionTest <<<<<<<<<<\n");
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
			assertEquals("Checking error text", "Failed to request authentication token information. Text: {\"error\":\"access_denied\"  ,\"error_description\":\"Session quota is reached.\" } ", errorInfo.error().text());
		}
		finally
		{
			consumerReactor.close();
		}
	}
	
	@Test
	public void RDPSubmitTokenRenewalWithoutTokenSessionTest()
	{
		System.out.println("\n>>>>>>>>> Running RDPSubmitTokenRenewalWithoutTokenSessionTest <<<<<<<<<<\n");	
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
	public void RDPSubmitCredentialInCallbackTest() {
		RDPSubmitCredentialInCallback(false, null);
	}

	@SuppressWarnings("deprecation")
	@Test
	public void RDPSubmitCredentialInCallbackTest_WebSocket_Json() {
		RDPSubmitCredentialInCallback(true, "tr_json2");
	}

	@SuppressWarnings("deprecation")
	private void RDPSubmitCredentialInCallback(boolean isWebsocket, String protocol)
	{
		System.out.println("\n>>>>>>>>> Running RDPSubmitCredentialInCallbackTest <<<<<<<<<<\n");	
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
				
				authOptions.username(loginRequest.userName().toString());
				authOptions.clientId(consumerRole.clientId().toString());
				authOptions.password(loginRequest.password().toString());
				
				assertTrue("Expected SUCCESS", restClient.getAuthAccessTokenInfo(authOptions, restConnectOptions, authTokenInfo, true, errorInfo) == ReactorReturnCodes.SUCCESS);
			}
			
			for(int i = 1 ;i < 90; i++)
			{
				consumerReactor.dispatch(-1, 1000);
			}
			
			/* 3 success and 1 fails */
			assertTrue("Checks the number of AuthTokenEvent", consumerReactor._countAuthTokenEventCallbackCalls == 4);

		}
		finally
		{
			consumerReactor.close();
		}
	}
}
