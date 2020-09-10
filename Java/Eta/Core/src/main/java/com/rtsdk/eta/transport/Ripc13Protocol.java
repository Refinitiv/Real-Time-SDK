package com.rtsdk.eta.transport;

import java.net.InetAddress;
import java.net.UnknownHostException;
import java.nio.ByteBuffer;

class Ripc13Protocol extends IpcProtocol
{
    /* This includes MessageLength(2), Flags(1), ConnectionVersion(4), Flags(1),
     * HeaderLength(1), CompBitmapSize(1), PingTimeout(1), SessionFlags(1),
     * ProtocolType(1), MajorVersion(1), MinorVersion(1).
     * It does not include compression bitmap, hostName, ipAddress, and productVersion container,
     * which the specification includes and will need to be added on later. */
    static final int MIN_CONNECT_REQUEST_HEADER = 15;

    Ripc13Protocol()
    {
    }

    @Override
    int connectionVersion()
    {
        return Ripc.ConnectionVersions.VERSION13;
    }

    @Override
    int ripcVersion()
    {
        return Ripc.RipcVersions.VERSION13;
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
        short headerLength = 0;
        String hostName;
        String ipAddress;
        // using short to hold unsigned byte for hostNameLen, ipAddressLen and componentVersion.
        short hostNameLen;
        short ipAddressLen;
        short componentVersionLen = 0;

        hostName = InetAddress.getLocalHost().getHostName();
        hostNameLen = (short)hostName.length();
        ipAddress = InetAddress.getLocalHost().getHostAddress();
        ipAddressLen = (byte)ipAddress.length();

        /* add the Connected Component Version length to headerSize */
        if (_protocolOptions._componentInfo != null && _protocolOptions._componentInfo.componentVersion() != null)
        {
            componentVersionLen = (short)_protocolOptions._componentInfo.componentVersion().length();
            if (componentVersionLen > MAX_COMPONENT_VERSION_LENGTH)
                componentVersionLen = MAX_COMPONENT_VERSION_LENGTH;
        }

        /* messageLength is CONNECT_REQ_HEADER(15) + hostname(variable),
         * hostnamelen(1), ipaddress(variable), ipaddresslen(1), and
         * componentVersionContainerLen(1), componentVersionTextLen(1),
         * componentVersion(variable) and compressionBitmapSize */
        messageLength = MIN_CONNECT_REQUEST_HEADER + hostNameLen + ipAddressLen + 2 + componentVersionLen + 2;

        /* headerLength is CONNECT_REQ_HEADER(15) + hostname(variable),
         * hostnamelen(1), ipaddress(variable), ipaddresslen(1), and compressionBitmapSize. */
        headerLength = (short)(MIN_CONNECT_REQUEST_HEADER + hostNameLen + ipAddressLen + 2);

        /* add compression to messageLength */
        if (_protocolOptions._sessionInDecompress != 0)
        {
            messageLength += Ripc.COMP_BITMAP_SIZE;
            headerLength += Ripc.COMP_BITMAP_SIZE;
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
        buffer.put((byte)headerLength);

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
            buffer.put(_protocolOptions._compressionBitmap);
        }

        /* Ping Timeout */
        buffer.put((byte)_protocolOptions._pingTimeout);

        /* Session Flags - value set when decoding ConnectAck */
        _protocolOptions._serverSessionFlags = 0;
        buffer.put(_protocolOptions._serverSessionFlags);

        /* Protocol Type */
        buffer.put((byte)_protocolOptions._protocolType);

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

        /* Connected Component Version Container Length */
        buffer.put((byte)(componentVersionLen + 1));
        
        /* Connected Component Version Length */
        buffer.put((byte)componentVersionLen);
        
        /* Connected Component Version */
        if (componentVersionLen > 0)
        {
            ByteBuffer cv = _protocolOptions._componentInfo.componentVersion().data();
            int pos = _protocolOptions._componentInfo.componentVersion().position();
            cv.limit(pos + componentVersionLen);
            cv.position(pos);
            buffer.put(cv);
        }

        buffer.flip();
        return buffer;
    }

