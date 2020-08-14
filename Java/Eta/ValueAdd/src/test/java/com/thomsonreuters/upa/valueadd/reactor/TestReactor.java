///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.reactor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Queue;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
/*
 * This class represents a single Reactor.
 * It providers simple ways to connect components (such as a consumer and provider)
 * and dispatch for events. The dispatched events are copied (including any underlying data such as 
 * messages) and stored into an event queue. You can then retrieve those events and test that they are 
 * correct.
 */
public class TestReactor {
	
    /** The associated reactor. */
	Reactor _reactor;
	
	/** Queue of events received from calling dispatch. */
	Queue<TestReactorEvent> _eventQueue;
	
	/** List of components associated with this reactor. */
	LinkedList<TestReactorComponent> _componentList;
	
	/** Reusable ReactorErrorInfo. */
	ReactorErrorInfo _errorInfo;
	
	/** Selector used when dispatching. */
	Selector _selector;
	
	public int _countAuthTokenEventCallbackCalls = 0;

    /** Controls whether reactor does XML tracing. */
    private static boolean _enableReactorXmlTracing = false;

	/** Creates a TestReactor. */
	public TestReactor()
	{

		ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
		
		_eventQueue = new LinkedList<TestReactorEvent>();
		_componentList = new LinkedList<TestReactorComponent>();
		_errorInfo = ReactorFactory.createReactorErrorInfo();
        if (_enableReactorXmlTracing)
            reactorOptions.enableXmlTracing();
		_reactor = ReactorFactory.createReactor(reactorOptions, _errorInfo);
		
		try {
			_selector = Selector.open();
		} catch (IOException e) {
			e.printStackTrace();
			fail("Caught I/O exception");
		}
	}

	/** Creates a TestReactor. */
	public TestReactor(ReactorOptions reactorOptions)
	{
		
		_eventQueue = new LinkedList<TestReactorEvent>();
		_componentList = new LinkedList<TestReactorComponent>();
		_errorInfo = ReactorFactory.createReactorErrorInfo();
        if (_enableReactorXmlTracing)
            reactorOptions.enableXmlTracing();
		_reactor = ReactorFactory.createReactor(reactorOptions, _errorInfo);
		
		try {
			_selector = Selector.open();
		} catch (IOException e) {
			e.printStackTrace();
			fail("Caught I/O exception");
		}
	}	
	
    /** Enables XML tracing on created reactors (convenience function for test debugging). */
    public static void enableReactorXmlTracing()
    {
        _enableReactorXmlTracing = true;
    }

	/** Calls dispatch on the component's Reactor, and will store received events for later review.
	 * The test will verify that the exact number of events is received, and will fail if fewer
	 * or more are received.
	 * @param expectedEventCount The exact number of events that should be received.
	 */
	public void dispatch(int expectedEventCount){
		dispatch(expectedEventCount, false);
	}

	/** Calls dispatch on the component's Reactor, and will store received events for later review.
	 * The test will verify that the exact number of events is received, and will fail if fewer
	 * or more are received.
	 * Used for forced close connection from client, use {@link #dispatch(int)} for ordinary scenarios
	 * @param expectedEventCount The exact number of events that should be received.
	 * @param channelWasClosed true if client forcible have closed channel
	 */
	public void dispatch(int expectedEventCount, boolean channelWasClosed)
	{
		long timeoutMsec;
		
        if (expectedEventCount > 0)
        	timeoutMsec = 1000;
        else
        	timeoutMsec = 200;
        dispatch(expectedEventCount, timeoutMsec, channelWasClosed);
	}
	
	
	/** Waits for notification on the component's Reactor, and calls dispatch when triggered. It will 
	 * store any received events for later review.
	 * Waiting for notification stops once the expected number of events is received (unless that number is zero, in which case it waits for the full specified time). 
	 * @param expectedEventCount The exact number of events that should be received.
	 * @param timeoutMsec The maximum time to wait for all events.
	 */
	public void dispatch(int expectedEventCount, long timeoutMsec){
		dispatch(expectedEventCount, timeoutMsec, false);
	}

