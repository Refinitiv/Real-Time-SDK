/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using System;
using System.Linq;

using Xunit;
using Xunit.Abstractions;
using Xunit.Categories;
using LSEG.Eta.Common;
using LSEG.Eta.Transports.Internal;
using LSEG.Eta.Tests;

namespace LSEG.Eta.Transports.Tests
{
    [Category("ByteBuffer")]
    public class RipcBufferTests : IDisposable
    {
        const string _address = "127.0.0.1";
        const string _hostName = "localhost";
        const string _componentVersionString = "BOOTLESS";

        public RipcBufferTests(ITestOutputHelper output)
        {

        }

        public void Dispose()
        {
        }

        [Fact, Category("Unit")]
        public void ReplyStructPassedByReferenceOk()
        {
            RipcConnectionReply reply = default(RipcConnectionReply);

            internalMethod(ref reply);

            Assert.Equal(RipcFlags.DATA, reply.Flags);
        }

        void internalMethod(ref RipcConnectionReply replyRef)
        {
            Assert.IsType<RipcConnectionReply>(replyRef);

            Assert.True(replyRef.GetType().IsValueType);

            replyRef.Flags = RipcFlags.DATA;
        }

        #region RipcConnectionRequest

        [Fact, Category("Unit")]
        void RipcConnectionRequestWriteToBufferOk()
        {
            byte[] expected = {
                  0x00,                                      // [0] MessageLength[0]
                  (byte)(RipcConnectionRequest.FixedSizeOf   // [1] MessageLength[1]
                         + _address.Length                    //  
                         + _hostName.Length                   //  
                         + _componentVersionString.Length),   //  
                  (byte)RipcFlags.NONE,                       // [2]RipcFlags
                  0x00,                                       // [3]ConnectionVersion[0]
                  0x00,                                       // [4]ConnectionVersion[1]
                  0x00,                                       // [5]ConnectionVersion[2]
                  (byte)RipcVersionInfo.CurrentVersion,       // [6]ConnectionVersion[3]
                  (byte)default(Unused),                      // [7]Unused
                  (byte)(RipcConnectionRequest.HeaderFixedSizeOf // [8]HeaderLength 
                         + _address.Length                    //  
                         + _hostName.Length),                 //  
                  0x00,                                       // [9]RIPC Compression TypeBitmap Length
                  RipcVersionInfo.PingTimeout,                // [0]PingTimeout
                  (byte)RipcSessionFlags.NONE,                 // [1]SessionFlags
                  (byte)ProtocolType.RWF,                 // [2]ProtocolType
                  RipcVersionInfo.MajorVersion,               // [3]MajorVersion
                  RipcVersionInfo.MinorVersion,               // [4]MinorVersion
                  (byte)_hostName.Length,                     // [5]HostNameLength
                  (byte)_hostName[0],                         // [6]HostName
                  (byte)_hostName[1],                         // [7]
                  (byte)_hostName[2],                         // [8]
                  (byte)_hostName[3],                         // [9]
                  (byte)_hostName[4],                         // [0]
                  (byte)_hostName[5],                         // [1]
                  (byte)_hostName[6],                         // [2]
                  (byte)_hostName[7],                         // [3]
                  (byte)_hostName[8],                         // [4]
                  (byte)_address.Length,                      // [5]AddressLength
                  (byte)_address[0],                          // [6]IP Address
                  (byte)_address[1],                          // [7]
                  (byte)_address[2],                          // [8]
                  (byte)_address[3],                          // [9]
                  (byte)_address[4],                          // [0]
                  (byte)_address[5],                          // [1]
                  (byte)_address[6],                          // [2]
                  (byte)_address[7],                          // [3]
                  (byte)_address[8],                          // [4]
                  (byte)(_componentVersionString.Length + 1), // [5]ComponentVersionLength
                  (byte)(_componentVersionString.Length),     // [6]VersionStringLength
                  (byte)_componentVersionString[0],           // [7]ComponentVersionString
                  (byte)_componentVersionString[1],           // [8]
                  (byte)_componentVersionString[2],           // [9]
                  (byte)_componentVersionString[3],           // [0]
                  (byte)_componentVersionString[4],           // [1]
                  (byte)_componentVersionString[5],           // [2]
                  (byte)_componentVersionString[6],           // [3]
                  (byte)_componentVersionString[7]            // [4]
                                                                
         };

            RipcConnectionRequest request = new RipcConnectionRequest()
            {
                Flags = RipcFlags.NONE,
                ConnectionVersion = RipcVersionInfo.CurrentVersion,
                PingTimeout = (byte)RipcVersionInfo.PingTimeout,
                HostName = _hostName,
                IPAddress = _address,
                ProtocolType = ProtocolType.RWF,
                MajorVersion = RipcVersionInfo.MajorVersion,
                MinorVersion = RipcVersionInfo.MinorVersion,
                ComponentVersionString = _componentVersionString

            };

            ByteBuffer actual = new ByteBuffer(request.MessageLength);
            actual.Write(request);
            Assert.Equal(expected, actual.Contents);
        }