    @Override
    ByteBuffer encodeConnectionAck(ByteBuffer buffer, Error error)
    {
        int flags = IPC_EXTENDED_FLAGS;
        int componentVersionLen = 0;

        /* force compression here */
        if ((_protocolOptions._sessionOutCompression != 0) && (_protocolOptions._forceCompression))
            flags |= RIPC_COMP_FORCE;

        /* add the Connected Component Version length to headerSize */
        if (_protocolOptions._componentInfo != null && _protocolOptions._componentInfo.componentVersion() != null)
        {
            componentVersionLen = (short)_protocolOptions._componentInfo.componentVersion().length();
            if (componentVersionLen > MAX_COMPONENT_VERSION_LENGTH)
                componentVersionLen = MAX_COMPONENT_VERSION_LENGTH;
        }

        /* messageLength is CONNECT_ACK_HEADER(10) + maxUserMsgSize(2) +
         * SessionFlags(1) + PingTimeout(1) + MajorVersion(1) + MinorVersion(1)
         * + CompressionType(2) + CompressionLevel(1) +
         * ConnectedComponentContainerLength(1) + ComponentNameLength(1) + ComponentName(variable). */
        int messageLength = CONNECT_ACK_HEADER + 9 + componentVersionLen + 2;

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
        buffer.putInt(ripcVersion());

        /* Max User Msg Size */
        buffer.putShort((short)_protocolOptions._maxUserMsgSize);

        /* Session Flags */
        buffer.put(_protocolOptions._serverSessionFlags);

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

        /* Connected Component (a.k.a. Product) Version Container Length */
        buffer.put((byte)(componentVersionLen + 1));
        
        /* Connected Component (a.k.a. Product) Version Length */
        buffer.put((byte)componentVersionLen);
        
        /* Connected Component (a.k.a. Product) Version */
        if (componentVersionLen > 0)
        {
            ByteBuffer cv = _protocolOptions._componentInfo.componentVersion().data();
            int pos = _protocolOptions._componentInfo.componentVersion().position();
            cv.limit(pos + componentVersionLen);
            cv.position(pos);
            buffer.put(cv);
        }

        /* flip for writing */
        buffer.flip();
        return buffer;
    }

