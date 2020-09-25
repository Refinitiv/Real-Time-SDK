///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.eta.valueadd.reactor;

import static org.junit.Assert.assertEquals;
import static org.junit.Assert.assertNotNull;
import static org.junit.Assert.assertTrue;
import static org.junit.Assert.fail;

import java.io.IOException;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.Queue;
import java.util.Set;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.DecodeIterator;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.codec.StateCodes;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.transport.AcceptOptions;
import com.refinitiv.eta.transport.BindOptions;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelInfo;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.InProgFlags;
import com.refinitiv.eta.transport.InProgInfo;
import com.refinitiv.eta.transport.ReadArgs;
import com.refinitiv.eta.transport.Server;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WriteArgs;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.directory.DirectoryRequest;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsg;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgFactory;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginMsgType;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRefresh;
import com.refinitiv.eta.valueadd.domainrep.rdm.login.LoginRequest;

/** A component that uses UPA directly, instead of a Reactor. */
public class CoreComponent
{
    Channel _channel;
    Selector _selector;
    Error _error;
    Server _server;
	ReadArgs _readArgs;
    InProgInfo _inProg;
	Queue<ReadEvent> _eventQueue;
	int _maxFragmentSize;
	EncodeIterator _eIter;
	DecodeIterator _dIter;
	WriteArgs _writeArgs;
	
    int _defaultSessionLoginStreamId;
    boolean _defaultSessionLoginStreamIdIsSet;
    int _defaultSessionDirectoryStreamId;
    boolean _defaultSessionDirectoryStreamIdIsSet;
    
    static int _portToBind = 17123; /** A port to use when binding servers. Incremented with each bind. */
    
    public CoreComponent()
    {
		_readArgs = TransportFactory.createReadArgs();
        _error = TransportFactory.createError();
        _inProg = TransportFactory.createInProgInfo();
        _eventQueue = new LinkedList<ReadEvent>();
        _eIter = CodecFactory.createEncodeIterator();
        _dIter = CodecFactory.createDecodeIterator();
        _writeArgs = TransportFactory.createWriteArgs();
        try {
            _selector = Selector.open();
        } catch (IOException e) {
            e.printStackTrace();
            fail("Caught I/O exception");
        }
    }
    
    /** Returns component's server, if any. */
    public Server server()
    {
        return _server;
    }

    /** Returns the port of the component's server. */
    public int serverPort()
    {
        assertNotNull(_server);
        return _server.portNumber();
    }

    /** Closes the component's channel. */
    public void closeChannel()
    {
        if (_channel != null)
        {
            assertEquals(TransportReturnCodes.SUCCESS, _channel.close(_error));
            _channel = null;
        }
    }
    
	/** Closes a component. */
    public void close()
    {
        closeChannel();

        if (_server != null)
        {
            assertEquals(TransportReturnCodes.SUCCESS, _server.close(_error));
            _server = null;
        }
    }
    
    
    /** Sets up a server. */
    public void bind(ConsumerProviderSessionOptions opts)
    {
        if (opts.connectionType() != ConnectionTypes.RELIABLE_MCAST)
        {
            BindOptions bindOpts = TransportFactory.createBindOptions();
            bindOpts.clear();
            bindOpts.majorVersion(Codec.majorVersion());
            bindOpts.minorVersion(Codec.minorVersion());
            bindOpts.pingTimeout(opts.pingTimeout());
            bindOpts.minPingTimeout(opts.pingTimeout());
            /* allow multiple tests to run at the same time, if the port is in use it might mean that
            another parallel test is using this port, so just try to get another port */
            while (_server == null)
            {
                bindOpts.serviceName(String.valueOf(_portToBind++));
                _server = Transport.bind(bindOpts, _error);
            }
        }

    }