        [Fact, Category("Unit")]
        public void RipcConnectionRequestGetWithEmptyStringsOk()
        {
            RipcConnectionRequest expected = default(RipcConnectionRequest);
            ByteBuffer buffer = new ByteBuffer(RipcConnectionRequest.FixedSizeOf);
            buffer.Write(expected);
            buffer.Flip();

            RipcConnectionRequest actual = default(RipcConnectionRequest);
            Assert.True(buffer.Read(ref actual));
        }

        [Fact, Category("Unit")]
        public void RipcConnectionRequestComponentVersionStringTooLong()
        {
            var request = default(RipcConnectionRequest);

            int size = RipcVersionInfo.ComponentVersionStringLenMax + 1;
            request.ComponentVersionString = $"{new string('X', size)}";
            Assert.Equal(RipcVersionInfo.ComponentVersionStringLenMax,
                         request.VersionStringLength);
        }

        [Fact, Category("Unit")]
        public void RipcConnectionRequestGetOk()
        {
            RipcConnectionRequest expected = new RipcConnectionRequest
            {
                Flags = RipcFlags.NONE,
                ConnectionVersion = RipcVersionInfo.CurrentVersion,
                PingTimeout = (byte)RipcVersionInfo.PingTimeout,
                HostName = _hostName,
                IPAddress = _address,
                ProtocolType = ProtocolType.RWF,
                MajorVersion = RipcVersionInfo.MajorVersion,
                MinorVersion = RipcVersionInfo.MinorVersion,
                ComponentVersionString = _componentVersionString
            };

            ByteBuffer buffer = new ByteBuffer(expected.MessageLength);
            buffer.Write(expected);
            buffer.Flip();

            RipcConnectionRequest actual = default(RipcConnectionRequest);
            bool result = buffer.Read(ref actual);
            Assert.True(result, $"Amount of deserialized buffer ({buffer.Position}) <> MessageLength ({actual.MessageLength})");

            Assert.Equal<RipcConnectionRequest>(expected, actual);
        }

        #endregion

        #region RipcConnectionReply(Ack,Nak)

