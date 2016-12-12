package com.thomsonreuters.upa.transport;

import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SocketChannel;
import java.util.ArrayList;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.Int;

/* Implements a JNI channel that hooks into the UPAC API at the top level. */
public class JNIChannel extends UpaNode implements Channel
{
    /* SocketChannel of this UPA channel. */
    public SocketChannel _scktChannel;
    
    /* Old SocketChannel of this UPA channel - used in rsslRead Channel Change events. */
    public SocketChannel _oldScktChannel;
    
    /* State of this UPA channel. */
    public int _state;
    
    /* Type that this connection is. */
    public int _connectionType;
    
    /* When returned through rsslAccept, this contains the IP address of the connected client. */
    public String _clientIP;
    
    /* When returned through rsslAccept, this contains the hostname of the connected client. */
    public String _clientHostname;
    
    /* Contains the interval of time that a message or ping should be sent. */
    public int _pingTimeout;
    
    /* Contains the major version number of the encoder/decoder that should be used. */
    public int _majorVersion;
    
    /* Contains the minor version number of the encoder/decoder that should be used. */
    public int _minorVersion;
    
    /* Contains the protocol type of the encoder/decoder that should be used. */
    public int _protocolType;
    
    /* A user specified object, possibly a closure. */
    public Object _userSpecObject;

    /* Component version info passed in through ConnectOptions */
    ComponentInfo _connectOptsComponentInfo = null;

    /* Component Info to send during RIPC handshake (ConnectReq or ConnectAck).
     * This is hidden from user. Internal clients can use Channel.ioctl() to set. */
    ComponentInfo _componentInfo = new ComponentInfoImpl();    

    JNIProtocol _transport;
    Int _tempInt = CodecFactory.createInt();
    JNIBuffer _readBuffer = new JNIBuffer();
    boolean _isServerChannel;

    String _serviceName, _interfaceName;
    boolean _isBlocking;

    // channel locking variables - channel locking is done in pure Java
    final Lock _realReadLock = new ReentrantLock();
    final Lock _realWriteLock = new ReentrantLock();
    final Lock _dummyReadLock = new DummyLock();
    final Lock _dummyWriteLock = new DummyLock();
    Lock _readLock;
    Lock _writeLock;

    // packing buffer update length
    int _bufUpdateLen = 2;

    JNIChannel()
    {
        // not client specified, use default from MANIFEST.MF
        _componentInfo.componentVersion().data(Transport._defaultComponentVersionBuffer, 0,
                                               Transport._defaultComponentVersionBuffer.limit());
    }

    void readLocking(boolean locking)
    {
        if (locking)
        {
            _readLock = _realReadLock;
        }
        else
        {
            _readLock = _dummyReadLock;
        }
    }

    void writeLocking(boolean locking)
    {
        if (locking)
        {
            _writeLock = _realWriteLock;
        }
        else
        {
            _writeLock = _dummyWriteLock;
        }
    }

    private void lockReadWriteLocks()
    {
        _readLock.lock();
        _writeLock.lock();
    }

    private void unlockReadWriteLocks()
    {
        _readLock.unlock();
        _writeLock.unlock();
    }