	/** Waits for notification on the component's Reactor, and calls dispatch when triggered. It will
	 * store any received events for later review.
	 * Waiting for notification stops once the expected number of events is received (unless that number is zero, in which case it waits for the full specified time).
	 * Used for forced close connection from client, use {@link #dispatch(int)} for ordinary scenarios
	 * @param expectedEventCount The exact number of events that should be received.
	 * @param timeoutMsec The maximum time to wait for all events.
	 * @param channelWasClosed true if client forcible have closed channel
	 */
	public void dispatch(int expectedEventCount, long timeoutMsec, boolean channelWasClosed)
	{
        int selectRet = 0;
        long currentTimeUsec, stopTimeUsec;
        int lastDispatchRet = 0;

        /* Ensure no events were missed from previous calls to dispatch.
           But don't check event queue size when expectedEventCount is set to -1 */
        if(expectedEventCount != -1 )
        	assertEquals(0, _eventQueue.size());

        currentTimeUsec = System.nanoTime()/1000;
        
        stopTimeUsec =  (timeoutMsec);
        stopTimeUsec *= 1000;
        stopTimeUsec += currentTimeUsec;

        do
        {
            if (lastDispatchRet == 0)
            {
                try
				{
                    long selectTime;

					//check if channel still exists and opened
					if(_reactor.reactorChannel()==null || _reactor.reactorChannel().selectableChannel()==null || !_reactor.reactorChannel().selectableChannel().isOpen()) {
						if (channelWasClosed) {
							return;
						} else {
							fail("No selectable channel exists");
						}
					}

					_reactor.reactorChannel().selectableChannel().register(_selector, SelectionKey.OP_READ, _reactor.reactorChannel());
				   
					for (TestReactorComponent component : _componentList)
						if (component.reactorChannel() != null && component.reactorChannelIsUp() &&
							component.reactorChannel().selectableChannel() != null )
							component.reactorChannel().selectableChannel().register(_selector, SelectionKey.OP_READ, component.reactorChannel());
	
					selectTime = (stopTimeUsec - currentTimeUsec)/1000;
					
					if (selectTime > 0)
					{
						selectRet = _selector.select(selectTime);
					}
					else
					{
						selectRet = _selector.selectNow();
					}
				}
				catch (ClosedChannelException e)
				{
					e.printStackTrace();
					fail("Caught ClosedChannelException.");
				}
				catch (IOException e)
				{
					e.printStackTrace();
					fail("Caught IOException.");
				}
            }
            else
                selectRet = 1;

            if (selectRet > 0)
            {
                ReactorDispatchOptions dispatchOpts = ReactorFactory.createReactorDispatchOptions();

                do
                {
	                dispatchOpts.clear();
	                lastDispatchRet = _reactor.dispatchAll(_selector.selectedKeys(), dispatchOpts, _errorInfo);
	                assertTrue("Dispatch failed: " + lastDispatchRet + "(" + _errorInfo.location() + " -- "+ _errorInfo.error().text() + ")", 
	                		lastDispatchRet >= 0);
                } while (lastDispatchRet > 0);
            }

            currentTimeUsec = System.nanoTime()/1000;
            
            /* If we've hit our expected number of events, drop the stopping time to at most 100ms from now.  
             * Keep dispatching a short time to ensure no unexpected events are received, and that any internal flush events are processed. */
            if (expectedEventCount > 0 && _eventQueue.size() == expectedEventCount)
            {
                long stopTimeUsecNew = currentTimeUsec + 100000;
                if ((stopTimeUsec - stopTimeUsecNew) > 0)
                {
                    stopTimeUsec = stopTimeUsecNew;
                }
            }

        } while(currentTimeUsec < stopTimeUsec);
        
        /* Don't check event queue size when expectedEventCount is set to -1 */
        if(expectedEventCount != -1 )
        	assertEquals(expectedEventCount, _eventQueue.size());
    }
	
	public int handleServiceEndpointEvent(ReactorServiceEndpointEvent event)
	{
		_eventQueue.add(new TestReactorEvent(TestReactorEventTypes.SERVICE_DISC_ENDPOINT, event));
		return ReactorReturnCodes.SUCCESS;
	}
	
