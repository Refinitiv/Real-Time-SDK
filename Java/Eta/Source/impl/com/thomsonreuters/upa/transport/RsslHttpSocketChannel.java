package com.thomsonreuters.upa.transport;

import java.io.IOException;
import java.lang.management.ManagementFactory;
import java.net.InetAddress;
import java.net.InetSocketAddress;
import java.nio.ByteBuffer;
import com.thomsonreuters.proxy.authentication.CredentialName;
import com.thomsonreuters.proxy.authentication.CredentialsFactory;
import com.thomsonreuters.proxy.authentication.ICredentials;
import com.thomsonreuters.proxy.authentication.IProxyAuthenticator;
import com.thomsonreuters.proxy.authentication.IProxyAuthenticatorResponse;
import com.thomsonreuters.proxy.authentication.ProxyAuthenticationException;
import com.thomsonreuters.proxy.authentication.ProxyAuthenticatorFactory;
import com.thomsonreuters.proxy.authentication.ResponseCodeException;

// Init state transitions for consumer with http (or encrypted) connection going through a proxy server:
//
//                            +-------------------------+
//                            |     PROXY_CONNECTING    | 
//                            |         (start)         | <.....+
//                            +-------------------------+       .
// send HTTP CONNECT                       |                    .
// (initChnlProxyConnecting())             |                    .
//                                         |                    .  if authentication process
//                                         |                    .  created new connection
//                                         v                    .
//                            +-------------------------+       .
//                            |  CLIENT_WAIT_PROXY_ACK  | ......+
//                            |    (authentication)     |
//                            +-------------------------+
// proxy conn. established                 |
// so send HTTP POST                       |
// (initChnlWaitProxyAck(inProg, error))   |
//                                         |
//                                         v
//                            +-------------------------+
//                            |  CLIENT_WAIT_HTTP_ACK   |
//                            |  (waiting for HTTP OK)  |
//                            +-------------------------+
// received HTTP OK                        |
// (initChnlWaitHttpAck(error))            |
// so send RIPC connect                    |
// (initChnlSendConnectReq(inProg, error)) |
//                                         |
//                                         v
//                            +--------------------------+
//                            |        WAIT_ACK          |
//                            | (waiting for RIPC reply) |
//                            +--------------------------+
// received RIPC reply                     |
// so go to ACTIVE                         |
//                                         |
//                                         |
//                                         v
//                          +-------------------------------+
//                          |            ACTIVE             |
//                          | (ready to send login request) |
//                          +-------------------------------+


// Init state transitions for consumer with http connection direct (no proxy server):
//
//                            +-------------------------+
//                            |     HTTP_CONNECTING     | 
//                            |         (start)         |
//                            +-------------------------+
// send HTTP POST                          |
// (initChnlHttpConnecting())              |
//                                         |
//                                         |
//                                         v
//                            +-------------------------+
//                            |  CLIENT_WAIT_HTTP_ACK   |
//                            |  (waiting for HTTP OK)  |
//                            +-------------------------+
// received HTTP OK                        |
// (initChnlWaitHttpAck(error))            |
// so send RIPC connect                    |
// (initChnlSendConnectReq(inProg, error)) |
//                                         |
//                                         v
//                            +--------------------------+
//                            |        WAIT_ACK          |
//                            | (waiting for RIPC reply) |
//                            +--------------------------+
// received RIPC reply                     |
// so go to ACTIVE                         |
//                                         |
//                                         |
//                                         v
//                          +-------------------------------+
//                          |            ACTIVE             |
//                          | (ready to send login request) |
//                          +-------------------------------+


class RsslHttpSocketChannel extends RsslSocketChannel
{
    protected boolean _httpProxy = false;
    String _httpProxyHost = null;
    protected int _httpProxyPort = 0;
    protected String _objectName = null;

    protected int HTTP_HEADER3 = 0;          // 1st HTTP chunk header size (value=3 when http)
    protected int HTTP_HEADER_END_SIZE = 4;  // HTTP header end size, i.e. /r/n/r/n (value = 4)
    byte[] _pidBytes;              // store process ID
    byte[] _ipAddressBytes;        // store ip address
    byte[] _httpSessionIDbytes;    // store http session ID
    byte[] HTTP_OK_FROM_SERVER;    // store first chunk of received HTTP OK message
    byte[] HTTP_TransferEncoding;  // store "Transfer Encoding" chunk of received HTTP OK message
    byte[] HTTP_ContentType;       // store "Content Type" chunk of received HTTP OK message

    protected static final String CHAR_ENCODING = "US-ASCII";
    protected static final String USER_AGENT = "User-Agent: UPA/Java\r\n";
    protected static final String PROXY_CONNECTION_KEEP_ALIVE = "Proxy-Connection: Keep-Alive\r\n";
    protected static final String PRAGMA_NO_CACHE = "Pragma: no-cache\r\n";
    protected static final String EOL = "\r\n";
    ByteBuffer _httpPOSTwriteBuffer = ByteBuffer.allocateDirect(DEFAULT_HIGH_WATER_MARK);
    protected int _httpOpCode = 0x80;  // OpCode for HTTP POST request from consumer
    protected int _httpSessionID = 0;  // HTTP session ID (returned to consumer in response to HTTP POST request)
    int httpOKretry = 0;  // needed for recovery through a proxy
    protected int httpOKsize;
    protected int firstHTTPchunkHeader_size;
    protected int firstHTTPchunk_size = 7;

    protected long _bytesWritten = 0;
    long _totalWriteTunnelCount = 0;
    static final int MAX_BYTES_WRITTEN_PER_CONNECTION = Integer.MAX_VALUE - 10000;

    // when reconnectClient() called or when reaching MAX_BYTES_WRITTEN_PER_CONNECTION, go into httpReconnectState
    boolean _httpReconnectState = false;
    protected int _httpReconnectOpCode = 0x84;  // OpCode for HTTP POST Reconnect request from consumer
    protected int HTTP_RECONNECT_ACK_SIZE = 6;  // Reconnect ACK is 6 bytes: 0x31 0x0D 0x0A 0x03 0x0D 0x0A
    boolean _httpReconnectProxyActive = false;
    boolean rcvACKnewChannel = false;
    boolean readAllRemainingOldSocketChannel = false;
    boolean rcvEndOfResponseOldChannel = false;  // received 0x30 0x0d 0x0A 0x0D 0x0A ?
    CryptoHelper _oldCrypto = null;
    private static final int CRLFCRLF = 0x0D0A0D0A;  // value of bytes for /r/n/r/n
    private long endOfOldSocketMessage = 0;