        [Fact, Category("Unit")]
        public void RipcConnectionReplyNakWriteToBufferOk()
        {
            const short TestMessageLength = 25;
            const string TestText = "BOOTLESS REQUEST";

            byte[] expected = { 0x00,
                             (byte)TestMessageLength,
                             (byte)RipcFlags.HAS_OPTIONAL_FLAGS,
                             (byte)RipcOpCode.CONNECT_NAK,
                             (byte)TestMessageLength,
                             (byte)default(Unused),
                             0x00,
                             16,
                             (byte)'B', (byte)'O', (byte)'O', (byte)'T',
                             (byte)'L', (byte)'E', (byte)'S', (byte)'S',
                             (byte)' ', (byte)'R', (byte)'E', (byte)'Q',
                             (byte)'U', (byte)'E', (byte)'S', (byte)'T',
                             0x00
         };

            RipcConnectionReplyNak reply = new RipcConnectionReplyNak
            {          // 1234567890ABCDEF
                Text = TestText,
                RipcReply = new RipcConnectionReply
                {
                    Flags = RipcFlags.HAS_OPTIONAL_FLAGS,
                    OpCode = RipcOpCode.CONNECT_NAK,
                }
            };

            ByteBuffer buffer = new ByteBuffer(reply.MessageLength);

            buffer.Write(reply);

            byte[] actual = buffer.Contents;

            Assert.Equal(expected, actual);
        }

        [Fact, Category("Unit")]
        public void RipcConnectionReplyNakGetOk()
        {
            const short TestMessageLength = 25;
            const string TestText = "BOOTLESS REQUEST";

            RipcConnectionReplyNak expected = new RipcConnectionReplyNak
            {
                RipcReply = new RipcConnectionReply
                {
                    MessageLength = 0,
                    Flags = RipcFlags.HAS_OPTIONAL_FLAGS,
                    OpCode = RipcOpCode.CONNECT_NAK,
                    HeaderLength = (byte)0xFF
                },
                Text = TestText
            };

            ByteBuffer byteBuffer = new ByteBuffer(TestMessageLength);
            byteBuffer.Write(expected);
            byteBuffer.Flip();

            RipcConnectionReplyNak actual = default(RipcConnectionReplyNak);
            Assert.True(byteBuffer.Read(ref actual));

            Assert.Equal<RipcConnectionReplyNak>(expected, actual);

        }


        [Fact, Category("Unit")]
        public void RipcConnectionReplyAckWriteToBufferOk()
        {
            const short TestMessageLength = 29;
            const string TestText = "BOOTLESS";
            const short MaxUserMsgSize = 8192;

            byte[] expected = { 0x00,                                    // [00] MessageLength LOB
                             (byte)TestMessageLength,                    // [01] MessageLength HOB
                             (byte)RipcFlags.HAS_OPTIONAL_FLAGS,         // [02] Flags
                             (byte)RipcOpCode.CONNECT_ACK,               // [03] OpCode
                             (byte)RipcConnectionReplyAck.HeaderLength,  // [04] HeaderLength
                             (byte)default(Unused),                      // [05] Unused
                             0x00,                                       // [06] RIPCVersion[0]
                             0x00,                                       // [07] RIPCVersion[1]
                             0x00,                                       // [08] RIPCVersion[2]
                             (byte)RipcVersionInfo.CurrentVersion,       // [09] RIPCVersion[3]
                             (byte)(MaxUserMsgSize >> 8),                // [10] MaxUserMsgSize[0]
                             0x00,                                       // [11] MaxUserMsgSize[1]
                             (byte)RipcSessionFlags.NONE,                 // [12] SessionFlags
                             0xFF,                                       // [13] PingTimeOut = 255
                             0x0E,                                       // [14] MajorVersion
                             0x00,                                       // [15] MinorVersion
                             0x7F,                                       // [16] CompType[0]
                             0xFF,                                       // [17] CompType[1]
                             0x00,                                       // [18] CompLevel
                             (byte)9,                                    // [19] ComponentVersionLength
                             (byte)TestText.Length,                      // [20] VersionStringLength
                             (byte)'B', (byte)'O', (byte)'O', (byte)'T', // [21] ComponentVersionString[0..3]
                             (byte)'L', (byte)'E', (byte)'S', (byte)'S'  // [25] ComponentVersionString[4..7]
         };

            RipcConnectionReplyAck reply = new RipcConnectionReplyAck
            {          // 1234567890ABCDEF
                RipcReply = new RipcConnectionReply
                {
                    Flags = RipcFlags.HAS_OPTIONAL_FLAGS,
                    OpCode = RipcOpCode.CONNECT_ACK,
                },
                RipcVersion = RipcVersionInfo.CurrentVersion,
                MaxUserMsgSize = RipcVersionInfo.MaxUserMsgSize,
                SessionFlags = (byte)RipcSessionFlags.NONE,
                PingTimeout = RipcVersionInfo.PingTimeout,
                MajorVersion = RipcVersionInfo.MajorVersion,
                MinorVersion = RipcVersionInfo.MinorVersion,
                ComponentType = short.MaxValue,
                ComponentLevel = 0x00,

                ComponentVersionString = TestText
            };

            ByteBuffer buffer = new ByteBuffer(reply.MessageLength);

            buffer.Write(reply);

            byte[] actual = buffer.Contents;
            Assert.Equal(expected, actual);
        }