    @Override
    public int info(ChannelInfo info, Error error)
    {
        assert (info != null) : "info cannot be null";
        assert (error != null) : "error cannot be null";

        try
        {
            _writeLock.lock();
            if (info.componentInfo() == null)
            {
                ((ChannelInfoImpl)info)._receivedComponentInfoList = new ArrayList<ComponentInfo>(1);
            }
            ((ChannelInfoImpl)info)._clientHostname = _clientHostname;
            ((ChannelInfoImpl)info)._clientIP = _clientIP;
            return rsslGetChannelInfo(this, (ChannelInfoImpl)info, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslGetChannelInfo() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            _writeLock.unlock();
        }
    }

    native int rsslGetChannelInfo(JNIChannel chnl, ChannelInfoImpl info, ErrorImpl error);

    @Override
    public int ioctl(int code, Object value, Error error)
    {
        assert (value != null) : "value cannot be null";
        assert (error != null) : "error cannot be null";

        try
        {
            _writeLock.lock();
            return rsslIoctl(this, code, value, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslIoctl() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            _writeLock.unlock();
        }
    }

    @Override
    public int ioctl(int code, int value, Error error)
    {
        assert (error != null) : "error cannot be null";

        try
        {
            _writeLock.lock();
            _tempInt.value(value);
            return rsslIoctl(this, code, _tempInt, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslIoctl() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            _writeLock.unlock();
        }
    }
	
    native int rsslIoctl(JNIChannel chnl, int code, Object value, ErrorImpl error);

    @Override
    public int bufferUsage(Error error)
    {
        assert (error != null) : "error cannot be null";

        try
        {
            _writeLock.lock();
            return rsslBufferUsage(this, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslBufferUsage() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            _writeLock.unlock();
        }
    }

    native int rsslBufferUsage(JNIChannel chnl, ErrorImpl error);

    @Override
    public int init(InProgInfo inProg, Error error)
    {
        assert (inProg != null) : "inProg cannot be null";
        assert (error != null) : "error cannot be null";

        try
        {
            lockReadWriteLocks();
            return rsslInitChannel(this, (InProgInfoImpl)inProg, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslInitChannel() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            unlockReadWriteLocks();
        }
    }

    native int rsslInitChannel(JNIChannel chnl, InProgInfoImpl inProg, ErrorImpl error);

    @Override
    public int close(Error error)
    {
        assert (error != null) : "error cannot be null";

        try
        {
            lockReadWriteLocks();
            int retVal = rsslCloseChannel(this, (ErrorImpl)error);
            _state = ChannelState.INACTIVE;
            _scktChannel.close();
            if (_oldScktChannel != null)
            {
                _oldScktChannel.close();
            }
            returnToPool();
            return retVal;
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslCloseChannel() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            unlockReadWriteLocks();
        }
    }

    native int rsslCloseChannel(JNIChannel chnl, ErrorImpl error);

    @Override
    public TransportBuffer read(ReadArgs readArgs, Error error)
    {
        assert (readArgs != null) : "readArgs cannot be null";
        assert (error != null) : "error cannot be null";

        try
        {
            if (_readLock.trylock())
            {
                return rsslRead(this, (ReadArgsImpl)readArgs, (ErrorImpl)error);
            }
            else
            {
                // failed to obtain the lock
                ((ReadArgsImpl)readArgs).readRetVal(TransportReturnCodes.READ_IN_PROGRESS);
                return null;
            }
        }
        catch (Exception e)
        {
            ((ReadArgsImpl)readArgs).readRetVal(TransportReturnCodes.FAILURE);
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslRead() exception: " + error.text());
            return null;
        }
        finally
        {
            _readLock.unlock();
        }
    }

    native JNIBuffer rsslRead(JNIChannel chnl, ReadArgsImpl readArgs, ErrorImpl error);

    @Override
    public TransportBuffer getBuffer(int size, boolean packedBuffer, Error error)
    {
        assert (error != null) : "error cannot be null";

        try
        {
            _writeLock.lock();
            TransportBuffer buf = rsslGetBuffer(this, size, packedBuffer, (ErrorImpl)error);
            ((JNIBuffer)buf)._isWriteBuffer = true;
            if (packedBuffer)
            {
                ((JNIBuffer)buf)._isPacked = true;
            }
            else
            {
                ((JNIBuffer)buf)._isPacked = false;
            }
            return buf;
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslGetBuffer() exception: " + error.text());
            return null;
        }
        finally
        {
            _writeLock.unlock();
        }
    }

    native JNIBuffer rsslGetBuffer(JNIChannel chnl, int size, boolean packedBuffer, ErrorImpl error);

    @Override
    public int releaseBuffer(TransportBuffer buffer, Error error)
    {
        assert (buffer != null) : "buffer cannot be null";
        assert (error != null) : "error cannot be null";

        try
        {
            _writeLock.lock();
            return rsslReleaseBuffer((JNIBuffer)buffer, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslReleaseBuffer() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            _writeLock.unlock();
        }
    }

    native int rsslReleaseBuffer(JNIBuffer buffer, ErrorImpl error);

    @Override
    public int packBuffer(TransportBuffer buffer, Error error)
    {
        assert (buffer != null) : "buffer cannot be null";
        assert (error != null) : "error cannot be null";

        try
        {
            int remainingLength = rsslPackBuffer(this, (JNIBuffer)buffer, (ErrorImpl)error);

            /* update buffer position */
            ((JNIBuffer)buffer)._data.position(((JNIBuffer)buffer)._data.position() + _bufUpdateLen);
            ((JNIBuffer)buffer)._position = ((JNIBuffer)buffer)._data.position();

            return remainingLength;
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslPackBuffer() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
    }

    native int rsslPackBuffer(JNIChannel chnl, JNIBuffer buffer, ErrorImpl error);

    @Override
    public int write(TransportBuffer buffer, WriteArgs writeArgs, Error error)
    {
        assert (buffer != null) : "buffer cannot be null";
        assert (writeArgs != null) : "writeArgs cannot be null";
        assert (error != null) : "error cannot be null";

        try
        {
            _writeLock.lock();
            return rsslWrite(this, (JNIBuffer)buffer, (WriteArgsImpl)writeArgs, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslWrite() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            _writeLock.unlock();
        }
    }

    native int rsslWrite(JNIChannel chnl, JNIBuffer buffer, WriteArgsImpl writeArgs, ErrorImpl error);

    @Override
    public int flush(Error error)
    {
        assert (error != null) : "error cannot be null";

        try
        {
            _writeLock.lock();
            return rsslFlush(this, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslFlush() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            _writeLock.unlock();
        }
    }

    native int rsslFlush(JNIChannel chnl, ErrorImpl error);

    @Override
    public int ping(Error error)
    {
        assert (error != null) : "error cannot be null";

        try
        {
            _writeLock.lock();
            return rsslPing(this, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslPing() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            _writeLock.unlock();
        }
    }

    native int rsslPing(JNIChannel chnl, ErrorImpl error);

    @Override
    public int reconnectClient(Error error)
    {
        assert (error != null) : "error cannot be null";

        try
        {
            lockReadWriteLocks();
            return rsslReconnectClient(this, (ErrorImpl)error);
        }
        catch (Exception e)
        {
            setError(error, this, TransportReturnCodes.FAILURE, error.sysError(),
                     "JNI rsslReconnectClient() exception: " + error.text());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            unlockReadWriteLocks();
        }
    }

    native int rsslReconnectClient(JNIChannel chnl, ErrorImpl error);

    @Override
    public int majorVersion()
    {
        return _majorVersion;
    }

    @Override
    public int minorVersion()
    {
        return _minorVersion;
    }

    @Override
    public int protocolType()
    {
        return _protocolType;
    }

    @Override
    public int state()
    {
        return _state;
    }

    @Override
    public SocketChannel scktChannel()
    {
        return _scktChannel;
    }

    @Override
    public SocketChannel oldScktChannel()
    {
        return _oldScktChannel;
    }

    @Override
    public SelectableChannel selectableChannel()
    {
        return _scktChannel;
    }

    @Override
    public SelectableChannel oldSelectableChannel()
    {
        return _oldScktChannel;
    }

    @Override
    public int pingTimeout()
    {
        return _pingTimeout;
    }

    @Override
    public Object userSpecObject()
    {
        return _userSpecObject;
    }

    @Override
    public boolean blocking()
    {
        return _scktChannel.isBlocking();
    }

    @Override
    public int connectionType()
    {
        return _connectionType;
    }

    void transport(JNIProtocol transport)
    {
        _transport = transport;
    }

    public void open()
    {
        try
        {
            if (_scktChannel == null || !_scktChannel.isOpen())
            {
                _scktChannel = SocketChannel.open();
            }

            if (_connectionType == ConnectionTypes.RELIABLE_MCAST)
            {
                _bufUpdateLen += 6;
            }
        }
        catch (Exception e)
        {
            System.out.println("SocketChannel.open() Exception1: " + e.getMessage());
        }
    }

    void initComponentInfo(Error error)
    {
        /* Connected Component Info */

        if (_connectOptsComponentInfo != null)
        {
            byte divider = (byte)'|';
            int totalLength = _connectOptsComponentInfo.componentVersion().data().limit() + 1 + Transport._defaultComponentVersionBuffer.limit();
            int origLimit = _connectOptsComponentInfo.componentVersion().data().limit();
            if (totalLength > 253)
            {
                // the total component data length is too long, so truncate the
                // user defined data
                totalLength = 253;
                _connectOptsComponentInfo.componentVersion().data().limit(253 - Transport._defaultComponentVersionBuffer.limit() - 1);
            }

            // append the user defined connect opts componentVersionInfo to the default value
            ByteBuffer combinedBuf = ByteBuffer.allocate(totalLength);

            Transport._defaultComponentVersionBuffer.mark();
            combinedBuf.put(Transport._defaultComponentVersionBuffer);
            Transport._defaultComponentVersionBuffer.reset();
            combinedBuf.put(divider);
            combinedBuf.put(_connectOptsComponentInfo.componentVersion().data());

            // the combined length of the new buffer includes the user defined data, the '|', and the default component version data
            _componentInfo.componentVersion().data(combinedBuf, 0, totalLength);

            // reset the limit for this buffer in case it was truncated
            _connectOptsComponentInfo.componentVersion().data().limit(origLimit);
        }
        else
        {
            // not client specified, use default from MANIFEST.MF
            _componentInfo.componentVersion().data(Transport._defaultComponentVersionBuffer, 0, Transport._defaultComponentVersionBuffer.limit());
        }

        ioctl(IoctlCodes.COMPONENT_INFO, _componentInfo, error);
    }

    void initConnOptsComponentInfo(ConnectOptions opts, Error error)
    {
        if (opts.componentVersion() != null)
        {
            ByteBuffer connectOptsCompVerBB = ByteBuffer.wrap(opts.componentVersion().getBytes());
            _connectOptsComponentInfo = new ComponentInfoImpl();
            _connectOptsComponentInfo.componentVersion().data(connectOptsCompVerBB);
        }
    }

    public JNIBuffer readBuffer(ByteBuffer data, int length)
    {
        _readBuffer._data = data;
        _readBuffer._position = 0;
        _readBuffer._length = length;
        _readBuffer._startPosition = 0;
        _readBuffer._isWriteBuffer = false;

        return _readBuffer;
    }

    void isServerChannel(boolean isServerChannel)
    {
        _isServerChannel = isServerChannel;
    }

    void serviceName(String serviceName)
    {
        _serviceName = serviceName;
    }

    void interfaceName(String interfaceName)
    {
        _interfaceName = interfaceName;
    }

    /* The following is for the UPAC JNI code. */

    /* Socket ID of this UPA channel. */
    public int _socketId;
    
    /* Old Socket Id of this UPA channel - used in rsslRead FD Change events. */
    public int _oldSocketId;
    
    /* Pointer to the actual rsslChannel in C. */
    public long _rsslChannelCPtr;

    /* for clearing dummy bytes used for notification */
    ByteBuffer _clearBuffer = ByteBuffer.allocate(64);

    /* Connects the channel to the internal C server port. */
    public void connectJavaChannel(int serverPort)
    {
        try
        {
            try
            {
                if (_scktChannel == null || !_scktChannel.isOpen())
                {
                    _scktChannel = SocketChannel.open();
                }
            }
            catch (Exception e)
            {
                System.out.println("SocketChannel.open() Exception2: " + e.getMessage());
            }
            _scktChannel.socket().connect(new InetSocketAddress("localhost", serverPort));
            _scktChannel.configureBlocking(false);
        }
        catch (Exception e)
        {
            System.out.println("\nJNI connectJavaChannel Exception: " + e.getMessage());
        }
    }
	
    /* Writes the C file descriptor and connection type to the internal C server port. */
    public void writeJavaChannel(int chnlSocketFD, int connectionType)
    {
        ByteBuffer buffer;
        String bufferString = String.valueOf(chnlSocketFD) + ":" +
                              String.valueOf(connectionType) + ":" +
                              (_isServerChannel ? "1" : "0");

        buffer = ByteBuffer.wrap(bufferString.getBytes());
        try
        {
            _scktChannel.write(buffer);
        }
        catch (Exception e)
        {
            System.out.println("\nJNI writeJavaChannel Exception: " + e.getMessage());
        }
    }

    /* Clears dummy bytes from the channel. */
    public void clearJavaChannel()
    {
        try
        {
            _clearBuffer.clear();
            _scktChannel.read(_clearBuffer);
        }
        catch (Exception e)
        {
            System.out.println("\nJNI clearJavaChannel Exception: " + e.getMessage());
        }
    }

    /* Configures blocking mode for the channel. */
    public void configureBlockingMode(boolean blocking)
    {
        try
        {
            _isBlocking = blocking;
            _scktChannel.configureBlocking(blocking);
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
}
