/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Diagnostics;
using System.IO;
using System.Threading;


using LSEG.Eta.Transports;
using LSEG.Eta.Common;
using LSEG.Eta.Transports.Internal;
using System.Net.Sockets;
using System.Text;
using System.Net.Security;
using System.Collections.Generic;
using System.Runtime.InteropServices;
using LSEG.Eta.Transports.proxy;

namespace LSEG.Eta.Internal.Interfaces
{
    /// <summary>
    /// Channel state change delegate.
    /// </summary>
    internal delegate bool ConnectionStateChangeHandler(ChannelBase sender, EventArgs sea);

    internal sealed class ChannelBase : IChannel, IInternalChannel
    {
        private Error m_transportError = new Error();
        internal class BigBuffersPool
        {
            internal Pool[] _pools = new Pool[32];
            internal int _maxSize = 0;
            internal int _maxPool = 0;
            internal int _fragmentSize;
            internal ChannelBase _poolOwner;

            internal BigBuffersPool(int fragmentSize, ChannelBase poolOwner)
            {
                // this pool should be created after the fragment size is known
                _poolOwner = poolOwner;
                _fragmentSize = fragmentSize;
                _maxSize = fragmentSize * 2;
            }

            internal EtaNode Poll(int size, bool isWriteBuffer)
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
                    ByteBuffer byteBuffer = new ByteBuffer(poolSize);
                    if (isWriteBuffer)
                    {
                        return new TransportBuffer(pool, byteBuffer, true);
                    }
                    else
                    {
                        var resBuffer = new TransportBuffer(pool, byteBuffer, true);
                        resBuffer.IsReadMode = true;
                        return resBuffer;
                    }
                }

                // The size is smaller then max, so traverse through pools to find available buffer
                for (int i = poolIndex; i <= _maxPool; i++)
                {
                    if (_pools[i] != null)
                    {
                        buffer = _pools[i].Poll();
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

                ByteBuffer bBuffer = new ByteBuffer(poolSize);
                if (isWriteBuffer)
                {
                    return new TransportBuffer(_pools[poolIndex], bBuffer, true);
                }
                else
                {
                    var resBuffer = new TransportBuffer(_pools[poolIndex], bBuffer, true);
                    resBuffer.IsReadMode = true;
                    return resBuffer;
                }
            }
        }

        /// <summary>
        /// Represents initialize channel states
        /// </summary>
        public enum InitChnlState
        {
            INACTIVE = 0,
            READ_HDR = 1,
            COMPLETE = 2,
            ACTIVE = 3,
            CONNECTING = 4,
            WAIT_ACK = 6,

            PROXY_CONNECTING = 13,
            CLIENT_WAIT_PROXY_ACK = 14,
            CLIENT_WAIT_HTTP_ACK = 15,

            /// <summary>
            /// used when the far end closes the connection and need to reconnect to Init()
            /// </summary>
            RECONNECTING = 16,

            /// <summary>
            /// used with connection version 14 or higher for key exchange
            /// </summary>
            WAIT_CLIENT_KEY = 18,
        }

        internal void SocketBufferToRecycle(SocketBuffer buffer)
        {
            if (buffer != m_CurrentBuffer)
            {
                buffer.ReturnToPool();
                if (m_Used > 0)
                {
                    --m_Used;
                }
            }
        }

        #region SocketChannel

        private readonly ISocketChannel _socketChannel;
        public ISocketChannel SocketChannel { get => _socketChannel; }

        #endregion

        internal InitChnlState InitChannelState { get; set; } = InitChnlState.INACTIVE;

        internal RipcVersions RipcVersion { get; private set; }
    
        internal const string DEFAULT_PRIORITY_FLUSH_ORDER = "HMHLHM";
        private const int MAX_FLUSH_STRATEGY = 32;
        internal const int DEFAULT_HIGH_WATER_MARK = 6144;
        internal const int DEFAULT_MAX_FRAGMENT_SIZE = 6144;
        internal const int DEFAULT_CLIENT_KEY_HIGH_WATER_MARK = 14;

        internal const int IPC_PACKING = 0x10;

        internal static int READ_RECEIVE_BUFFER_SIZE = DEFAULT_MAX_FRAGMENT_SIZE;

        internal int m_totalBytesQueued = 0;
        internal int m_highWaterMark = 0;
        internal bool m_isFlushOrderPending = false;

        // Queues to priority flush order
        internal EtaQueue m_highPriorityQueue = new EtaQueue();
        internal EtaQueue m_mediumPriorityQueue = new EtaQueue();
        internal EtaQueue m_lowPriorityQueue = new EtaQueue();

        private readonly EtaQueue[] m_flushOrder = new EtaQueue[MAX_FLUSH_STRATEGY];
        private int m_flushOrderPosition = 0;
        internal readonly TransportBuffer[] m_writeBufferArray = new TransportBuffer[MAX_FLUSH_STRATEGY];
        internal readonly List<ArraySegment<byte>> m_ArraySengmentList = new(MAX_FLUSH_STRATEGY);
        internal int m_writeArrayMaxPosition = 0;
        internal int m_writeArrayPosition = 0;

        private Locker _readLocker;
        private Locker _writeLocker;

        internal ReadBufferStateMachine m_ReadBufferStateMachine;
        internal TransportBuffer m_AppReadBuffer;


        internal BigBuffersPool m_BigBuffersPool;

        internal ComponentInfo ComponentInfo { get; set; } = new ComponentInfo();
        string PortStr { get; set; } = "";

        // Component version info passed in through ConnectOptions
        ComponentInfo m_ConnectOptsComponentInfo;

        #region Server Channel
        internal ServerImpl m_ServerImpl;
        internal bool NakMount { get; set; }
        #endregion

        #region Ping Buffer

        private static ByteBuffer _pingBuffer;
        private ByteBuffer PingBuffer
        {
            get
            {
                if (_pingBuffer == null)
                {
                    _pingBuffer = new ByteBuffer(RipcDataMessage.HeaderSize);

                    // pre-populate ping message since always the same
                    _pingBuffer.Write((short)RipcDataMessage.HeaderSize); // ripc header length
                    _pingBuffer.Write((byte)RipcFlags.DATA);              // ripc flag indicating data
                }
                return _pingBuffer;
            }
        }
        #endregion

        private ByteBuffer _readBuffer;

        #region Output buffers

        internal Pool m_AvailableBuffers;

        internal SocketBuffer m_CurrentBuffer;

        // Current number of used buffer;
        internal int m_Used;
        #endregion

        internal ConnectOptions ConnectionOptions { get; private set; }

        // Max fragmentation message size including message's header
        internal int InternalFragmentSize { get; set; }

        internal IProtocolFunctions ProtocolFunctions { get; private set; }

        internal ChannelInfo m_ChannelInfo = new();

        private readonly IpcProtocolManager m_IpcProtocolManager = new IpcProtocolManager();

        // Handles RIPC Protocol versions
        private IpcProtocol m_IpcProtocol;

        // RIPC compression variables
        internal byte m_SessionCompLevel = 6;
        internal int m_SessionCompLowThreshold = 0;
        internal const int ZLIB_COMPRESSION_THRESHOLD = 30;
        internal const int LZ4_COMPRESSION_THRESHOLD = 300;
        internal CompressionType m_SessionInDecompress;
        internal CompressionType m_SessionOutCompression;
        internal TransportBuffer m_DecompressBuffer;
        internal readonly Compressor m_ZlibCompressor = new ZlibCompressor();
        internal readonly Compressor m_Lz4Compressor = new Lz4Compressor();
        internal Compressor m_Compressor;
        internal int m_CompressPriority = 99; // messages of this priority are compressed
        internal long Shared_Key = 0;

        internal bool IsProviderHTTP { get; set; } = false;

        internal readonly ByteBuffer m_InitChnlReadClientKeyBuffer = new ByteBuffer(DEFAULT_CLIENT_KEY_HIGH_WATER_MARK);
        internal readonly ByteBuffer m_InitChnlReadBuffer = new ByteBuffer(DEFAULT_HIGH_WATER_MARK);
        internal readonly ByteBuffer m_InitChnlWriteBuffer = new ByteBuffer(DEFAULT_HIGH_WATER_MARK);

        const int IPC_100_CONN_ACK = 10;

        internal bool m_IsJunitTest = false;

        private ProtocolBase m_SocketProtocol;

        #region proxy
        private bool IsProxyEnabled { get; set; } = false;
        internal const string CHAR_ENCODING = "US-ASCII";
        internal const string USER_AGENT = "User-Agent: ETA/CSharp\r\n";
        internal const string PROXY_CONNECTION_KEEP_ALIVE = "Proxy-Connection: Keep-Alive\r\n";
        internal const string PRAGMA_NO_CACHE = "Pragma: no-cache\r\n";
        internal const string EOL = "\r\n";
        internal const string END_OF_RESPONSE = "\r\n\r\n";
        private string AdditionalHttpConnectParams { get; set; } = null;

        //Used to re-assemble "incomplete" HTTP responses while connecting thorough a proxy
        internal StringBuilder m_ProxyConnectResponse = new StringBuilder();
        internal ProxyAuthenticator m_ProxyAuthenticator;
        internal Credentials m_Credentials = new Credentials();

        // The number of times we ignored an HTTP response while connecting through a proxy
        internal int m_IgnoredConnectResponse = 0;

        // The maximum number of times we will ignore an HTTP response while connecting thorough a proxy
        internal const int MAX_IGNORED_RESPONSES = 10000;
        #endregion

        #region HTTP tunneling
        internal int HTTP_HEADER4 = 0;
        internal int CHUNKEND_SIZE = 0;
        #endregion

        #region Encrypted channel

        internal TcpClient TcpClient { get; set; }
        internal SslStream SslStream { get; set; }

        #endregion

        internal ChannelBase(ProtocolBase socketProtocol, ConnectOptions connectionOptions, ISocketChannel socketChannel)
        {
            if (socketProtocol == null)
            {
                throw new ArgumentNullException(nameof(socketProtocol));
            }

            m_SocketProtocol = socketProtocol;
            _socketChannel = socketChannel;

            ConnectionType = connectionOptions.ConnectionType;

            ConnectionOptions = new ConnectOptions();
            connectionOptions.CopyTo(ConnectionOptions);
            State = ChannelState.INACTIVE;

            MajorVersion = ConnectionOptions.MajorVersion;
            MinorVersion = ConnectionOptions.MinorVersion;

            UserSpecObject = ConnectionOptions.UserSpecObject;

            // Initialize the default flush strategy
            FlushOrder(DEFAULT_PRIORITY_FLUSH_ORDER);
            m_highWaterMark = DEFAULT_HIGH_WATER_MARK;

            m_ChannelInfo.MaxFragmentSize = DEFAULT_MAX_FRAGMENT_SIZE;
            InternalFragmentSize = m_ChannelInfo.MaxFragmentSize + RipcDataMessage.HeaderSize;

            m_ChannelInfo.CompressionType = ConnectionOptions.CompressionType;
            m_ChannelInfo.PingTimeout = ConnectionOptions.PingTimeout;
            m_ChannelInfo.GuaranteedOutputBuffers = ConnectionOptions.GuaranteedOutputBuffers;
            m_ChannelInfo.MaxOutputBuffers = m_ChannelInfo.GuaranteedOutputBuffers;
            m_ChannelInfo.NumInputBuffers = ConnectionOptions.NumInputBuffers;
            m_ChannelInfo.SysSendBufSize = ConnectionOptions.SysSendBufSize;
            m_ChannelInfo.SysRecvBufSize = ConnectionOptions.SysRecvBufSize;
            MajorVersion = ConnectionOptions.MajorVersion;
            MinorVersion = ConnectionOptions.MinorVersion;
            ProtocolType = ConnectionOptions.ProtocolType;
            UserSpecObject = ConnectionOptions.UserSpecObject;
            Blocking = ConnectionOptions.Blocking;

            if (ConnectionOptions.ComponentVersion != null)
            {
                ByteBuffer connectOptsCompVerBB = new ByteBuffer(Encoding.ASCII.GetBytes(ConnectionOptions.ComponentVersion));
                m_ConnectOptsComponentInfo = new ComponentInfo();
                m_ConnectOptsComponentInfo.ComponentVersion.Data(connectOptsCompVerBB);
            }

            // compression
            m_SessionInDecompress = ConnectionOptions.CompressionType;

            HostName = ConnectionOptions.UnifiedNetworkInfo.Address;
            PortStr = ConnectionOptions.UnifiedNetworkInfo.ServiceName;
            Port = SocketProtocol.ParsePort(ConnectionOptions.UnifiedNetworkInfo.ServiceName);

            if(Port == -1)
            {
                throw new TransportException($"Failed to parse a port number for the service name {ConnectionOptions.UnifiedNetworkInfo.ServiceName}");
            }

            ConnectionOptions.UnifiedNetworkInfo.Port = Port;

            IsProxyEnabled = ConnectionOptions.IsProxyEnabled();

            if(IsProxyEnabled)
            {
                int proxyPort = SocketProtocol.ParsePort(ConnectionOptions.ProxyOptions.ProxyPort);

                if(proxyPort == -1)
                {
                    throw new TransportException($"Failed to parse a proxy port number for the service name {ConnectionOptions.ProxyOptions.ProxyPort}");
                }

                ConnectionOptions.ProxyOptions.Port = proxyPort;
                m_Credentials = ReadProxyCredentials(ConnectionOptions);
                m_ProxyAuthenticator = new ProxyAuthenticator(m_Credentials, ConnectionOptions.ProxyOptions.ProxyHostName);
            }

            _readLocker = ConnectionOptions.ChannelReadLocking
               ? new MonitorWriteLocker(new object())
               : new NoLocker();

            _writeLocker = ConnectionOptions.ChannelWriteLocking
                         ? new MonitorWriteLocker(new object())
                         : new NoLocker();

            ProtocolFunctions = new RipcProtocolFunctions(this);

            m_AppReadBuffer = new TransportBuffer(new ByteBuffer(null, true), 0, false)
            {
                IsReadMode = true
            };

            m_AvailableBuffers = new Pool(this);
        }

