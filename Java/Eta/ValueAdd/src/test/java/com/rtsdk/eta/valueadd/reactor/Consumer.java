///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.eta.valueadd.reactor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;

import com.rtsdk.eta.codec.CodecFactory;
import com.rtsdk.eta.codec.CodecReturnCodes;
import com.rtsdk.eta.codec.DataStates;
import com.rtsdk.eta.codec.DataTypes;
import com.rtsdk.eta.codec.DecodeIterator;
import com.rtsdk.eta.codec.Msg;
import com.rtsdk.eta.codec.MsgClasses;
import com.rtsdk.eta.codec.RequestMsg;
import com.rtsdk.eta.codec.StateCodes;
import com.rtsdk.eta.codec.StreamStates;
import com.rtsdk.eta.rdm.ClassesOfService.AuthenticationTypes;
import com.rtsdk.eta.rdm.ClassesOfService.DataIntegrityTypes;
import com.rtsdk.eta.rdm.ClassesOfService.FlowControlTypes;
import com.rtsdk.eta.rdm.ClassesOfService.GuaranteeTypes;
import com.rtsdk.eta.rdm.DomainTypes;
import com.rtsdk.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.rtsdk.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.rtsdk.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.rtsdk.eta.valueadd.domainrep.rdm.login.LoginRequest;
import com.rtsdk.eta.valueadd.reactor.TunnelStreamMsg.TunnelStreamAck;


/** Represents a consumer component. */
public class Consumer extends TestReactorComponent implements ReactorAuthTokenEventCallback, ReactorServiceEndpointEventCallback, ConsumerCallback, TunnelStreamStatusEventCallback, TunnelStreamDefaultMsgCallback, TunnelStreamQueueMsgCallback
{
    AckRangeList _ackRangeList = new AckRangeList();
    AckRangeList _nakRangeList = new AckRangeList();
    
    public Consumer(TestReactor testReactor)
    {
        super(testReactor);
        _reactorRole = ReactorFactory.createConsumerRole();
    }
    
    @Override
    public int reactorServiceEndpointEventCallback(ReactorServiceEndpointEvent event)
    {
    	return _testReactor.handleServiceEndpointEvent(event);
    }
    
    @Override
    public int reactorChannelEventCallback(ReactorChannelEvent event)
    {
        return _testReactor.handleChannelEvent(event);
    }

    @Override
    public int defaultMsgCallback(ReactorMsgEvent event)
    {
        return _testReactor.handleDefaultMsgEvent(event);
    }

    @Override
    public int rdmLoginMsgCallback(RDMLoginMsgEvent event)
    {
        return _testReactor.handleLoginMsgEvent(event);
    }

    @Override
    public int reactorAuthTokenEventCallback(ReactorAuthTokenEvent event)
    {
        return _testReactor.handleAuthTokenEvent(event);
    }    
    
    @Override
    public int rdmDirectoryMsgCallback(RDMDirectoryMsgEvent event)
    {
        return _testReactor.handleDirectoryMsgEvent(event);
    }

    @Override
    public int rdmDictionaryMsgCallback(RDMDictionaryMsgEvent event)
    {
        return _testReactor.handleDictionaryMsgEvent(event);
    }

    @Override
    public int queueMsgCallback(TunnelStreamQueueMsgEvent event)
    {
        return _testReactor.handleQueueMsgEvent(event);
    }

    @Override
    public int defaultMsgCallback(TunnelStreamMsgEvent event)
    {
        return _testReactor.handleTunnelStreamMsgEvent(event);
    }

    @Override
    public int statusEventCallback(TunnelStreamStatusEvent event)
    {
        return _testReactor.handleTunnelStreamStatusEvent(event);
    }
    
    /** Information returned from openTunnelStream function. */
    public class OpenedTunnelStreamInfo
    {
        private int _providerTunnelStreamId;
        private TunnelStream _consumerTunnelStream;
        private TunnelStream _providerTunnelStream;
        
        public OpenedTunnelStreamInfo(TunnelStream consumerTunnelStream, TunnelStream providerTunnelStream, int providerTunnelStreamId)
        {
            _providerTunnelStreamId = providerTunnelStreamId;
            _consumerTunnelStream = consumerTunnelStream;
            _providerTunnelStream = providerTunnelStream;
        }
        
        /** Provider-side tunnel stream ID. This may not match the consumer side when
         * the watchlist is enabled. */
        public int providerTunnelStreamId() { return _providerTunnelStreamId; }

