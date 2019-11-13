///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.upa.transport;

import static org.junit.Assert.*;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.NotYetConnectedException;


import org.junit.Test;

import com.thomsonreuters.upa.codec.Codec;
import com.thomsonreuters.upa.test.network.replay.NetworkReplay;
import com.thomsonreuters.upa.test.network.replay.NetworkReplayFactory;
import com.thomsonreuters.upa.transport.BindOptions;
import com.thomsonreuters.upa.transport.BindOptionsImpl;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelInfo;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.ConnectionTypes;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.InitArgs;
import com.thomsonreuters.upa.transport.Server;
import com.thomsonreuters.upa.transport.RsslSocketChannel;
import com.thomsonreuters.upa.transport.SocketProtocol.TrackingPool;
import com.thomsonreuters.upa.transport.Transport;
import com.thomsonreuters.upa.transport.TransportFactory;
import com.thomsonreuters.upa.transport.TransportReturnCodes;
import com.thomsonreuters.upa.transport.UnifiedNetworkInfoImpl;

public class TransportJunit
{

    private static final int DEFAULT_LISTEN_PORT = 4324;
    private static final String BASE_TEST_DATA_DIR_NAME = "src/test/resources/com/thomsonreuters/upa/transport/RsslSocketChannelJunit";

    public RsslSocketChannel getNetworkReplayChannel(Protocol transport, int numBuffers)
    {
        final Error error = TransportFactory.createError();
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/020_single_complete_message.txt";
        final NetworkReplay replay = NetworkReplayFactory.create();

        ConnectOptions opts = TransportFactory.createConnectOptions();

        opts.userSpecObject("TEST CHANNEL");
        opts.unifiedNetworkInfo().address("localhost");
        opts.unifiedNetworkInfo().serviceName("4324");
        opts.majorVersion(Codec.majorVersion());
        opts.minorVersion(Codec.minorVersion());
        opts.protocolType(Codec.protocolType());
        opts.guaranteedOutputBuffers(numBuffers);
        
        try
        {
            replay.parseFile(inputFile);
            
            replay.startListener(DEFAULT_LISTEN_PORT);

            RsslSocketChannel channel = new RsslSocketChannel()
            {
                @Override
                protected int read(ByteBuffer dst) throws NotYetConnectedException, IOException
                {
                    return replay.read(dst);
                }
                @Override
                protected int initChnlReadFromChannel(ByteBuffer dst, Error error) throws IOException
                {
                    return replay.read(dst);
                }
                @Override
                public int close(Error error)
                {
                    int ret = TransportReturnCodes.SUCCESS;
                    
                    // first set channel state to closed
                    try
                    {
                        lockReadWriteLocks();
                        
                        if (_state != ChannelState.INACTIVE)
                        {
                            _state = ChannelState.INACTIVE;
                            if (_compressor != null)
                            {
                                _compressor.close();
                            }
                        }
                        else
                        {
                            error.channel(this);
                            error.errorId(TransportReturnCodes.FAILURE);
                            error.sysError(0);
                            error.text("socket channel is inactive ");
                            ret = TransportReturnCodes.FAILURE;
                        }
                    }
                    finally
                    {
                        unlockReadWriteLocks();
                        if (ret == TransportReturnCodes.FAILURE)
                        {
                            return ret;
                        }
                    }

                    try
                    {
                        Transport._globalLock.lock();
                        
                        // when closing channel the buffers should not be used
                        // the check here is to make sure all resources are cleaned up
                        if (_used > 0)
                        {
                            // release buffers
                            for (int i = _writeArrayPosition; i < _writeArrayMaxPosition; i++)
                            {
                                releaseBufferInternal(_releaseBufferArray[i]);
                            }
                            clearQueues();
                            _writeArrayMaxPosition = 0;
                            _writeArrayPosition = 0;
                            _isFlushPending = false;
                            _totalBytesQueued = 0;
                        }

                        // close socket
                        try 
                        {
                            _scktChannel.close();
                            if(_encrypted)
                            {
                                if(_crypto != null)
                                    _crypto.cleanup();
                            }
                        } 
                        catch (IOException e) 
                        {
                            error.channel(this);
                            error.errorId(TransportReturnCodes.FAILURE);
                            error.sysError(0);
                            error.text("socket channel close failed ");
                            ret = TransportReturnCodes.FAILURE;
                        }

                        // move all buffers from _availableBuffers pool to the global pool
                        shrinkGuaranteedOutputBuffers(_availableBuffers.size());

                        if (_server != null)
                        {
                            // unregister from server
                            _server.removeChannel();
                            _server = null;
                        }           
                        
                        // reset this channel to default values, prior to adding back in the pool.
                        resetToDefault();

                        if(_readIoBuffer != null)
                        {
                            _readIoBuffer.returnToPool();
                            _readIoBuffer = null;
                        }
                        
                        // return channel to pool
                        // there's no pooling for this fake channel - returnToPool();
                    }
                    finally
                    {
                        Transport._globalLock.unlock();
                    }
                    
                    return ret;
                }
            };            

            channel._transport = (SocketProtocol) transport;
            channel.connect(opts, error);
            assertTrue(channel.state() == ChannelState.INITIALIZING);

            waitForChannelActive(channel); // wait for the channel to become active
            return channel;

        }
        catch (IOException e)
        {
            fail(e.getLocalizedMessage());
        }
        finally
        {
            if (replay != null)
            {
                replay.stopListener();
            }
        }
		return null;
    }  

