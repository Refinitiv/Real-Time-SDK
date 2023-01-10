/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022-2023 Refinitiv. All rights reserved.              --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Internal.Interfaces;
using LSEG.Eta.Tests;
using LSEG.Eta.Transports.Internal;
using System.Text;

using Xunit;
using Xunit.Categories;
using LSEG.Eta.Internal;

namespace LSEG.Eta.Transports.Tests
{
    [Collection("Transport")]
    [Category("Unit")]
    [Category("ReadWriteTest")]
    public class SocketReadWriteTests
    {
        const string RWF_MSG_1 = "123456RWFEncodedBufferxxxfweirfsdfkjl";

        const string RWF_MSG_2 = "999999999999988888888888888888998eqwewqerqwer9qwtqfafjakefjaskerjfra;9345353453433243324-fasdfj0000000NULL";

        const string RWF_MSG_3 = "tw49etesgjga9ithopksdgpise9tw45jopdgk";

        const string RWF_MSG_LONG_LENGTH_1 = $"{RWF_MSG_1}{RWF_MSG_2 }{RWF_MSG_1}{RWF_MSG_2 }{RWF_MSG_1}{RWF_MSG_2}";
        const string RWF_MSG_LONG_LENGTH_2 = $"{RWF_MSG_2}{RWF_MSG_2 }{RWF_MSG_2}{RWF_MSG_2 }{RWF_MSG_1}{RWF_MSG_1}";
        const string RWF_MSG_LONG_LENGTH_3 = $"{RWF_MSG_LONG_LENGTH_1}{RWF_MSG_LONG_LENGTH_2}";

        const int LARGE_FRAGMENT_SIZE = 310; /* This is larger than the default low compression threshold for ZLIB and LZ4 compressions. */

        /* 1 fastest, 6 optimal, 9 best, 0 no compression */
        readonly int[] ZLIB_COMPRESSION_LEVELS = new int[] { 1, 6, 9, 0};

