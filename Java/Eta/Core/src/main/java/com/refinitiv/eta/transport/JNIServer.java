/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.net.InetSocketAddress;
import java.nio.channels.SelectableChannel;
import java.nio.channels.ServerSocketChannel;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.Int;

/* Implements a JNI server that hooks into the ETAC API at the top level. */
public class JNIServer extends EtaNode implements Server
{
    JNIProtocol _transport;
    ComponentInfo _componentInfo = new ComponentInfoImpl();
    Int _tempInt = CodecFactory.createInt();
    BindOptions _bindOpts;

    /* ServerSocketChannel of this ETA server. */
    public ServerSocketChannel _srvrScktChannel;

    /* State of this ETA server. */
    public int _state;

    /* Port number this server is bound to. */
    public int _portNumber;

    /* A user specified object, possibly a closure. */
    public Object _userSpecObject;

    JNIServer()
    {
        _componentInfo.componentVersion().data(Transport._defaultComponentVersionBuffer, 0,
                Transport._defaultComponentVersionBuffer.limit());
    }

    @Override
    public int info(ServerInfo info, Error error)
    {
        try
        {
            return rsslGetServerInfo(this, (ServerInfoImpl)info, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, null, TransportReturnCodes.FAILURE, error.sysError(),
                    "JNI rsslGetServerInfo() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
    }

    native int rsslGetServerInfo(JNIServer srvr, ServerInfoImpl info, ErrorImpl error);

    @Override
    public int ioctl(int code, Object value, Error error)
    {
        try
        {
            return rsslServerIoctl(this, code, value, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, null, TransportReturnCodes.FAILURE, error.sysError(),
                    "JNI rsslServerIoctl() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
    }

    @Override
    public int ioctl(int code, int value, Error error)
    {
        try
        {
            if (code != IoctlCodes.SYSTEM_READ_BUFFERS)
            {
                _tempInt.value(value);
                return rsslServerIoctl(this, code, _tempInt, (ErrorImpl)error);
            }
            else
            {
                // SYSTEM_READ_BUFFERS not supported for ETAC
                return TransportReturnCodes.SUCCESS;
            }
        }
        catch (Exception e)
        {
            setError(error, null, TransportReturnCodes.FAILURE, error.sysError(),
                    "JNI rsslServerIoctl() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
    }

    native int rsslServerIoctl(JNIServer srvr, int code, Object value, ErrorImpl error);

    @Override
    public int bufferUsage(Error error)
    {
        try
        {
            return rsslServerBufferUsage(this, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, null, TransportReturnCodes.FAILURE, error.sysError(),
                    "JNI rsslServerBufferUsage() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
    }

    native int rsslServerBufferUsage(JNIServer srvr, ErrorImpl error);

    @Override
    public int close(Error error)
    {
        try
        {
            _state = ChannelState.INACTIVE;
            releaseServer();
            return rsslCloseServer(this, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, null, TransportReturnCodes.FAILURE, error.sysError(),
                    "JNI rsslCloseServer() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
    }

    native int rsslCloseServer(JNIServer srvr, ErrorImpl error);

    @Override
    public Channel accept(AcceptOptions opts, Error error)
    {
        JNIChannel channel = (JNIChannel)_transport.channel(opts, this, null, error);

        /* Give our Component Info to the Channel.
         * No need for deep copies here since the channel will never re-connect.
         * The channel can simply use our Component Info. */
        if (channel != null)
            channel._componentInfo = _componentInfo;

        return channel;
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

    @Override
    public Object userSpecObject()
    {
        return _userSpecObject;
    }

    @Override
    public int state()
    {
        return _state;
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

    void transport(JNIProtocol transport)
    {
        _transport = transport;
    }

    void bindOptions(BindOptions opts)
    {
        _bindOpts = opts;
    }

    BindOptions bindOptions()
    {
        return _bindOpts;
    }

    private void releaseServer()
    {
        try
        {
            Transport._globalLock.lock();

            // return this server to server pool
            returnToPool();
        }
        finally
        {
            Transport._globalLock.unlock();
        }
    }

    public void open()
    {
        try
        {
            _srvrScktChannel = ServerSocketChannel.open();
        }
        catch (Exception e)
        {
            System.out.println("ServerSocketChannel.open() Exception: " + e.getMessage());
        }
    }

    /* The following is for the ETAC JNI code. */

    /* Socket ID of this RSSL server. */
    public int _socketId;

    /* Pointer to the actual rsslServer in C. */
    public long _rsslServerCPtr;

    /* If true, the server will be allowed to block. */
    public boolean _serverBlocking;

    /* If true, the channels will be allowed to block. */
    public boolean _channelsBlocking;

    /* Binds the server to the port specified by the native C code. */
    public boolean bindJavaServer(int serverPort)
    {
        boolean retVal = true;
        try
        {
            _srvrScktChannel.configureBlocking(false);
            _srvrScktChannel.socket().bind(new InetSocketAddress("localhost", serverPort));
        }
        catch (Exception e)
        {
            retVal = false;
        }

        return retVal;
    }

    /* Accepts a connection for the native C code. */
    public void acceptJavaServer()
    {
        try
        {
            if (!_srvrScktChannel.isBlocking())
            {
                _srvrScktChannel.accept();
            }
        }
        catch (Exception e)
        {
            System.out.println("\nJNI acceptJavaServer Exception: " + e.getMessage());
        }
    }

    /* Closes the server port. */
    public void closeJavaServer(int serverPort)
    {
        try
        {
            _srvrScktChannel.close();
        }
        catch (Exception e)
        {
            System.out.println("\nJNI closeJavaServer Exception: " + e.getMessage());
        }
    }

    /* Configures blocking mode for the server. */
    public void configureBlockingMode()
    {
        try
        {
            _srvrScktChannel.configureBlocking(_serverBlocking);
        }
        catch (Exception e)
        {
            System.out.println("\nJNI configureBlockingMode Exception: " + e.getMessage());
        }
    }

    void setError(Error error, JNIChannel chnl, int retVal, int syserr, String text)
    {
        error.channel(chnl);
        error.sysError(syserr);
        error.errorId(retVal);
        error.text(text);
    }

    @Override
    public int connectionType() {
        return _bindOpts.connectionType();
    }
}