        [Fact, Category("Unit")]
        public void RipcConnectionReplyAckGetOk()
        {
            const short TestMessageLength = 29;
            const string TestText = "BOOTLESS";

            RipcConnectionReplyAck expected = new RipcConnectionReplyAck
            {
                RipcReply = new RipcConnectionReply
                {
                    Flags = RipcFlags.HAS_OPTIONAL_FLAGS,
                    OpCode = RipcOpCode.CONNECT_ACK,
                },
                RipcVersion = RipcVersionInfo.CurrentVersion,
                MaxUserMsgSize = RipcVersionInfo.MaxUserMsgSize,
                SessionFlags = (byte)RipcSessionFlags.NONE,
                PingTimeout = RipcVersionInfo.PingTimeout,
                MajorVersion = RipcVersionInfo.MajorVersion,
                MinorVersion = RipcVersionInfo.MinorVersion,
                ComponentType = short.MaxValue,
                ComponentLevel = 0x00,

                ComponentVersionString = TestText
            };

            ByteBuffer buffer = new ByteBuffer(TestMessageLength);
            buffer.Write(expected);
            buffer.Flip();

            RipcConnectionReplyAck actual = default(RipcConnectionReplyAck);
            Assert.True(buffer.Read(ref actual));

            Assert.Equal<RipcConnectionReplyAck>(expected, actual);

        }

        #endregion

        #region Load Tests

        #region Support Methods

        private static byte[] CreatePayload(int width, ushort? offset = null, byte filler = 0x00)
        {
            offset = offset ?? RipcDataMessage.HeaderSize;
            byte[] payload = new byte[width - offset.Value];
            for (int i = 0; i < width - offset; i++)
            {
                payload[i] = (byte)((i == 0)
                                    ? 0xCC
                                    : (i == (width - 1 - offset)
                                        ? 0xFF
                                        : (byte)(i + offset) | filler));
            }
            return payload;
        }


        private static byte[] CreatePackedPayload(short messageWidth, int messageCount)
        {
            byte[] packedPayload = new byte[(ushort)((messageWidth + sizeof(ushort)) * messageCount)];

            for (int i = 0; i < messageCount; i++)
            {
                byte[] payload = CreatePayload(messageWidth, 0, (byte)(i << 4));
                Buffer.BlockCopy(payload, 0, packedPayload, i * (messageWidth + sizeof(ushort)) + sizeof(ushort), messageWidth);
                byte[] messageLength = BitConverter.GetBytes(System.Net.IPAddress.HostToNetworkOrder(messageWidth));
                Buffer.BlockCopy(messageLength, 0, packedPayload, i * (messageWidth + sizeof(ushort)), sizeof(ushort));
            }
            return packedPayload;
        }

        private static void CreateRipcMessage(int width, out RipcDataMessage message, RipcFlags flags = RipcFlags.DATA)
        {
            // Create a RipcMessage with a known payload.
            byte[] payload = CreatePayload(width);

            message = new RipcDataMessage(payload);
            message.Flags = flags;
        }