    /** Stores the received channel event, and updates the relevant component's channel information. */ 
	public int handleChannelEvent(ReactorChannelEvent event)
	{
		TestReactorComponent component = (TestReactorComponent)event.reactorChannel().userSpecObj(); 
				
		switch(event.eventType())
		{
		case ReactorChannelEventTypes.CHANNEL_OPENED:
			assertEquals(false, component.reactorChannelIsUp());
			break;
		case ReactorChannelEventTypes.CHANNEL_UP:
			component.reactorChannelIsUp(true);
			break;
		case ReactorChannelEventTypes.CHANNEL_DOWN:
		case ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING:
            if (component.reactorChannelIsUp())
            {
                /* If channel was up, the selectableChannel should be present. */
                assertNotNull(component.reactorChannel().selectableChannel());
                assertNotNull(component.reactorChannel().selectableChannel().keyFor(_selector));

                /* Cancel key. */
                component.reactorChannel().selectableChannel().keyFor(_selector).cancel();
            }

			component.reactorChannelIsUp(false);
			break;
		case ReactorChannelEventTypes.CHANNEL_READY:
		case ReactorChannelEventTypes.FD_CHANGE:
		case ReactorChannelEventTypes.WARNING:
			assertNotNull(component.reactorChannel());
			break;
		default:
			fail("Unhandled ReactorChannelEventType.");
				
		}
		
		if (component.reactorChannel() != null)
            assertEquals(component.reactorChannel(), event.reactorChannel());
        else
            component.reactorChannel(event.reactorChannel());
		
		_eventQueue.add(new TestReactorEvent(TestReactorEventTypes.CHANNEL_EVENT, event));
		return ReactorReturnCodes.SUCCESS;
	}
	
	/** Stores a login message event. */
	public int handleLoginMsgEvent(RDMLoginMsgEvent event)
	{
		_eventQueue.add(new TestReactorEvent(TestReactorEventTypes.LOGIN_MSG, event));
		return ReactorReturnCodes.SUCCESS;
	}
	
	/** Stores a auth token event. */
	public int handleAuthTokenEvent(ReactorAuthTokenEvent event)
	{
		_eventQueue.add(new TestReactorEvent(TestReactorEventTypes.AUTH_TOKEN_EVENT, event));
		_countAuthTokenEventCallbackCalls++;
		return ReactorReturnCodes.SUCCESS;
	}	
	
	/** Stores a directory message event. */
	public int handleDirectoryMsgEvent(RDMDirectoryMsgEvent event)
	{
		_eventQueue.add(new TestReactorEvent(TestReactorEventTypes.DIRECTORY_MSG, event));
		return ReactorReturnCodes.SUCCESS;
	}

	/** Stores a dictionary message event. */
	public int handleDictionaryMsgEvent(RDMDictionaryMsgEvent event)
	{
		_eventQueue.add(new TestReactorEvent(TestReactorEventTypes.DICTIONARY_MSG, event));	
		return ReactorReturnCodes.SUCCESS;
	}
	
	/** Stores a message event. */
	public int handleDefaultMsgEvent(ReactorMsgEvent event)
	{
		_eventQueue.add(new TestReactorEvent(TestReactorEventTypes.MSG, event));
		return ReactorReturnCodes.SUCCESS;
	}
    
    /** Stores a tunnel stream message event. */
    public int handleTunnelStreamMsgEvent(TunnelStreamMsgEvent event)
    {
        _eventQueue.add(new TestReactorEvent(TestReactorEventTypes.TUNNEL_STREAM_MSG, event));
        return ReactorReturnCodes.SUCCESS;
    }
    
    /** Stores a tunnel stream queue message event. */
    public int handleQueueMsgEvent(TunnelStreamQueueMsgEvent event)
    {
        _eventQueue.add(new TestReactorEvent(TestReactorEventTypes.TUNNEL_STREAM_QUEUE_MSG, event));
        return ReactorReturnCodes.SUCCESS;
    }
    
