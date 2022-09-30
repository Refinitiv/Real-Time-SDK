/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.            --
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Text;
using Refinitiv.Eta.Common;

namespace Refinitiv.Eta.Transports.Internal
{
    internal static class RipcExtensions
    {
        private static ByteBuffer WriteString(this ByteBuffer buffer, string text)
        {
            byte length = (byte)(text ?? string.Empty).Length;

            byte[] data = ASCIIEncoding.ASCII.GetBytes(text ?? string.Empty);

            buffer.Write((byte)length);
            buffer.Put(data);

            return buffer;
        }

        private static ByteBuffer WriteBytes(this ByteBuffer buffer, byte[] data)
        {
            byte length = (byte)(data ?? Array.Empty<byte>()).Length;
            buffer.Write(length);
            if (length > 0)
                buffer.Put(data);
            return buffer;
        }

        private static ByteBuffer WriteCString(this ByteBuffer buffer, string text)
        {
            short length = (short)(text ?? string.Empty).Length;

            byte[] data = ASCIIEncoding.ASCII.GetBytes(text ?? string.Empty);

            buffer.Write(length);
            buffer.Put(data);
            buffer.Write((byte)0x00);

            return buffer;
        }

        private static string ReadString(this ByteBuffer buffer)
        {
            int length = buffer.ReadByte();
            byte[] data = new byte[length];
            buffer.ReadBytesInto(data, 0, length);
            return ASCIIEncoding.ASCII.GetString(data, 0, length);
        }


        private static string ReadBigString(this ByteBuffer buffer)
        {
            int length = buffer.ReadShort();
            byte[] data = new byte[length];
            buffer.ReadBytesInto(data, 0, length);
            return ASCIIEncoding.ASCII.GetString(data, 0, length);
        }

        private static byte[] ReadBytes(this ByteBuffer buffer)
        {
            int length = buffer.ReadByte();
            byte[] data = null;
            if (length > 0)
                buffer.ReadBytesInto(data, 0, length);
            return data;
        }

        #region Ripc Connect{Reply,Request}

        public static ByteBuffer Write(this ByteBuffer buffer, RipcConnectionRequest request)
        {
            buffer.Write(request.MessageLength)
                  .Write((byte)request.Flags)
                  .Write(request.ConnectionVersion)
                  .Write((byte)request.FlagsEx)
                  .Write(request.HeaderLength)
                  .WriteBytes(request.RipcCompressionMap)
                  .Write(request.PingTimeout)
                  .Write((byte)request.SessionFlags)
                  .Write((byte)request.ProtocolType)
                  .Write(request.MajorVersion)
                  .Write(request.MinorVersion)
                  .WriteString(request.HostName)
                  .WriteString(request.IPAddress)
                  .Write(request.ComponentVersionLength)
                  .WriteString(request.ComponentVersionString)
                  ;

            return buffer;
        }

        public static ByteBuffer Put(this RipcConnectionRequest request, ByteBuffer buffer)
        {           
            return buffer.Write(request);
        }

        private static ByteBuffer Write(this ByteBuffer buffer, RipcConnectionReply reply)
        {
            long position = buffer.Position;

            buffer.Write((short)reply.MessageLength)
               .Write((byte)reply.Flags)
               .Write((byte)reply.OpCode)
               .Write((byte)reply.HeaderLength)
               .Write((byte)reply.Unused)
               ;

            if (buffer.Position - position > reply.MessageLength)
                throw new InvalidOperationException($"RipcConnectionReply.Put - Buffer overrun.");

            return buffer;
        }

        private static ByteBuffer Put(this RipcConnectionReply reply, ByteBuffer buffer)
        {
            return buffer.Write(reply);
        }

        public static ByteBuffer Write(this ByteBuffer buffer, RipcConnectionReplyNak reply)
        {
            long position = buffer.Position;

            reply.RipcReply.MessageLength = reply.MessageLength;
            reply.RipcReply.HeaderLength = reply.HeaderLength;
            reply.RipcReply.Put(buffer)
               .WriteCString(reply.Text)
                ;

            if (buffer.Position - position > reply.MessageLength)
                throw new InvalidOperationException($"RipcConnectionReplyNak.Put - Buffer overrun.");

            return buffer;
        }

        public static ByteBuffer Put(this RipcConnectionReplyNak reply, ByteBuffer buffer)
        {
            return buffer.Write(reply);
        }

        public static ByteBuffer Write(this ByteBuffer buffer, RipcConnectionReplyAck reply)
        {
            // Poor man's v-table!
            reply.RipcReply.Put(buffer)
               .Write(reply.RipcVersion)
               .Write(reply.MaxUserMsgSize)
               .Write(reply.SessionFlags)
               .Write(reply.PingTimeout)
               .Write(reply.MajorVersion)
               .Write(reply.MinorVersion)
               .Write((short)reply.ComponentType)
               .Write(reply.ComponentLevel)
               .Write(reply.ComponentVersionLength)
               .WriteString(reply.ComponentVersionString)
               ;
            return buffer;
        }

        public static ByteBuffer Put(this RipcConnectionReplyAck reply, ByteBuffer buffer)
        {
            return buffer.Write(reply);
        }


        public static ByteBuffer Write(this ByteBuffer buffer, RipcDataMessage message)
        {
            buffer.Write((short)message.MessageLength)
               .Write((byte)message.Flags)
               .Put(message.Payload)
               ;

            return buffer;
        }


