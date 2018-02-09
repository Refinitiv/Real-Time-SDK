package com.thomsonreuters.upa.transport;

import java.io.BufferedWriter;
import java.io.FileWriter;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.HashMap;
import java.util.Map;

import com.thomsonreuters.upa.transport.SocketProtocol.TrackingPool;

class RsslHttpSocketChannelProvider
{
    RsslSocketChannel rsslSocketChannel;
    protected ByteBuffer _httpBuffer = ByteBuffer.allocateDirect(5);

    byte[] _httpSessionIDbytes = new byte[4];
    boolean _isProvider = false;
    int HTTP_HEADER3 = 3;
    int HTTP_HEADER6 = 6;
    int CHUNKEND_SIZE = 2;

    protected ByteBuffer _pingWriteBuffer = ByteBuffer.allocateDirect(RsslSocketChannel.PING_BUFFER_SIZE + HTTP_HEADER3 + CHUNKEND_SIZE);

    protected ByteBuffer _pipeBuffer = ByteBuffer.allocateDirect(1);

    boolean _reConnFromPOST = false;
    boolean _wininetStream = false;
    boolean _wininetControl = false;
    boolean _javaSession = false;
    int _commonSessionId = -1;
    java.nio.channels.SocketChannel _outboundStreamingChannel;

    int _newControlSessionId;
    int _newStreamingSessionId;
    boolean _streamingChannelCreated = false;

    boolean _needCloseOldChannel = false;

    boolean _httpPOSTRead = false;
    boolean _opcodeRead = false;
    boolean _ripcMsgPending = false;

    int _wininetSessionThreshold = 10;
    int _socketSessionThreshold = 20;

    static final int RIPC_JAVA_WITH_HTTP_TUNNELING = 0x80;

    static final int RIPC_JAVA_WITH_HTTP_TUNNELING_RECONNECT = 0x84;

    static final int RIPC_WININET_STREAMING = 0x40;

    static final int RIPC_WININET_CONTROL = 0x41;

    static final int RIPC_WININET_STREAMING_RECONNECT = 0x42;

    static final int RIPC_WININET_CONTROL_RECONNECT = 0x43;

    static Map<Integer, java.nio.channels.SocketChannel> _sessionIdSocketMap = new HashMap<Integer, java.nio.channels.SocketChannel>();

    static Map<Integer, RsslSocketChannel> _sessionMap = new HashMap<Integer, RsslSocketChannel>();

    StringBuffer _http200Response;
    StringBuffer _http200ResponseReconn;

    BufferedWriter _out = null;
    FileWriter fstream;

    class StreamingSess
    {
        Integer _controlId;
        Integer _currentStreamingId;
        Integer _newStreamingId;
        Integer _newControlId;
        PipeNode _pipeNode;
    }

    static Map<Integer, StreamingSess> _wininetStreamingMap = new HashMap<Integer, StreamingSess>();

    static final boolean debugPrint = "true".equals(System.getProperty("httpDebug"));

    class PipeNode extends UpaNode
    {
        java.nio.channels.Pipe _pipe;

        PipeNode(Pool pool)
        {
            pool(pool);
        }

        PipeNode()
        {
        }

        @Override
        void returnToPool()
        {
            if (!_inPool)
            {
                _pipePool.add(this);
            }
        }
    }

    static Pool _pipePool = new Pool(RsslSocketChannel.class);
    PipeNode _pipeNode;

    protected RsslHttpSocketChannelProvider(RsslSocketChannel rsslSocketChannel)
    {
        this.rsslSocketChannel = rsslSocketChannel;
        setupPingWriteBuffer();
        setupHttpBuffer();
        _pipeBuffer.put((byte)0);
        _newControlSessionId = -1;
        _newStreamingSessionId = -1;
        _opcodeRead = false;
        _httpPOSTRead = false;
        _ripcMsgPending = false;
        _reConnFromPOST = false;
        _wininetStream = false;
        _wininetControl = false;
        _streamingChannelCreated = false;
        _needCloseOldChannel = false;

        _http200Response = new StringBuffer("HTTP/1.1 200 OK\r\n");
        _http200Response.append("Transfer-Encoding: chunked\r\n");
        _http200Response.append("Content-Type: application/octet-stream\r\n");
        _http200Response.append("\r\n");
        _http200Response.append("7\r\n"); // 3 + 7 + 2 "7" here becomes acsii

        _http200ResponseReconn = new StringBuffer("HTTP/1.1 200 OK\r\n");
        _http200ResponseReconn.append("Transfer-Encoding: chunked\r\n");
        _http200ResponseReconn.append("Content-Type: application/octet-stream\r\n");
        _http200ResponseReconn.append("\r\n");
        _http200ResponseReconn.append("1\r\n"); // 3 + 7 + 2 "7" here becomes acsii
    }