        private Credentials ReadProxyCredentials(ConnectOptions connectionOptions)
        {
            Credentials credentials = new Credentials();
            if(!string.IsNullOrEmpty(connectionOptions.ProxyOptions.ProxyUserName))
            {
                credentials.SetKeyValue(CredentialName.USERNAME, connectionOptions.ProxyOptions.ProxyUserName);
            }

            if(!string.IsNullOrEmpty(connectionOptions.ProxyOptions.ProxyPassword))
            {
                credentials.SetKeyValue(CredentialName.PASSWORD, connectionOptions.ProxyOptions.ProxyPassword);
            }

            return credentials;
        }

        internal ChannelBase(SocketProtocol socketProtocol, AcceptOptions acceptOptions, IServer server, ISocketChannel socketChannel)
        {
            if (socketProtocol == null)
            {
                throw new ArgumentNullException(nameof(socketProtocol));
            }

            m_SocketProtocol = socketProtocol;
            m_ServerImpl = (ServerImpl)server;

            _socketChannel = socketChannel;
            Socket = _socketChannel.Socket;

            socketChannel.SetReadWriteHandlers(false);
           
            State = ChannelState.INITIALIZING;

            ConnectionType = m_ServerImpl.BindOptions.ConnectionType;

            m_ChannelInfo.CompressionType = m_ServerImpl.BindOptions.CompressionType;
            m_ChannelInfo.CompressionThresHold = GetDefaultCompressionTreshold(m_ChannelInfo.CompressionType);
            m_ChannelInfo.PingTimeout = m_ServerImpl.BindOptions.PingTimeout;
            m_ChannelInfo.GuaranteedOutputBuffers = m_ServerImpl.BindOptions.GuaranteedOutputBuffers;
            m_ChannelInfo.MaxOutputBuffers = m_ServerImpl.BindOptions.MaxOutputBuffers;
            m_ChannelInfo.NumInputBuffers = m_ServerImpl.BindOptions.NumInputBuffers;
            m_ChannelInfo.SysSendBufSize = acceptOptions.SysSendBufSize;
            m_ChannelInfo.SysRecvBufSize = acceptOptions.SysRecvBufSize;
            MajorVersion = m_ServerImpl.BindOptions.MajorVersion;
            MinorVersion = m_ServerImpl.BindOptions.MinorVersion;
            ProtocolType = m_ServerImpl.BindOptions.ProtocolType;
            Blocking = m_ServerImpl.BindOptions.ChannelIsBlocking;
            m_SessionCompLevel = (byte)m_ServerImpl.BindOptions.CompressionLevel;

            UserSpecObject = acceptOptions.UserSpecObject ?? m_ServerImpl.BindOptions.UserSpecObject;
            NakMount = acceptOptions.NakMount;

            // Initialize the default flush strategy
            FlushOrder(DEFAULT_PRIORITY_FLUSH_ORDER);
            m_highWaterMark = DEFAULT_HIGH_WATER_MARK;

            m_ChannelInfo.MaxFragmentSize = m_ServerImpl.BindOptions.MaxFragmentSize;
            InternalFragmentSize = m_ChannelInfo.MaxFragmentSize + RipcDataMessage.HeaderSize;

            _readLocker = acceptOptions.ChannelReadLocking
               ? new MonitorWriteLocker(new object())
               : new NoLocker();

            _writeLocker = acceptOptions.ChannelWriteLocking
                         ? new MonitorWriteLocker(new object())
                         : new NoLocker();

            ProtocolFunctions = new RipcProtocolFunctions(this);

            m_AppReadBuffer = new TransportBuffer(new ByteBuffer(null, true), 0, false)
            {
                IsReadMode = true
            };

            m_AvailableBuffers = new Pool(this);

            InitChannelState = InitChnlState.READ_HDR;
        }

            // This is used for unit testing only for setting channel behaviors
       internal ChannelBase(ConnectOptions connectionOptions, ISocketChannel socketChannel, ChannelState state, int HighWaterMark,
            string flushStrategy = DEFAULT_PRIORITY_FLUSH_ORDER, int maxFragmentSize = DEFAULT_MAX_FRAGMENT_SIZE, RipcVersions ripcVersion = RipcVersions.VERSION13, int numInputBufs = 10)
        {
            _socketChannel = socketChannel;

            ConnectionOptions = new ConnectOptions();
            connectionOptions.CopyTo(ConnectionOptions);

            State = state;

            MajorVersion = ConnectionOptions.MajorVersion;
            MinorVersion = ConnectionOptions.MinorVersion;

            UserSpecObject = ConnectionOptions.UserSpecObject;

            // Initialize the default flush strategy
            FlushOrder(flushStrategy);
            m_highWaterMark = HighWaterMark;

            m_ChannelInfo.MaxFragmentSize = maxFragmentSize;
            InternalFragmentSize = m_ChannelInfo.MaxFragmentSize + RipcDataMessage.HeaderSize;

            _readLocker = ConnectionOptions.ChannelReadLocking
               ? new MonitorWriteLocker(new object())
               : new NoLocker();

            _writeLocker = ConnectionOptions.ChannelWriteLocking
                         ? new MonitorWriteLocker(new object())
                         : new NoLocker();

            m_SocketProtocol = new SocketProtocol(); // Output buffer pool per protocol type.
            m_AvailableBuffers = new Pool(this);
            GrowGuaranteedOutputBuffers(ConnectionOptions.GuaranteedOutputBuffers);

            ProtocolFunctions = new RipcProtocolFunctions(this);

            RipcVersion = ripcVersion;

            /* Creates a read buffer to handle input messages */
            m_ReadBufferStateMachine = new ReadBufferStateMachine(this, ProtocolFunctions, ripcVersion, null, 
                numInputBufs * InternalFragmentSize, null);

            m_AppReadBuffer = new TransportBuffer(new ByteBuffer(null, true), 0, false)
            {
                IsReadMode = true
            };

            m_BigBuffersPool = new BigBuffersPool(InternalFragmentSize, this);
        }

        Lazy<BinaryWriter> _logStream = new Lazy<BinaryWriter>(() => {
        var stream = new FileStream($"eta.channelbase.{DateTime.UtcNow:yyyyMMdd-HHmmss}.etastream", FileMode.Create, FileAccess.Write);

        return new BinaryWriter(stream);
        });

        #region IChannel

        public TransportReturnCode Init(InProgInfo inProg, out Error error)
        {
            Debug.Assert(inProg != null);
            error = null;

            TransportReturnCode ret = TransportReturnCode.FAILURE;
            inProg.Clear();

            try
            {
                EnterReadWriteLocks();

                if(_socketChannel.FinishConnect(this) == false)
                {
                    if (_socketChannel.IsEncrypted && _socketChannel.IsAuthenFailure)
                    {
                        string reason = ".";

                        if(string.IsNullOrEmpty(_socketChannel.AuthenFailureMessage) == false)
                        {
                            reason = $". Reason:{_socketChannel.AuthenFailureMessage}";
                        }

                        error = new Error
                        {
                            Channel = this,
                            ErrorId = TransportReturnCode.FAILURE,
                            SysError = 0,
                            Text = $"Failed to create an encrypted connection to the remote endpoint{reason}"
                        };

                        return TransportReturnCode.FAILURE;
                    }
                    else
                    {
                        return TransportReturnCode.CHAN_INIT_IN_PROGRESS;
                    }
                }

                switch(InitChannelState)
                {
                    case InitChnlState.CONNECTING:
                        ret = InitChnlSendConnectReq(inProg, out error);
                        break;
                    case InitChnlState.WAIT_ACK:
                        ret = InitChnlWaitConnectAck(inProg, out error);
                        if(InitChannelState == InitChnlState.RECONNECTING && !ConnectionOptions.Blocking)
                        {
                            // The far end closed connection. Try another protocol.
                            inProg.OldSocket = _socketChannel.Socket;
                            OldSocket = inProg.OldSocket;
                            if (Connect(out error) == TransportReturnCode.SUCCESS)
                            {
                                if(IsProxyEnabled)
                                {
                                    InitChannelState = InitChnlState.PROXY_CONNECTING;
                                }
                                else
                                {
                                    InitChannelState = InitChnlState.CONNECTING;
                                }

                                inProg.Flags = InProgFlags.SCKT_CHNL_CHANGE;
                                inProg.NewSocket = _socketChannel.Socket;
                                Socket = _socketChannel.Socket;
                                return TransportReturnCode.CHAN_INIT_IN_PROGRESS;
                            }
                            else
                            {
                                error = new Error
                                {
                                    Channel = this,
                                    ErrorId = TransportReturnCode.FAILURE,
                                    SysError = 0,
                                    Text = "Handshake failed with far end. No more Protocols to try."
                                };

                                return TransportReturnCode.FAILURE;
                            }
                        }
                        break;
                    case InitChnlState.PROXY_CONNECTING:
                        ret = InitChnlProxyConnecting();
                        break;
                    case InitChnlState.CLIENT_WAIT_PROXY_ACK:
                        ret = InitChnlWaitProxyAck(inProg, out error);
                        if(ret == TransportReturnCode.FAILURE)
                        {
                            return TransportReturnCode.FAILURE;
                        }

                        if(m_ProxyAuthenticator.IsAuthenticated)
                        {
                            if (_socketChannel.IsEncrypted)
                            {
                                _socketChannel.PostProxyInit();
                                InitChannelState = InitChnlState.CONNECTING;
                                ret = TransportReturnCode.CHAN_INIT_IN_PROGRESS;
                            }
                            else
                            {
                                ret = InitChnlSendConnectReq(inProg, out error);
                            }
                        }
                        else
                        {
                            ret = TransportReturnCode.CHAN_INIT_IN_PROGRESS;
                        }

                        break;
                    case InitChnlState.READ_HDR:
                        ret = InitChnlReadHdr(inProg, out error);
                        break;
                    case InitChnlState.WAIT_CLIENT_KEY:
                        ret = InitChnlReadClientKey(inProg, out error);
                        break;
                    default:
                        break;
                }
            }
            catch(Exception exp)
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = exp.Message
                };

                return TransportReturnCode.FAILURE;
            }
            finally
            {
                ExitReadWriteLocks();
            }

