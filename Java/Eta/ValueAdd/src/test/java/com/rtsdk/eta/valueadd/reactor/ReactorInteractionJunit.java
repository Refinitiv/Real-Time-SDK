///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.eta.valueadd.reactor;

import org.junit.Test;

import com.rtsdk.eta.codec.CloseMsg;
import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.DataStates;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.codec.RefreshMsg;
import com.rtsdk.eta.codec.RequestMsg;
import com.rtsdk.eta.codec.StreamStates;
import com.rtsdk.eta.rdm.DomainTypes;

import static org.junit.Assert.*;

public class ReactorInteractionJunit
{
	
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
		initProviderRole(provider);

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
        createRefreshMessage(refreshMsg, providerStreamId, "TRI.N");

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
        /** in case value is N>0 - consumer is going to listen for the messages and will forcible close channel after Nth message*/
        private int _forceCloseChannelMessageNumber;
        /** counter for consumer messages to drop connection */
        private int _consumerDefaultMessagesCounter = 0;
        /** makes consumer to return ReactorReturnCodes.FAILURE after channel is closed */
        private boolean _returnFailure;

        public CloseOnDefaultMsgConsumer(TestReactor testReactor, int breakChannelMessageNumber, boolean returnFailure)
        {
            super(testReactor);
            _forceCloseChannelMessageNumber = breakChannelMessageNumber;
            _returnFailure = returnFailure; 
        }

