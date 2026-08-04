/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025,2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.net.UnknownHostException;
import java.nio.ByteBuffer;

abstract class IpcProtocol
{
    protected static class ConnectionReplyState
    {
        int msgLen;
        byte flags;
        int opCode;
        int bufferIndex;
    }

    IpcProtocolOptions _protocolOptions = new IpcProtocolOptions();
    Channel _channel;

    protected static final int RIPC_COMP_MAX_TYPE = Ripc.CompressionTypes.LZ4;
    
    /* Array definitions, index of byte in bitmap, bit within the byte, decimal value when sent */
    protected static final int RIPC_COMP_BYTEINDEX = 0;
    protected static final int RIPC_COMP_BYTEBIT = 1;
    protected static final int RIPC_COMP_TYPE = 2;
    /* force compression for connection level */
    protected static final int RIPC_COMP_FORCE = 0x80;
    protected static final int RIPC_COMP_BITMAP_SIZE = 1;
    protected byte _ripccompressions[][] = { { 0, 0x00, Ripc.CompressionTypes.NONE },  /* no compression */
                                             { 0, 0x01, Ripc.CompressionTypes.ZLIB },  /* zlib compression */
                                             { 0, 0x02, Ripc.CompressionTypes.LZ4 } }; /* LZ4 compression */

    protected static final String CONNECTION_REFUSED = "Connection refused.";
    protected static final byte[] CONNECTION_REFUSED_BYTES = CONNECTION_REFUSED.getBytes();

    /* extra byte of flags is present */
    protected static final int IPC_EXTENDED_FLAGS = 0x1;
    protected static final int IPC_CONNACK = 0x1; /* this is a connection ack */
    protected static final int IPC_CONNNAK = 0x2; /* this is a connection nak */

    /* Key exchange flag value */
    protected static final int KEY_EXCHANGE = 0x8;

    /* This includes MessageLength(2), Flags(1), Extended Flags(1), HeaderLength(1), Unknown(1), IpcVersion(4). */
    static final int CONNECT_ACK_HEADER = 10;

    /* This includes MessageLength(2), Flags(1), Extended Flags(1), HeaderLength(1), Unknown(1), TextLength(2). */
    static final int CONNECT_NAK_HEADER = 8;

    /* This includes MessageLength(2) and Flags(1). */
    static final int HEADER_SIZE = 3;

    /* The max length of the Component Version is 254 since the Connected Component Container length
     * is one byte and the component version length itself will take up one byte.
     */
    static final int MAX_COMPONENT_VERSION_LENGTH = 253;

    /* Used to access the protocol options. */
    IpcProtocolOptions protocolOptions()
    {
        return _protocolOptions;
    }

    /* Returns the header size, which includes the messageLength and Flags fields. */
    int headerSize()
    {
        return HEADER_SIZE;
    }

    /* Sets the options for a client connection. */
    void options(ConnectOptions options)
    {
        assert (options != null);
        _protocolOptions.options(options);
    }

    /* Sets the options for a server connection. */
    void options(BindOptions bindOptions)
    {
        assert (bindOptions != null);
        _protocolOptions.options(bindOptions);
    }

    /* Sets the channel for the IPCProtocol */
    void channel(Channel channel)
    {
        assert (channel != null);
        _channel = channel;
    }

    protected int populateError(Error error, int errorId, String text)
    {
        error.channel(_channel);
        error.errorId(errorId);
        error.sysError(0);
        error.text(text);
        return errorId;
    }

