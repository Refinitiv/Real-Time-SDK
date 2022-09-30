/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Internal;
using System.Diagnostics;
using System.Text;

namespace Refinitiv.Eta.Internal
{
    class Ripc14Protocol : IpcProtocol
    {
        /* This includes MessageLength(2), Flags(1), ConnectionVersion(4), Flags(1),
        * HeaderLength(1), CompBitmapSize(1), PingTimeout(1), SessionFlags(1),
        * ProtocolType(1), MajorVersion(1), MinorVersion(1).
        * It does not include compression bitmap, hostName, ipAddress, and productVersion container,
        * which the specification includes and will need to be added on later.
        * Also includes the key negotiation elements and has the three way handshake */
        const int MIN_CONNECT_REQUEST_HEADER = 15;

        private EncryptDecryptHelpers m_EncryptDecryptHelpers = new EncryptDecryptHelpers();

        public override ConnectionsVersions ConnectionVersion()
        {
            return ConnectionsVersions.VERSION14;
        }

        public override TransportReturnCode DecodeClientKey(ByteBuffer buffer, int offset, out Error error)
        {
            error = null;
            ushort msgLen = 0; // unsigned short
            short keyExchange = 0;
            short keyLength = 0;
            int bufferIndex = offset;

            /* Message Length */
            msgLen = (ushort)(buffer.ReadShortAt(bufferIndex) & 0xFFFF);
            bufferIndex += 2;

            buffer.ReadPosition = offset;

            /* key exchange */
            keyExchange = (short)(buffer.ReadByteAt(bufferIndex++) & 0xFF);

            /* key length */
            keyLength = (short)(buffer.ReadByteAt(bufferIndex++) & 0xFF);

            if (keyLength > 0)
            {
                /* If encryptionType is TR_SL_1 its 8 bytes */
                if (ProtocolOptions.EncryptionType == EncryptDecryptHelpers.TR_SL1_64)
                {
                    ProtocolOptions.Send_Key = buffer.ReadLongAt(bufferIndex);
                    bufferIndex += 8;
                    ProtocolOptions.Shared_Key = m_EncryptDecryptHelpers.FastModPow(ProtocolOptions.Send_Key, ProtocolOptions.Random_Key, ProtocolOptions._P);
                }
                else
                {
                    bufferIndex += keyLength;
                    /* set encryption type to none and expose 0 key */
                    ProtocolOptions.EncryptionType = 0;
                }
            }
            else
            {
                /* client can't do our encryption */
                bufferIndex += keyLength;
                /* set encryption type to none and expose 0 key */
                ProtocolOptions.EncryptionType = 0;
            }

            /* set the position to the end of the message */
            buffer.ReadPosition = msgLen + offset;

            return TransportReturnCode.SUCCESS;
        }

