/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import static org.junit.Assert.*;

import java.io.IOException;

import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.Msg;
import com.refinitiv.eta.transport.BindOptions;
import com.refinitiv.eta.transport.ConnectionTypes;
import com.refinitiv.eta.transport.Server;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.valueadd.domainrep.rdm.MsgBase;

/** A component represents a consumer, provider, etc. on the network (note the Consumer and Provider subclasses). */

public abstract class TestReactorComponent {
	
    /** Reactor channel associated with this component, if connected. */
	ReactorChannel _reactorChannel;
	
	/** Whether the reactor channel associated with this component is up. */
	boolean _reactorChannelIsUp;
	
	/** ReactorRole associated with this component. */
	ReactorRole _reactorRole;
	
	/** Server associated with this component, if any. */
	Server _server;
	
	/** Reusable ReactorErrorInfo. */
	ReactorErrorInfo _errorInfo;
	
	/** Reactor associated with this component. */
	TestReactor _testReactor;
    
    int _defaultSessionLoginStreamId;
    boolean _defaultSessionLoginStreamIdIsSet;
    int _defaultSessionDirectoryStreamId;
    boolean _defaultSessionDirectoryStreamIdIsSet;
    
    static int _portToBind = 16123; /** A port to use when binding servers. Incremented with each bind. */
    static int _portToBindForReconnectTest = 16000; /** A port to use when binding servers. Incremented with each bind. */

    /** Returns the port the next bind call will use. Useful for testing reconnection
     * to servers that won't be bound until later in a test. */
    static int nextReservedServerPort()
    {
        return _portToBindForReconnectTest;
    }

    /** Returns the port of the component's server. */
    int serverPort()
    {
        assertNotNull(_server);
        return _server.portNumber();
    }
	
	TestReactor testReactor()
	{
		return _testReactor;
	}
	
	void testReactor(TestReactor testReactor)
	{
		_testReactor = testReactor;
	}
	
	protected TestReactorComponent(TestReactor testReactor)
	{

		_errorInfo = ReactorFactory.createReactorErrorInfo();
		_testReactor = testReactor;
		_testReactor.addComponent(this);
	}
	
	public ReactorRole reactorRole()
	{
		return _reactorRole;
	}
	
	public ReactorChannel reactorChannel()
	{
		return _reactorChannel;
	}
	
	public void reactorChannel(ReactorChannel reactorChannel)
	{
		_reactorChannel = reactorChannel;
	}
	
	public boolean reactorChannelIsUp()
	{
		return _reactorChannelIsUp;
	}
	
	public void reactorChannelIsUp(boolean reactorChannelIsUp)
	{
		_reactorChannelIsUp = reactorChannelIsUp;
	}
	
	/** Stores the login stream ID for this component. Used if a login stream is automatically setup as part of opening a session. */
	public void defaultSessionLoginStreamId(int defaultSessionLoginStreamId)
	{
		_defaultSessionLoginStreamIdIsSet = true;
		_defaultSessionLoginStreamId = defaultSessionLoginStreamId;
		_defaultSessionLoginStreamIdIsSet = true;
	}
	
	/** If a login stream was automatically opened as part of opening a session, returns the ID of that stream for this component. */
	public int defaultSessionLoginStreamId()
	{
		assertTrue(_defaultSessionLoginStreamIdIsSet);
		return _defaultSessionLoginStreamId;
	}
	
	/** Stores the directory stream ID for this component. Used if a directory stream is automatically setup as part of opening a session. */
	public void defaultSessionDirectoryStreamId(int defaultSessionDirectoryStreamId)
	{
		_defaultSessionDirectoryStreamIdIsSet = true;
		_defaultSessionDirectoryStreamId = defaultSessionDirectoryStreamId;
		_defaultSessionDirectoryStreamIdIsSet = true;
	}

	/** If a directory stream was automatically opened as part of opening a session, returns the ID of that stream for this component. */
	public int defaultSessionDirectoryStreamId()
	{
		assertTrue(_defaultSessionDirectoryStreamIdIsSet);
		return _defaultSessionDirectoryStreamId;
	}
	