    protected int decodeConnectionReplyCommon(ByteBuffer buffer, int offset, Error error,
            ConnectionReplyState state, boolean hasSessionCompType)
    {
        state.bufferIndex = offset;
        int readableBytes = buffer.position() - offset;

        if (readableBytes < Short.BYTES)
        {
            return populateError(error, TransportReturnCodes.FAILURE,
                    "Insufficient bytes available to read message length (" + readableBytes + ")");
        }

        /* Message Length */
        state.msgLen = buffer.getShort(state.bufferIndex) & 0xFFFF;
        if (state.msgLen > readableBytes)
        {
            return populateError(error, TransportReturnCodes.FAILURE,
                    "Message length (" + state.msgLen + ") exceeds available bytes (" + readableBytes + ")");
        }
        state.bufferIndex += 2;

        buffer.position(offset);

        /* RIPC Flags */
        state.flags = buffer.get(state.bufferIndex++);
        state.opCode = 0;
        if ((state.flags & Ripc.Flags.HAS_OPTIONAL_FLAGS) > 0)
            state.opCode = buffer.get(state.bufferIndex++);

        if ((state.opCode & Ripc.Flags.Optional.CONNECT_NAK) > 0)
            return decodeConnectionNak(buffer, error, state);
        else if ((state.opCode & Ripc.Flags.Optional.CONNECT_ACK) == 0)
            return populateError(error, TransportReturnCodes.FAILURE, "Invalid IPC Mount Opcode (" + state.opCode + ")");

        /* This is a ConnectAck */

        /* skip HeaderLen and Unknown (unused) byte */
        state.bufferIndex += 2;

        /* IPC Version number */
        /* read an unsigned int into a signed int */
        int ripcVersionNumber = buffer.getInt(state.bufferIndex);
        state.bufferIndex += 4;
        if (ripcVersionNumber != ripcVersion())
        {
            populateError(error, TransportReturnCodes.CHAN_INIT_REFUSED, "incorrect version received from server");
            return TransportReturnCodes.FAILURE;
        }

        /* Maximum User Message Size */
        /* convert from signed short to unsigned short */
        _protocolOptions._maxUserMsgSize = buffer.getShort(state.bufferIndex) & 0xFFFF;
        state.bufferIndex += 2;

        /* Session Flags */
        _protocolOptions._serverSessionFlags = buffer.get(state.bufferIndex++);

        /* Ping Timeout - convert from signed byte to unsigned byte */
        _protocolOptions._pingTimeout = buffer.get(state.bufferIndex++) & 0xFF;

        /* Major Version */
        _protocolOptions._majorVersion = buffer.get(state.bufferIndex++) & 0xFF;

        /* Minor Version */
        _protocolOptions._minorVersion = buffer.get(state.bufferIndex++) & 0xFF;

        /* convert from signed short to unsigned short */
        int compressionType = buffer.getShort(state.bufferIndex) & 0xFFFF;
        state.bufferIndex += 2;
        if (compressionType > Ripc.CompressionTypes.MAX_DEFINED)
        {
            return populateError(error, TransportReturnCodes.FAILURE,
                    "Server wants to do unknown compression type " + compressionType);
        }

        /* check if the server has forced compression */
        if ((state.flags & Ripc.Flags.FORCE_COMPRESSION) > 0 && compressionType == Ripc.CompressionTypes.NONE)
        {
            /* The server has forced compression. Use ZLIB since that is what everyone supports for older RIPC versions. */
            compressionType = Ripc.CompressionTypes.ZLIB;
        }
        if (hasSessionCompType)
            _protocolOptions._sessionCompType = compressionType;
        _protocolOptions._sessionInDecompress = compressionType;
        _protocolOptions._sessionOutCompression = compressionType;

        /* Compression Level */
        _protocolOptions._sessionCompLevel = (short)(buffer.get(state.bufferIndex++) & 0xFF);

        return TransportReturnCodes.SUCCESS;
    }

    protected int decodeConnectionNak(ByteBuffer buffer, Error error, ConnectionReplyState state)
    {
        /* ConnectNak received. */
        /* Header Length */
        int hdrLen = buffer.get(state.bufferIndex);
        if (state.msgLen != hdrLen)
        {
            return populateError(error, TransportReturnCodes.CHAN_INIT_REFUSED,
                    "Message length (" + state.msgLen + ") doesn't equal header length (" + hdrLen + ")");
        }

        /* set bufferIndex to position of error text length */
        state.bufferIndex += 2;
        int errorTextLength = buffer.getShort(state.bufferIndex) & 0xFFFF;

        /* Text length should be positive and should be equal the message length minus the header length */
        if (errorTextLength > 0 && errorTextLength == (state.msgLen - CONNECT_NAK_HEADER))
        {
            byte[] errorText = new byte[errorTextLength];
            state.bufferIndex += 2;
            buffer.position(state.bufferIndex);
            buffer.get(errorText, 0, errorTextLength);
            return populateError(error, TransportReturnCodes.CHAN_INIT_REFUSED, new String(errorText));
        }

        return populateError(error, TransportReturnCodes.CHAN_INIT_REFUSED,
                "Text length (" + errorTextLength + ") isn't positive or doesn't fit message length (" + state.msgLen + ")");
    }