        public override TransportReturnCode DecodeConnectionReply(ByteBuffer buffer, int offset, out Error error)
        {
            int msgLen = 0; // unsigned short
            byte flags = 0;
            int opCode = 0;
            int ripcVersionNumber; // unsigned int
            int bufferIndex = offset;
            error = null;

            /* Message Length */
            msgLen = buffer.ReadShortAt(bufferIndex) & 0xFFFF;
            bufferIndex += 2;

            buffer.ReadPosition = offset;

            /* RIPC Flags */
            flags = (byte)buffer.ReadByteAt(bufferIndex++);
            if ((flags & (byte)RipcFlags.HAS_OPTIONAL_FLAGS) > 0)
                opCode = buffer.ReadByteAt(bufferIndex++);

            if ((opCode & (byte)RipcOpCode.CONNECT_NAK) > 0)
            {
                /* ConnectNak received. */
                /* set bufferIndex to position of error text length */
                bufferIndex += 2;
                int errorTextLength = buffer.ReadShortAt(bufferIndex) & 0xFFFF;
                if (errorTextLength > 0)
                {
                    byte[] errorText = new byte[errorTextLength - 1];
                    bufferIndex += 2;
                    buffer.ReadPosition = bufferIndex;
                    buffer.ReadBytesInto(errorText, 0, errorTextLength - 1);

                    error = new Error
                    {
                        Channel = Channel,
                        ErrorId = TransportReturnCode.CHAN_INIT_REFUSED,
                        SysError = 0,
                        Text = Encoding.ASCII.GetString(errorText)
                    };
                }

                return TransportReturnCode.CHAN_INIT_REFUSED;
            }
            else if ((opCode & (byte)RipcOpCode.CONNECT_ACK) == 0)
            {
                error = new Error
                {
                    Channel = Channel,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = $"Invalid IPC Mount Opcode ({opCode})"
                };

                return TransportReturnCode.FAILURE;
            }

            /* This is a ConnectAck */

            /* skip HeaderLen and Unknown (unused) byte */
            bufferIndex += 2; // skip headerLen and unused byte

            /* IPC Version number */
            /* read an unsigned int into a signed int */
            ripcVersionNumber = buffer.ReadIntAt(bufferIndex);
            bufferIndex += 4;
            if (ripcVersionNumber != (int)RipcVersion())
            {
                error = new Error
                {
                    Channel = Channel,
                    ErrorId = TransportReturnCode.CHAN_INIT_REFUSED,
                    SysError = 0,
                    Text = "incorrect version received from server"
                };

                return TransportReturnCode.FAILURE;
            }

            /* Maximum User Message Size */
            /* convert from signed short to unsigned short */
            ProtocolOptions.MaxUserMsgSize = buffer.ReadShortAt(bufferIndex) & 0xFFFF;
            bufferIndex += 2;

            /* Session Flags */
            ProtocolOptions.ServerSessionFlags = (RipcSessionFlags)buffer.ReadByteAt(bufferIndex++);

            /* Ping Timeout - convert from signed byte to unsigned byte */
            ProtocolOptions.PingTimeout = buffer.ReadByteAt(bufferIndex++) & 0xFF;

            /* Major Version */
            ProtocolOptions.MajorVersion = buffer.ReadByteAt(bufferIndex++) & 0xFF;

            /* Minor Version */
            ProtocolOptions.MinorVersion = buffer.ReadByteAt(bufferIndex++) & 0xFF;

            /* convert from signed short to unsigned short */
            int compressionType = buffer.ReadShortAt(bufferIndex) & 0xFFFF;
            bufferIndex += 2;
            if (compressionType > RipcCompressionTypes.MAX_DEFINED)
            {
                error = new Error
                {
                    Channel = Channel,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = $"Server wants to do unknown compression type {compressionType}"
                };

                return TransportReturnCode.FAILURE;
            }

            /* check if the server has forced compression */
            if ((flags & (byte)RipcFlags.FORCE_COMPRESSION) > 0 && compressionType == RipcCompressionTypes.NONE)
            {
                /* The server has forced compression. Use ZLIB since that is what everyone supports for RIPC13. */
                compressionType = RipcCompressionTypes.ZLIB;
            }

            ProtocolOptions.SessionCompType = (ushort)compressionType;
            ProtocolOptions.SessionInDecompress = (CompressionType)compressionType;
            ProtocolOptions.SessionOutCompression = (CompressionType)compressionType;

            /* Compression Level */
            ProtocolOptions.SessionCompLevel = (byte)(buffer.ReadByteAt(bufferIndex++) & 0xFF);

            /* Key exchange */
            /* this is client side of the conn ack */
            if (ProtocolOptions.KeyExchange == true)
            {
                short keyExchange = (short)(buffer.ReadByteAt(bufferIndex++) & 0xFF);
                short keyExchangeType = (short)(buffer.ReadByteAt(bufferIndex++) & 0xFF);
                short encLen = (short)(buffer.ReadByteAt(bufferIndex++) & 0xFF);
                if (keyExchangeType == EncryptDecryptHelpers.TR_SL1_64)
                {
                    ProtocolOptions._P = buffer.ReadLongAt(bufferIndex);
                    bufferIndex += 8;
                    ProtocolOptions._G = buffer.ReadLongAt(bufferIndex);
                    bufferIndex += 8;
                    ProtocolOptions.Send_Key = buffer.ReadLongAt(bufferIndex);
                    bufferIndex += 8;
                    ProtocolOptions.EncryptionType = EncryptDecryptHelpers.TR_SL1_64;
                }
                else
                {
                    /* zero out everything */
                    /* we don't know how to handle other types right now */
                    bufferIndex += encLen;
                    ProtocolOptions.EncryptionType = 0;
                }
            }

            /* Server Component Version */
            ProtocolOptions.ReceivedComponentVersionList.Clear();
            /* Connected Component Version - container length (uint8) */
            int componentVersionContainerLen = buffer.ReadByteAt(bufferIndex++) & 0xFF;
            if (componentVersionContainerLen > 0)
            {
                /* Connected Component Version - name length (u15-rb) */
                int componentVersionLen = buffer.ReadByteAt(bufferIndex++) & 0xFF;
                if (componentVersionLen > 0)
                {
                    ComponentInfo ci = new ComponentInfo();
                    ci.ComponentVersion.Data(buffer, bufferIndex, componentVersionLen);
                    ProtocolOptions.ReceivedComponentVersionList.Add(ci);
                    bufferIndex += componentVersionLen;
                }
            }

            /* set the position to the end of the message */
            buffer.ReadPosition = msgLen + offset;

            return TransportReturnCode.SUCCESS;
        }

