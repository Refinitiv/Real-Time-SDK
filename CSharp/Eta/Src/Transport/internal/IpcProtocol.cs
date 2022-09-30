/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using Refinitiv.Eta.Common;
using Refinitiv.Eta.Transports;
using Refinitiv.Eta.Transports.Interfaces;
using Refinitiv.Eta.Transports.Internal;
using System.Diagnostics;
using System.Text;

namespace Refinitiv.Eta.Internal
{
   abstract class IpcProtocol
   {
        public IpcProtocolOptions ProtocolOptions { get; set; } = new IpcProtocolOptions();

        public IChannel Channel { get; set; }

        protected const int RIPC_COMP_MAX_TYPE = RipcCompressionTypes.LZ4;

        /* Array definitions, index of byte in bitmap, bit within the byte, decimal value when sent */
        protected const int RIPC_COMP_BYTEINDEX = 0;
        protected const int RIPC_COMP_BYTEBIT = 1;
        protected const int RIPC_COMP_TYPE = 2;
        /* force compression for connection level */
        protected const int RIPC_COMP_FORCE = 0x80;
        protected static readonly byte RIPC_COMP_BITMAP_SIZE = 1;

        protected int [,] Ripccompressions = new int[3,3]{ 
                                    { 0, 0x00, RipcCompressionTypes.NONE},  /* no compression */
                                    { 0, 0x01, RipcCompressionTypes.ZLIB},  /* zlib compression */
                                    { 0, 0x02, RipcCompressionTypes.LZ4 } }; /* LZ4 compression */

        protected const string CONNECTION_REFUSED = "Connection refused.";
        protected byte[] CONNECTION_REFUSED_BYTES = Encoding.ASCII.GetBytes(CONNECTION_REFUSED);

        /* extra byte of flags is present */
        protected const int IPC_EXTENDED_FLAGS = 0x1;
        protected const int IPC_CONNACK = 0x1; /* this is a connection ack */
        protected const int IPC_CONNNAK = 0x2; /* this is a connection nak */

        /* Key exchange flag value */
        protected const int KEY_EXCHANGE = 0x8;

        /* This includes MessageLength(2), Flags(1), Extended Flags(1), HeaderLength(1), Unknown(1), IpcVersion(4). */
        public static readonly int CONNECT_ACK_HEADER = 10;

        /* This includes MessageLength(2), Flags(1), Extended Flags(1), HeaderLength(1), Unknown(1), TextLength(2). */
        public static readonly int CONNECT_NAK_HEADER = 8;

        /* This includes MessageLength(2) and Flags(1). */
        public static readonly int HEADER_SIZE = 3;

        /* The max length of the Component Version is 254 since the Connected Component Container length
         * is one byte and the component version length itself will take up one byte.
         */
        public static readonly ushort MAX_COMPONENT_VERSION_LENGTH = 253;

        public void Options(ConnectOptions options)
        {
            Debug.Assert(options != null);
            ProtocolOptions.Options(options);
        }

        public void Options(BindOptions options)
        {
            Debug.Assert(options != null);
            ProtocolOptions.Options(options);
        }

        /* Returns the connection version of this protocol. */
        public abstract ConnectionsVersions ConnectionVersion();

        /* Returns the RIPC version of this protocol */
        public abstract RipcVersions RipcVersion();

        /* Returns the minimum size of the connection request header for this protocol. */
        public abstract int MinConnectRequestHeader();

        /* Encodes the RIPC ConnectionReq message and flip the buffer so that it is ready to be written.
         * 
         * Returns a ByteBuffer encoded with the ConnectionReq message.
         */
        public abstract ByteBuffer EncodeConnectionReq(ByteBuffer byteBuffer);

        /* Encodes the RIPC ConnectionAck message and flip the buffer so that it is ready to be written.
         * 
         * buffer is the byteBuffer to encode into.
         * 
         * Returns ByteBuffer encoded with the ConnectionAck message.
         */
        public abstract ByteBuffer EncodeConnectionAck(ByteBuffer buffer, out Error error);

        /* Encodes the RIPC ConnectionNak message and flip the buffer so that it is ready to be written.
         * 
         * buffer is the ByteBuffer to encode into.
         * 
         * Returns a ByteBuffer encoded with the ConnectionNak message.
         */
        public abstract ByteBuffer EncodeConnectionNak(ByteBuffer buffer);

        /* Decodes the RIPC ConnectionReq message and populates IpcProtocolOptions with data read.
         * 
         * byteBuffer is the ByteBuffer to decode from.
         * msgLen is the length of message.
         * 
         * Returns TransportReturnCode.SUCCESS or TransportReturnCode.FAILURE
         */
        public abstract TransportReturnCode DecodeConnectionReq(ByteBuffer byteBuffer, int position, int msgLen, out Error error);

        /* Decodes a RIPC ConnectionReply (ConnectionAck or ConnectionNak) and
         * populates IpcProtocolOptions with data read.
         * After decoding the ConnectionReply, sets the buffer's position past the message read.
         * 
         * byteBuffer is the ByteBuffer to decode from
         * offset is the offset into the byteBuffer where the RIPC message starts. Will always be zero if not tunneling.
         *            
         * error is the Error to populate if an error occurs.
         * 
         * Returns TransportReturnCode.CHAN_INIT_REFUSED, TransportReturnCode.FAILURE, TransportReturnCode.SUCCESS.
         */
        public abstract TransportReturnCode DecodeConnectionReply(ByteBuffer byteBuffer, int offset, out Error error);

        /* Encodes the third leg of the ripc handshake which includes the client encryption key information.
         * This is only applicable to connection version 14 and higher and older versions should never get into here.
         * 
         * buffer is the ByteBuffer to encode into.
         * 
         * Returns a ByteBuffer encoded with the client key.
         */
        public abstract ByteBuffer EncodeClientKey(ByteBuffer buffer, out Error error);

        /* Decodes the third leg of the ripc handshake (conn version 14 or higher)
         * that contains the client key for encryption.
         * Should return error on older versions as they should never get into here.
         * 
         * byteBuffer is the ByteBuffer to decode from
         * offset is the offset into the byteBuffer where the RIPC message starts. Will always be zero if not tunneling.
         * error is the Error to populate if an error occurs.
         * 
         * Returns TransportReturnCode.CHAN_INIT_REFUSED, TransportReturnCode.FAILURE, TransportReturnCode.SUCCESS.
         */
        public abstract TransportReturnCode DecodeClientKey(ByteBuffer byteBuffer, int offset, out Error error);
    }
}
