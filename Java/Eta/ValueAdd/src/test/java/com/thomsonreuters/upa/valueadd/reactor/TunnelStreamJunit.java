///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.valueadd.reactor;
import static com.thomsonreuters.upa.valueadd.reactor.SlicedBufferPool.TUNNEL_STREAM_HDR_SIZE;
import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertNull;
import static org.junit.Assert.assertTrue;
import static java.lang.Math.abs;
import static org.mockito.Mockito.*;


import java.nio.ByteBuffer;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SocketChannel;
import java.util.Arrays;
import java.util.Collection;

import org.junit.After;
import org.junit.Before;
import org.junit.Test;
import org.junit.runners.Parameterized;
import org.junit.runners.Parameterized.Parameters;
import org.junit.runner.RunWith;


import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataStates;
import com.thomsonreuters.upa.codec.DataTypes;
import com.thomsonreuters.upa.codec.DecodeIterator;
import com.thomsonreuters.upa.codec.EncodeIterator;
import com.thomsonreuters.upa.codec.Msg;
import com.thomsonreuters.upa.codec.MsgClasses;
import com.thomsonreuters.upa.codec.RequestMsg;
import com.thomsonreuters.upa.codec.RefreshMsg;
import com.thomsonreuters.upa.codec.StateCodes;
import com.thomsonreuters.upa.codec.StreamStates;
import com.thomsonreuters.upa.rdm.ClassesOfService.AuthenticationTypes;
import com.thomsonreuters.upa.rdm.ClassesOfService.DataIntegrityTypes;
import com.thomsonreuters.upa.rdm.ClassesOfService.FlowControlTypes;
import com.thomsonreuters.upa.rdm.ClassesOfService.GuaranteeTypes;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InProgInfo;
import com.thomsonreuters.upa.transport.InitArgs;
import com.thomsonreuters.upa.transport.ReadArgs;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.WriteArgs;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.thomsonreuters.upa.valueadd.domainrep.rdm.login.LoginMsgType;
import org.mockito.InjectMocks;

/** Tests related to TunnelStreams. */
@RunWith(value = Parameterized.class)
public class TunnelStreamJunit
{
    /** Reusable ReactorErrorInfo */
    ReactorErrorInfo _errorInfo = ReactorFactory.createReactorErrorInfo();
    TunnelStreamSubmitOptions _tsSubmitOpts = ReactorFactory.createTunnelStreamSubmitOptions();


    boolean _enableWatchlist;

    @Parameters
    public static Collection<Object[]> config()
    {
        Object[][] config = new Object[][]
                {
                        {false},
                        {true},
                };

        return Arrays.asList(config);
    }

    public TunnelStreamJunit(boolean enableWatchlist)
    {
        _enableWatchlist = enableWatchlist;

    }


    /** This provider always rejects tunnel streams. */
    class TunnelStreamRejectProvider extends Provider
    {
        TunnelStreamRejectOptions _rejectOptions = ReactorFactory.createTunnelStreamRejectOptions();

        public TunnelStreamRejectProvider(TestReactor reactor)
        {
            super(reactor);
        }

        @Override
        public int listenerCallback(TunnelStreamRequestEvent event)
        {
            super.listenerCallback(event);

            /* Accept the tunnel stream request. */
            _rejectOptions.clear();
            _rejectOptions.state().streamState(StreamStates.CLOSED);
            _rejectOptions.state().dataState(DataStates.SUSPECT);
            _rejectOptions.state().code(StateCodes.NOT_ENTITLED);
            assertEquals(ReactorReturnCodes.SUCCESS, reactorChannel().rejectTunnelStream(event, _rejectOptions, _errorInfo));

            return ReactorReturnCodes.SUCCESS;
        }
    }

    @Before
    public void initializeTransport()
    {
        InitArgs initArgs = TransportFactory.createInitArgs();
        initArgs.globalLocking(true);
        assertEquals(TransportReturnCodes.SUCCESS, Transport.initialize(initArgs, _errorInfo.error()));
    }

    @After
    public void uninitializeTransport()
    {
        assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
    }

    @Test
    public void tunnelStreamMsgExchangeTest()
    {
        tunnelStreamMsgExchangeTest(false);
    }

    @Test
    public void tunnelStreamMsgExchangeTest_Authenticate()
    {
        tunnelStreamMsgExchangeTest(true);
    }

    public void tunnelStreamMsgExchangeTest(boolean authenticate)
    {
        /* Test opening a TunnelStream and exchanging a couple messages (consumer to prov, then prov to consumer). */

        TestReactorEvent event;
        TunnelStreamStatusEvent tsStatusEvent;
        TunnelStreamMsgEvent tsMsgEvent;
        TunnelStream consTunnelStream;
        TunnelStream provTunnelStream;
        TransportBuffer buffer;

        /* Setup some sample data. */
        String sampleString = "PETER CAPALDI"; 
        ByteBuffer sampleData = ByteBuffer.allocateDirect(sampleString.length());
        sampleData.put(sampleString.getBytes());

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
        consumerRole.watchlistOptions().enableWatchlist(_enableWatchlist);


        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);

        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     
        if (authenticate)
            tsOpenOpts.classOfService().authentication().type(AuthenticationTypes.OMM_LOGIN);

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        consTunnelStream = openedTsInfo.consumerTunnelStream();
        provTunnelStream = openedTsInfo.providerTunnelStream();

        /* Test tunnel stream accessors */
        assertEquals(5, consTunnelStream.streamId());
        if (!_enableWatchlist) /* Watchlist will likely use different stream ID. */
            assertEquals(5, provTunnelStream.streamId());
        assertEquals(DomainTypes.SYSTEM, consTunnelStream.domainType());
        assertEquals(DomainTypes.SYSTEM, provTunnelStream.domainType());
        assertEquals(Provider.defaultService().serviceId(), consTunnelStream.serviceId());
        assertEquals(Provider.defaultService().serviceId(), provTunnelStream.serviceId());
        assertEquals(consumer.reactorChannel(), consTunnelStream.reactorChannel());
        assertEquals(provider.reactorChannel(), provTunnelStream.reactorChannel());
        assertEquals(false, consTunnelStream.isProvider());
        assertEquals(true, provTunnelStream.isProvider());
        assertEquals("Tunnel1", consTunnelStream.name());
        assertEquals("Tunnel1", provTunnelStream.name());
        assertEquals(consumer, consTunnelStream.userSpecObject());
        assertEquals(null, provTunnelStream.userSpecObject());
        assertEquals(StreamStates.OPEN, consTunnelStream.state().streamState());
        assertEquals(DataStates.OK, consTunnelStream.state().dataState());
        assertEquals(StateCodes.NONE, consTunnelStream.state().code());
        assertEquals(StreamStates.OPEN, provTunnelStream.state().streamState());
        assertEquals(DataStates.OK, provTunnelStream.state().dataState());
        assertEquals(StateCodes.NONE, provTunnelStream.state().code());

