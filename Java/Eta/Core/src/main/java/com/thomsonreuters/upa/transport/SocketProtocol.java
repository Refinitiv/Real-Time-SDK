package com.thomsonreuters.upa.transport;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.NotYetConnectedException;
import java.util.ArrayList;
import java.util.HashMap;

import com.thomsonreuters.upa.transport.AcceptOptions;
import com.thomsonreuters.upa.transport.BindOptions;
import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.ChannelState;
import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.Error;
import com.thomsonreuters.upa.transport.Server;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

import com.thomsonreuters.upa.test.network.replay.NetworkReplay;
import com.thomsonreuters.upa.test.network.replay.NetworkReplayFactory;

class SocketProtocol implements ProtocolInt 
{
    class TrackingPool extends Pool
    {
        TrackingPool(Object o)
        {
            super(o);
        }

        // This pool is used for channels and servers.
        // It keeps track of the channels/servers that are active in addition to the channels/servers that are available.
        final UpaQueue _active = new UpaQueue();

        @Override
        void add(UpaNode node)
        {
            // remove the node from _active queue
            if (_active.size() > 0)
            {
                _active.remove(node);
            }

            // add back to channel queue
            super.add(node);
        }
    }

    private static final int NUMBER_SOCKET_CHANNELS = 2;
    private static final int NUMBER_SERVERS = 1;
    final Pool _channelPool = new TrackingPool(this);
    final Pool _serverPool = new TrackingPool(this);
    final HashMap<Integer, Pool> _writeBufferChannelPools = new HashMap<Integer, Pool>();
    final ArrayList<RsslSocketChannel> _busyList = new ArrayList<RsslSocketChannel>();
    private RsslSocketChannel channel;

    // if _http true, then we have HTTP on the corresponding channel
    private boolean _http = false;

    // if _encrypted true, then we have Encrypted messages on the corresponding channel
    private boolean _encrypted = false;

    SocketProtocol()
    {
        // The global lock is locked by Transport

        // create pools used by this transport
        // 1. add channels to channel pool
        for (int i = 0; i < NUMBER_SOCKET_CHANNELS; i++)
        {
            RsslSocketChannel channel = new RsslSocketChannel(this, _channelPool);
            channel.returnToPool();
        }

        // 2. buffers will be added to the pool when connected and size is known from server

        // 3. add servers to server pool
        for (int i = 0; i < NUMBER_SERVERS; i++)
        {
            ServerImpl server = new ServerImpl(this, _serverPool);
            server.returnToPool();
        }
    }

    SocketProtocol(ConnectOptions options)
    {
        // The global lock is locked by Transport

        // create pools used by this transport
        // 1. add channels to channel pool
        for (int i = 0; i < NUMBER_SOCKET_CHANNELS; i++)
        {
            // for SOCKET connectionTypes create RsslSocketChannel
            // for HTTP connectionType create RsslHttpSocketChannel
            // for ENCRYPTED connectionType create RsslEncryptedSocketChannel
            if (options.connectionType() == ConnectionTypes.SOCKET)
            {
                channel = new RsslSocketChannel(this, _channelPool);
            }
            else if (options.connectionType() == ConnectionTypes.HTTP)
            {
                _http = true;
                channel = new RsslHttpSocketChannel(this, _channelPool);
            }
            else if (options.connectionType() == ConnectionTypes.ENCRYPTED)
            {
                _http = true;
                _encrypted = true;
                channel = new RsslEncryptedSocketChannel(this, _channelPool);
            }
            else
            {
                System.out.println("Connection type " + options.connectionType() + " not implemented");
            }
            channel.returnToPool();
        }

        // 2. buffers will be added to the pool when connected and size is known from server

        // 3. add servers to server pool
        for (int i = 0; i < NUMBER_SERVERS; i++)
        {
            ServerImpl server = new ServerImpl(this, _serverPool);
            server.returnToPool();
        }
    }

    // set the HTTP flag indicating that the corresponding channel has HTTP
    void setHTTP()
    {
        _http = true;
    }

    boolean isHTTP()
    {
        return _http;
    }

    // set the Encrypted flag indicating that the corresponding channel has Encrypted
    void setEncryped()
    {
        _encrypted = true;
    }

    boolean isEncrypted()
    {
        return _encrypted;
    }

    @Override
    public Channel channel(ConnectOptions options, Error error)
    {
        // The global lock is locked by Transport
        RsslSocketChannel channel;

        // TcpOptsImpl has a "replay" property, that, if non-null specifies
        // the name of a NetworkReplay file to use (to replay data from the file
        // instead of reading data from the network)
        if (((TcpOptsImpl)options.tcpOpts()).replay() == null)
        {
            // this is the "normal" (production code) case
            channel = (RsslSocketChannel)_channelPool.poll();
            if (channel == null)
            {
                if (_encrypted)
                    channel = new RsslEncryptedSocketChannel(this, _channelPool);
                else if (_http)
                    channel = new RsslHttpSocketChannel(this, _channelPool);
                else
                    channel = new RsslSocketChannel(this, _channelPool);
            }
        }
        else
        {
            // For debugging only: create a NetworkReplayChannel and replay the data in the specified file
            channel = createReplaySocketChannel(((TcpOptsImpl)options.tcpOpts()).replay());
        }

        if (channel._http)
        {
            ((RsslHttpSocketChannel)channel)._proxyCredentails = null;
            ((RsslHttpSocketChannel)channel)._proxyAuthenticator = null;
            ((RsslHttpSocketChannel)channel)._proxyConnectResponse.setLength(0);
            ((RsslHttpSocketChannel)channel)._ignoredConnectResponses = 0;
        }
        int ret = channel.connect(options, error);
        if (ret != TransportReturnCodes.SUCCESS)
        {
            channel.resetToDefault();
            channel.returnToPool();
            return null;
        }

        ((TrackingPool)_channelPool)._active.add(channel);
        return channel;
    }
	