        public override TransportReturnCode DecodeConnectionReq(ByteBuffer buffer, int position, int messageLength, out Error error)
        {
            Debug.Assert(buffer != null);
            error = null;

            if (messageLength < MIN_CONNECT_REQUEST_HEADER)
            {
                error = new Error
                {
                    Channel = Channel,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = $"Invalid Ripc connection request size ({messageLength})"
                };

                return TransportReturnCode.FAILURE;
            }

            // the opCode (at position 2) is never verified

            /* Key exchange */
            int flags = buffer.ReadByteAt(position + 7) & 0xFF;
            if ((flags & KEY_EXCHANGE) == KEY_EXCHANGE)
            {
                ProtocolOptions.KeyExchange = true;
                /* currently this is the only type, if others are added we need to hook in config */
                ProtocolOptions.EncryptionType = EncryptDecryptHelpers.TR_SL1_64;
            }

            /* HeaderLength */
            int headerLength = buffer.ReadByteAt(position + 8) & 0xFF;

            /* Compression Bitmap Size */
            int compBitmapSize = buffer.ReadByteAt(position + 9) & 0xFF;

            /* Only care about compression if the server wants to do compression */
            if ((compBitmapSize > 0 && ProtocolOptions.SessionInDecompress > 0) || ProtocolOptions.ServerForceCompression)
            {
                /* take bitmap off wire if its there */
                if (compBitmapSize > 0)
                {
                    byte compbitmap;

                    for (int i = 0; i <= RIPC_COMP_MAX_TYPE; i++)
                    {
                        short idx = (short)Ripccompressions[i, RIPC_COMP_BYTEINDEX];
                        if ((idx < RIPC_COMP_BITMAP_SIZE) && (idx < (short)compBitmapSize))
                        {
                            compbitmap = (byte)buffer.ReadByteAt(position + 10 + idx);
                            if ((compbitmap & Ripccompressions[i, RIPC_COMP_BYTEBIT]) > 0)
                            {
                                ProtocolOptions.SessionOutCompression = (CompressionType)Ripccompressions[i, RIPC_COMP_TYPE];
                                break;
                            }
                        }
                    }
                }
                else
                // force compression
                {
                    ProtocolOptions.SessionOutCompression = ProtocolOptions.SessionInDecompress;
                }

                if (ProtocolOptions.SessionOutCompression > (CompressionType)RIPC_COMP_MAX_TYPE)
                    ProtocolOptions.SessionOutCompression = 0;
            }

            /* set buffer index past compression bitmap info */
            int bufferIndex = position + 10 + compBitmapSize;

            /* Ping Timeout */
            ProtocolOptions.PingTimeout = buffer.ReadByteAt(bufferIndex++) & 0xFF;

            /* Session Flags */
            ProtocolOptions.ServerSessionFlags |= (RipcSessionFlags)buffer.ReadByteAt(bufferIndex++);

            /* Protocol Type */
            ProtocolOptions.ProtocolType = (ProtocolType)(buffer.ReadByteAt(bufferIndex++) & 0xFF);

            /* Major Version */
            ProtocolOptions.MajorVersion = buffer.ReadByteAt(bufferIndex++) & 0xFF;

            /* Minor Version */
            ProtocolOptions.MinorVersion = buffer.ReadByteAt(bufferIndex++) & 0xFF;

            /* HostName */
            int hostNameLen = buffer.ReadByteAt(bufferIndex++) & 0xFF;
            if (hostNameLen > 0)
            {
                byte[] hostNameBytes = new byte[hostNameLen];
                buffer.ReadPosition = bufferIndex;
                buffer.ReadBytesInto(hostNameBytes, 0, hostNameLen);
                bufferIndex += hostNameLen;
                ProtocolOptions.ClientHostName = Encoding.ASCII.GetString(hostNameBytes);
            }

            /* IpAddress */
            int ipAddressLen = buffer.ReadByteAt(bufferIndex++) & 0xFF;
            if (ipAddressLen > 0)
            {
                byte[] ipAddressBytes = new byte[ipAddressLen];
                buffer.ReadPosition = bufferIndex;
                buffer.ReadBytesInto(ipAddressBytes, 0, ipAddressLen);
                bufferIndex += ipAddressLen;
                ProtocolOptions.ClientIpAddress = Encoding.ASCII.GetString(ipAddressBytes);
            }

            /* Connected Component Version - container length (uint8) */
            /* read and convert from signed byte to unsigned int */
            int componentVersionLen = 0;
            int componentVersionContainerLen = buffer.ReadByteAt(bufferIndex++) & 0xFF;

            ProtocolOptions.ReceivedComponentVersionList.Clear();
            if (componentVersionContainerLen > 0)
            {
                /* Connected Component Version - name length (uint8) */
                /* read and convert from signed byte to unsigned int */
                componentVersionLen = buffer.ReadByteAt(bufferIndex++) & 0xFF;
                if (componentVersionLen > 0)
                {
                    ComponentInfo ci = new ComponentInfo();
                    int tmpPos = buffer.Position;
                    int tmpLimit = buffer.Limit;

                    /* Allocate buffer */
                    ci.ComponentVersion.Data(new ByteBuffer(componentVersionLen));

                    /* Copy component version into buffer. */
                    buffer.WritePosition = bufferIndex;
                    buffer.Limit = (bufferIndex + componentVersionLen);
                    ci.ComponentVersion.Data().Put(buffer.Contents, buffer.WritePosition, buffer.Limit);
                    buffer.WritePosition = tmpPos;
                    buffer.Limit = tmpLimit;

                    ProtocolOptions.ReceivedComponentVersionList.Add(ci);
                    bufferIndex += componentVersionLen;
                }
            }

            int adjustedHeader = headerLength + componentVersionContainerLen + 1;
            if (adjustedHeader != messageLength && (messageLength - adjustedHeader) == 2)
            {
                /* Browsers put CR/NEWL after the end of all messages they send.
                 * If this is the case, ignore that last two characters. */
                if (buffer.ReadByteAt(position + messageLength - 2) == '\r' && buffer.ReadByteAt(position + messageLength - 1) == '\n')
                    messageLength = adjustedHeader;
            }

            if (adjustedHeader != messageLength)
            {
                error = new Error
                {
                    Channel = Channel,
                    ErrorId = TransportReturnCode.FAILURE,
                    SysError = 0,
                    Text = $"Invalid header size {adjustedHeader}"
                };

                return TransportReturnCode.FAILURE;
            }

            return TransportReturnCode.SUCCESS;
        }