    /** Stores a tunnel stream status event. */
    public int handleTunnelStreamStatusEvent(TunnelStreamStatusEvent event)
    {
        _eventQueue.add(new TestReactorEvent(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event));
        return ReactorReturnCodes.SUCCESS;
    }
 
    /** Stores a tunnel stream request event. */
    public int handleTunnelStreamRequestEvent(TunnelStreamRequestEvent event)
    {
        _eventQueue.add(new TestReactorEvent(event));
        return ReactorReturnCodes.SUCCESS;
    }
	
	/** Retrieves an event from the list of events received from a dispatch call. */
	TestReactorEvent pollEvent()
	{
		return _eventQueue.poll();
	}
	
	/** Adds a component to the Reactor. */
	void addComponent(TestReactorComponent component)
	{
	    assertFalse(_componentList.contains(component));
	    _componentList.add(component);
	}
	
    /** Adds a component's server to the selector. */
    void registerComponentServer(TestReactorComponent component)
    {
        assertNotNull(component.server());
        try
        {
            component.server().selectableChannel().register(_selector, SelectionKey.OP_ACCEPT);
        }
        catch (ClosedChannelException e)
        {
            e.printStackTrace();
            fail("Caught ClosedChannelException");
        }
        
    }

	/** Removes a component from the Reactor. */
	void removeComponent(TestReactorComponent component)
	{
	    assertTrue(_componentList.contains(component));
		_componentList.remove(component);
	}
	
