///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.reactor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertFalse;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;

import org.junit.Test;

import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.DomainTypes;

public class ReactorInteractionJunit {
	
    /** Reusable ReactorErrorInfo */
    ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
    
    @Test
    public void SimpleRequestTest_Watchlist()
    {
		/* Test a simple request/refresh exchange with the watchlist enabled. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
                
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
        consumerRole.watchlistOptions().enableWatchlist(true);
        consumerRole.watchlistOptions().channelOpenCallback(consumer);
        
        
        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
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
        
        /* Provider receives request. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
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
        
        /* Provider sends refresh .*/
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
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
	/** Consumer component that closes on any message received via the default callback. 
	 * Used by SimpleRequestTest_CloseFromCallback to test sending a close in response to
	 * the refresh. */
    class CloseOnDefaultMsgConsumer extends Consumer
    {
        public CloseOnDefaultMsgConsumer(TestReactor testReactor)
        {
            super(testReactor);
        }

        @Override
        public int defaultMsgCallback(ReactorMsgEvent event)
        {
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
            super.defaultMsgCallback(event);
           
            closeMsg.clear();
            closeMsg.msgClass(MsgClasses.CLOSE);
            closeMsg.domainType(event.msg().domainType());
            closeMsg.streamId(event.msg().streamId());
            submit(closeMsg, submitOptions);
            return ReactorReturnCodes.SUCCESS;
        }
    }
    
	@Test
	public void SimpleRequestTest_CloseFromCallback()
	{
		/* Test a simple request/refresh  exchange (no watchlist), followed by
		 * a close from the consumer. */

	    ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
		RequestMsg requestMsg = (RequestMsg)msg;
		RefreshMsg refreshMsg = (RefreshMsg)msg;
		RequestMsg receivedRequestMsg;
		RefreshMsg receivedRefreshMsg;
		CloseMsg receivedCloseMsg;
		
		/* Create reactors. */
		TestReactor consumerReactor = new TestReactor();
		TestReactor providerReactor = new TestReactor();
				
		/* Create consumer. */
		Consumer consumer = new CloseOnDefaultMsgConsumer(consumerReactor);
		ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        consumerRole.watchlistOptions().channelOpenCallback(consumer);
		
        /* Create provider. */
		Provider provider = new Provider(providerReactor);
		ProviderRole providerRole = (ProviderRole)provider.reactorRole();
		providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
		
		
		/* Connect the consumer and provider. Setup login & directory streams automatically. */
		ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
		opts.setupDefaultLoginStream(true);
		opts.setupDefaultDirectoryStream(true);
		provider.bind(opts);
		TestReactor.openSession(consumer, provider, opts);
		
		/* Consumer sends request. */
		requestMsg.clear();
		requestMsg.msgClass(MsgClasses.REQUEST);
		requestMsg.applyStreaming();
		requestMsg.domainType(DomainTypes.MARKET_PRICE);
		requestMsg.streamId(5);
		requestMsg.msgKey().applyHasServiceId();
		requestMsg.msgKey().serviceId(Provider.defaultService().serviceId());
		requestMsg.msgKey().applyHasName();
		requestMsg.msgKey().name().data("TRI.N");
		assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
		
		/* Provider receives request. */
		providerReactor.dispatch(1);
		event = providerReactor.pollEvent();
		assertEquals(TestReactorEventTypes.MSG, event.type());
		msgEvent = (ReactorMsgEvent)event.reactorEvent();
		assertNotNull(msgEvent.transportBuffer());
		assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
		
		receivedRequestMsg = (RequestMsg)msgEvent.msg();
		assertTrue(receivedRequestMsg.checkStreaming());
	    assertFalse(receivedRequestMsg.checkNoRefresh());
		assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
		assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
		assertTrue(receivedRequestMsg.msgKey().checkHasName());
		assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
		assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
		assertEquals(5, requestMsg.streamId());
		
		/* Provider sends refresh .*/
		refreshMsg.clear();
		refreshMsg.msgClass(MsgClasses.REFRESH);
		refreshMsg.domainType(DomainTypes.MARKET_PRICE);
		refreshMsg.streamId(5);
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
		assertNotNull(msgEvent.transportBuffer());
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
		
		/* Provider receives close (Consumer submitted one from inside the callback); */
		providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertNotNull(msgEvent.transportBuffer());
        assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
        receivedCloseMsg = (CloseMsg)msgEvent.msg();
        assertEquals(5, receivedCloseMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());
				
		TestReactorComponent.closeSession(consumer, provider);
	}
}
