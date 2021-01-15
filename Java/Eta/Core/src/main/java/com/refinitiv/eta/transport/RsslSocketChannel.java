package com.refinitiv.eta.transport;

import java.io.IOException;
import java.net.*;
import java.nio.ByteBuffer;
import java.nio.channels.SelectableChannel;

import com.refinitiv.proxy.authentication.CredentialName;
import com.refinitiv.proxy.authentication.CredentialsFactory;
import com.refinitiv.proxy.authentication.ICredentials;
import com.refinitiv.proxy.authentication.IProxyAuthenticator;
import com.refinitiv.proxy.authentication.IProxyAuthenticatorResponse;
import com.refinitiv.proxy.authentication.ProxyAuthenticationException;
import com.refinitiv.proxy.authentication.ProxyAuthenticatorFactory;
import com.refinitiv.proxy.authentication.ResponseCodeException;

// Init state transitions for consumer with socket connection:
//
//                           +--------------------------+
//                           |         CONNECTING       | 
//                           |          (start)         |
//                           +--------------------------+
// send RIPC connect                       |
// (initChnlSendConnectReq(inProg, error)) |
//                                         |
//                                         |
//                                         v
//                           +--------------------------+
//                           |        WAIT_ACK          |
//                           | (waiting for RIPC reply) |
//                           +--------------------------+
// received RIPC reply                    |
// so go to ACTIVE                        |
//                                        |
//                                        |
//                                        v
//                         +-------------------------------+
//                         |            ACTIVE             |
//                         | (ready to send login request) |
//                         +-------------------------------+

class RsslSocketChannel extends EtaNode implements Channel
{
    class BigBuffersPool
    {
        Pool[] _pools = new Pool[32];
        int _maxSize = 0;
        int _maxPool = 0;
        int _fragmentSize;
        RsslSocketChannel _poolOwner;

        BigBuffersPool(int fragmentSize, RsslSocketChannel poolOwner)
        {
            // this pool should be created after the fragment size is known
            _poolOwner = poolOwner;
            _fragmentSize = fragmentSize;
            _maxSize = fragmentSize * 2;
        }

        EtaNode poll(int size, boolean isWriteBuffer)
        {
            EtaNode buffer = null;
            // determine which pool to use
            int poolSize = _fragmentSize * 2;
            int poolIndex = 0;
            while (size > poolSize)
            {
                poolSize = poolSize * 2;
                poolIndex++;
            }
            if (poolSize > _maxSize)
            {
                _maxSize = poolSize;
                _maxPool = poolIndex;
                // create pool and a buffer of this size
                Pool pool = new Pool(_poolOwner);
                _pools[poolIndex] = pool;
                if (isWriteBuffer)
                {
                    return new BigBuffer(pool, poolSize);
                }
                else
                {
                    return new ByteBufferPair(pool, poolSize, true);
                }
            }

            // The size is smaller then max, so traverse through pools to find available buffer
            for (int i = poolIndex; i <= _maxPool; i++)
            {
                if (_pools[i] != null)
                {
                    buffer = _pools[i].poll();
                    if (buffer != null)
                        return buffer;
                }
            }

            // There was no available buffer, so create new.
            // First check if the pool exists.
            if (_pools[poolIndex] == null)
            {
                Pool pool = new Pool(_poolOwner);
                _pools[poolIndex] = pool;
            }
            if (isWriteBuffer)
            {
                return new BigBuffer(_pools[poolIndex], poolSize);
            }
            else
            {
                return new ByteBufferPair(_pools[poolIndex], poolSize, true);
            }
        }
    }

    static final int READ_RECEIVE_BUFFER_SIZE = 64 * 1024; // KB
    static final int MIN_READ_BUFFER_SIZE = READ_RECEIVE_BUFFER_SIZE * 2;

    // for memory management
    SocketProtocol _transport; // for junit is not final, even though it should be
    final Lock _realReadLock = new ReentrantLock();
    final Lock _realWriteLock = new ReentrantLock();
    final Lock _dummyReadLock = new DummyLock();
    final Lock _dummyWriteLock = new DummyLock();
    int _lockType;
    Lock _readLock;
    Lock _writeLock;
    ServerImpl _server = null; // set on accept
    int _used = 0;
    final Pool _availableBuffers = new Pool(this);     // pool of SocketBuffers
    final Pool _availableHTTPBuffers = new Pool(this); // pool of HTTPSocketBuffers
    SocketBuffer _currentBuffer = null;
    BigBuffersPool _bigBuffersPool;

    // info that is set on accept or connect from options
    final ChannelInfoImpl _channelInfo = new ChannelInfoImpl();

    // for write(), flush() and ping() methods
    final static String DEFAULT_PRIORITY_FLUSH_ORDER = "HMHLHM";
    final private int MAX_FLUSH_STRATEGY = 32;
    final static int PING_BUFFER_SIZE = 3;

    protected final int DEFAULT_HIGH_WATER_MARK = 6144;
    protected final int DEFAULT_CLIENT_KEY_HIGH_WATER_MARK = 14;
    final private EtaQueue[] _flushOrder = new EtaQueue[MAX_FLUSH_STRATEGY];
    int _flushOrderPosition = 0;
    final ByteBuffer[] _gatheringWriteArray = new ByteBuffer[MAX_FLUSH_STRATEGY];
    final TransportBufferImpl[] _releaseBufferArray = new TransportBufferImpl[MAX_FLUSH_STRATEGY];
    int _writeArrayMaxPosition = 0;
    int _writeArrayPosition = 0;
    boolean _isFlushPending = false;
    final EtaQueue _highPriorityQueue = new EtaQueue();
    final EtaQueue _mediumPriorityQueue = new EtaQueue();
    final EtaQueue _lowPriorityQueue = new EtaQueue();
    protected ByteBuffer _pingBuffer = ByteBuffer.allocateDirect(PING_BUFFER_SIZE);
    int _highWaterMark;
    int _totalBytesQueued = 0;

    // RIPC handshake manager
    IpcProtocolManager _ipcProtocolManager = new IpcProtocolManager();

    // Component Info to send during RIPC handshake (ConnectReq or ConnectAck).
    // This is hidden from user.
    // Internal clients can use Channel.ioctl() to set.
    ComponentInfo _componentInfo = new ComponentInfoImpl();

    // Component version info passed in through ConnectOptions
    ComponentInfo _connectOptsComponentInfo = null;

    byte[] _junitTestBuffer;   // only used for JUnit testing
    int _junitTestBufPosition; // only used for JUnit testing
    boolean _isJunitTest;

    // for proxy
    protected static final String CHAR_ENCODING = "US-ASCII";
    protected static final String USER_AGENT = "User-Agent: ETA/Java\r\n";
    protected static final String PROXY_CONNECTION_KEEP_ALIVE = "Proxy-Connection: Keep-Alive\r\n";
    protected static final String PRAGMA_NO_CACHE = "Pragma: no-cache\r\n";
    protected static final String EOL = "\r\n";
    protected boolean _httpProxy = false;
    String _httpProxyHost = null;
    protected int _httpProxyPort = 0;
    protected String _objectName = null;
    protected ICredentials _proxyCredentails = null;
    protected IProxyAuthenticator _proxyAuthenticator = null;
    protected String _additionalHttpConnectParams = null;
    String db;  // debug env variable
    ByteBuffer _httpPOSTwriteBuffer = ByteBuffer.allocateDirect(DEFAULT_HIGH_WATER_MARK);
    int httpOKretry = 0;  // needed for recovery through a proxy
    protected int httpOKsize;
    
    ConnectOptions _cachedConnectOptions = null;
    InetSocketAddress _cachedInetSocketAddress = null;
    InetSocketAddress _cachedBindInetSocketAddress = null;

    // Used only for debugging
    long _totalBytesRead = 0;

    // Used to print/log debug output, must be used within the scope of a lock
    StringBuilder _debugOutput;

    SocketHelper _scktChannel;
    SocketHelper _oldScktChannel;

    int _state;

    
    // RsslConnectOptions start (for client connections)
    protected int _majorVersion;
    protected int _minorVersion;
    protected int _protocolType;
    // RsslConnectOptions end (for client connections)

    
    // RsslAcceptOptions start (for server accepted connections)
    protected boolean _nakMount;
    // RsslAcceptOptions end (for server accepted connections)

    
    // common RsslConnectOptions and RsslAcceptOptions
    protected Object _userSpecObject;

    // for JUNIT
    private Integer _overrideReadBufferCapacity = null;

    // RIPC handshake state
    protected int _initChnlState;

    // RIPC handshake buffers - using separate read and write buffers buffer,
    // so that we don't need to create additional memory to store the Component (Version) Info.

    protected ByteBuffer _initChnlReadClientKeyBuffer = ByteBuffer.allocateDirect(DEFAULT_CLIENT_KEY_HIGH_WATER_MARK);
    protected ByteBuffer _initChnlReadBuffer = ByteBuffer.allocateDirect(DEFAULT_HIGH_WATER_MARK);
    protected ByteBuffer _initChnlWriteBuffer = ByteBuffer.allocateDirect(DEFAULT_HIGH_WATER_MARK);

    // RIPC Protocol version
    IpcProtocol _ipcProtocol;

    // RIPC compression variables
    short _sessionCompLevel = 6;
    int _sessionCompLowThreshold = 0;
    final int ZLIB_COMPRESSION_THRESHOLD = 30;
    final int LZ4_COMPRESSION_THRESHOLD = 300;
    int _sessionInDecompress;
    int _sessionOutCompression;

    TransportBufferImpl _decompressBuffer;
    final Compressor _ZlibCompressor = new ZlibCompressor();
    final Compressor _Lz4Compressor = new Lz4Compressor();
    Compressor _compressor;
    protected int _compressPriority = 99; // messages of this priority are compressed

    // RIPC constants
    static final int IPC_100_CONN_ACK = 10;
    static final int IPC_DATA = 0x2; // normal, uncompressed data - this is mostly here as an integrity check
    static final int IPC_PACKING = 0x10; // packing is on in this buffer

    static final int RIPC_HDR_SIZE = 3;
    static final int RIPC_PACKED_HDR_SIZE = 2;

    /* _internalMaxFragmentSize is used internal to represent the size of the buffers that will be create internally.
     * The size will be RIPC MAX_USER_MSG_SIZE + RIPC_HDR_SIZE.
     * Note that this is different from _channelInfo._maxFragmentSize which is returned to the user
     * and is RIPC MAX_USER_MSG_SIZE - RIPC PACKED_HDR_SIZE.
     */
    int _internalMaxFragmentSize;

    ByteBufferPair _readIoBuffer;
    protected TransportBufferImpl _appReadBuffer;
    protected ReadBufferStateMachine _readBufStateMachine;

    protected String _host = null;
    protected String _port = null;
    protected int _portIntValue = 0;

    // for http tunneling
    protected boolean _http = false;
    protected boolean _encrypted = false;
    protected int HTTP_HEADER4 = 0;   // HTTP chunk header size for RIPC Connection Reply (value=4 when http)
    protected int CHUNKEND_SIZE = 0;  // HTTP chunk end size, i.e. /r/n (value = 2 when http)
    protected int HTTP_PROVIDER_EXTRA = 8;

    boolean _isProviderHTTP = false;
    RsslHttpSocketChannelProvider _providerHelper = null;
    Integer _providerSessionId;
    boolean _needCloseSocket = true;

    long _shared_key; // shared encryption key - 0 if not negotiated

    /* The end of an HTTP response */
    protected static final String END_OF_RESPONSE = "\r\n\r\n";

    /* The maximum number of times we will ignore an HTTP response while connecting thorough a proxy */
    protected static final int MAX_IGNORED_RESPONSES = 10000;

    /* Used to re-assemble "incomplete" HTTP responses while connecting thorough a proxy */
    protected StringBuilder _proxyConnectResponse = new StringBuilder();

    /* The number of times we ignored an HTTP response while connecting through a proxy */
    protected int _ignoredConnectResponses = 0;

    /* A pool of ByteBufferPair} used to reassemble fragmented messages
     * (the main "read IO buffer" is also acquired from this pool). 
     * Using a Red-Black tree to sort the buffers by size ensures all operations on the
     * pool (search, insert, remove) happen in O(log n) time.
     */
    private final BigBuffersPool _bufferPool = new BigBuffersPool(6144, this);

    // initialize channel states
    class InitChnlState
    {
        public static final int INACTIVE = 0;
        public static final int READ_HDR = 1;
        public static final int COMPLETE = 2;
        public static final int ACTIVE = 3;
        public static final int CONNECTING = 4;
        public static final int WAIT_ACK = 6;

        public static final int PROXY_CONNECTING = 13;      // used when http tunneling
        public static final int CLIENT_WAIT_PROXY_ACK = 14; // used when http tunneling
        public static final int CLIENT_WAIT_HTTP_ACK = 15;  // used when http tunneling

        public static final int RECONNECTING = 16;    // used when the far end closes the connection and need to reconnect in init()

        public static final int HTTP_CONNECTING = 17; // used when http tunneling

        public static final int WAIT_CLIENT_KEY = 18; // used with conn version 14 or higher for key exchange
    }

    protected RsslSocketChannel(SocketProtocol transport, Pool channelPool, SocketHelper socketHelper)
    {
        // _maxFragmentSize (which we return to the user) is RIPC MAX_USER_MSG_SIZE - RIPC PACKED_HDR_SIZE.
        // Buffer sizes will be allocated at _internalMaxFragmentSize as RIPC MAX_USER_MSG_SIZE + RIPC_HDR_SIZE.
        _channelInfo._maxFragmentSize = (6 * 1024) - RIPC_PACKED_HDR_SIZE;
        _internalMaxFragmentSize = (6 * 1024) + RIPC_HDR_SIZE;

        _transport = transport;
        _ipcProtocol = null;

        // initialize _priorityFlushStrategy
        flushOrder(DEFAULT_PRIORITY_FLUSH_ORDER);

        // initialize _highWaterMark
        _highWaterMark = DEFAULT_HIGH_WATER_MARK;

        // associate with pool
        pool(channelPool);

        // pre-populate ping message since always the same
        _pingBuffer.putShort(0, (short)RIPC_HDR_SIZE); // ripc header length
        _pingBuffer.put(2, (byte)2); // ripc flag indicating data

        // possible http tunneling, so set ReadBufferSTateMachine accordingly
        if (transport.isHTTP())
            _readBufStateMachine = new ReadBufferStateMachineHTTP(this);
        else
            _readBufStateMachine = new ReadBufferStateMachine(this); // no http tunneling

        _providerHelper = null;
        _isProviderHTTP = false;
        _needCloseSocket = true;
        _state = ChannelState.INACTIVE;

        _shared_key = 0;
        _scktChannel = socketHelper;
    }

    RsslSocketChannel(SocketProtocol transport, Pool channelPool, boolean encrypted)
    {
        this(transport, channelPool, encrypted ? new EncryptedSocketHelper() : new SocketHelper());
        _encrypted = encrypted;
    }
    
    RsslSocketChannel(SocketProtocol transport, Pool channelPool)
    {
        this(transport, channelPool, false);
    }

    /* TEST ONLY: This is not a valid constructor. (only for JUnit tests) */
    protected RsslSocketChannel()
    {
        _transport = null;
        _pool = null;
        _ipcProtocol = null;

        // _maxFragmentSize (which we return to the user) is RIPC MAX_USER_MSG_SIZE - RIPC PACKED_HDR_SIZE.
        // Buffer sizes will be allocated at _internalMaxFragmentSize as RIPC MAX_USER_MSG_SIZE + RIPC_HDR_SIZE.
        _channelInfo._maxFragmentSize = (6 * 1024) - RIPC_PACKED_HDR_SIZE;
        _internalMaxFragmentSize = (6 * 1024) + RIPC_HDR_SIZE;
        _appReadBuffer = new TransportBufferImpl(_internalMaxFragmentSize);

        // initialize _priorityFlushStrategy
        flushOrder(DEFAULT_PRIORITY_FLUSH_ORDER);

        // initialize _highWaterMark
        _highWaterMark = DEFAULT_HIGH_WATER_MARK;

        _readLock = new ReentrantLock();
        _writeLock = new ReentrantLock();

        _readBufStateMachine = new ReadBufferStateMachine(this);
        _providerHelper = null;
        _isProviderHTTP = false;
        _needCloseSocket = true;
        _state = ChannelState.INACTIVE;
        _shared_key = 0;
        _scktChannel = new SocketHelper();
    }

    /* TEST ONLY: This is not a valid constructor. (only for JUnit tests) */
    protected RsslSocketChannel(int connectionType)
    {
        _transport = null;
        _pool = null;
        _ipcProtocol = null;

        // _maxFragmentSize (which we return to the user) is RIPC MAX_USER_MSG_SIZE - RIPC PACKED_HDR_SIZE.
        // Buffer sizes will be allocated at _internalMaxFragmentSize as RIPC MAX_USER_MSG_SIZE + RIPC_HDR_SIZE.
        _channelInfo._maxFragmentSize = (6 * 1024) - RIPC_PACKED_HDR_SIZE;
        _internalMaxFragmentSize = (6 * 1024) + RIPC_HDR_SIZE;
        _appReadBuffer = new TransportBufferImpl(_internalMaxFragmentSize);

        // initialize _priorityFlushStrategy
        flushOrder(DEFAULT_PRIORITY_FLUSH_ORDER);

        // initialize _highWaterMark
        _highWaterMark = DEFAULT_HIGH_WATER_MARK;

        _readLock = new ReentrantLock();
        _writeLock = new ReentrantLock();

        _readBufStateMachine = new ReadBufferStateMachine(this);
        _shared_key = 0;
        
        _scktChannel = connectionType == ConnectionTypes.ENCRYPTED ? new EncryptedSocketHelper() : new SocketHelper();
    }

