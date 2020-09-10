package com.rtsdk.eta.transport;

import java.io.IOException;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.net.NetworkInterface;
import java.net.SocketAddress;
import java.net.SocketException;
import java.net.StandardProtocolFamily;
import java.net.StandardSocketOptions;
import java.nio.ByteBuffer;
import java.nio.channels.DatagramChannel;
import java.nio.channels.MembershipKey;
import java.nio.channels.SelectableChannel;
import java.nio.channels.SocketChannel;

public class RsslSeqMulticastSocketChannel extends EtaNode implements Channel
{
    private final int EDF_SEQ_NUM_LEN = 4;
    private final int EDF_MSG_LEN_LEN = 2;
    private final int EDF_PING_LEN = 12;
    private final int EDF_VERSION_LEN = 1;
    private final int EDF_FLAGS_LEN = 1;
    private final int SEQ_MCAST_FLAGS_RETRANSMIT = 0x2;
    private final int EDF_PROTOCOL_TYPE_LEN = 1;
    private final int EDF_HDR_LENGTH_LEN = 1;
    private final int EDF_PROTO_VER_MAJOR_LEN = 1;
    private final int EDF_PROTO_VER_MINOR_LEN = 1;
    private final int EDF_INSTANCE_ID_LEN = 2;

    private final byte EDF_MAX_VERSION = 1;

    private final int EDF_MAX_HDR_LEN = 14;

    private NetworkInterface _ni;
    private InetAddress _group;
    private InetSocketAddress _inetSocketAddress;
    private int _portInt;
    private int _sendPortInt;
    private MembershipKey _key;
    private ByteBuffer _byteData;
    private TransportBufferImpl _readData;
    private TransportBufferImpl _writeData;
    private boolean _finishedReading;
    private int _messageSize;
    private long _readSeqNum;
    private long _writeSeqNum = 0;
    private Lock _lock;
    int _state;
    int _endReadMsgPosition;
    int _beginReadMsgPosition;
    int _endWriteMsgPosition;
    int _beginWriteMsgPosition;
    DatagramChannel _datagramChannel;
    SocketAddress _senderAddress;
    boolean _bufferInUse = false;
    TransportBuffer _writePing = new TransportBufferImpl(EDF_PING_LEN);
    private int _instanceId;
    int _readInstanceId = 0;

    // info that is set on accept or connect from options
    final ChannelInfoImpl _channelInfo = new ChannelInfoImpl();

    
    // RsslConnectOptions start (for client connections)
    protected int _majorVersion;
    protected int _minorVersion;
    protected int _protocolType;
    // RsslConnectOptions end (for client connections)

    
    // common RsslConnectOptions and RsslAcceptOptions
    protected Object _userSpecObject;

    protected String _host = null;
    protected String _port = null;
    protected String _interface = null;
    protected String _sendHost = null;
    protected String _sendPort = null;

    protected boolean _blocking = false;

    public RsslSeqMulticastSocketChannel(SequencedMulticastProtocol socketProtocol, Pool channelPool)
    {
        // associate with pool
        pool(channelPool);

        _lock = new ReentrantLock();
        _lock = new ReentrantLock();
    }

    @Override
    public int connectionType()
    {
        return ConnectionTypes.SEQUENCED_MCAST;
    }
    
    @Override
    public int close(Error error)
    {
        assert (error != null) : "error cannot be null";

        int ret = TransportReturnCodes.SUCCESS;

        // first set channel state to closed
        try
        {
            if (_state != ChannelState.INACTIVE)
            {
                _state = ChannelState.INACTIVE;
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
            if (ret == TransportReturnCodes.FAILURE)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is inactive");

                return TransportReturnCodes.FAILURE;
            }
        }

        try
        {
            // close socket
            try
            {
                if (_key != null && _key.isValid())
                    _key.drop();
                _datagramChannel.close();
            }
            catch (IOException e)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel close failed ");
                ret = TransportReturnCodes.FAILURE;
            }

            // return channel to pool
            returnToPool();
        }
        finally
        {

        }