            if (ret < TransportReturnCode.SUCCESS)
            {
                if (error == null)
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = ret,
                        SysError = 0,
                        Text = "Error occurred during connection process."
                    };
                }

                if(m_ReadBufferStateMachine != null)
                {
                    m_ReadBufferStateMachine.Buffer._data = null;
                    m_ReadBufferStateMachine = null;
                }
            }
            else
            {
                ProtocolFunctions.SetReadBufferStateMachine(m_ReadBufferStateMachine);
            }

            return ret;
        }

        public ITransportBuffer Read(ReadArgs readArgs, out Error error)
        {
            _readLocker.Enter();

            try
            {
                error = null;
                TransportBuffer transportBuffer = null;
                TransportReturnCode returnValue = 0;

                if (State != ChannelState.ACTIVE)
                {
                    readArgs.BytesRead = -1;
                    readArgs.ReadRetVal = TransportReturnCode.FAILURE;
                    error = new Error(TransportReturnCode.FAILURE, "Channel is not active", channel: this);
                    return null;
                }

                // initialize bytesRead and uncompressedBytesRead
                readArgs.BytesRead = 0;
                readArgs.UncompressedBytesRead = 0;

                // Update the read state machine if there is next message in the buffer
                UpdateState(readArgs, out error);

                // Read more data from network if we don't have data to give to user
                if(m_ReadBufferStateMachine.State != ReadBufferStateMachine.BufferState.KNOWN_COMPLETE)
                {
                    PerformReadIO(readArgs, out error);
                }

                switch(m_ReadBufferStateMachine.State)
                {
                    case ReadBufferStateMachine.BufferState.KNOWN_COMPLETE:

                        if(!ProtocolFunctions.IsPingMessage())
                        {
                            int entireMessageLength = m_ReadBufferStateMachine.CurrentMessageLength();

                            returnValue = UpdateAppReadBuffer(entireMessageLength, readArgs);
                            if(m_ReadBufferStateMachine.DataLength() != 0 && returnValue >= TransportReturnCode.SUCCESS)
                            {
                                transportBuffer = m_AppReadBuffer;
                            }
                            else if (returnValue == TransportReturnCode.SUCCESS)
                            {
                                returnValue = TransportReturnCode.READ_WOULD_BLOCK;
                            }
                        }
                        else
                        {
                            readArgs.UncompressedBytesRead = readArgs.BytesRead;
                            returnValue = TransportReturnCode.READ_PING;
                        }

                        break;

                    case ReadBufferStateMachine.BufferState.NO_DATA:
                        returnValue = TransportReturnCode.READ_WOULD_BLOCK;
                        break;

                    case ReadBufferStateMachine.BufferState.END_OF_STREAM:

                        try
                        {
                            _writeLocker.Enter();

                            if (State != ChannelState.CLOSED)
                            {
                                CloseFromError();
                            }

                            State = ChannelState.CLOSED;

                            if (error is null)
                            {
                                error = new Error
                                {
                                    Channel = this,
                                    ErrorId = TransportReturnCode.FAILURE,
                                    SysError = 0,
                                    Text = "SocketChannel.Receive returned -1 (end-of-stream)"
                                };
                            }
                        }
                        finally
                        {
                            _writeLocker.Exit();
                        }


                        returnValue = TransportReturnCode.FAILURE;

                        break;
                    default:
                        returnValue = (TransportReturnCode)(m_ReadBufferStateMachine.Buffer.WritePosition - m_ReadBufferStateMachine.CurrentMsgStartPos);
                        break;
                }

                readArgs.ReadRetVal = returnValue;
                return transportBuffer;
            }
            catch (Exception e)
            {
                try
                {
                    _writeLocker.Enter();

                    if (State != ChannelState.CLOSED)
                    {
                        CloseFromError();
                    }

                    State = ChannelState.CLOSED;

                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = e.Message
                    };
                    readArgs.ReadRetVal = TransportReturnCode.FAILURE;
                }
                finally
                {
                    _writeLocker.Exit();
                }

                return null;
            }
            finally
            {
                _readLocker.Exit();
            }
        }

        private void CloseFromError()
        {
            ReleaseAllBuffers();

            /* Removes this Channel from the Channel list. */
            m_SocketProtocol.CloseChannel(this);

            DisposeInternalBuffers();

            if (_socketChannel.IsEncrypted)
            {
                if (SslStream != null)
                {
                    SslStream.Close();

                    if (TcpClient.Connected)
                    {
                        TcpClient.Close();
                    }
                }
            }
            else
            {
                _socketChannel.Disconnect();
            }
        }

        public TransportReturnCode BufferUsage(out Error error)
        {
            error = null;
            TransportReturnCode ret;
            try
            {
                _writeLocker.Enter();
                if ((State == ChannelState.ACTIVE) || (State == ChannelState.CLOSED))
                {
                    ret = (TransportReturnCode)m_Used;
                }
                else
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = "channel not in active or closed state."
                    };

                    ret = TransportReturnCode.FAILURE;
                }
            }
            finally
            {
                _writeLocker.Exit();
            }

            return ret;
        }

        public TransportReturnCode Close(out Error error)
        {
            error = null;

            try
            {
                EnterReadWriteLocks();

                if (State != ChannelState.CLOSED)
                {
                    ReleaseAllBuffers();

                    /* Removes this Channel from the Channel list. */
                    m_SocketProtocol.CloseChannel(this);

                    /* Closes encrypted connection streams if any */
                    if (_socketChannel.IsEncrypted)
                    {
                        if (SslStream != null)
                        {
                            SslStream.Close();

                            if (TcpClient.Connected)
                            {
                                TcpClient.Close();
                            }
                        }
                    }
                    else
                    {
                        _socketChannel.Disconnect();
                    }

                    return TransportReturnCode.SUCCESS;
                }
            }
            finally
            {
                State = (State == ChannelState.INITIALIZING)
                            ? ChannelState.INACTIVE
                            : ChannelState.CLOSED;

                _socketChannel.Dispose();
                DisposeInternalBuffers();

                ExitReadWriteLocks();
            }

            return TransportReturnCode.SUCCESS;
        }

        private void DisposeInternalBuffers()
        {
            if (m_AppReadBuffer != null && m_AppReadBuffer.Data != null)
                m_AppReadBuffer.Data.Dispose();
            if (m_DecompressBuffer != null && m_DecompressBuffer.Data != null)
                m_DecompressBuffer.Data.Dispose();
            if (m_ReadBufferStateMachine != null)
                m_ReadBufferStateMachine.DisposeInternalBuffers();
        }

        private void ReleaseAllBuffers()
        {
            try
            {
                Transport.GlobalLocker.Enter();

                TransportBuffer buffer;
                while ((buffer = (TransportBuffer)m_highPriorityQueue.Poll()) != null)
                {
                    ReleaseBufferInternal(buffer);
                }
                while ((buffer = (TransportBuffer)m_mediumPriorityQueue.Poll()) != null)
                {
                    ReleaseBufferInternal(buffer);
                }
                while ((buffer = (TransportBuffer)m_lowPriorityQueue.Poll()) != null)
                {
                    ReleaseBufferInternal(buffer);
                }
                for (int i = 0; i < m_writeBufferArray.Length; i++)
                {
                    if (m_writeBufferArray[i] != null)
                    {
                        ReleaseBufferInternal(m_writeBufferArray[i]);
                    }
                    m_writeBufferArray[i] = null;
                }
                if (m_CurrentBuffer != null)
                {
                    m_CurrentBuffer.ReturnToPool();
                }

                m_writeArrayMaxPosition = 0;
                m_writeArrayPosition = 0;
                m_isFlushOrderPending = false;
                m_totalBytesQueued = 0;
                m_CurrentBuffer = null;

                // move all buffers from _availableBuffers pool to the global pool
                ShrinkGuaranteedOutputBuffers(m_AvailableBuffers.Size);
            }
            finally
            {
                Transport.GlobalLocker.Exit();
            }
        }


        public ChannelState State
        { get; private set; } = ChannelState.INACTIVE;

#if DEBUG
#region MessagesReadCount

        private long _messagesReadCount = 0;
        public long MessagesReadCount
        { get => Interlocked.Read(ref _messagesReadCount); }

#endregion
#endif

        public bool IsMessageReady { get; private set; }

#if DEBUG
        public long LastReceivedStamp { get; private set; }
