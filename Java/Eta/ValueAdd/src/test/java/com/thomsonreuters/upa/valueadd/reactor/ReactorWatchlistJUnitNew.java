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
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static java.lang.Math.abs;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.List;

import org.junit.Test;

import com.thomsonreuters.upa.codec.AckMsg;
import com.thomsonreuters.upa.codec.Array;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CloseMsg;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.ElementEntry;
import com.thomsonreuters.upa.codec.ElementList;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.FilterEntryActions;
import com.thomsonreuters.upa.codec.Int;
import com.thomsonreuters.upa.codec.Map;
import com.thomsonreuters.upa.codec.MapEntry;
import com.thomsonreuters.upa.codec.MapEntryActions;
import com.thomsonreuters.upa.codec.MapEntryFlags;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.NakCodes;
import com.thomsonreuters.upa.codec.PostMsg;
import com.thomsonreuters.upa.codec.QosRates;
import com.thomsonreuters.upa.codec.QosTimeliness;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.RefreshMsgFlags;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RequestMsgFlags;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StatusMsg;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.codec.UInt;
import com.thomsonreuters.upa.codec.GenericMsg;
import com.thomsonreuters.upa.codec.UpdateMsg;
import com.thomsonreuters.upa.rdm.Directory;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.rdm.Login;
import com.thomsonreuters.upa.rdm.SymbolList;
import com.thomsonreuters.upa.rdm.ElementNames;
import com.thomsonreuters.upa.rdm.ViewTypes;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryUpdate;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.Service.ServiceGroup;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequest;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRequestFlags;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginClose;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginRefresh;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginStatus;

public class ReactorWatchlistJUnitNew
{
    @Test
    public void itemMultipartRefreshTimeoutTest()
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        
        // wait for multi-part request timeout
        try
        {
            Thread.sleep(5000);
        } catch (InterruptedException e) { }
        // dispatch for timeout
        consumerReactor.dispatch(1);
        
        /* Provider receives close and re-request. */
        providerReactor.dispatch(2);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

        /* Consumer receives status message for timeout. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        StatusMsg statusMsg = (StatusMsg)msgEvent.msg();
        assertTrue(statusMsg.checkHasState());
        assertTrue(statusMsg.state().text().toString().equals("Request timeout"));
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void directoryUserRequestTest()
    {
        /* Test the user calling the initial directory SubmitRequest with no default directory request */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
                
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new Consumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        consumerRole.watchlistOptions().enableWatchlist(true);
        consumerRole.watchlistOptions().channelOpenCallback(consumer);
        consumerRole.watchlistOptions().requestTimeout(3000);
        
        /* Create provider. */
        Provider provider = new Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        
        /* Connect the consumer and provider. Setup login stream automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        // Consumer submits source directory request
        DirectoryRequest directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
        directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
        directoryRequest.streamId(2);
        directoryRequest.filter(Directory.ServiceFilterFlags.INFO |
                Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP);
        directoryRequest.applyStreaming();
        assertNotNull(directoryRequest);
        assertTrue(consumer.submit(directoryRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
                
        /* Consumer receives directory refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        
        // Check Codec Message
        assertEquals(MsgClasses.REFRESH, directoryMsgEvent.msg().msgClass());
        RefreshMsg refreshMsg = (RefreshMsg)directoryMsgEvent.msg();
        assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
        assertEquals(2, refreshMsg.streamId());
        assertTrue(refreshMsg.checkSolicited());
        assertTrue(refreshMsg.checkRefreshComplete());

        // Check RDM Message
        assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
        DirectoryRefresh directoryRefresh = (DirectoryRefresh)directoryMsgEvent.rdmDirectoryMsg();
        assertEquals(2, directoryRefresh.streamId());
        assertTrue(directoryRefresh.checkSolicited());
        assertTrue(directoryRefresh.serviceList().get(0).checkHasInfo());
        assertTrue(directoryRefresh.serviceList().get(0).checkHasState());
        assertTrue(directoryRefresh.serviceList().get(0).groupStateList().size() == 0); // cached refresh doesn't have group stuff even though user requests it
        
        // reissue same source directory request
        assertTrue(consumer.submit(directoryRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
                
        /* Consumer receives directory refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        
        // Check Codec Message
        assertEquals(MsgClasses.REFRESH, directoryMsgEvent.msg().msgClass());
        refreshMsg = (RefreshMsg)directoryMsgEvent.msg();
        assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
        assertEquals(2, refreshMsg.streamId());
        assertTrue(refreshMsg.checkSolicited());
        assertTrue(refreshMsg.checkRefreshComplete());

        // Check RDM Message
        assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
        directoryRefresh = (DirectoryRefresh)directoryMsgEvent.rdmDirectoryMsg();
        assertEquals(2, directoryRefresh.streamId());
        assertTrue(directoryRefresh.checkSolicited());
        assertTrue(directoryRefresh.serviceList().get(0).checkHasInfo());
        assertTrue(directoryRefresh.serviceList().get(0).checkHasState());
        assertTrue(directoryRefresh.serviceList().get(0).groupStateList().size() == 0); // cached refresh doesn't have group stuff even though user requests it

        // reissue same source directory request with no state filter and extra load filter 
        directoryRequest.filter(Directory.ServiceFilterFlags.INFO |
                                Directory.ServiceFilterFlags.LOAD | Directory.ServiceFilterFlags.GROUP);
        assertTrue(consumer.submit(directoryRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
                
        /* Consumer receives directory refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        
        // Check Codec Message
        assertEquals(MsgClasses.REFRESH, directoryMsgEvent.msg().msgClass());
        refreshMsg = (RefreshMsg)directoryMsgEvent.msg();
        assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
        assertEquals(2, refreshMsg.streamId());
        assertTrue(refreshMsg.checkSolicited());
        assertTrue(refreshMsg.checkRefreshComplete());

        // Check RDM Message
        assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
        directoryRefresh = (DirectoryRefresh)directoryMsgEvent.rdmDirectoryMsg();
        assertEquals(2, directoryRefresh.streamId());
        assertTrue(directoryRefresh.checkSolicited());
        assertTrue(directoryRefresh.serviceList().get(0).checkHasInfo());
        assertFalse(directoryRefresh.serviceList().get(0).checkHasState());
        assertTrue(directoryRefresh.serviceList().get(0).groupStateList().size() == 0); // cached refresh doesn't have group stuff even though user requests it
        assertFalse(directoryRefresh.serviceList().get(0).checkHasLoad()); // cached refresh doesn't have load stuff even though user requests it

        // reissue same source directory request with state filter back on and extra load filter 
        directoryRequest.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE |
                                Directory.ServiceFilterFlags.LOAD | Directory.ServiceFilterFlags.GROUP);
        assertTrue(consumer.submit(directoryRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
                
        /* Consumer receives directory refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        
        // Check Codec Message
        assertEquals(MsgClasses.REFRESH, directoryMsgEvent.msg().msgClass());
        refreshMsg = (RefreshMsg)directoryMsgEvent.msg();
        assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
        assertEquals(2, refreshMsg.streamId());
        assertTrue(refreshMsg.checkSolicited());
        assertTrue(refreshMsg.checkRefreshComplete());

        // Check RDM Message
        assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
        directoryRefresh = (DirectoryRefresh)directoryMsgEvent.rdmDirectoryMsg();
        assertEquals(2, directoryRefresh.streamId());
        assertTrue(directoryRefresh.checkSolicited());
        assertTrue(directoryRefresh.serviceList().get(0).checkHasInfo());
        assertTrue(directoryRefresh.serviceList().get(0).checkHasState());
        assertTrue(directoryRefresh.serviceList().get(0).groupStateList().size() == 0); // cached refresh doesn't have group stuff even though user requests it
        assertFalse(directoryRefresh.serviceList().get(0).checkHasLoad()); // cached refresh doesn't have load stuff even though user requests it

        // Consumer submits source directory request with a different stream id and no info filter
        directoryRequest.streamId(3);
        directoryRequest.filter(Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP);
        assertTrue(consumer.submit(directoryRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
                
        /* Consumer receives directory refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        
        // Check Codec Message
        assertEquals(MsgClasses.REFRESH, directoryMsgEvent.msg().msgClass());
        refreshMsg = (RefreshMsg)directoryMsgEvent.msg();
        assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
        assertEquals(3, refreshMsg.streamId());
        assertTrue(refreshMsg.checkSolicited());
        assertTrue(refreshMsg.checkRefreshComplete());

        // Check RDM Message
        assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
        directoryRefresh = (DirectoryRefresh)directoryMsgEvent.rdmDirectoryMsg();
        assertEquals(3, directoryRefresh.streamId());
        assertTrue(directoryRefresh.checkSolicited());
        assertFalse(directoryRefresh.serviceList().get(0).checkHasInfo());
        assertTrue(directoryRefresh.serviceList().get(0).checkHasState());
        assertTrue(directoryRefresh.serviceList().get(0).groupStateList().size() == 0); // cached refresh doesn't have group stuff even though user requests it
        
        // Consumer submits source directory request with a different stream id and all filters
        directoryRequest.streamId(4);
        directoryRequest.filter(Directory.ServiceFilterFlags.DATA |
                                Directory.ServiceFilterFlags.GROUP |
                                Directory.ServiceFilterFlags.INFO |
                                Directory.ServiceFilterFlags.LINK |
                                Directory.ServiceFilterFlags.LOAD |
                                Directory.ServiceFilterFlags.SEQ_MCAST |
                                Directory.ServiceFilterFlags.STATE);
        assertTrue(consumer.submit(directoryRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
                
        /* Consumer receives directory refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        
        // Check Codec Message
        assertEquals(MsgClasses.REFRESH, directoryMsgEvent.msg().msgClass());
        refreshMsg = (RefreshMsg)directoryMsgEvent.msg();
        assertEquals(DomainTypes.SOURCE, refreshMsg.domainType());
        assertEquals(4, refreshMsg.streamId());
        assertTrue(refreshMsg.checkSolicited());
        assertTrue(refreshMsg.checkRefreshComplete());

        // Check RDM Message
        assertEquals(DirectoryMsgType.REFRESH, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
        directoryRefresh = (DirectoryRefresh)directoryMsgEvent.rdmDirectoryMsg();
        assertEquals(4, directoryRefresh.streamId());
        assertTrue(directoryRefresh.checkSolicited());
        assertTrue(directoryRefresh.serviceList().get(0).checkHasInfo());
        assertTrue(directoryRefresh.serviceList().get(0).checkHasState());
        assertTrue(directoryRefresh.serviceList().get(0).groupStateList().size() == 0); // cached refresh doesn't have group stuff even though user requests it
        assertFalse(directoryRefresh.serviceList().get(0).checkHasData());
        assertFalse(directoryRefresh.serviceList().get(0).checkHasLink());
        assertFalse(directoryRefresh.serviceList().get(0).checkHasLoad());
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void itemServiceUpdatedTest()
    {
        /* Test a simple request/refresh exchange with the watchlist enabled. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RDMDirectoryMsgEvent directoryMsgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        refreshMsg.applyRefreshComplete();
        Buffer groupId = CodecFactory.createBuffer();
        groupId.data("1234431");
        refreshMsg.groupId(groupId);
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
        
        /* Provider sends service update .*/
        DirectoryUpdate directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE);
        
        WlService wlService = new WlService();
        wlService.rdmService().applyHasState();
        wlService.rdmService().action(MapEntryActions.UPDATE);
        wlService.rdmService().state().applyHasStatus();
        wlService.rdmService().state().status().dataState(2);
        wlService.rdmService().state().status().streamState(12);
        wlService.rdmService().serviceId(1);
        
        directoryUpdateMsg.serviceList().add(wlService.rdmService());

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        consumerReactor.dispatch(2);

        /* Consumer receives status with StreamState 12 and dataState 2. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(12, receivedStatusMsg.state().streamState());
        assertEquals(2, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        /* Consumer receives update. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        DirectoryUpdate receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        assertTrue(receivedUpdateMsg.serviceList().size() == 1);
        assertTrue(receivedUpdateMsg.serviceList().get(0).checkHasState());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasInfo());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasData());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLink());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLoad());
        assertTrue(receivedUpdateMsg.serviceList().get(0).groupStateList().size() == 0);

        /* Stream should be considered closed. */
        assertEquals(0, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).streamList().size());
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void itemServiceUpDownMultipleItemsTest()
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
        UpdateMsg receivedUpdateMsg;
        StatusMsg receivedStatusMsg;
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        refreshMsg.applyRefreshComplete();
        Buffer groupId = CodecFactory.createBuffer();
        groupId.data("1234431");
        refreshMsg.groupId(groupId);
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
        
        /* Consumer sends request. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("IBM.N");
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
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
        refreshMsg.msgKey().name().data("IBM.N");
        refreshMsg.applyRefreshComplete();
        groupId.data("1234431");
        refreshMsg.groupId(groupId);
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
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("IBM.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

        /* Provider sends service update .*/
        DirectoryUpdate directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.STATE);
        
        WlService wlService = new WlService();
        wlService.rdmService().applyHasState();
        wlService.rdmService().action(MapEntryActions.UPDATE);
        wlService.rdmService().state().applyHasStatus();
        wlService.rdmService().state().action(FilterEntryActions.SET);
        wlService.rdmService().state().status().dataState(DataStates.SUSPECT);
        wlService.rdmService().state().status().streamState(StreamStates.CLOSED_RECOVER);
        wlService.rdmService().state().applyHasAcceptingRequests();
        wlService.rdmService().state().acceptingRequests(1);
        wlService.rdmService().state().serviceState(0);
        wlService.rdmService().serviceId(1);
        
        directoryUpdateMsg.serviceList().add(wlService.rdmService());

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives group status and item status for the items (the second pair of item status is from the recovery attempt). */
        consumerReactor.dispatch(5);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(6, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertTrue(receivedUpdateMsg.checkHasMsgKey());
        assertEquals(DomainTypes.SOURCE, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.MAP, receivedUpdateMsg.containerType());
        
        assertEquals(0, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).streamList().size());

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(6, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        // empty event queue
        event = consumerReactor.pollEvent();
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void itemGroupUpdatedTest()
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
        StatusMsg receivedStatusMsg;
        RDMDirectoryMsgEvent directoryMsgEvent;
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        refreshMsg.applyRefreshComplete();
        Buffer groupId = CodecFactory.createBuffer();
        groupId.data("1234431");
        refreshMsg.groupId(groupId);
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
        
        /* Provider sends group update .*/
        DirectoryUpdate directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE);
        
        WlService wlService = new WlService();
        wlService.rdmService().applyHasState();
        wlService.rdmService().action(MapEntryActions.UPDATE);
        wlService.rdmService().serviceId(1);
        ServiceGroup serviceGroup = new ServiceGroup();
        serviceGroup.applyHasStatus();
        serviceGroup.status().dataState(DataStates.SUSPECT);
        serviceGroup.status().streamState(StreamStates.OPEN);
        serviceGroup.group(groupId);
        wlService.rdmService().groupStateList().add(serviceGroup);
        
        directoryUpdateMsg.serviceList().add(wlService.rdmService());

        UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
        updateMsg.clear();

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        consumerReactor.dispatch(2);

        /* Consumer receives Open/Suspect item status. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        /* Consumer receives update. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        DirectoryUpdate receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        assertTrue(receivedUpdateMsg.serviceList().size() == 1);
        assertTrue(receivedUpdateMsg.serviceList().get(0).checkHasState());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasInfo());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasData());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLink());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLoad());
        assertTrue(receivedUpdateMsg.serviceList().get(0).groupStateList().size() == 1);

        assertEquals(DataStates.SUSPECT, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).itemGroupTableGet(groupId).openStreamList().get(0).state().dataState());
        assertEquals(StreamStates.OPEN, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).itemGroupTableGet(groupId).openStreamList().get(0).state().streamState());
    
        /* Provider sends group update with mergeToGroup.*/
        directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE);
        
        wlService = new WlService();
        wlService.rdmService().applyHasState();
        wlService.rdmService().action(MapEntryActions.UPDATE);
        wlService.rdmService().serviceId(1);
        serviceGroup = new ServiceGroup();
        serviceGroup.applyHasStatus();
        serviceGroup.status().dataState(DataStates.OK);
        serviceGroup.status().streamState(StreamStates.OPEN);
        serviceGroup.group(groupId);
        Buffer groupToMerge = CodecFactory.createBuffer();
        groupToMerge.data("43211234");
        serviceGroup.applyHasMergedToGroup();
        serviceGroup.mergedToGroup(groupToMerge);
        wlService.rdmService().groupStateList().clear();
        wlService.rdmService().groupStateList().add(serviceGroup);
        
        directoryUpdateMsg.serviceList().add(wlService.rdmService());

        updateMsg = (UpdateMsg)CodecFactory.createMsg();
        updateMsg.clear();

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        consumerReactor.dispatch(2);

        /* Consumer receives Open/Ok item status. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.OK, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        /* Consumer receives update. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        assertTrue(receivedUpdateMsg.serviceList().size() == 1);
        assertTrue(receivedUpdateMsg.serviceList().get(0).checkHasState());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasInfo());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasData());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLink());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLoad());
        assertTrue(receivedUpdateMsg.serviceList().get(0).groupStateList().size() == 1);

        assertEquals(DataStates.OK, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).itemGroupTableGet(groupToMerge).openStreamList().get(0).state().dataState());
        assertEquals(StreamStates.OPEN, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).itemGroupTableGet(groupToMerge).openStreamList().get(0).state().streamState());

        /* Provider sends group update on a group we don't have, shouldn't make any changes.*/
        directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE);
        
        wlService = new WlService();
        wlService.rdmService().applyHasState();
        wlService.rdmService().action(MapEntryActions.UPDATE);
        wlService.rdmService().serviceId(1);
        serviceGroup = new ServiceGroup();
        serviceGroup.applyHasStatus();
        serviceGroup.status().dataState(4);
        serviceGroup.status().streamState(14);
        Buffer badBuffer = CodecFactory.createBuffer();
        badBuffer.data("This Isn't correct, clearly");
        serviceGroup.group(badBuffer);
        wlService.rdmService().groupStateList().clear();
        wlService.rdmService().groupStateList().add(serviceGroup);
        
        directoryUpdateMsg.serviceList().add(wlService.rdmService());

        updateMsg = (UpdateMsg)CodecFactory.createMsg();
        updateMsg.clear();

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives update. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        assertTrue(receivedUpdateMsg.serviceList().size() == 1);
        assertTrue(receivedUpdateMsg.serviceList().get(0).checkHasState());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasInfo());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasData());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLink());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLoad());
        assertTrue(receivedUpdateMsg.serviceList().get(0).groupStateList().size() == 1);

        /* Current item group state should be unchanged. */
        assertEquals(DataStates.OK, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).itemGroupTableGet(groupToMerge).openStreamList().get(0).state().dataState());
        assertEquals(StreamStates.OPEN, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).itemGroupTableGet(groupToMerge).openStreamList().get(0).state().streamState());

        /* Provider sends group update on a groupToMerge we don't have, shouldn't make any changes.*/
        directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE);
        
        wlService = new WlService();
        wlService.rdmService().applyHasState();
        wlService.rdmService().action(MapEntryActions.UPDATE);
        wlService.rdmService().serviceId(1);
        serviceGroup = new ServiceGroup();
        serviceGroup.applyHasStatus();
        serviceGroup.status().dataState(5);
        serviceGroup.status().streamState(15);
        serviceGroup.group(badBuffer);
        Buffer groupToMergeTwo = CodecFactory.createBuffer();
        groupToMergeTwo.data("12345");
        serviceGroup.applyHasMergedToGroup();
        serviceGroup.mergedToGroup(groupToMergeTwo);
        wlService.rdmService().groupStateList().clear();
        wlService.rdmService().groupStateList().add(serviceGroup);
        
        directoryUpdateMsg.serviceList().add(wlService.rdmService());

        updateMsg = (UpdateMsg)CodecFactory.createMsg();
        updateMsg.clear();

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives update. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        assertTrue(receivedUpdateMsg.serviceList().size() == 1);
        assertTrue(receivedUpdateMsg.serviceList().get(0).checkHasState());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasInfo());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasData());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLink());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLoad());
        assertTrue(receivedUpdateMsg.serviceList().get(0).groupStateList().size() == 1);

        /* Current item group state should be unchanged. */
        assertEquals(DataStates.OK, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).itemGroupTableGet(groupToMerge).openStreamList().get(0).state().dataState());
        assertEquals(StreamStates.OPEN, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).itemGroupTableGet(groupToMerge).openStreamList().get(0).state().streamState());

        /* Provider sends group update to groupToMerge of the same group Id */
        directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE);
        
        wlService = new WlService();
        wlService.rdmService().applyHasState();
        wlService.rdmService().action(MapEntryActions.UPDATE);
        wlService.rdmService().serviceId(1);
        serviceGroup = new ServiceGroup();
        serviceGroup.applyHasStatus();
        serviceGroup.status().dataState(DataStates.SUSPECT);
        serviceGroup.status().streamState(StreamStates.OPEN);
        serviceGroup.group(groupToMerge);
        serviceGroup.applyHasMergedToGroup();
        serviceGroup.mergedToGroup(groupToMerge);
        wlService.rdmService().groupStateList().clear();
        wlService.rdmService().groupStateList().add(serviceGroup);
        
        directoryUpdateMsg.serviceList().add(wlService.rdmService());

        updateMsg = (UpdateMsg)CodecFactory.createMsg();
        updateMsg.clear();

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        consumerReactor.dispatch(2);

        /* Consumer receives Open/Suspect item status. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        /* Consumer receives update. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        assertTrue(receivedUpdateMsg.serviceList().size() == 1);
        assertTrue(receivedUpdateMsg.serviceList().get(0).checkHasState());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasInfo());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasData());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLink());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLoad());
        assertTrue(receivedUpdateMsg.serviceList().get(0).groupStateList().size() == 1);

        assertEquals(DataStates.SUSPECT, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).itemGroupTableGet(groupToMerge).openStreamList().get(0).state().dataState());
        assertEquals(StreamStates.OPEN, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).itemGroupTableGet(groupToMerge).openStreamList().get(0).state().streamState());
        
        /* Provider sends group update that also has status update. Group update takes precedence as it happens second */
        directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE);
        
        wlService = new WlService();
        wlService.rdmService().state().status().dataState(3);
        wlService.rdmService().state().status().streamState(13);
        wlService.rdmService().applyHasState();
        wlService.rdmService().state().applyHasStatus();
        wlService.rdmService().action(MapEntryActions.UPDATE);
        wlService.rdmService().serviceId(1);
        serviceGroup = new ServiceGroup();
        serviceGroup.applyHasStatus();
        serviceGroup.status().dataState(4);
        serviceGroup.status().streamState(14);
        serviceGroup.group(groupToMerge);
        wlService.rdmService().groupStateList().clear();
        wlService.rdmService().groupStateList().add(serviceGroup);
        
        directoryUpdateMsg.serviceList().add(wlService.rdmService());

        updateMsg = (UpdateMsg)CodecFactory.createMsg();
        updateMsg.clear();

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives update. */
        consumerReactor.dispatch(2);

        /* Consumer receives status with StreamState 14 and dataState 4. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(13, receivedStatusMsg.state().streamState());
        assertEquals(3, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

        /* Consumer receives update. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        assertTrue(receivedUpdateMsg.serviceList().size() == 1);
        assertTrue(receivedUpdateMsg.serviceList().get(0).checkHasState());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasInfo());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasData());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLink());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLoad());
        assertTrue(receivedUpdateMsg.serviceList().get(0).groupStateList().size() == 1);

        /* Stream should be considered closed. */
        assertEquals(0, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).streamList().size());
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    /* Used by privateStreamOpenCallbackSubmitTest and privateStreamOpenCallbackSubmitReSubmitTest. */
    class SendItemsFromOpenCallbackConsumer extends Consumer
    {
        boolean _privateStream;
        
        public SendItemsFromOpenCallbackConsumer(TestReactor testReactor, boolean privateStream)
        {
            super(testReactor);
            
            _privateStream = privateStream;
        }

        @Override
        public int reactorChannelEventCallback(ReactorChannelEvent event)
        {
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            Msg msg = CodecFactory.createMsg();
            RequestMsg requestMsg = (RequestMsg)msg;
            
            if (event.eventType() == ReactorChannelEventTypes.CHANNEL_OPENED)
            {
                super.reactorChannelEventCallback(event);
                
                /* Consumer sends private stream request. */
                requestMsg.clear();
                requestMsg.msgClass(MsgClasses.REQUEST);
                requestMsg.streamId(5);
                requestMsg.domainType(DomainTypes.MARKET_PRICE);
                requestMsg.applyStreaming();
                requestMsg.msgKey().applyHasName();
                requestMsg.msgKey().name().data("TRI.N");
                if (_privateStream)
                {
                    requestMsg.applyPrivateStream();
                }
                submitOptions.clear();
                submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
                assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            }
            else
            {
                return super.reactorChannelEventCallback(event);
            }
            
            return ReactorReturnCodes.SUCCESS;
        }
    }


    @Test
    public void privateStreamOpenCallbackSubmitTest()
    {
        TestReactorEvent event;
        ReactorChannelEvent chnlEvent;
        ReactorMsgEvent msgEvent;
        StatusMsg receivedStatusMsg;
                
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new SendItemsFromOpenCallbackConsumer(consumerReactor, true);
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
        
        // connect consumer
        consumer.testReactor().connect(opts, consumer, provider.serverPort());
        
        // Consumer receives CHANNEL_OPENED event
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        chnlEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_OPENED, chnlEvent.eventType());
        
        // Consumer receives "Closed, Recoverable/Suspect" StatusMsg from request submitted in channel open callback
        consumer.testReactor().dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertTrue(receivedStatusMsg.checkHasState());
        assertEquals(StreamStates.CLOSED_RECOVER, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    
    @Test
    public void fanoutTest()
    {

    	ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
    	TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        GenericMsg genericMsg = (GenericMsg)msg;
        GenericMsg receivedGenericMsg;
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        DirectoryRequest _directoryRequest = (DirectoryRequest)DirectoryMsgFactory.createMsg();
        DirectoryRefresh directoryRefresh = (DirectoryRefresh)DirectoryMsgFactory.createMsg();
       
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
        consumerRole.watchlistOptions().requestTimeout(3000);
       
        
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
        
        /* Consumer sends second request. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Provider receives request. */
        providerReactor.dispatch(2);
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
        refreshMsg.applyRefreshComplete();
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.applySolicited();
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives first refresh. */
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
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        /* Consumer receives second refresh on other stream. */
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
        
        /* Provider sends generic msg .*/
        genericMsg.clear();
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.MARKET_PRICE);
        genericMsg.streamId(providerStreamId);
        genericMsg.containerType(DataTypes.NO_DATA);
        genericMsg.applyHasMsgKey();
        genericMsg.msgKey().applyHasServiceId();
        genericMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        genericMsg.msgKey().applyHasName();
        genericMsg.msgKey().name().data("TRI.N");
        
        assertTrue(provider.submitAndDispatch(genericMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives generic msg. */
        consumerReactor.dispatch(2);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
        
        
        receivedGenericMsg = (GenericMsg)msgEvent.msg();
        assertTrue(receivedGenericMsg.checkHasMsgKey());
        assertTrue(receivedGenericMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedGenericMsg.msgKey().checkHasName());
        assertTrue(receivedGenericMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedGenericMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedGenericMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        /* Consume receives second generic msg on other stream */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
        
        
        receivedGenericMsg = (GenericMsg)msgEvent.msg();
        assertTrue(receivedGenericMsg.checkHasMsgKey());
        assertTrue(receivedGenericMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedGenericMsg.msgKey().checkHasName());
        assertTrue(receivedGenericMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedGenericMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedGenericMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        /* Consumer sends second directory request */
        _directoryRequest.clear();
        _directoryRequest.rdmMsgType(DirectoryMsgType.REQUEST);
        _directoryRequest.streamId(10);
        _directoryRequest.filter(Directory.ServiceFilterFlags.INFO |
                Directory.ServiceFilterFlags.STATE | Directory.ServiceFilterFlags.GROUP);
        _directoryRequest.applyStreaming();
        submitOptions.clear();
        assertTrue(consumer.submit(_directoryRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives second directory refresh on second stream */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        assertEquals(10, msgEvent.msg().streamId());
        
        directoryRefresh.rdmMsgType(DirectoryMsgType.REFRESH);

        /* Provider sends generic msg on source directory stream.*/
        genericMsg.clear();
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.SOURCE);
        genericMsg.streamId(provider.defaultSessionDirectoryStreamId());
        genericMsg.containerType(DataTypes.NO_DATA);
        genericMsg.applyHasMsgKey();
        genericMsg.msgKey().applyHasServiceId();
        genericMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        genericMsg.msgKey().applyHasName();
        genericMsg.msgKey().name().data("More Source");
        
        assertTrue(provider.submitAndDispatch(genericMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives generic msg on source directory. */
        consumerReactor.dispatch(2);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
        
        
        receivedGenericMsg = (GenericMsg)msgEvent.msg();
        assertTrue(receivedGenericMsg.checkHasMsgKey());
        assertTrue(receivedGenericMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedGenericMsg.msgKey().checkHasName());
        assertTrue(receivedGenericMsg.msgKey().name().toString().equals("More Source"));
        assertEquals(DomainTypes.SOURCE, receivedGenericMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedGenericMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        
        /* Consume receives second generic msg on other source directory stream */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
        
        
        receivedGenericMsg = (GenericMsg)msgEvent.msg();
        assertTrue(receivedGenericMsg.checkHasMsgKey());
        assertTrue(receivedGenericMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedGenericMsg.msgKey().checkHasName());
        assertTrue(receivedGenericMsg.msgKey().name().toString().equals("More Source"));
        assertEquals(DomainTypes.SOURCE, receivedGenericMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedGenericMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        
        /* Provider sends generic msg on login stream */
        genericMsg.clear();
        genericMsg.msgClass(MsgClasses.GENERIC);
        genericMsg.domainType(DomainTypes.LOGIN);
        genericMsg.streamId(provider.defaultSessionLoginStreamId());
        genericMsg.containerType(DataTypes.NO_DATA);
        genericMsg.applyHasMsgKey();
        genericMsg.msgKey().applyHasServiceId();
        genericMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        genericMsg.msgKey().applyHasName();
        genericMsg.msgKey().name().data("More User");
        
        assertTrue(provider.submitAndDispatch(genericMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives generic msg on login stream */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());
        
        
        receivedGenericMsg = (GenericMsg)msgEvent.msg();
        assertTrue(receivedGenericMsg.checkHasMsgKey());
        assertTrue(receivedGenericMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedGenericMsg.msgKey().checkHasName());
        assertTrue(receivedGenericMsg.msgKey().name().toString().equals("More User"));
        assertEquals(DomainTypes.LOGIN, receivedGenericMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedGenericMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        
        TestReactorComponent.closeSession(consumer, provider);
    }

    @Test
    public void emptyStatusMsgTest()
    {

    	ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
    	TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        StatusMsg statusMsg = (StatusMsg)msg;
        StatusMsg receivedStatusMsg;
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        UpdateMsg updateMsg = (UpdateMsg)msg;
        UpdateMsg receivedUpdateMsg;        
       
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
        consumerRole.watchlistOptions().requestTimeout(3000);
       
        
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
        refreshMsg.applyRefreshComplete();
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.applySolicited();
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives first refresh. */
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
               
        /* Provider sends status msg .*/
        statusMsg.clear();
        statusMsg.msgClass(MsgClasses.STATUS);
        statusMsg.domainType(DomainTypes.MARKET_PRICE);
        statusMsg.streamId(providerStreamId);
        statusMsg.containerType(DataTypes.NO_DATA);
        statusMsg.applyHasMsgKey();
        statusMsg.msgKey().applyHasServiceId();
        statusMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        statusMsg.msgKey().applyHasName();
        statusMsg.msgKey().name().data("TRI.N");
        
        assertTrue(provider.submitAndDispatch(statusMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Provider sends an update. */
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(providerStreamId);
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.containerType(DataTypes.NO_DATA);
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
	/* Consumer receives status msg. */
	consumerReactor.dispatch(2);
	event = consumerReactor.pollEvent();
	assertEquals(TestReactorEventTypes.MSG, event.type());
	msgEvent = (ReactorMsgEvent) event.reactorEvent();
	assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
	receivedStatusMsg = (StatusMsg) msgEvent.msg();
    assertEquals(5, receivedStatusMsg.streamId());
	assertTrue(receivedStatusMsg.checkHasMsgKey());
	assertTrue(receivedStatusMsg.msgKey().checkHasServiceId());
	assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
	assertTrue(receivedStatusMsg.msgKey().checkHasName());
	assertTrue(receivedStatusMsg.msgKey().name().toString().equals("TRI.N"));
	assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
	assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
	assertFalse(receivedStatusMsg.checkHasState());
	assertNotNull(msgEvent.streamInfo());
    assertNotNull(msgEvent.streamInfo().serviceName());
    assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));      
        
        /* Consumer receives update */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(5, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));        
        
	TestReactorComponent.closeSession(consumer, provider);
    }

    @Test
    public void privateStreamOpenCallbackSubmitReSubmitTest()
    {
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
        Consumer consumer = new SendItemsFromOpenCallbackConsumer(consumerReactor, true);
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
        opts.numStatusEvents(1); // set number of expected status message from request submitted in channel open callback
        TestReactor.openSession(consumer, provider, opts);
        
        // resubmit private stream request message
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        requestMsg.applyPrivateStream();
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
        assertTrue(receivedRequestMsg.checkPrivateStream());
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
        refreshMsg.applyPrivateStream();
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
        assertTrue(receivedRefreshMsg.checkPrivateStream());
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

    @Test
    public void privateStreamSubmitTest()
    {
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
        
        // submit private stream request message
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        requestMsg.applyPrivateStream();
        requestMsg.applyHasExtendedHdr();
        Buffer extendedHdrBuffer = CodecFactory.createBuffer();
        extendedHdrBuffer.data("EXTENDED HEADER");
        requestMsg.extendedHeader(extendedHdrBuffer);
        requestMsg.containerType(DataTypes.OPAQUE);
        Buffer encodeDataBodyBuffer = CodecFactory.createBuffer();
        encodeDataBodyBuffer.data("ENCODED DATA BODY");
        requestMsg.encodedDataBody(encodeDataBodyBuffer);
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
        assertTrue(receivedRequestMsg.checkPrivateStream());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        assertTrue(receivedRequestMsg.checkHasExtendedHdr());
        assertTrue(receivedRequestMsg.extendedHeader().toString().equals("EXTENDED HEADER"));
        assertEquals(DataTypes.OPAQUE, receivedRequestMsg.containerType());
        assertTrue(receivedRequestMsg.encodedDataBody().toString().equals("ENCODED DATA BODY"));
        
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyPrivateStream();
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
        assertTrue(receivedRefreshMsg.checkPrivateStream());
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

    @Test
    public void privateStreamSubmitReissueTest()
    {
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
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
        
        // submit private stream request message
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        requestMsg.applyPrivateStream();
        requestMsg.applyHasExtendedHdr();
        Buffer extendedHdrBuffer = CodecFactory.createBuffer();
        extendedHdrBuffer.data("EXTENDED HEADER");
        requestMsg.extendedHeader(extendedHdrBuffer);
        requestMsg.containerType(DataTypes.OPAQUE);
        Buffer encodeDataBodyBuffer = CodecFactory.createBuffer();
        encodeDataBodyBuffer.data("ENCODED DATA BODY");
        requestMsg.encodedDataBody(encodeDataBodyBuffer);
        requestMsg.applyHasPriority();
        requestMsg.priority().count(11);
        requestMsg.priority().priorityClass(22);
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
        assertEquals(3, receivedRequestMsg.streamId());
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertTrue(receivedRequestMsg.checkPrivateStream());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        assertTrue(receivedRequestMsg.checkHasExtendedHdr());
        assertTrue(receivedRequestMsg.extendedHeader().toString().equals("EXTENDED HEADER"));
        assertEquals(DataTypes.OPAQUE, receivedRequestMsg.containerType());
        assertTrue(receivedRequestMsg.encodedDataBody().toString().equals("ENCODED DATA BODY"));
        assertTrue(receivedRequestMsg.checkHasPriority());
        assertEquals(11, receivedRequestMsg.priority().count());
        assertEquals(22, receivedRequestMsg.priority().priorityClass());
        
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyPrivateStream();
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
        assertTrue(receivedRefreshMsg.checkPrivateStream());
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
        
        // consumer reissues original request with different priority
        requestMsg.priority().count(5);
        requestMsg.priority().priorityClass(6);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Provider receives request. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(3, receivedRequestMsg.streamId()); // stream id should be same as first request
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertTrue(receivedRequestMsg.checkPrivateStream());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        assertTrue(receivedRequestMsg.checkHasExtendedHdr());
        assertTrue(receivedRequestMsg.extendedHeader().toString().equals("EXTENDED HEADER"));
        assertEquals(DataTypes.OPAQUE, receivedRequestMsg.containerType());
        assertTrue(receivedRequestMsg.encodedDataBody().toString().equals("ENCODED DATA BODY"));
        assertTrue(receivedRequestMsg.checkHasPriority());
        assertEquals(5, receivedRequestMsg.priority().count());
        assertEquals(6, receivedRequestMsg.priority().priorityClass());
        
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyPrivateStream();
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
        assertTrue(receivedRefreshMsg.checkPrivateStream());
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
    
    @Test
    public void privateStreamAggregationTest()
    {
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
                
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
        
        // submit private stream request message twice
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        requestMsg.applyPrivateStream();
        requestMsg.applyHasExtendedHdr();
        Buffer extendedHdrBuffer = CodecFactory.createBuffer();
        extendedHdrBuffer.data("EXTENDED HEADER");
        requestMsg.extendedHeader(extendedHdrBuffer);
        requestMsg.containerType(DataTypes.OPAQUE);
        Buffer encodeDataBodyBuffer = CodecFactory.createBuffer();
        encodeDataBodyBuffer.data("ENCODED DATA BODY");
        requestMsg.encodedDataBody(encodeDataBodyBuffer);
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        requestMsg.streamId(6);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Provider receives two requests since requests aren't aggregated. */
        providerReactor.dispatch(2);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(3, receivedRequestMsg.streamId());
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertTrue(receivedRequestMsg.checkPrivateStream());
        assertFalse(receivedRequestMsg.checkHasPriority());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        assertTrue(receivedRequestMsg.checkHasExtendedHdr());
        assertTrue(receivedRequestMsg.extendedHeader().toString().equals("EXTENDED HEADER"));
        assertEquals(DataTypes.OPAQUE, receivedRequestMsg.containerType());
        assertTrue(receivedRequestMsg.encodedDataBody().toString().equals("ENCODED DATA BODY"));
        
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(4, receivedRequestMsg.streamId()); // stream id should be different from first request
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertTrue(receivedRequestMsg.checkPrivateStream());
        assertFalse(receivedRequestMsg.checkHasPriority());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        assertTrue(receivedRequestMsg.checkHasExtendedHdr());
        assertTrue(receivedRequestMsg.extendedHeader().toString().equals("EXTENDED HEADER"));
        assertEquals(DataTypes.OPAQUE, receivedRequestMsg.containerType());
        assertTrue(receivedRequestMsg.encodedDataBody().toString().equals("ENCODED DATA BODY"));
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void privateStreamNonPrivateStreamAggregationTest()
    {
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
                
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
        
        // submit non private stream request first then private stream request second
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        requestMsg.applyHasExtendedHdr();
        Buffer extendedHdrBuffer = CodecFactory.createBuffer();
        extendedHdrBuffer.data("EXTENDED HEADER");
        requestMsg.extendedHeader(extendedHdrBuffer);
        requestMsg.containerType(DataTypes.OPAQUE);
        Buffer encodeDataBodyBuffer = CodecFactory.createBuffer();
        encodeDataBodyBuffer.data("ENCODED DATA BODY");
        requestMsg.encodedDataBody(encodeDataBodyBuffer);
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        requestMsg.applyPrivateStream();
        requestMsg.streamId(6);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Provider receives two requests since requests aren't aggregated. */
        providerReactor.dispatch(2);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(3, receivedRequestMsg.streamId());
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertTrue(receivedRequestMsg.checkHasPriority()); // non private stream request should have priority
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        assertFalse(receivedRequestMsg.checkHasExtendedHdr()); // non private stream request should not have extended header
        assertEquals(DataTypes.NO_DATA, receivedRequestMsg.containerType()); // non private stream request should not have data body
        
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(4, receivedRequestMsg.streamId()); // stream id should be different from first request
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertTrue(receivedRequestMsg.checkPrivateStream());
        assertFalse(receivedRequestMsg.checkHasPriority()); // private stream request should not have priority
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        assertTrue(receivedRequestMsg.checkHasExtendedHdr()); // private stream request should have extended header
        assertTrue(receivedRequestMsg.extendedHeader().toString().equals("EXTENDED HEADER"));
        assertEquals(DataTypes.OPAQUE, receivedRequestMsg.containerType()); // private stream request should have data body
        assertTrue(receivedRequestMsg.encodedDataBody().toString().equals("ENCODED DATA BODY"));
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void privateStreamRecoveryTest()
    {
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
        StatusMsg receivedStatusMsg;
        ReactorChannelEvent chnlEvent;
                
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
        opts.reconnectAttemptLimit(1);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        // submit private stream request message
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        requestMsg.applyPrivateStream();
        requestMsg.applyHasExtendedHdr();
        Buffer extendedHdrBuffer = CodecFactory.createBuffer();
        extendedHdrBuffer.data("EXTENDED HEADER");
        requestMsg.extendedHeader(extendedHdrBuffer);
        requestMsg.containerType(DataTypes.OPAQUE);
        Buffer encodeDataBodyBuffer = CodecFactory.createBuffer();
        encodeDataBodyBuffer.data("ENCODED DATA BODY");
        requestMsg.encodedDataBody(encodeDataBodyBuffer);
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
        assertTrue(receivedRequestMsg.checkPrivateStream());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        assertTrue(receivedRequestMsg.checkHasExtendedHdr());
        assertTrue(receivedRequestMsg.extendedHeader().toString().equals("EXTENDED HEADER"));
        assertEquals(DataTypes.OPAQUE, receivedRequestMsg.containerType());
        assertTrue(receivedRequestMsg.encodedDataBody().toString().equals("ENCODED DATA BODY"));
        
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyPrivateStream();
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
        assertTrue(receivedRefreshMsg.checkPrivateStream());
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
        
        // force recovery
        provider.closeChannel();
        
        // dispatch all events (login, directory, directory, msg, channel)
        consumer.testReactor().dispatch(4);
        
        // ignore first 3 events which are chanel down and login/directory recovery messages
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        chnlEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, chnlEvent.eventType());
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(DomainTypes.LOGIN, msgEvent.msg().domainType());
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(DomainTypes.SOURCE, msgEvent.msg().domainType());
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        
        // Consumer receives "Closed, Recoverable/Suspect" StatusMsg and CHANNEL_DOWN_RECONNECTING event 
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertTrue(receivedStatusMsg.checkHasState());
        assertEquals(StreamStates.CLOSED_RECOVER, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void batchRequestTest()
    {
        /* Test a simple batch request/refresh exchange with the watchlist enabled. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        msg.containerType(DataTypes.ELEMENT_LIST);
        requestMsg.applyStreaming();
        requestMsg.applyHasBatch();

        encodeBatch(consumer.reactorChannel(), requestMsg);
        
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        // Received status message with closed batch stream
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        assertEquals("DEFAULT_SERVICE", msgEvent.streamInfo()._serviceName);
        
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.CLOSED, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.OK, receivedStatusMsg.state().dataState());
        assertEquals("Stream closed for batch", receivedStatusMsg.state().text().toString());
        
        /* Provider receives request. */
        providerReactor.dispatch(2);
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
        
        assertTrue(provider.submit(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refresh. */
        consumerReactor.dispatch(1);
        
        // Received Refresh for TRI.N
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message with item TRI.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("TRI.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        
        // Provider processes request for IBM.N
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
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
        refreshMsg.msgKey().name().data("IBM.N");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        // Consumer receives refresh for IBM.N
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message with item IBM.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("IBM.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());

        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void batchRequestNormalRequestBeforeTest()
    {
        /* Test a simple batch request/refresh exchange with the watchlist enabled. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        refreshMsg.applyRefreshComplete();
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
        
        /* Consumer sends request. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        msg.containerType(DataTypes.ELEMENT_LIST);
        requestMsg.applyStreaming();
        requestMsg.applyHasBatch();

        encodeBatch(consumer.reactorChannel(), requestMsg);
        
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        // Received status message with closed batch stream
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        assertEquals("DEFAULT_SERVICE", msgEvent.streamInfo()._serviceName);
        
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.CLOSED, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.OK, receivedStatusMsg.state().dataState());
        assertEquals("Stream closed for batch", receivedStatusMsg.state().text().toString());
        
        /* Provider receives request. */
        providerReactor.dispatch(2);
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
        
        assertTrue(provider.submit(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refresh. */
        consumerReactor.dispatch(2);
        
        // Received Refresh for TRI.N
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message with item TRI.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("TRI.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        
        // Received Refresh for TRI.N
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message with item TRI.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("TRI.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        
        // Provider processes request for IBM.N
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
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
        refreshMsg.msgKey().name().data("IBM.N");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        // Consumer receives refresh for IBM.N
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message with item IBM.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("IBM.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());

        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void batchRequestNormalRequestAfterTest()
    {
        /* Test a simple batch request/refresh exchange with the watchlist enabled. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        msg.containerType(DataTypes.ELEMENT_LIST);
        requestMsg.applyStreaming();
        requestMsg.applyHasBatch();

        encodeBatch(consumer.reactorChannel(), requestMsg);
        
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        // Received status message with closed batch stream
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        assertEquals("DEFAULT_SERVICE", msgEvent.streamInfo()._serviceName);
        
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.CLOSED, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.OK, receivedStatusMsg.state().dataState());
        assertEquals("Stream closed for batch", receivedStatusMsg.state().text().toString());
        
        /* Provider receives request. */
        providerReactor.dispatch(2);
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
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("TRI.N");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submit(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refresh. */
        consumerReactor.dispatch(1);
        
        // Received Refresh for TRI.N
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message with item TRI.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("TRI.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());

        // Provider processes request for IBM.N
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
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
        refreshMsg.msgKey().name().data("IBM.N");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        // Consumer receives refresh for IBM.N
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message with item IBM.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("IBM.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        
        /* Consumer sends request. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(15);
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
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("TRI.N");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refresh. */
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
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        // Second fanout
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
    
    @Test
    public void batchRequestOutOfOrderTest()
    {
        /* Test a batch request/refresh exchange with the watchlist enabled, where two requests are sent
         * before both are recieved. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;
        int providerStreamId;
        WlInteger testUserSpecObjOne = ReactorFactory.createWlInteger();
        testUserSpecObjOne.value(997);
        WlInteger testUserSpecObjTwo = ReactorFactory.createWlInteger();
        testUserSpecObjTwo.value(998);
                
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        msg.containerType(DataTypes.ELEMENT_LIST);
        requestMsg.applyStreaming();
        requestMsg.applyHasBatch();

        encodeBatch(consumer.reactorChannel(), requestMsg);
        
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObjOne);
        assertTrue(consumer.submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer sends second request. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(10);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        msg.containerType(DataTypes.ELEMENT_LIST);
        requestMsg.applyStreaming();
        requestMsg.applyHasBatch();

        encodeBatch(consumer.reactorChannel(), requestMsg);
        
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObjTwo);
        assertTrue(consumer.submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        // Received status messages with closed batch stream
        consumerReactor.dispatch(2);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        assertEquals("DEFAULT_SERVICE", msgEvent.streamInfo()._serviceName);
        assertEquals(testUserSpecObjOne, msgEvent.streamInfo().userSpecObject());
        
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.CLOSED, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.OK, receivedStatusMsg.state().dataState());
        assertEquals("Stream closed for batch", receivedStatusMsg.state().text().toString());
        assertEquals(testUserSpecObjOne, msgEvent.streamInfo().userSpecObject());
        
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        assertEquals("DEFAULT_SERVICE", msgEvent.streamInfo()._serviceName);
        assertEquals(testUserSpecObjTwo, msgEvent.streamInfo().userSpecObject());
        
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.CLOSED, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.OK, receivedStatusMsg.state().dataState());
        assertEquals("Stream closed for batch", receivedStatusMsg.state().text().toString());
        assertEquals(testUserSpecObjTwo, msgEvent.streamInfo().userSpecObject());
        
        /* Provider receives request. */
        providerReactor.dispatch(4);
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
        
        assertTrue(provider.submit(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refresh. */
        consumerReactor.dispatch(2);
        
        // Received Refresh for TRI.N
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message with item TRI.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("TRI.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        
        // Received Refresh for TRI.N
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message with item TRI.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("TRI.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        
        // Provider processes request for IBM.N
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        
        providerStreamId = receivedRequestMsg.streamId();
        
        // Provider processes request for TRI.N
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
        
        // Provider processes request for TRI.N
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
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
        refreshMsg.msgKey().name().data("IBM.N");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        // Consumer receives refresh for IBM.N
        consumerReactor.dispatch(2);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message with item IBM.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("IBM.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message with item IBM.N (again)
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("IBM.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());

        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void batchRequestOverlappingStreamsTest()
    {
        /* Test a batch request where a stream we would create overlaps an already created stream, and fails. */
        
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        
        /* Consumer sends request on stream id 6. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("ABC.D");
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("ABC.D"));
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
        refreshMsg.msgKey().name().data("ABC.D");
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
        assertEquals("ABC.D", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        
        /* Consumer sends request. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        msg.containerType(DataTypes.ELEMENT_LIST);
        requestMsg.applyStreaming();
        requestMsg.applyHasBatch();

        encodeBatch(consumer.reactorChannel(), requestMsg);
        
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        
        assertTrue(consumer.reactorChannel().submit(requestMsg, submitOptions, errorInfo) == ReactorReturnCodes.FAILURE);
        assertEquals("Item in batch has same ID as existing stream.", errorInfo.error().text());

        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void batchRequestServiceIdAndServiceNameTest()
    {
        /* Test a batch request where we set the serviceId on the request, as well as serviceName in watchlist, to fail. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
                
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        msg.containerType(DataTypes.ELEMENT_LIST);
        requestMsg.applyStreaming();
        requestMsg.applyHasBatch();
        requestMsg.applyMsgKeyInUpdates();
        requestMsg.msgKey().applyHasServiceId();
        requestMsg.msgKey().serviceId(10);

        encodeBatch(consumer.reactorChannel(), requestMsg);
        
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        
        assertTrue(consumer.reactorChannel().submit(requestMsg, submitOptions, errorInfo) == ReactorReturnCodes.INVALID_USAGE);
        assertEquals("Cannot submit request with both service name and service id specified.", errorInfo.error().text());

        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void batchRequestMsgKeyItemNameTest()
    {
        /* Test a batch request where MsgKey ItemName is set on the request, which should fail */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
                
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        msg.containerType(DataTypes.ELEMENT_LIST);
        requestMsg.applyStreaming();
        requestMsg.applyHasBatch();
        requestMsg.applyMsgKeyInUpdates();
        requestMsg.msgKey().applyHasName();
        Buffer itemName = CodecFactory.createBuffer();
        itemName.data("BAD");
        requestMsg.msgKey().name(itemName);

        encodeBatch(consumer.reactorChannel(), requestMsg);
        
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        
        assertTrue(consumer.reactorChannel().submit(requestMsg, submitOptions, errorInfo) == ReactorReturnCodes.FAILURE);
        assertEquals("Requested batch has name in message key.", errorInfo.error().text());

        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void batchRequestSymbolListTest()
    {
        /* Test a batch request/refresh exchange that has a symbolList, with the watchlist enabled. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
		requestMsg.domainType(DomainTypes.SYMBOL_LIST);	       
		requestMsg.containerType(DataTypes.ELEMENT_LIST);	       
		requestMsg.applyHasQos();
		requestMsg.qos().rate(QosRates.TICK_BY_TICK);
		requestMsg.qos().timeliness(QosTimeliness.REALTIME);
		requestMsg.applyHasPriority();
		requestMsg.priority().priorityClass(1);
		requestMsg.priority().count(1);	      
		requestMsg.applyStreaming();
        requestMsg.applyHasBatch();

        encodeBatchWithSymbolList(consumer.reactorChannel(), requestMsg);
        
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        
        // Received status message with closed batch stream
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(DomainTypes.SYMBOL_LIST, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.CLOSED, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.OK, receivedStatusMsg.state().dataState());
        assertEquals("Stream closed for batch", receivedStatusMsg.state().text().toString());
        
        /* Provider receives request. */
        providerReactor.dispatch(2);
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
        assertEquals(DomainTypes.SYMBOL_LIST, receivedRequestMsg.domainType());
        
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.SYMBOL_LIST);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("TRI.N");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submit(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS /* Don't call dispatch, need to process other request */);
        
        /* Consumer receives refresh. */
        consumerReactor.dispatch(1);
        
        // Received Refresh for TRI.N
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message for TRI.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("TRI.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.SYMBOL_LIST, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        
        // Provider processes request for IBM.N
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
        assertEquals(DomainTypes.SYMBOL_LIST, receivedRequestMsg.domainType());
        
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.SYMBOL_LIST);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("IBM.N");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        // Consumer receives refresh for IBM.N
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        // Received refresh message for IBM.N
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals("IBM.N", receivedRefreshMsg.msgKey().name().toString());
        assertEquals(DomainTypes.SYMBOL_LIST, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());

        TestReactorComponent.closeSession(consumer, provider);
    }

	private void encodeBatch(ReactorChannel rc, RequestMsg msg) {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();

        /* encode message */
        encodeIter.setBufferAndRWFVersion(buf, rc.majorVersion(), rc.minorVersion());

        // encode payload

        ElementList eList = CodecFactory.createElementList();
        ElementEntry eEntry = CodecFactory.createElementEntry();
        Array elementArray = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        Buffer itemName = CodecFactory.createBuffer();

        eList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, eList.encodeInit(encodeIter, null, 0));

        // encode Batch
        eEntry.name(ElementNames.BATCH_ITEM_LIST);
        eEntry.dataType(DataTypes.ARRAY);
        assertEquals(CodecReturnCodes.SUCCESS, eEntry.encodeInit(encodeIter, 0));

        /* Encode the array of requested item names */
        elementArray.primitiveType(DataTypes.ASCII_STRING);
        elementArray.itemLength(0);

        assertEquals(CodecReturnCodes.SUCCESS, elementArray.encodeInit(encodeIter));
        itemName.data("TRI.N");
        assertEquals(CodecReturnCodes.SUCCESS, ae.encode(encodeIter, itemName));
        itemName.data("IBM.N");
        assertEquals(CodecReturnCodes.SUCCESS, ae.encode(encodeIter, itemName));

        assertEquals(CodecReturnCodes.SUCCESS, elementArray.encodeComplete(encodeIter, true));

        assertEquals(CodecReturnCodes.SUCCESS, eEntry.encodeComplete(encodeIter, true));
        
        assertEquals(CodecReturnCodes.SUCCESS, eList.encodeComplete(encodeIter, true));

        msg.encodedDataBody(buf);
	}
	

	private void encodeBatchWithSymbolList(ReactorChannel rc, RequestMsg msg) {
        Buffer buf = CodecFactory.createBuffer();
        buf.data(ByteBuffer.allocate(1024));
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();

        /* clear encode iterator */
        encodeIter.clear();
        
        /* encode message */
        encodeIter.setBufferAndRWFVersion(buf, rc.majorVersion(), rc.minorVersion());

        // encode payload

        ElementList eList = CodecFactory.createElementList();
        ElementEntry eEntry = CodecFactory.createElementEntry();
        Array elementArray = CodecFactory.createArray();
        ArrayEntry ae = CodecFactory.createArrayEntry();
        Buffer itemName = CodecFactory.createBuffer();

        eList.applyHasStandardData();
        assertEquals(CodecReturnCodes.SUCCESS, eList.encodeInit(encodeIter, null, 0));

        // encode Batch
        eEntry.name(ElementNames.BATCH_ITEM_LIST);
        eEntry.dataType(DataTypes.ARRAY);
        assertEquals(CodecReturnCodes.SUCCESS, eEntry.encodeInit(encodeIter, 0));

        /* Encode the array of requested item names */
        elementArray.primitiveType(DataTypes.ASCII_STRING);
        elementArray.itemLength(0);

        assertEquals(CodecReturnCodes.SUCCESS, elementArray.encodeInit(encodeIter));
        itemName.data("TRI.N");
        assertEquals(CodecReturnCodes.SUCCESS, ae.encode(encodeIter, itemName));
        itemName.data("IBM.N");
        assertEquals(CodecReturnCodes.SUCCESS, ae.encode(encodeIter, itemName));

        assertEquals(CodecReturnCodes.SUCCESS, elementArray.encodeComplete(encodeIter, true));
        
        assertEquals(CodecReturnCodes.SUCCESS, eEntry.encodeComplete(encodeIter, true));
        
		ElementList behaviorList = CodecFactory.createElementList();
		ElementEntry dataStreamEntry = CodecFactory.createElementEntry();	
		UInt tempUInt = CodecFactory.createUInt();
		
		eEntry.clear();
		eEntry.name().data(":SymbolListBehaviors");
		eEntry.dataType(DataTypes.ELEMENT_LIST);
		
		assertEquals(ReactorReturnCodes.SUCCESS, eEntry.encodeInit(encodeIter, 0));

		behaviorList.clear();
		behaviorList.applyHasStandardData();

		assertEquals(ReactorReturnCodes.SUCCESS, behaviorList.encodeInit(encodeIter, null, 0));
	       
		dataStreamEntry.clear();
		dataStreamEntry.name().data(":DataStreams");
		dataStreamEntry.dataType(DataTypes.UINT);
		tempUInt.value(SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
		assertEquals(ReactorReturnCodes.SUCCESS, dataStreamEntry.encode(encodeIter, tempUInt));

		assertEquals(ReactorReturnCodes.SUCCESS, behaviorList.encodeComplete(encodeIter, true));

        assertEquals(CodecReturnCodes.SUCCESS, eEntry.encodeComplete(encodeIter, true));
        
        assertEquals(CodecReturnCodes.SUCCESS, eList.encodeComplete(encodeIter, true));
        
        msg.encodedDataBody(buf);
	}

    /* Encodes an ElementEntry requesting enhanced symbol list behaviors. */
    private void encodeSymbolListBehaviorsElement(EncodeIterator encIter, long dataStreamFlags)
    {
        int ret;
		ElementList behaviorList = CodecFactory.createElementList();
		ElementEntry elementEntry = CodecFactory.createElementEntry();
		ElementEntry dataStreamEntry = CodecFactory.createElementEntry();	
		UInt tempUInt = CodecFactory.createUInt();
		
		elementEntry.clear();
		elementEntry.name().data(":SymbolListBehaviors");
		elementEntry.dataType(DataTypes.ELEMENT_LIST);
		
		ret = elementEntry.encodeInit(encIter, 0);
		assertTrue(ret >= ReactorReturnCodes.SUCCESS);

		behaviorList.clear();
		behaviorList.applyHasStandardData();
		ret = behaviorList.encodeInit(encIter, null, 0);
		assertTrue(ret >= ReactorReturnCodes.SUCCESS);
	       
		dataStreamEntry.clear();
		dataStreamEntry.name().data(":DataStreams");
		dataStreamEntry.dataType(DataTypes.UINT);
		tempUInt.value(dataStreamFlags);
	    ret = dataStreamEntry.encode(encIter, tempUInt); 
	    assertTrue(ret >= ReactorReturnCodes.SUCCESS);
		ret = behaviorList.encodeComplete(encIter, true);
		assertTrue(ret >= ReactorReturnCodes.SUCCESS);		
		ret = elementEntry.encodeComplete(encIter, true);
		assertTrue(ret >= ReactorReturnCodes.SUCCESS);		
    }

   @Test
    public void symbolListDataStreamTest()
    {
        /* Test a symbolList data stream request/refresh exchange with the watchlist enabled. */
        
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        requestMsg.domainType(DomainTypes.SYMBOL_LIST);
        requestMsg.applyStreaming();
        
	    Buffer payload = CodecFactory.createBuffer(); 
	    payload.data(ByteBuffer.allocate(1024));

	    EncodeIterator encIter = CodecFactory.createEncodeIterator();
		encIter.clear();
		encIter.setBufferAndRWFVersion(payload, consumerReactor._reactor.reactorChannel().majorVersion(),
				consumerReactor._reactor.reactorChannel().minorVersion());
			
		requestMsg.clear();
		/* set-up message */
		requestMsg.msgClass(MsgClasses.REQUEST);
		requestMsg.streamId(5);
		requestMsg.domainType(DomainTypes.SYMBOL_LIST);	       
		requestMsg.containerType(DataTypes.ELEMENT_LIST);	       
		requestMsg.applyHasQos();
		requestMsg.qos().rate(QosRates.TICK_BY_TICK);
		requestMsg.qos().timeliness(QosTimeliness.REALTIME);
		requestMsg.applyHasPriority();
		requestMsg.priority().priorityClass(1);
		requestMsg.priority().count(1);	      
		requestMsg.applyStreaming();
	       		
		ElementList elementList = CodecFactory.createElementList();
		elementList.clear();
		elementList.applyHasStandardData();
		
		int ret = elementList.encodeInit(encIter, null, 0);
		assertTrue(ret >= ReactorReturnCodes.SUCCESS);

        encodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
		ret = elementList.encodeComplete(encIter, true);
		assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        requestMsg.encodedDataBody(payload);        
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
        assertEquals(DomainTypes.SYMBOL_LIST, receivedRequestMsg.domainType());        
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.SYMBOL_LIST);
        refreshMsg.streamId(providerStreamId);
           
        refreshMsg.containerType(DataTypes.MAP); 
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
 
        Map tempMap = CodecFactory.createMap();
        MapEntry tempMapEntry = CodecFactory.createMapEntry();
        Buffer tempBuffer = CodecFactory.createBuffer();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        Buffer msgBuf = CodecFactory.createBuffer(); 
        msgBuf.data(ByteBuffer.allocate(2048));
 
        /* encode message */
        encodeIter.setBufferAndRWFVersion(msgBuf, consumerReactor._reactor.reactorChannel().majorVersion(),
        		consumerReactor._reactor.reactorChannel().minorVersion());
        tempMap.clear();
        tempMap.flags(0);
        tempMap.containerType(DataTypes.NO_DATA);
        tempMap.keyPrimitiveType(DataTypes.BUFFER);
        ret = tempMap.encodeInit(encodeIter, 0, 0);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);

        /* encode map entry */
        tempMapEntry.clear();
        tempMapEntry.flags(MapEntryFlags.NONE);
        tempBuffer.clear();
 
        tempBuffer.data("FB.O");
        tempMapEntry.action(MapEntryActions.ADD);

        ret = tempMapEntry.encodeInit(encodeIter, tempBuffer, 0);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);

        ret = tempMapEntry.encodeComplete(encodeIter, true);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        tempBuffer.data("AAPL.O");
        ret = tempMapEntry.encode(encodeIter, tempBuffer);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        tempBuffer.data("NFLX.O");
        ret = tempMapEntry.encode(encodeIter, tempBuffer);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        tempBuffer.data("GOOGL.O");
        ret = tempMapEntry.encode(encodeIter, tempBuffer);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        ret = tempMap.encodeComplete(encodeIter, true);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);         
        refreshMsg.encodedDataBody(msgBuf);         
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);       
                
        // watchlist receives a symbol list data refresh and send out each item request
        consumerReactor._reactor._reactorOptions.enableXmlTracing();
        consumerReactor.dispatch(1);
        consumerReactor._eventQueue.clear();
        
        // provider receives request and send refresh back for the first market price item
        providerReactor.dispatch(4);
      
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
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
        refreshMsg.msgKey().name(receivedRequestMsg.msgKey().name());
        refreshMsg.applyRefreshComplete();        
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
 
        providerReactor.pollEvent();
        providerReactor.pollEvent();
        providerReactor.pollEvent();
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
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals(receivedRequestMsg.msgKey().name().toString()));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        
        TestReactorComponent.closeSession(consumer, provider);
    }    

   @Test
   public void symbolListDataSnapshotTest()
   {
       /* Test a symbol list snapshot request, that also requests data snapshots. */
       ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
       TestReactorEvent event;
       ReactorMsgEvent msgEvent;
       Msg msg = CodecFactory.createMsg();
       RequestMsg requestMsg = (RequestMsg)msg;
       RequestMsg receivedRequestMsg;
       RefreshMsg refreshMsg = (RefreshMsg)msg;
       RefreshMsg receivedRefreshMsg;
       UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
       CloseMsg receivedCloseMsg;
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
       consumerRole.watchlistOptions().requestTimeout(3000);

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
       opts.reconnectAttemptLimit(-1);
       provider.bind(opts);
       TestReactor.openSession(consumer, provider, opts);

       Buffer payload = CodecFactory.createBuffer(); 
       payload.data(ByteBuffer.allocate(1024));

       EncodeIterator encIter = CodecFactory.createEncodeIterator();
       encIter.clear();
       encIter.setBufferAndRWFVersion(payload, consumerReactor._reactor.reactorChannel().majorVersion(),
               consumerReactor._reactor.reactorChannel().minorVersion());

       requestMsg.clear();
       /* set-up message */
       requestMsg.msgClass(MsgClasses.REQUEST);
       requestMsg.streamId(5);
       requestMsg.domainType(DomainTypes.SYMBOL_LIST);	       
       requestMsg.msgKey().applyHasName();
       requestMsg.msgKey().name().data("SYMBOL_LIST");
       requestMsg.containerType(DataTypes.ELEMENT_LIST);	       
       requestMsg.applyHasQos();
       requestMsg.qos().rate(QosRates.TICK_BY_TICK);
       requestMsg.qos().timeliness(QosTimeliness.REALTIME);
       requestMsg.applyHasPriority();
       requestMsg.priority().priorityClass(1);
       requestMsg.priority().count(1);	      

       ElementList elementList = CodecFactory.createElementList();
       elementList.clear();
       elementList.applyHasStandardData();

       int ret = elementList.encodeInit(encIter, null, 0);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);

       encodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_SNAPSHOTS);
       ret = elementList.encodeComplete(encIter, true);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);
       requestMsg.encodedDataBody(payload);        
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
       assertFalse(receivedRequestMsg.checkStreaming());
       assertFalse(receivedRequestMsg.checkNoRefresh());
       assertTrue(receivedRequestMsg.msgKey().checkHasName());
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("SYMBOL_LIST"));
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertEquals(DomainTypes.SYMBOL_LIST, receivedRequestMsg.domainType());        
       providerStreamId = receivedRequestMsg.streamId();

       /* Provider sends non-streaming refresh, with symbol list payload.*/
       refreshMsg.clear();
       refreshMsg.msgClass(MsgClasses.REFRESH);
       refreshMsg.domainType(DomainTypes.SYMBOL_LIST);
       refreshMsg.streamId(providerStreamId);
       refreshMsg.containerType(DataTypes.MAP); 
       refreshMsg.applySolicited();
       refreshMsg.applyRefreshComplete();
       refreshMsg.applyHasMsgKey();
       refreshMsg.msgKey().applyHasServiceId();
       refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
       refreshMsg.msgKey().applyHasName();
       refreshMsg.msgKey().name().data("SYMBOL_LIST");
       refreshMsg.state().streamState(StreamStates.NON_STREAMING);
       refreshMsg.state().dataState(DataStates.OK);

       Map tempMap = CodecFactory.createMap();
       MapEntry tempMapEntry = CodecFactory.createMapEntry();
       Buffer tempBuffer = CodecFactory.createBuffer();
       EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
       Buffer msgBuf = CodecFactory.createBuffer(); 
       msgBuf.data(ByteBuffer.allocate(2048));

       /* encode message */
       encodeIter.setBufferAndRWFVersion(msgBuf, consumerReactor._reactor.reactorChannel().majorVersion(),
               consumerReactor._reactor.reactorChannel().minorVersion());
       tempMap.clear();
       tempMap.flags(0);
       tempMap.containerType(DataTypes.NO_DATA);
       tempMap.keyPrimitiveType(DataTypes.BUFFER);
       ret = tempMap.encodeInit(encodeIter, 0, 0);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);

       /* encode map entry */
       tempMapEntry.clear();
       tempMapEntry.flags(MapEntryFlags.NONE);
       tempBuffer.clear();

       tempBuffer.data("FB.O");
       tempMapEntry.action(MapEntryActions.ADD);

       ret = tempMapEntry.encodeInit(encodeIter, tempBuffer, 0);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);

       ret = tempMapEntry.encodeComplete(encodeIter, true);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);
       tempBuffer.data("AAPL.O");
       ret = tempMapEntry.encode(encodeIter, tempBuffer);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);
       tempBuffer.data("NFLX.O");
       ret = tempMapEntry.encode(encodeIter, tempBuffer);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);
       tempBuffer.data("GOOGL.O");
       ret = tempMapEntry.encode(encodeIter, tempBuffer);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);
       ret = tempMap.encodeComplete(encodeIter, true);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);         
       refreshMsg.encodedDataBody(msgBuf);         
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);       

       /* Watchlist recevies SymbolList refresh, and internally sends requests for each item. */
       consumerReactor.dispatch(1);

       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(5, receivedRefreshMsg.streamId());
       assertTrue(receivedRefreshMsg.checkHasMsgKey());
       assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
       assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
       assertTrue(receivedRefreshMsg.msgKey().checkHasName());
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("SYMBOL_LIST"));
       assertEquals(DomainTypes.SYMBOL_LIST, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.MAP, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());

       /* Provider receives requests for the items in the symbol list. */
       providerReactor.dispatch(4);

       int itemStreamId1, itemStreamId2, itemStreamId3, itemStreamId4;

       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertFalse(receivedRequestMsg.checkStreaming());
       assertFalse(receivedRequestMsg.checkNoRefresh());
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("FB.O"));
       itemStreamId1 = receivedRequestMsg.streamId();

       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertFalse(receivedRequestMsg.checkStreaming());
       assertFalse(receivedRequestMsg.checkNoRefresh());
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("AAPL.O"));
       itemStreamId2 = receivedRequestMsg.streamId();

       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertFalse(receivedRequestMsg.checkStreaming());
       assertFalse(receivedRequestMsg.checkNoRefresh());
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("NFLX.O"));
       itemStreamId3 = receivedRequestMsg.streamId();

       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertFalse(receivedRequestMsg.checkStreaming());
       assertFalse(receivedRequestMsg.checkNoRefresh());
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("GOOGL.O"));
       itemStreamId4 = receivedRequestMsg.streamId();

       /* Provider sends refreshes .*/
       refreshMsg.clear();
       refreshMsg.msgClass(MsgClasses.REFRESH);
       refreshMsg.domainType(DomainTypes.MARKET_PRICE);
       refreshMsg.streamId(itemStreamId1);
       refreshMsg.containerType(DataTypes.NO_DATA);
       refreshMsg.applySolicited();
       refreshMsg.applyRefreshComplete();
       refreshMsg.applyHasMsgKey();
       refreshMsg.msgKey().applyHasServiceId();
       refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
       refreshMsg.msgKey().applyHasName();
       refreshMsg.msgKey().name().data("FB.O");
       refreshMsg.applyRefreshComplete();        
       refreshMsg.state().streamState(StreamStates.NON_STREAMING);
       refreshMsg.state().dataState(DataStates.OK);
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       refreshMsg.msgKey().name().data("AAPL.O");
       refreshMsg.streamId(itemStreamId2);
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       refreshMsg.msgKey().name().data("NFLX.O");
       refreshMsg.streamId(itemStreamId3);
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       refreshMsg.msgKey().name().data("GOOGL.O");
       refreshMsg.streamId(itemStreamId4);
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       /* Consumer receives refreshes. */
       consumerReactor.dispatch(4);

       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertTrue(receivedRefreshMsg.streamId() < 0);
       assertTrue(receivedRefreshMsg.checkHasMsgKey());
       assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
       assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
       assertTrue(receivedRefreshMsg.msgKey().checkHasName());
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("FB.O"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());

       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertTrue(receivedRefreshMsg.streamId() < 0);
       assertTrue(receivedRefreshMsg.checkHasMsgKey());
       assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
       assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
       assertTrue(receivedRefreshMsg.msgKey().checkHasName());
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("AAPL.O"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());

       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertTrue(receivedRefreshMsg.streamId() < 0);
       assertTrue(receivedRefreshMsg.checkHasMsgKey());
       assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
       assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
       assertTrue(receivedRefreshMsg.msgKey().checkHasName());
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("NFLX.O"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());

       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertTrue(receivedRefreshMsg.streamId() < 0);
       assertTrue(receivedRefreshMsg.checkHasMsgKey());
       assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
       assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
       assertTrue(receivedRefreshMsg.msgKey().checkHasName());
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("GOOGL.O"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());

       /* Send updates. Consumer should not receive them. */
       updateMsg.clear();
       updateMsg.msgClass(MsgClasses.UPDATE);
       updateMsg.streamId(itemStreamId1);
       updateMsg.domainType(DomainTypes.MARKET_PRICE);
       updateMsg.containerType(DataTypes.NO_DATA);
       assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       updateMsg.streamId(itemStreamId2);
       assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       updateMsg.streamId(itemStreamId3);
       assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       updateMsg.streamId(itemStreamId4);
       assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       consumerReactor.dispatch(0);

       /* Provider gets CloseMsgs for the unwanted updates. */
       providerReactor.dispatch(4);

       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
       receivedCloseMsg = (CloseMsg)msgEvent.msg();
       assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());        
       assertEquals(itemStreamId1, receivedCloseMsg.streamId());

       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
       receivedCloseMsg = (CloseMsg)msgEvent.msg();
       assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());        
       assertEquals(itemStreamId2, receivedCloseMsg.streamId());

       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
       receivedCloseMsg = (CloseMsg)msgEvent.msg();
       assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());        
       assertEquals(itemStreamId3, receivedCloseMsg.streamId());

       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
       receivedCloseMsg = (CloseMsg)msgEvent.msg();
       assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());        
       assertEquals(itemStreamId4, receivedCloseMsg.streamId());

       /* Disconnect provider. */
       provider.closeChannel();

       /* Consumer receives Channel Event, Login Status, Directory Update, nothing else (symbol list and items were snapshots, and
        * so should not be re-requested). */
       consumerReactor.dispatch(3);
       event = consumer.testReactor().pollEvent();
       assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
       ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
       assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());

       RDMLoginMsgEvent loginMsgEvent;                
       event = consumer.testReactor().pollEvent();
       assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
       loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
       assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());  

       RDMDirectoryMsgEvent directoryMsgEvent;                
       event = consumer.testReactor().pollEvent();
       assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
       directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
       assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());   

       /* Reconnect and reestablish login/directory streams. */
       TestReactor.openSession(consumer, provider, opts, true);

       /* Provider receives nothing else (no recovery). */
       providerReactor.dispatch(0);

       TestReactorComponent.closeSession(consumer, provider);

   }    

    @Test
    public void symbolListDataStreamTest_Reconnect()
    {
        /* Test recovery of symbol list data streams after reconnect:
         * 1) Open a symbol list stream, requesting data streams
         * 2) Establish symbol list and item streams
         * 3) Kill/restart connection
         * 4) Re-establish symbol list and item streams */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
        opts.reconnectAttemptLimit(-1);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        /* Consumer sends symbol list request, which requests data streams. */
	    Buffer payload = CodecFactory.createBuffer(); 
	    payload.data(ByteBuffer.allocate(1024));

	    EncodeIterator encIter = CodecFactory.createEncodeIterator();
		encIter.clear();
		encIter.setBufferAndRWFVersion(payload, consumerReactor._reactor.reactorChannel().majorVersion(),
				consumerReactor._reactor.reactorChannel().minorVersion());
			
		requestMsg.clear();
		requestMsg.msgClass(MsgClasses.REQUEST);
		requestMsg.streamId(5);
		requestMsg.domainType(DomainTypes.SYMBOL_LIST);	       
		requestMsg.containerType(DataTypes.ELEMENT_LIST);	       
		requestMsg.applyHasQos();
		requestMsg.qos().rate(QosRates.TICK_BY_TICK);
		requestMsg.qos().timeliness(QosTimeliness.REALTIME);
		requestMsg.applyHasPriority();
		requestMsg.priority().priorityClass(1);
		requestMsg.priority().count(1);	      
		requestMsg.applyStreaming();
		requestMsg.msgKey().applyHasName();
		requestMsg.msgKey().name().data("SYM_LIST");
	       		
		ElementList elementList = CodecFactory.createElementList();
		elementList.clear();
		elementList.applyHasStandardData();
		
		int ret = elementList.encodeInit(encIter, null, 0);
		assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        encodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
		ret = elementList.encodeComplete(encIter, true);
		assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        requestMsg.encodedDataBody(payload);        
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("SYM_LIST"));
        assertEquals(DomainTypes.SYMBOL_LIST, receivedRequestMsg.domainType());        
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.SYMBOL_LIST);
        refreshMsg.streamId(providerStreamId);
           
        refreshMsg.containerType(DataTypes.MAP); 
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("SYM_LIST");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
 
        Map tempMap = CodecFactory.createMap();
        MapEntry tempMapEntry = CodecFactory.createMapEntry();
        Buffer tempBuffer = CodecFactory.createBuffer();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        Buffer msgBuf = CodecFactory.createBuffer(); 
        msgBuf.data(ByteBuffer.allocate(2048));
 
        /* encode message */
        encodeIter.setBufferAndRWFVersion(msgBuf, consumerReactor._reactor.reactorChannel().majorVersion(),
        		consumerReactor._reactor.reactorChannel().minorVersion());
        tempMap.clear();
        tempMap.flags(0);
        tempMap.containerType(DataTypes.NO_DATA);
        tempMap.keyPrimitiveType(DataTypes.BUFFER);
        ret = tempMap.encodeInit(encodeIter, 0, 0);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);

        /* encode map entry */
        tempMapEntry.clear();
        tempMapEntry.flags(MapEntryFlags.NONE);
        tempBuffer.clear();
 
        tempBuffer.data("FB.O");
        tempMapEntry.action(MapEntryActions.ADD);
        ret = tempMapEntry.encode(encodeIter, tempBuffer);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        tempBuffer.data("AAPL.O");
        ret = tempMapEntry.encode(encodeIter, tempBuffer);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);

        ret = tempMap.encodeComplete(encodeIter, true);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);         

        refreshMsg.encodedDataBody(msgBuf);         
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);       
                
        // watchlist receives a symbol list data refresh and send out each item request
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
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("SYM_LIST"));
        assertEquals(DomainTypes.SYMBOL_LIST, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.MAP, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        /* Provider receives requests for FB and AAPL. */
        providerReactor.dispatch(2);

        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("FB.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
        int provFBStreamId = receivedRequestMsg.streamId();

        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("AAPL.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
        int provAAPLStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refreshes for FB and AAPL .*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(provFBStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("FB.O");
        refreshMsg.applyRefreshComplete();        
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(provAAPLStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("AAPL.O");
        refreshMsg.applyRefreshComplete();        
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
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("FB.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertTrue(receivedRefreshMsg.streamId() < 0);
        int consFBStreamId = receivedRefreshMsg.streamId();

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("AAPL.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertTrue(receivedRefreshMsg.streamId() < 0);
        int consAAPLStreamId = receivedRefreshMsg.streamId();

        /* Disconnect provider. */
        provider.closeChannel();
        
        /* Consumer receives Login Status, Directory Update, and item status for SYM_LIST, FB.O & AAPL.O. */
        consumerReactor.dispatch(6);
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());

        RDMLoginMsgEvent loginMsgEvent;                
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
        loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
        assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());  

        RDMDirectoryMsgEvent directoryMsgEvent;                
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());   

        /* SYM_LIST */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(DomainTypes.SYMBOL_LIST, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertEquals(5, receivedStatusMsg.streamId());

        /* FB.O */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertEquals(consFBStreamId, receivedStatusMsg.streamId());

        /* AAPL.O */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertEquals(consAAPLStreamId, receivedStatusMsg.streamId());

        /* Reconnect and reestablish login/directory streams. */
        TestReactor.openSession(consumer, provider, opts, true);
        
        /* Provider receives requests for SYM_LIST, FB.O, and AAPL.O. */
        providerReactor.dispatch(3);
        
        /* SYM_LIST */
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("SYM_LIST"));
        assertEquals(DomainTypes.SYMBOL_LIST, receivedRequestMsg.domainType());        
        providerStreamId = receivedRequestMsg.streamId();
        
        /* FB.O */
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("FB.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
        provFBStreamId = receivedRequestMsg.streamId();
        
        /* AAPL.O */
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("AAPL.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
        provAAPLStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refreshes for SYM_LIST, FB and AAPL .*/

        /* SYM_LIST (with same payload) */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.SYMBOL_LIST);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.MAP); 
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("SYM_LIST");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.encodedDataBody(msgBuf);         
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);       
                    
        /* FB.O */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(provFBStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("FB.O");
        refreshMsg.applyRefreshComplete();        
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* AAPL.O */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(provAAPLStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("AAPL.O");
        refreshMsg.applyRefreshComplete();        
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refreshes. */
        consumerReactor.dispatch(3);

        /* SYM_LIST */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("SYM_LIST"));
        assertEquals(DomainTypes.SYMBOL_LIST, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.MAP, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        /* FB.O */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("FB.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertEquals(consFBStreamId, receivedRefreshMsg.streamId());

        /* AAPL.O */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("AAPL.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertEquals(consAAPLStreamId, receivedRefreshMsg.streamId());
        
        TestReactorComponent.closeSession(consumer, provider);
    }    

    @Test
    public void symbolListDataStreamUpdateSymbolListTest()
    {
        /* Test updates to a symbol list with a consumer requesting symbol list data streams:
         * - Initially send a symbol list refresh with three items, which will be automatically requested. Refresh these items.
         * - Send an update to add a fourth item, which will be automatically requested. Refresh this item (with a non-streaming refresh).
         * - Send an update to add the same fourth item again. Since it is not open, it will be automatically requested. Refresh it (streaming this time).
         * - Close the fourth item from the consumer.
         * - Send an update to add the same fourth item yet again. Since it is not open, it will be automatically requested. Refresh it.
         * - Send an update to add the same fourth item yet again. This time, the item is still open, so it shouldn't be requested again.
         */ 
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
        UpdateMsg receivedUpdateMsg;
        CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
        CloseMsg receivedCloseMsg;
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
        consumerRole.watchlistOptions().requestTimeout(3000);

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
        opts.reconnectAttemptLimit(-1);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);

        Buffer payload = CodecFactory.createBuffer(); 
        payload.data(ByteBuffer.allocate(1024));

        EncodeIterator encIter = CodecFactory.createEncodeIterator();
        encIter.clear();
        encIter.setBufferAndRWFVersion(payload, consumerReactor._reactor.reactorChannel().majorVersion(),
                consumerReactor._reactor.reactorChannel().minorVersion());

        requestMsg.clear();
        /* set-up message */
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.SYMBOL_LIST);	       
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("SYMBOL_LIST");
        requestMsg.containerType(DataTypes.ELEMENT_LIST);	       
        requestMsg.applyHasQos();
        requestMsg.qos().rate(QosRates.TICK_BY_TICK);
        requestMsg.qos().timeliness(QosTimeliness.REALTIME);
        requestMsg.applyHasPriority();
        requestMsg.priority().priorityClass(1);
        requestMsg.priority().count(1);
        requestMsg.applyStreaming();

        ElementList elementList = CodecFactory.createElementList();
        elementList.clear();
        elementList.applyHasStandardData();

        int ret = elementList.encodeInit(encIter, null, 0);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);

        encodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
        ret = elementList.encodeComplete(encIter, true);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        requestMsg.encodedDataBody(payload);        
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
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("SYMBOL_LIST"));
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertEquals(DomainTypes.SYMBOL_LIST, receivedRequestMsg.domainType());        
        providerStreamId = receivedRequestMsg.streamId();

        /* Provider sends refresh, with symbol list payload.*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.SYMBOL_LIST);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.MAP); 
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("SYMBOL_LIST");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);

        Map tempMap = CodecFactory.createMap();
        MapEntry tempMapEntry = CodecFactory.createMapEntry();
        Buffer tempBuffer = CodecFactory.createBuffer();
        EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
        Buffer msgBuf = CodecFactory.createBuffer(); 
        msgBuf.data(ByteBuffer.allocate(2048));

        /* encode message */
        encodeIter.setBufferAndRWFVersion(msgBuf, consumerReactor._reactor.reactorChannel().majorVersion(),
                consumerReactor._reactor.reactorChannel().minorVersion());
        tempMap.clear();
        tempMap.flags(0);
        tempMap.containerType(DataTypes.NO_DATA);
        tempMap.keyPrimitiveType(DataTypes.BUFFER);
        ret = tempMap.encodeInit(encodeIter, 0, 0);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);

        /* encode map entry */
        tempMapEntry.clear();
        tempMapEntry.flags(MapEntryFlags.NONE);
        tempBuffer.clear();

        tempBuffer.data("FB.O");
        tempMapEntry.action(MapEntryActions.ADD);
        ret = tempMapEntry.encode(encodeIter, tempBuffer);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        tempBuffer.data("AAPL.O");
        ret = tempMapEntry.encode(encodeIter, tempBuffer);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        tempBuffer.data("NFLX.O");
        ret = tempMapEntry.encode(encodeIter, tempBuffer);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        ret = tempMap.encodeComplete(encodeIter, true);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);         

        refreshMsg.encodedDataBody(msgBuf);         
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);       

        /* Watchlist recevies SymbolList refresh, and internally sends requests for each item. */
        consumerReactor.dispatch(1);

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("SYMBOL_LIST"));
        assertEquals(DomainTypes.SYMBOL_LIST, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.MAP, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());

        /* Provider receives requests for the items in the symbol list. */
        providerReactor.dispatch(3);

        int itemStreamId1, itemStreamId2, itemStreamId3;

        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("FB.O"));
        itemStreamId1 = receivedRequestMsg.streamId();

        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("AAPL.O"));
        itemStreamId2 = receivedRequestMsg.streamId();

        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("NFLX.O"));
        itemStreamId3 = receivedRequestMsg.streamId();

        /* Provider sends refreshes .*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(itemStreamId1);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("FB.O");
        refreshMsg.applyRefreshComplete();        
        refreshMsg.state().streamState(StreamStates.NON_STREAMING);
        refreshMsg.state().dataState(DataStates.OK);
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        refreshMsg.msgKey().name().data("AAPL.O");
        refreshMsg.streamId(itemStreamId2);
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        refreshMsg.msgKey().name().data("NFLX.O");
        refreshMsg.streamId(itemStreamId3);
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives refreshes. */
        consumerReactor.dispatch(3);

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertTrue(receivedRefreshMsg.streamId() < 0);
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("FB.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertTrue(receivedRefreshMsg.streamId() < 0);
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("AAPL.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertTrue(receivedRefreshMsg.streamId() < 0);
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("NFLX.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());

        /* Provider sends an update with a new item. */
        encodeIter.setBufferAndRWFVersion(msgBuf, consumerReactor._reactor.reactorChannel().majorVersion(),
                consumerReactor._reactor.reactorChannel().minorVersion());
        tempMap.clear();
        tempMap.flags(0);
        tempMap.containerType(DataTypes.NO_DATA);
        tempMap.keyPrimitiveType(DataTypes.BUFFER);
        ret = tempMap.encodeInit(encodeIter, 0, 0);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);

        tempMapEntry.clear();
        tempMapEntry.flags(MapEntryFlags.NONE);
        tempMapEntry.action(MapEntryActions.ADD);
        tempBuffer.clear();
        tempBuffer.data("GOOGL.O");
        ret = tempMapEntry.encode(encodeIter, tempBuffer);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);
        ret = tempMap.encodeComplete(encodeIter, true);
        assertTrue(ret >= ReactorReturnCodes.SUCCESS);         

        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(providerStreamId);
        updateMsg.domainType(DomainTypes.SYMBOL_LIST);
        updateMsg.containerType(DataTypes.MAP);
        updateMsg.encodedDataBody(msgBuf);         
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);       

        /* Consumer receives symbol list update. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(5, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.SYMBOL_LIST, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.MAP, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());

        int itemStreamId4;

        /* Provider receives request for new item. */
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
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("GOOGL.O"));
        itemStreamId4 = receivedRequestMsg.streamId();

        /* Provider sends non-streaming refresh for the item. */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(itemStreamId4);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("GOOGL.O");
        refreshMsg.applyRefreshComplete();        
        refreshMsg.state().streamState(StreamStates.NON_STREAMING);
        refreshMsg.state().dataState(DataStates.OK);
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertTrue(receivedRefreshMsg.streamId() < 0);
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("GOOGL.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());

        /* Provider sends symbol list update with the new item again. */
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);       

        /* Consumer receives symbol list update. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(5, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.SYMBOL_LIST, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.MAP, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());

        /* Provider receives request for new item. */
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
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("GOOGL.O"));
        itemStreamId4 = receivedRequestMsg.streamId();

        /* Provider sends streaming refresh for the item. */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(itemStreamId4);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("GOOGL.O");
        refreshMsg.applyRefreshComplete();        
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
        assertTrue(receivedRefreshMsg.streamId() < 0);
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("GOOGL.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        
        int consumerItemStreamId4 = receivedRefreshMsg.streamId();

        /* Consumer closes item. */
        closeMsg.clear();
        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(consumerItemStreamId4);
        closeMsg.domainType(DomainTypes.MARKET_PRICE);
        closeMsg.containerType(DataTypes.NO_DATA); 
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Provider receives close. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
        receivedCloseMsg = (CloseMsg)msgEvent.msg();
        assertEquals(itemStreamId4, receivedCloseMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());

        /* Provider sends symbol list update with the new item yet again. */
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);       

        /* Consumer receives symbol list update. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(5, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.SYMBOL_LIST, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.MAP, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());

        /* Provider receives request for new item. */
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
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("GOOGL.O"));
        itemStreamId4 = receivedRequestMsg.streamId();

        /* Provider sends streaming refresh for the item. */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(itemStreamId4);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("GOOGL.O");
        refreshMsg.applyRefreshComplete();        
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
        assertTrue(receivedRefreshMsg.streamId() < 0);
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("GOOGL.O"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());

        /* Provider sends symbol list update with the new item yet again. */
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);       

        /* Consumer receives symbol list update. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(5, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.SYMBOL_LIST, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.MAP, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());

        /* Item is already open this time, so provider does not receive a request for it. */
        providerReactor.dispatch(0);
        
        TestReactorComponent.closeSession(consumer, provider);
    }    

    @Test
    public void symbolListDataStreamTest_MsgKey()
    {
        /* Test that the watchlist adds the MsgKey to symbol list data streams when appropriate --
         * namely when the initial response does not contain one. */

        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();
        StatusMsg receivedStatusMsg;
        int providerStreamId;

        int streamStates[] = new int[] { StreamStates.OPEN, StreamStates.CLOSED, StreamStates.CLOSED_RECOVER};
                
        for (int i = 0; i < streamStates.length; ++i)
        {
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
            consumerRole.watchlistOptions().requestTimeout(3000);

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
            opts.reconnectAttemptLimit(-1);
            provider.bind(opts);
            TestReactor.openSession(consumer, provider, opts);

            /* Consumer sends symbol list request, which requests data streams. */
            Buffer payload = CodecFactory.createBuffer(); 
            payload.data(ByteBuffer.allocate(1024));

            EncodeIterator encIter = CodecFactory.createEncodeIterator();
            encIter.clear();
            encIter.setBufferAndRWFVersion(payload, consumerReactor._reactor.reactorChannel().majorVersion(),
                    consumerReactor._reactor.reactorChannel().minorVersion());

            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.SYMBOL_LIST);	       
            requestMsg.containerType(DataTypes.ELEMENT_LIST);	       
            requestMsg.applyHasQos();
            requestMsg.qos().rate(QosRates.TICK_BY_TICK);
            requestMsg.qos().timeliness(QosTimeliness.REALTIME);
            requestMsg.applyHasPriority();
            requestMsg.priority().priorityClass(1);
            requestMsg.priority().count(1);	      
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("SYM_LIST");

            ElementList elementList = CodecFactory.createElementList();
            elementList.clear();
            elementList.applyHasStandardData();

            int ret = elementList.encodeInit(encIter, null, 0);
            assertTrue(ret >= ReactorReturnCodes.SUCCESS);
            encodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
            ret = elementList.encodeComplete(encIter, true);
            assertTrue(ret >= ReactorReturnCodes.SUCCESS);
            requestMsg.encodedDataBody(payload);        
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
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("SYM_LIST"));
            assertEquals(DomainTypes.SYMBOL_LIST, receivedRequestMsg.domainType());        
            providerStreamId = receivedRequestMsg.streamId();

            /* Provider sends refresh .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.SYMBOL_LIST);
            refreshMsg.streamId(providerStreamId);

            refreshMsg.containerType(DataTypes.MAP); 
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("SYM_LIST");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            refreshMsg.applySolicited();
            refreshMsg.applyRefreshComplete();

            Map tempMap = CodecFactory.createMap();
            MapEntry tempMapEntry = CodecFactory.createMapEntry();
            Buffer tempBuffer = CodecFactory.createBuffer();
            EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
            Buffer msgBuf = CodecFactory.createBuffer(); 
            msgBuf.data(ByteBuffer.allocate(2048));

            /* encode message */
            encodeIter.setBufferAndRWFVersion(msgBuf, consumerReactor._reactor.reactorChannel().majorVersion(),
                    consumerReactor._reactor.reactorChannel().minorVersion());
            tempMap.clear();
            tempMap.flags(0);
            tempMap.containerType(DataTypes.NO_DATA);
            tempMap.keyPrimitiveType(DataTypes.BUFFER);
            ret = tempMap.encodeInit(encodeIter, 0, 0);
            assertTrue(ret >= ReactorReturnCodes.SUCCESS);

            /* encode map entry */
            tempMapEntry.clear();
            tempMapEntry.flags(MapEntryFlags.NONE);
            tempBuffer.clear();

            tempBuffer.data("FB.O");
            tempMapEntry.action(MapEntryActions.ADD);
            ret = tempMapEntry.encode(encodeIter, tempBuffer);
            assertTrue(ret >= ReactorReturnCodes.SUCCESS);
            tempBuffer.data("AAPL.O");
            ret = tempMapEntry.encode(encodeIter, tempBuffer);
            assertTrue(ret >= ReactorReturnCodes.SUCCESS);

            ret = tempMap.encodeComplete(encodeIter, true);
            assertTrue(ret >= ReactorReturnCodes.SUCCESS);         

            refreshMsg.encodedDataBody(msgBuf);         
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);       

            // watchlist receives a symbol list data refresh and send out each item request
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
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("SYM_LIST"));
            assertEquals(DomainTypes.SYMBOL_LIST, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.MAP, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

            /* Provider receives requests for FB and AAPL. */
            providerReactor.dispatch(2);

            event = providerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("FB.O"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
            int provFBStreamId = receivedRequestMsg.streamId();

            event = providerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertFalse(receivedRequestMsg.checkNoRefresh());
            assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("AAPL.O"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
            int provAAPLStreamId = receivedRequestMsg.streamId();

            /* Provider sends a refresh for FB with no key. */
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(provFBStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applyRefreshComplete();        
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            refreshMsg.applySolicited();
            refreshMsg.applyRefreshComplete();
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Provider sends a StatusMsg with no key for FB. */
            statusMsg.clear();
            statusMsg.msgClass(MsgClasses.STATUS);
            statusMsg.domainType(DomainTypes.MARKET_PRICE);
            statusMsg.streamId(provFBStreamId);
            statusMsg.containerType(DataTypes.NO_DATA);
            statusMsg.applyHasState();
            statusMsg.state().streamState(streamStates[i]);
            statusMsg.state().dataState(DataStates.SUSPECT);
            assertTrue(provider.submitAndDispatch(statusMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Provider sends a StatusMsg with no key for AAPL. */
            statusMsg.clear();
            statusMsg.msgClass(MsgClasses.STATUS);
            statusMsg.domainType(DomainTypes.MARKET_PRICE);
            statusMsg.streamId(provAAPLStreamId);
            statusMsg.containerType(DataTypes.NO_DATA);
            statusMsg.applyHasState();
            statusMsg.state().streamState(streamStates[i]);
            statusMsg.state().dataState(DataStates.SUSPECT);
            assertTrue(provider.submitAndDispatch(statusMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives messages. */
            consumerReactor.dispatch(3);

            /* Refresh for FB. WL should add key since this is the initial response. */
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("FB.O"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertTrue(receivedRefreshMsg.streamId() < 0);
            int consFBStreamId = receivedRefreshMsg.streamId();

            /* StatusMsg for FB. No key should be present as it was not sent and this is not the initial response. */
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            receivedStatusMsg = (StatusMsg)msgEvent.msg();
            assertFalse(receivedStatusMsg.checkHasMsgKey());
            assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
            if (streamStates[i] == StreamStates.CLOSED_RECOVER)
                assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
            else
                assertEquals(streamStates[i], receivedStatusMsg.state().streamState());
            
            assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertTrue(receivedStatusMsg.streamId() < 0);
            assertEquals(consFBStreamId, receivedStatusMsg.streamId());

            /* StatusMsg for AAPL. WL should add key since this is the initial response. */
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            receivedStatusMsg = (StatusMsg)msgEvent.msg();
            assertTrue(receivedStatusMsg.checkHasMsgKey());
            assertTrue(receivedStatusMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedStatusMsg.msgKey().serviceId());
            assertTrue(receivedStatusMsg.msgKey().checkHasName());
            assertTrue(receivedStatusMsg.msgKey().name().toString().equals("AAPL.O"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
            if (streamStates[i] == StreamStates.CLOSED_RECOVER)
                assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
            else
                assertEquals(streamStates[i], receivedStatusMsg.state().streamState());
            
            assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertTrue(receivedStatusMsg.streamId() < 0);
            
            if (streamStates[i] == StreamStates.CLOSED_RECOVER)
            {
                /* Provider sent ClosedRecover status, so it receives the requests again. */
                providerReactor.dispatch(2);

                event = providerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

                receivedRequestMsg = (RequestMsg)msgEvent.msg();
                assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
                assertTrue(receivedRequestMsg.checkStreaming());
                assertFalse(receivedRequestMsg.checkNoRefresh());
                assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
                assertTrue(receivedRequestMsg.msgKey().name().toString().equals("FB.O"));
                assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        

                event = providerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

                receivedRequestMsg = (RequestMsg)msgEvent.msg();
                assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
                assertTrue(receivedRequestMsg.checkStreaming());
                assertFalse(receivedRequestMsg.checkNoRefresh());
                assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
                assertTrue(receivedRequestMsg.msgKey().name().toString().equals("AAPL.O"));
                assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
            }
                
            TestReactorComponent.closeSession(consumer, provider);
        }
    }    

   /* Used by symbolListDataStreamTest_FromCallback. */
   class SymbolListRequestFromCallbackConsumer extends Consumer
   {
       public SymbolListRequestFromCallbackConsumer(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int reactorChannelEventCallback(ReactorChannelEvent event)
       {
           super.reactorChannelEventCallback(event);
           
           if (event.eventType() == ReactorChannelEventTypes.CHANNEL_OPENED)
           {
               ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
               RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();

               /* Consumer sends symbol list request, which requests data streams. */
               Buffer payload = CodecFactory.createBuffer(); 
               payload.data(ByteBuffer.allocate(1024));

               EncodeIterator encIter = CodecFactory.createEncodeIterator();
               encIter.clear();
               encIter.setBufferAndRWFVersion(payload, reactorChannel().majorVersion(),
                       reactorChannel().minorVersion());

               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(5);
               requestMsg.domainType(DomainTypes.SYMBOL_LIST);	       
               requestMsg.containerType(DataTypes.ELEMENT_LIST);	       
               requestMsg.applyHasQos();
               requestMsg.qos().rate(QosRates.TICK_BY_TICK);
               requestMsg.qos().timeliness(QosTimeliness.REALTIME);
               requestMsg.applyHasPriority();
               requestMsg.priority().priorityClass(1);
               requestMsg.priority().count(1);	      
               requestMsg.applyStreaming();
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("SYM_LIST");

               ElementList elementList = CodecFactory.createElementList();
               elementList.clear();
               elementList.applyHasStandardData();

               int ret = elementList.encodeInit(encIter, null, 0);
               assertTrue(ret >= ReactorReturnCodes.SUCCESS);
               encodeSymbolListBehaviorsElement(encIter, SymbolList.SymbolListDataStreamRequestFlags.SYMBOL_LIST_DATA_STREAMS);
               ret = elementList.encodeComplete(encIter, true);
               assertTrue(ret >= ReactorReturnCodes.SUCCESS);
               requestMsg.encodedDataBody(payload);        
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

           }
           
           return ReactorReturnCodes.SUCCESS;
       }
   }

   @Test
   public void symbolListDataStreamTest_FromChannelOpenCallback()
   {
       /* Test requesting a symbol list with data streams from the channel-open callback. */

       ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
       TestReactorEvent event;
       ReactorMsgEvent msgEvent;
       Msg msg = CodecFactory.createMsg();
       RequestMsg receivedRequestMsg;
       RefreshMsg refreshMsg = (RefreshMsg)msg;
       RefreshMsg receivedRefreshMsg;
       int providerStreamId;
       int ret;

       /* Create reactors. */
       TestReactor consumerReactor = new TestReactor();
       TestReactor providerReactor = new TestReactor();

       /* Create consumer. */
       Consumer consumer = new SymbolListRequestFromCallbackConsumer(consumerReactor);
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
       consumerRole.watchlistOptions().requestTimeout(3000);

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
       opts.reconnectAttemptLimit(-1);
       opts.numStatusEvents(1);
       provider.bind(opts);
       TestReactor.openSession(consumer, provider, opts);

       /* Provider receives symbol list request. */
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
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("SYM_LIST"));
       assertEquals(DomainTypes.SYMBOL_LIST, receivedRequestMsg.domainType());        
       providerStreamId = receivedRequestMsg.streamId();

       /* Provider sends refresh .*/
       refreshMsg.clear();
       refreshMsg.msgClass(MsgClasses.REFRESH);
       refreshMsg.domainType(DomainTypes.SYMBOL_LIST);
       refreshMsg.streamId(providerStreamId);

       refreshMsg.containerType(DataTypes.MAP); 
       refreshMsg.applyHasMsgKey();
       refreshMsg.msgKey().applyHasServiceId();
       refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
       refreshMsg.msgKey().applyHasName();
       refreshMsg.msgKey().name().data("SYM_LIST");
       refreshMsg.state().streamState(StreamStates.OPEN);
       refreshMsg.state().dataState(DataStates.OK);

       Map tempMap = CodecFactory.createMap();
       MapEntry tempMapEntry = CodecFactory.createMapEntry();
       Buffer tempBuffer = CodecFactory.createBuffer();
       EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
       Buffer msgBuf = CodecFactory.createBuffer(); 
       msgBuf.data(ByteBuffer.allocate(2048));

       /* encode message */
       encodeIter.setBufferAndRWFVersion(msgBuf, consumerReactor._reactor.reactorChannel().majorVersion(),
               consumerReactor._reactor.reactorChannel().minorVersion());
       tempMap.clear();
       tempMap.flags(0);
       tempMap.containerType(DataTypes.NO_DATA);
       tempMap.keyPrimitiveType(DataTypes.BUFFER);
       ret = tempMap.encodeInit(encodeIter, 0, 0);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);

       /* encode map entry */
       tempMapEntry.clear();
       tempMapEntry.flags(MapEntryFlags.NONE);
       tempBuffer.clear();

       tempBuffer.data("FB.O");
       tempMapEntry.action(MapEntryActions.ADD);
       ret = tempMapEntry.encode(encodeIter, tempBuffer);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);
       tempBuffer.data("AAPL.O");
       ret = tempMapEntry.encode(encodeIter, tempBuffer);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);

       ret = tempMap.encodeComplete(encodeIter, true);
       assertTrue(ret >= ReactorReturnCodes.SUCCESS);         

       refreshMsg.encodedDataBody(msgBuf);         
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);       

       // watchlist receives a symbol list data refresh and send out each item request
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
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("SYM_LIST"));
       assertEquals(DomainTypes.SYMBOL_LIST, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.MAP, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

       /* Provider receives requests for FB and AAPL. */
       providerReactor.dispatch(2);

       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertTrue(receivedRequestMsg.checkStreaming());
       assertFalse(receivedRequestMsg.checkNoRefresh());
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("FB.O"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
       int provFBStreamId = receivedRequestMsg.streamId();

       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertTrue(receivedRequestMsg.checkStreaming());
       assertFalse(receivedRequestMsg.checkNoRefresh());
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("AAPL.O"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());        
       int provAAPLStreamId = receivedRequestMsg.streamId();

       /* Provider sends refreshes for FB and AAPL .*/
       refreshMsg.clear();
       refreshMsg.msgClass(MsgClasses.REFRESH);
       refreshMsg.domainType(DomainTypes.MARKET_PRICE);
       refreshMsg.streamId(provFBStreamId);
       refreshMsg.containerType(DataTypes.NO_DATA);
       refreshMsg.applyHasMsgKey();
       refreshMsg.msgKey().applyHasServiceId();
       refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
       refreshMsg.msgKey().applyHasName();
       refreshMsg.msgKey().name().data("FB.O");
       refreshMsg.applyRefreshComplete();        
       refreshMsg.state().streamState(StreamStates.OPEN);
       refreshMsg.state().dataState(DataStates.OK);
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       refreshMsg.clear();
       refreshMsg.msgClass(MsgClasses.REFRESH);
       refreshMsg.domainType(DomainTypes.MARKET_PRICE);
       refreshMsg.streamId(provAAPLStreamId);
       refreshMsg.containerType(DataTypes.NO_DATA);
       refreshMsg.applyHasMsgKey();
       refreshMsg.msgKey().applyHasServiceId();
       refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
       refreshMsg.msgKey().applyHasName();
       refreshMsg.msgKey().name().data("AAPL.O");
       refreshMsg.applyRefreshComplete();        
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
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("FB.O"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertTrue(receivedRefreshMsg.streamId() < 0);

       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertTrue(receivedRefreshMsg.checkHasMsgKey());
       assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
       assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
       assertTrue(receivedRefreshMsg.msgKey().checkHasName());
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("AAPL.O"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertTrue(receivedRefreshMsg.streamId() < 0);

       TestReactorComponent.closeSession(consumer, provider);
   }    

   @Test
   public void singleOpenZeroOpenCallbackSubmitRecoverTest()
   {
       TestReactorEvent event;
               
       /* Create reactors. */
       TestReactor consumerReactor = new TestReactor();
       TestReactor providerReactor = new TestReactor();
               
       /* Create consumer with SingleOpen and AllowSuspect of 0. */
       Consumer consumer = new SendItemsFromOpenCallbackConsumer(consumerReactor, false);
       ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
       consumerRole.initDefaultRDMLoginRequest();
       consumerRole.rdmLoginRequest().attrib().applyHasSingleOpen();
       consumerRole.rdmLoginRequest().attrib().singleOpen(0);
       consumerRole.rdmLoginRequest().attrib().applyHasAllowSuspectData();
       consumerRole.rdmLoginRequest().attrib().allowSuspectData(0);
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
       opts.numStatusEvents(1); // set number of expected status message from request submitted in channel open callback
       TestReactor.openSession(consumer, provider, opts);
       
       /* Provider should never receive request since no single open. */
       providerReactor.dispatch(0);
       event = providerReactor.pollEvent();
       assertEquals(event, null);
       
       TestReactorComponent.closeSession(consumer, provider);
   }
   
   @Test
   public void itemPauseResumeTest()
   {
       ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
       TestReactorEvent event;
       ReactorMsgEvent msgEvent;
       Msg msg = CodecFactory.createMsg();
       RequestMsg requestMsg1 = (RequestMsg)CodecFactory.createMsg();
       RequestMsg requestMsg2 = (RequestMsg)CodecFactory.createMsg();
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
       
       // submit two aggregated request messages
       requestMsg1.clear();
       requestMsg1.msgClass(MsgClasses.REQUEST);
       requestMsg1.streamId(5);
       requestMsg1.domainType(DomainTypes.MARKET_PRICE);
       requestMsg1.applyStreaming();
       requestMsg1.msgKey().applyHasName();
       requestMsg1.msgKey().name().data("LUV.N");

       requestMsg1.applyHasPriority();
       requestMsg1.priority().count(11);
       requestMsg1.priority().priorityClass(22);
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       assertTrue(consumer.submitAndDispatch(requestMsg1, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       requestMsg2.clear();
       requestMsg2.msgClass(MsgClasses.REQUEST);
       requestMsg2.streamId(6);
       requestMsg2.domainType(DomainTypes.MARKET_PRICE);
       requestMsg2.applyStreaming();
       requestMsg2.msgKey().applyHasName();
       requestMsg2.msgKey().name().data("LUV.N");

       requestMsg2.applyHasPriority();
       requestMsg2.priority().count(10);
       requestMsg2.priority().priorityClass(22);
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       assertTrue(consumer.submitAndDispatch(requestMsg2, submitOptions) >= ReactorReturnCodes.SUCCESS);
              
       /* Provider receives requests. */
       providerReactor.dispatch(2);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertEquals(3, receivedRequestMsg.streamId());
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertTrue(receivedRequestMsg.checkStreaming());
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertTrue(receivedRequestMsg.msgKey().checkHasName());
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("LUV.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(11, receivedRequestMsg.priority().count());
       assertEquals(22, receivedRequestMsg.priority().priorityClass());
       
       providerStreamId = receivedRequestMsg.streamId();
       
       // 2nd request 
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertEquals(3, receivedRequestMsg.streamId());
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertTrue(receivedRequestMsg.checkStreaming());
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertTrue(receivedRequestMsg.msgKey().checkHasName());
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("LUV.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(21, receivedRequestMsg.priority().count());
       assertEquals(22, receivedRequestMsg.priority().priorityClass());
       
       providerStreamId = receivedRequestMsg.streamId();
                     
       /* Provider sends refresh .*/
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
       refreshMsg.msgKey().name().data("LUV.N");
       refreshMsg.state().streamState(StreamStates.OPEN);
       refreshMsg.state().dataState(DataStates.OK);
       
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
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
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       
       // 2nd event to consumer
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertTrue(receivedRefreshMsg.checkHasMsgKey());
       assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
       assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
       assertTrue(receivedRefreshMsg.msgKey().checkHasName());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));       
       
       // consumer reissues 2nd request with pause
       requestMsg2.applyPause();
       assertTrue(consumer.submitAndDispatch(requestMsg2, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       // consumer reissues 1st request with pause
       requestMsg1.applyPause();
       assertTrue(consumer.submitAndDispatch(requestMsg1, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Provider receives pause request. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertEquals(3, receivedRequestMsg.streamId()); // stream id should be same as first request
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertTrue(receivedRequestMsg.checkStreaming()); 
       // PAUSE
       assertTrue(receivedRequestMsg.checkPause()); 
       providerStreamId = receivedRequestMsg.streamId();       
       
       // resume
       // consumer reissues 1st request with no pause
       requestMsg1.flags(requestMsg1.flags() & ~RequestMsgFlags.PAUSE);
       assertTrue(consumer.submitAndDispatch(requestMsg1, submitOptions) >= ReactorReturnCodes.SUCCESS);
              
       /* Provider receives request. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertEquals(3, receivedRequestMsg.streamId()); // stream id should be same as first request
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertTrue(receivedRequestMsg.checkStreaming());
       // NO PAUSE now
       assertFalse(receivedRequestMsg.checkPause()); 
       providerStreamId = receivedRequestMsg.streamId();
        
       TestReactorComponent.closeSession(consumer, provider);
   }
    
   @Test
   public void loginPauseResumeTest()
   {
       ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
       TestReactorEvent event;
       ReactorMsgEvent msgEvent;
       Msg msg = CodecFactory.createMsg();
       RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
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
       
       // submit a item request messages
       requestMsg.clear();
       requestMsg.msgClass(MsgClasses.REQUEST);
       requestMsg.streamId(5);
       requestMsg.domainType(DomainTypes.MARKET_PRICE);
       requestMsg.applyStreaming();
       requestMsg.msgKey().applyHasName();
       requestMsg.msgKey().name().data("JPM.N");

       requestMsg.applyHasPriority();
       requestMsg.priority().count(11);
       requestMsg.priority().priorityClass(22);
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
                     
       /* Provider receives requests. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertEquals(3, receivedRequestMsg.streamId());
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertTrue(receivedRequestMsg.checkStreaming());
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertTrue(receivedRequestMsg.msgKey().checkHasName());
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("JPM.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(11, receivedRequestMsg.priority().count());
       assertEquals(22, receivedRequestMsg.priority().priorityClass());
       providerStreamId = receivedRequestMsg.streamId();
                            
       /* Provider sends refresh .*/
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
       refreshMsg.msgKey().name().data("JPM.N");
       refreshMsg.state().streamState(StreamStates.OPEN);
       refreshMsg.state().dataState(DataStates.OK);
       
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
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
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       
       // consumer reissues login with pause
       LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
       assertNotNull(loginRequest);
       loginRequest.applyPause();
       loginRequest.applyNoRefresh();
       submitOptions.clear();
       assertTrue(consumer.submitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Provider receives request. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertEquals(1, receivedRequestMsg.streamId()); // stream id should be same as first request
       // Login paused and item would be paused too
       
       // check that login handler is awaiting resume all
       assertTrue(consumerReactor._componentList.get(0).reactorChannel().watchlist().loginHandler()._awaitingResumeAll);
       // all items should be paused
       int pausedCount = consumerReactor._componentList.get(0).reactorChannel().watchlist().itemHandler()._streamList.get(0).numPausedRequestsCount();
       int itemCount = consumerReactor._componentList.get(0).reactorChannel().watchlist().itemHandler()._streamList.get(0).userRequestList().size();
       assertEquals(1, pausedCount);
       assertEquals(1, itemCount);
      
       // consumer reissues login with resume
       loginRequest.flags(loginRequest.flags() &~LoginRequestFlags.PAUSE_ALL);
       submitOptions.clear();
       assertTrue(consumer.submitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Provider receives request. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertEquals(1, receivedRequestMsg.streamId()); // stream id should be same as first request
       
       // check that login handler is not awaiting resume all
       assertFalse(consumerReactor._componentList.get(0).reactorChannel().watchlist().loginHandler()._awaitingResumeAll);
       // all items should be resumed
       pausedCount = consumerReactor._componentList.get(0).reactorChannel().watchlist().itemHandler()._streamList.get(0).numPausedRequestsCount();
       itemCount = consumerReactor._componentList.get(0).reactorChannel().watchlist().itemHandler()._streamList.get(0).userRequestList().size();
 
       assertEquals(0, pausedCount);
       assertEquals(1, itemCount);
       
       TestReactorComponent.closeSession(consumer, provider);
   }   
 
   @Test
   public void loginPauseResumeTokenTest()
   {
       ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
       TestReactorEvent event;
       ReactorMsgEvent msgEvent;
       Msg msg = CodecFactory.createMsg();
       RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
       RequestMsg receivedRequestMsg;
       RefreshMsg refreshMsg = (RefreshMsg)msg;
       RefreshMsg receivedRefreshMsg;
       int providerStreamId;
       String authenticationToken1 = "authenticationToken1";
       String authenticationToken2 = "authenticationToken2";
               
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
       
       /* add authToken to login request */
       consumerRole.rdmLoginRequest().userNameType(Login.UserIdTypes.AUTHN_TOKEN);
       Buffer authTokenBuffer = CodecFactory.createBuffer();
       authTokenBuffer.data(authenticationToken1);
       consumerRole.rdmLoginRequest().userName(authTokenBuffer);

       /* Connect the consumer and provider. Setup login & directory streams automatically. */
       ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
       opts.setupDefaultLoginStream(true);
       opts.setupDefaultDirectoryStream(true);
       provider.bind(opts);
       TestReactor.openSession(consumer, provider, opts);
       
       // submit a item request messages
       requestMsg.clear();
       requestMsg.msgClass(MsgClasses.REQUEST);
       requestMsg.streamId(5);
       requestMsg.domainType(DomainTypes.MARKET_PRICE);
       requestMsg.applyStreaming();
       requestMsg.msgKey().applyHasName();
       requestMsg.msgKey().name().data("JPM.N");

       requestMsg.applyHasPriority();
       requestMsg.priority().count(11);
       requestMsg.priority().priorityClass(22);
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
                     
       /* Provider receives requests. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertEquals(3, receivedRequestMsg.streamId());
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertTrue(receivedRequestMsg.checkStreaming());
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertTrue(receivedRequestMsg.msgKey().checkHasName());
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("JPM.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(11, receivedRequestMsg.priority().count());
       assertEquals(22, receivedRequestMsg.priority().priorityClass());
       providerStreamId = receivedRequestMsg.streamId();
                            
       /* Provider sends refresh .*/
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
       refreshMsg.msgKey().name().data("JPM.N");
       refreshMsg.state().streamState(StreamStates.OPEN);
       refreshMsg.state().dataState(DataStates.OK);
       
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
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
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       
       // consumer reissues login with pause
       LoginRequest  loginRequest = consumerRole.rdmLoginRequest();
       assertNotNull(loginRequest);
       loginRequest.applyPause();
       loginRequest.applyNoRefresh();
       submitOptions.clear();
       assertTrue(consumer.submitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Provider receives request. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertEquals(1, receivedRequestMsg.streamId()); // stream id should be same as first request
       // Login paused and item would be paused too
       
       // check that login handler is awaiting resume all
       assertTrue(consumerReactor._componentList.get(0).reactorChannel().watchlist().loginHandler()._awaitingResumeAll);
       // all items should be paused
       int pausedCount = consumerReactor._componentList.get(0).reactorChannel().watchlist().itemHandler()._streamList.get(0).numPausedRequestsCount();
       int itemCount = consumerReactor._componentList.get(0).reactorChannel().watchlist().itemHandler()._streamList.get(0).userRequestList().size();
       assertEquals(1, pausedCount);
       assertEquals(1, itemCount);
      
       // consumer reissues login with resume with token change
       authTokenBuffer.data(authenticationToken2);
       loginRequest.userName(authTokenBuffer);
       loginRequest.flags(loginRequest.flags() &~LoginRequestFlags.PAUSE_ALL);
       submitOptions.clear();
       assertTrue(consumer.submitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Provider receives request. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertEquals(1, receivedRequestMsg.streamId()); // stream id should be same as first request
       
       // check that login handler is still awaiting resume all since token change doesn't result in resume
       assertTrue(consumerReactor._componentList.get(0).reactorChannel().watchlist().loginHandler()._awaitingResumeAll);
       // all items should still be paused
       pausedCount = consumerReactor._componentList.get(0).reactorChannel().watchlist().itemHandler()._streamList.get(0).numPausedRequestsCount();
       itemCount = consumerReactor._componentList.get(0).reactorChannel().watchlist().itemHandler()._streamList.get(0).userRequestList().size();
       assertEquals(1, pausedCount);
       assertEquals(1, itemCount);

       // consumer reissues login with resume again with same token
       loginRequest.flags(loginRequest.flags() &~LoginRequestFlags.PAUSE_ALL);
       submitOptions.clear();
       assertTrue(consumer.submitAndDispatch(loginRequest, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Provider receives request. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertEquals(1, receivedRequestMsg.streamId()); // stream id should be same as first request
       
       // check that login handler is not awaiting resume all
       assertFalse(consumerReactor._componentList.get(0).reactorChannel().watchlist().loginHandler()._awaitingResumeAll);
       // all items should be resumed
       pausedCount = consumerReactor._componentList.get(0).reactorChannel().watchlist().itemHandler()._streamList.get(0).numPausedRequestsCount();
       itemCount = consumerReactor._componentList.get(0).reactorChannel().watchlist().itemHandler()._streamList.get(0).userRequestList().size();
 
       assertEquals(0, pausedCount);
       assertEquals(1, itemCount);
       
       TestReactorComponent.closeSession(consumer, provider);
   }   
 
   @Test
   public void itemDoubleCloseTest()
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
       CloseMsg closeMsg = (CloseMsg)msg;
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
       consumerRole.watchlistOptions().requestTimeout(3000);
       
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
       refreshMsg.applyRefreshComplete();
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
       
       /* Consumer sends close. */
       closeMsg.clear();
       closeMsg.msgClass(MsgClasses.CLOSE);
       closeMsg.streamId(5);
       closeMsg.domainType(DomainTypes.MARKET_PRICE);
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       assertTrue(consumer.submitAndDispatch(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       /* Consumer sends close again. */
       closeMsg.clear();
       closeMsg.msgClass(MsgClasses.CLOSE);
       closeMsg.streamId(5);
       closeMsg.domainType(DomainTypes.MARKET_PRICE);
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       assertTrue(consumer.submitAndDispatch(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
   }
   
   /* Used by serviceDownCloseItemRecoverTest. */
   class CloseUserRequestFromDirectoryCallbackConsumer extends Consumer
   {
       public CloseUserRequestFromDirectoryCallbackConsumer(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
       {
           super.rdmDirectoryMsgCallback(event);
           
           // close user stream 7
           CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
           closeMsg.clear();
           closeMsg.msgClass(MsgClasses.CLOSE);
           closeMsg.streamId(7);
           closeMsg.domainType(DomainTypes.MARKET_PRICE);
           closeMsg.containerType(DataTypes.NO_DATA); 

           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
           if ( (event.reactorChannel().submit(closeMsg, submitOptions, errorInfo)) !=  CodecReturnCodes.SUCCESS)
           {
               assertTrue("rdmDirectoryMsgCallback() submit close failed", false);
           }
           
           return ReactorReturnCodes.SUCCESS;
       }
   }

   @Test
   public void serviceDownCloseItemRecoverTest()
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
       UpdateMsg receivedUpdateMsg;
       StatusMsg receivedStatusMsg;
       int providerStreamId;
       WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
       testUserSpecObj.value(997);
               
       /* Create reactors. */
       TestReactor consumerReactor = new TestReactor();
       TestReactor providerReactor = new TestReactor();
               
       /* Create consumer. */
       Consumer consumer = new CloseUserRequestFromDirectoryCallbackConsumer(consumerReactor);
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
       consumerRole.watchlistOptions().requestTimeout(3000);
       
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
       submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
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
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().count());
       
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
       refreshMsg.applyRefreshComplete();
       Buffer groupId = CodecFactory.createBuffer();
       groupId.data("1234431");
       refreshMsg.groupId(groupId);
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
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       
       /* Consumer sends request. */
       requestMsg.clear();
       requestMsg.msgClass(MsgClasses.REQUEST);
       requestMsg.streamId(6);
       requestMsg.domainType(DomainTypes.MARKET_PRICE);
       requestMsg.applyStreaming();
       requestMsg.msgKey().applyHasName();
       requestMsg.msgKey().name().data("IBM.N");
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
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
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().count());
       
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
       refreshMsg.msgKey().name().data("IBM.N");
       refreshMsg.applyRefreshComplete();
       groupId.data("1234431");
       refreshMsg.groupId(groupId);
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
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("IBM.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

       /* Consumer sends request. */
       requestMsg.clear();
       requestMsg.msgClass(MsgClasses.REQUEST);
       requestMsg.streamId(7);
       requestMsg.domainType(DomainTypes.MARKET_PRICE);
       requestMsg.applyStreaming();
       requestMsg.msgKey().applyHasName();
       requestMsg.msgKey().name().data("TRI.N");
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
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
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(2, receivedRequestMsg.priority().count());
       
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
       refreshMsg.applyRefreshComplete();
       groupId.data("1234431");
       refreshMsg.groupId(groupId);
       refreshMsg.state().streamState(StreamStates.OPEN);
       refreshMsg.state().dataState(DataStates.OK);
       
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Consumer receives 2 refreshes - one for each TRI.N. */
       consumerReactor.dispatch(2);
       // first refresh
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
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       // second refresh
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
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

       /* Provider sends service update to bring service down.*/
       DirectoryUpdate directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
       directoryUpdateMsg.clear();
       directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
       directoryUpdateMsg.streamId(2);
       directoryUpdateMsg.applyHasFilter();
       directoryUpdateMsg.filter(Directory.ServiceFilterFlags.STATE);
       
       WlService wlService = new WlService();
       wlService.rdmService().applyHasState();
       wlService.rdmService().action(MapEntryActions.UPDATE);
       wlService.rdmService().state().applyHasStatus();
       wlService.rdmService().state().action(FilterEntryActions.SET);
       wlService.rdmService().state().status().dataState(DataStates.SUSPECT);
       wlService.rdmService().state().status().streamState(StreamStates.CLOSED_RECOVER);
       wlService.rdmService().state().applyHasAcceptingRequests();
       wlService.rdmService().state().acceptingRequests(1);
       wlService.rdmService().state().serviceState(0);
       wlService.rdmService().serviceId(1);
       
       directoryUpdateMsg.serviceList().add(wlService.rdmService());

       assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Consumer receives directory update. Consumer receives status messages for 5, 7, and 6.
        * Consumer receives status for 5 and 6 again (due to recovery attempt), but not 7 since it was closed during rdmDirectoryMsgCallback. */
       consumerReactor.dispatch(6);
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedStatusMsg = (StatusMsg)msgEvent.msg();
       assertEquals(5, receivedStatusMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
       assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedStatusMsg = (StatusMsg)msgEvent.msg();
       assertEquals(7, receivedStatusMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
       assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedStatusMsg = (StatusMsg)msgEvent.msg();
       assertEquals(6, receivedStatusMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
       assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
       
       receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
       assertTrue(receivedUpdateMsg.checkHasMsgKey());
       assertEquals(DomainTypes.SOURCE, receivedUpdateMsg.domainType());
       assertEquals(DataTypes.MAP, receivedUpdateMsg.containerType());
       
       assertEquals(0, consumerReactor._componentList.get(0).reactorChannel().watchlist().directoryHandler().service(1).streamList().size());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedStatusMsg = (StatusMsg)msgEvent.msg();
       assertEquals(5, receivedStatusMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
       assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedStatusMsg = (StatusMsg)msgEvent.msg();
       assertEquals(6, receivedStatusMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
       assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       /* Provider sends service update to bring service back up.*/
       wlService.rdmService().state().status().dataState(DataStates.OK);
       wlService.rdmService().state().status().streamState(StreamStates.OPEN);
       wlService.rdmService().state().serviceState(1);
       assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       // dispatch service update sent to consumer
       consumerReactor.dispatch(1);
       event = consumerReactor.pollEvent();

       /* Provider receives 2 requests for recovery (stream ids 5 and 6). */
       providerReactor.dispatch(2);
       
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
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().count());

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
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().count());
       
       TestReactorComponent.closeSession(consumer, provider);
   }

   @Test
   public void serviceDownOpenItemRecoverTest()
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
       WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
       testUserSpecObj.value(997);
               
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
       consumerRole.watchlistOptions().requestTimeout(3000);
       
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
       submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
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
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().count());
       
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
       refreshMsg.applyRefreshComplete();
       Buffer groupId = CodecFactory.createBuffer();
       groupId.data("1234431");
       refreshMsg.groupId(groupId);
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
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       /* Consumer sends request. */
       requestMsg.clear();
       requestMsg.msgClass(MsgClasses.REQUEST);
       requestMsg.streamId(6);
       requestMsg.domainType(DomainTypes.MARKET_PRICE);
       requestMsg.applyStreaming();
       requestMsg.msgKey().applyHasName();
       requestMsg.msgKey().name().data("IBM.N");
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
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
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().count());
       
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
       refreshMsg.msgKey().name().data("IBM.N");
       refreshMsg.applyRefreshComplete();
       groupId.data("1234431");
       refreshMsg.groupId(groupId);
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
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("IBM.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

       /* Consumer sends request. */
       requestMsg.clear();
       requestMsg.msgClass(MsgClasses.REQUEST);
       requestMsg.streamId(7);
       requestMsg.domainType(DomainTypes.MARKET_PRICE);
       requestMsg.applyStreaming();
       requestMsg.msgKey().applyHasName();
       requestMsg.msgKey().name().data("TRI.N");
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
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
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(2, receivedRequestMsg.priority().count());
       
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
       refreshMsg.applyRefreshComplete();
       groupId.data("1234431");
       refreshMsg.groupId(groupId);
       refreshMsg.state().streamState(StreamStates.OPEN);
       refreshMsg.state().dataState(DataStates.OK);
       
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Consumer receives 2 refreshes - one for each TRI.N. */
       consumerReactor.dispatch(2);
       // first refresh
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
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       // second refresh
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
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

       /* Provider sends service update to bring service down.*/
       DirectoryUpdate directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
       directoryUpdateMsg.clear();
       directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
       directoryUpdateMsg.streamId(2);
       directoryUpdateMsg.applyHasFilter();
       directoryUpdateMsg.filter(Directory.ServiceFilterFlags.STATE);
       
       WlService wlService = new WlService();
       wlService.rdmService().applyHasState();
       wlService.rdmService().action(MapEntryActions.UPDATE);
       wlService.rdmService().state().applyHasStatus();
       wlService.rdmService().state().action(FilterEntryActions.SET);
       wlService.rdmService().state().status().dataState(DataStates.OK);
       wlService.rdmService().state().status().streamState(StreamStates.OPEN);
       wlService.rdmService().state().applyHasAcceptingRequests();
       wlService.rdmService().state().acceptingRequests(0);
       wlService.rdmService().state().serviceState(1);
       wlService.rdmService().serviceId(1);
       
       directoryUpdateMsg.serviceList().add(wlService.rdmService());

       assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Consumer receives update. Consumer receives 3 updates for stream id 5, 7 and 6 plus service update. */
       consumerReactor.dispatch(4);
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(5, receivedRefreshMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(7, receivedRefreshMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(6, receivedRefreshMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertTrue(receivedRefreshMsg.checkHasMsgKey());
       assertEquals(DomainTypes.SOURCE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.MAP, receivedRefreshMsg.containerType());
       
       // open 2 new items - one existing name and one new name 
       requestMsg.clear();
       requestMsg.msgClass(MsgClasses.REQUEST);
       requestMsg.streamId(8);
       requestMsg.domainType(DomainTypes.MARKET_PRICE);
       requestMsg.applyStreaming();
       requestMsg.msgKey().applyHasName();
       requestMsg.msgKey().name().data("IBM.N");
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
       assertTrue(consumer.submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       requestMsg.clear();
       requestMsg.msgClass(MsgClasses.REQUEST);
       requestMsg.streamId(9);
       requestMsg.domainType(DomainTypes.MARKET_PRICE);
       requestMsg.applyStreaming();
       requestMsg.msgKey().applyHasName();
       requestMsg.msgKey().name().data("MSI.N");
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
       assertTrue(consumer.submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       // provider should receive no request since service is not accepting requests
       providerReactor.dispatch(0);
       
       /* Provider sends service update to bring service back up.*/
       wlService.rdmService().state().acceptingRequests(1);
       assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Consumer receives update. Consumer receives 2 open/suspect updates for stream ids 8 and 9, 6 open/ok updates for stream id 5, 6, 7, 8 and 9 plus 1 service update. */
       consumerReactor.dispatch(8);
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(8, receivedRefreshMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.SUSPECT, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(9, receivedRefreshMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.SUSPECT, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(5, receivedRefreshMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(7, receivedRefreshMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(6, receivedRefreshMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(8, receivedRefreshMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(9, receivedRefreshMsg.streamId());
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
       assertNotNull(msgEvent.streamInfo().userSpecObject());
       assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
       
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
       
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertTrue(receivedRefreshMsg.checkHasMsgKey());
       assertEquals(DomainTypes.SOURCE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.MAP, receivedRefreshMsg.containerType());
       
       // provider should now receive two requests for 2 new items submitted when service was down
       providerReactor.dispatch(2);
       
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
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(2, receivedRequestMsg.priority().count());

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
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("MSI.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().count());
       
       TestReactorComponent.closeSession(consumer, provider);
   }

   @Test
   public void closeConsumerChannelTest()
   {
       /* Test opening some streams and then closing the channel from the consumer side. */

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
       
       /* Close consumer channel. */
       consumer.close();

       /* Provider receives channel-down event. */
       providerReactor.dispatch(2);
       

       RDMDirectoryMsgEvent directoryMsgEvent;                
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
       directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
       assertEquals(provider.defaultSessionDirectoryStreamId(), directoryMsgEvent.rdmDirectoryMsg().streamId());
       assertEquals(DirectoryMsgType.CLOSE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());   
   
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
       ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
       assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());

       provider.close();
       consumerReactor.close();
       providerReactor.close();
   }
   
   /* Used by submitPostOnItemRefeshTest. */
   class PostFromDefaultMsgCallbackConsumer extends Consumer
   {
       public PostFromDefaultMsgCallbackConsumer(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int defaultMsgCallback(ReactorMsgEvent event)
       {
           super.defaultMsgCallback(event);
           
           Msg msg = event.msg();
           
           switch (msg.msgClass())
           {
               case MsgClasses.REFRESH:
                   // send post message
                   PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
                   postMsg.clear();
                   postMsg.msgClass(MsgClasses.POST);
                   postMsg.streamId(msg.streamId());
                   postMsg.domainType(msg.domainType());
                   postMsg.containerType(DataTypes.NO_DATA); 

                   ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
                   ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
                   if ((event.reactorChannel().submit(postMsg, submitOptions, errorInfo)) !=  CodecReturnCodes.SUCCESS)
                   {
                       assertTrue("defaultMsgCallback() submit post failed", false);
                   }
                   break;
               default:
                   break;
           }
           
           return ReactorReturnCodes.SUCCESS;
       }
   }

   @Test
   public void submitPostOnItemRefeshTest()
   {
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
       Consumer consumer = new PostFromDefaultMsgCallbackConsumer(consumerReactor);
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
       consumerRole.watchlistOptions().requestTimeout(3000);
       
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
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().count());
       
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
       refreshMsg.applyRefreshComplete();
       Buffer groupId = CodecFactory.createBuffer();
       groupId.data("1234431");
       refreshMsg.groupId(groupId);
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
              
       /* Provider receives post. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       
       TestReactorComponent.closeSession(consumer, provider);
   }

   
   /* Used by submitOffstreamPostOnItemRefeshTest. */
   class OffPostFromDefaultMsgCallbackConsumer extends Consumer
   {
       public OffPostFromDefaultMsgCallbackConsumer(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int defaultMsgCallback(ReactorMsgEvent event)
       {
           super.defaultMsgCallback(event);
           
           Msg msg = event.msg();
           
           switch (msg.msgClass())
           {
               case MsgClasses.REFRESH:
                   // if solicited, send off stream post message with refresh message as payload
                   if ((msg.flags() & RefreshMsgFlags.SOLICITED) > 0)
                   {
                       PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
                       postMsg.clear();
                       postMsg.msgClass(MsgClasses.POST);
                       postMsg.streamId(1);
                       postMsg.domainType(msg.domainType());
                       postMsg.containerType(DataTypes.MSG);
                       postMsg.applyAck();
                       postMsg.applyHasPostId();
                       postMsg.postId(1);
                       postMsg.applyHasSeqNum();
                       postMsg.seqNum(1);
                       postMsg.applyHasMsgKey();
                       postMsg.msgKey().applyHasServiceId();
                       postMsg.msgKey().applyHasName();
                       postMsg.msgKey().applyHasNameType();
                       postMsg.msgKey().serviceId(msg.msgKey().serviceId());
                       postMsg.msgKey().nameType(1);
                       postMsg.msgKey().name(msg.msgKey().name());
                       
                       msg.flags(msg.flags() & ~RefreshMsgFlags.SOLICITED);
                       
                       EncodeIterator eIter = CodecFactory.createEncodeIterator();
                       Buffer buffer = CodecFactory.createBuffer();
                       buffer.data(ByteBuffer.allocate(1024));
                       eIter.setBufferAndRWFVersion(buffer, event.reactorChannel().majorVersion(), event.reactorChannel().minorVersion());
                       // reset all flags and set stream id to 0 to simulate bad refresh message
                       msg.flags(0);
                       msg.streamId(0);
                       msg.encode(eIter);
                       postMsg.encodedDataBody(buffer);
    
                       ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
                       ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
                       if ((event.reactorChannel().submit(postMsg, submitOptions, errorInfo)) !=  CodecReturnCodes.SUCCESS)
                       {
                           assertTrue("defaultMsgCallback() submit post failed", false);
                       }
                   }
                   break;
               default:
                   break;
           }
           
           return ReactorReturnCodes.SUCCESS;
       }
   }

   @Test
   public void submitOffstreamPostOnItemRefeshTest()
   {
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
       Consumer consumer = new OffPostFromDefaultMsgCallbackConsumer(consumerReactor);
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
       consumerRole.watchlistOptions().requestTimeout(3000);
       
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
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().count());
       
       providerStreamId = receivedRequestMsg.streamId();
       
       /* Provider sends refresh .*/
       refreshMsg.clear();
       refreshMsg.msgClass(MsgClasses.REFRESH);
       refreshMsg.applySolicited();
       refreshMsg.domainType(DomainTypes.MARKET_PRICE);
       refreshMsg.streamId(providerStreamId);
       refreshMsg.containerType(DataTypes.NO_DATA);
       refreshMsg.applyHasMsgKey();
       refreshMsg.msgKey().applyHasServiceId();
       refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
       refreshMsg.msgKey().applyHasName();
       refreshMsg.msgKey().name().data("TRI.N");
       refreshMsg.applyRefreshComplete();
       Buffer groupId = CodecFactory.createBuffer();
       groupId.data("1234431");
       refreshMsg.groupId(groupId);
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
              
       /* Provider receives offstream post. */
       PostMsg offstreamPost;
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.POST, msgEvent.msg().msgClass());
       assertEquals(consumerRole.rdmLoginRequest().streamId(), msgEvent.msg().streamId());
       offstreamPost = (PostMsg)msgEvent.msg();
       
       // provider sends post ACK to consumer
       AckMsg ackMsg = (AckMsg)CodecFactory.createMsg();
       ackMsg.msgClass(MsgClasses.ACK);
       ackMsg.streamId(consumerRole.rdmLoginRequest().streamId());
       ackMsg.domainType(offstreamPost.domainType());
       ackMsg.ackId(offstreamPost.postId());
       ackMsg.applyHasNakCode();
       ackMsg.nakCode(NakCodes.NONE);
       ackMsg.applyHasSeqNum();
       ackMsg.seqNum(offstreamPost.seqNum());
       assertTrue(provider.submitAndDispatch(ackMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Consumer receives ACK. */
       consumerReactor.dispatch(1);
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.ACK, msgEvent.msg().msgClass());
       
       // extract refresh message from post and send to consumer
       DecodeIterator dIter = CodecFactory.createDecodeIterator();
       dIter.setBufferAndRWFVersion(offstreamPost.encodedDataBody(), msgEvent.reactorChannel().majorVersion(), msgEvent.reactorChannel().minorVersion());
       Msg extractedMsg = CodecFactory.createMsg();
       assertEquals(extractedMsg.decode(dIter), ReactorReturnCodes.SUCCESS);
       assertEquals(0, extractedMsg.flags());
       extractedMsg.streamId(3);
       assertTrue(provider.submitAndDispatch(extractedMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       /* Consumer receives extracted refresh message. */
       consumerReactor.dispatch(1);
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
       assertEquals(0, msgEvent.msg().flags());
       
       // sleep for 7 seconds and no more consumer events should be received
       try
       {
           Thread.sleep(7000);
       }
       catch (InterruptedException e) {}

       consumerReactor.dispatch(0);
       
       TestReactorComponent.closeSession(consumer, provider);
   }

    @Test
    public void snapshotOnStreamingAggregationTest()
    {
        snapshotOnStreamingAggregationTest(false);
    }

    @Test
    public void snapshotOnStreamingAggregationTest_dispatchBetweenItemRequests()
    {
        snapshotOnStreamingAggregationTest(true);
    }

    public void snapshotOnStreamingAggregationTest(boolean dispatchBetweenItemRequests)
    {
		/* Test aggregation of a snapshot request onto a streaming request. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
        UpdateMsg receivedUpdateMsg;
        int providerStreamId;
        WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
        testUserSpecObj.value(997);
        
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
        
        /* Consumer sends streaming request. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        if (dispatchBetweenItemRequests)
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        else
            assertTrue(consumer.submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer sends snapshot request for same item (does not apply streaming flag). */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Provider receives one request, priority 1/1 (snapshot doesn't count towards priority). */
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
        assertTrue(receivedRequestMsg.checkHasPriority());
        assertEquals(1, receivedRequestMsg.priority().priorityClass());
        assertEquals(1, receivedRequestMsg.priority().count());
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
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refreshes, one with state OPEN on stream 5, and one NON-STREAMING on 6. */
        consumerReactor.dispatch(2);
        
        /* Streaming refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkRefreshComplete());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

        /* Snapshot refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(6, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkRefreshComplete());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
        
        /* Provider sends an update. */
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(providerStreamId);
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.containerType(DataTypes.NO_DATA);
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives the update, only on stream 5. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(5, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

        /* Request the snapshot again on stream 6. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
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
        assertTrue(receivedRequestMsg.checkHasPriority());
        assertEquals(1, receivedRequestMsg.priority().priorityClass());
        assertEquals(1, receivedRequestMsg.priority().count());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        assertEquals(providerStreamId, receivedRequestMsg.streamId());
        
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
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives nonstreaming refresh, only on stream 6 (refresh was solicited by stream 6; stream 5 doesn't need it). */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(6, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkRefreshComplete());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

        /* Provider sends an update. */
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(providerStreamId);
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.containerType(DataTypes.NO_DATA);
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives the update, only on stream 5. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(5, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        TestReactorComponent.closeSession(consumer, provider);
    }

    @Test
    public void streamingAggregationTest()
    {
    	streamingAggregationTest(false);
    }

    @Test
    public void streamingAggregationTest_dispatchBetweenItemRequests()
    {
    	streamingAggregationTest(true);
    }

    public void streamingAggregationTest(boolean dispatchBetweenItemRequests)
    {
		/* Test aggregation of two streaming items. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
        UpdateMsg receivedUpdateMsg;
        int providerStreamId;
        WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
        testUserSpecObj.value(997);
        
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
        
        /* Consumer sends streaming request. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        if (dispatchBetweenItemRequests)
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        else
            assertTrue(consumer.submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer sends streaming request for same item . */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI.N");
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        providerReactor.dispatch(2);
        
        /* Provider receives request. */
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

        /* Provider receives priority change due to second request. */
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
        assertTrue(receivedRequestMsg.checkHasPriority());
        assertEquals(1, receivedRequestMsg.priority().priorityClass());
        assertEquals(2, receivedRequestMsg.priority().count());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        assertEquals(providerStreamId, receivedRequestMsg.streamId());
        
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
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives two refreshes. */
        consumerReactor.dispatch(2);
        
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkRefreshComplete());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(6, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkRefreshComplete());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
        
        /* Provider sends an update. */
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(providerStreamId);
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.containerType(DataTypes.NO_DATA);
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives the update on both streams. */
        consumerReactor.dispatch(2);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(5, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(6, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void streamingSnapshotBeforeChannelReadyTest()
    {
        /* Test aggregation of streaming then Snapshot requests before channel ready */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
        UpdateMsg receivedUpdateMsg;
        int providerStreamId;
        WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
        testUserSpecObj.value(997);
        
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new streamingSnapshotBeforeChannelReady(consumerReactor);
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
        opts.numStatusEvents(2); // set number of expected status message from request submitted in channel open callback
        TestReactor.openSession(consumer, provider, opts);
        

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
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refreshes, one with state OPEN on stream 5, one NON-STREAMING on 6 */
        consumerReactor.dispatch(2);
        
        /* Streaming refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
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
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

        /* Snapshot refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(6, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

        /* Provider sends an update. */
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(providerStreamId);
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.containerType(DataTypes.NO_DATA);
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives the update, only on stream 5. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(5, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void snapshotStreamingBeforeChannelReadyTest()
    {
        /* Test aggregation of streaming then Snapshot requests before channel ready */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
        UpdateMsg receivedUpdateMsg;
        int providerStreamId;
        WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
        testUserSpecObj.value(997);
        
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new snapshotStreamingBeforeChannelReady(consumerReactor);
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
        opts.numStatusEvents(2); // set number of expected status message from request submitted in channel open callback
        TestReactor.openSession(consumer, provider, opts);
        

        /* Provider receives request. */
        providerReactor.dispatch(1);
        
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
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
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refreshes, one with state OPEN on stream 5, one NON-STREAMING on 6 */
        consumerReactor.dispatch(2);
        
        /* Streaming refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

        /* Snapshot refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(6, receivedRefreshMsg.streamId());
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
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

        /* Provider sends an update. */
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(providerStreamId);
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.containerType(DataTypes.NO_DATA);
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives the update, only on stream 6. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(6, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void snapshotStreamingViewMixAggregationBeforeChannelReadyTest()
    {
        /* Test aggregation of 4 requests, Snapshot, Snapshot-View, Streaming, and Streaming-View */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
        UpdateMsg receivedUpdateMsg;
        int providerStreamId;
        WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
        testUserSpecObj.value(997);
        
        /* Create reactors. */
        TestReactor.enableReactorXmlTracing();
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new snapshotStreamingViewMixAggregationBeforeChannelReady(consumerReactor);
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
        opts.numStatusEvents(4); // set number of expected status message from request submitted in channel open callback
        TestReactor.openSession(consumer, provider, opts);
        

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
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refreshes, one with state OPEN on stream 5, one NON-STREAMING on 6, one OPEN on stream 7, and NON-STREAMING on stream 8. */
        consumerReactor.dispatch(4);
        
        /* Streaming refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
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
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

        /* Snapshot refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(6, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
        
        /* Streaming refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(7, receivedRefreshMsg.streamId());
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
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

        /* Snapshot refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(8, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
        
        /* Provider sends an update. */
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(providerStreamId);
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.containerType(DataTypes.NO_DATA);
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives the update, fans out to 2 streams */
        consumerReactor.dispatch(2);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(5, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(7, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        TestReactorComponent.closeSession(consumer, provider);
    }
   
    @Test
    public void snapshotAggregationOnClosedStreamTest()
    {
        /* Test aggregation of a snapshot request, using a previously closed stream. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
        WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
        testUserSpecObj.value(997);
        
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
        
        /* Consumer sends snapshot request for TRI1, TRI2, TRI3 */
        
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI1");
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI2");
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(7);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI3");
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Provider receives request. */
        providerReactor.dispatch(3);
        
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertFalse(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI1"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        providerStreamId = receivedRequestMsg.streamId();
        
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertFalse(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI2"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        //providerStreamId = receivedRequestMsg.streamId();
        
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertFalse(receivedRequestMsg.checkStreaming());
        assertFalse(receivedRequestMsg.checkNoRefresh());
        assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
        assertTrue(receivedRequestMsg.msgKey().checkHasName());
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI3"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        //providerStreamId = receivedRequestMsg.streamId();
        
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
        refreshMsg.msgKey().name().data("TRI1");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refresh */
        consumerReactor.dispatch(1);

        /* Snapshot refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI1"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
        
        /* Consumer sends TRI2 snapshot reusing TRI1 stream */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI2");
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Provider receives nothing, because of aggregation. */
        providerReactor.dispatch(0);
        
        /* Provider sends refresh .*/
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(4);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("TRI2");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refresh */
        consumerReactor.dispatch(2);

        /* Snapshot refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(6, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI2"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
        
        /* Snapshot refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI2"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
        
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
        refreshMsg.msgKey().name().data("TRI3");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refresh */
        consumerReactor.dispatch(1);
        
        /* Snapshot refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(7, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI3"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());

        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void streamingAggregationCloseReuseStreamTest()
    {
        /* Test aggregation of a streaming request, using a previously closed stream. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
        WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
        testUserSpecObj.value(997);
        
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
        
        /* Consumer sends streaming request for TRI1, TRI2, TRI3 */
        
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI1");
        requestMsg.applyStreaming();
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI2");
        requestMsg.applyStreaming();
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(7);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI3");
        requestMsg.applyStreaming();
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Provider receives request. */
        providerReactor.dispatch(3);
        
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI1"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        providerStreamId = receivedRequestMsg.streamId();
        
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI2"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        //providerStreamId = receivedRequestMsg.streamId();
        
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI3"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        //providerStreamId = receivedRequestMsg.streamId();
        
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
        refreshMsg.msgKey().name().data("TRI1");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refresh */
        consumerReactor.dispatch(1);

        /* Streaming refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI1"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        assertNotNull(msgEvent.streamInfo().userSpecObject());
        assertEquals(testUserSpecObj, msgEvent.streamInfo().userSpecObject());
        
        /* Consumer sends TRI1 close */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.CLOSE);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer sends TRI2 streaming reusing TRI1 stream */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI2");
        requestMsg.applyStreaming();
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Provider receives request. */
        providerReactor.dispatch(2);
        
        event = providerReactor.pollEvent(); // Skip close message
        
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI2"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());

        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void snapshotBeforeStreamingRequest()
    {
		/* Test aggregation of a snapshot request onto a streaming request. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
        UpdateMsg receivedUpdateMsg;
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
        
        /* Consumer sends snapshot request */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
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
        assertFalse(receivedRequestMsg.checkStreaming());
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
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives refresh, NON-STREAMING on 5. */
        consumerReactor.dispatch(1);

        /* Snapshot refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        /* Consumer sends streaming request for same item */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
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
        refreshMsg.applyRefreshComplete();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives refresh, STREAMING on 6. */
        consumerReactor.dispatch(1);
        
        /* Streaming refresh. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(6, receivedRefreshMsg.streamId());
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
        
        /* Provider sends an update. */
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(providerStreamId);
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.containerType(DataTypes.NO_DATA);
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives the update, only on stream 6. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
        receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
        assertEquals(6, receivedUpdateMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
   @Test
   public void loginClosedRecoverTest_SingleOpenOn()
   {
       loginClosedRecoverTest(true);
   }
   
   @Test
   public void loginClosedRecoverTest_SingleOpenOff()
   {
       loginClosedRecoverTest(false);
   }
   
   
   public void loginClosedRecoverTest(boolean singleOpen)
   {
       /* Tests behavior in response to receiving login closed/recover.
        * We should see:
        * - Watchlist disconnects in response to receiving this
        *   (and unless the login stream was previously established, ensure
        *   the connection can backoff).
        * - Tests include:
        *   - Provider sending ClosedRecover login StatusMsg/RefreshMsg in response to request.
        *   - As above, but after sending an Open/Ok login refresh first.  */
       
       ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
       TestReactorEvent event;
       ReactorMsgEvent msgEvent;
       Msg msg = CodecFactory.createMsg();
       RequestMsg requestMsg = (RequestMsg)msg;
       LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg(), recvLoginRefresh;
       LoginStatus loginStatus = (LoginStatus)LoginMsgFactory.createMsg(), recvLoginStatus;
       DirectoryRequest recvDirectoryRequest;
       DirectoryUpdate recvDirectoryUpdate;
       RDMLoginMsgEvent loginMsgEvent;
       RDMDirectoryMsgEvent directoryMsgEvent;
       StatusMsg recvStatusMsg;
       ReactorChannelEvent channelEvent;
       LoginRequest recvLoginRequest;
       
       final int reconnectMinDelay = 1000, reconnectMaxDelay = 3000;
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
       consumerRole.watchlistOptions().enableWatchlist(true);
       consumerRole.watchlistOptions().channelOpenCallback(consumer);
       consumerRole.watchlistOptions().requestTimeout(3000);
       consumerRole.rdmLoginRequest().applyHasAttrib();
       consumerRole.rdmLoginRequest().attrib().applyHasSingleOpen();
       consumerRole.rdmLoginRequest().attrib().singleOpen(singleOpen ? 1 : 0);
       
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
       opts.reconnectAttemptLimit(-1);
       opts.reconnectMinDelay(reconnectMinDelay);
       opts.reconnectMaxDelay(reconnectMaxDelay);
       
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
       
       /* Do two loops of sending login ClosedRecover status from the provider.
        * Make sure the Reactor is backing off the reconnect time
        * on the second attempt. */
       
       int provLoginStreamId = provider.defaultSessionLoginStreamId();
       for (int i = 0; i < 2; ++i)
       {
           /* Calculate delay time (doubles for each iteration) */
           expectedReconnectDelayTimeMs = reconnectMinDelay;
           for (int j = 0; j < i; ++j) 
               expectedReconnectDelayTimeMs *= 2;
           
           /* Provider sends login closed-recover. */
           loginStatus.clear();
           loginStatus.rdmMsgType(LoginMsgType.STATUS);
           loginStatus.streamId(provLoginStreamId);
           loginStatus.applyHasState();
           loginStatus.state().streamState(StreamStates.CLOSED_RECOVER);
           loginStatus.state().dataState(DataStates.SUSPECT);
           submitOptions.clear();
           assertTrue(provider.submitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           consumerReactor.dispatch(i == 0 ? 4 : 2);
                  
           /* Consumer receives open/suspect login status. */
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           recvLoginStatus = (LoginStatus)loginMsgEvent.rdmLoginMsg();
           assertEquals(consumer.defaultSessionLoginStreamId(), recvLoginStatus.streamId());
           assertTrue(recvLoginStatus.checkHasState());
           assertEquals(StreamStates.OPEN, recvLoginStatus.state().streamState());
           assertEquals(DataStates.SUSPECT, recvLoginStatus.state().dataState());
           
           /* Consumer receives channel-down event. */
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
           
           if (i == 0)
           {
               /* Consumer receives directory update. */
               event = consumerReactor.pollEvent();
               assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
               directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
               assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
               recvDirectoryUpdate = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
               assertEquals(consumer.defaultSessionDirectoryStreamId(), recvDirectoryUpdate.streamId());
               
               /* Consumer receives item status (Open vs. ClosedRecover, depending on single-open setting) */
               event = consumerReactor.pollEvent();
               assertEquals(TestReactorEventTypes.MSG, event.type());
               msgEvent = (ReactorMsgEvent)event.reactorEvent();
               recvStatusMsg = (StatusMsg)msgEvent.msg();
               assertEquals(5, recvStatusMsg.streamId());
               assertTrue(recvStatusMsg.checkHasState());
               assertEquals ((singleOpen ? StreamStates.OPEN : StreamStates.CLOSED_RECOVER),
                             recvStatusMsg.state().streamState());
           }
                  
           startTimeNano = System.nanoTime();
           
           /* Provider receives channel-down event. */
           providerReactor.dispatch(1);
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
           
           provider.closeChannel();
           
           /* Wait for the reconnect. */
           providerReactor.accept(opts, provider, expectedReconnectDelayTimeMs + 1000);
           
           /* Consumer receives channel-up event (and should internally push out login request). */
           consumerReactor.dispatch(1);
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
           
           /* Provider receives channel-up event. */
           providerReactor.dispatch(3);
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
           
           deviationTimeMs = abs((event.nanoTime() - startTimeNano)/1000000 - expectedReconnectDelayTimeMs);
           assertTrue( "Reconnection delay off by " + deviationTimeMs + "ms.", deviationTimeMs < 300);
           
           /* Provider receives channel-ready event. */
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
           
           /* Provider receives relogin. */
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           recvLoginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
           provLoginStreamId = recvLoginRequest.streamId(); 
       }
       
       /* Do two loops of sending login refresh, THEN ClosedRecover status from the provider.
        * Make sure the Reactor is NOT backing off the reconnect time
        * on the second attempt. */
       for (int i = 0; i < 2; ++i)
       {                  
           /* Calculate delay time. */
           expectedReconnectDelayTimeMs = reconnectMinDelay;
          
           /* Provider sends login refresh. */
           loginRefresh.clear();
           loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
           loginRefresh.applySolicited();
           loginRefresh.streamId(provLoginStreamId);
           loginRefresh.state().streamState(StreamStates.OPEN);
           loginRefresh.state().dataState(DataStates.OK);
           submitOptions.clear();
           assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           /* Consumer receives login refresh .*/
           consumerReactor.dispatch(1);
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           recvLoginRefresh = (LoginRefresh)loginMsgEvent.rdmLoginMsg();
           assertEquals(consumer.defaultSessionLoginStreamId(), recvLoginRefresh.streamId());
           assertTrue(recvLoginRefresh.checkSolicited());
           assertEquals(StreamStates.OPEN, recvLoginRefresh.state().streamState());
           assertEquals(DataStates.OK, recvLoginRefresh.state().dataState());
                      
           /* Provider receives directory request. */
           providerReactor.dispatch(1);
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
           directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
           recvDirectoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
           assertEquals(DirectoryMsgType.REQUEST, recvDirectoryRequest.rdmMsgType());
           
           /* Provider sends login closed-recover. */
           loginStatus.clear();
           loginStatus.rdmMsgType(LoginMsgType.STATUS);
           loginStatus.streamId(provLoginStreamId);
           loginStatus.applyHasState();
           loginStatus.state().streamState(StreamStates.CLOSED_RECOVER);
           loginStatus.state().dataState(DataStates.SUSPECT);
           submitOptions.clear();
           assertTrue(provider.submitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           consumerReactor.dispatch(2);
           
           /* Consumer receives open/suspect login status. */
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           recvLoginStatus = (LoginStatus)loginMsgEvent.rdmLoginMsg();
           assertEquals(consumer.defaultSessionLoginStreamId(), recvLoginStatus.streamId());
           assertTrue(recvLoginStatus.checkHasState());
           assertEquals(StreamStates.OPEN, recvLoginStatus.state().streamState());
           assertEquals(DataStates.SUSPECT, recvLoginStatus.state().dataState());
           
                 
           /* Consumer receives channel-down event. */
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
           
           startTimeNano = System.nanoTime();
           
           providerReactor.dispatch(1);
           
           /* Provider receives channel-down event. */
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
           
           provider.closeChannel();
           
           /* Wait for the reconnect. */
           providerReactor.accept(opts, provider, expectedReconnectDelayTimeMs + 1000);
           
           /* Consumer receives channel-up event (and should internally push out login request). */
           consumerReactor.dispatch(1);
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
           
           /* Provider receives channel-up event. */
           providerReactor.dispatch(3);
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
           
           deviationTimeMs = abs((event.nanoTime() - startTimeNano)/1000000 - expectedReconnectDelayTimeMs);
           assertTrue( "Reconnection delay off by " + deviationTimeMs + "ms.", deviationTimeMs < 300);
           
           /* Provider receives channel-ready event. */
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
           
           /* Provider receives relogin. */
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           recvLoginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
           provLoginStreamId = recvLoginRequest.streamId(); 
       }

       /*** Same test as above, but with ClosedRecover Login Refresh instead of status. ***/
       
       /* Send a login refresh to establish the stream (and reset the reconnect timer). */
       loginRefresh.clear();
       loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
       loginRefresh.applySolicited();
       loginRefresh.streamId(provLoginStreamId);
       loginRefresh.state().streamState(StreamStates.OPEN);
       loginRefresh.state().dataState(DataStates.OK);
       submitOptions.clear();
       assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Consumer receives login refresh .*/
       consumerReactor.dispatch(1);
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
       loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
       assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
       recvLoginRefresh = (LoginRefresh)loginMsgEvent.rdmLoginMsg();
       assertEquals(consumer.defaultSessionLoginStreamId(), recvLoginRefresh.streamId());
       assertTrue(recvLoginRefresh.checkSolicited());
       assertEquals(StreamStates.OPEN, recvLoginRefresh.state().streamState());
       assertEquals(DataStates.OK, recvLoginRefresh.state().dataState());
       
       /* Provider receives directory request. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
       directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
       recvDirectoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
       assertEquals(DirectoryMsgType.REQUEST, recvDirectoryRequest.rdmMsgType());

       /* Do two loops of sending login ClosedRecover from the provider.
        * Make sure the Reactor is backing off the reconnect time
        * on the second attempt. */
       
       for (int i = 0; i < 2; ++i)
       {
           /* Calculate delay time (doubles for each iteration) */
           expectedReconnectDelayTimeMs = reconnectMinDelay;
           for (int j = 0; j < i; ++j) 
               expectedReconnectDelayTimeMs *= 2;
           
           /* Provider sends login closed-recover. */
           loginRefresh.clear();
           loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
           loginRefresh.streamId(provLoginStreamId);
           loginRefresh.state().streamState(StreamStates.CLOSED_RECOVER);
           loginRefresh.state().dataState(DataStates.SUSPECT);
           submitOptions.clear();
           assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           consumerReactor.dispatch(2);
                  
           /* Consumer receives open/suspect login refresh. */
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           recvLoginRefresh = (LoginRefresh)loginMsgEvent.rdmLoginMsg();
           assertEquals(consumer.defaultSessionLoginStreamId(), recvLoginRefresh.streamId());
           assertEquals(StreamStates.OPEN, recvLoginRefresh.state().streamState());
           assertEquals(DataStates.SUSPECT, recvLoginRefresh.state().dataState());

                  
           /* Consumer receives channel-down event. */
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
           
           startTimeNano = System.nanoTime();
           
           /* Provider receives channel-down event. */
           providerReactor.dispatch(1);
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
           
           provider.closeChannel();
           
           /* Wait for the reconnect. */
           providerReactor.accept(opts, provider, expectedReconnectDelayTimeMs + 1000);
           
           /* Consumer receives channel-up event (and should internally push out login request). */
           consumerReactor.dispatch(1);
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
           
           /* Provider receives channel-up event. */
           providerReactor.dispatch(3);
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
           
           deviationTimeMs = abs((event.nanoTime() - startTimeNano)/1000000 - expectedReconnectDelayTimeMs);
           assertTrue( "Reconnection delay off by " + deviationTimeMs + "ms.", deviationTimeMs < 300);
           
           /* Provider receives channel-ready event. */
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
           
           /* Provider receives relogin. */
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           recvLoginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
           provLoginStreamId = recvLoginRequest.streamId(); 
       }
       
       /* Do two loops of sending login refresh, THEN ClosedRecover from the provider.
        * Make sure the Reactor is NOT backing off the reconnect time
        * on the second attempt. */
       for (int i = 0; i < 2; ++i)
       {                  
           /* Calculate delay time. */
           expectedReconnectDelayTimeMs = reconnectMinDelay;
          
           /* Provider sends login refresh. */
           loginRefresh.clear();
           loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
           loginRefresh.applySolicited();
           loginRefresh.streamId(provLoginStreamId);
           loginRefresh.state().streamState(StreamStates.OPEN);
           loginRefresh.state().dataState(DataStates.OK);
           submitOptions.clear();
           assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           /* Consumer receives login refresh .*/
           consumerReactor.dispatch(1);
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           recvLoginRefresh = (LoginRefresh)loginMsgEvent.rdmLoginMsg();
           assertEquals(consumer.defaultSessionLoginStreamId(), recvLoginRefresh.streamId());
           assertTrue(recvLoginRefresh.checkSolicited());
           assertEquals(StreamStates.OPEN, recvLoginRefresh.state().streamState());
           assertEquals(DataStates.OK, recvLoginRefresh.state().dataState());
           
           /* Provider receives directory request. */
           providerReactor.dispatch(1);
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
           directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
           recvDirectoryRequest = (DirectoryRequest)directoryMsgEvent.rdmDirectoryMsg();
           assertEquals(DirectoryMsgType.REQUEST, recvDirectoryRequest.rdmMsgType());
           
           /* Provider sends login closed-recover. */
           loginRefresh.clear();
           loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
           loginRefresh.streamId(provLoginStreamId);
           loginRefresh.state().streamState(StreamStates.CLOSED_RECOVER);
           loginRefresh.state().dataState(DataStates.SUSPECT);
           submitOptions.clear();
           assertTrue(provider.submitAndDispatch(loginRefresh, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           consumerReactor.dispatch(2);
           
           /* Consumer receives open/suspect login refresh. */
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           recvLoginRefresh = (LoginRefresh)loginMsgEvent.rdmLoginMsg();
           assertEquals(consumer.defaultSessionLoginStreamId(), recvLoginRefresh.streamId());
           assertEquals(StreamStates.OPEN, recvLoginRefresh.state().streamState());
           assertEquals(DataStates.SUSPECT, recvLoginRefresh.state().dataState());
                  
           /* Consumer receives channel-down event. */
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());
           
           startTimeNano = System.nanoTime();
           
           providerReactor.dispatch(1);
           
           /* Provider receives channel-down event. */
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN, channelEvent.eventType());
           
           provider.closeChannel();
           
           /* Wait for the reconnect. */
           providerReactor.accept(opts, provider, expectedReconnectDelayTimeMs + 1000);
           
           /* Consumer receives channel-up event (and should internally push out login request). */
           consumerReactor.dispatch(1);
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
           
           /* Provider receives channel-up event. */
           providerReactor.dispatch(3);
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
           
           deviationTimeMs = abs((event.nanoTime() - startTimeNano)/1000000 - expectedReconnectDelayTimeMs);
           assertTrue( "Reconnection delay off by " + deviationTimeMs + "ms.", deviationTimeMs < 300);
           
           /* Provider receives channel-ready event. */
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
           channelEvent = (ReactorChannelEvent)event.reactorEvent();
           assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
           
           /* Provider receives relogin. */
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           recvLoginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
           provLoginStreamId = recvLoginRequest.streamId(); 
       }
              
       TestReactorComponent.closeSession(consumer, provider);
       consumerReactor.close();
       providerReactor.close();
   }

   /* Used by submitOffstreamPostOnItemRefeshTest. */
   class PriorityChangeFromCallbackConsumer extends Consumer
   {
       public PriorityChangeFromCallbackConsumer(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int defaultMsgCallback(ReactorMsgEvent event)
       {
           RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           super.defaultMsgCallback(event);

           requestMsg.clear();
           requestMsg.msgClass(MsgClasses.REQUEST);
           requestMsg.streamId(5);
           requestMsg.domainType(DomainTypes.MARKET_PRICE);
           requestMsg.applyStreaming();
           requestMsg.applyNoRefresh();
           requestMsg.msgKey().applyHasName();
           requestMsg.msgKey().name().data("TRI.N");
           requestMsg.applyHasPriority();
           requestMsg.priority().priorityClass(1);
           requestMsg.priority().count(2);
           submitOptions.clear();
           submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
           assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           return ReactorReturnCodes.SUCCESS;
       }
   }

   @Test
   public void priorityChangeInAndOutOfCallbackTest()
   {
       /* Simple test of changing priority both inside and outside a callback.
        * Reproduced ETA-2144 (a NullPointerException when changing priority) */
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
       Consumer consumer = new PriorityChangeFromCallbackConsumer(consumerReactor);
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
       consumerRole.watchlistOptions().requestTimeout(3000);
       
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
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().count());
       
       providerStreamId = receivedRequestMsg.streamId();
       
       /* Provider sends refresh .*/
       refreshMsg.clear();
       refreshMsg.msgClass(MsgClasses.REFRESH);
       refreshMsg.applySolicited();
       refreshMsg.domainType(DomainTypes.MARKET_PRICE);
       refreshMsg.streamId(providerStreamId);
       refreshMsg.containerType(DataTypes.NO_DATA);
       refreshMsg.applyHasMsgKey();
       refreshMsg.msgKey().applyHasServiceId();
       refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
       refreshMsg.msgKey().applyHasName();
       refreshMsg.msgKey().name().data("TRI.N");
       refreshMsg.applyRefreshComplete();
       Buffer groupId = CodecFactory.createBuffer();
       groupId.data("1234431");
       refreshMsg.groupId(groupId);
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
              
       /* Provider receives priority change. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
              
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertTrue(receivedRequestMsg.checkStreaming());
       assertTrue(receivedRequestMsg.checkNoRefresh());
       
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertTrue(receivedRequestMsg.msgKey().checkHasName());
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().priorityClass());
       assertEquals(2, receivedRequestMsg.priority().count());

       /* Consumer sends priority change, not in callback. */
       requestMsg.clear();
       requestMsg.msgClass(MsgClasses.REQUEST);
       requestMsg.streamId(5);
       requestMsg.domainType(DomainTypes.MARKET_PRICE);
       requestMsg.applyStreaming();
       requestMsg.applyNoRefresh();
       requestMsg.msgKey().applyHasName();
       requestMsg.msgKey().name().data("TRI.N");
       requestMsg.applyHasPriority();
       requestMsg.priority().priorityClass(1);
       requestMsg.priority().count(3);
       submitOptions.clear();
       submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
       assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

       /* Provider receives priority change. */
       providerReactor.dispatch(1);
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
              
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
       assertTrue(receivedRequestMsg.checkStreaming());
       assertTrue(receivedRequestMsg.checkNoRefresh());
       assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
       assertTrue(receivedRequestMsg.msgKey().checkHasName());
       assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().priorityClass());
       assertEquals(3, receivedRequestMsg.priority().count());
       
       consumerReactor.dispatch(0);
       
       TestReactorComponent.closeSession(consumer, provider);
   }

   /* Used by snapshotAggregationBeforeChannelReadyTestd Test. */
   class sendMultipleSnapshotsBeforeChannelReady extends Consumer
   {
       public sendMultipleSnapshotsBeforeChannelReady(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int reactorChannelEventCallback(ReactorChannelEvent event)
       {
           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           Msg msg = CodecFactory.createMsg();
           RequestMsg requestMsg = (RequestMsg)msg;
           
           if (event.eventType() == ReactorChannelEventTypes.CHANNEL_OPENED)
           {
               super.reactorChannelEventCallback(event);
               
               /* Consumer sends first snapshot request. */
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(5);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
               
               /* Consumer sends second aggregated snapshot request. */
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(6);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           }
           else
           {
               return super.reactorChannelEventCallback(event);
           }
           
           return ReactorReturnCodes.SUCCESS;
       }
   }

   /* Used by itemViewAggregateSnapshotBeforeChannelReadyTest Test. */
   class itemViewAggregateSnapshotBeforeChannelReadyTest extends Consumer
   {
       public itemViewAggregateSnapshotBeforeChannelReadyTest(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int reactorChannelEventCallback(ReactorChannelEvent event)
       {
           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           Msg msg = CodecFactory.createMsg();
           RequestMsg requestMsg = (RequestMsg)msg;
           List<Integer> viewFieldList = new ArrayList<Integer>();
           
           if (event.eventType() == ReactorChannelEventTypes.CHANNEL_OPENED)
           {
               super.reactorChannelEventCallback(event);
               
            // submit request view streaming message
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(5);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.applyStreaming();
               requestMsg.applyHasView();
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("VRX");
               viewFieldList.add(22);
               viewFieldList.add(22);
               viewFieldList.add(6);
               viewFieldList.add(0);
               viewFieldList.add(130);
               viewFieldList.add(1131);
               viewFieldList.add(1025);                
               encodeViewFieldIdList(reactorChannel(), viewFieldList, requestMsg);

               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
               
               // submit request non-view snapshot message
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(6);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("VRX");         

               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           }
           else
           {
               return super.reactorChannelEventCallback(event);
           }
           
           return ReactorReturnCodes.SUCCESS;
       }
   }
   
   /* Used by snapshotStreamingViewMixAggregationBeforeChannelReadyTest Test. */
   class snapshotStreamingViewMixAggregationBeforeChannelReady extends Consumer
   {
       public snapshotStreamingViewMixAggregationBeforeChannelReady(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int reactorChannelEventCallback(ReactorChannelEvent event)
       {
           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           Msg msg = CodecFactory.createMsg();
           RequestMsg requestMsg = (RequestMsg)msg;
           List<Integer> viewFieldList = new ArrayList<Integer>();
           WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
           testUserSpecObj.value(997);
           
           if (event.eventType() == ReactorChannelEventTypes.CHANNEL_OPENED)
           {
               super.reactorChannelEventCallback(event);
               
               /* Consumer sends streaming request. */
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(5);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.applyStreaming();
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

               /* Consumer sends snapshot request for same item (does not apply streaming flag). */
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(6);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
               
               /* Consumer sends streaming request for same item with view. */
               
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(7);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.applyStreaming();
               requestMsg.applyHasView();
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               viewFieldList.add(22);
               viewFieldList.add(22);
               viewFieldList.add(6);
               viewFieldList.add(0);
               viewFieldList.add(130);
               viewFieldList.add(1131);
               viewFieldList.add(1025);                
               encodeViewFieldIdList(reactorChannel(), viewFieldList, requestMsg);

               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

               /* Consumer sends snapshot request with view for same item (does not apply streaming flag). */
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(8);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.applyHasView();
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               viewFieldList.add(22);
               viewFieldList.add(22);
               viewFieldList.add(6);
               viewFieldList.add(0);
               viewFieldList.add(130);
               viewFieldList.add(1131);
               viewFieldList.add(1025);                
               encodeViewFieldIdList(reactorChannel(), viewFieldList, requestMsg);

               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           }
           else
           {
               return super.reactorChannelEventCallback(event);
           }
           
           return ReactorReturnCodes.SUCCESS;
       }
   }
   
   /* Used by streamingSnapshotBeforeChannelReadyTest Test. */
   class streamingSnapshotBeforeChannelReady extends Consumer
   {
       public streamingSnapshotBeforeChannelReady(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int reactorChannelEventCallback(ReactorChannelEvent event)
       {
           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           Msg msg = CodecFactory.createMsg();
           RequestMsg requestMsg = (RequestMsg)msg;
           WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
           testUserSpecObj.value(997);
           
           if (event.eventType() == ReactorChannelEventTypes.CHANNEL_OPENED)
           {
               super.reactorChannelEventCallback(event);
               
               /* Consumer sends streaming request. */
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(5);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.applyStreaming();
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

               /* Consumer sends snapshot request for same item (does not apply streaming flag). */
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(6);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           }
           else
           {
               return super.reactorChannelEventCallback(event);
           }
           
           return ReactorReturnCodes.SUCCESS;
       }
   }
   
   /* Used by snapshotStreamingBeforeChannelReadyTest Test. */
   class snapshotStreamingBeforeChannelReady extends Consumer
   {
       public snapshotStreamingBeforeChannelReady(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int reactorChannelEventCallback(ReactorChannelEvent event)
       {
           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           Msg msg = CodecFactory.createMsg();
           RequestMsg requestMsg = (RequestMsg)msg;
           WlInteger testUserSpecObj = ReactorFactory.createWlInteger();
           testUserSpecObj.value(997);
           
           if (event.eventType() == ReactorChannelEventTypes.CHANNEL_OPENED)
           {
               super.reactorChannelEventCallback(event);
               
               /* Consumer sends snapshot request (does not apply streaming flag). */
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(5);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
               
               /* Consumer sends streaming request for same request. */
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(6);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.applyStreaming();
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               submitOptions.requestMsgOptions().userSpecObj(testUserSpecObj);
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           }
           else
           {
               return super.reactorChannelEventCallback(event);
           }
           
           return ReactorReturnCodes.SUCCESS;
       }
   }
   
   
   @Test
   public void snapshotAggregationBeforeChannelReadyTest()
   {
		/* Test aggregation of a snapshot request onto a streaming request. */
       
       ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
       TestReactorEvent event;
       ReactorMsgEvent msgEvent;
       RequestMsg receivedRequestMsg;
       RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
       RefreshMsg receivedRefreshMsg;
       int providerStreamId;
       
       /* Create reactors. */
       TestReactor consumerReactor = new TestReactor();
       TestReactor providerReactor = new TestReactor();
	       
		/* Create consumer. */
		Consumer consumer = new sendMultipleSnapshotsBeforeChannelReady(consumerReactor);
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
		opts.numStatusEvents(2); // set number of expected status message from request submitted in channel open callback
		TestReactor.openSession(consumer, provider, opts);

       /* Provider receives request. */
       providerReactor.dispatch(1);
       
       event = providerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
       receivedRequestMsg = (RequestMsg)msgEvent.msg();
       assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
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
       refreshMsg.state().streamState(StreamStates.NON_STREAMING);
       refreshMsg.state().dataState(DataStates.OK);
       refreshMsg.applyRefreshComplete();
       assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
       /* Consumer receives refreshes, one with state OPEN on stream 5, and one NON-STREAMING on 6. */
       consumerReactor.dispatch(2);
       
       /* Snapshot refresh. */
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(5, receivedRefreshMsg.streamId());
       assertTrue(receivedRefreshMsg.checkHasMsgKey());
       assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
       assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
       assertTrue(receivedRefreshMsg.msgKey().checkHasName());
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

       /* Snapshot refresh. */
       event = consumerReactor.pollEvent();
       assertEquals(TestReactorEventTypes.MSG, event.type());
       msgEvent = (ReactorMsgEvent)event.reactorEvent();
       assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
       receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
       assertEquals(6, receivedRefreshMsg.streamId());
       assertTrue(receivedRefreshMsg.checkHasMsgKey());
       assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
       assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
       assertTrue(receivedRefreshMsg.msgKey().checkHasName());
       assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
       assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
       assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
       assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
       assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
       assertNotNull(msgEvent.streamInfo());
       assertNotNull(msgEvent.streamInfo().serviceName());
       assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

       TestReactorComponent.closeSession(consumer, provider);
   }

   /* Used by itemCloseAndReopenTest. 
    * Closes TRI on stream 5 and reopens it. */
   class ItemCloseAndReopenConsumer extends Consumer
   {
       public ItemCloseAndReopenConsumer(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int defaultMsgCallback(ReactorMsgEvent event)
       {
           CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
           RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           super.defaultMsgCallback(event);

           if (event.msg().msgClass() == MsgClasses.UPDATE)
           {
               closeMsg.clear();
               closeMsg.msgClass(MsgClasses.CLOSE);
               closeMsg.streamId(5);
               closeMsg.domainType(DomainTypes.MARKET_PRICE);
               submitOptions.clear();
               assertTrue(submit(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(5);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.applyStreaming();
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
               
           }
           
           return ReactorReturnCodes.SUCCESS;
       }
   }

   @Test
   public void itemCloseAndReopenTest()
   {
       /* Test closing and reopening an item inside and outside the msg callback 
        * (the former reproduced ETA-2163). */

       ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
       TestReactorEvent event;
       ReactorMsgEvent msgEvent;
       RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
       RequestMsg receivedRequestMsg;
       RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
       RefreshMsg receivedRefreshMsg;
       UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
       UpdateMsg receivedUpdateMsg;
       CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
       CloseMsg receivedCloseMsg;
       int providerStreamId;
       
       /* Create reactors. */
       TestReactor consumerReactor = new TestReactor();
       TestReactor providerReactor = new TestReactor();
               
       /* Create consumer. */
       Consumer consumer = new ItemCloseAndReopenConsumer(consumerReactor);
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
       consumerRole.watchlistOptions().requestTimeout(3000);
       
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
       assertTrue(receivedRequestMsg.checkHasPriority());
       assertEquals(1, receivedRequestMsg.priority().priorityClass());
       assertEquals(1, receivedRequestMsg.priority().count());
       providerStreamId = receivedRequestMsg.streamId();

       for (int i = 0; i < 3; ++i)
       {
           /* Provider sends an update. */
           updateMsg.clear();
           updateMsg.msgClass(MsgClasses.UPDATE);
           updateMsg.streamId(providerStreamId);
           updateMsg.domainType(DomainTypes.MARKET_PRICE);
           updateMsg.containerType(DataTypes.NO_DATA);
           assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           /* Consumer does not receive it (no refresh received yet). */
           consumerReactor.dispatch(0);
           
           /* Provider sends refresh .*/
           refreshMsg.clear();
           refreshMsg.msgClass(MsgClasses.REFRESH);
           refreshMsg.applySolicited();
           refreshMsg.domainType(DomainTypes.MARKET_PRICE);
           refreshMsg.streamId(providerStreamId);
           refreshMsg.containerType(DataTypes.NO_DATA);
           refreshMsg.applyHasMsgKey();
           refreshMsg.msgKey().applyHasServiceId();
           refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
           refreshMsg.msgKey().applyHasName();
           refreshMsg.msgKey().name().data("TRI.N");
           refreshMsg.applyRefreshComplete();
           Buffer groupId = CodecFactory.createBuffer();
           groupId.data("5555");
           refreshMsg.groupId(groupId);
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
           
           /* Provider sends an update. */
           updateMsg.clear();
           updateMsg.msgClass(MsgClasses.UPDATE);
           updateMsg.streamId(providerStreamId);
           updateMsg.domainType(DomainTypes.MARKET_PRICE);
           updateMsg.containerType(DataTypes.NO_DATA);
           assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           /* Consumer receives update (closes/reopens request in callback) */
           consumerReactor.dispatch(1);
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.MSG, event.type());
           msgEvent = (ReactorMsgEvent)event.reactorEvent();
           assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
           receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
           assertEquals(5, receivedUpdateMsg.streamId());
           assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
           assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
           assertNotNull(msgEvent.streamInfo());
           assertNotNull(msgEvent.streamInfo().serviceName());
           assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

           /* Provider receives close and re-request. */
           providerReactor.dispatch(2);
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.MSG, event.type());
           msgEvent = (ReactorMsgEvent)event.reactorEvent();
           assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
           receivedCloseMsg = (CloseMsg)msgEvent.msg();
           assertEquals(providerStreamId, receivedCloseMsg.streamId());
           assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());

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
           assertTrue(receivedRequestMsg.checkHasPriority());
           assertEquals(1, receivedRequestMsg.priority().priorityClass());
           assertEquals(1, receivedRequestMsg.priority().count());
           providerStreamId = receivedRequestMsg.streamId();
           
       }
       
       for (int i = 0; i < 3; ++i)
       {
           /* Provider sends an update. */
           updateMsg.clear();
           updateMsg.msgClass(MsgClasses.UPDATE);
           updateMsg.streamId(providerStreamId);
           updateMsg.domainType(DomainTypes.MARKET_PRICE);
           updateMsg.containerType(DataTypes.NO_DATA);
           assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           /* Consumer does not receive it. */
           consumerReactor.dispatch(0);

           /* Provider sends refresh .*/
           refreshMsg.clear();
           refreshMsg.msgClass(MsgClasses.REFRESH);
           refreshMsg.applySolicited();
           refreshMsg.domainType(DomainTypes.MARKET_PRICE);
           refreshMsg.streamId(providerStreamId);
           refreshMsg.containerType(DataTypes.NO_DATA);
           refreshMsg.applyHasMsgKey();
           refreshMsg.msgKey().applyHasServiceId();
           refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
           refreshMsg.msgKey().applyHasName();
           refreshMsg.msgKey().name().data("TRI.N");
           refreshMsg.applyRefreshComplete();
           Buffer groupId = CodecFactory.createBuffer();
           groupId.data("5555");
           refreshMsg.groupId(groupId);
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
           
           /* Consumer sends close. */
           closeMsg.clear();
           closeMsg.msgClass(MsgClasses.CLOSE);
           closeMsg.streamId(5);
           closeMsg.domainType(DomainTypes.MARKET_PRICE);
           submitOptions.clear();
           assertTrue(consumer.submitAndDispatch(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

           /* Provider receives close. */
           providerReactor.dispatch(1);
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.MSG, event.type());
           msgEvent = (ReactorMsgEvent)event.reactorEvent();
           assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());

           receivedCloseMsg = (CloseMsg)msgEvent.msg();
           assertEquals(providerStreamId, receivedCloseMsg.streamId());
           assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());
           
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
           assertTrue(receivedRequestMsg.checkHasPriority());
           assertEquals(1, receivedRequestMsg.priority().priorityClass());
           assertEquals(1, receivedRequestMsg.priority().count());

           providerStreamId = receivedRequestMsg.streamId();

       }
       
       
       consumerReactor.dispatch(0);
       
       TestReactorComponent.closeSession(consumer, provider);
   }

   /* Used by closeFromCallbackTest. */
   class CloseFromCallbackConsumer extends Consumer
   {
       public CloseFromCallbackConsumer(TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int defaultMsgCallback(ReactorMsgEvent event)
       {
           CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           super.defaultMsgCallback(event);

           if (event.msg().msgClass() == MsgClasses.UPDATE
                   || event.msg().msgClass() == MsgClasses.STATUS)
           {
               closeMsg.clear();
               closeMsg.msgClass(MsgClasses.CLOSE);
               closeMsg.streamId(5);
               closeMsg.domainType(DomainTypes.MARKET_PRICE);
               submitOptions.clear();
               assertTrue(submit(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

               closeMsg.clear();
               closeMsg.msgClass(MsgClasses.CLOSE);
               closeMsg.streamId(6);
               closeMsg.domainType(DomainTypes.MARKET_PRICE);
               submitOptions.clear();
               assertTrue(submit(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           }
           
           return ReactorReturnCodes.SUCCESS;
       }
   }

   @Test
   public void itemCloseFromCallbackTest()
   {	   
       /* Opening two streams for an item and closing both within the callback. 
        * Tested in response to an update message as well as a group status. */

       ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
       TestReactorEvent event;
       ReactorMsgEvent msgEvent;
       RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
       RequestMsg receivedRequestMsg;
       RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
       RefreshMsg receivedRefreshMsg;
       UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
       UpdateMsg receivedUpdateMsg;
       StatusMsg receivedStatusMsg;
       CloseMsg receivedCloseMsg;
       RDMDirectoryMsgEvent directoryMsgEvent;
       int providerStreamId;
               
       /* Create reactors. */
       TestReactor consumerReactor = new TestReactor();
       TestReactor providerReactor = new TestReactor();
               
       /* Create consumer. */
       Consumer consumer = new CloseFromCallbackConsumer(consumerReactor);
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
       consumerRole.watchlistOptions().requestTimeout(3000);
       
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

       
       /* Test consumer closes in response to an update. */
       for (int i = 0; i < 3; ++i)
       {
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
           assertTrue(receivedRequestMsg.checkHasPriority());
           assertEquals(1, receivedRequestMsg.priority().priorityClass());
           assertEquals(1, receivedRequestMsg.priority().count());
           providerStreamId = receivedRequestMsg.streamId();

           /* Consumer sends request. */
           requestMsg.clear();
           requestMsg.msgClass(MsgClasses.REQUEST);
           requestMsg.streamId(6);
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
           assertTrue(receivedRequestMsg.checkHasPriority());
           assertEquals(1, receivedRequestMsg.priority().priorityClass());
           assertEquals(2, receivedRequestMsg.priority().count());
           providerStreamId = receivedRequestMsg.streamId();

           /* Provider sends refresh .*/
           refreshMsg.clear();
           refreshMsg.msgClass(MsgClasses.REFRESH);
           refreshMsg.applySolicited();
           refreshMsg.domainType(DomainTypes.MARKET_PRICE);
           refreshMsg.streamId(providerStreamId);
           refreshMsg.containerType(DataTypes.NO_DATA);
           refreshMsg.applyHasMsgKey();
           refreshMsg.msgKey().applyHasServiceId();
           refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
           refreshMsg.msgKey().applyHasName();
           refreshMsg.msgKey().name().data("TRI.N");
           refreshMsg.applyRefreshComplete();
           Buffer groupId = CodecFactory.createBuffer();
           groupId.data("1234431");
           refreshMsg.groupId(groupId);
           refreshMsg.state().streamState(StreamStates.OPEN);
           refreshMsg.state().dataState(DataStates.OK);

           assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

           /* Consumer receives refresh. */
           consumerReactor.dispatch(2);

           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.MSG, event.type());
           msgEvent = (ReactorMsgEvent)event.reactorEvent();
           assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
           receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
           assertEquals(5, receivedRefreshMsg.streamId());
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

           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.MSG, event.type());
           msgEvent = (ReactorMsgEvent)event.reactorEvent();
           assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
           receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
           assertEquals(6, receivedRefreshMsg.streamId());
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
           
           /* Provider sends an update. */
           updateMsg.clear();
           updateMsg.msgClass(MsgClasses.UPDATE);
           updateMsg.streamId(providerStreamId);
           updateMsg.domainType(DomainTypes.MARKET_PRICE);
           updateMsg.containerType(DataTypes.NO_DATA);
           assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           /* Consumer receives update (closes on first update fanout, so only receives one). */
           consumerReactor.dispatch(1);
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.MSG, event.type());
           msgEvent = (ReactorMsgEvent)event.reactorEvent();
           assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());
           receivedUpdateMsg = (UpdateMsg)msgEvent.msg();
           assertEquals(5, receivedUpdateMsg.streamId());
           assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
           assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
           assertNotNull(msgEvent.streamInfo());
           assertNotNull(msgEvent.streamInfo().serviceName());
           assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
          

           /* Provider receives reissue and close. */
           providerReactor.dispatch(2);

           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.MSG, event.type());
           msgEvent = (ReactorMsgEvent)event.reactorEvent();
           assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
           receivedRequestMsg = (RequestMsg)msgEvent.msg();
           assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
           assertTrue(receivedRequestMsg.checkStreaming());
           assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
           assertTrue(receivedRequestMsg.msgKey().checkHasName());
           assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
           assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
           assertTrue(receivedRequestMsg.checkHasPriority());
           assertEquals(1, receivedRequestMsg.priority().priorityClass());
           assertEquals(1, receivedRequestMsg.priority().count());
           providerStreamId = receivedRequestMsg.streamId();

           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.MSG, event.type());
           msgEvent = (ReactorMsgEvent)event.reactorEvent();
           assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
           receivedCloseMsg = (CloseMsg)msgEvent.msg();
           assertEquals(providerStreamId, receivedCloseMsg.streamId());
           assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());
       }

       /* Test consumer closes in response to group status. */
       for (int i = 0; i < 2; ++i)
       {
    	   /* Test closing in response to open & closed group status messages. */
           int msgStreamState = (i == 0) ? StreamStates.OPEN : StreamStates.CLOSED;
           
	       for (int j = 0; j < 3; ++j)
	       {
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
	           assertTrue(receivedRequestMsg.checkHasPriority());
	           assertEquals(1, receivedRequestMsg.priority().priorityClass());
	           assertEquals(1, receivedRequestMsg.priority().count());
	           providerStreamId = receivedRequestMsg.streamId();
	
	           /* Consumer sends request. */
	           requestMsg.clear();
	           requestMsg.msgClass(MsgClasses.REQUEST);
	           requestMsg.streamId(6);
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
	           assertTrue(receivedRequestMsg.checkHasPriority());
	           assertEquals(1, receivedRequestMsg.priority().priorityClass());
	           assertEquals(2, receivedRequestMsg.priority().count());
	           providerStreamId = receivedRequestMsg.streamId();
	
	           /* Provider sends refresh .*/
	           refreshMsg.clear();
	           refreshMsg.msgClass(MsgClasses.REFRESH);
	           refreshMsg.applySolicited();
	           refreshMsg.domainType(DomainTypes.MARKET_PRICE);
	           refreshMsg.streamId(providerStreamId);
	           refreshMsg.containerType(DataTypes.NO_DATA);
	           refreshMsg.applyHasMsgKey();
	           refreshMsg.msgKey().applyHasServiceId();
	           refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
	           refreshMsg.msgKey().applyHasName();
	           refreshMsg.msgKey().name().data("TRI.N");
	           refreshMsg.applyRefreshComplete();
	           Buffer groupId = CodecFactory.createBuffer();
	           groupId.data("5555");
	           refreshMsg.groupId(groupId);
	           refreshMsg.state().streamState(StreamStates.OPEN);
	           refreshMsg.state().dataState(DataStates.OK);
	
	           assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
	
	           /* Consumer receives refresh. */
	           consumerReactor.dispatch(2);
	
	           event = consumerReactor.pollEvent();
	           assertEquals(TestReactorEventTypes.MSG, event.type());
	           msgEvent = (ReactorMsgEvent)event.reactorEvent();
	           assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
	           receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
	           assertEquals(5, receivedRefreshMsg.streamId());
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
	
	           event = consumerReactor.pollEvent();
	           assertEquals(TestReactorEventTypes.MSG, event.type());
	           msgEvent = (ReactorMsgEvent)event.reactorEvent();
	           assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
	           receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
	           assertEquals(6, receivedRefreshMsg.streamId());
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

               /* Provider sends group status .*/
               ServiceGroup serviceGroup = new ServiceGroup();
               serviceGroup.clear();
               serviceGroup.group().data("5555");
               serviceGroup.applyHasMergedToGroup();
               serviceGroup.mergedToGroup().data("7777");
               serviceGroup.applyHasStatus();
               serviceGroup.status().streamState(msgStreamState);
               serviceGroup.status().dataState(DataStates.SUSPECT);

               Service service = DirectoryMsgFactory.createService();
               service.clear();
               service.serviceId(Provider.defaultService().serviceId());
               service.action(MapEntryActions.UPDATE);
               service.groupStateList().add(serviceGroup);

               DirectoryUpdate directoryUpdate = (DirectoryUpdate)DirectoryMsgFactory.createMsg();

               directoryUpdate.clear();
               directoryUpdate.rdmMsgType(DirectoryMsgType.UPDATE);
               directoryUpdate.streamId(provider.defaultSessionDirectoryStreamId());
               directoryUpdate.serviceList().add(service);
               assertTrue(provider.submitAndDispatch(directoryUpdate, submitOptions) >= ReactorReturnCodes.SUCCESS);

               /* Consumer receives directory update,
                * and open/suspect status (closes on first update fanout, so only receives one). */
               consumerReactor.dispatch(2);

               event = consumerReactor.pollEvent();
               assertEquals(TestReactorEventTypes.MSG, event.type());
               msgEvent = (ReactorMsgEvent)event.reactorEvent();
               assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
               receivedStatusMsg = (StatusMsg)msgEvent.msg();
               assertEquals(5, receivedStatusMsg.streamId());
               assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
               assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
               assertEquals(msgStreamState, receivedStatusMsg.state().streamState());
               assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
               assertNotNull(msgEvent.streamInfo());
               assertNotNull(msgEvent.streamInfo().serviceName());
               assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

               event = consumerReactor.pollEvent();
               assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
               directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
               assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());

               if (msgStreamState == StreamStates.OPEN)
               {
                   /* Provider receives reissue and close. */
                   providerReactor.dispatch(2);

                   event = providerReactor.pollEvent();
                   assertEquals(TestReactorEventTypes.MSG, event.type());
                   msgEvent = (ReactorMsgEvent)event.reactorEvent();
                   assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
                   receivedRequestMsg = (RequestMsg)msgEvent.msg();
                   assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
                   assertTrue(receivedRequestMsg.checkStreaming());
                   assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
                   assertTrue(receivedRequestMsg.msgKey().checkHasName());
                   assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
                   assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
                   assertTrue(receivedRequestMsg.checkHasPriority());
                   assertEquals(1, receivedRequestMsg.priority().priorityClass());
                   assertEquals(1, receivedRequestMsg.priority().count());
                   providerStreamId = receivedRequestMsg.streamId();

                   event = providerReactor.pollEvent();
                   assertEquals(TestReactorEventTypes.MSG, event.type());
                   msgEvent = (ReactorMsgEvent)event.reactorEvent();
                   assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
                   receivedCloseMsg = (CloseMsg)msgEvent.msg();
                   assertEquals(providerStreamId, receivedCloseMsg.streamId());
                   assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());
               }
               else
               {
                   /* Provider receives nothing (stream is already closed). */
                   providerReactor.dispatch(0);
               }
           }
       }
       
       TestReactorComponent.closeSession(consumer, provider);
   }

   /* Used by closeFromCallbackTest. */
   class CloseLoginStreamFromCallbackConsumer extends Consumer
   {
       public CloseLoginStreamFromCallbackConsumer (TestReactor testReactor)
       {
           super(testReactor);
       }

       @Override
       public int reactorChannelEventCallback(ReactorChannelEvent event)
       {
           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           Msg msg = CodecFactory.createMsg();
           RequestMsg requestMsg = (RequestMsg)msg;
           
           super.reactorChannelEventCallback(event);

           if (event.eventType() == ReactorChannelEventTypes.CHANNEL_OPENED)
           {
               /* Consumer sends item request. */
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(6);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.applyStreaming();
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("TRI.N");
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           }
           
           return ReactorReturnCodes.SUCCESS;
       }

       @Override
       public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
       {
           CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           
           super.rdmLoginMsgCallback(event);

           assertEquals(LoginMsgType.STATUS, event.rdmLoginMsg().rdmMsgType());
           closeMsg.clear();
           closeMsg.msgClass(MsgClasses.CLOSE);
           closeMsg.streamId(1);
           closeMsg.domainType(DomainTypes.LOGIN);
           submitOptions.clear();
           assertTrue(submit(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
           
           return ReactorReturnCodes.SUCCESS;
       }
   }

   @Test
   public void loginCloseFromCallbackTest()
   {
       for (int i = 0; i < 2; ++i)
       {
           ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
           TestReactorEvent event;
           RDMLoginMsgEvent loginMsgEvent;
           LoginStatus receivedLoginStatus;
           int provLoginStreamId;
           
           /* Create reactors. */
           TestReactor consumerReactor = new TestReactor();
           TestReactor providerReactor = new TestReactor();

           /* Create consumer. */
           Consumer consumer = new CloseLoginStreamFromCallbackConsumer(consumerReactor);
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
           consumerRole.watchlistOptions().requestTimeout(3000);

           /* Create provider. */
           Provider provider = new Provider(providerReactor);
           ProviderRole providerRole = (ProviderRole)provider.reactorRole();
           providerRole.channelEventCallback(provider);
           providerRole.loginMsgCallback(provider);
           providerRole.directoryMsgCallback(provider);
           providerRole.dictionaryMsgCallback(provider);
           providerRole.defaultMsgCallback(provider);

           /* Connect the consumer and provider. */
           ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
           opts.setupDefaultLoginStream(false);
           opts.setupDefaultDirectoryStream(false);
           opts.numStatusEvents(1);
           provider.bind(opts);
           TestReactor.openSession(consumer, provider, opts);

           /* Provider receives login request. */
           provider.testReactor().dispatch(1);
           event = provider.testReactor().pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           LoginRequest receivedLoginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
           provLoginStreamId = receivedLoginRequest.streamId();

           /* Provider sends login refresh. */
           LoginStatus loginStatus = (LoginStatus)LoginMsgFactory.createMsg();

           loginStatus.clear();
           loginStatus.rdmMsgType(LoginMsgType.STATUS);
           loginStatus.applyHasState();
           loginStatus.streamId(provLoginStreamId);

           if (i == 0)
               loginStatus.state().streamState(StreamStates.OPEN);
           else
               loginStatus.state().streamState(StreamStates.CLOSED);

           loginStatus.state().dataState(DataStates.SUSPECT);
           loginStatus.state().code(StateCodes.NOT_ENTITLED);
           loginStatus.state().text().data("Not permissioned.");

           submitOptions.clear();
           assertTrue(provider.submitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCodes.SUCCESS);

           /* Consumer receives login status. */		
           consumerReactor.dispatch(1);
           event = consumerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           receivedLoginStatus = (LoginStatus)loginMsgEvent.rdmLoginMsg();
           assertEquals(consumerRole.rdmLoginRequest().streamId(), receivedLoginStatus.streamId());
           assertTrue(receivedLoginStatus.checkHasState());
           
           if (i == 0)
               assertEquals(StreamStates.OPEN, receivedLoginStatus.state().streamState());
           else
               assertEquals(StreamStates.CLOSED, receivedLoginStatus.state().streamState());
           
           assertEquals(DataStates.SUSPECT, receivedLoginStatus.state().dataState());

           /* Provider receives login close if stream was was open. */
           providerReactor.dispatch(1);
           event = providerReactor.pollEvent();
           assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
           loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
           assertEquals(LoginMsgType.CLOSE, loginMsgEvent.rdmLoginMsg().rdmMsgType());
           assertEquals(provLoginStreamId, loginMsgEvent.rdmLoginMsg().streamId());

           TestReactorComponent.closeSession(consumer, provider);
       }
   }

    @Test
    public void redirectedTest()
    {
		/* Test receiving a StatusMsg or RefreshMsg with a REDIRECTED stream state for an item. 
         * Tests with and without RequestMsg.applyMsgKeyInUpdates(). */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();
        StatusMsg receivedStatusMsg;
        int providerStreamId;
                
        for (int i = 0 ; i < 2; ++i)
        {
            /* Create reactors. */
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();

            boolean applyMsgKeyInUpdates = (i == 0);

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

            /* -- Redirecting StatusMsg -- */

            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            if (applyMsgKeyInUpdates)
                requestMsg.applyMsgKeyInUpdates();
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

            /* Provider sends redirecting StatusMsg .*/
            statusMsg.clear();
            statusMsg.msgClass(MsgClasses.STATUS);
            statusMsg.domainType(DomainTypes.MARKET_PRICE);
            statusMsg.streamId(providerStreamId);
            statusMsg.containerType(DataTypes.NO_DATA); 

            statusMsg.applyHasState();
            statusMsg.state().streamState(StreamStates.REDIRECTED);
            statusMsg.state().dataState(DataStates.OK);

            statusMsg.applyHasMsgKey();
            statusMsg.msgKey().applyHasServiceId();
            statusMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            statusMsg.msgKey().applyHasName();
            statusMsg.msgKey().name().data("RTRSY.O"); // It's turn back the clock night!

            assertTrue(provider.submitAndDispatch(statusMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives redirecting status with new key. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

            receivedStatusMsg = (StatusMsg)msgEvent.msg();
            assertTrue(receivedStatusMsg.checkHasMsgKey());
            assertTrue(receivedStatusMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedStatusMsg.msgKey().serviceId());
            assertTrue(receivedStatusMsg.msgKey().checkHasName());
            assertTrue(receivedStatusMsg.msgKey().name().toString().equals("RTRSY.O"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
            assertTrue(receivedStatusMsg.checkHasState());
            assertEquals(StreamStates.REDIRECTED, receivedStatusMsg.state().streamState());
            assertEquals(DataStates.OK, receivedStatusMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

            /* -- Redirecting RefreshMsg -- */

            /* Consumer sends request. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI.N");
            if (applyMsgKeyInUpdates)
                requestMsg.applyMsgKeyInUpdates();
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

            /* Provider sends redirecting RefreshMsg .*/
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA); 

            refreshMsg.state().streamState(StreamStates.REDIRECTED);
            refreshMsg.state().dataState(DataStates.OK);

            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("RTRSY.O"); // It's turn back the clock night!
            refreshMsg.applySolicited();
            refreshMsg.applyRefreshComplete();

            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives redirecting refresh with new key. */
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
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("RTRSY.O"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.REDIRECTED, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

            TestReactorComponent.closeSession(consumer, provider);
        }
    }

    @Test
    public void msgKeyInUpdatesTest()
    {
        /* Test that the requestMsg.msgKeyInUpdates() flag does not appear on the wire even if requested,
         * and that the WL adds keys to Refresh/Update/Status/Generic/AckMsgs if it is requested.
         * Tests with and without RequestMsg.applyMsgKeyInUpdates().
         * Tests requesting both by service name and service ID. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
        UpdateMsg receivedUpdateMsg;
        GenericMsg genericMsg = (GenericMsg)CodecFactory.createMsg();
        GenericMsg receivedGenericMsg;
        StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();
        StatusMsg receivedStatusMsg;
        PostMsg postMsg = (PostMsg)CodecFactory.createMsg();
        PostMsg receivedPostMsg;
        AckMsg ackMsg = (AckMsg)CodecFactory.createMsg();
        AckMsg receivedAckMsg;
        int providerStreamId;

        for(int i = 0; i < 2; ++i)
        {
            for (int j = 0; j < 2; ++j)
            {
                boolean applyMsgKeyInUpdates = (i == 0);
                boolean requestByServiceName = (j == 0);

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
                consumerRole.watchlistOptions().requestTimeout(1000);
                consumerRole.watchlistOptions().postAckTimeout(1000);

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
                if (applyMsgKeyInUpdates)
                    requestMsg.applyMsgKeyInUpdates();
                submitOptions.clear();
                if (requestByServiceName)
                    submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
                else
                {
                    requestMsg.msgKey().applyHasServiceId();
                    requestMsg.msgKey().serviceId(Provider.defaultService().serviceId());
                }
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
                assertFalse(receivedRequestMsg.checkMsgKeyInUpdates());
                assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
                assertTrue(receivedRequestMsg.msgKey().checkHasName());
                assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
                assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());

                providerStreamId = receivedRequestMsg.streamId();

                /* Provider sends refresh, with no key .*/
                refreshMsg.clear();
                refreshMsg.msgClass(MsgClasses.REFRESH);
                refreshMsg.domainType(DomainTypes.MARKET_PRICE);
                refreshMsg.streamId(providerStreamId);
                refreshMsg.containerType(DataTypes.NO_DATA); 
                refreshMsg.state().streamState(StreamStates.OPEN);
                refreshMsg.state().dataState(DataStates.OK);
                refreshMsg.applySolicited();
                refreshMsg.applyRefreshComplete();

                assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

                /* Consumer receives refresh, with key present if requested. */
                consumerReactor.dispatch(1);
                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

                receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
                if (applyMsgKeyInUpdates)
                {
                    assertTrue(receivedRefreshMsg.checkHasMsgKey());
                    assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
                    assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
                    assertTrue(receivedRefreshMsg.msgKey().checkHasName());
                    assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
                }
                else
                    assertFalse(receivedRefreshMsg.checkHasMsgKey());

                assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
                assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
                assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
                assertNotNull(msgEvent.streamInfo());
                if (requestByServiceName)
                {
                    assertNotNull(msgEvent.streamInfo().serviceName());
                    assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
                }
                else
                    assertNull(msgEvent.streamInfo().serviceName());
                
                /* Provider sends update, with no key. */
                updateMsg.clear();
                updateMsg.msgClass(MsgClasses.UPDATE);
                updateMsg.domainType(DomainTypes.MARKET_PRICE);
                updateMsg.streamId(providerStreamId);
                updateMsg.containerType(DataTypes.NO_DATA); 

                assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

                /* Consumer receives update, with key present if requested. */
                consumerReactor.dispatch(1);
                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.UPDATE, msgEvent.msg().msgClass());

                receivedUpdateMsg = (UpdateMsg)msgEvent.msg();

                if (applyMsgKeyInUpdates)
                {
                    assertTrue(receivedUpdateMsg.checkHasMsgKey());
                    assertTrue(receivedUpdateMsg.msgKey().checkHasServiceId());
                    assertEquals(Provider.defaultService().serviceId(), receivedUpdateMsg.msgKey().serviceId());
                    assertTrue(receivedUpdateMsg.msgKey().checkHasName());
                    assertTrue(receivedUpdateMsg.msgKey().name().toString().equals("TRI.N"));
                }
                else
                    assertFalse(receivedUpdateMsg.checkHasMsgKey());

                assertEquals(DomainTypes.MARKET_PRICE, receivedUpdateMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedUpdateMsg.containerType());
                assertNotNull(msgEvent.streamInfo());
                if (requestByServiceName)
                {
                    assertNotNull(msgEvent.streamInfo().serviceName());
                    assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
                }
                else
                    assertNull(msgEvent.streamInfo().serviceName());

                /* Provider sends generic message, with no key.*/
                genericMsg.clear();
                genericMsg.msgClass(MsgClasses.GENERIC);
                genericMsg.domainType(DomainTypes.MARKET_PRICE);
                genericMsg.streamId(providerStreamId);
                genericMsg.containerType(DataTypes.NO_DATA); 
                assertTrue(provider.submitAndDispatch(genericMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

                /* Consumer receives generic, with key present if requested. */
                consumerReactor.dispatch(1);
                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.GENERIC, msgEvent.msg().msgClass());

                receivedGenericMsg = (GenericMsg)msgEvent.msg();

                if (applyMsgKeyInUpdates)
                {
                    assertTrue(receivedGenericMsg.checkHasMsgKey());
                    assertTrue(receivedGenericMsg.msgKey().checkHasServiceId());
                    assertEquals(Provider.defaultService().serviceId(), receivedGenericMsg.msgKey().serviceId());
                    assertTrue(receivedGenericMsg.msgKey().checkHasName());
                    assertTrue(receivedGenericMsg.msgKey().name().toString().equals("TRI.N"));
                }
                else
                    assertFalse(receivedGenericMsg.checkHasMsgKey());

                assertEquals(DomainTypes.MARKET_PRICE, receivedGenericMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedGenericMsg.containerType());
                assertNotNull(msgEvent.streamInfo());
                if (requestByServiceName)
                {
                    assertNotNull(msgEvent.streamInfo().serviceName());
                    assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
                }
                else
                    assertNull(msgEvent.streamInfo().serviceName());

                /* Consumer sends post (so it can receive AckMsg). */
                postMsg.clear();
                postMsg.msgClass(MsgClasses.POST);
                postMsg.streamId(5);
                postMsg.domainType(DomainTypes.MARKET_PRICE);
                postMsg.containerType(DataTypes.NO_DATA); 
                postMsg.applyHasPostId();
                postMsg.applyAck();
                postMsg.postId(7);
                postMsg.applyPostComplete();
                submitOptions.clear();
                assertTrue(consumer.submitAndDispatch(postMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

                /* Provider receives post. */
                providerReactor.dispatch(1);
                event = providerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.POST, msgEvent.msg().msgClass());

                receivedPostMsg = (PostMsg)msgEvent.msg();
                assertEquals(providerStreamId, receivedPostMsg.streamId());
                assertEquals(DomainTypes.MARKET_PRICE, receivedPostMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedPostMsg.containerType());
                assertTrue(receivedPostMsg.checkAck());
                assertTrue(receivedPostMsg.checkHasPostId());
                assertEquals(7, receivedPostMsg.postId());
                assertTrue(receivedPostMsg.checkPostComplete());

                /* Provider sends AckMsg for post, with no key. */
                ackMsg.msgClass(MsgClasses.ACK);
                ackMsg.streamId(providerStreamId);
                ackMsg.domainType(DomainTypes.MARKET_PRICE);
                ackMsg.ackId(7);
                assertTrue(provider.submitAndDispatch(ackMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

                /* Consumer receives AckMsg, with key present if requested. */
                consumerReactor.dispatch(1);
                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.ACK, msgEvent.msg().msgClass());

                receivedAckMsg = (AckMsg)msgEvent.msg();
                if (applyMsgKeyInUpdates)
                {
                    assertTrue(receivedAckMsg.checkHasMsgKey());
                    assertTrue(receivedAckMsg.msgKey().checkHasServiceId());
                    assertEquals(Provider.defaultService().serviceId(), receivedAckMsg.msgKey().serviceId());
                    assertTrue(receivedAckMsg.msgKey().checkHasName());
                    assertTrue(receivedAckMsg.msgKey().name().toString().equals("TRI.N"));
                }
                else
                    assertFalse(receivedAckMsg.checkHasMsgKey());
                
                assertEquals(DomainTypes.MARKET_PRICE, receivedAckMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedAckMsg.containerType());

                assertNotNull(msgEvent.streamInfo());
                if (requestByServiceName)
                {
                    assertNotNull(msgEvent.streamInfo().serviceName());
                    assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
                }
                else
                    assertNull(msgEvent.streamInfo().serviceName());

                /* -- Test internally-generated timeout nack -- */

                /* Consumer sends post. This will be nacked. */
                postMsg.clear();
                postMsg.msgClass(MsgClasses.POST);
                postMsg.streamId(5);
                postMsg.domainType(DomainTypes.MARKET_PRICE);
                postMsg.containerType(DataTypes.NO_DATA); 
                postMsg.applyHasPostId();
                postMsg.applyAck();
                postMsg.postId(7);
                postMsg.applyPostComplete();
                submitOptions.clear();
                assertTrue(consumer.submitAndDispatch(postMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

                /* Provider receives post. */
                providerReactor.dispatch(1);
                event = providerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.POST, msgEvent.msg().msgClass());

                receivedPostMsg = (PostMsg)msgEvent.msg();
                assertEquals(providerStreamId, receivedPostMsg.streamId());
                assertEquals(DomainTypes.MARKET_PRICE, receivedPostMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedPostMsg.containerType());
                assertTrue(receivedPostMsg.checkAck());
                assertTrue(receivedPostMsg.checkHasPostId());
                assertEquals(7, receivedPostMsg.postId());
                assertTrue(receivedPostMsg.checkPostComplete());

                /* Provider does not respond. */

                /* Consumer receives nack, with key present if requested. */
                consumerReactor.dispatch(1, 2000);
                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.ACK, msgEvent.msg().msgClass());

                receivedAckMsg = (AckMsg)msgEvent.msg();
                assertTrue(receivedAckMsg.checkHasNakCode());
                if (applyMsgKeyInUpdates)
                {
                    assertTrue(receivedAckMsg.checkHasMsgKey());
                    assertTrue(receivedAckMsg.msgKey().checkHasServiceId());
                    assertEquals(Provider.defaultService().serviceId(), receivedAckMsg.msgKey().serviceId());
                    assertTrue(receivedAckMsg.msgKey().checkHasName());
                    assertTrue(receivedAckMsg.msgKey().name().toString().equals("TRI.N"));
                }
                else
                    assertFalse(receivedAckMsg.checkHasMsgKey());
                
                assertEquals(DomainTypes.MARKET_PRICE, receivedAckMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedAckMsg.containerType());

                assertNotNull(msgEvent.streamInfo());
                if (requestByServiceName)
                {
                    assertNotNull(msgEvent.streamInfo().serviceName());
                    assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
                }
                else
                    assertNull(msgEvent.streamInfo().serviceName());

                /* Provider sends status, with no key .*/
                statusMsg.clear();
                statusMsg.msgClass(MsgClasses.STATUS);
                statusMsg.domainType(DomainTypes.MARKET_PRICE);
                statusMsg.streamId(providerStreamId);
                statusMsg.containerType(DataTypes.NO_DATA); 
                statusMsg.applyHasState();
                statusMsg.state().streamState(StreamStates.OPEN);
                statusMsg.state().dataState(DataStates.SUSPECT);

                assertTrue(provider.submitAndDispatch(statusMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

                /* Consumer receives status, with key present if requested. */
                consumerReactor.dispatch(1);
                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

                receivedStatusMsg = (StatusMsg)msgEvent.msg();
                if (applyMsgKeyInUpdates)
                {
                    assertTrue(receivedStatusMsg.checkHasMsgKey());
                    assertTrue(receivedStatusMsg.msgKey().checkHasServiceId());
                    assertEquals(Provider.defaultService().serviceId(), receivedStatusMsg.msgKey().serviceId());
                    assertTrue(receivedStatusMsg.msgKey().checkHasName());
                    assertTrue(receivedStatusMsg.msgKey().name().toString().equals("TRI.N"));
                }
                else
                    assertFalse(receivedStatusMsg.checkHasMsgKey());

                assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
                assertTrue(statusMsg.checkHasState());
                assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
                assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
                assertNotNull(msgEvent.streamInfo());
                if (requestByServiceName)
                {
                    assertNotNull(msgEvent.streamInfo().serviceName());
                    assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
                }
                else
                    assertNull(msgEvent.streamInfo().serviceName());

                /* -- Test internally-generated status msg. -- */

                /* Consumer sends request. */
                requestMsg.clear();
                requestMsg.msgClass(MsgClasses.REQUEST);
                requestMsg.streamId(6);
                requestMsg.domainType(DomainTypes.MARKET_PRICE);
                requestMsg.applyStreaming();
                requestMsg.msgKey().applyHasName();
                requestMsg.msgKey().name().data("TRI.N");
                if (applyMsgKeyInUpdates)
                    requestMsg.applyMsgKeyInUpdates();
                submitOptions.clear();
                if (requestByServiceName)
                    submitOptions.serviceName("UNKNOWN_SERVICE");
                else
                {
                    requestMsg.msgKey().applyHasServiceId();
                    requestMsg.msgKey().serviceId(1 + Provider.defaultService().serviceId());
                }
                assertTrue(consumer.submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

                /* Provider receives nothing. */
                providerReactor.dispatch(0);

                /* Consumer receives status, with key present if requested. */
                consumerReactor.dispatch(1);
                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

                receivedStatusMsg = (StatusMsg)msgEvent.msg();
                if (applyMsgKeyInUpdates)
                {
                    assertTrue(receivedStatusMsg.checkHasMsgKey());
                    assertTrue(receivedStatusMsg.msgKey().name().toString().equals("TRI.N"));
                    
                    if (requestByServiceName)
                    {
                        assertFalse(receivedStatusMsg.msgKey().checkHasServiceId()); /* Request used name of a nonexistent service, so WL doesn't have an ID to add. */
                        assertTrue(receivedStatusMsg.msgKey().checkHasName());
                        
                    }
                    else
                    {
                        assertTrue(receivedStatusMsg.msgKey().checkHasServiceId());
                        assertEquals(1 + Provider.defaultService().serviceId(), receivedStatusMsg.msgKey().serviceId());
                    }
                }
                else
                    assertFalse(receivedStatusMsg.checkHasMsgKey());

                assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
                assertTrue(statusMsg.checkHasState());
                assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
                assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
                assertNotNull(msgEvent.streamInfo());
                if (requestByServiceName)
                {
                    assertNotNull(msgEvent.streamInfo().serviceName());
                    assertTrue(msgEvent.streamInfo().serviceName().equals("UNKNOWN_SERVICE"));
                }
                else
                    assertNull(msgEvent.streamInfo().serviceName());

                /* -- Test internally-generated request-timeout StatusMsg. -- */

                /* Consumer sends request for a new item. */
                requestMsg.clear();
                requestMsg.msgClass(MsgClasses.REQUEST);
                requestMsg.streamId(7);
                requestMsg.domainType(DomainTypes.MARKET_PRICE);
                requestMsg.applyStreaming();
                requestMsg.msgKey().applyHasName();
                requestMsg.msgKey().name().data("IBM.N");
                if (applyMsgKeyInUpdates)
                    requestMsg.applyMsgKeyInUpdates();
                submitOptions.clear();
                if (requestByServiceName)
                    submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
                else
                {
                    requestMsg.msgKey().applyHasServiceId();
                    requestMsg.msgKey().serviceId(Provider.defaultService().serviceId());
                }
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
                assertFalse(receivedRequestMsg.checkMsgKeyInUpdates());
                assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
                assertTrue(receivedRequestMsg.msgKey().checkHasName());
                assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
                assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());

                /* Provider sends no response. */

                /* Consumer receives status, with key present if requested. */
                consumerReactor.dispatch(1, 2000);
                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

                receivedStatusMsg = (StatusMsg)msgEvent.msg();
                if (applyMsgKeyInUpdates)
                {
                    assertTrue(receivedStatusMsg.checkHasMsgKey());
                    assertTrue(receivedStatusMsg.msgKey().checkHasName());
                    assertTrue(receivedStatusMsg.msgKey().name().toString().equals("IBM.N"));
                    assertTrue(receivedStatusMsg.msgKey().checkHasServiceId());
                    assertEquals(Provider.defaultService().serviceId(), receivedStatusMsg.msgKey().serviceId());
                }
                else
                    assertFalse(receivedStatusMsg.checkHasMsgKey());

                assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
                assertTrue(statusMsg.checkHasState());
                assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
                assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
                assertNotNull(msgEvent.streamInfo());
                if (requestByServiceName)
                {
                    assertNotNull(msgEvent.streamInfo().serviceName());
                    assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
                }
                else
                    assertNull(msgEvent.streamInfo().serviceName());
                
                
                /* Provider receives close & re-request again. */
                providerReactor.dispatch(2);
                
                event = providerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());

                event = providerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

                receivedRequestMsg = (RequestMsg)msgEvent.msg();
                assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
                assertTrue(receivedRequestMsg.checkStreaming());
                assertFalse(receivedRequestMsg.checkNoRefresh());
                assertFalse(receivedRequestMsg.checkMsgKeyInUpdates());
                assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
                assertTrue(receivedRequestMsg.msgKey().checkHasName());
                assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
                assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());

                TestReactorComponent.closeSession(consumer, provider);
            }
        }
    }
    
    @Test
    public void itemViewAggregateStreamingTest()
    {
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg1 = (RequestMsg)CodecFactory.createMsg();
        RequestMsg requestMsg2 = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
        List<Integer> viewFieldList = new ArrayList<Integer>();
                
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
        
        // submit two aggregated request messages
        requestMsg1.clear();
        requestMsg1.msgClass(MsgClasses.REQUEST);
        requestMsg1.streamId(5);
        requestMsg1.domainType(DomainTypes.MARKET_PRICE);
        requestMsg1.applyStreaming();
        requestMsg1.applyHasView();
        requestMsg1.msgKey().applyHasName();
        requestMsg1.msgKey().name().data("VRX");
        viewFieldList.add(22);
        viewFieldList.add(22);
        viewFieldList.add(6);
        viewFieldList.add(0);
        viewFieldList.add(130);
        viewFieldList.add(1131);
        viewFieldList.add(1025);				
        encodeViewFieldIdList(consumer.reactorChannel(), viewFieldList, requestMsg1);

        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg1, submitOptions) >= ReactorReturnCodes.SUCCESS);
  
        /* Provider receives one request. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
  
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
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
        refreshMsg.msgKey().name().data("VRX");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
  
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
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));        
        
        
        // Consumer send 2nd request
        requestMsg2.clear();
        requestMsg2.msgClass(MsgClasses.REQUEST);
        requestMsg2.streamId(6);
        requestMsg2.domainType(DomainTypes.MARKET_PRICE);
        requestMsg2.applyStreaming();
        requestMsg2.applyHasView();
        requestMsg2.msgKey().applyHasName();
        requestMsg2.msgKey().name().data("VRX");
        viewFieldList.clear();
        viewFieldList.add(8);
        viewFieldList.add(88);
        viewFieldList.add(130);
        viewFieldList.add(24);
        viewFieldList.add(989);
        viewFieldList.add(45);
        viewFieldList.add(45);
        encodeViewFieldIdList(consumer.reactorChannel(), viewFieldList, requestMsg2);
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg2, submitOptions) >= ReactorReturnCodes.SUCCESS);
               
        /* Provider receives one request. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
  
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
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
        refreshMsg.msgKey().name().data("VRX");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
  
       
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
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        // 2nd event to consumer
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));       
        
        ///////////////////////////////////////////////////////////
        // consumer reissues 2nd request without view
        requestMsg2.clear();
        requestMsg2.msgClass(MsgClasses.REQUEST);
        requestMsg2.streamId(6);
        requestMsg2.domainType(DomainTypes.MARKET_PRICE);
        requestMsg2.applyStreaming();
//        requestMsg2.applyHasView();
        requestMsg2.msgKey().applyHasName();
        requestMsg2.msgKey().name().data("VRX");

        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg2, submitOptions) >= ReactorReturnCodes.SUCCESS);
              
        /* Provider receives request. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(3, receivedRequestMsg.streamId()); // stream id should be same as first request
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        // NO View
        assertFalse(receivedRequestMsg.checkHasView()); 
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
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
        refreshMsg.msgKey().name().data("VRX");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
  
       
        consumerReactor.dispatch(2);
        event = consumerReactor.pollEvent(); 
        event = consumerReactor.pollEvent(); 
        
        // consumer reissues 1st request with pause
        requestMsg1.applyPause();
        assertTrue(consumer.submitAndDispatch(requestMsg1, submitOptions) >= ReactorReturnCodes.SUCCESS);
                 
        // consumer reissues 2nd request with pause
        requestMsg2.applyPause();
        assertTrue(consumer.submitAndDispatch(requestMsg2, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Provider receives pause request. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(3, receivedRequestMsg.streamId()); // stream id should be same as first request
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming()); 
        // PAUSE 
        assertTrue(receivedRequestMsg.checkPause()); 
        providerStreamId = receivedRequestMsg.streamId();       
                
                   
        // resume
        // consumer reissues 1st request with no pause
        requestMsg1.flags(requestMsg1.flags() & ~RequestMsgFlags.PAUSE);
        assertTrue(consumer.submitAndDispatch(requestMsg1, submitOptions) >= ReactorReturnCodes.SUCCESS);
               
        /* Provider receives request. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(3, receivedRequestMsg.streamId()); // stream id should be same as first request
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        // NO PAUSE now
        assertFalse(receivedRequestMsg.checkPause()); 
        providerStreamId = receivedRequestMsg.streamId();
         
        TestReactorComponent.closeSession(consumer, provider);
    }   
    
    
    @Test
    public void itemViewAggregateSnapshotBeforeChannelReadyTest()
    {
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;
                
        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();
                
        /* Create consumer. */
        Consumer consumer = new itemViewAggregateSnapshotBeforeChannelReadyTest(consumerReactor);
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
        opts.numStatusEvents(2); // set number of expected status message from request submitted in channel open callback
        TestReactor.openSession(consumer, provider, opts);

        /* Provider receives one request. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
  
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
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
        refreshMsg.msgKey().name().data("VRX");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
  
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
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));        
        
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));        

        /* Provider receives one view request. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
  
        providerStreamId = receivedRequestMsg.streamId();
        
        /* Provider sends refresh .*/
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
        refreshMsg.msgKey().name().data("VRX");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
       
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
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
         
        TestReactorComponent.closeSession(consumer, provider);
    }   
    
 	private void encodeViewFieldIdList(ReactorChannel rc, List<Integer> fieldIdList, RequestMsg msg)
 	{
 		Buffer buf = CodecFactory.createBuffer();
 		buf.data(ByteBuffer.allocate(1024));	
 		Int tempInt = CodecFactory.createInt();
 		UInt tempUInt = CodecFactory.createUInt();
 		Array viewArray = CodecFactory.createArray();
 		EncodeIterator encodeIter = CodecFactory.createEncodeIterator();
 		encodeIter.setBufferAndRWFVersion(buf, rc.majorVersion(), rc.minorVersion());
 		ElementList elementList = CodecFactory.createElementList();
 		ElementEntry elementEntry = CodecFactory.createElementEntry();		
 		ArrayEntry arrayEntry = CodecFactory.createArrayEntry();

 		elementList.applyHasStandardData();
 		assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeInit(encodeIter, null, 0));
 	       
 		elementEntry.clear();
 		elementEntry.name(ElementNames.VIEW_TYPE);
 		elementEntry.dataType(DataTypes.UINT);

 		tempUInt.value(ViewTypes.FIELD_ID_LIST);
  
         assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encode(encodeIter, tempUInt));
 		
         elementEntry.clear();
         elementEntry.name(ElementNames.VIEW_DATA);
         elementEntry.dataType(DataTypes.ARRAY);
         assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeInit(encodeIter, 0));
         viewArray.primitiveType(DataTypes.INT);
         viewArray.itemLength(2);

         assertEquals(CodecReturnCodes.SUCCESS, viewArray.encodeInit(encodeIter));

         for (Integer viewField : fieldIdList)
         {
         	arrayEntry.clear();
         	tempInt.value(viewField);
         	assertEquals(CodecReturnCodes.SUCCESS, arrayEntry.encode(encodeIter, tempInt));
         }
         assertEquals(CodecReturnCodes.SUCCESS, viewArray.encodeComplete(encodeIter, true));
         assertEquals(CodecReturnCodes.SUCCESS, elementEntry.encodeComplete(encodeIter, true));
         assertEquals(CodecReturnCodes.SUCCESS, elementList.encodeComplete(encodeIter, true));	
 		
         msg.containerType(DataTypes.ELEMENT_LIST);
         msg.encodedDataBody(buf);		
 	
 	}
    
    @Test
    public void itemRequestMultipleTimeoutTestWithServiceUpdate()
    {
        /* Test multiple request timeouts with a service update and make sure item fanout happens only once. */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
                
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
        consumerRole.watchlistOptions().requestTimeout(3000);
        
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
    
        /* Provider waits for request timeout. */
        try
        {
			Thread.sleep(5000);
		}
        catch (InterruptedException e1)
        {
			assert(false);
		}
        /* Consumer dispatches timeout and gets status message. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        StatusMsg statusMsg = (StatusMsg)msgEvent.msg();
        assertTrue(statusMsg.checkHasState());
        assertTrue(statusMsg.state().text().toString().equals("Request timeout"));
        
        /* Provider receives close and request. */
        providerReactor.dispatch(2);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
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

        /* Provider waits for request timeout. */
        try
        {
			Thread.sleep(5000);
		}
        catch (InterruptedException e1)
        {
			assert(false);
		}
        /* Consumer dispatches timeout and gets status message. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        statusMsg = (StatusMsg)msgEvent.msg();
        assertTrue(statusMsg.checkHasState());
        assertTrue(statusMsg.state().text().toString().equals("Request timeout"));
        
        /* Provider receives close and request. */
        providerReactor.dispatch(2);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
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
        
        /* Provider sends service update with ServiceState of 0.*/
        DirectoryUpdate directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE);
        
        WlService wlService = new WlService();
        wlService.rdmService().applyHasState();
        wlService.rdmService().action(MapEntryActions.UPDATE);
        wlService.rdmService().state().applyHasAcceptingRequests();
        wlService.rdmService().state().acceptingRequests(1);
        wlService.rdmService().state().serviceState(0);
        wlService.rdmService().state().applyHasStatus();
        wlService.rdmService().state().status().dataState(DataStates.SUSPECT);
        wlService.rdmService().state().status().streamState(StreamStates.OPEN);
        wlService.rdmService().serviceId(1);
        
        directoryUpdateMsg.serviceList().add(wlService.rdmService());

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Consumer receives update and status message. */
        consumerReactor.dispatch(2);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        RDMDirectoryMsgEvent directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        DirectoryUpdate receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        
        TestReactorComponent.closeSession(consumer, provider);
    }

    @Test
    public void loginReissue_Scenario_A_Test()
    {
        String test = "loginReissue_Scenario_A_Test()";
        System.out.println("\n" + test + " Running...");
        System.out.println("/*   CONS                 WatchList                 PROV\n" +
                           "   1) |    Cons Request[0] -> |                       |\n" +
                           "   2) |                       |    Prov Request[0] -> |\n" +
                           "   3) |    Cons Request[1] -> |                       |\n" +
                           "   4) |                       | <- Prov Refresh[0]    |\n" +
                           "   5) | <- Cons Refresh[0]    |                       |\n" +
                           "   6) |                       |    Prov Request[1] -> |\n" +
                           "   7) |                       | <- Prov Refresh[1]    |\n" +
                           "   8) | <- Cons Refresh[1]    |                       |\n" +
                           "*/");

        int loginStreamId;
        TestReactorEvent event;
        RDMLoginMsgEvent loginMsgEvent;
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        
        LoginRequest[] consRequest = { null, null };
        LoginRefresh[] consRefresh = { null, null };
        LoginRequest[] provRequest = { null, null };
        LoginRefresh[] provRefresh = { null, null };
        
        // Data arrays - index [0] is used for the initial request and refresh, the other indices are for the subsequent reissue requests and refreshes.
        String[] userNames = { "userName_0", "userName_1" };
        String[] authenticationTokens = { "authenticationToken_0", "authenticationToken_1" };
        String[] authenticationExts = { "authenticationExt_0", "authenticationExt_1" };
        String[] authenticationExtResps = { "authenticationExtResp_0", "authenticationExtResp_1", };
        long[] authenticationTTReissues = { 123123000, 123123001 };

        int[] userNameTypes = { Login.UserIdTypes.NAME, Login.UserIdTypes.EMAIL_ADDRESS, Login.UserIdTypes.TOKEN, Login.UserIdTypes.COOKIE, Login.UserIdTypes.AUTHN_TOKEN };
        for (int userNameType : userNameTypes)
        {
            System.out.println(test + " loop: userNameType = " + userNameType);
            
            // Create reactors
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();
            consumerReactor._reactor._reactorOptions.enableXmlTracing();
            providerReactor._reactor._reactorOptions.enableXmlTracing();
     
            // Create consumer and the initial login request message with data using index [0]
            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();

            System.out.println(test + " 1) Consumer creating login request[0]");
            consRequest[0] = consumerRole.rdmLoginRequest();
            loginStreamId = consRequest[0].streamId();
            consRequest[0].applyHasUserNameType();
            consRequest[0].userNameType(userNameType);
            
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[0].userName().data(authenticationTokens[0]);
                consRequest[0].applyHasAuthenticationExtended();
                consRequest[0].authenticationExtended().data(authenticationExts[0]);
            }
            else
                consRequest[0].userName().data(userNames[0]);
            
            consumerRole.channelEventCallback(consumer);
            consumerRole.loginMsgCallback(consumer);
            consumerRole.directoryMsgCallback(consumer);
            consumerRole.dictionaryMsgCallback(consumer);
            consumerRole.defaultMsgCallback(consumer);
            consumerRole.watchlistOptions().enableWatchlist(true);
            consumerRole.watchlistOptions().channelOpenCallback(consumer);

            // Create provider.
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.reactorRole();
            providerRole.channelEventCallback(provider);
            providerRole.loginMsgCallback(provider);
            providerRole.directoryMsgCallback(provider);
            providerRole.dictionaryMsgCallback(provider);
            providerRole.defaultMsgCallback(provider);
     
            // Connect the consumer and provider. Disable the automatic setup of login & directory streams.
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.setupDefaultLoginStream(false);
            opts.setupDefaultDirectoryStream(false);
            provider.bind(opts);
            System.out.println(test + " 1) Consumer sending login request[0]");
            TestReactor.openSession(consumer, provider, opts);
            System.out.println(test + " 1) Consumer sent login request[0]");

            System.out.println(test + " 2) Provider dispatching, expects login request[0]");
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            System.out.println(test + " 2) Provider received login request[0]");
            
            provRequest[0] = (LoginRequest)loginMsgEvent.rdmLoginMsg();
            assertEquals(provRequest[0].streamId(), loginStreamId);
            assertTrue(provRequest[0].checkHasUserNameType());
            assertEquals(provRequest[0].userNameType(), userNameType);
            
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                assertEquals(provRequest[0].userName().toString(), authenticationTokens[0]);
                assertTrue(provRequest[0].checkHasAuthenticationExtended());
                assertEquals(provRequest[0].authenticationExtended().toString(), authenticationExts[0]);
            }
            else
                assertEquals(provRequest[0].userName().toString(), userNames[0]);
            System.out.println(test + " 2) Provider validated login request[0]");

            System.out.println(test + " 3) Consumer creating login request[1]");
            consumerRole.initDefaultRDMLoginRequest();
            consRequest[1] = consumerRole.rdmLoginRequest();
            assertNotNull(consRequest[1]);
            consRequest[1].applyHasUserNameType();
            consRequest[1].userNameType(userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[1].userName().data(authenticationTokens[1]);
                consRequest[1].applyHasAuthenticationExtended();
                consRequest[1].authenticationExtended().data(authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                consRequest[1].userName().data(userNames[1]);
            else
                consRequest[1].userName().data(userNames[0]);
            
            System.out.println(test + " 3) Consumer sending login request[1]");
            submitOptions.clear();
            assertTrue(consumer.submitAndDispatch(consRequest[1], submitOptions) >= ReactorReturnCodes.SUCCESS);
            System.out.println(test + " 3) Consumer sent login request[1]");

            provider.testReactor().dispatch(0);
            System.out.println(test + " 3.1) Confirmed Watchlist did not send request[1] before receiving refresh[0]");

            System.out.println(test + " 4) Provider creating login refresh[0]");
            provRefresh[0] = (LoginRefresh)LoginMsgFactory.createMsg();
            provRefresh[0].clear();
            provRefresh[0].rdmMsgType(LoginMsgType.REFRESH);
            provRefresh[0].streamId(loginStreamId);
            provRefresh[0].applySolicited();
            provRefresh[0].applyHasUserNameType();
            provRefresh[0].userNameType(userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[0].userName().data(userNames[0]);
                provRefresh[0].applyHasAuthenticationTTReissue();
                provRefresh[0].authenticationTTReissue(authenticationTTReissues[0]);
                provRefresh[0].applyHasAuthenticationExtendedResp();
                provRefresh[0].authenticationExtendedResp().data(authenticationExtResps[0]);
            }
            else
            {
                provRefresh[0].applyHasUserName();
                provRefresh[0].userName().data(userNames[0]);
            }
            provRefresh[0].state().streamState(StreamStates.OPEN);
            provRefresh[0].state().dataState(DataStates.OK);
     
            System.out.println(test + " 4) Provider sending login refresh[0]");
            submitOptions.clear();
            assertTrue(provider.submitAndDispatch(provRefresh[0], submitOptions) >= ReactorReturnCodes.SUCCESS);
            System.out.println(test + " 4) Provider sent login refresh[0]");

            System.out.println(test + " 5) Consumer dispatching, expects login refresh[0] (if so, will send Directory request)");
            consumer.testReactor().dispatch(1);
            event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            assertEquals(MsgClasses.REFRESH, loginMsgEvent.msg().msgClass());
            System.out.println(test + " 5) Consumer received login refresh[0]");
            
            consRefresh[0] = (LoginRefresh)loginMsgEvent.rdmLoginMsg();
            assertEquals(consRefresh[0].streamId(), loginStreamId);
            assertEquals(consRefresh[0].state().streamState(), StreamStates.OPEN);
            assertEquals(consRefresh[0].state().dataState(), DataStates.OK);
            assertTrue(consRefresh[0].checkHasUserName());
            assertTrue(consRefresh[0].checkHasUserNameType());
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                assertTrue(consRefresh[0].userName().data().get() == 0x0);
                assertTrue(consRefresh[0].checkHasAuthenticationTTReissue());
                assertEquals(consRefresh[0].authenticationTTReissue(), authenticationTTReissues[0]);
                assertTrue(consRefresh[0].checkHasAuthenticationExtendedResp());
                assertEquals(consRefresh[0].authenticationExtendedResp().toString(), authenticationExtResps[0]);
            }
            else
                assertEquals(consRefresh[0].userName().toString(), userNames[0]);
            System.out.println(test + " 5) Consumer validated login refresh[0]");
            
            System.out.println(test + " 6) Provider dispatching, expects Login request[1] and Directory request[1]");
            provider.testReactor().dispatch(2);
            // validate Login request[1]
            event = providerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            LoginRequest loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
            assertEquals(loginRequest.streamId(), loginStreamId);
            assertTrue(loginRequest.checkHasUserNameType());
            assertEquals(loginRequest.userNameType(), userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                assertEquals(loginRequest.userName().toString(), authenticationTokens[1]);
                assertTrue(loginRequest.checkHasAuthenticationExtended());
                assertEquals(loginRequest.authenticationExtended().toString(), authenticationExts[1]);
            }
            else if(userNameType == Login.UserIdTypes.TOKEN)
                assertEquals(loginRequest.userName().toString(), userNames[1]);
            else
                assertEquals(loginRequest.userName().toString(), userNames[0]);
            System.out.println(test + " 6) Provider validated login request[1]");

            // validate Directory request[1]
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
            System.out.println(test + " 6) Provider received Directory request[1]");
            System.out.println(test + " 6) Provider does not validate Directory request[1]");

            System.out.println(test + " 7) Prov creating login refresh[1]");
            provRefresh[1] = (LoginRefresh)LoginMsgFactory.createMsg();
            provRefresh[1].clear();
            provRefresh[1].rdmMsgType(LoginMsgType.REFRESH);
            provRefresh[1].streamId(loginStreamId);
            provRefresh[1].applySolicited();
            provRefresh[1].applyHasUserNameType();
            provRefresh[1].userNameType(userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[1].userName().data(userNames[1]);
                provRefresh[1].applyHasAuthenticationTTReissue();
                provRefresh[1].authenticationTTReissue(authenticationTTReissues[1]);
                provRefresh[1].applyHasAuthenticationExtendedResp();
                provRefresh[1].authenticationExtendedResp().data(authenticationExtResps[1]);
            }
            else
            {
                provRefresh[1].applyHasUserName();
                provRefresh[1].userName().data(userNames[1]);
            }
            provRefresh[1].state().streamState(StreamStates.OPEN);
            provRefresh[1].state().dataState(DataStates.OK);
     
            System.out.println(test + " 7) Prov sending login refresh[1]");
            submitOptions.clear();
            assertTrue(provider.submitAndDispatch(provRefresh[1], submitOptions) >= ReactorReturnCodes.SUCCESS);
            System.out.println(test + " 7) Prov sent login refresh[1]");

            System.out.println(test + " 8) Consumer dispatching, expects login refresh[1]");
            consumer.testReactor().dispatch(1);
            event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            assertEquals(MsgClasses.REFRESH, loginMsgEvent.msg().msgClass());
            System.out.println(test + " 8) Consumer received login refresh[1]");
            
            consRefresh[1] = (LoginRefresh)loginMsgEvent.rdmLoginMsg();
            assertEquals(consRefresh[1].streamId(), loginStreamId);
            assertEquals(consRefresh[1].state().streamState(), StreamStates.OPEN);
            assertEquals(consRefresh[1].state().dataState(), DataStates.OK);
            assertTrue(consRefresh[1].checkHasUserName());
            assertTrue(consRefresh[1].checkHasUserNameType());
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                assertTrue(consRefresh[1].userName().data().get() == 0x0);
                assertTrue(consRefresh[1].checkHasAuthenticationTTReissue());
                assertEquals(consRefresh[1].authenticationTTReissue(), authenticationTTReissues[1]);
                assertTrue(consRefresh[1].checkHasAuthenticationExtendedResp());
                assertEquals(consRefresh[1].authenticationExtendedResp().toString(), authenticationExtResps[1]);
            }
            else
                assertEquals(consRefresh[1].userName().toString(), userNames[1]);
            System.out.println(test + " 8) Consumer validated login refresh[1]");
        }
        System.out.println(test + " Done\n");
    }
        
    @Test
    public void loginReissue_Scenario_B_Test()
    {
        String test = "loginReissue_Scenario_B_Test()";
        System.out.println("\n" + test + " Running...");
        System.out.println("/*   CONS                 WatchList                 PROV\n" +
                           "   1) |    Cons Request[0] -> |                       |\n" +
                           "   2) |                       |    Prov Request[0] -> |\n" +
                           "   3) |    Cons Request[1] -> |                       |\n" +
                           "   4) |    Cons Request[2] -> |                       |\n" +
                           "   5) |                       | <- Prov Refresh[0]    |\n" +
                           "   6) | <- Cons Refresh[0]    |                       |\n" +
                           "   7) |                       |    Prov Request[2] -> |\n" +
                           "   8) |                       | <- Prov Refresh[2]    |\n" +
                           "   9) | <- Cons Refresh[2]    |                       |\n" +
                           "*/");
        int loginStreamId;
        TestReactorEvent event;
        RDMLoginMsgEvent loginMsgEvent;
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        
        LoginRequest[] consRequest = { null, null, null };
        LoginRefresh[] consRefresh = { null, null, null };
        LoginRequest[] provRequest = { null, null, null };
        LoginRefresh[] provRefresh = { null, null, null };
        
        // Data arrays - index [0] is used for the initial request and refresh, the other indices are for the subsequent reissue requests and refreshes.
        String[] userNames = { "userName_0", "userName_1", "userName_2" };
        String[] authenticationTokens = { "authenticationToken_0", "authenticationToken_1", "authenticationToken_2" };
        String[] authenticationExts = { "authenticationExt_0", "authenticationExt_1", "authenticationExt_2" };
        String[] authenticationExtResps = { "authenticationExtResp_0", "authenticationExtResp_1", "authenticationExtResp_2" };
        long[] authenticationTTReissues = { 123123000, 123123001, 123123002 };

        int[] userNameTypes = { Login.UserIdTypes.NAME, Login.UserIdTypes.EMAIL_ADDRESS, Login.UserIdTypes.TOKEN, Login.UserIdTypes.COOKIE, Login.UserIdTypes.AUTHN_TOKEN };
        for (int userNameType : userNameTypes)
        {
            System.out.println(test + " loop: userNameType = " + userNameType);
            
            // Create reactors
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();
            consumerReactor._reactor._reactorOptions.enableXmlTracing();
            providerReactor._reactor._reactorOptions.enableXmlTracing();
     
            // Create consumer and the initial login request message with data using index [0]
            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();

            System.out.println(test + " 1) Consumer creating login request[0]");
            consRequest[0] = consumerRole.rdmLoginRequest();
            loginStreamId = consRequest[0].streamId();
            consRequest[0].applyHasUserNameType();
            consRequest[0].userNameType(userNameType);
            
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[0].userName().data(authenticationTokens[0]);
                consRequest[0].applyHasAuthenticationExtended();
                consRequest[0].authenticationExtended().data(authenticationExts[0]);
            }
            else
                consRequest[0].userName().data(userNames[0]);
            
            consumerRole.channelEventCallback(consumer);
            consumerRole.loginMsgCallback(consumer);
            consumerRole.directoryMsgCallback(consumer);
            consumerRole.dictionaryMsgCallback(consumer);
            consumerRole.defaultMsgCallback(consumer);
            consumerRole.watchlistOptions().enableWatchlist(true);
            consumerRole.watchlistOptions().channelOpenCallback(consumer);

            // Create provider.
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.reactorRole();
            providerRole.channelEventCallback(provider);
            providerRole.loginMsgCallback(provider);
            providerRole.directoryMsgCallback(provider);
            providerRole.dictionaryMsgCallback(provider);
            providerRole.defaultMsgCallback(provider);
     
            // Connect the consumer and provider. Disable the automatic setup of login & directory streams.
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.setupDefaultLoginStream(false);
            opts.setupDefaultDirectoryStream(false);
            provider.bind(opts);
            System.out.println(test + " 1) Consumer sending login request[0]");
            TestReactor.openSession(consumer, provider, opts);
            System.out.println(test + " 1) Consumer sent login request[0]");

            System.out.println(test + " 2) Provider dispatching, expects login request[0]");
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            System.out.println(test + " 2) Provider received login request[0]");
            
            provRequest[0] = (LoginRequest)loginMsgEvent.rdmLoginMsg();
            assertEquals(provRequest[0].streamId(), loginStreamId);
            assertTrue(provRequest[0].checkHasUserNameType());
            assertEquals(provRequest[0].userNameType(), userNameType);
            
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                assertEquals(provRequest[0].userName().toString(), authenticationTokens[0]);
                assertTrue(provRequest[0].checkHasAuthenticationExtended());
                assertEquals(provRequest[0].authenticationExtended().toString(), authenticationExts[0]);
            }
            else
                assertEquals(provRequest[0].userName().toString(), userNames[0]);
            System.out.println(test + " 2) Provider validated login request[0]");

            System.out.println(test + " 3) Consumer creating login request[1]");
            consumerRole.initDefaultRDMLoginRequest();
            consRequest[1] = consumerRole.rdmLoginRequest();
            assertNotNull(consRequest[1]);
            consRequest[1].applyHasUserNameType();
            consRequest[1].userNameType(userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[1].userName().data(authenticationTokens[1]);
                consRequest[1].applyHasAuthenticationExtended();
                consRequest[1].authenticationExtended().data(authenticationExts[1]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                consRequest[1].userName().data(userNames[1]);
            else
                consRequest[1].userName().data(userNames[0]);
            
            System.out.println(test + " 3) Consumer sending login request[1]");
            submitOptions.clear();
            assertTrue(consumer.submitAndDispatch(consRequest[1], submitOptions) >= ReactorReturnCodes.SUCCESS);
            System.out.println(test + " 3) Consumer sent login request[1]");

            provider.testReactor().dispatch(0);
            System.out.println(test + " 3.1) Confirmed Watchlist did not send request[1] before receiving refresh[0]");

            System.out.println(test + " 4) Consumer creating login request[2]");
            consumerRole.initDefaultRDMLoginRequest();
            consRequest[2] = consumerRole.rdmLoginRequest();
            assertNotNull(consRequest[2]);
            consRequest[2].applyHasUserNameType();
            consRequest[2].userNameType(userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                consRequest[2].userName().data(authenticationTokens[2]);
                consRequest[2].applyHasAuthenticationExtended();
                consRequest[2].authenticationExtended().data(authenticationExts[2]);
            }
            else if (userNameType == Login.UserIdTypes.TOKEN)
                consRequest[2].userName().data(userNames[2]);
            else
                consRequest[2].userName().data(userNames[0]);
            
            System.out.println(test + " 4) Consumer sending login request[2]");
            submitOptions.clear();
            assertTrue(consumer.submitAndDispatch(consRequest[2], submitOptions) >= ReactorReturnCodes.SUCCESS);
            System.out.println(test + " 4) Consumer sent login request[2]");

            provider.testReactor().dispatch(0);
            System.out.println(test + " 4.1) Confirmed Watchlist did not send request[1] before receiving refresh[0]");

            System.out.println(test + " 5) Provider creating login refresh[0]");
            provRefresh[0] = (LoginRefresh)LoginMsgFactory.createMsg();
            provRefresh[0].clear();
            provRefresh[0].rdmMsgType(LoginMsgType.REFRESH);
            provRefresh[0].streamId(loginStreamId);
            provRefresh[0].applySolicited();
            provRefresh[0].applyHasUserNameType();
            provRefresh[0].userNameType(userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[0].userName().data(userNames[0]);
                provRefresh[0].applyHasAuthenticationTTReissue();
                provRefresh[0].authenticationTTReissue(authenticationTTReissues[0]);
                provRefresh[0].applyHasAuthenticationExtendedResp();
                provRefresh[0].authenticationExtendedResp().data(authenticationExtResps[0]);
            }
            else
            {
                provRefresh[0].applyHasUserName();
                provRefresh[0].userName().data(userNames[0]);
            }
            provRefresh[0].state().streamState(StreamStates.OPEN);
            provRefresh[0].state().dataState(DataStates.OK);
     
            System.out.println(test + " 5) Provider sending login refresh[0]");
            submitOptions.clear();
            assertTrue(provider.submitAndDispatch(provRefresh[0], submitOptions) >= ReactorReturnCodes.SUCCESS);
            System.out.println(test + " 5) Provider sent login refresh[0]");

            System.out.println(test + " 6) Consumer dispatching, expects login refresh[0] (if so, will send Directory request)");
            consumer.testReactor().dispatch(1);
            event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            assertEquals(MsgClasses.REFRESH, loginMsgEvent.msg().msgClass());
            System.out.println(test + " 6) Consumer received login refresh[0]");
            
            consRefresh[0] = (LoginRefresh)loginMsgEvent.rdmLoginMsg();
            assertEquals(consRefresh[0].streamId(), loginStreamId);
            assertEquals(consRefresh[0].state().streamState(), StreamStates.OPEN);
            assertEquals(consRefresh[0].state().dataState(), DataStates.OK);
            assertTrue(consRefresh[0].checkHasUserName());
            assertTrue(consRefresh[0].checkHasUserNameType());
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                assertTrue(consRefresh[0].userName().data().get() == 0x0);
                assertTrue(consRefresh[0].checkHasAuthenticationTTReissue());
                assertEquals(consRefresh[0].authenticationTTReissue(), authenticationTTReissues[0]);
                assertTrue(consRefresh[0].checkHasAuthenticationExtendedResp());
                assertEquals(consRefresh[0].authenticationExtendedResp().toString(), authenticationExtResps[0]);
            }
            else
                assertEquals(consRefresh[0].userName().toString(), userNames[0]);
            System.out.println(test + " 6) Consumer validated login refresh[0]");

            System.out.println(test + " 7) Provider dispatching, expects Login Request[2] and Directory request[1]");
            
            provider.testReactor().dispatch(2);
            // validate Login request[2]
            event = providerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            LoginRequest loginRequest = (LoginRequest)loginMsgEvent.rdmLoginMsg();
            assertEquals(loginRequest.streamId(), loginStreamId);
            assertTrue(loginRequest.checkHasUserNameType());
            assertEquals(loginRequest.userNameType(), userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                assertEquals(loginRequest.userName().toString(), authenticationTokens[2]);
                assertTrue(loginRequest.checkHasAuthenticationExtended());
                assertEquals(loginRequest.authenticationExtended().toString(), authenticationExts[2]);
            }
            else if(userNameType == Login.UserIdTypes.TOKEN)
                assertEquals(loginRequest.userName().toString(), userNames[2]);
            else
                assertEquals(loginRequest.userName().toString(), userNames[0]);
            System.out.println(test + " 4.1) Provider validated login request[2]");
            
            // validate Directory request[1]
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
            System.out.println(test + " 7) Provider received Directory request[1]");
            System.out.println(test + " 7) Provider does not validate Directory request[1]");

            System.out.println(test + " 8) Prov creating login refresh[1]");
            provRefresh[1] = (LoginRefresh)LoginMsgFactory.createMsg();
            provRefresh[1].clear();
            provRefresh[1].rdmMsgType(LoginMsgType.REFRESH);
            provRefresh[1].streamId(loginStreamId);
            provRefresh[1].applySolicited();
            provRefresh[1].applyHasUserNameType();
            provRefresh[1].userNameType(userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[1].userName().data(userNames[1]);
                provRefresh[1].applyHasAuthenticationTTReissue();
                provRefresh[1].authenticationTTReissue(authenticationTTReissues[1]);
                provRefresh[1].applyHasAuthenticationExtendedResp();
                provRefresh[1].authenticationExtendedResp().data(authenticationExtResps[1]);
            }
            else
            {
                provRefresh[1].applyHasUserName();
                provRefresh[1].userName().data(userNames[1]);
            }
            provRefresh[1].state().streamState(StreamStates.OPEN);
            provRefresh[1].state().dataState(DataStates.OK);
     
            System.out.println(test + " 8) Prov sending login refresh[1]");
            submitOptions.clear();
            assertTrue(provider.submitAndDispatch(provRefresh[1], submitOptions) >= ReactorReturnCodes.SUCCESS);
            System.out.println(test + " 8) Prov sent login refresh[1]");

            System.out.println(test + " 9) Consumer dispatching, expects login refresh[1]");
            consumer.testReactor().dispatch(1);
            event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            assertEquals(MsgClasses.REFRESH, loginMsgEvent.msg().msgClass());
            System.out.println(test + " 9) Consumer received login refresh[1]");
            
            consRefresh[1] = (LoginRefresh)loginMsgEvent.rdmLoginMsg();
            assertEquals(consRefresh[1].streamId(), loginStreamId);
            assertEquals(consRefresh[1].state().streamState(), StreamStates.OPEN);
            assertEquals(consRefresh[1].state().dataState(), DataStates.OK);
            assertTrue(consRefresh[1].checkHasUserName());
            assertTrue(consRefresh[1].checkHasUserNameType());
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                assertTrue(consRefresh[1].userName().data().get() == 0x0);
                assertTrue(consRefresh[1].checkHasAuthenticationTTReissue());
                assertEquals(consRefresh[1].authenticationTTReissue(), authenticationTTReissues[1]);
                assertTrue(consRefresh[1].checkHasAuthenticationExtendedResp());
                assertEquals(consRefresh[1].authenticationExtendedResp().toString(), authenticationExtResps[1]);
            }
            else
                assertEquals(consRefresh[1].userName().toString(), userNames[1]);
            System.out.println(test + " 9) Consumer validated login refresh[1]");
        }
        System.out.println(test + " Done\n");
    }
        
    @Test
    public void loginReissue_Scenario_C_Test()
    {
        String test = "loginReissueTest_Scenario_C_Test()";
        System.out.println("\n" + test + " Running...");
        int loginStreamId;
        TestReactorEvent event;
        RDMLoginMsgEvent loginMsgEvent;
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        
        LoginRequest[] consRequest = { null, null, null, null };
        LoginRefresh[] consRefresh = { null, null, null, null };
        LoginRequest[] provRequest = { null, null, null, null };
        LoginRefresh[] provRefresh = { null, null, null, null };

        // Data arrays - index [0] is used for the initial request and refresh, the other indices are for the subsequent reissue requests and refreshes.
        String[] userNames = { "userName_0", "userName_1", "userName_2", "userName_3" };
        String[] authenticationTokens = { "authenticationToken_0", "authenticationToken_1", "authenticationToken_2", "authenticationToken_3" };
        String[] authenticationExts = { "authenticationExt_0", "authenticationExt_1", "authenticationExt_2", "authenticationExt_3" };
        String[] authenticationExtResps = { "authenticationExtResp_0", "authenticationExtResp_1", "authenticationExtResp_2", "authenticationExtResp_3" };
        long[] authenticationTTReissues = { 123123000, 123123001, 123123002, 123123003 };

        // Run a test for each userNameType
        int[] userNameTypes = { Login.UserIdTypes.NAME, Login.UserIdTypes.EMAIL_ADDRESS, Login.UserIdTypes.TOKEN, Login.UserIdTypes.COOKIE, Login.UserIdTypes.AUTHN_TOKEN };
        for (int userNameType : userNameTypes)
        {
            boolean reissueSuccess = true;
            System.out.println(test + " loop: userNameType = " + userNameType);

            // Create reactors.
            TestReactor consumerReactor = new TestReactor();
            TestReactor providerReactor = new TestReactor();
            consumerReactor._reactor._reactorOptions.enableXmlTracing();
            providerReactor._reactor._reactorOptions.enableXmlTracing();
     
            // Create consumer.
            Consumer consumer = new Consumer(consumerReactor);
            ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
            consumerRole.initDefaultRDMLoginRequest();
            consumerRole.initDefaultRDMDirectoryRequest();
            
            System.out.println(test + " 1) Consumer creating login request[0]");
            consRequest[0] = consumerRole.rdmLoginRequest();
            consRequest[0].applyHasUserNameType();
            consRequest[0].userNameType(userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                // Setting the userName on a login request with userNameType = AUTHN_TOKEN is allowed, however it will be ignored
                consRequest[0].userName().data(userNames[0]);
                consRequest[0].userName().data(authenticationTokens[0]);
                consRequest[0].applyHasAuthenticationExtended();
                consRequest[0].authenticationExtended().data(authenticationExts[0]);
            }
            else
                consRequest[0].userName().data(userNames[0]);
            consumerRole.channelEventCallback(consumer);
            consumerRole.loginMsgCallback(consumer);
            consumerRole.directoryMsgCallback(consumer);
            consumerRole.dictionaryMsgCallback(consumer);
            consumerRole.defaultMsgCallback(consumer);
            consumerRole.watchlistOptions().enableWatchlist(true);
            consumerRole.watchlistOptions().channelOpenCallback(consumer);

            // Create provider.
            Provider provider = new Provider(providerReactor);
            ProviderRole providerRole = (ProviderRole)provider.reactorRole();
            providerRole.channelEventCallback(provider);
            providerRole.loginMsgCallback(provider);
            providerRole.directoryMsgCallback(provider);
            providerRole.dictionaryMsgCallback(provider);
            providerRole.defaultMsgCallback(provider);
     
            // Connect the consumer and provider. Disable the automatic setup of login & directory streams.
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.setupDefaultLoginStream(false);
            opts.setupDefaultDirectoryStream(false);
            provider.bind(opts);
            System.out.println(test + " 1) Consumer sending login request[0]");
            TestReactor.openSession(consumer, provider, opts);
            System.out.println(test + " 1) Consumer sent login request[0]");

            System.out.println(test + " 2) Provider dispatching, expects login request[0]");
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            System.out.println(test + " 2) Provider received login request[0]");
            
            provRequest[0] = (LoginRequest)loginMsgEvent.rdmLoginMsg();
            loginStreamId = provRequest[0].streamId();
            assertTrue(provRequest[0].checkHasUserNameType());
            assertEquals(provRequest[0].userNameType(), userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                // The userName on any login request with userNameType = AUTHN_TOKEN has no data
                assertEquals(provRequest[0].userName().toString(), authenticationTokens[0]);
                assertTrue(provRequest[0].checkHasAuthenticationExtended());
                assertEquals(provRequest[0].authenticationExtended().toString(), authenticationExts[0]);
            }
            else
                assertEquals(provRequest[0].userName().toString(), userNames[0]);
            System.out.println(test + " 2) Provider validated login request[0]");
     
            System.out.println(test + " 3) Provider creating login refresh[0]");
            provRefresh[0] = (LoginRefresh)LoginMsgFactory.createMsg();
            provRefresh[0].clear();
            provRefresh[0].rdmMsgType(LoginMsgType.REFRESH);
            provRefresh[0].streamId(loginStreamId);
            provRefresh[0].applySolicited();
            provRefresh[0].applyHasUserName();
            provRefresh[0].applyHasUserNameType();
            provRefresh[0].userNameType(userNameType);
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                provRefresh[0].userName().data(userNames[0]);
                provRefresh[0].applyHasAuthenticationTTReissue();
                provRefresh[0].authenticationTTReissue(authenticationTTReissues[0]);
                provRefresh[0].applyHasAuthenticationExtendedResp();
                provRefresh[0].authenticationExtendedResp().data(authenticationExtResps[0]);
            }
            else
                provRefresh[0].userName().data(userNames[0]);
            provRefresh[0].state().streamState(StreamStates.OPEN);
            provRefresh[0].state().dataState(DataStates.OK);
     
            System.out.println(test + " 3) Provider sending login refresh[0]");
            submitOptions.clear();
            assertTrue(provider.submitAndDispatch(provRefresh[0], submitOptions) >= ReactorReturnCodes.SUCCESS);
            System.out.println(test + " 3) Provider sent login refresh[0]");
            
            System.out.println(test + " 4) Consumer dispatching, expects login refresh[0] (if so, will send Directory request)");
            consumer.testReactor().dispatch(1);
            event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
            assertEquals(MsgClasses.REFRESH, loginMsgEvent.msg().msgClass());
            System.out.println(test + " 4) Consumer received login refresh[0]");
            
            consRefresh[0] = (LoginRefresh)loginMsgEvent.rdmLoginMsg();
            assertEquals(consRefresh[0].streamId(), loginStreamId);
            assertEquals(consRefresh[0].state().streamState(), StreamStates.OPEN);
            assertEquals(consRefresh[0].state().dataState(), DataStates.OK);
            assertTrue(consRefresh[0].checkHasUserName());
            assertTrue(consRefresh[0].checkHasUserNameType());
            if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
            {
                assertTrue(consRefresh[0].userName().data().get() == 0x0);
                assertTrue(consRefresh[0].checkHasAuthenticationTTReissue());
                assertEquals(consRefresh[0].authenticationTTReissue(), authenticationTTReissues[0]);
                assertTrue(consRefresh[0].checkHasAuthenticationExtendedResp());
                assertEquals(consRefresh[0].authenticationExtendedResp().toString(), authenticationExtResps[0]);
            }
            else
                assertEquals(consRefresh[0].userName().toString(), userNames[0]);
            System.out.println(test + " 4) Consumer validated login refresh[0]");
                            
            // Send a number of reissue requests and corresponding refreshes, using the for loop counter as the request/refresh data index.
            for (int k=1; k<=3; k++)
            {
                System.out.println(test + " " + (4*k+1) + ") Consumer creating login request[" + k + "]");
                consumerRole.initDefaultRDMLoginRequest();
                consRequest[k] = consumerRole.rdmLoginRequest();
                assertNotNull(consRequest[k]);
                consRequest[k].applyHasUserNameType();
                consRequest[k].userNameType(userNameType);
                boolean matchRequestedUserName = false;
                boolean mismatchUserNameTypes = false;
                if (k == 2) matchRequestedUserName = true;
                if (k == 3) mismatchUserNameTypes = true;
                if (userNameType == Login.UserIdTypes.AUTHN_TOKEN || userNameType == Login.UserIdTypes.TOKEN)
                {
                    if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                    {
                        if (matchRequestedUserName == true)
                            consRequest[k].userName().data(authenticationTokens[0]);
                        else
                            consRequest[k].userName().data(authenticationTokens[k]);
                        consRequest[k].applyHasAuthenticationExtended();
                        consRequest[k].authenticationExtended().data(authenticationExts[k]);
                    }
                    else if (userNameType == Login.UserIdTypes.TOKEN)
                    {
                        if (matchRequestedUserName == true)
                            consRequest[k].userName().data(userNames[0]);
                        else
                            consRequest[k].userName().data(userNames[k]);
                    }
                    if (mismatchUserNameTypes == true)
                    {
                        if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                            consRequest[k].userNameType(Login.UserIdTypes.TOKEN);
                        else if (userNameType == Login.UserIdTypes.TOKEN)
                            consRequest[k].userNameType(Login.UserIdTypes.AUTHN_TOKEN);
                        System.out.println(test + " " + (4*k+1) + ") Consumer sending login request[" + k + "], mismatchUserNameTypes");
                        submitOptions.clear();
                        assertTrue(consumer.reactorChannel().submit(consRequest[k], submitOptions, errorInfo) == ReactorReturnCodes.INVALID_USAGE);
                        assertEquals("Login userNameType does not match existing request", errorInfo.error().text());
                        System.out.println(test + " " + (4*k+1) + ") Consumer sent login request[" + k + "], mismatchUserNameTypes");
                        reissueSuccess = false;
                    }
                    else
                    {
                        System.out.println(test + " " + (4*k+1) + ") Consumer sending login request[" + k + "]");
                        submitOptions.clear();
                        assertTrue(consumer.submitAndDispatch(consRequest[k], submitOptions) >= ReactorReturnCodes.SUCCESS);
                        System.out.println(test + " " + (4*k+1) + ") Consumer sent login request[" + k + "]");
                    }
                }
                else
                {
                    if (matchRequestedUserName == true)
                    {
                        consRequest[k].userName().data(userNames[0]);
                        submitOptions.clear();
                        if (mismatchUserNameTypes == true)
                        {
                            consRequest[k].userNameType(Login.UserIdTypes.TOKEN);
                            System.out.println(test + " " + (4*k+1) + ") Consumer sending login request[" + k + "], mismatchUserNameTypes");
                            assertTrue(consumer.reactorChannel().submit(consRequest[k], submitOptions, errorInfo) == ReactorReturnCodes.INVALID_USAGE);
                            assertEquals("Login userNameType does not match existing request", errorInfo.error().text());
                            System.out.println(test + " " + (4*k+1) + ") Consumer sent login request[" + k + "], mismatchUserNameTypes");
                            reissueSuccess = false;
                        }
                        else
                        {
                            System.out.println(test + " " + (4*k+1) + ") Consumer sending login request[" + k + "]");
                            assertTrue(consumer.submitAndDispatch(consRequest[k], submitOptions) >= ReactorReturnCodes.SUCCESS);
                            System.out.println(test + " " + (4*k+1) + " ) Consumer sent login request[" + k + "]");
                        }
                    }
                    else
                    {
                        consRequest[k].userName().data(userNames[k]);
                        submitOptions.clear();
                        if (mismatchUserNameTypes == true)
                        {
                            consRequest[k].userNameType(Login.UserIdTypes.AUTHN_TOKEN);
                            System.out.println(test + " " + (4*k+1) + ") Consumer sending login request[" + k + "], mismatchUserNameTypes");
                            assertTrue(consumer.reactorChannel().submit(consRequest[k], submitOptions, errorInfo) == ReactorReturnCodes.INVALID_USAGE);
                            assertEquals("Login userNameType does not match existing request", errorInfo.error().text());
                            System.out.println(test + " " + (4*k+1) + ") Consumer sent login request[" + k + "], mismatchUserNameTypes");
                            reissueSuccess = false;
                        }
                        else
                        {
                            consRequest[k].userName().data(userNames[0]);
                            System.out.println(test + " " + (4*k+1) + ") Consumer sending login request[" + k + "]");
                            assertTrue(consumer.submitAndDispatch(consRequest[k], submitOptions) >= ReactorReturnCodes.SUCCESS);
                            System.out.println(test + " " + (4*k+1) + ") Consumer sent login request[" + k + "]");
                        }
                    }
                }
                
                if (reissueSuccess == true)
                {
                    // Provider receives login reissue request (and directory request), then verifies data using index [k].
                    if (k == 1)
                    {
                        System.out.println(test + " " + (5*k+1) + ") Provider dispatching, expects Directory and login request[" + k + "]");
                        provider.testReactor().dispatch(2);
                        event = provider.testReactor().pollEvent();
                        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
                        System.out.println(test + " " + (5*k+1) + ") Provider received Directory request");
                    }
                    else
                    {
                        System.out.println(test + " " + (5*k+1) + ") Provider dispatching, expects login request[" + k + "]");
                        provider.testReactor().dispatch(1);
                    }
                    
                    event = provider.testReactor().pollEvent();
                    assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
                    loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
                    assertEquals(LoginMsgType.REQUEST, loginMsgEvent.rdmLoginMsg().rdmMsgType());
                    System.out.println(test + " " + (5*k+1) + ") Provider received login request[" + k + "]");

                    provRequest[k] = (LoginRequest)loginMsgEvent.rdmLoginMsg();
                    loginStreamId = provRequest[k].streamId();
                    assertTrue(provRequest[k].checkHasUserNameType());
                    assertEquals(provRequest[k].userNameType(), userNameType);
                    if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                    {
                        if (matchRequestedUserName == true)
                            assertEquals(provRequest[k].userName().toString(), authenticationTokens[0]);
                        else
                            assertEquals(provRequest[k].userName().toString(), authenticationTokens[k]);
                        assertTrue(provRequest[k].checkHasAuthenticationExtended());
                        assertEquals(provRequest[k].authenticationExtended().toString(), authenticationExts[k]);
                    }
                    else if (userNameType == Login.UserIdTypes.TOKEN)
                    {
                        if (matchRequestedUserName == true)
                            assertEquals(provRequest[k].userName().toString(), userNames[0]);
                        else
                            assertEquals(provRequest[k].userName().toString(), userNames[k]);
                    }
                    System.out.println(test + " " + (5*k+1) + ") Provider validated login request[" + k + "]");

                    System.out.println(test + " " + (6*k+1) + ") Provider creating login refresh[" + k + "]");
                    provRefresh[k] = (LoginRefresh)LoginMsgFactory.createMsg();
                    provRefresh[k].clear();
                    provRefresh[k].rdmMsgType(LoginMsgType.REFRESH);
                    provRefresh[k].streamId(loginStreamId);
                    provRefresh[k].applySolicited();
                    provRefresh[k].applyHasUserName();
                    provRefresh[k].applyHasUserNameType();
                    provRefresh[k].userNameType(userNameType);
                    if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                    {
                        provRefresh[k].userName().data(userNames[k]);
                        provRefresh[k].applyHasAuthenticationTTReissue();
                        provRefresh[k].authenticationTTReissue(authenticationTTReissues[k]);
                        provRefresh[k].applyHasAuthenticationExtendedResp();
                        provRefresh[k].authenticationExtendedResp().data(authenticationExtResps[k]);
                    }
                    else if (userNameType == Login.UserIdTypes.TOKEN)
                        provRefresh[k].userName().data(userNames[k]);
                    else
                        provRefresh[k].userName().data(userNames[0]);
                    provRefresh[k].state().streamState(StreamStates.OPEN);
                    provRefresh[k].state().dataState(DataStates.OK);
             
                    System.out.println(test + " " + (6*k+1) + ") Provider sending login refresh[" + k + "]");
                    submitOptions.clear();
                    assertTrue(provider.submitAndDispatch(provRefresh[k], submitOptions) >= ReactorReturnCodes.SUCCESS);
                    System.out.println(test + " " + (6*k+1) + ") Provider sent login refresh[" + k + "]");

                    System.out.println(test + " " + (7*k+1) + ") Consumer dispatching, expects login refresh[" + k + "]");
                    consumer.testReactor().dispatch(1);
                    event = consumer.testReactor().pollEvent();
                    assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
                    loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
                    assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
                    assertEquals(MsgClasses.REFRESH, loginMsgEvent.msg().msgClass());
                    System.out.println(test + " " + (7*k+1) + ") Consumer received login refresh[" + k + "]");
                    
                    consRefresh[k] = (LoginRefresh)loginMsgEvent.rdmLoginMsg();
                    assertEquals(consRefresh[k].streamId(), loginStreamId);
                    assertEquals(consRefresh[k].state().streamState(), StreamStates.OPEN);
                    assertEquals(consRefresh[k].state().dataState(), DataStates.OK);
                    assertTrue(consRefresh[k].checkHasUserName());
                    assertTrue(consRefresh[k].checkHasUserNameType());
                    if (userNameType == Login.UserIdTypes.AUTHN_TOKEN)
                    {
                        assertTrue(consRefresh[k].userName().data().get() == 0x0);
                        assertTrue(consRefresh[k].checkHasAuthenticationTTReissue());
                        assertEquals(consRefresh[k].authenticationTTReissue(), authenticationTTReissues[k]);
                        assertTrue(consRefresh[k].checkHasAuthenticationExtendedResp());
                        assertEquals(consRefresh[k].authenticationExtendedResp().toString(), authenticationExtResps[k]);
                    }
                    else if (userNameType == Login.UserIdTypes.TOKEN)
                        assertEquals(consRefresh[k].userName().toString(), userNames[k]);
                    else
                        assertEquals(consRefresh[k].userName().toString(), userNames[0]);
                    System.out.println(test + " " + (7*k+1) + ") Consumer validated login refresh[" + k + "]");
                }
            }
        }
        System.out.println(test + " Done\n");
    }
    
    /* Used by privateStreamOpenCallbackSubmitTest and privateStreamOpenCallbackSubmitReSubmitTest. */
    class SendItemsFromDefaultMsgCallbackConsumer extends Consumer
    {
        public SendItemsFromDefaultMsgCallbackConsumer(TestReactor testReactor)
        {
            super(testReactor);
        }

        @Override
        public int defaultMsgCallback(ReactorMsgEvent event)
        {
             RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
             ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
             super.defaultMsgCallback(event);

             if (event.msg().msgClass() == MsgClasses.REFRESH && event.msg().streamId() == 5)
             {
            	 //sending snapshot request which has been open, then closed before
                 requestMsg.clear();
                 requestMsg.msgClass(MsgClasses.REQUEST);
                 requestMsg.streamId(7);
                 requestMsg.domainType(DomainTypes.MARKET_PRICE);
                 requestMsg.msgKey().applyHasName();
                 requestMsg.msgKey().name().data("TRI.N");
                 submitOptions.clear();
                 submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
                 assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
                 
               //sending realtime request
                 requestMsg.clear();
                 requestMsg.msgClass(MsgClasses.REQUEST);
                 requestMsg.streamId(8);
                 requestMsg.domainType(DomainTypes.MARKET_PRICE);
                 requestMsg.applyStreaming();
                 requestMsg.msgKey().applyHasName();
                 requestMsg.msgKey().name().data("CallbackItem2");
                 submitOptions.clear();
                 submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
                 assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
             }
                        
            return ReactorReturnCodes.SUCCESS;
        }
    }
        
    @Test
    public void SendItemsFromDefaultMsgCallbackConsumerTest()
    {
    	/* Opening two items, one snapshot, another one realtime, 
    	 * and resend one same item, one diff item during the callback. */
    	
    	  ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
          TestReactorEvent event;
          ReactorMsgEvent msgEvent;
          RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
          RequestMsg receivedRequestMsg;
          RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
          RefreshMsg receivedRefreshMsg;
                  
          /* Create reactors. */
          TestReactor consumerReactor = new TestReactor();
          TestReactor providerReactor = new TestReactor();
                  
          /* Create consumer. */
          Consumer consumer = new SendItemsFromDefaultMsgCallbackConsumer(consumerReactor);
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
          consumerRole.watchlistOptions().requestTimeout(3000);
          
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

          /* Consumer sends snapshot request. */
          requestMsg.clear();
          requestMsg.msgClass(MsgClasses.REQUEST);
          requestMsg.streamId(5);
          requestMsg.domainType(DomainTypes.MARKET_PRICE);
          requestMsg.msgKey().applyHasName();
          requestMsg.msgKey().name().data("TRI.N");
          submitOptions.clear();
          submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
          assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

          /* Provider receives snapshot request. */
          providerReactor.dispatch(1);
          event = providerReactor.pollEvent();
          assertEquals(TestReactorEventTypes.MSG, event.type());
          msgEvent = (ReactorMsgEvent)event.reactorEvent();
          assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

          receivedRequestMsg = (RequestMsg)msgEvent.msg();
          assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
          assertFalse(receivedRequestMsg.checkStreaming());
          assertFalse(receivedRequestMsg.checkNoRefresh());
          assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
          assertTrue(receivedRequestMsg.msgKey().checkHasName());
          assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
          assertTrue(receivedRequestMsg.checkHasPriority());
          assertEquals(1, receivedRequestMsg.priority().priorityClass());
          assertEquals(1, receivedRequestMsg.priority().count());
          int providerStreamId1 = receivedRequestMsg.streamId();

          /* Consumer sends streaming request. */
          requestMsg.clear();
          requestMsg.msgClass(MsgClasses.REQUEST);
          requestMsg.streamId(6);
          requestMsg.domainType(DomainTypes.MARKET_PRICE);
          requestMsg.applyStreaming();
          requestMsg.msgKey().applyHasName();
          requestMsg.msgKey().name().data("CallbackItem1");
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
          assertTrue(receivedRequestMsg.msgKey().name().toString().equals("CallbackItem1"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
          assertTrue(receivedRequestMsg.checkHasPriority());
          assertEquals(1, receivedRequestMsg.priority().priorityClass());
          assertEquals(1, receivedRequestMsg.priority().count());
          int providerStreamId2 = receivedRequestMsg.streamId();

          /* Provider sends refresh .*/
          refreshMsg.clear();
          refreshMsg.msgClass(MsgClasses.REFRESH);
          refreshMsg.applySolicited();
          refreshMsg.domainType(DomainTypes.MARKET_PRICE);
          refreshMsg.streamId(providerStreamId1);
          refreshMsg.containerType(DataTypes.NO_DATA);
          refreshMsg.applyHasMsgKey();
          refreshMsg.msgKey().applyHasServiceId();
          refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
          refreshMsg.msgKey().applyHasName();
          refreshMsg.msgKey().name().data("TRI.N");
          refreshMsg.applyRefreshComplete();
          Buffer groupId = CodecFactory.createBuffer();
          groupId.data("1234431");
          refreshMsg.groupId(groupId);
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
          assertEquals(5, receivedRefreshMsg.streamId());
          assertTrue(receivedRefreshMsg.checkHasMsgKey());
          assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
          assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
          assertTrue(receivedRefreshMsg.msgKey().checkHasName());
          assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
          assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
          assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
          assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
          assertNotNull(msgEvent.streamInfo());
          assertNotNull(msgEvent.streamInfo().serviceName());
          assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
          
          /* Provider receives request from callback. */
          providerReactor.dispatch(2);
         
          event = providerReactor.pollEvent();
          assertEquals(TestReactorEventTypes.MSG, event.type());
          msgEvent = (ReactorMsgEvent)event.reactorEvent();
          assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

          receivedRequestMsg = (RequestMsg)msgEvent.msg();
          assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
          assertFalse(receivedRequestMsg.checkStreaming());
          assertFalse(receivedRequestMsg.checkNoRefresh());
          assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
          assertTrue(receivedRequestMsg.msgKey().checkHasName());
          assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
          assertTrue(receivedRequestMsg.checkHasPriority());
          assertEquals(1, receivedRequestMsg.priority().priorityClass());
          assertEquals(1, receivedRequestMsg.priority().count());
          int providerStreamId3 = receivedRequestMsg.streamId();
          assertFalse(providerStreamId1 == providerStreamId3); 
          
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
          assertTrue(receivedRequestMsg.msgKey().name().toString().equals("CallbackItem2"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
          assertTrue(receivedRequestMsg.checkHasPriority());
          assertEquals(1, receivedRequestMsg.priority().priorityClass());
          assertEquals(1, receivedRequestMsg.priority().count());
          int providerStreamId4 = receivedRequestMsg.streamId();

          /* Provider sends refresh for "CallbackItem1".*/
          refreshMsg.clear();
          refreshMsg.msgClass(MsgClasses.REFRESH);
          refreshMsg.applySolicited();
          refreshMsg.domainType(DomainTypes.MARKET_PRICE);
          refreshMsg.streamId(providerStreamId2);
          refreshMsg.containerType(DataTypes.NO_DATA);
          refreshMsg.applyHasMsgKey();
          refreshMsg.msgKey().applyHasServiceId();
          refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
          refreshMsg.msgKey().applyHasName();
          refreshMsg.msgKey().name().data("CallbackItem1");
          refreshMsg.applyRefreshComplete();
          Buffer groupId1 = CodecFactory.createBuffer();
          groupId1.data("1234431");
          refreshMsg.groupId(groupId1);
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
          assertEquals(6, receivedRefreshMsg.streamId());
          assertTrue(receivedRefreshMsg.checkHasMsgKey());
          assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
          assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
          assertTrue(receivedRefreshMsg.msgKey().checkHasName());
          assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("CallbackItem1"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
          assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
          assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
          assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
          assertNotNull(msgEvent.streamInfo());
          assertNotNull(msgEvent.streamInfo().serviceName());
          assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
          
          /* Provider sends refresh for "TRI.N" .*/
          refreshMsg.clear();
          refreshMsg.msgClass(MsgClasses.REFRESH);
          refreshMsg.applySolicited();
          refreshMsg.domainType(DomainTypes.MARKET_PRICE);
          refreshMsg.streamId(providerStreamId3);
          refreshMsg.containerType(DataTypes.NO_DATA);
          refreshMsg.applyHasMsgKey();
          refreshMsg.msgKey().applyHasServiceId();
          refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
          refreshMsg.msgKey().applyHasName();
          refreshMsg.msgKey().name().data("TRI.N");
          refreshMsg.applyRefreshComplete();
          groupId1 = CodecFactory.createBuffer();
          groupId1.data("1234431");
          refreshMsg.groupId(groupId1);
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
          assertEquals(7, receivedRefreshMsg.streamId());
          assertTrue(receivedRefreshMsg.checkHasMsgKey());
          assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
          assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
          assertTrue(receivedRefreshMsg.msgKey().checkHasName());
          assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
          assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
          assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
          assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
          assertNotNull(msgEvent.streamInfo());
          assertNotNull(msgEvent.streamInfo().serviceName());
          assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
      
          
          /* Provider sends refresh for "CallbackItem2" .*/
          refreshMsg.clear();
          refreshMsg.msgClass(MsgClasses.REFRESH);
          refreshMsg.applySolicited();
          refreshMsg.domainType(DomainTypes.MARKET_PRICE);
          refreshMsg.streamId(providerStreamId4);
          refreshMsg.containerType(DataTypes.NO_DATA);
          refreshMsg.applyHasMsgKey();
          refreshMsg.msgKey().applyHasServiceId();
          refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
          refreshMsg.msgKey().applyHasName();
          refreshMsg.msgKey().name().data("CallbackItem2");
          refreshMsg.applyRefreshComplete();
          groupId1 = CodecFactory.createBuffer();
          groupId1.data("1234431");
          refreshMsg.groupId(groupId1);
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
          assertEquals(8, receivedRefreshMsg.streamId());
          assertTrue(receivedRefreshMsg.checkHasMsgKey());
          assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
          assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
          assertTrue(receivedRefreshMsg.msgKey().checkHasName());
          assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("CallbackItem2"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
          assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
          assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
          assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
          assertNotNull(msgEvent.streamInfo());
          assertNotNull(msgEvent.streamInfo().serviceName());
          assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

          TestReactorComponent.closeSession(consumer, provider);
      }

    class SendItemsFromDefaultMsgCallbackConsumer1 extends Consumer
    {
        public SendItemsFromDefaultMsgCallbackConsumer1(TestReactor testReactor)
        {
            super(testReactor);
        }

        @Override
        public int defaultMsgCallback(ReactorMsgEvent event)
        {
             RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
             ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
             super.defaultMsgCallback(event);

             if (event.msg().msgClass() == MsgClasses.REFRESH && event.msg().streamId() == 5)
             {
            	//sending snapshot request which has been open, still waiting for refresh or response
               requestMsg.clear();
               requestMsg.msgClass(MsgClasses.REQUEST);
               requestMsg.streamId(7);
               requestMsg.domainType(DomainTypes.MARKET_PRICE);
               requestMsg.msgKey().applyHasName();
               requestMsg.msgKey().name().data("CallbackItem1");
               submitOptions.clear();
               submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
               assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
             }
                        
            return ReactorReturnCodes.SUCCESS;
        }
    }
    
    @Test
    public void SendItemsFromDefaultMsgCallbackConsumer1Test()
    {
    	/* Opening two items, one snapshot, another one realtime, 
    	 * and resend one same item which is waiting for refresh msg in watchlist. */
    	
    	  ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
          TestReactorEvent event;
          ReactorMsgEvent msgEvent;
          RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
          RequestMsg receivedRequestMsg;
          RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
          RefreshMsg receivedRefreshMsg;
                  
          /* Create reactors. */
          TestReactor consumerReactor = new TestReactor();
          TestReactor providerReactor = new TestReactor();
                  
          /* Create consumer. */
          Consumer consumer = new SendItemsFromDefaultMsgCallbackConsumer1(consumerReactor);
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
          consumerRole.watchlistOptions().requestTimeout(3000);
          
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

          /* Consumer sends snapshot request. */
          requestMsg.clear();
          requestMsg.msgClass(MsgClasses.REQUEST);
          requestMsg.streamId(5);
          requestMsg.domainType(DomainTypes.MARKET_PRICE);
          requestMsg.msgKey().applyHasName();
          requestMsg.msgKey().name().data("TRI.N");
          submitOptions.clear();
          submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
          assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

          /* Provider receives snapshot request. */
          providerReactor.dispatch(1);
          event = providerReactor.pollEvent();
          assertEquals(TestReactorEventTypes.MSG, event.type());
          msgEvent = (ReactorMsgEvent)event.reactorEvent();
          assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

          receivedRequestMsg = (RequestMsg)msgEvent.msg();
          assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
          assertFalse(receivedRequestMsg.checkStreaming());
          assertFalse(receivedRequestMsg.checkNoRefresh());
          assertEquals(Provider.defaultService().serviceId(), receivedRequestMsg.msgKey().serviceId());
          assertTrue(receivedRequestMsg.msgKey().checkHasName());
          assertTrue(receivedRequestMsg.msgKey().name().toString().equals("TRI.N"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
          assertTrue(receivedRequestMsg.checkHasPriority());
          assertEquals(1, receivedRequestMsg.priority().priorityClass());
          assertEquals(1, receivedRequestMsg.priority().count());
          int providerStreamId1 = receivedRequestMsg.streamId();

          /* Consumer sends streaming request. */
          requestMsg.clear();
          requestMsg.msgClass(MsgClasses.REQUEST);
          requestMsg.streamId(6);
          requestMsg.domainType(DomainTypes.MARKET_PRICE);
          requestMsg.applyStreaming();
          requestMsg.msgKey().applyHasName();
          requestMsg.msgKey().name().data("CallbackItem1");
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
          assertTrue(receivedRequestMsg.msgKey().name().toString().equals("CallbackItem1"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
          assertTrue(receivedRequestMsg.checkHasPriority());
          assertEquals(1, receivedRequestMsg.priority().priorityClass());
          assertEquals(1, receivedRequestMsg.priority().count());
          int providerStreamId2 = receivedRequestMsg.streamId();

          /* Provider sends refresh .*/
          refreshMsg.clear();
          refreshMsg.msgClass(MsgClasses.REFRESH);
          refreshMsg.applySolicited();
          refreshMsg.domainType(DomainTypes.MARKET_PRICE);
          refreshMsg.streamId(providerStreamId1);
          refreshMsg.containerType(DataTypes.NO_DATA);
          refreshMsg.applyHasMsgKey();
          refreshMsg.msgKey().applyHasServiceId();
          refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
          refreshMsg.msgKey().applyHasName();
          refreshMsg.msgKey().name().data("TRI.N");
          refreshMsg.applyRefreshComplete();
          Buffer groupId = CodecFactory.createBuffer();
          groupId.data("1234431");
          refreshMsg.groupId(groupId);
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
          assertEquals(5, receivedRefreshMsg.streamId());
          assertTrue(receivedRefreshMsg.checkHasMsgKey());
          assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
          assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
          assertTrue(receivedRefreshMsg.msgKey().checkHasName());
          assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("TRI.N"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
          assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
          assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
          assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
          assertNotNull(msgEvent.streamInfo());
          assertNotNull(msgEvent.streamInfo().serviceName());
          assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
          
          /* Provider sends refresh for "CallbackItem1".*/
          refreshMsg.clear();
          refreshMsg.msgClass(MsgClasses.REFRESH);
          refreshMsg.applySolicited();
          refreshMsg.domainType(DomainTypes.MARKET_PRICE);
          refreshMsg.streamId(providerStreamId2);
          refreshMsg.containerType(DataTypes.NO_DATA);
          refreshMsg.applyHasMsgKey();
          refreshMsg.msgKey().applyHasServiceId();
          refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
          refreshMsg.msgKey().applyHasName();
          refreshMsg.msgKey().name().data("CallbackItem1");
          refreshMsg.applyRefreshComplete();
          Buffer groupId1 = CodecFactory.createBuffer();
          groupId1.data("1234431");
          refreshMsg.groupId(groupId1);
          refreshMsg.state().streamState(StreamStates.OPEN);
          refreshMsg.state().dataState(DataStates.OK);

          assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
          
          /* Consumer receives refresh. */
          consumerReactor.dispatch(2);
          
          event = consumerReactor.pollEvent();
          assertEquals(TestReactorEventTypes.MSG, event.type());
          msgEvent = (ReactorMsgEvent)event.reactorEvent();
          assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
          receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
          assertEquals(6, receivedRefreshMsg.streamId());
          assertTrue(receivedRefreshMsg.checkHasMsgKey());
          assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
          assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
          assertTrue(receivedRefreshMsg.msgKey().checkHasName());
          assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("CallbackItem1"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
          assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
          assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
          assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
          assertNotNull(msgEvent.streamInfo());
          assertNotNull(msgEvent.streamInfo().serviceName());
          assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
          
          event = consumerReactor.pollEvent();
          assertEquals(TestReactorEventTypes.MSG, event.type());
          msgEvent = (ReactorMsgEvent)event.reactorEvent();
          assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
          receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
          assertEquals(7, receivedRefreshMsg.streamId());
          assertTrue(receivedRefreshMsg.checkHasMsgKey());
          assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
          assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
          assertTrue(receivedRefreshMsg.msgKey().checkHasName());
          assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("CallbackItem1"));
          assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
          assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
          assertEquals(StreamStates.NON_STREAMING, receivedRefreshMsg.state().streamState());
          assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
          assertNotNull(msgEvent.streamInfo());
          assertNotNull(msgEvent.streamInfo().serviceName());
          assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

          TestReactorComponent.closeSession(consumer, provider);
      }
    
    @Test
    public void changeViewByReissueRequestWhilePendingRefreshTest()
    {
        /* Test changing a view on an item by another request of same user stream while waiting for that item's refresh. */
    	/* steps:
    	 * request an item request "TRI" on stream 5 by one user
    	 * receive refresh on "TRI"
    	 * reissue item request "TRI" with View (22,25,-32768,32767) on stream 5
    	 * reissue item request "TRI" with View (25, 1025) on stream 5
    	 * receive refresh on "TRI"
    	 * no refresh fanout to consumer
    	 */
    	
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;

        int providerStreamId;
        List<Integer> viewFieldList = new ArrayList<Integer>();
        
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
        
        /* Request TRI (no view). */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI");

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
        providerStreamId = receivedRequestMsg.streamId();


        /* Provider sends refresh */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("TRI");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        submitOptions.clear();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));   

        
        /* Reissue TRI with BID/ASK view. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.applyHasView();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI");
        viewFieldList.add(22);
        viewFieldList.add(25);
        viewFieldList.add(-32768);
        viewFieldList.add(32767);        
        encodeViewFieldIdList(consumer.reactorChannel(), viewFieldList, requestMsg);

        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        providerReactor.dispatch(1);
        
        /* Provider receives reissued request. */
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(providerStreamId, receivedRequestMsg.streamId());
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertTrue(receivedRequestMsg.checkHasView()); 
        assertTrue(checkHasCorrectView(provider, receivedRequestMsg, viewFieldList)); 
        
        /* Reissue TRI again, now with ASK/QUOTIM. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.applyHasView();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI");
        viewFieldList.add(25);
        viewFieldList.add(1025);
        encodeViewFieldIdList(consumer.reactorChannel(), viewFieldList, requestMsg);
        
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Provider receives nothing (still waiting on refresh for original BID/ASK view). */
        providerReactor.dispatch(0);
        
        /* Provider sends refresh (this would be for the original view). */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("TRI");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        submitOptions.clear();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));   
        
        /* Provider receives updated view request. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(providerStreamId, receivedRequestMsg.streamId());
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertTrue(receivedRequestMsg.checkHasView()); 
        assertTrue(checkHasCorrectView(provider, receivedRequestMsg, viewFieldList)); 

        /* Provider sends refresh */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("TRI");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        submitOptions.clear();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.dispatch(1);

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));        

        TestReactorComponent.closeSession(consumer, provider);
    }
    
    @Test
    public void changeViewByPartialReissueRequestWhilePendingRefreshTest()
    {
        /* Test changing a view on an item by another request of diff user stream while waiting for that item's refresh. */
    	/* steps:
    	 * request an item request "TRI" on stream 5 by one user
    	 * receive refresh on "TRI"
    	 * reissue item request "TRI" with View (22,25) on stream 5
    	 * request third item request "TRI" with View (25, 1025) on stream 6 by second user
    	 * receive refresh from provider
    	 * will fanout to two users
    	 */
        
        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;

        int providerStreamId;
        List<Integer> viewFieldList = new ArrayList<Integer>();
        
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
        
        /* Request TRI (no view). */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI");

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
        providerStreamId = receivedRequestMsg.streamId();


        /* Provider sends refresh */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("TRI");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        submitOptions.clear();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));   

        
        /* Reissue TRI with BID/ASK view. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(5);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.applyHasView();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI");
        viewFieldList.add(22);
        viewFieldList.add(25);
        encodeViewFieldIdList(consumer.reactorChannel(), viewFieldList, requestMsg);

        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        providerReactor.dispatch(1);
        
        /* Provider receives reissued request. */
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(providerStreamId, receivedRequestMsg.streamId());
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertTrue(receivedRequestMsg.checkHasView()); 
        assertTrue(checkHasCorrectView(provider, requestMsg, viewFieldList)); 
        
        /* Request TRI again (not reissue), now with ASK/QUOTIM. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6); //from diff user
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.applyHasView();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("TRI");
        viewFieldList.add(25);
        viewFieldList.add(1025);
        encodeViewFieldIdList(consumer.reactorChannel(), viewFieldList, requestMsg);
        
        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Provider receives nothing (still waiting on refresh for original BID/ASK view). */
        providerReactor.dispatch(0);
        
        /* Provider sends refresh (this would be for the original view). */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("TRI");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        submitOptions.clear();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));   
        
        /* Provider receives updated view request. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());
        
        receivedRequestMsg = (RequestMsg)msgEvent.msg();
        assertEquals(providerStreamId, receivedRequestMsg.streamId());
        assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
        assertTrue(receivedRequestMsg.checkStreaming());
        assertTrue(receivedRequestMsg.checkHasView()); 
        assertTrue(checkHasCorrectView(provider, receivedRequestMsg, viewFieldList)); 

        /* Provider sends refresh */
        refreshMsg.clear();
        refreshMsg.msgClass(MsgClasses.REFRESH);
        refreshMsg.domainType(DomainTypes.MARKET_PRICE);
        refreshMsg.streamId(providerStreamId);
        refreshMsg.containerType(DataTypes.NO_DATA);
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.applyHasMsgKey();
        refreshMsg.msgKey().applyHasServiceId();
        refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        refreshMsg.msgKey().applyHasName();
        refreshMsg.msgKey().name().data("TRI");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        submitOptions.clear();
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives one refresh, will fan out diff user (streamid 5 and 6) . */
        consumerReactor.dispatch(2);

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(5, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));        

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertEquals(6, receivedRefreshMsg.streamId());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));     
        
        TestReactorComponent.closeSession(consumer, provider);
    }
    
    private boolean checkHasCorrectView( Provider provider, RequestMsg requestMsg, List<Integer> viewFieldList)
    {
    	ElementList elementList = CodecFactory.createElementList();
    	ElementEntry elementEntry = CodecFactory.createElementEntry();
    	DecodeIterator dIter = CodecFactory.createDecodeIterator();
		Buffer viewDataElement = null;
		elementEntry.clear();
		elementList.clear();
		elementEntry.clear();
		dIter.clear();
		int numOfFields = 0;
		int ret;
		int majorVersion =  provider.channel().majorVersion();
		int minorVersion =  provider.channel().minorVersion();
		
		dIter.setBufferAndRWFVersion(requestMsg.encodedDataBody(), majorVersion, minorVersion);
				
		if (requestMsg.containerType() != DataTypes.ELEMENT_LIST)
			return false;	
     
    	if ( elementList.decode(dIter, null) != CodecReturnCodes.SUCCESS )
    		return false;
    	
    	boolean viewDataFound = false;
    	boolean hasViewType = false;
		while ((ret = elementEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
		{
			if (ret != CodecReturnCodes.SUCCESS)
				return false;
			else
			{
				if (elementEntry.name().equals(ElementNames.VIEW_TYPE) &&
						elementEntry.dataType() == DataTypes.UINT) 
				{
					hasViewType = true;
				}
				
				if (elementEntry.name().equals(ElementNames.VIEW_DATA) &&
					elementEntry.dataType() == DataTypes.ARRAY) 
				{
					viewDataElement = elementEntry.encodedData();
					viewDataFound = true;
				}
			}
		} // while
					
	    
		if (!viewDataFound || !hasViewType)
		{
			return false;
		}
		else
		{
			dIter.clear();
			dIter.setBufferAndRWFVersion(viewDataElement, majorVersion, minorVersion);
			
			Array _viewArray = CodecFactory.createArray();
			ArrayEntry _viewArrayEntry = CodecFactory.createArrayEntry();
			_viewArray.clear();
			Int _fieldId = CodecFactory.createInt();
			
			if ((ret = _viewArray.decode(dIter)) == CodecReturnCodes.SUCCESS)
			{
				if (_viewArray.primitiveType() != DataTypes.INT)
					return false;		

				while ((ret = _viewArrayEntry.decode(dIter)) != CodecReturnCodes.END_OF_CONTAINER)
				{								
					if (ret < CodecReturnCodes.SUCCESS)
						return false;		
					else
					{								
						if ((ret = _fieldId.decode(dIter)) == CodecReturnCodes.SUCCESS)
						{
							if (!viewFieldList.contains(Integer.valueOf((int)_fieldId.toLong())))
									return false;
							numOfFields++;
						}
						else
							return false;		
					}								
				}// while
			}
			else
				return false;		
		}
    	
		if (numOfFields > viewFieldList.size())
			return false;
		
    	return true;
    }

    @Test
    public void groupMergeAndStatusFanoutTest()
    {
        /* Test updating an item group via item-specific message and group status. 
         * - Open an item (and another item)
         * - Change the first item's group via item message. Send a group close on the original group to make sure it moved.
         * - Change the first item's group again via group status. Send a group close to close it. Send a group close on the 
         *   previous group to make sure nothing happens.
         */

        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)msg;
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;
        RDMDirectoryMsgEvent directoryMsgEvent;
        DirectoryUpdate directoryUpdateMsg = (DirectoryUpdate)DirectoryMsgFactory.createMsg();
        DirectoryUpdate receivedUpdateMsg;
        Service service = DirectoryMsgFactory.createService();
        ServiceGroup serviceGroup = new ServiceGroup();
        int providerStreamId;

        Buffer groupId1 = CodecFactory.createBuffer();
        groupId1.data("ONE");
        
        Buffer groupId2 = CodecFactory.createBuffer();
        groupId2.data("TWO");
        
        Buffer groupId3 = CodecFactory.createBuffer();
        groupId3.data("TREE");
        
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
        consumerRole.watchlistOptions().requestTimeout(3000);

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

        /* Provider sends refresh, setting item on group ONE. */
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
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        refreshMsg.groupId(groupId1);
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

        /* Provider sends refresh, moving item to group TWO. */
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
        refreshMsg.applyRefreshComplete();
        refreshMsg.groupId(groupId2);
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

        /* Provider sends group update on group ONE. This should not close any items. */
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE);

        service.clear();
        service.applyHasState();
        service.action(MapEntryActions.UPDATE);
        service.serviceId(1);
        serviceGroup.clear();
        serviceGroup.applyHasStatus();
        serviceGroup.status().dataState(DataStates.SUSPECT);
        serviceGroup.status().streamState(StreamStates.CLOSED);
        serviceGroup.group(groupId1);
        service.groupStateList().add(serviceGroup);

        directoryUpdateMsg.serviceList().add(service);

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        consumerReactor.dispatch(1);

        /* Directory update with the group status. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        assertTrue(receivedUpdateMsg.serviceList().size() == 1);
        assertTrue(receivedUpdateMsg.serviceList().get(0).checkHasState());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasInfo());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasData());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLink());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLoad());
        assertTrue(receivedUpdateMsg.serviceList().get(0).groupStateList().size() == 1);

        /* Merge group TWO to group TREE. */
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE);

        service.clear();
        service.applyHasState();
        service.action(MapEntryActions.UPDATE);
        service.serviceId(1);
        serviceGroup.clear();
        serviceGroup.group(groupId2);
        serviceGroup.applyHasMergedToGroup();
        serviceGroup.mergedToGroup(groupId3);
        service.groupStateList().add(serviceGroup);
        directoryUpdateMsg.serviceList().add(service);

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives update. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        assertTrue(receivedUpdateMsg.serviceList().size() == 1);
        assertTrue(receivedUpdateMsg.serviceList().get(0).checkHasState());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasInfo());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasData());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLink());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLoad());
        assertTrue(receivedUpdateMsg.serviceList().get(0).groupStateList().size() == 1);

        /* Consumer requests second item. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("IBM.N");
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        providerStreamId = receivedRequestMsg.streamId();

        /* Provider sends refresh, putting the second item in group TWO. */
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
        refreshMsg.applyRefreshComplete();
        refreshMsg.applySolicited();
        refreshMsg.groupId(groupId2);
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
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("IBM.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

        /* Provider sends group update on group TREE, to close first item. */
        directoryUpdateMsg.clear();
        directoryUpdateMsg.rdmMsgType(DirectoryMsgType.UPDATE);
        directoryUpdateMsg.streamId(2);
        directoryUpdateMsg.applyHasFilter();
        directoryUpdateMsg.filter(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE);

        service.clear();
        service.applyHasState();
        service.action(MapEntryActions.UPDATE);
        service.serviceId(1);
        serviceGroup.clear();
        serviceGroup.applyHasStatus();
        serviceGroup.status().dataState(DataStates.SUSPECT);
        serviceGroup.status().streamState(StreamStates.CLOSED);
        serviceGroup.group(groupId3);
        service.groupStateList().add(serviceGroup);

        directoryUpdateMsg.serviceList().add(service);

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        consumerReactor.dispatch(2);

        /* Consumer receives StatusMsg closing the first item. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId());
        assertTrue(receivedStatusMsg.checkHasState());
        assertEquals(StreamStates.CLOSED, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

        /* Directory update with the group status. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        assertTrue(receivedUpdateMsg.serviceList().size() == 1);
        assertTrue(receivedUpdateMsg.serviceList().get(0).checkHasState());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasInfo());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasData());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLink());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLoad());
        assertTrue(receivedUpdateMsg.serviceList().get(0).groupStateList().size() == 1);

        /* Provider sends the same group status again (group TREE). */
        service.clear();
        service.applyHasState();
        service.action(MapEntryActions.UPDATE);
        service.serviceId(1);
        serviceGroup.clear();
        serviceGroup.applyHasStatus();
        serviceGroup.status().dataState(DataStates.SUSPECT);
        serviceGroup.status().streamState(StreamStates.CLOSED);
        serviceGroup.group(groupId3);
        service.groupStateList().add(serviceGroup);
        directoryUpdateMsg.serviceList().add(service);

        assertTrue(provider.submitAndDispatch(directoryUpdateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives the directory update, but no StatusMsg. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.UPDATE, directoryMsgEvent.msg().msgClass());
        receivedUpdateMsg = (DirectoryUpdate)directoryMsgEvent.rdmDirectoryMsg();
        assertTrue(receivedUpdateMsg.checkHasFilter());
        assertEquals(Directory.ServiceFilterFlags.INFO | Directory.ServiceFilterFlags.STATE, receivedUpdateMsg.filter());
        assertTrue(receivedUpdateMsg.serviceList().size() == 1);
        assertTrue(receivedUpdateMsg.serviceList().get(0).checkHasState());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasInfo());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasData());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLink());
        assertFalse(receivedUpdateMsg.serviceList().get(0).checkHasLoad());
        assertTrue(receivedUpdateMsg.serviceList().get(0).groupStateList().size() == 1);

        TestReactorComponent.closeSession(consumer, provider);
    }

    /* Used by StreamReopenTest. 
     * Receives a close for TRI, then opens IBM on the same stream. */
    class StreamReopenConsumer extends Consumer
    {
        public StreamReopenConsumer(TestReactor testReactor)
        {
            super(testReactor);
        }

        @Override
        public int defaultMsgCallback(ReactorMsgEvent event)
        {
            RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
            ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
            super.defaultMsgCallback(event);

            if (event.msg().msgClass() == MsgClasses.STATUS)
            {
                requestMsg.clear();
                requestMsg.msgClass(MsgClasses.REQUEST);
                requestMsg.streamId(5);
                requestMsg.domainType(DomainTypes.MARKET_PRICE);
                requestMsg.applyStreaming();
                requestMsg.msgKey().applyHasName();
                requestMsg.msgKey().name().data("IBM.N");
                submitOptions.clear();
                submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
                assertTrue(submit(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
            }

            return ReactorReturnCodes.SUCCESS;
        }
    }

    @Test
    public void streamReopenTest_SingleOpenOn()
    {
        streamReopenTest(true);
    }

    @Test
    public void streamReopenTest_SingleOpenOff()
    {
        streamReopenTest(false);
    }

    public void streamReopenTest(boolean singleOpen)
    {
        /* Test reusing an item stream to open another item inside a callback (i.e. the item stream is closed, and another item
         * is requested using the same streamId while inside the callback) . */

        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        StatusMsg statusMsg = (StatusMsg)CodecFactory.createMsg();
        StatusMsg receivedStatusMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        int providerStreamId;

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new StreamReopenConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();

        if (!singleOpen)
        {
            consumerRole.rdmLoginRequest().attrib().applyHasSingleOpen();
            consumerRole.rdmLoginRequest().attrib().singleOpen(0);
            consumerRole.rdmLoginRequest().attrib().applyHasAllowSuspectData();
            consumerRole.rdmLoginRequest().attrib().allowSuspectData(0);
        }

        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        consumerRole.watchlistOptions().enableWatchlist(true);
        consumerRole.watchlistOptions().channelOpenCallback(consumer);
        consumerRole.watchlistOptions().requestTimeout(3000);

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

        /* Provider receives request for TRI. */
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
        assertTrue(receivedRequestMsg.checkHasPriority());
        assertEquals(1, receivedRequestMsg.priority().priorityClass());
        assertEquals(1, receivedRequestMsg.priority().count());
        providerStreamId = receivedRequestMsg.streamId();

        /* Provider sends status msg .*/
        statusMsg.clear();
        statusMsg.msgClass(MsgClasses.STATUS);
        statusMsg.domainType(DomainTypes.MARKET_PRICE);
        statusMsg.streamId(providerStreamId);
        statusMsg.containerType(DataTypes.NO_DATA);
        statusMsg.applyHasState();
        statusMsg.state().streamState(StreamStates.CLOSED);
        statusMsg.state().dataState(DataStates.SUSPECT);
        statusMsg.applyHasMsgKey();
        statusMsg.msgKey().applyHasServiceId();
        statusMsg.msgKey().serviceId(Provider.defaultService().serviceId());
        statusMsg.msgKey().applyHasName();
        statusMsg.msgKey().name().data("TRI.N");
        
        submitOptions.clear();
        assertTrue(provider.submitAndDispatch(statusMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives status msg. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent) event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        receivedStatusMsg = (StatusMsg) msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId());
        assertTrue(receivedStatusMsg.checkHasMsgKey());
        assertTrue(receivedStatusMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedStatusMsg.msgKey().serviceId());
        assertTrue(receivedStatusMsg.msgKey().checkHasName());
        assertTrue(receivedStatusMsg.msgKey().name().toString().equals("TRI.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertTrue(receivedStatusMsg.checkHasState());
        assertEquals(StreamStates.CLOSED, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));      


        /* Provider receives request for IBM. */
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
        assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRequestMsg.domainType());
        assertTrue(receivedRequestMsg.checkHasPriority());
        assertEquals(1, receivedRequestMsg.priority().priorityClass());
        assertEquals(1, receivedRequestMsg.priority().count());
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
        refreshMsg.msgKey().name().data("IBM.N");
        refreshMsg.state().streamState(StreamStates.OPEN);
        refreshMsg.state().dataState(DataStates.OK);
        refreshMsg.applySolicited();
        refreshMsg.applyRefreshComplete();
        
        assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives refresh. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());
        
        receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
        assertTrue(receivedRefreshMsg.checkHasMsgKey());
        assertTrue(receivedRefreshMsg.checkSolicited());
        assertTrue(receivedRefreshMsg.checkRefreshComplete());
        assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
        assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
        assertTrue(receivedRefreshMsg.msgKey().checkHasName());
        assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("IBM.N"));
        assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
        assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
        assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));


        TestReactorComponent.closeSession(consumer, provider);
    }

    @Test
    public void viewAggregateAndReconnectTest()
    {
        /* Test recovering an aggregated view.
         * - Send a request with a view.
         * - Send a request for the same item, with a different view. This request will be waiting for the first to get its refresh.
         * - In one case (i == 0 below), refresh the items. In the other case (i == 1), don't refresh them.
         * - Disconnect the provider. Both view requests should get the status indicating recovery.
         * - Reconnect. Provider should get a request for the aggregate view. */

        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;

        int providerStreamId;
        List<Integer> viewFieldList = new ArrayList<Integer>();

        for (int i = 0; i < 2; ++i)
        {
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
            opts.reconnectAttemptLimit(-1);
            provider.bind(opts);
            TestReactor.openSession(consumer, provider, opts);

            /* Request TRI with BID/ASK view. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(5);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.applyHasView();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            viewFieldList.add(22);
            viewFieldList.add(25);
            encodeViewFieldIdList(consumer.reactorChannel(), viewFieldList, requestMsg);

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
            assertTrue(receivedRequestMsg.checkHasView()); 
            assertTrue(checkHasCorrectView(provider, receivedRequestMsg, viewFieldList));
            providerStreamId = receivedRequestMsg.streamId();
            
            /* Request TRI on another stream with a different view. */
            requestMsg.clear();
            requestMsg.msgClass(MsgClasses.REQUEST);
            requestMsg.streamId(6);
            requestMsg.domainType(DomainTypes.MARKET_PRICE);
            requestMsg.applyStreaming();
            requestMsg.applyHasView();
            requestMsg.msgKey().applyHasName();
            requestMsg.msgKey().name().data("TRI");
            viewFieldList.clear();
            viewFieldList.add(25);
            viewFieldList.add(1025);
            encodeViewFieldIdList(consumer.reactorChannel(), viewFieldList, requestMsg);

            submitOptions.clear();
            submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
            assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Provider receives nothing (still waiting on refresh for original BID/ASK view). */
            providerReactor.dispatch(0);

            if (i == 0) /* If i is 0, refresh the items. */
            {
                /* Provider sends refresh. */
                refreshMsg.clear();
                refreshMsg.msgClass(MsgClasses.REFRESH);
                refreshMsg.domainType(DomainTypes.MARKET_PRICE);
                refreshMsg.streamId(providerStreamId);
                refreshMsg.containerType(DataTypes.NO_DATA);
                refreshMsg.applySolicited();
                refreshMsg.applyRefreshComplete();
                refreshMsg.applyHasMsgKey();
                refreshMsg.msgKey().applyHasServiceId();
                refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
                refreshMsg.msgKey().applyHasName();
                refreshMsg.msgKey().name().data("TRI");
                refreshMsg.state().streamState(StreamStates.OPEN);
                refreshMsg.state().dataState(DataStates.OK);
                submitOptions.clear();
                assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

                /* Consumer receives refresh for first request. */
                consumerReactor.dispatch(1);

                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

                receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
                assertEquals(5, receivedRefreshMsg.streamId());
                assertTrue(receivedRefreshMsg.checkSolicited()); 
                assertTrue(receivedRefreshMsg.checkHasMsgKey());
                assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
                assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
                assertTrue(receivedRefreshMsg.msgKey().checkHasName());
                assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
                assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
                assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
                assertNotNull(msgEvent.streamInfo());
                assertNotNull(msgEvent.streamInfo().serviceName());
                assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));     

                /* Provider receives request with aggregated view. */
                providerReactor.dispatch(1);
                event = providerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

                receivedRequestMsg = (RequestMsg)msgEvent.msg();
                assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
                assertTrue(receivedRequestMsg.checkStreaming());
                assertTrue(receivedRequestMsg.checkHasView());
                viewFieldList.clear();
                viewFieldList.add(22);
                viewFieldList.add(25);
                viewFieldList.add(1025);
                assertTrue(checkHasCorrectView(provider, receivedRequestMsg, viewFieldList));
                assertEquals(providerStreamId, receivedRequestMsg.streamId());
                
                /* Provider sends refresh. */
                assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

                /* Consumer receives refreshes for each request. */
                consumerReactor.dispatch(2);

                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

                receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
                assertEquals(5, receivedRefreshMsg.streamId());
                assertTrue(receivedRefreshMsg.checkSolicited()); 
                assertTrue(receivedRefreshMsg.checkHasMsgKey());
                assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
                assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
                assertTrue(receivedRefreshMsg.msgKey().checkHasName());
                assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
                assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
                assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
                assertNotNull(msgEvent.streamInfo());
                assertNotNull(msgEvent.streamInfo().serviceName());
                assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));     

                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.MSG, event.type());
                msgEvent = (ReactorMsgEvent)event.reactorEvent();
                assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

                receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
                assertEquals(6, receivedRefreshMsg.streamId());
                assertTrue(receivedRefreshMsg.checkSolicited());
                assertTrue(receivedRefreshMsg.checkHasMsgKey());
                assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
                assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
                assertTrue(receivedRefreshMsg.msgKey().checkHasName());
                assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
                assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
                assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
                assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
                assertNotNull(msgEvent.streamInfo());
                assertNotNull(msgEvent.streamInfo().serviceName());
                assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
                
                /* Reissue this request, but don't change the view. */
                requestMsg.clear();
                requestMsg.msgClass(MsgClasses.REQUEST);
                requestMsg.streamId(5);
                requestMsg.domainType(DomainTypes.MARKET_PRICE);
                requestMsg.applyStreaming();
                requestMsg.applyPause();
                requestMsg.applyHasView();
                requestMsg.msgKey().applyHasName();
                requestMsg.msgKey().name().data("TRI");

                submitOptions.clear();
                submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
                assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
                
                /* This has no effect, so provider receives nothing. */
                providerReactor.dispatch(0);
            }

            /* Disconnect provider. */
            provider.closeChannel();

            /* Consumer receives channel event, Login Status, Directory Update, and status for the item streams. */
            consumerReactor.dispatch(5);
            event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());

            RDMLoginMsgEvent loginMsgEvent;                
            event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
            loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
            assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());  

            RDMDirectoryMsgEvent directoryMsgEvent;                
            event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
            directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
            assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());   

            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            receivedStatusMsg = (StatusMsg)msgEvent.msg();
            assertEquals(5, receivedStatusMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
            assertTrue(receivedStatusMsg.checkHasState());
            assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
            assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
            receivedStatusMsg = (StatusMsg)msgEvent.msg();
            assertEquals(6, receivedStatusMsg.streamId());
            assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
            assertTrue(receivedStatusMsg.checkHasState());
            assertEquals(StreamStates.OPEN, receivedStatusMsg.state().streamState());
            assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

            /* Reconnect and reestablish login/directory streams. */
            TestReactor.openSession(consumer, provider, opts, true);

            /* Provider receives request again. */
            providerReactor.dispatch(1);
            event = providerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REQUEST, msgEvent.msg().msgClass());

            receivedRequestMsg = (RequestMsg)msgEvent.msg();
            assertTrue(receivedRequestMsg.msgKey().checkHasServiceId());
            assertTrue(receivedRequestMsg.checkStreaming());
            assertTrue(receivedRequestMsg.checkHasView());
            viewFieldList.clear();
            viewFieldList.add(22);
            viewFieldList.add(25);
            viewFieldList.add(1025);
            assertTrue(checkHasCorrectView(provider, receivedRequestMsg, viewFieldList));
            providerStreamId = receivedRequestMsg.streamId();

            /* Provider sends refresh. */
            refreshMsg.clear();
            refreshMsg.msgClass(MsgClasses.REFRESH);
            refreshMsg.domainType(DomainTypes.MARKET_PRICE);
            refreshMsg.streamId(providerStreamId);
            refreshMsg.containerType(DataTypes.NO_DATA);
            refreshMsg.applySolicited();
            refreshMsg.applyRefreshComplete();
            refreshMsg.applyHasMsgKey();
            refreshMsg.msgKey().applyHasServiceId();
            refreshMsg.msgKey().serviceId(Provider.defaultService().serviceId());
            refreshMsg.msgKey().applyHasName();
            refreshMsg.msgKey().name().data("TRI");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            submitOptions.clear();
            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives refreshes for each request. */
            consumerReactor.dispatch(2);

            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertEquals(5, receivedRefreshMsg.streamId());
            assertTrue(receivedRefreshMsg.checkSolicited()); 
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));     

            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertEquals(6, receivedRefreshMsg.streamId());
            assertTrue(receivedRefreshMsg.checkSolicited());
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString())); 

            TestReactorComponent.closeSession(consumer, provider);
        }
    }

    @Test
    public void openWindowReconnectTest_SingleOpenOn()
    {
        openWindowReconnectTest(true);
    }

    @Test
    public void openWindowReconnectTest_SingleOpenOff()
    {
        openWindowReconnectTest(false);
    }

    public void openWindowReconnectTest(boolean singleOpen)
    {
        /* Test that an item waiting on the OpenWindow is recovered after a disconnect. 
         * - Start a session where the Provider's Service's OpenWindow is 1.
         * - Send a request for two items. The second item is left waiting on the open window.
         * - Disconnect the provider and ensure both items receive status messages.
         * - Reconnect and ensure both items are recovered if singleOpen is enabled. */

        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        Msg msg = CodecFactory.createMsg();
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)msg;
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;

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

        if (!singleOpen)
        {
            consumerRole.rdmLoginRequest().attrib().applyHasSingleOpen();
            consumerRole.rdmLoginRequest().attrib().singleOpen(0);
            consumerRole.rdmLoginRequest().attrib().applyHasAllowSuspectData();
            consumerRole.rdmLoginRequest().attrib().allowSuspectData(0);
        }
        
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
        opts.reconnectAttemptLimit(-1);
        opts.openWindow(1);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);
        
        /* Request TRI. */
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

        /* Request IBM. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(6);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("IBM.N");

        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);
        
        /* Request GOOG. */
        requestMsg.clear();
        requestMsg.msgClass(MsgClasses.REQUEST);
        requestMsg.streamId(7);
        requestMsg.domainType(DomainTypes.MARKET_PRICE);
        requestMsg.applyStreaming();
        requestMsg.msgKey().applyHasName();
        requestMsg.msgKey().name().data("GOOG.O");

        submitOptions.clear();
        submitOptions.serviceName(Provider.defaultService().info().serviceName().toString());
        assertTrue(consumer.submitAndDispatch(requestMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Provider does not receive this request since TRI & GOOG are in the OpenWindow. */
        providerReactor.dispatch(0);

        /* Disconnect provider. */
        provider.closeChannel();

        /* Consumer receives channel event, Login Status, Directory Update, and status for the item streams. */
        consumerReactor.dispatch(6);
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());

        RDMLoginMsgEvent loginMsgEvent;                
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
        loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
        assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());  

        RDMDirectoryMsgEvent directoryMsgEvent;                
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());   

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId()); assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertTrue(receivedStatusMsg.checkHasState());
        assertEquals(singleOpen ? StreamStates.OPEN : StreamStates.CLOSED_RECOVER, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(6, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertTrue(receivedStatusMsg.checkHasState());
        assertEquals(singleOpen ? StreamStates.OPEN : StreamStates.CLOSED_RECOVER, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());
        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(7, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertTrue(receivedStatusMsg.checkHasState());
        assertEquals(singleOpen ? StreamStates.OPEN : StreamStates.CLOSED_RECOVER, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());
        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

        /* Reconnect and reestablish login/directory streams. */
        TestReactor.openSession(consumer, provider, opts, true);

        if (singleOpen)
        {
            /* Provider receives TRI request again. */
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
            refreshMsg.applySolicited();
            refreshMsg.applyRefreshComplete();

            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertEquals(5, receivedRefreshMsg.streamId());
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.checkSolicited());
            assertTrue(receivedRefreshMsg.checkRefreshComplete());
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

            /* Provider receives IBM request. */
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
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("IBM.N"));
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
            refreshMsg.msgKey().name().data("IBM.N");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            refreshMsg.applySolicited();
            refreshMsg.applyRefreshComplete();

            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertEquals(6, receivedRefreshMsg.streamId());
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.checkSolicited());
            assertTrue(receivedRefreshMsg.checkRefreshComplete());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("IBM.N"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

            /* Provider receives GOOG request. */
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
            assertTrue(receivedRequestMsg.msgKey().name().toString().equals("GOOG.O"));
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
            refreshMsg.msgKey().name().data("GOOG.O");
            refreshMsg.state().streamState(StreamStates.OPEN);
            refreshMsg.state().dataState(DataStates.OK);
            refreshMsg.applySolicited();
            refreshMsg.applyRefreshComplete();

            assertTrue(provider.submitAndDispatch(refreshMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

            /* Consumer receives refresh. */
            consumerReactor.dispatch(1);
            event = consumerReactor.pollEvent();
            assertEquals(TestReactorEventTypes.MSG, event.type());
            msgEvent = (ReactorMsgEvent)event.reactorEvent();
            assertEquals(MsgClasses.REFRESH, msgEvent.msg().msgClass());

            receivedRefreshMsg = (RefreshMsg)msgEvent.msg();
            assertEquals(7, receivedRefreshMsg.streamId());
            assertTrue(receivedRefreshMsg.checkHasMsgKey());
            assertTrue(receivedRefreshMsg.checkSolicited());
            assertTrue(receivedRefreshMsg.checkRefreshComplete());
            assertTrue(receivedRefreshMsg.msgKey().checkHasServiceId());
            assertEquals(Provider.defaultService().serviceId(), receivedRefreshMsg.msgKey().serviceId());
            assertTrue(receivedRefreshMsg.msgKey().checkHasName());
            assertTrue(receivedRefreshMsg.msgKey().name().toString().equals("GOOG.O"));
            assertEquals(DomainTypes.MARKET_PRICE, receivedRefreshMsg.domainType());
            assertEquals(DataTypes.NO_DATA, receivedRefreshMsg.containerType());
            assertEquals(StreamStates.OPEN, receivedRefreshMsg.state().streamState());
            assertEquals(DataStates.OK, receivedRefreshMsg.state().dataState());
            assertNotNull(msgEvent.streamInfo());
            assertNotNull(msgEvent.streamInfo().serviceName());
            assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        }
        else
        {
            /* Provider receives nothing (no items recovered). */
            providerReactor.dispatch(0);
        }

        TestReactorComponent.closeSession(consumer, provider);
    }

    @Test
    public void consumerLoginCloseTest()
    {
        /* Test closing login stream from consumer. Test that an item requested is also considered closed when this happens. */

        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;
        CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
        RDMLoginMsgEvent loginMsgEvent;
        LoginClose receivedLoginClose;
        UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
        CloseMsg receivedCloseMsg;
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
        opts.reconnectAttemptLimit(-1);
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
        
        /* Consumer closes login stream. */
        closeMsg.clear();
        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(consumer.defaultSessionLoginStreamId());
        closeMsg.domainType(DomainTypes.LOGIN);
        assertTrue(consumer.submit(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives status for TRI. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.CLOSED, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());

        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));

        /* Provider receives login close. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();

        loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
        assertEquals(LoginMsgType.CLOSE, loginMsgEvent.rdmLoginMsg().rdmMsgType());

        receivedLoginClose = (LoginClose)loginMsgEvent.rdmLoginMsg();
        assertEquals(provider.defaultSessionLoginStreamId(), receivedLoginClose.streamId());

        /* Provider sends an update. */
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(providerStreamId);
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.containerType(DataTypes.NO_DATA);
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer does not receive it. */
        consumerReactor.dispatch(0);

        /* Provider receives close for the update. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
        receivedCloseMsg = (CloseMsg)msgEvent.msg();
        assertEquals(providerStreamId, receivedCloseMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());

        /* Disconnect provider. */
        provider.closeChannel();

        /* Consumer receives Channel Event, Directory Update, nothing else (only item is closed). */
        consumerReactor.dispatch(2);
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());

        RDMDirectoryMsgEvent directoryMsgEvent;                
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals(DirectoryMsgType.UPDATE, directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());   

        TestReactorComponent.closeSession(consumer, provider);
    }

    @Test
    public void loginClosedStatusTest()
    {
        /* Test closing login stream from provider. Test that an item requested is also considered closed when this happens. */

        ReactorSubmitOptions submitOptions = ReactorFactory.createReactorSubmitOptions();
        TestReactorEvent event;
        ReactorMsgEvent msgEvent;
        RequestMsg requestMsg = (RequestMsg)CodecFactory.createMsg();
        RequestMsg receivedRequestMsg;
        RefreshMsg refreshMsg = (RefreshMsg)CodecFactory.createMsg();
        RefreshMsg receivedRefreshMsg;
        StatusMsg receivedStatusMsg;
        RDMLoginMsgEvent loginMsgEvent;
        UpdateMsg updateMsg = (UpdateMsg)CodecFactory.createMsg();
        CloseMsg closeMsg = (CloseMsg)CodecFactory.createMsg();
        CloseMsg receivedCloseMsg;
        LoginStatus loginStatus = (LoginStatus)LoginMsgFactory.createMsg();
        LoginStatus receivedLoginStatus;
        ReactorErrorInfo errorInfo = ReactorFactory.createReactorErrorInfo();
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
        opts.reconnectAttemptLimit(-1);
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

        /* Provider sends redirecting StatusMsg .*/
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

        /* Provider sends login Closed status. */
        loginStatus.clear();
        loginStatus.rdmMsgType(LoginMsgType.STATUS);
        loginStatus.streamId(provider.defaultSessionLoginStreamId());
        loginStatus.applyHasState();
        loginStatus.state().streamState(StreamStates.CLOSED);
        loginStatus.state().dataState(DataStates.SUSPECT);
        submitOptions.clear();
        assertTrue(provider.submitAndDispatch(loginStatus, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer receives closed/suspect login status. */
        consumerReactor.dispatch(2);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
        loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
        assertEquals(LoginMsgType.STATUS, loginMsgEvent.rdmLoginMsg().rdmMsgType());
        receivedLoginStatus = (LoginStatus)loginMsgEvent.rdmLoginMsg();
        assertEquals(consumer.defaultSessionLoginStreamId(), receivedLoginStatus.streamId());
        assertTrue(receivedLoginStatus.checkHasState());
        assertEquals(StreamStates.CLOSED, receivedLoginStatus.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedLoginStatus.state().dataState());

        /* Consumer receives status for TRI. */
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.STATUS, msgEvent.msg().msgClass());

        receivedStatusMsg = (StatusMsg)msgEvent.msg();
        assertEquals(5, receivedStatusMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedStatusMsg.domainType());
        assertEquals(DataTypes.NO_DATA, receivedStatusMsg.containerType());
        assertEquals(StreamStates.CLOSED, receivedStatusMsg.state().streamState());
        assertEquals(DataStates.SUSPECT, receivedStatusMsg.state().dataState());

        assertNotNull(msgEvent.streamInfo());
        assertNotNull(msgEvent.streamInfo().serviceName());
        assertTrue(msgEvent.streamInfo().serviceName().equals(Provider.defaultService().info().serviceName().toString()));
        
        /* Provider sends an update for TRI. */
        updateMsg.clear();
        updateMsg.msgClass(MsgClasses.UPDATE);
        updateMsg.streamId(providerStreamId);
        updateMsg.domainType(DomainTypes.MARKET_PRICE);
        updateMsg.containerType(DataTypes.NO_DATA);
        assertTrue(provider.submitAndDispatch(updateMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Consumer does not receive it since the item is closed. */
        consumerReactor.dispatch(0);

        /* Provider receives close for the update. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.MSG, event.type());
        msgEvent = (ReactorMsgEvent)event.reactorEvent();
        assertEquals(MsgClasses.CLOSE, msgEvent.msg().msgClass());
        receivedCloseMsg = (CloseMsg)msgEvent.msg();
        assertEquals(providerStreamId, receivedCloseMsg.streamId());
        assertEquals(DomainTypes.MARKET_PRICE, receivedCloseMsg.domainType());

        /* Consumer closes login stream. */
        closeMsg.clear();
        closeMsg.msgClass(MsgClasses.CLOSE);
        closeMsg.streamId(consumer.defaultSessionLoginStreamId());
        closeMsg.domainType(DomainTypes.LOGIN);
        assertTrue(consumer.submitAndDispatch(closeMsg, submitOptions) >= ReactorReturnCodes.SUCCESS);

        /* Provider receives nothing. */
        providerReactor.dispatch(0);

        TestReactorComponent.closeSession(consumer, provider);
    }
}
