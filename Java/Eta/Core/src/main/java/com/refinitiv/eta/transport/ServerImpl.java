package com.refinitiv.eta.transport;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.SocketException;
import java.nio.ByteBuffer;
import java.nio.channels.SelectableChannel;
import java.nio.channels.ServerSocketChannel;
import java.util.Objects;

class ServerImpl extends EtaNode implements Server
{
    class SharedPool extends Pool
    {
        SharedPool(Object o)
        {
            super(o);
            _isSharedPoolBuffer = true;
        }

        int _currentUse = 0; // number of shared buffers currently in use.
        int _peakUse = 0;    // peak number of shared buffers used.
        int _sharedPoolBufferCount = 0; // number of shared pool buffers created.
        Lock _sharedPoolLock;

        @Override
        void add(EtaNode node)
        {
            try
            {
                _sharedPoolLock.lock();
                super.add(node);
                --_currentUse;
            }
            finally
            {
                _sharedPoolLock.unlock();
            }
        }

        EtaNode poll()
        {
            _sharedPoolLock.lock();
            SocketBuffer buffer = null;
            try
            {
                buffer = (SocketBuffer)super.poll();
                if (buffer != null)
                {
                    ++_currentUse;
                }
                else if (_sharedPoolBufferCount < _bindOpts.sharedPoolSize())
                {
                    // first create one buffer and use it
                    buffer = new SocketBuffer(this, bufferSize());
                    ++_peakUse;
                    ++_currentUse;
                    ++_sharedPoolBufferCount;

                    // then create more buffers, as they should be added to pool in bulk
                    int buffersToAdd = ADDED_BUFFERS - 1;
                    if (buffersToAdd > _bindOpts.sharedPoolSize() - _currentUse)
                        buffersToAdd = _bindOpts.sharedPoolSize() - _currentUse;
                    for (int i = 0; i < buffersToAdd; i++)
                    {
                        EtaNode node;
                        node = new SocketBuffer(this, bufferSize());
                        ++_sharedPoolBufferCount;
                        super.add(node);
                    }
                }
                if (_currentUse > _peakUse)
                {
                    _peakUse = _currentUse;
                }
            }
            finally
            {
                _sharedPoolLock.unlock();
            }

            return buffer;
        }

        int info(ServerInfo info, Error error)
        {
            int ret = TransportReturnCodes.SUCCESS;
            try
            {
                _sharedPoolLock.lock();
                if (_state == ChannelState.ACTIVE)
                {
                    ((ServerInfoImpl)info).currentBufferUsage(_currentUse);
                    ((ServerInfoImpl)info).peakBufferUsage(_peakUse);
                }
                else
                {
                    error.channel(null);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("server not in active state ");
                    ret = TransportReturnCodes.FAILURE;
                }
            }
            finally
            {
                _sharedPoolLock.unlock();
            }
            return ret;
        }

        int bufferUsage(Error error)
        {
            int ret;
            try
            {
                _sharedPoolLock.lock();
                if (_state == ChannelState.ACTIVE)
                    ret = _currentUse;
                else
                {
                    error.channel(null);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("server not in active state ");
                    ret = TransportReturnCodes.FAILURE;
                }
            }
            finally
            {
                _sharedPoolLock.unlock();
            }
            return ret;
        }

        void resetPeakUse()
        {
            _peakUse = _currentUse;
        }
    }

    private static final int ADDED_BUFFERS = 100;
    // memory management
    final ProtocolInt _transport;
    final Pool _sharedPool = new SharedPool(this);
    final Lock _realSharedPoolLock = new ReentrantLock();
    final Lock _dummySharedPoolLock = new DummyLock();
    int _numChannels;

    @SuppressWarnings("unused")
    final private ServerInfo _serverInfo = new ServerInfoImpl(); // RsslServerInfo
    BindOptionsImpl _bindOpts = new BindOptionsImpl();
    private ServerSocketChannel _srvrScktChannel;
    private Object _userSpecObject;
    int _state;
    private int _portNumber;
    private int _connType;
    ComponentInfo _componentInfo = new ComponentInfoImpl();
    int _sessionId = 1;
    private EncryptedContextHelper _context;

    ServerImpl(ProtocolInt transport, Pool pool)
    {
        pool(pool);
        _transport = transport;
        _state = ChannelState.INACTIVE;
        _componentInfo.componentVersion().data(Transport._defaultComponentVersionBuffer, 0,
                Transport._defaultComponentVersionBuffer.limit());
        _context = null;
    }

