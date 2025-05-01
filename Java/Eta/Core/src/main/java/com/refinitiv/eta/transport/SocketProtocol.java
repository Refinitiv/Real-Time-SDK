/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.nio.channels.NotYetConnectedException;
import java.util.ArrayList;
import java.util.HashMap;

import com.refinitiv.eta.test.network.replay.NetworkReplay;
import com.refinitiv.eta.test.network.replay.NetworkReplayFactory;

class SocketProtocol implements ProtocolInt
{
    private int _connectionType;
    private int _subProtocol;

    class TrackingPool extends Pool
    {
        TrackingPool(Object o)
        {
            super(o);
        }

        // This pool is used for channels and servers.
        // It keeps track of the channels/servers that are active in addition to the channels/servers that are available.
        final EtaQueue _active = new EtaQueue();

        @Override
        void add(EtaNode node)
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
    
    SocketProtocol()
    {
        _connectionType = ConnectionTypes.SOCKET;
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

    SocketProtocol(BindOptions options)
    {
        _connectionType = options.connectionType();
        if(_connectionType == ConnectionTypes.ENCRYPTED)
        {
            /* This is just to make sure everything flows through cleanly. */
            _subProtocol = ConnectionTypes.SOCKET;
        }

        // The global lock is locked by Transport

        // create pools used by this transport
        // 1. add channels to channel pool
        for (int i = 0; i < NUMBER_SOCKET_CHANNELS; i++)
        {
            RsslSocketChannel channel = createChannel();
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

        _connectionType = options.connectionType();
        if(_connectionType == ConnectionTypes.ENCRYPTED)
        {
            if(options.tunnelingInfo().tunnelingType().equals("None"))
            {
                _subProtocol = options.encryptionOptions().connectionType();
            }
            else
            {
                _subProtocol = ConnectionTypes.HTTP;
            }
        }

        // create pools used by this transport
        // 1. add channels to channel pool
        for (int i = 0; i < NUMBER_SOCKET_CHANNELS; i++)
        {
            RsslSocketChannel channel = createChannel();
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

    boolean isHTTP()
    {
        return _connectionType == ConnectionTypes.HTTP || _subProtocol == ConnectionTypes.HTTP;
    }

    boolean isEncrypted()
    {
        return _connectionType == ConnectionTypes.ENCRYPTED;
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
                channel = createChannel();
            }
        }
        else
        {
            // For debugging only: create a NetworkReplayChannel and replay the data in the specified file
            channel = createReplaySocketChannel(((TcpOptsImpl)options.tcpOpts()).replay(), options.connectionType());
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

    private RsslSocketChannel createChannel()
    {
        if (_connectionType == ConnectionTypes.ENCRYPTED)
            if(_subProtocol == ConnectionTypes.SOCKET || _subProtocol == ConnectionTypes.WEBSOCKET)
                return new RsslSocketChannel(this, _channelPool, _connectionType, true);
            else
                return new RsslHttpSocketChannel(this, _channelPool, true);
        else if (_connectionType == ConnectionTypes.HTTP)
            return new RsslHttpSocketChannel(this, _channelPool, false);
        else if (_connectionType == ConnectionTypes.SOCKET || _connectionType == ConnectionTypes.WEBSOCKET)
            return new RsslSocketChannel(this, _channelPool, _connectionType, false);
        else
            throw new IllegalArgumentException("Connection type " + _connectionType + " not implemented");
    }

    /* USED ONLY FOR DEBUGGING
     * Creates a new instance of a RsslSocketChannel and overrides its RsslSocketChannel::read(ByteBuffer) method
     */
    private RsslSocketChannel createReplaySocketChannel(String replayFilename, int connectionType)
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
            if (_connectionType == ConnectionTypes.ENCRYPTED)
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
            else if (_connectionType == ConnectionTypes.HTTP)
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
                consumerChannel = new RsslSocketChannel(this, _channelPool, connectionType, isEncrypted())
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
    public Channel channel(AcceptOptions options, Server srvr, Object object, Error error)
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
                channel = new RsslSocketChannel(this, _channelPool, srvr.connectionType(), (srvr.connectionType() == ConnectionTypes.ENCRYPTED));
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
        if (channel.setChannelAccept(options, server.bindOptions(), socketChannel, error) < TransportReturnCodes.SUCCESS)
        {
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