    /* USED ONLY FOR DEBUGGING
     * Creates a new instance of a RsslSocketChannel and overrides its RsslSocketChannel::read(ByteBuffer) method
     */
    private RsslSocketChannel createReplaySocketChannel(String replayFilename)
    {
        assert (replayFilename != null);

        final int DEFAULT_LISTEN_PORT = 14002;
        RsslSocketChannel consumerChannel = null;

        try
        {
            final NetworkReplay replay = NetworkReplayFactory.create();

            java.util.Date d = new java.util.Date();
            System.out.println(d + " parsing replay file " + replayFilename + "...");

            replay.parseFile(replayFilename);
            System.out.println(d + " read " + replay.recordsInQueue() + " records from replay file " + replayFilename);

            replay.startListener(DEFAULT_LISTEN_PORT);

            // allocate a channel and override it's read() to read from our NetworkReplay
            // NOTE: a RsslSocketChannel is *normally* constructed by via its factory
            if (_encrypted)
            {
                consumerChannel = new RsslEncryptedSocketChannel(this, _channelPool)
                {
                    @Override
                    protected int read(ByteBuffer dst) throws NotYetConnectedException, IOException
                    {
                        return replay.read(dst);
                    }
                };
            }
            else if (_http)
            {
                consumerChannel = new RsslHttpSocketChannel(this, _channelPool)
                {
                    @Override
                    protected int read(ByteBuffer dst) throws NotYetConnectedException, IOException
                    {
                        return replay.read(dst);
                    }
                };
            }
            else
            {
                consumerChannel = new RsslSocketChannel(this, _channelPool)
                {
                    @Override
                    protected int read(ByteBuffer dst) throws NotYetConnectedException, IOException
                    {
                        return replay.read(dst);
                    }
                };
            }
        }
        catch (IOException e)
        {
            throw new RuntimeException(e);
        }

        return consumerChannel;
    }
    
    @Override
    public Channel channel(AcceptOptions options, Server srvr, Object object)
    {
        // The global lock is locked by Transport
        java.nio.channels.SocketChannel socketChannel = (java.nio.channels.SocketChannel)object;
        ServerImpl server = (ServerImpl)srvr;

        RsslSocketChannel channel = null;
        _busyList.clear();

        while (true)
        {
            channel = (RsslSocketChannel)_channelPool.poll();

            if (channel == null)
            {
                channel = new RsslSocketChannel(this, _channelPool);
                // System.out.println("POOL EXPAND = " + _channelPool._queue._size + " ACTIVESIZE = " + ((TrackingPool)_channelPool)._active.size() + " iCount = " + iCount);
                break;
            }
            else if (channel._scktChannel != null && !channel.canReuseSessionId(channel._providerSessionId))
            {
                _busyList.add(channel);
                continue;
            }
            else
            {
                break;
            }
        }

        for (RsslSocketChannel chnl : _busyList)
        {
            chnl.returnToPool();
        }

        channel._server = server;
        if (channel.setChannelAccept(options, server.bindOptions(), socketChannel) < TransportReturnCodes.SUCCESS)
        {
            System.out.println("channel set accept failed. ");
            channel.returnToPool();
            return null;
        }

        ((TrackingPool)_channelPool)._active.add(channel);

        server._numChannels++;
        return channel;
    }

    @Override
    public Server server(BindOptions options, Error error)
    {
        // The global lock is locked by Transport.
        // The server is null if no more available servers are in the pool.
        ServerImpl server = (ServerImpl)_serverPool.poll();
        if (server == null)
        {
            server = new ServerImpl(this, _serverPool);
        }
        if (server.bind(options, error) != TransportReturnCodes.SUCCESS)
        {
            server.returnToPool();
            return null;
        }

        ((TrackingPool)_serverPool)._active.add(server);

        return server;
    }

    @Override
    public void uninitialize()
    {
        // The global lock is locked by Transport.
        // This does not close channels and servers.

        // The error is ignored, since the application is closing.
        Error error = new ErrorImpl();
        RsslSocketChannel channel;
        while ((channel = (RsslSocketChannel)((TrackingPool)_channelPool)._active.poll()) != null)
        {
            if (channel._state == ChannelState.ACTIVE)
                channel.close(error);
        }
        _channelPool.clear();
        ServerImpl server;
        while ((server = (ServerImpl)((TrackingPool)_serverPool)._active.poll()) != null)
        {
            server.close(error);
        }
        _serverPool.clear();
        for (Integer key : _writeBufferChannelPools.keySet())
        {
            _writeBufferChannelPools.get(key).clear();
        }
        _writeBufferChannelPools.clear();
    }

    @Override
    public Pool getPool(int poolSpec)
    {
        Pool pool = _writeBufferChannelPools.get(poolSpec);
        if (pool == null)
        {
            pool = new Pool(this);
            pool._isProtocolBuffer = true;
        }
        _writeBufferChannelPools.put(poolSpec, pool);
        return pool;
    }

}