        return ret;
    }

    /* Write the EDF Header
     * 
     * Preconditions: everything has been range checked to ensure that no overflow is possible
     * 
     * Returns: 0 on success, -1 on failure (with the error populated).
     * The TransportBuffferImpl's position will be set to the beginning of the header.
     */
    private int writeHdr(TransportBufferImpl bufferInt, int flags, long seqNum, Error error)
    {
        int offset = 0;
        int iter = 0;
        int hdrlen = EDF_VERSION_LEN + EDF_FLAGS_LEN + EDF_PROTOCOL_TYPE_LEN + EDF_HDR_LENGTH_LEN + EDF_INSTANCE_ID_LEN + EDF_PROTO_VER_MAJOR_LEN
                + EDF_PROTO_VER_MINOR_LEN + EDF_SEQ_NUM_LEN;
        /* Should check input flags and compute the offset here.
         * Since this there aren't any, this is 0 for both, and we can leave the values as-is. */
        bufferInt.data().position(offset + iter);

        /* put on version info */
        bufferInt.data().put(EDF_MAX_VERSION);
        
        /* put on flags */
        bufferInt.data().put((byte)flags);
        
        /* protocol type */
        bufferInt.data().put((byte)_protocolType);

        /* put on length. This is static right now */
        bufferInt.data().put((byte)hdrlen);

        /* instance Id */
        bufferInt.data().putShort((short)_instanceId);

        /* protocol major */
        bufferInt.data().put((byte)_majorVersion);

        /* protocol minor */
        bufferInt.data().put((byte)_minorVersion);

        bufferInt.data().putInt((int)seqNum);

        /* Set the buffer's position to the beginning of the header.
         * If we needed to offset it due to optional members, set it to that offset. */
        bufferInt.data().position(0);

        return 0;
    }

    @Override
    public int init(InProgInfo inProg, Error error)
    {
        assert (inProg != null) : "inProg cannot be null";
        assert (error != null) : "error cannot be null";

        inProg.clear();

        return TransportReturnCodes.SUCCESS;
    }

    /* opts should be checked for null prior to this call. */
    void dataFromOptions(ConnectOptions opts)
    {
        assert (opts != null);

        _channelInfo._compressionType = opts.compressionType();
        _channelInfo._pingTimeout = opts.pingTimeout();
        _channelInfo._guaranteedOutputBuffers = 1;
        _channelInfo._maxOutputBuffers = 1;
        _channelInfo._numInputBuffers = opts.numInputBuffers();
        _channelInfo._sysSendBufSize = opts.sysSendBufSize();
        _channelInfo._sysRecvBufSize = opts.sysRecvBufSize();
        _majorVersion = opts.majorVersion();
        _minorVersion = opts.minorVersion();
        _protocolType = opts.protocolType();
        _instanceId = opts.seqMCastOpts().instanceId();
        _userSpecObject = opts.userSpecObject();
        _blocking = opts.blocking();
        
        if (opts.seqMCastOpts().maxMsgSize() > 65493)
            _channelInfo._maxFragmentSize = 65493;
        else
            _channelInfo._maxFragmentSize = opts.seqMCastOpts().maxMsgSize();

        if (_channelInfo._sysSendBufSize <= 0)
            _channelInfo.sysSendBufSize(_channelInfo._maxFragmentSize);
        if (_channelInfo._sysRecvBufSize <= 0)
            _channelInfo.sysRecvBufSize(_channelInfo._maxFragmentSize);

        if ((opts.segmentedNetworkInfo().recvAddress() != null && opts.segmentedNetworkInfo().recvAddress().length() > 0)
                || (opts.segmentedNetworkInfo().recvServiceName() != null && opts.segmentedNetworkInfo().recvServiceName().length() > 0))
        {
            _host = opts.segmentedNetworkInfo().recvAddress();
            _port = opts.segmentedNetworkInfo().recvServiceName();
            _interface = opts.segmentedNetworkInfo().interfaceName();
            _sendHost = opts.segmentedNetworkInfo().sendAddress();
            _sendPort = opts.segmentedNetworkInfo().sendServiceName();
        }
        else
        {
            _host = opts.unifiedNetworkInfo().address();
            _port = opts.unifiedNetworkInfo().serviceName();
            _interface = opts.unifiedNetworkInfo().interfaceName();
        }
    }

    int connect(ConnectOptions opts, Error error)
    {
        if (opts != null)
            dataFromOptions(opts);
        try
        {
            if (_channelInfo._maxFragmentSize <= 0) // Unacceptable for UDP, must be between 6 and 65507 (including header)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("Specified maximum buffer size must be between 0 and 65493");

                _state = ChannelState.CLOSED;

                return TransportReturnCodes.FAILURE;
            }

            if (_port != null && (_portInt = GetServiceByName.getServiceByName(_port)) == TransportReturnCodes.FAILURE)
                _portInt = Integer.valueOf(_port);

            if (_portInt < 1 || _portInt > 65535) // Port out of range
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("Specified port value is out of range (1-65535 inclusive)");

                _state = ChannelState.CLOSED;

                return TransportReturnCodes.FAILURE;
            }

            _ni = NetworkInterface.getByInetAddress(InetAddress.getByName(_interface));

            if (_ni == null)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("Invalid Network Interface on connect()");

                _state = ChannelState.CLOSED;

                return TransportReturnCodes.FAILURE;
            }
        }
        catch (NumberFormatException e1)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Number format exception for recieve address port number on connect()");

            _state = ChannelState.CLOSED;

            return TransportReturnCodes.FAILURE;
        }
        catch (IOException e1)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Invalid Network Interface on connect()");

            _state = ChannelState.CLOSED;

            return TransportReturnCodes.FAILURE;
        }

        try
        {
            if (System.getProperty("os.name").contains("Windows")) // Windows OS, only bind by port
            {
                _group = InetAddress.getByName(_host);
                _datagramChannel = DatagramChannel.open(StandardProtocolFamily.INET)
                        .setOption(StandardSocketOptions.SO_REUSEADDR, true)
                        .setOption(StandardSocketOptions.IP_MULTICAST_IF, _ni)
                        .bind(new InetSocketAddress(_portInt));
            }
            else
            // Not windows OS, bind by address and port
            {
                _group = InetAddress.getByName(_host);
                _datagramChannel = DatagramChannel.open(StandardProtocolFamily.INET)
                        .setOption(StandardSocketOptions.SO_REUSEADDR, true)
                        .setOption(StandardSocketOptions.IP_MULTICAST_IF, _ni)
                        .bind(new InetSocketAddress(_group, _portInt));
            }
            _datagramChannel.configureBlocking(_blocking);

            // Expose Key in future in GetChannelinfo()? Don't forget to drop() key on uninit
            _key = _datagramChannel.join(_group, _ni);

            if ((_sendHost != null && _sendHost.length() > 0) && (_sendPort != null && _sendPort.length() > 0))
            {
                if (_sendPort != null && (_sendPortInt = GetServiceByName.getServiceByName(_sendPort)) == TransportReturnCodes.FAILURE)
                    _sendPortInt = Integer.valueOf(_sendPort);

                if (InetAddress.getByName(_sendHost).isMulticastAddress())
                    _inetSocketAddress = new InetSocketAddress(InetAddress.getByName(_sendHost), _sendPortInt);
                else
                {
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Send address is not a multicast address");

                    _state = ChannelState.CLOSED;

                    return TransportReturnCodes.FAILURE;
                }
            }
            else
                _inetSocketAddress = new InetSocketAddress(_group, _portInt);
        }
        catch (Exception e)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getLocalizedMessage());

            _state = ChannelState.CLOSED;

            return TransportReturnCodes.FAILURE;
        }

        _state = ChannelState.ACTIVE;
        _finishedReading = true;

        _readData = new TransportBufferImpl(_channelInfo._maxFragmentSize + EDF_MAX_HDR_LEN);
        _byteData = ByteBuffer.allocate(_channelInfo._maxFragmentSize + EDF_MAX_HDR_LEN);
        _writeData = new TransportBufferImpl(_channelInfo._maxFragmentSize + EDF_MAX_HDR_LEN);

        try
        {
            if (_channelInfo._sysRecvBufSize > 0)
            {
                _datagramChannel.setOption(StandardSocketOptions.SO_RCVBUF, _channelInfo._sysRecvBufSize);
            }
            else
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("sysRecvBufSize cannot be less than 1");

                _state = ChannelState.CLOSED;

                return TransportReturnCodes.FAILURE;
            }
            if (_channelInfo._sysSendBufSize > 0)
            {
                _datagramChannel.setOption(StandardSocketOptions.SO_SNDBUF, _channelInfo._sysSendBufSize);
            }
            else
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("sysSendBufSize cannot be less than 1");

                _state = ChannelState.CLOSED;

                return TransportReturnCodes.FAILURE;
            }

            _bufferInUse = false;
        }
        catch (SocketException e)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getLocalizedMessage());

            _state = ChannelState.CLOSED;

            return TransportReturnCodes.FAILURE;
        }
        catch (IOException e)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getLocalizedMessage());

            _state = ChannelState.CLOSED;

            return TransportReturnCodes.FAILURE;
        }

        return TransportReturnCodes.SUCCESS;
    }

    @Override
    public TransportBuffer read(ReadArgs readArgs, Error error)
    {
        assert (readArgs != null) : "readArgs cannot be null";
        assert (error != null) : "error cannot be null";

        int readRetVal;

        if (_state != ChannelState.ACTIVE)
        {
            ((ReadArgsImpl)readArgs).readRetVal(TransportReturnCodes.FAILURE);
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Sequenced Multicast channel not active");

            return null;
        }

        try
        {
            _lock.lock();

            // Initial read amounts
            ((ReadArgsImpl)readArgs)._bytesRead = 0;
            ((ReadArgsImpl)readArgs)._uncompressedBytesRead = 0;
            ((ReadArgsImpl)readArgs).flags(0);
            ((ReadArgsImpl)readArgs).readRetVal(TransportReturnCodes.FAILURE);

            if (_finishedReading) // We were at beginning of message, not reading packed data
                try
                {
                    _byteData.clear();

                    _senderAddress = _datagramChannel.receive(_byteData);

                    // Set limit to position, since position is currently at the end of the message
                    _byteData.limit(_byteData.position());

                    if (_senderAddress == null && _byteData.limit() == 0) // No data read, empty or non-existent message
                    {
                        ((ReadArgsImpl)readArgs).readRetVal(TransportReturnCodes.READ_WOULD_BLOCK);
                        return null;
                    }

                    // We read a new packet, increment number of packets read
                    ((MCastStatsImpl)_channelInfo._multicastStats).mcastRcvd(_channelInfo._multicastStats.mcastRcvd() + 1);

                    // Set end of message to actual end of packet
                    _endReadMsgPosition = _byteData.limit();
                    // Reset position to 0 to start reading
                    _byteData.position(0);

                    // Set beginning of message to beginning of this particular message
                    _beginReadMsgPosition = _byteData.position();

                    // Read Sequence number
                    if (_byteData.remaining() < EDF_PING_LEN)
                    {
                        ((ReadArgsImpl)readArgs).readRetVal(TransportReturnCodes.FAILURE);
                        error.channel(this);
                        error.errorId(TransportReturnCodes.FAILURE);
                        error.sysError(0);
                        error.text("Incoming UDP packet is too short to contain a UPA Sequenced Multicast Header");
                        return null;
                    }

                    int hdrVersion;

                    int flags;
                    int protoType;
                    int hdrLen;
                    @SuppressWarnings("unused")
                    int protoMajor;
                    @SuppressWarnings("unused")
                    int protoMinor;
                    int iter = 0;

                    hdrVersion = _byteData.get(iter) & 0xFF;
                    iter += EDF_VERSION_LEN;

                    if (hdrVersion != EDF_MAX_VERSION)
                    {
                        ((ReadArgsImpl)readArgs).readRetVal(TransportReturnCodes.FAILURE);
                        error.channel(this);
                        error.errorId(TransportReturnCodes.FAILURE);
                        error.sysError(0);
                        error.text("Unknown UPA Sequenced Multicast header version.");
                        return null;
                    }

                    flags = _byteData.get(iter) & 0xFF;

                    if ((flags & SEQ_MCAST_FLAGS_RETRANSMIT) > 0)
                    {
                        ((ReadArgsImpl)readArgs)._flags |= ReadFlags.READ_RETRANSMIT;
                    }

                    iter += EDF_FLAGS_LEN;

                    protoType = _byteData.get(iter) & 0xFF;
                    iter += EDF_PROTOCOL_TYPE_LEN;
                    if (protoType != _protocolType)
                    {
                        ((ReadArgsImpl)readArgs).readRetVal(TransportReturnCodes.FAILURE);
                        error.channel(this);
                        error.errorId(TransportReturnCodes.FAILURE);
                        error.sysError(0);
                        error.text("Protocol type does not match configured protocol.");
                        return null;
                    }

                    hdrLen = _byteData.get(iter) & 0xFF;
                    iter += EDF_HDR_LENGTH_LEN;

                    _readInstanceId = _byteData.getShort(iter) & 0xFFFF;
                    iter += EDF_INSTANCE_ID_LEN;

                    protoMajor = _byteData.get(iter) & 0xFF;
                    iter += EDF_PROTO_VER_MAJOR_LEN;

                    protoMinor = _byteData.get(iter) & 0xFF;
                    iter += EDF_PROTO_VER_MINOR_LEN;

                    _readSeqNum = _byteData.getInt(iter) & 0xFFFFFFFFL;
                    iter += EDF_SEQ_NUM_LEN;

                    // Check if this is a ping
                    if (_endReadMsgPosition == EDF_PING_LEN)
                    {
                        ((ReadArgsImpl)readArgs)._flags |= ReadFlags.READ_SEQNUM | ReadFlags.READ_NODE_ID | ReadFlags.READ_INSTANCE_ID;
                        ((ReadArgsImpl)readArgs)._senderAddress = _senderAddress;
                        ((ReadArgsImpl)readArgs)._seqNum = _readSeqNum;
                        ((ReadArgsImpl)readArgs)._instanceId = _readInstanceId;
                        ((ReadArgsImpl)readArgs)._bytesRead = _endReadMsgPosition;
                        ((ReadArgsImpl)readArgs)._uncompressedBytesRead = _endReadMsgPosition;
                        ((ReadArgsImpl)readArgs)._readRetVal = TransportReturnCodes.READ_PING;
                        return null;
                    }

                    // We have parsed the entire header, now move the cursor to the end of the header, as indicated by the hdrLen

                    _byteData.position(hdrLen);

                    // Read message size (2 bytes)
                    if (_byteData.remaining() >= 2)
                        _messageSize = _byteData.getShort();
                    else
                        _messageSize = 0;

                    _messageSize = _messageSize & 0xFFFF; // Convert signed short to unsigned

                    if (_messageSize + _byteData.position() > _byteData.capacity()) // Our buffer is not big enough to read contents
                    {
                        error.channel(this);
                        error.errorId(TransportReturnCodes.FAILURE);
                        error.sysError(0);
                        error.text("Received buffer of size greater than set max buffer size");
                        _state = ChannelState.CLOSED;
                        return null;
                    }

                    if (_byteData.position() + _messageSize < _byteData.capacity())
                        _byteData.limit(_byteData.position() + _messageSize);
                    else
                        _byteData.limit(_byteData.capacity());

                    if (_byteData.limit() < _endReadMsgPosition) // Still more to read, packed message
                    {
                        // Set retVal to the remaining bytes in the message
                        _finishedReading = false;
                        readRetVal = _endReadMsgPosition - _byteData.limit();
                    }
                    else
                    {
                        readRetVal = TransportReturnCodes.SUCCESS;
                        _finishedReading = true;
                    }
                }
                catch (IOException e)
                {
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text(e.getLocalizedMessage());

                    return null;
                }
            else //We were not finished reading previous message because it was packed data
            {
                // Set new position to end of first message
                _byteData.position(_byteData.limit());

                if ((_byteData.limit() + EDF_MSG_LEN_LEN) >= _byteData.capacity())
                {
                    // We are over capacity, we cannot read this buffer because our max msg size is not big enough
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Received buffer of size greater than set max buffer size");
                    _state = ChannelState.CLOSED;
                    _finishedReading = true;
                    return null;
                }

                // Set new limit to end of messageSize
                _byteData.limit(_byteData.limit() + EDF_MSG_LEN_LEN);

                // Set beginning of message to beginning of this particular message
                _beginReadMsgPosition = _byteData.position();

                // We are reading packed data, get message size
                if (_byteData.remaining() >= EDF_MSG_LEN_LEN)
                    _messageSize = _byteData.getShort();
                else
                    _messageSize = 0;

                _messageSize = _messageSize & 0xFFFF; // Convert signed short to unsigned

                if (_messageSize + _byteData.position() > _byteData.capacity()) // Our buffer is not big enough to read contents
                {
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Received buffer of size greater than set max buffer size");
                    _state = ChannelState.CLOSED;
                    _finishedReading = true;
                    return null;
                }

                _byteData.limit(_byteData.position() + _messageSize);

                if (_byteData.limit() < _endReadMsgPosition) // Still more to read, packed message
                {
                    // Set retVal to the remaining bytes in the message
                    readRetVal = _endReadMsgPosition - _byteData.limit();
                    _finishedReading = false;
                }
                else
                {
                    readRetVal = TransportReturnCodes.SUCCESS;
                    _finishedReading = true;
                }
            }
        }
        finally
        {
            _lock.unlock();
        }

        ((ReadArgsImpl)readArgs)._flags |= ReadFlags.READ_SEQNUM | ReadFlags.READ_NODE_ID | ReadFlags.READ_INSTANCE_ID;
        ((ReadArgsImpl)readArgs)._senderAddress = _senderAddress;
        ((ReadArgsImpl)readArgs)._seqNum = _readSeqNum;
        ((ReadArgsImpl)readArgs)._instanceId = _readInstanceId;
        ((ReadArgsImpl)readArgs)._bytesRead = _byteData.limit() - _beginReadMsgPosition;
        ((ReadArgsImpl)readArgs)._uncompressedBytesRead = _byteData.limit() - _beginReadMsgPosition;
        ((ReadArgsImpl)readArgs)._readRetVal = readRetVal;

        _readData.data(_byteData);
        return _readData;
    }

    public int write(TransportBuffer bufferInt, WriteArgs writeArgs, Error error)
    {
        if (_state != ChannelState.ACTIVE)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Sequenced Multicast channel not active");

            return TransportReturnCodes.FAILURE;
        }

        ((WriteArgsImpl)writeArgs)._bytesWritten = 0;
        ((WriteArgsImpl)writeArgs)._uncompressedBytesWritten = 0;

        _writeData = (TransportBufferImpl)bufferInt;

        if (_writeData == null)
        {
            // User hasn't set a message at all on this packet, do not send.
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Encoded buffer of length zero cannot be written.");

            return TransportReturnCodes.FAILURE;
        }

        try
        {
            _lock.lock();

            _endWriteMsgPosition = _writeData.data().position();

            if (_endWriteMsgPosition > _writeData._packedMsgLengthPosition) // User put a message in buffer
            {
                _writeData.data().position(_writeData._packedMsgLengthPosition - EDF_MSG_LEN_LEN);
                _writeData.data().putShort((short)(_endWriteMsgPosition - _writeData.data().position() - EDF_MSG_LEN_LEN));

                _writeData._packedMsgLengthPosition = _endWriteMsgPosition + EDF_MSG_LEN_LEN; // New packed message length
            }
            else
            {
                if (_writeData._packedMsgLengthPosition == EDF_MAX_HDR_LEN)
                {
                    // User hasn't set a message at all on this packet, do not send.
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Encoded buffer of length zero cannot be written.");

                    return TransportReturnCodes.FAILURE;
                }
                // Remove length from ending of buffer since there is no additional message
                _endWriteMsgPosition = _writeData._packedMsgLengthPosition - EDF_MSG_LEN_LEN;
            }

            if (((writeArgs.flags() & WriteFlags.WRITE_RETRANSMIT) > 0 && (writeArgs.flags() & WriteFlags.WRITE_SEQNUM) == 0)
                    || ((writeArgs.flags() & WriteFlags.WRITE_RETRANSMIT) == 0 && (writeArgs.flags() & WriteFlags.WRITE_SEQNUM) > 0))
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("WRITE_SEQNUM flag and WRITE_RETRANSMIT flags have to be both on for retransmit message.");

                return TransportReturnCodes.FAILURE;
            }

            long seqNum = 0;
            if ((writeArgs.flags() & WriteFlags.WRITE_SEQNUM) > 0)
            {
                seqNum = writeArgs.seqNum();
                if (seqNum < 0 || seqNum > 4294967295L)
                {
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Input SeqNum out of bound.");
                    return TransportReturnCodes.FAILURE;
                }
            }
            else
            {
                // Increment and set sequence number on packet
                _writeSeqNum++;
                if (_writeSeqNum > 4294967295L)
                {
                    _writeSeqNum = 1;
                }
                seqNum = _writeSeqNum;
            }

            int flags = 0x0;
            if ((writeArgs.flags() & WriteFlags.WRITE_RETRANSMIT) > 0)
                flags = SEQ_MCAST_FLAGS_RETRANSMIT;

            // Put the header onto the buffer
            if (writeHdr(_writeData, flags, seqNum, error) == -1)
            {
                // Error info will be populated by the writeHdr method
                return TransportReturnCodes.FAILURE;
            }

            int hdrOffset = _writeData.data().position();

            _writeData.data().limit(_endWriteMsgPosition); // Set limit to end of packed message length

            try
            {
                if (_datagramChannel.send(_writeData.data(), _inetSocketAddress) != 0)
                {
                    ((WriteArgsImpl)writeArgs)._bytesWritten = _writeData.data().limit() - hdrOffset;
                    ((WriteArgsImpl)writeArgs)._uncompressedBytesWritten = _writeData.data().limit() - hdrOffset;
                    // We sent a new packet, increment number of packets sent
                    ((MCastStatsImpl)_channelInfo._multicastStats).mcastSent(_channelInfo._multicastStats.mcastSent() + 1);
                    // Write data
                    _bufferInUse = false;

                    return TransportReturnCodes.SUCCESS;
                }
                else
                    return TransportReturnCodes.WRITE_CALL_AGAIN;
            }
            catch (IOException e)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text(e.getLocalizedMessage());

                return TransportReturnCodes.FAILURE;
            }
        }
        finally
        {
            _lock.unlock();
        }
    }

    @Override
    public TransportBuffer getBuffer(int size, boolean packedBuffer, Error error)
    {
        try
        {
            _lock.lock();

            // return FAILURE if channel not active
            if (_state != ChannelState.ACTIVE)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in the active state for getBuffer");

                return null;
            }

            // return error if buffer already in use
            if (_bufferInUse)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.NO_BUFFERS);
                error.sysError(0);
                error.text("Buffer already in use");

                return null;
            }

            if (size > _channelInfo._maxFragmentSize)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("Buffer size exceeds maxMsgSize");

                return null;
            }

            _writeData.data().position(EDF_MAX_HDR_LEN);
            _writeData._packedMsgLengthPosition = EDF_MAX_HDR_LEN;
            _writeData.data().limit(_writeData.data().position() + size); // Limit of the returned buffer is the size requested

            _bufferInUse = true;

        }
        finally
        {
            _lock.unlock();
        }

        return _writeData;
    }

    @Override
    public int packBuffer(TransportBuffer bufferInt, Error error)
    {
        assert (bufferInt != null) : "buffer cannot be null";
        assert (error != null) : "error cannot be null";

        if (_state != ChannelState.ACTIVE)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Sequenced Multicast channel not active");

            return TransportReturnCodes.FAILURE;
        }

        _writeData = (TransportBufferImpl)bufferInt;

        if (bufferInt.length() > _writeData.capacity() - _writeData.data().position())
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Buffer size exceeds maxMsgSize");

            return TransportReturnCodes.FAILURE;
        }

        _endWriteMsgPosition = _writeData.data().position();

        if (_writeData._packedMsgLengthPosition >= _writeData.capacity())
            return 0; // No more packing possible

        _writeData.data().position(_writeData._packedMsgLengthPosition - EDF_MSG_LEN_LEN);
        _writeData.data().putShort((short)(_endWriteMsgPosition - _writeData.data().position() - EDF_MSG_LEN_LEN));

        _writeData._packedMsgLengthPosition = _endWriteMsgPosition + EDF_MSG_LEN_LEN;

        if (_writeData.data().limit() >= _endWriteMsgPosition + EDF_MSG_LEN_LEN)
            _writeData.data().position(_endWriteMsgPosition + EDF_MSG_LEN_LEN);

        return (_writeData.data().limit() - _writeData._packedMsgLengthPosition); // Return how many bytes are left for packing
    }

    @Override
    public int flush(Error error)
    {
        if (_state == ChannelState.ACTIVE)
            return TransportReturnCodes.SUCCESS;
        else
            return TransportReturnCodes.FAILURE;
    }

    @Override
    public int info(ChannelInfo info, Error error)
    {
        assert (info != null) : "info cannot be null";
        assert (error != null) : "error cannot be null";

        int ret = TransportReturnCodes.SUCCESS;
        try
        {
            _lock.lock();
            if (_state == ChannelState.ACTIVE)
            {
                ((ChannelInfoImpl)info).maxFragmentSize(_channelInfo._maxFragmentSize);
                ((ChannelInfoImpl)info).maxOutputBuffers(_channelInfo._maxOutputBuffers);
                ((ChannelInfoImpl)info).guaranteedOutputBuffers(_channelInfo._guaranteedOutputBuffers);
                ((ChannelInfoImpl)info).numInputBuffers(_channelInfo._numInputBuffers);
                ((ChannelInfoImpl)info).pingTimeout(_channelInfo._pingTimeout);
                ((ChannelInfoImpl)info).clientToServerPings(_channelInfo._clientToServerPings);
                ((ChannelInfoImpl)info).serverToClientPings(_channelInfo._serverToClientPings);
                ((ChannelInfoImpl)info).compressionType(_channelInfo._compressionType);
                ((ChannelInfoImpl)info).compressionThreshold(_channelInfo._compressionThreshold);
                ((ChannelInfoImpl)info).priorityFlushStrategy(_channelInfo._priorityFlushStrategy);
                ((ChannelInfoImpl)info)._receivedComponentInfoList = _channelInfo._receivedComponentInfoList;
                ((ChannelInfoImpl)info).clientIP(_channelInfo._clientIP);
                ((ChannelInfoImpl)info).clientHostname(_channelInfo.clientHostname());
                ((ChannelInfoImpl)info).sysRecvBufSize(_datagramChannel.socket().getReceiveBufferSize());
                ((ChannelInfoImpl)info).sysSendBufSize(_datagramChannel.socket().getSendBufferSize());
                ((ChannelInfoImpl)info).multicastStats(_channelInfo._multicastStats);
            }
            else
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("channel not in active state ");
                ret = TransportReturnCodes.FAILURE;
            }
        }
        catch (SocketException e)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getLocalizedMessage());

            _state = ChannelState.CLOSED;

            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            _lock.unlock();
        }
        return ret;
    }

    @Override
    public int ioctl(int code, Object value, Error error)
    {
        return TransportReturnCodes.SUCCESS;
    }

    @Override
    public int ioctl(int code, int value, Error error)
    {
        assert (error != null) : "error cannot be null";

        int retCode = TransportReturnCodes.FAILURE;
        try
        {
            _lock.lock();

            // return FAILURE if channel not active or not initializing
            if ((_state != ChannelState.ACTIVE) && (_state != ChannelState.INITIALIZING))
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in the active or initializing state");

                return TransportReturnCodes.FAILURE;
            }

            switch (code)
            {
                case IoctlCodes.SYSTEM_WRITE_BUFFERS:
                    if (value > 0)
                    {
                        _datagramChannel.setOption(StandardSocketOptions.SO_SNDBUF, value);
                        retCode = TransportReturnCodes.SUCCESS;
                    }
                    else
                    {
                        error.channel(this);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text("value must be (1 >= value < 2^31");
                    }
                    break;
                case IoctlCodes.SYSTEM_READ_BUFFERS:
                    if (value > 0)
                    {
                        _datagramChannel.setOption(StandardSocketOptions.SO_RCVBUF, value);
                        retCode = TransportReturnCodes.SUCCESS;
                    }
                    else
                    {
                        error.channel(this);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text("value must be (1 >= value < 2^31");
                    }
                    break;
                default:
                    error.channel(this);
                    error.errorId(retCode);
                    error.sysError(0);
                    error.text("Code is not valid.");
            }
        }
        catch (SocketException e)
        {
            _state = ChannelState.CLOSED;
            error.channel(this);
            error.errorId(retCode);
            error.text("exception occurred when setting value \"" + value + "\" for IoctlCode \"" + code + "\", exception=" + e.toString());
        }
        catch (IllegalArgumentException e)
        {
            error.channel(this);
            error.errorId(retCode);
            error.text(e.getLocalizedMessage());
        }
        catch (IOException e)
        {
            error.channel(this);
            error.errorId(retCode);
            error.text(e.getLocalizedMessage());
        }
        finally
        {
            _lock.unlock();
        }

        return retCode;
    }

    @Override
    public int bufferUsage(Error error)
    {
        if (_state == ChannelState.ACTIVE)
            return TransportReturnCodes.SUCCESS;
        else
            return TransportReturnCodes.FAILURE;
    }

    @Override
    public int releaseBuffer(TransportBuffer bufferInt, Error error)
    {
        assert (error != null) : "error cannot be null";

        if (bufferInt == null)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("buffer cannot be null");
            return TransportReturnCodes.FAILURE;
        }

        try
        {
            _lock.lock();

            // return FAILURE if channel inactive
            if (_state == ChannelState.INACTIVE)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is in inactive state");
                return TransportReturnCodes.FAILURE;
            }

            if (_bufferInUse)
            {
                _bufferInUse = false;
                _writeData._data.clear();
            }
        }
        finally
        {
            _lock.unlock();
        }
        return TransportReturnCodes.SUCCESS;
    }

    @Override
    public int ping(Error error)
    {
        // Send a header with the current sequence number
        _writePing.data().clear();

        if (writeHdr((TransportBufferImpl)_writePing, 0, _writeSeqNum, error) == -1)
        {
            // Error text and info will be set in the writeHdr method
            return TransportReturnCodes.FAILURE;
        }

        try
        {
            _lock.lock();

            // We sent a new packet, increment number of packets sent
            ((MCastStatsImpl)_channelInfo._multicastStats).mcastSent(_channelInfo._multicastStats.mcastSent() + 1);

            // Write data
            _datagramChannel.send(_writePing.data(), _inetSocketAddress);

            ((MCastStatsImpl)_channelInfo._multicastStats).mcastSent(_channelInfo._multicastStats.mcastSent() + 1);
            return TransportReturnCodes.SUCCESS;

        }
        catch (IOException e)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getLocalizedMessage());

            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            _lock.unlock();
        }
    }

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

    @Override @Deprecated
    public SocketChannel scktChannel()
    {
        return null;
    }

    @Override @Deprecated
    public SocketChannel oldScktChannel()
    {
        return null;
    }

    @Override
    public SelectableChannel selectableChannel()
    {
        return _datagramChannel;
    }

    @Override
    public SelectableChannel oldSelectableChannel()
    {
        return null;
    }

    @Override
    public int pingTimeout()
    {
        return _channelInfo._pingTimeout;
    }

    @Override
    public Object userSpecObject()
    {
        return _userSpecObject;
    }

    @Override
    public boolean blocking()
    {
        return _datagramChannel.isBlocking();
    }

    public int reconnectClient(Error error)
    {
        return TransportReturnCodes.SUCCESS;
    }

	@Override
	public String hostname() {
		// TODO Auto-generated method stub
		return null;
	}

}