    /**
     * Waits for the specified channel to become active
     * 
     * @param channel
     * @throws IOException
     */
    private void waitForChannelActive(final RsslSocketChannel channel) throws IOException
    {
        assertTrue(channel != null);

        final int MAX_ATTEMPTS = 10;

        final Error error = TransportFactory.createError();
        final InProgInfo inProg = TransportFactory.createInProgInfo();
        int retval;

        int attempts = 0;
        // Wait for channel to become active. This finalizes the three-way
        // handshake.
        while (channel.state() != ChannelState.ACTIVE && attempts < MAX_ATTEMPTS)
        {
            if (channel.state() == ChannelState.INITIALIZING)
            {
                if ((retval = channel.init(inProg, error)) < TransportReturnCodes.SUCCESS)
                {
                    String initFail = "\nChannel " + channel.selectableChannel() + " inactive: "
                            + error.text();
                    System.err.println(initFail);
                    throw new IOException(initFail);
                }
                else
                {
                    switch (retval)
                    {
                        case TransportReturnCodes.CHAN_INIT_IN_PROGRESS:
                            System.out.println("\nChannel " + channel.selectableChannel()
                                    + " In Progress...");
                            break;
                        case TransportReturnCodes.SUCCESS:
                            System.out.println("\nChannel " + channel.selectableChannel()
                                    + " Is Active");
                            break;
                        default:
                            String badRet = "\nBad return value channel="
                                    + channel.selectableChannel() + " <" + error.text() + ">";
                            System.out.println(badRet);
                            throw new IOException(badRet);
                    }
                }
            }

            try
            {
                Thread.sleep(1000);
            }
            catch (Exception e)
            {
                throw new IOException("Thread.sleep() exception: " + e.getLocalizedMessage());
            }

            ++attempts;
        }

        if (channel.state() != ChannelState.ACTIVE)
        {
            throw new IOException("channel never became active");
        }
    }