        /** Consumer's new tunnel stream. */
        public TunnelStream consumerTunnelStream() { return _consumerTunnelStream; }

        /** Providers's new tunnel stream (not set on core providers). */
        public TunnelStream providerTunnelStream() { return _providerTunnelStream; }
    }

    /** Opens a tunnel stream between this consumer and a TunnelStreamProvider. */
    public OpenedTunnelStreamInfo openTunnelStream(TunnelStreamProvider provider, TunnelStreamOpenOptions tsOpenOpts)
    {
        TunnelStream consTunnelStream;
        TunnelStream provTunnelStream;
        TestReactorEvent event;
        TunnelStreamStatusEvent tsStatusEvent;

        /* Open a TunnelStream. */
        assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel().openTunnelStream(tsOpenOpts, _errorInfo));
        testReactor().dispatch(0);
        
        /* Provider receives tunnel stream request event. */
        provider.testReactor().dispatch(2);
        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_REQUEST, event.type());
        TunnelStreamRequestEvent tsRequestEvent = event.tunnelStreamRequestEvent();
        assertEquals(tsOpenOpts.name(), tsRequestEvent.name());
        assertEquals(Provider.defaultService().serviceId(), tsRequestEvent.serviceId());
        assertEquals(provider.reactorChannel(), tsRequestEvent.reactorChannel());
 
        /* Provider already accepted in its callback, so it should get the status event as well. */
        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(provider.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.OK, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        provTunnelStream = tsStatusEvent.tunnelStream();

        if (tsOpenOpts.classOfService().authentication().type() == AuthenticationTypes.OMM_LOGIN)
        {
            /* Consumer receives nothing yet (sends authentication request). */
            testReactor().dispatch(0);

            Msg msg = CodecFactory.createMsg();
            LoginRequest loginRequest = (LoginRequest)LoginMsgFactory.createMsg();
            LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();
            DecodeIterator dIter = CodecFactory.createDecodeIterator();

            /* Provider receives authentication login request. */
            provider.testReactor().dispatch(1);
            event = provider.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());
            TunnelStreamMsgEvent tsMsgEvent = (TunnelStreamMsgEvent)event.reactorEvent();
            assertEquals(DataTypes.MSG, tsMsgEvent.containerType());
            assertNotNull(tsMsgEvent.msg());
            assertNotNull(tsMsgEvent.transportBuffer());
            dIter.clear();
            dIter.setBufferAndRWFVersion(tsMsgEvent.transportBuffer(), provTunnelStream.classOfService().common().protocolMajorVersion(),
				provTunnelStream.classOfService().common().protocolMinorVersion());
            assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));
            assertEquals(MsgClasses.REQUEST, msg.msgClass());
            assertEquals(DomainTypes.LOGIN, msg.domainType());
            loginRequest.rdmMsgType(LoginMsgType.REQUEST);
            assertEquals(CodecReturnCodes.SUCCESS, loginRequest.decode(dIter, msg));

            /* Provider sends login refresh. */
            loginRefresh.clear();
            loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
            loginRefresh.streamId(loginRequest.streamId());
            loginRefresh.state().streamState(StreamStates.OPEN);
            loginRefresh.state().dataState(DataStates.OK);
            loginRefresh.applyHasUserName();
            loginRefresh.userName().data(loginRequest.userName().data(), 
                    loginRequest.userName().position(), loginRequest.userName().length());
            loginRefresh.applySolicited();

            if (loginRequest.checkHasUserNameType())
            {
                loginRefresh.applyHasUserNameType();
                loginRefresh.userNameType(loginRequest.userNameType());
            }

            if (loginRequest.checkHasAttrib())
            {
                loginRefresh.applyHasAttrib();
                loginRefresh.attrib(loginRequest.attrib());
            }

            assertEquals(ReactorReturnCodes.SUCCESS, provTunnelStream.submit(loginRefresh, _errorInfo));
            provider.testReactor().dispatch(0);
        }
         
        /* Consumer receives tunnel stream status event. */
        testReactor().dispatch(1);
        event = testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.OK, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());  
        assertNotNull(tsStatusEvent.tunnelStream());
        consTunnelStream = tsStatusEvent.tunnelStream();

        if (tsOpenOpts.classOfService().authentication().type() == AuthenticationTypes.OMM_LOGIN)
        {
            /* A LoginRefresh should be present. */
            assertNotNull(tsStatusEvent.authInfo());
            assertNotNull(tsStatusEvent.authInfo().loginMsg());
            assertEquals(LoginMsgType.REFRESH, tsStatusEvent.authInfo().loginMsg().rdmMsgType());

            LoginRefresh loginRefresh = (LoginRefresh)tsStatusEvent.authInfo().loginMsg();
            assertEquals(StreamStates.OPEN, loginRefresh.state().streamState());
            assertEquals(DataStates.OK, loginRefresh.state().dataState());
            assertEquals(StateCodes.NONE, loginRefresh.state().code());
        }
        else
        {
            assertEquals(null, tsStatusEvent.authInfo());
        }

        return new OpenedTunnelStreamInfo(consTunnelStream, provTunnelStream, provTunnelStream.streamId());
    }
    
    /** Opens a tunnel stream between this consumer and a TunnelStreamCoreProvider. */
    public OpenedTunnelStreamInfo openTunnelStream(TunnelStreamCoreProvider provider, TunnelStreamOpenOptions tsOpenOpts, ClassOfService provClassOfService)
    {
        TestReactorEvent event;
        ReadEvent readEvent;
        TunnelStreamStatusEvent tsStatusEvent;        
        Msg msg;
        
        assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel().openTunnelStream(tsOpenOpts, _errorInfo));
        testReactor().dispatch(0);
        
        /* Provider receives tunnel stream request event. */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        msg = readEvent.msg();
                
        assertEquals(MsgClasses.REQUEST, msg.msgClass());
        assertEquals(tsOpenOpts.domainType(), msg.domainType());
        provider.acceptTunnelStreamRequest(this, (RequestMsg)msg, provClassOfService);
        
        /* Consumer receives tunnel stream status event. */
        testReactor().dispatch(1);
        event = testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.OK, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());  
        assertNotNull(tsStatusEvent.tunnelStream());
        testReactor().dispatch(0);       
        
        return new OpenedTunnelStreamInfo(tsStatusEvent.tunnelStream(), null, msg.streamId());
    }

    /**  Convenience function for establishing a queue messaging stream.
     * Initializes the consumer's role, and opens up a session & tunnel stream with queue 
     * messaging enabled. */
    public OpenedTunnelStreamInfo openSessionAndQueueMessagingTunnelStream(TunnelStreamCoreProvider provider, boolean persistLocally, boolean recoveringChannel, ClassOfService provClassOfService,
            boolean enableWatchlist)
    {
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.reconnectAttemptLimit(-1);

        ConsumerRole consumerRole = (ConsumerRole)reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(this);
        consumerRole.loginMsgCallback(this);
        consumerRole.directoryMsgCallback(this);
        consumerRole.dictionaryMsgCallback(this);
        consumerRole.defaultMsgCallback(this);
        consumerRole.watchlistOptions().enableWatchlist(enableWatchlist);

        CoreComponent.openSession(this, provider, opts, recoveringChannel);

        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("QueueMessagingTunnelStream");
        tsOpenOpts.statusEventCallback(this);
        tsOpenOpts.queueMsgCallback(this);
        tsOpenOpts.defaultMsgCallback(this);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.classOfService().flowControl().type(FlowControlTypes.BIDIRECTIONAL);
        tsOpenOpts.classOfService().guarantee().type(GuaranteeTypes.PERSISTENT_QUEUE);
        tsOpenOpts.classOfService().guarantee().persistLocally(persistLocally);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        
        return openTunnelStream(provider, tsOpenOpts, provClassOfService);
    }
    

    /** Closes a tunnelStream to a provider.
     * 
     * @param provider Provider component.
     * @param consTunnelStream Consumer's tunnel stream.
     * @param consTunnelStream Provider's tunnel stream.
     * @param finalStatusEvent Whether the consumer & provider want to get the final TunnelStreamStatusEvent.
     */
    public void closeTunnelStream(TunnelStreamProvider provider, TunnelStream consTunnelStream,
            TunnelStream provTunnelStream, boolean finalStatusEvent)
    {
        TestReactorEvent event;
        TunnelStreamStatusEvent tsStatusEvent;
        
        /* Close tunnel stream from consumer. */
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.close(finalStatusEvent, _errorInfo));
        testReactor().dispatch(0);
        
        /* Provider receives & acks FIN, and gets open/suspect status. */
        provider.testReactor().dispatch(1);
        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(provTunnelStream, tsStatusEvent.tunnelStream());
        provider.testReactor().dispatch(0);
        
        /* Consumer receives tunnel ack, and tunnel fin, sends close. */
        if (finalStatusEvent)
        {
            /* Consumer receives close status. */
            testReactor().dispatch(1);
            event = testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
            tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
            assertEquals(StreamStates.CLOSED_RECOVER, tsStatusEvent.state().streamState());
            assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
            assertEquals(consTunnelStream, tsStatusEvent.tunnelStream());
        }
        else
            testReactor().dispatch(0);
        
        /* Provider receives tunnel ack, tunnel fin, and close. */
        provider.testReactor().dispatch(1);
        event = provider.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(StreamStates.CLOSED, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(provTunnelStream, tsStatusEvent.tunnelStream());

    }
    
    /** Closes a tunnelStream to a core provider.
     * 
     * @param provider Core provider component.
     * @param consTunnelStream Consumer's tunnel stream.
     * @param provTunnelStreamId Stream ID on provider side (may not match consumer if watchlist is enabled).
     * @param provFinSeqNum Sequence number to use as the provider's FIN.
     * @param finalStatusEvent Whether the consumerw wants to get the final TunnelStreamStatusEvent.
     */
    public void closeTunnelStream(TunnelStreamCoreProvider provider, TunnelStream consTunnelStream, int provTunnelStreamId, int provFinSeqNum, boolean finalStatusEvent)
    {
        TunnelStreamMsg tunnelMsg;
        TestReactorEvent event;
        ReadEvent readEvent;
        Msg msg;
        TunnelStreamAck tunnelStreamAck = new TunnelStreamMsgImpl();
        
        /* Close tunnel stream from consumer. */
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.close(finalStatusEvent, _errorInfo));
        testReactor().dispatch(0);
        
        /* Provider receives FIN. */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        msg = readEvent.msg();
        tunnelMsg = readEvent.tunnelMsg(msg, _ackRangeList, _nakRangeList);
        assertEquals(TunnelStreamMsg.OpCodes.ACK, tunnelMsg.opCode());
        assertEquals(provTunnelStreamId, tunnelMsg.streamId());
        assertEquals(consTunnelStream.domainType(), tunnelMsg.domainType());
        assertEquals(((TunnelStreamAck)tunnelMsg).flag(), 0x1);
        
        /* Provider acks FIN. */
        tunnelStreamAck.clearAck();
        tunnelStreamAck.recvWindow(TunnelStream.DEFAULT_RECV_WINDOW);
        tunnelStreamAck.seqNum(((TunnelStreamAck)tunnelMsg).seqNum());
        ((TunnelStreamMsg)tunnelStreamAck).streamId(provTunnelStreamId);
        ((TunnelStreamMsg)tunnelStreamAck).domainType(consTunnelStream.domainType());
        provider.submitTunnelStreamAck(tunnelStreamAck);
        
        /* Provider sends FIN. */
        tunnelStreamAck.clearAck();
        tunnelStreamAck.recvWindow(TunnelStream.DEFAULT_RECV_WINDOW);
        ((TunnelStreamMsg)tunnelStreamAck).streamId(provTunnelStreamId);
        ((TunnelStreamMsg)tunnelStreamAck).domainType(consTunnelStream.domainType());
        tunnelStreamAck.seqNum(provFinSeqNum);
        provider.submitTunnelStreamAck(tunnelStreamAck, null, null, 0x1);
        
        /* Consumer receives tunnel ack, and tunnel fin, sends close. */
        if (finalStatusEvent)
        {
            /* Consumer receives close status. */
            TunnelStreamStatusEvent tsStatusEvent;
            testReactor().dispatch(1);
            event = testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
            tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
            assertEquals(StreamStates.CLOSED_RECOVER, tsStatusEvent.state().streamState());
            assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
            assertEquals(consTunnelStream, tsStatusEvent.tunnelStream());
        }
        else
            testReactor().dispatch(0);
        
        /* Provider receives ack of FIN. */
        provider.dispatch(2);
        readEvent = provider.pollEvent();
        msg = readEvent.msg();
        tunnelMsg = readEvent.tunnelMsg(msg, _ackRangeList, _nakRangeList);
        assertEquals(TunnelStreamMsg.OpCodes.ACK, tunnelMsg.opCode());
        assertEquals(provTunnelStreamId, tunnelMsg.streamId());
        assertEquals(consTunnelStream.domainType(), tunnelMsg.domainType());
        assertEquals(provFinSeqNum, ((TunnelStreamAck)tunnelMsg).seqNum());
        
        /* Provider receives close. */
        readEvent = provider.pollEvent();
        msg = readEvent.msg();
        assertEquals(MsgClasses.CLOSE, msg.msgClass());
    }
}