        private static void CreatePackedRipcMessage(short messageWidth, int messageCount, out RipcDataMessage message)
        {
            // Create a RipcMessage with a known payload.
            byte[] packedPayload = CreatePackedPayload(messageWidth, messageCount);

            message = new RipcDataMessage(packedPayload, RipcFlags.DATA | RipcFlags.PACKING);
        }

        private static ByteBuffer FillByteBufferWithMessage(int width, out RipcDataMessage message)
        {
            ByteBuffer buffer = new ByteBuffer(width);

            message = default(RipcDataMessage);
            CreateRipcMessage(width, out message);

            buffer.Write(message);

            return buffer;
        }

        private static ByteBuffer CreateByteBufferWithPackedMessages(short messageWidth, int messageCount, out RipcDataMessage packedMessage)
        {
            CreatePackedRipcMessage(messageWidth, messageCount, out packedMessage);

            // ByteBuffer now contains the one Packed Message as it's payload.
            ByteBuffer buffer = new ByteBuffer(packedMessage.MessageLength);
            buffer.Write(packedMessage);

            return buffer;
        }

        #endregion

        [Fact]
        [Category("Unit")]
        [Category("RipcMessage")]
        public void WritesRipcNormalDataMessageToBufferOk()
        {
            const int width = 32;
            RipcDataMessage message;
            ByteBuffer buffer = FillByteBufferWithMessage(width, out message);

            Assert.Equal(message.Payload, buffer.Contents.Skip(RipcDataMessage.HeaderSize));
        }


        [Fact]
        [Category("Unit")]
        [Category("RipcMessage")]
        public void ReadRipcNormalDataMessageFromBufferOk()
        {
            const int width = 19;

            ByteBuffer buffer = FillByteBufferWithMessage(width, out RipcDataMessage message);

            message = new RipcDataMessage();

            buffer.Read(ref message, true);

            Assert.Equal(buffer.ReadShortAt(0), (short)message.MessageLength);
            Assert.Equal(buffer.ReadByteAt(sizeof(short)), (byte)message.Flags);
            Assert.Equal(buffer.Contents.Skip(RipcDataMessage.HeaderSize), message.Payload);
        }

        [Fact]
        [Category("Unit")]
        [Category("RipcMessage")]
        public void ReadOneRipcPackedDataMessageFromBufferOk()
        {
            const int messageWidth = 16;
            const int messageCount = 1;

            ByteBuffer buffer = CreateByteBufferWithPackedMessages(messageWidth, messageCount, out RipcDataMessage packedMessage);

            Assert.Equal(buffer.ReadShortAt(0), (short)packedMessage.MessageLength);
            Assert.Equal(buffer.ReadByteAt(sizeof(short)), (byte)packedMessage.Flags);
            Assert.Equal(buffer.Contents.Skip(RipcDataMessage.HeaderSize), packedMessage.Payload);
        }


        [Fact]
        [Category("Unit")]
        [Category("RipcMessage")]
        public void ReadThreeMessagesRipcPackedDataFromBufferOk()
        {
            const int messageWidth = 16;
            const int messageCount = 3;

            ByteBuffer buffer = CreateByteBufferWithPackedMessages(messageWidth, messageCount, out RipcDataMessage packedMessage);

            Assert.Equal(buffer.ReadShortAt(0), (short)packedMessage.MessageLength);
            Assert.Equal(buffer.ReadByteAt(sizeof(short)), (byte)packedMessage.Flags);
            Assert.Equal(buffer.Contents.Skip(RipcDataMessage.HeaderSize), packedMessage.Payload);
        }

