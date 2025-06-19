/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.net.UnknownHostException;
import java.nio.ByteBuffer;

abstract class IpcProtocol
{
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