        [Fact]
        public void WriteReadOneRIPCMessageTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();
            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);
            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, ChannelBase.DEFAULT_HIGH_WATER_MARK, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER,
                ChannelBase.DEFAULT_MAX_FRAGMENT_SIZE);

            ITransportBuffer messageBuffer = channel.GetBuffer(150, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes(RWF_MSG_1);

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs()
            {
                Flags = WriteFlags.DIRECT_SOCKET_WRITE
            }, out error);

            Assert.Equal(TransportReturnCode.SUCCESS, ret);
            Assert.Null(error);

            ReadArgs readArgs = new ReadArgs();

            ITransportBuffer recevBuf = channel.Read(readArgs, out error);

            Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
            Assert.NotNull(recevBuf);
            Assert.Null(error);

            Assert.True(TestUtilities.CompareByteArray(dataByteArray, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));
            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void WriteReadTwoRIPCMessagesTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();
            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, ChannelBase.DEFAULT_HIGH_WATER_MARK, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER,
                ChannelBase.DEFAULT_MAX_FRAGMENT_SIZE);

            for (int i = 0; i < 5; i++)
            {
                ITransportBuffer messageBuffer = channel.GetBuffer(150, false, out error);

                Assert.NotNull(messageBuffer);
                Assert.Null(error);

                ITransportBuffer messageBuffer2 = channel.GetBuffer(200, false, out error);
                Assert.NotNull(messageBuffer2);
                Assert.Null(error);

                byte[] dataByteArray1 = Encoding.ASCII.GetBytes(RWF_MSG_1);

                messageBuffer.Data.Put(dataByteArray1);

                TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs()
                {
                    Flags = WriteFlags.DIRECT_SOCKET_WRITE
                }, out error);

                Assert.Equal(TransportReturnCode.SUCCESS, ret);
                Assert.Null(error);

                byte[] dataByteArray2 = Encoding.ASCII.GetBytes(RWF_MSG_2);

                messageBuffer2.Data.Put(dataByteArray2);

                ret = channel.Write(messageBuffer2, new WriteArgs()
                {
                    Flags = WriteFlags.DIRECT_SOCKET_WRITE
                }, out error);

                Assert.Equal(TransportReturnCode.SUCCESS, ret);
                Assert.Null(error);

                ReadArgs readArgs = new ReadArgs();

                ITransportBuffer recevBuf = channel.Read(readArgs, out error);

                // There is more data to read as the read return value more than TransportReturnCode.SUCCESS
                Assert.Equal(RWF_MSG_2.Length + RipcDataMessage.HeaderSize, (int)readArgs.ReadRetVal);
                Assert.NotNull(recevBuf);
                Assert.Null(error);

                Assert.True(TestUtilities.CompareByteArray(dataByteArray1, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));

                ITransportBuffer recevBuf2 = channel.Read(readArgs, out error);
                Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
                Assert.NotNull(recevBuf2);
                Assert.Null(error);

                Assert.True(TestUtilities.CompareByteArray(dataByteArray2, 0, recevBuf2.Data.Contents, recevBuf2.GetDataStartPosition(), recevBuf2.Length()));

                mockChannel.Clear();
            }

            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void WriteAndReadAFragmentedMessageForRIPC13Test()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();
            int maxFragmentSize = 30;

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, ChannelBase.DEFAULT_HIGH_WATER_MARK, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER,
                maxFragmentSize, RipcVersions.VERSION13, 10); // Sets the max fragmentation size to 30 bytes

            ITransportBuffer messageBuffer = channel.GetBuffer(150, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes(RWF_MSG_2);

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs()
            {
                Flags = WriteFlags.DIRECT_SOCKET_WRITE
            }, out error);

            Assert.Equal(TransportReturnCode.SUCCESS, ret);
            Assert.Null(error);
            ByteBuffer networkBuffer = mockChannel.GetNetworkBuffer();
            int remainingBufLength = networkBuffer.WritePosition;
            int InternalMaxFragmentSize = maxFragmentSize + RipcDataMessage.HeaderSize;

            ReadArgs readArgs = new ReadArgs();

            ITransportBuffer recevBuf = channel.Read(readArgs, out error);
            int readRet = (int)readArgs.ReadRetVal;
            remainingBufLength -= InternalMaxFragmentSize;
            Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS && readRet == remainingBufLength);
            Assert.Equal(networkBuffer.WritePosition, readArgs.BytesRead);
            Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);
            Assert.Null(recevBuf);
            Assert.Null(error);

            recevBuf = channel.Read(readArgs, out error);
            readRet = (int)readArgs.ReadRetVal;
            remainingBufLength -= InternalMaxFragmentSize;
            Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS && readRet == remainingBufLength);
            Assert.Equal(0, readArgs.BytesRead);
            Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);
            Assert.Null(recevBuf);
            Assert.Null(error);

            recevBuf = channel.Read(readArgs, out error);
            readRet = (int)readArgs.ReadRetVal;
            remainingBufLength -= InternalMaxFragmentSize;
            Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS && readRet == remainingBufLength);
            Assert.Equal(0, readArgs.BytesRead);
            Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);
            Assert.Null(recevBuf);
            Assert.Null(error);

            recevBuf = channel.Read(readArgs, out error);
            readRet = (int)readArgs.ReadRetVal;
            remainingBufLength -= InternalMaxFragmentSize;
            Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS && readRet == remainingBufLength);
            Assert.Equal(0, readArgs.BytesRead);
            Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);
            Assert.Null(recevBuf);
            Assert.Null(error);

            recevBuf = channel.Read(readArgs, out error);
            Assert.True(readArgs.ReadRetVal == TransportReturnCode.SUCCESS);
            Assert.Equal(0, readArgs.BytesRead);
            Assert.Equal(8, readArgs.UncompressedBytesRead);
            Assert.NotNull(recevBuf);
            Assert.Null(error);

            Assert.True(TestUtilities.CompareByteArray(dataByteArray, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));

            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));
        }

        [Fact]
        public void WriteAndReadAFragmentedMessageForRIPC12Test()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();
            int maxFragmentSize = 30;

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, ChannelBase.DEFAULT_HIGH_WATER_MARK, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER,
                maxFragmentSize, RipcVersions.VERSION12, 10); // Sets the max fragmentation size to 30 bytes

            ITransportBuffer messageBuffer = channel.GetBuffer(150, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes(RWF_MSG_2);

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs()
            {
                Flags = WriteFlags.DIRECT_SOCKET_WRITE
            }, out error);

            Assert.Equal(TransportReturnCode.SUCCESS, ret);
            Assert.Null(error);
            ByteBuffer networkBuffer = mockChannel.GetNetworkBuffer();
            int remainingBufLength = networkBuffer.WritePosition;
            int InternalMaxFragmentSize = maxFragmentSize + RipcDataMessage.HeaderSize;

            ReadArgs readArgs = new ReadArgs();

            ITransportBuffer recevBuf = channel.Read(readArgs, out error);
            int readRet = (int)readArgs.ReadRetVal;
            remainingBufLength -= InternalMaxFragmentSize;
            Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS && readRet == remainingBufLength);
            Assert.Equal(networkBuffer.WritePosition, readArgs.BytesRead);
            Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);
            Assert.Null(recevBuf);
            Assert.Null(error);

            recevBuf = channel.Read(readArgs, out error);
            readRet = (int)readArgs.ReadRetVal;
            remainingBufLength -= InternalMaxFragmentSize;
            Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS && readRet == remainingBufLength);
            Assert.Equal(0, readArgs.BytesRead);
            Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);
            Assert.Null(recevBuf);
            Assert.Null(error);

            recevBuf = channel.Read(readArgs, out error);
            readRet = (int)readArgs.ReadRetVal;
            remainingBufLength -= InternalMaxFragmentSize;
            Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS && readRet == remainingBufLength);
            Assert.Equal(0, readArgs.BytesRead);
            Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);
            Assert.Null(recevBuf);
            Assert.Null(error);

            recevBuf = channel.Read(readArgs, out error);
            Assert.True(readArgs.ReadRetVal == TransportReturnCode.SUCCESS);
            Assert.Equal(0, readArgs.BytesRead);
            Assert.Equal(31, readArgs.UncompressedBytesRead);
            Assert.NotNull(recevBuf);
            Assert.Null(error);

            Assert.True(TestUtilities.CompareByteArray(dataByteArray, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));

            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void WriteAndReadANormalMessageFromBigBufferTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();
            int maxFragmentSize = 60;

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, ChannelBase.DEFAULT_HIGH_WATER_MARK, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER,
                maxFragmentSize, RipcVersions.VERSION12, 10); // Sets the max fragmentation size to 60 bytes

            ITransportBuffer messageBuffer = channel.GetBuffer(150, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes(RWF_MSG_1);

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs()
            {
                Flags = WriteFlags.DIRECT_SOCKET_WRITE
            }, out error);

            Assert.Equal(TransportReturnCode.SUCCESS, ret);
            Assert.Null(error);

            ReadArgs readArgs = new ReadArgs();

            ITransportBuffer recevBuf = channel.Read(readArgs, out error);

            Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
            Assert.NotNull(recevBuf);
            Assert.Null(error);

            Assert.True(TestUtilities.CompareByteArray(dataByteArray, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));
            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void Write3PackedMsgTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, ChannelBase.DEFAULT_HIGH_WATER_MARK, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER,
                ChannelBase.DEFAULT_MAX_FRAGMENT_SIZE);

            ITransportBuffer messageBuffer = channel.GetBuffer(500, true, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] msg = Encoding.ASCII.GetBytes("packed test #1");

            messageBuffer.Data.Put(msg);
            Assert.True(channel.PackBuffer(messageBuffer, out error) > 0);

            msg = Encoding.ASCII.GetBytes("packed test #2");
            messageBuffer.Data.Put(msg);
            Assert.True(channel.PackBuffer(messageBuffer, out error) > 0);

            msg = Encoding.ASCII.GetBytes("final packing test #3");
            messageBuffer.Data.Put(msg);
            
            WriteArgs writeArgs = new WriteArgs() { };
            TransportReturnCode ret = channel.Write(messageBuffer, writeArgs, out error);

            Assert.True(ret >= TransportReturnCode.SUCCESS);
            Assert.Null(error);

            Assert.Equal(58, writeArgs.BytesWritten);
            Assert.Equal(58, writeArgs.UncompressedBytesWritten);

            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void WriteRead2PackedMsgTest()
        {
            ConnectOptions conOpt = new ConnectOptions();
            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, ChannelBase.DEFAULT_HIGH_WATER_MARK, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER,
                ChannelBase.DEFAULT_MAX_FRAGMENT_SIZE);

            ITransportBuffer messageBuffer = channel.GetBuffer(500, true, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray1 = Encoding.ASCII.GetBytes("abcdefghijk");

            messageBuffer.Data.Put(dataByteArray1);
            Assert.True(channel.PackBuffer(messageBuffer, out error) > 0);

            byte[]  dataByteArray2 = Encoding.ASCII.GetBytes("lmnoprstuv");
            messageBuffer.Data.Put(dataByteArray2);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs()
            {
                Flags = WriteFlags.DIRECT_SOCKET_WRITE
            }, out error);

            Assert.Equal(TransportReturnCode.SUCCESS, ret);
            Assert.Null(error);

            ReadArgs readArgs = new ReadArgs();

            ITransportBuffer recevBuf = channel.Read(readArgs, out error);
            Assert.NotNull(recevBuf);
            Assert.Equal(dataByteArray1.Length, recevBuf.Length());
            Assert.True(TestUtilities.CompareByteArray(dataByteArray1, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));

            mockChannel.ClearNetworkBuffer(); //channel should already get all data into internal buffer after the first read,
                                              //hence network buffer should be empty

            recevBuf = channel.Read(readArgs, out error);
            Assert.NotNull(recevBuf);
            Assert.Equal(dataByteArray2.Length, recevBuf.Length());
            Assert.True(TestUtilities.CompareByteArray(dataByteArray2, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));

            recevBuf = channel.Read(readArgs, out error);
            Assert.Null(recevBuf);
            Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);

            //write packed message again
            messageBuffer = channel.GetBuffer(500, true, out error);
            messageBuffer.Data.Put(dataByteArray1);
            Assert.True(channel.PackBuffer(messageBuffer, out error) > 0);

            ret = channel.Write(messageBuffer, new WriteArgs()
            {
                Flags = WriteFlags.DIRECT_SOCKET_WRITE
            }, out error);
            Assert.Equal(TransportReturnCode.SUCCESS, ret);
            Assert.Null(error);

            recevBuf = channel.Read(readArgs, out error);
            Assert.NotNull(recevBuf);
            Assert.Equal(dataByteArray1.Length, recevBuf.Length());
            Assert.True(TestUtilities.CompareByteArray(dataByteArray1, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));

            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }


		private static void WriteCompressedRWFMessages(CompressionType compressionType, bool directWrite, int compressionLevel = 6, int fragmentSize = ChannelBase.DEFAULT_MAX_FRAGMENT_SIZE)
        {
            ConnectOptions conOpt = new()
            {
                CompressionType = compressionType
            };

            MockChannel mockChannel = new MockChannel();

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, ChannelBase.DEFAULT_HIGH_WATER_MARK, 
                ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER,
                fragmentSize);

            channel.m_Compressor = compressionType == CompressionType.ZLIB ? new ZlibCompressor() : new Lz4Compressor();

            if (compressionType == CompressionType.ZLIB)
                channel.m_Compressor.CompressionLevel = (byte)compressionLevel;

            channel.m_SessionInDecompress = compressionType;
            channel.m_SessionOutCompression = compressionType;
            channel.m_SessionCompLowThreshold = compressionType == CompressionType.ZLIB ? ChannelBase.ZLIB_COMPRESSION_THRESHOLD : 
                ChannelBase.LZ4_COMPRESSION_THRESHOLD;
            channel.m_DecompressBuffer = new TransportBuffer(channel.InternalFragmentSize);

            for (int i = 1; i <= 2; i++)
            {

                string stringInput;

                if(fragmentSize == ChannelBase.DEFAULT_MAX_FRAGMENT_SIZE)
                    stringInput = i == 1 ? RWF_MSG_LONG_LENGTH_1 : RWF_MSG_LONG_LENGTH_2;
                else
                    stringInput = i == 1 ? RWF_MSG_1 : RWF_MSG_3;

                ITransportBuffer messageBuffer = channel.GetBuffer(stringInput.Length, false, out error);

                Assert.NotNull(messageBuffer);
                Assert.Null(error);

                byte[] dataByteArray = Encoding.ASCII.GetBytes(stringInput);

                messageBuffer.Data.Put(dataByteArray);

                TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs()
                {
                    Flags = directWrite ? WriteFlags.DIRECT_SOCKET_WRITE : WriteFlags.NO_FLAGS
                }, out error);

                if (directWrite == false)
                {
                    Assert.True(ret > TransportReturnCode.SUCCESS);
                    ret = channel.Flush(out error);
                    Assert.Null(error);

                    Assert.Equal(TransportReturnCode.SUCCESS, ret);
                }
                else
                {
                    Assert.Equal(TransportReturnCode.SUCCESS, ret);
                    Assert.Null(error);
                }

                ReadArgs readArgs = new ReadArgs();

                ITransportBuffer recevBuf;

                if(compressionType == CompressionType.ZLIB && fragmentSize < ChannelBase.DEFAULT_MAX_FRAGMENT_SIZE)
                {
                    recevBuf = channel.Read(readArgs, out error);
                    Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
                    Assert.Null(recevBuf);
                    Assert.Null(error);
                }

                recevBuf = channel.Read(readArgs, out error);
                Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
                Assert.NotNull(recevBuf);
                Assert.Null(error);

                Assert.Equal(dataByteArray.Length, recevBuf.Length());
                Assert.True(TestUtilities.CompareByteArray(dataByteArray, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));
            }

            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out _));

            Transport.Uninitialize();
        }

        [Fact]
        public void WriteZLIBCompressedRWFMessage_DirectWriteTest()
        {
            for (int i = 0; i < 4; i++)
                WriteCompressedRWFMessages(CompressionType.ZLIB, true, ZLIB_COMPRESSION_LEVELS[i]);
        }

        [Fact]
        public void WriteZLIBCompressedRWFMessage_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteCompressedRWFMessages(CompressionType.ZLIB, false, ZLIB_COMPRESSION_LEVELS[i]);
        }

        [Fact]
        public void WriteLZ4CompressedRWFMessage_DirectWriteTest()
        {
            WriteCompressedRWFMessages(CompressionType.LZ4, true);
        }

        [Fact]
        public void WriteLZ4CompressedRWFMessage_Test()
        {
            WriteCompressedRWFMessages(CompressionType.LZ4, false);
        }

        [Fact]
        public void WriteZLIBCompressedRWFMessage_DirectWriteTest_SmallFragment()
        {
            for (int i = 0; i < 4; i++)
                WriteCompressedRWFMessages(CompressionType.ZLIB, true, ZLIB_COMPRESSION_LEVELS[i], RWF_MSG_1.Length);
        }

        [Fact]
        public void WriteZLIBCompressedRWFMessage_Test_SmallFragment()
        {
            for (int i = 0; i < 4; i++)
                WriteCompressedRWFMessages(CompressionType.ZLIB, false, ZLIB_COMPRESSION_LEVELS[i], RWF_MSG_1.Length);
        }

        [Fact]
        public void WriteLZ4CompressedRWFMessage_DirectWriteTest_SmallFragment()
        {
            WriteCompressedRWFMessages(CompressionType.LZ4, true, RWF_MSG_1.Length);
        }

        [Fact]
        public void WriteLZ4CompressedRWFMessage_Test_SmallFragment()
        {
            WriteCompressedRWFMessages(CompressionType.LZ4, false, RWF_MSG_1.Length);
        }

        private static void WriteFragmentedRWFMessage(RipcVersions ripcVersion, CompressionType compressionType, bool directWrite, int compressionLevel = 6, int fragmentSize = 30)
        {
            ConnectOptions conOpt = new()
            {
                CompressionType = compressionType
            };

            MockChannel mockChannel = new MockChannel();
            int maxFragmentSize = fragmentSize;

            string inputString = fragmentSize == 30 ? RWF_MSG_2 : RWF_MSG_LONG_LENGTH_3;

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = true
            };

            Transport.Initialize(initArgs, out Error error);

            ChannelBase channel = new ChannelBase(conOpt, mockChannel, ChannelState.ACTIVE, ChannelBase.DEFAULT_HIGH_WATER_MARK, 
                ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER,
                maxFragmentSize, ripcVersion, 10);

            channel.m_Compressor = compressionType == CompressionType.ZLIB ? new ZlibCompressor() : new Lz4Compressor();

            if(compressionType == CompressionType.ZLIB)
            {
                channel.m_Compressor.CompressionLevel = (byte)compressionLevel;
            }

            channel.m_SessionInDecompress = compressionType;
            channel.m_SessionOutCompression = compressionType;
            channel.m_SessionCompLowThreshold = compressionType == CompressionType.ZLIB ? ChannelBase.ZLIB_COMPRESSION_THRESHOLD :
                ChannelBase.LZ4_COMPRESSION_THRESHOLD;
            channel.m_DecompressBuffer = new TransportBuffer(channel.InternalFragmentSize);

            ITransportBuffer messageBuffer = channel.GetBuffer(inputString.Length, false, out error);

            Assert.NotNull(messageBuffer);
            Assert.Null(error);

            byte[] dataByteArray = Encoding.ASCII.GetBytes(inputString);

            messageBuffer.Data.Put(dataByteArray);

            TransportReturnCode ret = channel.Write(messageBuffer, new WriteArgs()
            {
                Flags = directWrite ? WriteFlags.DIRECT_SOCKET_WRITE : WriteFlags.NO_FLAGS
            }, out error);

            if (directWrite == false)
            {
                Assert.True(ret > TransportReturnCode.SUCCESS);
                ret = channel.Flush(out error);
                Assert.Null(error);

                Assert.Equal(TransportReturnCode.SUCCESS, ret);
            }
            else
            {
                Assert.Equal(TransportReturnCode.SUCCESS, ret);
                Assert.Null(error);
            }

            Assert.Equal(TransportReturnCode.SUCCESS, ret);
            Assert.Null(error);
            ByteBuffer networkBuffer = mockChannel.GetNetworkBuffer();
            int InternalMaxFragmentSize = maxFragmentSize + RipcDataMessage.HeaderSize;

            ReadArgs readArgs = new ReadArgs();

            ITransportBuffer recevBuf = channel.Read(readArgs, out error);
            int readRet = (int)readArgs.ReadRetVal;
            Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
            Assert.Equal(networkBuffer.WritePosition, readArgs.BytesRead);
            if(compressionType == CompressionType.NONE)
                Assert.Equal(networkBuffer.WritePosition, readArgs.UncompressedBytesRead);
            else if(compressionLevel != 0)
                Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);

            Assert.Null(recevBuf);
            Assert.Null(error);

            
            recevBuf = channel.Read(readArgs, out error);
            readRet = (int)readArgs.ReadRetVal;
            Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
            Assert.Equal(0, readArgs.BytesRead);
            if (compressionType == CompressionType.NONE)
                Assert.Equal(0, readArgs.UncompressedBytesRead);
            else if (compressionLevel != 0 && maxFragmentSize > 30)
                Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);

            Assert.Null(recevBuf);
            Assert.Null(error);

            recevBuf = channel.Read(readArgs, out error);
            readRet = (int)readArgs.ReadRetVal;
            Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
            Assert.Equal(0, readArgs.BytesRead);
            if (compressionType == CompressionType.NONE)
                Assert.Equal(0, readArgs.UncompressedBytesRead);
            else if (compressionLevel != 0 && maxFragmentSize > 30)
                Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);

            Assert.Null(recevBuf);
            Assert.Null(error);

            if (compressionType == CompressionType.ZLIB && fragmentSize == 30)
            {
                recevBuf = channel.Read(readArgs, out error);
                readRet = (int)readArgs.ReadRetVal;
                Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
                Assert.Equal(0, readArgs.BytesRead);
                if (compressionType == CompressionType.NONE)
                    Assert.Equal(0, readArgs.UncompressedBytesRead);
                else if (compressionLevel != 0 && maxFragmentSize > 30)
                    Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);

                Assert.Null(recevBuf);
                Assert.Null(error);

                /* Addtional read for RIPC version 13 or higher */
                if (ripcVersion >= RipcVersions.VERSION13)
                {
                    recevBuf = channel.Read(readArgs, out error);
                    Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
                    Assert.Equal(0, readArgs.BytesRead);
                    if (compressionType == CompressionType.NONE)
                        Assert.Equal(0, readArgs.UncompressedBytesRead);
                    else if (compressionLevel != 0 && maxFragmentSize > 30)
                        Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);

                    Assert.Null(recevBuf);
                    Assert.Null(error);
                }
            }
            else if (compressionType == CompressionType.LZ4 && fragmentSize == 30)
            {
                /* Addtional read for RIPC version 13 or higher */
                if (ripcVersion >= RipcVersions.VERSION13)
                {
                    recevBuf = channel.Read(readArgs, out error);
                    Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
                    Assert.Equal(0, readArgs.BytesRead);
                    if (compressionType == CompressionType.NONE)
                        Assert.Equal(0, readArgs.UncompressedBytesRead);
                    else if (compressionLevel != 0 && maxFragmentSize > 30)
                        Assert.Equal(InternalMaxFragmentSize, readArgs.UncompressedBytesRead);
                    Assert.Null(recevBuf);
                    Assert.Null(error);
                }
            }

            if (compressionLevel != 0)
            {
                if(compressionLevel == 1 && fragmentSize == 30) // One more read
                {
                    recevBuf = channel.Read(readArgs, out error);
                    Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
                    Assert.Equal(0, readArgs.BytesRead);
                    Assert.Null(recevBuf);
                    Assert.Null(error);
                }

                recevBuf = channel.Read(readArgs, out error);
                Assert.True(readArgs.ReadRetVal == TransportReturnCode.SUCCESS);
                Assert.Equal(0, readArgs.BytesRead);
                Assert.NotNull(recevBuf);
                Assert.Null(error);
            }
            else
            {
                /* No compression */
                do
                {
                    recevBuf = channel.Read(readArgs, out error);
                    Assert.Null(error);
                } while (readArgs.ReadRetVal > TransportReturnCode.SUCCESS);

                Assert.NotNull(recevBuf);
            }

            Assert.Equal(dataByteArray.Length, recevBuf.Length());
            Assert.True(TestUtilities.CompareByteArray(dataByteArray, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));

            Assert.Equal(TransportReturnCode.SUCCESS, channel.Close(out error));

            Transport.Uninitialize();
        }

        [Fact]
        public void Write_RIPC14_FragmentedRWFMessage_ZLIB_DirectWrite_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION14, CompressionType.ZLIB, true, ZLIB_COMPRESSION_LEVELS[i]);
        }

        [Fact]
        public void Write_RIPC14_FragmentedRWFMessage_ZLIB_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION14, CompressionType.ZLIB, false, ZLIB_COMPRESSION_LEVELS[i]);
        }

        [Fact]
        public void Write_RIPC13_FragmentedRWFMessage_ZLIB_DirectWrite_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION13, CompressionType.ZLIB, true, ZLIB_COMPRESSION_LEVELS[i]);
        }

        [Fact]
        public void Write_RIPC13_FragmentedRWFMessage_ZLIB_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION13, CompressionType.ZLIB, false, ZLIB_COMPRESSION_LEVELS[i]);
        }

        [Fact]
        public void Write_RIPC12_FragmentedRWFMessage_ZLIB_DirectWrite_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION12, CompressionType.ZLIB, true, ZLIB_COMPRESSION_LEVELS[i]);
        }

        [Fact]
        public void Write_RIPC12_FragmentedRWFMessage_ZLIB_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION12, CompressionType.ZLIB, false, ZLIB_COMPRESSION_LEVELS[i]);
        }

        [Fact]
        public void Write_RIPC14_FragmentedRWFMessage_LZ4_DirectWrite_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION14, CompressionType.LZ4, true);
        }

        [Fact]
        public void Write_RIPC14_FragmentedRWFMessage_LZ4_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION14, CompressionType.LZ4, false);
        }

        [Fact]
        public void Write_RIPC13_FragmentedRWFMessage_LZ4_DirectWrite_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION13, CompressionType.LZ4, true);
        }

        [Fact]
        public void Write_RIPC13_FragmentedRWFMessage_LZ4_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION13, CompressionType.LZ4, false);
        }

        [Fact]
        public void Write_RIPC12_FragmentedRWFMessage_LZ4_DirectWrite_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION12, CompressionType.LZ4, true);
        }

        [Fact]
        public void Write_RIPC12_FragmentedRWFMessage_LZ4_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION12, CompressionType.LZ4, false);
        }

        [Fact]
        public void Write_RIPC14_FragmentedRWFMessage_ZLIB_DirectWrite_LargeFragmentSize_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION14, CompressionType.ZLIB, true, ZLIB_COMPRESSION_LEVELS[i], LARGE_FRAGMENT_SIZE);
        }

        [Fact]
        public void Write_RIPC14_FragmentedRWFMessage_ZLIB_LargeFragmentSize_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION14, CompressionType.ZLIB, false, ZLIB_COMPRESSION_LEVELS[i], LARGE_FRAGMENT_SIZE);
        }

        [Fact]
        public void Write_RIPC13_FragmentedRWFMessage_ZLIB_DirectWrite_LargeFragmentSize_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION13, CompressionType.ZLIB, true, ZLIB_COMPRESSION_LEVELS[i], LARGE_FRAGMENT_SIZE);
        }

        [Fact]
        public void Write_RIPC13_FragmentedRWFMessage_ZLIB_LargeFragmentSize_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION13, CompressionType.ZLIB, false, ZLIB_COMPRESSION_LEVELS[i], LARGE_FRAGMENT_SIZE);
        }

        [Fact]
        public void Write_RIPC12_FragmentedRWFMessage_ZLIB_DirectWrite_LargeFragmentSize_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION12, CompressionType.ZLIB, true, ZLIB_COMPRESSION_LEVELS[i], LARGE_FRAGMENT_SIZE);
        }

        [Fact]
        public void Write_RIPC12_FragmentedRWFMessage_ZLIB_LargeFragmentSize_Test()
        {
            for (int i = 0; i < 4; i++)
                WriteFragmentedRWFMessage(RipcVersions.VERSION12, CompressionType.ZLIB, false, ZLIB_COMPRESSION_LEVELS[i], LARGE_FRAGMENT_SIZE);
        }

        [Fact]
        public void Write_RIPC14_FragmentedRWFMessage_LZ4_DirectWrite_LargeFragmentSize_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION14, CompressionType.LZ4, true, LARGE_FRAGMENT_SIZE);
        }

        [Fact]
        public void Write_RIPC14_FragmentedRWFMessage_LZ4_LargeFragmentSize_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION14, CompressionType.LZ4, false, LARGE_FRAGMENT_SIZE);
        }

        [Fact]
        public void Write_RIPC13_FragmentedRWFMessage_LZ4_DirectWrite_LargeFragmentSize_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION13, CompressionType.LZ4, true, LARGE_FRAGMENT_SIZE);
        }

        [Fact]
        public void Write_RIPC13_FragmentedRWFMessage_LZ4_LargeFragmentSize_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION13, CompressionType.LZ4, false, LARGE_FRAGMENT_SIZE);
        }

        [Fact]
        public void Write_RIPC12_FragmentedRWFMessage_LZ4_DirectWrite_LargeFragmentSize_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION12, CompressionType.LZ4, true, LARGE_FRAGMENT_SIZE);
        }

        [Fact]
        public void Write_RIPC12_FragmentedRWFMessage_LZ4_LargeFragmentSize_Test()
        {
            WriteFragmentedRWFMessage(RipcVersions.VERSION12, CompressionType.LZ4, false, LARGE_FRAGMENT_SIZE);
        }
		
    }
}
