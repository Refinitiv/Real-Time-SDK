/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2022,2025 LSEG. All rights reserved.
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
import java.util.List;
import java.util.Objects;
import java.util.Optional;

import org.junit.Before;
import org.junit.Test;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CloseMsg;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataDictionary;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DataTypes;
import com.refinitiv.eta.codec.GenericMsg;
import com.refinitiv.eta.codec.MapEntryActions;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.MsgClasses;
import com.refinitiv.eta.codec.PostMsg;
import com.refinitiv.eta.codec.RefreshMsg;
import com.refinitiv.eta.codec.RequestMsg;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StatusMsg;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.codec.UpdateMsg;
import com.refinitiv.eta.rdm.DomainTypes;
import com.refinitiv.eta.rdm.Directory.WarmStandbyDirectoryServiceTypes;
import com.refinitiv.eta.rdm.Login.ServerTypes;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.dictionary.DictionaryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryConsumerStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginConsumerConnectionStatus;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;

public class ReactorWatchlistWarmStandbyJunit {
	private static final Buffer proxyHost = CodecFactory.createBuffer();
	private static final Buffer proxyPort = CodecFactory.createBuffer();
	private static final Buffer proxyUser = CodecFactory.createBuffer();
	private static final Buffer proxyPassword = CodecFactory.createBuffer();
	private static final Buffer proxyLocalHostname = CodecFactory.createBuffer();
	private static final Buffer proxyDomain = CodecFactory.createBuffer();

	public ReactorWatchlistWarmStandbyJunit() {
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
	

	
	@Test
	public void WarmStandbyLoginConnectTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyLoginConnectTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
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
        CloseMsg closeMsg = (CloseMsg)msg;
        CloseMsg receivedCloseMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor();
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
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        
	        consumer.testReactor().dispatch(-1);
			
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
            
            /* Consumer sends close message */
            closeMsg.clear();
            closeMsg.msgClass(MsgClasses.CLOSE);
            closeMsg.domainType(DomainTypes.MARKET_PRICE);
            closeMsg.streamId(5);
            closeMsg.containerType(DataTypes.NO_DATA);
            assertTrue(consumer.submitAndDispatch(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 1 receives close. */
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
            
            receivedCloseMsg = (CloseMsg)msgEvent.msg();
            assertEquals(receivedCloseMsg.streamId(), providerStreamId);
            
            /* Provider 2 receives close. */
            provider2.testReactor().dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
            
            receivedCloseMsg = (CloseMsg)msgEvent.msg();
            assertEquals(receivedCloseMsg.streamId(), providerStreamId);

		}
		finally
		{
			//TestReactorComponent.closeSession(consumer, provider);
	       consumer.close();
	       provider.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}
	
	@Test
	public void WarmStandbyLoginGenericMsgTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyLoginGenericMsgTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
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
	    GenericMsg genericMsg = (GenericMsg)msg;
	    GenericMsg receivedGenericMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor();
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
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        
	        consumer.testReactor().dispatch(-1);
			
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
            refreshMsg.applyRefreshComplete();
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
            refreshMsg.applyRefreshComplete();
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
            
    		/* Consumer sends generic messages. */
            genericMsg.clear();
            genericMsg.msgClass(MsgClasses.GENERIC);
            genericMsg.streamId(5);
            genericMsg.domainType(DomainTypes.MARKET_PRICE);
            genericMsg.applyHasMsgKey();
            genericMsg.msgKey().applyHasName();
            genericMsg.msgKey().name().data("TRI.N");
            genericMsg.msgKey().applyHasServiceId();
            genericMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            submitOptions.clear();

            assertTrue(consumer.submitAndDispatch(genericMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            // Provider 1 gets generic msg
            providerReactor.dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
            
            receivedGenericMsg = (GenericMsg)msgEvent.msg();
            assertTrue(receivedGenericMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedGenericMsg.msgKey().serviceId());
            assertTrue(receivedGenericMsg.msgKey().checkHasName());
            assertTrue(receivedGenericMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedGenericMsg.domainType());
            
            	
         // Provider 2 gets nothing
            providerReactor2.dispatch(0);

		}
		finally
		{
			//TestReactorComponent.closeSession(consumer, provider);
	       consumer.close();
	       provider.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}
	
	@Test
	public void WarmStandbyLoginPostMsgTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyLoginPostMsgTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
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
        PostMsg postMsg = (PostMsg)msg;
        PostMsg receivedPostMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor();
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
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        
	        consumer.testReactor().dispatch(-1);
			
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
            refreshMsg.applyRefreshComplete();
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
            refreshMsg.applyRefreshComplete();
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
            
            /* Consumer sends post messages. */
	        postMsg.clear();
	        postMsg.msgClass(MsgClasses.POST);
	        postMsg.streamId(5);
	        postMsg.domainType(DomainTypes.MARKET_PRICE);
	        postMsg.applyHasMsgKey();
	        postMsg.msgKey().applyHasName();
	        postMsg.msgKey().name().data("TRI.N");
	        postMsg.msgKey().applyHasServiceId();
	        postMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            submitOptions.clear();

            assertTrue(consumer.submitAndDispatch(postMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            // Provider 1 gets post msg
            providerReactor.dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.POST, msgEvent.msg().msgClass());
            
            receivedPostMsg = (PostMsg)msgEvent.msg();
            assertTrue(receivedPostMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedPostMsg.msgKey().serviceId());
            assertTrue(receivedPostMsg.msgKey().checkHasName());
            assertTrue(receivedPostMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedPostMsg.domainType());
            
            	
            // Provider 2 gets post message
            providerReactor2.dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.POST, msgEvent.msg().msgClass());
            
            receivedPostMsg = (PostMsg)msgEvent.msg();
            assertTrue(receivedPostMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedPostMsg.msgKey().serviceId());
            assertTrue(receivedPostMsg.msgKey().checkHasName());
            assertTrue(receivedPostMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedPostMsg.domainType());

		}
		finally
		{
			//TestReactorComponent.closeSession(consumer, provider);
	       consumer.close();
	       provider.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}
	
	@Test
	public void WarmStandbyLoginPrivateStreamTest()
	{
		WarmStandbyLoginPrivateStreamPreReadyTest(true);
		WarmStandbyLoginPrivateStreamPreReadyTest(false);
		
		WarmStandbyLoginPrivateStreamPostReadyTest(true);
		WarmStandbyLoginPrivateStreamPostReadyTest(false);
	}
	
	public void WarmStandbyLoginPrivateStreamPreReadyTest(boolean useServiceName)
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyLoginPrivateStreamPreReadyTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
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
			consumerReactor = new TestReactor();
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
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
			
            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            requestMsg.applyPrivateStream();
            submitOptions.clear();
            if(useServiceName)
            	submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            else
            {
            	requestMsg.msgKey().applyHasServiceId();
            	requestMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            }
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
			
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
	        
	        consumer.testReactor().dispatch(-1);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 2nd connection, so this should set to 1 */
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
            refreshMsg.applySolicited();
            refreshMsg.applyPrivateStream();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives nothing. */
            provider2.testReactor().dispatch(0);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRefreshMsg.checkPrivateStream());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            if(useServiceName)
            {
	            assertNotNull(msgEvent.streamInfo().serviceName());
	            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            }
            
            // kill provider 1.
            provider.close();
            
            /* Consumer receives FD_CHANGE event and then an open suspect status*/
            consumerReactor.dispatch(2);
            
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
            
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.CLOSED_RECOVER);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
			