        [Fact]
        [Category("Unit")]
        [Category("RipcMessage")]
        public void LoadTransportBufferWithOneRipcPackedDataMessageFromBufferOk()
        {
            const int messageWidth = 16;
            const int messageCount = 1;

            ByteBuffer buffer = CreateByteBufferWithPackedMessages(messageWidth, messageCount, out RipcDataMessage packedMessage);

            // First sub-message loaded.
            TransportBuffer transportBuffer = TransportBuffer.Load(ref buffer);

            byte[] expectedPayload = CreatePayload(messageWidth, 0);
            Assert.True(transportBuffer.GetIsRipcMessage());
            Assert.Equal(messageWidth, transportBuffer.Length());
            Assert.Equal(expectedPayload, transportBuffer.Data.Contents.Skip(RipcDataMessage.HeaderSize));

            // No more messages to unload.
            transportBuffer = TransportBuffer.Load(ref buffer);
            Assert.Null(transportBuffer);
        }

        [Fact]
        [Category("Unit")]
        [Category("RipcMessage")]
        public void LoadTransportBufferWithSeveralRipcPackedDataMessagesFromBufferOk()
        {
            const int messageWidth = 11;
            const int messageCount = 5;

            TransportBuffer transportBuffer;

            ByteBuffer buffer = CreateByteBufferWithPackedMessages(messageWidth, messageCount, out RipcDataMessage packedMessage);

            for (int i = 0; i < messageCount; i++)
            {
                byte[] expectedPayload = CreatePayload(messageWidth, 0, (byte)(i << 4));
                // First sub-message loaded.
                transportBuffer = TransportBuffer.Load(ref buffer);

                Assert.True(transportBuffer.GetIsRipcMessage());
                Assert.Equal(messageWidth, transportBuffer.Length());
                Assert.Equal(expectedPayload, transportBuffer.Data.Contents.Skip(RipcDataMessage.HeaderSize));
            }
            // No more messages to unload.
            transportBuffer = TransportBuffer.Load(ref buffer);
            Assert.Null(transportBuffer);
        }

        [Fact]
        [Category("Unit")]
        [Category("RipcMessage")]
        public void LoadTransportBufferWithSeveralRipcDataMessagesFromBufferOk()
        {
            const int messageWidth = 11;
            const int messageCount = 5;

            int bufferCapacity = (messageWidth + RipcDataMessage.HeaderSize) * messageCount;
            ByteBuffer expectedBuffer = FillByteBufferWithMessage(messageWidth, out RipcDataMessage message);

            ByteBuffer ioBuffer = new ByteBuffer(bufferCapacity);

            for(int i = 0; i < messageCount; i++)
            {
                ioBuffer.Put(expectedBuffer);
            }

            Assert.Equal(bufferCapacity, ioBuffer.Capacity);

            int j = 0;
            TransportBuffer transportBuffer;
            while((transportBuffer = TransportBuffer.Load(ref ioBuffer)) != null)
            {
                Assert.Equal(expectedBuffer, transportBuffer.Data);
                j++;
            }

            Assert.Equal(messageCount, j);
        }


        [Fact]
        [Category("Unit")]
        [Category("RipcMessage")]
        public void LoadTransportBufferWithSeveralRipcPackedDataSeveralMessagesFromBufferOk()
        {
            // Width of payload portion.
            const int messageWidth = 9;

            // Number of messages per packet
            const int messageCount = 3;

            // Number of sub-messages per Packed Message
            const int subMessageCount = 2;

            int bufferCapacity = (((messageWidth + sizeof(ushort)) * subMessageCount ) + RipcDataMessage.HeaderSize) 
                                        * messageCount;


            ByteBuffer ioBuffer = new ByteBuffer(bufferCapacity);

            for (int i = 0; i < messageCount; i++)
            {
                ByteBuffer packedBuffer = CreateByteBufferWithPackedMessages(messageWidth, subMessageCount, out RipcDataMessage packedMessage);
                ioBuffer.Put(packedBuffer);
            }

            Assert.Equal(bufferCapacity, ioBuffer.Capacity);

            int j = 0;
            TransportBuffer transportBuffer;
            while ((transportBuffer = TransportBuffer.Load(ref ioBuffer)) != null)
            {
                j++;
            }

            Assert.Equal(messageCount * subMessageCount + messageCount - 1, j);
        }

        #endregion
    }
}
