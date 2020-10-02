package com.refinitiv.eta.transport;

import java.util.ArrayList;
import java.util.List;

class IpcProtocolOptions
{
    boolean _serverForceCompression = false;
    int _sessionInDecompress;
    int _sessionOutCompression;
    short _sessionCompLevel; // unsigned byte
    int _sessionCompType; // unsigned short
    byte _compressionBitmap;
    boolean _forceCompression;

    int _protocolType;
    int _majorVersion;
    int _minorVersion;
    boolean _nakMount = false;
    byte _serverSessionFlags;
    int _pingTimeout;

    boolean _keyExchange = false;
    long _P;
    long _G;
    long _send_key;
    long _random_key;
    long _shared_key; // this is the key used for encryption/decryption; the other things are needed to calculate this
    byte _encryptionType;
    
    /* Unsigned short. This is the RIPC MAX_USER_MSG_SIZE that will be sent in
     * the RIPC ConnectAck message from the server to consumer.
     * The value will be taken from the BindOptions.maxFragmentSize().
     * 
     * Note: Buffer sizes in ETAJ will be MAX_USER_MSG_SIZE + RIPC_HDR_SIZE.
     * Channel.info.maxFragmentSize() will be MAX_USER_MSG_SIZE - PACKED_HDR_SIZE. */
    int _maxUserMsgSize;
    
    String _clientHostName;
    String _clientIpAddress;

    /* This is the Component Info that we are going to encode on ConnectReq and ConnectAck */
    ComponentInfo _componentInfo;
    
    /* This is the Component Info(s) that we receive when decoding ConnectReq and ConnectAck. */
    List<ComponentInfo> _receivedComponentVersionList = new ArrayList<ComponentInfo>(1);

    void options(ConnectOptions options)
    {
        _sessionInDecompress = options.compressionType();
        _sessionOutCompression = 0;
        _sessionCompLevel = (byte)0; // value from ConnectAck
        _compressionBitmap = (byte)_sessionInDecompress;
        _pingTimeout = options.pingTimeout();
        _protocolType = options.protocolType();
        _majorVersion = options.majorVersion();
        _minorVersion = options.minorVersion();
        _serverSessionFlags = (byte)0; // value from ConnectAck
        _forceCompression = false;
        _keyExchange = false;
        _P = 0;
        _G = 0;
        _send_key = 0;
        _random_key = 0;
        _shared_key = 0;
        _encryptionType = 0;
    }
    
    void options(BindOptions bindOptions)
    {
        _serverForceCompression = bindOptions.forceCompression();
        _sessionInDecompress = bindOptions.compressionType();
        _sessionOutCompression = 0;
        _sessionCompLevel = (byte)bindOptions.compressionLevel();
        _compressionBitmap = (byte)_sessionInDecompress;
        _forceCompression = bindOptions.forceCompression();
        _protocolType = bindOptions.protocolType();
        _majorVersion = bindOptions.majorVersion();
        _minorVersion = bindOptions.minorVersion();
        _maxUserMsgSize = bindOptions.maxFragmentSize();
        _keyExchange = false;
        _P = 0;
        _G = 0;
        _send_key = 0;
        _random_key = 0;
        _shared_key = 0;
        _encryptionType = 0;

        /* _pingTimeout is not set for bindOptions since it will be negotiated when the ConnectReq is received. */

        /* _serverSessionFlags will be the logical OR of the bindOptions server
         * to client pings and client to server pings value, OR'ed with the sessionFlags read from the ConnectReq.
         * The value will be used/sent in the sessionFlags for the ConnectAck. */
        if (bindOptions.serverToClientPings())
            _serverSessionFlags |= Ripc.SessionFlags.SERVER_TO_CLIENT_PING;
        if (bindOptions.clientToServerPings())
            _serverSessionFlags |= Ripc.SessionFlags.CLIENT_TO_SERVER_PING;
    }
}
