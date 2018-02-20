package com.thomsonreuters.upa.transport;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

class Ripc11Protocol extends IpcProtocol
{
    /* This includes MessageLength(2), Flags(1), ConnectionVersion(4), Flags(1),
     * HeaderLength(1), CompBitmapSize(1), PingTimeout(1), SessionFlags(1),
     * MajorVersion(1), MinorVersion(1).
     * It does not include compression info, hostName, and ipAddress,
     * which the specification includes and will need to be added on later. */
    static final int MIN_CONNECT_REQUEST_HEADER = 14;

    Ripc11Protocol()
    {
    }

    @Override
    int connectionVersion()
    {
        return Ripc.ConnectionVersions.VERSION11;
    }

    @Override
    int ripcVersion()
    {
        return Ripc.RipcVersions.VERSION11;
    }

    @Override
    int minConnectRequestHeader()
    {
        return MIN_CONNECT_REQUEST_HEADER;
    }

    @Override
    ByteBuffer encodeConnectionReq(ByteBuffer buffer) throws UnknownHostException
    {
        assert (buffer != null);

        int messageLength = 0;
        String hostName;
        String ipAddress;
        // using short to hold unsigned byte for hostNameLen and ipAddressLen.
        short hostNameLen;
        short ipAddressLen;

        hostName = InetAddress.getLocalHost().getHostName();
        hostNameLen = (short)hostName.length();
        ipAddress = InetAddress.getLocalHost().getHostAddress();
        ipAddressLen = (byte)ipAddress.length();

        messageLength = (short)(MIN_CONNECT_REQUEST_HEADER + hostNameLen + ipAddressLen + 2);

        /* add compression to messageLength */
        if (_protocolOptions._sessionInDecompress != 0)
        {
            messageLength += Ripc.COMP_BITMAP_SIZE;
        }

        buffer.clear();

        /* Message Length */
        buffer.putShort((short)messageLength);

        /* RIPC Flags - 0=None (ConnectionReq) */
        buffer.put((byte)0);

        /* connectionVersion - 4 bytes */
        buffer.putInt(connectionVersion());

        /* flags - current not used */
        buffer.put((byte)0);

        /* header size */
        buffer.put((byte)messageLength);

        if (_protocolOptions._sessionInDecompress == 0)
        {
            /* compression bitmap size */
            buffer.put((byte)0);
        }
        else
        {
            /* compression bitmap size */
            buffer.put(Ripc.COMP_BITMAP_SIZE);

            /* Compression Type Bitmap */
            buffer.put((byte)_protocolOptions._compressionBitmap);
        }

        /* Ping Timeout */
        buffer.put((byte)_protocolOptions._pingTimeout);

        /* Session Flags - value set when decoding ConnectAck */
        _protocolOptions._serverSessionFlags = 0;
        buffer.put((byte)_protocolOptions._serverSessionFlags);

        /* Major Version */
        buffer.put((byte)_protocolOptions._majorVersion);

        /* Minor Version */
        buffer.put((byte)_protocolOptions._minorVersion);

        /* HostName */
        buffer.put((byte)hostNameLen);
        if (hostNameLen > 0)
        {
            buffer.put(hostName.getBytes());
        }

        /* IpAddress */
        buffer.put((byte)ipAddressLen);
        if (ipAddressLen > 0)
        {
            buffer.put(ipAddress.getBytes());
        }

        buffer.flip();
        return buffer;
    }

    @Override
    ByteBuffer encodeConnectionAck(ByteBuffer buffer, Error error)
    {
        int flags = IPC_EXTENDED_FLAGS;

        /* force compression here */
        if ((_protocolOptions._sessionOutCompression != 0) && (_protocolOptions._forceCompression))
            flags |= RIPC_COMP_FORCE;

        /* messageLength is CONNECT_ACK_HEADER(10) + maxUserMsgSize(2) +
         * SessionFlags(1) + PingTimeout(1) + MajorVersion(1) + MinorVersion(1)
         * + CompressionType(2) + CompressionLevel(1) */
        int messageLength = CONNECT_ACK_HEADER + 9;

        buffer.clear();

        /* Message Length */
        buffer.putShort((short)messageLength);

        /* RIPC Flags */
        buffer.put((byte)flags);

        /* Extended Flags */
        buffer.put((byte)IPC_CONNACK);

        /* Header Length */
        buffer.put((byte)CONNECT_ACK_HEADER);

        /* Unknown - unused */
        buffer.put((byte)0);

        /* IPC Version */
        buffer.putInt((int)ripcVersion());

        /* Max User Msg Size */
        buffer.putShort((short)_protocolOptions._maxUserMsgSize);

        /* Session Flags */
        buffer.put((byte)_protocolOptions._serverSessionFlags);

        /* Ping Timeout */
        buffer.put((byte)_protocolOptions._pingTimeout);

        /* Major Version */
        buffer.put((byte)_protocolOptions._majorVersion);

        /* Minor Version */
        buffer.put((byte)_protocolOptions._minorVersion);

        /* Compression Type */
        buffer.putShort((short)_protocolOptions._sessionOutCompression);

        /* Compression Level */
        buffer.put((byte)_protocolOptions._sessionCompLevel);

        /* flip for writing */
        buffer.flip();
        return buffer;
    }