#endif

        public int MajorVersion
        {
            get; internal set;
        }

        public int MinorVersion
        {
            get; internal set;
        }

        public Socket Socket { get; private set; }

        public Transports.ProtocolType ProtocolType {get; private set; }

        public int PingTimeOut { get => m_ChannelInfo.PingTimeout; }

        public object UserSpecObject { get; private set; }

        public bool Blocking { get; private set; }

        public ConnectionType ConnectionType { get; private set; }

        public string HostName { get; private set; } = "";

        public int Port { get; private set; } = 0;

        public Socket OldSocket { get; internal set; }

        #endregion

        internal TransportReturnCode Connect(out Error error)
        {
            error = null;
            bool res = _socketChannel.Connect(ConnectionOptions, out error);

            if (!res)
                return TransportReturnCode.FAILURE;

            Socket = _socketChannel.Socket;

            if (IsProxyEnabled)
            {
                InitChannelState = InitChnlState.PROXY_CONNECTING;
            }
            else
            {
                InitChannelState = InitChnlState.CONNECTING;
            }

            State = ChannelState.INITIALIZING;

            /* if re-connecting, don't block and call channel.init() until channel is active, since this case is a recursive call. */
            bool blockUntilActive = InitChannelState != InitChnlState.RECONNECTING;

            // The first time, m_IpcProtocol will be null prior to nextProtocol call.
            // If nextProtocol returns null, don't override m_IpcProtocool,
            // otherwise the next call to nextProtocol will return the newest protocol.
            // We want it to continue to return null so that we can abort and fail the connection.
            IpcProtocol ipcProtocol = m_IpcProtocolManager.NextProtocol(this, m_IpcProtocol, ConnectionOptions);
            if (ipcProtocol != null)
            {
                m_IpcProtocol = ipcProtocol;
                m_IpcProtocol.Options(ConnectionOptions);
            }
            else
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = "No more RIPC protocol to try"
                };

                return TransportReturnCode.FAILURE;
            }

            // if blocking connect, call channel.init() and get into ACTIVE state before returning
            if (ConnectionOptions.Blocking && blockUntilActive)
            {
                InProgInfo inProg = new InProgInfo();
                TransportReturnCode ret;

                while (State != ChannelState.ACTIVE)
                {
                    if ((ret = Init(inProg, out error)) < TransportReturnCode.SUCCESS)
                    {
                        if(ret == TransportReturnCode.FAILURE && InitChannelState == InitChnlState.RECONNECTING)
                        {
                            // The far end closed connection. Try another protocol.
                            if(Connect(out error) == TransportReturnCode.SUCCESS)
                            {
                                InitChannelState = InitChnlState.CONNECTING;
                                continue;
                            }
                            else
                            {
                                error = new Error
                                {
                                    Channel = this,
                                    ErrorId = TransportReturnCode.FAILURE,
                                    SysError = 0,
                                    Text = "Handshake failed with far end. No more Protocols to try"
                                };

                                ret = TransportReturnCode.FAILURE;
                            }
                        }

                        return ret;
                    }
                }
            }

            return TransportReturnCode.SUCCESS;
        }

        private bool IsDataBufferEmpty(ResultObject resultObject)
        {
            if (m_ReadBufferStateMachine.Buffer.ReadPosition == m_ReadBufferStateMachine.Buffer.WritePosition)
            {
                m_ReadBufferStateMachine.Buffer.Compact();
                return true;
            }
            else
            {
                return false;
            }
        }

        private bool CheckHasNextRipcMessage(ResultObject resultObject)
        {
            RipcDataMessage dataMessage = default(RipcDataMessage);

            return m_ReadBufferStateMachine.Buffer.HasMoreMessage(ref dataMessage, resultObject.Buffer.ReadPosition);
        }

        private void LoadReadBuffer(ResultObject resultObject)
        {
#if DEBUG
            LastReceivedStamp = DateTime.UtcNow.Ticks;
#endif
            RipcDataMessage dataMessage = default(RipcDataMessage);

            if (resultObject.Buffer.ReadAt(ref dataMessage, resultObject.Buffer.ReadPosition, false))
            {
                _readBuffer = resultObject.Buffer;
                IsMessageReady = true;
            }
            else
            {
                throw new InvalidOperationException($"ChannelBase::ReadMessage: *Broken Packe...");
            }
        }

        private bool HandleSocketReceiveRet(ref SocketError socketError, ref ReadArgs readArgs, out Error error)
        {
            error = null;

            if (socketError != SocketError.Success)
            {
                if (socketError == SocketError.WouldBlock)
                {
                    readArgs.BytesRead = -1;
                    readArgs.ReadRetVal = TransportReturnCode.READ_WOULD_BLOCK;
                    return false;
                }
                else
                {
                    readArgs.BytesRead = -1;
                    readArgs.ReadRetVal = TransportReturnCode.FAILURE;
                    error = new Error(TransportReturnCode.FAILURE, "Fails to read data from socket", channel: this);
                    return false;
                }
            }
            return true;
        }

        public override string ToString()
        {
            StringBuilder s = new StringBuilder();
            s.Append("Rssl Channel");
            s.Append($"{NewLine}\tscktChannel: ");
            s.Append(SocketChannel != null ? SocketChannel.ToString() : "null");
            s.Append($"{NewLine}\tconnected: ");
            if (SocketChannel != null)
                s.Append(SocketChannel.IsConnected);
            else
                s.Append("false");
            s.Append($"{NewLine}\tstate: ");
            s.Append(State.ToString());
            s.Append($"{NewLine}\tconnectionType: ");
            s.Append(ConnectionType.ToString());
            if (m_ChannelInfo != null)
            {
                s.Append($"{NewLine}\tclientIP: ");
                s.Append(m_ChannelInfo.ClientIP);
                s.Append($"{NewLine}\tclientHostname: ");
                s.Append(m_ChannelInfo.ClientHostname);
                s.Append($"{NewLine}\tpingTimeout: ");
                s.Append(m_ChannelInfo.PingTimeout);
            }
            s.Append($"{NewLine}\tmajorVersion: ");
            s.Append(MajorVersion);
            s.Append($"{NewLine}\tminorVersion: ");
            s.Append(MinorVersion);
            s.Append($"{NewLine}\tprotocolType: ");
            s.Append(ProtocolType);
            s.Append($"{NewLine}\tuserSpecObject: ");
            s.Append(UserSpecObject != null ? UserSpecObject.ToString() : "null");
            s.Append(NewLine);

            return s.ToString();
        }

        public ITransportBuffer GetBuffer(int size, bool packedBuffer, out Error error)
        {
            error = null;
            ITransportBuffer transportBuffer = null;

            _writeLocker.Enter();

            try
            {
                if (State != ChannelState.ACTIVE)
                {
                    m_transportError.Text = "Channel is not in active state for GetBuffer";
                    m_transportError.Channel = this;
                    m_transportError.ErrorId = TransportReturnCode.FAILURE;
                    m_transportError.SysError = 0;

                    error = m_transportError;

                    return transportBuffer;
                }

                int headerLength = ProtocolFunctions.EstimateHeaderLength();
                int sizeWithHeaders = size + headerLength;
                if (packedBuffer)
                {
                    sizeWithHeaders += RipcDataMessage.PackedHeaderSize;
                }

                // Checks whether the requested size is larger that a normal buffer
                if (sizeWithHeaders > InternalFragmentSize)
                {
                    if(!packedBuffer)
                    {
                        transportBuffer = ProtocolFunctions.GetBigBuffer(size, out error);
                        if (transportBuffer is null)
                        {
                            m_transportError.Text = $"Channel out of buffers, error: {error?.Text}";
                            m_transportError.Channel = this;
                            m_transportError.ErrorId = TransportReturnCode.FAILURE;
                            m_transportError.SysError = 0;

                            error = m_transportError;

                            return transportBuffer;
                        }
                    }
                    else
                    {
                        m_transportError.Text = "Packing buffer must fit in maxFragmentSize";
                        m_transportError.Channel = this;
                        m_transportError.ErrorId = TransportReturnCode.FAILURE;
                        m_transportError.SysError = 0;

                        error = m_transportError;

                        return transportBuffer;
                    }
                }
                else
                {
                    transportBuffer = GetBufferInternal(size, packedBuffer, headerLength);

                    if (transportBuffer is null)
                    {
                        m_transportError.Text = "Channel out of buffers";
                        m_transportError.Channel = this;
                        m_transportError.ErrorId = TransportReturnCode.FAILURE;
                        m_transportError.SysError = 0;

                        error = m_transportError;

                        return transportBuffer;
                    }
                }
            }
            finally
            {
                _writeLocker.Exit();
            }

            (transportBuffer as TransportBuffer).IsOwnedByApp = true;

            return transportBuffer;
        }

        public TransportReturnCode Write(ITransportBuffer msgBuf, WriteArgs writeArgs, out Error error)
        {
            TransportReturnCode ret = TransportReturnCode.SUCCESS;
            error = null;

            if (msgBuf is null || msgBuf.Length() == 0)
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = msgBuf != null ? "Buffer of length zero cannot be written" : "msgBuf cannot be null"
                };

                return TransportReturnCode.FAILURE;
            }

            TransportBuffer transportBuffer = msgBuf as TransportBuffer;

            if (transportBuffer.IsOwnedByApp == false)
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = "Application does not own this buffer."
                };

                return TransportReturnCode.FAILURE;
            }

            _writeLocker.Enter();

            try
            {
                if (State != ChannelState.ACTIVE)
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = "Channel is not in active state for write"
                    };

                    return TransportReturnCode.FAILURE;
                }

                short ripcHdrFlags = (short)RipcFlags.DATA;
                int msgLen = msgBuf.Length();

                // determine whether the buffer holds a large message to be fragmented
                if (transportBuffer.IsBigBuffer == false)
                {
                    if (transportBuffer.IsPacked)
                    {
                        ProtocolFunctions.PackBuffer(transportBuffer, false, this, out error);
                        msgLen = transportBuffer.PackedLength();
                        ripcHdrFlags |= (short)RipcFlags.PACKING;
                    }

                    bool compressedDataSent = ProtocolFunctions.CompressedData(this, msgLen, writeArgs);

                    if (compressedDataSent)
                    {
                        ret = (TransportReturnCode)ProtocolFunctions.WriteCompressed(transportBuffer, writeArgs, out error);

                        if (ret < TransportReturnCode.SUCCESS)
                        {
                            if (ret == TransportReturnCode.FAILURE)
                            {
                                CloseFromError();
                                State = ChannelState.CLOSED;
                            }

                            return ret;
                        }
                    }
                    else
                    {
                        // A normal message which is less than the max fragmentation size
                        ProtocolFunctions.PrependTransportHdr(transportBuffer, ripcHdrFlags);

                        ret = WriteBuffersQueued(transportBuffer, writeArgs, out error);

                        if (ret < TransportReturnCode.SUCCESS)
                        {
                            if (ret == TransportReturnCode.FAILURE)
                            {
                                CloseFromError();
                                State = ChannelState.CLOSED;
                            }

                            return ret;
                        }

                        writeArgs.UncompressedBytesWritten = writeArgs.BytesWritten;
                    }
                }
                else
                {   // message fragmentation

                    if (transportBuffer.IsWritePaused || ProtocolFunctions.WriteAsFragmentedMessage(transportBuffer))
                    {
                        ret = WriteBigBuffer(transportBuffer, writeArgs, out error);
                        if (ret < TransportReturnCode.SUCCESS)
                        {
                            if (ret == TransportReturnCode.FAILURE)
                            {
                                CloseFromError();
                                State = ChannelState.CLOSED;
                            }

                            return ret;
                        }
                    }
                    else
                    {   // The actual payload is less than the maximum fragmentation size
                        TransportBuffer buffer = transportBuffer.FirstBuffer;

                        transportBuffer.FirstBuffer = null;

                        buffer.HeaderLength = ProtocolFunctions.EstimateHeaderLength();
                        buffer.Data.WritePosition = ProtocolFunctions.EstimateHeaderLength();

                        transportBuffer.Data.Flip();
                        buffer.Data.Put(transportBuffer.Data);

                        ProtocolFunctions.PrependTransportHdr(buffer, ripcHdrFlags, true);

                        ret = WriteBuffersQueued(buffer, writeArgs, out error);

                        if (ret < TransportReturnCode.SUCCESS)
                        {
                            if (ret == TransportReturnCode.FAILURE)
                            {
                                CloseFromError();
                                State = ChannelState.CLOSED;
                            }

                            return ret;
                        }

                        writeArgs.UncompressedBytesWritten = writeArgs.BytesWritten;
                    }

                    if (ret != TransportReturnCode.WRITE_CALL_AGAIN)
                    {
                        transportBuffer.ReturnToPool();
                    }
                }

                if (ret >= TransportReturnCode.SUCCESS || ret == TransportReturnCode.WRITE_FLUSH_FAILED && State == ChannelState.ACTIVE)
                {
                    transportBuffer.IsOwnedByApp = false;
                }
            }
            catch (Exception ex)
            {
                if (State != ChannelState.CLOSED)
                {
                    CloseFromError();
                }

                State = ChannelState.CLOSED;

                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = ex.Message
                };

                ret = TransportReturnCode.FAILURE;
            }
            finally
            {
                _writeLocker.Exit();
            }

            return ret;
        }

        public TransportReturnCode Ping(out Error error)
        {
            TransportReturnCode ret = TransportReturnCode.SUCCESS;
            error = null;

            _writeLocker.Enter();

            try
            {
                if (State != ChannelState.ACTIVE)
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = "Channel is not in the active state for ping"
                    };

                    return TransportReturnCode.FAILURE;
                }

                int byteWritten = _socketChannel.Send(PingBuffer.Contents, 0, PingBuffer.Limit, out error);

                if (byteWritten >= 0)
                {
                    if (byteWritten < PingBuffer.Limit)
                    {
                        ret = (TransportReturnCode)(PingBuffer.Limit - byteWritten);
                    }
                }
                else
                {
                    error.Channel = this;
                    error.Text = "Channel Ping failed.";
                }
            }
            finally
            {
                _writeLocker.Exit();
            }

            return ret;
        }

        public TransportReturnCode ReleaseBuffer(ITransportBuffer buffer, out Error error)
        {
            error = null;

            TransportBuffer transportBuffer = buffer as TransportBuffer;

            if(transportBuffer is null || transportBuffer.IsOwnedByApp == false)
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = transportBuffer != null ? "Application does not own this buffer." : "buffer must not be null"
                };

                return TransportReturnCode.FAILURE;
            }

            _writeLocker.Enter();

            try
            {
                if(State == ChannelState.INACTIVE)
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = "Channel is in inactive state."
                    };

                    return TransportReturnCode.FAILURE;
                }

                ReleaseBufferInternal(buffer);

                if (error != null)
                {
                    error.Channel = this;
                    return TransportReturnCode.FAILURE;
                }
                else
                {
                    return TransportReturnCode.SUCCESS;
                }
            }
            finally
            {
                _writeLocker.Exit();
            }
        }

        public TransportReturnCode PackBuffer(ITransportBuffer bufferInt, out Error error)
        {
            Debug.Assert(bufferInt != null);
            TransportBuffer buffer = (TransportBuffer)bufferInt;

            try
            {
                _writeLocker.Enter() ;

                if (buffer.IsOwnedByApp == false)
                {
                    error = new Error()
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = "Application does not own this buffer."
                    };
                    return TransportReturnCode.FAILURE;
                }

                // return FAILURE if channel not active
                if (State != ChannelState.ACTIVE)
                {
                    error = new Error()
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = "Socket channel is not in active state for pack."
                    };
                    return TransportReturnCode.FAILURE;
                }

                return (TransportReturnCode)ProtocolFunctions.PackBuffer(buffer, true, this, out error);
            }
            finally
            {
                _writeLocker.Exit();
            }
        }

        public TransportReturnCode Flush(out Error error)
        {
            error = null;

            TransportReturnCode ret = TransportReturnCode.SUCCESS;
            int scktBytesWritten = 0;
            int cumulativeBytesPendingWrite = 0;

            _writeLocker.Enter();

            try
            {
                if(State != ChannelState.ACTIVE)
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = "Socket channel is not in the active state for flush"
                    };

                    return TransportReturnCode.FAILURE;
                }

                // write all bytes queued.
                while (m_totalBytesQueued > 0
                    && State != ChannelState.INACTIVE
                    && State != ChannelState.CLOSED)
                {
                    cumulativeBytesPendingWrite = FillWriteBufferArray();

                    // write all bffers to socket
                    scktBytesWritten = WriteGatheringBufferArray(cumulativeBytesPendingWrite, out error);
                    if (scktBytesWritten < cumulativeBytesPendingWrite)
                    {
                        break; // partial write buffers
                    }
                }

                ret = (TransportReturnCode)m_totalBytesQueued;
            }
            finally
            {
                _writeLocker.Exit();
            }

            return ret;
        }

        internal SocketBuffer GetSocketBuffer()
        {
            SocketBuffer buffer = (SocketBuffer)m_AvailableBuffers.Poll();

            if (buffer == null)
            {
                if (m_ServerImpl != null && m_Used < m_ChannelInfo.MaxOutputBuffers)
                {
                    buffer = m_ServerImpl.GetBufferFromServerPool();
                }
            }

            if (buffer != null)
            {
                buffer.Clear();
                ++m_Used;
            }
            return buffer;
        }

        internal TransportReturnCode WriteBigBuffer(TransportBuffer bigBuffer, WriteArgs writeArgs, out Error error)
        {
            error = null;
            TransportReturnCode ret = TransportReturnCode.SUCCESS;
            int bytesLeft = 0;

            /* Checks whether the compression is enabled */
            bool doCompression = ProtocolFunctions.CompressedData(this, m_SessionCompLowThreshold, writeArgs);

            // Checks whether this is the first write call for the buffer
            if(bigBuffer.IsWritePaused == false)
            {
                bigBuffer.Data.Flip();
                bytesLeft = bigBuffer.Data.Limit;

                if (!doCompression)
                {
                    bytesLeft -= ProtocolFunctions.PopulateFragment(bigBuffer, true/* first fragment */, bigBuffer.FirstBuffer, 
                                                (byte)(RipcFlags.HAS_OPTIONAL_FLAGS | RipcFlags.DATA), writeArgs);

                    WriteFragment(bigBuffer.FirstBuffer, writeArgs);
                }
                else
                {
                    ret = (TransportReturnCode)ProtocolFunctions.WriteFragmentCompressed(bigBuffer, bigBuffer.FirstBuffer, writeArgs, true /* first fragment */, out error);
                    if (ret > TransportReturnCode.SUCCESS)
                    {
                        bytesLeft -= (int)ret;
                    }
                    else
                        return ret;
                }

                bigBuffer.FirstBuffer = null;
            }
            else
            {
                // When resuming a paused write, gets remaining bytes in big buffer
                bytesLeft = ProtocolFunctions.RemaingBytesAfterPausing(bigBuffer);
            }
            
            // Gets buffers for the rest of the data and copy data to the buffers
            while(bytesLeft > 0)
            {
                TransportBuffer nextBuffer = GetBufferInternal(InternalFragmentSize - RipcDataMessage.HeaderSize, false, RipcDataMessage.HeaderSize);
                if(nextBuffer is null)
                {
                    ret = (TransportReturnCode)FlushInternal(out error);
                    if(ret < TransportReturnCode.SUCCESS)
                    {
                        return ret;
                    }

                    if ((nextBuffer = GetBufferInternal(InternalFragmentSize - RipcDataMessage.HeaderSize, false, RipcDataMessage.HeaderSize)) is null)
                    {
                        bigBuffer.IsWritePaused = true;
                        return TransportReturnCode.WRITE_CALL_AGAIN;
                    }
                }

                if (!doCompression || ProtocolFunctions.CheckCompressionFragmentedMsg(bytesLeft))
                {
                    bytesLeft -= ProtocolFunctions.PopulateFragment(bigBuffer, false, nextBuffer, (byte)(RipcFlags.HAS_OPTIONAL_FLAGS | RipcFlags.DATA), writeArgs);

                    WriteFragment(nextBuffer, writeArgs);
                }
                else
                {
                    ret = (TransportReturnCode)ProtocolFunctions.WriteFragmentCompressed(bigBuffer, nextBuffer, writeArgs, false /* not first */, out error);
                    if (ret > TransportReturnCode.SUCCESS)
                        bytesLeft -= (int)ret;
                    else
                        return ret;
                }
            }

            if((writeArgs.Flags & WriteFlags.DIRECT_SOCKET_WRITE) > 0 || m_totalBytesQueued > m_highWaterMark)
            {
                if ((ret = (TransportReturnCode)FlushInternal(out error)) < TransportReturnCode.SUCCESS)
                {
                    return ret;
                }
            }
            else
            {
                ret = (TransportReturnCode)m_totalBytesQueued;
            }

            return ret;
        }

        internal void WriteFragment(TransportBuffer buffer, WriteArgs writeArgs)
        {
            // Adds to buffer queue
            AddToPriorityQueue(buffer, writeArgs.Priority);

            m_totalBytesQueued += buffer.TotalLength;
        }

        internal TransportReturnCode WriteBuffersQueued(TransportBuffer buffer, WriteArgs writeArgs, out Error error)
        {
            int ret = 0;
            int scktBytesWritten = 0;
            error = null;

            AddToPriorityQueue(buffer, writeArgs.Priority);

            if (m_totalBytesQueued > 0)
            {
                m_totalBytesQueued += buffer.TotalLength;

                scktBytesWritten = buffer.TotalLength;
                writeArgs.BytesWritten = scktBytesWritten;

                // if direct socket write or high water mark reached, call flush
                if ( ( (writeArgs.Flags & WriteFlags.DIRECT_SOCKET_WRITE) != 0) || m_totalBytesQueued > m_highWaterMark )
                {
                    ret = FlushInternal(out error);

                    if (ret < (int)TransportReturnCode.SUCCESS)
                        return (TransportReturnCode)ret;
                }
                else
                {
                    ret = m_totalBytesQueued;
                }
            }
            else
            {   // No buffer queued.
                m_totalBytesQueued += buffer.TotalLength;

                if ( (writeArgs.Flags & WriteFlags.DIRECT_SOCKET_WRITE) != 0 )
                {
                    ret = FlushInternal(out error);

                    if (ret < (int)TransportReturnCode.SUCCESS)
                        return (TransportReturnCode)ret;

                    scktBytesWritten = buffer.TotalLength - ret;
                    writeArgs.BytesWritten = scktBytesWritten;
                }
                else
                {
                    scktBytesWritten = buffer.TotalLength;
                    writeArgs.BytesWritten = scktBytesWritten;

                    // if high water mark reached, call flush
                    if( m_totalBytesQueued > m_highWaterMark)
                    {
                        ret = FlushInternal(out error);

                        if (ret < (int)TransportReturnCode.SUCCESS)
                            return (TransportReturnCode)ret;
                    }
                    else
                    {
                        ret = m_totalBytesQueued;
                    }
                }
            }

            return (TransportReturnCode)ret;
        }

        internal int FlushInternal(out Error error)
        {
            Debug.Assert(State == ChannelState.ACTIVE);

            int ret = 0;
            int scktBytesWritten = 0;
            int cumulativeBytesPendingWrite = 0;
            error = null;

            // write all bytes queued.
            while(m_totalBytesQueued > 0
                && State != ChannelState.INACTIVE
                && State != ChannelState.CLOSED)
            {
                cumulativeBytesPendingWrite = FillWriteBufferArray();

                // write all bffers to socket
                scktBytesWritten = WriteGatheringBufferArray(cumulativeBytesPendingWrite, out error);
                if(scktBytesWritten < cumulativeBytesPendingWrite)
                {
                    break; // partial write buffers
                }
            }

            if (error is null)
            {
                ret = m_totalBytesQueued;
            }
            else
            {
                ret = (int)error.ErrorId;
            }

            return ret;
        }

        private void AddToPriorityQueue(TransportBuffer buffer, WritePriorities writePrioritie)
        {
            switch(writePrioritie)
            {
                case WritePriorities.HIGH:
                    m_highPriorityQueue.Add(buffer);
                    break;
                case WritePriorities.MEDIUM:
                    m_mediumPriorityQueue.Add(buffer);
                    break;
                case WritePriorities.LOW:
                    m_lowPriorityQueue.Add(buffer);
                    break;
            }
        }

        private void FlushOrder(String flushStrategy)
        {
            m_ChannelInfo.PriorityFlushStrategy = flushStrategy;
            for(int i = 0; i < flushStrategy.Length; i++)
            {
                if(flushStrategy[i] == 'H')
                {
                    m_flushOrder[i] = m_highPriorityQueue;
                }
                else if (flushStrategy[i] == 'M')
                {
                    m_flushOrder[i] = m_mediumPriorityQueue;
                }
                else if (flushStrategy[i] == 'L')
                {
                    m_flushOrder[i] = m_lowPriorityQueue;
                }
            }
        }

        private int FillWriteBufferArray()
        {
            int remainingBytesQueued = m_totalBytesQueued;
            int cumulativeBytesPendingWrite = 0;
            int flushOrderPosition = 0;
            int bufferSize = 0;
            TransportBuffer buffer;

            if(m_isFlushOrderPending == false)
            {
                m_writeArrayMaxPosition = 0;
                m_writeArrayPosition = 0;

                while(m_writeArrayMaxPosition < m_writeBufferArray.Length
                    && remainingBytesQueued > 0
                    && State != ChannelState.INACTIVE
                    && State != ChannelState.CLOSED
                    )
                {
                    for (int i = m_flushOrderPosition; i < m_ChannelInfo.PriorityFlushStrategy.Length
                        && remainingBytesQueued > 0 && m_writeArrayMaxPosition < m_writeBufferArray.Length; i++, flushOrderPosition = i)
                    {
                        buffer = (TransportBuffer)m_flushOrder[i].Poll();
                        if(buffer != null)
                        {
                            buffer.Data.Flip(); // Change from write to read mode.
                            m_writeBufferArray[m_writeArrayMaxPosition] = buffer;
                            m_writeArrayMaxPosition++;
                            bufferSize = buffer.Data.Limit - buffer.StartPosition;
                            cumulativeBytesPendingWrite += bufferSize;
                            remainingBytesQueued -= bufferSize;
                        }
                    }

                    m_flushOrderPosition = 0;
                }

                m_flushOrderPosition = flushOrderPosition % m_ChannelInfo.PriorityFlushStrategy.Length;
            }
            else
            {
                for(int i = m_writeArrayPosition; i < m_writeArrayMaxPosition; i++)
                {
                    cumulativeBytesPendingWrite += m_writeBufferArray[i].Data.Limit - m_writeBufferArray[i].Data.Position;
                }
            }

            return cumulativeBytesPendingWrite;
        }

        private int WriteGatheringBufferArray(int cumulativeBytesPendingWrite, out Error error)
        {
            int scktBytesWritten = 0;
            TransportBuffer msgBuffer;
            error = null;

            if (SslStream != null) /* Checks for encrypted connection as the SslStream class doesn't support writing a list of buffers */
            {
                for (int i = m_writeArrayPosition; i < m_writeArrayMaxPosition; i++)
                {
                    msgBuffer = m_writeBufferArray[i];
                    scktBytesWritten += _socketChannel.Send(msgBuffer.Data._data, msgBuffer.StartPosition, msgBuffer.Data.Limit - msgBuffer.StartPosition, out error);

                    if (error != null) // Checks whether there is an error
                        break;
                }
            }
            else
            {
                m_ArraySengmentList.Clear();
                for (int i = m_writeArrayPosition; i < m_writeArrayMaxPosition; i++)
                {
                    msgBuffer = m_writeBufferArray[i];
                    ArraySegment<byte> buffer = new(msgBuffer.Data._data, msgBuffer.StartPosition, msgBuffer.Data.Limit - msgBuffer.StartPosition);
                    m_ArraySengmentList.Add(buffer);
                }

                if(m_ArraySengmentList.Count > 0)
                    scktBytesWritten = _socketChannel.Send(m_ArraySengmentList, out error);
            }

            if(scktBytesWritten == cumulativeBytesPendingWrite) // Sent all pending buffers
            {
                // release all buffers
                for(int i = m_writeArrayPosition; i < m_writeArrayMaxPosition; i++)
                {
                    ReleaseBufferInternal(m_writeBufferArray[i]);
                }

                m_totalBytesQueued -= cumulativeBytesPendingWrite;

                m_isFlushOrderPending = false;
            }
            else
            {   // handle partial pending buffers sent
                int bytesReleased = 0;
                int index = m_writeArrayPosition;

                while (bytesReleased < scktBytesWritten) // Releases buffers that were written
                {
                    bytesReleased += m_writeBufferArray[index].Data.Limit - m_writeBufferArray[index].StartPosition;
                    if (bytesReleased <= scktBytesWritten) // Releases full buffer
                    {
                        ReleaseBufferInternal(m_writeBufferArray[index]);
                    }
                    else
                    {   // Releases paritial buffer
                        int pos = (int)(m_writeBufferArray[index].Data.Limit - (bytesReleased - scktBytesWritten));
                        m_writeBufferArray[index].Data.ReadPosition = pos;
                        m_writeBufferArray[index].StartPosition = pos;
                        break;
                    }
                    index++;
                }

                m_isFlushOrderPending = true;
                m_writeArrayPosition = index;

                if (scktBytesWritten > 0)
                {
                    m_totalBytesQueued -= scktBytesWritten;
                }
            }

            return scktBytesWritten;
        }

        internal void UpdateState(ReadArgs readArgs, out Error error)
        {
            error = null;
            
            // if the previous sate was KNOWN_COMPLETE, advance the state machine,
            // and rewind the read IO buffer if there is no more data in it.
            if(m_ReadBufferStateMachine.State == ReadBufferStateMachine.BufferState.KNOWN_COMPLETE)
            {
                if(m_ReadBufferStateMachine.AdvanceOnApplicationRead(readArgs, out error) == ReadBufferStateMachine.BufferState.NO_DATA)
                {
                    m_ReadBufferStateMachine.Buffer.Rewind(); // no more data to read so rewind the buffer
                }
            }
        }

        internal void PerformReadIO(ReadArgs readArgs, out Error error)
        {
            error = null;
            switch(m_ReadBufferStateMachine.State)
            {
                case ReadBufferStateMachine.BufferState.KNOWN_INSUFFICENT:
                case ReadBufferStateMachine.BufferState.UNKNOWN_INSUFFICIENT:

                    m_ReadBufferStateMachine.Buffer.ReadPosition = m_ReadBufferStateMachine.CurrentMsgStartPos;
                    m_ReadBufferStateMachine.Buffer.Compact();
                    m_ReadBufferStateMachine.AdvanceOnCompact();

                    goto case ReadBufferStateMachine.BufferState.END_OF_STREAM;

                case ReadBufferStateMachine.BufferState.END_OF_STREAM:
                case ReadBufferStateMachine.BufferState.NO_DATA:
                case ReadBufferStateMachine.BufferState.KNOWN_INCOMPLETE:
                case ReadBufferStateMachine.BufferState.UNKNOWN_INCOMPLETE:
                
                    // read data from the socket channel
                    int bytesRead = SocketChannel.Receive(m_ReadBufferStateMachine, out error);
                    m_ReadBufferStateMachine.AdvanceOnSocketChannelRead(bytesRead, readArgs, out Error advanceError);

                    if(error is null && advanceError != null)
                    {
                        error = advanceError;
                    }

                    break;

                default:
                    Debug.Assert(false); // invalid state, must not reach here
                    break;
            }
        }

        internal unsafe TransportReturnCode UpdateAppReadBuffer(int entireMessageLength, ReadArgs readArgs)
        {
            Debug.Assert(entireMessageLength > 0);

            TransportReturnCode returnValue;

            ByteBuffer byteBuffer = m_ReadBufferStateMachine.DataBuffer();

            m_AppReadBuffer.Data._data = byteBuffer._data;
            m_AppReadBuffer.Data._pointer = byteBuffer._pointer;
            m_AppReadBuffer.Data.ReadPosition = m_ReadBufferStateMachine.DataPosition();
            m_AppReadBuffer.Data.WritePosition = m_ReadBufferStateMachine.DataPosition() + m_ReadBufferStateMachine.DataLength();

            returnValue = (TransportReturnCode)(m_ReadBufferStateMachine.Buffer.WritePosition - (m_ReadBufferStateMachine.CurrentMsgStartPos + entireMessageLength));

            // we cannot exactly-SUCCESS if we are in the middle of processing a packed message
            if (m_ReadBufferStateMachine.HasRemainingPackedData())
            {
                ++returnValue; // we have not processed the last part of a packed message
            }

            // handle compressed message
            if (m_ReadBufferStateMachine.SubState == ReadBufferStateMachine.BufferSubState.PROCESSING_COMPRESSED_MESSAGE)
            {
                int compressedDataLength = m_AppReadBuffer.Length();
                int uncompressedLength = m_Compressor.Decompress(m_AppReadBuffer, m_DecompressBuffer, compressedDataLength);

                readArgs.UncompressedBytesRead = uncompressedLength + ProtocolFunctions.AdditionalHeaderLength() + RipcLengths.HEADER;

                m_AppReadBuffer.Data._data = m_DecompressBuffer.Data._data;
                m_AppReadBuffer.Data._pointer = m_DecompressBuffer.Data._pointer;
                m_AppReadBuffer.Data.ReadPosition = m_DecompressBuffer.Data.ReadPosition;
                m_AppReadBuffer.Data.WritePosition = m_DecompressBuffer.Data.WritePosition;
            }

            Debug.Assert(returnValue >= TransportReturnCode.SUCCESS);

            return returnValue;
        }

        internal TransportReturnCode SetChannelAccept(AcceptOptions acceptOptions, BindOptions bindOptions, out Error error)
        {
            error = null;
            TransportReturnCode ret = TransportReturnCode.SUCCESS;

            m_ChannelInfo.CompressionType = bindOptions.CompressionType;
            m_ChannelInfo.PingTimeout = bindOptions.PingTimeout;
            m_ChannelInfo.GuaranteedOutputBuffers = bindOptions.GuaranteedOutputBuffers;
            m_ChannelInfo.MaxOutputBuffers = bindOptions.MaxOutputBuffers;
            m_ChannelInfo.NumInputBuffers = bindOptions.NumInputBuffers;
            MajorVersion = bindOptions.MajorVersion;
            MinorVersion = bindOptions.MinorVersion;
            ProtocolType = bindOptions.ProtocolType;
            UserSpecObject = acceptOptions.UserSpecObject;

            return ret;
        }

        public TransportReturnCode Info(ChannelInfo info, out Error error)
        {
            error = null;
            TransportReturnCode ret = TransportReturnCode.SUCCESS;

            try
            {
                _writeLocker.Enter();

                if (State == ChannelState.ACTIVE)
                {
                    info.MaxFragmentSize = m_ChannelInfo.MaxFragmentSize;
                    info.MaxOutputBuffers = m_ChannelInfo.MaxOutputBuffers;
                    info.GuaranteedOutputBuffers = m_ChannelInfo.GuaranteedOutputBuffers;
                    info.NumInputBuffers = m_ChannelInfo.NumInputBuffers;
                    info.PingTimeout = m_ChannelInfo.PingTimeout;
                    info.ClientToServerPings = m_ChannelInfo.ClientToServerPings;
                    info.ServerToClientPings = m_ChannelInfo.ServerToClientPings;
                    info.SysSendBufSize = m_ChannelInfo.SysSendBufSize;
                    info.SysRecvBufSize = m_ChannelInfo.SysRecvBufSize;
                    info.CompressionType = m_ChannelInfo.CompressionType;
                    info.CompressionThresHold = m_ChannelInfo.CompressionThresHold;
                    info.PriorityFlushStrategy = m_ChannelInfo.PriorityFlushStrategy;
                    info.ComponentInfoList = m_ChannelInfo.ComponentInfoList;
                    info.ClientIP = m_ChannelInfo.ClientIP;
                    info.ClientHostname = m_ChannelInfo.ClientHostname;

                    if(SslStream != null)
                    {
                        info.EncryptionProtocol = SslStream.SslProtocol;
                    }
                    else
                    {
                        info.EncryptionProtocol = System.Security.Authentication.SslProtocols.None;
                    }
                }
                else
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = "channel not in active state"
                    };

                    ret = TransportReturnCode.FAILURE;
                }
            }
            finally
            {
                _writeLocker.Exit();
            }

            return ret;
        }

        public TransportReturnCode IOCtl(IOCtlCode code, object value, out Error error)
        {
            Debug.Assert(value != null, "value is null");

            error = null;

            TransportReturnCode retCode = TransportReturnCode.SUCCESS;

            try
            {
                _writeLocker.Enter();

                if ((State != ChannelState.ACTIVE) && (State != ChannelState.INITIALIZING))
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"channel is not in the active or initializing state"
                    };

                    return TransportReturnCode.FAILURE;
                }

                switch(code)
                {
                    case IOCtlCode.PRIORITY_FLUSH_ORDER:
                        {
                            string errorString;
                            if ((errorString = ValidatePriorityFlushOrder(value)) == null)
                            {
                                FlushOrder((string)value);
                            }
                            else
                            {
                                error = new Error
                                {
                                    Channel = this,
                                    ErrorId = TransportReturnCode.FAILURE,
                                    Text = errorString
                                };

                                retCode = TransportReturnCode.FAILURE;
                            }
                            break;
                        }
                    default:
                        {
                            error = new Error
                            {
                                Channel = this,
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = "Code is not valid."
                            };
                            retCode = TransportReturnCode.FAILURE;
                            break;
                        }
                }
            }
            finally
            {
                _writeLocker.Exit();
            }

            return retCode;
        }

        public TransportReturnCode IOCtl(IOCtlCode code, int value, out Error error)
        {
            error = null;

            TransportReturnCode retCode = TransportReturnCode.SUCCESS;

            try
            {
                _writeLocker.Enter();

                if ((State != ChannelState.ACTIVE) && (State != ChannelState.INITIALIZING))
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        Text = $"channel is not in the active or initializing state"
                    };

                    return TransportReturnCode.FAILURE;
                }

                switch(code)
                {
                    case IOCtlCode.MAX_NUM_BUFFERS:
                        {
                            /* must be at least as large as guaranteedOutputBuffers. */
                            if (value >= 0)
                            {
                                int diff = value - m_ChannelInfo.GuaranteedOutputBuffers;
                                if (diff < 0)
                                    value = m_ChannelInfo.GuaranteedOutputBuffers;
                                retCode = (TransportReturnCode)(m_ChannelInfo.MaxOutputBuffers = value);
                            }
                            else
                            {
                                error = new Error
                                {
                                    Channel = this,
                                    ErrorId = TransportReturnCode.FAILURE,
                                    Text = $"value must be (0 >= value < 2^31"
                                };

                                retCode = TransportReturnCode.FAILURE;
                            }
                            break;
                        }
                    case IOCtlCode.NUM_GUARANTEED_BUFFERS:
                        {
                            /* NUM_GUARANTEED_BUFFERS:
                             * the per channel number of guaranteedOutputBuffers that ETA will create for the client. */
                            if (value >= 0)
                            {
                                retCode = (TransportReturnCode)AdjustGuaranteedOutputBuffers(value);
                            }
                            else
                            {
                                error = new Error
                                {
                                    Channel = this,
                                    ErrorId = TransportReturnCode.FAILURE,
                                    Text = $"value must be (0 >= value < 2^31"
                                };

                                retCode= TransportReturnCode.FAILURE;
                            }
                            break;
                        }
                    case IOCtlCode.HIGH_WATER_MARK:
                        {
                            if (value >= 0)
                            {
                                m_highWaterMark = value;
                                retCode = TransportReturnCode.SUCCESS;
                            }
                            else
                            {
                                error = new Error
                                {
                                    Channel = this,
                                    ErrorId = TransportReturnCode.FAILURE,
                                    Text = $"value must be (0 >= value < 2^31"
                                };

                                retCode = TransportReturnCode.FAILURE;
                            }
                            break;
                        }
                    case IOCtlCode.SYSTEM_WRITE_BUFFERS:
                        {
                            if (value >= 0)
                            {
                                _socketChannel.Socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer, value);
                                m_ChannelInfo.SysSendBufSize = (int)_socketChannel.Socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.SendBuffer);
                            }
                            else
                            {
                                error = new Error
                                {
                                    Channel = this,
                                    ErrorId = TransportReturnCode.FAILURE,
                                    Text = $"value must be (0 >= value < 2^31"
                                };

                                retCode = TransportReturnCode.FAILURE;
                            }
                            break;
                        }
                    case IOCtlCode.SYSTEM_READ_BUFFERS:
                        {
                            if (value >= 0)
                            {
                                _socketChannel.Socket.SetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer, value);
                                m_ChannelInfo.SysRecvBufSize = (int)_socketChannel.Socket.GetSocketOption(SocketOptionLevel.Socket, SocketOptionName.ReceiveBuffer);
                            }
                            else
                            {
                                error = new Error
                                {
                                    Channel = this,
                                    ErrorId = TransportReturnCode.FAILURE,
                                    Text = $"value must be (0 >= value < 2^31"
                                };

                                retCode = TransportReturnCode.FAILURE;
                            }
                            break;
                        }
                    case IOCtlCode.COMPRESSION_THRESHOLD:
                        {
                            if (m_ChannelInfo.CompressionType != CompressionType.NONE)
                            {
                                int compressionThreshold = GetDefaultCompressionTreshold(m_ChannelInfo.CompressionType);

                                if (value >= compressionThreshold)
                                {
                                    m_ChannelInfo.CompressionThresHold = m_SessionCompLowThreshold = value;
                                }
                                else
                                {
                                    error = new Error
                                    {
                                        Channel = this,
                                        ErrorId = TransportReturnCode.FAILURE,
                                        Text = $"value must be equal to or greater than {compressionThreshold}"
                                    };

                                    retCode = TransportReturnCode.FAILURE;
                                }
                            }
                            break;
                        }
                    default:
                        {
                            error = new Error
                            {
                                Channel = this,
                                ErrorId = TransportReturnCode.FAILURE,
                                Text = "Code is not valid."
                            };
                            retCode = TransportReturnCode.FAILURE;
                            break;
                        }
                }
            }
            catch (Exception ex)
            {
                CloseFromError();

                State = ChannelState.CLOSED;

                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    Text = $"exception occurred when setting value '{value}' for IOCtlCode '{code}', exception={ex.Message}"
                };

            }
            finally
            {
                _writeLocker.Exit();
            }


            return retCode;
        }


        private static int GetDefaultCompressionTreshold(CompressionType ct) =>
            ct == CompressionType.ZLIB ? ZLIB_COMPRESSION_THRESHOLD : LZ4_COMPRESSION_THRESHOLD;

        internal void EnterReadWriteLocks()
        {
            _readLocker.Enter();
            _writeLocker.Enter();
        }

        internal void ExitReadWriteLocks()
        {
            _readLocker.Exit();
            _writeLocker.Exit();
        }

        internal TransportReturnCode InitChnlSendConnectReq(InProgInfo inProg, out Error error)
        {
            error = null;

            /* Connected Component Info */
            if(ComponentInfo.ComponentVersion.Data() == null)
            {
                try
                {
                    Transport.GlobalLocker.Enter();

                    if(m_ConnectOptsComponentInfo != null)
                    {
                        int totalLength =
                                m_ConnectOptsComponentInfo.ComponentVersion.Data().Limit + 1 + Transport.DefaultComponentVersionBuffer.Limit;
                        int origLimit = m_ConnectOptsComponentInfo.ComponentVersion.Data().Limit;
                        if (totalLength > 253)
                        {
                            // the total component data length is too long, so truncate the user defined data
                            totalLength = 253;
                            m_ConnectOptsComponentInfo.ComponentVersion.Data().Limit = 253 - Transport.DefaultComponentVersionBuffer.Limit - 1;
                        }

                        // append the user defined connect opts componentVersionInfo to the default value
                        ByteBuffer combinedBuf = new ByteBuffer(totalLength);

                        combinedBuf.Put(Transport.DefaultComponentVersionBuffer);
                        combinedBuf.Write(Transport.COMP_VERSION_DIVIDER);
                        combinedBuf.Put(m_ConnectOptsComponentInfo.ComponentVersion.Data());

                        // the combined length of the new buffer includes the user defined data, the '|', and the default component version data
                        ComponentInfo.ComponentVersion.Data(combinedBuf, 0, totalLength);

                        // reset the limit for this buffer in case it was truncated
                        m_ConnectOptsComponentInfo.ComponentVersion.Data().Limit = origLimit;
                    }
                    else
                    {
                        ComponentInfo.ComponentVersion.Data(Transport.DefaultComponentVersionBuffer, 0, Transport.DefaultComponentVersionBuffer.Limit);
                    }
                }
                finally
                {
                    Transport.GlobalLocker.Exit();
                }
            }

            m_IpcProtocol.ProtocolOptions.ComponentInfo = ComponentInfo;
            m_IpcProtocol.ProtocolOptions.ProtocolType = ProtocolType;
            m_IpcProtocol.EncodeConnectionReq(m_InitChnlWriteBuffer);
            _socketChannel.Send(m_InitChnlWriteBuffer._data, m_InitChnlWriteBuffer.Position, m_InitChnlWriteBuffer.Limit, out error);

            // Clears the read buffer prior to reading.
            m_InitChnlReadBuffer.Clear();
            InitChannelState = InitChnlState.WAIT_ACK;

            return TransportReturnCode.CHAN_INIT_IN_PROGRESS;
        }

        internal TransportReturnCode InitChnlWaitConnectAck(InProgInfo inProg, out Error error)
        {
            error = null;
            /* offset is the offset into the ByteBuffer where the RIPC message starts.
             * Will always be 0 if not tunneling. */
            int offset = 0;
            int cc = 0;

            try
            {
                cc = ProtocolFunctions.InitChnlReadFromChannel(m_InitChnlReadBuffer, out error);

                if(cc > 0)
                {
#if DEBUG
                    //System.Console.WriteLine("cc = " + cc);
#endif
                }
                else if (cc == 0)
                {
                    /* This will happen when there is no data at all to be read
                     * and the read fails with error IPC_WOULD_BLOCK or EINTR */
                    return TransportReturnCode.CHAN_INIT_IN_PROGRESS;
                }
                else
                {
                    /* This will happen if all IpcProtocols were rejected. */
                    return TransportReturnCode.FAILURE;
                }

                if(cc < IPC_100_CONN_ACK)
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = "Invalid IPC Mount Ack"
                    };

                    return TransportReturnCode.FAILURE;
                }

                int savePos = m_InitChnlReadBuffer.Position;
                TransportReturnCode retCode = m_IpcProtocol.DecodeConnectionReply(m_InitChnlReadBuffer, offset + HTTP_HEADER4, out error);
                m_InitChnlReadBuffer.ReadPosition = savePos;
                if (retCode != TransportReturnCode.SUCCESS)
                    return retCode;

                /* InternalFragmentSize is used internal to represent the size of the buffers that will be create internally.
                * The size will be RIPC MAX_USER_MSG_SIZE + RIPC_HDR_SIZE.
                * Note that this is different from _channelInfo._maxFragmentSize which is returned to
                * the user and is RIPC MAX_USER_MSG_SIZE - RIPC PACKED_HDR_SIZE.
                */
                IpcProtocolOptions protocolOptions = m_IpcProtocol.ProtocolOptions;
                InternalFragmentSize = protocolOptions.MaxUserMsgSize + RipcDataMessage.HeaderSize > 63335 ? 65535 : protocolOptions.MaxUserMsgSize + RipcDataMessage.HeaderSize;

                //_appReadBuffer = new TransportBufferImpl(_internalMaxFragmentSize);
                m_ChannelInfo.MaxFragmentSize = protocolOptions.MaxUserMsgSize - RipcDataMessage.PackedHeaderSize;
                m_ChannelInfo.PingTimeout = protocolOptions.PingTimeout;
                m_ChannelInfo.ClientToServerPings = (((protocolOptions.ServerSessionFlags & RipcSessionFlags.CLIENT_TO_SERVER_PING) == RipcSessionFlags.CLIENT_TO_SERVER_PING) ? true : false);
                m_ChannelInfo.ServerToClientPings = (((protocolOptions.ServerSessionFlags & RipcSessionFlags.SERVER_TO_CLIENT_PING) == RipcSessionFlags.SERVER_TO_CLIENT_PING) ? true : false);
                m_ChannelInfo.CompressionType = (CompressionType)protocolOptions.SessionCompType;
                m_SessionCompLevel = protocolOptions.SessionCompLevel;
                m_SessionInDecompress = protocolOptions.SessionInDecompress;
                m_SessionOutCompression = protocolOptions.SessionOutCompression;

                m_BigBuffersPool = new BigBuffersPool(InternalFragmentSize, this);

                /* Received Component Version Information */
                m_ChannelInfo.ComponentInfoList = protocolOptions.ReceivedComponentVersionList;

                // Set the shared key to 0 to cover in reconnect cases
                Shared_Key = 0;

                // allocate buffers to this channel, _availableBuffers size will initally be zero.
                GrowGuaranteedOutputBuffers(m_ChannelInfo.GuaranteedOutputBuffers);

                if (m_SessionInDecompress > RipcCompressionTypes.NONE)
                {
                    m_DecompressBuffer = new TransportBuffer(InternalFragmentSize);

                    if (m_SessionInDecompress == (CompressionType)RipcCompressionTypes.ZLIB)
                    {
                        m_Compressor = m_ZlibCompressor;
                        m_SessionCompLowThreshold = ZLIB_COMPRESSION_THRESHOLD;
                        m_Compressor.CompressionLevel  = m_SessionCompLevel;
                    }
                    else if (m_SessionInDecompress == (CompressionType)RipcCompressionTypes.LZ4)
                    {
                        m_Compressor = m_Lz4Compressor;
                        m_SessionCompLowThreshold = LZ4_COMPRESSION_THRESHOLD;
                    }

                    m_Compressor.MaxCompressionLength = InternalFragmentSize;
                }

                RipcVersion = m_IpcProtocol.RipcVersion();
                m_ReadBufferStateMachine = new ReadBufferStateMachine(this, ProtocolFunctions, RipcVersion, _socketChannel.Socket, 
                    ConnectionOptions.NumInputBuffers * InternalFragmentSize, null);

                /* If we read more bytes than in the header, put them into the read buffer.
                 * This can happen in quick applications that send data as soon as the session becomes active. */
                if (cc > m_InitChnlReadBuffer.ReadPosition)
                {
                    int putback = cc - m_InitChnlReadBuffer.ReadPosition;
                    byte[] tempBuf = new byte[putback];
                    m_InitChnlReadBuffer.ReadBytesInto(tempBuf, 0, putback);
                    m_ReadBufferStateMachine.Buffer.Clear();
                    m_ReadBufferStateMachine.Buffer.Put(tempBuf);
                }

                /* If we are doing Key exchange, send the client ack here */
                if (protocolOptions.KeyExchange == true)
                {
                    return InitChnlSendClientAck(inProg, out error);
                }
                else
                {
                    InitChannelState = InitChnlState.ACTIVE;
                    State = ChannelState.ACTIVE;
                    return TransportReturnCode.SUCCESS;
                }
            }
            catch (Exception exp)
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = exp.Message
                };

                return TransportReturnCode.FAILURE;
            }
        }

        internal TransportReturnCode InitChnlSendClientAck(InProgInfo inProg, out Error error)
        {
            error = null;

            m_InitChnlWriteBuffer.Clear();
            m_IpcProtocol.EncodeClientKey(m_InitChnlWriteBuffer, out error);

            SocketChannel.Send(m_InitChnlWriteBuffer._data, m_InitChnlWriteBuffer.BufferPosition(), m_InitChnlWriteBuffer.Limit, out error);

            if(error != null)
            {
                return error.ErrorId;
            }

            InitChannelState = InitChnlState.ACTIVE;
            State = ChannelState.ACTIVE;
            RipcVersion = m_IpcProtocol.RipcVersion();
            m_ReadBufferStateMachine = new ReadBufferStateMachine(this, ProtocolFunctions, RipcVersion, _socketChannel.Socket, 
                ConnectionOptions.NumInputBuffers * InternalFragmentSize, null);

            // set shared key - client side would be calculated here
            Shared_Key = m_IpcProtocol.ProtocolOptions.Shared_Key;

            return TransportReturnCode.SUCCESS;
        }

        internal TransportReturnCode InitChnlReadHdr(InProgInfo inProg, out Error error)
        {
            int cc;

            try
            {
                cc = ProtocolFunctions.InitChnlReadFromChannelProvider(m_InitChnlReadBuffer, out error);

                if (cc > 0)
                {
                    return InitChnlProcessHdr(inProg, out error, cc);
                }
                else if (cc == 0)
                {
                    /* This will happen when there is no data at all to be read, and the read fails with error _IPC_WOULD_BLOCK or EINTR */
                    return TransportReturnCode.CHAN_INIT_IN_PROGRESS;
                }
                else if (InitChannelState == InitChnlState.RECONNECTING)
                {
                    /* This happens when the far end (server) closes the connection and we need to try another protocol. */
                    Shared_Key = 0;
                    return TransportReturnCode.CHAN_INIT_IN_PROGRESS;
                }
                else
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = "Could not read IPC Mount Request"
                    };

                    return TransportReturnCode.FAILURE;
                }
            }
            catch (Exception exp)
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = exp.Message
                };

                return TransportReturnCode.FAILURE;
            }
        }

        internal TransportReturnCode InitChnlReadClientKey(InProgInfo inProg, out Error error)
        {
            int cc;

            try
            {
                /* This is intended to read 14 bytes in a single read.
                 * The message it should get is 4 or 12 bytes. */
                cc = ProtocolFunctions.InitChnlReadFromChannelProvider(m_InitChnlReadClientKeyBuffer, out error);
                if (cc > 0)
                {
                    TransportReturnCode retVal = 0;
                    retVal = InitChnlProcessClientKey(inProg, out error, cc);
                    // In case we read more than the message, put it in the input buffer.
                    // Set shared key - client side would be calculated here.
                    Shared_Key = m_IpcProtocol.ProtocolOptions.Shared_Key;
                    if (cc > m_InitChnlReadClientKeyBuffer.ReadPosition)
                    {
                        int putback = cc - m_InitChnlReadClientKeyBuffer.ReadPosition;
                        byte[] tempBuf = new byte[putback];
                        m_InitChnlReadClientKeyBuffer.ReadBytesInto(tempBuf, 0, putback);
                        m_ReadBufferStateMachine.Buffer.Clear();
                        m_ReadBufferStateMachine.Buffer.Put(tempBuf);
                    }

                    return retVal;
                }
                else if (cc == 0)
                {
                    /* This will happen when there is no data at all to be read, and the read fails with error _IPC_WOULD_BLOCK or EINTR */
                    return TransportReturnCode.CHAN_INIT_IN_PROGRESS;
                }
                else if (InitChannelState == InitChnlState.RECONNECTING)
                {
                    /* This happens when the far end (server) closes the connection and we need to try another protocol. */
                    Shared_Key = 0;
                    return TransportReturnCode.CHAN_INIT_IN_PROGRESS;
                }
                else
                {
                    error = new Error
                    {
                        Channel = this,
                        ErrorId = TransportReturnCode.FAILURE,
                        SysError = 0,
                        Text = "Could not read IPC Mount Request"
                    };

                    return TransportReturnCode.FAILURE;
                }
            }
            catch (Exception exp)
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = exp.Message
                };

                return TransportReturnCode.FAILURE;
            }
        }

        internal TransportReturnCode InitChnlProcessClientKey(InProgInfo inProg, out Error error, int cc)
        {
            error = null;

            int offset = 0;
            if (cc < 4)
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = $"Invalid mount request size <{cc}>"
                };

                return TransportReturnCode.FAILURE;
            }

            TransportReturnCode retval = m_IpcProtocol.DecodeClientKey(m_InitChnlReadClientKeyBuffer, offset, out error);
            if (retval != TransportReturnCode.SUCCESS)
                return retval;

            InitChannelState = InitChnlState.ACTIVE;
            State = ChannelState.ACTIVE;
            RipcVersion = m_IpcProtocol.RipcVersion();
            m_ReadBufferStateMachine = new ReadBufferStateMachine(this, ProtocolFunctions, RipcVersion, _socketChannel.Socket, 
                m_ChannelInfo.NumInputBuffers * InternalFragmentSize, null);

            return TransportReturnCode.SUCCESS;
        }

        internal TransportReturnCode InitChnlProcessHdr(InProgInfo inProg, out Error error, int cc)
        {
            error = null;

            /* use a long to hold versionNumber as an unsigned int */
            int versionNumber;
            int length = 0;
            int opCode = 0;

            if (cc < 7)
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = $"Invalid mount request size <{cc}>"
                };

                return TransportReturnCode.FAILURE;
            }

            int pos = 0;

            /* InitChnlReadFromChannel will guarantee that length == cc */
            length = m_InitChnlReadBuffer.ReadUShortAt(pos);
            opCode = m_InitChnlReadBuffer.ReadByteAt(pos + 2);

            /* read and convert from a signed to unsigned int (long) */
            versionNumber = m_InitChnlReadBuffer.ReadIntAt(pos + 3);

            IpcProtocol p = m_IpcProtocolManager.DetermineProtocol(this, (ConnectionsVersions)versionNumber);
            if (p != null)
            {
                m_IpcProtocol = p;
                m_IpcProtocol.Options(m_ServerImpl.BindOptions);
                m_IpcProtocol.ProtocolOptions.ProtocolType = ProtocolType;
            }
            else
            {
                error = new Error
                {
                    Channel = null,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = $"Unsupported RIPC version number <{versionNumber}"
                };

                return TransportReturnCode.FAILURE;
            }

            TransportReturnCode retval = m_IpcProtocol.DecodeConnectionReq(m_InitChnlReadBuffer, pos, length, out error);
            if (retval != TransportReturnCode.SUCCESS)
                return retval;

            IpcProtocolOptions protocolOptions = m_IpcProtocol.ProtocolOptions;
            InternalFragmentSize = protocolOptions.MaxUserMsgSize + RipcDataMessage.HeaderSize > 63335 ? 65535 : protocolOptions.MaxUserMsgSize + RipcDataMessage.HeaderSize;

            m_BigBuffersPool = new BigBuffersPool(InternalFragmentSize, this);

            /* allocate buffers to this channel */
            GrowGuaranteedOutputBuffers(m_ServerImpl.BindOptions.GuaranteedOutputBuffers);
            m_ChannelInfo.MaxFragmentSize = protocolOptions.MaxUserMsgSize - RipcDataMessage.PackedHeaderSize;
            m_SessionOutCompression = protocolOptions.SessionOutCompression;
            m_SessionInDecompress = m_SessionOutCompression;

            /* Compression both directions */
            if (m_SessionInDecompress > RipcCompressionTypes.NONE)
            {
                m_DecompressBuffer = new TransportBuffer(InternalFragmentSize);

                if (m_SessionInDecompress == (CompressionType)RipcCompressionTypes.ZLIB)
                {
                    m_Compressor = m_ZlibCompressor;
                    m_SessionCompLowThreshold = ZLIB_COMPRESSION_THRESHOLD;
                }
                else if (m_SessionInDecompress == (CompressionType)RipcCompressionTypes.LZ4)
                {
                    m_Compressor = m_Lz4Compressor;
                    m_SessionCompLowThreshold = LZ4_COMPRESSION_THRESHOLD;
                }

                m_Compressor.CompressionLevel = m_SessionCompLevel;

                m_Compressor.MaxCompressionLength = InternalFragmentSize;
            }

            /* Negotiate the ping timeout between the server's values (pingTimeout
             * and minPingTimeout) and what the client sent in the RIPC ConnectReq.
             * The server's pingTimeout is the maximum allowed pingTimeout.
             * The server's minPingTimeout is the minimum allowed pingTimeout.
             * If the client's pingTimeout is within the server's pingTimeout and
             * minPingTimout, the client's pingTimeout will be used.
             * Otherwise, the server's max or min value will be used.
             */
            int clientPingTimeout = protocolOptions.PingTimeout;
            if (clientPingTimeout > m_ServerImpl.BindOptions.PingTimeout)
            {
                /* client's pingTimout was larger than server's (max) pingTimeout */
                m_ChannelInfo.PingTimeout = m_ServerImpl.BindOptions.PingTimeout;
                protocolOptions.PingTimeout = m_ServerImpl.BindOptions.PingTimeout;
            }
            else if (clientPingTimeout < m_ServerImpl.BindOptions.MinPingTimeout)
            {
                /* client's pingTimeout was smaller than server's minPingTimeout */
                m_ChannelInfo.PingTimeout = m_ServerImpl.BindOptions.MinPingTimeout;
                protocolOptions.PingTimeout = m_ServerImpl.BindOptions.MinPingTimeout;
            }
            else
            {
                /* client's pingTimeout is within the server's max and min pingTimeout */
                m_ChannelInfo.PingTimeout = clientPingTimeout;
                protocolOptions.PingTimeout = clientPingTimeout;
            }

            m_ChannelInfo.ClientToServerPings =
                    (((protocolOptions.ServerSessionFlags & RipcSessionFlags.CLIENT_TO_SERVER_PING) == RipcSessionFlags.CLIENT_TO_SERVER_PING) ? true : false);
            m_ChannelInfo.ServerToClientPings =
                    (((protocolOptions.ServerSessionFlags & RipcSessionFlags.SERVER_TO_CLIENT_PING) == RipcSessionFlags.SERVER_TO_CLIENT_PING) ? true : false);

            /* validate protocolType */
            if (ProtocolType != protocolOptions.ProtocolType)
            {
                /* cause NAK to be sent */
                NakMount = true;
            }

            /* use the smaller Major/Minor version number */
            if (protocolOptions.MajorVersion < MajorVersion)
            {
                /* client major version is smaller than ours */
                MajorVersion = protocolOptions.MajorVersion;
                MinorVersion = protocolOptions.MinorVersion;
            }
            else if (protocolOptions.MajorVersion == MajorVersion)
            {
                if (protocolOptions.MinorVersion < MinorVersion)
                {
                    /* client major version matches ours, however client minor version is smaller than ours, use client version */
                    MinorVersion = protocolOptions.MinorVersion;
                }
            }
            /* else we leave the versions alone as ours are smaller */

            /* get host name and ip address */
            m_ChannelInfo.ClientHostname = protocolOptions.ClientHostName;
            m_ChannelInfo.ClientIP = protocolOptions.ClientIpAddress;

            /* received Component Version Information */
           m_ChannelInfo.ComponentInfoList = protocolOptions.ReceivedComponentVersionList;

            /* create read/write buffer pools */
            //CreateReadBuffers(m_ChannelInfo.NumInputBuffers);

            if (protocolOptions.KeyExchange == true)
            {
                m_InitChnlReadClientKeyBuffer.Clear(); /* Clear buffer before reading key. */
                InitChannelState = InitChnlState.WAIT_CLIENT_KEY;
            }
            else
            {
                InitChannelState = InitChnlState.COMPLETE;
            }

            return InitChnlFinishSess(inProg, out error);
        }

        internal TransportReturnCode InitChnlFinishSess(InProgInfo inProg, out Error error)
        {
            error = null;

            if(NakMount)
            {
                InitChnlRejectSession(out error);
                return TransportReturnCode.FAILURE;
            }
            else
            {
                /* Connected Component Info */
                m_IpcProtocol.ProtocolOptions.ComponentInfo = ComponentInfo;

                ByteBuffer bufferToWrite = m_IpcProtocol.EncodeConnectionAck(m_InitChnlWriteBuffer, out error);
                SocketChannel.Send(bufferToWrite._data, bufferToWrite.BufferPosition(), bufferToWrite.Limit, out error);
            }

            /* If we don't have to wait for the key to come back to us, we are active */
            if (InitChannelState == InitChnlState.COMPLETE)
            {
                InitChannelState = InitChnlState.ACTIVE;
                State = ChannelState.ACTIVE;
                RipcVersion = m_IpcProtocol.RipcVersion();
                m_ReadBufferStateMachine = new ReadBufferStateMachine(this, ProtocolFunctions, RipcVersion, _socketChannel.Socket,
                    m_ChannelInfo.NumInputBuffers * InternalFragmentSize, null);

                return TransportReturnCode.SUCCESS;
            }
            else
            {
                /* clear read buffer before reading again */
                m_InitChnlReadBuffer.Clear();
                return TransportReturnCode.CHAN_INIT_IN_PROGRESS;
            }
        }

        internal TransportReturnCode InitChnlRejectSession(out Error error)
        {
            error = null;

            ByteBuffer bufferToWrite = m_IpcProtocol.EncodeConnectionNak(m_InitChnlWriteBuffer);

            SocketChannel.Send(bufferToWrite._data, bufferToWrite.BufferPosition(), bufferToWrite.Limit, out error);

            error = new Error
            {
                Channel = this,
                ErrorId = TransportReturnCode.FAILURE,
                SysError = 0,
                Text = "Rejected the connection request by the server."
            };

            SocketChannel.Socket.Close();

            InitChannelState = InitChnlState.INACTIVE;

            return TransportReturnCode.SUCCESS;
        }

        internal bool CheckIsProviderHTTP(ByteBuffer dst)
        {
            int endPos = dst.Position;
            if (dst.Position > 4)
            {
                string startHTTP_POST_REQUEST_FROM_CLIENT = "";

                byte[] tempBuf = new byte[4];

                dst.ReadPosition = 0;
                dst.ReadBytesInto(tempBuf, 0, 4);

                dst.ReadPosition = endPos;

                startHTTP_POST_REQUEST_FROM_CLIENT = Encoding.ASCII.GetString(tempBuf);

                if (startHTTP_POST_REQUEST_FROM_CLIENT.StartsWith("POST")) // http
                    return true;
                else
                    return false;
            }

            return false;
        }

        internal TransportReturnCode InitChnlProxyConnecting()
        {
            string connectRequest = BuildHttpConnectRequest();

            Socket.Send(Encoding.ASCII.GetBytes(connectRequest));

            InitChannelState = InitChnlState.CLIENT_WAIT_PROXY_ACK;

            return TransportReturnCode.CHAN_INIT_IN_PROGRESS;
        }

        internal string BuildHttpConnectRequest()
        {
            StringBuilder connectRequest = new StringBuilder();
            connectRequest.Append(BuildHttpConnectRequestPrefix());

            if (!string.IsNullOrEmpty(AdditionalHttpConnectParams))
            {
                connectRequest.Append(AdditionalHttpConnectParams);
                AdditionalHttpConnectParams = null;
            }

            connectRequest.Append(EOL);

            return connectRequest.ToString();
        }

        /// <summary>
        /// Returns a string containing the "common" HTTP Connect request.
        /// A suffix (a trailing \r\n) must be appended to the returned value
        /// (to make it a valid HTTP request).
        /// </summary>
        /// <returns>The connect request without the trailing(\r\n)</returns>
        private string BuildHttpConnectRequestPrefix()
        {
            StringBuilder sb = new StringBuilder();

            sb.Append("CONNECT ");
            sb.Append(ConnectionOptions.UnifiedNetworkInfo.Address);
            sb.Append(":");
            sb.Append(ConnectionOptions.UnifiedNetworkInfo.Port);
            sb.Append(" HTTP/1.1\r\n");
            sb.Append(USER_AGENT);
            sb.Append(PROXY_CONNECTION_KEEP_ALIVE);
            sb.Append("Content-Length: 0\r\n");
            sb.Append("Host: ");
            sb.Append(ConnectionOptions.UnifiedNetworkInfo.Address);
            sb.Append(":");
            sb.Append(ConnectionOptions.UnifiedNetworkInfo.Port);
            sb.Append("\r\n");
            sb.Append(PRAGMA_NO_CACHE);

            // NOTE: this will *not* be a valid HTTP request until a final "\r\n" is appended to it
            return sb.ToString();
        }

        internal TransportReturnCode InitChnlWaitProxyAck(InProgInfo inProg, out Error error)
        {
            int cc;

            cc = InitProxyReadFromChannel(m_InitChnlReadBuffer, out error);

            if (cc == (int)TransportReturnCode.FAILURE)
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = "Could not read HTTP OK (reply to HTTP CONNECT)"
                };
            }

            // do authentication handling
            ReadHttpConnectResponse(m_InitChnlReadBuffer, inProg, out error);
            m_InitChnlReadBuffer.Clear();

            error = null;
            return TransportReturnCode.SUCCESS;
        }

        private int InitProxyReadFromChannel(ByteBuffer dest, out Error error)
        {
            error = null;
            dest.Clear(); // needed for recovery through a proxy

            // Proxy interactions are not encrypted, so do direct write to the socket.
            int bytesRead = Socket.Receive(dest.Contents);
            dest.WritePosition = bytesRead;

            if (bytesRead > 0)
            {
                if(dest.Position > 2)
                {
                    int messageLength = (dest.ReadShortAt(0) & 0xFF);
                    if(dest.Position >= messageLength)
                    {
                        // We have at least one complete message.
                        return dest.Position;
                    }
                }
            }
            else if (bytesRead == -1)
            {
                error = new Error
                {
                    Channel = this,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = "Proxy has cut the connection"
                };

                return (int)TransportReturnCode.FAILURE;

            }

            // we don't have a complete message, or no bytes were read.
            return 0;
        }

        private void ReadHttpConnectResponse(ByteBuffer reader, InProgInfo inProg, out Error error)
        {
            int responseSize = reader.Position;
            reader.Flip();

            try
            {
                byte[] tempBuf = new byte[responseSize];
                reader.ReadBytesInto(tempBuf, 0, responseSize);

                string response = Encoding.ASCII.GetString(tempBuf);

                m_ProxyConnectResponse.Append(response); // used to combine "incomplete" responses from the proxy.

                if (!m_ProxyAuthenticator.IsAuthenticated && m_ProxyConnectResponse.ToString().Contains(END_OF_RESPONSE))
                {
                    // process the response to determine whether or not we are authenticated
                    ProxyAuthenticatorResponse authenticatorResponse = null;

                    try
                    {
                        authenticatorResponse = m_ProxyAuthenticator.ProcessResponse(m_ProxyConnectResponse.ToString());
                    }
                    catch (ResponseCodeException)
                    {
                        ++m_IgnoredConnectResponse;
                        if (m_IgnoredConnectResponse < MAX_IGNORED_RESPONSES)
                        {
                            error = null;
                            return;
                        }
                        else
                        {
                            throw; // too many ignored response
                        }
                    }

                    if (m_ProxyAuthenticator.IsAuthenticated)
                    {
                        // Connection established to proxy..
                    }
                    else
                    {
                        // this will be added to the next http CONNECT message:
                        AdditionalHttpConnectParams = authenticatorResponse.ProxyAuthorization;

                        if (authenticatorResponse.IsProxyConnectionClose)
                        {
                            ForceReconnect(inProg, out error);
                        }
                        else
                        {
                            string connectRequest = BuildHttpConnectRequest();

                            Socket.Send(Encoding.ASCII.GetBytes(connectRequest));
                        }
                    }

                    m_ProxyConnectResponse.Length = 0; // we are done with the current response from the proxy.
                }
            }
            catch(ProxyAuthenticationException)
            {
                throw;
            }
            catch(Exception)
            {
                throw;
            }

            error = null;
        }

        private void ForceReconnect(InProgInfo inProg, out Error error)
        {
            // Close the current socket.
            try
            {
                _socketChannel.Disconnect();
            }
            catch(Exception )
            {
            }

            inProg.OldSocket = _socketChannel.Socket;
            m_IpcProtocol = null;
            Connect(out error);

            inProg.Flags = InProgFlags.SCKT_CHNL_CHANGE;
            inProg.NewSocket = _socketChannel.Socket;
        }

        internal TransportBuffer GetBufferInternal(int size, bool packedBuffer, int headerLength)
        {
            // This method should be called when the buffer size + header is less then fragment size.
            // The calling method chain should have lock set.
            TransportBuffer buffer = null;

            if (m_CurrentBuffer != null)
            {
                buffer = m_CurrentBuffer.GetBufferSlice(size, packedBuffer, headerLength);
                if (buffer == null)
                {
                    SocketBuffer socketBuffer = m_CurrentBuffer;
                    if (socketBuffer.m_SlicesPool.AreAllSlicesBack())
                    {
                        m_CurrentBuffer = null;
                        SocketBufferToRecycle(socketBuffer);
                    }
                }
            }

            if (buffer is null)
            {
                m_CurrentBuffer = (SocketBuffer)m_AvailableBuffers.Poll();
                if (m_CurrentBuffer == null)
                {
                    /* This is Server's channel so just try to get from the shared pool if any. */
                    if (m_ServerImpl != null && m_Used < m_ChannelInfo.MaxOutputBuffers)
                    {
                        m_CurrentBuffer = m_ServerImpl.GetBufferFromServerPool();
                    }
                }

                if (m_CurrentBuffer != null)
                {
                    m_CurrentBuffer.Clear();
                    ++m_Used;
                    buffer = m_CurrentBuffer.GetBufferSlice(size, packedBuffer, headerLength);
                }
            }

            if (buffer != null)
            {
                buffer.IsOwnedByApp = true;
                Debug.Assert(buffer.Next == null);
            }

            return buffer;
        }

        internal void ReleaseBufferInternal(ITransportBuffer buffer)
        {
            // This method is used by write call, which is already guarded by lock.
            TransportBuffer transportBuffer = buffer as TransportBuffer;
            transportBuffer.IsOwnedByApp = false;
            if (!m_IsJunitTest)
            {
                if (!transportBuffer.IsBigBuffer && ((SocketBuffer)transportBuffer.Pool.PoolOwner).Pool.IsSharedPoolBuffer)
                {
                    if (buffer != m_CurrentBuffer)
                    {
                        transportBuffer.ReturnToPool();
                        if (m_Used > 0)
                        {
                            --m_Used;
                        }
                    }
                }
                else
                {
                    transportBuffer.ReturnToPool();
                }
            }
            else
            {
                transportBuffer.ReturnToPool();
            }
        }

        // This is used for unit testing to override the starting connection version
        internal void StartingConnectVersion(ConnectionsVersions connectionsVersion)
        {
            m_IpcProtocolManager.SetStartingRipcVersion(connectionsVersion);
        }

        private static string ValidatePriorityFlushOrder(Object value)
        {
            if (value != null && (value is String))
            {
                string valueStr = (string)value;
                char[] charArray = valueStr.ToCharArray();
                int length = charArray.Length;
                if (length <= 32)
                {
                    // ensure at least one 'H' and one 'M' is specified.
                    // without incurring GC.
                    bool hasH = false, hasM = false;
                    for (int i = 0; i<length; i++)
                    {
                        if (charArray[i] == 'H')
                            hasH = true;
                        else if (charArray[i] == 'M')
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

        private int AdjustGuaranteedOutputBuffers(int value)
        {
            int diff = value - m_ChannelInfo.GuaranteedOutputBuffers;
            if (diff > 0)
            {
                /* If guaranteedOutputBuffers grow larger than maxOutputBuffers, adjust maxOutputBuffers. */
                m_ChannelInfo.GuaranteedOutputBuffers += GrowGuaranteedOutputBuffers(diff);
                if (m_ChannelInfo.MaxOutputBuffers < m_ChannelInfo.GuaranteedOutputBuffers)
                    m_ChannelInfo.MaxOutputBuffers = m_ChannelInfo.GuaranteedOutputBuffers;

                return m_ChannelInfo.GuaranteedOutputBuffers;
            }
            else if (diff < 0)
            {
                // shrink buffers
                return m_ChannelInfo.GuaranteedOutputBuffers -= ShrinkGuaranteedOutputBuffers(diff * -1);
            }
            else
            {
                // nothing changed.
                return m_ChannelInfo.GuaranteedOutputBuffers;
            }
        }

        internal int GrowGuaranteedOutputBuffers(int numToGrow)
        {
            try
            {
                Transport.GlobalLocker.Enter();

                Pool bufferPool = null;

                int numToCreate = 0;
                bufferPool = m_SocketProtocol.GetPool(InternalFragmentSize);
                numToCreate = numToGrow - bufferPool.Poll(m_AvailableBuffers, numToGrow);

                if (numToCreate > 0)
                {
                    // need to create more buffers and add to the m_AvailableBuffers pool
                    SocketBuffer buffer;
                    for (int i = 0; i < numToCreate; i++)
                    {

                        buffer = new SocketBuffer(m_AvailableBuffers, InternalFragmentSize);

                        buffer.ReturnToPool();
                    }
                }
                return numToGrow;
            }
            finally
            {
                Transport.GlobalLocker.Exit();
            }
        }

        internal int ShrinkGuaranteedOutputBuffers(int numToShrink)
        {
            try
            {
                Transport.GlobalLocker.Enter();
                Pool bufferPool = m_SocketProtocol.GetPool(InternalFragmentSize);
                return bufferPool.Add(m_AvailableBuffers, numToShrink);
            }
            finally
            {
                Transport.GlobalLocker.Exit();
            }
        }
    }
}