    protected int initChnlHandlePost(InProgInfo inProg, int bytesRead, ByteBuffer readBuffer, ByteBuffer writeBuffer, Error error)
            throws IOException
    {
        int status = initChnlReadPOSTFromChannel(inProg, bytesRead, readBuffer, error);

        if (status == TransportReturnCodes.FAILURE)
        {
            error.channel(rsslSocketChannel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Could not read HTTP OK (reply to HTTP CONNECT)");
            rsslSocketChannel._needCloseSocket = true;
            closeStreamingSocket();
            return TransportReturnCodes.FAILURE;
        }
        // 13 bytes not arrived yet
        if (status == TransportReturnCodes.CHAN_INIT_IN_PROGRESS)
            return status;

        if (!_reConnFromPOST)
        {
            if (!_wininetControl)
            {
                // java, wininet streaming
                writeBuffer.clear();
                if (setupPOSThttpResponse(writeBuffer, rsslSocketChannel._providerSessionId) == TransportReturnCodes.FAILURE)
                {
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Could not set HTTP OK response (reply to HTTP CONNECT)");
                    rsslSocketChannel._needCloseSocket = true;
                    closeStreamingSocket();
                    return TransportReturnCodes.FAILURE;
                }
                try
                {
                    rsslSocketChannel._scktChannel.write(writeBuffer);
                    if (debugPrint)
                        System.out.println(" Send POST ack back to Consumer");
                }
                catch (Exception e)
                {
                    if (debugPrint)
                        System.out.println(" Exception in POST ack back to Consumer");
                    rsslSocketChannel._needCloseSocket = true;
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Could not write POST ack back...");
                    closeStreamingSocket();
                    return TransportReturnCodes.FAILURE;
                }
            }
        }
        else
        // reconnect
        {
            if (!_wininetControl)
            {
                if (_wininetStream)
                {
                    writeBuffer.clear();
                    if (setupServerReconnectionPOSTACKresponseWininet(writeBuffer) == TransportReturnCodes.FAILURE)
                    {
                        error.channel(rsslSocketChannel);
                        error.errorId(TransportReturnCodes.FAILURE);
                        error.sysError(0);
                        error.text("Could not set HTTP OK response (reply to HTTP CONNECT)");
                        rsslSocketChannel._needCloseSocket = true;
                        closeStreamingSocket();
                        return TransportReturnCodes.FAILURE;
                    }
                }
                else
                {// java
                    writeBuffer.clear();
                    if (setupServerReconnectionPOSTACKresponse(writeBuffer) == TransportReturnCodes.FAILURE)
                    {
                        error.channel(rsslSocketChannel);
                        error.errorId(TransportReturnCodes.FAILURE);
                        error.sysError(0);
                        error.text("Could not set HTTP OK response (reply to HTTP CONNECT)");
                        rsslSocketChannel._needCloseSocket = true;
                        return TransportReturnCodes.FAILURE;
                    }
                }

                _reConnFromPOST = false;

                try
                {
                    rsslSocketChannel._scktChannel.write(writeBuffer);
                    if (debugPrint)
                        System.out.println(" Reconnect, wrote the POST ACK back to Consumer");
                }
                catch (Exception e)
                {
                    rsslSocketChannel._needCloseSocket = true;
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Could not write POST ack back..");
                    closeStreamingSocket();
                    return TransportReturnCodes.FAILURE;
                }
            }
            else
            // wininet control
            {
                writeBuffer.clear();
                if (setupServerReconnectionPOSTACKresponse(writeBuffer) == TransportReturnCodes.FAILURE)
                {
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Could not set HTTP OK response (reply to HTTP CONNECT)");
                    rsslSocketChannel._needCloseSocket = true;
                    closeStreamingSocket();
                    return TransportReturnCodes.FAILURE;
                }

                java.nio.channels.SocketChannel newStreamingChannel = getPairingStreamingChannel();

                try
                {
                    if (debugPrint)
                        System.out.println(" new Control ACK on new Streaming socket = " + newStreamingChannel.toString());

                    if (newStreamingChannel != null)
                        newStreamingChannel.write(writeBuffer);
                    else
                    {
                        error.channel(rsslSocketChannel);
                        error.errorId(TransportReturnCodes.FAILURE);
                        error.sysError(0);
                        error.text("Could not response to HTTP OK response (reply to HTTP CONNECT)");
                        rsslSocketChannel._needCloseSocket = true;
                        closeStreamingSocket();
                        return TransportReturnCodes.FAILURE;
                    }
                }
                catch (Exception e)
                {
                    rsslSocketChannel._needCloseSocket = true;
                    closeStreamingSocket();
                }
            }
        }

        writeBuffer.clear();

        return status;
    }

    /* Read a POST request from consumer */
    protected int initChnlReadPOSTFromChannel(InProgInfo inProg, int bytesRead, ByteBuffer reader, Error error) throws IOException
    {
        int tmpPos = reader.position();
        reader.position(0);
        // HTTP headers should end in 0x0D0A0D0A (/r/n/r/n)
        boolean endOfAllHttpHeaders = false;
        while (!endOfAllHttpHeaders && (reader.hasRemaining()))
            if (reader.get() == 0x0D)
                if (reader.get() == 0x0A) // end of an HTTP header
                    if (reader.getShort() == 0x0D0A) // end of all the HTTP headers
                    {
                        endOfAllHttpHeaders = true;
                    }

        // after the HTTP headers for POST we might have the 13 byte message
        // read 1-byte opCode for RIPC_JAVA_WITH_HTTP_TUNNELING (0x60)
        // or for RIPC_JAVA_WITH_HTTP_TUNNELING_RECONNECT (0x64)
        // or any WININET (0x40, 0x41, 0x42, 0x43)
        if ((tmpPos - reader.position()) >= 13)
        {
            if (!_opcodeRead)
            {
                if (debugPrint)
                    System.out.println("Read in seperate 13 bytes");
            }

            int status1 = parsePOSThttpRequest(inProg, tmpPos, reader, error);

            if (!_ripcMsgPending)
                rsslSocketChannel._initChnlReadBuffer.clear();
            return status1;
        }
        else
        {
            if (debugPrint)
                System.out.println("no 13Bytes yet");

            reader.position(tmpPos);
            return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
        }
    }

    /* Parse "HTTP OK" response to "POST HTTP" */
    private int parsePOSThttpRequest(InProgInfo inProg, int bytesRead, ByteBuffer reader, Error error)
    {
        extraCleanup();
        extraCleanup2();
        if (debugPrint)
            System.out.println("============== SocketServer adding channel in Provider, id =  " + rsslSocketChannel._providerSessionId +
                               " sock = " + rsslSocketChannel._scktChannel.toString());

        short opCode = 0;
        try
        {
            reader.get();
            reader.get();
            byte opCode0 = reader.get();
            opCode = (short)(opCode0 & 0x000000ff); // opCode possibilities are below 0xff=255

            if (debugPrint)
                System.out.println(" HTTP Provider got opCode = " + opCode);
        }
        catch (Exception e)
        {
            System.out.println("Provider cannot read opCode.");
        }

        if (opCode == 0)
        {
            return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
        }

        _sessionMap.put(rsslSocketChannel._providerSessionId, rsslSocketChannel);
        _sessionIdSocketMap.put(rsslSocketChannel._providerSessionId, rsslSocketChannel._scktChannel);

        switch (opCode)
        {
            case RIPC_JAVA_WITH_HTTP_TUNNELING:
            {
                if (debugPrint)
                    System.out.println("Provider tunneling, provider side session id = " + rsslSocketChannel._providerSessionId);

                _opcodeRead = true;
                _javaSession = true;
                return TransportReturnCodes.SUCCESS;
            }
            case RIPC_WININET_STREAMING:
            {
                if (debugPrint)
                    System.out.println("Provider got winnet stream channel setup, session id = " + rsslSocketChannel._providerSessionId);

                _wininetStream = true;
                _wininetControl = false;
                _commonSessionId = rsslSocketChannel._providerSessionId;
                _opcodeRead = true;

                if (debugPrint)
                    System.out.println("_sessionMap = " + _sessionMap);
                if (debugPrint)
                    System.out.println("_sessionIdSocketMapSize = " + _sessionIdSocketMap.size() + "_sessionIdSocketMap = " + _sessionIdSocketMap);
                if (debugPrint)
                    System.out.println("__wininetStreamingMap Size = " + _wininetStreamingMap.size());

                PipeNode pipeNode = null;
                try
                {
                    pipeNode = (PipeNode)_pipePool.poll();
                    if (pipeNode == null)
                    {
                        pipeNode = new PipeNode(_pipePool);
                    }
                    else if (pipeNode._pipe.sink().isOpen() || pipeNode._pipe.source().isOpen())
                    {
                    	pipeNode._pipe.source().close();
                    	pipeNode._pipe.sink().close();
                        pipeNode.returnToPool();
                        pipeNode = new PipeNode(_pipePool);
                    }

                    pipeNode._pipe = java.nio.channels.Pipe.open();

                    ((InProgInfoImpl)inProg).oldSelectableChannel(rsslSocketChannel._scktChannel);
                    ((InProgInfoImpl)inProg).flags(InProgFlags.SCKT_CHNL_CHANGE);
                    pipeNode._pipe.source().configureBlocking(false);
                    pipeNode._pipe.sink().configureBlocking(false);
                    ((InProgInfoImpl)inProg).newSelectableChannel(pipeNode._pipe.source());
                    _pipeNode = pipeNode;
                }
                catch (IOException e)
                {
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("HTTP Provider cannot create internal pipe.");
                    System.out.println("Provider cannot create socket");
                    return TransportReturnCodes.FAILURE;
                }

                StreamingSess sess;
                if (_wininetStreamingMap.containsKey(rsslSocketChannel._providerSessionId))
                {
                    sess = (StreamingSess)_wininetStreamingMap.get(rsslSocketChannel._providerSessionId);
                }
                else
                {
                    sess = new StreamingSess();
                    _wininetStreamingMap.put(rsslSocketChannel._providerSessionId, sess);
                }
                sess._pipeNode = pipeNode;

                return TransportReturnCodes.SUCCESS;
            }
            case RIPC_WININET_CONTROL:
            {
                _wininetControl = true;
                _wininetStream = false;

                _commonSessionId = reader.getInt();
                if (debugPrint)
                    System.out.println("Provider got winnet control channel setup, session id = " + rsslSocketChannel._providerSessionId +
                                       " pair sessionId = " + _commonSessionId);

                if (!_sessionMap.containsKey(_commonSessionId))
                {
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Streaming session not exist before control session is estabished");
                    return TransportReturnCodes.FAILURE;
                }

                StreamingSess sess = null;
                if (_wininetStreamingMap.containsKey(_commonSessionId))
                {
                    sess = (StreamingSess)_wininetStreamingMap.get(_commonSessionId);
                    sess._controlId = rsslSocketChannel._providerSessionId;
                }
                else
                {
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Streaming session not exist before control session is estabished...");
                    return TransportReturnCodes.FAILURE;
                }

                _opcodeRead = true;

                _ripcMsgPending = false;

                // 2 bytes processId and 4 bytes IP address
                if ((reader.position() + 6) < bytesRead)
                {
                    if (debugPrint)
                        System.out.println(" More msgs in the buffer,  current pos = " + reader.position() + " inBytesNum = " + bytesRead);
                    reader.position(reader.position() + 6);
                    _ripcMsgPending = true;
                }

                if (debugPrint)
                    System.out.println("_sessionMap = " + _sessionMap);
                if (debugPrint)
                    System.out.println("_sessionIdSocketMapSize = " + _sessionIdSocketMap.size() + "_sessionIdSocketMap = " + _sessionIdSocketMap);
                if (debugPrint)
                    System.out.println("__wininetStreamingMap Size = " + _wininetStreamingMap.size());

                RsslSocketChannel streamingRsslChannel = _sessionMap.get(_commonSessionId);
                if (!streamingRsslChannel._nakMount && !rsslSocketChannel._nakMount)
                    streamingRsslChannel._needCloseSocket = false;

                // moved from above streaming channel creation
                streamingRsslChannel._providerHelper._streamingChannelCreated = true;

                if (sess._pipeNode == null)
                {
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Pipe is null for streaming channel.");
                    return TransportReturnCodes.FAILURE;
                }
                try
                {
                    _pipeBuffer.flip();
                    int numBytes = sess._pipeNode._pipe.sink().write(_pipeBuffer);
                    if (debugPrint)
                        System.out.println(" Write 1 bytes to PIPE numBytes = " + numBytes);
                }
                catch (Exception e)
                {
                    e.printStackTrace();
                }
                return TransportReturnCodes.SUCCESS;
            }
            case RIPC_JAVA_WITH_HTTP_TUNNELING_RECONNECT:
            {
                _reConnFromPOST = true;
                _opcodeRead = true;
                _javaSession = true;
                rsslSocketChannel._needCloseSocket = false;
                _commonSessionId = reader.getInt();

                if (debugPrint)
                    System.out.println(" Provider receives Java reconnect request with base sessionId of " + _commonSessionId);

                if (!_sessionMap.containsKey(_commonSessionId))
                {
                    rsslSocketChannel._needCloseSocket = true;
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Streaming channel was removed");
                    return TransportReturnCodes.FAILURE;
                }

                RsslSocketChannel oldChannel = (RsslSocketChannel)_sessionMap.get(_commonSessionId);

                rsslSocketChannel._oldScktChannel = oldChannel._scktChannel;

                oldChannel._providerHelper._newControlSessionId = rsslSocketChannel._providerSessionId;

                handleOldJavaChannel(rsslSocketChannel, error);

                oldChannel._providerHelper._needCloseOldChannel = true;

                error.channel(rsslSocketChannel);
                error.errorId(TransportReturnCodes.SUCCESS);
                error.sysError(0);
                error.text("READ_FD_CHANGE");
                return TransportReturnCodes.READ_FD_CHANGE;
            }
            case RIPC_WININET_STREAMING_RECONNECT:
            {
                _wininetStream = true;
                _reConnFromPOST = true;
                _opcodeRead = true;

                rsslSocketChannel._needCloseSocket = false;
                _commonSessionId = reader.getInt();

                StreamingSess sess = null;
                if (_wininetStreamingMap.containsKey(_commonSessionId))
                {
                    sess = (StreamingSess)_wininetStreamingMap.get(_commonSessionId);
                    sess._currentStreamingId = sess._newStreamingId == null ? _commonSessionId : sess._newStreamingId;
                    sess._newStreamingId = rsslSocketChannel._providerSessionId;
                }
                else
                {
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Streaming session not exist before reconn is estabished...");
                    return TransportReturnCodes.FAILURE;
                }

                if (debugPrint)
                    System.out.println(" Provider receives reconnect WININET streaming channel request with sessionId of " + _commonSessionId +
                                       " my sessionid is " + rsslSocketChannel._providerSessionId);

                if (debugPrint)
                    System.out.println("_sessionMap = " + _sessionMap);
                if (debugPrint)
                    System.out.println("_sessionIdSocketMapSize = " + _sessionIdSocketMap.size() + "_sessionIdSocketMap = " + _sessionIdSocketMap);
                if (debugPrint)
                    System.out.println("__wininetStreamingMap Size = " + _wininetStreamingMap.size());
                error.channel(rsslSocketChannel);
                error.errorId(TransportReturnCodes.SUCCESS);
                error.sysError(0);
                error.text("READ_FD_CHANGE");
                return TransportReturnCodes.READ_FD_CHANGE;
            }
            case RIPC_WININET_CONTROL_RECONNECT:
            {
                _reConnFromPOST = true;
                _wininetControl = true;
                _opcodeRead = true;

                rsslSocketChannel._needCloseSocket = false;
                _commonSessionId = reader.getInt();
                if (debugPrint)
                    System.out.println("Provider got reconnect control channel, session id = " + _commonSessionId +
                                       " my sessionid is " + rsslSocketChannel._providerSessionId);

                int reconnectControlBaseSessionId = -1;

                StreamingSess sess = null;
                if (_wininetStreamingMap.containsKey(_commonSessionId))
                {
                    sess = (StreamingSess)_wininetStreamingMap.get(_commonSessionId);
                    reconnectControlBaseSessionId = sess._controlId;
                    sess._newControlId = rsslSocketChannel._providerSessionId;
                }
                else
                {
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text("Streaming session not exist before reconn is estabished.");
                    return TransportReturnCodes.FAILURE;
                }

                RsslSocketChannel oldChannel = (RsslSocketChannel)_sessionMap.get(reconnectControlBaseSessionId);
                oldChannel._providerHelper._newControlSessionId = rsslSocketChannel._providerSessionId;

                oldChannel._providerHelper._newStreamingSessionId = sess._newStreamingId;
                oldChannel._providerHelper._needCloseOldChannel = true;

                if (debugPrint)
                    System.out.println("_sessionMap = " + _sessionMap);
                if (debugPrint)
                    System.out.println("_sessionIdSocketMapSize = " + _sessionIdSocketMap.size() + "_sessionIdSocketMap = " + _sessionIdSocketMap);
                if (debugPrint)
                    System.out.println("__wininetStreamingMap Size = " + _wininetStreamingMap.size());

                error.channel(rsslSocketChannel);
                error.errorId(TransportReturnCodes.SUCCESS);
                error.sysError(0);
                error.text("READ_FD_CHANGE");
                return TransportReturnCodes.READ_FD_CHANGE;
            }
            default:
            {
                System.out.println(" unrecognized opcode " + opCode);
                return TransportReturnCodes.CHAN_INIT_IN_PROGRESS;
            }
        }
    }

    protected long write(ByteBuffer[] srcs, int offset, int length) throws IOException
    {
        if (_wininetControl)
        {
            if (_outboundStreamingChannel != null)
            {
                try
                {
                    long numBytes = _outboundStreamingChannel.write(srcs, offset, length);
                    if (debugPrint)
                        System.out.println("RsslSocketChannel WireWrite(),  numBytes = " + numBytes + _outboundStreamingChannel.toString());

                    return numBytes;
                }
                catch (Exception e)
                {
                    if (debugPrint)
                        System.out.println("RsslSocketChannel WireWrite() failed. ");
                    rsslSocketChannel._needCloseSocket = true;
                    closeStreamingSocket();
                    throw e;
                }
            }
            return 0;
        }
        else
        {
            try
            {
                long numBytes = rsslSocketChannel._scktChannel.write(srcs, offset, length);
                if (debugPrint)
                    System.out.println("Java wireWrite, numBytes = " + numBytes + " on " + rsslSocketChannel._scktChannel.toString());
                return numBytes;
            }
            catch (Exception e)
            {
                throw e;
            }
        }
    }

    public int write(TransportBuffer bufferInt, WriteArgs writeArgs, Error error)
    {
        if (debugPrint)
            System.out.println(" providerWrite()  sessionid is " + rsslSocketChannel._providerSessionId +
                               " commonId = " + _commonSessionId + " winnetControl = " + _wininetControl);

        if (_wininetControl)
        {
            _outboundStreamingChannel = _sessionIdSocketMap.get(_commonSessionId);
        }

        assert (bufferInt != null) : "buffer cannot be null";
        assert (writeArgs != null) : "writeArgs cannot be null";
        assert (error != null) : "error cannot be null";

        int retVal = TransportReturnCodes.SUCCESS;

        HTTPTransportBufferImpl buffer = (HTTPTransportBufferImpl)bufferInt;
        if (buffer.encodedLength() == 0)
        {
            error.channel(this.rsslSocketChannel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Encoded buffer of length zero cannot be written");
            rsslSocketChannel._needCloseSocket = true;
            closeStreamingSocket();
            return TransportReturnCodes.FAILURE;
        }

        int ripcHdrFlags = RsslSocketChannel.IPC_DATA;
        int msgLen = 0;

        if (buffer != null)
            msgLen = buffer.length();

        try
        {
            rsslSocketChannel._writeLock.lock();

            if (rsslSocketChannel._state != ChannelState.ACTIVE && !_wininetStream)
            {
                error.channel(rsslSocketChannel);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in active state for write");
                rsslSocketChannel._needCloseSocket = true;
                closeStreamingSocket();
                return TransportReturnCodes.FAILURE;
            }

            // determine whether the buffer holds a message to be fragmented
            if (buffer.isBigBuffer() == false)
            {
                // normal message - no fragmentation

                // pack if enabled
                if (buffer._isPacked)
                {
                    buffer.pack(false, rsslSocketChannel, error);
                    msgLen = buffer.packedLen();
                    ripcHdrFlags |= RsslSocketChannel.IPC_PACKING;
                }

                // compress if enabled
                boolean compressedDataSent = false;
                if (rsslSocketChannel._sessionOutCompression > 0
                        && (writeArgs.flags() & WriteFlags.DO_NOT_COMPRESS) == 0)
                {
                    // only compress if within low and high thresholds
                    if (msgLen >= rsslSocketChannel._sessionCompLowThreshold)
                    {
                        // set _compressPriority if necessary
                        if (rsslSocketChannel._compressPriority == 99)
                        {
                            rsslSocketChannel._compressPriority = writeArgs.priority();
                        }
                        // only compress with initial message priority
                        if (writeArgs.priority() == rsslSocketChannel._compressPriority)
                        {
                            retVal = writeNormalCompressed(buffer, writeArgs, error);
                            compressedDataSent = true;
                        }
                    }
                }

                if (!compressedDataSent) // no compression: send normal buffer
                {
                    buffer.populateRipcHeader(ripcHdrFlags);
                    buffer.populateHTTPOverhead();

                    if (rsslSocketChannel._totalBytesQueued > 0) // buffers queued
                    {
                        retVal = rsslSocketChannel.writeWithBuffersQueued(buffer, writeArgs, error);
                        if (retVal < TransportReturnCodes.SUCCESS)
                        {
                            rsslSocketChannel._needCloseSocket = true;
                            closeStreamingSocket();
                            error.channel(rsslSocketChannel);
                            error.errorId(TransportReturnCodes.FAILURE);
                            error.sysError(0);
                            error.text("WriteWithBuffersQueued failed");
                            return retVal;
                        }
                    }
                    else
                    // no buffers queued
                    {
                        retVal = rsslSocketChannel.writeWithNoBuffersQueued(buffer, writeArgs, error);
                        if (retVal < TransportReturnCodes.SUCCESS)
                        {
                            rsslSocketChannel._needCloseSocket = true;
                            error.channel(rsslSocketChannel);
                            error.errorId(TransportReturnCodes.FAILURE);
                            error.sysError(0);
                            error.text("writeWithNoBuffersQueued failed");                            
                            closeStreamingSocket();
                            return retVal;
                        }
                    }

                    ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(msgLen + RsslSocketChannel.RIPC_HDR_SIZE + HTTP_HEADER6 + CHUNKEND_SIZE);
                }
            }
            else
            // message fragmentation
            {
                // check if the buffer has to be fragmented; if not, use the Ripc protocol
                if ((((BigBuffer)buffer)._isWritePaused)
                        || ((buffer._data.position() + RsslSocketChannel.RIPC_HDR_SIZE + HTTP_HEADER6 + CHUNKEND_SIZE) > rsslSocketChannel._internalMaxFragmentSize))
                {
                    // the message has to be fragmented
                    retVal = writeBigBuffer((BigBuffer)buffer, writeArgs, error);
                    if (retVal == TransportReturnCodes.WRITE_CALL_AGAIN || retVal == TransportReturnCodes.WRITE_FLUSH_FAILED)
                        return retVal;
                    else if (retVal < TransportReturnCodes.SUCCESS)
                    {
                        rsslSocketChannel._needCloseSocket = true;
                        closeStreamingSocket();
                        return retVal;
                    }
                }
                else
                {
                    // send the message using Ripc protocol

                    // copy the data from bigBuffer to the first transport buffer
                    HTTPTransportBufferImpl transportBuffer = (HTTPTransportBufferImpl)((BigBuffer)buffer)._firstBuffer;
                    ((BigBuffer)buffer)._firstBuffer = null;

                    transportBuffer._data.position(3 + HTTP_HEADER6);

                    buffer._data.limit(buffer._data.position());
                    buffer._data.position(0);

                    transportBuffer._data.put(buffer._data);
                    transportBuffer.populateRipcHeader(RsslSocketChannel.IPC_DATA);
                    transportBuffer.populateHTTPOverhead();

                    if (rsslSocketChannel._totalBytesQueued > 0)
                    {
                        retVal = rsslSocketChannel.writeWithBuffersQueued(transportBuffer, writeArgs, error);
                        if (retVal < TransportReturnCodes.SUCCESS)
                        {
                            rsslSocketChannel._needCloseSocket = true;
                            closeStreamingSocket();
                            return retVal;
                        }
                    }
                    else
                    // no buffers queued
                    {
                        retVal = rsslSocketChannel.writeWithNoBuffersQueued(transportBuffer, writeArgs, error);
                        if (retVal < TransportReturnCodes.SUCCESS)
                        {
                            rsslSocketChannel._needCloseSocket = true;
                            closeStreamingSocket();
                            return retVal;
                        }
                    }
                }
                // The buffer has to be returned to the bigBuffer pool
                if (retVal != TransportReturnCodes.WRITE_CALL_AGAIN)
                    ((BigBuffer)buffer).returnToPool();
            }
        }
        catch (CompressorException e)
        {
            rsslSocketChannel._state = ChannelState.CLOSED;
            rsslSocketChannel._needCloseSocket = true;
            closeStreamingSocket();
            error.channel(rsslSocketChannel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Compression write failed...");
            retVal = TransportReturnCodes.FAILURE;
            rsslSocketChannel.populateErrorDetails(error, TransportReturnCodes.FAILURE,
                                                   "CompressorException: " + e.getLocalizedMessage());
        }
        catch (Exception e)
        {
            rsslSocketChannel._state = ChannelState.CLOSED;
            rsslSocketChannel._needCloseSocket = true;
            error.channel(rsslSocketChannel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Channel is closed already...");
            closeStreamingSocket();
            retVal = TransportReturnCodes.FAILURE;
            rsslSocketChannel.populateErrorDetails(error, TransportReturnCodes.FAILURE,
                                                   "Exception: " + e.getLocalizedMessage());
        }
        finally
        {
            rsslSocketChannel._writeLock.unlock();
        }

        return retVal;
    }

    protected int writeNormalCompressed(HTTPTransportBufferImpl buffer, WriteArgs writeArgs, Error error)
    {
        int retVal = 0;
        int bytesForBuffer = 0;
        int totalBytes = 0;
        int ripcHdrFlags = Ripc.Flags.COMPRESSION;
        int msgLen = buffer.length();

        // http_hdr_size not subtract here as _internalMaxFragmentSize does not include it
        final int MAX_BYTES_FOR_BUFFER =
                rsslSocketChannel._internalMaxFragmentSize - RsslSocketChannel.RIPC_HDR_SIZE - HTTP_HEADER6 - CHUNKEND_SIZE;

        // An extra buffer might be needed: get it now before compression
        HTTPTransportBufferImpl compFragmentBuffer = getBufferInternal(MAX_BYTES_FOR_BUFFER, false);
        if (compFragmentBuffer == null)
        {
            retVal = rsslSocketChannel.flushInternal(error);
            if (retVal < TransportReturnCodes.SUCCESS)
                return retVal;
            compFragmentBuffer = getBufferInternal(MAX_BYTES_FOR_BUFFER, false);
            if (compFragmentBuffer == null)
                return TransportReturnCodes.NO_BUFFERS;
        }

        if (buffer._isPacked)
        {
            ripcHdrFlags |= Ripc.Flags.PACKING;
            msgLen = buffer.packedLen();
        }
        int compressedBytesLen = rsslSocketChannel._compressor.compress(buffer, buffer.dataStartPosition(), msgLen);

        byte[] compressedBytes = rsslSocketChannel._compressor.compressedData();

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
        buffer.data().limit(buffer.dataStartPosition() + bytesForBuffer + CHUNKEND_SIZE);
        buffer.data().put(compressedBytes, 0, bytesForBuffer);
        // Do this last (before write), so that internal buffer length and position set
        buffer.populateRipcHeader(ripcHdrFlags);
        buffer.populateHTTPOverhead();

        if (rsslSocketChannel._totalBytesQueued > 0)
        {
            retVal = rsslSocketChannel.writeWithBuffersQueued(buffer, writeArgs, error);
        }
        else
        {
            retVal = rsslSocketChannel.writeWithNoBuffersQueued(buffer, writeArgs, error);
        }
        // First part stats: User data bytes + overhead (ignore compression)
        ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(msgLen + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE);
        totalBytes = buffer._length;

        // Send extra message if there are bytes that did not fit in the first part
        if (retVal >= TransportReturnCodes.SUCCESS && compressedBytesLen > MAX_BYTES_FOR_BUFFER)
        {
            // Remaining compressed bytes to be sent
            bytesForBuffer = compressedBytesLen - MAX_BYTES_FOR_BUFFER;

            // Populate second message
            compFragmentBuffer.data().position(compFragmentBuffer.dataStartPosition());
            compFragmentBuffer.data().limit(compFragmentBuffer.dataStartPosition() + bytesForBuffer + CHUNKEND_SIZE);
            compFragmentBuffer.data().put(compressedBytes, MAX_BYTES_FOR_BUFFER, bytesForBuffer);

            compFragmentBuffer.populateRipcHeader(Ripc.Flags.COMPRESSION);
            compFragmentBuffer.populateHTTPOverhead();

            ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(writeArgs.uncompressedBytesWritten() + Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE);
            totalBytes += compFragmentBuffer._length;

            // Write second part and flush
            if (rsslSocketChannel._totalBytesQueued > 0)
            {
                retVal = rsslSocketChannel.writeWithBuffersQueued(compFragmentBuffer, writeArgs, error);
            }
            else
            {
                retVal = rsslSocketChannel.writeWithNoBuffersQueued(compFragmentBuffer, writeArgs, error);
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
        
        // http_hdr_size not subtract here as _internalMaxFragmentSize does not include it
        final int MAX_BYTES_FOR_BUFFER =
                rsslSocketChannel._internalMaxFragmentSize - RsslSocketChannel.RIPC_HDR_SIZE - HTTP_HEADER6 - CHUNKEND_SIZE;

        // An extra buffer might be needed: get it now before compression to ensure it is available if needed
        HTTPTransportBufferImpl compFragmentBuffer = getBufferInternal(MAX_BYTES_FOR_BUFFER, false);
        if (compFragmentBuffer == null)
        {
            retVal = rsslSocketChannel.flushInternal(error);
            if (retVal < TransportReturnCodes.SUCCESS)
                return retVal;
            compFragmentBuffer = getBufferInternal(MAX_BYTES_FOR_BUFFER, false);
            if (compFragmentBuffer == null)
                return TransportReturnCodes.NO_BUFFERS;
        }

        // Determine how many bytes can fit in the fragment, depending on
        // if this is the first/next fragment, and how many bytes remain in the big buffer.
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
        // (big buffer position points at data to be sent)
        compressedLen = rsslSocketChannel._compressor.compress(bigBuffer.data(), bigBuffer.data().position(), userBytesForFragment);

        compressedBytes = rsslSocketChannel._compressor.compressedData();

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
        rsslSocketChannel.writeFragment(fragment, writeArgs);

        // If there are extra bytes that could not fit in the fragment,
        // write the remainder of the compressed bytes into an extra message.
        // Extra bytes start at position userBytesForFragment (after data sent in previous message)
        if (extraBytes > 0)
        {
            // Populate second message
            compFragmentBuffer.data().position(compFragmentBuffer.dataStartPosition());
            compFragmentBuffer.data().limit(compFragmentBuffer.dataStartPosition() + extraBytes);
            compFragmentBuffer.data().put(compressedBytes, userBytesForFragment, extraBytes);
            compFragmentBuffer.populateRipcHeader(Ripc.Flags.COMPRESSION);

            rsslSocketChannel.writeFragment(compFragmentBuffer, writeArgs);

            extraTotalLength = Ripc.Lengths.HEADER + extraBytes; // actual length on wire
            extraHeaderLength = Ripc.Lengths.HEADER + HTTP_HEADER6 + CHUNKEND_SIZE; // overhead (header) from sending extra part
        }
        else
        {
            compFragmentBuffer.returnToPool();
        }

        // Actual bytes on wire is total length of first fragment, plus total length on wire of extra bytes (if sent)
        ((WriteArgsImpl)writeArgs).bytesWritten(writeArgs.bytesWritten() + fragment._length + extraTotalLength);

        // Uncompressed bytes is the number of bytes taken from the big buffer before compression,
        // plus overhead for the one (or two) messages sent on wire
        ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(writeArgs.uncompressedBytesWritten() +
                                                            userBytesForFragment + headerLength + extraHeaderLength);

        // Adjust big buffer for next call
        // -- set the limit to end of big buffer user data
        bigBuffer.data().limit(limit);
        // -- new position will be set just after the data inserted in this fragment
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
        if (rsslSocketChannel._sessionOutCompression > 0 && (writeArgs.flags() & WriteFlags.DO_NOT_COMPRESS) == 0)
        {
            // set _compressPriority if necessary
            if (rsslSocketChannel._compressPriority == 99)
            {
                rsslSocketChannel._compressPriority = writeArgs.priority();
            }
            // only compress high priority
            if (writeArgs.priority() == rsslSocketChannel._compressPriority)
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
                bytesLeft = bytesLeft - buffer._firstBuffer.populateFragment(buffer, true,
                                                                             Ripc.Flags.HAS_OPTIONAL_FLAGS | RsslSocketChannel.IPC_DATA, writeArgs);
                rsslSocketChannel.writeFragment(buffer._firstBuffer, writeArgs);
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
            HTTPSocketBuffer sBuffer = (HTTPSocketBuffer)rsslSocketChannel.getSocketBuffer();
            if (sBuffer == null)
            {
                retVal = rsslSocketChannel.flushInternal(error);
                if (retVal < TransportReturnCodes.SUCCESS)
                    return retVal;

                if ((sBuffer = (HTTPSocketBuffer)rsslSocketChannel.getSocketBuffer()) == null)
                {
                    buffer._isWritePaused = true;
                    return TransportReturnCodes.WRITE_CALL_AGAIN;
                }
            }
            HTTPTransportBufferImpl nextBuffer = sBuffer.getBufferSliceForFragment(rsslSocketChannel._internalMaxFragmentSize);

            if (!doCompression || bytesLeft < rsslSocketChannel._sessionCompLowThreshold)
            {
                bytesLeft = bytesLeft - nextBuffer.populateFragment(buffer, false,
                                                                    Ripc.Flags.HAS_OPTIONAL_FLAGS | RsslSocketChannel.IPC_DATA, writeArgs);
                rsslSocketChannel.writeFragment(nextBuffer, writeArgs);
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
        if ((writeArgs.flags() & WriteFlags.DIRECT_SOCKET_WRITE) > 0 || rsslSocketChannel._totalBytesQueued > rsslSocketChannel._highWaterMark)
        {
            if ((retVal = rsslSocketChannel.flushInternal(error)) < TransportReturnCodes.SUCCESS)
            {
                return retVal;
            }
        }
        else
        {
            retVal = rsslSocketChannel._totalBytesQueued;
        }

        return retVal;
    }

    public TransportBuffer getBuffer(int size, boolean packedBuffer, Error error)
    {
        assert (error != null) : "error cannot be null";

        int sizeWithHeaders = size + RsslSocketChannel.RIPC_HDR_SIZE;
        if (packedBuffer)
            sizeWithHeaders += RsslSocketChannel.RIPC_PACKED_HDR_SIZE;

        sizeWithHeaders += HTTP_HEADER6 + CHUNKEND_SIZE;

        HTTPTransportBufferImpl buffer = null;
        try
        {
            rsslSocketChannel._writeLock.lock();

            // return FAILURE if channel not active
            if (rsslSocketChannel._state != ChannelState.ACTIVE)
            {
                error.channel(rsslSocketChannel);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in the active state for getBuffer...");
                rsslSocketChannel._needCloseSocket = true;
                if (_wininetControl || _wininetStream)
                    closeStreamingSocket();
                return null;
            }

            // check if we need big buffer instead of normal buffer
            if (sizeWithHeaders > rsslSocketChannel._internalMaxFragmentSize)
            {
                if (!packedBuffer)
                {
                    buffer = (HTTPTransportBufferImpl)rsslSocketChannel.getBigBuffer(size);
                    if (buffer == null)
                    {
                        error.channel(rsslSocketChannel);
                        error.errorId(TransportReturnCodes.NO_BUFFERS);
                        error.sysError(0);
                        error.text("channel out of buffers");
                    }
                    return buffer;
                }
                else
                {
                    error.channel(rsslSocketChannel);
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
                error.channel(rsslSocketChannel);
                error.errorId(TransportReturnCodes.NO_BUFFERS);
                error.sysError(0);
                error.text("channel out of buffers");
            }
        }
        finally
        {
            rsslSocketChannel._writeLock.unlock();
        }

        return buffer;
    }

    protected HTTPTransportBufferImpl getBufferInternal(int size, boolean packedBuffer)
    {
        // This method should be called when the buffer size + header is less, then fragment size.
        // The calling method chain should have lock set.
        HTTPTransportBufferImpl buffer = null;

        if (rsslSocketChannel._currentBuffer != null)
        {
            HTTPSocketBuffer currentBuffer = (HTTPSocketBuffer)rsslSocketChannel._currentBuffer;

            buffer = currentBuffer.getBufferSlice(size, packedBuffer);

            if (buffer == null)
            {
                HTTPSocketBuffer temp = (HTTPSocketBuffer)rsslSocketChannel._currentBuffer;
                if (temp._slicesPool.areAllSlicesBack())
                {
                    rsslSocketChannel._currentBuffer = null;

                    rsslSocketChannel.socketBufferToRecycle(temp);
                }
            }
        }

        if (buffer == null)
        {
            rsslSocketChannel._currentBuffer = (HTTPSocketBuffer)rsslSocketChannel._availableHTTPBuffers.poll();

            if (rsslSocketChannel._currentBuffer == null)
            {
                if (debugPrint)
                    System.out.println(" ProviderHelper getBufferInternal currentBuffer is null, _providerSessionId = "
                                       + rsslSocketChannel._providerSessionId);
            }
            if (rsslSocketChannel._currentBuffer != null)
            {
                rsslSocketChannel._currentBuffer.clear();
                ++rsslSocketChannel._used;

                buffer = ((HTTPSocketBuffer)rsslSocketChannel._currentBuffer).getBufferSlice(size, packedBuffer);
            }
        }
        if (buffer == null)
        {
            if (debugPrint)
                System.out.println("HTTP Provider Buffer is null...");
        }

        return buffer;
    }

    void releaseBufferInternal(TransportBuffer bufferInt)
    {
        HTTPTransportBufferImpl buffer = (HTTPTransportBufferImpl)bufferInt;
        buffer.returnToPool();
    }

    protected int initChnlFinishSess(InProgInfo inProg, Error error) throws IndexOutOfBoundsException, IOException
    {
        if (rsslSocketChannel._ipcProtocol.ripcVersion() <= Ripc.RipcVersions.VERSION12)
        {
            error.channel(rsslSocketChannel);
            error.errorId(TransportReturnCodes.NO_BUFFERS);
            error.sysError(0);
            error.text(" RIPC12 or older version not supported.");
            System.out.println(" RIPC version 12 or older is not supported");
            return TransportReturnCodes.FAILURE;
        }

        if (_wininetControl)
        {
            java.nio.channels.SocketChannel streamSocket = getPairingStreamingChannel();

            if (streamSocket != null)
            {
                streamSocket.write(rsslSocketChannel._ipcProtocol.encodeHTTPConnectionAck(rsslSocketChannel._initChnlWriteBuffer, error));
            }
            else
            {
                error.channel(rsslSocketChannel);
                error.errorId(TransportReturnCodes.NO_BUFFERS);
                error.sysError(0);
                error.text("Wininet control channel cannot locate its streaming channel..");
                if (debugPrint)
                    System.out.println(" Wininet control channel cannot locate its streaming channel");
                return TransportReturnCodes.FAILURE;
            }
        }
        else
        {
            rsslSocketChannel._scktChannel.write(rsslSocketChannel._ipcProtocol.encodeHTTPConnectionAck(rsslSocketChannel._initChnlWriteBuffer, error));
        }

        return TransportReturnCodes.SUCCESS;
    }

    public int ping(Error error)
    {
        assert (error != null) : "error cannot be null";

        int retVal = TransportReturnCodes.SUCCESS;

        // send ping

        try
        {
            rsslSocketChannel._writeLock.lock();

            // return FAILURE if channel not active
            if (rsslSocketChannel._state != ChannelState.ACTIVE)
            {
                error.channel(rsslSocketChannel);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is not in the active state for ping");

                if (_wininetControl || _wininetStream)
                    closeStreamingSocket();
                rsslSocketChannel._needCloseSocket = true;

                return TransportReturnCodes.FAILURE;
            }

            if (rsslSocketChannel._totalBytesQueued > 0)
            {
                // call flush since bytes queued
                retVal = rsslSocketChannel.flushInternal(error);
                if (retVal < TransportReturnCodes.SUCCESS)
                {
                    error.channel(rsslSocketChannel);
                    error.errorId(TransportReturnCodes.FAILURE);
                    error.sysError(0);
                    error.text(" Ping flush internal failed");
                    if (_wininetControl || _wininetStream)
                        closeStreamingSocket();
                    rsslSocketChannel._needCloseSocket = true;

                    return retVal;
                }
            }
            else
            // send ping buffer
            {
                _pingWriteBuffer.rewind();

                if (!rsslSocketChannel._encrypted)
                {
                    retVal = RsslSocketChannel.RIPC_HDR_SIZE + HTTP_HEADER3 + CHUNKEND_SIZE - rsslSocketChannel._scktChannel.write(_pingWriteBuffer);
                }
                else
                {
                    rsslSocketChannel._crypto.write(_pingWriteBuffer);
                    retVal = 0;
                }
            }
        }
        catch (Exception e)
        {
            rsslSocketChannel._state = ChannelState.CLOSED;
            error.channel(rsslSocketChannel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text(" Ping failed..");
            if (_wininetControl || _wininetStream)
                closeStreamingSocket();
            rsslSocketChannel._needCloseSocket = true;

            return TransportReturnCodes.FAILURE;
        }
        finally
        {
            rsslSocketChannel._writeLock.unlock();
        }
        return retVal;
    }

    private int setupPOSThttpResponse(ByteBuffer buf, int sessionID)
    {
        // include HTTP Session ID
        ByteRoutines.putInt(_httpSessionIDbytes, 0, sessionID);

        byte[] data = { (byte)0x00,
                        (byte)0x07, // 2-byte length (=7 bytes)
                        (byte)0x00, // 1-byte for opCode
                        _httpSessionIDbytes[0], _httpSessionIDbytes[1], _httpSessionIDbytes[2],
                        _httpSessionIDbytes[3] // 4-byte HTTP session ID
        };
        try
        {
            // add the "HTTP 200 OK" response header
            buf.put((_http200Response.toString()).getBytes("US-ASCII"));

            buf.put(data);
            buf.put("\r\n".getBytes("US-ASCII"));
            buf.flip(); // ready for reading

            if (debugPrint)
                System.out.println(" Provider HTTP OK 200 reply to session  " + rsslSocketChannel._providerSessionId);
        }
        catch (IOException e)
        {
            rsslSocketChannel._state = ChannelState.CLOSED;
            return TransportReturnCodes.FAILURE;
        }
        return TransportReturnCodes.SUCCESS;

    }

    private int setupServerReconnectionPOSTACKresponse(ByteBuffer buf)
    {
        byte[] data = { (byte)0x31, (byte)0x0D, (byte)0x0A, (byte)0x03, (byte)0x0D, (byte)0x0A };

        buf.put(data);

        buf.flip(); // ready for reading
        return TransportReturnCodes.SUCCESS;
    }

    // may not need this, the above method works for regular wininet with no reconnect though
    private int setupServerReconnectionPOSTACKresponseWininet(ByteBuffer buf)
    {
        // 0x01 or 0x03 makes no difference
        byte[] data = { (byte)0x03, (byte)0x0D, (byte)0x0A };

        try
        {
            buf.put((_http200ResponseReconn.toString()).getBytes("US-ASCII"));
            buf.put(data);
        }
        catch (IOException e)
        {
            return TransportReturnCodes.FAILURE;
        }

        buf.flip(); // ready for reading
        return TransportReturnCodes.SUCCESS;
    }

    void setupPingWriteBuffer()
    {
        _pingWriteBuffer.putShort(3, (short)RsslSocketChannel.RIPC_HDR_SIZE); // ripc header length
        _pingWriteBuffer.put(5, (byte)2); // ripc flag indicating data
        byte[] dataLengthBytesTemp = Integer.toHexString(3).getBytes(); // 33 hex
        byte[] dataLengthBytes = { 0x3B, 0x3B, 0x3B, 0x3B }; // initialize (3Bh is what C++ uses)
        for (int i = 0; i < dataLengthBytesTemp.length; i++)
            dataLengthBytes[i] = dataLengthBytesTemp[i];
        // length will not take up more than 1 bytes
        _pingWriteBuffer.put(0, dataLengthBytes[0]);
        _pingWriteBuffer.put(1, (byte)0x0D);
        _pingWriteBuffer.put(2, (byte)0x0A);

        _pingWriteBuffer.put(6, (byte)0x0D);
        _pingWriteBuffer.put(7, (byte)0x0A);
    }

    @SuppressWarnings("deprecation")
    private void handleOldJavaChannel(RsslSocketChannel channel, Error error)
    {
        // in base channel
        try
        {
            _httpBuffer.put(0, (byte)0x0);

            channel.oldScktChannel().write(_httpBuffer);

            if (debugPrint)
                System.out.println(" Provider sent the LAST 5 BYTES on oldChannel.  " + channel.oldSelectableChannel().toString());
        }
        catch (IOException e)
        {
            error.channel(rsslSocketChannel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Error write terminal bytes to HTTP old channel.");
        }
    }

    public int closeJavaOldSocket()
    {// in cases of NO_DATA and simulated read (-1) case, on base channel
        try
        {
            Transport._globalLock.lock();
            try
            {
                if (debugPrint)
                    System.out.println(" Java close socket from server side = " + rsslSocketChannel.selectableChannel().toString());
                
                rsslSocketChannel._oldScktChannel = rsslSocketChannel.scktChannel();
                _sessionIdSocketMap.remove(rsslSocketChannel._providerSessionId);
                rsslSocketChannel.selectableChannel().close();

                java.nio.channels.SocketChannel newControlScktChannel =
                        _sessionIdSocketMap.get(rsslSocketChannel._providerHelper._newControlSessionId);

                if (newControlScktChannel != null)
                {
                    rsslSocketChannel._scktChannel = newControlScktChannel;
                    _sessionIdSocketMap.put(rsslSocketChannel._providerSessionId, rsslSocketChannel._scktChannel);

                    _sessionIdSocketMap.remove(rsslSocketChannel._providerHelper._newControlSessionId);
                    return TransportReturnCodes.READ_FD_CHANGE;
                }
                else
                {
                    rsslSocketChannel._needCloseSocket = true;
                    return TransportReturnCodes.FAILURE;
                }
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
        }
        finally
        {
            Transport._globalLock.unlock();
        }
        return TransportReturnCodes.FAILURE;
    }

    public void closeStreamingSocket()
    {
        closeStreamingSocket(_commonSessionId);
    }

    private void closeStreamingSocket(Integer commonSessionId)
    {
        if (debugPrint)
            System.out.println("Close stream channel.. _commonSessionId is " + commonSessionId);

        java.nio.channels.SocketChannel socket = _sessionIdSocketMap.get(commonSessionId);
        try
        {
            Transport._globalLock.lock();

            if (socket != null)
            {
                if (debugPrint)
                    System.out.println(" Close streaming socket = " + socket.toString());
                socket.close();
            }
        }
        catch (Exception e)
        {
            e.printStackTrace();
        }
        finally
        {
            Transport._globalLock.unlock();
        }

        _sessionIdSocketMap.remove(commonSessionId);

        Integer newSocketId = null;
        Integer newControlId = null;
        if (_wininetStreamingMap.containsKey(commonSessionId))
        {
            StreamingSess sess = (StreamingSess)_wininetStreamingMap.get(commonSessionId);
            if (sess._newStreamingId != null && sess._newStreamingId.intValue() != -1)
                newSocketId = sess._newStreamingId;
            if (sess._newControlId != null && sess._newControlId.intValue() != -1)
                newControlId = sess._newControlId;

            java.nio.channels.SocketChannel newSocket = _sessionIdSocketMap.get(newSocketId);
            java.nio.channels.SocketChannel newControlSocket = _sessionIdSocketMap.get(newControlId);

            _sessionIdSocketMap.remove(newSocketId);
            _sessionIdSocketMap.remove(newControlId);

            try
            {
                Transport._globalLock.lock();
                try
                {
                    if (newSocket != null)
                    {
                        if (debugPrint)
                            System.out.println(" Close new streaming socket = " + newSocket.toString());
                        newSocket.close();
                    }
                }
                catch (Exception e)
                {
                }

                try
                {
                    // unlikely to occur
                    if (newControlSocket != null)
                    {
                        if (debugPrint)
                            System.out.println(" Close new control socket = " + newControlSocket.toString());
                        newControlSocket.close();
                    }
                }
                catch (Exception e)
                {
                }

                if (_wininetStreamingMap.containsKey(commonSessionId))
                {
                    _sessionIdSocketMap.remove(sess._controlId);
                    _sessionIdSocketMap.remove(sess._currentStreamingId);
                    _sessionIdSocketMap.remove(sess._newStreamingId);
                    _sessionIdSocketMap.remove(sess._newControlId);
                    sess._controlId = -1;
                    sess._currentStreamingId = -1;
                    sess._newStreamingId = -1;
                    sess._newControlId = -1;
                    sess._pipeNode._pipe.source().close();
                    sess._pipeNode._pipe.sink().close();
                    sess._pipeNode.returnToPool();

                    _wininetStreamingMap.remove(commonSessionId);
                    sess = null;
                }

                if (debugPrint)
                    System.out.println("_sessionMap = " + _sessionMap);
                if (debugPrint)
                    System.out.println("_sessionIdSocketMapSize = " + _sessionIdSocketMap.size() +
                                       "_sessionIdSocketMap = " + _sessionIdSocketMap);
                if (debugPrint)
                    System.out.println("__wininetStreamingMap Size = " + _wininetStreamingMap.size());
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }
            finally
            {
                Transport._globalLock.unlock();
            }
        }
        else
        // try Java stream
        {
            if (debugPrint)
                System.out.println("Trying to close streaming socket, yet commonId of " + commonSessionId + " not in streamingMap");
            if (!_javaSession)
                return;

            if (debugPrint)
                System.out.println("Close Java new socket " + rsslSocketChannel._providerHelper._newControlSessionId);

            java.nio.channels.SocketChannel newControlScktChannel = _sessionIdSocketMap.get(rsslSocketChannel._providerHelper._newControlSessionId);
            java.nio.channels.SocketChannel currentScktChannel = _sessionIdSocketMap.get(rsslSocketChannel._providerSessionId);

            _sessionIdSocketMap.remove(rsslSocketChannel._providerHelper._newControlSessionId);
            _sessionIdSocketMap.remove(rsslSocketChannel._providerSessionId);

            if (newControlScktChannel != null)
            {
                try
                {
                    Transport._globalLock.lock();
                    try
                    {
                        if (debugPrint)
                            System.out.println(" Close new Java socket = " + newControlScktChannel.toString());
                        newControlScktChannel.close();
                    }
                    catch (Exception e)
                    {
                    }
                }
                catch (Exception e)
                {
                    e.printStackTrace();
                }
                finally
                {
                    Transport._globalLock.unlock();
                }
            }
            if (currentScktChannel != null)
            {
                try
                {
                    Transport._globalLock.lock();
                    try
                    {
                        if (debugPrint)
                            System.out.println(" Close current Java socket = " + currentScktChannel.toString());
                        currentScktChannel.close();
                    }
                    catch (Exception e)
                    {
                    }
                }
                catch (Exception e)
                {
                    e.printStackTrace();
                }
                finally
                {
                    Transport._globalLock.unlock();
                }
            }
        }
    }
    
    @SuppressWarnings("deprecation")
    public int switchWininetSession(Error error)
    {
        // on base control rsslChannel

        if (debugPrint)
            System.out.println(" Will close Provider side socket " + rsslSocketChannel.selectableChannel().toString());

        try
        {
            Transport._globalLock.lock();
            try
            {
                rsslSocketChannel._oldScktChannel = rsslSocketChannel.scktChannel();
                _sessionIdSocketMap.remove(rsslSocketChannel._providerSessionId);
                rsslSocketChannel.scktChannel().close();

                java.nio.channels.SocketChannel newControlScktChannel =
                        _sessionIdSocketMap.get(rsslSocketChannel._providerHelper._newControlSessionId);

                rsslSocketChannel._scktChannel = newControlScktChannel;

                _sessionIdSocketMap.remove(rsslSocketChannel._providerHelper._newControlSessionId);
                if (newControlScktChannel != null)
                {
                    _sessionIdSocketMap.put(rsslSocketChannel._providerSessionId, rsslSocketChannel._scktChannel);
                }
            }
            catch (Exception e)
            {
                e.printStackTrace();
            }

            if (debugPrint)
                System.out.println("_sessionMap = " + _sessionMap);
            if (debugPrint)
                System.out.println("_sessionIdSocketMapSize = " + _sessionIdSocketMap.size() +
                                   "_sessionIdSocketMap = " + _sessionIdSocketMap);
            if (debugPrint)
                System.out.println("__wininetStreamingMap Size = " + _wininetStreamingMap.size());
        }
        finally
        {
            Transport._globalLock.unlock();
        }

        java.nio.channels.SocketChannel streamingSocket = null;
        if (_wininetStreamingMap.containsKey(_commonSessionId))
        {
            streamingSocket = _sessionIdSocketMap.get(_commonSessionId);

            if (streamingSocket != null)
            {
                if (_needCloseOldChannel)
                {
                    try
                    {
                        _httpBuffer.rewind();
                        int numBytes = streamingSocket.write(_httpBuffer);
                        if (debugPrint)
                            System.out.println(" Write 5 bytes to wininet old streaming channel " + streamingSocket +
                                               " numBytes = " + numBytes);

                        _needCloseOldChannel = false;
                    }
                    catch (Exception e)
                    {
                        e.printStackTrace();
                    }
                }

                try
                {
                    if (debugPrint)
                        System.out.println("Close streaming socket from this end:  " + streamingSocket.toString());
                    _sessionIdSocketMap.remove(_commonSessionId);
                    streamingSocket.close();

                    java.nio.channels.SocketChannel newStreamingScktChannel =
                            _sessionIdSocketMap.get(rsslSocketChannel._providerHelper._newStreamingSessionId);
                    
                    _sessionIdSocketMap.remove(rsslSocketChannel._providerHelper._newStreamingSessionId);

                    if (newStreamingScktChannel != null)
                    {
                        _sessionIdSocketMap.put(_commonSessionId, newStreamingScktChannel);
                    }

                    if (debugPrint)
                        System.out.println("_sessionMap = " + _sessionMap);
                    if (debugPrint)
                        System.out.println("_sessionIdSocketMapSize = " + _sessionIdSocketMap.size() +
                                           "_sessionIdSocketMap = " + _sessionIdSocketMap);
                    if (debugPrint)
                        System.out.println("__wininetStreamingMap Size = " + _wininetStreamingMap.size());
                }
                catch (Exception e)
                {
                    e.printStackTrace();
                }
            }
        }
        if (rsslSocketChannel._scktChannel == null)
        {
            error.channel(rsslSocketChannel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("underlying socket channel is null ");
            return TransportReturnCodes.FAILURE;
        }
        else
        {
            error.channel(rsslSocketChannel);
            error.errorId(TransportReturnCodes.SUCCESS);
            error.sysError(0);
            error.text("READ_FD_CHANGE ");
            return TransportReturnCodes.READ_FD_CHANGE;
        }
    }

    public int close(Error error)
    {
        assert (error != null) : "error cannot be null";
        int ret = TransportReturnCodes.SUCCESS;

        try
        {
            rsslSocketChannel.lockReadWriteLocks();

            if (rsslSocketChannel._state == ChannelState.INITIALIZING)
            {
                if (debugPrint)
                    System.out.println("Force close underlier to true");
                rsslSocketChannel._needCloseSocket = true;
            }

            if (rsslSocketChannel._state != ChannelState.INACTIVE)
            {
                rsslSocketChannel._state = ChannelState.INACTIVE;
                if (rsslSocketChannel._compressor != null)
                {
                    rsslSocketChannel._compressor.close();
                }
            }
            else
            {
                error.channel(rsslSocketChannel);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is inactive ");
                ret = TransportReturnCodes.FAILURE;
            }
        }
        finally
        {
            rsslSocketChannel.unlockReadWriteLocks();
            if (ret == TransportReturnCodes.FAILURE)
            {
                error.channel(rsslSocketChannel);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel is inactive... ");
                return ret;
            }
        }

        try
        {
            Transport._globalLock.lock();

            // release buffers
            TransportBuffer buffer = null;
            while ((buffer = (TransportBuffer)rsslSocketChannel._highPriorityQueue.poll()) != null)
            {
                releaseBufferInternal(buffer);
            }
            while ((buffer = (TransportBuffer)rsslSocketChannel._mediumPriorityQueue.poll()) != null)
            {
                releaseBufferInternal(buffer);
            }
            while ((buffer = (TransportBuffer)rsslSocketChannel._lowPriorityQueue.poll()) != null)
            {
                releaseBufferInternal(buffer);
            }
            for (int i = 0; i < rsslSocketChannel._releaseBufferArray.length; i++)
            {
                if (rsslSocketChannel._releaseBufferArray[i] != null)
                {
                    releaseBufferInternal(rsslSocketChannel._releaseBufferArray[i]);
                }
                rsslSocketChannel._releaseBufferArray[i] = null;
                rsslSocketChannel._gatheringWriteArray[i] = null;
            }
            rsslSocketChannel._writeArrayMaxPosition = 0;
            rsslSocketChannel._writeArrayPosition = 0;
            rsslSocketChannel._isFlushPending = false;
            rsslSocketChannel._totalBytesQueued = 0;

            try
            {
                if (debugPrint)
                    System.out.println(" In session of " + rsslSocketChannel._providerSessionId +
                                       " _needCloseSocket = " + rsslSocketChannel._needCloseSocket +
                                       " _scktChannel = " + rsslSocketChannel._scktChannel);

                if (rsslSocketChannel._needCloseSocket)
                {
                    _sessionIdSocketMap.remove(rsslSocketChannel._providerSessionId);
                    if (rsslSocketChannel._scktChannel != null)
                    {
                        if (debugPrint)
                            System.out.println("RsslSocketChannel hard close socket of " + rsslSocketChannel._scktChannel.toString());
                    }
                    else
                    {
                        if (debugPrint)
                        {
                            System.out.println("RsslSocketChannel hard close socket of null");
                        }
                    }
                    closeStreamingSocket();

                    if (rsslSocketChannel._scktChannel != null)
                        rsslSocketChannel._scktChannel.close();

                    if (rsslSocketChannel._encrypted)
                    {
                        if (rsslSocketChannel._crypto != null)
                            rsslSocketChannel._crypto.cleanup();
                    }
                }
                else
                {
                    if (debugPrint)
                        System.out.println("RsslSocketChannel < NOT CLEANING UP> underlier");
                }

                clearResource();
                if (_sessionMap.isEmpty())
                {
                    _sessionIdSocketMap.clear();
                }
            }
            catch (IOException e)
            {
                error.channel(rsslSocketChannel);
                error.errorId(TransportReturnCodes.FAILURE);
                error.sysError(0);
                error.text("socket channel close failed ");
                ret = TransportReturnCodes.FAILURE;
            }
        }
        finally
        {
            Transport._globalLock.unlock();
        }
        return ret;
    }

    public void clearResource()
    {
        rsslSocketChannel.shrinkGuaranteedOutputBuffers(rsslSocketChannel._availableHTTPBuffers.size());

        if (rsslSocketChannel._readIoBuffer != null)
        {
            rsslSocketChannel._readIoBuffer.returnToPool();
            rsslSocketChannel._readIoBuffer = null;
        }

        // System.out.println(" activeSize = " + ((TrackingPool)rsslSocketChannel._pool)._active._size +
        //                    " head = " + ((TrackingPool)rsslSocketChannel._pool)._active._head
        //                    " tail = " + ((TrackingPool)rsslSocketChannel._pool)._active._tail);
        // if (((TrackingPool)rsslSocketChannel._pool)._active._head != null)
        // {
        //    System.out.println(" head_next = " + ((TrackingPool)rsslSocketChannel._pool)._active._head._next);
        // }
        // if (((TrackingPool)rsslSocketChannel._pool)._active._tail != null)
        // {
        //    System.out.println(" tail_next = " + ((TrackingPool)rsslSocketChannel._pool)._active._tail._next);
        // }

        // System.out.println(" returnToPOOL for sessionId = " + rsslSocketChannel._providerSessionId);

        rsslSocketChannel.resetToDefault();

        rsslSocketChannel.returnToPool();

        if (debugPrint)
            System.out.println("POOLSIZE = " + rsslSocketChannel._pool._queue._size +
                               " ACTIVESIZE = " + ((TrackingPool)rsslSocketChannel._pool)._active.size());

        if (rsslSocketChannel._server != null)
        {
            rsslSocketChannel._server.removeChannel(rsslSocketChannel);
        }
        _sessionMap.remove(rsslSocketChannel._providerSessionId);
        if (debugPrint)
            System.out.println("remove entry in _sessionMap, key = " + rsslSocketChannel._providerSessionId);

        if (debugPrint)
            System.out.println("_sessionMap = " + _sessionMap);
        if (debugPrint)
            System.out.println("_sessionIdSocketMapSize = " + _sessionIdSocketMap.size() +
                               " _sessionIdSocketMap = " + _sessionIdSocketMap);
        if (debugPrint)
            System.out.println("__wininetStreamingMap Size = " + _wininetStreamingMap.size());
    }

    public boolean wininetStreamingComplete()
    {
        return _streamingChannelCreated;
    }

    public void initChnlRejectSession(Error error)
    {
        if (debugPrint)
            System.out.println(" close socket due to session reject:  " + rsslSocketChannel.selectableChannel().toString());
        _sessionIdSocketMap.remove(rsslSocketChannel._providerSessionId);

        try
        {
            rsslSocketChannel.selectableChannel().close();
            closeStreamingSocket();
        }
        catch (Exception e)
        {
        }
    }

    void setupHttpBuffer()
    {
        _httpBuffer.put(0, (byte)0x30);
        _httpBuffer.put(1, (byte)0x0D);
        _httpBuffer.put(2, (byte)0x0A);
        _httpBuffer.put(3, (byte)0x0D);
        _httpBuffer.put(4, (byte)0x0A);
    }

    java.nio.channels.SocketChannel getPairingStreamingChannel()
    {
        if (_wininetStreamingMap.containsKey(_commonSessionId))
        {
            StreamingSess sess = (StreamingSess)_wininetStreamingMap.get(_commonSessionId);
            if (sess._newStreamingId != null && sess._newStreamingId.intValue() != -1)
                return _sessionIdSocketMap.get(sess._newStreamingId);
            else
                return _sessionIdSocketMap.get(_commonSessionId);
        }
        else
            return null;
    }

    void extraCleanup()
    {
        if (_wininetStreamingMap.size() <= _wininetSessionThreshold)
            return;
        // just in case, should not reach here really.
        boolean acted = false;
        for (Map.Entry<Integer, StreamingSess> entry : _wininetStreamingMap.entrySet())
        {
            Integer commonId = entry.getKey();
            boolean found = false;
            for (Map.Entry<Integer, RsslSocketChannel> rsslEntry : _sessionMap.entrySet())
            {
                RsslSocketChannel rsslSocketChannel = rsslEntry.getValue();
                if (rsslSocketChannel._providerHelper != null && rsslSocketChannel._providerHelper._commonSessionId == commonId)
                {
                    found = true;
                    break;
                }
            }
            if (!found)
            {
                if (debugPrint)
                {
                    System.out.println("Inconsistent map entry in winnetStreamingMap, clean up, commonId = " + commonId);
                    System.out.println(" _wininetStreamingMap.size() = " + _wininetStreamingMap.size());
                }

                closeStreamingSocket(commonId);
                acted = true;
                if (debugPrint)
                    System.out.println(" _wininetStreamingMap.size() now = " + _wininetStreamingMap.size());
            }
        }
        if (acted)
            _wininetSessionThreshold = _wininetSessionThreshold / 2;
        else
            _wininetSessionThreshold = _wininetSessionThreshold * 2;

        if (debugPrint)
            System.out.println("Change _wininetSessionThreshold to " + _wininetSessionThreshold);
    }
        
    void extraCleanup2()
    {
        // should be avoided, indeed never really reach here. for testing this is useful
        if (_sessionIdSocketMap.size() <= _socketSessionThreshold)
            return;
        boolean acted = false;
        try
        {
            if (fstream == null && debugPrint)
            {
                fstream = new FileWriter("socketSweep.log", true); // true tells to append data.
                _out = new BufferedWriter(fstream);
            }
        }
        catch (IOException e)
        {
            System.err.println("Error: " + e.getMessage());
        }

        ArrayList<Integer> currentList = new ArrayList<Integer>();

        for (Map.Entry<Integer, RsslSocketChannel> rsslEntry : _sessionMap.entrySet())
        {
            RsslSocketChannel rsslSocketChannel = rsslEntry.getValue();
            if (rsslSocketChannel._providerHelper != null && rsslSocketChannel._providerHelper._wininetControl)
            {
                Integer streamId = rsslSocketChannel._providerHelper._commonSessionId;
                if (streamId != null && streamId != -1)
                    currentList.add(streamId);
                Integer controlId = rsslSocketChannel._providerSessionId;
                if (controlId != null && controlId != -1)
                    currentList.add(controlId);

                StreamingSess sess = _wininetStreamingMap.get(streamId);
                if (sess != null)
                {
                    Integer newStreamId = sess._newStreamingId;
                    Integer newControlId = sess._newControlId;
                    if (newStreamId != null && newStreamId != -1)
                        currentList.add(newStreamId);

                    if (newControlId != null && newControlId != -1)
                        currentList.add(newControlId);
                }
            }
            else if (rsslSocketChannel._providerHelper != null && rsslSocketChannel._providerHelper._wininetStream)
            {
                Integer streamId = rsslSocketChannel._providerHelper._commonSessionId;
                if (streamId != null && streamId != -1)
                    currentList.add(streamId);

                StreamingSess sess = _wininetStreamingMap.get(streamId);
                if (sess != null)
                {
                    Integer newStreamId = sess._newStreamingId;
                    Integer newControlId = sess._newControlId;

                    if (newStreamId != null && newStreamId != -1)
                        currentList.add(newStreamId);

                    if (newControlId != null && newControlId != -1)
                        currentList.add(newControlId);

                    Integer controlId = sess._controlId;
                    if (controlId != null && controlId != -1)
                        currentList.add(controlId);
                }
            }
            else if (rsslSocketChannel._providerHelper != null && rsslSocketChannel._providerHelper._javaSession)
            {
                Integer javaSessionId = rsslSocketChannel._providerSessionId;
                Integer javaNewSessionId = rsslSocketChannel._providerHelper._newControlSessionId;
                if (javaSessionId != null && javaSessionId != -1)
                    currentList.add(javaSessionId);
                if (javaNewSessionId != null && javaNewSessionId != -1)
                    currentList.add(javaNewSessionId);
            }
        }

        ArrayList<Integer> listToGo = new ArrayList<Integer>();
        for (Map.Entry<Integer, java.nio.channels.SocketChannel> entry : _sessionIdSocketMap.entrySet())
        {
            Integer id = entry.getKey();
            if (currentList.contains(id))
                continue;
            else
            {
                listToGo.add(id);
            }
        }

        for (int i = 0; i < listToGo.size(); i++)
        {
            Integer idToGo = listToGo.get(i);

            _sessionIdSocketMap.remove(idToGo);
            acted = true;
            try
            {
                if (debugPrint && _out != null)
                    _out.write("session Id to go =  + idToGo + " + new SimpleDateFormat("yyyyMMdd_HHmmss").format(Calendar.getInstance().getTime()));
            }
            catch (Exception e)
            {
            }
        }
        if (acted)
            _socketSessionThreshold = _socketSessionThreshold / 2;
        else
            _socketSessionThreshold = _socketSessionThreshold * 2;
    }

}
