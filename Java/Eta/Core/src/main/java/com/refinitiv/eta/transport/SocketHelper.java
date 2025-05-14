/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.io.IOException;
import java.net.Socket;
import java.net.SocketAddress;
import java.net.SocketOption;
import java.nio.ByteBuffer;
import java.nio.channels.ClosedChannelException;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SelectionKey;
import java.nio.channels.Selector;
import java.nio.channels.SocketChannel;
import java.nio.channels.spi.SelectorProvider;
import java.util.Set;

public class SocketHelper
{
    protected SocketChannel _socket;
    protected boolean _completedProxy = false;

    public SocketHelper()
    {
    }

    public SocketHelper(SocketChannel _socket)
    {
        this._socket = _socket;
    }

    public SocketChannel bind(SocketAddress local) throws IOException
    {
        return _socket.bind(local);
    }


    public <T> SocketChannel setOption(SocketOption<T> name, T value) throws IOException
    {
        return _socket.setOption(name, value);
    }


    public SocketChannel shutdownInput() throws IOException
    {
        return _socket.shutdownInput();
    }

    public SocketChannel shutdownOutput() throws IOException
    {
        return _socket.shutdownOutput();
    }

    public Socket socket()
    {
        return _socket.socket();
    }

    public boolean isConnectionPending()
    {
        return _socket.isConnectionPending();
    }

    public boolean connect(SocketAddress remote, boolean proxy) throws IOException
    {
        return _socket.connect(remote);
    }

    public boolean finishConnect() throws IOException
    {
        return _socket.finishConnect();
    }
    
    public SocketAddress getRemoteAddress() throws IOException
    {
        return _socket.getRemoteAddress();
    }


    public long read(ByteBuffer[] dsts, int offset, int length) throws IOException
    {
        return _socket.read(dsts, offset, length);
    }

    public int write(ByteBuffer src) throws IOException
    {
        return _socket.write(src);
    }

    public long write(ByteBuffer[] srcs, int offset, int length) throws IOException
    {
        return _socket.write(srcs, offset, length);
    }

    public SocketAddress getLocalAddress() throws IOException
    {
        return _socket.getLocalAddress();
    }

    public SelectorProvider provider()
    {
        return _socket.provider();
    }

    public int validOps()
    {
        return _socket.validOps();
    }

    public boolean isRegistered()
    {
        return _socket.isRegistered();
    }

    public SelectionKey keyFor(Selector sel)
    {
        return _socket.keyFor(sel);
    }

    public SelectionKey register(Selector sel, int ops, Object att) throws ClosedChannelException
    {
        return _socket.register(sel, ops, att);
    }

    public SelectableChannel configureBlocking(boolean block) throws IOException
    {
        return _socket.configureBlocking(block);
    }

    public boolean isConnected()
    {
        return _socket.isConnected();
    }

    public boolean isBlocking()
    {
        return _socket.isBlocking();
    }

    public int read(ByteBuffer dst) throws IOException
    {
        return _socket.read(dst);
    }

    public Object blockingLock()
    {
        return _socket.blockingLock();
    }

    public boolean isOpen()
    {
        return _socket.isOpen();
    }

    public void close() throws IOException
    {
        _socket.close();
    }

    public void completedProxyConnection()
    {
        _completedProxy = true;
    }

    public boolean postProxyInit() throws IOException
    {
        /* No-op here, used in encrypted case. */
        return true;
    }
    
    public String getActiveTLSVersion() throws IOException
    {
        /* Used in encrypted case. */
        return "None";
    }

    public long read(ByteBuffer[] dsts) throws IOException
    {
        return _socket.read(dsts);
    }

    public long write(ByteBuffer[] srcs) throws IOException
    {
        return _socket.write(srcs);
    }

    public <T> T getOption(SocketOption<T> name) throws IOException
    {
        return _socket.getOption(name);
    }

    public Set<SocketOption<?>> supportedOptions()
    {
        return _socket.supportedOptions();
    }

    public void setSocketChannel(SocketChannel socket)
    {
        this._socket = socket;
    }

    public SocketChannel getSocketChannel()
    {
        return _socket;
    }
    
    public void copy(SocketHelper dstSocket)
    {
    	dstSocket._socket = _socket;
    	dstSocket._completedProxy = _completedProxy;
    }

    public void initialize(ConnectOptions options) throws IOException
    {
        _completedProxy = false;
    }

    public void initialize(BindOptions options) throws IOException
    {
    	// No proxy connections for servers 
    	_completedProxy = true;
    }
}