        /* Test consumer tunnel stream class of service values */
        assertEquals(Codec.protocolType(), consTunnelStream.classOfService().common().protocolType());
        assertEquals(Codec.majorVersion(), consTunnelStream.classOfService().common().protocolMajorVersion());
        assertEquals(Codec.minorVersion(), consTunnelStream.classOfService().common().protocolMinorVersion());
        assertEquals(614400, consTunnelStream.classOfService().common().maxMsgSize());
        if (authenticate)
            assertEquals(AuthenticationTypes.OMM_LOGIN, consTunnelStream.classOfService().authentication().type());
        else
            assertEquals(AuthenticationTypes.NOT_REQUIRED, consTunnelStream.classOfService().authentication().type());
        assertEquals(FlowControlTypes.BIDIRECTIONAL, consTunnelStream.classOfService().flowControl().type());
        assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, consTunnelStream.classOfService().flowControl().sendWindowSize());
        assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, consTunnelStream.classOfService().flowControl().recvWindowSize());
        assertEquals(DataIntegrityTypes.RELIABLE, consTunnelStream.classOfService().dataIntegrity().type());
        assertEquals(GuaranteeTypes.NONE, consTunnelStream.classOfService().guarantee().type());

        /* Test provider tunnel stream class of service values */
        assertEquals(Codec.protocolType(), provTunnelStream.classOfService().common().protocolType());
        assertEquals(Codec.majorVersion(), provTunnelStream.classOfService().common().protocolMajorVersion());
        assertEquals(Codec.minorVersion(), provTunnelStream.classOfService().common().protocolMinorVersion());
        assertEquals(614400, provTunnelStream.classOfService().common().maxMsgSize());
        if (authenticate)
            assertEquals(AuthenticationTypes.OMM_LOGIN, provTunnelStream.classOfService().authentication().type());
        else
            assertEquals(AuthenticationTypes.NOT_REQUIRED, provTunnelStream.classOfService().authentication().type());
        assertEquals(FlowControlTypes.BIDIRECTIONAL, provTunnelStream.classOfService().flowControl().type());
        assertEquals(provTunnelStream.classOfService().common().maxFragmentSize(), provTunnelStream.classOfService().flowControl().sendWindowSize());
        assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, provTunnelStream.classOfService().flowControl().recvWindowSize());
        assertEquals(DataIntegrityTypes.RELIABLE, provTunnelStream.classOfService().dataIntegrity().type());
        assertEquals(GuaranteeTypes.NONE, provTunnelStream.classOfService().guarantee().type());

        /* Consumer sends an opaque buffer to the provider. */
        _tsSubmitOpts.clear();
        _tsSubmitOpts.containerType(DataTypes.OPAQUE);
        assertNotNull(buffer = consTunnelStream.getBuffer(13, _errorInfo));
        sampleData.position(0);
        sampleData.limit(sampleString.length());
        buffer.data().put(sampleData);
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        consumerReactor.dispatch(0);

        /* Provider receives the buffer. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());
        tsMsgEvent = (TunnelStreamMsgEvent)event.reactorEvent();
        assertEquals(DataTypes.OPAQUE, tsMsgEvent.containerType());
        assertNotNull(buffer = tsMsgEvent.transportBuffer());
        assertEquals(sampleString.length(), buffer.length());
        assertTrue(buffer.toString().equals(sampleString));

        /* Provider sends an opaque buffer to the consumer. */
        _tsSubmitOpts.clear();
        _tsSubmitOpts.containerType(DataTypes.OPAQUE);
        assertNotNull(buffer = provTunnelStream.getBuffer(13, _errorInfo));
        sampleData.position(0);
        sampleData.limit(sampleString.length());
        buffer.data().put(sampleData);
        assertEquals(ReactorReturnCodes.SUCCESS, provTunnelStream.submit(buffer, _tsSubmitOpts, _errorInfo));
        providerReactor.dispatch(0);

        /* Consumer receives the buffer. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());
        tsMsgEvent = (TunnelStreamMsgEvent)event.reactorEvent();
        assertEquals(DataTypes.OPAQUE, tsMsgEvent.containerType());
        assertNotNull(buffer = tsMsgEvent.transportBuffer());
        assertEquals(sampleString.length(), buffer.length());
        assertTrue(buffer.toString().equals(sampleString));

        /* Provider closes the tunnel stream, starting FIN/ACK teardown */
        provTunnelStream.close(false, _errorInfo);
        providerReactor.dispatch(0);

        /* Consumer receives the open/suspect event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);

        /* Provider Reactor internally responds to FIN (no message). */
        providerReactor.dispatch(0);

        /* Consumer receives the close event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.CLOSED, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);

        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }


    /** This consumer submit post message in the callback message of TunnelStreamMsgEvent. */
    class MsgSubmitConsumer extends Consumer
    {
    	MsgSubmitConsumer(TestReactor reactor)
    	{
            super(reactor);
        }

        @Override
        public int defaultMsgCallback(TunnelStreamMsgEvent event)
        {
            super.defaultMsgCallback(event);
            Msg msg = CodecFactory.createMsg();

            assertEquals(MsgClasses.REFRESH, event.msg().msgClass() );

            msg.clear();
            msg.msgClass(MsgClasses.POST);
            msg.domainType(DomainTypes.MARKET_PRICE);
            msg.streamId(6); // Set stream ID of post message
            assertEquals(ReactorReturnCodes.SUCCESS, event.tunnelStream().submit(msg, _errorInfo));

            // Confirm that submitting did not modify our received message.
            assertEquals(MsgClasses.REFRESH, event.msg().msgClass() );

            return ReactorReturnCodes.SUCCESS;
        }
    }

    @Test
    public void tunnelStreamMsgTestSubmittingMsgInCallbackTest()
    {
        /* Test opening a TunnelStream to exchange messages and submitting a post message in the client callback */

        TestReactorEvent event;
        TunnelStreamStatusEvent tsStatusEvent;
        TunnelStreamMsgEvent tsMsgEvent;
        TunnelStream consTunnelStream;
        TunnelStream provTunnelStream;

        /* Setup some sample data. */
        String sampleString = "PETER CAPALDI"; 
        ByteBuffer sampleData = ByteBuffer.allocateDirect(sampleString.length());
        sampleData.put(sampleString.getBytes());

        /* Create reactors. */
        TestReactor consumerReactor = new TestReactor();
        TestReactor providerReactor = new TestReactor();

        /* Create consumer. */
        Consumer consumer = new MsgSubmitConsumer(consumerReactor);
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        consumerRole.watchlistOptions().enableWatchlist(_enableWatchlist);


        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);

        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        consTunnelStream = openedTsInfo.consumerTunnelStream();
        provTunnelStream = openedTsInfo.providerTunnelStream();

        /* Test tunnel stream accessors */
        assertEquals(5, consTunnelStream.streamId());
        /* Provider sends a refresh to the consumer. */
        Msg msg = CodecFactory.createMsg();

        _tsSubmitOpts.clear();
        _tsSubmitOpts.containerType(DataTypes.MSG);
        msg.clear();
        msg.msgClass(MsgClasses.REFRESH);
        msg.streamId(5);
        msg.containerType(DataTypes.NO_DATA);
        ((RefreshMsg)msg).state().streamState(StreamStates.OPEN);
        ((RefreshMsg)msg).state().dataState(DataStates.OK);

        assertEquals(ReactorReturnCodes.SUCCESS, provTunnelStream.submit(msg, _errorInfo));
        providerReactor.dispatch(0);

        /* Consumer receives the message. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());
        tsMsgEvent = (TunnelStreamMsgEvent)event.reactorEvent();
        assertEquals(DataTypes.MSG, tsMsgEvent.containerType());
        assertEquals(MsgClasses.REFRESH, tsMsgEvent.msg().msgClass());

        /* Provider receives a post. */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());
        tsMsgEvent = (TunnelStreamMsgEvent)event.reactorEvent();
        assertEquals(DataTypes.MSG, tsMsgEvent.containerType());
        assertEquals(MsgClasses.POST, tsMsgEvent.msg().msgClass());
        assertEquals(6, tsMsgEvent.msg().streamId());

        /* Provider closes the tunnel stream, starting FIN/ACK teardown */
        provTunnelStream.close(false, _errorInfo);
        providerReactor.dispatch(0);

        /* Consumer receives the open/suspect event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);

        /* Provider Reactor internally responds to FIN (no message). */
        providerReactor.dispatch(0);

        /* Consumer receives the close event */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(null, tsStatusEvent.authInfo());
        assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
        assertEquals(StreamStates.CLOSED, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
        assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
        assertNotNull(tsStatusEvent.tunnelStream());
        consumerReactor.dispatch(0);

        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }


    @Test
    public void tunnelStreamCleanupTest()
    {
        /* Tests that the Reactor's TunnelStream lists are empty after the following scenarios:
         * - Normal FIN/ACK teardown
         * - Disconnection
         * - Disconnect (where the consumer doesn't close the TunnelStream)
         * - Rejected TunnelStreams
         */
        
        /* The channel-close portion reproduced ETA-1893(ConcurrentModificationException problem). 
        /* The rejection test reproduced ETA-1948(TunnelStreamRequestEvent channel read problem).  */

        /* If a leak is suspected, the runtime of this test can be set, so that memory usage
         * can more easily be observed via JConsole or JProfiler.
         * No need to manually request garbage collection; the test already does it.
         * NOTE: Portions of this test (at least the disconnection portion) show growth
         * up to a point. Based on profiling, this may be due to an object called
         * the ArrayNotificationBuffer and levels off eventually
         * (at last test, this occurred at around 15-16MB; this can take several minutes to reach). */
        long runTimeSec = 0;
        int minRunCount = 3;
        int tunnelStreamCount = 6;

        int runCount;
        TunnelStream consTunnels[] = new TunnelStream[tunnelStreamCount];
        TunnelStream provTunnels[] = new TunnelStream[tunnelStreamCount];
        long startTime;
        TestReactorEvent event;

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
        consumerRole.watchlistOptions().enableWatchlist(_enableWatchlist);


        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        opts.reconnectAttemptLimit(-1);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);

        /* Control check, in case some other test uncovers a problem. */
        assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
        assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
        assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
        assertEquals(0, provider.reactorChannel().streamIdtoTunnelStreamTable().size());

        assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
        assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
        assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
        assertEquals(0, consumer.reactorChannel().streamIdtoTunnelStreamTable().size());

        /* Set TunnelStream open options. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     

        /*** Test for proper cleanup after normal closing of streams. ***/
        if (runTimeSec > 0) System.out.println("Running TunnelStream normal teardown cleanup test.");
        startTime = System.nanoTime();
        runCount = 0;
        do
        {            
            for (int i = 0; i < tunnelStreamCount; ++i)
            {
                tsOpenOpts.streamId(i+5);
                Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
                consTunnels[i] = openedTsInfo.consumerTunnelStream();
                provTunnels[i] = openedTsInfo.providerTunnelStream();
            }

            for (int i = 0; i < tunnelStreamCount; ++i)
                consumer.closeTunnelStream(provider, consTunnels[i], provTunnels[i], true);
            System.gc();

            assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
            assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
            assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
            assertEquals(0, provider.reactorChannel().streamIdtoTunnelStreamTable().size());

            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
            assertEquals(0, consumer.reactorChannel().streamIdtoTunnelStreamTable().size());


        }
        while (++runCount < minRunCount || System.nanoTime() < startTime + 1000000000 * runTimeSec);

        /*** Test for proper cleanup after a disconnection. ***/

        if (runTimeSec > 0) System.out.println("Running TunnelStream disconnection cleanup test.");
        startTime = System.nanoTime();
        runCount = 0;
        do
        {            
            for (int i = 0; i < tunnelStreamCount; ++i)
            {
                tsOpenOpts.streamId(i+5);
                Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
                consTunnels[i] = openedTsInfo.consumerTunnelStream();
                provTunnels[i] = openedTsInfo.providerTunnelStream();
            }

            provider.closeChannel();
            consumerReactor.dispatch(1 /* Channel down event */ + (_enableWatchlist ? 2 : 0)+ tunnelStreamCount /* Tunnel Stream Status Events */);

            event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());

            if (_enableWatchlist)
            {
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
            }

            for (int i = 0; i < tunnelStreamCount; ++i)
            {
                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
                TunnelStreamStatusEvent tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
                assertEquals(null, tsStatusEvent.authInfo());
                assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
                assertEquals(StreamStates.CLOSED_RECOVER, tsStatusEvent.state().streamState());
                assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
                assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
                assertNotNull(tsStatusEvent.tunnelStream());
                assertEquals(consTunnels[i], tsStatusEvent.tunnelStream());

                assertEquals(ReactorReturnCodes.SUCCESS, tsStatusEvent.tunnelStream().close(false, _errorInfo));
            }
            System.gc();

            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
            assertEquals(0, consumer.reactorChannel().streamIdtoTunnelStreamTable().size());

            /* Reopen session. */
            TestReactor.openSession(consumer, provider, opts, true);

            assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
            assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
            assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
            assertEquals(0, provider.reactorChannel().streamIdtoTunnelStreamTable().size());
        }
        while (++runCount < minRunCount || System.nanoTime() < startTime + 1000000000 * runTimeSec);

        /*** Test for proper cleanup after a disconnection, even if consumer does not properly close. ***/

        if (runTimeSec > 0) System.out.println("Running TunnelStream disconnection cleanup test (consumer doesn't close).");
        startTime = System.nanoTime();
        runCount = 0;
        do
        {            
            for (int i = 0; i < tunnelStreamCount; ++i)
            {
                tsOpenOpts.streamId(i+5);
                Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
                consTunnels[i] = openedTsInfo.consumerTunnelStream();
                provTunnels[i] = openedTsInfo.providerTunnelStream();
            }

            provider.closeChannel();
            consumerReactor.dispatch(1 + /* Channel down event */ + (_enableWatchlist ? 2 : 0)+ tunnelStreamCount /* Tunnel Stream Status Events */);

            event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
            assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());

            if (_enableWatchlist)
            {
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
            }

            for (int i = 0; i < tunnelStreamCount; ++i)
            {
                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
                TunnelStreamStatusEvent tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
                assertEquals(null, tsStatusEvent.authInfo());
                assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
                assertEquals(StreamStates.CLOSED_RECOVER, tsStatusEvent.state().streamState());
                assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
                assertEquals(StateCodes.NONE, tsStatusEvent.state().code());
                assertNotNull(tsStatusEvent.tunnelStream());
                assertEquals(consTunnels[i], tsStatusEvent.tunnelStream());
            }
            System.gc();

            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
            assertEquals(0, consumer.reactorChannel().streamIdtoTunnelStreamTable().size());


            /* Reopen session. */
            TestReactor.openSession(consumer, provider, opts, true);

            assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
            assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
            assertEquals(0, provider.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
            assertEquals(0, provider.reactorChannel().streamIdtoTunnelStreamTable().size());


        }
        while (++runCount < minRunCount || System.nanoTime() < startTime + 1000000000 * runTimeSec);

        TestReactorComponent.closeSession(consumer, provider);

        /*** Test proper cleanup after a reject. ***/

        /* Create consumer. */
        consumer = new Consumer(consumerReactor);
        consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);
        consumerRole.watchlistOptions().enableWatchlist(_enableWatchlist);


        /* Create provider that rejects all tunnel streams. */
        TunnelStreamRejectProvider rejectProvider = new TunnelStreamRejectProvider(providerReactor);
        providerRole = (ProviderRole)rejectProvider.reactorRole();
        providerRole.channelEventCallback(rejectProvider);
        providerRole.loginMsgCallback(rejectProvider);
        providerRole.directoryMsgCallback(rejectProvider);
        providerRole.dictionaryMsgCallback(rejectProvider);
        providerRole.defaultMsgCallback(rejectProvider);
        providerRole.tunnelStreamListenerCallback(rejectProvider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        rejectProvider.bind(opts);
        TestReactor.openSession(consumer, rejectProvider, opts);

        /* Control check. */
        assertEquals(0, rejectProvider.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
        assertEquals(0, rejectProvider.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
        assertEquals(0, rejectProvider.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
        assertEquals(0, rejectProvider.reactorChannel().streamIdtoTunnelStreamTable().size());
        assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
        assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
        assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
        assertEquals(0, consumer.reactorChannel().streamIdtoTunnelStreamTable().size());

        /* Set TunnelStream open options. */
        tsOpenOpts.clear();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);   

        if (runTimeSec > 0) System.out.println("Running TunnelStream reject cleanup test.");
        startTime = System.nanoTime();
        runCount = 0;
        do
        {            
            for (int i = 0; i < tunnelStreamCount; ++i)
            {
                tsOpenOpts.streamId(i+5);
                assertEquals(ReactorReturnCodes.SUCCESS, consumer.reactorChannel().openTunnelStream(tsOpenOpts, _errorInfo));
            }

            consumerReactor.dispatch(0);
            providerReactor.dispatch(tunnelStreamCount);
            for (int i = 0; i < tunnelStreamCount; ++i)
            {
                event = providerReactor.pollEvent();
                assertEquals(event.type(), TestReactorEventTypes.TUNNEL_STREAM_REQUEST);
                if (!_enableWatchlist) /* Watchlist will likely use different stream ID. */
                {
                    TunnelStreamRequestEvent requestEvent = event.tunnelStreamRequestEvent();
                    assertEquals(5+i, requestEvent.streamId());
                }
            }

            consumerReactor.dispatch(0); // dispatch for request retry

            // provider receives request retry
            providerReactor.dispatch(tunnelStreamCount);
            for (int i = 0; i < tunnelStreamCount; ++i)
            {
                event = providerReactor.pollEvent();
                assertEquals(event.type(), TestReactorEventTypes.TUNNEL_STREAM_REQUEST);
                if (!_enableWatchlist) /* Watchlist will likely use different stream ID. */
                {
                    TunnelStreamRequestEvent requestEvent = event.tunnelStreamRequestEvent();
                    assertEquals(5+i, requestEvent.streamId());
                }
            }

            consumerReactor.dispatch(tunnelStreamCount);

            for (int i = 0; i < tunnelStreamCount; ++i)
            {
                event = consumerReactor.pollEvent();
                assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
                TunnelStreamStatusEvent tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
                assertEquals(null, tsStatusEvent.authInfo());
                assertEquals(consumer.reactorChannel(), tsStatusEvent.reactorChannel());
                assertEquals(StreamStates.CLOSED, tsStatusEvent.state().streamState());
                assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());
                assertEquals(StateCodes.NOT_ENTITLED, tsStatusEvent.state().code());
                assertNotNull(tsStatusEvent.tunnelStream());
                assertEquals(ReactorReturnCodes.SUCCESS, tsStatusEvent.tunnelStream().close(false, _errorInfo));
            }
            System.gc();

            assertEquals(0, rejectProvider.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
            assertEquals(0, rejectProvider.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
            assertEquals(0, rejectProvider.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
            assertEquals(0, rejectProvider.reactorChannel().streamIdtoTunnelStreamTable().size());

            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamDispatchList.count());
            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamList.count());
            assertEquals(0, consumer.reactorChannel().tunnelStreamManager()._tunnelStreamTimeoutList.count());
            assertEquals(0, consumer.reactorChannel().streamIdtoTunnelStreamTable().size());

        }
        while (++runCount < minRunCount || System.nanoTime() < startTime + 1000000000 * runTimeSec);

        TestReactorComponent.closeSession(consumer, rejectProvider);

        consumerReactor.close();
        providerReactor.close();


    }

    /* Encodes a sample message. Used with TunnelStreamBufferLengthAndCopyTest*/
    public void encodeSampleMsg(TunnelStream tunnelStream, TransportBuffer buffer)
    {
        EncodeIterator eIter = CodecFactory.createEncodeIterator();
        Msg msg = CodecFactory.createMsg();

        assertEquals(CodecReturnCodes.SUCCESS, eIter.setBufferAndRWFVersion(buffer,
                tunnelStream.classOfService().common().protocolMajorVersion(),
                tunnelStream.classOfService().common().protocolMinorVersion()));

        msg.msgClass(MsgClasses.GENERIC);
        msg.domainType(200);
        msg.streamId(1985);
        msg.containerType(DataTypes.OPAQUE);
        msg.encodedDataBody().data("Sample");

        assertEquals(CodecReturnCodes.SUCCESS, msg.encode(eIter));
    }

    /* Decodes a sample message and ensures it is correct. Used with TunnelStreamBufferLengthAndCopyTest*/
    public void decodeSampleMsg(TunnelStream tunnelStream, TransportBuffer buffer)
    {
        DecodeIterator dIter = CodecFactory.createDecodeIterator();
        Msg msg = CodecFactory.createMsg();

        assertEquals(CodecReturnCodes.SUCCESS, dIter.setBufferAndRWFVersion(buffer,
                tunnelStream.classOfService().common().protocolMajorVersion(),
                tunnelStream.classOfService().common().protocolMinorVersion()));

        assertEquals(CodecReturnCodes.SUCCESS, msg.decode(dIter));

        assertEquals(MsgClasses.GENERIC, msg.msgClass());
        assertEquals(200, msg.domainType());
        assertEquals(1985, msg.streamId());
        assertEquals(DataTypes.OPAQUE, msg.containerType());
        assertTrue(msg.encodedDataBody().toString().equals("Sample"));
    }


    /* Used with TunnelStreamBufferLengthAndCopyTest; tests the length, capacity, and copy functions
     * with the received buffer.
     * It expects two buffers; first is OPAQUE, second is a MSG. 
     * */
    class TunnelStreamBufferLengthAndCopyTest_Provider extends TunnelStreamProvider
    {
        TunnelStream _tunnelStream; /** Provider's tunnel stream. */
        int _expectedMsgBufferLength; /** Expected length of the transport buffer containing the MSG. */
        int _msgCount = 0; /** Tracks which incoming buffer the provider is expecting. */

        public void tunnelStream(TunnelStream tunnelStream)
        {
            _tunnelStream = tunnelStream;
        }

        public void expectedMsgBufferLength(int length)
        {
            _expectedMsgBufferLength = length;
        }

        TunnelStreamBufferLengthAndCopyTest_Provider(TestReactor reactor)
        {
            super(reactor);
        }

        @Override
        public int defaultMsgCallback(TunnelStreamMsgEvent event)
        {
            super.defaultMsgCallback(event);
            TransportBuffer tunnelBuffer;
            assertNotNull(tunnelBuffer = event.transportBuffer());

            if (_msgCount == 0)
            {
                /* First buffer is opaque. */
                assertEquals(DataTypes.OPAQUE, event.containerType());

                assertEquals(8, tunnelBuffer.length());
                assertEquals(8, tunnelBuffer.capacity());

                /* Copy the buffer and test for correct value. */
                ByteBuffer destBuffer = ByteBuffer.allocate(8);
                assertEquals(ReactorReturnCodes.SUCCESS, tunnelBuffer.copy(destBuffer));
                assertEquals(8, tunnelBuffer.capacity());
                assertEquals(8, tunnelBuffer.length());
                destBuffer.position(0);

                assertEquals(5, tunnelBuffer.data().getInt());
                assertEquals(7, tunnelBuffer.data().getInt());

                assertEquals(5, destBuffer.getInt());
                assertEquals(7, destBuffer.getInt());
                ++_msgCount;
            }
            else
            {
                /* Second buffer is a message. */
                assertEquals(1, _msgCount);
                assertEquals(_expectedMsgBufferLength, event.transportBuffer().length());
                decodeSampleMsg(_tunnelStream, event.transportBuffer());
                ++_msgCount;
            }

            return ReactorReturnCodes.SUCCESS;
        }
    }

    @Test
    public void tunnelStreamBufferLengthAndCopyTest()
    {
        /* Test that outbound and inbound buffers have correct lengths and can be copied. */

        TestReactorEvent event;

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
        consumerRole.watchlistOptions().enableWatchlist(_enableWatchlist);

        /* Create provider. */
        TunnelStreamBufferLengthAndCopyTest_Provider provider = new TunnelStreamBufferLengthAndCopyTest_Provider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);

        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        TunnelStream consTunnelStream = openedTsInfo.consumerTunnelStream();
        TunnelStream provTunnelStream = openedTsInfo.providerTunnelStream();
        TransportBuffer tunnelBuffer;

        provider.tunnelStream(provTunnelStream);

        assertNotNull(tunnelBuffer = consTunnelStream.getBuffer(500, _errorInfo));
        assertEquals(500, tunnelBuffer.capacity());
        assertEquals(500, tunnelBuffer.length());

        /* Write two 4-byte integers into the buffer. */
        int bufferStartPos = tunnelBuffer.data().position();
        tunnelBuffer.data().putInt(5);
        tunnelBuffer.data().putInt(7);
        assertEquals(500, tunnelBuffer.capacity());
        assertEquals(8, tunnelBuffer.length());

        /* Copy the buffer and test for correct value. */
        ByteBuffer destBuffer = ByteBuffer.allocate(8);
        assertEquals(ReactorReturnCodes.SUCCESS, tunnelBuffer.copy(destBuffer));
        assertEquals(500, tunnelBuffer.capacity());
        assertEquals(8, tunnelBuffer.length());
        destBuffer.position(0);

        tunnelBuffer.data().position(bufferStartPos);
        assertEquals(5, tunnelBuffer.data().getInt());
        assertEquals(7, tunnelBuffer.data().getInt());

        assertEquals(5, destBuffer.getInt());
        assertEquals(7, destBuffer.getInt());

        /* Test that copying to a small buffer fails. */
        destBuffer = ByteBuffer.allocate(7);
        assertEquals(ReactorReturnCodes.INVALID_USAGE, tunnelBuffer.copy(destBuffer));
        assertEquals(500, tunnelBuffer.capacity());
    	assertEquals(8, tunnelBuffer.length());    	

        /* Send this buffer to the provider. */
        _tsSubmitOpts.clear();
        _tsSubmitOpts.containerType(DataTypes.OPAQUE);
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(tunnelBuffer, _tsSubmitOpts, _errorInfo));
        consumerReactor.dispatch(0);

        /* Provider receives buffer (length & copy tests are done in the provider callback where the buffer is received;
         * the buffer we would get here is just a CopiedTransportBuffer, not the actual received buffer). */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());
        consumerReactor.dispatch(0);

        /* Now try sending a message. */
        assertNotNull(tunnelBuffer = consTunnelStream.getBuffer(500, _errorInfo));
        encodeSampleMsg(consTunnelStream, tunnelBuffer);
        provider.expectedMsgBufferLength(tunnelBuffer.length());

        _tsSubmitOpts.clear();
        _tsSubmitOpts.containerType(DataTypes.MSG);
        assertEquals(ReactorReturnCodes.SUCCESS, consTunnelStream.submit(tunnelBuffer, _tsSubmitOpts, _errorInfo));
        consumerReactor.dispatch(0);


        /* Provider receives message (and decodes it in callback). */
        providerReactor.dispatch(1);
        event = providerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_MSG, event.type());

        consumer.closeTunnelStream(provider, consTunnelStream, provTunnelStream, true);
        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }

    
    
        
        


    /* Mock channel used with the WriteCallAgainTest. Returns WRITE_CALL_AGAIN on the first write attempt*/
    class WriteCallAgainChannel implements Channel
    {
        int _state = ChannelState.ACTIVE;
        Channel consChnl;
        int writeCount;

        void setChannel(Channel channel)
        {
            consChnl = channel;
        }

        @Override
        public int info(ChannelInfo info, Error error)
        {
            return 0;
        }

        @Override
        public int ioctl(int code, Object value, Error error)
        {
            return 0;
        }

        @Override
        public int ioctl(int code, int value, Error error)
        {
            return 0;
        }

        @Override
        public int bufferUsage(Error error)
        {
            return 0;
        }

        @Override
        public int init(InProgInfo inProg, Error error)
        {
            return 0;
        }

        @Override
        public int close(Error error)
        {
            return 0;
        }

        @Override
        public TransportBuffer read(ReadArgs readArgs, Error error)
        {
            return consChnl.read(readArgs, error);
        }

        @Override
        public TransportBuffer getBuffer(int size, boolean packedBuffer, Error error)
        {
            return consChnl.getBuffer(size, packedBuffer, error);
        }

        @Override
        public int releaseBuffer(TransportBuffer buffer, Error error)
        {
            return TransportReturnCodes.SUCCESS;
        }

        @Override
        public int packBuffer(TransportBuffer buffer, Error error)
        {
            return 0;
        }

        @Override
        public int write(TransportBuffer buffer, WriteArgs writeArgs, Error error)
        {
            int retval;

            switch (writeCount)
            {
                case 0:
                    retval = TransportReturnCodes.WRITE_CALL_AGAIN;
                    break;
                default:
                    retval = consChnl.write(buffer, writeArgs, error);
                    break;
            }

            writeCount++;
            return retval;
        }

        @Override
        public int flush(Error error)
        {
            return consChnl.flush(error);
        }

        @Override
        public int ping(Error error)
        {
            return 0;
        }

        @Override
        public int majorVersion()
        {
            return 14;
        }

        @Override
        public int minorVersion()
        {
            return 0;
        }

        @Override
        public int protocolType()
        {
            return 0;
        }

        @Override
        public int state()
        {
            return _state;
        }

        @Override
        public SocketChannel scktChannel()
        {
            return null;
        }

        @Override
        public SocketChannel oldScktChannel()
        {
            return null;
        }

        @Override
        public SelectableChannel selectableChannel()
        {
            return consChnl.selectableChannel();
        }

        @Override
        public SelectableChannel oldSelectableChannel()
        {
            return null;
        }

        @Override
        public int pingTimeout()
        {
            return 0;
        }

        @Override
        public Object userSpecObject()
        {
            return null;
        }

        @Override
        public boolean blocking()
        {
            return false;
        }

        @Override
        public int reconnectClient(Error error)
        {
            return 0;
        }

        @Override
        public int connectionType()
        {
            return 0;
        }

        @Override
        public String hostname() {
            // TODO Auto-generated method stub
            return null;
        }
    }


    
    @Test
    public void tunnelStreamResponseTimeoutTest()
    {   

        /* Test request timeout on a TunnelStream.
         * 1) Open a tunnel stream from consumer, with no response from provider.
         *    Should get the status event at the appropriate time. Close the tunnel stream.
         * 2) Open another tunnel stream, which provider accepts.
         *    Wait a second, then close the tunnel stream and open it again.
         *    Should get a status event at the appropriate time. */

        ReadEvent readEvent;
        Msg provMsg = CodecFactory.createMsg();
        RequestMsg recvRequestMsg;
        TestReactorEvent event;
        int provTunnelStreamId;
        TunnelStreamStatusEvent tsStatusEvent;
        int responseTimeout = 2;
        long timeOfOpenMs, timeoutDeviationMs;
        long allowedTimeoutDeviationMs = 300;
        TunnelStream consTunnelStream;

        /* Create reactor, consumer, and provider. */
        TestReactor consumerReactor = new TestReactor();
        Consumer consumer = new Consumer(consumerReactor);
        TunnelStreamCoreProvider provider = new TunnelStreamCoreProvider();
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();

        CoreComponent.openSession(consumer, provider, opts, false);

        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     
        tsOpenOpts.responseTimeout(responseTimeout);

        assertEquals(ReactorReturnCodes.SUCCESS, consumer.reactorChannel().openTunnelStream(tsOpenOpts, _errorInfo));
        timeOfOpenMs = System.currentTimeMillis();
        consumerReactor.dispatch(0);


        /* Provider receives request (but does nothing with it). */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        provMsg = readEvent.msg();
        assertEquals(MsgClasses.REQUEST, provMsg.msgClass());
        recvRequestMsg = (RequestMsg)provMsg;
        assertEquals(tsOpenOpts.domainType(), recvRequestMsg.domainType());
        assertTrue(recvRequestMsg.checkStreaming());
        assertTrue(recvRequestMsg.checkPrivateStream());
        assertTrue(recvRequestMsg.checkQualifiedStream());;
        assertTrue(recvRequestMsg.msgKey().checkHasFilter());
        assertTrue(recvRequestMsg.msgKey().checkHasServiceId());
        assertEquals(DataTypes.FILTER_LIST, recvRequestMsg.containerType());
        provTunnelStreamId = recvRequestMsg.streamId();

        /* Consumer receives timeout event (dispatch stops when event is received). */
        consumerReactor.dispatch(1, responseTimeout * 1000 + 5000);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(StreamStates.CLOSED_RECOVER, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());

        /* Check that status arrived at around the appropriate time. */
        timeoutDeviationMs = System.currentTimeMillis() - (timeOfOpenMs + responseTimeout * 1000);
        assertTrue( "TunnelStream response timeout was off by " + timeoutDeviationMs + "ms.", abs(timeoutDeviationMs) < allowedTimeoutDeviationMs);

        assertEquals(ReactorReturnCodes.SUCCESS, tsStatusEvent.tunnelStream().close(false, _errorInfo));

        /* Open another tunnel stream. */
        assertEquals(ReactorReturnCodes.SUCCESS, consumer.reactorChannel().openTunnelStream(tsOpenOpts, _errorInfo));
        consumerReactor.dispatch(0);

        /* Provider accepts this one. */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        provMsg = readEvent.msg();
        assertEquals(MsgClasses.REQUEST, provMsg.msgClass());
        recvRequestMsg = (RequestMsg)provMsg;
        assertEquals(tsOpenOpts.domainType(), recvRequestMsg.domainType());
        assertTrue(recvRequestMsg.checkStreaming());
        assertTrue(recvRequestMsg.checkPrivateStream());
        assertTrue(recvRequestMsg.checkQualifiedStream());;
        assertTrue(recvRequestMsg.msgKey().checkHasFilter());
        assertTrue(recvRequestMsg.msgKey().checkHasServiceId());
        assertEquals(DataTypes.FILTER_LIST, recvRequestMsg.containerType());
        provTunnelStreamId = recvRequestMsg.streamId();
        provider.acceptTunnelStreamRequest(consumer, recvRequestMsg, null);

        /* Consumer receives response. */
        consumerReactor.dispatch(1);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(StreamStates.OPEN, tsStatusEvent.state().streamState());
        assertEquals(DataStates.OK, tsStatusEvent.state().dataState());
        consTunnelStream = tsStatusEvent.tunnelStream();

        /* Wait one second, then close the stream. */
        consumerReactor.dispatch(0, 1000);
        consumer.closeTunnelStream(provider, consTunnelStream, provTunnelStreamId, 1, true);

        /* Open another tunnel stream. */
        assertEquals(ReactorReturnCodes.SUCCESS, consumer.reactorChannel().openTunnelStream(tsOpenOpts, _errorInfo));
        timeOfOpenMs = System.currentTimeMillis();
        consumerReactor.dispatch(0);

        /* Provider receives request (but does nothing with it). */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        provMsg = readEvent.msg();
        assertEquals(MsgClasses.REQUEST, provMsg.msgClass());
        recvRequestMsg = (RequestMsg)provMsg;
        assertEquals(tsOpenOpts.domainType(), recvRequestMsg.domainType());
        assertTrue(recvRequestMsg.checkStreaming());
        assertTrue(recvRequestMsg.checkPrivateStream());
        assertTrue(recvRequestMsg.checkQualifiedStream());;
        assertTrue(recvRequestMsg.msgKey().checkHasFilter());
        assertTrue(recvRequestMsg.msgKey().checkHasServiceId());
        assertEquals(DataTypes.FILTER_LIST, recvRequestMsg.containerType());
        provTunnelStreamId = recvRequestMsg.streamId();

        /* Consumer receives timeout event (dispatch stops when event is received). */
        consumerReactor.dispatch(1, responseTimeout * 1000 + 5000);
        event = consumerReactor.pollEvent();
        assertEquals(TestReactorEventTypes.TUNNEL_STREAM_STATUS, event.type());
        tsStatusEvent = (TunnelStreamStatusEvent)event.reactorEvent();
        assertEquals(StreamStates.CLOSED_RECOVER, tsStatusEvent.state().streamState());
        assertEquals(DataStates.SUSPECT, tsStatusEvent.state().dataState());

        /* Check that status arrived at around the appropriate time. */
        timeoutDeviationMs = System.currentTimeMillis() - (timeOfOpenMs + responseTimeout * 1000);
        assertTrue( "TunnelStream response timeout was off by " + timeoutDeviationMs + "ms.", abs(timeoutDeviationMs) < allowedTimeoutDeviationMs);

        assertEquals(ReactorReturnCodes.SUCCESS, tsStatusEvent.tunnelStream().close(false, _errorInfo));
        provider.close();
        consumer.close();
        consumerReactor.close();
    }


    

    @Test
    public void tunnelStreamOpenWhileDisconnectedTest()
    {
        /* Make sure a TunnelStream cannot be opened while the channel is down. */

        TestReactorEvent event;

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
        consumerRole.watchlistOptions().enableWatchlist(_enableWatchlist);


        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        opts.reconnectAttemptLimit(-1);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);

        /* Disconnect channel. */
        provider.close();

        /* Consumer receives disconnection event. */
        consumerReactor.dispatch(1 + (_enableWatchlist ? 2 : 0));

        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        ReactorChannelEvent channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_DOWN_RECONNECTING, channelEvent.eventType());

        if (_enableWatchlist)
        {
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
        }

        /* Open a TunnelStream. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name("Tunnel1");
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     
        assertEquals(ReactorReturnCodes.FAILURE, consumer.reactorChannel().openTunnelStream(tsOpenOpts, _errorInfo));

        consumer.close();
        providerReactor.close();
        consumerReactor.close();
    }

    @Test
    public void tunnelStreamLongNameTest()
    {
        /* Test using a couple long TunnelStream names -- 255 is the maximum possible name. */

        TunnelStream consTunnelStream;
        TunnelStream provTunnelStream;

        /* This name is 256 characters long. Opening a TunnelStream with this name should fail. */
        String tsName256 = "5555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555";

        /* This name is 255 characters long. Opening a TunnelStream with this name should succeed. */
        String tsName255 = "555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555555";

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
        consumerRole.watchlistOptions().enableWatchlist(_enableWatchlist);

        /* Create provider. */
        TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
        ProviderRole providerRole = (ProviderRole)provider.reactorRole();
        providerRole.channelEventCallback(provider);
        providerRole.loginMsgCallback(provider);
        providerRole.directoryMsgCallback(provider);
        providerRole.dictionaryMsgCallback(provider);
        providerRole.defaultMsgCallback(provider);
        providerRole.tunnelStreamListenerCallback(provider);

        /* Connect the consumer and provider. Setup login & directory streams automatically. */
        ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
        provider.bind(opts);
        TestReactor.openSession(consumer, provider, opts);

        /* Negative test -- open a TunnelStream with a 256-character name. This should fail. */
        TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
        tsOpenOpts.name(tsName256);
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     
        assertEquals(ReactorReturnCodes.FAILURE, consumer.reactorChannel().openTunnelStream(tsOpenOpts, _errorInfo));

        /* Open a TunnelStream with a  255-character name. This should succeed. */
        tsOpenOpts.clear();
        tsOpenOpts.name(tsName255);
        tsOpenOpts.statusEventCallback(consumer);
        tsOpenOpts.queueMsgCallback(consumer);
        tsOpenOpts.defaultMsgCallback(consumer);
        tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
        tsOpenOpts.streamId(5);
        tsOpenOpts.serviceId(Provider.defaultService().serviceId());
        tsOpenOpts.domainType(DomainTypes.SYSTEM);
        tsOpenOpts.userSpecObject(consumer);     

        Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
        consTunnelStream = openedTsInfo.consumerTunnelStream();
        provTunnelStream = openedTsInfo.providerTunnelStream();

        assertEquals(5, consTunnelStream.streamId());
        if (!_enableWatchlist) /* Watchlist will likely use different stream ID. */
            assertEquals(5, provTunnelStream.streamId());
        assertEquals(tsName255, consTunnelStream.name());
        assertEquals(tsName255, provTunnelStream.name());

        TestReactorComponent.closeSession(consumer, provider);
        consumerReactor.close();
        providerReactor.close();
    }

    @Test
    public void tunnelStreamMaxMsgSizeTest()
    {
        /* Test getting buffers with different maxMsgSizes. */
        for (int maxMsgSize: new int[]{1000, 6144, 12000})
        {
            TunnelStream consTunnelStream;
            TunnelStream provTunnelStream;

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
            consumerRole.watchlistOptions().enableWatchlist(_enableWatchlist);

            /* Create provider. */
            TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
            provider.maxMsgSize(maxMsgSize);
            ProviderRole providerRole = (ProviderRole)provider.reactorRole();
            providerRole.channelEventCallback(provider);
            providerRole.loginMsgCallback(provider);
            providerRole.directoryMsgCallback(provider);
            providerRole.dictionaryMsgCallback(provider);
            providerRole.defaultMsgCallback(provider);
            providerRole.tunnelStreamListenerCallback(provider);

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.setupDefaultLoginStream(true);
            opts.setupDefaultDirectoryStream(true);
            provider.bind(opts);
            TestReactor.openSession(consumer, provider, opts);

            /* Open a TunnelStream. */
            TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
            tsOpenOpts.name("Tunnel1");
            tsOpenOpts.statusEventCallback(consumer);
            tsOpenOpts.queueMsgCallback(consumer);
            tsOpenOpts.defaultMsgCallback(consumer);
            tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
            tsOpenOpts.streamId(5);
            tsOpenOpts.serviceId(Provider.defaultService().serviceId());
            tsOpenOpts.domainType(DomainTypes.SYSTEM);
            tsOpenOpts.userSpecObject(consumer);     

            Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
            consTunnelStream = openedTsInfo.consumerTunnelStream();
            provTunnelStream = openedTsInfo.providerTunnelStream();

            /* Test tunnel stream accessors */
            assertEquals(5, consTunnelStream.streamId());
            if (!_enableWatchlist) /* Watchlist will likely use different stream ID. */
                assertEquals(5, provTunnelStream.streamId());
            assertEquals(DomainTypes.SYSTEM, consTunnelStream.domainType());
            assertEquals(DomainTypes.SYSTEM, provTunnelStream.domainType());
            assertEquals(Provider.defaultService().serviceId(), consTunnelStream.serviceId());
            assertEquals(Provider.defaultService().serviceId(), provTunnelStream.serviceId());
            assertEquals(consumer.reactorChannel(), consTunnelStream.reactorChannel());
            assertEquals(provider.reactorChannel(), provTunnelStream.reactorChannel());
            assertEquals(false, consTunnelStream.isProvider());
            assertEquals(true, provTunnelStream.isProvider());
            assertEquals("Tunnel1", consTunnelStream.name());
            assertEquals("Tunnel1", provTunnelStream.name());
            assertEquals(consumer, consTunnelStream.userSpecObject());
            assertEquals(null, provTunnelStream.userSpecObject());
            assertEquals(StreamStates.OPEN, consTunnelStream.state().streamState());
            assertEquals(DataStates.OK, consTunnelStream.state().dataState());
            assertEquals(StateCodes.NONE, consTunnelStream.state().code());
            assertEquals(StreamStates.OPEN, provTunnelStream.state().streamState());
            assertEquals(DataStates.OK, provTunnelStream.state().dataState());
            assertEquals(StateCodes.NONE, provTunnelStream.state().code());

            /* Test consumer tunnel stream class of service values */
            assertEquals(Codec.protocolType(), consTunnelStream.classOfService().common().protocolType());
            assertEquals(Codec.majorVersion(), consTunnelStream.classOfService().common().protocolMajorVersion());
            assertEquals(Codec.minorVersion(), consTunnelStream.classOfService().common().protocolMinorVersion());
            assertEquals(maxMsgSize, consTunnelStream.classOfService().common().maxMsgSize());
            assertEquals(AuthenticationTypes.NOT_REQUIRED, consTunnelStream.classOfService().authentication().type());
            assertEquals(FlowControlTypes.BIDIRECTIONAL, consTunnelStream.classOfService().flowControl().type());
            assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, consTunnelStream.classOfService().flowControl().sendWindowSize());
            assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, consTunnelStream.classOfService().flowControl().recvWindowSize());
            assertEquals(DataIntegrityTypes.RELIABLE, consTunnelStream.classOfService().dataIntegrity().type());
            assertEquals(GuaranteeTypes.NONE, consTunnelStream.classOfService().guarantee().type());

            /* Test provider tunnel stream class of service values */
            assertEquals(Codec.protocolType(), provTunnelStream.classOfService().common().protocolType());
            assertEquals(Codec.majorVersion(), provTunnelStream.classOfService().common().protocolMajorVersion());
            assertEquals(Codec.minorVersion(), provTunnelStream.classOfService().common().protocolMinorVersion());
            assertEquals(maxMsgSize, provTunnelStream.classOfService().common().maxMsgSize());
            assertEquals(AuthenticationTypes.NOT_REQUIRED, provTunnelStream.classOfService().authentication().type());
            assertEquals(FlowControlTypes.BIDIRECTIONAL, provTunnelStream.classOfService().flowControl().type());
            assertEquals(provTunnelStream.classOfService().common().maxFragmentSize(), provTunnelStream.classOfService().flowControl().sendWindowSize());
            assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, provTunnelStream.classOfService().flowControl().recvWindowSize());
            assertEquals(DataIntegrityTypes.RELIABLE, provTunnelStream.classOfService().dataIntegrity().type());
            assertEquals(GuaranteeTypes.NONE, provTunnelStream.classOfService().guarantee().type());

            /* Consumer gets a tunnelstream buffer. This should fail. */
            assertNull(consTunnelStream.getBuffer(maxMsgSize + 1, _errorInfo));

            /* Provider gets a tunnelstream buffer. This should fail. */
            assertNull(provTunnelStream.getBuffer(maxMsgSize + 1, _errorInfo));

            /* Consumer gets a tunnelstream buffer. This should succeed. */
            assertNotNull(consTunnelStream.getBuffer(maxMsgSize, _errorInfo));

            /* Provider gets a tunnelstream buffer. This should succeed. */
            assertNotNull(provTunnelStream.getBuffer(maxMsgSize, _errorInfo));

            /* Close the tunnelstreams. */
            provTunnelStream.close(false, _errorInfo);
            consTunnelStream.close(false, _errorInfo);

            TestReactorComponent.closeSession(consumer, provider);
            consumerReactor.close();
            providerReactor.close();
        }
    }

    @Test
    public void tunnelStreamInfoErrorsTest(){
        TestReactor testReactor = new TestReactor();
        ReactorChannel reactorChannel = mock(ReactorChannel.class, CALLS_REAL_METHODS);
        when(reactorChannel.reactor()).thenReturn(testReactor._reactor);
        TunnelStream tunnelStreamMock = new TunnelStream(reactorChannel);

        try {
            tunnelStreamMock.info(null, null);
        }catch (AssertionError ex){
            assertTrue(ex.getMessage().equals("errorInfo cannot be null"));
        }

        assertEquals(tunnelStreamMock.info(null, _errorInfo), ReactorReturnCodes.INVALID_USAGE);
        assertEquals(_errorInfo.error().text(), "tunnelStreamInfo cannot be null");
        assertEquals(_errorInfo.location(), "TunnelStream.info");

        TunnelStreamInfo customSreamInfo = new TunnelStreamInfo() {
            @Override
            public int buffersUsed() {
                return 0;
            }

            @Override
            public int ordinaryBuffersUsed() {
                return 0;
            }

            @Override
            public int bigBuffersUsed() {
                return 0;
            }

            @Override
            public void clear() {

            }
        };

        assertEquals(tunnelStreamMock.info(customSreamInfo, _errorInfo), ReactorReturnCodes.INVALID_USAGE);
        assertEquals(_errorInfo.error().text(), "invalid tunnelStreamInfo parameter type");

    }

    @Test
    public void tunnelStreamBuffersUsedTest()
    {
        /* Test getting buffers with different maxMsgSizes. */
        for (int maxMsgSize: new int[]{1000, 6144, 12000})
        {
            TunnelStream consTunnelStream;
            TunnelStream provTunnelStream;

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
            consumerRole.watchlistOptions().enableWatchlist(_enableWatchlist);

            /* Create provider. */
            TunnelStreamProvider provider = new TunnelStreamProvider(providerReactor);
            provider.maxMsgSize(maxMsgSize);
            ProviderRole providerRole = (ProviderRole)provider.reactorRole();
            providerRole.channelEventCallback(provider);
            providerRole.loginMsgCallback(provider);
            providerRole.directoryMsgCallback(provider);
            providerRole.dictionaryMsgCallback(provider);
            providerRole.defaultMsgCallback(provider);
            providerRole.tunnelStreamListenerCallback(provider);

            /* Connect the consumer and provider. Setup login & directory streams automatically. */
            ConsumerProviderSessionOptions opts = new ConsumerProviderSessionOptions();
            opts.setupDefaultLoginStream(true);
            opts.setupDefaultDirectoryStream(true);
            provider.bind(opts);
            TestReactor.openSession(consumer, provider, opts);

            /* Open a TunnelStream. */
            TunnelStreamOpenOptions tsOpenOpts = ReactorFactory.createTunnelStreamOpenOptions();
            tsOpenOpts.name("Tunnel1");
            tsOpenOpts.statusEventCallback(consumer);
            tsOpenOpts.queueMsgCallback(consumer);
            tsOpenOpts.defaultMsgCallback(consumer);
            tsOpenOpts.classOfService().dataIntegrity().type(DataIntegrityTypes.RELIABLE);
            tsOpenOpts.streamId(5);
            tsOpenOpts.serviceId(Provider.defaultService().serviceId());
            tsOpenOpts.domainType(DomainTypes.SYSTEM);
            tsOpenOpts.userSpecObject(consumer);

            Consumer.OpenedTunnelStreamInfo openedTsInfo = consumer.openTunnelStream(provider, tsOpenOpts);
            consTunnelStream = openedTsInfo.consumerTunnelStream();
            provTunnelStream = openedTsInfo.providerTunnelStream();

            /* Test tunnel stream accessors */
            assertEquals(5, consTunnelStream.streamId());
            if (!_enableWatchlist) /* Watchlist will likely use different stream ID. */
                assertEquals(5, provTunnelStream.streamId());
            assertEquals(DomainTypes.SYSTEM, consTunnelStream.domainType());
            assertEquals(DomainTypes.SYSTEM, provTunnelStream.domainType());
            assertEquals(Provider.defaultService().serviceId(), consTunnelStream.serviceId());
            assertEquals(Provider.defaultService().serviceId(), provTunnelStream.serviceId());
            assertEquals(consumer.reactorChannel(), consTunnelStream.reactorChannel());
            assertEquals(provider.reactorChannel(), provTunnelStream.reactorChannel());
            assertEquals(false, consTunnelStream.isProvider());
            assertEquals(true, provTunnelStream.isProvider());
            assertEquals("Tunnel1", consTunnelStream.name());
            assertEquals("Tunnel1", provTunnelStream.name());
            assertEquals(consumer, consTunnelStream.userSpecObject());
            assertEquals(null, provTunnelStream.userSpecObject());
            assertEquals(StreamStates.OPEN, consTunnelStream.state().streamState());
            assertEquals(DataStates.OK, consTunnelStream.state().dataState());
            assertEquals(StateCodes.NONE, consTunnelStream.state().code());
            assertEquals(StreamStates.OPEN, provTunnelStream.state().streamState());
            assertEquals(DataStates.OK, provTunnelStream.state().dataState());
            assertEquals(StateCodes.NONE, provTunnelStream.state().code());

            /* Test consumer tunnel stream class of service values */
            assertEquals(Codec.protocolType(), consTunnelStream.classOfService().common().protocolType());
            assertEquals(Codec.majorVersion(), consTunnelStream.classOfService().common().protocolMajorVersion());
            assertEquals(Codec.minorVersion(), consTunnelStream.classOfService().common().protocolMinorVersion());
            assertEquals(maxMsgSize, consTunnelStream.classOfService().common().maxMsgSize());
            assertEquals(AuthenticationTypes.NOT_REQUIRED, consTunnelStream.classOfService().authentication().type());
            assertEquals(FlowControlTypes.BIDIRECTIONAL, consTunnelStream.classOfService().flowControl().type());
            assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, consTunnelStream.classOfService().flowControl().sendWindowSize());
            assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, consTunnelStream.classOfService().flowControl().recvWindowSize());
            assertEquals(DataIntegrityTypes.RELIABLE, consTunnelStream.classOfService().dataIntegrity().type());
            assertEquals(GuaranteeTypes.NONE, consTunnelStream.classOfService().guarantee().type());

            /* Test provider tunnel stream class of service values */
            assertEquals(Codec.protocolType(), provTunnelStream.classOfService().common().protocolType());
            assertEquals(Codec.majorVersion(), provTunnelStream.classOfService().common().protocolMajorVersion());
            assertEquals(Codec.minorVersion(), provTunnelStream.classOfService().common().protocolMinorVersion());
            assertEquals(maxMsgSize, provTunnelStream.classOfService().common().maxMsgSize());
            assertEquals(AuthenticationTypes.NOT_REQUIRED, provTunnelStream.classOfService().authentication().type());
            assertEquals(FlowControlTypes.BIDIRECTIONAL, provTunnelStream.classOfService().flowControl().type());
            assertEquals(provTunnelStream.classOfService().common().maxFragmentSize(), provTunnelStream.classOfService().flowControl().sendWindowSize());
            assertEquals(TunnelStream.DEFAULT_RECV_WINDOW, provTunnelStream.classOfService().flowControl().recvWindowSize());
            assertEquals(DataIntegrityTypes.RELIABLE, provTunnelStream.classOfService().dataIntegrity().type());
            assertEquals(GuaranteeTypes.NONE, provTunnelStream.classOfService().guarantee().type());

            TunnelStreamInfo tunnelStreamInfo = ReactorFactory.createTunnelStreamInfo();
            TunnelStreamInfo zeroBuffersUsedInfo = ReactorFactory.createTunnelStreamInfo();
            /* Consumer: check initial buffer used count before allocation, should be zero */
            consTunnelStream.info(tunnelStreamInfo, _errorInfo);
            assertEquals(zeroBuffersUsedInfo, tunnelStreamInfo);
            /* Consumer gets a tunnelstream buffer. This should fail. */
            assertNull(consTunnelStream.getBuffer(maxMsgSize + 1, _errorInfo));
            /* used buffers still zero */
            consTunnelStream.info(tunnelStreamInfo, _errorInfo);
            assertEquals(zeroBuffersUsedInfo, tunnelStreamInfo);

            /* Provider: same buffer used count checks, zero before allocation */
            provTunnelStream.info(tunnelStreamInfo, _errorInfo);
            assertEquals(zeroBuffersUsedInfo, tunnelStreamInfo);
            /* Provider gets a tunnelstream buffer. This should fail. */
            assertNull(provTunnelStream.getBuffer(maxMsgSize + 1, _errorInfo));
            /* zero remains after failed attempt */
            provTunnelStream.info(tunnelStreamInfo, _errorInfo);
            assertEquals(zeroBuffersUsedInfo, tunnelStreamInfo);

            // store allocated buffers so we can release them afterwards
            int desiredGetBufferTestCount = 5;
            TransportBuffer []consBuffers = new TransportBuffer[desiredGetBufferTestCount + 1];
            TransportBuffer []provBuffers = new TransportBuffer[desiredGetBufferTestCount + 1];
            TunnelStreamInfo expectedTunnelStreamInfo = null;

            // request buffer several times
            for (int i=1; i<=desiredGetBufferTestCount; i++) {
                //get consumer buffer
                assertNotNull((consBuffers[i]=consTunnelStream.getBuffer(maxMsgSize, _errorInfo)));
                assertEquals(consTunnelStream.info(tunnelStreamInfo, _errorInfo), ReactorReturnCodes.SUCCESS);
                expectedTunnelStreamInfo = getExpectedUsedBuffers(maxMsgSize, i);
                assertEquals(expectedTunnelStreamInfo, tunnelStreamInfo);

                //same for provider
                assertNotNull((provBuffers[i]=provTunnelStream.getBuffer(maxMsgSize, _errorInfo)));
                assertEquals(provTunnelStream.info(tunnelStreamInfo, _errorInfo), ReactorReturnCodes.SUCCESS);
                assertEquals(expectedTunnelStreamInfo, tunnelStreamInfo);
            }

            // release previously requested buffers
            for (int i=desiredGetBufferTestCount; i>=1; i--) {
                //release consumer buffer
                assertEquals(consTunnelStream.releaseBuffer(consBuffers[i], _errorInfo), ReactorReturnCodes.SUCCESS);
                assertEquals(consTunnelStream.info(tunnelStreamInfo, _errorInfo), ReactorReturnCodes.SUCCESS);
                expectedTunnelStreamInfo = getExpectedUsedBuffers(maxMsgSize, i-1);
                assertEquals(expectedTunnelStreamInfo, tunnelStreamInfo);

                //same for provider
                assertEquals(provTunnelStream.releaseBuffer(provBuffers[i], _errorInfo), ReactorReturnCodes.SUCCESS);
                assertEquals(provTunnelStream.info(tunnelStreamInfo, _errorInfo), ReactorReturnCodes.SUCCESS);
                assertEquals(expectedTunnelStreamInfo, tunnelStreamInfo);
            }

            /* as far as API uses internal and user counters for sliced buffer
               requesting buffers once again to check UsedBuffer tunnelStreamInfo is still relevant
             */
            for (int i=1; i<=desiredGetBufferTestCount; i++) {
                //get consumer buffer
                assertNotNull((consBuffers[i]=consTunnelStream.getBuffer(maxMsgSize, _errorInfo)));
                assertEquals(consTunnelStream.info(tunnelStreamInfo, _errorInfo), ReactorReturnCodes.SUCCESS);
                expectedTunnelStreamInfo = getExpectedUsedBuffers(maxMsgSize, i);
                assertEquals(expectedTunnelStreamInfo, tunnelStreamInfo);

                //same for provider
                assertNotNull((provBuffers[i]=provTunnelStream.getBuffer(maxMsgSize, _errorInfo)));
                assertEquals(provTunnelStream.info(tunnelStreamInfo, _errorInfo), ReactorReturnCodes.SUCCESS);
                assertEquals(expectedTunnelStreamInfo, tunnelStreamInfo);
            }

            /* Close the tunnelstreams. */
            provTunnelStream.close(false, _errorInfo);
            consTunnelStream.close(false, _errorInfo);

            TestReactorComponent.closeSession(consumer, provider);
            consumerReactor.close();
            providerReactor.close();
        }
    }

    /* get expected buffer used count assuming same-sized buffers are being requested each time */
    TunnelStreamInfo getExpectedUsedBuffers(int unifiedMsgSize, int bufferNo) {

        if (bufferNo==0)
            return new TunnelStreamInfoImpl(0, 0);

        if (unifiedMsgSize <= CosCommon.DEFAULT_MAX_FRAGMENT_SIZE) {
            /* for small buffers API allocates big buffer, so getting number of userBuffers which slicedBuffer can hold*/
            int sliceBufferSize = CosCommon.DEFAULT_MAX_FRAGMENT_SIZE + TUNNEL_STREAM_HDR_SIZE;
            int requiredSpaceForUserBuffer = unifiedMsgSize + TUNNEL_STREAM_HDR_SIZE;
            int smallBuffersCountPerSliceBuffer = sliceBufferSize / requiredSpaceForUserBuffer;
            return new TunnelStreamInfoImpl( 1 + (bufferNo-1) / smallBuffersCountPerSliceBuffer, 0);
        } else {
            return new TunnelStreamInfoImpl(0, bufferNo);
        }
    }

}