	/** Associates a component with this reactor and opens a connection. */
	void connect(ConsumerProviderSessionOptions opts, TestReactorComponent component, int port)
	{
        ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
        ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
        int ret;

        connectOpts.clear();
        connectOpts.connectionList().add(connectInfo);
        connectOpts.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
        connectOpts.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());
        connectOpts.connectionList().get(0).connectOptions().connectionType(opts.connectionType());
        connectOpts.connectionList().get(0).connectOptions().userSpecObject(component);
        connectOpts.reconnectAttemptLimit(opts.reconnectAttemptLimit());
        connectOpts.reconnectMinDelay(opts.reconnectMinDelay());
        connectOpts.reconnectMaxDelay(opts.reconnectMaxDelay());
        connectOpts.connectionList().get(0).connectOptions().pingTimeout(opts.pingTimeout());
        connectOpts.connectionList().get(0).initTimeout(opts.consumerChannelInitTimeout());
        
        
        if (opts.connectionType() != ConnectionTypes.RELIABLE_MCAST)
        {
            connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address("localhost");
            connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(port));
        }
        else
        {
        	
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().recvAddress("235.1.1.1");
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().recvServiceName("15011");
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().sendAddress("235.1.1.1");
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().sendServiceName("15011");
            
            // NOTE This used to increment. If problems come up with connecting via multicast, should look into why.
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().unicastServiceName("12345");
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().interfaceName("localhost");
            connectOpts.connectionList().get(0).connectOptions().sysSendBufSize(64);
            connectOpts.connectionList().get(0).connectOptions().sysRecvBufSize(64);
        }
        
        ret = _reactor.connect(connectOpts, component.reactorRole(), _errorInfo);
        assertEquals("Connect failed: " + ret + "(" + _errorInfo.location() + " -- "+ _errorInfo.error().text() + ")",
        		ReactorReturnCodes.SUCCESS, ret);

        /* Clear ReactorConnectOptions after connecting -- this tests whether the Reactor is properly saving the options. */
        connectOpts.clear();

    }
	
    /** Associates a component with this reactor and accepts a connection. */
    void accept(ConsumerProviderSessionOptions opts, TestReactorComponent component)
	{
        accept(opts, component, 5000);
	}
	
	/** Associates a component with this reactor and accepts a connection. */
	void accept(ConsumerProviderSessionOptions opts, TestReactorComponent component, long timeoutMsec)
	{
        ReactorAcceptOptions    acceptOpts = ReactorFactory.createReactorAcceptOptions();
        ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();

        if (opts.connectionType() != ConnectionTypes.RELIABLE_MCAST)
        {
            assertNotNull(component.server());
            assertNull(component.reactorChannel());
            
            /* Wait for server channel to trigger. */
            long stopTimeMsec = System.currentTimeMillis() + timeoutMsec;
                    
            do
            {
                try
                {
                    assertTrue(_selector.select(stopTimeMsec - System.currentTimeMillis()) >= 0);
                   
                    Iterator<SelectionKey> iter = _selector.selectedKeys().iterator();
                    
                    while(iter.hasNext())
                    {
                        SelectionKey key = iter.next();
                        
                        if (key.channel() == _reactor.reactorChannel().selectableChannel())
                        {
                            /* Reactor's channel triggered. Should get no events. */
                            dispatch(0, 100);
                        }
                        else if (key.channel() == component.server().selectableChannel())
                        {
                            _selector.selectedKeys().clear();
                            
                            /* Accept connection. */
                            acceptOpts.clear();
                            acceptOpts.acceptOptions().userSpecObject(component);
                            assertEquals(ReactorReturnCodes.SUCCESS, _reactor.accept(component.server(), acceptOpts, component.reactorRole(), _errorInfo));
                            return;
                        }
                            
                    }
                }
                catch (IOException e)
                {
                    e.printStackTrace();
                    fail("Caught I/O exception");
                }
            } while (System.currentTimeMillis() < stopTimeMsec);
            
            fail("Server did not receive accept notification.");
        }
        else
        {
            connectOpts.clear();
            connectOpts.connectionList().get(0).connectOptions().userSpecObject(component);
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().recvAddress("235.1.1.1");
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().recvServiceName("15011");
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().sendAddress("235.1.1.1");
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().sendServiceName("15011");

            //NOTE This used to increment. If problems come up with connecting via multicast, should look into why.
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().unicastServiceName("12346");
            connectOpts.connectionList().get(0).connectOptions().segmentedNetworkInfo().interfaceName("localhost");
            connectOpts.connectionList().get(0).connectOptions().connectionType(ConnectionTypes.RELIABLE_MCAST);
            
            
            connectOpts.connectionList().get(0).connectOptions().sysSendBufSize(64);
            connectOpts.connectionList().get(0).connectOptions().sysRecvBufSize(64);
            assertEquals(ReactorReturnCodes.SUCCESS, _reactor.connect(connectOpts, component.reactorRole(), _errorInfo));

            /* Clear ReactorConnectOptions after connecting -- this tests whether the Reactor is properly saving the options. */
            connectOpts.clear();
        }
	}
	
	/** Connects a Consumer and Provider component to each other. */
	public static void openSession(Consumer consumer, Provider provider , ConsumerProviderSessionOptions opts)
    {
        openSession(consumer, provider, opts, false);
    }

	/** Connects a Consumer and Provider component to each other. */
	public static void openSession(Consumer consumer, Provider provider , ConsumerProviderSessionOptions opts, boolean recoveringChannel)
	{
		TestReactorEvent event;
		ReactorChannelEvent channelEvent;
		RDMLoginMsgEvent loginMsgEvent;
		RDMDirectoryMsgEvent directoryMsgEvent;
		ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

        if (!recoveringChannel)
            consumer.testReactor().connect(opts, consumer, provider.serverPort());

		/* Preset login message required if automatically setting up login stream. */
		assertTrue(opts.setupDefaultLoginStream() == false || consumerRole.rdmLoginRequest() != null);

		/* Preset directory message required, or watchlist must be enabled, if automatically setting up directory stream. */
		assertTrue(opts.setupDefaultDirectoryStream() == false 
				|| consumerRole.watchlistOptions().enableWatchlist() == true
				|| consumerRole.rdmDirectoryRequest() != null);
		
		/* If watchlist enabled, should get ChannelOpenCallback */
		if (consumerRole.watchlistOptions().enableWatchlist() && consumerRole.watchlistOptions().channelOpenCallback() != null
		        && recoveringChannel == false)
		{
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_OPENED, channelEvent.eventType());
		}
		else
		{
			consumer.testReactor().dispatch(0);
		}
		
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
		
		/* Consumer receives channel-up and any status events due the watchlist items submitted in channel open callback. */
		consumer.testReactor().dispatch(1 + opts.numStatusEvents());
	    for (int i = 0; i < opts.numStatusEvents(); i++)
	    {
	        event = consumer.testReactor().pollEvent();
	        assertEquals(TestReactorEventTypes.MSG, event.type());
	        ReactorMsgEvent msgEvent = (ReactorMsgEvent)event.reactorEvent();
	        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
	    }
		event = consumer.testReactor().pollEvent();
		assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
		channelEvent = (ReactorChannelEvent)event.reactorEvent();
		assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
		
		if (consumerRole.rdmLoginRequest() == null)
		{
			/* Consumer receives channel-ready, then we're done. */
			consumer.testReactor().dispatch(1);
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			provider.testReactor().dispatch(0);

			return;
		}
		
		if (opts.setupDefaultLoginStream() == false)
			return;
		
		/* Provider receives login request. */
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
        loginRefresh.state().streamState(StreamStates.OPEN);
        loginRefresh.state().dataState(DataStates.OK);
        loginRefresh.state().code(StateCodes.NONE);
        loginRefresh.state().text().data("Login OK");
        
        submitOptions.clear();
        assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives login refresh. */		
		if (consumerRole.rdmDirectoryRequest() == null && consumerRole.watchlistOptions().enableWatchlist() == false)
			consumer.testReactor().dispatch(2);
		else
			consumer.testReactor().dispatch(1);
		
		/* Save the stream ID used by each component to open the login stream (may be different if the watchlist is enabled). */
		consumer.defaultSessionLoginStreamId(consumerRole.rdmLoginRequest().streamId());
		provider.defaultSessionLoginStreamId(loginRequest.streamId());
		
		event = consumer.testReactor().pollEvent();
		assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
		loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
		assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
		
		if (consumerRole.rdmDirectoryRequest() == null && consumerRole.watchlistOptions().enableWatchlist() == false)
		{
			/* Consumer receives channel-ready. */
			event = consumer.testReactor().pollEvent();
			assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
			channelEvent = (ReactorChannelEvent)event.reactorEvent();
			assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
			
			provider.testReactor().dispatch(0);

			/* If watchlist is not enabled, no directory exchange occurs. We're done. */
			if (consumerRole.watchlistOptions().enableWatchlist() == false)
				return;
		}
		
		if (opts.setupDefaultDirectoryStream() == false && consumerRole.watchlistOptions().enableWatchlist() == false)
			return;

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
        Service service2 = DirectoryMsgFactory.createService();
        Provider.defaultService().copy(service);
        Provider.defaultService2().copy(service2);
        
        // Apply OpenWindow to the service if one is specified.
        if (opts.openWindow() >= 0)
        {
	        service.applyHasLoad();
	        service.load().applyHasOpenWindow();
	        service.load().openWindow(opts.openWindow());
        }
        
        directoryRefresh.serviceList().add(service);
        if (opts.setupSecondDefaultDirectoryStream())
        {
        	directoryRefresh.serviceList().add(service2);
        }
        submitOptions.clear();
        assertTrue(provider.submitAndDispatch(directoryRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);

        if (opts.setupDefaultDirectoryStream() == true)
            consumer.testReactor().dispatch(2);
        else
            consumer.testReactor().dispatch(1);
        
        /* Consumer receives directory refresh. */
		event = consumer.testReactor().pollEvent();
		if (opts.setupDefaultDirectoryStream() == true)
		{
    		assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
    		directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
    		if (!recoveringChannel || consumerRole.watchlistOptions().enableWatchlist() == false)
    		    assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
    		else
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
		else // only channel event comes in this case
		{
		    /* Consumer receives channel-ready. */
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            channelEvent = (ReactorChannelEvent)event.reactorEvent();
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
        }
	}

    /** Cleans up the TestReactor's resources (e.g. its reactor) */
    public void close()
    {
        if (_reactor != null)
        {
            assertEquals(ReactorReturnCodes.SUCCESS, _reactor.shutdown(_errorInfo));
            _reactor = null;
        }
    }
	
}