    /** Accepts a channel. Used by acceptAndInitChannel. Can be run alone to
     * avoid initializing a channel. */
    void accept(int connectionType)
    {
        if (connectionType != ConnectionTypes.RELIABLE_MCAST)
        {
            AcceptOptions acceptOpts = TransportFactory.createAcceptOptions();
            int selRet;

            // make sure the connect has triggered the bind socket before calling accept();
            assertNotNull(_server);
            try
            {
                _server.selectableChannel().register(_selector, SelectionKey.OP_ACCEPT, _server);
                selRet = _selector.select(5000);
            }
            catch (IOException e)
            {
                e.printStackTrace();
                fail("Caught IOException.");
                return;
            }
            assertTrue("No accept notification", selRet > 0);
            Set<SelectionKey> keySet = null;
            keySet = _selector.selectedKeys();
            Iterator<SelectionKey> iter = keySet.iterator();
            while (iter.hasNext())
            {
                SelectionKey key = iter.next();
                iter.remove();
                assertTrue(key.isValid());

                if (key.isAcceptable())
                {
                    acceptOpts.clear();

                    if ((_channel = _server.accept(acceptOpts, _error)) == null)
                    {
                        System.out.printf("rsslAccept() failed: %d(%s)\n", _error.errorId(), _error.text());
                        fail();
                    }
                }
            }
        }
        else
        {
            ConnectOptions connectOpts = TransportFactory.createConnectOptions();

            connectOpts.clear();
            connectOpts.segmentedNetworkInfo().recvAddress("235.1.1.1");
            connectOpts.segmentedNetworkInfo().recvServiceName("15011");
            connectOpts.segmentedNetworkInfo().sendAddress("235.1.1.1");
            connectOpts.segmentedNetworkInfo().sendServiceName("15011");
            connectOpts.segmentedNetworkInfo().unicastServiceName("16011");
            connectOpts.segmentedNetworkInfo().interfaceName("localhost");
            connectOpts.connectionType(ConnectionTypes.RELIABLE_MCAST);

            if ((_channel = Transport.connect(connectOpts, _error)) == null)
            {
                System.out.printf("rsslConnect() failed: %d(%s)\n", _error.errorId(), _error.text());
                fail();
            }

            // register channel for read
            try
            {
                _channel.selectableChannel().register(_selector, SelectionKey.OP_READ, _channel);
            }
            catch (ClosedChannelException e)
            {
                e.printStackTrace();
                fail("Caught ClosedChannelException.");
            }
        }
    }
    
    /** Accepts and initializes a Channel.
     * This is intended to be run against a Reactor-based client, where initialization
     * is performed by an internal thread, so we shouldn't need to do anything with the
     * other side to get an active channel. */
    void acceptAndInitChannel(int connectionType)
    {
        try
        {
            accept(connectionType);

            // Initialize channel.
            _channel.selectableChannel().register(_selector, SelectionKey.OP_WRITE, _channel);
            
            do
            {
                int ret;
                
                _channel.selectableChannel().register(_selector, SelectionKey.OP_READ, _channel);
           
                ret = _selector.select(5000);
                assertTrue("No init notification", ret > 0);
                
                Set<SelectionKey> keySet = null;
                keySet = _selector.selectedKeys();
                Iterator<SelectionKey> iter = keySet.iterator();
                assertTrue(iter.hasNext());
                
                SelectionKey key = iter.next();
                iter.remove();
                assertTrue(key.isValid());
                
                if (key.isReadable() || key.isWritable())
                {
                    ret = _channel.init(_inProg, _error);
                    if (ret == TransportReturnCodes.SUCCESS)
                        break;
                    
                    assertEquals(TransportReturnCodes.CHAN_INIT_IN_PROGRESS, ret);
                    
                    /* If socket channel changes, register for write notification again. */
                    if ((_inProg.flags() & InProgFlags.SCKT_CHNL_CHANGE) != 0)
                        _channel.selectableChannel().register(_selector, SelectionKey.OP_WRITE, _channel);
                    
                }
            } while (true);
        }
        catch (Exception e)
        {
            e.printStackTrace();
            fail();
        }       
        
        assertNotNull(_channel);
        assertEquals(_channel.state(), ChannelState.ACTIVE);
        
        ChannelInfo channelInfo = TransportFactory.createChannelInfo();
        assertEquals(TransportReturnCodes.SUCCESS, _channel.info(channelInfo, _error));
    }
    
    /** Waits for notification on the component's channel, and reads when triggered. It will 
     * store any received events for later review.
     * Waiting for notification stops once the expected number of events is received (unless that number is zero, in which case it waits for the full specified time). 
     * @param expectedEventCount The exact number of events that should be received.
     * @param timeoutMsec The maximum time to wait for all events.
     */
    public void dispatch(int expectedEventCount)
    {
        long timeoutMsec;
        
        if (expectedEventCount > 0)
            timeoutMsec = 1000;
        else
            timeoutMsec = 200;
        dispatch(expectedEventCount, timeoutMsec);
    }
    