        public override ByteBuffer EncodeClientKey(ByteBuffer buffer, out Error error)
        {
            /* Right now we only have TR_SL_1 key and exchange.
            * If we add more, we need to make this know the different lengths
            * and base it off of the type of keys being exchanged.
            * Current version is 2 (len) + 1 (key exchange) + 1 (key length) + 8 (key) == 12 */
            short messageLength;
            error = null;

            buffer.Clear();

            if (ProtocolOptions.EncryptionType == EncryptDecryptHelpers.TR_SL1_64)
            {
                messageLength = 12;
                /* Message Length */
                buffer.Write(messageLength);
                /* Key Exchange flag */
                buffer.Write((byte)KEY_EXCHANGE);
                /* key length */
                buffer.Write((byte)8);

                /* client key */
                ProtocolOptions.Random_Key = m_EncryptDecryptHelpers.RandLong();
                long client_send_key = m_EncryptDecryptHelpers.FastModPow(ProtocolOptions._G, ProtocolOptions.Random_Key, ProtocolOptions._P);
                ProtocolOptions.Shared_Key =
                        m_EncryptDecryptHelpers.FastModPow(ProtocolOptions.Send_Key, ProtocolOptions.Random_Key, ProtocolOptions._P);
                buffer.Write(client_send_key);
            }
            else
            {
                messageLength = 4;
                /* Message Length */
                buffer.Write(messageLength);
                /* Key Exchange flag */
                buffer.Write((byte)KEY_EXCHANGE);
                buffer.Write((byte)0);
                /* set exposed key to 0 */
            }

            /* flip for writing */
            buffer.Flip();

            return buffer;
        }