    protected boolean releaseOldCrypto = false;

    // for proxy authentication
    protected ICredentials _proxyCredentails = null;
    protected IProxyAuthenticator _proxyAuthenticator = null;
    protected String _additionalHttpConnectParams = null;
    String db;  // debug env variable

    RsslHttpSocketChannel(SocketProtocol transport, Pool channelPool)
    {
        super(transport, channelPool);

        _http = true;
        HTTP_HEADER4 = 4;
        HTTP_HEADER3 = 3;
        CHUNKEND_SIZE = 2;

        setPIDstructs();
        _httpSessionIDbytes = new byte[4];
        HTTP_OK_FROM_SERVER = new byte[15];
        HTTP_TransferEncoding = new byte[26];
        HTTP_ContentType = new byte[38];

        _internalMaxFragmentSize = (6 * 1024) + RIPC_HDR_SIZE;
    }

    /* TEST ONLY: This is not a valid constructor. (only for JUnit tests) */
    protected RsslHttpSocketChannel()
    {
        _transport = null;
        _pool = null;
        _ipcProtocol = null;

        /* _maxFragmentSize (which we return to the user) is RIPC
         * MAX_USER_MSG_SIZE - RIPC PACKED_HDR_SIZE. Buffer sizes will be
         * allocated at _internalMaxFragmentSize as RIPC MAX_USER_MSG_SIZE + RIPC_HDR_SIZE. */
        _channelInfo._maxFragmentSize = (6 * 1024) - RIPC_PACKED_HDR_SIZE;
        _internalMaxFragmentSize = (6 * 1024) + RIPC_HDR_SIZE;
        _appReadBuffer = new TransportBufferImpl(_internalMaxFragmentSize);

        // initialize _priorityFlushStrategy
        flushOrder(DEFAULT_PRIORITY_FLUSH_ORDER);

        // initialize _highWaterMark
        _highWaterMark = DEFAULT_HIGH_WATER_MARK;

        _readLock = new ReentrantLock();
        _writeLock = new ReentrantLock();

        _readBufStateMachine = new ReadBufferStateMachineHTTP(this);

        _http = true;
        HTTP_HEADER4 = 4;
        HTTP_HEADER3 = 3;
        CHUNKEND_SIZE = 2;

        setPIDstructs();
        _httpSessionIDbytes = new byte[4];
        HTTP_OK_FROM_SERVER = new byte[15];
        HTTP_TransferEncoding = new byte[26];
        HTTP_ContentType = new byte[38];
    }
    
    /* TEST ONLY: This is not a valid constructor. (only for JUnit tests) */
    protected RsslHttpSocketChannel(int connectionType)
    {
        _transport = null;
        _pool = null;
        _ipcProtocol = null;

        /* _maxFragmentSize (which we return to the user) is RIPC
         * MAX_USER_MSG_SIZE - RIPC PACKED_HDR_SIZE. Buffer sizes will be
         * allocated at _internalMaxFragmentSize as RIPC MAX_USER_MSG_SIZE + RIPC_HDR_SIZE. */
        _channelInfo._maxFragmentSize = (6 * 1024) - RIPC_PACKED_HDR_SIZE;
        _internalMaxFragmentSize = (6 * 1024) + RIPC_HDR_SIZE;
        _appReadBuffer = new TransportBufferImpl(_internalMaxFragmentSize);

        // initialize _priorityFlushStrategy
        flushOrder(DEFAULT_PRIORITY_FLUSH_ORDER);

        // initialize _highWaterMark
        _highWaterMark = DEFAULT_HIGH_WATER_MARK;

        _readLock = new ReentrantLock();
        _writeLock = new ReentrantLock();

        _readBufStateMachine = new ReadBufferStateMachineHTTP(this);

        _http = true;
        HTTP_HEADER4 = 4;
        HTTP_HEADER3 = 3;
        CHUNKEND_SIZE = 2;

        setPIDstructs();
        _httpSessionIDbytes = new byte[4];
        HTTP_OK_FROM_SERVER = new byte[15];
        HTTP_TransferEncoding = new byte[26];
        HTTP_ContentType = new byte[38];

        if (connectionType == ConnectionTypes.ENCRYPTED)
            _encrypted = true;
    }

    @Override
    public int connectionType()
    {
        return ConnectionTypes.HTTP;
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
            dataFromOptions(opts);
        }

        // the first time, _ipcProtocol will be null prior to nextProtocol call.
        // If nextProtocol returns null, don't override _ipcProtocool, otherwise
        // the next call to nextProtocol will return the newest protocol.
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
            _scktChannel = java.nio.channels.SocketChannel.open();
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

                // create this the 1st time
                if (_cachedBindInetSocketAddress == null)
                {
                    _cachedBindInetSocketAddress = new InetSocketAddress(interfaceName, 0);
                }