        @Override
        public int defaultMsgCallback(ReactorMsgEvent event)
        {
            super.defaultMsgCallback(event);

            //ordinary scenario - sending close message
            if(_forceCloseChannelMessageNumber <=0){
                ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
                CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
           
                closeMsg.clear();
                closeMsg.msgClass(MsgClasses.CLOSE);
                closeMsg.domainType(event.msg().domainType());
                closeMsg.streamId(event.msg().streamId());
                submit(closeMsg, submitOptions);
                return ReactorReturnCodes.SUCCESS;
            }

            /**unexpected consumer failure scenario, going to close client channel upon
             * _forceCloseChannelMessageNumber-th message arrival
             *
             */
            if(_forceCloseChannelMessageNumber > 0) {
                _consumerDefaultMessagesCounter++;
                //forced channel closing after _forceCloseChannelMessageNumber message was received
                if (_consumerDefaultMessagesCounter == _forceCloseChannelMessageNumber) {
                    
                    event.reactorChannel().close(_errorInfo);
                    assert _errorInfo._code == ReactorReturnCodes.SUCCESS : "Consumer close channel failed";
                    return _returnFailure ? ReactorReturnCodes.FAILURE : ReactorReturnCodes.SUCCESS;
                } else if(_consumerDefaultMessagesCounter > _forceCloseChannelMessageNumber) {
                    fail("Consumer message handler was invoked after channel forced closing");
                }
            }
            return ReactorReturnCodes.SUCCESS;
        }
    }
    
	@Test
	public void SimpleRequestTest_CloseFromCallback()
	{
		/* Test a simple request/refresh  exchange (no watchlist), followed by
		 * a close from the consumer. */
		SimpleRequestTest_CloseFromCallbackInner(0);
	}

	@Test
	public void SimpleRequestTest_UnexpectedCloseFromCallback()
	{
		/* Test a simple request/refresh exchange followed by
		 * force channel close from the consumer. */
		SimpleRequestTest_CloseFromCallbackInner(20);
    }

	@Test
	public void SimpleRequestTest_UnexpectedCloseFromCallbackWithoutReactorShutdown()
	{
		int failureCount =20;
		ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();

		/* Create reactors. */
		TestReactor consumerReactor = new TestReactor();
		TestReactor providerReactor = new TestReactor();

		/* Create consumer. */
		Consumer consumer = new CloseOnDefaultMsgConsumer(consumerReactor, failureCount, false);
		initConsumerRole(consumer);

		/* Create provider. */
		Provider provider = new Provider(providerReactor);
		initProviderRole(provider);
		
		/* Connect the consumer and provider. Setup login & directory streams automatically. */
		ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
		opts.setupDefaultLoginStream(true);
		opts.setupDefaultDirectoryStream(true);
		provider.bind(opts);
		TestReactor.openSession(consumer, provider, opts);
		RefreshMsg refreshMsg = (RefreshMsg) CodecFactory.createMsg();
		//sending messages for force disconnect scenario, starting from 1 as as handler counts every message
		for(int i=1; i<failureCount*2; i++) {
			/* Provider sends refresh .*/
			createRefreshMessage(refreshMsg, 5, "TRI.N." + i);
			int result = provider.submitAndDispatch(refreshMsg, submitOptions, false);
			assertTrue(result >= ReactorReturnCodes.SUCCESS);
		}
		
		//Only messages that are dispatched before channel is closed are expected to be passed to consumer
		consumerReactor.dispatch(failureCount, false);

		consumer.close();
		provider.close();	
	}
	
	public void SimpleRequestTest_CloseFromCallbackInner(int failureCount)
	{

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
		Consumer consumer = new CloseOnDefaultMsgConsumer(consumerReactor, failureCount, true);
		initConsumerRole(consumer);

		/* Create provider. */
		Provider provider = new Provider(providerReactor);
		initProviderRole(provider);


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
		createRefreshMessage(refreshMsg, 5, "TRI.N");
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
		assertEquals("TRI.N", receivedRefreshMsg.msgKey().name().toString());
		assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
		assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
		assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
		assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());

		//sending messages for force disconnect scenario, starting from 1 as as handler counts every message
		for(int i=1; i<failureCount*2; i++) {
            /* Provider sends refresh .*/
            consumerReactor.pollEvent();
            createRefreshMessage(refreshMsg, 5, "TRI.N." + i);
            int result = provider.submitAndDispatch(refreshMsg, submitOptions, i >= failureCount - 1);
            if (i <= failureCount) {
                assertTrue(result >= ReactorReturnCodes.SUCCESS);
            } else {
                assertFalse(result >= ReactorReturnCodes.SUCCESS);
            }
            consumerReactor.dispatch(1, i >= failureCount - 1);
        }

		//closing message is expected only for ordinary scenario
		if(failureCount<=0) {
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
        }

		//clear messages for unexpected channel close scenario
		if(failureCount>0){
		    providerReactor.pollEvent();
        }
		TestReactorComponent.closeSession(consumer, provider,  failureCount>0);
	}

	private void initConsumerRole(Consumer consumer)
	{
		ConsumerRole consumerRole = (ConsumerRole) consumer.reactorRole();
		consumerRole.initDefaultRDMLoginRequest();
		consumerRole.initDefaultRDMDirectoryRequest();
		consumerRole.channelEventCallback(consumer);
		consumerRole.loginMsgCallback(consumer);
		consumerRole.directoryMsgCallback(consumer);
		consumerRole.dictionaryMsgCallback(consumer);
		consumerRole.defaultMsgCallback(consumer);
		consumerRole.watchlistOptions().channelOpenCallback(consumer);
	}
	
	private void initProviderRole(Provider provider)
	{
		ProviderRole providerRole = (ProviderRole) provider.reactorRole();
		providerRole.channelEventCallback(provider);
		providerRole.loginMsgCallback(provider);
		providerRole.directoryMsgCallback(provider);
		providerRole.dictionaryMsgCallback(provider);
		providerRole.defaultMsgCallback(provider);
	}

	private void createRefreshMessage(RefreshMsg refreshMsg, int streamId, String msgKeyName) {
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(streamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data(msgKeyName);
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
    }

    @Test
    public void VerifyConsumerRoleMsgCopyTest()
    {
        /* Verify message attached to the ConsumerRole are copied. */
        
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
        
        /* Obtain reference to ReactorChannel ConsumerRole and verify that
         * user ConsumerRole and ReactorChannel ConsumerRole are not the same. */
        ConsumerRole reactorChnlConsumerRole = (ConsumerRole)consumer.channel().role();
        assertFalse(consumerRole == reactorChnlConsumerRole);

        /* Verify that user ConsumerRole messages and ReactorChannel
         * ConsumerRole messages are not the same. */
        assertFalse(consumerRole.rdmLoginRequest() == reactorChnlConsumerRole.rdmLoginRequest());
        assertFalse(consumerRole.rdmDirectoryRequest() == reactorChnlConsumerRole.rdmDirectoryRequest());
        
        /* Verify that other user ConsumerRole and ReactorChannel ConsumerRole attributes are equal. */
        assertTrue(consumerRole.channelEventCallback() == reactorChnlConsumerRole.channelEventCallback());
        assertTrue(consumerRole.defaultMsgCallback() == reactorChnlConsumerRole.defaultMsgCallback());
        assertTrue(consumerRole.dictionaryDownloadMode() == reactorChnlConsumerRole.dictionaryDownloadMode());
        assertTrue(consumerRole.dictionaryMsgCallback() == reactorChnlConsumerRole.dictionaryMsgCallback());
        assertTrue(consumerRole.directoryMsgCallback() == reactorChnlConsumerRole.directoryMsgCallback());
        assertTrue(consumerRole.loginMsgCallback() == reactorChnlConsumerRole.loginMsgCallback());
        assertTrue(consumerRole.watchlistOptions().channelOpenCallback() == reactorChnlConsumerRole.watchlistOptions().channelOpenCallback());
        assertTrue(consumerRole.watchlistOptions().enableWatchlist() == reactorChnlConsumerRole.watchlistOptions().enableWatchlist());
        assertTrue(consumerRole.watchlistOptions().itemCountHint() == reactorChnlConsumerRole.watchlistOptions().itemCountHint());
        assertTrue(consumerRole.watchlistOptions().maxOutstandingPosts() == reactorChnlConsumerRole.watchlistOptions().maxOutstandingPosts());
        assertTrue(consumerRole.watchlistOptions().obeyOpenWindow() == reactorChnlConsumerRole.watchlistOptions().obeyOpenWindow());
        assertTrue(consumerRole.watchlistOptions().postAckTimeout() == reactorChnlConsumerRole.watchlistOptions().postAckTimeout());
        assertTrue(consumerRole.watchlistOptions().requestTimeout() == reactorChnlConsumerRole.watchlistOptions().requestTimeout());
       
        /* Obtain reference to ReactorChannel ProviderRole and verify that
         * user ProviderRole and ReactorChannel ProviderRole are not the same. */
        ProviderRole reactorChnlProviderRole = (ProviderRole)provider.channel().role();
        assertFalse(providerRole == reactorChnlProviderRole);

        /* Verify that other user ProviderRole and ReactorChannel ProviderRole attributes are equal. */
        assertTrue(providerRole.channelEventCallback() == reactorChnlProviderRole.channelEventCallback());
        assertTrue(providerRole.defaultMsgCallback() == reactorChnlProviderRole.defaultMsgCallback());
        assertTrue(providerRole.dictionaryMsgCallback() == reactorChnlProviderRole.dictionaryMsgCallback());
        assertTrue(providerRole.directoryMsgCallback() == reactorChnlProviderRole.directoryMsgCallback());
        assertTrue(providerRole.loginMsgCallback() == reactorChnlProviderRole.loginMsgCallback());
        assertTrue(providerRole.tunnelStreamListenerCallback() == reactorChnlProviderRole.tunnelStreamListenerCallback());

        /* Close session. */
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void VerifyNIProviderRoleMsgCopyTest()
    {
        /* Verify message attached to the NIProviderRole are copied. */
        
        /* Create dummy ReactorChannel for test. */
        ReactorChannel reactorChannel = ReactorFactory.createReactorChannel();
        
        /* Create user NIProviderRole, initialize default messages and set on dummy ReactorChannel. */
        NIProviderRole userNIProviderRole = ReactorFactory.createNIProviderRole();
        userNIProviderRole.initDefaultRDMLoginRequest();
        userNIProviderRole.initDefaultRDMDirectoryRefresh("DIRECT_FEED", 1);
        reactorChannel.role(userNIProviderRole);
                
        /* Obtain reference to ReactorChannel NIProviderRole and verify that
         * user NIProviderRole and ReactorChannel NIProviderRole are not the same. */
        NIProviderRole reactorChnlNIProviderRole = (NIProviderRole)reactorChannel.role();
        assertFalse(userNIProviderRole == reactorChnlNIProviderRole);

        /* Verify that user NIProviderRole messages and ReactorChannel
         * NIProviderRole messages are not the same. */
        assertFalse(userNIProviderRole.rdmLoginRequest() == reactorChnlNIProviderRole.rdmLoginRequest());
        assertFalse(userNIProviderRole.rdmDirectoryRefresh() == reactorChnlNIProviderRole.rdmDirectoryRefresh());
        
        /* Verify that other user NIProviderRole and ReactorChannel NIProviderRole attributes are equal. */
        assertTrue(userNIProviderRole.channelEventCallback() == reactorChnlNIProviderRole.channelEventCallback());
        assertTrue(userNIProviderRole.defaultMsgCallback() == reactorChnlNIProviderRole.defaultMsgCallback());
        assertTrue(userNIProviderRole.loginMsgCallback() == reactorChnlNIProviderRole.loginMsgCallback());
   }
}