    @Override
    public int connectionType()
    {
        if (_encrypted)
        {
            return ConnectionTypes.ENCRYPTED;
        }
        else
        {
            return ConnectionTypes.SOCKET;
        }
    }

    /* Resets this object back to default values.
     * This is called from Channel.close() prior to the Channel being returned to the pool.
     */
    protected void resetToDefault()
    {
        _ipcProtocol = null; // reset the Ipc protocol for next connect().
        _channelInfo.clear();
        _componentInfo.componentVersion().clear();
        _cachedConnectOptions = null;
        _cachedInetSocketAddress = null;
        _cachedBindInetSocketAddress = null;

        if (DEFAULT_PRIORITY_FLUSH_ORDER.equals(_channelInfo._priorityFlushStrategy) == false)
            flushOrder(DEFAULT_PRIORITY_FLUSH_ORDER);

        // initialize _highWaterMark
        _highWaterMark = DEFAULT_HIGH_WATER_MARK;

        _providerHelper = null;
        _isProviderHTTP = false;

        _used = 0;
        _currentBuffer = null;
        _needCloseSocket = true;
        _state = ChannelState.INACTIVE;
        _shared_key = 0;
    }

    /* This method is used for dependency injection of the read lock in unit tests
     * (lock is an implementation of RsslLock)
     */
    protected void readLock(Lock lock)
    {
        if (lock == null)
        {
            throw new NullPointerException();
        }

        _readLock = lock;
    }

    /*
     * This method is overridden by the JUnit tests to simulate reading from a
     * real java.nio.channels.SocketChannel
     * 
     * Reads a sequence of bytes from this channel into the given buffer.
     * An attempt is made to read up to r bytes from the channel, where r is the
     * number of bytes remaining in the buffer, that is, dst.remaining(), at the
     * moment this method is invoked.
     * Suppose that a byte sequence of length n is read, where 0 <= n <= r.
     * This byte sequence will be transferred into the buffer so that the first byte
     * in the sequence is at index p and the last byte is at index p + n - 1,
     * where p is the buffer's position at the moment this method is invoked.
     * Upon return the buffer's position will be equal to p + n; its limit will
     * not have changed.
     * A read operation might not fill the buffer, and in fact it might not read
     * any bytes at all. Whether or not it does so depends upon the nature and
     * state of the channel. A socket channel in non-blocking mode, for example,
     * cannot read any more bytes than are immediately available from the
     * socket's input buffer; similarly, a file channel cannot read any more
     * bytes than remain in the file. It is guaranteed, however, that if a
     * channel is in blocking mode and there is at least one byte remaining in
     * the buffer then this method will block until at least one byte is read.
     * This method may be invoked at any time. If another thread has already
     * initiated a read operation upon this channel, however, then an invocation
     * of this method will block until the first operation is complete.
     * Specified by: read(...) in ReadableByteChannel
     * 
     * (dst is the buffer into which bytes are to be transferred)
     * 
     * Returns the number of bytes read, possibly zero, or -1 if the channel has reached end-of-stream
     * 
     * Throws IOException If some other I/O error occurs
     */
    protected int read(ByteBuffer dst) throws IOException
    {
        return _scktChannel.read(dst);
    }

    /* This method is overridden by the JUnit tests to simulate reading from a
     * real java.nio.channels.SocketChannel
     * This method is only used during init() (RIPC handshake).
     * It will buffer input until a full message is read.
     * 
     * Returns  the number of bytes read, or -1 if the connection was closed.
     * 
     * Throws IOException
     */
    protected int initChnlReadFromChannel(ByteBuffer dst, Error error) throws IOException
    {
        int bytesRead = read(dst);

        if (bytesRead > 0)
        {
            // note that we could cache the msgLen, but normally we should be reading an entire ConnectAck/ConnectNak here.
            if (dst.position() > (HTTP_HEADER4 + 2))
            {
                int messageLength = (dst.getShort(HTTP_HEADER4) & 0xFF);
                if (dst.position() >= (HTTP_HEADER4 + messageLength + CHUNKEND_SIZE))
                {
                    // we have at least one complete message
                    return dst.position();
                }
            }
        }
        else if (bytesRead == -1)
        {
            // The connection was closed by far end (server). Need to try another protocol.
            _initChnlState = InitChnlState.RECONNECTING;
            return -1;
        }

        // we don't have a complete message, or no bytes were read.
        return 0;
    }

    protected int initChnlReadFromChannelProvider(ByteBuffer dst, Error error) throws IOException
    {
        int bytesRead = read(dst);

        if (bytesRead <= 1 && _providerHelper != null && _providerHelper.wininetStreamingComplete())
        {
            if (RsslHttpSocketChannelProvider.debugPrint)
                System.out.println(" Got pipe notify  " + bytesRead);
            _providerHelper._pipeNode._pipe.sink().close();
            _providerHelper._pipeNode._pipe.source().close();
            _providerHelper._pipeNode.returnToPool();
            // fake active for later close() to distinguish if coming from this
            // notify or timeout & INITIALING state
            _state = ChannelState.ACTIVE;
            return -1;
        }

        if (bytesRead > 0)
        {
            if (dst.position() > (HTTP_HEADER4 + 2))
            {
                if (checkIsProviderHTTP(dst))
                {
                    _isProviderHTTP = true;
                }

                if (_isProviderHTTP != true)
                {
                    int messageLength = (dst.getShort(HTTP_HEADER4) & 0xFF);
                    if (dst.position() >= (HTTP_HEADER4 + messageLength + CHUNKEND_SIZE))
                    {
                        // we have at least one complete message
                        return dst.position();
                    }
                }
                else
                {
                    return dst.position();
                }
            }
            else
            {
                if (RsslHttpSocketChannelProvider.debugPrint)
                    System.out.println("bufferPos = " + dst.position());
            }
        }
        else if (bytesRead == -1)
        {
            _initChnlState = InitChnlState.RECONNECTING;
            return -1;
        }

        return 0;
    }

    protected boolean checkIsProviderHTTP(ByteBuffer dst)
    {
        int endPos = dst.position();
        if (dst.position() > 4)
        {
            String startHTTP_POST_REQUEST_FROM_CLIENT = "";

            byte[] tempBuf = new byte[4];

            dst.position(0);
            dst.get(tempBuf, 0, 4);

            dst.position(endPos);

            startHTTP_POST_REQUEST_FROM_CLIENT = new String(tempBuf);

            if (startHTTP_POST_REQUEST_FROM_CLIENT.startsWith("POST")) // http
                return true;
            else
                return false;
        }

        return false;
    }

    /* This method is overridden by the JUnit tests to simulate writing to a
     * real java.nio.channels.SocketChannel
     * 
     * srcs is the buffers from which bytes are to be retrieved
     * offset is the offset within the buffer array of the first buffer from
     *               which bytes are to be retrieved; must be non-negative and no larger than srcs.length
     * length is the maximum number of buffers to be accessed;
     *               must be non-negative and no larger than srcs.length - offset
     * 
     * Returns the number of bytes written, possibly zero
     * 
     * Throws IOException if some other I/O error occurs
     */
    protected long write(ByteBuffer[] srcs, int offset, int length) throws IOException
    {
        if (_providerHelper != null)
            return _providerHelper.write(srcs, offset, length);

        return _scktChannel.write(srcs, offset, length);
    }