                _scktChannel.bind(_cachedBindInetSocketAddress);
            }

            if (_cachedConnectOptions.tcpOpts().tcpNoDelay())
            {
                _scktChannel.socket().setTcpNoDelay(true);
            }

            // connect
            _scktChannel.connect(_cachedInetSocketAddress);

            if (_httpProxy)
                _initChnlState = InitChnlState.PROXY_CONNECTING;
            else
            {
                _initChnlState = InitChnlState.HTTP_CONNECTING;
                try
                {
                    initializeEngine();
                }
                catch (IOException e)
                {
                    if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                        System.out.println("IOException initializeTLS: " + e.getMessage());
                }
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
                        if (ret == TransportReturnCodes.FAILURE && _initChnlState == InitChnlState.RECONNECTING)
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
                                    _initChnlState = InitChnlState.HTTP_CONNECTING;
                                    try
                                    {
                                        initializeEngine();
                                    }
                                    catch (IOException e)
                                    {
                                        if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                                            System.out.println("IOException initializeTLS: " + e.getMessage());
                                    }
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
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getLocalizedMessage());
            ret = TransportReturnCodes.FAILURE;
        }
        return ret;
    }

    public int ping(Error error)
    {
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
                if (retVal < TransportReturnCodes.SUCCESS)
                    return retVal;
            }
            else
            // send ping buffer
            {
                _pingBuffer.rewind();

                if (!_encrypted)
                {
                    int numBytes = _scktChannel.write(_pingBuffer);
                    retVal = RIPC_HDR_SIZE - numBytes;
                }
                else
                {
                    _crypto.write(_pingBuffer);
                    retVal = 0;
                }
            }
        }
        catch (Exception e)
        {
            _state = ChannelState.CLOSED;
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getLocalizedMessage());
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            _writeLock.unlock();
        }

        // check for doing http reconnect
        if ((_server == null) && (_totalWriteTunnelCount > MAX_BYTES_WRITTEN_PER_CONNECTION))
            reconnectClient(error);

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

        _httpProxy = opts.tunnelingInfo().HTTPproxy();
        if (_httpProxy)
        {
            _httpProxyHost = opts.tunnelingInfo().HTTPproxyHostName();
            _httpProxyPort = opts.tunnelingInfo().HTTPproxyPort();
        }
        _objectName = opts.tunnelingInfo().objectName();
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

    public int init(InProgInfo inProg, Error error)
    {
        assert (inProg != null) : "inProg cannot be null";
        assert (error != null) : "error cannot be null";

        int retVal = TransportReturnCodes.FAILURE;
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
                case InitChnlState.WAIT_ACK:
                    retVal = initChnlWaitConnectAck(inProg, error);
                    if (_initChnlState == InitChnlState.RECONNECTING && !_cachedConnectOptions.blocking())
                    {
                        // The far end closed connection. Try another protocol.
                        ((InProgInfoImpl)inProg).oldSelectableChannel(_scktChannel);
                        if (connect(_cachedConnectOptions, error) == TransportReturnCodes.SUCCESS)
                        {
                            if (_httpProxy)
                                _initChnlState = InitChnlState.PROXY_CONNECTING;
                            else
                                _initChnlState = InitChnlState.HTTP_CONNECTING;

                            ((InProgInfoImpl)inProg).flags(InProgFlags.SCKT_CHNL_CHANGE);
                            ((InProgInfoImpl)inProg).newSelectableChannel(_scktChannel);
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
                case InitChnlState.READ_HDR:
                    retVal = initChnlReadHdr(inProg, error);
                    break;
                case InitChnlState.HTTP_CONNECTING:
                    retVal = initChnlHttpConnecting();
                    break;
                case InitChnlState.PROXY_CONNECTING:
                    retVal = initChnlProxyConnecting();
                    break;
                case InitChnlState.CLIENT_WAIT_PROXY_ACK:
                    retVal = initChnlWaitProxyAck(inProg, error); // may throw a ProxyAuthenticationException
                    break;
                case InitChnlState.CLIENT_WAIT_HTTP_ACK:
                    retVal = initChnlWaitHttpAck(error); // will return 0 if didn't get all HTTP OK bytes
                    if (retVal > TransportReturnCodes.SUCCESS)
                        retVal = initChnlSendConnectReq(inProg, error);
                    else if (retVal == 0)
                        return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
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
            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            unlockReadWriteLocks();
        }

        return retVal;
    }

    protected void initializeEngine() throws IOException
    {
        // no further initialization needed for RsslHttpSocketChannel
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

        _scktChannel.write(ByteBuffer.wrap((connectRequest.toString()).getBytes(CHAR_ENCODING)));

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

        if (_proxyAuthenticator.isAuthenticated())
        {
            _httpPOSTwriteBuffer = setupClientPOSThttpRequest(_initChnlWriteBuffer);
            if (_httpPOSTwriteBuffer == null)
                return TransportReturnCodes.FAILURE;

            try
            {
                _scktChannel.write(_httpPOSTwriteBuffer);
            }
            catch (Exception e)
            {
                return TransportReturnCodes.FAILURE;
            }

            _initChnlWriteBuffer.clear();

            _initChnlState = InitChnlState.CLIENT_WAIT_HTTP_ACK;
        }

        return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
    }

    /* Build client POST HTTP POSTrequest */
    protected ByteBuffer setupClientPOSThttpRequest(ByteBuffer buf)
    {
        assert (buf != null);

        buf.clear();

        StringBuffer httpPostRequest = null;
        if (_objectName.equals(""))
            httpPostRequest = new StringBuffer("POST / HTTP/1.1\r\n");
        else
            httpPostRequest = new StringBuffer("POST /" + _objectName + " HTTP/1.1\r\n");
        httpPostRequest.append("Accept: application/octet-stream\r\n");
        httpPostRequest.append(USER_AGENT);
        httpPostRequest.append("Host: " + _host + ":" + _port + "\r\n");
        httpPostRequest.append("Content-Length: ");
        httpPostRequest.append(MAX_BYTES_WRITTEN_PER_CONNECTION);
        httpPostRequest.append(EOL);
        httpPostRequest.append("Cache-Control: no-cache\r\n");
        httpPostRequest.append("\r\n");

        byte[] data = { (byte)0x00, (byte)13, // 2-byte length (=13 bytes)
                (byte)_httpOpCode,            // 1-byte opCode
                (byte)0x00, (byte)0x00, (byte)0x00, (byte)0x00, // 4-byte HTTP sessionId
                _pidBytes[0], _pidBytes[1],   // 2-byte processId
                _ipAddressBytes[3], _ipAddressBytes[2], _ipAddressBytes[1], _ipAddressBytes[0] // 4-byte IPaddress
        };
        try
        {
            // add the HTTP POST request header
            buf.put((httpPostRequest.toString()).getBytes("US-ASCII"));

            buf.put(data);
            buf.flip(); // ready for reading
        }
        catch (IOException e)
        {
            return null;
        }

        return buf;
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

                    // possible extra initialization (none for HTTP, initaializeTLS for Encrypted)
                    initializeEngine();
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
                        _scktChannel.write(ByteBuffer.wrap((connectRequest.toString()).getBytes(CHAR_ENCODING)));
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
        int bytesRead = _scktChannel.read(dst);
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
            close(error);

            // The connection was closed by far end (proxy server). Need to try again?
            _initChnlState = InitChnlState.RECONNECTING;
            return -1;
        }

        // we don't have a complete message, or no bytes were read.
        return 0;
    }

    /* Read "HTTP OK" response to "POST HTTP".
     * Called when in the init state CLIENT_WAIT_HTTP_ACK.
     */
    private int initChnlWaitHttpAck(Error error) throws IOException
    {
        int cc = 0;
        httpOKretry = 0;
        int httpOKretryMacCount = 10000;
        // try up to httpOKretryMacCount times to get all expected bytes of "HTTP OK" response to "POST HTTP" are available
        while (httpOKretry <= httpOKretryMacCount)
        {
            // initChnlReadFromChannelHTTPok will return 0 if not all expected bytes of "HTTP OK" response to "POST HTTP" are available
            cc = initChnlReadFromChannelHTTPok(_initChnlReadBuffer, error);

            if (cc == 0)
                httpOKretry++; // didn't get all expected bytes of "HTTP OK" response to "POST HTTP"
            else
                break; // got all expected bytes of "HTTP OK" response to "POST HTTP"
        }

        if (cc == TransportReturnCodes.FAILURE)
        {
            httpOKretry = 0;
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Could not read POST HTTP OK");
            return TransportReturnCodes.FAILURE;
        }

        _totalBytesRead += cc;
        return cc;
    }

    /* Read "HTTP OK" response to "POST HTTP" into ByteBuffer dst. */
    private int initChnlReadFromChannelHTTPok(ByteBuffer dst, Error error) throws IOException
    {
        int bytesRead = read(dst);
        // System.out.println(Transport.toHexString(dst, 0, dst.position()));

        if (bytesRead > 0)
        {
            // note that we could cache the msgLen, but normally we should be reading an entire HTTP OK here.

            httpOKsize = HTTP_OK_FROM_SERVER.length + CHUNKEND_SIZE + HTTP_TransferEncoding.length + CHUNKEND_SIZE + HTTP_ContentType.length
                    + HTTP_HEADER_END_SIZE;
            firstHTTPchunkHeader_size = HTTP_HEADER3 + firstHTTPchunk_size + CHUNKEND_SIZE; // e.g. 0x37 0x0D 0x0a 00 07 00 00 00 00 01 0x0D 0x0A
                                                                                            // (in this example chunk size is 07 and http session ID is 01)
            if (dst.position() >= (httpOKsize + firstHTTPchunkHeader_size))
            {
                int retVal = parsePOSThttpResponse(dst);
                if (retVal == TransportReturnCodes.FAILURE)
                {
                    dst.clear();
                    throw new IOException("Unable to tunnel through " + _host + ":" + _port);
                }

                // we have at least one complete message
                return dst.position();
            }
        }
        else if (bytesRead == -1)
        {
            if (_readIoBuffer != null)
                _readIoBuffer.buffer().clear();
            if (_httpProxy)
            {
                _proxyAuthenticator = null;
                close(error);
            }

            // The connection was closed by far end (server).
            _initChnlState = InitChnlState.RECONNECTING;
            return -1;
        }

        // we don't have a complete message, or no bytes were read.
        return 0;
    }

    /* Parse "HTTP OK" response to "POST HTTP" */
    private int parsePOSThttpResponse(ByteBuffer reader)
    {
        int bufferIndex = 0;
        reader.position(bufferIndex);

        reader.get(HTTP_OK_FROM_SERVER, 0, 15);

        bufferIndex += 15; // skip "HTTP/1.1 200 OK"
        String startHTTP_OK_FROM_SERVER = new String(HTTP_OK_FROM_SERVER);
        if (!(startHTTP_OK_FROM_SERVER.contains("200") || startHTTP_OK_FROM_SERVER.contains("OK")))
        {
            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                System.out.println("Unable to tunnel through " + _host + ":" + _port +
                                   ".  HTTP POST response starts with \"" + startHTTP_OK_FROM_SERVER + "\"");
            return TransportReturnCodes.FAILURE;
        }

        bufferIndex += 2; // skip /r/n

        reader.position(bufferIndex);
        reader.get(HTTP_TransferEncoding, 0, 26);
        bufferIndex += 26; // skip "Transfer-Encoding: chunked"

        bufferIndex += 2; // skip /r/n

        reader.position(bufferIndex);
        reader.get(HTTP_ContentType, 0, 38);
        bufferIndex += 38; // skip "Content-Type: application/octet-stream"

        // HTTP headers should end in 0x0D0x0A0x0D0x0A (/r/n/r/n)
        bufferIndex += 4; // skip /r/n/r/n

        bufferIndex += 3; // skip first data chunk header + /r/n (chunkSize + 0x0D + 0x0A)

        bufferIndex += 2; // skip FirstDataChunkSize

        bufferIndex++; // skip outFlags

        _httpSessionID = reader.getInt(bufferIndex); // HTTP Session ID for possible http reconnect

        bufferIndex += 4; // skip HTTP sessionID
        // http reconnect
        // System.out.println("HTTP SessionID (for possible http reconnect) = " + _httpSessionID);
        putInt(_httpSessionIDbytes, 0, _httpSessionID);

        bufferIndex += 2; // skip /r/n

        // put reader position at the end of the response to HTTP POST
        reader.position(bufferIndex);

        return TransportReturnCodes.SUCCESS;
    }

    public TransportBuffer read(ReadArgs readArgs, Error error)
    {
        assert (readArgs != null) : "readArgs cannot be null";
        assert (error != null) : "error cannot be null";

        TransportBuffer data = null; // the data returned to the user
        int returnValue;

        setHTTPHeaders();

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

                if (_httpReconnectState)
                {
                    if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                        System.out.println(" RECON rcvACKnewChannel = " + rcvACKnewChannel + 
                                           " rcvEndOfResponseOldChannel = " + rcvEndOfResponseOldChannel);
                    if (rcvACKnewChannel && rcvEndOfResponseOldChannel)
                    {
                        _httpReconnectState = false;
                        _httpReconnectProxyActive = false;
                        rcvACKnewChannel = false;
                        rcvEndOfResponseOldChannel = false;

                        readAllRemainingOldSocketChannel = false;
                        _oldScktChannel.close();

                        returnValue = TransportReturnCodes.READ_FD_CHANGE;
                        // clear _readIoBuffer so it's ready for the new channel
                        _readIoBuffer.buffer().clear();

                        _readBufStateMachine._state = ReadBufferState.NO_DATA;
                        _readBufStateMachine._subState = ReadBufferSubState.NORMAL;
                        _readBufStateMachine._currentMsgStartPos = 0;
                        _readBufStateMachine._currentMsgRipcLen = -1;
                        _readBufStateMachine._currentMsgRipcFlags = 0;
                        _readBufStateMachine._lastReadPosition = 0;
                        _readBufStateMachine._dataBuffer = null;
                        _readBufStateMachine._dataPosition = 0;
                        _readBufStateMachine._dataLength = 0;
                        // may need to do more for fragmentation
                        _readBufStateMachine._lastReassembledFragmentId = 0;

                        ((ReadArgsImpl)readArgs).readRetVal(returnValue);
                        return data;
                    }
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

                        if (releaseOldCrypto)
                        {
                            // end of the oldcrypto channel
                            returnValue = TransportReturnCodes.READ_FD_CHANGE;
                            _oldScktChannel.close();

                            releaseOldCrypto = false;
                            readAllRemainingOldSocketChannel = false;

                            _httpReconnectState = false;
                            _httpReconnectProxyActive = false;
                            rcvACKnewChannel = false;
                            rcvEndOfResponseOldChannel = false;
                            _oldScktChannel.close();
                        }
                        else
                            returnValue = TransportReturnCodes.READ_WOULD_BLOCK;
                        break;
                    case END_OF_STREAM:
                        if (!_httpReconnectState)
                        {
                            close(error);
                            _state = ChannelState.CLOSED;
                        }
                        if (_httpProxy)
                        {
                            _proxyAuthenticator = null;

                            if (_state != ChannelState.CLOSED)
                            {
                                close(error); // needed for recovery through a proxy
                            }
                        }

                        returnValue = TransportReturnCodes.FAILURE;
                        populateErrorDetails(error, TransportReturnCodes.FAILURE, "Closed channel - read End-Of-Stream");
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
            if (!_httpReconnectState)
                _state = ChannelState.CLOSED;
            if (_httpProxy)
            {
                _proxyAuthenticator = null;
                close(error); // needed for recovery through a proxy
            }

            returnValue = TransportReturnCodes.FAILURE;
            populateErrorDetails(error, TransportReturnCodes.FAILURE, "CompressorException: " + e.getLocalizedMessage());
        }
        catch (Exception e)
        {
            if (!_httpReconnectState)
                _state = ChannelState.CLOSED;
            if (_httpProxy)
            {
                _proxyAuthenticator = null;
                close(error); // needed for recovery through a proxy
            }

            returnValue = TransportReturnCodes.FAILURE;
            populateErrorDetails(error, TransportReturnCodes.FAILURE, e.getLocalizedMessage());
        }
        finally
        {
            _readLock.unlock();
        }
        ((ReadArgsImpl)readArgs).readRetVal(returnValue);

        return data;
    }

    protected long write(ByteBuffer[] srcs, int offset, int length) throws IOException
    {
        _bytesWritten = _scktChannel.write(srcs, offset, length);

        _totalWriteTunnelCount += _bytesWritten;
        return _bytesWritten;
    }

    /* Performs Read IO. Also includes httpReconnectClient logic.
     * (throws IOException if an IO exception occurs)
     */
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
                // read from the channel:
                // note *replace* the call to read() below with a call to readAndPrintForReplay() to collect network replay data (for debugging) only!

                // read all remaining data from _oldCrypto or _oldScktChannel until it's closed
                int bytesRead = 0;
                if (_httpReconnectState)
                {
                    int saveOldScktChannelBytesRead = 0;
                    while (!readAllRemainingOldSocketChannel)
                    {
                        if (_encrypted)
                        {
                            try
                            {
                                // read from _oldCrypto
                                bytesRead = _oldCrypto.read(_readIoBuffer.buffer());
                                saveOldScktChannelBytesRead = saveOldScktChannelBytesRead + bytesRead;
                            }
                            catch (IOException e)
                            {
                                if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                                    System.out.println("oldCrypto exception = " + e.toString());
                                // old channel could encounter exception involving SSL engine
                                // with/without terminal bytes - 5 bytes (end of message or for 0x0 + /n/r/n/r)
                                readAllRemainingOldSocketChannel = true;
                                // only release old channel when all msgs are read
                                releaseOldCrypto = true;
                            }
                        }
                        else
                        {
                            // read from oldScktChannel
                            bytesRead = _oldScktChannel.read(_readIoBuffer.buffer());
                            if (bytesRead > 0)
                            {
                                if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                                    System.out.println("HttpSocketChannel::performReadIO(ReadArgsImpl) in _httpReconnectState  bytesRead==" + bytesRead);
                            }
                            if (bytesRead != -1)
                            {
                                saveOldScktChannelBytesRead = saveOldScktChannelBytesRead + bytesRead;
                            }
                            else
                            {
                                _oldScktChannel.close();
                                readAllRemainingOldSocketChannel = true;
                            }
                        }

                        if (_readIoBuffer.buffer().position() >= 5) // read at least 5 bytes (check for end of message or for 0x0 + /n/r/n)
                        {
                            endOfOldSocketMessage = _readIoBuffer.buffer().getInt(_readIoBuffer.buffer().position() - 4);
                            if (endOfOldSocketMessage < 0)
                            {
                                // convert to unsigned.
                                endOfOldSocketMessage &= 0xFFFFFFFF;
                            }
                            if (endOfOldSocketMessage == CRLFCRLF)
                            {
                                if (_readIoBuffer.buffer().get(_readIoBuffer.buffer().position() - 5) == 0)
                                {
                                    readAllRemainingOldSocketChannel = true;
                                }
                            }
                        }
                    }// end of while
                    bytesRead = saveOldScktChannelBytesRead;
                }
                else
                // not httpReconnectState
                {
                    bytesRead = read(_readIoBuffer.buffer());
                    if (bytesRead == -1)
                    {
                        System.out.println("-1 bytes read");
                    }
                }

