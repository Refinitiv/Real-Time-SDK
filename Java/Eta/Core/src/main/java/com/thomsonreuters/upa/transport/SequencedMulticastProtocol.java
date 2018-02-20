package com.thomsonreuters.upa.transport;

import java.util.HashMap;

public class SequencedMulticastProtocol implements ProtocolInt
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
    final Pool _channelPool = new TrackingPool(this);
    final HashMap<Integer, Pool> _writeBufferChannelPools = new HashMap<Integer, Pool>();
    private RsslSeqMulticastSocketChannel _channel;

    SequencedMulticastProtocol()
    {
        // The global lock is locked by Transport

        // create pools used by this transport
        for (int i = 0; i < NUMBER_SOCKET_CHANNELS; i++)
        {
            RsslSeqMulticastSocketChannel channel = new RsslSeqMulticastSocketChannel(this, _channelPool);
            channel.returnToPool();
        }

        // buffers will be added to the pool when connected and size is known from server
    }

    SequencedMulticastProtocol(ConnectOptions options)
    {
        for (int i = 0; i < NUMBER_SOCKET_CHANNELS; i++)
        {
            if (options.connectionType() == ConnectionTypes.SEQUENCED_MCAST)
            {
                _channel = new RsslSeqMulticastSocketChannel(this, _channelPool);
            }
            else
            {
                System.out.println("Connection type " + options.connectionType() + " not implemented");
            }
            _channel.returnToPool();
        }
        // buffers will be added to the pool when connected and size is known from server
    }

    @Override
    public Channel channel(ConnectOptions options, Error error)
    {
        // The global lock is locked by Transport
        RsslSeqMulticastSocketChannel channel = null;

        // TcpOptsImpl has a "replay" property, that, if non-null specifies
        // the name of a NetworkReplay file to use (to replay data from the file instead of reading data from the network).
        if (((TcpOptsImpl)options.tcpOpts()).replay() == null)
        {
            // this is the "normal" (production code) case
            channel = (RsslSeqMulticastSocketChannel)_channelPool.poll();
            if (channel == null)
            {
                channel = new RsslSeqMulticastSocketChannel(this, _channelPool);
            }
        }

        int ret = channel.connect(options, error);
        if (ret != TransportReturnCodes.SUCCESS)
        {
            channel.returnToPool();
            return null;
        }
        ((TrackingPool)_channelPool)._active.add(channel);
        return channel;
    }

    @Override
    public Channel channel(AcceptOptions options, Server srvr, Object object)
    {
        // The global lock is locked by Transport
        RsslSeqMulticastSocketChannel channel = (RsslSeqMulticastSocketChannel)_channelPool.poll();
        if (channel == null)
        {
            channel = new RsslSeqMulticastSocketChannel(this, _channelPool);
        }

        ((TrackingPool)_channelPool)._active.add(channel);
        return channel;
    }

    @Override
    public Pool getPool(int poolSpec)
    {
        Pool pool = _writeBufferChannelPools.get((Integer)poolSpec);
        if (pool == null)
            pool = new Pool(this);
        _writeBufferChannelPools.put((Integer)poolSpec, pool);
        return pool;
    }

    @Override
    public void uninitialize()
    {
        // The global lock is locked by Transport.
        // This does not close channels and servers.

        // The error is ignored, since the application is closing.
        Error error = new ErrorImpl();
        RsslSeqMulticastSocketChannel channel;
        while ((channel = (RsslSeqMulticastSocketChannel)((TrackingPool)_channelPool)._active.poll()) != null)
        {
            if (channel._state == ChannelState.ACTIVE)
                channel.close(error);
        }
        _channelPool.clear();

        for (Integer key : _writeBufferChannelPools.keySet())
        {
            _writeBufferChannelPools.get(key).clear();
        }
        _writeBufferChannelPools.clear();
    }

    @Override
    public Server server(BindOptions opts, Error error)
    {
        return null;
    }

}
