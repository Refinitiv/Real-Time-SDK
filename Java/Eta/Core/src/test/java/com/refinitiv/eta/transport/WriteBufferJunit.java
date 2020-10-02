package com.refinitiv.eta.transport;

import static org.junit.Assert.*;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.NotYetConnectedException;

import org.junit.Test;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.EncodeIterator;
import com.refinitiv.eta.test.network.replay.NetworkReplay;
import com.refinitiv.eta.test.network.replay.NetworkReplayFactory;
import com.refinitiv.eta.transport.Channel;
import com.refinitiv.eta.transport.ChannelState;
import com.refinitiv.eta.transport.ConnectOptions;
import com.refinitiv.eta.transport.Error;
import com.refinitiv.eta.transport.InitArgs;
import com.refinitiv.eta.transport.RsslSocketChannel;
import com.refinitiv.eta.transport.Transport;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.transport.TransportFactory;
import com.refinitiv.eta.transport.TransportReturnCodes;
import com.refinitiv.eta.transport.WriteArgs;

public class WriteBufferJunit
{

    private static final int DEFAULT_LISTEN_PORT = 4325;
    private static final String BASE_TEST_DATA_DIR_NAME = "src/test/resources/com/refinitiv/eta/transport/SocketChannelJunit";
    private final static String RIPC_BASE_TEST_DATA_DIR_NAME = "src/test/resources/com/refinitiv/eta/transport/RipcHandshakeJunit";

    final static int SLEEPTIMEMS = 25; // ms

    /*
     * This field denotes the current (i.e. default) RIPC version.
     */
    final static int CURRENT_RIPC_VERSION = 14;

    final static String receivedComponentVersion = "Component Test version 1.0";
    
    /* gets a network replay channel to replay file data */
    public RsslSocketChannel getNetworkReplayChannel(Protocol transport, int numBuffers)
    {
        final Error error = TransportFactory.createError();
        final String inputFile = BASE_TEST_DATA_DIR_NAME + "/020_single_complete_message.txt";
        final NetworkReplay replay = NetworkReplayFactory.create();

        ConnectOptions opts = TransportFactory.createConnectOptions();

        opts.userSpecObject("TEST CHANNEL");
        opts.unifiedNetworkInfo().address("localhost");
        opts.unifiedNetworkInfo().serviceName("4325");
        opts.majorVersion(Codec.majorVersion());
        opts.minorVersion(Codec.minorVersion());
        opts.protocolType(Codec.protocolType());
        opts.guaranteedOutputBuffers(numBuffers);
        
        try
        {
            replay.parseFile(inputFile);
            replay.startListener(DEFAULT_LISTEN_PORT);
        }
        catch (IOException e)
        {
            fail("exception occurred, " + e.toString());
        }
        
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
                	if (_state != ChannelState.CLOSED)
                	{
                		_state = ChannelState.CLOSED;
            		}
            		else
            		{
        				error.channel(this);
        				error.errorId(TransportReturnCodes.FAILURE);
        				error.sysError(0);
        				error.text("socket channel is already closed ");
        				ret = TransportReturnCodes.FAILURE;
            		}
                }
                finally
                {
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
        				try
        				{
        					_writeLock.lock();
        					_used = 0;
        				}
        				finally
        				{
        					_writeLock.unlock();
        				}
        			}
        			// move all buffers from _availableBuffers pool to the global pool
        			Pool bufferPool =_transport.getPool(_internalMaxFragmentSize);
        			bufferPool.add(_availableBuffers, _availableBuffers.size());

        			if (_server != null)
        			{
        				// unregister from server
        				_server.removeChannel();
        				_server = null;
        			}
        			