//                //bytesRead = readAndPrintForReplay(); // for NetworkReplay replace the above read lines with this one
                
                _readBufStateMachine.advanceOnSocketChannelRead(bytesRead, readArgs);
                break;
            default:
                assert (false); // code should not reach here
                break;
        }
    }

    /* WARNING: Creates Garbage. For debugging only, this method reads data from the network and prints the data as hex
     * (so it can later be played back using NetworkReplay)
     * 
     * Returns the number of bytes read from the network.
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
        _debugOutput.append(_readBufStateMachine.currentRipcMessagePosition() + _readBufStateMachine.HTTP_HEADER6);
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

    /* Write POST message. Called when in the init state HTTP_CONNECTING. */
    int initChnlHttpConnecting() throws IOException
    {
        httpOKretry = 0; // needed for recovery through a proxy

        _httpPOSTwriteBuffer = setupClientPOSThttpRequest(_initChnlWriteBuffer);

        if (_httpPOSTwriteBuffer == null)
            return TransportReturnCodes.FAILURE;
        try
        {
            _scktChannel.write(_httpPOSTwriteBuffer);
        }
        catch (Exception e)
        {
            return TransportReturnCodes.FAILURE;
        }

        _initChnlState = InitChnlState.CLIENT_WAIT_HTTP_ACK;

        return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
    }

    /* Store process ID and IP address.
     * These are needed by the Server to have a unique HTTP Session.
     * They are also needed during the http reconnect process.
     */
    private void setPIDstructs()
    {
        // get process ID into a 2-byte buffer
        String pid = ManagementFactory.getRuntimeMXBean().getName();
        String[] Ids = pid.split("@");

        Long osProcessId = Long.valueOf(Ids[0]);
        // (e.g. in Windows: we get the processID of java.exe or javaw.exe)

        _pidBytes = new byte[2];
        putShort(_pidBytes, 0, osProcessId.shortValue());

        // get IP address into a 4-byte buffer
        try
        {
            _ipAddressBytes = InetAddress.getLocalHost().getAddress();
        }
        catch (IOException e)
        {
        }
    }

    private final static void putShort(byte[] buf, int offset, short v) throws ArrayIndexOutOfBoundsException
    {
        buf[offset++] = (byte)((v >> 8) & 0xFF);
        buf[offset++] = (byte)(v & 0xFF);
    }

    static void putInt(byte[] buf, int offset, int v) throws ArrayIndexOutOfBoundsException
    {
        buf[offset++] = (byte)((v >> 24) & 0xFF);
        buf[offset++] = (byte)((v >> 16) & 0xFF);
        buf[offset++] = (byte)((v >> 8) & 0xFF);
        buf[offset++] = (byte)(v & 0xFF);
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

        ((InProgInfoImpl)inProg).oldSelectableChannel(_scktChannel);

        _ipcProtocol = null;
        this.connect(_cachedConnectOptions, error);

        ((InProgInfoImpl)inProg).flags(InProgFlags.SCKT_CHNL_CHANGE);
        ((InProgInfoImpl)inProg).newSelectableChannel(_scktChannel);
    }

    /* Reconnect and bridge connections.
     * After reconnectClient(Error) is completed, on the next read/performReadIO call
     * we will keep reading from _oldScktChannel until we receive 0x01 0x0D 0x0A 0x0D 0x0A
     * and then set rcvEndOfResponseOldChannel to true and return TransportReturnCodes.READ_FD_CHANGE to the application.
     * The application will then cancel the old channel read select and add the new channel read/write select
     * (e.g. see ChannelSession::readInt(PingHandler, ResponseCallback, Error))
     * and all the next reads will be from the new _scktChannel.
     */
    public int reconnectClient(Error error)
    {
        assert (error != null) : "error cannot be null";

        int retVal = TransportReturnCodes.SUCCESS;

        try
        {
            lockReadWriteLocks();
            if (_httpReconnectState)
                return retVal;

            // System.out.println("before connectHTTPreconnectState, old socketChannel is:" + _oldScktChannel);
            // if(_oldScktChannel != null)
            //   System.out.println("before connectHTTPreconnectState, old socketChannel.hashCode() is:" + _oldScktChannel.hashCode());
            // System.out.println("before connectHTTPreconnectState, new socketChannel is:" + _scktChannel);
            // System.out.println("before connectHTTPreconnectState, new socketChannel.hashCode() is:" + _scktChannel.hashCode());

            // first flush
            int ret = flush(error);
            if (ret < TransportReturnCodes.SUCCESS)
            {
                if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                    System.out.println("http channel flush failed (during HttpSocketChannel::reconnectClient) with returned code: " + ret +
                                       " - " + error.text());
            }
            _oldScktChannel = _scktChannel;

            // open new connection
            connectHTTPreconnectState(_cachedConnectOptions, error);

            // System.out.println("after connectHTTPreconnectState, old socketChannel is:" + _oldScktChannel);
            // System.out.println("after connectHTTPreconnectState, old socketChannel.hashCode() is:" + _oldScktChannel.hashCode());
            // System.out.println("after connectHTTPreconnectState, new socketChannel is:" + _scktChannel);
            // System.out.println("after connectHTTPreconnectState, new socketChannel.hashCode() is:" + _scktChannel.hashCode());

            boolean connected = false;
            while (!connected)
            {
                try
                {
                    if (_scktChannel.finishConnect())
                        connected = true;
                }
                catch (Exception e)
                {
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text(e.getLocalizedMessage());
                    retVal = TransportReturnCodes.FAILURE;
                }
            }

            if (_httpProxy)
            {
                // for the new connection do complete proxy authentication if necessary
                doProxyAuthenticationHTTPreconnectState(error);
            }

            // new connection is ready
            // so send http+13 with _httpReconnectOpCode (64h) on new connection and check back for an ACK
            try
            {
                // System.out.println("HttpSocketChannel  reconnectACK(error)");
                reconnectACK(error);
                if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                    System.out.println("after HttpSocketChannel::httpReconnect(error) new socketChannel is:" + _scktChannel);
            }
            catch (Exception e)
            {
                error.channel(this);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text(e.getLocalizedMessage());
                retVal = TransportReturnCodes.FAILURE;
            }

            _httpReconnectState = true;
            _totalWriteTunnelCount = 0;
        }
        finally
        {
            unlockReadWriteLocks();
        }

        return retVal;
    }

    /* HTTP CONNECT to proxy and complete any necessary proxy authentication. */
    protected void doProxyAuthenticationHTTPreconnectState(Error error)
    {
        try
        {
            // send HTTP CONNECT with no credentials first
            writeHttpConnectRequest_noCredentialsReconnectState();

            // loop here until authenticated
            _initChnlReadBuffer.clear();
            int cc = 0;
            _additionalHttpConnectParams = null;
            _ignoredConnectResponses = 0;
            _proxyConnectResponse = new StringBuilder();

            // new ProxyAuthenticator
            _proxyAuthenticator = ProxyAuthenticatorFactory.create(_proxyCredentails, _httpProxyHost);

            while (!_httpReconnectProxyActive)
            {
                _initChnlReadBuffer.clear();
                cc = initProxyChnlReadFromChannel(_initChnlReadBuffer, error);
                // System.out.println(Transport.toHexString(_initChnlReadBuffer, 0, _initChnlReadBuffer.position()));

                if (cc == TransportReturnCodes.FAILURE)
                {
                    error.channel(this);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Could not read HTTP OK (reply to HTTP CONNECT)");
                }

                _totalBytesRead += cc;

                // do authentication handling
                if (cc != 0)
                {
                    readHttpConnectResponseReconnectState(_initChnlReadBuffer, error);
                }
            }
        }
        catch (IOException e)
        {
            error.channel(this);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getLocalizedMessage());
        }
    }

    /* Send HTTP CONNECT without credentials for httpReconnectState */
    protected int writeHttpConnectRequest_noCredentialsReconnectState() throws IOException
    {
        String connectRequest = buildHttpConnectRequest();
        _scktChannel.write(ByteBuffer.wrap((connectRequest.toString()).getBytes(CHAR_ENCODING)));

        return TransportReturnCodes.SUCCESS;
    }

    /* Authentication handling for httpReconnectState */
    private void readHttpConnectResponseReconnectState(ByteBuffer reader, Error error)
    {
        int responseSize = reader.position();
        int bufferIndex = 0;
        reader.position(bufferIndex);

        try
        {
            byte[] tempBuf = new byte[responseSize];
            reader.get(tempBuf, 0, responseSize);
            String response = new String(tempBuf);
            if (!response.isEmpty())
            {
                if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                {
                    //for debugging
                    System.out.println("-- begin response -- ");
                    System.out.println(response);
                    System.out.println("-- end response --");
                }
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
                    initializeEngine();
                    _httpReconnectProxyActive = true;
                }
                else
                {
                    // this will be added to the next http CONNECT message:
                    _additionalHttpConnectParams = authenticatorResponse.getProxyAuthorization();

                    if (authenticatorResponse.isProxyConnectionClose())
                    {
                        if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                            System.out.println("*** The proxy requested a close (during authentication during httpRecconnectState). Reconnecting. ***\n");
                        forceReconnectInHTTPreconnectState(error);

                        reader.clear();

                        // finish new connection
                        boolean connected = false;
                        while (!connected)
                        {
                            try
                            {
                                if (_scktChannel.finishConnect())
                                    connected = true;
                            }
                            catch (Exception e)
                            {
                                error.channel(this);
                                error.errorId(TransportReturnCodes.FAILURE);
                                error.sysError(0);
                                error.text(e.getLocalizedMessage());
                            }
                        }
                    }

                    // write http CONNECT message
                    String connectRequest = buildHttpConnectRequest();
                    if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                        System.out.println(connectRequest);
                    _scktChannel.write(ByteBuffer.wrap((connectRequest.toString()).getBytes(CHAR_ENCODING)));
                }

                _proxyConnectResponse.setLength(0); // we are done with the current response from the proxy
            }
        }
        catch (ProxyAuthenticationException e)
        {
            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                System.out.println("ProxyAuthenticationException in readCONNECThttpResponse: " + e.getMessage());
        }
        catch (IOException e)
        {
            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                System.out.println("IOException in readCONNECThttpResponse: " + e.getMessage());
        }
    }
    
    /* Reconnect to proxy because we had authenticatorResponse.isProxyConnectionClose() during httpReconnectState.
     * (different from forceReconnect(InProgInfo, Error))
     */
    protected void forceReconnectInHTTPreconnectState(Error error)
    {
        // close current _scktChannel
        try
        {
            _scktChannel.close();
        }
        catch (IOException e)
        {
            System.err.println("IOException forceReconnectInHTTPreconnectState(Error): " + e.getMessage());
        }

        // open new _scktChannel
        connectHTTPreconnectState(_cachedConnectOptions, error);
    }

    /* Connect on new socketChannel for reconnectClient (in _httpReconnectState) */
    int connectHTTPreconnectState(ConnectOptions opts, Error error)
    {
        int ret = TransportReturnCodes.SUCCESS;

        try
        {
            _totalBytesRead = 0;
            _scktChannel = java.nio.channels.SocketChannel.open();

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
                if (_http && _httpProxy)
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

                // create this the 1st time
                if (_cachedBindInetSocketAddress == null)
                {
                    _cachedBindInetSocketAddress = new InetSocketAddress(interfaceName, 0);
                }

                _scktChannel.bind(_cachedBindInetSocketAddress);
            }

            if (_cachedConnectOptions.tcpOpts().tcpNoDelay())
            {
                _scktChannel.socket().setTcpNoDelay(true);
            }

            // connect
            _scktChannel.connect(_cachedInetSocketAddress);

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

        }
        catch (Exception e)
        {
            error.channel(null);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(e.getLocalizedMessage());
            ret = TransportReturnCodes.FAILURE;
        }
        return ret;
    }
    	
    /* Only for httpReconnectState.
     * First handshake on new connection (send HTTP POST ... and get back a Reconnect ACK).
     * The Reconnect ACK is 0x31, 0x0D, 0x0A, 0x03, 0x0D, 0x0A
     */
    private void reconnectACK(Error error) throws IOException
    {
        setupClientPOSThttpRequestReconnectState(_initChnlWriteBuffer);
        boolean connected = false;
        while (!connected)
        {
            if (_scktChannel.finishConnect())
            {
                connected = true;
                // send ClientPOSThttpRequest in ReconnectState
                _scktChannel.write(_initChnlWriteBuffer);
            }
        }

        _initChnlReadBuffer.clear();

        while (_initChnlReadBuffer.position() < HTTP_RECONNECT_ACK_SIZE)
        {
            _scktChannel.read(_initChnlReadBuffer);
        }
        boolean rcvReconnectACK = readHTTPreconnectResponseACK(_initChnlReadBuffer);
        if (rcvReconnectACK)
        {
            rcvACKnewChannel = true;
        }
        else
            throw new IOException("rcvReconnectACK during httpReconnectState==false");
    }
    
    /* Create "POST HTTP" message (with _httpReconnectOpCode) for httpReconnectState */
    protected void setupClientPOSThttpRequestReconnectState(ByteBuffer buf)
    {
        StringBuffer httpPostRequest = null;
        if (_objectName.equals(""))
            httpPostRequest = new StringBuffer("POST / HTTP/1.1\r\n");
        else
            httpPostRequest = new StringBuffer("POST /" + _objectName + " HTTP/1.1\r\n");
        httpPostRequest.append(USER_AGENT);
        httpPostRequest.append("Host: " + _host + ":" + _port + "\r\n");
        httpPostRequest.append("Content-Length: 2147483647\r\n");
        httpPostRequest.append("Cache-Control: no-cache\r\n");
        httpPostRequest.append("\r\n");
        byte[] data = { (byte)0x00, (byte)13,  // 2-byte length (=13 bytes)
                (byte)_httpReconnectOpCode,    // 1-byte opcode for RIPC_JAVA_TUNNEL_RECONNECT
                _httpSessionIDbytes[0], _httpSessionIDbytes[1], _httpSessionIDbytes[2], _httpSessionIDbytes[3], // 4-byte HTTP Session ID
                _pidBytes[0], _pidBytes[1],    // 2-byte processId
                _ipAddressBytes[3], _ipAddressBytes[2], _ipAddressBytes[1], _ipAddressBytes[0] // 4-byte IPaddress
        };
        try
        {
            buf.clear();
            buf.put((httpPostRequest.toString()).getBytes("US-ASCII")); // add the HTTP POST request header
            buf.put(data);
            buf.flip(); // ready for reading
        }
        catch (IOException e)
        {
            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                System.out.println("IOException in setupClientReconnectPOSThttpRequest: " + e.getMessage());
        }
    }

    /* Only for httpReconnectState
     * Read response to HTTP CONNECT during httpReconnectState
     */
    protected boolean readHTTPreconnectResponseACK(ByteBuffer reader)
    {
        int bufferIndex = 0;
        reader.position(bufferIndex);
 
        try
        {
            // we expect 0x31 0x0D 0x0A 0x03 0x0D 0x0A
            byte ASCIIsize = reader.get(bufferIndex);
            if (ASCIIsize != 0x31)
                throw new IOException("Unable to reconnect tunnel. Response to Reconnect POST starts with: " + ASCIIsize);

            bufferIndex += 3; // skip 'ASCII size' + /r/n
            byte reconnectACK = reader.get(bufferIndex);

            if (reconnectACK != 3)
                throw new IOException("Unable to reconnect tunnel. Response to Reconnect POST is: " + reconnectACK + "Expected value of 3");
            // no need to read the rest of the response to Reconnect POST
        }
        catch (IOException e)
        {
            if ((db = System.getProperty("javax.net.debug")) != null && db.equals("all"))
                System.out.println("IOException in readReconnectResponse: " + e.getMessage());
        }
        return true;
    }

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
        s.append("\n\tclientIP: ");
        s.append(_channelInfo._clientIP);
        s.append("\n\tclientHostname: ");
        s.append(_channelInfo._clientHostname);
        s.append("\n\tpingTimeout: ");
        s.append(_channelInfo._pingTimeout);
        s.append("\n\tmajorVersion: ");
        s.append(_majorVersion);
        s.append("\n\tminorVersion: ");
        s.append(_minorVersion);
        s.append("\n\tprotocolType: ");
        s.append(_protocolType);
        s.append("\n\tuserSpecObject: ");
        s.append(_userSpecObject);
        s.append("\n\t\t_httpSessionID: ");
        s.append(_httpSessionID);
        s.append("\n\t\thttpOKretry: ");
        s.append(httpOKretry);
        s.append("\n\t\thttpOKsize: ");
        s.append(httpOKsize);
        s.append("\n\t\tfirstHTTPchunkHeader_size: ");
        s.append(firstHTTPchunkHeader_size);
        s.append("\n\t\tfirstHTTPchunk_size: ");
        s.append(firstHTTPchunk_size);
        s.append("\n\t\t_bytesWritten: ");
        s.append(_bytesWritten);
        s.append("\n\t\t_totalWriteTunnelCount: ");
        s.append(_totalWriteTunnelCount);
        s.append("\n\t\t_httpReconnectState: ");
        s.append(_httpReconnectState);
        s.append("\n\t\t_httpReconnectProxyActive: ");
        s.append(_httpReconnectProxyActive);
        s.append("\n\t\trcvACKnewChannel: ");
        s.append(rcvACKnewChannel);
        s.append("\n\t\treadAllRemainingOldSocketChannel: ");
        s.append(readAllRemainingOldSocketChannel);
        s.append("\n\t\trcvEndOfResponseOldChannel: ");
        s.append(rcvEndOfResponseOldChannel);
        s.append("\n\t\t_oldCrypto: ");
        s.append(_oldCrypto);
        s.append("\n\t\tendOfOldSocketMessage: ");
        s.append(endOfOldSocketMessage);
        s.append("\n\t\t_proxyCredentails: ");
        s.append(_proxyCredentails);
        s.append("\n\t\t_proxyAuthenticator: ");
        s.append(_proxyAuthenticator);
        s.append("\n\t\t_proxyConnectResponse: ");
        s.append(_proxyConnectResponse);
        s.append("\n\t\t_ignoredConnectResponses: ");
        s.append(_ignoredConnectResponses);
        s.append("\n\t\t_additionalHttpConnectParams: ");
        s.append(_additionalHttpConnectParams);
        // if(_readIoBuffer != null)
        // {
        //    s.append("\n\t_readIoBuffer.buffer().capacity(): ");
        //    s.append(_readIoBuffer.buffer().capacity());
        //    s.append("\n\t_readIoBuffer.buffer().position(): ");
        //    s.append(_readIoBuffer.buffer().position());
        //    s.append("\n\t_readIoBuffer.buffer().limit(): ");
        //    s.append(_readIoBuffer.buffer().limit());
        // }
        s.append("\n");

        return s.toString();
    }

    private void setHTTPHeaders()
    {
        HTTP_HEADER3 = 3;
        HTTP_HEADER4 = 4;
        CHUNKEND_SIZE = 2;
    }

}
