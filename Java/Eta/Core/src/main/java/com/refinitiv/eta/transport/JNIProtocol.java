/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

public class JNIProtocol implements ProtocolInt
{
    class TrackingPool extends Pool
    {
        TrackingPool(Object o)
        {
            super(o);
        }

        // This pool is used for channels and servers.
        // It keeps track of the channels/servers that are active
        // in addition to the channels/servers that are available.
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

    private final int LOCK_GLOBAL = 2; // ETAC - only the global lock is enabled
    boolean _isEtacInitialized; // flag to track whether ETAC is initialized

    final Pool _channelPool = new TrackingPool(this);
    final Pool _serverPool = new TrackingPool(this);
    Error _error = new ErrorImpl();

    {
        // load ETAC JNI library that contains native methods
        System.loadLibrary("rsslEtaJNI");
    }

    native int rsslInitialize(int rsslLocking, ErrorImpl error);

    native int rsslUninitialize();

    @Override
    public Channel channel(ConnectOptions opts, Error error)
    {
        try
        {
            // initialize ETAC if not yet done so
            if (!_isEtacInitialized)
            {
                int rsslLocking = 0;
                if (Transport._globalLocking)
                {
                    rsslLocking = LOCK_GLOBAL;
                }
                if (rsslInitialize(rsslLocking, (ErrorImpl)error) != TransportReturnCodes.SUCCESS)
                {
                    System.out.printf("rsslInitialize(): failed <%s>\n", error.text());
                    return null;
                }
                _isEtacInitialized = true;
            }

            // The global lock is locked by Transport
            JNIChannel channel = getChannel();
            channel.isServerChannel(false);
            channel.serviceName(opts.unifiedNetworkInfo().serviceName());
            channel.interfaceName(opts.unifiedNetworkInfo().interfaceName());

            // connect ETAC channel
            if (rsslConnect((ConnectOptionsImpl)opts, (ErrorImpl)error, channel) == TransportReturnCodes.SUCCESS)
            {
                // initialize JNIChannel
                channel.open();
                channel.readLocking(opts.channelReadLocking());
                channel.writeLocking(opts.channelWriteLocking());
                channel.pool(_channelPool);
                channel.transport(this);
                channel.initConnOptsComponentInfo(opts, error);
                channel.initComponentInfo(error);
                ((TrackingPool)_channelPool)._active.add(channel);
            }
            else
            {
                channel.returnToPool();
                channel = null;
            }

            return channel;
        }
        catch (Exception e)
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            String errorText = error.text();
            error.text("JNI rsslConnect() exception: " + errorText);
            return null;
        }
    }

    native int rsslConnect(ConnectOptionsImpl opts, ErrorImpl error, JNIChannel channel);

    @Override
    public Server server(BindOptions opts, Error error)
    {
        try
        {
            // Reliable Multicast connection on a server is not supported
            if (opts.connectionType() == ConnectionTypes.RELIABLE_MCAST)
            {
                String errorText = " Error: Reliable Multicast connection type (4) is currently not supported for a server";
                error.text("JNI rsslBind() exception: " + errorText);
                return null;
            }

            // initialize ETAC if not yet done so
            if (!_isEtacInitialized)
            {
                int rsslLocking = 0;
                if (Transport._globalLocking)
                {
                    rsslLocking = LOCK_GLOBAL;
                }
                if (rsslInitialize(rsslLocking, (ErrorImpl)error) != TransportReturnCodes.SUCCESS)
                {
                    System.out.printf("rsslInitialize(): failed <%s>\n", error.text());
                    return null;
                }
                _isEtacInitialized = true;
            }

            // The global lock is locked by Transport
            JNIServer server = getServer();
            server.open();
            server.bindOptions(opts);

            // bind ETAC server
            if (rsslBind((BindOptionsImpl)opts, (ErrorImpl)error, server) == TransportReturnCodes.SUCCESS)
            {
                // initialize JNIServer
                server.pool(_serverPool);
                server.transport(this);
                ((TrackingPool)_serverPool)._active.add(server);
            }
            else
            {
                server.returnToPool();
                server = null;
            }

            return server;
        }
        catch (Exception e)
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            String errorText = error.text();
            error.text("JNI rsslBind() exception: " + errorText);
            return null;
        }
    }

    native int rsslBind(BindOptionsImpl opts, ErrorImpl error, JNIServer server);

    @Override
    public Channel channel(AcceptOptions opts, Server srvr, Object object, Error error)
    {
        try
        {
            // The global lock is locked by Transport
            JNIChannel channel = getChannel();
            channel.isServerChannel(true);
            channel.serviceName(((JNIServer)srvr).bindOptions().serviceName());
            channel.interfaceName(((JNIServer)srvr).bindOptions().interfaceName());

            // accept ETAC channel
            if (rsslAccept((JNIServer)srvr, (AcceptOptionsImpl)opts, (ErrorImpl)error, channel) == TransportReturnCodes.SUCCESS)
            {
                // initialize JNIChannel
                channel.open();
                channel.readLocking(opts.channelReadLocking());
                channel.writeLocking(opts.channelWriteLocking());
                channel.pool(_channelPool);
                channel.transport(this);
                channel.initComponentInfo(error);
                ((TrackingPool)_channelPool)._active.add(channel);
            }
            else
            {
                channel.returnToPool();
                channel = null;
            }

            return channel;
        }
        catch (Exception e)
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            String errorText = error.text();
            error.text("JNI rsslAccept() exception: " + errorText);
            return null;
        }
    }

    native int rsslAccept(JNIServer srvr, AcceptOptionsImpl opts, ErrorImpl error, JNIChannel channel);

    @Override
    public Pool getPool(int poolSpec)
    {
        // JNI gets its buffer pool from ETAC
        return null;
    }

    @Override
    public void uninitialize()
    {
        // The global lock is locked by Transport.
        // This does not close channels and servers.

        // The error is ignored, since the application is closing.
        JNIChannel channel;
        while ((channel = (JNIChannel)((TrackingPool)_channelPool)._active.poll()) != null)
        {
            if (channel._state != ChannelState.INACTIVE)
                channel.close(_error);
        }
        _channelPool.clear();
        JNIServer server;
        while ((server = (JNIServer)((TrackingPool)_serverPool)._active.poll()) != null)
        {
            server.close(_error);
        }
        _serverPool.clear();

        // uninitialize ETAC
        if (_isEtacInitialized)
        {
            rsslUninitialize();
            _isEtacInitialized = false;
        }
    }

    JNIChannel getChannel()
    {
        JNIChannel channel = (JNIChannel)_channelPool.poll();
        if (channel == null)
        {
            channel = new JNIChannel();
        }

        return channel;
    }

    JNIServer getServer()
    {
        JNIServer server = (JNIServer)_serverPool.poll();
        if (server == null)
        {
            server = new JNIServer();
        }

        return server;
    }
}