	/** Waits for notification on the component's channel, and reads when triggered. It will 
	 * store any received events for later review.
	 * Waiting for notification stops once the expected number of events is received (unless that number is zero, in which case it waits for the full specified time). 
	 * @param expectedEventCount The exact number of events that should be received.
	 * @param timeoutMsec The maximum time to wait for all events.
	 */
	public void dispatch(int expectedEventCount, long timeoutMsec)
	{
        int selectRet = 0;
        long currentTimeUsec, stopTimeUsec;
        int lastReadRet = 0;

        /* Ensure no events were missed from previous calls to dispatch. */
        assertEquals(0, _eventQueue.size());

        currentTimeUsec = System.nanoTime()/1000;
        
        stopTimeUsec = timeoutMsec;
        stopTimeUsec *= 1000;
        stopTimeUsec += currentTimeUsec;

        do
        {
            if (lastReadRet == 0)
            {
                try
				{
                    long selectTime;

					_channel.selectableChannel().register(_selector, SelectionKey.OP_READ, _channel);
	
					selectTime = (stopTimeUsec - currentTimeUsec)/1000;
					
					if (selectTime > 0)
					{
						selectRet = _selector.select(selectTime);
					}
					else
					{
						selectRet = _selector.selectNow();
					}
					
	                
	                if (selectRet > 0)
	                {
	                    assertTrue(_selector.selectedKeys().remove(_channel.selectableChannel().keyFor(_selector)));
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
                do
                {
	                _readArgs.clear();
	                TransportBuffer buffer = _channel.read(_readArgs, _error);
	                lastReadRet = _readArgs.readRetVal();
	                if (lastReadRet == TransportReturnCodes.READ_PING || lastReadRet == TransportReturnCodes.READ_WOULD_BLOCK) continue; 
	                
	                _eventQueue.add(new ReadEvent(buffer, _channel, lastReadRet));
	                
                } while (lastReadRet > 0);
            }

            currentTimeUsec = System.nanoTime()/1000;

        } while(currentTimeUsec < stopTimeUsec && (expectedEventCount == 0 || _eventQueue.size() < expectedEventCount));
        
        assertEquals(expectedEventCount, _eventQueue.size());
    }
	
	/* Submit a Codec Msg to the channel. */
	void submit(Msg msg)
    {
        submit(null, msg);
    }
    
	/* Submit a Domain Representation message to the channel. */
	void submit(MsgBase msgBase)
	{
	    submit(msgBase, null);
	}
	
	/* Helper to send a domain rep message or Codec msg to a channel. */
	private void submit(MsgBase msgBase, Msg msg)
	{
	    int ret;
	    int bufferSize = 256;
	    
	    assert (msgBase != null && msg == null || msgBase == null && msg != null);
	    
	    do
	    {
	        TransportBuffer buffer = _channel.getBuffer(bufferSize, false, _error);
    	    assertNotNull(buffer);
    	    assertEquals(TransportReturnCodes.SUCCESS, _eIter.setBufferAndRWFVersion(buffer, _channel.majorVersion(), _channel.minorVersion()));
    	    
    	    if (msgBase != null)
    	        ret = msgBase.encode(_eIter);
    	    else
    	        ret = msg.encode(_eIter);
    	    
    	    switch (ret)
    	    {
    	        case CodecReturnCodes.BUFFER_TOO_SMALL:
    	            assertEquals(TransportReturnCodes.SUCCESS, _channel.releaseBuffer(buffer, _error));
    	            bufferSize *= 2;
    	            break;
    	        default:
    	            assertEquals(CodecReturnCodes.SUCCESS, ret);
    	            writeBuffer(buffer);
    	            return;
    	    }
	    } while (true);	    
	}
	
	void writeBuffer(TransportBuffer buffer)
	{
	    int ret;
	    
	    ret = _channel.write(buffer, _writeArgs, _error);
	    assertTrue(ret >= 0 || ret == TransportReturnCodes.WRITE_FLUSH_FAILED && _channel.state() == ChannelState.ACTIVE);
	    
	    while (ret > 0)
	    {
	        try
	        {
	            _channel.selectableChannel().register(_selector, SelectionKey.OP_WRITE, _channel);
	            _selector.select(0);
	            ret = _channel.flush(_error);
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
	        
	        assertTrue(ret >= 0);
	    }
	}
	

	/** Retrieves an event from the list of events received from a dispatch call. */
    ReadEvent pollEvent()
    {
        return _eventQueue.poll();
    }
    
    /** Stores the directory stream ID for this component. Used if a directory stream is automatically setup as part of opening a session. */
    public void defaultSessionDirectoryStreamId(int defaultSessionDirectoryStreamId)
    {
        _defaultSessionDirectoryStreamId = defaultSessionDirectoryStreamId;
        _defaultSessionDirectoryStreamIdIsSet = true;
    }

    /** If a directory stream was automatically opened as part of opening a session, returns the ID of that stream for this component. */
    public int defaultSessionDirectoryStreamId()
    {
        assertTrue(_defaultSessionDirectoryStreamIdIsSet);
        return _defaultSessionDirectoryStreamId;
    }
    
	/** Opens a channel between a Reactor-Based consumer and a core provider. Sets up login and directory streams. */
    public static void openSession(Consumer consumer, CoreComponent provider, ConsumerProviderSessionOptions opts,  boolean recoveringChannel)
    {
        TestReactorEvent event;
        ReactorChannelEvent channelEvent;
        RDMLoginMsgEvent loginMsgEvent;
        RDMDirectoryMsgEvent directoryMsgEvent;
        ReadEvent readEvent;
        LoginMsg loginMsg;
        DirectoryMsg directoryMsg;
        
                
        /* Create consumer. */
        ConsumerRole consumerRole = (ConsumerRole)consumer.reactorRole();
        consumerRole.initDefaultRDMLoginRequest();
        consumerRole.initDefaultRDMDirectoryRequest();
        consumerRole.channelEventCallback(consumer);
        consumerRole.loginMsgCallback(consumer);      
        consumerRole.directoryMsgCallback(consumer);
        consumerRole.dictionaryMsgCallback(consumer);
        consumerRole.defaultMsgCallback(consumer);

        opts.setupDefaultLoginStream(true);
        opts.setupDefaultDirectoryStream(true);
     
        if (!recoveringChannel)
        {
            /* Connect the consumer and provider. */
            provider.bind(opts);
            consumer.testReactor().connect(opts, consumer, provider.serverPort());
        }
        else
            assertNotNull(provider.server());
        
        provider.acceptAndInitChannel(opts.connectionType());

        /* Consumer receives channel-up */
        consumer.testReactor().dispatch(1);
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_UP, channelEvent.eventType());
        
        /* Provider receives login request. */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        loginMsg = readEvent.loginMsg();
        assertEquals(loginMsg.rdmMsgType(), LoginMsgType.REQUEST);
        
        /* Provider sends login refresh. */
        LoginRequest loginRequest = (LoginRequest)loginMsg;
        LoginRefresh loginRefresh = (LoginRefresh)LoginMsgFactory.createMsg();

        loginRefresh.clear();
        loginRefresh.rdmMsgType(LoginMsgType.REFRESH);
        loginRefresh.applySolicited();
        loginRefresh.userName(loginRequest.userName());
        loginRefresh.streamId(loginRequest.streamId());
        loginRefresh.features().applyHasSupportOptimizedPauseResume();
        loginRefresh.features().supportOptimizedPauseResume(1);
        loginRefresh.features().applyHasSupportViewRequests();
        loginRefresh.features().supportViewRequests(1);
        loginRefresh.state().streamState(StreamStates.OPEN);
        loginRefresh.state().dataState(DataStates.OK);
        loginRefresh.state().code(StateCodes.NONE);
        loginRefresh.state().text().data("Login OK");

        provider.submit(loginRefresh);
        
        /* Consumer receives loginRefresh. */
        consumer.testReactor().dispatch(1);
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.LOGIN_MSG, event.type());
        loginMsgEvent = (RDMLoginMsgEvent)event.reactorEvent();
        assertEquals(LoginMsgType.REFRESH, loginMsgEvent.rdmLoginMsg().rdmMsgType());
        
        if (consumerRole.rdmDirectoryRequest() == null)
        {
            /* Consumer receives channel-ready. */
            event = consumer.testReactor().pollEvent();
            assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
            channelEvent = (ReactorChannelEvent)event.reactorEvent();
            assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
            
            provider.dispatch(0);

            /* If watchlist is not enabled, no directory exchange occurs. We're done. */
            if (consumerRole.watchlistOptions().enableWatchlist() == false)
                return;
        }
        
        if (opts.setupDefaultDirectoryStream() == false)
            return;

        /* Provider receives directory request. */
        provider.dispatch(1);
        readEvent = provider.pollEvent();
        directoryMsg = readEvent.directoryMsg();
        assertEquals(DirectoryMsgType.REQUEST, directoryMsg.rdmMsgType());
        
        /* Provider sends a default directory refresh. */
        DirectoryRequest directoryRequest = (DirectoryRequest)directoryMsg;
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

        directoryRefresh.serviceList().add(Provider.defaultService());
        provider.submit(directoryRefresh);

        consumer.testReactor().dispatch(2);
        
        /* Consumer receives directory refresh. */
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.DIRECTORY_MSG, event.type());
        directoryMsgEvent = (RDMDirectoryMsgEvent)event.reactorEvent();
        assertEquals((recoveringChannel && consumerRole.watchlistOptions().enableWatchlist() ? 
                DirectoryMsgType.UPDATE : DirectoryMsgType.REFRESH), 
                     directoryMsgEvent.rdmDirectoryMsg().rdmMsgType());
        
        /* Save the stream ID used by each component to open the directory stream (may be different if the watchlist is enabled). */
        consumer.defaultSessionDirectoryStreamId(consumerRole.rdmDirectoryRequest().streamId());
        provider.defaultSessionDirectoryStreamId(directoryRequest.streamId());
        
        /* Consumer receives channel-ready. */
        event = consumer.testReactor().pollEvent();
        assertEquals(TestReactorEventTypes.CHANNEL_EVENT, event.type());
        channelEvent = (ReactorChannelEvent)event.reactorEvent();
        assertEquals(ReactorChannelEventTypes.CHANNEL_READY, channelEvent.eventType());
    }
}