    /**
     * GIVEN a locking type of  {@link RsslLockingTypes#NONE NONE}
     * WHEN {@link Transport#initialize initialize} is invoked
     * THEN it will return {@link TransportReturnCodes#SUCCESS SUCEESS}.
     */
    @Test
    public void initializeWithNoneTest()
    {
        final Error error = TransportFactory.createError();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        }
        finally
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /**
     * GIVEN a locking type of {@link RsslLockingTypes#GLOBAL GLOBAL} 
     * WHEN {@link Transport#initialize initialize} is invoked the second
     * time with a different locking type of {@link RsslLockingTypes#GLOBAL_AND_CHANNEL
     * GLOBAL_AND_CHANNEL} 
     * THEN it will return {@link TransportReturnCodes#FAILURE FAILURE}.
     */
    @Test
    public void initializeWithDifferentLockTypeTest()
    {
        final Error error = TransportFactory.createError();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(true);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.FAILURE,
                         Transport.initialize(initArgs, error));
        }
        finally
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /**
     * 
     * @return RsslBindOptions populated with default options.
     */
    private BindOptions getDefaultBindOptions()
    {
        BindOptions bindOpts = TransportFactory.createBindOptions();
        bindOpts.userSpecObject("TEST SERVER");
        bindOpts.serviceName("14005");
        bindOpts.majorVersion(Codec.majorVersion());
        bindOpts.minorVersion(Codec.minorVersion());
        bindOpts.protocolType(Codec.protocolType());
        return bindOpts;
    }
    
    /**
     * GIVEN default bind options
     * WHEN {@link Transport#bind(BindOptions, Error) bind} is invoked
     * THEN {@link Transport#bind(BindOptions, Error) bind} is expected
     * to succeed and return an {@link Server}.
     */
    @Test
    public void bindTest()
    {
        final Error error = TransportFactory.createError();
        Server server = null;
        BindOptions bindOpts = getDefaultBindOptions();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));

            server = Transport.bind(bindOpts, error);
            assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
                    + error.sysError(), server);
        }
        finally
        {
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /**
     * GIVEN default bind options
     * WHEN {@link Transport#bind(BindOptions, Error) bind} is invoked
     * THEN {@link Transport#bind(BindOptions, Error) bind} is expected
     * to succeed and return an {@link Server}.
     */
    @Test
    public void bindServiceNameTest()
    {
        final Error error = TransportFactory.createError();
        Server server = null;
        BindOptions bindOpts = getDefaultBindOptions();
        bindOpts.serviceName("ssl_provider");

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));

            server = Transport.bind(bindOpts, error);
            assertEquals(8102, server.portNumber());
            assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
                    + error.sysError(), server);
        }
        finally
        {
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /**
     * Set the service name in bind options to a known service
     * Expect the port number that correspond (in services file) 
     * to this service
     */
    @Test
    public void bindServiceNameFromEtcTest()
    {
        BindOptions bindOpts = getDefaultBindOptions();
        bindOpts.serviceName("klogin");
        assertEquals(543, ((BindOptionsImpl)bindOpts).port());
    }
    
    /**
     * GIVEN default bind options
     * WHEN {@link BindOptions#guaranteedOutputBuffers(bufferCount)} 
     * or {@link BindOptions#maxOutputBuffers(bufferCount)} are invoked
     * THEN guaranteed and max values are kept in sync according to the
     * rule that max >= guaranteed and guaranteed >= 5. 
     */
    @Test
    public void bindOptionsBufferTest()
    {
        final Error error = TransportFactory.createError();
        Server server = null;
        BindOptions bindOpts;
        
        // Test: Minimum value for guaranteed is 5
        bindOpts = getDefaultBindOptions();
        bindOpts.guaranteedOutputBuffers(2);
        assertEquals(5, bindOpts.guaranteedOutputBuffers());
        
        // Test: If guaranteed is set above max, then max will automatically
        // be adjusted, since max must be at least the size of guaranteed.
        bindOpts = getDefaultBindOptions();
        int currentMax = bindOpts.maxOutputBuffers();
        bindOpts.guaranteedOutputBuffers(currentMax + 20);
        assertEquals(currentMax + 20, bindOpts.maxOutputBuffers());

        // Test: max cannot be set below guaranteed
        bindOpts = getDefaultBindOptions();
        bindOpts.maxOutputBuffers(bindOpts.guaranteedOutputBuffers() - 2);
        assertEquals(bindOpts.guaranteedOutputBuffers(), bindOpts.maxOutputBuffers());
        
        // Test: negative max value is ignored
        bindOpts = getDefaultBindOptions();
        currentMax = bindOpts.maxOutputBuffers();
        BindOptionsImpl._runningInJunits = true;
        bindOpts.maxOutputBuffers(-4);
        BindOptionsImpl._runningInJunits = false;
        assertEquals(currentMax, bindOpts.maxOutputBuffers());
        
        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));

            server = Transport.bind(bindOpts, error);

            assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
                    + error.sysError(), server);
        }
        finally
        {
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    private ConnectOptions getDefaultConnectOptions()
    {
        ConnectOptions connectOpts = TransportFactory.createConnectOptions();
        connectOpts.userSpecObject("TEST CHANNEL");
        connectOpts.unifiedNetworkInfo().address("localhost");
        connectOpts.unifiedNetworkInfo().serviceName("4324");
        connectOpts.majorVersion(Codec.majorVersion());
        connectOpts.minorVersion(Codec.minorVersion());
        connectOpts.protocolType(Codec.protocolType());
        return connectOpts;
    }
    
    /**
     * GIVEN a non initialized {@link Transport}
     * WHEN {@link Transport#connect connect} is invoked
     * THEN {@link Transport#connect connect} will fail,
     * return a null channel and set errorId to 
     * {@link TransportReturnCodes#INIT_NOT_INITIALIZED INIT_NOT_INITIALIZED}.
     */
    @Test
    public void connectBeforeInitializeTest()
    {
        final Error error = TransportFactory.createError();

        ConnectOptions connectOpts = getDefaultConnectOptions();

        Channel channel = Transport.connect(connectOpts, error);
        assertNull("Expected channel to be null since connect was called prior to initialize.",
                   channel);
        assertEquals(TransportReturnCodes.INIT_NOT_INITIALIZED, error.errorId());
    }

    /**
     * The service name in the connect options, unified network info, is set to a 
     * string that can be found in services file. The code internally should asign a port 
     * number mapped to the service name.
     * Then the service name is set to well known services.
     */
    @Test
    public void connectOptsServiceNameTest()
    {
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.unifiedNetworkInfo().serviceName("kerberos-adm");
        int port = ((UnifiedNetworkInfoImpl)connectOpts.unifiedNetworkInfo()).port();
        assertEquals(749, port);

        connectOpts.unifiedNetworkInfo().serviceName("triarch_sink");
        port = ((UnifiedNetworkInfoImpl)connectOpts.unifiedNetworkInfo()).port();
        assertEquals(8101, port);

        connectOpts.unifiedNetworkInfo().serviceName("triarch_src");
        port = ((UnifiedNetworkInfoImpl)connectOpts.unifiedNetworkInfo()).port();
        assertEquals(8102, port);

        connectOpts.unifiedNetworkInfo().serviceName("triarch_dbms");
        port = ((UnifiedNetworkInfoImpl)connectOpts.unifiedNetworkInfo()).port();
        assertEquals(8103, port);

        connectOpts.unifiedNetworkInfo().serviceName("rmds_ssl_sink");
        port = ((UnifiedNetworkInfoImpl)connectOpts.unifiedNetworkInfo()).port();
        assertEquals(8101, port);

        connectOpts.unifiedNetworkInfo().serviceName("rmds_ssl_source");
        port = ((UnifiedNetworkInfoImpl)connectOpts.unifiedNetworkInfo()).port();
        assertEquals(8103, port);

        connectOpts.unifiedNetworkInfo().serviceName("ssl_consumer");
        port = ((UnifiedNetworkInfoImpl)connectOpts.unifiedNetworkInfo()).port();
        assertEquals(8101, port);

        connectOpts.unifiedNetworkInfo().serviceName("ssl_provider");
        port = ((UnifiedNetworkInfoImpl)connectOpts.unifiedNetworkInfo()).port();
        assertEquals(8102, port);

        connectOpts.unifiedNetworkInfo().serviceName("_consumer");
        port = ((UnifiedNetworkInfoImpl)connectOpts.unifiedNetworkInfo()).port();
        assertEquals(14002, port);

        connectOpts.unifiedNetworkInfo().serviceName("_provider");
        port = ((UnifiedNetworkInfoImpl)connectOpts.unifiedNetworkInfo()).port();
        assertEquals(14003, port);

        connectOpts.unifiedNetworkInfo().serviceName("klogin");
        port = ((UnifiedNetworkInfoImpl)connectOpts.unifiedNetworkInfo()).port();
        assertEquals(543, port);
    }

    @Test
    public void reconnectTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));

            channel = Transport.connect(connectOpts, error);
            assertNotNull(channel);
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            channel.reconnectClient(error);
            errorCode = error.errorId();
            assertEquals(TransportReturnCodes.FAILURE, errorCode);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /**
     * GIVEN an initialized {@link Transport} and a valid
     * {@link ConnectionTypes#SOCKET},
     * WHEN {@link Transport#connect connect} is invoked
     * THEN @link RsslTransport#connect connect} will succeed,
     * return a non-null channel and set errorId to 
     * {@link TransportReturnCodes#SUCCESS SUCCESS}.
     * The test connects 11 times (10 initial channels + 1 on demand)
     * Then all channel should be different objects.
     * THEN all the channels are closed.
     */
    @Test
    public void connect11Test()
    {
        final Error error = TransportFactory.createError();
        Channel[] channel = new Channel[11];
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
            for (int i = 0; i<11; ++i)
            {
                channel[i] = Transport.connect(connectOpts, error);
                assertNotNull(channel);
                int errorCode = error.errorId();
                assertEquals(TransportReturnCodes.SUCCESS, errorCode);
                for (int j=0; j<i; j++)
                {
                	assertNotSame(channel[i], channel[j]);
                }
            }
        }
        finally
        {
        	assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /**
     * GIVEN an initialized {@link Transport} and a valid
     * {@link ConnectionTypes#SOCKET},
     * WHEN {@link Transport#connect connect} is invoked
     * THEN @link RsslTransport#connect connect} will succeed,
     * return a non-null channel and set errorId to 
     * {@link TransportReturnCodes#SUCCESS SUCCESS}.
     * THEN the channel is closed.
     * The test connects 11 times 
     * THEN the first ten channels should be different objects.
     * AND the last channel should be the same object as the first.
     */
    @Test
    public void connectReuse11Test()
    {
        final Error error = TransportFactory.createError();
        Channel[] channel = new Channel[11];
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);
        
        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
            
            for (int i = 0; i < 2; ++i)
            {
                channel[i] = Transport.connect(connectOpts, error);
                assertNotNull(channel);
                int errorCode = error.errorId();
                assertEquals(TransportReturnCodes.SUCCESS, errorCode);
                for (int j = 0; j < i; j++)
                {
                	assertNotSame(channel[i], channel[j]);
                }
                channel[i].close(error);
            }
            channel[10] = Transport.connect(connectOpts, error);
            assertEquals(channel[0], channel[10]);
        }
        finally
        {
        		if (channel[10] != null)
        			channel[10].close(error);
        	assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /**
     * GIVEN an initialized {@link Transport} and a valid
     * {@link ConnectionTypes#SOCKET},
     * WHEN {@link Transport#connect connect} is invoked
     * THEN @link RsslTransport#connect connect} will succeed,
     * return a non-null channel and set errorId to 
     * {@link TransportReturnCodes#SUCCESS SUCCESS}.
     * WHEN {@link Channel#getInfo getInfo} is invoked
     * THEN it returns {@link TransportReturnCodes#FAILURE FAILURE}.
     */
    @Test
    public void errorGetInfoTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        ChannelInfo info = TransportFactory.createChannelInfo();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
            channel = Transport.connect(connectOpts, error);
            assertNotNull(channel);
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            int ret = channel.info(info, error);
            assertEquals((ret<0), true);
            assertEquals(error.errorId(), TransportReturnCodes.FAILURE);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /**
     * GIVEN an initialized {@link Transport} and a valid
     * {@link ConnectionTypes#SOCKET},
     * WHEN {@link Transport#connect connect} is invoked
     * THEN @link RsslTransport#connect connect} will succeed,
     * return a non-null channel and set errorId to 
     * {@link TransportReturnCodes#SUCCESS SUCCESS}.
     * WHEN {@link Channel#bufferUsage bufferUsage} is invoked
     * THEN it returns {@link TransportReturnCodes#FAILURE FAILURE}.
     */
    @Test
    public void errorBufferUsageTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
            channel = Transport.connect(connectOpts, error);
            assertNotNull(channel);
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            int ret = channel.bufferUsage(error);
            assertEquals((ret<0), true);
            assertEquals(error.errorId(), TransportReturnCodes.FAILURE);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    

    /**
     * GIVEN default bind options
     * WHEN {@link Transport#bind(BindOptions, Error) bind} is invoked
     * THEN {@link Transport#bind(BindOptions, Error) bind} is expected
     * to succeed and return an {@link Server}.
     * GIVEN the test binds to 6 different servers (5 initial and one on demand)
     * ALL servers are different objects
     * THEN the servers are closed
     */
    @Test
    public void bind6Test()
    {
        final Error error = TransportFactory.createError();
        Server[] servers = new Server[6];
        BindOptions bindOpts = getDefaultBindOptions();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));

            for (int i=0; i<6; i++)
            {
            	bindOpts.serviceName(Integer.toString(14105+i));
            	servers[i] = Transport.bind(bindOpts, error);
            	assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
            			+ error.sysError(), servers[i]);
                for (int j=0; j<i; j++)
                {
                	assertNotSame(servers[i], servers[j]);
                }
            }
        }
        finally
        {
        	assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /**
     * GIVEN default bind options
     * WHEN {@link Transport#bind(BindOptions, Error) bind} is invoked
     * THEN {@link Transport#bind(BindOptions, Error) bind} is expected
     * to succeed and return an {@link Server}.
     * GIVEN the test binds to a server
     * THEN the channel is closed.
     * The test binds 2 times with different port number (ServiceName) option
     * THEN the first five servers should be different objects.
     * AND the last server should be the same object as the first.
     */
    @Test
    public void bindReuse2Test()
    {
        final Error error = TransportFactory.createError();
        Server[] servers = new Server[2];
        BindOptions bindOpts = getDefaultBindOptions();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));

            for (int i = 0; i < 2; i++)
            {
            	bindOpts.serviceName(Integer.toString(14205+i));
            	servers[i] = Transport.bind(bindOpts, error);
            	assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
            			+ error.sysError(), servers[i]);
                servers[i].close(error);
            }
            assertEquals(servers[0], servers[1]);
        }
        finally
        {
        	assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /**
     * GIVEN default bind options
     * WHEN {@link Transport#bind(BindOptions, Error) bind} is invoked
     * THEN {@link Transport#bind(BindOptions, Error) bind} is expected
     * to succeed and return an {@link Server}.
     * WHEN {@link Server#bufferUsage(Error) bufferUsage} is invoked
     * THEN {@link TransportReturnCodes.SUCCESS} is expected
     */
    
    @Test
    public void serverBufferUsageTest()
    {
        final Error error = TransportFactory.createError();
        Server server = null;
        BindOptions bindOpts = getDefaultBindOptions();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));

            server = Transport.bind(bindOpts, error);
            assertNotNull(error.text() + " errorId=" + error.errorId() + " sysErrorId="
                    + error.sysError(), server);
            int ret = server.bufferUsage(error);
            assertEquals(0, ret);
            assertEquals(error.errorId(), TransportReturnCodes.SUCCESS);            
        }
        finally
        {
            if (server != null)
                server.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
        
    }
    


    @Test
    public void trackingPoolTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        Channel channel1 = null;
        Channel channel2 = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
            channel = Transport.connect(connectOpts, error);

            SocketProtocol transport = (SocketProtocol)Transport._transports[0];            
            TrackingPool chanPool = (TrackingPool)transport._channelPool;            

            assertNotNull(channel);
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(1, chanPool._active._size);
            assertEquals(channel, chanPool._active._head);
            assertEquals(channel, chanPool._active._tail);
            assertEquals(1, chanPool._queue._size);
            
            channel1 = Transport.connect(connectOpts, error);
            assertNotNull(channel1);
            errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(2, chanPool._active._size);
            assertEquals(channel, chanPool._active._head);
            assertEquals(channel1, chanPool._active._tail);
            assertEquals(0, chanPool._queue._size);
            
            channel.close(error);
            assertEquals(1, chanPool._active._size);
            assertEquals(channel1, chanPool._active._head);
            assertEquals(channel1, chanPool._active._tail);
            assertEquals(1, chanPool._queue._size);
            
            channel2 = Transport.connect(connectOpts, error);
            assertEquals(channel2, channel);
            assertEquals(2, chanPool._active._size);
            assertEquals(channel1, chanPool._active._head);
            assertEquals(channel, chanPool._active._tail);
            assertEquals(0, chanPool._queue._size);
            
            channel.close(error);
            channel1.close(error);
            assertEquals(0, chanPool._active._size);
            assertEquals(null, chanPool._active._head);
            assertEquals(null, chanPool._active._tail);
            assertEquals(2, chanPool._queue._size);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            if (channel1 != null)
                channel1.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    @Test
    public void connectOptionsCopyTest()
    {
        /* Test the ConnectOptions.copy() method. */

        ConnectOptions srcOpts = TransportFactory.createConnectOptions();
        ConnectOptions destOpts = TransportFactory.createConnectOptions();

        /* Test copying the default options. */
        
        /* Copy options */
        srcOpts.copy(destOpts);

        /* Ensure source and destination options have the correct values. */
        for (int i = 0; i < 2; ++i)
        {
            ConnectOptions testOpts = ((i == 0) ? destOpts : srcOpts);

            assertNull(testOpts.componentVersion());
            assertEquals(ConnectionTypes.SOCKET, testOpts.connectionType());
            assertEquals(CompressionTypes.NONE, testOpts.compressionType());
            assertEquals(false, testOpts.blocking());
            assertEquals(60, testOpts.pingTimeout());
            assertEquals(50, testOpts.guaranteedOutputBuffers());
            assertEquals(10, testOpts.numInputBuffers());
            assertEquals(0, testOpts.majorVersion());
            assertEquals(0, testOpts.minorVersion());
            assertEquals(0, testOpts.protocolType());
            assertNull(testOpts.userSpecObject());
            assertNull(testOpts.unifiedNetworkInfo().address());
            assertNull(testOpts.unifiedNetworkInfo().serviceName());
            assertNull(testOpts.unifiedNetworkInfo().interfaceName());
            assertNull(testOpts.unifiedNetworkInfo().unicastServiceName());
            assertNull(testOpts.segmentedNetworkInfo().recvAddress());
            assertNull(testOpts.segmentedNetworkInfo().recvServiceName());
            assertNull(testOpts.segmentedNetworkInfo().unicastServiceName());
            assertNull(testOpts.segmentedNetworkInfo().interfaceName());
            assertNull(testOpts.segmentedNetworkInfo().sendAddress());
            assertNull(testOpts.segmentedNetworkInfo().sendServiceName());
            assertTrue(testOpts.tunnelingInfo().tunnelingType().equals("None"));
            assertEquals(false, testOpts.tunnelingInfo().HTTPproxy());
            assertNull(testOpts.tunnelingInfo().HTTPproxyHostName());
            assertEquals(0, testOpts.tunnelingInfo().HTTPproxyPort());
            assertTrue(testOpts.tunnelingInfo().objectName().equals(""));
            assertTrue(testOpts.tunnelingInfo().KeystoreType().equals("JKS"));
            assertNull(testOpts.tunnelingInfo().KeystoreFile());
            assertNull(testOpts.tunnelingInfo().KeystorePasswd());
            assertTrue(testOpts.tunnelingInfo().SecurityProtocol().equals("TLS"));
            assertTrue(testOpts.tunnelingInfo().SecurityProvider().equals("SunJSSE"));
            assertTrue(testOpts.tunnelingInfo().KeyManagerAlgorithm().equals("SunX509"));
            assertTrue(testOpts.tunnelingInfo().TrustManagerAlgorithm().equals("PKIX"));
            assertNull(testOpts.credentialsInfo().HTTPproxyUsername());
            assertNull(testOpts.credentialsInfo().HTTPproxyPasswd());
            assertNull(testOpts.credentialsInfo().HTTPproxyDomain());
            assertNull(testOpts.credentialsInfo().HTTPproxyLocalHostname());
            assertNull(testOpts.credentialsInfo().HTTPproxyKRB5configFile());
            assertEquals(false, testOpts.tcpOpts().tcpNoDelay());
            assertNull(((TcpOptsImpl)testOpts.tcpOpts()).replay());
            assertEquals(false, testOpts.multicastOpts().disconnectOnGaps());
            assertEquals(5, testOpts.multicastOpts().packetTTL());
            assertNull(testOpts.multicastOpts().tcpControlPort());
            assertEquals(0, testOpts.multicastOpts().portRoamRange());
            assertEquals(0, testOpts.shmemOpts().maxReaderLag());
            assertEquals(false, testOpts.channelReadLocking());
            assertEquals(false, testOpts.channelWriteLocking());
            assertEquals(0, testOpts.sysSendBufSize());
            assertEquals(0, testOpts.sysRecvBufSize());
            assertEquals(3000, testOpts.seqMCastOpts().maxMsgSize());
            assertEquals(0, testOpts.seqMCastOpts().instanceId());
        }
        
        /* Now test copying all options. */
        
        srcOpts.componentVersion("HARTNELL");
        srcOpts.connectionType(ConnectionTypes.SEQUENCED_MCAST);
        srcOpts.compressionType(CompressionTypes.LZ4);
        srcOpts.blocking(true);
        srcOpts.pingTimeout(55);
        srcOpts.guaranteedOutputBuffers(66);
        srcOpts.numInputBuffers(77);
        srcOpts.majorVersion(88);
        srcOpts.minorVersion(99);
        srcOpts.protocolType(128);
        srcOpts.userSpecObject(this);
        srcOpts.unifiedNetworkInfo().address("TROUGHTON");
        srcOpts.unifiedNetworkInfo().serviceName("5555");
        srcOpts.unifiedNetworkInfo().interfaceName("PERTWEE");
        srcOpts.unifiedNetworkInfo().unicastServiceName("TOM.BAKER");
        srcOpts.segmentedNetworkInfo().recvAddress("DAVISON");
        srcOpts.segmentedNetworkInfo().recvServiceName("6666");
        srcOpts.segmentedNetworkInfo().unicastServiceName("7777");
        srcOpts.segmentedNetworkInfo().interfaceName("COLIN.BAKER");
        srcOpts.segmentedNetworkInfo().sendAddress("MCCOY");
        srcOpts.segmentedNetworkInfo().sendServiceName("MCGANN");
        srcOpts.tunnelingInfo().tunnelingType("encrypted");
        srcOpts.tunnelingInfo().HTTPproxy(true);
        srcOpts.tunnelingInfo().HTTPproxyHostName("HURT");
        srcOpts.tunnelingInfo().HTTPproxyPort(8888);
        srcOpts.tunnelingInfo().objectName("ECCLESTON");
        srcOpts.tunnelingInfo().KeystoreType("TENNANT");
        srcOpts.tunnelingInfo().KeystoreFile("SMITH");
        srcOpts.tunnelingInfo().KeystorePasswd("CAPALDI");
        srcOpts.tunnelingInfo().SecurityProtocol("STEWART");
        srcOpts.tunnelingInfo().SecurityProvider("FRAKES");
        srcOpts.tunnelingInfo().KeyManagerAlgorithm("DORN");
        srcOpts.tunnelingInfo().TrustManagerAlgorithm("SIRTIS");
        srcOpts.credentialsInfo().HTTPproxyUsername("SPINER");
        srcOpts.credentialsInfo().HTTPproxyPasswd("BURTON");
        srcOpts.credentialsInfo().HTTPproxyDomain("CRUSHER");
        srcOpts.credentialsInfo().HTTPproxyLocalHostname("WHEATON");
        srcOpts.credentialsInfo().HTTPproxyKRB5configFile("SCHULTZ");
        srcOpts.tcpOpts().tcpNoDelay(true);
        ((TcpOptsImpl)srcOpts.tcpOpts()).replay("BROOKS");
        srcOpts.multicastOpts().disconnectOnGaps(true);
        srcOpts.multicastOpts().packetTTL(99);
        srcOpts.multicastOpts().tcpControlPort("9999");
        srcOpts.multicastOpts().portRoamRange(12);
        srcOpts.shmemOpts().maxReaderLag(11111);
        srcOpts.channelReadLocking(true);
        srcOpts.channelWriteLocking(true);
        srcOpts.sysSendBufSize(22222);
        srcOpts.sysRecvBufSize(33333);
        srcOpts.seqMCastOpts().maxMsgSize(44444);
        srcOpts.seqMCastOpts().instanceId(55555);

        /* Copy options */
        srcOpts.copy(destOpts);

        /* Ensure source and destination options have the correct values. */
        for (int i = 0; i < 2; ++i)
        {
            ConnectOptions testOpts = ((i == 0) ? destOpts : srcOpts);

            assertTrue(testOpts.componentVersion().toString().equals("HARTNELL"));
            assertEquals(ConnectionTypes.SEQUENCED_MCAST, testOpts.connectionType());
            assertEquals(CompressionTypes.LZ4, testOpts.compressionType());
            assertEquals(true, testOpts.blocking());
            assertEquals(55, testOpts.pingTimeout());
            assertEquals(66, testOpts.guaranteedOutputBuffers());
            assertEquals(77, testOpts.numInputBuffers());
            assertEquals(88, testOpts.majorVersion());
            assertEquals(99, testOpts.minorVersion());
            assertEquals(128, testOpts.protocolType());
            assertEquals(this, testOpts.userSpecObject());
            assertTrue(testOpts.unifiedNetworkInfo().address().equals("TROUGHTON"));
            assertTrue(testOpts.unifiedNetworkInfo().serviceName().equals("5555"));
            assertEquals(5555, ((UnifiedNetworkInfoImpl)testOpts.unifiedNetworkInfo()).port());
            assertTrue(testOpts.unifiedNetworkInfo().interfaceName().equals("PERTWEE"));
            assertTrue(testOpts.unifiedNetworkInfo().unicastServiceName().equals("TOM.BAKER"));
            assertTrue(testOpts.segmentedNetworkInfo().recvAddress().equals("DAVISON"));
            assertTrue(testOpts.segmentedNetworkInfo().recvServiceName().equals("6666"));
            assertTrue(testOpts.segmentedNetworkInfo().unicastServiceName().equals("7777"));
            assertTrue(testOpts.segmentedNetworkInfo().interfaceName().equals("COLIN.BAKER"));
            assertTrue(testOpts.segmentedNetworkInfo().sendAddress().equals("MCCOY"));
            assertTrue(testOpts.segmentedNetworkInfo().sendServiceName().equals("MCGANN"));
            assertTrue(testOpts.tunnelingInfo().tunnelingType().equals("encrypted"));
            assertEquals(true, testOpts.tunnelingInfo().HTTPproxy());
            assertTrue(testOpts.tunnelingInfo().HTTPproxyHostName().equals("HURT"));
            assertEquals(8888, testOpts.tunnelingInfo().HTTPproxyPort());
            assertTrue(testOpts.tunnelingInfo().objectName().equals("ECCLESTON"));
            assertTrue(testOpts.tunnelingInfo().KeystoreType().equals("TENNANT"));
            assertTrue(testOpts.tunnelingInfo().KeystoreFile().equals("SMITH"));
            assertTrue(testOpts.tunnelingInfo().KeystorePasswd().equals("CAPALDI"));
            assertTrue(testOpts.tunnelingInfo().SecurityProtocol().equals("STEWART"));
            assertTrue(testOpts.tunnelingInfo().SecurityProvider().equals("FRAKES"));
            assertTrue(testOpts.tunnelingInfo().KeyManagerAlgorithm().equals("DORN"));
            assertTrue(testOpts.tunnelingInfo().TrustManagerAlgorithm().equals("SIRTIS"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyUsername().equals("SPINER"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyPasswd().equals("BURTON"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyDomain().equals("CRUSHER"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyLocalHostname().equals("WHEATON"));
            assertTrue(testOpts.credentialsInfo().HTTPproxyKRB5configFile().equals("SCHULTZ"));
            assertEquals(true, testOpts.tcpOpts().tcpNoDelay());
            assertTrue(((TcpOptsImpl)testOpts.tcpOpts()).replay().equals("BROOKS"));
            assertEquals(true, testOpts.multicastOpts().disconnectOnGaps());
            assertEquals(99, testOpts.multicastOpts().packetTTL());
            assertTrue(testOpts.multicastOpts().tcpControlPort().equals("9999"));
            assertEquals(12, testOpts.multicastOpts().portRoamRange());
            assertEquals(11111, testOpts.shmemOpts().maxReaderLag());
            assertEquals(true, testOpts.channelReadLocking());
            assertEquals(true, testOpts.channelWriteLocking());
            assertEquals(22222, testOpts.sysSendBufSize());
            assertEquals(33333, testOpts.sysRecvBufSize());
            assertEquals(44444, testOpts.seqMCastOpts().maxMsgSize());
            assertEquals(55555, testOpts.seqMCastOpts().instanceId());
        }

    }
    
}
