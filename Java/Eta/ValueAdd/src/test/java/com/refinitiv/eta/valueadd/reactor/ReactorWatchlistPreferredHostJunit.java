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

import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.ArrayList;
import java.util.LinkedList;
import java.util.List;
import java.util.Objects;
import java.util.Optional;

import org.junit.Before;
import org.junit.Test;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Login.ServerTypes;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryConsumerStatus;
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

public class ReactorWatchlistPreferredHostJunit {
	private static final Buffer proxyHost = CodecFactory.createBuffer();
	private static final Buffer proxyPort = CodecFactory.createBuffer();
	private static final Buffer proxyUser = CodecFactory.createBuffer();
	private static final Buffer proxyPassword = CodecFactory.createBuffer();
	private static final Buffer proxyLocalHostname = CodecFactory.createBuffer();
	private static final Buffer proxyDomain = CodecFactory.createBuffer();

	public ReactorWatchlistPreferredHostJunit() {
		proxyHost.data(System.getProperty("proxyHost"));
		proxyPort.data(System.getProperty("proxyPort"));
		proxyUser.data(System.getProperty("proxyUser"));
		proxyPassword.data(System.getProperty("proxyPassword"));
		proxyLocalHostname.data(System.getProperty("proxyLocalHostname"));
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
			SelectableChannel tmpChannel;


			switch (eventType)
			{
			case ReactorChannelEventTypes.CHANNEL_UP:
				_channelUpEventCount++;
				// register this new reactorChannel for OP_READ with the selector
				try
				{
					for(int i = 0; i < rc.warmStandbyChannelInfo().oldSelectableChannelList().size(); i++)
					{
						tmpChannel = rc.warmStandbyChannelInfo().oldSelectableChannelList().get(i);
						
						if(!rc.warmStandbyChannelInfo().selectableChannelList().contains(tmpChannel))
						{
							SelectionKey removeKey = tmpChannel.keyFor(_selector);
							
							if(removeKey != null)
								removeKey.cancel();
						}
					}
					
					for(int i = 0; i < rc.warmStandbyChannelInfo().oldSelectableChannelList().size(); i++)
					{
						tmpChannel = rc.warmStandbyChannelInfo().oldSelectableChannelList().get(i);
						
						if(tmpChannel.keyFor(_selector) == null)
						{
							tmpChannel.register(_selector, SelectionKey.OP_READ, event.reactorChannel());
						}
					}
					
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

				for(int i = 0; i < rc.warmStandbyChannelInfo().oldSelectableChannelList().size(); i++)
				{
					tmpChannel = rc.warmStandbyChannelInfo().oldSelectableChannelList().get(i);
					
					if(!rc.warmStandbyChannelInfo().selectableChannelList().contains(tmpChannel))
					{
						SelectionKey removeKey = tmpChannel.keyFor(_selector);
						
						if(removeKey != null)
							removeKey.cancel();
					}
				}

				break;
			case ReactorChannelEventTypes.CHANNEL_READY:
				_channelReadyEventCount++;
				break;
			case ReactorChannelEventTypes.FD_CHANGE:
				try
				{
					for(int i = 0; i < rc.warmStandbyChannelInfo().oldSelectableChannelList().size(); i++)
					{
						tmpChannel = rc.warmStandbyChannelInfo().oldSelectableChannelList().get(i);
						
						if(!rc.warmStandbyChannelInfo().selectableChannelList().contains(tmpChannel))
						{
							SelectionKey removeKey = tmpChannel.keyFor(_selector);
							
							if(removeKey != null)
								removeKey.cancel();
						}
					}
					
					for(int i = 0; i < rc.warmStandbyChannelInfo().oldSelectableChannelList().size(); i++)
					{
						tmpChannel = rc.warmStandbyChannelInfo().oldSelectableChannelList().get(i);
						
						if(tmpChannel.keyFor(_selector) == null)
						{
							tmpChannel.register(_selector, SelectionKey.OP_READ, event.reactorChannel());
						}
					}
					
					rc.channel().selectableChannel().register(_selector, SelectionKey.OP_READ, rc);
				}
				catch (ClosedChannelException e)
				{
					assertTrue("Exception occurred, " + e.getLocalizedMessage(), false);
				}
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
	
	void checkChannelDownReconnecting(Consumer consumer, int port, int sleepTime, boolean switchReactorChannel)
	{
		checkChannelDownReconnecting(consumer, port, sleepTime, switchReactorChannel, 1);
	}
	
	void checkChannelDownReconnecting(Consumer consumer, int port, int sleepTime, boolean switchReactorChannel, int reconnectAttemptLimit)
	{
		
		for (int i = 0; i < reconnectAttemptLimit; i++)
		{
			try {
				Thread.sleep(sleepTime);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			consumer.testReactor().switchingReactorChannel = switchReactorChannel;
			
			consumer.testReactor().dispatch(1);
			/* Channel down reconnecting */
			TestReactorEvent event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			int compare = Integer.valueOf(channelEvent.reactorChannel().reactor()._reactorChannelQueue.peek().getCurrentReactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName());
			assertEquals("ChannelDownReconnecting check failed. ", port, compare);
			
		}
	}

	@Test
	public void PreferredHostConnectionListConnectionUpTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionListConnectionUpTest <<<<<<<<<<\n");

		// 3 Configured and running providers, with 3rd being the preferred host.
		// Connect first to preferred host instead of others
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		int port1;
		int port2;
		int port3;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);

			port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);

			port2 = provider2.bindGetPort(opts);

			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			port3 = provider3.bindGetPort(opts);

			// Set preferred host options, with connection list index to 2
			ReactorPreferredHostOptions preferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
			preferredHostOptions.connectionListIndex(2);
			preferredHostOptions.isPreferredHostEnabled(true);
			opts.preferredHostOptions(preferredHostOptions);
			
			int[] ports = {port1, port2, port3};
			consumerReactor.connectList(opts, consumer, ports);
			
			consumer.testReactor().dispatch(0);
			
			provider3.testReactor().accept(opts, provider3);

			/* Provider receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider receives consumer directory request. */
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}

	@Test
	public void PreferredHostConnectionList_InvalidServerFunctionCallTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_InvalidServerFunctionCallTest <<<<<<<<<<\n");

		// 3 Configured and running providers, with 3rd being the preferred host.
		// Connect first to preferred host instead of others
		// Only Provider 3 is active
		// Ioctl changes preferred index to 0
		// Call fallback
		// Switchover shouldn't happen and we should receive appropriate FALLBACK_DONE message
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		int port1;
		int port2;
		int port3;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);