	Server server()
	{
		return _server;
	}
	
	public void bindForReconnectTest(ConsumerProviderSessionOptions opts)
	{
        if (opts.connectionType() != ConnectionTypes.RELIABLE_MCAST)
        {
            BindOptions bindOpts = TransportFactory.createBindOptions();
            bindOpts.clear();
            bindOpts.majorVersion(Codec.majorVersion());
            bindOpts.minorVersion(Codec.minorVersion());
            bindOpts.serviceName(String.valueOf(_portToBindForReconnectTest++));
            bindOpts.pingTimeout(opts.pingTimeout());
            bindOpts.minPingTimeout(opts.pingTimeout());
            _server = Transport.bind(bindOpts, _errorInfo.error());
            assertNotNull("bind failed: " + _errorInfo.error().errorId() + "(" + _errorInfo.error().text() + ")",
                _server);
        }

        _testReactor.registerComponentServer(this);
	}

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
			bindOpts.guaranteedOutputBuffers(opts.getNumOfGuaranteedBuffers());
			bindOpts.compressionType(opts.compressionType());

            if (opts.getProtocolList() != null)
            	bindOpts.wSocketOpts().protocols(opts.getProtocolList());
            /* allow multiple tests to run at the same time, if the port is in use it might mean that
               another parallel test is using this port, so just try to get another port */
            while (_server == null)
            {
                bindOpts.serviceName(String.valueOf(_portToBind++));
                _server = Transport.bind(bindOpts, _errorInfo.error());
                
            }
        }
        
        _testReactor.registerComponentServer(this);
	}	
	
	public void bind(ConsumerProviderSessionOptions opts, int port)
	{
        if (opts.connectionType() != ConnectionTypes.RELIABLE_MCAST)
        {
        	BindOptions bindOpts = TransportFactory.createBindOptions();
            bindOpts.clear();
            bindOpts.majorVersion(Codec.majorVersion());
            bindOpts.minorVersion(Codec.minorVersion());
            bindOpts.pingTimeout(opts.pingTimeout());
            bindOpts.minPingTimeout(opts.pingTimeout());
			bindOpts.guaranteedOutputBuffers(opts.getNumOfGuaranteedBuffers());
			bindOpts.compressionType(opts.compressionType());

            if (opts.getProtocolList() != null)
            	bindOpts.wSocketOpts().protocols(opts.getProtocolList());
            /* allow multiple tests to run at the same time, if the port is in use it might mean that
               another parallel test is using this port, so just try to get another port */
            while (_server == null)
            {
                bindOpts.serviceName(String.valueOf(port));
                _server = Transport.bind(bindOpts, _errorInfo.error());
                
            }
        }
        
        _testReactor.registerComponentServer(this);
	}	
	
	public int bindGetPort(ConsumerProviderSessionOptions opts)
	{
        if (opts.connectionType() != ConnectionTypes.RELIABLE_MCAST)
        {
        	BindOptions bindOpts = TransportFactory.createBindOptions();
            bindOpts.clear();
            bindOpts.majorVersion(Codec.majorVersion());
            bindOpts.minorVersion(Codec.minorVersion());
            bindOpts.pingTimeout(opts.pingTimeout());
            bindOpts.minPingTimeout(opts.pingTimeout());
			bindOpts.guaranteedOutputBuffers(opts.getNumOfGuaranteedBuffers());
			bindOpts.compressionType(opts.compressionType());

            if (opts.getProtocolList() != null)
            	bindOpts.wSocketOpts().protocols(opts.getProtocolList());
            /* allow multiple tests to run at the same time, if the port is in use it might mean that
               another parallel test is using this port, so just try to get another port */
            while (_server == null)
            {
                bindOpts.serviceName(String.valueOf(++_portToBind));
                _server = Transport.bind(bindOpts, _errorInfo.error());
                
            }
        }
        
        _testReactor.registerComponentServer(this);
        return _portToBind;
	}	
	
    /** Sends a Msg to the component's channel. */
    int submit(Msg msg, ReactorSubmitOptions submitOptions)
    {
    	return submit(msg, submitOptions, false);
    }

    int submit(Msg msg, ReactorSubmitOptions submitOptions, boolean expectFailure)
    {
        int ret;

        ret = _reactorChannel.submit(msg, submitOptions, _errorInfo);
        
        if (!expectFailure) {
            assertTrue("submit failed: " + ret + "(" + _errorInfo.location() + "--" + _errorInfo.error().text() + ")",
                    ret >= ReactorReturnCodes.SUCCESS);
        }

        return ret;
    }
    
    /** Sends a Msg to the component's channel, and dispatches to ensure no events are received and any internal flush events are processed. */
    int submitAndDispatch(Msg msg, ReactorSubmitOptions submitOptions)
    {
        return submitAndDispatch(msg, submitOptions, false);
    }

    /** Sends a Msg to the component's channel, and dispatches. Allowing any unprocessed messages left*/
    int submitAndDispatch(Msg msg, ReactorSubmitOptions submitOptions, boolean expectFailures)
    {
        int ret = submit(msg, submitOptions, expectFailures);
        testReactor().dispatch(expectFailures ? -1 : 0);
        return ret;
    }
    

    /** Sends an RDM message to the component's channel. */
    int submit(MsgBase msg, ReactorSubmitOptions submitOptions)
    {
        int ret;

        ret = _reactorChannel.submit(msg, submitOptions, _errorInfo);
        
        assertTrue("submit failed: " + ret + "(" + _errorInfo.location() + "--" + _errorInfo.error().text() + ")",
                ret >= ReactorReturnCodes.SUCCESS);
        
        return ret;
    }
    
	/** Sends an RDM message to the component's channel, and dispatches to ensure no events are received and any internal flush events are processed. */
    int submitAndDispatch(MsgBase msg, ReactorSubmitOptions submitOptions)
    {
        int ret = submit(msg, submitOptions);
        testReactor().dispatch(0);
        return ret;
    }
	
	/** Disconnect a consumer and provider component and clean them up. */
	public static void closeSession(Consumer consumer, Provider provider)
	{
		closeSession(consumer,provider,false);
	}
	/** Disconnect a consumer and provider component and clean them up.
	 * Do additional checks to not fail on dirty client disonnection.
	 * */
	public static void closeSession(Consumer consumer, Provider provider, boolean expectConsumerFailure)
	{
		/* Make sure there's nothing left in the dispatch queue. */
		if (consumer.testReactor() != null) consumer.testReactor().dispatch(0, expectConsumerFailure);
		if (provider.testReactor() != null) provider.testReactor().dispatch(0);
		
		consumer.close();
		provider.close();
	}

    /** Closes the component's channel. */
    void closeChannel()
    {
        if(ReactorChannel.State.CLOSED !=_reactorChannel.state()
        		&& _reactorChannel.reactor() != null){
            assertEquals(ReactorReturnCodes.SUCCESS, _reactorChannel.close(_errorInfo));
        }
        _reactorChannelIsUp = false;
        _reactorChannel = null;
    }
    
	/** Close a component and remove it from its associated TestReactor. 
     * Closes any associated server and reactor channel. */
	void close()
	{
        assertNotNull(_testReactor);
		if (_server != null)
		{
			assertEquals(TransportReturnCodes.SUCCESS, _server.close(_errorInfo.error()));
			_server = null;
		}
		
		if (_reactorChannel != null)
            closeChannel();

        _testReactor.removeComponent(this);
        _testReactor = null;
	}
	
	/** Close a component and remove it from its associated TestReactor. 
     * Closes any associated server and reactor channel including Selector */
	void closeChannelAndSelector()
	{
        assertNotNull(_testReactor);
		if (_server != null)
		{
			assertEquals(TransportReturnCodes.SUCCESS, _server.close(_errorInfo.error()));
			_server = null;
		}
		
		try {
			_testReactor._selector.close();
		} catch (IOException e) {
			e.printStackTrace();
		}
		
		if (_reactorChannel != null)
            closeChannel();

        _testReactor.removeComponent(this);
        _testReactor = null;
	}
	
	int info(ReactorChannelInfo info)
	{
		return _reactorChannel.info(info, _errorInfo);
	}
}