        public static bool Read(this ByteBuffer buffer, ref RipcConnectionReply replyRef)
        {
            if (buffer.Capacity < RipcConnectionReply.FixedSizeOf)
            {   // Not enuff data
                return false;
            }

            replyRef.MessageLength = (ushort)buffer.ReadShort();
            replyRef.Flags = (RipcFlags)buffer.ReadByte();

            if (replyRef.Flags != RipcFlags.HAS_OPTIONAL_FLAGS)
            {
                // Not a Connection request
                throw new InvalidOperationException("Incorrect layout for RipcConnectionReply");
            }

            replyRef.OpCode = (RipcOpCode)buffer.ReadByte();

            if (replyRef.OpCode != RipcOpCode.CONNECT_ACK && replyRef.OpCode != RipcOpCode.CONNECT_NAK)
            {
                throw new InvalidOperationException($"Incorrect layout for RipcConnectionReply; OpCode ({replyRef.OpCode}) Not {{ACK, NAK}}");
            }

            replyRef.HeaderLength = (byte)buffer.ReadByte();
            if (replyRef.HeaderLength > replyRef.MessageLength)
            {
                throw new InvalidOperationException($"Incorrect layout for RipcConnectionReply; HeaderLength ({replyRef.HeaderLength} > MessageLength ({replyRef.MessageLength}))");
            }

            byte unused = (byte)buffer.ReadByte();
            return true;
        }

        public static bool Read(this ByteBuffer buffer, ref RipcConnectionReplyAck replyRef)
        {
            long position = buffer.Position;

            if (!buffer.Read(ref replyRef.RipcReply))
            {
                return false;
            }

            if (replyRef.RipcReply.OpCode != RipcOpCode.CONNECT_ACK)
            {
                throw new InvalidOperationException($"Incorrect layout for {nameof(RipcConnectionReplyAck)}: OpCode ({replyRef.RipcReply.OpCode}) not ({RipcOpCode.CONNECT_ACK})");
            }

            replyRef.RipcVersion = buffer.ReadInt();
            replyRef.MaxUserMsgSize = buffer.ReadShort();
            replyRef.SessionFlags = (byte)buffer.ReadByte();
            replyRef.PingTimeout = (byte)buffer.ReadByte();
            replyRef.MajorVersion = (byte)buffer.ReadByte();
            replyRef.MinorVersion = (byte)buffer.ReadByte();
            replyRef.ComponentType = buffer.ReadShort();
            replyRef.ComponentLevel = (byte)buffer.ReadByte();

            byte b = (byte)buffer.ReadByte();

            int textLength = buffer.ReadByte();
            byte[] text = new byte[textLength];
            buffer.ReadBytesInto(text, 0, textLength);
            replyRef.ComponentVersionString = ASCIIEncoding.ASCII.GetString(text, 0, textLength);

            return replyRef.MessageLength == (buffer.Position - position);
        }

        public static bool Read(this ByteBuffer buffer, ref RipcConnectionReplyNak replyRef)
        {
            long position = buffer.Position;

            if (!buffer.Read(ref replyRef.RipcReply))
            {
                return false;
            }

            if (replyRef.RipcReply.OpCode != RipcOpCode.CONNECT_NAK)
            {
                throw new InvalidOperationException($"Incorrect layout for {nameof(RipcConnectionReplyNak)}: OpCode ({replyRef.RipcReply.OpCode}) not ({RipcOpCode.CONNECT_NAK})");
            }

            replyRef.Text = buffer.ReadBigString();
            byte useless = (byte)buffer.ReadByte();


            return replyRef.MessageLength == (buffer.Position - position);
        }

        /// <summary>
        /// Unmarshall from a byte[] held by a ByteBuffer.
        /// </summary>
        /// <returns>false : Not enough data yet.</returns>
        public static bool Read(this ByteBuffer buffer, ref RipcConnectionRequest requestRef)
        {
            long position = buffer.Position;

            short messageLength = buffer.ReadShort();

            requestRef.Flags = (RipcFlags)buffer.ReadByte();

            if (requestRef.Flags != RipcFlags.NONE)
            {  // Not a Connection request
                throw new InvalidOperationException("Incorrect layout for RipcConnectionRequest");
            }

            requestRef.ConnectionVersion = buffer.ReadInt();

            // Advance past non-settable field: Unused, HeaderLength
            byte unused = (byte)buffer.ReadByte();
            unused = (byte)buffer.ReadByte();

            requestRef.RipcCompressionMap = buffer.ReadBytes();

            requestRef.PingTimeout = (byte)buffer.ReadByte();
            requestRef.SessionFlags = (RipcSessionFlags)buffer.ReadByte();
            requestRef.ProtocolType = (ProtocolType)buffer.ReadByte();
            requestRef.MajorVersion = (byte)buffer.ReadByte();
            requestRef.MinorVersion = (byte)buffer.ReadByte();
            requestRef.HostName = buffer.ReadString();
            requestRef.IPAddress = buffer.ReadString();
            unused = (byte)buffer.ReadByte();
            requestRef.ComponentVersionString = buffer.ReadString();

            return requestRef.MessageLength == (buffer.Position - position);
        }
        #endregion
    }
}