    int bind(BindOptions options, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        // copy the bind options to data member
        ((BindOptionsImpl)options).copyTo(_bindOpts);

        // validate specified WS protocols
        final String wsProtocolCheckStr = WebSocketSupportedProtocols.validateProtocolList(_bindOpts.wSocketOpts().protocols());
        if (Objects.nonNull(wsProtocolCheckStr)) {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Invalid protocol found in protocol list. protocol: " + wsProtocolCheckStr);
            return TransportReturnCodes.FAILURE;
        }

        _portNumber = _bindOpts.port();
        _connType = _bindOpts.connectionType();
        try
        {
            // first check if it's encrypted.  If so, initialize the encrypted context helper
            if(options.connectionType() == ConnectionTypes.ENCRYPTED)
            {
                _context = new EncryptedContextHelper(options);
            }

            // if configured, specify the interface name
            InetSocketAddress socketAddress = null;

            String interfaceName = _bindOpts.interfaceName();

            if (interfaceName == null)
                socketAddress = new InetSocketAddress(_portNumber);
            else if (interfaceName.isEmpty())
                socketAddress = new InetSocketAddress(_portNumber);
            else
            {
                if (interfaceName.equals("0") || interfaceName.equals("localhost"))
                {
                    String ipAddress = InetAddress.getLocalHost().getHostAddress();
                    interfaceName = ipAddress;
                }
                socketAddress = new InetSocketAddress(interfaceName, _portNumber);
            }

            // create ServerSocketChannel
            _srvrScktChannel = ServerSocketChannel.open();
            _srvrScktChannel.configureBlocking(options.serverBlocking());

            // sendBufSize will be set in accept via AcceptOptions
            // for values larger than 64K, recvBufSize must be set prior to bind.
            if (options.sysRecvBufSize() > 0)
                _srvrScktChannel.socket().setReceiveBufferSize(options.sysRecvBufSize());
            else if (_srvrScktChannel.socket().getReceiveBufferSize() > RsslSocketChannel.READ_RECEIVE_BUFFER_SIZE)
                _srvrScktChannel.socket().setReceiveBufferSize(_srvrScktChannel.socket().getReceiveBufferSize());
            else
                _srvrScktChannel.socket().setReceiveBufferSize(RsslSocketChannel.READ_RECEIVE_BUFFER_SIZE);

            _srvrScktChannel.socket().bind(socketAddress);
            _state = ChannelState.ACTIVE;
            // set shared pool lock
            if (_bindOpts.sharedPoolLock())
                ((SharedPool)_sharedPool)._sharedPoolLock = _realSharedPoolLock;
            else
                ((SharedPool)_sharedPool)._sharedPoolLock = _dummySharedPoolLock;

            if(_bindOpts.componentVersion() != null)
            {
                try
                {
                    Transport._globalLock.lock();

                    // user specified info was passed in through the bind Opts
                    byte divider = (byte)'|';
                    ByteBuffer connectOptsCompVerBB = ByteBuffer.wrap(_bindOpts.componentVersion().getBytes());
                    int totalLength = connectOptsCompVerBB.limit() + 1 + Transport._defaultComponentVersionBuffer.limit();
                    if (totalLength > 253)
                    {
                        // the total component data length is too long, so truncate the user defined data
                        totalLength = 253;
                        connectOptsCompVerBB.limit(253 - Transport._defaultComponentVersionBuffer.limit() - 1);
                    }

                    // append the user defined connect opts componentVersionInfo to the default value
                    ByteBuffer combinedBuf = ByteBuffer.allocate(totalLength);

                    Transport._defaultComponentVersionBuffer.mark();
                    combinedBuf.put(Transport._defaultComponentVersionBuffer);
                    Transport._defaultComponentVersionBuffer.reset();
                    combinedBuf.put(divider);
                    combinedBuf.put(connectOptsCompVerBB);

                    // the combined length of the new buffer includes the user defined data, the '|', and the default component version data
                    _componentInfo.componentVersion().data(combinedBuf, 0, totalLength);
                }
                finally
                {
                    Transport._globalLock.unlock();
                }
            }
        }
        catch (Exception e)
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getMessage());
            ret = TransportReturnCodes.FAILURE;
        }

        return ret;
    }

    @Override
    public Channel accept(AcceptOptions options, Error error)
    {
        if (_state != ChannelState.ACTIVE)
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("socket not in active state");
            return null;
        }

        RsslSocketChannel channel = null;
        java.nio.channels.SocketChannel socketChannel = null;
        try
        {
            Transport._globalLock.lock();
            socketChannel = _srvrScktChannel.accept();
            if (socketChannel == null) {
            	 error.channel(null);
                 error.errorId(TransportReturnCodes.FAILURE);
                 error.sysError(0);
                 error.text("This channel is in non-blocking mode and no connection is available to be accepted");
                 return null;
            }

            // sysRecvBufSize would have been set in bind(), no need to repeat.
            if (options.sysSendBufSize() > 0)
                socketChannel.socket().setSendBufferSize(options.sysSendBufSize());
            else if (socketChannel.socket().getSendBufferSize() > RsslSocketChannel.READ_RECEIVE_BUFFER_SIZE)
                // set send buffer size to system default.
                socketChannel.socket().setSendBufferSize(socketChannel.socket().getSendBufferSize());
            else
                // set send buffer size to default.
                socketChannel.socket().setSendBufferSize(RsslSocketChannel.READ_RECEIVE_BUFFER_SIZE);

            if (_bindOpts.channelsBlocking())
            {
                socketChannel.configureBlocking(true);
            }
            else
            {
                socketChannel.configureBlocking(false);
            }
            if (_bindOpts.tcpOpts().tcpNoDelay())
            {
                socketChannel.socket().setTcpNoDelay(true);
            }
            channel = (RsslSocketChannel)_transport.channel(options, this, socketChannel, error);

            /* Give our Component Info to the Channel.
             * No need for deep copies here since the channel will never re-connect.
             * The channel can simply use our Component Info. */
            if (channel != null)
            {
                channel._componentInfo = ((ComponentInfoImpl)_componentInfo).clone();

                if (channel._providerSessionId == null)
                {
                    channel._providerSessionId = Integer.valueOf(_sessionId);
                    _sessionId++;
                }
            }
        }
        catch (Exception e)
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("socket IO Exception");
            try
            {
                socketChannel.close();
            }
            catch (IOException e1)
            {
            }
            socketChannel = null;
            channel = null;
        }
        finally
        {
            Transport._globalLock.unlock();
        }

        return channel;
    }

    BindOptions bindOptions()
    {
        return _bindOpts;
    }

    @Override
    public String toString()
    {
        return "Server" + "\n" +
               "\tsrvrScktChannel: " + _srvrScktChannel + "\n" +
               "\tstate: " + _state + "\n" +
               "\tportNumber: " + _portNumber + "\n" +
               "\tuserSpecObject: " + _userSpecObject + "\n";
    }

    @Override
    public int info(ServerInfo info, Error error)
    {
        return ((SharedPool)_sharedPool).info(info, error);
    }

    @Override
    public int ioctl(int code, Object value, Error error)
    {
        int retCode = TransportReturnCodes.FAILURE;

        try
        {
            ((SharedPool)_sharedPool)._sharedPoolLock.lock();

            // return FAILURE if channel not active or not initializing
            if ((_state != ChannelState.ACTIVE) && (_state != ChannelState.INITIALIZING))
            {
                error.channel(null);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in the active or initializing state");
                return TransportReturnCodes.FAILURE;
            }
            switch (code)
            {
                case IoctlCodes.COMPONENT_INFO:
                {
                    if (value != null && (value.getClass() == ComponentInfoImpl.class))
                    {
                        _componentInfo = ((ComponentInfoImpl)value).clone();
                        retCode = TransportReturnCodes.SUCCESS;
                    }
                    else
                    {
                        error.channel(null);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text("Code is not valid.");
                    }
                    break;
                }
                case IoctlCodes.SERVER_PEAK_BUF_RESET:
                    ((SharedPool)_sharedPool).resetPeakUse();
                    retCode = TransportReturnCodes.SUCCESS;
                    break;
                default:
                    error.channel(null);
                    error.errorId(retCode);
                    error.sysError(0);
                    error.text("Code is not valid.");
            }
        }
        finally
        {
            ((SharedPool)_sharedPool)._sharedPoolLock.unlock();
        }
        return retCode;
    }

    @Override
    public int ioctl(int code, int value, Error error)
    {
        int retCode = TransportReturnCodes.FAILURE;

        try
        {
            ((SharedPool)_sharedPool)._sharedPoolLock.lock();

            // return FAILURE if channel not active or not initializing
            if ((_state != ChannelState.ACTIVE) && (_state != ChannelState.INITIALIZING))
            {
                error.channel(null);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in the active or initializing state");
                return TransportReturnCodes.FAILURE;
            }

            switch (code)
            {
                case IoctlCodes.SERVER_NUM_POOL_BUFFERS:
                {
                    /* SERVER_NUM_POOL_BUFFERS: the per server number of
                     * sharedPool buffers that ETAJ will share with channels of a server. */
                    if (value > 0)
                    {
                        retCode = adjustSharedPoolBuffers(value);
                    }
                    else
                    {
                        error.channel(null);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text("value must be greater than zero");
                    }
                    break;
                }
                case IoctlCodes.SYSTEM_READ_BUFFERS:
                {
                    if (value >= 0)
                    {
                        _srvrScktChannel.socket().setReceiveBufferSize(value);
                        retCode = TransportReturnCodes.SUCCESS;
                    }
                    else
                    {
                        error.channel(null);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text("value must be (0 >= value < 2^31");
                    }
                    break;
                }
                default:
                    error.channel(null);
                    error.errorId(retCode);
                    error.sysError(0);
                    error.text("Code is not valid.");
            }
        }
        catch (SocketException e)
        {
            _state = ChannelState.CLOSED;
            error.channel(null);
            error.errorId(retCode);
            error.sysError(0);
            error.text("failed to set SYSTEM_WRITE_BUFFERS, " + e.toString());
        }
        finally
        {
            ((SharedPool)_sharedPool)._sharedPoolLock.unlock();
        }
        return retCode;
    }

    /* Shrink the sharedPool by numToShrink.
     * Do this by adding numToShrink sharedPool to global pool (SocketProtocol).
     *
     * Returns the number actually shrunk, which may not be the numToShrink.
     */
    int shrinkSharedPoolBuffers(int numToShrink)
    {
        Pool bufferPool = _transport.getPool(bufferSize());
        return bufferPool.add(_sharedPool, numToShrink);
    }

    /* 1) If the value is larger than the current value, update the sharedPoolSize only.
     * 2) If the new value is smaller than the current value,
     * attempt to shrink the buffers by returning them to the global pool.
     * If the number of available buffers (not in use) is smaller than
     * what needs to be returned (because the buffers are being used),
     * just shrink by that number.
     *
     * Returns the new value of sharedPoolSize
     */
    int adjustSharedPoolBuffers(int value)
    {
        int diff = value - _bindOpts._sharedPoolSize;
        if (diff > 0)
        {
            // the new value is larger, update sharedPoolSize.
            return _bindOpts._sharedPoolSize = value;
        }
        else if (diff < 0)
        {
            // shrink buffers
            return _bindOpts._sharedPoolSize -= shrinkSharedPoolBuffers(diff * -1);
        }
        else
        {
            // nothing changed.
            return _bindOpts._sharedPoolSize;
        }
    }

    @Override
    public int bufferUsage(Error error)
    {
        return ((SharedPool)_sharedPool).bufferUsage(error);
    }

    @Override
    public int close(Error error)
    {
        if (_state != ChannelState.INACTIVE)
        {
            _state = ChannelState.INACTIVE;
        }
        else
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("socket channel is already inactive ");
            return TransportReturnCodes.FAILURE;
        }

        int ret = TransportReturnCodes.SUCCESS;
        try
        {
            _state = ChannelState.INACTIVE;
            _srvrScktChannel.close();

            if (_numChannels == 0)
                releaseServer();

        }
        catch (IOException e)
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("socket server close failed ");
            ret = TransportReturnCodes.FAILURE;
        }
        return ret;
    }

    @Override @Deprecated
    public ServerSocketChannel srvrScktChannel()
    {
        return _srvrScktChannel;
    }

    @Override
    public SelectableChannel selectableChannel()
    {
        return _srvrScktChannel;
    }

    @Override
    public int portNumber()
    {
        return _portNumber;
    }

    public int connectionType()
    {
        return _connType;
    }

    @Override
    public Object userSpecObject()
    {
        return _userSpecObject;
    }

    int bufferSize()
    {
        /* This buffer size is used internal and represents the size of the buffers that are used internally.
         * The size will be the RIPC MAX_USER_MSG_SIZE + RIPC_HDR_SIZE.
         * The RIPC MAX_USER_MSG_SIZE is the bindOps.maxFragmentSize().
         * Note that this value is different from _channelInfo._maxFragmentSize,
         * which is returned to the user and is RIPC MAX_USER_MSG_SIZE - RIPC PACKED_HDR_SIZE. */
        return _bindOpts.maxFragmentSize() + RsslSocketChannel.RIPC_HDR_SIZE;
    }

    SocketBuffer getBufferFromServerPool()
    {
        return (SocketBuffer)((SharedPool)_sharedPool).poll();
    }

    void socketBufferToRecycle(SocketBuffer buffer)
    {
        buffer.returnToPool();
    }

    void removeChannel(Channel chnl)
    {
        _numChannels--;

        if (_numChannels == 0)
        {
            if (_state == ChannelState.INACTIVE)
                releaseServer();
        }
    }

    void removeChannel()
    {
        _numChannels--;

        if (_numChannels == 0)
        {
            if (_state == ChannelState.INACTIVE)
                releaseServer();
        }
    }

    private void releaseServer()
    {
        try
        {
            Transport._globalLock.lock();

            // return buffers from the shared pool to global pool
            Pool pool = _transport.getPool(bufferSize());
            pool.add(_sharedPool, _sharedPool.size());

            // return this server to server pool
            returnToPool();
        }
        finally
        {
            Transport._globalLock.unlock();
        }
    }

    @Override
    public int state()
    {
        return _state;
    }
    
    public EncryptedContextHelper context() {
        return _context;
    }

}