    @Override
    public int info(ChannelInfo info, Error error)
    {
        assert (info != null) : "info cannot be null";
        assert (error != null) : "error cannot be null";

        int ret = TransportReturnCodes.SUCCESS;
        try
        {
            _writeLock.lock();
            if (_state == ChannelState.ACTIVE)
            {
                ((ChannelInfoImpl)info).maxFragmentSize(_channelInfo._maxFragmentSize);
                ((ChannelInfoImpl)info).maxOutputBuffers(_channelInfo._maxOutputBuffers);
                ((ChannelInfoImpl)info).guaranteedOutputBuffers(_channelInfo._guaranteedOutputBuffers);
                ((ChannelInfoImpl)info).numInputBuffers(_channelInfo._numInputBuffers);
                ((ChannelInfoImpl)info).pingTimeout(_channelInfo._pingTimeout);
                ((ChannelInfoImpl)info).clientToServerPings(_channelInfo._clientToServerPings);
                ((ChannelInfoImpl)info).serverToClientPings(_channelInfo._serverToClientPings);
                ((ChannelInfoImpl)info).sysSendBufSize(_scktChannel.socket().getSendBufferSize());
                ((ChannelInfoImpl)info).sysRecvBufSize(_scktChannel.socket().getReceiveBufferSize());
                ((ChannelInfoImpl)info).compressionType(_channelInfo._compressionType);
                ((ChannelInfoImpl)info).compressionThreshold(_channelInfo._compressionThreshold);
                ((ChannelInfoImpl)info).priorityFlushStrategy(_channelInfo._priorityFlushStrategy);
                ((ChannelInfoImpl)info)._receivedComponentInfoList = _channelInfo._receivedComponentInfoList;
                ((ChannelInfoImpl)info).clientIP(_channelInfo._clientIP);
                ((ChannelInfoImpl)info).clientHostname(_channelInfo.clientHostname());
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
            _state = ChannelState.CLOSED;
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Socket exception ");
            ret = TransportReturnCodes.FAILURE;
        }
        finally
        {
            _writeLock.unlock();
        }
        return ret;
    }

    /* Verify that the priority flush order is valid.
     * The value must be non null and an instance of String.
     * It must be less than or equal to 32 characters.
     * It must contain at least one 'H' and one 'M'.
     */
    private String validatePriorityFlushOrder(Object value)
    {
        if (value != null && (value.getClass() == String.class))
        {
            String string = (String)value;
            int length = string.length();
            if (length <= 32)
            {
                // ensure at least one 'H' and one 'M' is specified.
                // without incurring GC.
                boolean hasH = false, hasM = false;
                for (int i = 0; i < length; i++)
                {
                    if (string.charAt(i) == 'H')
                        hasH = true;
                    else if (string.charAt(i) == 'M')
                        hasM = true;
                    if (hasH && hasM)
                        return null; // validated!
                }
                return "value must contain at least on 'H' and one 'M'.";
            }
            else
            {
                return "value cannot exceed 32 characters.";
            }
        }
        else
        {
            return "value must be a String.";
        }
    }

    @Override
    public int ioctl(int code, Object value, Error error)
    {
        assert (value != null) : "value cannot be null";
        assert (error != null) : "error cannot be null";

        int retCode = TransportReturnCodes.FAILURE;
        try
        {
            _writeLock.lock();

            // return FAILURE if channel not active or not initializing
            if ((_state != ChannelState.ACTIVE) && (_state != ChannelState.INITIALIZING))
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in the active or initializing state");
                _needCloseSocket = true;
                if (_providerHelper != null)
                    _providerHelper.closeStreamingSocket();

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
                        // this is hidden from user, don't mention ComponentInfo.
                        // act like the default case.
                        error.channel(this);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text("Code is not valid.");
                    }
                    break;
                }
                case IoctlCodes.PRIORITY_FLUSH_ORDER:
                    String errorString;
                    if ((errorString = validatePriorityFlushOrder(value)) == null)
                    {
                        flushOrder((String)value);
                        retCode = TransportReturnCodes.SUCCESS;
                    }
                    else
                    {
                        error.channel(this);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text(errorString);
                    }
                    break;
                default:
                    error.channel(this);
                    error.errorId(retCode);
                    error.sysError(0);
                    error.text("Code is not valid.");
            }
        }
        finally
        {
            _writeLock.unlock();
        }

        return retCode;
    }

    @Override
    public int ioctl(int code, int value, Error error)
    {
        assert (error != null) : "error cannot be null";

        int retCode = TransportReturnCodes.FAILURE;
        try
        {
            _writeLock.lock();

            // return FAILURE if channel not active or not initializing
            if ((_state != ChannelState.ACTIVE) && (_state != ChannelState.INITIALIZING))
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in the active or initializing state");
                _needCloseSocket = true;
                if (_providerHelper != null)
                    _providerHelper.closeStreamingSocket();

                return TransportReturnCodes.FAILURE;
            }

            switch (code)
            {
            // IoctlCodes.COMPONENT_INFO handled in ioctl(int, Object, Error)
            // IoctlCodes.PRIORITY_FLUSH_ORDER handled in ioctl(int, Object, Error)
                case IoctlCodes.MAX_NUM_BUFFERS:
                    /* must be at least as large as guaranteedOutputBuffers. */
                    if (value >= 0)
                    {
                        int diff = value - _channelInfo._guaranteedOutputBuffers;
                        if (diff < 0)
                            value = _channelInfo._guaranteedOutputBuffers;
                        retCode = _channelInfo._maxOutputBuffers = value;
                    }
                    else
                    {
                        error.channel(this);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text("value must be (0 >= value < 2^31");
                    }
                    break;
                case IoctlCodes.NUM_GUARANTEED_BUFFERS:
                    /* NUM_GUARANTEED_BUFFERS:
                     * the per channel number of guaranteedOutputBuffers that ETAJ will create for the client. */
                    if (value >= 0)
                    {
                        retCode = adjustGuaranteedOutputBuffers(value);
                    }
                    else
                    {
                        error.channel(this);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text("value must be (0 >= value < 2^31");
                    }
                    break;
                case IoctlCodes.HIGH_WATER_MARK:
                    if (value >= 0)
                    {
                        _highWaterMark = value;
                        retCode = TransportReturnCodes.SUCCESS;
                    }
                    else
                    {
                        error.channel(this);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text("value must be (0 >= value < 2^31");
                    }
                    break;
                case IoctlCodes.SYSTEM_WRITE_BUFFERS:
                    if (value >= 0)
                    {
                        _scktChannel.socket().setSendBufferSize(value);
                        retCode = TransportReturnCodes.SUCCESS;
                    }
                    else
                    {
                        error.channel(this);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text("value must be (0 >= value < 2^31");
                    }
                    break;
                case IoctlCodes.SYSTEM_READ_BUFFERS:
                    if (value >= 0)
                    {
                        _scktChannel.socket().setReceiveBufferSize(value);
                        retCode = TransportReturnCodes.SUCCESS;
                    }
                    else
                    {
                        error.channel(this);
                        error.errorId(retCode);
                        error.sysError(0);
                        error.text("value must be (0 >= value < 2^31");
                    }
                    break;
                case IoctlCodes.COMPRESSION_THRESHOLD:
                	if (_channelInfo._compressionType == Ripc.CompressionTypes.NONE)
                		retCode = TransportReturnCodes.SUCCESS;
                	else
                	{
                		int compressionThreshold = (_channelInfo._compressionType == Ripc.CompressionTypes.ZLIB ? ZLIB_COMPRESSION_THRESHOLD : LZ4_COMPRESSION_THRESHOLD );

                		if (value >= compressionThreshold)
                		{
                			_channelInfo._compressionThreshold = _sessionCompLowThreshold = value;
                			retCode = TransportReturnCodes.SUCCESS;
                		}
                		else
                		{
                			error.channel(this);
                			error.errorId(retCode);
                			error.sysError(0);
                			error.text("value must be equal to or greater than " + compressionThreshold);
                		}
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
        finally
        {
            _writeLock.unlock();
        }

        return retCode;
    }

    /* Grow the guaranteedOutputBuffer (a.k.a. _availableBuffers) by the numToGrow specified.
     * 
     * numToGrow is the amount to grow. First attempt to get the buffers
     *                  from the global (SocketProtocol) pool, otherwise create them.
     * 
     * Returns the number grown.
     */
    int growGuaranteedOutputBuffers(int numToGrow)
    {
        try
        {
            Transport._globalLock.lock();

            Pool bufferPool = null;

            int numToCreate = 0;
            if (!_isProviderHTTP)
            {
                bufferPool = _transport.getPool(_internalMaxFragmentSize);
                numToCreate = numToGrow - bufferPool.poll(_availableBuffers, numToGrow);
            }
            else
            {
                bufferPool = _transport.getPool(_internalMaxFragmentSize + HTTP_PROVIDER_EXTRA);
                int numThere = bufferPool.poll(_availableHTTPBuffers, numToGrow);
                numToCreate = numToGrow - numThere;
            }

            if (numToCreate > 0)
            {
                // need to create more buffers and add to the _availableBuffers pool
                SocketBuffer buffer;
                for (int i = 0; i < numToCreate; i++)
                {
                    if (!_isProviderHTTP)
                    {
                        buffer = new SocketBuffer(_availableBuffers, _internalMaxFragmentSize);
                    }
                    else
                    {
                        buffer = new HTTPSocketBuffer(_availableHTTPBuffers, _internalMaxFragmentSize);
                    }

                    buffer.returnToPool();
                }
            }
            return numToGrow;
        }
        finally
        {
            Transport._globalLock.unlock();
        }
    }

    /* Shrink the guaranteedOutputBuffers by numToShrink. 
     * Do this by adding numToShrink guaranteedOutputBuffers to global pool (SocketProtocol).
     * 
     * Returns the number actually shrunk, which may not be the numToShrink.
     */
    int shrinkGuaranteedOutputBuffers(int numToShrink)
    {
        try
        {
            Transport._globalLock.lock();

            Pool bufferPool = null;

            if (!_isProviderHTTP)
            {
                bufferPool = _transport.getPool(_internalMaxFragmentSize);
                return bufferPool.add(_availableBuffers, numToShrink);
            }
            else
            {
                bufferPool = _transport.getPool(_internalMaxFragmentSize + HTTP_PROVIDER_EXTRA);
                return bufferPool.add(_availableHTTPBuffers, numToShrink);
            }
        }
        finally
        {
            Transport._globalLock.unlock();
        }
    }

    /* 1) If the value is larger than the current value, grow the number of output buffers.
     * 2) If the new value is larger than maxOutputBuffers, set maxOutputBuffers to the new value.
     * 3) If the new value is smaller than the current value, attempt to shrink the buffers
     * by returning them to the global pool.
     * If the number of available buffers (not in use) is smaller than what needs to be returned
     * (because the buffers are being used), just shrink by that number.
     * 
     * Returns the new value of guaranteedOutputBuffers.
     */
    int adjustGuaranteedOutputBuffers(int value)
    {
        int diff = value - _channelInfo._guaranteedOutputBuffers;
        if (diff > 0)
        {
            /* If guaranteedOutputBuffers grow larger than maxOutputBuffers, adjust maxOutputBuffers. */
            _channelInfo._guaranteedOutputBuffers += growGuaranteedOutputBuffers(diff);
            if (_channelInfo._maxOutputBuffers < _channelInfo._guaranteedOutputBuffers)
                _channelInfo._maxOutputBuffers = _channelInfo._guaranteedOutputBuffers;

            return _channelInfo._guaranteedOutputBuffers;
        }
        else if (diff < 0)
        {
            // shrink buffers
            return _channelInfo._guaranteedOutputBuffers -= shrinkGuaranteedOutputBuffers(diff * -1);
        }
        else
        {
            // nothing changed.
            return _channelInfo._guaranteedOutputBuffers;
        }
    }

    @Override
    public int bufferUsage(Error error)
    {
        assert (error != null) : "error cannot be null";

        int ret;
        try
        {
            _writeLock.lock();
            if ((_state == ChannelState.ACTIVE) || (_state == ChannelState.CLOSED))
            {
                ret = _used;
            }
            else
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("channel not in active or closed state ");
                ret = TransportReturnCodes.FAILURE;
            }
        }
        finally
        {
            _writeLock.unlock();
        }
        return ret;
    }

    @Override
    public int close(Error error)
    {
        assert (error != null) : "error cannot be null";

        if (_providerHelper != null)
            return _providerHelper.close(error);

        int ret = TransportReturnCodes.SUCCESS;

        // first set channel state to closed
        try
        {
            lockReadWriteLocks();

            if (_state != ChannelState.INACTIVE)
            {
                _state = ChannelState.INACTIVE;
                if (_compressor != null)
                {
                    _compressor.close();
                }
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
            unlockReadWriteLocks();
            if (ret == TransportReturnCodes.FAILURE)
            {
                return ret;
            }
        }

        try
        {
            Transport._globalLock.lock();

            // When closing channel the buffers should not be used.
            // The check here is to make sure all resources are cleaned up release buffers.
            TransportBuffer buffer = null;
            while ((buffer = (TransportBuffer)_highPriorityQueue.poll()) != null)
            {
                releaseBufferInternal(buffer);
            }
            while ((buffer = (TransportBuffer)_mediumPriorityQueue.poll()) != null)
            {
                releaseBufferInternal(buffer);
            }
            while ((buffer = (TransportBuffer)_lowPriorityQueue.poll()) != null)
            {
                releaseBufferInternal(buffer);
            }
            for (int i = 0; i < _releaseBufferArray.length; i++)
            {
                if (_releaseBufferArray[i] != null)
                {
                    releaseBufferInternal(_releaseBufferArray[i]);
                }
                _releaseBufferArray[i] = null;
                _gatheringWriteArray[i] = null;
            }
            if (_currentBuffer != null)
            {
                _currentBuffer.returnToPool();
            }
            _writeArrayMaxPosition = 0;
            _writeArrayPosition = 0;
            _isFlushPending = false;
            _totalBytesQueued = 0;
            _currentBuffer = null;

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

            // move all buffers from _availableBuffers pool to the global pool
            shrinkGuaranteedOutputBuffers(_availableBuffers.size());

            if (_server != null)
            {
                _server.removeChannel(this);
            }

            // reset this channel to default values, prior to adding back in the pool.
            resetToDefault();

            if (_readIoBuffer != null)
            {
                _readIoBuffer.returnToPool();
                _readIoBuffer = null;
            }

            // return channel to pool
            returnToPool();
        }
        finally
        {
            Transport._globalLock.unlock();
        }

        return ret;
    }

    @Override
    public TransportBuffer read(ReadArgs readArgs, Error error)
    {
        assert (readArgs != null) : "readArgs cannot be null";
        assert (error != null) : "error cannot be null";

        TransportBuffer data = null; // the data returned to the user
        int returnValue;

        try
        {
            if (_readLock.trylock())
            {
                // return FAILURE if channel not active
                if (_state != ChannelState.ACTIVE)
                {
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("socket channel is not in the active state for read");
                    ((ReadArgsImpl)readArgs).readRetVal(TransportReturnCodes.FAILURE);
                    _needCloseSocket = true;
                    if (_providerHelper != null)
                        _providerHelper.closeStreamingSocket();

                    return null;
                }

                // initialize bytesRead and uncompressedBytesRead
                ((ReadArgsImpl)readArgs)._bytesRead = 0;
                ((ReadArgsImpl)readArgs)._uncompressedBytesRead = 0;

                updateState((ReadArgsImpl)readArgs);

                // if we don't already have data to give the user, read from the network
                if (_readBufStateMachine.state() != ReadBufferState.KNOWN_COMPLETE)
                {
                    performReadIO((ReadArgsImpl)readArgs);
                }

                // determine the return value, and optionally populate the buffer
                switch (_readBufStateMachine.state())
                {
                    case KNOWN_COMPLETE:
                        int entireMessageLength = _readBufStateMachine.currentRipcMessageLength();

                        if (entireMessageLength != Ripc.Lengths.HEADER)
                        {
                            returnValue = updateAppReadBuffer(entireMessageLength, (ReadArgsImpl)readArgs);
                            if (_readBufStateMachine.dataLength() != 0 && returnValue >= TransportReturnCodes.SUCCESS)
                            {
                                data = _appReadBuffer;
                            }
                            else if (returnValue == TransportReturnCodes.SUCCESS)
                            {
                                // return READ_WOULD_BLOCK if no more to read and not returning a buffer
                                returnValue = TransportReturnCodes.READ_WOULD_BLOCK;
                            }
                        }
                        else
                        {
                            ((ReadArgsImpl)readArgs)._uncompressedBytesRead = ((ReadArgsImpl)readArgs)._bytesRead;
                            returnValue = TransportReturnCodes.READ_PING;
                        }
                        break;
                    case NO_DATA:
                        returnValue = TransportReturnCodes.READ_WOULD_BLOCK;
                        break;
                    case END_OF_STREAM:
                        if (_providerHelper != null && _providerHelper._wininetControl)
                        {
                            if (RsslHttpSocketChannelProvider.debugPrint)
                                System.out.println(" Wininet consumer closed the old control socket...");

                            _readBufStateMachine._state = ReadBufferState.NO_DATA;
                            returnValue = _providerHelper.switchWininetSession(error);
                            if (returnValue == TransportReturnCodes.FAILURE)
                            {
                                if (RsslHttpSocketChannelProvider.debugPrint)
                                    System.out.println(" Wininet control channel error....");
                                error.channel(this);
                                error.errorId(TransportReturnCodes.FAILURE);
                                error.sysError(0);
                                error.text("Control channel error...");

                                _providerHelper.closeStreamingSocket();
                            }
                        }
                        else if (_providerHelper != null && _providerHelper._javaSession)
                        {
                            if (RsslHttpSocketChannelProvider.debugPrint)
                                System.out.println(" socket closed from the other end for Java session.");

                            _readBufStateMachine._state = ReadBufferState.NO_DATA;

                            returnValue = _providerHelper.closeJavaOldSocket();
                            if (returnValue == TransportReturnCodes.FAILURE)
                            {
                                if (RsslHttpSocketChannelProvider.debugPrint)
                                    System.out.println("Java channel error...");
                                _needCloseSocket = true;
                                error.channel(this);
                                error.errorId(TransportReturnCodes.FAILURE);
                                error.sysError(0);
                                error.text("Java channel error.....");
                                _providerHelper.closeStreamingSocket();
                            }
                        }
                        else
                        {
                            _state = ChannelState.CLOSED;
                            if (_httpProxy)
                            {
                                _proxyAuthenticator = null;
                            }

                            if (_providerHelper != null)
                            {
                                if (RsslHttpSocketChannelProvider.debugPrint)
                                    System.out.println(" got (-1) end-of-stream on socket " + _scktChannel.toString());
                            }
                            if (_providerHelper != null && _providerHelper._wininetControl)
                            {
                                if (RsslHttpSocketChannelProvider.debugPrint)
                                    System.out.println(" Winnet control channel error......");
                                _providerHelper.closeStreamingSocket();
                            }
                            error.channel(this);
                            error.errorId(TransportReturnCodes.FAILURE);
                            error.sysError(0);
                            error.text("Channel already closed...");
                            returnValue = TransportReturnCodes.FAILURE;

                            populateErrorDetails(error, TransportReturnCodes.FAILURE, "SocketChannel.read returned -1 (end-of-stream)");
                        }
                        break;
                    default:
                        returnValue = (_readIoBuffer.buffer().position() - _readBufStateMachine.currentRipcMessagePosition());
                        assert (returnValue > TransportReturnCodes.SUCCESS);
                        break;
                }
            }
            else
            {
                // failed to obtain the lock
                returnValue = TransportReturnCodes.READ_IN_PROGRESS;
            }
        }
        catch (CompressorException e)
        {
            _state = ChannelState.CLOSED;
            if (_httpProxy)
            {
                _proxyAuthenticator = null;
            }
            returnValue = TransportReturnCodes.FAILURE;
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            populateErrorDetails(error, TransportReturnCodes.FAILURE, "CompressorException: " + e.getLocalizedMessage());
        }
        catch (Exception e)
        {
            if (_providerHelper != null && _providerHelper._wininetControl)
            {
                // should be on original RsslChannel
                if (RsslHttpSocketChannelProvider.debugPrint)
                    System.out.println(" Wininet consumer closed the old control socket.");

                // check to see if streaming channel is still there in case of hard killing
                if (_providerHelper._needCloseOldChannel)
                {
                    returnValue = _providerHelper.switchWininetSession(error);
                }
                else
                {
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Channel closed already...");
                    _providerHelper.closeStreamingSocket();
                    returnValue = TransportReturnCodes.FAILURE;
                }
            }
            else if (_providerHelper != null)
            {
                returnValue = TransportReturnCodes.FAILURE;
                if (returnValue == TransportReturnCodes.FAILURE)
                {
                    if (RsslHttpSocketChannelProvider.debugPrint)
                        System.out.println("Java channel error.......");
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Java channel error.......");
                    _needCloseSocket = true;
                    _providerHelper.closeStreamingSocket();
                }
            }
            else
            {
                _needCloseSocket = true;
                _state = ChannelState.CLOSED;
                if (_httpProxy)
                {
                    _proxyAuthenticator = null;
                }
                returnValue = TransportReturnCodes.FAILURE;
                populateErrorDetails(error, TransportReturnCodes.FAILURE, e.getLocalizedMessage());
            }
        }
        finally
        {
            _readLock.unlock();
        }

        ((ReadArgsImpl)readArgs).readRetVal(returnValue);
        return data;
    }

    /* Updates the state machine when RsslSocketChannel::read(ReadArgs, Error) is invoked */
    protected void updateState(ReadArgsImpl readArgs)
    {
        // if the previous state was KNOWN_COMPLETE, advance the state machine,
        // and rewind the read IO buffer if there is no more data in it.
        if (_readBufStateMachine.state() == ReadBufferState.KNOWN_COMPLETE)
        {
            if (_readBufStateMachine.advanceOnApplicationRead(readArgs) == ReadBufferState.NO_DATA)
            {
                _readIoBuffer.buffer().rewind(); // no more data in the read IO buffer
            }
        }
    }

    /* Populates the specified error message with the provided error id and message
     * 
     * error is the error to populate
     * errorId is the error code
     * message is the text to display to end users
     */
    protected void populateErrorDetails(Error error, int errorId, String message)
    {
        error.channel(this);
        error.errorId(errorId);
        error.sysError(0);
        error.text(message);
    }

    /* Performs Read IO
     * 
     * Throws IOException if an IO exception occurs
     */
    @SuppressWarnings("fallthrough")
	private void performReadIO(ReadArgsImpl readArgs) throws IOException
    {
        switch (_readBufStateMachine.state())
        {
            case KNOWN_INSUFFICENT: // fall through
            case UNKNOWN_INSUFFICIENT:
                _readIoBuffer.buffer().limit(_readIoBuffer.buffer().position()); // because compact() copies everything up to the limit
                _readIoBuffer.buffer().position(_readBufStateMachine.currentRipcMessagePosition());
                _readIoBuffer.buffer().compact();
                _readBufStateMachine.advanceOnCompact();
                // fall through
            case END_OF_STREAM: // fall through
            case NO_DATA:       // fall through
            case KNOWN_INCOMPLETE:   // fall through
            case UNKNOWN_INCOMPLETE: // fall through
                // read from the channel: note *replace* the call to read()
                // below with a call to readAndPrintForReplay() to collect network replay data (for debugging) only!

                int bytesRead = read(_readIoBuffer.buffer());

//                int bytesRead = readAndPrintForReplay(); // for NetworkReplay replace the above line with this one

                _readBufStateMachine.advanceOnSocketChannelRead(bytesRead, readArgs);
                break;
            default:
                assert (false); // code should not reach here
                break;
        }
    }

    /* WARNING: Creates Garbage For debugging only, this method reads data from the network and prints the data as hex
     * (so it can later be played back using NetworkReplay).
     * 
     * Returns the number of bytes read from the network
     */
    @SuppressWarnings("unused")
    private int readAndPrintForReplay() throws IOException
    {
        // save the current position
        int posBeforeRead = _readIoBuffer.buffer().position();

        // read from the network
        int bytesRead = read(_readIoBuffer.buffer());
        _totalBytesRead += bytesRead;

        if (_debugOutput == null)
        {
            _debugOutput = new StringBuilder();
        }

        _debugOutput.setLength(0);
        _debugOutput.append("-- begin read (");
        _debugOutput.append(bytesRead);
        _debugOutput.append(" of ");
        _debugOutput.append(_totalBytesRead);
        _debugOutput.append(" total bytes) cur RIPC pos: ");
        _debugOutput.append(_readBufStateMachine.currentRipcMessagePosition());
        _debugOutput.append(" prev pos: ");
        _debugOutput.append(posBeforeRead);
        _debugOutput.append(" new pos: ");
        _debugOutput.append(_readIoBuffer.buffer().position());
        _debugOutput.append(" limit: ");
        _debugOutput.append(_readIoBuffer.buffer().limit());
        _debugOutput.append(" thread id: ");
        _debugOutput.append(Thread.currentThread().getId());
        _debugOutput.append(" --");
        System.out.println(_debugOutput.toString());

        System.out.println(Transport.toHexString(_readIoBuffer.buffer(), posBeforeRead, (_readIoBuffer.buffer().position() - posBeforeRead)));
        System.out.println("-- end read --");

        return bytesRead;
    }

    /* Updates the TransportBuffer that returns the message to the user.
     * If there are no more bytes in the read buffer, TransportReturnCodes.SUCCESS is returned.
     * If there are bytes remaining in the read buffer, a value greater than
     * TransportReturnCodes.SUCCESS is returned.
     * 
     * entireMessageLength is the length of the entire message (including the RIPC header).
     * 
     * If there are no more bytes in the read buffer, TransportReturnCodes.SUCCESS is returned.
     * If there are bytes remaining in the read buffer, a value greater than
     * TransportReturnCodes.SUCCESS is returned.
     */
    protected int updateAppReadBuffer(int entireMessageLength, ReadArgsImpl readArgs)
    {
        assert (entireMessageLength > 0);

        int returnValue = TransportReturnCodes.SUCCESS;

        ByteBufferPair pair = _readBufStateMachine.dataBuffer();

        // set the limit and position on the underlying read-only ByteBuffer
        pair.readOnly().limit(_readBufStateMachine.dataPosition() + _readBufStateMachine.dataLength());
        pair.readOnly().position(_readBufStateMachine.dataPosition());

        _appReadBuffer.data(pair.readOnly());
        returnValue = (_readIoBuffer.buffer().position() - (_readBufStateMachine.currentRipcMessagePosition() + entireMessageLength));

        // we cannot exactly-SUCCESS if we are in the middle of processing a packed message
        if (_readBufStateMachine.hasRemainingPackedData())
        {
            ++returnValue; // we have not processed the last part of a packed message
        }

        // handle compressed message
        if (_readBufStateMachine.subState() == ReadBufferSubState.PROCESSING_COMPRESSED_MESSAGE)
        {
            int uncompressedLength = _compressor.decompress(_appReadBuffer, _decompressBuffer, _appReadBuffer.length());
            readArgs._uncompressedBytesRead = uncompressedLength + _readBufStateMachine._httpOverhead + Ripc.Lengths.HEADER;

            _appReadBuffer.data(_decompressBuffer.data());
        }

        assert (returnValue >= TransportReturnCodes.SUCCESS);

        return returnValue;
    }

    protected SocketBuffer getSocketBuffer()
    {
        SocketBuffer buffer;
        if (!_isProviderHTTP)
            buffer = (SocketBuffer)_availableBuffers.poll();
        else
            buffer = (HTTPSocketBuffer)_availableHTTPBuffers.poll();

        if (buffer == null)
        {
            if (_server != null && _used < _channelInfo._maxOutputBuffers)
            {
                buffer = _server.getBufferFromServerPool();
            }
        }

        if (buffer != null)
        {
            buffer.clear();
            ++_used;
        }
        return buffer;
    }

    protected TransportBufferImpl getBigBuffer(int size)
    {
        BigBuffer.ripcVersion(_ipcProtocol.ripcVersion());
        BigBuffer buffer = null;
        SocketBuffer sBuffer = null;
        // get a transport buffer that will be first buffer where the big buffer will be copied to
        sBuffer = getSocketBuffer();
        if (sBuffer != null)
        {
            sBuffer._dataBuffer.position(0);
            TransportBufferImpl tBuffer = sBuffer.getBufferSliceForFragment(_internalMaxFragmentSize);

            // get big buffer from the pool, it returns non null
            buffer = (BigBuffer)_bigBuffersPool.poll(size, true);
            buffer._data.position(0);
            buffer._length = size;
            buffer.id();
            buffer._firstBuffer = tBuffer;
            buffer._isOwnedByApp = true;
        }
        return buffer;
    }

    @Override
    public TransportBuffer getBuffer(int size, boolean packedBuffer, Error error)
    {
        if (_providerHelper != null)
        {
            return _providerHelper.getBuffer(size, packedBuffer, error);
        }

        assert (error != null) : "error cannot be null";

        int sizeWithHeaders = size + RIPC_HDR_SIZE;
        if (packedBuffer)
            sizeWithHeaders += RIPC_PACKED_HDR_SIZE;

        TransportBufferImpl buffer = null;
        try
        {
            _writeLock.lock();

            // return FAILURE if channel not active
            if (_state != ChannelState.ACTIVE)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in the active state for getBuffer");
                _needCloseSocket = true;
                return null;
            }

            /* check if we need big buffer instead of normal buffer */
            if (sizeWithHeaders > _internalMaxFragmentSize)
            {
                if (!packedBuffer)
                {
                    buffer = getBigBuffer(size);
                    if (buffer == null)
                    {
                        error.channel(this);
                        error.errorId(TransportReturnCodes.NO_BUFFERS);
                        error.sysError(0);
                        error.text("channel out of buffers");
                    }
                    return buffer;
                }
                else
                {
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("packing buffer must fit in maxFragmentSize");
                    return null;
                }
            }

            // not a big buffer
            buffer = getBufferInternal(size, packedBuffer);
            if (buffer == null)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.NO_BUFFERS);
                error.sysError(0);
                error.text("channel out of buffers");
            }
        }
        finally
        {
            _writeLock.unlock();
        }

        return buffer;
    }

    protected TransportBufferImpl getBufferInternal(int size, boolean packedBuffer)
    {
        // This method should be called when the buffer size + header is less then fragment size.
        // The calling method chain should have lock set.
        TransportBufferImpl buffer = null;

        if (_currentBuffer != null)
        {
            buffer = _currentBuffer.getBufferSlice(size, packedBuffer);
            if (buffer == null)
            {
                SocketBuffer temp = _currentBuffer;
                if (temp._slicesPool.areAllSlicesBack())
                {
                    _currentBuffer = null;
                    socketBufferToRecycle(temp);
                }
            }
        }

        if (buffer == null)
        {
            _currentBuffer = (SocketBuffer)_availableBuffers.poll();
            if (_currentBuffer == null)
            {
                if (_server != null && _used < _channelInfo._maxOutputBuffers && !_isProviderHTTP)
                {
                    _currentBuffer = _server.getBufferFromServerPool();
                }
            }

            if (_currentBuffer != null)
            {
                _currentBuffer.clear();
                ++_used;
                buffer = _currentBuffer.getBufferSlice(size, packedBuffer);
            }
        }

        if (buffer != null)
        {
            buffer._isOwnedByApp = true;
            assert (buffer.next() == null);
        }

        return buffer;
    }

    @Override
    public int releaseBuffer(TransportBuffer bufferInt, Error error)
    {
        assert (bufferInt != null) : "buffer cannot be null";
        assert (error != null) : "error cannot be null";

        TransportBufferImpl buffer = (TransportBufferImpl)bufferInt;

        if (buffer._isOwnedByApp == false)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Application does not own this buffer.");
            return TransportReturnCodes.FAILURE;
        }

        try
        {
            _writeLock.lock();

            // return FAILURE if channel inactive
            if (_state == ChannelState.INACTIVE)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is in inactive state");
                return TransportReturnCodes.FAILURE;
            }
            releaseBufferInternal(bufferInt);
        }
        finally
        {
            _writeLock.unlock();
        }
        return TransportReturnCodes.SUCCESS;
    }

    void releaseBufferInternal(TransportBuffer bufferInt)
    {
        // This method is used by write call, which is already guarded by lock.
        TransportBufferImpl buffer = (TransportBufferImpl)bufferInt;
        buffer._isOwnedByApp = false;
        if (!_isJunitTest)
        {
            if (!buffer.isBigBuffer() && ((SocketBuffer)buffer._pool._poolOwner)._pool._isSharedPoolBuffer)
            {
                if (bufferInt != _currentBuffer)
                {
                    buffer.returnToPool();
                    if (_used > 0)
                    {
                        --_used;
                    }
                }
            }
            else
            {
                buffer.returnToPool();
            }
        }
        else
        {
            buffer.returnToPool();
        }
    }

    void clearQueues()
    {
        _highPriorityQueue.clear();
        _mediumPriorityQueue.clear();
        _lowPriorityQueue.clear();
    }

    @Override
    public int packBuffer(TransportBuffer bufferInt, Error error)
    {
        assert (bufferInt != null) : "buffer cannot be null";
        assert (error != null) : "error cannot be null";

        TransportBufferImpl buffer = (TransportBufferImpl)bufferInt;

        try
        {
            _writeLock.lock();

            if (buffer._isOwnedByApp == false)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("Application does not own this buffer.");
                return TransportReturnCodes.FAILURE;
            }

            // return FAILURE if channel not active
            if (_state != ChannelState.ACTIVE)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in active state for pack");
                return TransportReturnCodes.FAILURE;
            }

            return buffer.pack(true, this, error);
        }
        finally
        {
            _writeLock.unlock();
        }
    }

    /* Builds the fragment by taking the next chunk of data from the big buffer,
     * compressing it, and populating the fragment.
     * 
     * bigBuffer is the source of data to be fragmented
     * 
     * fragment is the fragment to be built with data compressed from bigBuffer
     * 
     * writeArgs is the firstFragment if this is the first fragment being built for
     *            the bigBuffer message
     * 
     * Returns the number of bytes from the bigBuffer which were encoded in this fragment, 
     * or TransportReturnCodes if there is a failure.
     */
    protected int writeFragmentCompressed(BigBuffer bigBuffer, TransportBufferImpl fragment, WriteArgs writeArgs, boolean firstFragment, Error error)
    {
        int userBytesForFragment;
        int position = bigBuffer._data.position();
        int limit = bigBuffer._data.limit();
        int flags = Ripc.Flags.HAS_OPTIONAL_FLAGS | Ripc.Flags.COMPRESSION;
        int optFlags = 0;
        int headerLength = 0;
        int compressedLen = 0;
        int maxPayloadSize = 0;
        byte[] compressedBytes;
        int extraBytes = 0;
        int extraTotalLength = 0;
        int extraHeaderLength = 0;
        int totalLength = 0;
        int retVal = TransportReturnCodes.SUCCESS;
        final int MAX_BYTES_FOR_BUFFER = _internalMaxFragmentSize - RIPC_HDR_SIZE;

        // An extra buffer might be needed: get it now before compression to ensure it is available if needed
        TransportBufferImpl compFragmentBuffer = getBufferInternal(MAX_BYTES_FOR_BUFFER, false);
        if (compFragmentBuffer == null)
        {
            retVal = flushInternal(error);
            if (retVal < TransportReturnCodes.SUCCESS)
                return retVal;
            compFragmentBuffer = getBufferInternal(MAX_BYTES_FOR_BUFFER, false);
            if (compFragmentBuffer == null)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.NO_BUFFERS);
                error.sysError(0);
                error.text("channel out of buffers");
                return TransportReturnCodes.NO_BUFFERS;
            }
        }

        // Determine how many bytes can fit in the fragment,
        // depending on if this is the first/next fragment, and how many bytes remain in the big buffer.
        if (firstFragment)
        {
            // First time: position is the end of data in the big buffer
            // -- make this the new limit
            limit = position;
            totalLength = position;
            optFlags = Ripc.Flags.Optional.FRAGMENT_HEADER;
            headerLength = TransportBufferImpl._firstFragmentHeaderLength;
            maxPayloadSize = fragment.data().capacity() - headerLength;

            bigBuffer._data.position(0); // start at the beginning
            userBytesForFragment = fragment.data().capacity() - headerLength;
            bigBuffer._data.limit(userBytesForFragment);
        }
        else
        {
            optFlags = Ripc.Flags.Optional.FRAGMENT;
            headerLength = TransportBufferImpl._nextFragmentHeaderLength;
            maxPayloadSize = fragment.data().capacity() - headerLength;

            int bytesRemaining = limit - position; // bytes remaining in big buffer
            if (fragment.data().capacity() <= (bytesRemaining + headerLength))
            {
                userBytesForFragment = fragment.data().capacity() - headerLength;
            }
            else
            {
                userBytesForFragment = bytesRemaining;
            }
        }

        // Compress the selected number of bytes (userBytesForFragment) for the fragment
        compressedLen = _compressor.compress(bigBuffer.data(),
                                             bigBuffer.data().position(), // big buffer position points at data to be sent
                                             userBytesForFragment); // number of bytes to compress

        compressedBytes = _compressor.compressedData();

        if (compressedLen > maxPayloadSize)
        {
            // There is going to be an extra message after this, so set the COMP_FRAGMENT flag
            flags |= Ripc.Flags.COMP_FRAGMENT;
            if (firstFragment)
            {
                fragment.populateFirstFragment(flags, optFlags, bigBuffer.fragmentId(), totalLength, compressedBytes, 0, maxPayloadSize);
            }
            else
            {
                fragment.populateNextFragment(flags, optFlags, bigBuffer.fragmentId(), compressedBytes, 0, maxPayloadSize);
            }

            extraBytes = compressedLen - maxPayloadSize;
        }
        else
        {
            if (firstFragment)
            {
                fragment.populateFirstFragment(flags, optFlags, bigBuffer.fragmentId(), totalLength, compressedBytes, 0, compressedLen);
            }
            else
            {
                fragment.populateNextFragment(flags, optFlags, bigBuffer.fragmentId(), compressedBytes, 0, compressedLen);
            }
        }

        // add to the priority queues
        writeFragment(fragment, writeArgs);

        // If there are extra bytes that could not fit in the fragment, write the remainder of the compressed bytes into an extra message.
        // Extra bytes start at position userBytesForFragment (after data sent in previous message)
        if (extraBytes > 0)
        {
            // Populate second message
            compFragmentBuffer.data().position(compFragmentBuffer.dataStartPosition());
            compFragmentBuffer.data().limit(compFragmentBuffer.dataStartPosition() + extraBytes);
            compFragmentBuffer.data().put(compressedBytes, userBytesForFragment, extraBytes);
            compFragmentBuffer.populateRipcHeader(Ripc.Flags.COMPRESSION);

            writeFragment(compFragmentBuffer, writeArgs);

            extraTotalLength = Ripc.Lengths.HEADER + extraBytes; // actual length on wire
            extraHeaderLength = Ripc.Lengths.HEADER; // overhead (header) from sending extra part
        }
        else
        {
            compFragmentBuffer.returnToPool();
        }

        // Actual bytes on wire is total length of first fragment, plus total length on wire of extra bytes (if sent)
        ((WriteArgsImpl)writeArgs).bytesWritten(writeArgs.bytesWritten() + fragment._length + extraTotalLength);
        
        // Uncompressed bytes is the number of bytes taken from the big buffer before
        // compression, plus overhead for the one (or two) messages sent on wire
        ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(writeArgs.uncompressedBytesWritten() +
                                                            userBytesForFragment + headerLength + extraHeaderLength);

        // Adjust big buffer for next call
        // -- set the limit to end of big buffer user data
        bigBuffer.data().limit(limit);
        // -- new position will be set just after the data inserted in this
        // fragment
        bigBuffer.data().position(bigBuffer.data().position() + userBytesForFragment);

        // Tell the caller how many payload bytes were put in this fragment (uncompressed)
        return userBytesForFragment;
    }

    protected int writeBigBuffer(BigBuffer buffer, WriteArgs writeArgs, Error error)
    {
        int retVal = TransportReturnCodes.SUCCESS;
        int bytesLeft;
        boolean doCompression = false;

        // set ripcHdrFlags based on if compression enabled
        // compress if enabled
        if (_sessionOutCompression > 0 && (writeArgs.flags() & WriteFlags.DO_NOT_COMPRESS) == 0)
        {
            // set _compressPriority if necessary
            if (_compressPriority == 99)
            {
                _compressPriority = writeArgs.priority();
            }
            // only compress high priority
            if (writeArgs.priority() == _compressPriority)
            {
                doCompression = true;
            }
        }

        // check if this is the first write call of this buffer
        if (!buffer._isWritePaused)
        {
            bytesLeft = buffer._data.position();
            // copy data from bigBuffer to the first fragment
            if (!doCompression)
            {
                bytesLeft = bytesLeft - buffer._firstBuffer.populateFragment(buffer, true, Ripc.Flags.HAS_OPTIONAL_FLAGS | IPC_DATA, writeArgs);
                writeFragment(buffer._firstBuffer, writeArgs);
            }
            else
            {
                retVal = writeFragmentCompressed(buffer, buffer._firstBuffer, writeArgs, true /* first fragment */, error);
                if (retVal > TransportReturnCodes.SUCCESS)
                    bytesLeft = bytesLeft - retVal;
                else
                    return retVal;
            }

            buffer._firstBuffer = null;
        }
        else
        {
            // When resuming a paused write, initialize bytes remaining in big buffer
            bytesLeft = buffer._data.limit() - buffer._data.position();
        }

        // get buffers for the rest of the data and copy the data to the buffers
        while (bytesLeft > 0)
        {
            SocketBuffer sBuffer = getSocketBuffer();
            if (sBuffer == null)
            {
                retVal = flushInternal(error);
                if (retVal < TransportReturnCodes.SUCCESS)
                    return retVal;

                if ((sBuffer = getSocketBuffer()) == null)
                {
                    buffer._isWritePaused = true;
                    return TransportReturnCodes.WRITE_CALL_AGAIN;
                }
            }
            TransportBufferImpl nextBuffer = sBuffer.getBufferSliceForFragment(_internalMaxFragmentSize);

            if (!doCompression || bytesLeft < _sessionCompLowThreshold)
            {
                bytesLeft = bytesLeft - nextBuffer.populateFragment(buffer, false, Ripc.Flags.HAS_OPTIONAL_FLAGS | IPC_DATA, writeArgs);
                writeFragment(nextBuffer, writeArgs);
            }
            else
            {
                retVal = writeFragmentCompressed(buffer, nextBuffer, writeArgs, false /* not first */, error);
                if (retVal > TransportReturnCodes.SUCCESS)
                    bytesLeft = bytesLeft - retVal;
                else
                    return retVal;
            }
        }

        // if direct socket write or high water mark reached, call flush
        if ((writeArgs.flags() & WriteFlags.DIRECT_SOCKET_WRITE) > 0 || _totalBytesQueued > _highWaterMark)
        {
            if ((retVal = flushInternal(error)) < TransportReturnCodes.SUCCESS)
            {
                return retVal;
            }
        }
        else
        {
            retVal = _totalBytesQueued;
        }

        return retVal;
    }

    protected void writeFragment(TransportBufferImpl buffer, WriteArgs writeArgs)
    {
        // queue buffer
        addToPriorityQueue(buffer, writeArgs.priority());

        // update total bytes queued
        _totalBytesQueued += buffer._length;
    }

    @Override
    public int write(TransportBuffer bufferInt, WriteArgs writeArgs, Error error)
    {
        if (_isProviderHTTP)
            return _providerHelper.write(bufferInt, writeArgs, error);

        assert (bufferInt != null) : "buffer cannot be null";
        assert (writeArgs != null) : "writeArgs cannot be null";
        assert (error != null) : "error cannot be null";

        int retVal = TransportReturnCodes.SUCCESS;
        TransportBufferImpl buffer = (TransportBufferImpl)bufferInt;

        if (buffer._isOwnedByApp == false)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Application does not own this buffer.");
            return TransportReturnCodes.FAILURE;
        }

        if (buffer.encodedLength() == 0)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Encoded buffer of length zero cannot be written");
            return TransportReturnCodes.FAILURE;
        }

        int ripcHdrFlags = IPC_DATA;
        int msgLen = 0;

        if (buffer != null)
            msgLen = buffer.length();

        try
        {
            _writeLock.lock();

            // return FAILURE if channel not active
            if (_state != ChannelState.ACTIVE)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in active state for write");
                return TransportReturnCodes.FAILURE;
            }

            // determine whether the buffer holds a message to be fragmented
            if (buffer.isBigBuffer() == false)
            {
                // normal message - no fragmentation

                // pack if enabled
                if (buffer._isPacked)
                {
                    buffer.pack(false, this, error);
                    msgLen = buffer.packedLen();
                    ripcHdrFlags |= IPC_PACKING;
                }

                // compress if enabled
                boolean compressedDataSent = false;
                if (_sessionOutCompression > 0 && (writeArgs.flags() & WriteFlags.DO_NOT_COMPRESS) == 0)
                {
                    // only compress if within low and high thresholds
                    if (msgLen >= _sessionCompLowThreshold)
                    {
                        // set _compressPriority if necessary
                        if (_compressPriority == 99)
                        {
                            _compressPriority = writeArgs.priority();
                        }
                        // only compress with initial message priority
                        if (writeArgs.priority() == _compressPriority)
                        {
                            retVal = writeNormalCompressed(buffer, writeArgs, error);
                            compressedDataSent = true;
                        }
                    }
                }

                if (!compressedDataSent) // no compression: send normal buffer
                {
                    buffer.populateRipcHeader(ripcHdrFlags);

                    if (_totalBytesQueued > 0) // buffers queued
                    {
                        retVal = writeWithBuffersQueued(buffer, writeArgs, error);
                        if (retVal < TransportReturnCodes.SUCCESS)
                            return retVal;
                    }
                    else
                    // no buffers queued
                    {
                        retVal = writeWithNoBuffersQueued(buffer, writeArgs, error);
                        if (retVal < TransportReturnCodes.SUCCESS)
                            return retVal;
                    }

                    ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(msgLen + RIPC_HDR_SIZE);
                }

            }
            else
            // message fragmentation
            {
                // check if the buffer has to be fragmented; if not, use the Ripc protocol
                if ((((BigBuffer)buffer)._isWritePaused) || ((buffer._data.position() + RIPC_HDR_SIZE) > _internalMaxFragmentSize))
                {
                    // the message has to be fragmented
                    retVal = writeBigBuffer((BigBuffer)buffer, writeArgs, error);
                    if (retVal < TransportReturnCodes.SUCCESS)
                        return retVal;
                }
                else
                {
                    // send the message using Ripc protocol

                    // copy the data from bigBuffer to the first transport buffer
                    TransportBufferImpl transportBuffer = ((BigBuffer)buffer)._firstBuffer;
                    ((BigBuffer)buffer)._firstBuffer = null;
                    transportBuffer._data.position(3);
                    buffer._data.limit(buffer._data.position());
                    buffer._data.position(0);
                    transportBuffer._data.put(buffer._data);
                    transportBuffer.populateRipcHeader(IPC_DATA);

                    if (_totalBytesQueued > 0) // buffers queued
                    {
                        retVal = writeWithBuffersQueued(transportBuffer, writeArgs, error);
                        if (retVal < TransportReturnCodes.SUCCESS)
                            return retVal;
                    }
                    else
                    // no buffers queued
                    {
                        retVal = writeWithNoBuffersQueued(transportBuffer, writeArgs, error);
                        if (retVal < TransportReturnCodes.SUCCESS)
                            return retVal;
                    }
                }
                // The buffer has to be returned to the bigBuffer pool
                if (retVal != TransportReturnCodes.WRITE_CALL_AGAIN)
                    ((BigBuffer)buffer).returnToPool();
            }

            if (retVal >= TransportReturnCodes.SUCCESS || retVal == TransportReturnCodes.WRITE_FLUSH_FAILED && _state == ChannelState.ACTIVE)
                buffer._isOwnedByApp = false;
        }
        catch (CompressorException e)
        {
            _state = ChannelState.CLOSED;
            retVal = TransportReturnCodes.FAILURE;
            populateErrorDetails(error, TransportReturnCodes.FAILURE, "CompressorException: " + e.getLocalizedMessage());
        }
        catch (Exception e)
        {
            _state = ChannelState.CLOSED;
            retVal = TransportReturnCodes.FAILURE;
            populateErrorDetails(error, TransportReturnCodes.FAILURE, "Exception: " + e.getLocalizedMessage());
        }
        finally
        {
            _writeLock.unlock();
        }

        return retVal;
    }

    /* Takes the uncompressed buffer, compresses the data and writes it.
     * One or two ripc messages can be created from the compressed data.
     * This method handles a normal message, not big buffers for fragmentation.
     * 
     * buffer is the uncompressed data to be sent
     * 
     * Returns count of bytes queued if successful, or TransportReturnCodes for error scenarios.
     */
    protected int writeNormalCompressed(TransportBufferImpl buffer, WriteArgs writeArgs, Error error)
    {
        int retVal = 0;
        int bytesForBuffer = 0;
        int totalBytes = 0;
        int ripcHdrFlags = Ripc.Flags.COMPRESSION;
        int msgLen = buffer.length();
        final int MAX_BYTES_FOR_BUFFER = _internalMaxFragmentSize - RIPC_HDR_SIZE;

        // An extra buffer might be needed: get it now before compression
        TransportBufferImpl compFragmentBuffer = getBufferInternal(MAX_BYTES_FOR_BUFFER, false);
        if (compFragmentBuffer == null)
        {
            retVal = flushInternal(error);
            if (retVal < TransportReturnCodes.SUCCESS)
                return retVal;
            compFragmentBuffer = getBufferInternal(MAX_BYTES_FOR_BUFFER, false);
            if (compFragmentBuffer == null)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.NO_BUFFERS);
                error.sysError(0);
                error.text("channel out of buffers");
                return TransportReturnCodes.NO_BUFFERS;
            }
        }

        if (buffer._isPacked)
        {
            ripcHdrFlags |= Ripc.Flags.PACKING;
            msgLen = buffer.packedLen();
        }

        int compressedBytesLen = _compressor.compress(buffer, buffer.dataStartPosition(), msgLen);

        byte[] compressedBytes = _compressor.compressedData();

        if (compressedBytesLen > MAX_BYTES_FOR_BUFFER)
        {
            // The compressed data will be split into two ripc messages since the compressed size exceeded the buffer size.
            // This is possible when the uncompressed data is near the buffer size, and the data compresses poorly (data size grows).
            bytesForBuffer = MAX_BYTES_FOR_BUFFER;
            ripcHdrFlags |= Ripc.Flags.COMP_FRAGMENT;
        }
        else
        {
            bytesForBuffer = compressedBytesLen;
        }

        // Transfer compressed bytes to the transport buffer
        buffer.data().position(buffer.dataStartPosition());
        buffer.data().limit(buffer.dataStartPosition() + bytesForBuffer);
        buffer.data().put(compressedBytes, 0, bytesForBuffer);
        // Do this last (before write), so that internal buffer length and position set
        buffer.populateRipcHeader(ripcHdrFlags);

        if (_totalBytesQueued > 0)
        {
            retVal = writeWithBuffersQueued(buffer, writeArgs, error);
        }
        else
        {
            retVal = writeWithNoBuffersQueued(buffer, writeArgs, error);
        }

        // First part stats: User data bytes + overhead (ignore compression)
        ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(msgLen + Ripc.Lengths.HEADER);
        totalBytes = buffer._length;

        // Send extra message if there are bytes that did not fit in the first part
        if (retVal >= TransportReturnCodes.SUCCESS && compressedBytesLen > MAX_BYTES_FOR_BUFFER)
        {
            // Remaining compressed bytes to be sent
            bytesForBuffer = compressedBytesLen - MAX_BYTES_FOR_BUFFER;

            // Populate second message
            compFragmentBuffer.data().position(compFragmentBuffer.dataStartPosition());
            compFragmentBuffer.data().limit(compFragmentBuffer.dataStartPosition() + bytesForBuffer);
            compFragmentBuffer.data().put(compressedBytes, MAX_BYTES_FOR_BUFFER, bytesForBuffer);
            compFragmentBuffer.populateRipcHeader(Ripc.Flags.COMPRESSION);

            ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(writeArgs.uncompressedBytesWritten() + Ripc.Lengths.HEADER);
            totalBytes += compFragmentBuffer._length;

            // Write second part and flush
            if (_totalBytesQueued > 0)
            {
                retVal = writeWithBuffersQueued(compFragmentBuffer, writeArgs, error);
            }
            else
            {
                retVal = writeWithNoBuffersQueued(compFragmentBuffer, writeArgs, error);
            }
        }
        else
        {
            compFragmentBuffer.returnToPool();
        }

        // Total bytes on wire, for one or two messages sent
        ((WriteArgsImpl)writeArgs).bytesWritten(totalBytes);

        return retVal;
    }

    /* Write method used when _totalBytesQueued is > 0.
     * 
     * Returns count of bytes queued if successful, or TransportReturnCodes for error scenarios.
     */
    protected int writeWithBuffersQueued(TransportBufferImpl buffer, WriteArgs writeArgs, Error error)
    {
        int retVal = TransportReturnCodes.SUCCESS;
        int scktBytesWritten = 0;

        // queue buffer
        addToPriorityQueue(buffer, writeArgs.priority());

        // update total bytes queued
        _totalBytesQueued += buffer._length;

        // set socket bytes written
        scktBytesWritten = buffer._length;
        ((WriteArgsImpl)writeArgs).bytesWritten(scktBytesWritten);

        // if direct socket write or high water mark reached, call flush
        if ((writeArgs.flags() & WriteFlags.DIRECT_SOCKET_WRITE) > 0 || _totalBytesQueued > _highWaterMark)
        {
            retVal = flushInternal(error);
        }
        else
        {
            retVal = _totalBytesQueued;
        }

        return retVal;
    }

    /* Write method used when _totalBytesQueued is 0.
     * 
     * Returns count of bytes queued if successful, or TransportReturnCodes for error scenarios.
     */
    protected int writeWithNoBuffersQueued(TransportBufferImpl buffer, WriteArgs writeArgs, Error error)
    {
        int retVal = TransportReturnCodes.SUCCESS;
        int scktBytesWritten = 0;

        // if direct socket write, put in queue and call flush
        if ((writeArgs.flags() & WriteFlags.DIRECT_SOCKET_WRITE) > 0)
        {
            // queue buffer
            addToPriorityQueue(buffer, writeArgs.priority());

            // update total bytes queued
            _totalBytesQueued += buffer._length;

            // call flush
            retVal = flushInternal(error);
            if (retVal < TransportReturnCodes.SUCCESS)
                return retVal;

            // set socket bytes written
            scktBytesWritten = buffer._length - retVal;
            ((WriteArgsImpl)writeArgs).bytesWritten(scktBytesWritten);
        }
        else
        {
            // queue buffer
            addToPriorityQueue(buffer, writeArgs.priority());

            // update total bytes queued
            _totalBytesQueued += buffer._length;

            // set socket bytes written
            scktBytesWritten = buffer._length;
            ((WriteArgsImpl)writeArgs).bytesWritten(scktBytesWritten);

            // if high water mark reached, call flush
            if (_totalBytesQueued > _highWaterMark)
            {
                if ((retVal = flushInternal(error)) < TransportReturnCodes.SUCCESS)
                {
                    return retVal;
                }
            }
            else
            {
                retVal = _totalBytesQueued;
            }
        }

        return retVal;
    }

    @Override
    public int flush(Error error)
    {
        assert (error != null) : "error cannot be null";

        int retVal = TransportReturnCodes.SUCCESS;
        long scktBytesWritten = 0;
        int cumulativeBytesPendingWrite = 0;

        try
        {
            _writeLock.lock();

            // return FAILURE if channel not active
            if (_state != ChannelState.ACTIVE)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in the active state for flush");
                _needCloseSocket = true;
                if (_providerHelper != null)
                    _providerHelper.closeStreamingSocket();
                return TransportReturnCodes.FAILURE;
            }

            // write all bytes queued
            while (_totalBytesQueued > 0
                    && _state != ChannelState.INACTIVE
                    && _state != ChannelState.CLOSED)
            {
                // fill gathering byte array
                cumulativeBytesPendingWrite = fillGatheringByteArray();

                // write gathering byte array
                scktBytesWritten = writeGatheringByteArray(cumulativeBytesPendingWrite);
                if (scktBytesWritten < cumulativeBytesPendingWrite)
                {
                    break;
                }
            }
            // set return value to _totalBytesQueued
            retVal = _totalBytesQueued;
        }
        catch (Exception e)
        {
            _state = ChannelState.CLOSED;
            retVal = TransportReturnCodes.WRITE_FLUSH_FAILED;
            error.channel(this);
            error.errorId(TransportReturnCodes.WRITE_FLUSH_FAILED);
            error.sysError(0);

            if (_providerHelper != null && _providerHelper._wininetControl)
                _providerHelper.closeStreamingSocket();

            error.text(e.getLocalizedMessage());
        }
        finally
        {
            _writeLock.unlock();
        }

        return retVal;
    }

    /*
     * Expects the writeLock to be previously locked and expects the state to be ACTIVE.
     * Caller is expected to catch IOException.
     */
    protected int flushInternal(Error error)
    {
        assert (_state == ChannelState.ACTIVE);

        int retVal = TransportReturnCodes.SUCCESS;
        long scktBytesWritten = 0;
        int cumulativeBytesPendingWrite = 0;

        try
        {
            // write all bytes queued
            while (_totalBytesQueued > 0 
                    && _state != ChannelState.INACTIVE 
                    && _state != ChannelState.CLOSED)
            {
                // fill gathering byte array
                cumulativeBytesPendingWrite = fillGatheringByteArray();

                // write gathering byte array
                scktBytesWritten = writeGatheringByteArray(cumulativeBytesPendingWrite);
                if (scktBytesWritten < cumulativeBytesPendingWrite)
                {
                    break;
                }
            }
        }
        catch (IOException e)
        {
            _state = ChannelState.CLOSED;
            error.channel(this);
            error.errorId(TransportReturnCodes.WRITE_FLUSH_FAILED);
            error.sysError(0);
            error.text("The channel is closed ");
            if (RsslHttpSocketChannelProvider.debugPrint)
                System.out.println("FlushInternal fails on socket " + _scktChannel.toString());
            return TransportReturnCodes.WRITE_FLUSH_FAILED;
        }

        // set return value to _totalBytesQueued
        retVal = _totalBytesQueued;

        return retVal;
    }

    int fillGatheringByteArray()
    {
        int remainingBytesQueued = _totalBytesQueued;
        int cumulativeBytesPendingWrite = 0;
        int flushOrderPosition = 0;

        if (_isFlushPending == false) // no previously pending flush
        {
            _writeArrayMaxPosition = 0;
            _writeArrayPosition = 0;

            // fill gathering write array from priority queues
            while (_writeArrayMaxPosition < _gatheringWriteArray.length 
                    && remainingBytesQueued > 0
                    && _state != ChannelState.INACTIVE 
                    && _state != ChannelState.CLOSED)
            {
                for (int i = _flushOrderPosition; i < _channelInfo._priorityFlushStrategy.length()
                        && remainingBytesQueued > 0
                        && _writeArrayMaxPosition < _gatheringWriteArray.length; i++, flushOrderPosition = i)
                {
                    TransportBufferImpl buffer = (TransportBufferImpl)_flushOrder[i].poll();
                    if (buffer != null)
                    {
                        _gatheringWriteArray[_writeArrayMaxPosition] = buffer.data();
                        _releaseBufferArray[_writeArrayMaxPosition] = buffer;
                        _writeArrayMaxPosition++;
                        int bufSize = buffer.data().limit() - buffer.data().position();
                        cumulativeBytesPendingWrite += bufSize;
                        remainingBytesQueued -= bufSize;
                    }
                }

                _flushOrderPosition = 0;
            }
            _flushOrderPosition = flushOrderPosition % _channelInfo._priorityFlushStrategy.length();
        }
        else
        // handle pending flush (not all previous bytes were flushed)
        {
            for (int i = _writeArrayPosition; i < _writeArrayMaxPosition; i++)
            {
                cumulativeBytesPendingWrite += (_gatheringWriteArray[i].limit() - _gatheringWriteArray[i].position());
            }
        }

        return cumulativeBytesPendingWrite;
    }

    long writeGatheringByteArray(int cumulativeBytesPendingWrite) throws IOException
    {
        // write gathering byte array
        long scktBytesWritten = write(_gatheringWriteArray, _writeArrayPosition, _writeArrayMaxPosition - _writeArrayPosition);

        if (scktBytesWritten == cumulativeBytesPendingWrite) // all pending buffers sent
        {
            // release buffers
            for (int i = _writeArrayPosition; i < _writeArrayMaxPosition; i++)
            {
                releaseBufferInternal(_releaseBufferArray[i]);
            }
            // adjust _totalBytesQueued by pending buffers written
            _totalBytesQueued -= cumulativeBytesPendingWrite;

            // reset _isFlushPending
            _isFlushPending = false;
        }
        else
        // handle partial pending buffers sent case
        {
            // release buffers that were written
            int bytesReleased = 0, idx = _writeArrayPosition;
            while (bytesReleased < scktBytesWritten)
            {
                bytesReleased += _releaseBufferArray[idx].data().limit() - _releaseBufferArray[idx]._startPosition;
                if (bytesReleased <= scktBytesWritten) // full buffer
                {
                    releaseBufferInternal(_releaseBufferArray[idx]);
                }
                else
                // partial buffer
                {
                    // adjust position of partially written buffer
                    int pos = (int)(_gatheringWriteArray[idx].limit() - (bytesReleased - scktBytesWritten));
                    _gatheringWriteArray[idx].position(pos);
                    _releaseBufferArray[idx]._startPosition = pos;

                    break;
                }
                idx++;
            }

            // set _isFlushPending and _writeArrayPosition
            _isFlushPending = true;
            _writeArrayPosition = idx;

            // adjust _totalBytesQueued by partial pending buffers written
            _totalBytesQueued -= scktBytesWritten;
        }

        return scktBytesWritten;
    }

    @Override
    public int ping(Error error)
    {
        if (_isProviderHTTP)
            return _providerHelper.ping(error);
        assert (error != null) : "error cannot be null";

        int retVal = TransportReturnCodes.SUCCESS;

        // send ping
        try
        {
            _writeLock.lock();

            // return FAILURE if channel not active
            if (_state != ChannelState.ACTIVE)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in the active state for ping");
                return TransportReturnCodes.FAILURE;
            }

            if (_totalBytesQueued > 0)
            {
                // call flush since bytes queued
                retVal = flushInternal(error);
            }
            else
            // send ping buffer
            {
                _pingBuffer.rewind();
                retVal = RIPC_HDR_SIZE - _scktChannel.write(_pingBuffer);
            }
        }
        catch (Exception e)
        {
            _state = ChannelState.CLOSED;
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            if (_providerHelper != null && _providerHelper._wininetControl)
                _providerHelper.closeStreamingSocket();
            error.text(e.getLocalizedMessage());
            retVal = TransportReturnCodes.FAILURE;
        }
        finally
        {
            _writeLock.unlock();
        }

        return retVal;
    }

    /* opts should be checked for null prior to this call. */
    void dataFromOptions(ConnectOptions opts)
    {
        assert (opts != null);

        _channelInfo._compressionType = opts.compressionType();
        _channelInfo._pingTimeout = opts.pingTimeout();
        _channelInfo._guaranteedOutputBuffers = opts.guaranteedOutputBuffers();
        _channelInfo._maxOutputBuffers = _channelInfo._guaranteedOutputBuffers;
        _channelInfo._numInputBuffers = opts.numInputBuffers();
        _channelInfo._sysSendBufSize = opts.sysSendBufSize();
        _channelInfo._sysRecvBufSize = opts.sysRecvBufSize();
        _majorVersion = opts.majorVersion();
        _minorVersion = opts.minorVersion();
        _protocolType = opts.protocolType();
        _userSpecObject = opts.userSpecObject();
        if (opts.componentVersion() != null)
        {
            ByteBuffer connectOptsCompVerBB = ByteBuffer.wrap(opts.componentVersion().getBytes());
            _connectOptsComponentInfo = new ComponentInfoImpl();
            _connectOptsComponentInfo.componentVersion().data(connectOptsCompVerBB);
        }

        // compression
        _sessionInDecompress = opts.compressionType();

        _host = opts.unifiedNetworkInfo().address();
        _port = opts.unifiedNetworkInfo().serviceName();
        _portIntValue = ((UnifiedNetworkInfoImpl)opts.unifiedNetworkInfo()).port();
        
        _httpProxy = opts.tunnelingInfo().HTTPproxy();
        if (_httpProxy)
        {
            _httpProxyHost = opts.tunnelingInfo().HTTPproxyHostName();
            _httpProxyPort = opts.tunnelingInfo().HTTPproxyPort();
        }
        
        _objectName = opts.tunnelingInfo().objectName();
    }

    int connect(ConnectOptions opts, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;

        /* if re-connecting, don't block and call channel.init() until channel is active, since this case is a recursive call. */
        boolean blockUntilActive = _initChnlState != InitChnlState.RECONNECTING;

        // The first time connect is called by SocketProtocol, cache the ConnectOptions.
        // However, if connect is called again with opts,
        if (_cachedConnectOptions == null)
        {
            if (opts != null)
            {
                _cachedConnectOptions = opts;
                try
                {
                    _scktChannel.initialize(_cachedConnectOptions);
                }
                catch (IOException ex)
                {
                    error.channel(null);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Failed to initialize socket channel: " + ex.getMessage());
                    return TransportReturnCodes.FAILURE;
                }
                dataFromOptions(opts);
            }
            else
            {
                error.channel(null);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("connect options must be specified");
                return TransportReturnCodes.FAILURE;
            }
        }

        // if connect is called with different opts than cached, use new opts.
        else if (opts != null && opts != _cachedConnectOptions)
        {
            _cachedConnectOptions = opts;
            try
            {
                _scktChannel.initialize(_cachedConnectOptions);
            }
            catch (IOException ex)
            {
                error.channel(null);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("Failed to initialize socket channel: " + ex.getMessage());
                return TransportReturnCodes.FAILURE;
            }
            dataFromOptions(opts);
        }

        // The first time, _ipcProtocol will be null prior to nextProtocol call.
        // If nextProtocol returns null, don't override _ipcProtocool,
        // otherwise the next call to nextProtocol will return the newest protocol.
        // We want it to continue to return null so that we can abort and fail the connection.
        IpcProtocol p = _ipcProtocolManager.nextProtocol(this, _ipcProtocol, _cachedConnectOptions);
        if (p != null)
        {
            _ipcProtocol = p;
            _ipcProtocol.options(_cachedConnectOptions);
        }
        else
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("No more protocols to try");
            return TransportReturnCodes.FAILURE;
        }

        try
        {
            _totalBytesRead = 0;
            _scktChannel.setSocketChannel(java.nio.channels.SocketChannel.open());
            // use the values from ConnectOptions if set, otherwise defaults.
            if (_cachedConnectOptions.sysRecvBufSize() > 0)
                _scktChannel.socket().setReceiveBufferSize(_cachedConnectOptions.sysRecvBufSize());
            else if (_scktChannel.socket().getReceiveBufferSize() > READ_RECEIVE_BUFFER_SIZE)
                _scktChannel.socket().setReceiveBufferSize(_scktChannel.socket().getReceiveBufferSize());
            else
                _scktChannel.socket().setReceiveBufferSize(READ_RECEIVE_BUFFER_SIZE);

            if (_cachedConnectOptions.sysSendBufSize() > 0)
                _scktChannel.socket().setSendBufferSize(_cachedConnectOptions.sysSendBufSize());
            else if (_scktChannel.socket().getSendBufferSize() > READ_RECEIVE_BUFFER_SIZE)
                _scktChannel.socket().setSendBufferSize(_scktChannel.socket().getSendBufferSize());
            else
                _scktChannel.socket().setSendBufferSize(READ_RECEIVE_BUFFER_SIZE);

            if (!_cachedConnectOptions.blocking())
            {
                _scktChannel.configureBlocking(false);
            }

            if (_cachedInetSocketAddress == null)
            {
                // for tunneling through a proxy, we will connect to a proxy
                if (_httpProxy)
                {
                    _cachedInetSocketAddress = new InetSocketAddress(_httpProxyHost, _httpProxyPort);
                    _proxyCredentails = readProxyCredentails(_cachedConnectOptions);
                    _proxyAuthenticator = ProxyAuthenticatorFactory.create(_proxyCredentails, _httpProxyHost);
                }
                else
                    _cachedInetSocketAddress = new InetSocketAddress(_cachedConnectOptions.unifiedNetworkInfo().address(),
                                                                     ((UnifiedNetworkInfoImpl)(_cachedConnectOptions.unifiedNetworkInfo())).port());
            }

            // if interfaceName is specified, bind to NIC
            String interfaceName = _cachedConnectOptions.unifiedNetworkInfo().interfaceName();
            if ((interfaceName != null) && (!interfaceName.isEmpty()))
            {
                // get equivalent IP address
                if (interfaceName.equals("0") || interfaceName.equals("localhost"))
                {
                    String ipAddress = InetAddress.getLocalHost().getHostAddress();
                    interfaceName = ipAddress;
                }

                _cachedBindInetSocketAddress = new InetSocketAddress(interfaceName, 0);

                _scktChannel.bind(_cachedBindInetSocketAddress);
            }

            if (_cachedConnectOptions.tcpOpts().tcpNoDelay())
            {
                try
                {
                    _scktChannel.socket().setTcpNoDelay(true);
                }
                catch (SocketException e) // fails on Windows Server 2008 64 bit
                {
                    System.out.println(""); // need print to avoid another exception
                    try
                    {
                        _scktChannel.socket().setTcpNoDelay(true);
                    }
                    catch (SocketException e2) // failed twice - return failure
                    {
                        _state = ChannelState.CLOSED;
                        error.channel(null);
                        error.errorId(TransportReturnCodes.FAILURE);
                        error.sysError(0);
                        error.text("java.net.Socket.setTcpNoDelay(true) failed");
                        return TransportReturnCodes.FAILURE;
                    }
                }
            }

            // connect
            _scktChannel.connect(_cachedInetSocketAddress, _httpProxy);

            if (_httpProxy)
                _initChnlState = InitChnlState.PROXY_CONNECTING;
            else
            {
                _initChnlState = InitChnlState.CONNECTING;
            }

            _state = ChannelState.INITIALIZING;
            error.channel(this);
            error.errorId(TransportReturnCodes.SUCCESS);

            if (_cachedConnectOptions.channelReadLocking())

                _readLock = _realReadLock;
            else
                _readLock = _dummyReadLock;

            if (_cachedConnectOptions.channelWriteLocking())
                _writeLock = _realWriteLock;
            else
                _writeLock = _dummyWriteLock;

            // if blocking connect, call channel.init() and get into ACTIVE state before returning
            if (_cachedConnectOptions.blocking() && blockUntilActive)
            {
                InProgInfo inProg = TransportFactory.createInProgInfo();
                while (_state != ChannelState.ACTIVE)
                {
                    if ((ret = init(inProg, error)) < TransportReturnCodes.SUCCESS)
                    {
                        if (ret == TransportReturnCodes.FAILURE 
                                && _initChnlState == InitChnlState.RECONNECTING)
                        {
                            // The far end closed connection. Try another protocol.
                            if (connect(_cachedConnectOptions, error) == TransportReturnCodes.SUCCESS)
                            {
                            	if (_httpProxy)
                                {
                                    _initChnlState = InitChnlState.PROXY_CONNECTING;
                                }
                                else
                                {
                                    _initChnlState = InitChnlState.CONNECTING;
                                }
                                continue;
                            }
                            else
                            {
                                error.channel(this);
                                error.errorId(TransportReturnCodes.FAILURE);
                                error.sysError(0);
                                error.text("Handshake failed with far end. No more Protocols to try");
                                ret = TransportReturnCodes.FAILURE;
                            }
                        }
                        return ret;
                    }
                }
            }
        }
        catch (Exception e)
        {
            _state = ChannelState.CLOSED;
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            if (e.getLocalizedMessage() != null)
            {
                error.text(e.getLocalizedMessage());
            }
            else
            {
                error.text(e.toString());
            }
            ret = TransportReturnCodes.FAILURE;
        }
        return ret;
    }

    protected void setDataFromOptions(AcceptOptions opts, BindOptions bindOptions)
    {
        if (opts == null)
        {
            throw new NullPointerException("accept options must be specified");
        }

        _channelInfo._compressionType = bindOptions.compressionType();
        _channelInfo._pingTimeout = bindOptions.pingTimeout();
        _channelInfo._guaranteedOutputBuffers = bindOptions.guaranteedOutputBuffers();
        _channelInfo._maxOutputBuffers = bindOptions.maxOutputBuffers();
        _channelInfo._numInputBuffers = bindOptions.numInputBuffers();
        _majorVersion = bindOptions.majorVersion();
        _minorVersion = bindOptions.minorVersion();
        _protocolType = bindOptions.protocolType();
        _userSpecObject = opts.userSpecObject();

        // compression
        _sessionInDecompress = bindOptions.compressionType();
        _sessionCompLevel = (short)bindOptions.compressionLevel();
    }

    protected int setChannelAccept(AcceptOptions acceptOptions, BindOptions bindOptions, java.nio.channels.SocketChannel socketChannel, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;
        setDataFromOptions(acceptOptions, bindOptions);
        _nakMount = acceptOptions.nakMount();

        // set socket channel
        // it was created by server accept
        _scktChannel.setSocketChannel(socketChannel);

        // clear _initChnlBuffer prior to reading.
        _initChnlReadBuffer.clear();

        
        _state = ChannelState.INITIALIZING;
        
        if(_encrypted)
        {
        	try
        	{
        		_scktChannel.initialize(bindOptions, _server.context());
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
        // init will not progress until EncryptedSocketHelper or SocketHelper finishConnect() returns true.
        // In the encrypted case, this will happen after the encryption dance has been completed.
       	_initChnlState = InitChnlState.READ_HDR;


        if (acceptOptions.channelReadLocking())
            _readLock = _realReadLock;
        else
            _readLock = _dummyReadLock;

        if (acceptOptions.channelWriteLocking())
            _writeLock = _realWriteLock;
        else
            _writeLock = _dummyWriteLock;

        // if blocking accept, call channel.init() and get into ACTIVE state before returning
        if (_scktChannel.isBlocking())
        {
            InProgInfo inProg = TransportFactory.createInProgInfo();
            while (_state != ChannelState.ACTIVE)
            {
                if ((ret = init(inProg, error)) < TransportReturnCodes.SUCCESS)
                {
                    return ret;
                }
            }
        }

        return ret;
    }

    /* To facilitate unit testing, overrides the capacity of the read buffer when it is allocated.
     * This method must be invoked before createReadBuffers(int)
     * 
     * capacity is the capacity of the "read buffer"
     */
    protected void overrideReadBufferCapacity(int capacity)
    {
        assert (_readIoBuffer == null);

        _overrideReadBufferCapacity = capacity;
    }

    private void createReadBuffers(int numReadBuffers)
    {
        final int readBufferCapacity;

        // note: only the unit test framework overrides the read buffer capacity
        if (_overrideReadBufferCapacity == null)
        {
            readBufferCapacity = Math.max((numReadBuffers + 2) * _internalMaxFragmentSize, MIN_READ_BUFFER_SIZE);
        }
        else
        {
            // only used during unit testing
            readBufferCapacity = _overrideReadBufferCapacity;
        }
        assert (readBufferCapacity > 0);

        _readIoBuffer = acquirePair(readBufferCapacity); // the "read buffer" for network I/O
        assert (_readIoBuffer != null);
        _appReadBuffer.data(_readIoBuffer.readOnly());
        _readBufStateMachine.initialize(_readIoBuffer);
    }

    @Override
    public String toString()
    {
        StringBuilder s = new StringBuilder();
        s.append("RsslChannel");
        s.append("\n\tscktChannel: ");
        s.append(_scktChannel);
        s.append("\n\tconnected: ");
        if (_scktChannel != null)
            s.append(_scktChannel.isConnected());
        else
            s.append("false");
        s.append("\n\toldScktChannel: ");
        s.append(_oldScktChannel);
        s.append("\n\tstate: ");
        s.append(ChannelState.toString(_state));
        s.append("\n\tconnectionType: ");
        s.append(ConnectionTypes.toString(connectionType()));
        if (_channelInfo != null)
        {
            s.append("\n\tclientIP: ");
            s.append(_channelInfo._clientIP);
            s.append("\n\tclientHostname: ");
            s.append(_channelInfo._clientHostname);
            s.append("\n\tpingTimeout: ");
            s.append(_channelInfo._pingTimeout);
        }
        s.append("\n\tmajorVersion: ");
        s.append(_majorVersion);
        s.append("\n\tminorVersion: ");
        s.append(_minorVersion);
        s.append("\n\tprotocolType: ");
        s.append(_protocolType);
        s.append("\n\tuserSpecObject: ");
        s.append(_userSpecObject);
        s.append("\n");

        return s.toString();
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

    @Override
    public String hostname()
    {
    	return _host;
    }

    @Override
    public int port()
    {
        return _portIntValue;
    }

    @Override @Deprecated
    public java.nio.channels.SocketChannel scktChannel()
    {
        return _scktChannel.getSocketChannel();
    }

    @Override @Deprecated
    public java.nio.channels.SocketChannel oldScktChannel()
    {
        return _oldScktChannel.getSocketChannel();
    }

    @Override
    public SelectableChannel selectableChannel()
    {
    	if (_providerHelper != null && _providerHelper._pipeNode != null)
    		return _providerHelper._pipeNode._pipe.source();
    	
        return _scktChannel.getSocketChannel();
    }

    @Override
    public SelectableChannel oldSelectableChannel()
    {
        return _oldScktChannel.getSocketChannel();
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
        return _scktChannel.isBlocking();
    }

    @Override
    public int init(InProgInfo inProg, Error error)
    {
        assert (inProg != null) : "inProg cannot be null";
        assert (error != null) : "error cannot be null";

        int retVal = TransportReturnCodes.FAILURE;
        // initialize this to success so we can distinguish between when internal called methods populate error vs. when they do not.
        error.errorId(TransportReturnCodes.SUCCESS);
        inProg.clear();

        try
        {
            lockReadWriteLocks();

            // don't exchange RIPC messages until nio socket channel is finished connecting
            if (!_scktChannel.finishConnect())
            {
                return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
            }

            switch (_initChnlState)
            {
                case InitChnlState.CONNECTING:
                    retVal = initChnlSendConnectReq(inProg, error);
                    break;
                case InitChnlState.WAIT_ACK:
                    retVal = initChnlWaitConnectAck(inProg, error);
                    if (_initChnlState == InitChnlState.RECONNECTING && !_cachedConnectOptions.blocking())
                    {
                        // The far end closed connection. Try another protocol.
                        ((InProgInfoImpl)inProg).oldSelectableChannel(_scktChannel.getSocketChannel());
                        if (connect(_cachedConnectOptions, error) == TransportReturnCodes.SUCCESS)
                        {
                        	if (_httpProxy)
                                _initChnlState = InitChnlState.PROXY_CONNECTING;
                            else
                               _initChnlState = InitChnlState.CONNECTING;

                            ((InProgInfoImpl)inProg).flags(InProgFlags.SCKT_CHNL_CHANGE);
                            ((InProgInfoImpl)inProg).newSelectableChannel(_scktChannel.getSocketChannel());
                            return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
                        }
                        else
                        {
                            error.channel(this);
                            error.errorId(TransportReturnCodes.FAILURE);
                            error.sysError(0);
                            error.text("Handshake failed with far end. No more Protocols to try.");
                            return TransportReturnCodes.FAILURE;
                        }
                    }
                    break;
                case InitChnlState.PROXY_CONNECTING:
                    retVal = initChnlProxyConnecting();
                    break;
                case InitChnlState.CLIENT_WAIT_PROXY_ACK:
                    retVal = initChnlWaitProxyAck(inProg, error); // may throw a ProxyAuthenticationException
                    if(retVal == TransportReturnCodes.FAILURE)
                    {
                    	_needCloseSocket = true;
                        return TransportReturnCodes.FAILURE;
                    }
                    if (_proxyAuthenticator.isAuthenticated())
                    {
                    	/* If this is encrypted, start the encryption dance. _scktChannel.finishConnect will return false until the TLS handshake has finished.  */
                    	if(_encrypted)
                    	{
                    		_scktChannel.postProxyInit();
                    		_initChnlState = InitChnlState.CONNECTING;
                    		// break out here, we're done
                    		break;
                    	}
                        retVal = initChnlSendConnectReq(inProg, error);
                    }
                    break;
                case InitChnlState.READ_HDR:
                    retVal = initChnlReadHdr(inProg, error);
                    break;
                case InitChnlState.WAIT_CLIENT_KEY:
                    retVal = initChnlReadClientKey(inProg, error);
                    break;
                default:
                    break;
            }
        }
        catch (IOException | ProxyAuthenticationException e)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getLocalizedMessage());
            _needCloseSocket = true;
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            unlockReadWriteLocks();
        }
        if ((retVal < TransportReturnCodes.SUCCESS) && (error.errorId() != TransportReturnCodes.SUCCESS))
        {
            error.channel(this);
            error.errorId(retVal);
            error.sysError(0);
            error.text("Error occurred during connection process.");
            if (_providerHelper != null && !_providerHelper._wininetStream)
            {
                _needCloseSocket = true;
                _providerHelper.closeStreamingSocket();
            }
        }
        return retVal;
    }

    protected int initChnlSendConnectReq(InProgInfo inProg, Error error) throws UnknownHostException, IOException
    {
        /* Connected Component Info */
        if (_componentInfo.componentVersion().data() == null)
        {
            try
            {
                Transport._globalLock.lock();

                if (_connectOptsComponentInfo != null)
                {
                    byte divider = (byte)'|';
                    int totalLength =
                            _connectOptsComponentInfo.componentVersion().data().limit() + 1 + Transport._defaultComponentVersionBuffer.limit();
                    int origLimit = _connectOptsComponentInfo.componentVersion().data().limit();
                    if (totalLength > 253)
                    {
                        // the total component data length is too long, so truncate the user defined data
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
            }
            finally
            {
                Transport._globalLock.unlock();
            }
        }
        _ipcProtocol.protocolOptions()._componentInfo = _componentInfo;
        _scktChannel.write(_ipcProtocol.encodeConnectionReq(_initChnlWriteBuffer));
        
        // clear _initChnlBuffer prior to reading.
        _initChnlReadBuffer.clear();
        _initChnlState = InitChnlState.WAIT_ACK;

        return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
    }

    /* Read and decode a ConnectAck (or ConnectNak).
     * 
     * Returns TransportReturnCodes
     */
    protected int initChnlWaitConnectAck(InProgInfo inProg, Error error)
    {
        /* offset is the offset into the ByteBuffer where the RIPC message starts.
         * Will always be 0 if not tunneling. */
        int offset = 0;
        int cc = 0;

        try
        {
            // clear is done when _initChnlState advances from CONNECTING to WAIT_ACK.
            cc = initChnlReadFromChannel(_initChnlReadBuffer, error);

            if (cc > 0)
            {
                _totalBytesRead += cc;
            }
            else if (cc == 0)
            {
                /* This will happen when there is no data at all to be read
                 * and the read fails with error _IPC_WOULD_BLOCK or EINTR */
                return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
            }
            else if (cc == -1)
            {
                /* This will happen if all IpcProtocols were rejected.
                 * initChnlReadFromChannel() will populate Error. */
                return TransportReturnCodes.FAILURE;
            }

            if (cc < IPC_100_CONN_ACK)
            {
                /* 451 ACK is more so it safe to assume an error */
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("Invalid IPC Mount Ack");
                return TransportReturnCodes.FAILURE;
            }

            // uncomment the statement below to record data for NetworkReplay:
            /* System.out.println(Transport.toHexString(_initChnlReadBuffer, 0, _initChnlReadBuffer.position())); */

            /* Decode the Connection Reply */
            int savePos = _initChnlReadBuffer.position();
            int retCode = _ipcProtocol.decodeConnectionReply(_initChnlReadBuffer, offset + HTTP_HEADER4, error);
            _initChnlReadBuffer.position(savePos);
            if (retCode != TransportReturnCodes.SUCCESS)
                return retCode;

            /* _internalMaxFragmentSize is used internal to represent the size of the buffers that will be create internally.
             * The size will be RIPC MAX_USER_MSG_SIZE + RIPC_HDR_SIZE.
             * Note that this is different from _channelInfo._maxFragmentSize which is returned to
             * the user and is RIPC MAX_USER_MSG_SIZE - RIPC PACKED_HDR_SIZE.
             */
            IpcProtocolOptions protocolOptions = _ipcProtocol.protocolOptions();
            _internalMaxFragmentSize = protocolOptions._maxUserMsgSize + RIPC_HDR_SIZE;

            _appReadBuffer = new TransportBufferImpl(_internalMaxFragmentSize);
            _channelInfo._maxFragmentSize = protocolOptions._maxUserMsgSize - RIPC_PACKED_HDR_SIZE;
            _channelInfo._pingTimeout = protocolOptions._pingTimeout;
            _channelInfo._clientToServerPings = (((protocolOptions._serverSessionFlags & Ripc.SessionFlags.CLIENT_TO_SERVER_PING) == 1) ? true : false);
            _channelInfo._serverToClientPings = (((protocolOptions._serverSessionFlags & Ripc.SessionFlags.SERVER_TO_CLIENT_PING) == 2) ? true : false);
            _channelInfo._compressionType = protocolOptions._sessionCompType;
            _sessionCompLevel = protocolOptions._sessionCompLevel;
            _sessionInDecompress = protocolOptions._sessionInDecompress;
            _sessionOutCompression = protocolOptions._sessionOutCompression;

            _bigBuffersPool = new BigBuffersPool(_internalMaxFragmentSize, this);

            /* received Component Version Information */
            _channelInfo._receivedComponentInfoList = protocolOptions._receivedComponentVersionList;

            // set shared key to 0 here to cover us in reconnect cases
            _shared_key = 0;

            if (_transport != null) // need to add this check for junit
            {
                // allocate buffers to this channel, _availableBuffers size will initally be zero.
                growGuaranteedOutputBuffers(_channelInfo._guaranteedOutputBuffers);
            }

            if (_sessionInDecompress > Ripc.CompressionTypes.NONE)
            {
                _decompressBuffer = new TransportBufferImpl(_internalMaxFragmentSize);

                if (_sessionInDecompress == Ripc.CompressionTypes.ZLIB)
                {
                    _compressor = _ZlibCompressor;
                    _sessionCompLowThreshold = ZLIB_COMPRESSION_THRESHOLD;
                    _compressor.compressionLevel(_sessionCompLevel);
                }
                else if (_sessionInDecompress == Ripc.CompressionTypes.LZ4)
                {
                    _compressor = _Lz4Compressor;
                    _sessionCompLowThreshold = LZ4_COMPRESSION_THRESHOLD;
                }

                _compressor.maxCompressionLength(_internalMaxFragmentSize);
            }

            // create read/write buffer pools
            createReadBuffers(_channelInfo._numInputBuffers);
            _readIoBuffer.buffer().clear();

            /* If we read more bytes than in the header, put them into the read buffer.
             * This can happen in quick applications that send data as soon as the session becomes active. */
            if (cc > _initChnlReadBuffer.position())
            {
                int putback = cc - _initChnlReadBuffer.position();
                byte[] tempBuf = new byte[putback];
                _initChnlReadBuffer.get(tempBuf);
                _readIoBuffer.buffer().clear();
                _readIoBuffer.buffer().put(tempBuf);
            }

            /* If we are doing Key exchange, send the client ack here */
            if (protocolOptions._keyExchange == true)
            {
                return initChnlSendClientAck(inProg, error);
            }
            else
            {
                _initChnlState = InitChnlState.ACTIVE;
                _state = ChannelState.ACTIVE;
                _readBufStateMachine.ripcVersion(_ipcProtocol.ripcVersion());
                return TransportReturnCodes.SUCCESS;
            }

        }
        catch (Exception e)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getLocalizedMessage());
            return TransportReturnCodes.FAILURE;
        }
    }

    protected int initChnlSendClientAck(InProgInfo inProg, Error error) throws IndexOutOfBoundsException, IOException
    {
        _initChnlWriteBuffer.clear();

        _scktChannel.write(_ipcProtocol.encodeClientKey(_initChnlWriteBuffer, error));

        _initChnlState = InitChnlState.ACTIVE;
        _state = ChannelState.ACTIVE;
        _readBufStateMachine.ripcVersion(_ipcProtocol.ripcVersion());

        // set shared key - client side would be calculated here
        _shared_key = _ipcProtocol.protocolOptions()._shared_key;

        return TransportReturnCodes.SUCCESS;
    }

    protected int initChnlReadHdr(InProgInfo inProg, Error error)
    {
        int cc;

        try
        {
            cc = initChnlReadFromChannelProvider(_initChnlReadBuffer, error);

            if (_isProviderHTTP)
            {
                if (_nakMount)
                {
                    _needCloseSocket = true;
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Cannot complete IPC mount. ");
                    _providerHelper.closeStreamingSocket();
                    return TransportReturnCodes.FAILURE;
                }

                if (_providerHelper == null)
                {
                    _providerHelper = new RsslHttpSocketChannelProvider(this);
                }

                if (!_providerHelper._opcodeRead)
                {
                    int status = _providerHelper.initChnlHandlePost(inProg, cc, _initChnlReadBuffer, _initChnlWriteBuffer, error);

                    if (status == TransportReturnCodes.READ_FD_CHANGE)
                    {
                        // fake ACTIVE to distinguish from timeout close() while in INIT phase for later close() call
                        _state = ChannelState.ACTIVE;
                        _totalBytesRead += cc;
                        return status;
                    }
                    if (status == TransportReturnCodes.FAILURE || status == TransportReturnCodes.CHAN_INIT_IN_PROGRESS)
                    {
                        _totalBytesRead += cc;

                        return status;
                    }
                    else if (_providerHelper._wininetStream)
                    {
                        _totalBytesRead += cc;
                        return TransportReturnCodes.CHAN_INIT_IN_PROGRESS; // simulated for application
                    }
                    else if (!_providerHelper._ripcMsgPending)
                    {
                        _totalBytesRead += cc;
                        return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
                    }
                }
            }

            if (cc > 0)
            {
                _totalBytesRead += cc;

                if (_providerHelper == null || !_providerHelper._wininetStream)
                {
                    return initChnlProcessHdr(inProg, error, cc);
                }
                else
                {
                    return TransportReturnCodes.SUCCESS;
                }
            }
            else if (cc == 0)
            {
                /* This will happen when there is no data at all to be read, and the read fails with error _IPC_WOULD_BLOCK or EINTR */
                return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
            }
            else if (_initChnlState == InitChnlState.RECONNECTING)
            {
                /* This happens when the far end (server) closes the connection and we need to try another protocol. */
                _shared_key = 0;
                return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
            }
            else
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                if (_providerHelper == null || _providerHelper._wininetControl)
                    error.text("Could not read IPC Mount Request");
                if (_providerHelper != null && _providerHelper._wininetControl)
                {
                    if (RsslHttpSocketChannelProvider.debugPrint)
                        System.out.println(" Winnet Control Channel Error..");
                    _providerHelper.closeStreamingSocket();
                }

                return TransportReturnCodes.FAILURE;
            }
        }
        catch (Exception e)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            if (_providerHelper != null && _providerHelper._wininetControl)
                _providerHelper.closeStreamingSocket();
            error.text(e.getLocalizedMessage());
            if (_providerHelper != null && RsslHttpSocketChannelProvider.debugPrint)
            {
                System.out.println("InitChnlReadHdr failed..");
            }
            return TransportReturnCodes.FAILURE;
        }
    }

    protected int initChnlProcessHdr(InProgInfo inProg, Error error, int cc) throws IndexOutOfBoundsException, IOException
    {
        /* use a long to hold versionNumber as an unsigned int */
        int versionNumber;
        int length = 0;
        @SuppressWarnings("unused")
        int opCode = 0;

        if (cc < 7)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Invalid mount request size <" + cc + ">");
            return TransportReturnCodes.FAILURE;
        }

        int pos = 0;
        if (_providerHelper != null && _providerHelper._ripcMsgPending)
        {
            pos = _initChnlReadBuffer.position();
        }

        /* initChnlReadFromChannel will guarantee that length == cc */
        length = _initChnlReadBuffer.getShort(pos);
        opCode = _initChnlReadBuffer.get(pos + 2);

        /* read and convert from a signed to unsigned int (long) */
        versionNumber = _initChnlReadBuffer.getInt(pos + 3);

        IpcProtocol p = _ipcProtocolManager.determineProtocol(this, versionNumber);
        if (p != null)
        {
            _ipcProtocol = p;
            _ipcProtocol.options(_server.bindOptions());
        }
        else
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Unsupported RIPC version number <" + versionNumber + ">" + "sessionId = " + _providerSessionId);
            return TransportReturnCodes.FAILURE;
        }
        int retval = _ipcProtocol.decodeConnectionReq(_initChnlReadBuffer, pos, length, error);
        if (retval != TransportReturnCodes.SUCCESS)
            return retval;

        IpcProtocolOptions protocolOptions = _ipcProtocol.protocolOptions();
        _internalMaxFragmentSize = protocolOptions._maxUserMsgSize + RIPC_HDR_SIZE;

        _appReadBuffer = new TransportBufferImpl(_internalMaxFragmentSize);

        /* create bigBuffersPool */
        _bigBuffersPool = new BigBuffersPool(_internalMaxFragmentSize, this);

        /* allocate buffers to this channel */
        growGuaranteedOutputBuffers(_server.bindOptions().guaranteedOutputBuffers());
        _channelInfo._maxFragmentSize = protocolOptions._maxUserMsgSize - RIPC_PACKED_HDR_SIZE;
        _sessionOutCompression = protocolOptions._sessionOutCompression;
        _sessionInDecompress = _sessionOutCompression;

        /* Compression both directions */
        if (_sessionInDecompress > Ripc.CompressionTypes.NONE)
        {
            _decompressBuffer = new TransportBufferImpl(_internalMaxFragmentSize);

            if (_sessionInDecompress == Ripc.CompressionTypes.ZLIB)
            {
                _compressor = _ZlibCompressor;
                _sessionCompLowThreshold = ZLIB_COMPRESSION_THRESHOLD;
            }
            else if (_sessionInDecompress == Ripc.CompressionTypes.LZ4)
            {
                _compressor = _Lz4Compressor;
                _sessionCompLowThreshold = LZ4_COMPRESSION_THRESHOLD;
            }
            _compressor.compressionLevel(_sessionCompLevel);

            _compressor.maxCompressionLength(_internalMaxFragmentSize);
        }

        /* Negotiate the ping timeout between the server's values (pingTimeout
         * and minPingTimeout) and what the client sent in the RIPC ConnectReq.
         * The server's pingTimeout is the maximum allowed pingTimeout.
         * The server's minPingTimeout is the minimum allowed pingTimeout.
         * If the client's pingTimeout is within the server's pingTimeout and
         * minPingTimout, the client's pingTimeout will be used.
         * Otherwise, the server's max or min value will be used.
         */
        int clientPingTimeout = protocolOptions._pingTimeout;
        if (clientPingTimeout > _server.bindOptions().pingTimeout())
        {
            /* client's pingTimout was larger than server's (max) pingTimeout */
            _channelInfo._pingTimeout = _server.bindOptions().pingTimeout();
            protocolOptions._pingTimeout = _server.bindOptions().pingTimeout();
        }
        else if (clientPingTimeout < _server.bindOptions().minPingTimeout())
        {
            /* client's pingTimeout was smaller than server's minPingTimeout */
            _channelInfo._pingTimeout = _server.bindOptions().minPingTimeout();
            protocolOptions._pingTimeout = _server.bindOptions().minPingTimeout();
        }
        else
        {
            /* client's pingTimeout is within the server's max and min pingTimeout */
            _channelInfo._pingTimeout = clientPingTimeout;
            protocolOptions._pingTimeout = clientPingTimeout;
        }

        _channelInfo._clientToServerPings =
                (((protocolOptions._serverSessionFlags & Ripc.SessionFlags.CLIENT_TO_SERVER_PING) == 1) ? true : false);
        _channelInfo._serverToClientPings =
                (((protocolOptions._serverSessionFlags & Ripc.SessionFlags.SERVER_TO_CLIENT_PING) == 2) ? true : false);

        /* validate protocolType */
        if (_protocolType != protocolOptions._protocolType)
        {
            /* cause NAK to be sent */
            _nakMount = true;
        }

        /* use the smaller Major/Minor version number */
        if (protocolOptions._majorVersion < _majorVersion)
        {
            /* client major version is smaller than ours */
            _majorVersion = protocolOptions._majorVersion;
            _minorVersion = protocolOptions._minorVersion;
        }
        else if (protocolOptions._majorVersion == _majorVersion)
        {
            if (protocolOptions._minorVersion < _minorVersion)
            {
                /* client major version matches ours, however client minor version is smaller than ours, use client version */
                _minorVersion = protocolOptions._minorVersion;
            }
        }
        /* else we leave the versions alone as ours are smaller */

        /* get host name and ip address */
        _channelInfo._clientHostname = protocolOptions._clientHostName;
        _channelInfo._clientIP = protocolOptions._clientIpAddress;

        /* received Component Version Information */
        _channelInfo._receivedComponentInfoList = protocolOptions._receivedComponentVersionList;

        /* create read/write buffer pools */
        createReadBuffers(_channelInfo._numInputBuffers);

        if (protocolOptions._keyExchange == true)
        {
            _initChnlReadClientKeyBuffer.clear(); /* Clear buffer before reading key. */
            _initChnlState = InitChnlState.WAIT_CLIENT_KEY;
        }
        else
            _initChnlState = InitChnlState.COMPLETE;

        return initChnlFinishSess(inProg, error);
    }

    protected int initChnlFinishSess(InProgInfo inProg, Error error) throws IndexOutOfBoundsException, IOException
    {
        if (_nakMount)
        {
            initChnlRejectSession(error);
            return TransportReturnCodes.FAILURE;
        }
        else
        {
            /* Connected Component Info */
            _ipcProtocol.protocolOptions()._componentInfo = _componentInfo;

            if (_providerHelper != null)
            {
                int retVal = _providerHelper.initChnlFinishSess(inProg, error);
                if (retVal != TransportReturnCodes.SUCCESS)
                    return retVal;
            }
            else
                _scktChannel.write(_ipcProtocol.encodeConnectionAck(_initChnlWriteBuffer, error));

            /* If we don't have to wait for the key to come back to us, we are active */
            if (_initChnlState == InitChnlState.COMPLETE)
            {
                _initChnlState = InitChnlState.ACTIVE;
                _state = ChannelState.ACTIVE;
                _readBufStateMachine.ripcVersion(_ipcProtocol.ripcVersion());
                return TransportReturnCodes.SUCCESS;
            }
            else
            {
                /* clear read buffer before reading again */
                _initChnlReadBuffer.clear();
                return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
            }
        }
    }

    protected int initChnlRejectSession(Error error) throws IOException
    {
        _scktChannel.write(_ipcProtocol.encodeConnectionNak(_initChnlWriteBuffer));
        error.channel(this);
        error.errorId(TransportReturnCodes.FAILURE);
        error.sysError(0);

        if (_providerHelper != null)
            _providerHelper.initChnlRejectSession(error);
        else
            _scktChannel.socket().close();

        _initChnlState = InitChnlState.INACTIVE;

        return TransportReturnCodes.SUCCESS;
    }

    protected int initChnlReadClientKey(InProgInfo inProg, Error error)
    {
        int cc;

        try
        {
            /* This is intended to read 14 bytes in a single read.
             * The message it should get is 4 or 12 bytes. */

            cc = initChnlReadFromChannelProvider(_initChnlReadClientKeyBuffer, error);
            if (cc > 0)
            {
                _totalBytesRead += cc;

                if (_providerHelper == null || !_providerHelper._wininetStream)
                {
                    int retVal = 0;
                    retVal = initChnlProcessClientKey(inProg, error, cc);
                    // In case we read more than the message, put it in the input buffer.
                    // Set shared key - client side would be calculated here.
                    _shared_key = _ipcProtocol.protocolOptions()._shared_key;
                    if (cc > _initChnlReadClientKeyBuffer.position())
                    {
                        int putback = cc - _initChnlReadClientKeyBuffer.position();
                        byte[] tempBuf = new byte[putback];
                        _initChnlReadClientKeyBuffer.get(tempBuf);
                        _readIoBuffer.buffer().clear();
                        _readIoBuffer.buffer().put(tempBuf);
                    }
                    return retVal;
                }
                else
                {
                    return TransportReturnCodes.SUCCESS;
                }
            }
            else if (cc == 0)
            {
                /* This will happen when there is no data at all to be read, and the read fails with error _IPC_WOULD_BLOCK or EINTR */
                return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
            }
            else if (_initChnlState == InitChnlState.RECONNECTING)
            {
                /* This happens when the far end (server) closes the connection and we need to try another protocol. */
                _shared_key = 0;
                return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
            }
            else
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                if (_providerHelper == null || _providerHelper._wininetControl)
                    error.text("Could not read IPC Mount Request");
                if (_providerHelper != null && _providerHelper._wininetControl)
                {
                    if (RsslHttpSocketChannelProvider.debugPrint)
                        System.out.println(" Winnet Control Channel Error..");
                    _providerHelper.closeStreamingSocket();
                }

                return TransportReturnCodes.FAILURE;
            }
        }
        catch (Exception e)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            if (_providerHelper != null && _providerHelper._wininetControl)
                _providerHelper.closeStreamingSocket();
            error.text(e.getLocalizedMessage());
            return TransportReturnCodes.FAILURE;
        }
    }

    protected int initChnlProcessClientKey(InProgInfo inProg, Error error, int cc)
            throws IndexOutOfBoundsException, IOException
    {
        int length = 0;
        @SuppressWarnings("unused")
        int opCode = 0;
        if (cc < 4)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Invalid mount request size <" + cc + ">");
            return TransportReturnCodes.FAILURE;
        }

        int retval = _ipcProtocol.decodeClientKey(_initChnlReadClientKeyBuffer, length, error);
        if (retval != TransportReturnCodes.SUCCESS)
            return retval;

        _initChnlState = InitChnlState.ACTIVE;
        _state = ChannelState.ACTIVE;
        _readBufStateMachine.ripcVersion(_ipcProtocol.ripcVersion());

        return TransportReturnCodes.SUCCESS;
    }

    ByteBufferPair acquirePair(int length)
    {
        ByteBufferPair pair = (ByteBufferPair)_bufferPool.poll(length, false);
        pair.buffer().limit(length);
        pair.buffer().position(0);
        return pair;
    }

    void releasePair(ByteBufferPair pair)
    {
        pair.returnToPool();
    }

    protected void flushOrder(String flushStrategy)
    {
        _channelInfo._priorityFlushStrategy = flushStrategy;
        for (int i = 0; i < flushStrategy.length(); i++)
        {
            if (flushStrategy.charAt(i) == 'H')
            {
                _flushOrder[i] = _highPriorityQueue;
            }
            else if (flushStrategy.charAt(i) == 'M')
            {
                _flushOrder[i] = _mediumPriorityQueue;
            }
            else if (flushStrategy.charAt(i) == 'L')
            {
                _flushOrder[i] = _lowPriorityQueue;
            }
        }
    }

    private void addToPriorityQueue(TransportBufferImpl buffer, int priority)
    {
        switch (priority)
        {
            case WritePriorities.HIGH:
                _highPriorityQueue.add(buffer);
                break;
            case WritePriorities.MEDIUM:
                _mediumPriorityQueue.add(buffer);
                break;
            case WritePriorities.LOW:
                _lowPriorityQueue.add(buffer);
                break;
            default:
                break;
        }
    }

    protected void lockReadWriteLocks()
    {
        _readLock.lock();
        _writeLock.lock();
    }

    protected void unlockReadWriteLocks()
    {
        _readLock.unlock();
        _writeLock.unlock();
    }

    void socketBufferToRecycle(SocketBuffer buffer)
    {
        if (buffer != _currentBuffer)
        {
            buffer.returnToPool();
            if (_used > 0)
            {
                --_used;
            }
        }
    }

    public int reconnectClient(Error error)
    {
        assert (error != null) : "error cannot be null";

        error.channel(this);
        error.errorId(TransportReturnCodes.FAILURE);
        error.sysError(0);
        error.text("channel does not support reconnectClient()");

        return TransportReturnCodes.FAILURE;
    }

    public boolean canReuseSessionId(Integer id)
    {
        if (RsslHttpSocketChannelProvider._sessionIdSocketMap == null)
            return true;
        if (RsslHttpSocketChannelProvider._sessionIdSocketMap.containsKey(id))
        {
            return false;
        }
        else
            return true;
    }
    
    /* Write CONNECT HTTP to proxy with no credentials (first attempt, i.e. initiating authentication).
     * Called when in the init state PROXY_CONNECTING.
     */
    int initChnlProxyConnecting() throws IOException
    {
        if (_readIoBuffer != null)
            _readIoBuffer.buffer().clear(); // needed for recovery through a proxy

        if (_proxyAuthenticator == null)
            _proxyAuthenticator = ProxyAuthenticatorFactory.create(_proxyCredentails, _httpProxyHost); // needed for recovery through a proxy
        httpOKretry = 0; // needed for recovery through a proxy

        String connectRequest = buildHttpConnectRequest();
        if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
            System.out.println(connectRequest);
        /* Perform a direct write here, because we will not have any encryption setup at this point */
        _scktChannel._socket.write(ByteBuffer.wrap((connectRequest.toString()).getBytes(CHAR_ENCODING)));

        _initChnlState = InitChnlState.CLIENT_WAIT_PROXY_ACK;

        return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
    }
    
    /* Build the HTTP CONNECT message with possible credentials. */
    protected String buildHttpConnectRequest()
    {
        StringBuilder connectRequest = new StringBuilder();
        connectRequest.append(buildHttpConnectRequestPrefix());

        // add any addtional connect parameters that may be required for authentication
        if (_additionalHttpConnectParams != null && _additionalHttpConnectParams.length() > 0)
        {
            connectRequest.append(_additionalHttpConnectParams);
            _additionalHttpConnectParams = null;
        }

        connectRequest.append(EOL);

        return connectRequest.toString();
    }

    /* Returns a string containing the "common" HTTP Connect request.
     * A suffix (a trailing \r\n) must be appended to the returned value
     * (to make it a valid HTTP request).
     */
    private final String buildHttpConnectRequestPrefix()
    {
        StringBuilder sb = new StringBuilder();

        sb.append("CONNECT ");
        sb.append(_host);
        sb.append(":");
        sb.append(_port);
        sb.append(" HTTP/1.1\r\n");
        sb.append(USER_AGENT);
        sb.append(PROXY_CONNECTION_KEEP_ALIVE);
        sb.append("Content-Length: 0\r\n");
        sb.append("Host: ");
        sb.append(_host);
        sb.append(":");
        sb.append(_port);
        sb.append("\r\n");
        sb.append(PRAGMA_NO_CACHE);

        // NOTE: this will *not* be a valid HTTP request until a final "\r\n" is appended to it

        return sb.toString();
    }

    /* Handle proxy reply to HTTP CONNECT and do any necessary proxy authentication. */
    protected int initChnlWaitProxyAck(InProgInfo inProg, Error error) throws IOException, ProxyAuthenticationException
    {
        int cc;

        cc = initProxyChnlReadFromChannel(_initChnlReadBuffer, error);
        // System.out.println(Transport.toHexString(_initChnlReadBuffer, 0, _initChnlReadBuffer.position()));

        if (cc == TransportReturnCodes.FAILURE)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Could not read HTTP OK (reply to HTTP CONNECT)");
            return TransportReturnCodes.FAILURE;
        }

        _totalBytesRead += cc;

        // do authentication handling
        readHttpConnectResponse(_initChnlReadBuffer, inProg, error);
        _initChnlReadBuffer.clear();

        return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
    }

    /* Read response message to HTTP CONNECT.
     * Do necessary proxy authentication.
     */
    protected void readHttpConnectResponse(ByteBuffer reader, InProgInfo inProg, Error error) throws ProxyAuthenticationException, IOException
    {
        int responseSize = reader.position();
        int bufferIndex = 0;
        reader.position(bufferIndex);

        try
        {
            byte[] tempBuf = new byte[responseSize];
            reader.get(tempBuf, 0, responseSize);
            String response = new String(tempBuf);

            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
            {
                //for debugging
                System.out.println("-- begin response -- ");
                System.out.println(response);
                System.out.println("-- end response --");
            }
            _proxyConnectResponse.append(response); // used to combine "incomplete" responses from the proxy

            if (!_proxyAuthenticator.isAuthenticated() && _proxyConnectResponse.toString().contains(END_OF_RESPONSE))
            {
                // process the response to determine whether or not we are authenticated
                IProxyAuthenticatorResponse authenticatorResponse = null;
                try
                {
                    authenticatorResponse = _proxyAuthenticator.processResponse(_proxyConnectResponse.toString());
                }
                catch (ResponseCodeException e)
                {
                    // the response from the proxy may, for example, contain an HTML error message intended for uses that we should ignore
                    ++_ignoredConnectResponses;

                    if (_ignoredConnectResponses < MAX_IGNORED_RESPONSES)
                    {
                        if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                            System.out.println(String.format("Ignoring a response from the proxy that did not contain a response code (%d/%d)",
                                                             _ignoredConnectResponses, MAX_IGNORED_RESPONSES));
                        return;
                    }
                    else
                    {
                        throw e; // too many ignored responses
                    }
                }

                if (_proxyAuthenticator.isAuthenticated())
                {
                    if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                        System.out.println("Connection established to proxy " + _httpProxyHost + ":" + _httpProxyPort);
                }
                else
                {
                    // this will be added to the next http CONNECT message:
                    _additionalHttpConnectParams = authenticatorResponse.getProxyAuthorization();

                    if (authenticatorResponse.isProxyConnectionClose())
                    {
                        if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                            System.out.println("*** The proxy requested a close (during authentication). Reconnecting. ***\n");
                        forceReconnect(inProg, error);
                    }
                    else
                    {
                        // write http CONNECT message
                        String connectRequest = buildHttpConnectRequest();
                        if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                            System.out.println(connectRequest);
                        /* Write directly to the socket instead of the channel to avoid encryption */
                        _scktChannel._socket.write(ByteBuffer.wrap((connectRequest.toString()).getBytes(CHAR_ENCODING)));
                    }
                }

                _proxyConnectResponse.setLength(0); // we are done with the current response from the proxy
            }
        }
        catch (ProxyAuthenticationException e)
        {
            // for extra debugging
            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                System.out.println("ProxyAuthenticationException in readCONNECThttpResponse: " + e.getMessage());
            throw e;
        }
        catch (IOException e)
        {
            // for extra debugging
            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                System.out.println("IOException in readCONNECThttpResponse: " + e.getMessage());
            throw e;
        }
    }

    /* Read a response from the Proxy Server into BytBuffer dst. */
    protected int initProxyChnlReadFromChannel(ByteBuffer dst, Error error) throws IOException
    {
        dst.clear(); // needed for recovery through a proxy
        /* Proxy interactions are not encrypted, so we will just do a direct read on the _socket */
        int bytesRead = _scktChannel._socket.read(dst);
        // System.out.println("RsslHttpSocketChannel::initProxyChnlReadFromChannel(ByteBuffer dst, Error error)    bytesRead=="+bytesRead);

        if (bytesRead > 0)
        {
            // note that we could cache the msgLen, but normally we should be reading an entire ConnectAck/ConnectNak here.

            if (dst.position() > 2)
            {
                int messageLength = (dst.getShort(0) & 0xFF);
                if (dst.position() >= messageLength)
                {
                    // we have at least one complete message
                    return dst.position();
                }
            }
        }
        else if (bytesRead == -1)
        {
            if (_readIoBuffer != null)
                _readIoBuffer.buffer().clear();
            _proxyAuthenticator = null;
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Proxy has cut the connection.");
            return TransportReturnCodes.FAILURE;
        }

        // we don't have a complete message, or no bytes were read.
        return 0;
    }
    
    /* Reconnect to proxy because of authenticatorResponse.isProxyConnectionClose() */
    private void forceReconnect(InProgInfo inProg, Error error)
    {
        // close current _scktChannel
        try
        {
            _scktChannel.close();
        }
        catch (IOException e)
        {
            System.err.println("IOException forceReconnect(): " + e.getMessage());
        }

        ((InProgInfoImpl)inProg).oldSelectableChannel(_scktChannel.getSocketChannel());

        _ipcProtocol = null;
        this.connect(_cachedConnectOptions, error);

        ((InProgInfoImpl)inProg).flags(InProgFlags.SCKT_CHNL_CHANGE);
        ((InProgInfoImpl)inProg).newSelectableChannel(_scktChannel.getSocketChannel());
    }
    
    /* Store Credentials (needed for Proxy Server authentication) */
    protected ICredentials readProxyCredentails(ConnectOptions connectOptions)
    {
        ICredentials credentails = CredentialsFactory.create();

        if (connectOptions.credentialsInfo().HTTPproxyDomain() != null
                && !connectOptions.credentialsInfo().HTTPproxyDomain().isEmpty())
        {
            credentails.set(CredentialName.DOMAIN, connectOptions.credentialsInfo().HTTPproxyDomain());
        }

        if (connectOptions.credentialsInfo().HTTPproxyUsername() != null
                && !connectOptions.credentialsInfo().HTTPproxyUsername().isEmpty())
        {
            credentails.set(CredentialName.USERNAME, connectOptions.credentialsInfo().HTTPproxyUsername());
        }

        if (connectOptions.credentialsInfo().HTTPproxyPasswd() != null
                && !connectOptions.credentialsInfo().HTTPproxyPasswd().isEmpty())
        {
            credentails.set(CredentialName.PASSWORD, connectOptions.credentialsInfo().HTTPproxyPasswd());
        }

        if (connectOptions.credentialsInfo().HTTPproxyLocalHostname() != null
                && !connectOptions.credentialsInfo().HTTPproxyLocalHostname().isEmpty())
        {
            credentails.set(CredentialName.LOCAL_HOSTNAME, connectOptions.credentialsInfo().HTTPproxyLocalHostname());
        }

        if (connectOptions.credentialsInfo().HTTPproxyKRB5configFile() != null
                && !connectOptions.credentialsInfo().HTTPproxyKRB5configFile().isEmpty())
        {
            credentails.set(CredentialName.KRB5_CONFIG_FILE, connectOptions.credentialsInfo().HTTPproxyKRB5configFile());
        }

        return credentails;
    }
}