        			// close socket
        			try 
        			{
        				_scktChannel.close();
        			} 
        			catch (IOException e) 
        			{
        				error.channel(this);
        				error.errorId(TransportReturnCodes.FAILURE);
        				error.sysError(0);
        				error.text("socket channel close failed ");
        				ret = TransportReturnCodes.FAILURE;
        			}
        			_channelInfo.clear();
            	}
            	finally
            	{
            		Transport._globalLock.unlock();
                }
                
                return ret;
            }

        };            

        try {
            channel._transport = (SocketProtocol) transport;
            
            channel._scktChannel = new SocketHelper();
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

    /*
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

    /* generate default ConnectOptions */
    private ConnectOptions getDefaultConnectOptions()
    {
        ConnectOptions connectOpts = TransportFactory.createConnectOptions();
        connectOpts.userSpecObject("TEST CHANNEL");
        connectOpts.unifiedNetworkInfo().address("localhost");
        connectOpts.unifiedNetworkInfo().serviceName("4325");
        connectOpts.majorVersion(Codec.majorVersion());
        connectOpts.minorVersion(Codec.minorVersion());
        connectOpts.protocolType(Codec.protocolType());
        return connectOpts;
    }

    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 2.
     * 
     * The connect returns SUCCESS. 
     * 
     * The getBuffer() is called three times on the channel. The buffer size
     * requested is bigger then fragment size/2.
     * 
     * The first two calls return valid buffers. The third call returns null.
     * 
     * The bufferUsage() called before each buffer allocation returns
     * respectively: 0, 1, 2.
     */
    @SuppressWarnings("deprecation")
	@Test
    public void allocateBuffersTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 2);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer = channel.getBuffer(4000, false, error);
            assertNotNull(buffer);
            assertEquals(1, channel.bufferUsage(error));
            
            buffer = channel.getBuffer(4000, false, error);
            assertNotNull(buffer);
            assertEquals(2, channel.bufferUsage(error));
            
            buffer = channel.getBuffer(4000, false, error);
            assertNotNull(buffer);
            initChannel.close(error);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 2.
     * 
     * The connect returns SUCCESS. The bufferUsage() returns 0.
     * 
     * The buffer1 is received by calling getBuffer() with the buffer size
     * bigger then fragment size/2. The buffer is released.
     * 
     * The bufferUsage goes from 1 to 0.
     * 
     * The buffer1 and buffer2 are received by calling GetBuffer twice with the
     * same size. The buffers are released.
     * 
     * The bufferUsage() returns 1, 2, 1, 0.
     * 
     */
    @SuppressWarnings("deprecation")
	@Test
    public void allocateAndReleaseBuffersTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 2);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer1 = channel.getBuffer(4000, false, error);
            assertNotNull(buffer1);
            assertEquals(1, channel.bufferUsage(error));
            
            channel.releaseBuffer(buffer1, error);
            assertEquals(1, channel.bufferUsage(error));            
            
            buffer1 = channel.getBuffer(4000, false, error);
            assertNotNull(buffer1);
            assertEquals(1, channel.bufferUsage(error));
            
            TransportBuffer buffer2 = channel.getBuffer(4000, false, error);
            assertNotNull(buffer2);
            assertEquals(2, channel.bufferUsage(error));
            
            TransportBuffer buffer3 = channel.getBuffer(4000, false, error);
            assertNotNull(buffer3);
            
            buffer3 = channel.getBuffer(4000, false, error);
            assertNotNull(buffer3);
            buffer3 = channel.getBuffer(4000, false, error);
            assertNotNull(buffer3);
            assertEquals(5, channel.bufferUsage(error));            
            buffer3 = channel.getBuffer(4000, false, error);
            assertEquals(5, channel.bufferUsage(error));
            // verify returned buffer is null and error code is NO_BUFFERS
            assertNull(buffer3);
            assertEquals(TransportReturnCodes.NO_BUFFERS, error.errorId());
            channel.releaseBuffer(buffer2, error);
            assertEquals(4, channel.bufferUsage(error));            
            
            channel.releaseBuffer(buffer1, error);
            assertEquals(3, channel.bufferUsage(error));
            initChannel.close(error);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 2.
     * 
     * The connect returns SUCCESS. The bufferUsage() returns 0.
     * 
     * Three buffers are received with size of 2045.
     * The bufferUsage is 1 after each getBuffer() call.
     * 
     * The forth buffer is received with the same size.
     * The bufferUsage is 2.
     * 
     * Release the first buffer.
     * The bufferUsage() returns 2.
     * 
     * Release the fourth buffer.
     * The bufferUsage() returns 1.
     * 
     * Release the second buffer.
     * The bufferUsage() returns 1.
     * 
     * Release the third buffer.
     * The bufferUsage() returns 0.
     * 
     */
    @SuppressWarnings("deprecation")
	@Test
    public void slice3BuffersTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 2);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer1 = channel.getBuffer(2045, false, error);
            assertNotNull(buffer1);
            assertEquals(1, channel.bufferUsage(error));
            
            TransportBuffer buffer2 = channel.getBuffer(2045, false, error);
            assertNotNull(buffer2);
            assertEquals(1, channel.bufferUsage(error));
            
            TransportBuffer buffer3 = channel.getBuffer(2045, false, error);
            assertNotNull(buffer3);
            assertEquals(1, channel.bufferUsage(error));
            
            TransportBuffer buffer4 = channel.getBuffer(2045, false, error);
            assertNotNull(buffer4);
            assertEquals(2, channel.bufferUsage(error));
            
            channel.releaseBuffer(buffer1, error);
            assertEquals(2, channel.bufferUsage(error));            
            
            channel.releaseBuffer(buffer4, error);
            assertEquals(2, channel.bufferUsage(error));            
            
            channel.releaseBuffer(buffer2, error);
            assertEquals(2, channel.bufferUsage(error));            
            
            channel.releaseBuffer(buffer3, error);
            assertEquals(1, channel.bufferUsage(error));
            initChannel.close(error);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 2.
     * 
     * The connect returns SUCCESS. The bufferUsage() returns 0.
     * 
     * Two buffers are received with size of 2045.
     * The bufferUsage is 1 after each getBuffer() call.
     * 
     * The third buffer is received with size of 2046.
     * The bufferUsage is 2.
     * 
     * Release the first buffer.
     * The bufferUsage() returns 2.
     * 
     * Release the second buffer.
     * The bufferUsage() returns 1.
     * 
     * Release the third buffer.
     * The bufferUsage() returns 0.
     * 
     */
    @SuppressWarnings("deprecation")
	@Test
    public void slice2BuffersTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 2);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer1 = channel.getBuffer(2046, false, error);
            assertNotNull(buffer1);
            assertEquals(1, channel.bufferUsage(error));
            
            TransportBuffer buffer2 = channel.getBuffer(2046, false, error);
            assertNotNull(buffer2);
            assertEquals(1, channel.bufferUsage(error));
            
            TransportBuffer buffer3 = channel.getBuffer(2047, false, error);
            assertNotNull(buffer3);
            assertEquals(2, channel.bufferUsage(error));
            
            channel.releaseBuffer(buffer1, error);
            assertEquals(2, channel.bufferUsage(error));            
            
            channel.releaseBuffer(buffer2, error);
            assertEquals(1, channel.bufferUsage(error));            
            
            channel.releaseBuffer(buffer3, error);
            assertEquals(1, channel.bufferUsage(error)); 
            initChannel.close(error);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 2.
     * 
     * The connect returns SUCCESS. 
     * 
     * The getBuffer() is called twice. The buffer size
     * requested is bigger then fragment size/2.
     * 
     * The buffers are released. 
     * 
     * The getBuffer() is called with the size bigger then fragment.
     * A valid buffer is returned and bufferUsage() returns 1. The
     * buffer is different from the buffers allocated previously.
     * 
     */
    @SuppressWarnings("deprecation")
	@Test
    public void bigBufferBaseTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 2);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer1 = channel.getBuffer(4000, false, error);
            assertNotNull(buffer1);
            assertEquals(1, channel.bufferUsage(error));
            
            TransportBuffer buffer2 = channel.getBuffer(4000, false, error);
            assertNotNull(buffer2);
            assertEquals(2, channel.bufferUsage(error));
            
            channel.releaseBuffer(buffer1, error);
            channel.releaseBuffer(buffer2, error);
            
            TransportBuffer buffer = channel.getBuffer(10000, false, error);
            assertNotNull(buffer);
            assertNotSame(buffer, buffer1);
            assertNotSame(buffer, buffer2);
            initChannel.close(error);
            
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 2.
     * 
     * The connect returns SUCCESS. 
     * 
     * The getBuffer() is called three times with the size bigger 
     * then fragment.
     * A valid buffer is returned twice and bufferUsage() returns 1 and 2. 
     * The third call returns null.
     * 
     * The buffers are released. Call to bufferUsage returns 0.
     * 
     */
    @SuppressWarnings("deprecation")
	@Test
    public void big3BufferTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 2);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            ((RsslSocketChannel)channel)._isJunitTest = true;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer1 = channel.getBuffer(10000, false, error);
            assertNotNull(buffer1);
            assertEquals(1, channel.bufferUsage(error));
            
            TransportBuffer buffer2 = channel.getBuffer(10000, false, error);
            assertNotNull(buffer2);
            assertEquals(2, channel.bufferUsage(error));
            
            TransportBuffer buffer = channel.getBuffer(10000, false, error);
            assertNotNull(buffer);
            
            channel.releaseBuffer(buffer1, error);
            channel.releaseBuffer(buffer2, error);

            assertEquals(1, channel.bufferUsage(error));
            initChannel.close(error);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }
    
    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 4.
     * 
     * The connect returns SUCCESS. 
     * 
     * The getBuffer() is called three times with the sizes: 10000, 10000, 
     * 10000, 10001.
     * Valid buffers are returned. 
     * 
     * The buffers are released. 
     * getBuffer with size 10001 returns the second buffer. 
     * The buffer is released.
     * getBuffer with size 10002 returns the third buffer. 
     * The buffer is released.
     * getBuffer with size 10003 returns the forth buffer. 
     * The buffer is released.
     * getBuffer with size 10000 returns the first buffer. 
     * The buffer is released. 
     * 
     */
    @SuppressWarnings("deprecation")
	@Test
    public void bigBufferSizeTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(4);

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 4);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            ((RsslSocketChannel)channel)._isJunitTest = true;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer1 = channel.getBuffer(10000, false, error);
            assertNotNull(buffer1);
            assertEquals(1, channel.bufferUsage(error));
            
            TransportBuffer buffer2 = channel.getBuffer(10000, false, error);
            assertNotNull(buffer2);
            assertEquals(2, channel.bufferUsage(error));
            
            TransportBuffer buffer3 = channel.getBuffer(10000, false, error);
            assertNotNull(buffer3);
            assertEquals(3, channel.bufferUsage(error));
            
            TransportBuffer buffer4 = channel.getBuffer(10001, false, error);
            assertNotNull(buffer4);
            assertEquals(4, channel.bufferUsage(error));
            
            channel.releaseBuffer(buffer1, error);
            channel.releaseBuffer(buffer2, error);
            channel.releaseBuffer(buffer3, error);
            channel.releaseBuffer(buffer4, error);

            TransportBuffer buffer = channel.getBuffer(10001, false, error);
            assertEquals(buffer, buffer1);
            channel.releaseBuffer(buffer, error);
            
            buffer = channel.getBuffer(10002, false, error);
            assertEquals(buffer, buffer2);
            channel.releaseBuffer(buffer, error);
            
            buffer = channel.getBuffer(10003, false, error);
            assertEquals(buffer, buffer3);
            channel.releaseBuffer(buffer, error);
            
            buffer = channel.getBuffer(10000, false, error);
            assertEquals(buffer, buffer4);
            channel.releaseBuffer(buffer, error);
            initChannel.close(error);
            
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 5.
     * 
     * The connect returns SUCCESS. 
     * 
     * The getBuffer() is called with the buffer size
     * of 3 times 6144 + 100.
     * The bufferUsage call returns 1.
     * 
     * The buffers position is set to the requested size - 50. 
     * 
     * The write() is called.
     * 
     * The getBuffer() is called with the buffer size
     * greater than the fragment size. The same buffer is
     * returned, as previously. 
     * 
     */
    @SuppressWarnings("deprecation")
	@Test
    public void bigBufferWriteFragmentedTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(5);
        WriteArgs wa = TransportFactory.createWriteArgs();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 5);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer1 = channel.getBuffer(18532, false, error);
            assertNotNull(buffer1);
            assertEquals(1, channel.bufferUsage(error));
            buffer1.data().position(18482);
 
            // This might fail, if the network is very slow (may return -9)
            assertEquals(channel.write(buffer1, wa, error), 0);
            
            TransportBuffer buffer2 = channel.getBuffer(7000, false, error);
            assertEquals(buffer1, buffer2);
            initChannel.close(error);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * In this test, the ETAJ client will connect to the TestServer and send a
     * ConnectReq. The TestServer will verify the ConnectReq as the current RIPC
     * version, for example version 13, then close the connection. The client
     * will fall back to RIPC version 12 and send a ConnectReq. The TestServer
     * will verify the ConnectReq and send a complete ConnectAck in a single
     * packet. initChannel() will be called until the channel is active
     * (indicating that the client received a valid ConnectAck), or until a
     * timeout occurs.
     * 
     * Assuming that the connect returns SUCCESS: 
     * 
     * The getBuffer() is called with the buffer size
     * of 3 times 6144 + 100.
     * The bufferUsage call returns 1.
     * 
     * The buffers position is set to the requested size - 50. 
     * 
     * The write() is called.
     * 
     * The getBuffer() is called with the buffer size
     * greater than the fragment size. The same buffer is
     * returned, as previously. 
     * 
     */
    @Test
    public void bigBufferWriteFragmentedRipc12Test()
    {
        /*
         * NOTE: The initial part of this test (which completes the RIPC
         * handshake) was taken copied ripcFallBack()
         */

        int fallBackRipcVersion = 12; 
        boolean noProtocolsExpectedFlag = false;
        String inputFile = RIPC_BASE_TEST_DATA_DIR_NAME
                + "/40_input_connectAck_Ripc12.txt";
        boolean DEBUG = false;
        
        NetworkReplay replay = null;
        InProgInfo inProg = TransportFactory.createInProgInfo();
        Error error = TransportFactory.createError();
        Channel consumerChannel = null;

        TestServer server = new TestServer(DEFAULT_LISTEN_PORT);
        Thread tServer = new Thread(server);
        tServer.start();

        try
        {
            replay = RipcHandshakeJunitTest.parseReplayFile(inputFile);

            RipcHandshakeJunitTest.initTransport(false); // initialize RSSL

            server.wait(TestServer.State.SETUP);

            // make ETA call to connect to server.
            if ((consumerChannel = RipcHandshakeJunitTest.connectToRsslServer("localhost",
                                                       String.valueOf(DEFAULT_LISTEN_PORT), error)) == null)
            {
                fail("Unable to connect to RSSL server: <" + error.text() + ">");
            }

            server.wait(TestServer.State.ACCEPTED);

            /* we want to force the client to fall back to fallBackRipcVersion */
            int fallBackCount = CURRENT_RIPC_VERSION - fallBackRipcVersion;
            int expectedRipcVersion = CURRENT_RIPC_VERSION;
            int channelState = ChannelState.INACTIVE;
            int retry;

            /*
             * this outside loop controls how many times we need to close the
             * connection to reject the client's connectReq
             */

            while (fallBackCount-- >= 0)
            {
                /*
                 * call initChannel() until we have something to read (the
                 * ConnectReq)
                 */
                retry = 5;
                while (retry-- > 0)
                {
                    if (DEBUG)
                    {
                        System.out.println("DEBUG: fallBackcount=" + fallBackCount + " retry="
                                + retry);
                    }
                    // initChannel should send connectReq to our server.
                    channelState = RipcHandshakeJunitTest.initChannel(consumerChannel, error, inProg);
                    assertEquals(ChannelState.INITIALIZING, channelState);

                    if (server.check(TestServer.State.READABLE))
                        break;
                    else
                        Thread.sleep(SLEEPTIMEMS);
                }

                server.wait(TestServer.State.READABLE);

                if (fallBackCount == -1)
                {
                    if (DEBUG)
                    {
                        System.out.println("DEBUG: fallBackCount == -1, getting out of while loop");
                    }
                    /* this should be the ConnectReq we want */
                    break;
                }

                server.readMessageFromSocket();
                if (DEBUG)
                {
                    System.out.println("DEBUG: expectedRipcVersion="
                                    + expectedRipcVersion
                                    + "\nhex:"
                                    + Transport.toHexString(server.buffer(), 0, server.buffer()
                                            .position()));
                }

                RipcHandshakeJunitTest.compareExpectedConnectReq(server.buffer(), expectedRipcVersion--, Transport.queryVersion().productVersion());

                if (DEBUG)
                {
                    System.out.println("DEBUG: closing socket");
                }
                server.closeSocket();

                if (DEBUG)
                {
                    System.out.println("DEBUG: waiting for server to accept new connection");
                }

                /* ETAJ should establish a new connection. */

                retry = 1000 / SLEEPTIMEMS;
                while (retry-- > 0)
                {
                    if (DEBUG)
                    {
                        System.out.println("DEBUG: waiting for server to accept new connection, retry="
                                        + retry);
                    }

                    // initChannel should send connectReq to our server.
                    channelState = RipcHandshakeJunitTest.initChannel(consumerChannel, error, inProg);
                    if (noProtocolsExpectedFlag && fallBackCount == 0
                            && channelState == ChannelState.INACTIVE)
                    {
                        // This test should fall back to no more protocols to
                        // try.
                        return;
                    }
                    else
                    {
                        assertEquals(ChannelState.INITIALIZING, channelState);

                        if (server.check(TestServer.State.ACCEPTED))
                            break;
                        else
                            Thread.sleep(SLEEPTIMEMS);
                    }
                }

                if (noProtocolsExpectedFlag && fallBackCount == 0)
                {
                    fail("test is expecting to fall back to no more protocols, but the channelState never transitioned to INACTIVE.");
                }

                if (DEBUG)
                {
                    System.out.println("DEBUG: server accepted new connection");
                }
            }

            server.readMessageFromSocket();
            if (DEBUG)
            {
                System.out.println("DEBUG2: expectedRipcVersion=" + expectedRipcVersion + "\nhex:"
                        + Transport.toHexString(server.buffer(), 0, server.buffer().position()));
            }

            RipcHandshakeJunitTest.compareExpectedConnectReq(server.buffer(), expectedRipcVersion, Transport.queryVersion().productVersion());

            if (DEBUG)
            {
                System.out.println("DEBUG: writing ConnectAck to our client");
            }
            // write ConnectAck to our client.
            server.writeMessageToSocket(replay.read());

            retry = 5;
            while (retry-- > 0)
            {
                // initChannel should read connectAck.
                // wait for the channel to become active
                channelState = RipcHandshakeJunitTest.initChannel(consumerChannel, error, inProg);
                if (channelState == ChannelState.ACTIVE)
                    break;
                else
                    Thread.sleep(SLEEPTIMEMS);
            }
            assertEquals(ChannelState.ACTIVE, channelState);
            assertNull(error.text());

            if (fallBackRipcVersion >= 13)
            {
                RipcHandshakeJunitTest.compareComponentVersionToReceivedComponentInfo(receivedComponentVersion, consumerChannel, error);
            }
            
            
            /*
             * Now to test BigBuffer and RIPC12 fragmentation.
             */
            WriteArgs wa = TransportFactory.createWriteArgs();
            TransportBuffer buffer1 = consumerChannel.getBuffer(18532, false, error);
            assertNotNull(buffer1);
            assertEquals(1, consumerChannel.bufferUsage(error));
            buffer1.data().position(18482);
 
            // This might fail, if the network is very slow (may return -9)
            assertEquals(consumerChannel.write(buffer1, wa, error), 0);
            
            TransportBuffer buffer2 = consumerChannel.getBuffer(7000, false, error);
            assertEquals(buffer1, buffer2);

        
        }
        catch (IOException | InterruptedException e)
        {
            System.out.println(e.toString());
            fail(e.getLocalizedMessage());
        }
        finally
        {
            RipcHandshakeJunitTest.cleanupForTestsWithTestServer(replay, consumerChannel, server, tServer, error);
        }

        try
        {
            tServer.join();
        }
        catch (InterruptedException e)
        {
            fail("tServer.join() failed, error=" + e.toString());
        }
    }

    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 2.
     * 
     * The connect returns SUCCESS. 
     * 
     * The getBuffer() is called with the buffer size
     * of 3 times 6144 + 100.
     * The bufferUsage call returns 1.
     * 
     * The buffers position is set to the requested size - 50. 
     * 
     * The write() returns the number of bytes not flushed yet (6218).
     * The getBuffer() called with the buffer size
     * greater than the fragment size returns null.
     * 
     *  After 10msec the flush() is called.
     * 
     * The getBuffer() is called with the buffer size
     * greater than the fragment size. The same buffer is
     * returned, as previously. 
     * 
     */
    @SuppressWarnings("deprecation")
	@Test
    public void bigBufferWriteWithFlushFragmentedTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);
        WriteArgs wa = TransportFactory.createWriteArgs();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 2);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer1 = channel.getBuffer(18532, false, error);
            assertNotNull(buffer1);
            assertEquals(1, channel.bufferUsage(error));
            buffer1.data().position(18482);
 
            TransportBuffer buffer2;
            int ret = channel.write(buffer1, wa, error);
            if (ret == 0)
            {
            	// everything was sent over network
                buffer2 = channel.getBuffer(7000, false, error);
                assertEquals(buffer1, buffer2);            	
            }
            else if (ret == TransportReturnCodes.WRITE_CALL_AGAIN)
            {
                try 
            	{
            		Thread.sleep(10);
            	} 
            	catch (InterruptedException e) 
            	{
            		e.printStackTrace();
            	}
                buffer2 = channel.getBuffer(7000, false, error);
                assertNull(buffer2);
                assertEquals(TransportReturnCodes.NO_BUFFERS, error.errorId());
                ret = channel.write(buffer1, wa, error);
            }
            
            if (ret != 0)
            {
            	while (channel.flush(error) != 0)
            	{
            		try 
            		{
            			Thread.sleep(10);
            		} 
            		catch (InterruptedException e) 
            		{
            			e.printStackTrace();
            		}
            	}

            	buffer2 = channel.getBuffer(7000, false, error);
            	assertEquals(buffer1, buffer2);
            }
            initChannel.close(error);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 5.
     * 
     * The connect returns SUCCESS. 
     * 
     * The getBuffer() is called with the buffer size
     * of 3 times 6144 + 100.
     * The bufferUsage call returns 1.
     * 
     * The buffers position is set to 6141. 
     * 
     * The write() is called.
     * The bufferUsage returns 1.
     * 
     * The getBuffer() is called with the buffer size
     * greater than the fragment size. The same buffer is
     * returned, as previously. 
     * 
     */
    @SuppressWarnings("deprecation")
	@Test
    public void bigBufferWriteRipcTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(5);
        WriteArgs wa = TransportFactory.createWriteArgs();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 2);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer1 = channel.getBuffer(18532, false, error);
            assertNotNull(buffer1);
            assertEquals(1, channel.bufferUsage(error));
            buffer1.data().position(5997);
            
            assertEquals(6000, channel.write(buffer1, wa, error));
            assertEquals(1, channel.bufferUsage(error));  // did not reach high water mark
            
            TransportBuffer buffer2 = channel.getBuffer(7000, false, error);
            assertEquals(buffer1, buffer2);
            initChannel.close(error);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 5.
     * 
     * The connect returns SUCCESS. 
     * 
     * The getBuffer() is called with the buffer size
     * of 3 times 6144 + 100.
     * The bufferUsage call returns 1.
     * 
     * The buffers position is set to 6141. 
     * 
     * The write() is called.
     * The bufferUsage returns 1.
     * 
     * The getBuffer() is called with the buffer size
     * greater than the fragment size. The same buffer is
     * returned, as previously. 
     * 
     */
    @SuppressWarnings("deprecation")
	@Test
    public void bigBufferPoolTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        WriteArgs wa = TransportFactory.createWriteArgs();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 2);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            ((RsslSocketChannel)channel)._isJunitTest = true;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer1 = channel.getBuffer(12388, false, error);
            assertNotNull(buffer1);
            assertEquals(24588, buffer1.data().capacity());
            assertEquals(1, channel.bufferUsage(error));
            assertEquals(null, ((RsslSocketChannel)channel)._bigBuffersPool._pools[0]);
            assertNotNull(((RsslSocketChannel)channel)._bigBuffersPool._pools[1]);
            assertEquals(0, ((RsslSocketChannel)channel)._bigBuffersPool._pools[1]._queue._size);
            assertEquals(24588, ((RsslSocketChannel)channel)._bigBuffersPool._maxSize);
            buffer1.data().position(6600);
            
            assertEquals(0, channel.write(buffer1, wa, error));
            assertEquals(0, channel.bufferUsage(error));
            assertEquals(1, ((RsslSocketChannel)channel)._bigBuffersPool._pools[1]._queue._size);
            
            TransportBuffer buffer2 = channel.getBuffer(6244, false, error);
            assertNotNull(buffer2);
            assertEquals(buffer1, buffer2);
            assertEquals(1, channel.bufferUsage(error));
            assertEquals(null, ((RsslSocketChannel)channel)._bigBuffersPool._pools[0]);
            assertNotNull(((RsslSocketChannel)channel)._bigBuffersPool._pools[1]);
            assertEquals(0, ((RsslSocketChannel)channel)._bigBuffersPool._pools[1]._queue._size);
            buffer2.data().position(6200);
            channel.releaseBuffer(buffer2, error);
            assertEquals(0, channel.bufferUsage(error));
            assertEquals(1, ((RsslSocketChannel)channel)._bigBuffersPool._pools[1]._queue._size);
            
            buffer1 = channel.getBuffer(6244, false, error);
            buffer2 = channel.getBuffer(6244, false, error);
            assertEquals(2, channel.bufferUsage(error));
            assertEquals(24588, buffer1.data().capacity());
            assertEquals(12294, buffer2.data().capacity());
            assertNotNull(((RsslSocketChannel)channel)._bigBuffersPool._pools[0]);
            assertNotNull(((RsslSocketChannel)channel)._bigBuffersPool._pools[1]);
            assertEquals(0, ((RsslSocketChannel)channel)._bigBuffersPool._pools[0]._queue._size);
            assertEquals(0, ((RsslSocketChannel)channel)._bigBuffersPool._pools[1]._queue._size);
            
            buffer1.data().position(6100);
            assertEquals(6103, channel.write(buffer1, wa, error));
            assertEquals(2, channel.bufferUsage(error));
            buffer2.data().position(6103);
            assertEquals(0, channel.write(buffer2, wa, error));
            assertEquals(1, ((RsslSocketChannel)channel)._bigBuffersPool._pools[1]._queue._size);
            assertEquals(0, channel.bufferUsage(error));
            assertEquals(1, ((RsslSocketChannel)channel)._bigBuffersPool._pools[0]._queue._size);
            
            initChannel.close(error);
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 2.
     * 
     * The connect returns SUCCESS. 
     * 
     * The getBuffer() with packing set to true is called on the channel. The buffer size
     * requested is equal to fragment size - 4 (non pack RIPC header+1).
     * 
     * The call returns null.
     */
    @SuppressWarnings("deprecation")
	@Test
    public void packBuffersNullTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
            channel = Transport.connect(connectOpts, error);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer = channel.getBuffer(6140, true, error);
            assertNull(buffer);
            assertEquals(TransportReturnCodes.NO_BUFFERS, error.errorId());
        }
        finally
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /*
     * A SOCKET transport is initialized
     * A channel is created on connect with connectOptions
     * where Guaranteed Output Buffers option is set to 2.
     * 
     * The connect returns SUCCESS. 
     * 
     * The getBuffer() with packing set to true is called on the channel. The buffer size
     * requested is equal to fragment size - 5.
     * 
     * The call returns a buffer.
     * 
     * Data is encoded into the buffer and packed twice. The data is verified.
     * 
     */
    @SuppressWarnings("deprecation")
	@Test
    public void packBuffersTest()
    {
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);
		byte [] data = {0,1,2,3,4,5,6,7};
		Buffer buf = CodecFactory.createBuffer();
		buf.data(ByteBuffer.wrap(data));
		EncodeIterator encIter = CodecFactory.createEncodeIterator();
		encIter.clear();

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
        	// need to create ProtocolInt for Socket, so need to connect
        	Channel initChannel = Transport.connect(connectOpts, error);
        	assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 2);
            assertNotNull(channel);
            try
            {
            	while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            TransportBuffer buffer = channel.getBuffer(6139, true, error);
            assertNotNull(buffer);
    		encIter.setBufferAndRWFVersion(buffer, Codec.majorVersion(), Codec.minorVersion());
    		
    		buf.encode(encIter);
    		assertEquals(13, buffer.data().position());
    		assertTrue(channel.packBuffer(buffer, error) > 0);
    		assertEquals(15, buffer.data().position());
    		buf.encode(encIter);
    		assertEquals(23, buffer.data().position());
    		assertTrue(channel.packBuffer(buffer, error) > 0);
    		assertEquals(25, buffer.data().position());
    		byte [] verifyData = {0, 0, 0, 0, 0x08, 0,1,2,3,4,5,6,7, 0, 0x08, 0,1,2,3,4,5,6,7};
    		for (int i=0; i<verifyData.length; i++)
    		{
    			assertEquals(verifyData[i], buffer.data().get(i));
    		}
    		initChannel.close(error);
        }
        finally
        {
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
    }

    /* Test the ETAQueue implementation. */
    @Test
    public void etaQueueTest()
    {
    	EtaQueue qe = new EtaQueue();
    	qe.clear();
    	assertEquals(0, qe._size);
    	assertEquals(null, qe._head);
    	assertEquals(null, qe._tail);    	
    	
    	EtaNode node = new EtaNode();
    	qe.add(node);
    	assertEquals(1, qe._size);
    	assertEquals(node, qe._head);
    	assertEquals(node, qe._tail);    	
    	qe.clear();
    	assertEquals(0, qe._size);
    	assertEquals(null, qe._head);
    	assertEquals(null, qe._tail);    	
    	
    	EtaNode node1 = new EtaNode();
    	qe.add(node);
    	qe.add(node1);
    	assertEquals(2, qe._size);
    	assertEquals(node, qe._head);
    	assertEquals(node1, qe._tail);    	
    	qe.clear();
    	assertEquals(0, qe._size);
    	assertEquals(null, qe._head);
    	assertEquals(null, qe._tail);    	
    	    	
    	EtaNode node2 = new EtaNode();
        EtaNode node3 = new EtaNode();
        EtaNode node4 = new EtaNode();
        EtaNode node5 = new EtaNode();
    	qe.add(node);
    	qe.add(node1);
    	qe.add(node2);
    	assertEquals(3, qe._size);
    	assertEquals(node, qe._head);
    	assertEquals(node2, qe._tail);    	
    	assertEquals(node, qe.poll());
    	assertEquals(2, qe._size);
        qe.add(node3);
        qe.add(node4);
        assertEquals(node1, qe._head);
    	assertEquals(node4, qe._tail);    	
        assertEquals(4, qe._size);
        assertEquals(node1, qe.poll());
    	assertEquals(3, qe._size);
    	assertEquals(node2, qe._head);
    	assertEquals(node4, qe._tail);    	
    	assertEquals(node2, qe.poll());
    	assertEquals(node3, qe.poll());
    	assertEquals(1, qe._size);
    	qe.add(node5);
        assertEquals(node4, qe.poll());
        assertEquals(node5, qe.poll());
        assertEquals(0, qe._size);
    	assertEquals(null, qe._head);
    	assertEquals(null, qe._tail);  	
        qe.add(node);
        qe.add(node1);
        qe.add(node2);
        assertEquals(3, qe._size);
        assertEquals(node, qe._head);
        assertEquals(node2, qe._tail);      
        assertEquals(node, qe.poll());
        assertEquals(2, qe._size);
        qe.add(node3);
        qe.add(node4);
        assertEquals(node1, qe._head);
        assertEquals(node4, qe._tail);      
        assertEquals(4, qe._size);
        assertEquals(node1, qe.poll());
        assertEquals(3, qe._size);
        assertEquals(node2, qe._head);
        assertEquals(node4, qe._tail);      
        assertEquals(node2, qe.poll());
        assertEquals(node3, qe.poll());
        assertEquals(1, qe._size);
        qe.add(node5);
        assertEquals(node4, qe.poll());
        assertEquals(node5, qe.poll());
        assertEquals(0, qe._size);
        assertEquals(null, qe._head);
        assertEquals(null, qe._tail);    	    	
    }
    
    /*
     * Tests for verifying read and write buffer capacity method.
     */
    @SuppressWarnings("deprecation")
	@Test
    public void getCapacityTest()
    {
        // 1. read buffer capacity
        final int BUFFER_SIZE = 1000;
        final int LIMIT = 500;
        TransportBufferImpl readBuf = new TransportBufferImpl(BUFFER_SIZE);
        
        //initial capacity = buffer size
        assertEquals(readBuf.capacity(), BUFFER_SIZE);
       
        ByteBuffer backingByteBuffer = readBuf.data();
        for (int i = 0; i < BUFFER_SIZE; i++)
        {
            backingByteBuffer.put((byte)(i % 256)); // 0-255
        }
      
        //capacity remains the same
        assertEquals(readBuf.capacity(), BUFFER_SIZE);
        
        //capacity after new limit
        backingByteBuffer.limit(LIMIT);
        assertEquals(readBuf.capacity(), LIMIT);
        
        // 2. write buffer capacity
        final Error error = TransportFactory.createError();
        Channel channel = null;
        ConnectOptions connectOpts = getDefaultConnectOptions();
        connectOpts.guaranteedOutputBuffers(2);

        try
        {
            InitArgs initArgs = TransportFactory.createInitArgs();
            initArgs.globalLocking(false);
            assertEquals(TransportReturnCodes.SUCCESS,
                         Transport.initialize(initArgs, error));
            // need to create ProtocolInt for Socket, so need to connect
            Channel initChannel = Transport.connect(connectOpts, error);
            assertNotNull(initChannel);
            channel = getNetworkReplayChannel(Transport._transports[0], 2);
            assertNotNull(channel);
            try
            {
                while (!((RsslSocketChannel)channel).scktChannel().finishConnect()) {}
            } catch (Exception e) {}
            ((RsslSocketChannel)channel)._state = ChannelState.ACTIVE;
            int errorCode = error.errorId();
            assertEquals(TransportReturnCodes.SUCCESS, errorCode);
            assertEquals(0, channel.bufferUsage(error));
            
            //write buffer capacity
            TransportBuffer buffer = channel.getBuffer(4000, false, error);
            assertNotNull(buffer);

            TransportBufferImpl writeBuf = (TransportBufferImpl)buffer;
            
            //capacity = size from getBuffer
            assertEquals(writeBuf.capacity(), 4000); 

            for (int i = 0; i < 100; i++)
            {
                writeBuf._data.put((byte)i);
            }
            
            //capacity remains size from getBuffer
            assertEquals(writeBuf.capacity(), 4000);  
            initChannel.close(error);
            
            //capacity after new limit
            writeBuf._data.limit(LIMIT);
            assertEquals(writeBuf.capacity(), LIMIT - 3); //capacity = New LIMIT minus 3 bytes reserved for ripc header 
        }
        finally
        {
            if (channel != null)
                channel.close(error);
            assertEquals(TransportReturnCodes.SUCCESS, Transport.uninitialize());
        }
     }
}