			port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);

			port2 = provider2.bindGetPort(opts);

			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			port3 = provider3.bindGetPort(opts);

			// Set preferred host options, with connection list index to 2
			ReactorPreferredHostOptions preferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
			preferredHostOptions.connectionListIndex(2);
			preferredHostOptions.isPreferredHostEnabled(true);
			opts.preferredHostOptions(preferredHostOptions);
			
			int[] ports = {port1, port2, port3};
			consumerReactor.connectList(opts, consumer, ports);
			
			consumer.testReactor().dispatch(0);
			
			provider3.testReactor().accept(opts, provider3);

			/* Provider receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider receives consumer directory request. */
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		// Disconnect Providers 1 and 2
    		provider.close();
    		provider2.close();
    		
    		// Call ioctl to change preferred index to 0
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.connectionListIndex(0);
    		consumer.reactorChannel().ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, consumer.reactorChannel().getEDPErrorInfo());
    		
    		// Wait a moment for ioctl to kick in.
    		
    		try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    		
    		// Call fallback method
    		consumer.reactorChannel().fallbackPreferredHost(null);
    		
    		/* Consumer receives PREFERRED HOST COMPLETE. */
    		consumer.testReactor().dispatch(1);
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());
    		
    		
		}
		finally
		{
	       consumer.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}

	@Test
	public void PreferredHostConnectionList_PH_Down_After_ConnectingTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_PH_Down_After_ConnectingTest <<<<<<<<<<\n");

		// 3 Configured and running providers, with 3rd being the preferred host.
		// Connect first to preferred host instead of others
		// Bring down the preferred host, reconnection should occur to provider 1.
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		int port1;
		int port2;
		int port3;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			port2 = provider2.bindGetPort(opts);

			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			port3 = provider3.bindGetPort(opts);

			// Set preferred host options, with connection list index to 2
			ReactorPreferredHostOptions preferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
			preferredHostOptions.connectionListIndex(2);
			preferredHostOptions.isPreferredHostEnabled(true);
			opts.preferredHostOptions(preferredHostOptions);
			
			int[] ports = {port1, port2, port3};
			consumerReactor.connectList(opts, consumer, ports);
			
			consumer.testReactor().dispatch(0);
			
			provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider receives consumer directory request. */
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Bring down provider 3. */
    		provider3.close();
    		providerReactor3.close();
    		
    		consumer.testReactor().dispatch(3);
    		
    		/* Channel down reconnecting */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
    		
    		/* Login Status Suspect Open */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)loginMsgEvent.msg()).state().text().toString());
            
            /* Consumer recieves directory update */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    		
			consumer.testReactor().dispatch(0);
			
			provider.testReactor().accept(opts, provider);

			/* Provider 1 receives channel-up/channel-ready */
			provider.testReactor().dispatch(2);
    		

			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

	        loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
	        loginRefresh.applyHasAttrib();
	        //loginRefresh.attrib().applyHasSingleOpen();
	        //loginRefresh.attrib().singleOpen(loginRequest.attrib().singleOpen());
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
			

			/* Provider receives consumer directory request. */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}
	@Test
	public void PreferredHostConnectionList_ioctlInvalidOptionsTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_ioctlInvalidOptionsTest <<<<<<<<<<\n");

		// 3 Configured and running providers, with 1st being the preferred host.
		// Bring up only Provider 1 and 2, not 3
		// Connect first to Provider 1
		// Consumer calls ioctl with each possible option one at a time with invalid input and expected results
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		int port1;
		int port2;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			port2 = provider2.bindGetPort(opts);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
    		
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);

			// Set preferred host options, with connection list index to 0
			ReactorPreferredHostOptions preferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
			preferredHostOptions.connectionListIndex(0);
			preferredHostOptions.isPreferredHostEnabled(true);
			opts.preferredHostOptions(preferredHostOptions);
			
			int[] ports = {port1, port2, port2 + 1};
			consumerReactor.connectList(opts, consumer, ports);
			
			consumer.testReactor().dispatch(0);
			
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}

    		/* Bring up provider 3. */
    		provider3.bindGetPort(opts);

			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			
			/* Provider receives consumer directory request. */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		// Consumer calls ioctl to set detectionTimeInterval to -1
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.detectionTimeInterval(-1);
    		ioctlCall.isPreferredHostEnabled(true);
    		int ret = consumer.reactorChannel().ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, consumer.reactorChannel().getEDPErrorInfo());
    		assertEquals(ReactorReturnCodes.FAILURE, ret);
    		
    		// Consumer calls ioctl to set detectionTimeSchedule to and invalid input
    		ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.detectionTimeSchedule("This can't be right");
    		ioctlCall.isPreferredHostEnabled(true);
    		ret = consumer.reactorChannel().ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, consumer.reactorChannel().getEDPErrorInfo());
    		assertEquals(ReactorReturnCodes.FAILURE, ret);
    		
    		// Consumer calls ioctl to set connectionListIndex to -1
    		ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.connectionListIndex(-1);
    		ioctlCall.isPreferredHostEnabled(true);
    		ret = consumer.reactorChannel().ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, consumer.reactorChannel().getEDPErrorInfo());
    		assertEquals(ReactorReturnCodes.FAILURE, ret);
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostConnectionList_ioctlDetectionTimeIntervalTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_ioctlDetectionTimeIntervalTest <<<<<<<<<<\n");

		// 3 Configured and running providers, with 1st being the preferred host.
		// Bring up only Provider 1 and 2, not 3 (later preferred host)
		// Connect first to Provider 1
		// Bring up provider 3 (later preferred host)
		// Consumer calls ioctl to set detectionTimeInterval to 5 and preferred host to index 2.
		// Consumer should reconnect to provider shortly after.
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		int port1;
		int port2;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			port2 = provider2.bindGetPort(opts);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
    		
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);

			// Set preferred host options, with connection list index to 0
			ReactorPreferredHostOptions preferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
			preferredHostOptions.connectionListIndex(0);
			preferredHostOptions.isPreferredHostEnabled(true);
			opts.preferredHostOptions(preferredHostOptions);
			
			int[] ports = {port1, port2, port2 + 1};
			consumerReactor.connectList(opts, consumer, ports);
			
			consumer.testReactor().dispatch(0);
			
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}

    		/* Bring up provider 3. */
    		provider3.bindGetPort(opts);

			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			
			/* Provider receives consumer directory request. */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		// Consumer calls ioctl to set detectionTimeInterval to 5 and connectionListIndex to 2.
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.detectionTimeInterval(5);
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.connectionListIndex(2);
    		consumer.reactorChannel().ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, consumer.reactorChannel().getEDPErrorInfo());
    		
    		// Wait up to 5 seconds (with some buffer time) for ioctl to kick in and detectionTimeInterval to occur.
    		
    		try {
				Thread.sleep(5100);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			provider3.testReactor().accept(opts, provider3);
			
    		consumer.testReactor().dispatch(5);

    		/* Login Status Suspect Open */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)loginMsgEvent.msg()).state().text().toString());
            
            /* Consumer receives directory update */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
    		/* Channel down reconnecting */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			/* Consumer gets channel up event from new connection to preferred host (provider 3) */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			/* Consumer shows preferred host complete */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());
    		
    		// Provider 1 can receive channel down, though unit test framework may be unreliable
    		provider.testReactor().dispatch(-1);
    		
			event = provider.testReactor().pollEvent();
			if (event != null)
			{
				assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
				channelEvent = (ReactorChannelEvent)event.reactorEvent();
				assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());	
				System.out.println("Old provider received channel down");
			}
			
			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(3);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());

			/* Provider 3 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives consumer directory request. */
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider 3 sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostConnectionList_ioctlDetectionTimeScheduleTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_ioctlDetectionTimeScheduleTest <<<<<<<<<<\n");

		// 3 Configured and running providers, with 1st being the preferred host.
		// Bring up only Provider 1 and 2, not 3 (later preferred host)
		// Connect first to Provider 1
		// Bring up provider 3 (later preferred host)
		// Consumer calls ioctl to set detectionTimeSchedule to occur every minute, and connection list index to 2
		// Consumer should reconnect to provider within a minute.
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		int port1;
		int port2;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			port2 = provider2.bindGetPort(opts);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
    		
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);

			// Set preferred host options, with connection list index to 0
			ReactorPreferredHostOptions preferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
			preferredHostOptions.connectionListIndex(0);
			preferredHostOptions.isPreferredHostEnabled(true);
			opts.preferredHostOptions(preferredHostOptions);
			
			int[] ports = {port1, port2, port2 + 1};
			consumerReactor.connectList(opts, consumer, ports);
			
			consumer.testReactor().dispatch(0);
			
			try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
    		/* Bring up provider 3. */
    		provider3.bindGetPort(opts);
    		
			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			
			/* Provider receives consumer directory request. */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		// Consumer calls ioctl to set detectionTimeSchedule to occur every minute.
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.detectionTimeSchedule("* * ? * *");
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.connectionListIndex(2);
    		consumer.reactorChannel().ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, consumer.reactorChannel().getEDPErrorInfo());
    		
    		// Wait up to 1 minute (with some buffer time) for ioctl to kick in and detectionTimeSchedule to occur.
    		
    		try {
				Thread.sleep(60100);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			provider3.testReactor().accept(opts, provider3);
			
    		consumer.testReactor().dispatch(5);

    		/* Login Status Suspect Open */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)loginMsgEvent.msg()).state().text().toString());
            
            /* Consumer receives directory update */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Consumer gets CHANNEL_DOWN_RECONNECTING */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			/* Consumer gets channel up */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
    		
			/* Consumer shows preferred host complete */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());
    		
    		// Provider 1 can receive channel down, though unit test framework may be unreliable
    		provider.testReactor().dispatch(-1);
    		
			event = provider.testReactor().pollEvent();
			if (event != null)
			{
				assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
				channelEvent = (ReactorChannelEvent)event.reactorEvent();
				assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());	
				System.out.println("Old provider received channel down");
			}
			
			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(3);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());

			/* Provider 3 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives consumer directory request. */
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider 3 sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostConnectionList_IoctlBeforePHSwitchComplete()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_IoctlBeforePHSwitchComplete <<<<<<<<<<\n");

		// 3 Configured and running providers, with 1st being the preferred host.
		// Bring up only Provider 1 and 2, not 3
		// Connect first to Provider 1
		// Consumer calls ioctl to set connectionListIndex to index 1
		// Consumer should call fallback method
		// Immediately after calling fallback method, set ioctl to connectionListIndex 2
		// Consumer should still connect to Provider 2, ignoring the ioctl call setting the index to 2.
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		int port1;
		int port2;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(-1);

			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			port2 = provider2.bindGetPort(opts);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
    		
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);

			// Set preferred host options, with connection list index to 0
			ReactorPreferredHostOptions preferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
			preferredHostOptions.connectionListIndex(0);
			preferredHostOptions.isPreferredHostEnabled(true);
			opts.preferredHostOptions(preferredHostOptions);
			
			int[] ports = {port1, port2, port2 + 1};
			consumerReactor.connectList(opts, consumer, ports);
			
			consumer.testReactor().dispatch(0);

			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			
			/* Provider receives consumer directory request. */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		// Consumer calls ioctl to set connection list index to 1
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.connectionListIndex(1);
    		consumer.reactorChannel().ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, consumer.reactorChannel().getEDPErrorInfo());
    		
    		// Give ioctl a moment to occur when worker is free
    		try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    		
    		// Consumer calls fallback method
    		consumer.reactorChannel().fallbackPreferredHost(null);
    		
    		// Consumer immediately calls ioctl changing preferred index to 2
    		ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
     		ioctlCall.isPreferredHostEnabled(true);
     		ioctlCall.connectionListIndex(2);
     		consumer.reactorChannel().ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, consumer.reactorChannel().getEDPErrorInfo());
			
			provider2.testReactor().accept(opts, provider2);
			
    		consumer.testReactor().dispatch(5);
    		
    		/* Login Status Suspect Open */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)loginMsgEvent.msg()).state().text().toString());
            
            /* Consumer receives directory update */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());

			/* Consumer gets channel down reconnecting */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			/* Consumer gets channel up */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
    		
			/* Consumer gets Preferred Host Complete */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());

    		// Provider 1 can receive channel down, though unit test framework may be unreliable
    		provider.testReactor().dispatch(-1);
    		
			event = provider.testReactor().pollEvent();
			if (event != null)
			{
				assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
				channelEvent = (ReactorChannelEvent)event.reactorEvent();
				assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());	
				System.out.println("Old provider received channel down");
			}

			/* provider 2 receives channel-up/channel-ready */
			provider2.testReactor().dispatch(3);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());

			/* provider 2 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
	        
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider2.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* provider 2 receives consumer directory request. */
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* provider 2 sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}

	
	@Test
	public void PreferredHostConnectionList_ioctlConnectionListIndex()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_ioctlConnectionListIndex <<<<<<<<<<\n");

		// 3 Configured and running providers, with 1st being the preferred host.
		// Bring up only Provider 1 and 2, not 3
		// Connect first to Provider 1
		// Consumer calls ioctl to set connectionListIndex to index 1
		// Consumer should reconnect to provider 2 after calling fallback method.
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		int port1;
		int port2;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(-1);

			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			port2 = provider2.bindGetPort(opts);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
    		
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);

			// Set preferred host options, with connection list index to 0
			ReactorPreferredHostOptions preferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
			preferredHostOptions.connectionListIndex(0);
			preferredHostOptions.isPreferredHostEnabled(true);
			opts.preferredHostOptions(preferredHostOptions);
			
			int[] ports = {port1, port2, port2 + 1};
			consumerReactor.connectList(opts, consumer, ports);
			
			consumer.testReactor().dispatch(0);

			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			
			/* Provider receives consumer directory request. */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		// Consumer calls ioctl to set connection list index to 1
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.connectionListIndex(1);
    		consumer.reactorChannel().ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, consumer.reactorChannel().getEDPErrorInfo());
    		
    		// Give ioctl a moment to occur when worker is free
    		try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    		
    		// Consumer calls fallback method
    		consumer.reactorChannel().fallbackPreferredHost(null);
			
			provider2.testReactor().accept(opts, provider2);
			
    		consumer.testReactor().dispatch(5);
    		
    		/* Login Status Suspect Open */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)loginMsgEvent.msg()).state().text().toString());
            
            /* Consumer receives directory update */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());

			/* Consumer gets channel down reconnecting */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			/* Consumer gets channel up */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
    		
			/* Consumer gets Preferred Host Complete */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());

    		// Provider 1 can receive channel down, though unit test framework may be unreliable
    		provider.testReactor().dispatch(-1);
    		
			event = provider.testReactor().pollEvent();
			if (event != null)
			{
				assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
				channelEvent = (ReactorChannelEvent)event.reactorEvent();
				assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());	
				System.out.println("Old provider received channel down");
			}

			/* provider 2 receives channel-up/channel-ready */
			provider2.testReactor().dispatch(3);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());

			/* provider 2 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
	        
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider2.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* provider 2 receives consumer directory request. */
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* provider 2 sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}

	@Test
	public void PreferredHostConnectionList_ioctlPreferredHostEnabled()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_ioctlPreferredHostEnabled <<<<<<<<<<\n");

		// 3 Configured and running providers, with 1st being the preferred host.
		// Bring up only Provider 1 and 2, not 3 (later preferred host)
		// Connect first to Provider 1
		// Bring up Provider 3
		// Consumer calls ioctl to set preferred host enabled to false and preferred host to index 2
		// Bring down Provider 1
		// Consumer should reconnect to provider 2, instead of 3.
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		int port1;
		int port2;
		int port3;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			port2 = provider2.bindGetPort(opts);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
    		
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			port3 = provider3.bindGetPort(opts);
			
			// Close provider right away now that we got the port
			provider3.close();
			providerReactor3.close();

			// Set preferred host options, with connection list index to 0
			ReactorPreferredHostOptions preferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
			preferredHostOptions.connectionListIndex(0);
			preferredHostOptions.isPreferredHostEnabled(true);
			opts.preferredHostOptions(preferredHostOptions);
			
			int[] ports = {port1, port2, port3};
			consumerReactor.connectList(opts, consumer, ports);
			
			consumer.testReactor().dispatch(0);
			
    		/* Bring up provider 3 again. */
			providerReactor3 = new TestReactor(true);
			provider3 = createDefaultProvider(provider3, providerReactor3);
    		provider3.bind(opts, port3);
    		
			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			
			/* Provider receives consumer directory request. */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		// Consumer calls ioctl to set preferred host enabled to false
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(false);
    		ioctlCall.connectionListIndex(2);
    		consumer.reactorChannel().ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, consumer.reactorChannel().getEDPErrorInfo());
    		
    		// Give ioctl a moment to occur when worker is free
    		try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    		
    		// Bring down provider 1
    		provider.close();
    		
    		consumer.testReactor().dispatch(3);
    		
    		/* Channel down reconnecting */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
    		
    		/* Login Status Suspect Open */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)loginMsgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)loginMsgEvent.msg()).state().text().toString());
            
            /* Consumer receives directory update */
    		event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			consumer.testReactor().dispatch(0);

			provider2.testReactor().accept(opts, provider2);

    		consumer.testReactor().dispatch(1);
    		
			/* Consumer gets channel up event from new connection to preferred host (provider 2) */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			/* provider 2 receives channel-up/channel-ready */
			provider2.testReactor().dispatch(3);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());

			/* provider 2 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider2.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* provider 2 receives consumer directory request. */
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* provider 2 sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
		}
		finally
		{
	       consumer.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	

	@Test
	public void PreferredHostConnectionList_ReactorPreferredHostInfoTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_ReactorPreferredHostInfoTest <<<<<<<<<<\n");

		// 3 Configured and running providers, with 1st being the preferred host.
		// Consumer configured for detectionTimeInterval with a time of 60 seconds
		// Bring up all Providers
		// Connect first to Provider 1
		// We should be able to get correct PreferredHostInfo, including remainingTime
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		int port1;
		int port2;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			port2 = provider2.bindGetPort(opts);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
    		
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
    		/* Bring up provider 3. */
    		provider3.bindGetPort(opts);
    		
			// Set preferred host options, with connection list index to 0
			ReactorPreferredHostOptions preferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
			preferredHostOptions.connectionListIndex(0);
			preferredHostOptions.isPreferredHostEnabled(true);
			preferredHostOptions.detectionTimeInterval(60);
			opts.preferredHostOptions(preferredHostOptions);
			
			int[] ports = {port1, port2, port2 + 1};
			consumerReactor.connectList(opts, consumer, ports);
			
			consumer.testReactor().dispatch(0);
			
			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			
			/* Provider receives consumer directory request. */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		// Consumer checks the ReactorPreferredHostInfo
    		ReactorChannelInfo reactorChannelInfo = ReactorFactory.createReactorChannelInfo();

    		consumer.info(reactorChannelInfo);
    		
    		// Ensure remaining detection time is between 30-60 seconds remaining, since this test shouldn't take that long
    		assertTrue(reactorChannelInfo.preferredHostInfo()._remainingDetectionTime > 30000
    				&& reactorChannelInfo.preferredHostInfo()._remainingDetectionTime < 60000);
    		
    		// Ensure connection list index is 0
    		assertEquals(0, reactorChannelInfo.preferredHostInfo()._connectionListIndex);
    		
    		// Ensure preferred host is enabled
    		assertEquals(true, reactorChannelInfo.preferredHostInfo()._isPreferredHostEnabled);
    		
    		// Ensure detection time interval set to 60
    		assertEquals(60, reactorChannelInfo.preferredHostInfo()._detectionTimeInterval);
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostConfig_ErrorCasesTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConfig_ErrorCasesTest <<<<<<<<<<\n");

		// Testing out of bounds error cases for indexes
		// Testing invalid cron schedule error case
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		Consumer consumer = null;
		Provider provider = null;
		int port1;
		
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			port1 = provider.bindGetPort(opts);

			// Set connection list index out of bounds
			ReactorPreferredHostOptions preferredHostOptions = ReactorFactory.createReactorPreferredHostOptions();
			preferredHostOptions.connectionListIndex(2);
			preferredHostOptions.isPreferredHostEnabled(true);
			opts.preferredHostOptions(preferredHostOptions);

			assertEquals(ReactorReturnCodes.FAILURE, consumerReactor.connectFailureTest(opts, consumer, port1));
			assertTrue(consumerReactor._errorInfo.error().text().equals("Configured preferredHostOptions connectionListIndex is out of bounds, aborting."));
			System.out.println(consumerReactor._errorInfo.error().text());
			
			// Set preferred host options, with connection list index to 2, which is invalid and should error
			preferredHostOptions.connectionListIndex(-1);
			preferredHostOptions.isPreferredHostEnabled(true);
			opts.preferredHostOptions(preferredHostOptions);

			assertEquals(ReactorReturnCodes.FAILURE, consumerReactor.connectFailureTest(opts, consumer, port1));
			assertTrue(consumerReactor._errorInfo.error().text().equals("Configured preferredHostOptions connectionListIndex is out of bounds, aborting."));
			System.out.println(consumerReactor._errorInfo.error().text());
			
			// Setup WSB
			List<Provider> wsbGroup1 = new ArrayList<Provider>();
			wsbGroup1.add(provider);
			
			// Set warmstandby group list index out of bounds
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(2);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			
			assertEquals(ReactorReturnCodes.FAILURE, consumerReactor.connectWsbFailureTest(connectOpts, opts, consumer, wsbGroup1, null, null, null));
			assertTrue(consumerReactor._errorInfo.error().text().equals("Configured preferredHostOptions warmStandbyGroupList is out of bounds, aborting."));
			System.out.println(consumerReactor._errorInfo.error().text());
			
			// Set warmstandby group list index out of bounds
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(-1);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			
			assertEquals(ReactorReturnCodes.FAILURE, consumerReactor.connectWsbFailureTest(connectOpts, opts, consumer, wsbGroup1, null, null, null));
			assertTrue(consumerReactor._errorInfo.error().text().equals("Configured preferredHostOptions warmStandbyGroupList is out of bounds, aborting."));
			System.out.println(consumerReactor._errorInfo.error().text());
			
			// Set invalid cron schedule time
			
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().detectionTimeSchedule("Behold, fail!");
			
			assertEquals(ReactorReturnCodes.FAILURE, consumerReactor.connectWsbFailureTest(connectOpts, opts, consumer, wsbGroup1, null, null, null));
			assertTrue(consumerReactor._errorInfo.error().text().equals("Parse exception occured on preferredHostOptions scheduled cron time, aborting."));
			System.out.println(consumerReactor._errorInfo.error().text());
			
			// Set correct opts
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().detectionTimeSchedule("* * ? * *");
			
			assertEquals(ReactorReturnCodes.SUCCESS, consumerReactor.connectWsbFailureTest(connectOpts, opts, consumer, wsbGroup1, null, null, null));
		}
		finally
		{
	       consumer.close();
	       provider.close();

	       providerReactor.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_LoginBased_ConnectionUpTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_LoginBased_ConnectionUpTest <<<<<<<<<<\n");

		// Consumer configured for 3 WSB Groups
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 3
		// Group 3 = Starting: 3, Standby 2
		
		// Consumer connects to Group 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			    
			// Add provider 3 as starting for group 3 and standby for group 2
			wsbGroup3.add(provider3);
			wsbGroup2.add(provider3);
			// Add provider 2 as standby for group 3
			wsbGroup3.add(provider2);
			
			    
			// Set preferred host options, with WSB group index to 2
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(2);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
			
			consumer.testReactor().dispatch(0);
			
			provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives consumer connection status and directory request. */
			provider3.testReactor().dispatch(2);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives request. */
            provider3.testReactor().dispatch(1);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives request. */
            provider2.testReactor().dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh with wrong item... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("IBM.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_LoginBased_InvalidWSBGroupFallbackFunctionCallTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_LoginBased_InvalidWSBGroupFallbackFunctionCallTest <<<<<<<<<<\n");

		// Consumer configured for 3 WSB Groups
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 3
		// Group 3 = Starting: 3, Standby 2
		
		// Consumer connects to Group 3
		// Take down provider 1.
		// Consumer calls ioctl to change preferred group index to 0
		// Call fallback method call
		// Consumer should not switchover but only get PREFERRED HOST COMPLETE callback
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			    
			// Add provider 3 as starting for group 3 and standby for group 2
			wsbGroup3.add(provider3);
			wsbGroup2.add(provider3);
			// Add provider 2 as standby for group 3
			wsbGroup3.add(provider2);
			
			    
			// Set preferred host options, with WSB group index to 2
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(2);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
			
			consumer.testReactor().dispatch(0);
			
			provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives consumer connection status and directory request. */
			provider3.testReactor().dispatch(2);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives request. */
            provider3.testReactor().dispatch(1);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives request. */
            provider2.testReactor().dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh with wrong item... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("IBM.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            // Close provider 1
            provider.close();
            
            // Consumer calls ioctl to switch to WSB PH Group index 0
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.warmStandbyGroupListIndex(0);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

    		// Wait a moment for the ioctl information to finish
    		try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    		
    		// Consumer calls fallback function
    		consumer.reactorChannel().fallbackPreferredHost(null);
    		
    		consumer.testReactor().dispatch(1);
    		
    		/* Consumer receives PREFERRED HOST COMPLETE. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());
    			    		
		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	
	@Test
	public void PreferredHostWarmStandby_ServiceBased_ConnectionUpTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ServiceBased_ConnectionUpTest <<<<<<<<<<\n");

		// Consumer configured for 3 WSB Groups
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 3
		// Group 3 = Starting: 3, Standby 2
		
		// Consumer connects to Group 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.SERVICE_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			    
			// Add provider 3 as starting for group 3 and standby for group 2
			wsbGroup3.add(provider3);
			wsbGroup2.add(provider3);
			// Add provider 2 as standby for group 3
			wsbGroup3.add(provider2);
			
			    
			// Set preferred host options, with WSB group index to 2
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(2);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
			
			consumer.testReactor().dispatch(0);
			
			provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives directory request. */
			provider3.testReactor().dispatch(1);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives directory request. */
			provider2.testReactor().dispatch(1);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives directory consumer status and item request. */
            provider3.testReactor().dispatch(2);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to Active */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.ACTIVE);
            
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives directory consumer status and request. */
            provider2.testReactor().dispatch(2);
            
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 2nd connection, so this should set to Standby */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.STANDBY);
            
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh with wrong item... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("IBM.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            

            /* Provider 3 sends update .*/
            UpdateMsg updateMsg = (UpdateMsg)msg;
            updateMsg.clear();
            updateMsg.msgClass(MsgClasses.UPDATE);
            updateMsg.domainType(DomainTypes.MARKET_PRICE);
            updateMsg.streamId(providerStreamId);
            updateMsg.containerType(DataTypes.NO_DATA);
            updateMsg.applyHasMsgKey();
            updateMsg.msgKey().applyHasServiceId();
            updateMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            updateMsg.msgKey().applyHasName();
            updateMsg.msgKey().name().data("TRI.N");
            
            assertTrue(provider3.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives update. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
            
            UpdateMsg receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
            assertTrue(receivedUpdateMsg.checkHasMsgKey());
            assertTrue(receivedUpdateMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedUpdateMsg.msgKey().serviceId());
            assertTrue(receivedUpdateMsg.msgKey().checkHasName());
            assertTrue(receivedUpdateMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
            assertFalse(receivedUpdateMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_LoginBased_ConnectionUpThenDownTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_LoginBased_ConnectionUpThenDownTest <<<<<<<<<<\n");

		// Consumer configured for 3, Login Based, WSB Groups
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 3
		// Group 3* (Preferred) = Starting: 3, Standby 2
		
		// Consumer connects to Preferred Group 3, then Group 3 goes down and Consumer reconnects to Group 1, where standby is still down
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			    
			// Add provider 3 as starting for group 3 and standby for group 2
			wsbGroup3.add(provider3);
			wsbGroup2.add(provider3);
			// Add provider 2 as standby for group 3
			wsbGroup3.add(provider2);
			
			    
			// Set preferred host options, with WSB group index to 2
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(2);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
			
			consumer.testReactor().dispatch(0);
			
			provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives consumer connection status and directory request. */
			provider3.testReactor().dispatch(2);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives request. */
            provider3.testReactor().dispatch(1);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives request. */
            provider2.testReactor().dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            /* Provider 3 and 2 closes, Consumer should reconnect to WSB Group 1, where the active server is up but standby is down */
            provider3.close();
            provider2.close();
            
            /* Consumer receives FD Change, open suspect, channel down reconnecting, login status open suspect, directory update, and channel down events */
            consumer.testReactor().dispatch(6);
            
            // FD_CHANGE event
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
            
			// MSG open suspect, "Service for this item was lost."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
			// Channel down reconnecting
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			// Login status open suspect
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
	        /* Consumer receives directory update. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    	
		    // MSG Open Suspect, "channel down."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());

            consumer.testReactor().dispatch(2, 5000);
            
        	int channel_down_reconnecting = 0;
            //There are two channel_down_reconnecting events
            for (int i = 0; i < 2; i++)
            {
            	event = consumerReactor.pollEvent();
            	if (event == null)
            		break;
            	switch (event.type())
            	{
            		case CHANNEL_EVENT:
            			channelEvent = (ReactorChannelEvent)event.reactorEvent();
            			if (channelEvent.eventType() == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
            				channel_down_reconnecting++;
            			break;
        			default:
        				assertEquals(true, false);
        				break;
            	}
            }
        	assertEquals(2, channel_down_reconnecting);

            provider.testReactor().accept(opts, provider, 10000);
            
			/* Provider 1 receives channel-up/channel-ready */
			provider.testReactor().dispatch(2);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			// Consumer receives Channel_Up
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
						
			
			/* Provider 1 receives login request */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider 1 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
	        
			// Consumer receives Login Refresh
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider 1 receives consumer connection status and directory request. */
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
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Consumer receives FD_CHANGE. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
    		
		}
		finally
		{
			//TestReactorComponent.closeSession(consumer, provider);
	       consumer.close();
	       provider.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ServiceBased_ConnectionUpThenDownTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ServiceBased_ConnectionUpThenDownTest <<<<<<<<<<\n");

		// Consumer configured for 3, Service Based, WSB Groups
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 3
		// Group 3* (Preferred) = Starting: 3, Standby 2
		
		// Consumer connects to Preferred Group 3, then Group 3 goes down and Consumer reconnects to Group 1, where standby is still down
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.SERVICE_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			    
			// Add provider 3 as starting for group 3 and standby for group 2
			wsbGroup3.add(provider3);
			wsbGroup2.add(provider3);
			// Add provider 2 as standby for group 3
			wsbGroup3.add(provider2);
			
			    
			// Set preferred host options, with WSB group index to 2
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(2);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
			
			consumer.testReactor().dispatch(0);
			
			provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives directory request. */
			provider3.testReactor().dispatch(1);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives directory request. */
			provider2.testReactor().dispatch(1);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives item request and directory consumer status. */
            provider3.testReactor().dispatch(2);
            
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to Active */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.ACTIVE);
            
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives item request and directory consumer status. */
            provider2.testReactor().dispatch(2);
            
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 2nd connection, so this should set to Standby */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.STANDBY);
            
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            /* Provider 3 and 2 closes, Consumer should reconnect to WSB Group 1, where the active server is up but standby is down */
            provider3.close();
            provider2.close();
            
            /* Consumer receives FD Change, open suspect, channel down reconnecting, login status open suspect, directory update, and channel down events */
            consumer.testReactor().dispatch(6);
            
            // FD_CHANGE event
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
            
			// MSG open suspect, "Service for this item was lost."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
			// Channel down reconnecting
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			// Login status open suspect
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
	        /* Consumer receives directory update. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    	
		    // MSG Open Suspect, "channel down."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());

            consumer.testReactor().dispatch(2, 5000);
            
        	int channel_down_reconnecting = 0;
			//There are two channel_down_reconnecting events
            for (int i = 0; i < 2; i++)
            {
            	event = consumerReactor.pollEvent();
            	if (event == null)
            		break;
            	switch (event.type())
            	{
            		case CHANNEL_EVENT:
            			channelEvent = (ReactorChannelEvent)event.reactorEvent();
            			if (channelEvent.eventType() == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
            				channel_down_reconnecting++;
            			break;
        			default:
        				assertEquals(true, false);
        				break;
            	}
            }
        	assertEquals(2, channel_down_reconnecting);

            provider.testReactor().accept(opts, provider, 10000);
            
			/* Provider 1 receives channel-up/channel-ready */
			provider.testReactor().dispatch(2);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			// Consumer receives Channel_Up
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
						
			
			/* Provider 1 receives login request */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider 1 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
	        
			// Consumer receives Login Refresh
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider 1 receives directory request. */
			provider.testReactor().dispatch(1);

			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Consumer receives FD_CHANGE. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
    		
		}
		finally
		{
			//TestReactorComponent.closeSession(consumer, provider);
	       consumer.close();
	       provider.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ServiceBased_ConnectionDownThenUpTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ServiceBased_ConnectionDownThenUpTest <<<<<<<<<<\n");

		// Consumer configured for 3, Service Based, WSB Groups
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 3
		// Group 3* (Preferred) = Starting: 3, Standby 2
		
		// Consumer fails to connect to Group 3, and round robins connections. Group 3 comes up later, and Consumer should connect to starting and standby servers
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.SERVICE_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			int port2 = provider2.bindGetPort(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			int port3 = provider3.bindGetPort(opts);
			    
			// Add provider 3 as starting for group 3 and standby for group 2
			wsbGroup3.add(provider3);
			wsbGroup2.add(provider3);
			// Add provider 2 as standby for group 3
			wsbGroup3.add(provider2);

			// Set preferred host options, with WSB group index to 2
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(2);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().detectionTimeInterval(5);
			
			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
			
			// Close all providers
			provider.close();
			provider2.close();
			provider3.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			
			try {
				Thread.sleep(5000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
	       providerReactor2.close();
	       providerReactor3.close();
	       
	       providerReactor2 = new TestReactor(true);
	       providerReactor3 = new TestReactor(true);
			
			provider2 = createDefaultProvider(provider2, providerReactor2);
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider2.bind(opts, port2);
			provider3.bind(opts, port3);
			
			try {
				Thread.sleep(5000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			provider3.testReactor().accept(opts, provider3, 5000);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives directory request. */
			provider3.testReactor().dispatch(1);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives directory request. */
			provider2.testReactor().dispatch(1);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives item request and directory consumer status. */
            provider3.testReactor().dispatch(2);
            
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to Active */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.ACTIVE);
            
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives item request and directory consumer status. */
            provider2.testReactor().dispatch(2);
            
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 2nd connection, so this should set to Standby */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.STANDBY);
            
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
		}
		finally
		{
	       consumer.close();
	       //provider2.close();
	       //provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_LoginBased_ConnectionList_ConnectionDownTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_LoginBased_ConnectionList_ConnectionDownTest <<<<<<<<<<\n");

		// Consumer configured for 1 WSB Group
		
		// Group 1* (Preferred) = Starting: 1, Standby 2
		// ConnectionList 1* (Preferred) = Provider 3
		
		// Consumer attempts to connect to Preferred Group 1, but group 1 is down so Consumer reconnects to ConnectionList entry 1
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1
			wsbGroup1.add(provider2);

			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);

			// Set preferred host options, with WSB group index to 0, connectionlist index to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider3);
			
			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, null, null, channelList);
			
			// Kill WSB group 1
			provider.close();
			provider2.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);
			
			// Attempt to connect to Provider 1 (Group 1) twice
			consumer.testReactor().dispatch(1);
			/* Channel down reconnecting */
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			/* Channel down reconnecting */
			consumer.testReactor().dispatch(1);
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			consumer.testReactor().dispatch(0);
			
			consumer.testReactor().switchingReactorChannel = true;
			
			provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives directory request. */
			provider3.testReactor().dispatch(1);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives request from consumer. */
            provider3.testReactor().dispatch(1);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

		}
		finally
		{
	       consumer.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_LoginBased_ConnectionList_ConnectionUpThenDownTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_LoginBased_ConnectionList_ConnectionUpThenDownTest <<<<<<<<<<\n");

		// Consumer configured for 1 WSB Group
		
		// Group 1* (Preferred) = Starting: 3, Standby 2
		// ConnectionList 1* (Preferred) = Provider 1
		
		// Consumer connects to group 1, then group 1 goes down so Consumer reconnects to ConnectionList entry 1
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);

			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
		    
			// Add provider 3 as starting for group 1, and provider 2 as standby for group 1
			wsbGroup1.add(provider3);
			wsbGroup1.add(provider2);


			// Set preferred host options, with WSB group index to 0, connectionlist index to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, null, null, channelList);

			provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives consumer connection status and directory request. */
			provider3.testReactor().dispatch(2);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives request. */
            provider3.testReactor().dispatch(1);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives request. */
            provider2.testReactor().dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh with wrong item... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("IBM.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            // Kill Group 1
			provider3.close();	// Starting for group 1
			provider2.close();	// Standby for group 1
			
			consumer.testReactor().switchingReactorChannel = true;
			
			/* Consumer receives FD Change, open suspect, channel down reconnecting, login status open suspect, directory update, and channel down events */
            consumer.testReactor().dispatch(6);
            
            // FD_CHANGE event
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
            
			// MSG open suspect, "Service for this item was lost."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
			// Channel down reconnecting
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			// Login status open suspect
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
	        /* Consumer receives directory update. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    	
		    // MSG Open Suspect, "channel down."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());

            try {
				Thread.sleep(5000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
            
			consumer.testReactor().switchingReactorChannel = true;
            
            consumer.testReactor().dispatch(2);
            
        	int channel_down_reconnecting = 0;
			//There are two channel_down_reconnecting events
            for (int i = 0; i < 2; i++)
            {
            	event = consumerReactor.pollEvent();
            	if (event == null)
            		break;
            	switch (event.type())
            	{
            		case CHANNEL_EVENT:
            			channelEvent = (ReactorChannelEvent)event.reactorEvent();
            			if (channelEvent.eventType() == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
            				channel_down_reconnecting++;
            			break;
        			default:
        				assertEquals(true, false);
        				break;
            	}
            }
        	assertEquals(2, channel_down_reconnecting);

			provider.testReactor().accept(opts, provider);

			/* Provider 1 receives channel-up/channel-ready */
			provider.testReactor().dispatch(2);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			consumer.testReactor().switchingReactorChannel = true;
			
			consumer.testReactor().dispatch(1);

			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
			

			/* Provider 1 receives directory request. */
			provider.testReactor().dispatch(1);

			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

            /* Provider 1 receives request from watchlist. */
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

		}
		finally
		{
	       //consumer.close(); TODO uncomment
	       provider.close();
	       
	       //consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	@Test
	public void PreferredHostWarmStandby_ServiceBased_ConnectionList_ConnectionUpThenDownThenUpTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ServiceBased_ConnectionList_ConnectionUpThenDownThenUpTest <<<<<<<<<<\n");

		// Consumer configured for 1 WSB Group
		
		// Group 1* (Preferred) = Starting: 3, Standby 2
		// ConnectionList 1* (Preferred) = Provider 1
		
		// Consumer connects to group 1, then group 1 goes down so Consumer reconnects to ConnectionList entry 1
		
		// Bring group 1 back up
		// Take ConnectionList entry 1 down
		// Consumer should reconnect to Group 1
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.SERVICE_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			int port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			int port2 = provider2.bindGetPort(opts);

			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			int port3 = provider3.bindGetPort(opts);
			
		    
			// Add provider 3 as starting for group 1, and provider 2 as standby for group 1
			wsbGroup1.add(provider3);
			wsbGroup1.add(provider2);


			// Set preferred host options, with WSB group index to 0, connectionlist index to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, null, null, channelList);

			provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives directory request. */
			provider3.testReactor().dispatch(1);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives directory request. */
			provider2.testReactor().dispatch(1);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives directory consumer status and item request. */
            provider3.testReactor().dispatch(2);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to Active */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.ACTIVE);
            
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives directory consumer status and item request. */
            provider2.testReactor().dispatch(2);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to Active */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.STANDBY);
            
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh with wrong item... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("IBM.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            // Kill Group 1
			provider3.close();
			provider2.close();
			
			consumer.testReactor().switchingReactorChannel = true;
			
			/* Consumer receives FD Change, open suspect, channel down reconnecting, login status open suspect, directory update, and channel down events */
            consumer.testReactor().dispatch(6);
            
            // FD_CHANGE event
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
            
			// MSG open suspect, "Service for this item was lost."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
			// Channel down reconnecting
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			// Login status open suspect
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
	        /* Consumer receives directory update. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    	
		    // MSG Open Suspect, "channel down."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());

            consumer.testReactor().switchingReactorChannel = true;
            
            consumer.testReactor().dispatch(2, 5000);
            
        	int channel_down_reconnecting = 0;
			//There are two channel_down_reconnecting events
            for (int i = 0; i < 2; i++)
            {
            	event = consumerReactor.pollEvent();
            	if (event == null)
            		break;
            	switch (event.type())
            	{
            		case CHANNEL_EVENT:
            			channelEvent = (ReactorChannelEvent)event.reactorEvent();
            			if (channelEvent.eventType() == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
            				channel_down_reconnecting++;
            			break;
        			default:
        				assertEquals(true, false);
        				break;
            	}
            }
        	assertEquals(2, channel_down_reconnecting);

            
			provider.testReactor().accept(opts, provider);

			/* Provider 1 receives channel-up/channel-ready */
			provider.testReactor().dispatch(2);
			
			// Restart providers 2 and 3
            providerReactor3 = new TestReactor(true);
            provider3 = createDefaultProvider(provider3, providerReactor3);
            provider3.bind(opts, port3);
            
            providerReactor2 = new TestReactor(true);
            provider2 = createDefaultProvider(provider2, providerReactor2);
            provider2.bind(opts, port2);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			consumer.testReactor().switchingReactorChannel = true;
			
			consumer.testReactor().dispatch(1);

			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
			

			/* Provider 1 receives directory request. */
			provider.testReactor().dispatch(1);

			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

            /* Provider 1 receives request from watchlist. */
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

            // Kill provider 1
            provider.close();
            
            // Consumer reconnects to group 1
            
            consumer.testReactor().dispatch(4);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            
            consumer.testReactor().switchingReactorChannel = true;
            
            provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives directory request. */
			provider3.testReactor().dispatch(1);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory update. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives directory request. */
			provider2.testReactor().dispatch(1);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
		}
		finally
		{
	       consumer.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	@Test
	public void PreferredHostWarmStandby_LoginBased_ConnectionList_ConnectionUpThenDownThenUpTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_LoginBased_ConnectionList_ConnectionUpThenDownThenUpTest <<<<<<<<<<\n");

		// Consumer configured for 1 WSB Group
		
		// Group 1* (Preferred) = Starting: 3, Standby 2
		// ConnectionList 1* (Preferred) = Provider 1
		
		// Consumer connects to group 1, then group 1 goes down so Consumer reconnects to ConnectionList entry 1
		
		// Bring group 1 back up
		// Take ConnectionList entry 1 down
		// Consumer should reconnect to Group 1
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			int port1 = provider.bindGetPort(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			int port2 = provider2.bindGetPort(opts);

			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			int port3 = provider3.bindGetPort(opts);
			
		    
			// Add provider 3 as starting for group 1, and provider 2 as standby for group 1
			wsbGroup1.add(provider3);
			wsbGroup1.add(provider2);


			// Set preferred host options, with WSB group index to 0, connectionlist index to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, null, null, channelList);

			provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives directory request and consumer connection status. */
			provider3.testReactor().dispatch(2);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);


			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives directory request and consumer connection status. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);


			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives item request. */
            provider3.testReactor().dispatch(1);

            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives item request. */
            provider2.testReactor().dispatch(1);

            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh with wrong item... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("IBM.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            // Kill Group 1
			provider3.close();
			provider2.close();
			
			consumer.testReactor().switchingReactorChannel = true;
			
			/* Consumer receives FD Change, open suspect, channel down reconnecting, login status open suspect, directory update, and channel down events */
            consumer.testReactor().dispatch(6);
            
            // FD_CHANGE event
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
            
			// MSG open suspect, "Service for this item was lost."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
			// Channel down reconnecting
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			// Login status open suspect
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
	        /* Consumer receives directory update. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    	
		    // MSG Open Suspect, "channel down."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());

            consumer.testReactor().switchingReactorChannel = true;
            
            consumer.testReactor().dispatch(2, 5000);
            
        	int channel_down_reconnecting = 0;
			//There are two channel_down_reconnecting events
            for (int i = 0; i < 2; i++)
            {
            	event = consumerReactor.pollEvent();
            	if (event == null)
            		break;
            	switch (event.type())
            	{
            		case CHANNEL_EVENT:
            			channelEvent = (ReactorChannelEvent)event.reactorEvent();
            			if (channelEvent.eventType() == ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING)
            				channel_down_reconnecting++;
            			break;
        			default:
        				assertEquals(true, false);
        				break;
            	}
            }
        	assertEquals(2, channel_down_reconnecting);

            
			provider.testReactor().accept(opts, provider);

			/* Provider 1 receives channel-up/channel-ready */
			provider.testReactor().dispatch(2);
			
			// Restart providers 2 and 3
            providerReactor3 = new TestReactor(true);
            provider3 = createDefaultProvider(provider3, providerReactor3);
            provider3.bind(opts, port3);
            
            providerReactor2 = new TestReactor(true);
            provider2 = createDefaultProvider(provider2, providerReactor2);
            provider2.bind(opts, port2);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			consumer.testReactor().switchingReactorChannel = true;
			
			consumer.testReactor().dispatch(1);

			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
			

			/* Provider 1 receives directory request. */
			provider.testReactor().dispatch(1);

			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

            /* Provider 1 receives request from watchlist. */
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

            // Kill provider 1
            provider.close();
            
            // Consumer reconnects to group 1
            
            consumer.testReactor().dispatch(4);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            
            consumer.testReactor().switchingReactorChannel = true;
            
            provider3.testReactor().accept(opts, provider3);

			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
	        
			/* Provider 3 receives directory request and consumer connection status. */
			provider3.testReactor().dispatch(2);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory update. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives directory request and consumer connection status. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
		}
		finally
		{
	       consumer.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_IoctlInvalidOptionsTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_IoctlInvalidOptionsTest <<<<<<<<<<\n");

		// Start only Providers 1 and 2
		// Consumer should connect to Group 1
		// Test setting invalid ioctl warmstandbygroupindex
		
		// Group 1* = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 1
		// Group 3 = Starting: 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);

			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			 
			// Set preferred host options, with WSB group index to 0 (We have to use ioctl to change this)
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(false);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
		
			consumer.testReactor().dispatch(0);
			
			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			

			/* Provider receives consumer connection status and directory request. */
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
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 1 receives request. */
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives request. */
            provider2.testReactor().dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
    		// Consumer calls ioctl to set warmStandbyGroupIndex to invalid value of -1
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.warmStandbyGroupListIndex(-1);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		int ret = reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());
    		assertEquals(ReactorReturnCodes.FAILURE, ret);
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	

	@Test
	public void PreferredHostWarmStandby_DetectionTimeIntervalTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_DetectionTimeIntervalTest <<<<<<<<<<\n");

		// Start only Providers 1 and 2
		// Consumer should connect to Group 1 after attempting to connect to Group 3.
		// Detection Time Interval should hit, and Consumer should reconnect to Group 3.
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 1
		// Group 3 = Starting: 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);

			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			 
			// Set preferred host options, with WSB group index to 0 (We have to use ioctl to change this)
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(false);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
		
			consumer.testReactor().dispatch(0);
			
			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			

			/* Provider receives consumer connection status and directory request. */
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
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 1 receives request. */
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives request. */
            provider2.testReactor().dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
    		// Consumer calls ioctl to set detectionTimeInterval to 5.
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.detectionTimeInterval(5);
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.warmStandbyGroupListIndex(2);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

    		// Wait up to 5 seconds (with some buffer time) for ioctl to kick in and detectionTimeInterval to occur.
    		
    		try {
				Thread.sleep(5100);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    		
    		/* Provider 3 accepts new connection */
    		provider3.testReactor().accept(opts, provider3);

    		consumerReactor.dispatch(-1);
            
            //FD_CHANGE or MSG event could be first
            for (int i = 0; i < 8; i++)
            {
            	event = consumerReactor.pollEvent();
            	if (event == null)
            		break;
            	switch (event.type())
            	{
            		case CHANNEL_EVENT:
            			// FD_CHANGE event
                		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
                		channelEvent = (ReactorChannelEvent)event.reactorEvent();
                		if (channelEvent.eventType() == ReactorChannelEventTypes.FD_CHANGE)
                		{
                			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
                			System.out.println("FD_Change Event.");
                		}
                		else if (channelEvent.eventType() == ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
                		{
                			assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());
                			System.out.println("Preferred Host Complete Event.");
                		}
            			break;
            		case MSG:
            			// MSG open suspect, "Service for this item was lost."
                        assertEquals(TestReactorEventTypes.MSG, event.type());
                        msgEvent = (ReactorMsgEvent)event.reactorEvent();
                        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
                        assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
                        assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
                        System.out.println("Status Msg Event: " + ((StatusMsg)msgEvent.msg()).state().text().toString());
            			break;
            		case LOGIN_MSG:
            			// Login status open suspect
                        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
                        msgEvent = (ReactorMsgEvent)event.reactorEvent();
                        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
                        assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
                        assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
                        System.out.println("Login Msg Event: " + ((StatusMsg)msgEvent.msg()).state().text().toString());
            			break;
            		case DIRECTORY_MSG:
            			/* Consumer receives directory update. */
                		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
                		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
            		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
            		    System.out.println("Directory Msg Event.");
            			break;
            		default:
            			break;
            	}
            }
            
            try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
            
    		/* Provider 1 receives channel down */
    		provider.testReactor().dispatch(1);
    		event = provider.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());

    		/* Provider 2 receives channel down */
    		provider2.testReactor().dispatch(1);
    		event = provider2.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());

            
			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(3);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider 3 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives consumer connection status and directory request. */
			provider3.testReactor().dispatch(2);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider 3 sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory update. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
            /* Provider 3 receives request from Consumer watchlist. */
            provider3.testReactor().dispatch(1);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_DetectionTimeScheduleTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_DetectionTimeScheduleTest <<<<<<<<<<\n");

		// Start only Providers 1 and 2
		// Consumer should connect to Group 1 after attempting to connect to Group 3.
		// Detection Time Schedule should hit, and Consumer should reconnect to Group 3.
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 1
		// Group 3 = Starting: 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;

		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;

		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);

			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			 
			// Set preferred host options, with WSB group index to 0 (We have to use ioctl to change this)
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(false);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
		
			consumer.testReactor().dispatch(0);

			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			

			/* Provider receives consumer connection status and directory request. */
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
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 1 receives request. */
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives request. */
            provider2.testReactor().dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
    		// Consumer calls ioctl to set detectionTimeSchedule to 1 minute.
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.detectionTimeSchedule("* * ? * *");
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.warmStandbyGroupListIndex(2);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

    		// Wait up to 60 seconds (with some buffer time) for ioctl to kick in and detectionTimeSchedule to occur.

    		try {
				Thread.sleep(60100);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    		
    		/* Provider 3 accepts new connection */
    		provider3.testReactor().accept(opts, provider3);

            consumerReactor.dispatch(-1);
            
            //FD_CHANGE or MSG event could be first
            for (int i = 0; i < 8; i++)
            {
            	event = consumerReactor.pollEvent();
            	if (event == null)
            		break;
            	switch (event.type())
            	{
            		case CHANNEL_EVENT:
            			// FD_CHANGE event
                		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
                		channelEvent = (ReactorChannelEvent)event.reactorEvent();
                		if (channelEvent.eventType() == ReactorChannelEventTypes.FD_CHANGE)
                		{
                			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
                		}
                		else if (channelEvent.eventType() == ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
                		{
                			assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());
                		}
            			break;
            		case MSG:
            			// MSG open suspect, "Service for this item was lost."
                        assertEquals(TestReactorEventTypes.MSG, event.type());
                        msgEvent = (ReactorMsgEvent)event.reactorEvent();
                        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
                        assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
                        assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
                        System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            			break;
            		case LOGIN_MSG:
            			// Login status open suspect
                        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
                        msgEvent = (ReactorMsgEvent)event.reactorEvent();
                        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
                        assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
                        assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
                        System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            			break;
            		case DIRECTORY_MSG:
            			/* Consumer receives directory update. */
                		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
                		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
            		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
            			break;
            		default:
            			break;
            	}
            }
            
            try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				// TODO Auto-generated catch block
				e.printStackTrace();
			}
            
    		/* Provider 1 receives close */
    		provider.testReactor().dispatch(3);
    		event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
            
    		event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
			
    		event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
			
			
    		/* Provider 2 receives close */
    		provider2.testReactor().dispatch(3);
    		event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
            
    		event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
			
    		event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
			
            
			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(3);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider 3 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());

			/* Provider 3 receives consumer connection status and directory request. */
			provider3.testReactor().dispatch(2);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider 3 sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

            /* Provider 3 receives request from Consumer watchlist. */
            provider3.testReactor().dispatch(1);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_FallbackMethodTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_FallbackMethodTest <<<<<<<<<<\n");

		// Start only Providers 1 and 2
		// Consumer should connect to Group 1 after attempting to connect to Group 3.
		// After fallback method is called the Consumer should reconnect to Group 3.
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 1
		// Group 3 = Starting: 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);

			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			 
			// Set preferred host options, with WSB group index to 0 (We have to use ioctl to change this)
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(false);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
		
			consumer.testReactor().dispatch(0);
			
			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			

			/* Provider receives consumer connection status and directory request. */
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
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 1 receives request. */
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives request. */
            provider2.testReactor().dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
    		// Consumer calls ioctl to set preferred host info
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.warmStandbyGroupListIndex(2);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

    		// Wait a moment for the ioctl information to finish
    		try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    		
    		// Consumer calls fallback method
    		reactorChannel.fallbackPreferredHost(reactorChannel.getEDPErrorInfo());
    		
    		/* Provider 3 accepts new connection */
    		provider3.testReactor().accept(opts, provider3);

    		consumerReactor.dispatch(-1);
            
            //FD_CHANGE or MSG event could be first
            for (int i = 0; i < 8; i++)
            {
            	event = consumerReactor.pollEvent();
            	if (event == null)
            		break;
            	switch (event.type())
            	{
            		case CHANNEL_EVENT:
            			// FD_CHANGE event
                		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
                		channelEvent = (ReactorChannelEvent)event.reactorEvent();
                		if (channelEvent.eventType() == ReactorChannelEventTypes.FD_CHANGE)
                		{
                			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
                			System.out.println("FD_Change Event.");
                		}
                		else if (channelEvent.eventType() == ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
                		{
                			assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());
                			System.out.println("Preferred Host Complete Event.");
                		}
            			break;
            		case MSG:
            			// MSG open suspect, "Service for this item was lost."
                        assertEquals(TestReactorEventTypes.MSG, event.type());
                        msgEvent = (ReactorMsgEvent)event.reactorEvent();
                        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
                        assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
                        assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
                        System.out.println("Status Msg Event: " + ((StatusMsg)msgEvent.msg()).state().text().toString());
            			break;
            		case LOGIN_MSG:
            			// Login status open suspect
                        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
                        msgEvent = (ReactorMsgEvent)event.reactorEvent();
                        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
                        assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
                        assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
                        System.out.println("Login Msg Event: " + ((StatusMsg)msgEvent.msg()).state().text().toString());
            			break;
            		case DIRECTORY_MSG:
            			/* Consumer receives directory update. */
                		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
                		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
            		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
            		    System.out.println("Directory Msg Event.");
            			break;
            		default:
            			break;
            	}
            }
            
            try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
            
    		/* Provider 1 receives channel down */
    		provider.testReactor().dispatch(1);
    		event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
			
    		/* Provider 2 receives channel down */
    		provider2.testReactor().dispatch(1);
    		event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
			
            
			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(3);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider 3 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives consumer connection status and directory request. */
			provider3.testReactor().dispatch(2);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider 3 sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
            /* Provider 3 receives request from Consumer watchlist. */
            provider3.testReactor().dispatch(1);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            /* Provider 3 sends update .*/
            UpdateMsg updateMsg = (UpdateMsg)msg;
            updateMsg.clear();
            updateMsg.msgClass(MsgClasses.UPDATE);
            updateMsg.domainType(DomainTypes.MARKET_PRICE);
            updateMsg.streamId(providerStreamId);
            updateMsg.containerType(DataTypes.NO_DATA);
            updateMsg.applyHasMsgKey();
            updateMsg.msgKey().applyHasServiceId();
            updateMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            updateMsg.msgKey().applyHasName();
            updateMsg.msgKey().name().data("TRI.N");
            
            assertTrue(provider3.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives update. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
            
            UpdateMsg receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
            assertTrue(receivedUpdateMsg.checkHasMsgKey());
            assertTrue(receivedUpdateMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedUpdateMsg.msgKey().serviceId());
            assertTrue(receivedUpdateMsg.msgKey().checkHasName());
            assertTrue(receivedUpdateMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
            assertFalse(receivedUpdateMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ServceBased_FallbackMethodTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ServceBased_FallbackMethodTest <<<<<<<<<<\n");

		// Start only Providers 1 and 2
		// Consumer should connect to Group 1 after attempting to connect to Group 3.
		// After fallback method is called the Consumer should reconnect to Group 3.
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 1
		// Group 3 = Starting: 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.SERVICE_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);

			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			 
			// Set preferred host options, with WSB group index to 0 (We have to use ioctl to change this)
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(false);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
		
			consumer.testReactor().dispatch(0);
			
			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
			

			/* Provider receives directory request. */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives directory request. */
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 1 receives directory consumer status and item request. */
            provider.testReactor().dispatch(2);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to Active */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.ACTIVE);
            
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives directory consumer status and item request. */
            provider2.testReactor().dispatch(2);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to Active */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.STANDBY);

            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
    		// Consumer calls ioctl to set preferred host info
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.warmStandbyGroupListIndex(2);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

    		// Wait a moment for the ioctl information to finish
    		try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    		
    		// Consumer calls fallback method
    		reactorChannel.fallbackPreferredHost(reactorChannel.getEDPErrorInfo());
    		
    		/* Provider 3 accepts new connection */
    		provider3.testReactor().accept(opts, provider3);

    		consumerReactor.dispatch(-1);
            
            //FD_CHANGE or MSG event could be first
            for (int i = 0; i < 8; i++)
            {
            	event = consumerReactor.pollEvent();
            	if (event == null)
            		break;
            	switch (event.type())
            	{
            		case CHANNEL_EVENT:
            			// FD_CHANGE event
                		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
                		channelEvent = (ReactorChannelEvent)event.reactorEvent();
                		if (channelEvent.eventType() == ReactorChannelEventTypes.FD_CHANGE)
                		{
                			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
                			System.out.println("FD_Change Event.");
                		}
                		else if (channelEvent.eventType() == ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE)
                		{
                			assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());
                			System.out.println("Preferred Host Complete Event.");
                		}
            			break;
            		case MSG:
            			// MSG open suspect, "Service for this item was lost."
                        assertEquals(TestReactorEventTypes.MSG, event.type());
                        msgEvent = (ReactorMsgEvent)event.reactorEvent();
                        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
                        assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
                        assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
                        System.out.println("Status Msg Event: " + ((StatusMsg)msgEvent.msg()).state().text().toString());
            			break;
            		case LOGIN_MSG:
            			// Login status open suspect
                        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
                        msgEvent = (ReactorMsgEvent)event.reactorEvent();
                        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
                        assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
                        assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
                        System.out.println("Login Msg Event: " + ((StatusMsg)msgEvent.msg()).state().text().toString());
            			break;
            		case DIRECTORY_MSG:
            			/* Consumer receives directory update. */
                		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
                		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
            		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
            		    System.out.println("Directory Msg Event.");
            			break;
            		default:
            			break;
            	}
            }
            
            try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
            
    		/* Provider 1 receives channel down */
    		provider.testReactor().dispatch(1);
    		event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
			
    		/* Provider 2 receives channel down */
    		provider2.testReactor().dispatch(1);
    		event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
			
            
			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(3);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider 3 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider 3 receives directory request. */
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider 3 sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
            // Provider 3 gets generic message to change to Active, and item request from Consumer Watchlist
            provider3.testReactor().dispatch(2);
            
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());

            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            /* Provider 3 sends update .*/
            UpdateMsg updateMsg = (UpdateMsg)msg;
            updateMsg.clear();
            updateMsg.msgClass(MsgClasses.UPDATE);
            updateMsg.domainType(DomainTypes.MARKET_PRICE);
            updateMsg.streamId(providerStreamId);
            updateMsg.containerType(DataTypes.NO_DATA);
            updateMsg.applyHasMsgKey();
            updateMsg.msgKey().applyHasServiceId();
            updateMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            updateMsg.msgKey().applyHasName();
            updateMsg.msgKey().name().data("TRI.N");
            
            assertTrue(provider3.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives update. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
            
            UpdateMsg receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
            assertTrue(receivedUpdateMsg.checkHasMsgKey());
            assertTrue(receivedUpdateMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedUpdateMsg.msgKey().serviceId());
            assertTrue(receivedUpdateMsg.msgKey().checkHasName());
            assertTrue(receivedUpdateMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
            assertFalse(receivedUpdateMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ServceBased_FallbackFunctionCallFromChannelList()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ServceBased_FallbackFunctionCallFromChannelList <<<<<<<<<<\n");

		// Start only Provider 3
		// Consumer should connect to Provider 3 after attempting to reconnect to Groups 1 and 2 in proper order
		// Start Provider 1/2
		// After fallback method is called the Consumer should reconnect to Group 1.
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 1
		// ChannelList = Provider 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		int sleepTime = 1200;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> channelList = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.SERVICE_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			int port1 = provider.bindGetPort(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			int port2 = provider2.bindGetPort(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			// Add Provider 3 to channel list
			channelList.add(provider3);
			 
			// Set preferred host options, with WSB group index to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			
			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, null, channelList);
			
			// Kill providers 1 and 2
			provider.close();
			provider2.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);
		
    		checkChannelDownReconnecting(consumer, port1, 500, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
			
			consumer.testReactor().dispatch(0);
			
			consumer.testReactor().switchingReactorChannel = true;
			
			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().accept(opts, provider3);
			
            // Restart Provider 1 and 2
            providerReactor = new TestReactor(true);
            provider = createDefaultProvider(provider, providerReactor);
            provider.bind(opts, port1);
            
            providerReactor2 = new TestReactor(true);
            provider2 = createDefaultProvider(provider2, providerReactor2);
            provider2.bind(opts, port2);
            
            provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider receives directory request. */
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		 /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives item request. */
            provider3.testReactor().dispatch(1);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
			
            // Consumer calls fallback to switchover to WSB Group 1 (Provider 1 / 2)
            consumer.reactorChannel().fallbackPreferredHost(null);
			
            consumer.testReactor().switchingReactorChannel = true;
            
			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(4);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
			

			/* Provider receives directory request. */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives directory request. */
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	@Test
	public void PreferredHostWarmStandby_LoginBased_FallbackFunctionCallFromChannelList()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_LoginBased_FallbackFunctionCallFromChannelList <<<<<<<<<<\n");

		// Start only Provider 3
		// Consumer should connect to Provider 3 after attempting to reconnect to Groups 1 and 2 in proper order
		// Start Provider 1/2
		// After fallback method is called the Consumer should reconnect to Group 1.
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 1
		// ChannelList = Provider 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		
		int sleepTime = 1200;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> channelList = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			int port1 = provider.bindGetPort(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			int port2 = provider2.bindGetPort(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			// Add Provider 3 to channel list
			channelList.add(provider3);
			 
			// Set preferred host options, with WSB group index to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			
			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, null, channelList);
			
			// Kill providers 1 and 2
			provider.close();
			provider2.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);
		
    		checkChannelDownReconnecting(consumer, port1, 500, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
			
			consumer.testReactor().dispatch(0);
			
			consumer.testReactor().switchingReactorChannel = true;
			
			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().accept(opts, provider3);
			
            // Restart Provider 1 and 2
            providerReactor = new TestReactor(true);
            provider = createDefaultProvider(provider, providerReactor);
            provider.bind(opts, port1);
            
            providerReactor2 = new TestReactor(true);
            provider2 = createDefaultProvider(provider2, providerReactor2);
            provider2.bind(opts, port2);
            
            provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider3.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider receives directory request. */
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider3.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		 /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives item request. */
            provider3.testReactor().dispatch(1);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
			
            // Consumer calls fallback to switchover to WSB Group 1 (Provider 1 / 2)
            consumer.reactorChannel().fallbackPreferredHost(null);
			
            consumer.testReactor().switchingReactorChannel = true;
            
			provider.testReactor().accept(opts, provider);

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
			
			consumer.testReactor().dispatch(4);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());
			
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
			

			/* Provider receives directory request and consumer connection status. */
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
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider2.testReactor().accept(opts, provider2);

			/* Provider receives channel-up/channel-ready */
			provider2.testReactor().dispatch(2);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives directory request and consumer connection status. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_LoginBased_FallBackWithInWSBGroupTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_LoginBased_FallBackWithInWSBGroupTest <<<<<<<<<<\n");

		// Configure group 3 to be preferred, with fallBackWithInWSBGroup = true
		// Consumer should connect to Group 3, Provider 3 with standby Provider 4.
		// Provider 3 is taken down, and Consumer should switch Provider 4 to active.
		// ioctl should change configured preferred group to index 0.
		// After fallback method is called the Consumer should reconnect to Group 3's Starting Server, Provider 3, instead of Group 1.
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 1
		// Group 3 = Starting: 3, Standby 4
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			int port3 = provider3.bindGetPort(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);
			
			provider4.bind(opts);
			
			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			// Add provider 4 as standby for group 3
			wsbGroup3.add(provider4);

			// Set preferred host options, with WSB group index to 2
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(2);
			connectOpts.reactorPreferredHostOptions().fallBackWithInWSBGroup(true);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
		
			consumer.testReactor().dispatch(0);

			provider3.testReactor().accept(opts, provider3);
			
			/* Provider receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
			
			/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
			provider.defaultSessionLoginStreamId(loginRequest.streamId());
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			

			/* Provider receives consumer connection status and directory request. */
			provider3.testReactor().dispatch(2);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider4.testReactor().accept(opts, provider4);

			/* Provider receives channel-up/channel-ready */
			provider4.testReactor().dispatch(2);
			
			event = provider4.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider4.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider4.testReactor().dispatch(1);
			event = provider4.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider4.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives consumer connection status and directory request. */
	        provider4.testReactor().dispatch(2);
			event = provider4.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 1st connection, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider4.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider4.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
			provider4.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives request. */
            provider3.testReactor().dispatch(1);
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 4 receives request. */
            provider4.testReactor().dispatch(1);
            event = provider4.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 4 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider4.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
    		// Consumer calls ioctl to set preferred host info, changing WSB group index to 0
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.fallBackWithInWSBGroup(true);
    		ioctlCall.warmStandbyGroupListIndex(0);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

    		// Wait a moment for the ioctl information to finish
    		try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    		
    		// Kill Provider 3. Consumer should switch Provider 4 to active.
    		
    		provider3.close();
    		providerReactor3.close();
    		
    		consumer.testReactor().dispatch(2);
    		
    		// FD_CHANGE event
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			// MSG open suspect, "Service for this item was lost."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
            // Provider 4 gets generic message to change to Active
            provider4.testReactor().dispatch(1);
            
            event = provider4.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
            
            // Restart Provider 3
            providerReactor3 = new TestReactor(true);
            provider3 = createDefaultProvider(provider3, providerReactor3);
            provider3.bind(opts, port3);
            
    		consumer.testReactor().dispatch(0);
            
    		/* Provider 3 accepts new connection */
    		provider3.testReactor().accept(opts, provider3);

            consumerReactor.dispatch(1);
            
            // FD_CHANGE event
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
            
			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(3);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider 3 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);

			/* Provider 3 receives consumer connection status and directory request. */
			provider3.testReactor().dispatch(2);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* Standby server */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.STANDBY);

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider 3 sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives Channel Ready. */
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

			// Consumer calls fallback method, should switch to Provider 3 as active instead of moving to WSB PH Group 1
			consumer.reactorChannel().fallbackPreferredHost(null);
			
	        consumer.testReactor().dispatch(1);
	        
	        event = consumer.testReactor().pollEvent();
	        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());

			consumer.testReactor().dispatch(0);

	        // Provider 4 receives Login Generic message switching to Standby
            provider4.testReactor().dispatch(1);
            
            event = provider4.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
            assertEquals(ServerTypes.STANDBY, ((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode());

            
	        // Provider 3 receives Request message
            provider3.testReactor().dispatch(2);
            
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            // Provider 3 reads the request message here
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            // Provider 3 receives Login Generic message switching to Active
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
            assertEquals(ServerTypes.ACTIVE, ((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode());
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       provider4.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ServiceBased_FallBackWithInWSBGroupTest()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ServiceBased_FallBackWithInWSBGroupTest <<<<<<<<<<\n");

		// Configure group 3 to be preferred, with fallBackWithInWSBGroup = true
		// Consumer should connect to Group 3, Provider 3 with standby Provider 4.
		// Provider 3 is taken down, and Consumer should switch Provider 4 to active.
		// ioctl should change configured preferred group to index 0.
		// After fallback method is called the Consumer should reconnect to Group 3's Starting Server, Provider 3, instead of Group 1.
		
		// Group 1 = Starting: 1, Standby 2
		// Group 2 = Starting: 2, Standby 1
		// Group 3 = Starting: 3, Standby 4
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);
			
			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.SERVICE_BASED);
			opts.reconnectAttemptLimit(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			    
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			int port3 = provider3.bindGetPort(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);
			
			provider4.bind(opts);
			
			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			// Add provider 4 as standby for group 3
			wsbGroup3.add(provider4);

			// Set preferred host options, with WSB group index to 2
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(2);
			connectOpts.reactorPreferredHostOptions().fallBackWithInWSBGroup(true);
			
			consumerReactor.connectWsb(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, null);
		
			consumer.testReactor().dispatch(0);

			provider3.testReactor().accept(opts, provider3);
			
			/* Provider receives channel-up/channel-ready */
			provider3.testReactor().dispatch(2);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
			
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(1);
	        
	        /* Consumer receives Login Refresh */
	        event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());

			/* Provider receives directory request. */
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
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
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

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
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Now handle the standby */
    		provider4.testReactor().accept(opts, provider4);

			/* Provider receives channel-up/channel-ready */
			provider4.testReactor().dispatch(2);
			
			event = provider4.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider4.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			consumer.testReactor().dispatch(1);
			
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			provider4.testReactor().dispatch(1);
			event = provider4.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();

			loginRefresh.clear();
	        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
	        loginRefresh.applySolicited();
	        loginRefresh.userName(loginRequest.userName());
	        loginRefresh.streamId(loginRequest.streamId());
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
	        
	        submitOptions.clear();
	        assertTrue(provider4.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);
			
			/* Provider receives directory request. */
	        provider4.testReactor().dispatch(1);

			event = provider4.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider4.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
			provider4.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 3 receives request. */
            provider3.testReactor().dispatch(2);
            
            event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
            
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 4 receives request. */
            provider4.testReactor().dispatch(2);
            
            event = provider4.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
            
            event = provider4.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 4 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider4.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
    		// Consumer calls ioctl to set preferred host info, changing WSB group index to 0
    		ReactorPreferredHostOptions ioctlCall = ReactorFactory.createReactorPreferredHostOptions();
    		ioctlCall.isPreferredHostEnabled(true);
    		ioctlCall.fallBackWithInWSBGroup(true);
    		ioctlCall.warmStandbyGroupListIndex(0);
    		ReactorChannel reactorChannel = consumer._testReactor._reactor._reactorChannelQueue.peek();
    		reactorChannel.ioctl(ReactorChannelIOCtlCode.FALLBACK_PREFERRED_HOST_OPTIONS, ioctlCall, reactorChannel.getEDPErrorInfo());

    		// Wait a moment for the ioctl information to finish
    		try {
				Thread.sleep(1000);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
    		
    		// Kill Provider 3. Consumer should switch Provider 4 to active.
    		
    		provider3.close();
    		providerReactor3.close();
    		
    		consumer.testReactor().dispatch(2);
    		
    		// FD_CHANGE event
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
			
			// MSG open suspect, "Service for this item was lost."
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
            // Provider 4 gets generic message to change to Active
            provider4.testReactor().dispatch(1);
            
            event = provider4.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
            
            // Restart Provider 3
            providerReactor3 = new TestReactor(true);
            provider3 = createDefaultProvider(provider3, providerReactor3);
            provider3.bind(opts, port3);
            
    		consumer.testReactor().dispatch(0);
            
    		/* Provider 3 accepts new connection */
    		provider3.testReactor().accept(opts, provider3);

            consumerReactor.dispatch(1);
            
            // FD_CHANGE event
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
            
			/* Provider 3 receives channel-up/channel-ready */
			provider3.testReactor().dispatch(3);
			
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			
			/* Provider 3 sends a default login refresh. */
			loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
			loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

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
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatch(0);

			/* Provider 3 receives directory request. */
			provider3.testReactor().dispatch(1);
			event = provider3.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.REQUEST, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			
			/* Provider 3 sends a default directory refresh. */
			directoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
			directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();

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

	        service = DirectoryMsgFactory.createService();
	        Provider.defaultService().copy(service);
	        
	        directoryRefresh.serviceList().add(service);
	        
	        submitOptions.clear();
	        assertTrue(provider3.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);

	        /* Consumer receives Channel Ready. */
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());

			// Consumer calls fallback method, should switch to Provider 3 as active instead of moving to WSB PH Group 1
			consumer.reactorChannel().fallbackPreferredHost(null);
			
	        consumer.testReactor().dispatch(1);
	        
	        event = consumer.testReactor().pollEvent();
	        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE, channelEvent.eventType());

	        // Provider 4 receives Directory Consumer Status switching to Standby
            provider4.testReactor().dispatch(1);

            event = provider4.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.STANDBY);
            
            
	        // Provider 3 receives Request message
            provider3.testReactor().dispatch(3);
            
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            // Provider 3 receives the initial Directory Consumer Status setting it to Standby
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.STANDBY);
            
            // Provider 3 reads the request message here
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            // Provider 3 receives Directory Consumer Status switching to Active
            event = provider3.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent._directoryMsg).consumerServiceStatusList().get(0).warmStandbyMode(), ServerTypes.ACTIVE);
            
            /* Provider 3 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider3.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            
            
		}
		finally
		{
	       consumer.close();
	       provider.close();
	       provider2.close();
	       provider3.close();
	       provider4.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ConnectionList_AllDownTest_EndPreferred()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ConnectionList_AllDownTest_EndPreferred <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 2 and WSB Group Index 2
		// Consumer should attempt to connect to preferred group (Group 3)
		// Failing that, it should move through the groups and channels properly
		
		// Group 1
		// Group 2
		// Group 3*
		// Channel 1
		// Channel 2
		// Channel 3*
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		TestReactor providerReactor5 = null;
		TestReactor providerReactor6 = null;
		TestReactor providerReactor7 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		Provider provider5 = null;
		Provider provider6 = null;
		Provider provider7 = null;

        int sleepTime = 1200;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);

			provider4.bind(opts);
			
			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			// Add provider 4 as standby for group 3
			wsbGroup3.add(provider4);
			
			/* Create provider reactor 5. */
			providerReactor5 = new TestReactor(true);
			
			/* Create provider 5. */
			provider5 = createDefaultProvider(provider5, providerReactor5);
			
			provider5.bind(opts);
			
			/* Create provider reactor 6. */
			providerReactor6 = new TestReactor(true);
			 
			/* Create provider 6. */
			provider6 = createDefaultProvider(provider6, providerReactor6);
			
			provider6.bind(opts);
			
			/* Create provider reactor 7. */
			providerReactor7 = new TestReactor(true);
			
			/* Create provider 7. */
			provider7 = createDefaultProvider(provider7, providerReactor7);
			
			provider7.bind(opts);
			
			// Set preferred host options, with WSB group index to 2 and connectionList index set to 2
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(2);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(2);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider5);
			channelList.add(provider6);
			channelList.add(provider7);
			
			int port1 = provider.serverPort();
			int port2 = provider2.serverPort();
			int port3 = provider3.serverPort();
			//int port4 = provider4.serverPort();	We don't use this
			int port5 = provider5.serverPort();
			int port6 = provider6.serverPort();
			int port7 = provider7.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			provider4.close();
			provider5.close();
			provider6.close();
			provider7.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port3, 500, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port7, sleepTime, true);
 
    		checkChannelDownReconnecting(consumer, port3, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port5, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port7, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port6, sleepTime, true);

		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
	       providerReactor5.close();
	       providerReactor6.close();
	       providerReactor7.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ConnectionList_AllDownTest_MidPreferred()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ConnectionList_AllDownTest_MidPreferred <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 1 and WSB Group Index 1
		// Consumer should attempt to connect to preferred group (Group 2)
		// Failing that, it should move through the groups and channels properly
		
		// Group 1
		// Group 2*
		// Group 3
		// Channel 1
		// Channel 2*
		// Channel 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		TestReactor providerReactor5 = null;
		TestReactor providerReactor6 = null;
		TestReactor providerReactor7 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		Provider provider5 = null;
		Provider provider6 = null;
		Provider provider7 = null;

        int sleepTime = 1200;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);

			provider4.bind(opts);
			
			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			// Add provider 4 as standby for group 3
			wsbGroup3.add(provider4);
			
			/* Create provider reactor 5. */
			providerReactor5 = new TestReactor(true);
			
			/* Create provider 5. */
			provider5 = createDefaultProvider(provider5, providerReactor5);
			
			provider5.bind(opts);
			
			/* Create provider reactor 6. */
			providerReactor6 = new TestReactor(true);
			 
			/* Create provider 6. */
			provider6 = createDefaultProvider(provider6, providerReactor6);
			
			provider6.bind(opts);
			
			/* Create provider reactor 7. */
			providerReactor7 = new TestReactor(true);
			
			/* Create provider 7. */
			provider7 = createDefaultProvider(provider7, providerReactor7);
			
			provider7.bind(opts);
			
			// Set preferred host options, with WSB group index to 1 and connectionList index set to 1
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(1);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(1);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider5);
			channelList.add(provider6);
			channelList.add(provider7);
			
			int port1 = provider.serverPort();
			int port2 = provider2.serverPort();
			int port3 = provider3.serverPort();
			//int port4 = provider4.serverPort();	We don't use this
			int port5 = provider5.serverPort();
			int port6 = provider6.serverPort();
			int port7 = provider7.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			provider4.close();
			provider5.close();
			provider6.close();
			provider7.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);
			
    		checkChannelDownReconnecting(consumer, port2, 500, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port6, sleepTime, true);
 
    		checkChannelDownReconnecting(consumer, port2, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port5, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port6, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port7, sleepTime, true);

		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
	       providerReactor5.close();
	       providerReactor6.close();
	       providerReactor7.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ConnectionList_AllDownTest_StartPreferred()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ConnectionList_AllDownTest_StartPreferred <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 0 and WSB Group Index 0
		// Consumer should attempt to connect to preferred group (Group 1)
		// Failing that, it should move through the groups and channels properly
		
		// Group 1*
		// Group 2
		// Group 3
		// Channel 1*
		// Channel 2
		// Channel 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		TestReactor providerReactor5 = null;
		TestReactor providerReactor6 = null;
		TestReactor providerReactor7 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		Provider provider5 = null;
		Provider provider6 = null;
		Provider provider7 = null;

        int sleepTime = 1200;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);

			provider4.bind(opts);
			
			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			// Add provider 4 as standby for group 3
			wsbGroup3.add(provider4);
			
			/* Create provider reactor 5. */
			providerReactor5 = new TestReactor(true);
			
			/* Create provider 5. */
			provider5 = createDefaultProvider(provider5, providerReactor5);
			
			provider5.bind(opts);
			
			/* Create provider reactor 6. */
			providerReactor6 = new TestReactor(true);
			 
			/* Create provider 6. */
			provider6 = createDefaultProvider(provider6, providerReactor6);
			
			provider6.bind(opts);
			
			/* Create provider reactor 7. */
			providerReactor7 = new TestReactor(true);
			
			/* Create provider 7. */
			provider7 = createDefaultProvider(provider7, providerReactor7);
			
			provider7.bind(opts);
			
			// Set preferred host options, with WSB group index to 0 and connectionList index set to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider5);
			channelList.add(provider6);
			channelList.add(provider7);
			
			int port1 = provider.serverPort();
			int port2 = provider2.serverPort();
			int port3 = provider3.serverPort();
			//int port4 = provider4.serverPort();	We don't use this
			int port5 = provider5.serverPort();
			int port6 = provider6.serverPort();
			int port7 = provider7.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			provider4.close();
			provider5.close();
			provider6.close();
			provider7.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port1, 500, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port5, sleepTime, true);
 
    		checkChannelDownReconnecting(consumer, port1, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port6, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port5, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port7, sleepTime, true);

		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
	       providerReactor5.close();
	       providerReactor6.close();
	       providerReactor7.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ConnectionList_AllDownTest_StartPreferred_2WSBGroups_3ChannelHosts()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ConnectionList_AllDownTest_StartPreferred_2WSBGroups_3ChannelHosts <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 0 and WSB Group Index 0
		// Consumer should attempt to connect to preferred group (Group 1)
		// Failing that, it should move through the groups and channels properly
		
		// Group 1*
		// Group 2
		// Channel 1*
		// Channel 2
		// Channel 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		TestReactor providerReactor5 = null;
		TestReactor providerReactor6 = null;
		TestReactor providerReactor7 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		Provider provider5 = null;
		Provider provider6 = null;
		Provider provider7 = null;

        int sleepTime = 1200;
        int reconnectAttemptLimit = 1;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(reconnectAttemptLimit);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);

			provider4.bind(opts);
			
			// Add provider 3 as active for group 2, and provider 4 as standby for group 2
			wsbGroup2.add(provider3);
			wsbGroup2.add(provider4);
			
			/* Create provider reactor 5. */
			providerReactor5 = new TestReactor(true);
			
			/* Create provider 5. */
			provider5 = createDefaultProvider(provider5, providerReactor5);
			
			provider5.bind(opts);
			
			/* Create provider reactor 6. */
			providerReactor6 = new TestReactor(true);
			 
			/* Create provider 6. */
			provider6 = createDefaultProvider(provider6, providerReactor6);
			
			provider6.bind(opts);
			
			/* Create provider reactor 7. */
			providerReactor7 = new TestReactor(true);
			
			/* Create provider 7. */
			provider7 = createDefaultProvider(provider7, providerReactor7);
			
			provider7.bind(opts);
			
			// Set preferred host options, with WSB group index to 0 and connectionList index set to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider5);
			channelList.add(provider6);
			channelList.add(provider7);
			
			int port1 = provider.serverPort();
			//int port2 = provider2.serverPort();	We don't use this, it's standby
			int port3 = provider3.serverPort();
			//int port4 = provider4.serverPort();	We don't use this, it's standby
			int port5 = provider5.serverPort();
			int port6 = provider6.serverPort();
			int port7 = provider7.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, null, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			provider4.close();
			provider5.close();
			provider6.close();
			provider7.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port1, 500, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port5, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port6, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port5, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port7, sleepTime, true);

		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
	       providerReactor5.close();
	       providerReactor6.close();
	       providerReactor7.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_negative()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_negative <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 0 and WSB Group Index 0
		// Consumer should attempt to connect to preferred group (Group 1)
		// Failing that, it should move through the groups and channels properly
		// Reconnect attempt limit of -1 should make us reconnect endlessly to the first group
		
		// Group 1*
		// Group 2
		// Group 3
		// Channel 1*
		// Channel 2
		// Channel 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		TestReactor providerReactor5 = null;
		TestReactor providerReactor6 = null;
		TestReactor providerReactor7 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		Provider provider5 = null;
		Provider provider6 = null;
		Provider provider7 = null;

        int sleepTime = 1200;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(-1);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);

			provider4.bind(opts);
			
			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			// Add provider 4 as standby for group 3
			wsbGroup3.add(provider4);
			
			/* Create provider reactor 5. */
			providerReactor5 = new TestReactor(true);
			
			/* Create provider 5. */
			provider5 = createDefaultProvider(provider5, providerReactor5);
			
			provider5.bind(opts);
			
			/* Create provider reactor 6. */
			providerReactor6 = new TestReactor(true);
			 
			/* Create provider 6. */
			provider6 = createDefaultProvider(provider6, providerReactor6);
			
			provider6.bind(opts);
			
			/* Create provider reactor 7. */
			providerReactor7 = new TestReactor(true);
			
			/* Create provider 7. */
			provider7 = createDefaultProvider(provider7, providerReactor7);

			provider7.bind(opts);
			
			// Set preferred host options, with WSB group index to 0 and connectionList index set to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider5);
			channelList.add(provider6);
			channelList.add(provider7);
			
			int port1 = provider.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			provider4.close();
			provider5.close();
			provider6.close();
			provider7.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port1, 500, false);
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
	       providerReactor5.close();
	       providerReactor6.close();
	       providerReactor7.close();
		}

	}

	@Test
	public void PreferredHostWarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_0()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_0 <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 0 and WSB Group Index 0
		// Consumer should attempt to connect to preferred group (Group 1)
		// Failing that, we will get a channel close and finish.
		
		// Group 1*
		// Group 2
		// Group 3
		// Channel 1*
		// Channel 2
		// Channel 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		TestReactor providerReactor5 = null;
		TestReactor providerReactor6 = null;
		TestReactor providerReactor7 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		Provider provider5 = null;
		Provider provider6 = null;
		Provider provider7 = null;

		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(0);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);

			provider4.bind(opts);
			
			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			// Add provider 4 as standby for group 3
			wsbGroup3.add(provider4);
			
			/* Create provider reactor 5. */
			providerReactor5 = new TestReactor(true);
			
			/* Create provider 5. */
			provider5 = createDefaultProvider(provider5, providerReactor5);
			
			provider5.bind(opts);
			
			/* Create provider reactor 6. */
			providerReactor6 = new TestReactor(true);
			 
			/* Create provider 6. */
			provider6 = createDefaultProvider(provider6, providerReactor6);
			
			provider6.bind(opts);
			
			/* Create provider reactor 7. */
			providerReactor7 = new TestReactor(true);
			
			/* Create provider 7. */
			provider7 = createDefaultProvider(provider7, providerReactor7);

			provider7.bind(opts);
			
			// Set preferred host options, with WSB group index to 0 and connectionList index set to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider5);
			channelList.add(provider6);
			channelList.add(provider7);

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			provider4.close();
			provider5.close();
			provider6.close();
			provider7.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		// We should get Channel_Down on our first connection attempt
    		
    		try {
    			Thread.sleep(500);
    		} catch (InterruptedException e) {
    			e.printStackTrace();
    		}

    		consumer.testReactor().dispatch(1);

    		/* Channel down */
    		TestReactorEvent event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
	       providerReactor5.close();
	       providerReactor6.close();
	       providerReactor7.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_1()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_1 <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 0 and WSB Group Index 0
		// Consumer should attempt to connect to preferred group (Group 1)
		// Failing that, it should move through the groups and channels properly
		// Reconnect attempt limit of 1 should be reached, and then we move to next group/channel
		
		// Group 1*
		// Group 2
		// Group 3
		// Channel 1*
		// Channel 2
		// Channel 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		TestReactor providerReactor5 = null;
		TestReactor providerReactor6 = null;
		TestReactor providerReactor7 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		Provider provider5 = null;
		Provider provider6 = null;
		Provider provider7 = null;

        int sleepTime = 1200;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);

			provider4.bind(opts);
			
			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			// Add provider 4 as standby for group 3
			wsbGroup3.add(provider4);
			
			/* Create provider reactor 5. */
			providerReactor5 = new TestReactor(true);
			
			/* Create provider 5. */
			provider5 = createDefaultProvider(provider5, providerReactor5);
			
			provider5.bind(opts);
			
			/* Create provider reactor 6. */
			providerReactor6 = new TestReactor(true);
			 
			/* Create provider 6. */
			provider6 = createDefaultProvider(provider6, providerReactor6);
			
			provider6.bind(opts);
			
			/* Create provider reactor 7. */
			providerReactor7 = new TestReactor(true);
			
			/* Create provider 7. */
			provider7 = createDefaultProvider(provider7, providerReactor7);

			provider7.bind(opts);
			
			// Set preferred host options, with WSB group index to 0 and connectionList index set to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider5);
			channelList.add(provider6);
			channelList.add(provider7);
			
			int port1 = provider.serverPort();
			int port2 = provider2.serverPort();
			int port3 = provider3.serverPort();
			//int port4 = provider4.serverPort();	We don't use this
			int port5 = provider5.serverPort();
			int port6 = provider6.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			provider4.close();
			provider5.close();
			provider6.close();
			provider7.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port1, 500, false);
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
		
    		checkChannelDownReconnecting(consumer, port5, sleepTime, true);
 
    		checkChannelDownReconnecting(consumer, port1, sleepTime, true);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
	
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
		
    		checkChannelDownReconnecting(consumer, port6, sleepTime, true);

		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
	       providerReactor5.close();
	       providerReactor6.close();
	       providerReactor7.close();
		}

	}
	
	@Test
	public void PreferredHost_Disabled_WarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_1()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHost_Disabled_WarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_1 <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 0 and WSB Group Index 0
		// Consumer should attempt to connect to preferred group (Group 1)
		// Failing that, it should move through the groups and channels properly
		// Reconnect attempt limit of 1 should be reached, and then we move to next group/channel
		
		// Preferred Host disabled
		// Group 1
		// Group 2
		// Group 3
		// Channel 1
		// Channel 2
		// Channel 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		TestReactor providerReactor5 = null;
		TestReactor providerReactor6 = null;
		TestReactor providerReactor7 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		Provider provider5 = null;
		Provider provider6 = null;
		Provider provider7 = null;

        int sleepTime = 1200;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);

			provider4.bind(opts);
			
			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			// Add provider 4 as standby for group 3
			wsbGroup3.add(provider4);
			
			/* Create provider reactor 5. */
			providerReactor5 = new TestReactor(true);
			
			/* Create provider 5. */
			provider5 = createDefaultProvider(provider5, providerReactor5);
			
			provider5.bind(opts);
			
			/* Create provider reactor 6. */
			providerReactor6 = new TestReactor(true);
			 
			/* Create provider 6. */
			provider6 = createDefaultProvider(provider6, providerReactor6);
			
			provider6.bind(opts);
			
			/* Create provider reactor 7. */
			providerReactor7 = new TestReactor(true);
			
			/* Create provider 7. */
			provider7 = createDefaultProvider(provider7, providerReactor7);

			provider7.bind(opts);
			
			// Set preferred host options to false
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(false);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider5);
			channelList.add(provider6);
			channelList.add(provider7);
			
			int port1 = provider.serverPort();
			int port2 = provider2.serverPort();
			int port3 = provider3.serverPort();
			int port5 = provider5.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			provider4.close();
			provider5.close();
			provider6.close();
			provider7.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port1, 500, false);
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

			try {
				Thread.sleep(sleepTime);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			consumer.testReactor().switchingReactorChannel = true;

			consumer.testReactor().dispatch(1);

			/* Channel down */
			TestReactorEvent event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
			int compare = Integer.valueOf(channelEvent.reactorChannel().reactor()._reactorChannelQueue.peek().getCurrentReactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName());
			assertEquals("ChannelDown check failed. ", port5, compare);
		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
	       providerReactor5.close();
	       providerReactor6.close();
	       providerReactor7.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_2()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_2 <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 0 and WSB Group Index 0
		// Consumer should attempt to connect to preferred group (Group 1)
		// Failing that, it should move through the groups and channels properly
		// Reconnect attempt limit of 2 should be reached, and then we move to next group/channel
		
		// Group 1*
		// Group 2
		// Group 3
		// Channel 1*
		// Channel 2
		// Channel 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		TestReactor providerReactor5 = null;
		TestReactor providerReactor6 = null;
		TestReactor providerReactor7 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		Provider provider5 = null;
		Provider provider6 = null;
		Provider provider7 = null;

        int sleepTime = 1200;
        int reconnectAttemptLimit = 2;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(reconnectAttemptLimit);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);

			provider4.bind(opts);
			
			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			// Add provider 4 as standby for group 3
			wsbGroup3.add(provider4);
			
			/* Create provider reactor 5. */
			providerReactor5 = new TestReactor(true);
			
			/* Create provider 5. */
			provider5 = createDefaultProvider(provider5, providerReactor5);
			
			provider5.bind(opts);
			
			/* Create provider reactor 6. */
			providerReactor6 = new TestReactor(true);
			 
			/* Create provider 6. */
			provider6 = createDefaultProvider(provider6, providerReactor6);
			
			provider6.bind(opts);
			
			/* Create provider reactor 7. */
			providerReactor7 = new TestReactor(true);
			
			/* Create provider 7. */
			provider7 = createDefaultProvider(provider7, providerReactor7);

			provider7.bind(opts);
			
			// Set preferred host options, with WSB group index to 0 and connectionList index set to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider5);
			channelList.add(provider6);
			channelList.add(provider7);
			
			int port1 = provider.serverPort();
			int port2 = provider2.serverPort();
			int port3 = provider3.serverPort();
			//int port4 = provider4.serverPort();	We don't use this
			int port5 = provider5.serverPort();
			int port6 = provider6.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			provider4.close();
			provider5.close();
			provider6.close();
			provider7.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port1, 500, false, 1);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false, reconnectAttemptLimit);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false, reconnectAttemptLimit);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false, reconnectAttemptLimit);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false, reconnectAttemptLimit);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false, reconnectAttemptLimit);

    		checkChannelDownReconnecting(consumer, port5, sleepTime, true, 1);
 
    		checkChannelDownReconnecting(consumer, port1, sleepTime, true, reconnectAttemptLimit);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false, reconnectAttemptLimit);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false, reconnectAttemptLimit);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false, reconnectAttemptLimit);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false, reconnectAttemptLimit);
    		
    		checkChannelDownReconnecting(consumer, port6, sleepTime, true, 1);

		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
	       providerReactor5.close();
	       providerReactor6.close();
	       providerReactor7.close();
		}

	}
	
	@Test
	public void PreferredHost_Disabled_WarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_2()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHost_Disabled_WarmStandby_ConnectionList_AllDownTest_ReconnectAttemptLimit_2 <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 0 and WSB Group Index 0
		// Consumer should attempt to connect to preferred group (Group 1)
		// Failing that, it should move through the groups and channels properly
		// Reconnect attempt limit of 2 should be reached, and then we move to next group/channel
		
		// Preferred Host disabled
		// Group 1
		// Group 2
		// Group 3
		// Channel 1
		// Channel 2
		// Channel 3
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;
		TestReactor providerReactor4 = null;
		TestReactor providerReactor5 = null;
		TestReactor providerReactor6 = null;
		TestReactor providerReactor7 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;
		Provider provider4 = null;
		Provider provider5 = null;
		Provider provider6 = null;
		Provider provider7 = null;

        int sleepTime = 1200;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		List<Provider> wsbGroup2 = new ArrayList<Provider>();
		List<Provider> wsbGroup3 = new ArrayList<Provider>();
		
		int reconnectAttemptLimit = 2;
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(reconnectAttemptLimit);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			wsbGroup2.add(provider2);
			// Add provider 1 as standby for group 2
			wsbGroup2.add(provider);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			/* Create provider reactor 4. */
			providerReactor4 = new TestReactor(true);
			
			/* Create provider 4. */
			provider4 = createDefaultProvider(provider4, providerReactor4);

			provider4.bind(opts);
			
			// Add provider 3 as starting for group 3
			wsbGroup3.add(provider3);
			// Add provider 4 as standby for group 3
			wsbGroup3.add(provider4);
			
			/* Create provider reactor 5. */
			providerReactor5 = new TestReactor(true);
			
			/* Create provider 5. */
			provider5 = createDefaultProvider(provider5, providerReactor5);
			
			provider5.bind(opts);
			
			/* Create provider reactor 6. */
			providerReactor6 = new TestReactor(true);
			 
			/* Create provider 6. */
			provider6 = createDefaultProvider(provider6, providerReactor6);
			
			provider6.bind(opts);
			
			/* Create provider reactor 7. */
			providerReactor7 = new TestReactor(true);
			
			/* Create provider 7. */
			provider7 = createDefaultProvider(provider7, providerReactor7);

			provider7.bind(opts);
			
			// Set preferred host options to false
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(false);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider5);
			channelList.add(provider6);
			channelList.add(provider7);
			
			int port1 = provider.serverPort();
			int port2 = provider2.serverPort();
			int port3 = provider3.serverPort();
			int port5 = provider5.serverPort();
			int port6 = provider6.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, wsbGroup2, wsbGroup3, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			provider4.close();
			provider5.close();
			provider6.close();
			provider7.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port1, 500, false);
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false, reconnectAttemptLimit);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false, reconnectAttemptLimit);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false, reconnectAttemptLimit);
    		
    		checkChannelDownReconnecting(consumer, port5, sleepTime, true);

			try {
				Thread.sleep(sleepTime);
			} catch (InterruptedException e) {
				e.printStackTrace();
			}
			
			consumer.testReactor().switchingReactorChannel = true;

			consumer.testReactor().dispatch(1);

			/* Channel down */
			TestReactorEvent event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
			int compare = Integer.valueOf(channelEvent.reactorChannel().reactor()._reactorChannelQueue.peek().getCurrentReactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName());
			assertEquals("ChannelDown check failed. ", port6, compare);
		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
	       providerReactor4.close();
	       providerReactor5.close();
	       providerReactor6.close();
	       providerReactor7.close();
		}

	}
	
	@Test
	public void PreferredHostConnectionList_AllDownTest_EndPreferred()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_AllDownTest_EndPreferred <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 2
		// Consumer should attempt to connect to preferred host
		// Failing that, we should go through the list of connections in correct order because reconnectAttemptLimit is -1

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;

		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;


        int sleepTime = 1200;

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(-1);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);

			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			// Set preferred host options with connectionList index set to 2
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(2);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider);
			channelList.add(provider2);
			channelList.add(provider3);

			int port1 = provider.serverPort();
			int port2 = provider2.serverPort();
			int port3 = provider3.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, null, null, null, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port3, 500, false);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	
	@Test
	public void PreferredHostConnectionList_AllDownTest_MidPreferred()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_AllDownTest_MidPreferred <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 1
		// Consumer should attempt to connect to preferred host
		// Failing that, we should go through all connections appropriately because reconnectAttemptLimit is -1

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;

		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;


        int sleepTime = 1200;

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(-1);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);

			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			// Set preferred host options with connectionList index set to 1
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(1);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider);
			channelList.add(provider2);
			channelList.add(provider3);

			int port1 = provider.serverPort();
			int port2 = provider2.serverPort();
			int port3 = provider3.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, null, null, null, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port2, 500, false);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostConnectionList_AllDownTest_StartPreferred()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_AllDownTest_StartPreferred <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 0
		// Consumer should attempt to connect to preferred host
		// Failing that, we should go through the list of connections in appropriate order because reconnectAttemptLimit is -1

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;

		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;


        int sleepTime = 1200;

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(-1);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);

			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);

			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);
			
			// Set preferred host options with connectionList index set to 1
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider);
			channelList.add(provider2);
			channelList.add(provider3);
			
			int port1 = provider.serverPort();
			int port2 = provider2.serverPort();
			int port3 = provider3.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, null, null, null, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);
		
    		checkChannelDownReconnecting(consumer, port1, 500, false);

    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);

    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port2, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, false);
		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();
		}

	}
	
	@Test
	public void PreferredHostConnectionList_AllDownTest_OnlyOneHostConfigured()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostConnectionList_AllDownTest_OnlyOneHostConfigured <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 0
		// Consumer should attempt to connect to preferred host
		// Only 1 provider is configured and it is the preferred host, so we should just retry it infinitely because reconnectAttemptLimit is -1

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;

		Consumer consumer = null;
		Provider provider = null;

        int sleepTime = 1200;

		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(-1);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);

			// Set preferred host options with connectionList index set to 1
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider);
			
			int port1 = provider.serverPort();
			
			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, null, null, null, channelList);

			// Kill all of the active providers
			provider.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port1, 500, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
		}

	}
	
	@Test
	public void PreferredHostWarmStandby_ConnectionList_AllDownTest_OnlyOneGroupAndHostConfigured()
	{
		System.out.println("\n>>>>>>>>> Running PreferredHostWarmStandby_ConnectionList_AllDownTest_OnlyOneGroupAndHostConfigured <<<<<<<<<<\n");

		// No providers started
		// Consumer configured for ConnectionListIndex 0 and WSB Group Index 0
		// Consumer should attempt to connect to preferred group (Group 1)
		// Failing that, it should move through the groups and channels properly
		
		// Group 1*
		// Channel 1*
		
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		TestReactor providerReactor3 = null;

		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		Provider provider3 = null;

        int sleepTime = 1200;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor(true);
			ReactorCallbackHandler consumerCallbackHandler = null;
			Selector consumerSelector = null;	  
			
			/* Create consumer. */
			consumerCallbackHandler = new ReactorCallbackHandler(consumerSelector);
			assertEquals(null, consumerCallbackHandler.lastChannelEvent());
			
			consumer = new Consumer(consumerReactor);
			ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
			consumerRole.initDefaultRDMLoginRequest();
			consumerRole.initDefaultRDMDirectoryRequest();
			consumerRole.channelEventCallback(consumer);
			consumerRole.loginMsgCallback(consumer);      
			consumerRole.directoryMsgCallback(consumer);
			consumerRole.dictionaryMsgCallback(consumer);
			consumerRole.defaultMsgCallback(consumer);
			
			consumerRole.watchlistOptions().enableWatchlist(true);

			// Connect the consumer and providers.
			ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
			opts.wsbMode(ReactorWarmStandbyMode.LOGIN_BASED);
			opts.reconnectAttemptLimit(1);
			opts.reconnectMinDelay(1000);
			opts.reconnectMaxDelay(1000);
			opts.consumerChannelInitTimeout(1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor(true);
			
			/* Create provider. */
			provider = createDefaultProvider(provider, providerReactor);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor(true);
			
			/* Create provider 2. */
			provider2 = createDefaultProvider(provider2, providerReactor2);
			
			provider2.bind(opts);
			
			// Add this provider as standby for group 1 and starting for group 2
			wsbGroup1.add(provider2);
			
			/* Create provider reactor 3. */
			providerReactor3 = new TestReactor(true);
			
			/* Create provider 3. */
			provider3 = createDefaultProvider(provider3, providerReactor3);
			
			provider3.bind(opts);

			// Set preferred host options, with WSB group index to 0 and connectionList index set to 0
			ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
			connectOpts.reactorPreferredHostOptions().isPreferredHostEnabled(true);
			connectOpts.reactorPreferredHostOptions().warmStandbyGroupListIndex(0);
			connectOpts.reactorPreferredHostOptions().connectionListIndex(0);
			
			List<Provider> channelList = new LinkedList<Provider>();
			channelList.add(provider3);

			int port1 = provider.serverPort();
			int port3 = provider3.serverPort();

			consumerReactor.connectWsbNoStart(connectOpts, opts, consumer, wsbGroup1, null, null, channelList);

			// Kill all of the active providers
			provider.close();
			provider2.close();
			provider3.close();
			
			consumerReactor.lateStartConnect(connectOpts, consumer);

    		checkChannelDownReconnecting(consumer, port1, 500, false);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, false);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port3, sleepTime, true);
    		
    		checkChannelDownReconnecting(consumer, port1, sleepTime, true);
		}
		finally
		{
	       consumer.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
	       providerReactor3.close();

		}

	}
}