    protected int decodeComponentVersion(ByteBuffer buffer, int bufferIndex)
    {
        _protocolOptions._receivedComponentVersionList.clear();

        /* Connected Component Version - container length (uint8) */
        int componentVersionContainerLen = buffer.get(bufferIndex++) & 0xFF;
        if (componentVersionContainerLen > 0)
        {
            /* Connected Component Version - name length (u15-rb) */
            int componentVersionLen = buffer.get(bufferIndex++) & 0xFF;
            if (componentVersionLen > 0)
            {
                ComponentInfo ci = new ComponentInfoImpl();
                ci.componentVersion().data(buffer, bufferIndex, componentVersionLen);
                _protocolOptions._receivedComponentVersionList.add(ci);
                bufferIndex += componentVersionLen;
            }
        }

        return bufferIndex;
    }

    /* Returns the connection version of this protocol. */
    abstract int connectionVersion();

    /* Returns the RIPC version of this protocol */
    abstract int ripcVersion();

    /* Returns the minimum size of the connection request header for this protocol. */
    abstract int minConnectRequestHeader();
    
    /* Encodes the RIPC ConnectionReq message and flip the buffer so that it is ready to be written.
     * 
     * Returns a ByteBuffer encoded with the ConnectionReq message.
     */
    abstract ByteBuffer encodeConnectionReq(ByteBuffer byteBuffer)  throws UnknownHostException;

    /* Encodes the RIPC ConnectionAck message and flip the buffer so that it is ready to be written.
     * 
     * buffer is the byteBuffer to encode into.
     * 
     * Returns ByteBuffer encoded with the ConnectionAck message.
     */
    abstract ByteBuffer encodeConnectionAck(ByteBuffer buffer, Error error);
    
    /* Encodes the RIPC ConnectionNak message and flip the buffer so that it is ready to be written.
     * 
     * buffer is the ByteBuffer to encode into.
     * 
     * Returns a ByteBuffer encoded with the ConnectionNak message.
     */
    abstract ByteBuffer encodeConnectionNak(ByteBuffer buffer);

    /* Decodes the RIPC ConnectionReq message and populates IpcProtocolOptions with data read.
     * 
     * byteBuffer is the ByteBuffer to decode from.
     * msgLen is the length of message.
     * 
     * Returns TransportReturnCodes.SUCCESS or TransportReturnCodes.FAILURE
     */
    abstract int decodeConnectionReq(ByteBuffer byteBuffer, int position, int msgLen, Error error);

    /* Decodes a RIPC ConnectionReply (ConnectionAck or ConnectionNak) and
     * populates IpcProtocolOptions with data read.
     * After decoding the ConnectionReply, sets the buffer's position past the message read.
     * 
     * byteBuffer is the ByteBuffer to decode from
     * offset is the offset into the byteBuffer where the RIPC message starts. Will always be zero if not tunneling.
     *            
     * error is the Error to populate if an error occurs.
     * 
     * Returns TransportReturnCodes.CHAN_INIT_REFUSED, TransportReturnCodes.FAILURE, TransportReturnCodes.SUCCESS.
     */
    abstract int decodeConnectionReply(ByteBuffer byteBuffer, int offset, Error error);

    /* Encodes the third leg of the ripc handshake which includes the client encryption key information.
     * This is only applicable to connection version 14 and higher and older versions should never get into here.
     * 
     * buffer is the ByteBuffer to encode into.
     * 
     * Returns a ByteBuffer encoded with the client key.
     */
    abstract ByteBuffer encodeClientKey(ByteBuffer buffer, Error error);    
    
    /* Decodes the third leg of the ripc handshake (conn version 14 or higher)
     * that contains the client key for encryption.
     * Should return error on older versions as they should never get into here.
     * 
     * byteBuffer is the ByteBuffer to decode from
     * offset is the offset into the byteBuffer where the RIPC message starts. Will always be zero if not tunneling.
     * error is the Error to populate if an error occurs.
     * 
     * Returns TransportReturnCodes.CHAN_INIT_REFUSED, TransportReturnCodes.FAILURE, TransportReturnCodes.SUCCESS.
     */
    abstract int decodeClientKey(ByteBuffer byteBuffer, int offset, Error error);
    
    /* Basically encodeConnectionAck with chunking header and footer around it. */
    abstract ByteBuffer encodeHTTPConnectionAck(ByteBuffer buffer, Error error);    

}
