package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;

import java.net.URI;

class WebSocketSession {

    /* Don't maintain compression context on messages we send (no outbound 'context takeover'). */
    private static final int DEFLATE_NO_OUTBOUND_CONTEXT_TAKEOVER = 0x1;
    /* Don't maintain compression context on messages we send (no outbound 'context takeover'). */
    private static final int DEFLATE_NO_INBOUND_CONTEXT_TAKEOVER = 0x2;

    private static final int COMPRESSION_SUPPORT_FLAG = 0x1;

    private static final int ORIGIN_SENDING_FLAG = 0x2;

    private HttpRequest handshakeOpeningRequest;

    private HttpResponse handshakeOpeningResponse;

    private boolean upgrade;

    private boolean connectionUpgrade;

    private boolean deflate;

    private boolean encrypted;

    private String protocolName;

    private int acceptedProtocol;

    private URI webSocketUri;

    private String host;

    private int port;

    private String completeHostName;

    private String origin;

    private Buffer wsSessionKey;

    private Buffer wsSessionAcceptKey;

    private int recoveryVersion;

    private int version;

    private WSocketOpts webSocketOpts = new WSocketOptsImpl();

    private int compressionFlags;

    private int sendFlags;

    WebSocketFrameHdr wsFrameHdr = new WebSocketFrameHdr();
    int maxPayload;
    boolean	recvClose;
    boolean sendClose;
    boolean pingRecvd;
    boolean sendPong;
    boolean isClient;
    int		maxMsgSize;
    ByteBufferPair	reassemblyBuffer; /* This is use to assemble fragmented messages. */
    int reassemblyBufferLength;
    int reassemblyBufferDataLength;
    
    int compressedLargeBufSize;
    int posCompressedLargeBuf;
    byte[] compressedLargeBuf; /* This is the compressed byte array for splitting fragmented messages. */
    int totalFragmentedHeaders;

    private boolean handshakeFinished;

    public void initialize() {
        wsSessionKey = CodecFactory.createBuffer();
        wsSessionAcceptKey = CodecFactory.createBuffer();
        handshakeOpeningRequest = new HttpRequest();
        handshakeOpeningResponse = new HttpResponse();
        acceptedProtocol = -1;
    }

    public void clear() {
        upgrade = false;
        connectionUpgrade = false;
        host = "";
        origin = "";
        if (wsSessionKey != null)
            wsSessionKey.clear();
        if (wsSessionAcceptKey != null)
            wsSessionAcceptKey.clear();
        recoveryVersion = 0;
        version = 0;
        if (handshakeOpeningRequest != null)
            handshakeOpeningRequest.clear();
        if (handshakeOpeningResponse != null)
            handshakeOpeningResponse.clear();
        webSocketOpts = null;
        reassemblyBuffer = null;
        reassemblyBufferLength = 0;
        reassemblyBufferDataLength = 0;
        wsFrameHdr.clear();
        deflate = false;
        protocolName = null;
        compressionFlags = 0;
        encrypted = false;
        sendFlags = 0;
        acceptedProtocol = -1;
        maxPayload = 0;
        recvClose = false;
        sendClose = false;
        pingRecvd = false;
        sendPong = false;
        isClient = false;
        maxMsgSize = 0;
        handshakeFinished = false;
        completeHostName = null;
        port = 0;
        compressedLargeBufSize = 0;
        posCompressedLargeBuf = 0;
        compressedLargeBuf = null;
        totalFragmentedHeaders = 0;
    }

    public void setProtocolVersion(String protocolName) {
        this.protocolName = protocolName;
        this.acceptedProtocol = WebSocketSupportedProtocols.defineProtocol(protocolName);
    }

    public HttpRequest getHandshakeOpeningRequest() {
        return handshakeOpeningRequest;
    }

    public HttpResponse getHandshakeOpeningResponse() {
        return handshakeOpeningResponse;
    }

    public boolean isEncrypted() {
        return encrypted;
    }

    public void setEncrypted(boolean encrypted) {
        this.encrypted = encrypted;
    }

    public boolean isUpgrade() {
        return upgrade;
    }

    public void setUpgrade(boolean upgrade) {
        this.upgrade = upgrade;
    }

    public boolean isDeflate() {
        return deflate;
    }

    public void setDeflate(boolean deflate) {
        this.deflate = deflate;
    }

    public int getAcceptedProtocol() {
        return acceptedProtocol;
    }

    public String getHost() {
        return host;
    }

    public void setHost(String host) {
        this.host = host;
    }

    public int getPort() {
        return port;
    }

    public void setPort(int port) {
        this.port = port;
    }

    public String getCompleteHostName() {
        return completeHostName;
    }

    public void setCompleteHostName(String completeHostName) {
        this.completeHostName = completeHostName;
    }

    public String getOrigin() {
        return origin;
    }

    public void setOrigin(String origin) {
        this.origin = origin;
    }

    public boolean isConnectionUpgrade() {
        return connectionUpgrade;
    }

    public void setConnectionUpgrade(boolean connectionUpgrade) {
        this.connectionUpgrade = connectionUpgrade;
    }

    public Buffer getWsSessionKey() {
        return wsSessionKey;
    }

    public Buffer getWsSessionAcceptKey() {
        return wsSessionAcceptKey;
    }

    public int getRecoveryVersion() {
        return recoveryVersion;
    }

    public void setRecoveryVersion(int recoveryVersion) {
        this.recoveryVersion = recoveryVersion;
    }

    public int getVersion() {
        return version;
    }

    public void setVersion(int version) {
        this.version = version;
    }

    public String getProtocolName() {
        return protocolName;
    }

    public URI getWebSocketUri() {
        return webSocketUri;
    }

    public void setWebSocketUri(URI webSocketUri) {
        this.webSocketUri = webSocketUri;
    }

    public WSocketOpts getWebSocketOpts() {
        return webSocketOpts;
    }

    public void setWebSocketOpts(WSocketOpts webSocketOpts) {
        this.webSocketOpts = webSocketOpts;
    }

    public void applyNoOutboundContextTakeOver() {
        this.compressionFlags |= DEFLATE_NO_OUTBOUND_CONTEXT_TAKEOVER;
    }

    public void applyNoInboundContextTakeOver() {
        this.compressionFlags |= DEFLATE_NO_INBOUND_CONTEXT_TAKEOVER;
    }

    public void applyCompressionSupport() {
        this.sendFlags |= COMPRESSION_SUPPORT_FLAG;
    }

    public void applyOriginSending() {
        this.sendFlags |= ORIGIN_SENDING_FLAG;
    }

    public boolean hasNoOutboundContext() {
        return (this.compressionFlags & DEFLATE_NO_OUTBOUND_CONTEXT_TAKEOVER) != 0;
    }

    public boolean hasNoInboundContext() {
        return (this.compressionFlags & DEFLATE_NO_INBOUND_CONTEXT_TAKEOVER) != 0;
    }

    public boolean hasCompressionSupport() {
        return (this.sendFlags & COMPRESSION_SUPPORT_FLAG) != 0;
    }

    public boolean hasOriginSending() {
        return (this.sendFlags & ORIGIN_SENDING_FLAG) != 0;
    }

    public boolean isHandshakeFinished() {
        return handshakeFinished;
    }

    public void setHandshakeFinished(boolean handshakeFinished) {
        this.handshakeFinished = handshakeFinished;
    }
}