        public override ByteBuffer EncodeConnectionAck(ByteBuffer buffer, out Error error)
        {
            int flags = IPC_EXTENDED_FLAGS;
            int componentVersionLen = 0;
            error = null;

            /* force compression here */
            if ((ProtocolOptions.SessionOutCompression != 0) && (ProtocolOptions.ForceCompression))
                flags |= RIPC_COMP_FORCE;

            /* add the Connected Component Version length to headerSize */
            if (ProtocolOptions.ComponentInfo != null && ProtocolOptions.ComponentInfo.ComponentVersion != null)
            {
                componentVersionLen = (short)ProtocolOptions.ComponentInfo.ComponentVersion.Length;
                if (componentVersionLen > MAX_COMPONENT_VERSION_LENGTH)
                    componentVersionLen = MAX_COMPONENT_VERSION_LENGTH;
            }

            /* messageLength is CONNECT_ACK_HEADER(10) + maxUserMsgSize(2) +
             * SessionFlags(1) + PingTimeout(1) + MajorVersion(1) + MinorVersion(1)
             * + CompressionType(2) + CompressionLevel(1) +
             * ConnectedComponentContainerLength(1) + ComponentNameLength(1) +
             * ComponentName(variable). + 8 byte p, 8 byte g, 8 byte server key, 1 byte key exchange */
            int messageLength = CONNECT_ACK_HEADER + 9 + componentVersionLen + 2;

            buffer.Clear();

            /* Message Length */
            int lenPos = buffer.WritePosition;
            buffer.WritePosition = lenPos + 2;

            /* RIPC Flags */
            buffer.Write((byte)flags);

            /* Extended Flags */
            buffer.Write((byte)IPC_CONNACK);

            /* Header Length */
            buffer.Write((byte)CONNECT_ACK_HEADER);

            /* Unknown - unused */
            buffer.Write((byte)0);

            /* IPC Version */
            buffer.Write((int)RipcVersion());

            /* Max User Msg Size */
            buffer.Write((short)ProtocolOptions.MaxUserMsgSize);

            /* Session Flags */
            buffer.Write((byte)ProtocolOptions.ServerSessionFlags);

            /* Ping Timeout */
            buffer.Write((byte)ProtocolOptions.PingTimeout);

            /* Major Version */
            buffer.Write((byte)ProtocolOptions.MajorVersion);

            /* Minor Version */
            buffer.Write((byte)ProtocolOptions.MinorVersion);

            /* Compression Type */
            buffer.Write((short)ProtocolOptions.SessionOutCompression);

            /* Compression Level */
            buffer.Write((byte)ProtocolOptions.SessionCompLevel);

            /* Key Exchange */
            if (ProtocolOptions.KeyExchange == true)
            {
                ProtocolOptions._P = m_EncryptDecryptHelpers.RandLong();
                ProtocolOptions._G = 5;
                buffer.Write((byte)KEY_EXCHANGE);
                /* This version only has one type (TR_SL_1),
                 * if we add others this needs to change to write the correct type */
                buffer.Write(ProtocolOptions.EncryptionType);
                buffer.Write((byte)24); // length of the key exchange stuff
                buffer.Write(ProtocolOptions._P);
                buffer.Write(ProtocolOptions._G);
                ProtocolOptions.Random_Key = m_EncryptDecryptHelpers.RandLong();

                long server_send_key = m_EncryptDecryptHelpers.FastModPow(ProtocolOptions._G, ProtocolOptions.Random_Key, ProtocolOptions._P);
                buffer.Write(server_send_key);
            }

            /* Connected Component (a.k.a. Product) Version Container Length */
            buffer.Write((byte)(componentVersionLen + 1));

            /* Connected Component (a.k.a. Product) Version Length */
            buffer.Write((byte)componentVersionLen);

            /* Connected Component (a.k.a. Product) Version */
            if (componentVersionLen > 0)
            {
                ByteBuffer compVersion = ProtocolOptions.ComponentInfo.ComponentVersion.Data();
                int pos = ProtocolOptions.ComponentInfo.ComponentVersion.Position;
                compVersion.Limit = pos + componentVersionLen;
                buffer.Put(compVersion);
            }

            /* put length in now */
            buffer.WriteAt(lenPos, (short)(buffer.Position - lenPos));

            /* flip for writing */
            buffer.Flip();
            return buffer;
        }