			// Provider 2 receives generic message saying that it's now the active.
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* new active, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);        
            
     
		}
		finally
		{
	       consumer.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}
	
	public void WarmStandbyLoginPrivateStreamPostReadyTest(boolean useServiceName)
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyLoginPrivateStreamPostReadyTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
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
			consumerReactor = new TestReactor();
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
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
    		
    		/* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            requestMsg.applyPrivateStream();
            submitOptions.clear();
            if(useServiceName)
            	submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            else
            {
            	requestMsg.msgKey().applyHasServiceId();
            	requestMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            }
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
    		
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
	        
	        consumer.testReactor().dispatch(-1);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 2nd connection, so this should set to 1 */
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
            refreshMsg.applySolicited();
            refreshMsg.applyPrivateStream();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 receives nothing. */
            provider2.testReactor().dispatch(0);
            
            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRefreshMsg.checkPrivateStream());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            if(useServiceName)
            {
	            assertNotNull(msgEvent.streamInfo().serviceName());
	            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            }
            
            // kill provider 1.
            provider.close();
            
            /* Consumer receives FD_CHANGE event and then an open suspect status*/
            consumerReactor.dispatch(2);
            
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
            
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.CLOSED_RECOVER);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
			
			// Provider 2 receives generic message saying that it's now the active.
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* new active, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);        
            
     
		}
		finally
		{
	       consumer.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}
	
	@Test
	public void WarmStandbyLoginDisconnectTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyLoginDisconnectTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
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
			consumerReactor = new TestReactor();
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
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        
	        consumer.testReactor().dispatch(-1);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 2nd connection, so this should set to 1 */
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
            refreshMsg.applySolicited();
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
            refreshMsg.applySolicited();
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
            
            // kill provider 1.
            provider.close();
            
            /* Consumer receives FD_CHANGE event and then an open suspect status*/
            consumerReactor.dispatch(2);
            
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
            
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
 

			
			// Provider 2 receives generic message saying that it's now the active.
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(1);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* new active, so this should set to 0 */
			assertEquals(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).warmStandbyInfo().warmStandbyMode(), ServerTypes.ACTIVE);
			
			// Send unsolicited refresh from provider 2
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
            assertTrue(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));         
            
     
		}
		finally
		{
	       consumer.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}
	
	@Test
	public void WarmStandbyPauseFailureTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyPauseFailureTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
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
			consumerReactor = new TestReactor();
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
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        
	        consumer.testReactor().dispatch(-1);
			
			/* Provider receives consumer connection status and directory request. */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
			loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
			assertEquals(LoginMsgType.CONSUMER_CONNECTION_STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
			assertTrue(((LoginConsumerConnectionStatus)loginMsgEvent.rdmLoginMsg()).checkHasWarmStandbyInfo());
			/* 2nd connection, so this should set to 1 */
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
            refreshMsg.applySolicited();
            refreshMsg.applyRefreshComplete();
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
            refreshMsg.applySolicited();
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
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
            
            /* Consumer sends pause request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            requestMsg.applyPause();
            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertFalse(consumer.submitAndDispatch(requestMsg, submitOptions, true) >= ReactorReturnCodes.SUCCESS);
            
     
		}
		finally
		{
			//TestReactorComponent.closeSession(consumer, provider);
	       consumer.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}	
	
	@Test
	public void WarmStandbyLoginConnectFailureTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyLoginConnectFailureTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor();
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
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        loginRefresh.features().supportStandbyMode(2);
	        loginRefresh.state().streamState(StreamStates.OPEN);
	        loginRefresh.state().dataState(DataStates.OK);
	        loginRefresh.state().code(StateCodes.NONE);
	        loginRefresh.state().text().data("Login OK");
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatchFailure(1, 1000, false);
	        
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());


		}
		finally
		{
			//TestReactorComponent.closeSession(consumer, provider);
	       consumer.close();
	       provider.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}	
	
	@Test
	public void WarmStandbyServiceConnectFailureTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyServiceConnectFailureTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor();
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
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        loginRefresh.features().supportStandbyMode(0);
	        loginRefresh.state().streamState(StreamStates.OPEN);
	        loginRefresh.state().dataState(DataStates.OK);
	        loginRefresh.state().code(StateCodes.NONE);
	        loginRefresh.state().text().data("Login OK");
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        consumer.testReactor().dispatchFailure(1, 1000, false);
	        
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
		}
		finally
		{
			//TestReactorComponent.closeSession(consumer, provider);
	       consumer.close();
	       provider.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}	
	

	@Test
	public void WarmStandbyServiceConnectTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyServiceConnectTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
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
        CloseMsg closeMsg = (CloseMsg)msg;
        CloseMsg receivedCloseMsg;
        int providerStreamId;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor();
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
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
    		
    		
    		/* Provider receives directory status. */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());

			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
    		
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
	        
	        consumer.testReactor().dispatch(-1);
			
			/* Provider receives consumer connection status and directory request. */
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
	        
	        /* Provider 2 receives directory status and request */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());

			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.STANDBY);

			 /* Provider 2 receives request. */
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
            refreshMsg.applyRefreshComplete();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
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
            refreshMsg.applyRefreshComplete();
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
            
            /* Consumer sends close message */
            closeMsg.clear();
            closeMsg.msgClass(MsgClasses.CLOSE);
            closeMsg.domainType(DomainTypes.MARKET_PRICE);
            closeMsg.streamId(5);
            closeMsg.containerType(DataTypes.NO_DATA);
            assertTrue(consumer.submitAndDispatch(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 1 receives close. */
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
            
            receivedCloseMsg = (CloseMsg)msgEvent.msg();
            assertEquals(receivedCloseMsg.streamId(), providerStreamId);
            
            /* Provider 2 receives close. */
            provider2.testReactor().dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
            
            receivedCloseMsg = (CloseMsg)msgEvent.msg();
            assertEquals(receivedCloseMsg.streamId(), providerStreamId);
     
		}
		finally
		{
			//TestReactorComponent.closeSession(consumer, provider);
	       consumer.close();
	       provider.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}
	
	
	@Test
	public void WarmStandbyServicePrivateStreamTest()
	{
		WarmStandbyServicePrivateStreamPreReadyTest(true);
		WarmStandbyServicePrivateStreamPreReadyTest(false);
		WarmStandbyServicePrivateStreamPostReadyTest(true);
		WarmStandbyServicePrivateStreamPostReadyTest(false);
	}
	
	public void WarmStandbyServicePrivateStreamPreReadyTest(boolean useServiceName)
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyServicePrivateStreamPreReadyTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		DirectoryRefresh receivedDirectoryRefresh;
		DirectoryUpdate receivedDirectoryUpdate;
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
			consumerReactor = new TestReactor();
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
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
			
			/* Consumer sends requests. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            requestMsg.applyPrivateStream();
            submitOptions.clear();
            if(useServiceName)
            	submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            else
            {
            	requestMsg.msgKey().applyHasServiceId();
            	requestMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            }

            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            	
        	requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(6);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("IBM.N");
            requestMsg.applyPrivateStream();
            submitOptions.clear();
            if(useServiceName)
            	submitOptions.serviceName(Provider.defaultService2().info().serviceName().toString());
            else
            {
            	requestMsg.msgKey().applyHasServiceId();
            	requestMsg.msgKey().serviceId(Provider.defaultService2().serviceId());
            }
        
       	
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
			
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
	        Service service2 = DirectoryMsgFactory.createService();
	        Provider.defaultService2().copy(service2);
	        
	        directoryRefresh.serviceList().add(service);

	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
		    receivedDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent.rdmDirectoryMsg();
		    assertEquals(receivedDirectoryRefresh.serviceList().size(), 1);
		    assertEquals(receivedDirectoryRefresh.serviceList().get(0).info().serviceName().toString(), Provider.defaultService().info().serviceName().toString());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		
    		/* Provider receives directory status and request */
			provider.testReactor().dispatch(2);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
			
			/* Provider 1 receives request. */
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertTrue(receivedRequestMsg.checkPrivateStream());
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
            refreshMsg.applyPrivateStream();
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
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
            if(useServiceName)
            {
	            assertNotNull(msgEvent.streamInfo().serviceName());
	            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            }
			
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
	        
	        consumer.testReactor().dispatch(-1);
			
			/* Provider receives consumer connection status and directory request. */
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
	        
	        directoryRefresh.serviceList().add(service);
	        directoryRefresh.serviceList().add(service2);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);
	        
	        /* Consumer gets a directory update. */
	        event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
		    receivedDirectoryUpdate = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
		    assertEquals(receivedDirectoryUpdate.serviceList().size(), 1);
		    assertEquals(receivedDirectoryUpdate.serviceList().get(0).serviceId(),  Provider.defaultService2().serviceId());
    		
	        
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			
			
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
           	        
	        /* Provider 2 receives directory status and request */
			provider2.testReactor().dispatch(3);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.STANDBY);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service2.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);


			 /* Provider 2 receives request. */
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertTrue(receivedRequestMsg.checkPrivateStream());
            assertEquals(Provider.defaultService2().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 does not get anything */
            provider.testReactor().dispatch(0);
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyPrivateStream();
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService2().serviceId());
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
            assertEquals(Provider.defaultService2().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("IBM.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertTrue(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            if(useServiceName)
            {
	            assertNotNull(msgEvent.streamInfo().serviceName());
	            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService2().info().serviceName().toString()));
            }
                        
           
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
	
	public void WarmStandbyServicePrivateStreamPostReadyTest(boolean useServiceName)
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyServicePrivateStreamPostReadyTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		DirectoryRefresh receivedDirectoryRefresh;
		DirectoryUpdate receivedDirectoryUpdate;
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
			consumerReactor = new TestReactor();
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
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        Service service2 = DirectoryMsgFactory.createService();
	        Provider.defaultService2().copy(service2);
	        
	        directoryRefresh.serviceList().add(service);

	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
		    receivedDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent.rdmDirectoryMsg();
		    assertEquals(receivedDirectoryRefresh.serviceList().size(), 1);
		    assertEquals(receivedDirectoryRefresh.serviceList().get(0).info().serviceName().toString(), Provider.defaultService().info().serviceName().toString());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Consumer sends requests. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            requestMsg.applyPrivateStream();
            submitOptions.clear();
            if(useServiceName)
            	submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            else
            {
            	requestMsg.msgKey().applyHasServiceId();
            	requestMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            }

            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            	
        	requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(6);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("IBM.N");
            requestMsg.applyPrivateStream();
            submitOptions.clear();
            if(useServiceName)
            	submitOptions.serviceName(Provider.defaultService2().info().serviceName().toString());
            else
            {
            	requestMsg.msgKey().applyHasServiceId();
            	requestMsg.msgKey().serviceId(Provider.defaultService2().serviceId());
            }
        
       	
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
    		
    		
    		/* Provider receives directory status and request */
			provider.testReactor().dispatch(2);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
			
			/* Provider 1 receives request. */
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertTrue(receivedRequestMsg.checkPrivateStream());
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
            refreshMsg.applyPrivateStream();
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
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
            if(useServiceName)
            {
	            assertNotNull(msgEvent.streamInfo().serviceName());
	            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
            }
			
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
	        
	        consumer.testReactor().dispatch(-1);
			
			/* Provider receives consumer connection status and directory request. */
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
	        
	        directoryRefresh.serviceList().add(service);
	        directoryRefresh.serviceList().add(service2);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);
	        
	        /* Consumer gets a directory update. */
	        event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
		    receivedDirectoryUpdate = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
		    assertEquals(receivedDirectoryUpdate.serviceList().size(), 1);
		    assertEquals(receivedDirectoryUpdate.serviceList().get(0).serviceId(),  Provider.defaultService2().serviceId());
    		
	        
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			
			
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
           	        
	        /* Provider 2 receives directory status and request */
			provider2.testReactor().dispatch(3);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.STANDBY);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service2.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);


			 /* Provider 2 receives request. */
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertTrue(receivedRequestMsg.checkPrivateStream());
            assertEquals(Provider.defaultService2().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Provider 1 does not get anything */
            provider.testReactor().dispatch(0);
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyPrivateStream();
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService2().serviceId());
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
            assertEquals(Provider.defaultService2().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("IBM.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertTrue(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            if(useServiceName)
            {
	            assertNotNull(msgEvent.streamInfo().serviceName());
	            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService2().info().serviceName().toString()));
            }
                        
           
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
	public void WarmStandbyServiceGenericTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyServiceGenericTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		DirectoryRefresh receivedDirectoryRefresh;
		DirectoryUpdate receivedDirectoryUpdate;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        GenericMsg genericMsg = (GenericMsg)msg;
        GenericMsg receivedGenericMsg;
        int providerStreamId11;
        int providerStreamId21;
        int providerStreamId22;


        
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor();
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
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        Service service2 = DirectoryMsgFactory.createService();
	        Provider.defaultService2().copy(service2);
	        
	        directoryRefresh.serviceList().add(service);

	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
		    receivedDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent.rdmDirectoryMsg();
		    assertEquals(receivedDirectoryRefresh.serviceList().size(), 1);
		    assertEquals(receivedDirectoryRefresh.serviceList().get(0).info().serviceName().toString(), Provider.defaultService().info().serviceName().toString());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Consumer sends requests. */
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

            	
        	requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(6);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("IBM.N");
            submitOptions.clear();
           	submitOptions.serviceName(Provider.defaultService2().info().serviceName().toString());

        
       	
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
    		
    		
    		/* Provider receives directory status and request */
			provider.testReactor().dispatch(2);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
			
			/* Provider 1 receives request. */
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
            
            
            providerStreamId11 = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId11);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
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
	        
	        consumer.testReactor().dispatch(-1);
			
			/* Provider receives consumer connection status and directory request. */
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
	        
	        directoryRefresh.serviceList().add(service);
	        directoryRefresh.serviceList().add(service2);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);
	        
	        /* Consumer gets a directory update. */
	        event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
		    receivedDirectoryUpdate = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
		    assertEquals(receivedDirectoryUpdate.serviceList().size(), 1);
		    assertEquals(receivedDirectoryUpdate.serviceList().get(0).serviceId(),  Provider.defaultService2().serviceId());
    		
	        
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			
			
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
           	        
	        /* Provider 2 receives directory status and request */
			provider2.testReactor().dispatch(4);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.STANDBY);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service2.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);


			 /* Provider 2 receives request 1. */
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
            
            providerStreamId21 = receivedRequestMsg.streamId();

			
            /* Provider 2 receives request 2. */
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService2().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId22 = receivedRequestMsg.streamId();
            
            /* Provider 1 does not get anything */
            provider.testReactor().dispatch(0);
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId21);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 sends refresh.*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId22);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService2().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("IBM.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives one refresh. */
            consumerReactor.dispatch(1);
            
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService2().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("IBM.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertTrue(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
	        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService2().info().serviceName().toString()));
	        
    		/* Consumer sends generic messages. */
            genericMsg.clear();
            genericMsg.msgClass(MsgClasses.GENERIC);
            genericMsg.streamId(5);
            genericMsg.domainType(DomainTypes.MARKET_PRICE);
            genericMsg.applyHasMsgKey();
            genericMsg.msgKey().applyHasName();
            genericMsg.msgKey().name().data("TRI.N");
            genericMsg.msgKey().applyHasServiceId();
            genericMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            submitOptions.clear();

            assertTrue(consumer.submitAndDispatch(genericMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            // Provider 1 gets generic msg
            providerReactor.dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
            
            receivedGenericMsg = (GenericMsg)msgEvent.msg();
            assertTrue(receivedGenericMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedGenericMsg.msgKey().serviceId());
            assertTrue(receivedGenericMsg.msgKey().checkHasName());
            assertTrue(receivedGenericMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedGenericMsg.domainType());
            
            	
         // Provider 2 gets nothing
            providerReactor2.dispatch(0);
            
            // Send 2nd generic message from consumer
            genericMsg.clear();
            genericMsg.msgClass(MsgClasses.GENERIC);
            genericMsg.streamId(6);
            genericMsg.domainType(DomainTypes.MARKET_PRICE);
            genericMsg.applyHasMsgKey();
            genericMsg.msgKey().applyHasName();
            genericMsg.msgKey().name().data("IBM.N");
            submitOptions.clear();
           	submitOptions.serviceName(Provider.defaultService2().info().serviceName().toString());

            assertTrue(consumer.submitAndDispatch(genericMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            // Provider 2 gets generic msg
            providerReactor2.dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
            
            receivedGenericMsg = (GenericMsg)msgEvent.msg();
            assertTrue(receivedGenericMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService2().serviceId(), receivedGenericMsg.msgKey().serviceId());
            assertTrue(receivedGenericMsg.msgKey().checkHasName());
            assertTrue(receivedGenericMsg.msgKey().name().toString().equals("IBM.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedGenericMsg.domainType());
            
            	
         // Provider 1 gets nothing
            providerReactor.dispatch(0);
        
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
	public void WarmStandbyServicePostTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyServicePostTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		DirectoryRefresh receivedDirectoryRefresh;
		DirectoryUpdate receivedDirectoryUpdate;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        PostMsg postMsg = (PostMsg)msg;
        PostMsg receivedPostMsg;
        int providerStreamId11;
        int providerStreamId21;
        int providerStreamId22;


        
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor();
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
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        Service service2 = DirectoryMsgFactory.createService();
	        Provider.defaultService2().copy(service2);
	        
	        directoryRefresh.serviceList().add(service);

	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);

	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
		    receivedDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent.rdmDirectoryMsg();
		    assertEquals(receivedDirectoryRefresh.serviceList().size(), 1);
		    assertEquals(receivedDirectoryRefresh.serviceList().get(0).info().serviceName().toString(), Provider.defaultService().info().serviceName().toString());
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
    		provider.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
    		/* Consumer receives channel-ready. */
    		event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
    		assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    		
    		/* Consumer sends requests. */
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

            	
        	requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(6);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("IBM.N");
            submitOptions.clear();
           	submitOptions.serviceName(Provider.defaultService2().info().serviceName().toString());

        
       	
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
    		
    		
    		/* Provider receives directory status and request */
			provider.testReactor().dispatch(2);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
			
			/* Provider 1 receives request. */
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
            
            
            providerStreamId11 = receivedRequestMsg.streamId();
            
            /* Provider 1 sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId11);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
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
	        
	        consumer.testReactor().dispatch(-1);
			
			/* Provider receives consumer connection status and directory request. */
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
	        
	        directoryRefresh.serviceList().add(service);
	        directoryRefresh.serviceList().add(service2);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(2);
	        
	        /* Consumer gets a directory update. */
	        event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
		    receivedDirectoryUpdate = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
		    assertEquals(receivedDirectoryUpdate.serviceList().size(), 1);
		    assertEquals(receivedDirectoryUpdate.serviceList().get(0).serviceId(),  Provider.defaultService2().serviceId());
    		
	        
			event = consumer.testReactor().pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			
			
    			    		
    		/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
    		provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
    		
           	        
	        /* Provider 2 receives directory status and request */
			provider2.testReactor().dispatch(4);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.STANDBY);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service2.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);


			 /* Provider 2 receives request 1. */
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
            
            providerStreamId21 = receivedRequestMsg.streamId();

			
            /* Provider 2 receives request 2. */
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
            
            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService2().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().checkHasName());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
            
            providerStreamId22 = receivedRequestMsg.streamId();
            
            /* Provider 1 does not get anything */
            provider.testReactor().dispatch(0);
            
            /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId21);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Provider 2 sends refresh.*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId22);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService2().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("IBM.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            /* Consumer receives one refresh. */
            consumerReactor.dispatch(1);
            
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService2().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("IBM.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertTrue(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
	        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService2().info().serviceName().toString()));
	        
    		/* Consumer sends post messages. */
	        postMsg.clear();
	        postMsg.msgClass(MsgClasses.POST);
	        postMsg.streamId(5);
	        postMsg.domainType(DomainTypes.MARKET_PRICE);
	        postMsg.applyHasMsgKey();
	        postMsg.msgKey().applyHasName();
	        postMsg.msgKey().name().data("TRI.N");
	        postMsg.msgKey().applyHasServiceId();
	        postMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            submitOptions.clear();

            assertTrue(consumer.submitAndDispatch(postMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            // Provider 1 gets post msg
            providerReactor.dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.POST, msgEvent.msg().msgClass());
            
            receivedPostMsg = (PostMsg)msgEvent.msg();
            assertTrue(receivedPostMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedPostMsg.msgKey().serviceId());
            assertTrue(receivedPostMsg.msgKey().checkHasName());
            assertTrue(receivedPostMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedPostMsg.domainType());
            
            	
         // Provider 2 gets post message
            providerReactor2.dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.POST, msgEvent.msg().msgClass());
            
            receivedPostMsg = (PostMsg)msgEvent.msg();
            assertTrue(receivedPostMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedPostMsg.msgKey().serviceId());
            assertTrue(receivedPostMsg.msgKey().checkHasName());
            assertTrue(receivedPostMsg.msgKey().name().toString().equals("TRI.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedPostMsg.domainType());
            
            // Send 2nd post message from consumer
            postMsg.clear();
            postMsg.msgClass(MsgClasses.POST);
            postMsg.streamId(6);
            postMsg.domainType(DomainTypes.MARKET_PRICE);
            postMsg.applyHasMsgKey();
            postMsg.msgKey().applyHasName();
            postMsg.msgKey().name().data("IBM.N");
            submitOptions.clear();
           	submitOptions.serviceName(Provider.defaultService2().info().serviceName().toString());

            assertTrue(consumer.submitAndDispatch(postMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
            // Provider 2 gets post msg
            providerReactor2.dispatch(1);
            event = provider2.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.POST, msgEvent.msg().msgClass());
            
            receivedPostMsg = (PostMsg)msgEvent.msg();
            assertTrue(receivedPostMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService2().serviceId(), receivedPostMsg.msgKey().serviceId());
            assertTrue(receivedPostMsg.msgKey().checkHasName());
            assertTrue(receivedPostMsg.msgKey().name().toString().equals("IBM.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedPostMsg.domainType());
            
            	
         // Provider 1 gets nothing
            providerReactor.dispatch(0);
        
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
	public void WarmStandbyServiceDisconnectTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyServiceDisconnectTest <<<<<<<<<<\n");

		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
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
			consumerReactor = new TestReactor();
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
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        Service service2 = DirectoryMsgFactory.createService();
	        Provider.defaultService2().copy(service2);
	        
	        directoryRefresh.serviceList().add(service);
	        directoryRefresh.serviceList().add(service2);

	        
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
    		
    		
    		/* Provider receives directory status. */
			provider.testReactor().dispatch(2);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service2.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
    		
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
	        
	        consumer.testReactor().dispatch(-1);
			
			/* Provider receives consumer connection status and directory request. */
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
	        
	        directoryRefresh.serviceList().add(service);
	        directoryRefresh.serviceList().add(service2);
	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

	        consumer.testReactor().dispatch(1);
	        
	        /* Consumer does not receive a directory refresh. */
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
	        
	        /* Provider 2 receives directory status and request */
			provider2.testReactor().dispatch(3);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.STANDBY);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service2.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.STANDBY);


			 /* Provider 2 receives request. */
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
            refreshMsg.applyRefreshComplete();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI.N");
            refreshMsg.msgKey().applyHasIdentifier();
            refreshMsg.msgKey().identifier(1);
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            
            assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            
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
            refreshMsg.applyRefreshComplete();
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
            
            provider.close();
            
            /* Consumer receives FD_CHANGE event and then an open suspect status*/
            consumerReactor.dispatch(2);
            
            event = consumerReactor.pollEvent();
    		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
    		channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.FD_CHANGE, channelEvent.eventType());
            
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
            
            /* Provider 2 receives directory status and request */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);

			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service2.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
			
			/* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyHasMsgKey();
            refreshMsg.applyRefreshComplete();
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
            assertTrue(receivedRefreshMsg.msgKey().checkHasIdentifier());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
		}
		finally
		{
	       consumer.close();
	       provider2.close();
	       
	       consumerReactor.close();
	       providerReactor.close();
	       providerReactor2.close();
		}

	}	
	
	@Test
	public void WarmStandbyServiceCloseDeleteServiceTest()
	{
		System.out.println("\n>>>>>>>>> Running WarmStandbyServiceCloseDeleteServiceTest <<<<<<<<<<\n");
	
		TestReactor consumerReactor = null;
		TestReactor providerReactor = null;
		TestReactor providerReactor2 = null;
		Consumer consumer = null;
		Provider provider = null;
		Provider provider2 = null;
		
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		DirectoryRefresh receivedDirectoryRefresh;
		DirectoryMsg directoryMsg = DirectoryMsgFactory.createMsg(); 
		DirectoryUpdate directoryUpdate = (DirectoryUpdate)directoryMsg;
		ReactorMsgEvent msgEvent;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
		Msg msg = CodecFactory.createMsg();
	    RequestMsg requestMsg = (RequestMsg)msg;
	    RequestMsg receivedRequestMsg;
	    RefreshMsg refreshMsg = (RefreshMsg)msg;
	    RefreshMsg receivedRefreshMsg;
	    int providerStreamId11;
	    int providerStreamId12;
	    int providerStreamId21;
	    int providerStreamId22;
	    int directoryStreamId1;
	    int directoryStreamId2;
		
		List<Provider> wsbGroup1 = new ArrayList<Provider>();
		 
		try {		
			/* Create consumer reactor. */
			consumerReactor = new TestReactor();
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
			opts.reconnectAttemptLimit(-1);
			
			/* Create provider reactor. */
			providerReactor = new TestReactor();
			
			/* Create provider. */
			provider = new Provider(providerReactor);
			ProviderRole providerRole = (ProviderRole)provider.reactorRole();
			providerRole.channelEventCallback(provider);
			providerRole.loginMsgCallback(provider);
			providerRole.directoryMsgCallback(provider);
			providerRole.dictionaryMsgCallback(provider);
			providerRole.defaultMsgCallback(provider);
			
			provider.bind(opts);
			
			wsbGroup1.add(provider);
			
			/* Create provider reactor 2. */
			providerReactor2 = new TestReactor();
			
			/* Create provider 2. */
			provider2 = new Provider(providerReactor2);
			ProviderRole providerRole2 = (ProviderRole)provider2.reactorRole();
			providerRole2.channelEventCallback(provider2);
			providerRole2.loginMsgCallback(provider2);
			providerRole2.directoryMsgCallback(provider2);
			providerRole2.dictionaryMsgCallback(provider2);
			providerRole2.defaultMsgCallback(provider2);
			
			provider2.bind(opts);
			    
			wsbGroup1.add(provider2);
			    
			consumerReactor.connectWsb(opts, consumer, wsbGroup1, null, null);
			
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
	        Service service2 = DirectoryMsgFactory.createService();
	        Provider.defaultService2().copy(service2);
	        
	        directoryRefresh.serviceList().add(service);
	        directoryRefresh.serviceList().add(service2);
	        
	        directoryStreamId1 = directoryRequest.streamId();
	
	        
	        submitOptions.clear();
	        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	
	        consumer.testReactor().dispatch(2);
	
	        /* Consumer receives directory refresh. */
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
		    receivedDirectoryRefresh = (DirectoryRefresh)directoryMsgEvent.rdmDirectoryMsg();
		    assertEquals(receivedDirectoryRefresh.serviceList().size(), 2);
		    assertEquals(receivedDirectoryRefresh.serviceList().get(0).info().serviceName().toString(), Provider.defaultService().info().serviceName().toString());
		    assertEquals(receivedDirectoryRefresh.serviceList().get(1).info().serviceName().toString(), Provider.defaultService2().info().serviceName().toString());
				    		
			/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
			consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
			provider.defaultSessionDirectoryStreamId(directoryRequest.streamId());
			
			/* Consumer receives channel-ready. */
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			/* Consumer sends requests. */
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
	
	        	
	    	requestMsg.clear();
	        requestMsg.msgClass(MsgClasses.REQUEST);
	        requestMsg.streamId(6);
	        requestMsg.domainType(DomainTypes.MARKET_PRICE);
	        requestMsg.applyStreaming();
	        requestMsg.msgKey().applyHasName();
	        requestMsg.msgKey().name().data("IBM.N");
	        submitOptions.clear();
	       	submitOptions.serviceName(Provider.defaultService2().info().serviceName().toString());
	
	    
	   	
	        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
			
			
			/* Provider receives directory status and request */
			provider.testReactor().dispatch(4);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
			
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service2.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
			
			/* Provider 1 receives request 1. */
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
	        
	        providerStreamId11 = receivedRequestMsg.streamId();

	        
	        /* Provider 1 receives request 2. */
	        event = provider.testReactor().pollEvent();
	        assertEquals(TestReactorEventTypes.MSG, event.type());
	        msgEvent = (ReactorMsgEvent)event.reactorEvent();
	        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
	        
	        receivedRequestMsg = (RequestMsg)msgEvent.msg();
	        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
	        assertTrue(receivedRequestMsg.checkStreaming());
	        assertFalse(receivedRequestMsg.checkNoRefresh());
	        assertEquals(Provider.defaultService2().serviceId(), receivedRequestMsg.msgKey().serviceId());
	        assertTrue(receivedRequestMsg.msgKey().checkHasName());
	        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
	        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
	        
	        
	        providerStreamId12 = receivedRequestMsg.streamId();
	        
	        /* Provider 1 sends refreshes .*/
	        refreshMsg.clear();
	        refreshMsg.msgClass(MsgClasses.REFRESH);
	        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
	        refreshMsg.streamId(providerStreamId11);
	        refreshMsg.containerType(DataTypes.NO_DATA);
	        refreshMsg.applyHasMsgKey();
	        refreshMsg.applyRefreshComplete();
	        refreshMsg.msgKey().applyHasServiceId();
	        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
	        refreshMsg.msgKey().applyHasName();
	        refreshMsg.msgKey().name().data("TRI.N");
	        refreshMsg.state().streamState(StreamStates.OPEN);
	        refreshMsg.state().dataState(DataStates.OK);
	        
	        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        refreshMsg.clear();
	        refreshMsg.msgClass(MsgClasses.REFRESH);
	        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
	        refreshMsg.streamId(providerStreamId12);
	        refreshMsg.containerType(DataTypes.NO_DATA);
	        refreshMsg.applyHasMsgKey();
	        refreshMsg.applyRefreshComplete();
	        refreshMsg.msgKey().applyHasServiceId();
	        refreshMsg.msgKey().serviceId(Provider.defaultService2().serviceId());
	        refreshMsg.msgKey().applyHasName();
	        refreshMsg.msgKey().name().data("IBM.N");
	        refreshMsg.state().streamState(StreamStates.OPEN);
	        refreshMsg.state().dataState(DataStates.OK);
	        
	        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        /* Consumer receives refreshes. */
	        consumerReactor.dispatch(2);
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
	        
	        event = consumerReactor.pollEvent();
	        assertEquals(TestReactorEventTypes.MSG, event.type());
	        msgEvent = (ReactorMsgEvent)event.reactorEvent();
	        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
	        
	        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
	        assertTrue(receivedRefreshMsg.checkHasMsgKey());
	        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
	        assertEquals(Provider.defaultService2().serviceId(), receivedRefreshMsg.msgKey().serviceId());
	        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
	        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("IBM.N"));
	        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
	        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
	        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
	        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
	        assertFalse(receivedRefreshMsg.msgKey().checkHasIdentifier());
	        assertNotNull(msgEvent.streamInfo());
			
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
	        
	        consumer.testReactor().dispatch(-1);
			
			/* Provider receives consumer connection status and directory request. */
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
	        
	        directoryRefresh.serviceList().add(service);
	        directoryRefresh.serviceList().add(service2);
	        
	        directoryStreamId2 = directoryRequest.streamId();

	        
	        submitOptions.clear();
	        assertTrue(provider2.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
	
	        consumer.testReactor().dispatch(1);
	        
	        /* Consumer does not get a directory update. */
	        event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
				    		
			/* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
			provider2.defaultSessionDirectoryStreamId(directoryRequest.streamId());
			
	       	        
	        /* Provider 2 receives directory status and request */
			provider2.testReactor().dispatch(4);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.STANDBY);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service2.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.STANDBY);
	
	
			 /* Provider 2 receives request 1. */
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
	        
	        providerStreamId21 = receivedRequestMsg.streamId();
	
			
	        /* Provider 2 receives request 2. */
	        event = provider2.testReactor().pollEvent();
	        assertEquals(TestReactorEventTypes.MSG, event.type());
	        msgEvent = (ReactorMsgEvent)event.reactorEvent();
	        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
	        
	        receivedRequestMsg = (RequestMsg)msgEvent.msg();
	        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
	        assertTrue(receivedRequestMsg.checkStreaming());
	        assertFalse(receivedRequestMsg.checkNoRefresh());
	        assertEquals(Provider.defaultService2().serviceId(), receivedRequestMsg.msgKey().serviceId());
	        assertTrue(receivedRequestMsg.msgKey().checkHasName());
	        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
	        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
	        
	        providerStreamId22 = receivedRequestMsg.streamId();
	        
	        /* Provider 1 does not get anything */
	        provider.testReactor().dispatch(0);
	        
	        /* Provider 2 sends refresh... this is to make sure that the consumer gets the correct refresh .*/
	        refreshMsg.clear();
	        refreshMsg.msgClass(MsgClasses.REFRESH);
	        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
	        refreshMsg.streamId(providerStreamId21);
	        refreshMsg.containerType(DataTypes.NO_DATA);
	        refreshMsg.applyHasMsgKey();
	        refreshMsg.applyRefreshComplete();
	        refreshMsg.msgKey().applyHasServiceId();
	        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
	        refreshMsg.msgKey().applyHasName();
	        refreshMsg.msgKey().name().data("TRI.N");
	        refreshMsg.msgKey().applyHasIdentifier();
	        refreshMsg.msgKey().identifier(1);
	        refreshMsg.state().streamState(StreamStates.OPEN);
	        refreshMsg.state().dataState(DataStates.OK);
	        
	        assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        /* Provider 2 sends refresh.*/
	        refreshMsg.clear();
	        refreshMsg.msgClass(MsgClasses.REFRESH);
	        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
	        refreshMsg.streamId(providerStreamId22);
	        refreshMsg.containerType(DataTypes.NO_DATA);
	        refreshMsg.applyHasMsgKey();
	        refreshMsg.applyRefreshComplete();
	        refreshMsg.msgKey().applyHasServiceId();
	        refreshMsg.msgKey().serviceId(Provider.defaultService2().serviceId());
	        refreshMsg.msgKey().applyHasName();
	        refreshMsg.msgKey().name().data("IBM.N");
	        refreshMsg.msgKey().applyHasIdentifier();
	        refreshMsg.msgKey().identifier(1);
	        refreshMsg.state().streamState(StreamStates.OPEN);
	        refreshMsg.state().dataState(DataStates.OK);
	        
	        assertTrue(provider2.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        /* Consumer receives nothing. */
	        consumerReactor.dispatch(0);
	        
	       
	        /* Submit a directory update with one service down and one service deleted */
	        directoryUpdate.clear();
	        directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
	        directoryUpdate.streamId(directoryRequest.streamId());
	        directoryUpdate.applyHasFilter();
	        directoryUpdate.filter(directoryRequest.filter());
	        
	        service.action(MapEntryActions.DELETE);
	        service2.action(MapEntryActions.UPDATE);
	        service2.applyHasState();
	        service2.state().serviceState(0);
	        
	        directoryUpdate.serviceList().add(service);
	        directoryUpdate.serviceList().add(service2);
	        
	        assertTrue(provider.submitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCodes.SUCCESS);
	        
	        /* Consumer receives a open suspect. */
	        consumerReactor.dispatch(1);
	        event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            assertEquals(((StatusMsg)msgEvent.msg()).state().dataState(), DataStates.SUSPECT);
            assertEquals(((StatusMsg)msgEvent.msg()).state().streamState(), StreamStates.OPEN);
            System.out.println(((StatusMsg)msgEvent.msg()).state().text().toString());
	        
	        /* Provider receives directory status and request */
			provider.testReactor().dispatch(1);
			event = provider.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service2.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.STANDBY);

	        
	        /* Provider 2 receives directory status and request */
			provider2.testReactor().dispatch(2);
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
			
			event = provider2.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
			directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
			assertEquals(DirectoryMsgType.CONSUMER_STATUS, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
			assertTrue(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).checkHasWarmStandbyMode());
			/* 1st connection, so this should set to 0 */
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).serviceId(), service2.serviceId());
			assertEquals(((DirectoryConsumerStatus)directoryMsgEvent.rdmDirectoryMsg()).consumerServiceStatusList().get(0).warmStandbyMode(), WarmStandbyDirectoryServiceTypes.ACTIVE);
	
	        
	    
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
    
}