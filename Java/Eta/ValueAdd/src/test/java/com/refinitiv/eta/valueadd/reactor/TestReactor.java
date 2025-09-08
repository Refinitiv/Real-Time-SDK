/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Objects;
import java.util.Queue;

import com.refinitiv.eta.codec.*;

import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.Service;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;
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
    
    private boolean initJsonConverter = false;
    
    /** Controls if we need to overwrite the reactorChannel in the component, switching between WSB and ConnectionList **/
    boolean switchingReactorChannel = false;
    
    /** Control whether or not this is a preferred host test, in which case we need to avoid some asserts that could occur due to timing conditions, but shouldn't reflect an error in the test case. */
    boolean isPreferredHostTest = false;
    
    final String DEFAULT_SERVICE = "DEFAULT_SERVICE";
    
    final String LDP_ENDPOINT_ADDRESS = "us-east-1-aws-1-med.optimized-pricing-api.refinitiv.net";
    final String LDP_ENDPOINT_PORT = "14002";

    final String LDP_ENDPOINT_ADDRESS_WEBSOCKET = "us-east-1-aws-3-lrg.optimized-pricing-api.refinitiv.net";
    final String LDP_ENDPOINT_PORT_WEBSOCKET = "443";
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
	
	/** Creates a TestReactor. */
	public TestReactor(boolean isPreferredHostTest)
	{

		ReactorOptions reactorOptions = ReactorFactory.createReactorOptions();
		_eventQueue = new LinkedList<TestReactorEvent>();
		_componentList = new LinkedList<TestReactorComponent>();
		_errorInfo = ReactorFactory.createReactorErrorInfo();
		this.isPreferredHostTest = isPreferredHostTest;
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
	public TestReactor(ReactorOptions reactorOptions, boolean isPreferredHostTest)
	{
		
		_eventQueue = new LinkedList<TestReactorEvent>();
		_componentList = new LinkedList<TestReactorComponent>();
		_errorInfo = ReactorFactory.createReactorErrorInfo();
		this.isPreferredHostTest = isPreferredHostTest;
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
    
    public void initJsonConverter(DataDictionary dictionary)
    {
    	if(initJsonConverter)
    		return;
    	
    	ReactorErrorInfo errorInfo = new ReactorErrorInfo();
        ReactorJsonConverterOptions options = new ReactorJsonConverterOptions();
        options.dataDictionary(dictionary);
        
        assertEquals(ReactorReturnCodes.SUCCESS,_reactor.initJsonConverter(options, errorInfo));
        
        initJsonConverter = true;
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
					{
						if(component.reactorChannel() != null && component.reactorChannelIsUp() )
						{
							if(component.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
							{
								for(int i = 0; i < component.reactorChannel().warmStandbyChannelInfo().selectableChannelList().size(); i++)
								{
									SelectableChannel tmpSelector = component.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i);
									
									if(tmpSelector != null)
									{
										tmpSelector.register(_selector,  SelectionKey.OP_READ, component.reactorChannel());
									}
										
								}
							}
							else if (component.reactorChannel().selectableChannel() != null )
							{
								component.reactorChannel().selectableChannel().register(_selector, SelectionKey.OP_READ, component.reactorChannel());
							}
						}
					}
	
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
        System.out.println(expectedEventCount + " eventQueue: " + _eventQueue.size());
        if(expectedEventCount != -1 )
        	assertEquals(expectedEventCount, _eventQueue.size());
        
        // Reset switching reactor channel to false now that this dispatch is complete
        switchingReactorChannel = false;
    }
	
	/** Waits for notification on the component's Reactor, and calls dispatch when triggered. It will
	 * store any received events for later review.
	 * Will only dispatch a single event.
	 * Waiting for notification stops once the expected number of events is received (unless that number is zero, in which case it waits for the full specified time).
	 * Used for forced close connection from client, use {@link #dispatch(int)} for ordinary scenarios
	 * @param timeoutMsec The maximum time to wait for all events.
	 * @param channelWasClosed true if client forcible have closed channel
	 */
	public void dispatchSingleEvent(long timeoutMsec, boolean channelWasClosed)
	{
        int selectRet = 0;
        long currentTimeUsec, stopTimeUsec;
        int lastDispatchRet = 0;

        /* Ensure no events were missed from previous calls to dispatch.
           But don't check event queue size when expectedEventCount is set to -1 */
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
					{
						if(component.reactorChannel() != null && component.reactorChannelIsUp() )
						{
							if(component.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
							{
								for(int i = 0; i < component.reactorChannel().warmStandbyChannelInfo().selectableChannelList().size(); i++)
								{
									SelectableChannel tmpSelector = component.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i);
									
									if(tmpSelector != null)
									{
										tmpSelector.register(_selector,  SelectionKey.OP_READ, component.reactorChannel());
									}
										
								}
							}
							else if (component.reactorChannel().selectableChannel() != null )
							{
								component.reactorChannel().selectableChannel().register(_selector, SelectionKey.OP_READ, component.reactorChannel());
							}
						}
					}
	
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
	                dispatchOpts.maxMessages(1);
	                lastDispatchRet = _reactor.dispatchAll(_selector.selectedKeys(), dispatchOpts, _errorInfo);
	                assertTrue("Dispatch failed: " + lastDispatchRet + "(" + _errorInfo.location() + " -- "+ _errorInfo.error().text() + ")", 
	                		lastDispatchRet >= 0);
                } while (lastDispatchRet > 0);
            }

            currentTimeUsec = System.nanoTime()/1000;
            
            /* If we've hit our expected number of events, drop the stopping time to at most 100ms from now.  
             * Keep dispatching a short time to ensure no unexpected events are received, and that any internal flush events are processed. */
            if (_eventQueue.size() == 1)
            {
                long stopTimeUsecNew = currentTimeUsec + 100000;
                if ((stopTimeUsec - stopTimeUsecNew) > 0)
                {
                    stopTimeUsec = stopTimeUsecNew;
                }
            }

        } while(currentTimeUsec < stopTimeUsec);
        
        /* Don't check event queue size when expectedEventCount is set to -1 */
       	assertEquals(1, _eventQueue.size());
    }
	
	
	/** Waits for notification on the component's Reactor, and calls dispatch when triggered. 
	 * This method expects dispatch to fail
	 *  It will store any received events for later review.
	 * Waiting for notification stops once the expected number of events is received (unless that number is zero, in which case it waits for the full specified time).
	 * Used for forced close connection from client, use {@link #dispatch(int)} for ordinary scenarios
	 * @param expectedEventCount The exact number of events that should be received.
	 * @param timeoutMsec The maximum time to wait for all events.
	 * @param channelWasClosed true if client forcible have closed channel
	 */
	public void dispatchFailure(int expectedEventCount, long timeoutMsec, boolean channelWasClosed)
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
					{
						if(component.reactorChannel() != null && component.reactorChannelIsUp() )
						{
							if(component.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
							{
								for(int i = 0; i < component.reactorChannel().warmStandbyChannelInfo().selectableChannelList().size(); i++)
								{
									SelectableChannel tmpSelector = component.reactorChannel().warmStandbyChannelInfo().selectableChannelList().get(i);
									
									if(tmpSelector != null)
									{
										tmpSelector.register(_selector,  SelectionKey.OP_READ, component.reactorChannel());
									}
										
								}
							}
							else if (component.reactorChannel().selectableChannel() != null )
							{
								component.reactorChannel().selectableChannel().register(_selector, SelectionKey.OP_READ, component.reactorChannel());
							}
						}
					}
	
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

        } while(currentTimeUsec < stopTimeUsec && lastDispatchRet >= 0);
        
        /* Check for dispatch failure here */
        assertTrue(lastDispatchRet < 0);
        
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
            	
            	if(component.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
    			{
    				for(int i = 0; i < component.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().size(); ++i)
    				{
    					SelectableChannel fdChangeChannel = component.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().get(i);
    					
    					if(fdChangeChannel != null && !component.reactorChannel().warmStandbyChannelInfo().selectableChannelList().contains(fdChangeChannel))
    					{
    						if (fdChangeChannel.keyFor(_selector) != null)
    							fdChangeChannel.keyFor(_selector).cancel();
    					}
    				}
    			}
    			else if (component.reactorChannel().selectableChannel() != null )
    			{
    				/* If channel was up, the selectableChannel should be present. */
                    assertNotNull(component.reactorChannel().selectableChannel());
                    assertNotNull(component.reactorChannel().selectableChannel().keyFor(_selector));

                    /* Cancel key. */
                    component.reactorChannel().selectableChannel().keyFor(_selector).cancel();
    			}
            }

			component.reactorChannelIsUp(false);
			break;
		case ReactorChannelEventTypes.FD_CHANGE:
			if (isPreferredHostTest)
			{
				if (component.reactorChannel() == null)
					break;
			}
			assertNotNull(component.reactorChannel());
			if(component.reactorChannel().reactorChannelType() == ReactorChannelType.WARM_STANDBY)
			{
				for(int i = 0; i < component.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().size(); ++i)
				{
					SelectableChannel fdChangeChannel = component.reactorChannel().warmStandbyChannelInfo().oldSelectableChannelList().get(i);
					
					if(fdChangeChannel != null && !component.reactorChannel().warmStandbyChannelInfo().selectableChannelList().contains(fdChangeChannel))
					{
						if (fdChangeChannel.keyFor(_selector) != null)
							fdChangeChannel.keyFor(_selector).cancel();
					}
				}
			}
			else if (component.reactorChannel().selectableChannel() != null )
			{
				 /* Cancel old key. */
                component.reactorChannel().oldSelectableChannel().keyFor(_selector).cancel();
			}
				
			break;
		case ReactorChannelEventTypes.CHANNEL_READY:
		case ReactorChannelEventTypes.WARNING:
			break;
		case ReactorChannelEventTypes.PREFERRED_HOST_COMPLETE:
			System.out.println("Preferred Host Complete callback received.");
			break;
		case ReactorChannelEventTypes.PREFERRED_HOST_STARTING_FALLBACK:
			System.out.println("Preferred Host Start Fallback callback received.");
			break;
		default:
			fail("Unhandled ReactorChannelEventType.");
				
		}
		
		if (component.reactorChannel() != null && !switchingReactorChannel)
            assertEquals("Active event reactorChannel is unexpectedly different than the one on the component.", component.reactorChannel(), event.reactorChannel());
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
        connectOpts.connectionList().get(0).connectOptions().compressionType(opts.compressionType());
        connectOpts.connectionList().get(0).connectOptions().numInputBuffers(20);
        
        connectOpts.reconnectAttemptLimit(opts.reconnectAttemptLimit());
        connectOpts.reconnectMinDelay(opts.reconnectMinDelay());
        connectOpts.reconnectMaxDelay(opts.reconnectMaxDelay());
        connectOpts.connectionList().get(0).connectOptions().pingTimeout(opts.pingTimeout());
        connectOpts.connectionList().get(0).initTimeout(opts.consumerChannelInitTimeout());
        if (opts.getProtocolList() != null)
			connectOpts.connectionList().get(0).connectOptions().wSocketOpts().protocols(opts.getProtocolList());
        
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
	
	/** Associates a component with this reactor and opens a connection. This is meant to check for errors
	 * during the connection. */
	int connectFailureTest(ConsumerProviderSessionOptions opts, TestReactorComponent component, int port)
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
        connectOpts.connectionList().get(0).connectOptions().compressionType(opts.compressionType());
        connectOpts.connectionList().get(0).connectOptions().numInputBuffers(20);
        
        connectOpts.reconnectAttemptLimit(opts.reconnectAttemptLimit());
        connectOpts.reconnectMinDelay(opts.reconnectMinDelay());
        connectOpts.reconnectMaxDelay(opts.reconnectMaxDelay());
        connectOpts.connectionList().get(0).connectOptions().pingTimeout(opts.pingTimeout());
        connectOpts.connectionList().get(0).initTimeout(opts.consumerChannelInitTimeout());
        if (opts.getProtocolList() != null)
			connectOpts.connectionList().get(0).connectOptions().wSocketOpts().protocols(opts.getProtocolList());
        
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
        
        connectOpts._reactorPreferredHostOptions = opts.preferredHostOptions();
        
        ret = _reactor.connect(connectOpts, component.reactorRole(), _errorInfo);

        /* Clear ReactorConnectOptions after connecting -- this tests whether the Reactor is properly saving the options. */
        connectOpts.clear();

        return ret;
    }
	
	/** Associates a component with this reactor and opens a connection list. */
	void connectList(ConsumerProviderSessionOptions opts, TestReactorComponent component, int[] ports)
	{
        ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
        ReactorConnectInfo connectInfo;
        int ret;

        connectOpts.clear();
        for (int i = 0; i < ports.length; ++i)
        {
        	connectInfo = ReactorFactory.createReactorConnectInfo();
            connectOpts.connectionList().add(connectInfo);
            connectOpts.connectionList().get(i).connectOptions().majorVersion(Codec.majorVersion());
            connectOpts.connectionList().get(i).connectOptions().minorVersion(Codec.minorVersion());
            connectOpts.connectionList().get(i).connectOptions().connectionType(opts.connectionType());
            connectOpts.connectionList().get(i).connectOptions().userSpecObject(component);
            connectOpts.connectionList().get(i).connectOptions().compressionType(opts.compressionType());
            connectOpts.connectionList().get(i).connectOptions().numInputBuffers(20);
            
            connectOpts.reconnectAttemptLimit(opts.reconnectAttemptLimit());
            connectOpts.reconnectMinDelay(opts.reconnectMinDelay());
            connectOpts.reconnectMaxDelay(opts.reconnectMaxDelay());
            connectOpts.connectionList().get(i).connectOptions().pingTimeout(opts.pingTimeout());
            connectOpts.connectionList().get(i).initTimeout(opts.consumerChannelInitTimeout());
            if (opts.getProtocolList() != null)
    			connectOpts.connectionList().get(i).connectOptions().wSocketOpts().protocols(opts.getProtocolList());
            
            connectOpts.connectionList().get(i).connectOptions().unifiedNetworkInfo().address("localhost");
            connectOpts.connectionList().get(i).connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(ports[i]));
        
        }
        
        connectOpts._reactorPreferredHostOptions = opts.preferredHostOptions();
        

        ret = _reactor.connect(connectOpts, component.reactorRole(), _errorInfo);
        assertEquals("Connect failed: " + ret + "(" + _errorInfo.location() + " -- "+ _errorInfo.error().text() + ")",
        		ReactorReturnCodes.SUCCESS, ret);

        /* Clear ReactorConnectOptions after connecting -- this tests whether the Reactor is properly saving the options. */
        connectOpts.clear();

    }
	
	/** Associates a component with this reactor and opens a connection. */
	void connectWsb(ConsumerProviderSessionOptions opts, TestReactorComponent component, List<Provider> wsbGroup1, List<Provider> wsbGroup2, Provider channelList)
	{
        ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
        ReactorWarmStandbyGroup wsbGroup;
        ReactorWarmStandbyServerInfo wsbServerInfo;
        int ret;
        
        connectOpts.clear();
        
        connectOpts.reconnectAttemptLimit(opts.reconnectAttemptLimit());
        connectOpts.reconnectMinDelay(opts.reconnectMinDelay());
        connectOpts.reconnectMaxDelay(opts.reconnectMaxDelay());
        

        if(channelList != null)
        {
	        ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
	        connectOpts.connectionList().add(connectInfo);
	        connectOpts.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
	        connectOpts.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());
	        connectOpts.connectionList().get(0).connectOptions().connectionType(opts.connectionType());
	        connectOpts.connectionList().get(0).connectOptions().userSpecObject(component);
	        connectOpts.connectionList().get(0).connectOptions().compressionType(opts.compressionType());
	        connectOpts.connectionList().get(0).connectOptions().numInputBuffers(20);
	        connectOpts.connectionList().get(0).initTimeout(opts.consumerChannelInitTimeout());

	        connectOpts.connectionList().get(0).connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				connectOpts.connectionList().get(0).connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address("localhost");
	        connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(channelList.serverPort()));
        }
        
        if(wsbGroup1 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(component);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup1.get(0).serverPort()));
	        
	        for(int i = 1; i < wsbGroup1.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(component);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup1.get(i).serverPort()));
		        
		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }
	        
	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        
        if(wsbGroup2 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(component);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup2.get(0).serverPort()));
	        
	        for(int i = 1; i < wsbGroup2.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(component);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup2.get(i).serverPort()));
		        
		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }
	        
	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        

        
        ret = _reactor.connect(connectOpts, component.reactorRole(), _errorInfo);
        assertEquals("Connect failed: " + ret + "(" + _errorInfo.location() + " -- "+ _errorInfo.error().text() + ")",
        		ReactorReturnCodes.SUCCESS, ret);

        /* Clear ReactorConnectOptions after connecting -- this tests whether the Reactor is properly saving the options. */
        connectOpts.clear();

    }
	
	/** Associates a component with this reactor and opens a connection using session management. */
	ReactorConnectOptions connectWsb_ByPort_SessionManagement_NoStart(ConsumerProviderSessionOptions opts, ReactorConnectOptions rcOpts, TestReactorComponent component, Consumer consumer, String protocolList, boolean isWebsocket, DataDictionary dictionary, List<Integer> wsbGroup1, List<Integer> wsbGroup2, Integer channelPort)
	{
		ReactorConnectOptions connectOpts = ReactorFactory.createReactorConnectOptions();
		rcOpts.copy(connectOpts);
        ReactorWarmStandbyGroup wsbGroup;
        ReactorWarmStandbyServerInfo wsbServerInfo;
        int ret;
        
        /*
         * For each of these, integers help tell what kind of connection we're testing with
         * -1 = not used and not put into configuration
         * 0 = Used for Session Management = Service Discovery
         * 1 = Misconfigured, cannot connect to it, Session Management disabled and port is 1.
         * 2 = Session Management enabled, Endpoint specified
         * 
         * Over 100 = port as-is, no Session Management enabled
         */

        if(channelPort != -1)
        {
	        ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
	        connectOpts.connectionList().add(connectInfo);
	        connectOpts.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
	        connectOpts.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());
	        connectOpts.connectionList().get(0).connectOptions().connectionType(opts.connectionType());
	        connectOpts.connectionList().get(0).connectOptions().userSpecObject(consumer);
	        connectOpts.connectionList().get(0).connectOptions().compressionType(opts.compressionType());
	        connectOpts.connectionList().get(0).connectOptions().numInputBuffers(20);
	        connectOpts.connectionList().get(0).initTimeout(opts.consumerChannelInitTimeout());

	        connectOpts.connectionList().get(0).connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				connectOpts.connectionList().get(0).connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        if (channelPort == 1 || channelPort > 100)
	        {
		        connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address("localhost");
		        connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(channelPort));	
	        }
	        else if (channelPort == 2)
	        {
	        	String address = LDP_ENDPOINT_ADDRESS;
	        	if (isWebsocket)
	        	{
	        		address = LDP_ENDPOINT_ADDRESS_WEBSOCKET;
		        	connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(LDP_ENDPOINT_PORT_WEBSOCKET));	
	        	}
	        	else
		        	connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(LDP_ENDPOINT_PORT));	
	        	connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address(address);

	        }
	        
	        // Handle session management options
	        if (channelPort == 0 || channelPort == 2)
	        {
	        	connectOpts.connectionList().get(0).connectOptions().connectionType(ConnectionTypes.ENCRYPTED);
				if (isWebsocket) {
					connectOpts.connectionList().get(0).connectOptions().encryptionOptions().connectionType(ConnectionTypes.WEBSOCKET);
				}	        	
	        }
	        
			if (isWebsocket) {
				connectOpts.connectionList().get(0).connectOptions().wSocketOpts().protocols(protocolList);
			}	        	

			connectOpts.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
			connectOpts.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());

			if(isWebsocket)
			{
				if(Objects.nonNull(System.getProperty("keyfile")))
					connectOpts.connectionList().get(0).connectOptions().encryptionOptions().KeystoreFile(System.getProperty("keyfile"));

				if(Objects.nonNull(System.getProperty("keypasswd")))
					connectOpts.connectionList().get(0).connectOptions().encryptionOptions().KeystorePasswd(System.getProperty("keypasswd"));

				connectOpts.connectionList().get(0).connectOptions().encryptionOptions().KeystoreType("JKS");
				connectOpts.connectionList().get(0).connectOptions().encryptionOptions().SecurityProtocol("TLS");
				connectOpts.connectionList().get(0).connectOptions().encryptionOptions().SecurityProtocolVersions(new String[] {"1.3", "1.2"});
				connectOpts.connectionList().get(0).connectOptions().encryptionOptions().SecurityProvider("SunJSSE");
				connectOpts.connectionList().get(0).connectOptions().encryptionOptions().KeyManagerAlgorithm("SunX509");
				connectOpts.connectionList().get(0).connectOptions().encryptionOptions().TrustManagerAlgorithm("PKIX");
				connectOpts.connectionList().get(0).connectOptions().userSpecObject(consumer);
				
				if (protocolList.contains("tr_json2") || protocolList.contains("rssl.json.v2")) {
					consumer.testReactor().initJsonConverter(dictionary);
	            		}
			}
			else
			{
				if(Objects.nonNull(System.getProperty("keyfile")))
					connectOpts.connectionList().get(0).connectOptions().tunnelingInfo().KeystoreFile(System.getProperty("keyfile"));

				if(Objects.nonNull(System.getProperty("keypasswd")))
					connectOpts.connectionList().get(0).connectOptions().tunnelingInfo().KeystorePasswd(System.getProperty("keypasswd"));

				connectOpts.connectionList().get(0).connectOptions().tunnelingInfo().objectName("");
				connectOpts.connectionList().get(0).connectOptions().tunnelingInfo().KeystoreType("JKS");
				connectOpts.connectionList().get(0).connectOptions().tunnelingInfo().SecurityProtocol("TLS");
				connectOpts.connectionList().get(0).connectOptions().tunnelingInfo().SecurityProvider("SunJSSE");
				connectOpts.connectionList().get(0).connectOptions().tunnelingInfo().KeyManagerAlgorithm("SunX509");
				connectOpts.connectionList().get(0).connectOptions().tunnelingInfo().TrustManagerAlgorithm("PKIX");		
				connectOpts.connectionList().get(0).connectOptions().tunnelingInfo().tunnelingType("encrypted");			
				connectOpts.connectionList().get(0).connectOptions().userSpecObject(consumer);
			}

			if (channelPort == 0 || channelPort == 2)
				connectOpts.connectionList().get(0).enableSessionManagement(true);
			connectOpts.connectionList().get(0).reactorAuthTokenEventCallback(consumer);	
        }
        
        if(wsbGroup1 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(consumer);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        if (wsbGroup1.get(0) == 1 || wsbGroup1.get(0) > 100)
	        {
	        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup1.get(0)));	
	        }
	        else if (wsbGroup1.get(0) == 2)
	        {
	        	String address = LDP_ENDPOINT_ADDRESS;
	        	if (isWebsocket)
	        	{
	        		address = LDP_ENDPOINT_ADDRESS_WEBSOCKET;	
		        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(LDP_ENDPOINT_PORT_WEBSOCKET));	
	        	}
	        	else
		        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(LDP_ENDPOINT_PORT));	
	        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address(address);
	        }
	        
	        // Handle session management options
	        if (wsbGroup1.get(0) == 0 || wsbGroup1.get(0) == 2)
	        {
		        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(ConnectionTypes.ENCRYPTED);
				if (isWebsocket) {
					wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().connectionType(ConnectionTypes.WEBSOCKET);
				}	        	
	        }
	        
			if (isWebsocket) {
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(protocolList);
			}	      

			wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
			wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());

			if(isWebsocket)
			{
				if(Objects.nonNull(System.getProperty("keyfile")))
					wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().KeystoreFile(System.getProperty("keyfile"));

				if(Objects.nonNull(System.getProperty("keypasswd")))
					wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().KeystorePasswd(System.getProperty("keypasswd"));

				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().KeystoreType("JKS");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().SecurityProtocol("TLS");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().SecurityProtocolVersions(new String[] {"1.3", "1.2"});
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().SecurityProvider("SunJSSE");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().KeyManagerAlgorithm("SunX509");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().TrustManagerAlgorithm("PKIX");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(consumer);
				
				if (protocolList.contains("tr_json2") || protocolList.contains("rssl.json.v2")) {
					consumer.testReactor().initJsonConverter(dictionary);
	            		}
			}
			else
			{
				if(Objects.nonNull(System.getProperty("keyfile")))
					wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().KeystoreFile(System.getProperty("keyfile"));

				if(Objects.nonNull(System.getProperty("keypasswd")))
					wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().KeystorePasswd(System.getProperty("keypasswd"));

				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().objectName("");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().KeystoreType("JKS");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().SecurityProtocol("TLS");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().SecurityProvider("SunJSSE");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().KeyManagerAlgorithm("SunX509");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().TrustManagerAlgorithm("PKIX");		
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().tunnelingType("encrypted");			
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(consumer);
			}

			if (wsbGroup1.get(0) == 0 || wsbGroup1.get(0) == 2)
				wsbGroup.startingActiveServer().reactorConnectInfo().enableSessionManagement(true);
			wsbGroup.startingActiveServer().reactorConnectInfo().reactorAuthTokenEventCallback(consumer);	
	        
	        for(int i = 1; i < wsbGroup1.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(consumer);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        if (wsbGroup1.get(1) != null && (wsbGroup1.get(1) == 1 || wsbGroup1.get(1) > 100))
		        {
		        	wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        	wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup1.get(wsbGroup1.get(1))));	
		        }
		        else if (wsbGroup1.get(1) != null && wsbGroup1.get(1) == 2)
		        {
		        	String address = LDP_ENDPOINT_ADDRESS;
		        	if (isWebsocket)
		        	{
		        		address = LDP_ENDPOINT_ADDRESS_WEBSOCKET;
			        	wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(LDP_ENDPOINT_PORT_WEBSOCKET));	
		        	}
		        	else
			        	wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(LDP_ENDPOINT_PORT));	
		        	wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address(address);
		        }
		        
		     // Handle session management options
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(ConnectionTypes.ENCRYPTED);
				if (isWebsocket) {
					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().connectionType(ConnectionTypes.WEBSOCKET);
				}
				
				if (isWebsocket) {
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(protocolList);
				}	      
				wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
				wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());

				if(isWebsocket)
				{
					if(Objects.nonNull(System.getProperty("keyfile")))
						wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().KeystoreFile(System.getProperty("keyfile"));

					if(Objects.nonNull(System.getProperty("keypasswd")))
						wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().KeystorePasswd(System.getProperty("keypasswd"));

					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().KeystoreType("JKS");
					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().SecurityProtocol("TLS");
					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().SecurityProtocolVersions(new String[] {"1.3", "1.2"});
					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().SecurityProvider("SunJSSE");
					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().KeyManagerAlgorithm("SunX509");
					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().TrustManagerAlgorithm("PKIX");
					wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(consumer);
					
					if (protocolList.contains("tr_json2") || protocolList.contains("rssl.json.v2")) {
						consumer.testReactor().initJsonConverter(dictionary);
		            		}
				}
				else
				{
					if(Objects.nonNull(System.getProperty("keyfile")))
						wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().KeystoreFile(System.getProperty("keyfile"));

					if(Objects.nonNull(System.getProperty("keypasswd")))
						wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().KeystorePasswd(System.getProperty("keypasswd"));

					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().objectName("");
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().KeystoreType("JKS");
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().SecurityProtocol("TLS");
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().SecurityProvider("SunJSSE");
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().KeyManagerAlgorithm("SunX509");
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().TrustManagerAlgorithm("PKIX");		
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().tunnelingType("encrypted");			
					wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(consumer);
				}

				if (wsbGroup1.get(1) == 0 || wsbGroup1.get(1) == 2)
					wsbServerInfo.reactorConnectInfo().enableSessionManagement(true);
				wsbServerInfo.reactorConnectInfo().reactorAuthTokenEventCallback(consumer);	
		        
		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }
	        
	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        
        if(wsbGroup2 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(consumer);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        if (wsbGroup2.get(0) == 1 || wsbGroup2.get(0) > 100)
	        {
	        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup2.get(0)));	
	        }
	        else if (wsbGroup2.get(0) == 2)
	        {
	        	String address = LDP_ENDPOINT_ADDRESS;
	        	if (isWebsocket)
	        	{
	        		address = LDP_ENDPOINT_ADDRESS_WEBSOCKET;
		        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(LDP_ENDPOINT_PORT_WEBSOCKET));	
	        	}
	        	else
		        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(LDP_ENDPOINT_PORT));	
	        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address(address);
	        }
	        
	        // Handle session management options
	        if (wsbGroup2.get(0) == 0 || wsbGroup2.get(0) == 2)
	        {
		        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(ConnectionTypes.ENCRYPTED);
				if (isWebsocket) {
					wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().connectionType(ConnectionTypes.WEBSOCKET);
				}	
	        }
			if (isWebsocket) {
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(protocolList);
			}	      
			wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
			wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());

			if(isWebsocket)
			{
				if(Objects.nonNull(System.getProperty("keyfile")))
					wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().KeystoreFile(System.getProperty("keyfile"));

				if(Objects.nonNull(System.getProperty("keypasswd")))
					wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().KeystorePasswd(System.getProperty("keypasswd"));

				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().KeystoreType("JKS");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().SecurityProtocol("TLS");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().SecurityProtocolVersions(new String[] {"1.3", "1.2"});
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().SecurityProvider("SunJSSE");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().KeyManagerAlgorithm("SunX509");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().encryptionOptions().TrustManagerAlgorithm("PKIX");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(consumer);
				
				if (protocolList.contains("tr_json2") || protocolList.contains("rssl.json.v2")) {
					consumer.testReactor().initJsonConverter(dictionary);
	            		}
			}
			else
			{
				if(Objects.nonNull(System.getProperty("keyfile")))
					wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().KeystoreFile(System.getProperty("keyfile"));

				if(Objects.nonNull(System.getProperty("keypasswd")))
					wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().KeystorePasswd(System.getProperty("keypasswd"));

				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().objectName("");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().KeystoreType("JKS");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().SecurityProtocol("TLS");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().SecurityProvider("SunJSSE");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().KeyManagerAlgorithm("SunX509");
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().TrustManagerAlgorithm("PKIX");		
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().tunnelingInfo().tunnelingType("encrypted");			
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(consumer);
			}

			if (wsbGroup2.get(0) == 0 || wsbGroup2.get(0) == 2)
				wsbGroup.startingActiveServer().reactorConnectInfo().enableSessionManagement(true);
			wsbGroup.startingActiveServer().reactorConnectInfo().reactorAuthTokenEventCallback(consumer);	
	        
	        for(int i = 1; i < wsbGroup2.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(consumer);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        if (wsbGroup2.get(1) != null && (wsbGroup2.get(1) == 1 || wsbGroup2.get(1) >= 100))
		        {
		        	wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        	wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup2.get(wsbGroup2.get(1))));	
		        }
		        else if (wsbGroup2.get(1) != null && wsbGroup2.get(1) == 2)
		        {
		        	String address = LDP_ENDPOINT_ADDRESS;
		        	if (isWebsocket)
		        	{
		        		address = LDP_ENDPOINT_ADDRESS_WEBSOCKET;
			        	wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(LDP_ENDPOINT_PORT_WEBSOCKET));	
		        	}
		        	else
			        	wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(LDP_ENDPOINT_PORT));	
		        	wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address(address);
		        }
		        
		        // Handle session management options
		        if (wsbGroup2.get(0) == 0 || wsbGroup2.get(0) == 2)
		        {
			        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(ConnectionTypes.ENCRYPTED);
					if (isWebsocket) {
						wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().connectionType(ConnectionTypes.WEBSOCKET);
					}
		        }
				if (isWebsocket) {
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(protocolList);
				}	      

				wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
				wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());

				if(isWebsocket)
				{
					if(Objects.nonNull(System.getProperty("keyfile")))
						wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().KeystoreFile(System.getProperty("keyfile"));

					if(Objects.nonNull(System.getProperty("keypasswd")))
						wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().KeystorePasswd(System.getProperty("keypasswd"));

					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().KeystoreType("JKS");
					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().SecurityProtocol("TLS");
					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().SecurityProtocolVersions(new String[] {"1.3", "1.2"});
					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().SecurityProvider("SunJSSE");
					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().KeyManagerAlgorithm("SunX509");
					wsbServerInfo.reactorConnectInfo().connectOptions().encryptionOptions().TrustManagerAlgorithm("PKIX");
					wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(consumer);
					
					if (protocolList.contains("tr_json2") || protocolList.contains("rssl.json.v2")) {
						consumer.testReactor().initJsonConverter(dictionary);
		            		}
				}
				else
				{
					if(Objects.nonNull(System.getProperty("keyfile")))
						wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().KeystoreFile(System.getProperty("keyfile"));

					if(Objects.nonNull(System.getProperty("keypasswd")))
						wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().KeystorePasswd(System.getProperty("keypasswd"));

					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().objectName("");
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().KeystoreType("JKS");
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().SecurityProtocol("TLS");
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().SecurityProvider("SunJSSE");
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().KeyManagerAlgorithm("SunX509");
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().TrustManagerAlgorithm("PKIX");		
					wsbServerInfo.reactorConnectInfo().connectOptions().tunnelingInfo().tunnelingType("encrypted");			
					wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(consumer);
				}

				if (wsbGroup2.get(1) == 0 || wsbGroup2.get(1) == 2)
				wsbServerInfo.reactorConnectInfo().enableSessionManagement(true);
				wsbServerInfo.reactorConnectInfo().reactorAuthTokenEventCallback(consumer);	
		        
		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }
	        
	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        
        return connectOpts;

    }
	
	/** Associates a component with this reactor and opens a connection. */
	void connectWsb(ReactorConnectOptions connectOpts, ConsumerProviderSessionOptions opts, TestReactorComponent component, List<Provider> wsbGroup1, List<Provider> wsbGroup2, List<Provider> wsbGroup3, List<Provider> channelList)
	{
        ReactorWarmStandbyGroup wsbGroup;
        ReactorWarmStandbyServerInfo wsbServerInfo;
        ReactorConnectInfo connectInfo;
        int ret;
        
        Buffer serviceName = CodecFactory.createBuffer();
		serviceName.data(DEFAULT_SERVICE);

        connectOpts.reconnectAttemptLimit(opts.reconnectAttemptLimit());
        connectOpts.reconnectMinDelay(opts.reconnectMinDelay());
        connectOpts.reconnectMaxDelay(opts.reconnectMaxDelay());
        

        if (channelList != null)
	        for (int i = 0; i < channelList.size(); ++i)
	        {
	        	connectInfo = ReactorFactory.createReactorConnectInfo();
	            connectOpts.connectionList().add(connectInfo);
	            connectOpts.connectionList().get(i).connectOptions().majorVersion(Codec.majorVersion());
	            connectOpts.connectionList().get(i).connectOptions().minorVersion(Codec.minorVersion());
	            connectOpts.connectionList().get(i).connectOptions().connectionType(opts.connectionType());
	            connectOpts.connectionList().get(i).connectOptions().userSpecObject(component);
	            connectOpts.connectionList().get(i).connectOptions().compressionType(opts.compressionType());
	            connectOpts.connectionList().get(i).connectOptions().numInputBuffers(20);
	            
	            connectOpts.reconnectAttemptLimit(opts.reconnectAttemptLimit());
	            connectOpts.reconnectMinDelay(opts.reconnectMinDelay());
	            connectOpts.reconnectMaxDelay(opts.reconnectMaxDelay());
	            connectOpts.connectionList().get(i).connectOptions().pingTimeout(opts.pingTimeout());
	            connectOpts.connectionList().get(i).initTimeout(opts.consumerChannelInitTimeout());
	            if (opts.getProtocolList() != null)
	    			connectOpts.connectionList().get(i).connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	            
	            connectOpts.connectionList().get(i).connectOptions().unifiedNetworkInfo().address("localhost");
	            connectOpts.connectionList().get(i).connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(channelList.get(i).serverPort()));
	           
	        }
        
        if(wsbGroup1 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(component);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup1.get(0).serverPort()));
	        wsbGroup.startingActiveServer().perServiceBasedOptions().serviceNameList().add(serviceName);
	        
	        for(int i = 1; i < wsbGroup1.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(component);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup1.get(i).serverPort()));
		        wsbServerInfo.perServiceBasedOptions().serviceNameList().add(serviceName);
		        
		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }

	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        
        if(wsbGroup2 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(component);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup2.get(0).serverPort()));
	        wsbGroup.startingActiveServer().perServiceBasedOptions().serviceNameList().add(serviceName);
	        
	        for(int i = 1; i < wsbGroup2.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(component);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup2.get(i).serverPort()));
		        wsbServerInfo.perServiceBasedOptions().serviceNameList().add(serviceName);
		        
		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }

	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        
        if(wsbGroup3 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(component);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup3.get(0).serverPort()));
	        wsbGroup.startingActiveServer().perServiceBasedOptions().serviceNameList().add(serviceName);
	        
	        for(int i = 1; i < wsbGroup3.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(component);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup3.get(i).serverPort()));
		        wsbServerInfo.perServiceBasedOptions().serviceNameList().add(serviceName);
		        
		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }
	        
	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        

        
        ret = _reactor.connect(connectOpts, component.reactorRole(), _errorInfo);
        assertEquals("Connect failed: " + ret + "(" + _errorInfo.location() + " -- "+ _errorInfo.error().text() + ")",
        		ReactorReturnCodes.SUCCESS, ret);

        /* Clear ReactorConnectOptions after connecting -- this tests whether the Reactor is properly saving the options. */
        connectOpts.clear();

    }
	
	/** Associates a component with this reactor and doesn't open a connection. */
	void connectWsbNoStart(ReactorConnectOptions connectOpts, ConsumerProviderSessionOptions opts, TestReactorComponent component, List<Provider> wsbGroup1, List<Provider> wsbGroup2, List<Provider> wsbGroup3, List<Provider> channelList)
	{
        ReactorWarmStandbyGroup wsbGroup;
        ReactorWarmStandbyServerInfo wsbServerInfo;
        ReactorConnectInfo connectInfo;

        connectOpts.reconnectAttemptLimit(opts.reconnectAttemptLimit());
        connectOpts.reconnectMinDelay(opts.reconnectMinDelay());
        connectOpts.reconnectMaxDelay(opts.reconnectMaxDelay());
        

        if (channelList != null)
	        for (int i = 0; i < channelList.size(); ++i)
	        {
	        	connectInfo = ReactorFactory.createReactorConnectInfo();
	            connectOpts.connectionList().add(connectInfo);
	            connectOpts.connectionList().get(i).connectOptions().majorVersion(Codec.majorVersion());
	            connectOpts.connectionList().get(i).connectOptions().minorVersion(Codec.minorVersion());
	            connectOpts.connectionList().get(i).connectOptions().connectionType(opts.connectionType());
	            connectOpts.connectionList().get(i).connectOptions().userSpecObject(component);
	            connectOpts.connectionList().get(i).connectOptions().compressionType(opts.compressionType());
	            connectOpts.connectionList().get(i).connectOptions().numInputBuffers(20);
	            
	            connectOpts.reconnectAttemptLimit(opts.reconnectAttemptLimit());
	            connectOpts.reconnectMinDelay(opts.reconnectMinDelay());
	            connectOpts.reconnectMaxDelay(opts.reconnectMaxDelay());
	            connectOpts.connectionList().get(i).connectOptions().pingTimeout(opts.pingTimeout());
	            connectOpts.connectionList().get(i).initTimeout(opts.consumerChannelInitTimeout());
	            if (opts.getProtocolList() != null)
	    			connectOpts.connectionList().get(i).connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	            
	            connectOpts.connectionList().get(i).connectOptions().unifiedNetworkInfo().address("localhost");
	            connectOpts.connectionList().get(i).connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(channelList.get(i).serverPort()));
	           
	        }
        
        if(wsbGroup1 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(component);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup1.get(0).serverPort()));

	        for(int i = 1; i < wsbGroup1.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(component);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup1.get(i).serverPort()));

		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }
	        
	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        
        if(wsbGroup2 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(component);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup2.get(0).serverPort()));

	        for(int i = 1; i < wsbGroup2.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(component);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup2.get(i).serverPort()));

		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }
	        
	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        
        if(wsbGroup3 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(component);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup3.get(0).serverPort()));

	        for(int i = 1; i < wsbGroup3.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(component);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup3.get(i).serverPort()));

		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }
	        
	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        
    }
	
	void lateStartConnect(ReactorConnectOptions connectOpts, TestReactorComponent component)
	{
        int ret = _reactor.connect(connectOpts, component.reactorRole(), _errorInfo);
        assertEquals("Connect failed: " + ret + "(" + _errorInfo.location() + " -- "+ _errorInfo.error().text() + ")",
        		ReactorReturnCodes.SUCCESS, ret);

        /* Clear ReactorConnectOptions after connecting -- this tests whether the Reactor is properly saving the options. */
        connectOpts.clear();

	}
	
	void lateStartConnect(ReactorConnectOptions connectOpts, TestReactorComponent component, boolean clearConnectOpts)
	{
        int ret = _reactor.connect(connectOpts, component.reactorRole(), _errorInfo);
        assertEquals("Connect failed: " + ret + "(" + _errorInfo.location() + " -- "+ _errorInfo.error().text() + ")",
        		ReactorReturnCodes.SUCCESS, ret);

        /* Clear ReactorConnectOptions after connecting -- this tests whether the Reactor is properly saving the options. */
        if (clearConnectOpts)
        	connectOpts.clear();

	}
	
	/** Associates a component with this reactor and opens a connection. This is meant to test failure scenarios */
	int connectWsbFailureTest(ReactorConnectOptions connectOpts, ConsumerProviderSessionOptions opts, TestReactorComponent component, List<Provider> wsbGroup1, List<Provider> wsbGroup2, List<Provider> wsbGroup3, Provider channelList)
	{
        ReactorWarmStandbyGroup wsbGroup;
        ReactorWarmStandbyServerInfo wsbServerInfo;
        int ret;
        
        connectOpts.reconnectAttemptLimit(opts.reconnectAttemptLimit());
        connectOpts.reconnectMinDelay(opts.reconnectMinDelay());
        connectOpts.reconnectMaxDelay(opts.reconnectMaxDelay());
        

        if(channelList != null)
        {
	        ReactorConnectInfo connectInfo = ReactorFactory.createReactorConnectInfo();
	        connectOpts.connectionList().add(connectInfo);
	        connectOpts.connectionList().get(0).connectOptions().majorVersion(Codec.majorVersion());
	        connectOpts.connectionList().get(0).connectOptions().minorVersion(Codec.minorVersion());
	        connectOpts.connectionList().get(0).connectOptions().connectionType(opts.connectionType());
	        connectOpts.connectionList().get(0).connectOptions().userSpecObject(component);
	        connectOpts.connectionList().get(0).connectOptions().compressionType(opts.compressionType());
	        connectOpts.connectionList().get(0).connectOptions().numInputBuffers(20);
	        connectOpts.connectionList().get(0).initTimeout(opts.consumerChannelInitTimeout());

	        connectOpts.connectionList().get(0).connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				connectOpts.connectionList().get(0).connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().address("localhost");
	        connectOpts.connectionList().get(0).connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(channelList.serverPort()));
        }
        
        if(wsbGroup1 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(component);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup1.get(0).serverPort()));
	        
	        for(int i = 1; i < wsbGroup1.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(component);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup1.get(i).serverPort()));
		        
		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }
	        
	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        
        if(wsbGroup2 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(component);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup2.get(0).serverPort()));
	        
	        for(int i = 1; i < wsbGroup2.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(component);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup2.get(i).serverPort()));
		        
		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }
	        
	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        
        if(wsbGroup3 != null)
        {
        	wsbGroup = ReactorFactory.createReactorWarmStandbyGroup();
        	
        	wsbGroup.warmStandbyMode(opts.wsbMode());
        	
        	wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().userSpecObject(component);
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().numInputBuffers(20);
	        wsbGroup.startingActiveServer().reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
	        
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
	        if (opts.getProtocolList() != null)
				wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
	        wsbGroup.startingActiveServer().reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup3.get(0).serverPort()));
	        
	        for(int i = 1; i < wsbGroup3.size(); i++)
	        {
	        	wsbServerInfo = ReactorFactory.createReactorWarmStandbyServerInfo();
	        	wsbServerInfo.reactorConnectInfo().connectOptions().majorVersion(Codec.majorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().minorVersion(Codec.minorVersion());
		        wsbServerInfo.reactorConnectInfo().connectOptions().connectionType(opts.connectionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().userSpecObject(component);
		        wsbServerInfo.reactorConnectInfo().connectOptions().compressionType(opts.compressionType());
		        wsbServerInfo.reactorConnectInfo().connectOptions().numInputBuffers(20);
		        wsbServerInfo.reactorConnectInfo().initTimeout(opts.consumerChannelInitTimeout());
		        
		        wsbServerInfo.reactorConnectInfo().connectOptions().pingTimeout(opts.pingTimeout());
		        if (opts.getProtocolList() != null)
					wsbServerInfo.reactorConnectInfo().connectOptions().wSocketOpts().protocols(opts.getProtocolList());
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().address("localhost");
		        wsbServerInfo.reactorConnectInfo().connectOptions().unifiedNetworkInfo().serviceName(String.valueOf(wsbGroup3.get(i).serverPort()));
		        
		        wsbGroup.standbyServerList().add(wsbServerInfo);
	        }
	        
	        connectOpts._reactorWarmStandyGroupList.add(wsbGroup);
        }
        

        
        ret = _reactor.connect(connectOpts, component.reactorRole(), _errorInfo);

        /* Clear ReactorConnectOptions after connecting -- this tests whether the Reactor is properly saving the options. */
        connectOpts.clear();

        return ret;
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

		if (_selector != null && _selector.isOpen())
		{
			try
			{
				_selector.close();
			}
			catch (Exception e)
			{
				System.out.println("Caught exception while closing selector: " + e);
				e.printStackTrace();
			}
		}

        if(_eventQueue != null)
        	_eventQueue.clear();
    }
	
}