        public override ByteBuffer EncodeConnectionNak(ByteBuffer buffer)
        {
            Debug.Assert(buffer != null);

            buffer.Clear();

            int textLength = CONNECTION_REFUSED.Length + 1 /* NULL for C */;
            int messageLength = CONNECT_NAK_HEADER + textLength;

            /* Message Length */
            buffer.Write((short)messageLength);

            /* RIPC Flags */
            buffer.Write((byte)IPC_EXTENDED_FLAGS);

            /* Extended Flag */
            buffer.Write((byte)IPC_CONNNAK);

            /* Header Length */
            buffer.Write((byte)CONNECT_NAK_HEADER);

            /* Unknown - used */
            buffer.Write((byte)0);

            /* Text Length */
            buffer.Write((short)textLength);

            /* Text */
            buffer.Put(CONNECTION_REFUSED_BYTES, 0, textLength - 1);

            /* NULL for C */
            buffer.Write((byte)0);

            buffer.Flip();
            return buffer;
        }

        public override ByteBuffer EncodeConnectionReq(ByteBuffer buffer)
        {
            Debug.Assert(buffer != null);

            int messageLength = 0;
            ushort headerLength = 0;
            string hostName;
            string ipAddress;
            ushort hostNameLen;
            ushort ipAddressLen;
            ushort componentVersionLen = 0;

            hostName = SocketProtocol.GetLocalHostname();
            hostNameLen = (ushort)hostName.Length;
            ipAddress = SocketProtocol.GetAddressByHostName(hostName);
            ipAddressLen = (ushort)ipAddress.Length;

            /* add the Connected Component Version length to headerSize */
            if (ProtocolOptions.ComponentInfo != null && ProtocolOptions.ComponentInfo.ComponentVersion != null)
            {
                componentVersionLen = (ushort)ProtocolOptions.ComponentInfo.ComponentVersion.Length;
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
            headerLength = (ushort)(MIN_CONNECT_REQUEST_HEADER + hostNameLen + ipAddressLen + 2);

            /* add compression to messageLength */
            if (ProtocolOptions.SessionInDecompress != 0)
            {
                messageLength += RIPC_COMP_BITMAP_SIZE;
                headerLength += RIPC_COMP_BITMAP_SIZE;
            }

            buffer.Clear();

            /* Message Length */
            buffer.Write((short)messageLength);

            /* RIPC Flags - 0=None (ConnectionReq) */
            buffer.Write((byte)0);

            /* connectionVersion - 4 bytes */
            buffer.Write((int)ConnectionVersion());

            /* Add the Key Exchange flag here */
            buffer.Write((byte)KEY_EXCHANGE);
            ProtocolOptions.KeyExchange = true;

            /* header size */
            buffer.Write((byte)headerLength);

            if (ProtocolOptions.SessionInDecompress == 0)
            {
                /* compression bitmap size */
                buffer.Write((byte)0);
            }
            else
            {
                /* compression bitmap size */
                buffer.Write(RIPC_COMP_BITMAP_SIZE);

                /* Compression Type Bitmap */
                buffer.Write(ProtocolOptions.CompressionBitmap);
            }

            /* Ping Timeout */
            buffer.Write((byte)ProtocolOptions.PingTimeout);

            /* Session Flags - value set when decoding ConnectAck */
            ProtocolOptions.ServerSessionFlags = 0;
            buffer.Write((byte)ProtocolOptions.ServerSessionFlags);

            /* Protocol Type */
            buffer.Write((byte)ProtocolOptions.ProtocolType);

            /* Major Version */
            buffer.Write((byte)ProtocolOptions.MajorVersion);

            /* Minor Version */
            buffer.Write((byte)ProtocolOptions.MinorVersion);

            /* HostName */
            buffer.Write((byte)hostNameLen);
            if (hostNameLen > 0)
            {
                buffer.Put(Encoding.ASCII.GetBytes(hostName));
            }

            /* IpAddress */
            buffer.Write((byte)ipAddressLen);
            if (ipAddressLen > 0)
            {
                buffer.Put(Encoding.ASCII.GetBytes(ipAddress));
            }

            /* Connected Component Version Container Length */
            buffer.Write((byte)(componentVersionLen + 1));

            /* Connected Component Version Length */
            buffer.Write((byte)componentVersionLen);

            /* Connected Component Version */
            if (componentVersionLen > 0)
            {
                ByteBuffer compVersion = ProtocolOptions.ComponentInfo.ComponentVersion.Data();
                int pos = ProtocolOptions.ComponentInfo.ComponentVersion.Position;
                compVersion.Limit = pos + componentVersionLen;
                buffer.Put(compVersion);
            }

            buffer.Flip();

            return buffer;
        }

        public override int MinConnectRequestHeader()
        {
            return MIN_CONNECT_REQUEST_HEADER;
        }

        public override RipcVersions RipcVersion()
        {
            return RipcVersions.VERSION14;
        }
    }
}