    @Override
    ByteBuffer encodeConnectionNak(ByteBuffer buffer)
    {
        assert (buffer != null);

        buffer.clear();

        int textLength = CONNECTION_REFUSED.length() + 1 /* NULL for C */;
        int messageLength = CONNECT_NAK_HEADER + textLength;

        /* Message Length */
        buffer.putShort((short)messageLength);

        /* RIPC Flags */
        buffer.put((byte)IPC_EXTENDED_FLAGS);

        /* Extended Flag */
        buffer.put((byte)IPC_CONNNAK);

        /* Header Length */
        buffer.put((byte)CONNECT_NAK_HEADER);

        /* Unknown - used */
        buffer.put((byte)0);

        /* Text Length */
        buffer.putShort((short)textLength);
        
        /* Text */
        buffer.put(CONNECTION_REFUSED_BYTES, 0, textLength - 1);
        
        /* NULL for C */
        buffer.put((byte)0);

        buffer.flip();
        return buffer;
    }

    @Override
    int decodeConnectionReq(ByteBuffer buffer, int position, int messageLength, Error error)
    {
        assert (buffer != null);

        if (messageLength < MIN_CONNECT_REQUEST_HEADER)
        {
            error.channel(_channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Invalid Ripc connection request size (" + messageLength + ")");
            return TransportReturnCodes.FAILURE;
        }

        // the opCode (at position 2) is never verified

        // flags (at position 7) is never used, so don't read it.

        /* HeaderLength */
        int headerLength = buffer.get(position + 8) & 0xFF;
        if (headerLength != messageLength && (messageLength - headerLength) == 2)
        {
            /* Browsers put CR/NEWL after the end of all messages they send.
             * If this is the case, ignore that last two characters. */
            if (buffer.get(position + messageLength - 2) == '\r' && buffer.get(position + messageLength - 1) == '\n')
                messageLength = headerLength;
        }

        if (headerLength != messageLength)
        {
            error.channel(_channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Invalid 10 header size " + headerLength);
            return TransportReturnCodes.FAILURE;
        }

        /* Compression Bitmap Size */
        int compBitmapSize = buffer.get(9) & 0xFF;

        /* Only care about compression if the server wants to do compression */
        if ((compBitmapSize > 0 && _protocolOptions._sessionInDecompress > 0) || _protocolOptions._serverForceCompression)
        {
            /* take bitmap off wire if its there */
            if (compBitmapSize > 0)
            {
                byte compbitmap;

                for (int i = 0; i <= RIPC_COMP_MAX_TYPE; i++)
                {
                    short idx = _ripccompressions[i][RIPC_COMP_BYTEINDEX];
                    if ((idx < RIPC_COMP_BITMAP_SIZE) && (idx < (short)compBitmapSize))
                    {
                        compbitmap = buffer.get(position + 10 + i);
                        if ((compbitmap & _ripccompressions[i][RIPC_COMP_BYTEBIT]) > 0)
                        {
                            _protocolOptions._sessionOutCompression = _ripccompressions[i][RIPC_COMP_TYPE];
                            break;
                        }
                    }
                }
            }
            else
            {
                /* if we are going to force compression, force zlib since we know all versions of ripc support this */
                _protocolOptions._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
            }

            if (_protocolOptions._sessionOutCompression > RIPC_COMP_MAX_TYPE)
                _protocolOptions._sessionOutCompression = 0;
        }

        /* set buffer index past compression bitmap info */
        int bufferIndex = position + 10 + compBitmapSize;

        /* Ping Timeout */
        _protocolOptions._pingTimeout = buffer.get(bufferIndex++) & 0xFF;

        /* Session Flags */
        _protocolOptions._serverSessionFlags |= buffer.get(bufferIndex++);

        /* Major Version */
        _protocolOptions._majorVersion = buffer.get(bufferIndex++) & 0xFF;

        /* Minor Version */
        _protocolOptions._minorVersion = buffer.get(bufferIndex++) & 0xFF;

        /* HostName */
        int hostNameLen = buffer.get(bufferIndex++) & 0xFF;
        if (hostNameLen > 0)
        {
            byte[] hostNameBytes = new byte[hostNameLen];
            buffer.position(bufferIndex);
            buffer.get(hostNameBytes, 0, hostNameLen);
            bufferIndex += hostNameLen;
            _protocolOptions._clientHostName = new String(hostNameBytes);
        }

        /* IpAddress */
        int ipAddressLen = buffer.get(bufferIndex++) & 0xFF;
        if (ipAddressLen > 0)
        {
            byte[] ipAddressBytes = new byte[ipAddressLen];
            buffer.position(bufferIndex);
            buffer.get(ipAddressBytes, 0, ipAddressLen);
            bufferIndex += ipAddressLen;
            _protocolOptions._clientIpAddress = new String(ipAddressBytes);
        }

        return TransportReturnCodes.SUCCESS;
    }

    @Override
    int decodeConnectionReply(ByteBuffer buffer, int offset, Error error)
    {
        int msgLen = 0; // unsigned short
        byte flags = 0;
        int opCode = 0;
        int ripcVersionNumber; // unsigned int
        int bufferIndex = offset;

        /* Message Length */
        msgLen = buffer.getShort(bufferIndex) & 0xFFFF;
        bufferIndex += 2;

        buffer.position(offset);

        /* RIPC Flags */
        flags = buffer.get(bufferIndex++);
        if ((flags & Ripc.Flags.HAS_OPTIONAL_FLAGS) > 0)
            opCode = buffer.get(bufferIndex++);

        if ((opCode & Ripc.Flags.Optional.CONNECT_NAK) > 0)
        {
            /* ConnectNak received. */
            /* set bufferIndex to position of error text length */
            bufferIndex += 2;
            int errorTextLength = buffer.getShort(bufferIndex) & 0xFFFF;
            if (errorTextLength > 0)
            {
                byte[] errorText = new byte[errorTextLength];
                bufferIndex += 2;
                buffer.position(bufferIndex);
                buffer.get(errorText, 0, errorTextLength);
                error.channel(_channel);
                error.errorId(TransportReturnCodes.CHAN_INIT_REFUSED);
                error.sysError(0);
                error.text(new String(errorText));
            }
            return TransportReturnCodes.CHAN_INIT_REFUSED;
        }
        else if ((opCode & Ripc.Flags.Optional.CONNECT_ACK) == 0)
        {
            error.channel(_channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Invalid IPC Mount Opcode (" + opCode + ")");
            return TransportReturnCodes.FAILURE;
        }

        /* This is a ConnectAck */

        /* skip HeaderLen and Unknown (unused) byte */
        bufferIndex += 2; // skip headerLen and unused byte

        /* IPC Version number */
        /* read an unsigned int into a signed int */
        ripcVersionNumber = buffer.getInt(bufferIndex);
        bufferIndex += 4;
        if (ripcVersionNumber != ripcVersion())
        {
            error.channel(_channel);
            error.errorId(TransportReturnCodes.CHAN_INIT_REFUSED);
            error.sysError(0);
            error.text("incorrect version received from server");
            return TransportReturnCodes.FAILURE;
        }

        /* Maximum User Message Size */
        /* convert from signed short to unsigned short */
        _protocolOptions._maxUserMsgSize = buffer.getShort(bufferIndex) & 0xFFFF;
        bufferIndex += 2;

        /* Session Flags */
        _protocolOptions._serverSessionFlags = buffer.get(bufferIndex++);
        
        /* Ping Timeout - convert from signed byte to unsigned byte */
        _protocolOptions._pingTimeout = buffer.get(bufferIndex++) & 0xFF;
        
        /* Major Version */
        _protocolOptions._majorVersion = buffer.get(bufferIndex++) & 0xFF;
        
        /* Minor Version */
        _protocolOptions._minorVersion = buffer.get(bufferIndex++) & 0xFF;

        /* convert from signed short to unsigned short */
        int compressionType = buffer.getShort(bufferIndex) & 0xFFFF;
        bufferIndex += 2;
        if (compressionType > Ripc.CompressionTypes.MAX_DEFINED)
        {
            error.channel(_channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Server wants to do unknown compression type " + compressionType);
            return TransportReturnCodes.FAILURE;
        }

        /* check if the server has forced compression */
        if ((flags & Ripc.Flags.FORCE_COMPRESSION) > 0 && compressionType == Ripc.CompressionTypes.NONE)
        {
            /* The server has forced compression. Use ZLIB since that is what everyone supports for RIPC12. */
            compressionType = Ripc.CompressionTypes.ZLIB;
        }
        _protocolOptions._sessionInDecompress = compressionType;
        _protocolOptions._sessionOutCompression = compressionType;

        /* Compression Level */
        _protocolOptions._sessionCompLevel = (short)(buffer.get(bufferIndex++) & 0xFF);

        /* set the position to the end of the message */
        buffer.position(msgLen + offset);

        return TransportReturnCodes.SUCCESS;
    }    

    @Override
    ByteBuffer encodeClientKey(ByteBuffer buffer, Error error)
    {
        /* not applicable for this version of the connection handshake */
        error.channel(_channel);
        error.errorId(TransportReturnCodes.CHAN_INIT_REFUSED);
        error.sysError(0);
        error.text("invalid connection handshake");
        return null;
    }

    @Override
    int decodeClientKey(ByteBuffer buffer, int offset, Error error)
    {
        /* not applicable for this version of the connection handshake */
        error.channel(_channel);
        error.errorId(TransportReturnCodes.CHAN_INIT_REFUSED);
        error.sysError(0);
        error.text("invalid connection handshake");
        return TransportReturnCodes.FAILURE;
    }

    @Override
    ByteBuffer encodeHTTPConnectionAck(ByteBuffer buffer, Error error)
    {
        /* not applicable for this version of the connection handshake */
        error.channel(_channel);
        error.errorId(TransportReturnCodes.CHAN_INIT_REFUSED);
        error.sysError(0);
        error.text("HTTP Tunneling is not valid for this connection version");
        return null;
    }

}