    ByteBuffer encodeHTTPConnectionAck(ByteBuffer buffer, Error error)
    {
        int flags = IPC_EXTENDED_FLAGS;
        int componentVersionLen = 0;

        /* force compression here */
        if ((_protocolOptions._sessionOutCompression != 0) && (_protocolOptions._forceCompression))
            flags |= RIPC_COMP_FORCE;

        /* add the Connected Component Version length to headerSize */
        if (_protocolOptions._componentInfo != null && _protocolOptions._componentInfo.componentVersion() != null)
        {
            componentVersionLen = (short)_protocolOptions._componentInfo.componentVersion().length();
            if (componentVersionLen > MAX_COMPONENT_VERSION_LENGTH)
                componentVersionLen = MAX_COMPONENT_VERSION_LENGTH;
        }

        /* messageLength is CONNECT_ACK_HEADER(10) + maxUserMsgSize(2) +
         * SessionFlags(1) + PingTimeout(1) + MajorVersion(1) + MinorVersion(1)
         * + CompressionType(2) + CompressionLevel(1) +
         * ConnectedComponentContainerLength(1) + ComponentNameLength(1) + ComponentName(variable). */
        int messageLength = CONNECT_ACK_HEADER + 9 + componentVersionLen + 2;

        buffer.clear();

        int dataStart0 = buffer.position();
        buffer.position(2); // data length (2-byte value will be put in here later)
        buffer.put((byte)0x0D);
        buffer.put((byte)0x0A);

        int dataStart = buffer.position();

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
        buffer.putInt(ripcVersion());

        /* Max User Msg Size */
        buffer.putShort((short)_protocolOptions._maxUserMsgSize);

        /* Session Flags */
        buffer.put(_protocolOptions._serverSessionFlags);

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

        /* Connected Component (a.k.a. Product) Version Container Length */
        buffer.put((byte)(componentVersionLen + 1));
        
        /* Connected Component (a.k.a. Product) Version Length */
        buffer.put((byte)componentVersionLen);
        
        /* Connected Component (a.k.a. Product) Version */
        if (componentVersionLen > 0)
        {
            ByteBuffer cv = _protocolOptions._componentInfo.componentVersion().data();
            int pos = _protocolOptions._componentInfo.componentVersion().position();
            cv.limit(pos + componentVersionLen);
            cv.position(pos);
            buffer.put(cv);
        }

        int end = buffer.position();
        int dataLength = end - dataStart;

        // this is the /r/n at the end of HTTP chunked header
        buffer.put((byte)0x0D);
        buffer.put((byte)0x0A);

        end = buffer.position();

        // now that we can get the data length, go put it in the beginning of HTTP chunk
        buffer.position(dataStart0); // position back to beginning to put data chunk length in ASCII

        // put data chunk length in ASCII
        byte[] dataLengthBytesTemp = Integer.toHexString(dataLength).getBytes();
        byte[] dataLengthBytes = { 0x3B, 0x3B, 0x3B, 0x3B };
        for (int i = 0; i < dataLengthBytesTemp.length; i++)
            dataLengthBytes[i] = dataLengthBytesTemp[i];
        // length will not take up more than 2 bytes
        buffer.put(dataLengthBytes[0]);
        buffer.put(dataLengthBytes[1]);

        /* flip for writing */
        buffer.position(end);
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

        /* Compression Bitmap Size */
        int compBitmapSize = buffer.get(position + 9) & 0xFF;

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
                        compbitmap = buffer.get(position + 10 + idx);
                        if ((compbitmap & _ripccompressions[i][RIPC_COMP_BYTEBIT]) > 0)
                        {
                            _protocolOptions._sessionOutCompression = _ripccompressions[i][RIPC_COMP_TYPE];
                            break;
                        }
                    }
                }
            }
            else
            // force compression
            {
                _protocolOptions._sessionOutCompression = _protocolOptions._sessionInDecompress;
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

        /* Protocol Type */
        _protocolOptions._protocolType = buffer.get(bufferIndex++) & 0xFF;

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

        /* Connected Component Version - container length (uint8) */
        /* read and convert from signed byte to unsigned int */
        int componentVersionLen = 0;
        int componentVersionContainerLen = buffer.get(bufferIndex++) & 0xFF;

        _protocolOptions._receivedComponentVersionList.clear();
        if (componentVersionContainerLen > 0)
        {
            /* Connected Component Version - name length (uint8) */
            /* read and convert from signed byte to unsigned int */
            componentVersionLen = buffer.get(bufferIndex++) & 0xFF;
            if (componentVersionLen > 0)
            {
                ComponentInfo ci = new ComponentInfoImpl();
                ci.componentVersion().data(buffer, bufferIndex, componentVersionLen);
                _protocolOptions._receivedComponentVersionList.add(ci);
                bufferIndex += componentVersionLen;
            }
        }

        int adjustedHeader = headerLength + componentVersionContainerLen + 1;
        if (adjustedHeader != messageLength && (messageLength - adjustedHeader) == 2)
        {
            /* Browsers put CR/NEWL after the end of all messages they send.
             * If this is the case, ignore that last two characters. */
            if (buffer.get(position + messageLength - 2) == '\r' && buffer.get(position + messageLength - 1) == '\n')
                messageLength = adjustedHeader;
        }

        if (adjustedHeader != messageLength)
        {
            error.channel(_channel);
            error.errorId(TransportReturnCodes.FAILURE);
            error.sysError(0);
            error.text("Invalid header size " + adjustedHeader);
            return TransportReturnCodes.FAILURE;
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
            /* The server has forced compression. Use ZLIB since that is what everyone supports for RIPC13. */
            compressionType = Ripc.CompressionTypes.ZLIB;
        }
        _protocolOptions._sessionCompType = compressionType;
        _protocolOptions._sessionInDecompress = compressionType;
        _protocolOptions._sessionOutCompression = compressionType;

        /* Compression Level */
        _protocolOptions._sessionCompLevel = (short)(buffer.get(bufferIndex++) & 0xFF);

        /* Server Component Version */
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

}
