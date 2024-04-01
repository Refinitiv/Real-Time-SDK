/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2023-2024 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Common;
using LSEG.Eta.Internal;
using LSEG.Eta.Internal.Interfaces;
using LSEG.Eta.Tests;
using LSEG.Eta.Transports;
using LSEG.Eta.Transports.Internal;
using System;
using System.Collections.Generic;
using System.Net;
using System.Net.Security;
using System.Net.Sockets;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using Xunit;
using Xunit.Categories;
using System.Runtime.InteropServices;

namespace LSEG.Eta.Transports.Tests
{
    public class SocketChannelTests
    {
		private const int TIMEOUTMS = 10000;

		private const string RWF_MSG_1 = "This is a complete RWF message";

		private const string RWF_MSG_2 = "This is a second RWF message";

        #region Misc Tests

        [Fact]
		public void ReadNoData()
		{
			InitializeConnection("14001", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);
			try
            {
				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = clientChannel.Read(readArgs, out Error error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Null(recevBuf);
				Assert.Null(error);
            }
            finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void ReadSingleCompleteMessage()
		{
			InitializeConnection("14002", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ITransportBuffer messageBuffer = clientChannel.GetBuffer(150, packedBuffer: false, out Error error);
				Assert.NotNull(messageBuffer);
				Assert.Null(error);
				byte[] DataByteArray = Encoding.ASCII.GetBytes(RWF_MSG_1);
				messageBuffer.Data.Put(DataByteArray);
				TransportReturnCode ret = clientChannel.Write(messageBuffer, new WriteArgs
				{
					Flags = WriteFlags.DIRECT_SOCKET_WRITE
				}, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, ret);
				Assert.Null(error);
				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = serverClientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.NotNull(recevBuf);
				Assert.Null(error);
				Assert.True(TestUtilities.CompareByteArray(DataByteArray, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));
				Assert.Equal(DataByteArray.Length, recevBuf.Length() - recevBuf.GetDataStartPosition() + 3);
				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Null(recevBuf);
				Assert.Null(error);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}			
		}

		[Fact]
		public void IoFailureHandling()
		{
			InitializeConnection("14003", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);
			
			try
            {
				ITransportBuffer messageBuffer = clientChannel.GetBuffer(150, packedBuffer: false, out Error error);
				Assert.NotNull(messageBuffer);
				Assert.Null(error);
				byte[] DataByteArray = Encoding.ASCII.GetBytes(RWF_MSG_1);
				messageBuffer.Data.Put(DataByteArray);
				TransportReturnCode ret = clientChannel.Write(messageBuffer, new WriteArgs
				{
					Flags = WriteFlags.DIRECT_SOCKET_WRITE
				}, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, ret);
				Assert.Null(error);
				ReadArgs readArgs = new ReadArgs();
				ChannelBase serverChannelBase = (ChannelBase)serverClientChannel;
				serverChannelBase.m_ReadBufferStateMachine.State = ReadBufferStateMachine.BufferState.END_OF_STREAM;
				ITransportBuffer recevBuf = serverClientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.FAILURE, readArgs.ReadRetVal);
				Assert.Null(recevBuf);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}			
		}

		[Fact]
		public void ReadTwoCompleteMessages()
		{
			InitializeConnection("14004", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ITransportBuffer messageBuffer = clientChannel.GetBuffer(150, packedBuffer: false, out Error error);
				Assert.NotNull(messageBuffer);
				Assert.Null(error);
				ITransportBuffer messageBuffer2 = clientChannel.GetBuffer(200, packedBuffer: false, out error);
				Assert.NotNull(messageBuffer2);
				Assert.Null(error);
				byte[] DataByteArray = Encoding.ASCII.GetBytes(RWF_MSG_1);
				byte[] DataByteArray2 = Encoding.ASCII.GetBytes(RWF_MSG_2);
				messageBuffer.Data.Put(DataByteArray);
				TransportReturnCode ret = clientChannel.Write(messageBuffer, new WriteArgs
				{
					Flags = WriteFlags.DIRECT_SOCKET_WRITE
				}, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, ret);
				Assert.Null(error);
				messageBuffer2.Data.Put(DataByteArray2);
				ret = clientChannel.Write(messageBuffer2, new WriteArgs
				{
					Flags = WriteFlags.DIRECT_SOCKET_WRITE
				}, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, ret);
				Assert.Null(error);
				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = serverClientChannel.Read(readArgs, out error);
				Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
				Assert.NotNull(recevBuf);
				Assert.Null(error);
				Assert.True(TestUtilities.CompareByteArray(DataByteArray, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));
				Assert.Equal(DataByteArray.Length, recevBuf.Length() - recevBuf.GetDataStartPosition() + 3);
				recevBuf = serverClientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.NotNull(recevBuf);
				Assert.Null(error);
				Assert.True(TestUtilities.CompareByteArray(DataByteArray2, 0, recevBuf.Data.Contents, recevBuf.GetDataStartPosition(), recevBuf.Length()));
				Assert.Equal(DataByteArray2.Length, recevBuf.Length());
				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Null(recevBuf);
				Assert.Null(error);
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}		
		}

		[Fact]
		public void ReleaseBufferTest()
		{
			BindOptions bindOpts = new BindOptions();
			ConnectOptions connectOpts = new ConnectOptions();
			Error error = new Error();
			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;

			bindOpts.ServiceName = "14048";
			bindOpts.ChannelIsBlocking = true;
			bindOpts.ServerBlocking = true;
			bindOpts.MinorVersion = Codec.Codec.MinorVersion();
			bindOpts.MajorVersion = Codec.Codec.MajorVersion();
			bindOpts.ProtocolType = ProtocolType.RWF;
			bindOpts.MaxOutputBuffers = 10;
			bindOpts.SharedPoolSize = 100;
			bindOpts.GuaranteedOutputBuffers = 5;
			bindOpts.MaxFragmentSize = 6144;

			connectOpts.UnifiedNetworkInfo.Address = "localhost";
			connectOpts.ProtocolType = ProtocolType.RWF;
			connectOpts.UnifiedNetworkInfo.ServiceName = "14048";
			connectOpts.ConnectionType = ConnectionType.SOCKET;
			connectOpts.MajorVersion = Codec.Codec.MajorVersion();
			connectOpts.MinorVersion = Codec.Codec.MinorVersion();
			connectOpts.NumInputBuffers = 10;
			connectOpts.GuaranteedOutputBuffers = 8;

			InitializeConnection(bindOpts, connectOpts, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				string testData = "releaseBufferTest";
				int bufLen = testData.Length;
				ByteBuffer byteBuf = new ByteBuffer(bufLen + 3);
				byteBuf.Put(Encoding.ASCII.GetBytes(testData));

				TransportBuffer buf1 = (TransportBuffer)clientChannel.GetBuffer(6100, false, out error);
				buf1.IsReadMode = false;

				Assert.Equal(6100, buf1.Length());
				Assert.Equal(1, (int)clientChannel.BufferUsage(out error));

				TransportBuffer buf2 = (TransportBuffer)clientChannel.GetBuffer(6100, false, out error);

				Assert.Equal(2, (int)clientChannel.BufferUsage(out error));

				TransportBuffer buf3 = (TransportBuffer)clientChannel.GetBuffer(6100, false, out error);

				Assert.Equal(3, (int)clientChannel.BufferUsage(out error));
				TransportBuffer buf4 = (TransportBuffer)clientChannel.GetBuffer(6100, false, out error);

				Assert.Equal(4, (int)clientChannel.BufferUsage(out error));

				TransportBuffer buf5 = (TransportBuffer)clientChannel.GetBuffer(6100, false, out error);

				Assert.Equal(5, (int)clientChannel.BufferUsage(out error));

				TransportBuffer buf6 = (TransportBuffer)clientChannel.GetBuffer(6100, false, out error);

				Assert.Equal(6, (int)clientChannel.BufferUsage(out error));

				TransportBuffer buf7 = (TransportBuffer)clientChannel.GetBuffer(6100, false, out error);

				Assert.Equal(7, (int)clientChannel.BufferUsage(out error));

				buf1.Data.Put(Encoding.ASCII.GetBytes(testData));
				clientChannel.Write(buf1, writeArgs, out error);
				clientChannel.Flush(out error);

				Assert.Equal(6, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf2, out error);
				Assert.Equal(5, (int)clientChannel.BufferUsage(out error));

				TransportBuffer buf8 = (TransportBuffer)clientChannel.GetBuffer(6100, false, out error);

				TransportBuffer buf = (TransportBuffer)clientChannel.GetBuffer(6100, false, out error);

				Assert.Equal(7, (int)clientChannel.BufferUsage(out error));
				Assert.Equal(buf1, buf);

				buf = (TransportBuffer)clientChannel.GetBuffer(6100, false, out error);
				Assert.Equal(8, (int)clientChannel.BufferUsage(out error));
				Assert.Equal(buf2, buf);

				clientChannel.ReleaseBuffer(buf1, out error);
				Assert.Equal(7, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf2, out error); // is not released because is current Buffer
				Assert.Equal(7, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf3, out error);
				Assert.Equal(6, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf4, out error);
				Assert.Equal(5, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf5, out error);
				Assert.Equal(4, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf6, out error);
				Assert.Equal(3, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf7, out error);
				Assert.Equal(2, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf8, out error);

				Assert.Equal(1, (int)clientChannel.BufferUsage(out error));

				clientChannel.ReleaseBuffer(buf1, out error);
				clientChannel.ReleaseBuffer(buf2, out error);
				clientChannel.ReleaseBuffer(buf3, out error);
				clientChannel.ReleaseBuffer(buf4, out error);
				clientChannel.ReleaseBuffer(buf5, out error);
				clientChannel.ReleaseBuffer(buf6, out error);
				clientChannel.ReleaseBuffer(buf7, out error);
				clientChannel.ReleaseBuffer(buf8, out error);

				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
				Assert.Equal(1, (int)clientChannel.BufferUsage(out error));

				TransportBuffer buf11 = (TransportBuffer)clientChannel.GetBuffer(60000, false, out error);
				buf11.IsReadMode = false;
				Assert.True(60000 == buf11.Length());
				Assert.Equal(2, (int)clientChannel.BufferUsage(out error));
				TransportBuffer buf12 = (TransportBuffer)clientChannel.GetBuffer(60000, false, out error);
				Assert.Equal(3, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf11, out error);
				Assert.Equal(2, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf11, out error);
				Assert.Equal(2, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf12, out error);
				Assert.Equal(1, (int)clientChannel.BufferUsage(out error));
				clientChannel.ReleaseBuffer(buf11, out error);
				Assert.Equal(1, (int)clientChannel.BufferUsage(out error));
				buf = (TransportBuffer)clientChannel.GetBuffer(60000, false, out error);
				Assert.Equal(buf, buf11);
				Assert.Equal(2, (int)clientChannel.BufferUsage(out error));
				buf = (TransportBuffer)clientChannel.GetBuffer(60000, false, out error);
				Assert.Equal(buf, buf12);
				Assert.Equal(3, (int)clientChannel.BufferUsage(out error));
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out error);
				if (serverClientChannel != null) serverClientChannel.Close(out error);
				if (server != null) server.Close(out error);

				Transport.Uninitialize();
			}			
		}
		
		[Fact]
		public void NumGuaranteedBuffersTwoChannelTest()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			int initialBufferCount = 20;

			ConnectOptions opts = new ConnectOptions();
			opts.UnifiedNetworkInfo.ServiceName = "12345";
			opts.GuaranteedOutputBuffers = initialBufferCount;

			// setup the Channel
			SocketProtocol transport = new SocketProtocol();
			ChannelBase rsslChnl1 = new ChannelBase(transport, opts, new SocketChannel());
			ChannelBase rsslChnl2 = new ChannelBase(transport, opts, new SocketChannel());
			
			rsslChnl1.GrowGuaranteedOutputBuffers(opts.GuaranteedOutputBuffers);
			rsslChnl2.GrowGuaranteedOutputBuffers(opts.GuaranteedOutputBuffers);

			// verify setup.
			Pool bufferPool = transport.GetPool(rsslChnl1.InternalFragmentSize);
			Assert.Equal(0, bufferPool.Size);
			Assert.Equal(initialBufferCount, rsslChnl1.m_AvailableBuffers.Size);
			Assert.Equal(initialBufferCount, rsslChnl1.m_ChannelInfo.GuaranteedOutputBuffers);
			Assert.Equal(initialBufferCount, rsslChnl1.m_ChannelInfo.MaxOutputBuffers);

			// verify setup.
			bufferPool = transport.GetPool(rsslChnl2.InternalFragmentSize);
			Assert.Equal(0, bufferPool.Size);
			Assert.Equal(initialBufferCount, rsslChnl2.m_AvailableBuffers.Size);
			Assert.Equal(initialBufferCount, rsslChnl2.m_ChannelInfo.GuaranteedOutputBuffers);
			Assert.Equal(initialBufferCount, rsslChnl2.m_ChannelInfo.MaxOutputBuffers);

			// release all buffers back to pool
			rsslChnl1.ShrinkGuaranteedOutputBuffers(rsslChnl1.m_AvailableBuffers.Size);
			rsslChnl2.ShrinkGuaranteedOutputBuffers(rsslChnl2.m_AvailableBuffers.Size);

			// verify release
			bufferPool = transport.GetPool(rsslChnl1.InternalFragmentSize);
			Assert.Equal(initialBufferCount * 2, bufferPool.Size);
			Assert.Equal(0, rsslChnl1.m_AvailableBuffers.Size);
			bufferPool = transport.GetPool(rsslChnl2.InternalFragmentSize);
			Assert.Equal(initialBufferCount * 2, bufferPool.Size);
			Assert.Equal(0, rsslChnl2.m_AvailableBuffers.Size);

			// get the buffers back
			rsslChnl1.GrowGuaranteedOutputBuffers(opts.GuaranteedOutputBuffers);
			rsslChnl2.GrowGuaranteedOutputBuffers(opts.GuaranteedOutputBuffers);

			// verify buffers are back
			bufferPool = transport.GetPool(rsslChnl1.InternalFragmentSize);
			Assert.Equal(0, bufferPool.Size);
			Assert.Equal(initialBufferCount, rsslChnl1.m_AvailableBuffers.Size);
			Assert.Equal(initialBufferCount, rsslChnl1.m_ChannelInfo.GuaranteedOutputBuffers);
			Assert.Equal(initialBufferCount, rsslChnl1.m_ChannelInfo.MaxOutputBuffers);

			// verify buffers are back
			bufferPool = transport.GetPool(rsslChnl2.InternalFragmentSize);
			Assert.Equal(0, bufferPool.Size);
			Assert.Equal(initialBufferCount, rsslChnl2.m_AvailableBuffers.Size);
			Assert.Equal(initialBufferCount, rsslChnl2.m_ChannelInfo.GuaranteedOutputBuffers);
			Assert.Equal(initialBufferCount, rsslChnl2.m_ChannelInfo.MaxOutputBuffers);

			// release all buffers back to pool
			rsslChnl1.ShrinkGuaranteedOutputBuffers(rsslChnl1.m_AvailableBuffers.Size);
			rsslChnl2.ShrinkGuaranteedOutputBuffers(rsslChnl2.m_AvailableBuffers.Size);

			// verify release
			bufferPool = transport.GetPool(rsslChnl1.InternalFragmentSize);
			Assert.Equal(initialBufferCount * 2, bufferPool.Size);
			Assert.Equal(0, rsslChnl1.m_AvailableBuffers.Size);
			bufferPool = transport.GetPool(rsslChnl2.InternalFragmentSize);
			Assert.Equal(initialBufferCount * 2, bufferPool.Size);
			Assert.Equal(0, rsslChnl2.m_AvailableBuffers.Size);

			// get the buffers back
			rsslChnl1.GrowGuaranteedOutputBuffers(opts.GuaranteedOutputBuffers);
			rsslChnl2.GrowGuaranteedOutputBuffers(opts.GuaranteedOutputBuffers);

			// verify buffers are back
			bufferPool = transport.GetPool(rsslChnl1.InternalFragmentSize);
			Assert.Equal(0, bufferPool.Size);
			Assert.Equal(initialBufferCount, rsslChnl1.m_AvailableBuffers.Size);
			Assert.Equal(initialBufferCount, rsslChnl1.m_ChannelInfo.GuaranteedOutputBuffers);
			Assert.Equal(initialBufferCount, rsslChnl1.m_ChannelInfo.MaxOutputBuffers);

			// verify buffers are back
			bufferPool = transport.GetPool(rsslChnl2.InternalFragmentSize);
			Assert.Equal(0, bufferPool.Size);
			Assert.Equal(initialBufferCount, rsslChnl2.m_AvailableBuffers.Size);
			Assert.Equal(initialBufferCount, rsslChnl2.m_ChannelInfo.GuaranteedOutputBuffers);
			Assert.Equal(initialBufferCount, rsslChnl2.m_ChannelInfo.MaxOutputBuffers);

			Transport.Uninitialize();
		}

		#endregion

		#region ReadBufferStateMachine Testing

		[Fact]
		public void ReadTwoPartKnownIncomplete()
		{
			InitializeConnection("14005", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);
			
			try
            {
				byte[] DataByteArray = new byte[80] { 1, 129, 2, 1, 124, 2, 1, 0, 0, 0, 1, 104, 0,
					9, 0, 33, 76, 111, 103, 105, 110, 32, 97, 99, 99, 101, 112, 116, 101, 100, 32,
					98, 121, 32, 104, 111, 115, 116, 32, 120, 120, 120, 120, 120, 120, 120, 120,
					120, 120, 2, 0, 0, 129, 75, 38, 16, 120, 120, 120, 120, 120, 120, 46, 120,
					120, 120, 120, 120, 120, 120, 120, 120, 1, 5, 129, 53, 8, 0, 13, 13
				};
				byte[] DataByteArray2 = new byte[305] { 65, 112, 112, 108, 105, 99, 97, 116, 105,
					111, 110, 73, 100, 17, 3, 50, 53, 54, 15, 65, 112, 112, 108, 105, 99, 97, 116,
					105, 111, 110, 78, 97, 109, 101, 17, 12, 114, 115, 115, 108, 80, 114, 111, 118,
					105, 100, 101, 114, 8, 80, 111, 115, 105, 116, 105, 111, 110, 17, 16, 49, 48,
					46, 57, 49, 46, 49, 54, 49, 46, 51, 51, 47, 110, 101, 116, 24, 80, 114, 111,
					118, 105, 100, 101, 80, 101, 114, 109, 105, 115, 115, 105, 111, 110, 80, 114,
					111, 102, 105, 108, 101, 4, 1, 1, 28, 80, 114, 111, 118, 105, 100, 101, 80,
					101, 114, 109, 105, 115, 115, 105, 111, 110, 69, 120, 112, 114, 101, 115, 115,
					105, 111, 110, 115, 4, 1, 1, 10, 83, 105, 110, 103, 108, 101, 79, 112, 101,
					110, 4, 1, 0, 16, 65, 108, 108, 111, 119, 83, 117, 115, 112, 101, 99, 116, 68,
					97, 116, 97, 4, 1, 1, 18, 83, 117, 112, 112, 111, 114, 116, 80, 97, 117, 115,
					101, 82, 101, 115, 117, 109, 101, 4, 1, 0, 27, 83, 117, 112, 112, 111, 114, 116,
					79, 112, 116, 105, 109, 105, 122, 101, 100, 80, 97, 117, 115, 101, 82, 101, 115,
					117, 109, 101, 4, 1, 0, 14, 83, 117, 112, 112, 111, 114, 116, 79, 77, 77, 80, 111,
					115, 116, 4, 1, 0, 19, 83, 117, 112, 112, 111, 114, 116, 86, 105, 101, 119, 82,
					101, 113, 117, 101, 115, 116, 115, 4, 1, 0, 20, 83, 117, 112, 112, 111, 114, 116,
					66, 97, 116, 99, 104, 82, 101, 113, 117, 101, 115, 116, 115, 4, 1, 0, 14, 83,
					117, 112, 112, 111, 114, 116, 83, 116, 97, 110, 100, 98, 121, 4, 1, 0
				};

				ChannelBase clientBase = (ChannelBase)clientChannel;
				clientBase.Socket.Send(DataByteArray);
				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = serverClientChannel.Read(readArgs, out Error error);
				Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
				Assert.Equal(ReadBufferStateMachine.BufferState.KNOWN_INCOMPLETE, ((ChannelBase)serverClientChannel).m_ReadBufferStateMachine.State);
				Assert.Null(recevBuf);
				Assert.Null(error);
				Assert.Equal(DataByteArray.Length, readArgs.BytesRead);
				clientBase.Socket.Send(DataByteArray2);
				readArgs.Clear();
				recevBuf = serverClientChannel.Read(readArgs, out error);
				Assert.True(readArgs.ReadRetVal == TransportReturnCode.SUCCESS);
				Assert.NotNull(recevBuf);
				Assert.Null(error);
				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Null(recevBuf);
				Assert.Null(error);
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void ReadTwoPartUnknownIncomplete()
		{
			InitializeConnection("14006", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);
			
			byte[] DataByteArray = new byte[1] { 1 };
			byte[] DataByteArray2 = new byte[384] { 129, 2, 1, 124, 2, 1, 0, 0, 0, 1, 104, 0, 9, 0, 33, 
				76, 111, 103, 105, 110, 32, 97, 99, 99, 101, 112, 116, 101, 100, 32, 98, 121, 32, 104, 
				111, 115, 116, 32, 120, 120, 120, 120, 120, 120, 120, 120, 120, 120, 2, 0, 0, 129, 75, 
				38, 16, 120, 120, 120, 120, 120, 120, 46, 120, 120, 120, 120, 120, 120, 120, 120, 120, 
				1, 5, 129, 53, 8, 0, 13, 13, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 73, 100, 
				17, 3, 50, 53, 54, 15, 65, 112, 112, 108, 105, 99, 97, 116, 105, 111, 110, 78, 97, 109, 
				101, 17, 12, 114, 115, 115, 108, 80, 114, 111, 118, 105, 100, 101, 114, 8, 80, 111, 
				115, 105, 116, 105, 111, 110, 17, 16, 49, 48, 46, 57, 49, 46, 49, 54, 49, 46, 51, 51, 
				47, 110, 101, 116, 24, 80, 114, 111, 118, 105, 100, 101, 80, 101, 114, 109, 105, 115, 
				115, 105, 111, 110, 80, 114, 111, 102, 105, 108, 101, 4, 1, 1, 28, 80, 114, 111, 118, 
				105, 100, 101, 80, 101, 114, 109, 105, 115, 115, 105, 111, 110, 69, 120, 112, 114, 
				101, 115, 115, 105, 111, 110, 115, 4, 1, 1, 10, 83, 105, 110, 103, 108, 101, 79, 112, 
				101, 110, 4, 1, 0, 16, 65, 108, 108, 111, 119, 83, 117, 115, 112, 101, 99, 116, 68, 
				97, 116, 97, 4, 1, 1, 18, 83, 117, 112, 112, 111, 114, 116, 80, 97, 117, 115, 101, 
				82, 101, 115, 117, 109, 101, 4, 1, 0, 27, 83, 117, 112, 112, 111, 114, 116, 79, 112,
				116, 105, 109, 105, 122, 101, 100, 80, 97, 117, 115, 101, 82, 101, 115, 117, 109, 101, 4, 1,
				0, 14, 83, 117, 112, 112, 111, 114, 116, 79, 77, 77, 80, 111, 115, 116, 4, 1, 0, 19,
				83, 117, 112, 112, 111, 114, 116, 86, 105, 101, 119, 82, 101, 113, 117, 101, 115, 116, 115, 4,
				1, 0, 20, 83, 117, 112, 112, 111, 114, 116, 66, 97, 116, 99, 104, 82, 101, 113, 117, 101,
				115, 116, 115, 4, 1, 0, 14, 83, 117, 112, 112, 111, 114, 116, 83, 116, 97, 110, 100, 98, 121, 4, 1, 0 };

			try
            {
				ChannelBase clientBase = (ChannelBase)clientChannel;
				clientBase.Socket.Send(DataByteArray);
				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = serverClientChannel.Read(readArgs, out Error error);
				Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
				Assert.Equal(ReadBufferStateMachine.BufferState.UNKNOWN_INCOMPLETE, ((ChannelBase)serverClientChannel).m_ReadBufferStateMachine.State);
				Assert.Null(recevBuf);
				Assert.Null(error);
				Assert.Equal(DataByteArray.Length, readArgs.BytesRead);
				clientBase.Socket.Send(DataByteArray2);
				readArgs.Clear();
				recevBuf = serverClientChannel.Read(readArgs, out error);
				Assert.True(readArgs.ReadRetVal == TransportReturnCode.SUCCESS);
				Assert.NotNull(recevBuf);
				Assert.Null(error);
				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Null(recevBuf);
				Assert.Null(error);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}		
		}

		[Fact]
		public void ReadKnownInsufficient()
		{
			InitializeConnection("14007", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ChannelBase clientBase = (ChannelBase)clientChannel;
				int length = clientBase.m_ReadBufferStateMachine.Buffer.Contents.Length + 10;
				byte[] DataArray = new byte[length + 3];
				DataArray[0] = (byte)(length + 3 >> 8);
				DataArray[1] = (byte)((uint)(length + 3) & 0xFFu);
				DataArray[2] = 2;
				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(DataArray);
				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = clientChannel.Read(readArgs, out Error error);
				Assert.Equal(ReadBufferStateMachine.BufferState.KNOWN_INSUFFICENT, clientBase.m_ReadBufferStateMachine.State);
				Assert.Null(error);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void ReadKnownInsufficientRemainingBuf()
		{
			InitializeConnection("14000", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ChannelBase clientBase = (ChannelBase)clientChannel;
				int length = clientBase.m_ReadBufferStateMachine.Buffer.Contents.Length;
				byte[] DataArray = new byte[length - 1];
				DataArray[0] = (byte)(length - 4 >> 8);
				DataArray[1] = (byte)((uint)(length - 4) & 0xFFu);
				DataArray[2] = 2;
				for (int k = 3; k < length - 4; k++)
				{
					DataArray[k] = (byte)k;
				}
				DataArray[length - 3] = 10;
				DataArray[length - 2] = 2;
				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(DataArray);
				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = clientChannel.Read(readArgs, out Error error);
				Assert.Equal(length - 4 - 3, recevBuf.Length());
				Assert.Null(error);
				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(ReadBufferStateMachine.BufferState.KNOWN_INCOMPLETE, clientBase.m_ReadBufferStateMachine.State);
				DataArray = new byte[7];
				for (int j = 0; j < 7; j++)
				{
					DataArray[j] = (byte)j;
				}
				serverBase.Socket.Send(DataArray);
				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(ReadBufferStateMachine.BufferState.KNOWN_COMPLETE, clientBase.m_ReadBufferStateMachine.State);
				Assert.Equal(7, recevBuf.Length());
				Assert.Null(error);
				for (int i = 0; i < 7; i++)
				{
					Assert.Equal(i, recevBuf.Data.Contents[recevBuf.GetDataStartPosition() + i]);
				}
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}		
		}

		[Fact]
		public void ReadUnknownInsufficient()
		{
			InitializeConnection("14008", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);
			
			try
            {
				ChannelBase clientBase = (ChannelBase)clientChannel;
				int length = clientBase.m_ReadBufferStateMachine.Buffer.Contents.Length;
				byte[] DataArray = new byte[length + 9];
				DataArray[0] = (byte)(length - 1 >> 8);
				DataArray[1] = (byte)((uint)(length - 1) & 0xFFu);
				DataArray[2] = 2;
				DataArray[length] = 10;
				DataArray[length + 1] = 2;
				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(DataArray);
				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = clientChannel.Read(readArgs, out Error error);
				Assert.Equal(length - 1 - RipcLengths.HEADER, recevBuf.Length());
				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(7, recevBuf.Length());
				Assert.Null(error);
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}			
		}

        #endregion

        #region Packed Messages
        [Fact]
		public void ReadSingleComplete1PartPackedMessage()
        {
			InitializeConnection("14009", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			byte[] DataArray = { 0x01, 0x7C, 0x12, 0x01, 0x77, 0x01, 0x75, 0x02, 0x01, 0x00, 0x00, 0x00, 0x01, 0x81, 0x68, 0x00,
									0x09, 0x00, 0x22, 0x4C, 0x6F, 0x67, 0x69, 0x6E, 0x20, 0x61, 0x63, 0x63, 0x65, 0x70, 0x74, 0x65,
									0x64, 0x20, 0x62, 0x79, 0x20, 0x68, 0x6F, 0x73, 0x74, 0x20, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78,
									0x78, 0x78, 0x78, 0x78, 0x2E, 0x02, 0x00, 0x00, 0x81, 0x42, 0x26, 0x10, 0x78, 0x78, 78, 0x78,
									0x78, 0x78, 0x2E, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x78, 0x01, 0x05, 0x81, 0x2C,
									0x08, 0x00, 0x0D, 0x10, 0x41, 0x6C, 0x6C, 0x6F, 0x77, 0x53, 0x75, 0x73, 0x70, 0x65, 0x63, 0x74,
									0x44, 0x61, 0x74, 0x61, 0x04, 0x01, 0x01, 0x0D, 0x41, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74,
									0x69, 0x6F, 0x6E, 0x49, 0x64, 0x11, 0x03, 0x32, 0x35, 0x36, 0x0F, 0x41, 0x70, 0x70, 0x6C, 0x69,
									0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x4E, 0x61, 0x6D, 0x65, 0x11, 0x03, 0x41, 0x44, 0x53, 0x08,
									0x50, 0x6F, 0x73, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x11, 0x10, 0x31, 0x30, 0x2E, 0x39, 0x31, 0x2E,
									0x31, 0x36, 0x31, 0x2E, 0x35, 0x39, 0x2F, 0x6E, 0x65, 0x74, 0x1C, 0x50, 0x72, 0x6F, 0x76, 0x69,
									0x64, 0x65, 0x50, 0x65, 0x72, 0x6D, 0x69, 0x73, 0x73, 0x69, 0x6F, 0x6E, 0x45, 0x78, 0x70, 0x72,
									0x65, 0x73, 0x73, 0x69, 0x6F, 0x6E, 0x73, 0x04, 0x01, 0x01, 0x18, 0x50, 0x72, 0x6F, 0x76, 0x69,
									0x64, 0x65, 0x50, 0x65, 0x72, 0x6D, 0x69, 0x73, 0x73, 0x69, 0x6F, 0x6E, 0x50, 0x72, 0x6F, 0x66,
									0x69, 0x6C, 0x65, 0x04, 0x01, 0x00, 0x0A, 0x53, 0x69, 0x6E, 0x67, 0x6C, 0x65, 0x4F, 0x70, 0x65,
									0x6E, 0x04, 0x01, 0x01, 0x0E, 0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x4F, 0x4D, 0x4D, 0x50,
									0x6F, 0x73, 0x74, 0x04, 0x01, 0x01, 0x12, 0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x50, 0x61,
									0x75, 0x73, 0x65, 0x52, 0x65, 0x73, 0x75, 0x6D, 0x65, 0x04, 0x01, 0x01, 0x0E, 0x53, 0x75, 0x70,
									0x70, 0x6F, 0x72, 0x74, 0x53, 0x74, 0x61, 0x6E, 0x64, 0x62, 0x79, 0x04, 0x01, 0x01, 0x14, 0x53,
									0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x42, 0x61, 0x74, 0x63, 0x68, 0x52, 0x65, 0x71, 0x75, 0x65,
									0x73, 0x74, 0x73, 0x04, 0x01, 0x01, 0x13, 0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x56, 0x69,
									0x65, 0x77, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x73, 0x04, 0x01, 0x01, 0x1B, 0x53, 0x75,
									0x70, 0x70, 0x6F, 0x72, 0x74, 0x4F, 0x70, 0x74, 0x69, 0x6D, 0x69, 0x7A, 0x65, 0x64, 0x50, 0x61,
									0x75, 0x73, 0x65, 0x52, 0x65, 0x73, 0x75, 0x6D, 0x65, 0x04, 0x01, 0x01 };

			try
            {
				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(DataArray);

				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = clientChannel.Read(readArgs, out Error error);
				Assert.NotNull(recevBuf);
				Assert.Equal(DataArray.Length - FirstPackedHeaderLength(), recevBuf.Length());
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);

				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}			
		}

		[Fact]
		public void ReadSinglePackedEmptyPayload()
		{
			InitializeConnection("14010", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			byte[] DataArray = { 0x00, 0x05, 0x12, 0x00, 0x00 };

			try
            {
				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(DataArray);

				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = clientChannel.Read(readArgs, out Error error);
				Assert.Null(recevBuf);
				Assert.Equal(FirstPackedHeaderLength(), readArgs.BytesRead);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}			
		}

		[Fact]
		public void ReadSingleComplete2PartPackedMessage()
		{
			InitializeConnection("14011", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			byte[] DataArray = { 0x00, 0x98, 0x12, 0x00, 0x48, 0x00, 0x09, 0x04, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x04, 0x00,
									0x08, 0x00, 0x0A, 0x00, 0x16, 0x03, 0x0C, 0x0A, 0x96, 0x00, 0x19, 0x04, 0x0A, 0x04, 0x22, 0xFC,
									0x00, 0x1E, 0x02, 0x0E, 0x05, 0x00, 0x1F, 0x02, 0x0E, 0x06, 0x01, 0x25, 0x04, 0x20, 0x44, 0x45,
									0x58, 0x01, 0x28, 0x04, 0x20, 0x4E, 0x59, 0x53, 0x00, 0x76, 0x01, 0x3C, 0x04, 0x01, 0x03, 0x12,
									0x01, 0x29, 0x0C, 0xC0, 0x01, 0x3C, 0x0F, 0x0F, 0x04, 0x03, 0xDE, 0x52, 0x35, 0x00, 0x49, 0x00,
									0x09, 0x04, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x04, 0x00, 0x08, 0x00, 0x0A, 0x00, 0x16, 0x04,
									0x0A, 0x04, 0x22, 0x98, 0x00, 0x19, 0x04, 0x0A, 0x04, 0x22, 0xFC, 0x00, 0x1E, 0x02, 0x0E, 0x07,
									0x00, 0x1F, 0x02, 0x0E, 0x06, 0x00, 0x76, 0x01, 0x3C, 0x01, 0x25, 0x04, 0x20, 0x4E, 0x59, 0x53,
									0x01, 0x28, 0x04, 0x20, 0x4E, 0x59, 0x53, 0x04, 0x01, 0x03, 0x12, 0x01, 0x29, 0x0C, 0xC0, 0x01,
									0x3C, 0x0F, 0x0F, 0x04, 0x03, 0xDE, 0x52, 0x35 };

			try
            {
				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(DataArray);

				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = clientChannel.Read(readArgs, out Error error);
				Assert.NotNull(recevBuf);
				Assert.Equal(DataArray[4], recevBuf.Length());
				Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.NotNull(recevBuf);
				Assert.Equal(DataArray.Length - FirstPackedHeaderLength() - DataArray[4] - RipcOffsets.PACKED_MSG_DATA, recevBuf.Length());
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Null(recevBuf);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}			
		}

		[Fact]
		public void Read3PartLastPartEmptyPackedMessage()
		{
			InitializeConnection("14012", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			byte[] DataArray = { 0x00, 0x9A, 0x12, 0x00, 0x48, 0x00, 0x09, 0x04, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x04, 0x00,
									0x08, 0x00, 0x0A, 0x00, 0x16, 0x03, 0x0C, 0x0A, 0x96, 0x00, 0x19, 0x04, 0x0A, 0x04, 0x22, 0xFC,
									0x00, 0x1E, 0x02, 0x0E, 0x05, 0x00, 0x1F, 0x02, 0x0E, 0x06, 0x01, 0x25, 0x04, 0x20, 0x44, 0x45,
									0x58, 0x01, 0x28, 0x04, 0x20, 0x4E, 0x59, 0x53, 0x00, 0x76, 0x01, 0x3C, 0x04, 0x01, 0x03, 0x12,
									0x01, 0x29, 0x0C, 0xC0, 0x01, 0x3C, 0x0F, 0x0F, 0x04, 0x03, 0xDE, 0x52, 0x35, 0x00, 0x49, 0x00,
									0x09, 0x04, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x04, 0x00, 0x08, 0x00, 0x0A, 0x00, 0x16, 0x04,
									0x0A, 0x04, 0x22, 0x98, 0x00, 0x19, 0x04, 0x0A, 0x04, 0x22, 0xFC, 0x00, 0x1E, 0x02, 0x0E, 0x07,
									0x00, 0x1F, 0x02, 0x0E, 0x06, 0x00, 0x76, 0x01, 0x3C, 0x01, 0x25, 0x04, 0x20, 0x4E, 0x59, 0x53,
									0x01, 0x28, 0x04, 0x20, 0x4E, 0x59, 0x53, 0x04, 0x01, 0x03, 0x12, 0x01, 0x29, 0x0C, 0xC0, 0x01,
									0x3C, 0x0F, 0x0F, 0x04, 0x03, 0xDE, 0x52, 0x35, 0x00, 0x00 };
			byte[] firstMsg = { 0x00, 0x09, 0x04, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x04, 0x00,
									0x08, 0x00, 0x0A, 0x00, 0x16, 0x03, 0x0C, 0x0A, 0x96, 0x00, 0x19, 0x04, 0x0A, 0x04, 0x22, 0xFC,
									0x00, 0x1E, 0x02, 0x0E, 0x05, 0x00, 0x1F, 0x02, 0x0E, 0x06, 0x01, 0x25, 0x04, 0x20, 0x44, 0x45,
									0x58, 0x01, 0x28, 0x04, 0x20, 0x4E, 0x59, 0x53, 0x00, 0x76, 0x01, 0x3C, 0x04, 0x01, 0x03, 0x12,
									0x01, 0x29, 0x0C, 0xC0, 0x01, 0x3C, 0x0F, 0x0F, 0x04, 0x03, 0xDE, 0x52, 0x35 };
			byte[] secondMsg = { 0x00, 0x09, 0x04, 0x06, 0x00, 0x00, 0x00, 0x06, 0x00, 0x04, 0x00, 0x08, 0x00, 0x0A, 0x00, 0x16, 0x04,
									0x0A, 0x04, 0x22, 0x98, 0x00, 0x19, 0x04, 0x0A, 0x04, 0x22, 0xFC, 0x00, 0x1E, 0x02, 0x0E, 0x07,
									0x00, 0x1F, 0x02, 0x0E, 0x06, 0x00, 0x76, 0x01, 0x3C, 0x01, 0x25, 0x04, 0x20, 0x4E, 0x59, 0x53,
									0x01, 0x28, 0x04, 0x20, 0x4E, 0x59, 0x53, 0x04, 0x01, 0x03, 0x12, 0x01, 0x29, 0x0C, 0xC0, 0x01,
									0x3C, 0x0F, 0x0F, 0x04, 0x03, 0xDE, 0x52, 0x35 };

			try
            {
				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(DataArray);

				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = clientChannel.Read(readArgs, out Error error);
				Assert.NotNull(recevBuf);
				Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
				Assert.Equal(firstMsg.Length, recevBuf.Length());
				for (int i = 0; i < firstMsg.Length; i++)
					Assert.Equal(firstMsg[i], recevBuf.Data.Contents[i + recevBuf.GetDataStartPosition()]);

				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.NotNull(recevBuf);
				Assert.Equal(secondMsg.Length, recevBuf.Length());
				Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);
				for (int i = 0; i < secondMsg.Length; i++)
					Assert.Equal(secondMsg[i], recevBuf.Data.Contents[i + recevBuf.GetDataStartPosition()]);

				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Null(recevBuf);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}		
		}

        #endregion

        #region Fragmented Messages

        [Fact]
		public void FragmentedMessage()
        {
			InitializeConnection("14013", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			byte[] fragmentHeader = { 0x00, 0x11, 0x03, 0x08, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
			byte[] fragment = { 0x00, 0x09, 0x03, 0x04, 0x00, 0x01, 0x07, 0x08, 0x09 };
			byte[] normalMsg = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

			try
            {
				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(fragmentHeader);

				int cumulativeBytesRead = 0;
				int cumulativeUncompressedBytesRead = 0;

				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = clientChannel.Read(readArgs, out Error error);
				Assert.Null(recevBuf);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Equal(fragmentHeader.Length, readArgs.BytesRead);
				cumulativeBytesRead += readArgs.BytesRead;
				cumulativeUncompressedBytesRead += readArgs.UncompressedBytesRead;

				serverBase.Socket.Send(fragment);
				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(fragment.Length, readArgs.BytesRead);
				cumulativeBytesRead += readArgs.BytesRead;
				cumulativeUncompressedBytesRead += readArgs.UncompressedBytesRead;

				Assert.Equal(cumulativeBytesRead, FirstFragmentHeaderLength(serverBase) + AdditionalFragmentHeaderLength(serverBase) + normalMsg.Length);
				Assert.Equal(cumulativeUncompressedBytesRead, FirstFragmentHeaderLength(serverBase) + AdditionalFragmentHeaderLength(serverBase) + normalMsg.Length);
				for (int i = 0; i < normalMsg.Length; i++)
					Assert.Equal(normalMsg[i], recevBuf.Data.Contents[i + recevBuf.GetDataStartPosition()]);

				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Null(recevBuf);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}		
		}

		[Fact]
		public void FragmentedMessageandFragInSingleReadIO()
        {
			InitializeConnection("14014", 1, 1, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			byte[] DataArray = { 0x00, 0x11, 0x03, 0x08, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x01, 0x00, 
				0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x00, 0x09, 0x03, 0x04, 0x00, 0x01, 0x07, 0x08, 0x09 };
			byte[] outputMsg = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

			try
            {
				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(DataArray);

				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = clientChannel.Read(readArgs, out Error error);
				Assert.True(readArgs.ReadRetVal > TransportReturnCode.SUCCESS);

				int bytesRead = readArgs.BytesRead;

				recevBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.NotNull(recevBuf);
				Assert.Equal(outputMsg.Length, recevBuf.Length());
				Assert.Equal(recevBuf.Length() + FirstFragmentHeaderLength(serverBase) + AdditionalFragmentHeaderLength(serverBase), bytesRead);
				for (int i = 0; i < outputMsg.Length; i++)
					Assert.Equal(outputMsg[i], recevBuf.Data.Contents[i + recevBuf.GetDataStartPosition()]);
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}		
		}

		[Fact]
		public void SmallerFragFollowsLarger()
        {
			InitializeConnection("14019", 1, 1, CompressionType.NONE, out IChannel client, out IChannel serverChannel, out IServer server);

			byte[] firstFragHeader = { 0x00, 0x11, 0x03, 0x08, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
			byte[] firstFrag = { 0x00, 0x09, 0x03, 0x04, 0x00, 0x01, 0x07, 0x08, 0x09 };
			byte[] secondFragHeader = { 0x00, 0x0D, 0x03, 0x08, 0x00, 0x00, 0x00, 0x05, 0x00, 0x01, 0x00, 0x01, 0x02 };
			byte[] secondFrag = { 0x00, 0x08, 0x03, 0x04, 0x00, 0x01, 0x03, 0x04 };
			byte[] firstMsg = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
			byte[] secondMsg = { 0x00, 0x01, 0x02, 0x03, 0x04 };

			int cumulativeBytesRead = 0;
			int cumulativeUncompressedbytesRead = 0;

			try
            {
				ChannelBase serverBase = (ChannelBase)serverChannel;
				serverBase.Socket.Send(firstFragHeader);

				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = client.Read(readArgs, out Error error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Equal(firstFragHeader.Length, readArgs.BytesRead);
				cumulativeBytesRead += readArgs.BytesRead;
				cumulativeUncompressedbytesRead += readArgs.UncompressedBytesRead;

				serverBase.Socket.Send(firstFrag);
				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.NotNull(recevBuf);
				Assert.Equal(firstFrag.Length, readArgs.BytesRead);
				Assert.Equal(firstMsg.Length, recevBuf.Length());
				for (int i = 0; i < firstMsg.Length; i++)
					Assert.Equal(firstMsg[i], recevBuf.Data.Contents[i + recevBuf.GetDataStartPosition()]);
				cumulativeBytesRead += readArgs.BytesRead;
				cumulativeUncompressedbytesRead += readArgs.UncompressedBytesRead;
				Assert.Equal(firstMsg.Length + FirstFragmentHeaderLength(serverBase) + AdditionalFragmentHeaderLength(serverBase), cumulativeBytesRead);
				Assert.Equal(firstMsg.Length + FirstFragmentHeaderLength(serverBase) + AdditionalFragmentHeaderLength(serverBase), cumulativeUncompressedbytesRead);

				cumulativeBytesRead = 0;
				cumulativeUncompressedbytesRead = 0;
				serverBase.Socket.Send(secondFragHeader);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Equal(secondFragHeader.Length, readArgs.BytesRead);
				cumulativeBytesRead += readArgs.BytesRead;
				cumulativeUncompressedbytesRead += readArgs.UncompressedBytesRead;

				serverBase.Socket.Send(secondFrag);
				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.NotNull(recevBuf);
				Assert.Equal(secondFrag.Length, readArgs.BytesRead);
				Assert.Equal(secondMsg.Length, recevBuf.Length());
				for (int i = 0; i < secondMsg.Length; i++)
					Assert.Equal(secondMsg[i], recevBuf.Data.Contents[i + recevBuf.GetDataStartPosition()]);
				cumulativeBytesRead += readArgs.BytesRead;
				cumulativeUncompressedbytesRead += readArgs.UncompressedBytesRead;
				Assert.Equal(secondMsg.Length + FirstFragmentHeaderLength(serverBase) + AdditionalFragmentHeaderLength(serverBase), cumulativeBytesRead);
				Assert.Equal(secondMsg.Length + FirstFragmentHeaderLength(serverBase) + AdditionalFragmentHeaderLength(serverBase), cumulativeUncompressedbytesRead);
			} finally
            {
				if (client != null) client.Close(out var error);
				if (serverChannel != null) serverChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}			
		}

		[Fact]
		public void OutOfSequenceFragment()
        {
			InitializeConnection("14015", 1, 1, CompressionType.NONE, out IChannel client, out IChannel serverChannel, out IServer server);

			byte[] outOfSeqFrag = { 0x00, 0x08, 0x03, 0x04, 0x63, 0x07, 0x08, 0x09 };
			byte[] normalMsg = { 0x00, 0x0D, 0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

			try
            {
				ChannelBase serverBase = (ChannelBase)serverChannel;
				serverBase.Socket.Send(outOfSeqFrag);

				Error error;
				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, outOfSeqFrag.Length);

				serverBase.Socket.Send(normalMsg);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(normalMsg.Length, recevBuf.Length() + RipcLengths.HEADER);
			} finally
            {
				if (client != null) client.Close(out var error);
				if (serverChannel != null) serverChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void FragmentLengthExceedsMaxInt()
		{
			InitializeConnection("14016", 1, 1, CompressionType.NONE, out IChannel client, out IChannel serverChannel, out IServer server);

			byte[] exceedFrag = { 0x00, 0x10, 0x03, 0x08, 0xB2, 0xD0, 0x5E, 0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
			byte[] normalMsg = { 0x00, 0x0D, 0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };
			byte[] fragment = { 0x00, 0x08, 0x03, 0x04, 0x01, 0x07, 0x08, 0x09 };

			try
            {
				ChannelBase serverBase = (ChannelBase)serverChannel;
				serverBase.Socket.Send(exceedFrag);

				Error error;
				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, exceedFrag.Length);

				serverBase.Socket.Send(normalMsg);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(normalMsg.Length, recevBuf.Length() + RipcLengths.HEADER);
				for (int i = 0; i < normalMsg.Length - RipcLengths.HEADER; i++)
					Assert.Equal(normalMsg[i + RipcLengths.HEADER], recevBuf.Data.Contents[i + recevBuf.GetDataStartPosition()]);

				serverBase.Socket.Send(fragment);
				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);

				serverBase.Socket.Send(normalMsg);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(normalMsg.Length, recevBuf.Length() + RipcLengths.HEADER);
				for (int i = 0; i < normalMsg.Length - RipcLengths.HEADER; i++)
					Assert.Equal(normalMsg[i + RipcLengths.HEADER], recevBuf.Data.Contents[i + recevBuf.GetDataStartPosition()]);
			} finally
            {
				if (client != null) client.Close(out var error);
				if (serverChannel != null) serverChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}		
		}

		[Fact]
		public void DuplicateFragmentHeader()
        {
			InitializeConnection("14017", 1, 1, CompressionType.NONE, out IChannel client, out IChannel serverChannel, out IServer server);

			byte[] firstFragHeader = { 0x00, 0x0D, 0x03, 0x08, 0x00, 0x00, 0x00, 0x05, 0x00, 0x01, 0x00, 0x01, 0x02 };
			byte[] normalMsg = { 0x00, 0x08, 0x02, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E };
			byte[] duplicateFragHeader = { 0x00, 0x11, 0x03, 0x08, 0x00, 0x00, 0x00, 0x0A, 0x00, 0x01, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06 };
			byte[] fragment = { 0x00, 0x09, 0x03, 0x04, 0x00, 0x01, 0x07, 0x08, 0x09 };
			byte[] reassembledMsg = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

			try
            {
				ChannelBase serverBase = (ChannelBase)serverChannel;
				serverBase.Socket.Send(firstFragHeader);

				Error error;
				ReadArgs readArgs = new ReadArgs();
				ITransportBuffer recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, firstFragHeader.Length);

				serverBase.Socket.Send(normalMsg);
				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(normalMsg.Length, recevBuf.Length() + RipcLengths.HEADER);
				for (int i = 0; i < normalMsg.Length - RipcLengths.HEADER; i++)
					Assert.Equal(normalMsg[i + RipcLengths.HEADER], recevBuf.Data.Contents[i + recevBuf.GetDataStartPosition()]);

				serverBase.Socket.Send(duplicateFragHeader);
				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, duplicateFragHeader.Length);

				serverBase.Socket.Send(fragment);
				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(reassembledMsg.Length, recevBuf.Length());
				for (int i = 0; i < reassembledMsg.Length; i++)
					Assert.Equal(reassembledMsg[i], recevBuf.Data.Contents[i + recevBuf.GetDataStartPosition()]);
			} finally
            {
				if (client != null) client.Close(out var error);
				if (serverChannel != null) serverChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void TwoFragmentedMessagesOfSameSizeAtSameTime()
        {
			InitializeConnection("14018", 1, 1, CompressionType.NONE, out IChannel client, out IChannel serverChannel, out IServer server);

			// setup buffers
			byte[] firstFragHeader = new byte[6001];
			byte[] secondFragHeader = new byte[6001];

			byte[] firstHeader = { 0x17, 0x71, 0x03, 0x08, 0x00, 0x00, 0x1F, 0x40, 0x00, 0x01 };
			byte[] secondHeader = { 0x17, 0x71, 0x03, 0x08, 0x00, 0x00, 0x1F, 0x40, 0x00, 0x20 };

			Buffer.BlockCopy(firstHeader, 0, firstFragHeader, 0, firstHeader.Length);
			Buffer.BlockCopy(secondHeader, 0, secondFragHeader, 0, secondHeader.Length);

			try
            {
				for (int i = 10; i < 6001; i++)
				{
					firstFragHeader[i] = (byte)i;
					secondFragHeader[i] = (byte)(i + 32);
				}

				byte[] firstFrag = new byte[2015];
				byte[] secondFrag = new byte[2015];
				byte[] firstFragH = { 0x07, 0xDF, 0x03, 0x04, 0x00, 0x01 };
				byte[] secondFragH = { 0x07, 0xDF, 0x03, 0x04, 0x00, 0x20 };

				Buffer.BlockCopy(firstFragH, 0, firstFrag, 0, firstFragH.Length);
				Buffer.BlockCopy(secondFragH, 0, secondFrag, 0, secondFragH.Length);

				for (int i = 10; i < 2015; i++)
				{
					firstFrag[i] = (byte)i;
					secondFrag[i] = (byte)(i + 32);
				}
				byte[] normalMsg = { 0x00, 0x0D, 0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

				Error error;
				ReadArgs readArgs = new ReadArgs();

				ChannelBase serverBase = (ChannelBase)serverChannel;
				serverBase.Socket.Send(firstFragHeader);

				ITransportBuffer recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, firstFragHeader.Length);

				serverBase.Socket.Send(secondFragHeader);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, secondFragHeader.Length);

				serverBase.Socket.Send(normalMsg);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, normalMsg.Length);
				Assert.Equal(normalMsg.Length, recevBuf.Length() + RipcLengths.HEADER);
				for (int i = 0; i < normalMsg.Length - RipcLengths.HEADER; i++)
					Assert.Equal(normalMsg[i + RipcLengths.HEADER], recevBuf.Data.Contents[recevBuf.GetDataStartPosition() + i]);

				serverBase.Socket.Send(firstFrag);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, firstFrag.Length);
				Assert.Equal(firstFrag.Length + firstFragHeader.Length, recevBuf.Length() + FirstFragmentHeaderLength(serverBase) + AdditionalFragmentHeaderLength(serverBase));
				for (int i = 0; i < 6001 - FirstFragmentHeaderLength(serverBase); i++)
					Assert.Equal(firstFragHeader[i + FirstFragmentHeaderLength(serverBase)], recevBuf.Data.Contents[recevBuf.GetDataStartPosition() + i]);
				for (int i = 0; i < 2015 - AdditionalFragmentHeaderLength(serverBase); i++)
					Assert.Equal(firstFrag[i + AdditionalFragmentHeaderLength(serverBase)],
						recevBuf.Data.Contents[recevBuf.GetDataStartPosition() + i + 6001 - FirstFragmentHeaderLength(serverBase)]);

				serverBase.Socket.Send(secondFrag);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, secondFrag.Length);
				Assert.Equal(secondFrag.Length + secondFragHeader.Length, recevBuf.Length() + FirstFragmentHeaderLength(serverBase) + AdditionalFragmentHeaderLength(serverBase));
				for (int i = 0; i < 6001 - FirstFragmentHeaderLength(serverBase); i++)
					Assert.Equal(secondFragHeader[i + FirstFragmentHeaderLength(serverBase)], recevBuf.Data.Contents[recevBuf.GetDataStartPosition() + i]);
				for (int i = 0; i < 2015 - AdditionalFragmentHeaderLength(serverBase); i++)
					Assert.Equal(secondFrag[i + AdditionalFragmentHeaderLength(serverBase)],
						recevBuf.Data.Contents[recevBuf.GetDataStartPosition() + i + 6001 - FirstFragmentHeaderLength(serverBase)]);
			} finally
            {
				if (client != null) client.Close(out var error);
				if (serverChannel != null) serverChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}		
		}

		[Fact]
		public void NormalMessageFollowsFragmentedMessage()
        {
			InitializeConnection("14020", 1, 1, CompressionType.NONE, out IChannel client, out IChannel serverChannel, out IServer server);

			// setup buffers
			byte[] firstFragHeader = new byte[6001];
			byte[] firstHeader = { 0x17, 0x71, 0x03, 0x08, 0x00, 0x00, 0x1F, 0x40, 0x00, 0x01 };

			try
            {
				Buffer.BlockCopy(firstHeader, 0, firstFragHeader, 0, firstHeader.Length);

				for (int i = 10; i < 6001; i++)
				{
					firstFragHeader[i] = (byte)i;
				}

				byte[] firstFrag = new byte[2015];
				byte[] firstFragH = { 0x07, 0xDF, 0x03, 0x04, 0x00, 0x01 };

				Buffer.BlockCopy(firstFragH, 0, firstFrag, 0, firstFragH.Length);

				for (int i = 10; i < 2015; i++)
				{
					firstFrag[i] = (byte)i;
				}
				byte[] normalMsg = { 0x00, 0x0D, 0x02, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09 };

				Error error;
				ReadArgs readArgs = new ReadArgs();

				ChannelBase serverBase = (ChannelBase)serverChannel;

				serverBase.Socket.Send(normalMsg);

				ITransportBuffer recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, normalMsg.Length);
				Assert.Equal(normalMsg.Length, recevBuf.Length() + RipcLengths.HEADER);
				for (int i = 0; i < normalMsg.Length - RipcLengths.HEADER; i++)
					Assert.Equal(normalMsg[i + RipcLengths.HEADER], recevBuf.Data.Contents[recevBuf.GetDataStartPosition() + i]);


				serverBase.Socket.Send(firstFragHeader);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, firstFragHeader.Length);

				serverBase.Socket.Send(normalMsg);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, normalMsg.Length);
				Assert.Equal(normalMsg.Length, recevBuf.Length() + RipcLengths.HEADER);
				for (int i = 0; i < normalMsg.Length - RipcLengths.HEADER; i++)
					Assert.Equal(normalMsg[i + RipcLengths.HEADER], recevBuf.Data.Contents[recevBuf.GetDataStartPosition() + i]);

				serverBase.Socket.Send(firstFrag);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, firstFrag.Length);
				Assert.Equal(firstFrag.Length + firstFragHeader.Length, recevBuf.Length() + FirstFragmentHeaderLength(serverBase) + AdditionalFragmentHeaderLength(serverBase));
				for (int i = 0; i < 6001 - FirstFragmentHeaderLength(serverBase); i++)
					Assert.Equal(firstFragHeader[i + FirstFragmentHeaderLength(serverBase)], recevBuf.Data.Contents[recevBuf.GetDataStartPosition() + i]);
				for (int i = 0; i < 2015 - AdditionalFragmentHeaderLength(serverBase); i++)
					Assert.Equal(firstFrag[i + AdditionalFragmentHeaderLength(serverBase)],
						recevBuf.Data.Contents[recevBuf.GetDataStartPosition() + i + 6001 - FirstFragmentHeaderLength(serverBase)]);

				serverBase.Socket.Send(normalMsg);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(readArgs.BytesRead, normalMsg.Length);
				Assert.Equal(normalMsg.Length, recevBuf.Length() + RipcLengths.HEADER);
				for (int i = 0; i < normalMsg.Length - RipcLengths.HEADER; i++)
					Assert.Equal(normalMsg[i + RipcLengths.HEADER], recevBuf.Data.Contents[recevBuf.GetDataStartPosition() + i]);

				recevBuf = client.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.READ_WOULD_BLOCK, readArgs.ReadRetVal);
			} finally
            {
				if (client != null) client.Close(out var error);
				if (serverChannel != null) serverChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}		
		}

		#endregion

		#region Write - Flush - Ping Testcases

		[Fact]
		public void BasicWFPTestCase1()
        {
			String testData = "basicWFPTestCase1";

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(testData.Length + 3, testData.Length + 3), ChannelState.ACTIVE, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			writeArgs.Priority = WritePriorities.HIGH;

			
			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			TransportBuffer transBuf = new TransportBuffer(testData.Length + 3);
			transBuf.SetDataPosition(3);
			byteBuf.Flip();
			transBuf.Data.Put(byteBuf);
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			Assert.True(channel.Write(transBuf, writeArgs, out Error writeError) == 0);
			Assert.Equal(testData.Length + 3, writeArgs.BytesWritten);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase2()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase2";
			String partialTestData = "WFPTestCase2";

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(5 + 3, 5 + 3), ChannelState.ACTIVE, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			writeArgs.Priority = WritePriorities.HIGH;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			TransportBuffer transBuf = new TransportBuffer(testData.Length + 3);
			transBuf.SetDataPosition(3);
			byteBuf.Flip();
			transBuf.Data.Put(byteBuf);
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			int partialBufLen = partialTestData.Length;
			ByteBuffer partialByteBuf = new ByteBuffer(partialBufLen);
			partialByteBuf.Put(Encoding.ASCII.GetBytes(partialTestData));
			
			Assert.True((int)channel.Write(transBuf, writeArgs, out Error writeError) == testData.Length - 5);
			Assert.Equal(5 + 3, writeArgs.BytesWritten);
			Assert.Equal(partialBufLen, channel.m_totalBytesQueued);
			TransportBuffer partBuf = new TransportBuffer(partialBufLen);
			partBuf.Data.Put(partialByteBuf);
			partBuf.SetDataPosition(0);
			Assert.Equal(Encoding.ASCII.GetString(channel.m_writeBufferArray[0].Data.Contents, 
				channel.m_writeBufferArray[0].Data.Position, 
				channel.m_writeBufferArray[0].Data.Limit - channel.m_writeBufferArray[0].Data.Position), 
				partialTestData);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase3()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase3";
			String partialTestData = "WFPTestCase3";

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(5 + 3, 5 + 3), ChannelState.ACTIVE, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			writeArgs.Priority = WritePriorities.MEDIUM;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			TransportBuffer transBuf = new TransportBuffer(testData.Length + 3);
			transBuf.SetDataPosition(3);
			byteBuf.Flip();
			transBuf.Data.Put(byteBuf);
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			int partialBufLen = partialTestData.Length;
			ByteBuffer partialByteBuf = new ByteBuffer(partialBufLen);
			partialByteBuf.Put(Encoding.ASCII.GetBytes(partialTestData));

			Assert.True((int)channel.Write(transBuf, writeArgs, out Error writeError) == testData.Length - 5);
			Assert.Equal(5 + 3, writeArgs.BytesWritten);
			Assert.Equal(partialBufLen, channel.m_totalBytesQueued);
			TransportBuffer partBuf = new TransportBuffer(partialBufLen);
			partBuf.Data.Put(partialByteBuf);
			partBuf.SetDataPosition(0);
			Assert.Equal(Encoding.ASCII.GetString(channel.m_writeBufferArray[0].Data.Contents, channel.m_writeBufferArray[0].Data.Position, channel.m_writeBufferArray[0].Data.Limit - channel.m_writeBufferArray[0].Data.Position),
				partialTestData);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase4()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase4";
			String partialTestData = "WFPTestCase4";

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(5 + 3, 5 + 3), ChannelState.ACTIVE, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			writeArgs.Priority = WritePriorities.LOW;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			TransportBuffer transBuf = new TransportBuffer(testData.Length + 3);
			transBuf.SetDataPosition(3);
			byteBuf.Flip();
			transBuf.Data.Put(byteBuf);
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			int partialBufLen = partialTestData.Length;
			ByteBuffer partialByteBuf = new ByteBuffer(partialBufLen);
			partialByteBuf.Put(Encoding.ASCII.GetBytes(partialTestData));

			Assert.True((int)channel.Write(transBuf, writeArgs, out Error writeError) == testData.Length - 5);
			Assert.Equal(5 + 3, writeArgs.BytesWritten);
			Assert.Equal(partialBufLen, channel.m_totalBytesQueued);
			TransportBuffer partBuf = new TransportBuffer(partialBufLen);
			partBuf.Data.Put(partialByteBuf);
			partBuf.SetDataPosition(0);
			Assert.Equal(Encoding.ASCII.GetString(channel.m_writeBufferArray[0].Data.Contents, channel.m_writeBufferArray[0].Data.Position, channel.m_writeBufferArray[0].Data.Limit - channel.m_writeBufferArray[0].Data.Position),
				partialTestData);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase5()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase5";

			TransportBuffer transBufferForWriting = new TransportBuffer(19);
			transBufferForWriting.IsReadMode = false;
			transBufferForWriting.IsOwnedByApp = true;
			transBufferForWriting.InPool = true;

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(17, 17), ChannelState.ACTIVE, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			channel.m_totalBytesQueued = 19;
			channel.m_isFlushOrderPending = true;
			channel.m_writeBufferArray[0] = transBufferForWriting;
			channel.m_writeArrayMaxPosition = 1;

			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			writeArgs.Priority = WritePriorities.HIGH;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(testData.Length + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.Data = byteBuf;
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			Assert.True((int)channel.Write(transBuf, writeArgs, out Error writeError) == 19);
			Assert.Equal(transBuf, channel.m_highPriorityQueue._tail);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase6()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase6";

			TransportBuffer transBufferForWriting = new TransportBuffer(19);
			transBufferForWriting.IsReadMode = false;
			transBufferForWriting.IsOwnedByApp = true;
			transBufferForWriting.InPool = true;

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(17, 17), ChannelState.ACTIVE, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			channel.m_totalBytesQueued = 19;
			channel.m_isFlushOrderPending = true;
			channel.m_writeBufferArray[0] = transBufferForWriting;
			channel.m_writeArrayMaxPosition = 1;

			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			writeArgs.Priority = WritePriorities.MEDIUM;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(testData.Length + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.Data = byteBuf;
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			Assert.True((int)channel.Write(transBuf, writeArgs, out Error writeError) == 19);
			Assert.Equal(transBuf, channel.m_mediumPriorityQueue._tail);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase7()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase7";

			TransportBuffer transBufferForWriting = new TransportBuffer(19);
			transBufferForWriting.IsReadMode = false;
			transBufferForWriting.IsOwnedByApp = true;
			transBufferForWriting.InPool = true;

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(17, 17), ChannelState.ACTIVE, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			channel.m_totalBytesQueued = 19;
			channel.m_isFlushOrderPending = true;
			channel.m_writeBufferArray[0] = transBufferForWriting;
			channel.m_writeArrayMaxPosition = 1;

			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			writeArgs.Priority = WritePriorities.LOW;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(testData.Length + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.Data = byteBuf;
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			Assert.True((int)channel.Write(transBuf, writeArgs, out Error writeError) == 19);
			Assert.Equal(transBuf, channel.m_lowPriorityQueue._tail);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase8()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase8";

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(5, 5), ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			channel.m_totalBytesQueued = 0;

			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeArgs.Priority = WritePriorities.HIGH;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(testData.Length + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.Data = byteBuf;
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			Assert.True((int)channel.Write(transBuf, writeArgs, out Error writeError) > 0);
			Assert.Equal(transBuf, channel.m_highPriorityQueue._tail);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase9()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase9";

			TransportBuffer transBufferForWriting = new TransportBuffer(19);
			transBufferForWriting.IsReadMode = false;
			transBufferForWriting.IsOwnedByApp = true;
			transBufferForWriting.InPool = true;

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(17, 17), ChannelState.ACTIVE, 10, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			channel.m_totalBytesQueued = 19;
			channel.m_isFlushOrderPending = true;
			channel.m_writeBufferArray[0] = transBufferForWriting;
			channel.m_writeArrayMaxPosition = 1;

			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeArgs.Priority = WritePriorities.HIGH;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(testData.Length + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.Data = byteBuf;
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			Assert.True(channel.Write(transBuf, writeArgs, out Error writeError) >= TransportReturnCode.SUCCESS);
			Assert.Equal(transBuf, channel.m_highPriorityQueue._tail);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase10()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase10";

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(5, 5), ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			channel.m_totalBytesQueued = 0;

			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeArgs.Priority = WritePriorities.MEDIUM;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(testData.Length + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.Data = byteBuf;
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			Assert.True((int)channel.Write(transBuf, writeArgs, out Error writeError) > 0);
			Assert.Equal(transBuf, channel.m_mediumPriorityQueue._tail);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase11()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase11";

			TransportBuffer transBufferForWriting = new TransportBuffer(19);
			transBufferForWriting.IsReadMode = false;
			transBufferForWriting.IsOwnedByApp = true;
			transBufferForWriting.InPool = true;

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(17, 17), ChannelState.ACTIVE, 10, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			channel.m_totalBytesQueued = 19;
			channel.m_isFlushOrderPending = true;
			channel.m_writeBufferArray[0] = transBufferForWriting;
			channel.m_writeArrayMaxPosition = 1;

			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeArgs.Priority = WritePriorities.MEDIUM;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(testData.Length + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.Data = byteBuf;
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			Assert.True(channel.Write(transBuf, writeArgs, out Error writeError) >= TransportReturnCode.SUCCESS);
			Assert.Equal(transBuf, channel.m_mediumPriorityQueue._tail);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase12()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase12";

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(5, 5), ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			channel.m_totalBytesQueued = 0;

			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeArgs.Priority = WritePriorities.LOW;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(testData.Length + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.Data = byteBuf;
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			Assert.True((int)channel.Write(transBuf, writeArgs, out Error writeError) > 0);
			Assert.Equal(transBuf, channel.m_lowPriorityQueue._tail);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase13()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			String testData = "basicWFPTestCase13";

			TransportBuffer transBufferForWriting = new TransportBuffer(19);
			transBufferForWriting.IsReadMode = false;
			transBufferForWriting.IsOwnedByApp = true;
			transBufferForWriting.InPool = true;

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(17, 17), ChannelState.ACTIVE, 10, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;
			channel.m_totalBytesQueued = 19;
			channel.m_isFlushOrderPending = true;
			channel.m_writeBufferArray[0] = transBufferForWriting;
			channel.m_writeArrayMaxPosition = 1;

			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeArgs.Priority = WritePriorities.LOW;

			ByteBuffer byteBuf = new ByteBuffer(testData.Length);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(testData.Length + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.Data = byteBuf;
			transBuf.IsReadMode = false;
			transBuf.IsOwnedByApp = true;
			transBuf.InPool = true;

			Assert.True(channel.Write(transBuf, writeArgs, out Error writeError) >= TransportReturnCode.SUCCESS);
			Assert.Equal(transBuf, channel.m_lowPriorityQueue._tail);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase14()
        {
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			string testDataHigh = "basicWFPTestCase14High";
			string testDataMedium = "basicWFPTestCase14Medium";
			string testDataLow = "basicWFPTestCase14Low";
			int bufLenHigh = testDataHigh.Length;
			int bufLenMedium = testDataMedium.Length;
			int bufLenLow = testDataLow.Length;
			ByteBuffer byteBufHigh = new ByteBuffer(bufLenHigh);
			byteBufHigh.Put(Encoding.ASCII.GetBytes(testDataHigh));
			ByteBuffer byteBufMedium = new ByteBuffer(bufLenMedium);
			byteBufMedium.Put(Encoding.ASCII.GetBytes(testDataMedium));
			ByteBuffer byteBufLow = new ByteBuffer(bufLenLow);
			byteBufLow.Put(Encoding.ASCII.GetBytes(testDataLow));
			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			TransportBuffer[] transBufHigh = new TransportBuffer[4];
			TransportBuffer[] transBufMedium = new TransportBuffer[4];
			TransportBuffer[] transBufLow = new TransportBuffer[4];
			
			for (int i = 0; i < 4; i++)
			{
				transBufHigh[i] = new TransportBuffer(bufLenHigh + 3);

				transBufHigh[i].IsReadMode = false;
				transBufHigh[i].SetDataPosition(3);
				byteBufHigh.Flip();
				transBufHigh[i].Data.Put(byteBufHigh);
				transBufHigh[i].InPool = true;
				transBufHigh[i].IsOwnedByApp = true;

				transBufMedium[i] = new TransportBuffer(bufLenMedium + 3);

				transBufMedium[i].IsReadMode = false;
				transBufMedium[i].SetDataPosition(3);
				byteBufMedium.Flip();
				transBufMedium[i].Data.Put(byteBufMedium);
				transBufMedium[i].InPool = true;
				transBufMedium[i].IsOwnedByApp = true;

				transBufLow[i] = new TransportBuffer(bufLenLow + 3);

				transBufLow[i].IsReadMode = false;
				transBufLow[i].SetDataPosition(3);
				byteBufLow.Flip();
				transBufLow[i].Data.Put(byteBufLow);
				transBufLow[i].InPool = true;
				transBufLow[i].IsOwnedByApp = true;
			}

			MockChannel mockSocketChannel = new MockChannel(0, 0);
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			int cumulativeBytesQueued = 0;
			TransportReturnCode writeReturnVal = 0;
			for (int i = 0; i < 4; i++)
			{
				// queue several buffers by calling the write method several times with no write flags
				cumulativeBytesQueued += bufLenHigh + 3;
				writeArgs.Priority = WritePriorities.HIGH;
				writeArgs.Flags = WriteFlags.NO_FLAGS;
				channel.Write(transBufHigh[i], writeArgs, out error);
				// bytesWritten should be scktBufHigh.getLength
				Assert.True(writeArgs.BytesWritten == transBufHigh[i].Length() + 3);
				cumulativeBytesQueued += bufLenMedium + 3;
				writeArgs.Priority = WritePriorities.MEDIUM;
				writeArgs.Flags = WriteFlags.NO_FLAGS;
				channel.Write(transBufMedium[i], writeArgs, out error);
				// bytesWritten should be scktBufMedium.getLength
				Assert.True(writeArgs.BytesWritten == transBufMedium[i].Length() + 3);
				cumulativeBytesQueued += bufLenLow + 3;
				writeArgs.Priority = WritePriorities.LOW;
				writeArgs.Flags = WriteFlags.NO_FLAGS;
				writeReturnVal = channel.Write(transBufLow[i], writeArgs, out error);
				// bytesWritten should be scktBufLow.getLength
				Assert.True(writeArgs.BytesWritten == transBufLow[i].Length() + 3);
			}

			Assert.True(channel.m_totalBytesQueued == cumulativeBytesQueued);
			mockSocketChannel.SendReturnValue = cumulativeBytesQueued;
			mockSocketChannel.SendListReturnValue = cumulativeBytesQueued;
			Assert.True(channel.Flush(out error) == TransportReturnCode.SUCCESS);

			// _totalBytesQueued in RsslSocketChannel should be 0
			Assert.True(channel.m_totalBytesQueued == 0);

			// priority queues in RsslSocketChannel should be empty
			Assert.Null(channel.m_highPriorityQueue._tail);
			Assert.Null(channel.m_mediumPriorityQueue._tail);
			Assert.Null(channel.m_lowPriorityQueue._tail);
			Console.WriteLine(channel.m_writeBufferArray[0]);

			// verify that the _gatherWriteArray's order matches expected flush order.
			Assert.Equal(transBufHigh[0], channel.m_writeBufferArray[0]); // H
			Assert.Equal(transBufMedium[0], channel.m_writeBufferArray[1]); // M
			Assert.Equal(transBufHigh[1], channel.m_writeBufferArray[2]); // H
			Assert.Equal(transBufLow[0], channel.m_writeBufferArray[3]); // L
			Assert.Equal(transBufHigh[2], channel.m_writeBufferArray[4]); // H
			Assert.Equal(transBufMedium[1], channel.m_writeBufferArray[5]); // M
			Assert.Equal(transBufHigh[3], channel.m_writeBufferArray[6]); // H
			Assert.Equal(transBufMedium[2], channel.m_writeBufferArray[7]); // M
			Assert.Equal(transBufLow[1], channel.m_writeBufferArray[8]); // L
			Assert.Equal(transBufMedium[3], channel.m_writeBufferArray[9]); // M
			Assert.Equal(transBufLow[2], channel.m_writeBufferArray[10]); // L
			Assert.Equal(transBufLow[3], channel.m_writeBufferArray[11]); // L

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase15()
        {
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			String testDataHigh = "basicWFPTestCase15High";
			String testDataMedium = "basicWFPTestCase15Medium";
			String testDataLow = "basicWFPTestCase15Low";
			int bufLenHigh = testDataHigh.Length;
			int bufLenMedium = testDataMedium.Length;
			int bufLenLow = testDataLow.Length;
			ByteBuffer byteBufHigh1 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh2 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh3 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufMedium1 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium2 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufLow = new ByteBuffer(bufLenLow);
			
			byteBufHigh1.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh2.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh3.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufMedium1.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium2.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufLow.Put(Encoding.ASCII.GetBytes(testDataLow));
			String partialTestData = "Case15Low";
			int partialBufLen = partialTestData.Length;
			ByteBuffer partialByteBuf = new ByteBuffer(partialBufLen);
			partialByteBuf.Put(Encoding.ASCII.GetBytes(partialTestData));
			
			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			MockChannel mockSocketChannel = new MockChannel(0, 0);
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			TransportBuffer transBufHigh1 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh1.IsReadMode = false;
            transBufHigh1.SetDataPosition(3);
            byteBufHigh1.Flip();
            transBufHigh1.Data.Put(byteBufHigh1);
			transBufHigh1.IsOwnedByApp = true;
			transBufHigh1.InPool = true;

			TransportBuffer transBufHigh2 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh2.IsReadMode = false;
			transBufHigh2.SetDataPosition(3);
			byteBufHigh2.Flip();
            transBufHigh2.Data.Put(byteBufHigh2);
			transBufHigh2.IsOwnedByApp = true;
			transBufHigh2.InPool = true;

			TransportBuffer transBufHigh3 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh3.IsReadMode = false;
			transBufHigh3.SetDataPosition(3);
            byteBufHigh3.Flip();
            transBufHigh3.Data.Put(byteBufHigh3);
			transBufHigh3.IsOwnedByApp = true;
			transBufHigh3.InPool = true;

			TransportBuffer transBufMedium1 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium1.IsReadMode = false;
			transBufMedium1.SetDataPosition(3);
            byteBufMedium1.Flip();
            transBufMedium1.Data.Put(byteBufMedium1);
			transBufMedium1.IsOwnedByApp = true;
			transBufMedium1.InPool = true;

			TransportBuffer transBufMedium2 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium2.IsReadMode = false;
			transBufMedium2.SetDataPosition(3);
            byteBufMedium2.Flip();
            transBufMedium2.Data.Put(byteBufMedium2);
			transBufMedium2.IsOwnedByApp = true;
			transBufMedium2.InPool = true;

			TransportBuffer transBufLow = new TransportBuffer(bufLenLow + 3);
			transBufLow.IsReadMode = false;
			transBufLow.SetDataPosition(3);
            byteBufLow.Flip();
            transBufLow.Data.Put(byteBufLow);
			transBufLow.IsOwnedByApp = true;
			transBufLow.InPool = true;

            // set _totalBytesQueued to 0 for no buffers queued
            channel.m_totalBytesQueued = 0;

			int cumulativeBytesQueued = 0;
			TransportReturnCode writeReturnVal = 0;

			// queue several buffers by calling the write method several times with no write flags
			cumulativeBytesQueued += bufLenHigh + 3;
            writeArgs.Priority = WritePriorities.HIGH;
            writeArgs.Flags = WriteFlags.NO_FLAGS;
            writeReturnVal = channel.Write(transBufHigh1, writeArgs, out error);
            // write return value should be cumulative bytes queued
            Assert.True((int)(int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh.ge.Length
			Assert.True(writeArgs.BytesWritten == transBufHigh1.Length() + 3);
			cumulativeBytesQueued += bufLenMedium + 3;
            writeArgs.Priority = WritePriorities.MEDIUM;
            writeArgs.Flags = WriteFlags.NO_FLAGS;
            writeReturnVal = channel.Write(transBufMedium1, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)(int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium.ge.Length
			Assert.True(writeArgs.BytesWritten == transBufMedium1.Length() + 3);
			cumulativeBytesQueued += bufLenHigh + 3;
            writeArgs.Priority = WritePriorities.HIGH;
            writeArgs.Flags = WriteFlags.NO_FLAGS;
            writeReturnVal = channel.Write(transBufHigh2, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)(int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh.ge.Length
			Assert.True(writeArgs.BytesWritten == transBufHigh2.Length() + 3);
			cumulativeBytesQueued += bufLenLow + 3;
            writeArgs.Priority = WritePriorities.LOW;
            writeArgs.Flags = WriteFlags.NO_FLAGS;
            writeReturnVal = channel.Write(transBufLow, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)(int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufLow.ge.Length
			Assert.True(writeArgs.BytesWritten == transBufLow.Length() + 3);
			cumulativeBytesQueued += bufLenHigh + 3;
            writeArgs.Priority = WritePriorities.HIGH;
            writeArgs.Flags = WriteFlags.NO_FLAGS;
            writeReturnVal = channel.Write(transBufHigh3, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)(int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh.ge.Length
			Assert.True(writeArgs.BytesWritten == transBufHigh3.Length() + 3);
			cumulativeBytesQueued += bufLenMedium + 3;
            writeArgs.Priority = WritePriorities.MEDIUM;
            writeArgs.Flags = WriteFlags.NO_FLAGS;
            writeReturnVal = channel.Write(transBufMedium2, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)(int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium.ge.Length
			Assert.True(writeArgs.BytesWritten == transBufMedium2.Length() + 3);

			// m_totalBytesQueued in RsslSocketChannel should be cumulative bytes queued
			Assert.True(channel.m_totalBytesQueued == cumulativeBytesQueued);

			mockSocketChannel.SendReturnValue = 92;
			mockSocketChannel.SendListReturnValue = 92;

			// flush should return bytes remaining to be sent
			Assert.True((int)channel.Flush(out error) == (cumulativeBytesQueued - 92));

			// _totalBytesQueued in RsslSocketChannel should be bytes remaining to be sent
			Assert.True(channel.m_totalBytesQueued == (cumulativeBytesQueued - 92));

			// _isFlushPending in RsslSocketChannel should be true
			Assert.True(channel.m_isFlushOrderPending == true);

			// _writeArrayPosition in RsslSocketChannel should be 3
			Assert.True(channel.m_writeArrayPosition == 3);

			// create SocketBuffer to compare to partial buffer in queue
			TransportBuffer partialBuf = new TransportBuffer(partialBufLen);
			partialBuf.Data.Put(partialByteBuf);

            // _gatheringWriteArray in ChannelBase should contain remaining bytes sent
            Assert.Equal(Encoding.ASCII.GetString(channel.m_writeBufferArray[channel.m_writeArrayPosition].Data.Contents, 
				channel.m_writeBufferArray[channel.m_writeArrayPosition].Data.Position, 
				channel.m_writeBufferArray[channel.m_writeArrayPosition].Data.Limit - channel.m_writeBufferArray[channel.m_writeArrayPosition].Data.Position),
				partialTestData);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase16()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			String testDataHigh = "basicWFPTestCase16High";
			String testDataMedium = "basicWFPTestCase16Medium";
			String testDataLow = "basicWFPTestCase16Low";
			int bufLenHigh = testDataHigh.Length;
			int bufLenMedium = testDataMedium.Length;
			int bufLenLow = testDataLow.Length;
			ByteBuffer byteBufHigh1 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh2 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh3 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh4 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh5 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh6 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh7 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh8 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh9 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh10 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh11 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh12 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh13 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh14 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh15 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh16 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh17 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh18 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh19 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh20 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh21 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh22 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh23 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufHigh24 = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufMedium1 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium2 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium3 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium4 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium5 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium6 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium7 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium8 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium9 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium10 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium11 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium12 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium13 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium14 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium15 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufMedium16 = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufLow1 = new ByteBuffer(bufLenLow);
			ByteBuffer byteBufLow2 = new ByteBuffer(bufLenLow);
			ByteBuffer byteBufLow3 = new ByteBuffer(bufLenLow);
			ByteBuffer byteBufLow4 = new ByteBuffer(bufLenLow);
			ByteBuffer byteBufLow5 = new ByteBuffer(bufLenLow);
			ByteBuffer byteBufLow6 = new ByteBuffer(bufLenLow);
			ByteBuffer byteBufLow7 = new ByteBuffer(bufLenLow);
			ByteBuffer byteBufLow8 = new ByteBuffer(bufLenLow);
			byteBufHigh1.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh2.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh3.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh4.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh5.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh6.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh7.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh8.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh9.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh10.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh11.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh12.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh13.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh14.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh15.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh16.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh17.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh18.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh19.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh20.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh21.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh22.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh23.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufHigh24.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufMedium1.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium2.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium3.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium4.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium5.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium6.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium7.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium8.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium9.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium10.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium11.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium12.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium13.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium14.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium15.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufMedium16.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufLow1.Put(Encoding.ASCII.GetBytes(testDataLow));
			byteBufLow2.Put(Encoding.ASCII.GetBytes(testDataLow));
			byteBufLow3.Put(Encoding.ASCII.GetBytes(testDataLow));
			byteBufLow4.Put(Encoding.ASCII.GetBytes(testDataLow));
			byteBufLow5.Put(Encoding.ASCII.GetBytes(testDataLow));
			byteBufLow6.Put(Encoding.ASCII.GetBytes(testDataLow));
			byteBufLow7.Put(Encoding.ASCII.GetBytes(testDataLow));
			byteBufLow8.Put(Encoding.ASCII.GetBytes(testDataLow));
			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			byte[] testBuffer = new byte[1224];
			int testBufPosition = 0;
			MockChannel mockSocketChannel = new MockChannel(10, 10);
			mockSocketChannel.SendListFunc = (buffers, throwExceptionOnSend, sendReturnValue) =>
			{
				int bytesWritten = 92, cumulativeBytes = 0;

				// write 16 bytes to testBuffer for each call
				foreach (var buf in buffers)
				{
					foreach (var b in buf)
					{
						testBuffer[testBufPosition++] = b;
						cumulativeBytes++;
						if (cumulativeBytes == bytesWritten)
						{
							break;
						}
					}
					if (cumulativeBytes == bytesWritten)
					{
						break;
					}
				}

				return (cumulativeBytes < bytesWritten) ? cumulativeBytes : bytesWritten;
			};
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, 5000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			TransportBuffer transBufHigh1 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh1.IsReadMode = false;
			transBufHigh1.SetDataPosition(3);
			byteBufHigh1.Flip();
			transBufHigh1.Data.Put(byteBufHigh1);
			transBufHigh1.InPool = true;
			transBufHigh1.IsOwnedByApp = true;

			TransportBuffer transBufHigh2 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh2.IsReadMode = false;
			transBufHigh2.SetDataPosition(3);
			byteBufHigh2.Flip();
			transBufHigh2.Data.Put(byteBufHigh2);
			transBufHigh2.InPool = true;
			transBufHigh2.IsOwnedByApp = true;

			TransportBuffer transBufHigh3 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh3.IsReadMode = false;
			transBufHigh3.SetDataPosition(3);
			byteBufHigh3.Flip();
			transBufHigh3.Data.Put(byteBufHigh3);
			transBufHigh3.InPool = true;
			transBufHigh3.IsOwnedByApp = true;

			TransportBuffer transBufHigh4 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh4.IsReadMode = false;
			transBufHigh4.SetDataPosition(3);
			byteBufHigh4.Flip();
			transBufHigh4.Data.Put(byteBufHigh4);
			transBufHigh4.InPool = true;
			transBufHigh4.IsOwnedByApp = true;

			TransportBuffer transBufHigh5 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh5.IsReadMode = false;
			transBufHigh5.SetDataPosition(3);
			byteBufHigh5.Flip();
			transBufHigh5.Data.Put(byteBufHigh5);
			transBufHigh5.InPool = true;
			transBufHigh5.IsOwnedByApp = true;

			TransportBuffer transBufHigh6 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh6.IsReadMode = false;
			transBufHigh6.SetDataPosition(3);
			byteBufHigh6.Flip();
			transBufHigh6.Data.Put(byteBufHigh6);
			transBufHigh6.InPool = true;
			transBufHigh6.IsOwnedByApp = true;

			TransportBuffer transBufHigh7 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh7.IsReadMode = false;
			transBufHigh7.SetDataPosition(3);
			byteBufHigh7.Flip();
			transBufHigh7.Data.Put(byteBufHigh7);
			transBufHigh7.InPool = true;
			transBufHigh7.IsOwnedByApp = true;

			TransportBuffer transBufHigh8 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh8.IsReadMode = false;
			transBufHigh8.SetDataPosition(3);
			byteBufHigh8.Flip();
			transBufHigh8.Data.Put(byteBufHigh8);
			transBufHigh8.InPool = true;
			transBufHigh8.IsOwnedByApp = true;

			TransportBuffer transBufHigh9 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh9.IsReadMode = false;
			transBufHigh9.SetDataPosition(3);
			byteBufHigh9.Flip();
			transBufHigh9.Data.Put(byteBufHigh9);
			transBufHigh9.InPool = true;
			transBufHigh9.IsOwnedByApp = true;

			TransportBuffer transBufHigh10 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh10.IsReadMode = false;
			transBufHigh10.SetDataPosition(3);
			byteBufHigh10.Flip();
			transBufHigh10.Data.Put(byteBufHigh10);
			transBufHigh10.InPool = true;
			transBufHigh10.IsOwnedByApp = true;

			TransportBuffer transBufHigh11 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh11.IsReadMode = false;
			transBufHigh11.SetDataPosition(3);
			byteBufHigh11.Flip();
			transBufHigh11.Data.Put(byteBufHigh11);
			transBufHigh11.InPool = true;
			transBufHigh11.IsOwnedByApp = true;

			TransportBuffer transBufHigh12 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh12.IsReadMode = false;
			transBufHigh12.SetDataPosition(3);
			byteBufHigh12.Flip();
			transBufHigh12.Data.Put(byteBufHigh12);
			transBufHigh12.InPool = true;
			transBufHigh12.IsOwnedByApp = true;

			TransportBuffer transBufHigh13 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh13.IsReadMode = false;
			transBufHigh13.SetDataPosition(3);
			byteBufHigh13.Flip();
			transBufHigh13.Data.Put(byteBufHigh13);
			transBufHigh13.InPool = true;
			transBufHigh13.IsOwnedByApp = true;

			TransportBuffer transBufHigh14 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh14.IsReadMode = false;
			transBufHigh14.SetDataPosition(3);
			byteBufHigh14.Flip();
			transBufHigh14.Data.Put(byteBufHigh14);
			transBufHigh14.InPool = true;
			transBufHigh14.IsOwnedByApp = true;

			TransportBuffer transBufHigh15 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh15.IsReadMode = false;
			transBufHigh15.SetDataPosition(3);
			byteBufHigh15.Flip();
			transBufHigh15.Data.Put(byteBufHigh15);
			transBufHigh15.InPool = true;
			transBufHigh15.IsOwnedByApp = true;

			TransportBuffer transBufHigh16 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh16.IsReadMode = false;
			transBufHigh16.SetDataPosition(3);
			byteBufHigh16.Flip();
			transBufHigh16.Data.Put(byteBufHigh16);
			transBufHigh16.InPool = true;
			transBufHigh16.IsOwnedByApp = true;

			TransportBuffer transBufHigh17 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh17.IsReadMode = false;
			transBufHigh17.SetDataPosition(3);
			byteBufHigh17.Flip();
			transBufHigh17.Data.Put(byteBufHigh17);
			transBufHigh17.InPool = true;
			transBufHigh17.IsOwnedByApp = true;

			TransportBuffer transBufHigh18 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh18.IsReadMode = false;
			transBufHigh18.SetDataPosition(3);
			byteBufHigh18.Flip();
			transBufHigh18.Data.Put(byteBufHigh18);
			transBufHigh18.InPool = true;
			transBufHigh18.IsOwnedByApp = true;

			TransportBuffer transBufHigh19 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh19.IsReadMode = false;
			transBufHigh19.SetDataPosition(3);
			byteBufHigh19.Flip();
			transBufHigh19.Data.Put(byteBufHigh19);
			transBufHigh19.InPool = true;
			transBufHigh19.IsOwnedByApp = true;

			TransportBuffer transBufHigh20 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh20.IsReadMode = false;
			transBufHigh20.SetDataPosition(3);
			byteBufHigh20.Flip();
			transBufHigh20.Data.Put(byteBufHigh20);
			transBufHigh20.InPool = true;
			transBufHigh20.IsOwnedByApp = true;

			TransportBuffer transBufHigh21 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh21.IsReadMode = false;
			transBufHigh21.SetDataPosition(3);
			byteBufHigh21.Flip();
			transBufHigh21.Data.Put(byteBufHigh21);
			transBufHigh21.InPool = true;
			transBufHigh21.IsOwnedByApp = true;

			TransportBuffer transBufHigh22 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh22.IsReadMode = false;
			transBufHigh22.SetDataPosition(3);
			byteBufHigh22.Flip();
			transBufHigh22.Data.Put(byteBufHigh22);
			transBufHigh22.InPool = true;
			transBufHigh22.IsOwnedByApp = true;

			TransportBuffer transBufHigh23 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh23.IsReadMode = false;
			transBufHigh23.SetDataPosition(3);
			byteBufHigh23.Flip();
			transBufHigh23.Data.Put(byteBufHigh23);
			transBufHigh23.InPool = true;
			transBufHigh23.IsOwnedByApp = true;

			TransportBuffer transBufHigh24 = new TransportBuffer(bufLenHigh + 3);
			transBufHigh24.IsReadMode = false;
			transBufHigh24.SetDataPosition(3);
			byteBufHigh24.Flip();
			transBufHigh24.Data.Put(byteBufHigh24);
			transBufHigh24.InPool = true;
			transBufHigh24.IsOwnedByApp = true;

			TransportBuffer transBufMedium1 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium1.IsReadMode = false;
			transBufMedium1.SetDataPosition(3);
			byteBufMedium1.Flip();
			transBufMedium1.Data.Put(byteBufMedium1);
			transBufMedium1.InPool = true;
			transBufMedium1.IsOwnedByApp = true;

			TransportBuffer transBufMedium2 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium2.IsReadMode = false;
			transBufMedium2.SetDataPosition(3);
			byteBufMedium2.Flip();
			transBufMedium2.Data.Put(byteBufMedium2);
			transBufMedium2.InPool = true;
			transBufMedium2.IsOwnedByApp = true;

			TransportBuffer transBufMedium3 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium3.IsReadMode = false;
			transBufMedium3.SetDataPosition(3);
			byteBufMedium3.Flip();
			transBufMedium3.Data.Put(byteBufMedium3);
			transBufMedium3.InPool = true;
			transBufMedium3.IsOwnedByApp = true;

			TransportBuffer transBufMedium4 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium4.IsReadMode = false;
			transBufMedium4.SetDataPosition(3);
			byteBufMedium4.Flip();
			transBufMedium4.Data.Put(byteBufMedium4);
			transBufMedium4.InPool = true;
			transBufMedium4.IsOwnedByApp = true;

			TransportBuffer transBufMedium5 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium5.IsReadMode = false;
			transBufMedium5.SetDataPosition(3);
			byteBufMedium5.Flip();
			transBufMedium5.Data.Put(byteBufMedium5);
			transBufMedium5.InPool = true;
			transBufMedium5.IsOwnedByApp = true;

			TransportBuffer transBufMedium6 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium6.IsReadMode = false;
			transBufMedium6.SetDataPosition(3);
			byteBufMedium6.Flip();
			transBufMedium6.Data.Put(byteBufMedium6);
			transBufMedium6.InPool = true;
			transBufMedium6.IsOwnedByApp = true;

			TransportBuffer transBufMedium7 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium7.IsReadMode = false;
			transBufMedium7.SetDataPosition(3);
			byteBufMedium7.Flip();
			transBufMedium7.Data.Put(byteBufMedium7);
			transBufMedium7.InPool = true;
			transBufMedium7.IsOwnedByApp = true;

			TransportBuffer transBufMedium8 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium8.IsReadMode = false;
			transBufMedium8.SetDataPosition(3);
			byteBufMedium8.Flip();
			transBufMedium8.Data.Put(byteBufMedium8);
			transBufMedium8.InPool = true;
			transBufMedium8.IsOwnedByApp = true;

			TransportBuffer transBufMedium9 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium9.SetDataPosition(3);
			byteBufMedium9.Flip();
			transBufMedium9.Data.Put(byteBufMedium9);
			transBufMedium9.IsReadMode = false;
			transBufMedium9.InPool = true;
			transBufMedium9.IsOwnedByApp = true;

			TransportBuffer transBufMedium10 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium10.SetDataPosition(3);
			byteBufMedium10.Flip();
			transBufMedium10.Data.Put(byteBufMedium10);
			transBufMedium10.IsReadMode = false;
			transBufMedium10.InPool = true;
			transBufMedium10.IsOwnedByApp = true;

			TransportBuffer transBufMedium11 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium11.SetDataPosition(3);
			byteBufMedium11.Flip();
			transBufMedium11.Data.Put(byteBufMedium11);
			transBufMedium11.IsReadMode = false;
			transBufMedium11.InPool = true;
			transBufMedium11.IsOwnedByApp = true;

			TransportBuffer transBufMedium12 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium12.SetDataPosition(3);
			byteBufMedium12.Flip();
			transBufMedium12.Data.Put(byteBufMedium12);
			transBufMedium12.IsReadMode = false;
			transBufMedium12.InPool = true;
			transBufMedium12.IsOwnedByApp = true;

			TransportBuffer transBufMedium13 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium13.SetDataPosition(3);
			byteBufMedium13.Flip();
			transBufMedium13.Data.Put(byteBufMedium13);
			transBufMedium13.IsReadMode = false;
			transBufMedium13.InPool = true;
			transBufMedium13.IsOwnedByApp = true;

			TransportBuffer transBufMedium14 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium14.SetDataPosition(3);
			byteBufMedium14.Flip();
			transBufMedium14.Data.Put(byteBufMedium14);
			transBufMedium14.IsReadMode = false;
			transBufMedium14.InPool = true;
			transBufMedium14.IsOwnedByApp = true;

			TransportBuffer transBufMedium15 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium15.SetDataPosition(3);
			byteBufMedium15.Flip();
			transBufMedium15.Data.Put(byteBufMedium15);
			transBufMedium15.IsReadMode = false;
			transBufMedium15.InPool = true;
			transBufMedium15.IsOwnedByApp = true;

			TransportBuffer transBufMedium16 = new TransportBuffer(bufLenMedium + 3);
			transBufMedium16.SetDataPosition(3);
			byteBufMedium16.Flip();
			transBufMedium16.Data.Put(byteBufMedium16);
			transBufMedium16.IsReadMode = false;
			transBufMedium16.InPool = true;
			transBufMedium16.IsOwnedByApp = true;

			TransportBuffer transBufLow1 = new TransportBuffer(bufLenLow + 3);
			transBufLow1.SetDataPosition(3);
			byteBufLow1.Flip();
			transBufLow1.Data.Put(byteBufLow1);
			transBufLow1.IsReadMode = false;
			transBufLow1.InPool = true;
			transBufLow1.IsOwnedByApp = true;

			TransportBuffer transBufLow2 = new TransportBuffer(bufLenLow + 3);
			transBufLow2.SetDataPosition(3);
			byteBufLow2.Flip();
			transBufLow2.Data.Put(byteBufLow2);
			transBufLow2.IsReadMode = false;
			transBufLow2.InPool = true;
			transBufLow2.IsOwnedByApp = true;

			TransportBuffer transBufLow3 = new TransportBuffer(bufLenLow + 3);
			transBufLow3.SetDataPosition(3);
			byteBufLow3.Flip();
			transBufLow3.Data.Put(byteBufLow3);
			transBufLow3.IsReadMode = false;
			transBufLow3.InPool = true;
			transBufLow3.IsOwnedByApp = true;

			TransportBuffer transBufLow4 = new TransportBuffer(bufLenLow + 3);
			transBufLow4.SetDataPosition(3);
			byteBufLow4.Flip();
			transBufLow4.Data.Put(byteBufLow4);
			transBufLow4.IsReadMode = false;
			transBufLow4.InPool = true;
			transBufLow4.IsOwnedByApp = true;

			TransportBuffer transBufLow5 = new TransportBuffer(bufLenLow + 3);
			transBufLow5.SetDataPosition(3);
			byteBufLow5.Flip();
			transBufLow5.Data.Put(byteBufLow5);
			transBufLow5.IsReadMode = false;
			transBufLow5.InPool = true;
			transBufLow5.IsOwnedByApp = true;

			TransportBuffer transBufLow6 = new TransportBuffer(bufLenLow + 3);
			transBufLow6.SetDataPosition(3);
			byteBufLow6.Flip();
			transBufLow6.Data.Put(byteBufLow6);
			transBufLow6.IsReadMode = false;
			transBufLow6.InPool = true;
			transBufLow6.IsOwnedByApp = true;

			TransportBuffer transBufLow7 = new TransportBuffer(bufLenLow + 3);
			transBufLow7.SetDataPosition(3);
			byteBufLow7.Flip();
			transBufLow7.Data.Put(byteBufLow7);
			transBufLow7.IsReadMode = false;
			transBufLow7.InPool = true;
			transBufLow7.IsOwnedByApp = true;

			TransportBuffer transBufLow8 = new TransportBuffer(bufLenLow + 3);
			transBufLow8.SetDataPosition(3);
			byteBufLow8.Flip();
			transBufLow8.Data.Put(byteBufLow8);
			transBufLow8.IsReadMode = false;
			transBufLow8.InPool = true;
			transBufLow8.IsOwnedByApp = true;

			int cumulativeBytesQueued = 0;
			TransportReturnCode writeReturnVal = 0;

			// queue several buffers by calling the write method several times with no write flags
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh1, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh1.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium1, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium1.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh2, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh2.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenLow + 3;
			writeArgs.Priority = WritePriorities.LOW;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufLow1, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufLow1.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenLow + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh3, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh3.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium2, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium2.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh4, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh4.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium3, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium3.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh5, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh5.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenLow + 3;
			writeArgs.Priority = WritePriorities.LOW;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufLow2, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufLow2.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenLow + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh6, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh6.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium4, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium4.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh7, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)(int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh7.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium5, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium5.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh8, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh8.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenLow + 3;
			writeArgs.Priority = WritePriorities.LOW;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufLow3, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufLow3.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenLow + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh9, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh9.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium6, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium6.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh10, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh10.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium7, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium7.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh11, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh11.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenLow + 3;
			writeArgs.Priority = WritePriorities.LOW;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufLow4, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufLow4.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenLow + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh12, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh12.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium8, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium8.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh13, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh13.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium9, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium9.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh14, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh14.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenLow + 3;
			writeArgs.Priority = WritePriorities.LOW;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufLow5, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufLow5.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenLow + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh15, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh15.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium10, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium10.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh16, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh16.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium11, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium11.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh17, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh17.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			
			cumulativeBytesQueued += bufLenLow + 3;
			writeArgs.Priority = WritePriorities.LOW;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufLow6, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufLow6.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenLow + 3);
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh18, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh18.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium12, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium12.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh19, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh19.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium13, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium13.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh20, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh20.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			cumulativeBytesQueued += bufLenLow + 3;
			writeArgs.Priority = WritePriorities.LOW;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufLow7, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufLow7.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenLow + 3);
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh21, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh21.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium14, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium14.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh22, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh22.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium15, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium15.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh23, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh23.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			cumulativeBytesQueued += bufLenLow + 3;
			writeArgs.Priority = WritePriorities.LOW;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufLow8, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufLow8.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenLow + 3);
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufHigh24, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufHigh24.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh+ 3);
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBufMedium16, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued);
			// bytesWritten should be scktBufMedium16.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);

			// _totalBytesQueued in RsslSocketChannel should be cumulative bytes queued
			Assert.True(channel.m_totalBytesQueued == cumulativeBytesQueued);

			while (cumulativeBytesQueued > 0)
			{
				int bytesFlushed = 0;
				TransportReturnCode flushRetVal;

				flushRetVal = channel.Flush(out error);
				bytesFlushed = cumulativeBytesQueued - (int)flushRetVal;

				// _totalBytesQueued in RsslSocketChannel should be bytes remaining to be sent
				Assert.True(channel.m_totalBytesQueued == (cumulativeBytesQueued - bytesFlushed));

				// decrement cumulativeBytesQueued by bytesFlushed
				cumulativeBytesQueued -= bytesFlushed;
			}

			// _totalBytesQueued in RsslSocketChannel should be 0
			Assert.True(channel.m_totalBytesQueued == 0);

			// verify bytes are written in correct order
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 3, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 28, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 55, bufLenHigh));
			Assert.Equal(testDataLow, Encoding.ASCII.GetString(testBuffer, 80, bufLenLow));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 104, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 129, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 156, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 181, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 208, bufLenHigh));
			Assert.Equal(testDataLow, Encoding.ASCII.GetString(testBuffer, 233, bufLenLow));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 257, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 282, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 309, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 334, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 361, bufLenHigh));
			Assert.Equal(testDataLow, Encoding.ASCII.GetString(testBuffer, 386, bufLenLow));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 410, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 435, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 462, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 487, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 514, bufLenHigh));
			Assert.Equal(testDataLow, Encoding.ASCII.GetString(testBuffer, 539, bufLenLow));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 563, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 588, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 615, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 640, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 667, bufLenHigh));
			Assert.Equal(testDataLow, Encoding.ASCII.GetString(testBuffer, 692, bufLenLow));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 716, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 741, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 768, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 793, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 820, bufLenHigh));
			Assert.Equal(testDataLow, Encoding.ASCII.GetString(testBuffer, 845, bufLenLow));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 869, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 894, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 921, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 946, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 973, bufLenHigh));
			Assert.Equal(testDataLow, Encoding.ASCII.GetString(testBuffer, 998, bufLenLow));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 1022, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 1047, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 1074, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 1099, bufLenMedium));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 1126, bufLenHigh));
			Assert.Equal(testDataLow, Encoding.ASCII.GetString(testBuffer, 1151, bufLenLow));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 1175, bufLenHigh));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 1200, bufLenMedium));

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase17()
        {
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			MockChannel mockSocketChannel = new MockChannel(3, 3);
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			Assert.True(channel.Ping(out error) == TransportReturnCode.SUCCESS);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase18()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			MockChannel mockSocketChannel = new MockChannel(1, 1);
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			Assert.True(channel.Ping(out error) > TransportReturnCode.SUCCESS);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase20()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			String testData = "basicWFPTestCase20";
			int bufLen = testData.Length;
			ByteBuffer byteBuf = new ByteBuffer(bufLen);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));

			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			MockChannel mockSocketChannel = new MockChannel(bufLen + 3, bufLen + 3);
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.INITIALIZING, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.SetDataPosition(3);
			transBuf.Data.Put(byteBuf);
			transBuf.InPool = true;
			transBuf.IsOwnedByApp = true;

			Assert.Equal(TransportReturnCode.FAILURE, channel.Write(transBuf, writeArgs, out error));

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase21()
        {
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);
			Error error = new Error();

			MockChannel mockSocketChannel = new MockChannel(10, 10);
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.INITIALIZING, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);

			Assert.Equal(TransportReturnCode.FAILURE, channel.Flush(out error));

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase22()
        {
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			String testData = "basicWFPTestCase20";
			int bufLen = testData.Length;
			ByteBuffer byteBuf = new ByteBuffer(bufLen);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));

			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			MockChannel mockSocketChannel = new MockChannel(bufLen + 3, bufLen + 3);
			mockSocketChannel.ThrowExceptionOnSend = true;
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.SetDataPosition(3);
			transBuf.Data.Put(byteBuf);
			transBuf.InPool = true;
			transBuf.IsOwnedByApp = true;

			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			writeArgs.Priority = WritePriorities.HIGH;

			Assert.Equal(TransportReturnCode.FAILURE, channel.Write(transBuf, writeArgs, out error));

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase23()
        {
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			String testData = "basicWFPTestCase23";
			int bufLen = testData.Length;
			ByteBuffer byteBuf = new ByteBuffer(bufLen);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			MockChannel mockSocketChannel = new MockChannel(0, 0);
			mockSocketChannel.ThrowExceptionOnSend = true;
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, testData.Length + 5, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			TransportBuffer transBuf = new TransportBuffer(bufLen + 3);
			transBuf.IsReadMode = false;
			transBuf.SetDataPosition(3);
			byteBuf.Flip();
			transBuf.Data.Put(byteBuf);
			transBuf.IsReadMode = false;
			transBuf.InPool = true;
			transBuf.IsOwnedByApp = true;

			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;

			Assert.True((int)channel.Write(transBuf, writeArgs, out error) == testData.Length + 3);

			TransportBuffer transBuf1 = new TransportBuffer(bufLen + 3);
			transBuf1.SetDataPosition(3);
			byteBuf.Flip();
			transBuf1.Data.Put(byteBuf);
			transBuf1.IsReadMode = false;
			transBuf1.InPool = true;
			transBuf1.IsOwnedByApp = true;

			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			Assert.True(channel.Write(transBuf1, writeArgs, out error) == TransportReturnCode.FAILURE);

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase24()
        {
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			String testData = "basicWFPTestCase24";
			int bufLen = testData.Length;
			ByteBuffer byteBuf = new ByteBuffer(bufLen + 3);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			MockChannel mockSocketChannel = new MockChannel((int)TransportReturnCode.WRITE_FLUSH_FAILED, (int)TransportReturnCode.WRITE_FLUSH_FAILED);
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, 50, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			SocketBuffer sBuf = new SocketBuffer(null, 6144);
			TransportBuffer transBuf = sBuf.GetBufferSlice(bufLen + 3, false, SocketBuffer.RIPC_WRITE_POSITION);
			transBuf.SetDataPosition(3);
			transBuf.IsReadMode = false;
			transBuf.InPool = true;
			transBuf.IsOwnedByApp = true;
			transBuf.Data.Put(byteBuf);

			Assert.True((int)channel.Write(transBuf, writeArgs, out error) == testData.Length + 3);
			int val = (int)channel.Flush(out error);
			Assert.True(val == testData.Length + 3); // WE RETURN TOTAL BYTES QUEUED, EXCEPTIONS THROWN ARE PROPAGATED

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase25()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);
			Error error = new Error();

			MockChannel mockSocketChannel = new MockChannel(10, 10);
			mockSocketChannel.ThrowExceptionOnSend = true;
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);

			Assert.ThrowsAny<Exception>(() => channel.Ping(out error));

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase26()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);
			Error error = new Error();

			MockChannel mockSocketChannel = new MockChannel(10, 10);
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.INITIALIZING, 0, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);

			Assert.Equal(TransportReturnCode.FAILURE, channel.Ping(out error));

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase27()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			String testDataHigh = "basicWFPTestCase27High";
			String testDataMedium = "basicWFPTestCase27Medium";
			String testDataLow = "basicWFPTestCase27Low";
			int bufLenHigh = testDataHigh.Length;
			int bufLenMedium = testDataMedium.Length;
			int bufLenLow = testDataLow.Length;
			ByteBuffer byteBufHigh = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufMedium = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufLow = new ByteBuffer(bufLenLow);
			byteBufHigh.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufMedium.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufLow.Put(Encoding.ASCII.GetBytes(testDataLow));
			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			byte[] testBuffer = new byte[1224];
			int testBufPosition = 0;
			MockChannel mockSocketChannel = new MockChannel(10, 10);
			mockSocketChannel.SendListFunc = (buffers, throwExceptionOnSend, sendReturnValue) =>
			{
				int bytesWritten = 16, cumulativeBytes = 0;

				// write 16 bytes to testBuffer for each call
				foreach (var buf in buffers)
				{
					foreach (var b in buf)
					{
						testBuffer[testBufPosition++] = b;
						cumulativeBytes++;
						if (cumulativeBytes == bytesWritten)
						{
							break;
						}
					}
					if (cumulativeBytes == bytesWritten)
					{
						break;
					}
				}

				return (cumulativeBytes < bytesWritten) ? cumulativeBytes : bytesWritten;
			};
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			// create SocketBuffer and set to test data
			TransportBuffer transBufHigh = new TransportBuffer(bufLenHigh + 3);
			transBufHigh.SetDataPosition(3);
			byteBufHigh.Flip();
			transBufHigh.Data.Put(byteBufHigh);
			transBufHigh.IsReadMode = false;
			transBufHigh.InPool = true;
			transBufHigh.IsOwnedByApp = true;

			TransportBuffer transBufMedium = new TransportBuffer(bufLenMedium + 3);
			transBufMedium.SetDataPosition(3);
			byteBufMedium.Flip();
			transBufMedium.Data.Put(byteBufMedium);
			transBufMedium.IsReadMode = false;
			transBufMedium.InPool = true;
			transBufMedium.IsOwnedByApp = true;

			TransportBuffer transBufLow = new TransportBuffer(bufLenLow + 3);
			transBufLow.SetDataPosition(3);
			byteBufLow.Flip();
			transBufLow.Data.Put(byteBufLow);
			transBufLow.IsReadMode = false;
			transBufLow.InPool = true;
			transBufLow.IsOwnedByApp = true;

			int cumulativeBytesQueued = 0, writeReturnVal = 0;

			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			writeReturnVal = (int)channel.Write(transBufHigh, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued - 16);
			// bytesWritten should be 16
			Assert.True(writeArgs.BytesWritten == 16);


			// queue a couple more buffers by calling the write method with no write flags
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = (int)channel.Write(transBufMedium, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued - 16);
			// bytesWritten should be scktBufHigh.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);
			cumulativeBytesQueued += bufLenLow + 3;
			writeArgs.Priority = WritePriorities.LOW;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = (int)channel.Write(transBufLow, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued - 16);
			// bytesWritten should be scktBufMedium.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenLow + 3);

			// _totalBytesQueued in RsslSocketChannel should be cumulative bytes queued
			Assert.True(channel.m_totalBytesQueued == cumulativeBytesQueued - 16);

			while (cumulativeBytesQueued > 0)
			{
				int bytesFlushed = 0, flushRetVal;

				flushRetVal = (int)channel.Flush(out error);
				bytesFlushed = cumulativeBytesQueued - flushRetVal;

				// _totalBytesQueued in RsslSocketChannel should be bytes remaining to be sent
				Assert.True(channel.m_totalBytesQueued == (cumulativeBytesQueued - bytesFlushed));

				// decrement cumulativeBytesQueued by bytesFlushed
				cumulativeBytesQueued -= bytesFlushed;
			}

			// _totalBytesQueued in RsslSocketChannel should be 0
			Assert.True(channel.m_totalBytesQueued == 0);

			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 3, testDataHigh.Length));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 28, testDataMedium.Length));
			Assert.Equal(testDataLow, Encoding.ASCII.GetString(testBuffer, 55, testDataLow.Length));

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase28()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			String testDataHigh = "basicWFPTestCase28High";
			String testDataMedium = "basicWFPTestCase28Medium";
			String testDataLow = "basicWFPTestCase28Low";
			int bufLenHigh = testDataHigh.Length;
			int bufLenMedium = testDataMedium.Length;
			int bufLenLow = testDataLow.Length;
			ByteBuffer byteBufHigh = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufMedium = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufLow = new ByteBuffer(bufLenLow);
			byteBufHigh.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufMedium.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufLow.Put(Encoding.ASCII.GetBytes(testDataLow));
			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			byte[] testBuffer = new byte[1224];
			int testBufPosition = 0;
			MockChannel mockSocketChannel = new MockChannel(10, 10);
			mockSocketChannel.SendListFunc = (buffers, throwExceptionOnSend, sendReturnValue) =>
			{
				int bytesWritten = 16, cumulativeBytes = 0;

				// write 16 bytes to testBuffer for each call
				foreach (var buf in buffers)
				{
					foreach (var b in buf)
					{
						testBuffer[testBufPosition++] = b;
						cumulativeBytes++;
						if (cumulativeBytes == bytesWritten)
						{
							break;
						}
					}
					if (cumulativeBytes == bytesWritten)
					{
						break;
					}
				}

				return (cumulativeBytes < bytesWritten) ? cumulativeBytes : bytesWritten;
			};
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			// create SocketBuffer and set to test data
			TransportBuffer transBufHigh = new TransportBuffer(bufLenHigh + 3);
			transBufHigh.SetDataPosition(3);
			byteBufHigh.Flip();
			transBufHigh.Data.Put(byteBufHigh);
			transBufHigh.IsReadMode = false;
			transBufHigh.InPool = true;
			transBufHigh.IsOwnedByApp = true;

			TransportBuffer transBufMedium = new TransportBuffer(bufLenMedium + 3);
			transBufMedium.SetDataPosition(3);
			byteBufMedium.Flip();
			transBufMedium.Data.Put(byteBufMedium);
			transBufMedium.IsReadMode = false;
			transBufMedium.InPool = true;
			transBufMedium.IsOwnedByApp = true;

			TransportBuffer transBufLow = new TransportBuffer(bufLenLow + 3);
			transBufLow.SetDataPosition(3);
			byteBufLow.Flip();
			transBufLow.Data.Put(byteBufLow);
			transBufLow.IsReadMode = false;
			transBufLow.InPool = true;
			transBufLow.IsOwnedByApp = true;

			int cumulativeBytesQueued = 0, writeReturnVal = 0;

			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			writeReturnVal = (int)channel.Write(transBufMedium, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued - 16);
			// bytesWritten should be 16
			Assert.True(writeArgs.BytesWritten == 16);


			// queue a couple more buffers by calling the write method with no write flags
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = (int)channel.Write(transBufHigh, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued - 16);
			// bytesWritten should be scktBufHigh.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			cumulativeBytesQueued += bufLenLow + 3;
			writeArgs.Priority = WritePriorities.LOW;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = (int)channel.Write(transBufLow, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued - 16);
			// bytesWritten should be scktBufMedium.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenLow + 3);

			// _totalBytesQueued in RsslSocketChannel should be cumulative bytes queued
			Assert.True(channel.m_totalBytesQueued == cumulativeBytesQueued - 16);

			while (cumulativeBytesQueued > 0)
			{
				int bytesFlushed = 0, flushRetVal;

				flushRetVal = (int)channel.Flush(out error);
				bytesFlushed = cumulativeBytesQueued - flushRetVal;

				// _totalBytesQueued in RsslSocketChannel should be bytes remaining to be sent
				Assert.True(channel.m_totalBytesQueued == (cumulativeBytesQueued - bytesFlushed));

				// decrement cumulativeBytesQueued by bytesFlushed
				cumulativeBytesQueued -= bytesFlushed;
			}

			// _totalBytesQueued in RsslSocketChannel should be 0
			Assert.True(channel.m_totalBytesQueued == 0);

			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 3, testDataMedium.Length));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 30, testDataHigh.Length));
			Assert.Equal(testDataLow, Encoding.ASCII.GetString(testBuffer, 55, testDataLow.Length));

			Transport.Uninitialize();
		}

		[Fact]
		public void BasicWFPTestCase29()
		{
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			String testDataHigh = "basicWFPTestCase29High";
			String testDataMedium = "basicWFPTestCase29Medium";
			String testDataLow = "basicWFPTestCase29Low";
			int bufLenHigh = testDataHigh.Length;
			int bufLenMedium = testDataMedium.Length;
			int bufLenLow = testDataLow.Length;
			ByteBuffer byteBufHigh = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufMedium = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufLow = new ByteBuffer(bufLenLow);
			byteBufHigh.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufMedium.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufLow.Put(Encoding.ASCII.GetBytes(testDataLow));
			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			byte[] testBuffer = new byte[1224];
			int testBufPosition = 0;
			MockChannel mockSocketChannel = new MockChannel(10, 10);
			mockSocketChannel.SendListFunc = (buffers, throwExceptionOnSend, sendReturnValue) =>
			{
				int bytesWritten = 16, cumulativeBytes = 0;

				// write 16 bytes to testBuffer for each call
				foreach (var buf in buffers)
				{
					foreach (var b in buf)
					{
						testBuffer[testBufPosition++] = b;
						cumulativeBytes++;
						if (cumulativeBytes == bytesWritten)
						{
							break;
						}
					}
					if (cumulativeBytes == bytesWritten)
					{
						break;
					}
				}

				return (cumulativeBytes < bytesWritten) ? cumulativeBytes : bytesWritten;
			};
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockSocketChannel, ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			// create SocketBuffer and set to test data
			TransportBuffer transBufHigh = new TransportBuffer(bufLenHigh + 3);
			transBufHigh.SetDataPosition(3);
			byteBufHigh.Flip();
			transBufHigh.Data.Put(byteBufHigh);
			transBufHigh.IsReadMode = false;
			transBufHigh.InPool = true;
			transBufHigh.IsOwnedByApp = true;

			TransportBuffer transBufMedium = new TransportBuffer(bufLenMedium + 3);
			transBufMedium.SetDataPosition(3);
			byteBufMedium.Flip();
			transBufMedium.Data.Put(byteBufMedium);
			transBufMedium.IsReadMode = false;
			transBufMedium.InPool = true;
			transBufMedium.IsOwnedByApp = true;

			TransportBuffer transBufLow = new TransportBuffer(bufLenLow + 3);
			transBufLow.SetDataPosition(3);
			byteBufLow.Flip();
			transBufLow.Data.Put(byteBufLow);
			transBufLow.IsReadMode = false;
			transBufLow.InPool = true;
			transBufLow.IsOwnedByApp = true;

			int cumulativeBytesQueued = 0, writeReturnVal = 0;

			cumulativeBytesQueued += bufLenLow + 3;
			writeArgs.Priority = WritePriorities.LOW;
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			writeReturnVal = (int)channel.Write(transBufLow, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued - 16);
			// bytesWritten should be 16
			Assert.True(writeArgs.BytesWritten == 16);


			// queue a couple more buffers by calling the write method with no write flags
			cumulativeBytesQueued += bufLenHigh + 3;
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = (int)channel.Write(transBufHigh, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued - 16);
			// bytesWritten should be scktBufHigh.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenHigh + 3);
			cumulativeBytesQueued += bufLenMedium + 3;
			writeArgs.Priority = WritePriorities.MEDIUM;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = (int)channel.Write(transBufMedium, writeArgs, out error);
			// write return value should be cumulative bytes queued
			Assert.True((int)writeReturnVal == cumulativeBytesQueued - 16);
			// bytesWritten should be scktBufMedium.getLength()
			Assert.True(writeArgs.BytesWritten == bufLenMedium + 3);

			// _totalBytesQueued in RsslSocketChannel should be cumulative bytes queued
			Assert.True(channel.m_totalBytesQueued == cumulativeBytesQueued - 16);

			while (cumulativeBytesQueued > 0)
			{
				int bytesFlushed = 0, flushRetVal;

				flushRetVal = (int)channel.Flush(out error);
				bytesFlushed = cumulativeBytesQueued - flushRetVal;

				// _totalBytesQueued in RsslSocketChannel should be bytes remaining to be sent
				Assert.True(channel.m_totalBytesQueued == (cumulativeBytesQueued - bytesFlushed));

				// decrement cumulativeBytesQueued by bytesFlushed
				cumulativeBytesQueued -= bytesFlushed;
			}

			// _totalBytesQueued in RsslSocketChannel should be 0
			Assert.True(channel.m_totalBytesQueued == 0);

			Assert.Equal(testDataLow, Encoding.ASCII.GetString(testBuffer, 3, testDataLow.Length));
			Assert.Equal(testDataHigh, Encoding.ASCII.GetString(testBuffer, 27, testDataHigh.Length));
			Assert.Equal(testDataMedium, Encoding.ASCII.GetString(testBuffer, 52, testDataMedium.Length));

			Transport.Uninitialize();
		}

		#endregion

		#region Decompression Testcases

		[Fact]
		public void CompressedWriteTest()
        {
			string testData = "aaabbbcccdddeeefffggghhhiiijjjkkklllmmmnnnooopppqqqrrrssstttuuuvvvwwwxxxyyyzzz";
			int bufLen = testData.Length;
			ByteBuffer byteBuf = new ByteBuffer(bufLen);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var initerror);

			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			ChannelBase channel = new ChannelBase(new ConnectOptions(), new MockChannel(10, 10), ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			// set-up channel for compression
			channel.m_SessionOutCompression = CompressionType.ZLIB;
			channel.m_SessionCompLevel = 9;
			channel.m_Compressor = channel.m_ZlibCompressor;
			channel.m_Compressor.CompressionLevel = channel.m_SessionCompLevel;

			// set m_totalBytesQueued to 0 for no buffers queued
			channel.m_totalBytesQueued = 0;
			// create SocketBuffer and set to test data
			TransportBuffer transBuf = new TransportBuffer(bufLen + 3);
			transBuf.SetDataPosition(3);
			byteBuf.Flip();
			transBuf.Data.Put(byteBuf);
			transBuf.IsReadMode = false;
			transBuf.InPool = true;
			transBuf.IsOwnedByApp = true;

			/* FIRST WRITE */
			// write should return the total bytes queued
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			Assert.True(channel.Write(transBuf, writeArgs, out error) > 0);
			Assert.Equal(74, writeArgs.BytesWritten);
			Assert.Equal(81, writeArgs.UncompressedBytesWritten);
			
			TransportBuffer buf = (TransportBuffer)channel.m_highPriorityQueue._tail;
			byte[] compressed_msg = 
			{ 
					0x00, 0x4A, 0x04, 0x78, 0xDA, 0x04, 0xC1, 0x85, 0x01, 0x00, 0x20, 0x08, 0x00, 
					0xB0, 0x5B, 0xC5, 0x00, 0x3B, 0xB0, 0xF0, 0x7A, 0x37, 0xA5, 0x14, 0x00, 0x68, 0xAD, 0x8D, 0x31, 0xD6, 
					0x5A, 0xE7, 0x1C, 0x22, 0x12, 0x91, 0xF7, 0x3E, 0x84, 0x10, 0x63, 0x4C, 0x29, 0xE5, 0x9C, 0x4B, 0x29, 
					0xB5, 0xD6, 0xD6, 0x5A, 0xEF, 0x7D, 0x8C, 0xC1, 0xCC, 0x73, 0xCE, 0xB5, 0xD6, 0xDE, 0xFB, 0x9C, 0x73, 
					0xEF, 0x15, 0x91, 0xF7, 0xDE, 0x07, 0x00, 0x00, 0xFF, 0xFF 
			};

			for (int i = 0; i < compressed_msg.Length; i++)
				Assert.Equal(compressed_msg[i], buf.Data.Contents[i]);

			byteBuf.Clear();
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			TransportBuffer transBuf1 = new TransportBuffer(bufLen + 3);
			transBuf1.SetDataPosition(3);
			byteBuf.Flip();
			transBuf1.Data.Put(byteBuf);
			transBuf1.IsReadMode = false;
			transBuf1.InPool = true;
			transBuf1.IsOwnedByApp = true;
			
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			Assert.True(channel.Write(transBuf1, writeArgs, out error) > 0);
			Assert.Equal(13, writeArgs.BytesWritten);
			Assert.Equal(81, writeArgs.UncompressedBytesWritten);
			buf = (TransportBuffer)channel.m_highPriorityQueue._tail;

			byte[] compressed_msg2 =
			{
				0x00, 0x0D, 0x04, 0x4A, 0xA4, 0xAA, 0x69, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF
			};

			for (int i = 0; i < compressed_msg2.Length; i++)
				Assert.Equal(compressed_msg2[i], buf.Data.Contents[i]);

			byteBuf.Clear();
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));
			TransportBuffer transBuf2 = new TransportBuffer(bufLen + 3);
			transBuf2.SetDataPosition(3);
			byteBuf.Flip();
			transBuf2.Data.Put(byteBuf);
			transBuf2.IsReadMode = false;
			transBuf2.InPool = true;
			transBuf2.IsOwnedByApp = true;

			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			Assert.True(channel.Write(transBuf2, writeArgs, out error) > 0);
			Assert.Equal(12, writeArgs.BytesWritten);
			Assert.Equal(81, writeArgs.UncompressedBytesWritten);
			buf = (TransportBuffer)channel.m_highPriorityQueue._tail;

			byte[] compressed_msg3 =
			{
				0x00, 0x0C, 0x04, 0xA2, 0xAE, 0x69, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF
			};

			for (int i = 0; i < compressed_msg3.Length; i++)
				Assert.Equal(compressed_msg3[i], buf.Data.Contents[i]);

			Transport.Uninitialize();
		}

		[Fact]
		public void CompressedPackedWriteTestModified()
        {
			ConnectOptions connectOpts = GetDefaultConnectOptions("14059");
			connectOpts.CompressionType = CompressionType.ZLIB;
			BindOptions bindOpts = GetDefaultBindOptions("14059");
			bindOpts.CompressionType = CompressionType.ZLIB;
			bindOpts.CompressionLevel = 9;
			InitializeConnection(bindOpts, connectOpts, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			byte[] packed_1_expected_1 = Encoding.ASCII.GetBytes("packed test #1");
			byte[] packed_1_expected_2 = Encoding.ASCII.GetBytes("packed test #2");
			byte[] packed_1_expected_3 = Encoding.ASCII.GetBytes("final packing test #3");

			byte[] packed_2_expected_1 = Encoding.ASCII.GetBytes("packing2 test #1");
			byte[] packed_2_expected_2 = Encoding.ASCII.GetBytes("packing2 test #2");
			byte[] packed_2_expected_3 = Encoding.ASCII.GetBytes("final packing test #3");

			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();
			ReadArgs readArgs = new ReadArgs();

			ITransportBuffer sendBuffer = serverClientChannel.GetBuffer(1000, true, out error);
			ByteBuffer bb = sendBuffer.Data;
			bb.Put(Encoding.ASCII.GetBytes("packed test #1"));
			Assert.True(serverClientChannel.PackBuffer(sendBuffer, out error) > 0);

			bb.Put(Encoding.ASCII.GetBytes("packed test #2"));
			Assert.True(serverClientChannel.PackBuffer(sendBuffer, out error) > 0);

			bb.Put(Encoding.ASCII.GetBytes("final packing test #3"));
			Assert.True(serverClientChannel.PackBuffer(sendBuffer, out error) > 0);

			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			serverClientChannel.Write(sendBuffer, writeArgs, out error);

			ITransportBuffer msgBuf = null;

			msgBuf = clientChannel.Read(readArgs, out error);
			Assert.True(msgBuf != null);
			Assert.Equal(1, (int)readArgs.ReadRetVal);
			for (int i = 0; i < packed_1_expected_1.Length; i++)
				Assert.Equal(packed_1_expected_1[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);

			msgBuf = clientChannel.Read(readArgs, out error);
			Assert.Equal(1, (int)readArgs.ReadRetVal);
			Assert.Equal(0, readArgs.BytesRead); // compressed bytes read
			for (int i = 0; i < packed_1_expected_2.Length; i++)
				Assert.Equal(packed_1_expected_2[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);

			msgBuf = clientChannel.Read(readArgs, out error);
			for (int i = 0; i < packed_1_expected_3.Length; i++)
				Assert.Equal(packed_1_expected_3[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);

			sendBuffer = serverClientChannel.GetBuffer(1000, true, out error);
			bb = sendBuffer.Data;
			bb.Put(Encoding.ASCII.GetBytes("packing2 test #1"));
			Assert.True(serverClientChannel.PackBuffer(sendBuffer, out error) > 0);

			bb.Put(Encoding.ASCII.GetBytes("packing2 test #2"));
			Assert.True(serverClientChannel.PackBuffer(sendBuffer, out error) > 0);

			bb.Put(Encoding.ASCII.GetBytes("final packing test #3"));
			Assert.True(serverClientChannel.PackBuffer(sendBuffer, out error) > 0);

			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
			serverClientChannel.Write(sendBuffer, writeArgs, out error);

			msgBuf = clientChannel.Read(readArgs, out error);
			Assert.True(msgBuf != null);
			Assert.Equal(1, (int)readArgs.ReadRetVal);
			for (int i = 0; i < packed_2_expected_1.Length; i++)
				Assert.Equal(packed_2_expected_1[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);

			msgBuf = clientChannel.Read(readArgs, out error);
			Assert.Equal(1, (int)readArgs.ReadRetVal);
			Assert.Equal(0, readArgs.BytesRead); // compressed bytes read
			for (int i = 0; i < packed_2_expected_2.Length; i++)
				Assert.Equal(packed_2_expected_2[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);

			msgBuf = clientChannel.Read(readArgs, out error);
			for (int i = 0; i < packed_2_expected_3.Length; i++)
				Assert.Equal(packed_2_expected_3[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);
		}

		[Fact]
		public void DecompressConsumerMessages()
        {
			InitializeConnection("14021", 1, 1, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			byte[] input_msg1 = { 0x01, 0x19, 0x04, 0x78, 0xDA, 0x64, 0x8E, 0x41, 0x4F, 0xC2, 0x40, 0x10, 0x46, 0x77, 0x11, 0x13,
									0x34, 0x82, 0x56, 0x3D, 0x90, 0xE8, 0xA1, 0x5E, 0x3C, 0xA2, 0x25, 0xB1, 0x89, 0x07, 0x0F, 0x6D,
									0xE4, 0x60, 0x14, 0xA8, 0x34, 0x91, 0xF3, 0xD2, 0x8E, 0x74, 0x92, 0xED, 0xEE, 0xBA, 0xBB, 0x15,
									0xF1, 0x86, 0xBF, 0xDC, 0xC5, 0x94, 0xA0, 0xF1, 0x30, 0xC9, 0xE4, 0xBD, 0x2F, 0xDF, 0x0C, 0xFD,
									0x6A, 0x50, 0x42, 0x08, 0x2D, 0xC8, 0x1E, 0xB9, 0x78, 0x92, 0x73, 0x14, 0x3E, 0xCB, 0x32, 0x50,
									0x16, 0x72, 0x7F, 0xB6, 0xF4, 0x0B, 0x69, 0xAC, 0x3F, 0x8E, 0x1E, 0x87, 0xD3, 0x78, 0x30, 0x19,
									0xF5, 0x1B, 0x84, 0xAC, 0x9E, 0x2F, 0x3B, 0x53, 0xE4, 0x1C, 0x59, 0xD9, 0x8B, 0x41, 0x0B, 0xD0,
									0x74, 0x77, 0x75, 0xD7, 0x22, 0xED, 0x76, 0xA4, 0x14, 0xC7, 0x8C, 0x59, 0x94, 0xE2, 0x21, 0xF7,
									0x76, 0xFA, 0x37, 0xE1, 0xE1, 0x2F, 0x34, 0x62, 0x25, 0x78, 0x07, 0xDA, 0x18, 0x9E, 0x68, 0xF9,
									0x8E, 0x39, 0xE8, 0x56, 0x22, 0x0D, 0xAE, 0x95, 0xD7, 0x0D, 0xAE, 0x7B, 0xB7, 0x41, 0x2F, 0x08,
									0xD7, 0x13, 0x5E, 0x6D, 0xEF, 0x75, 0xEB, 0x6C, 0x02, 0xBA, 0x44, 0x63, 0x5C, 0xD6, 0x81, 0x57,
									0xE4, 0xD0, 0xA4, 0xF4, 0xFC, 0x9F, 0x1B, 0x7C, 0x28, 0x0D, 0x3F, 0x9B, 0x71, 0x7E, 0x3F, 0x45,
									0x31, 0xE7, 0x30, 0x56, 0x20, 0x9A, 0x94, 0x1C, 0x45, 0x9C, 0xCB, 0x45, 0x5A, 0x19, 0x05, 0x99,
									0xBD, 0x67, 0x96, 0xB9, 0xC4, 0x71, 0x5A, 0x29, 0x25, 0xB5, 0x4D, 0x58, 0x65, 0x60, 0x02, 0xA6,
									0x2A, 0x5D, 0x2F, 0x39, 0xAB, 0xE9, 0x58, 0x59, 0x2C, 0xF1, 0x13, 0xF2, 0xBF, 0xBA, 0xB3, 0xD1,
									0xC3, 0xA1, 0xFB, 0xDF, 0xBA, 0x9A, 0x93, 0x9A, 0xBC, 0x20, 0x2C, 0x26, 0xF0, 0x56, 0x81, 0xB1,
									0xEE, 0x3E, 0x39, 0xAD, 0x71, 0xCC, 0x6C, 0x56, 0x6C, 0x39, 0xDD, 0x14, 0xA4, 0x96, 0x89, 0x7C,
									0xB6, 0x74, 0xC9, 0x6F, 0x00, 0x00, 0x00, 0xFF, 0xFF};

			byte[] input_msg2 = { 01, 0x31, 0x04, 0x74, 0x92, 0x4F, 0x4F, 0x83, 0x40, 0x10, 0xC5, 0x97, 0x2D, 0x2C, 0xD0, 0xA2,
									0x2D, 0xD1, 0x68, 0x62, 0x3C, 0x90, 0x7E, 0x08, 0xE3, 0xB5, 0xF2, 0x27, 0x21, 0xD6, 0xA0, 0x40,
									0xF5, 0x48, 0x10, 0xB6, 0x42, 0x02, 0x6C, 0xB3, 0x0B, 0x4D, 0xBC, 0xE9, 0x17, 0x27, 0x0E, 0xB4,
									0x9A, 0x5E, 0x3C, 0x6D, 0xB2, 0xF3, 0x66, 0xF7, 0xCD, 0xFB, 0x0D, 0xBA, 0xC7, 0x32, 0x30, 0xC0,
									0xDF, 0x85, 0xAE, 0xA3, 0x65, 0xC4, 0x3A, 0x9E, 0x51, 0xCB, 0x29, 0x39, 0x38, 0x65, 0xFC, 0xD3,
									0x0A, 0xE9, 0x16, 0xC6, 0x2A, 0x2C, 0x9B, 0xD5, 0xBB, 0x8A, 0x02, 0x18, 0xA0, 0xF0, 0xA5, 0x68,
									0xD0, 0xA1, 0x22, 0x59, 0x45, 0x12, 0x96, 0xA4, 0x5E, 0x4A, 0x90, 0x82, 0x31, 0x9C, 0xB7, 0x1A,
									0x9A, 0xC9, 0x63, 0xD6, 0x33, 0xC7, 0x0F, 0x5D, 0x3B, 0x4E, 0x3C, 0xD7, 0x75, 0xC8, 0x2B, 0x6D,
									0x72, 0xC6, 0xCD, 0x79, 0x5C, 0xB0, 0x5A, 0xB0, 0x06, 0xDE, 0xEC, 0x5A, 0xCA, 0x85, 0xE6, 0x8B,
									0xC3, 0x77, 0xE0, 0xCB, 0xB0, 0xD3, 0x5D, 0xFA, 0x5E, 0x56, 0x80, 0x84, 0x8A, 0x79, 0x8F, 0xA6,
									0xE0, 0x8C, 0x28, 0x44, 0xD5, 0xA6, 0x57, 0x97, 0x4E, 0x99, 0x0D, 0xA0, 0x52, 0x0E, 0xA5, 0x63,
									0xEC, 0x39, 0x48, 0x2E, 0x4C, 0x70, 0x4D, 0xC2, 0x37, 0xCF, 0xAB, 0x72, 0x15, 0x0E, 0xB7, 0xE9,
									0xEA, 0xC5, 0xA9, 0x76, 0x23, 0xFE, 0xD5, 0x4D, 0x5E, 0x58, 0x04, 0x25, 0x62, 0xC0, 0xF2, 0x49,
									0xCB, 0xC5, 0x31, 0x14, 0x01, 0xB7, 0x61, 0xDA, 0x7C, 0x0C, 0x49, 0x6B, 0x7E, 0x4B, 0xEB, 0x75,
									0x29, 0x5A, 0xF3, 0x3C, 0xD9, 0x3C, 0xAF, 0x12, 0x3F, 0x76, 0x9F, 0x92, 0xB5, 0x1F, 0xC5, 0x37,
									0xBF, 0xE2, 0xA0, 0x6B, 0x83, 0xED, 0x03, 0xC4, 0x18, 0x35, 0xE9, 0x4E, 0x14, 0x6C, 0xCC, 0xFD,
									0x7A, 0x35, 0xEE, 0x2F, 0xC0, 0xB7, 0x61, 0x11, 0x00, 0x1A, 0x87, 0xA4, 0xDB, 0x6E, 0x28, 0x61,
									0xDC, 0xA3, 0x3B, 0x0D, 0x4D, 0x8C, 0x88, 0xF2, 0x7D, 0x99, 0xD1, 0xA1, 0x30, 0x8C, 0x6E, 0xFE,
									0xB5, 0x9C, 0x70, 0x22, 0x87, 0xB6, 0x33, 0x45, 0x47, 0x38, 0x78, 0xFC, 0x01, 0x00, 0x00, 0xFF,
									0xFF };

			byte[] expected_msg1 = { 0x01, 0x82, 0x02, 0x01, 0x00, 0x00, 0x00, 0x01, 0x68, 0x00, 0x09, 0x00, 0x21, //0x01, 0x87, 0x02,
										0x4C, 0x6F, 0x67, 0x69, 0x6E, 0x20, 0x61, 0x63, 0x63, 0x65, 0x70, 0x74, 0x65, 0x64, 0x20, 0x62,
										0x79, 0x20, 0x68, 0x6F, 0x73, 0x74, 0x20, 0x4F, 0x41, 0x4B, 0x4D, 0x57, 0x42, 0x45, 0x52, 0x4E,
										0x32, 0x02, 0x00, 0x00, 0x81, 0x51, 0x26, 0x0E, 0x57, 0x69, 0x6C, 0x6C, 0x69, 0x61, 0x6D, 0x2E,
										0x42, 0x65, 0x72, 0x6E, 0x65, 0x72, 0x01, 0x05, 0x81, 0x3D, 0x08, 0x00, 0x0D, 0x0D, 0x41, 0x70,
										0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x49, 0x64, 0x11, 0x03, 0x32, 0x35, 0x36,
										0x0F, 0x41, 0x70, 0x70, 0x6C, 0x69, 0x63, 0x61, 0x74, 0x69, 0x6F, 0x6E, 0x4E, 0x61, 0x6D, 0x65,
										0x11, 0x0C, 0x72, 0x73, 0x73, 0x6C, 0x50, 0x72, 0x6F, 0x76, 0x69, 0x64, 0x65, 0x72, 0x08, 0x50,
										0x6F, 0x73, 0x69, 0x74, 0x69, 0x6F, 0x6E, 0x11, 0x18, 0x31, 0x30, 0x2E, 0x39, 0x31, 0x2E, 0x31,
										0x36, 0x31, 0x2E, 0x31, 0x36, 0x36, 0x2F, 0x4F, 0x41, 0x4B, 0x4D, 0x57, 0x42, 0x45, 0x52, 0x4E,
										0x32, 0x18, 0x50, 0x72, 0x6F, 0x76, 0x69, 0x64, 0x65, 0x50, 0x65, 0x72, 0x6D, 0x69, 0x73, 0x73,
										0x69, 0x6F, 0x6E, 0x50, 0x72, 0x6F, 0x66, 0x69, 0x6C, 0x65, 0x04, 0x01, 0x01, 0x1C, 0x50, 0x72,
										0x6F, 0x76, 0x69, 0x64, 0x65, 0x50, 0x65, 0x72, 0x6D, 0x69, 0x73, 0x73, 0x69, 0x6F, 0x6E, 0x45,
										0x78, 0x70, 0x72, 0x65, 0x73, 0x73, 0x69, 0x6F, 0x6E, 0x73, 0x04, 0x01, 0x01, 0x0A, 0x53, 0x69,
										0x6E, 0x67, 0x6C, 0x65, 0x4F, 0x70, 0x65, 0x6E, 0x04, 0x01, 0x00, 0x10, 0x41, 0x6C, 0x6C, 0x6F,
										0x77, 0x53, 0x75, 0x73, 0x70, 0x65, 0x63, 0x74, 0x44, 0x61, 0x74, 0x61, 0x04, 0x01, 0x01, 0x12,
										0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x50, 0x61, 0x75, 0x73, 0x65, 0x52, 0x65, 0x73, 0x75,
										0x6D, 0x65, 0x04, 0x01, 0x00, 0x1B, 0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x4F, 0x70, 0x74,
										0x69, 0x6D, 0x69, 0x7A, 0x65, 0x64, 0x50, 0x61, 0x75, 0x73, 0x65, 0x52, 0x65, 0x73, 0x75, 0x6D,
										0x65, 0x04, 0x01, 0x00, 0x0E, 0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x4F, 0x4D, 0x4D, 0x50,
										0x6F, 0x73, 0x74, 0x04, 0x01, 0x01, 0x13, 0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x56, 0x69,
										0x65, 0x77, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73, 0x74, 0x73, 0x04, 0x01, 0x00, 0x14, 0x53, 0x75,
										0x70, 0x70, 0x6F, 0x72, 0x74, 0x42, 0x61, 0x74, 0x63, 0x68, 0x52, 0x65, 0x71, 0x75, 0x65, 0x73,
										0x74, 0x73, 0x04, 0x01, 0x01, 0x0E, 0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x53, 0x74, 0x61,
										0x6E, 0x64, 0x62, 0x79, 0x04, 0x01, 0x00 };

			byte[] expected_msg2 = { 0x00, 0x38, 0x02, 0x04, 0x00, 0x00, 0x00, 0x02, 0x81, 0x68, 0x09, 0x09, 0x00,
									0x22, 0x53, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x20, 0x44, 0x69, 0x72, 0x65, 0x63, 0x74, 0x6F, 0x72,
									0x79, 0x20, 0x52, 0x65, 0x66, 0x72, 0x65, 0x73, 0x68, 0x20, 0x43, 0x6F, 0x6D, 0x70, 0x6C, 0x65,
									0x74, 0x65, 0x64, 0x02, 0x00, 0x00, 0x80, 0x05, 0x08, 0x00, 0x00, 0x00, 0x07, 0x00, 0x04, 0x07,
									0x00, 0x01, 0x02, 0x01, 0x01, 0xFE, 0x01, 0x5F, 0x00, 0x05, 0x02, 0x02, 0x01, 0xFE, 0x01, 0x1C,
									0x08, 0x00, 0x0B, 0x04, 0x4E, 0x61, 0x6D, 0x65, 0x11, 0x0B, 0x44, 0x49, 0x52, 0x45, 0x43, 0x54,
									0x5F, 0x46, 0x45, 0x45, 0x44, 0x06, 0x56, 0x65, 0x6E, 0x64, 0x6F, 0x72, 0x11, 0x0F, 0x54, 0x68,
									0x6F, 0x6D, 0x73, 0x6F, 0x6E, 0x20, 0x52, 0x65, 0x75, 0x74, 0x65, 0x72, 0x73, 0x08, 0x49, 0x73,
									0x53, 0x6F, 0x75, 0x72, 0x63, 0x65, 0x04, 0x01, 0x01, 0x0C, 0x43, 0x61, 0x70, 0x61, 0x62, 0x69,
									0x6C, 0x69, 0x74, 0x69, 0x65, 0x73, 0x0F, 0xFE, 0x00, 0x0A, 0x04, 0x01, 0x00, 0x06, 0x05, 0x06,
									0x07, 0x08, 0x0A, 0x16, 0x14, 0x44, 0x69, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x61, 0x72, 0x69, 0x65,
									0x73, 0x50, 0x72, 0x6F, 0x76, 0x69, 0x64, 0x65, 0x64, 0x0F, 0xFE, 0x00, 0x13, 0x11, 0x00, 0x00,
									0x02, 0x06, 0x52, 0x57, 0x46, 0x46, 0x6C, 0x64, 0x07, 0x52, 0x57, 0x46, 0x45, 0x6E, 0x75, 0x6D,
									0x10, 0x44, 0x69, 0x63, 0x74, 0x69, 0x6F, 0x6E, 0x61, 0x72, 0x69, 0x65, 0x73, 0x55, 0x73, 0x65,
									0x64, 0x0F, 0xFE, 0x00, 0x13, 0x11, 0x00, 0x00, 0x02, 0x06, 0x52, 0x57, 0x46, 0x46, 0x6C, 0x64,
									0x07, 0x52, 0x57, 0x46, 0x45, 0x6E, 0x75, 0x6D, 0x03, 0x51, 0x6F, 0x53, 0x0F, 0xFE, 0x00, 0x06,
									0x0C, 0x00, 0x00, 0x01, 0x01, 0x22, 0x10, 0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x73, 0x51,
									0x6F, 0x53, 0x52, 0x61, 0x6E, 0x67, 0x65, 0x04, 0x01, 0x00, 0x08, 0x49, 0x74, 0x65, 0x6D, 0x4C,
									0x69, 0x73, 0x74, 0x11, 0x0E, 0x5F, 0x55, 0x50, 0x41, 0x5F, 0x49, 0x54, 0x45, 0x4D, 0x5F, 0x4C,
									0x49, 0x53, 0x54, 0x1A, 0x53, 0x75, 0x70, 0x70, 0x6F, 0x72, 0x74, 0x73, 0x4F, 0x75, 0x74, 0x4F,
									0x66, 0x42, 0x61, 0x6E, 0x64, 0x53, 0x6E, 0x61, 0x70, 0x73, 0x68, 0x6F, 0x74, 0x73, 0x04, 0x01,
									0x00, 0x17, 0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x69, 0x6E, 0x67, 0x43, 0x6F, 0x6E, 0x73, 0x75,
									0x6D, 0x65, 0x72, 0x53, 0x74, 0x61, 0x74, 0x75, 0x73, 0x04, 0x01, 0x00, 0x02, 0x02, 0xFE, 0x00,
									0x36, 0x08, 0x00, 0x03, 0x0C, 0x53, 0x65, 0x72, 0x76, 0x69, 0x63, 0x65, 0x53, 0x74, 0x61, 0x74,
									0x65, 0x04, 0x01, 0x01, 0x11, 0x41, 0x63, 0x63, 0x65, 0x70, 0x74, 0x69, 0x6E, 0x67, 0x52, 0x65,
									0x71, 0x75, 0x65, 0x73, 0x74, 0x73, 0x04, 0x01, 0x01, 0x06, 0x53, 0x74, 0x61, 0x74, 0x75, 0x73,
									0x0D, 0x05, 0x09, 0x00, 0x02, 0x4F, 0x4B};



			try 
			{
				ReadArgs readArgs = new ReadArgs();
				Error error = new Error();
				ITransportBuffer msgBuf = null;

				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(input_msg1);

				// read login response from the channel
				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(msgBuf.Length(), expected_msg1.Length);
				for (int i = 0; i < expected_msg1.Length; i++)
					Assert.Equal(expected_msg1[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);

				serverBase.Socket.Send(input_msg2);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(msgBuf.Length(), expected_msg2.Length);
				for (int i = 0; i < expected_msg2.Length; i++)
					Assert.Equal(expected_msg2[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}		
		}

		[Fact]
		public void DecompressPackedMessage()
        {
			byte[] packed_1 = { 0x00, 0x2F, 0x14, 0x78, 0xDA, 0x62, 0xE0, 0x2B, 0x48, 0x4C, 0xCE, 0x4E, 0x4D, 0x51, 0x28, 0x49,
									0x2D, 0x2E, 0x51, 0x50, 0x36, 0x64, 0x40, 0xE5, 0x1B, 0x31, 0x88, 0xA6, 0x65, 0xE6,
									0x25, 0xE6, 0x28, 0x80, 0x44, 0x33, 0xF3, 0xD2, 0xA1, 0xC2, 0xC6, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF };

			byte[] packed_1_expected_1 = { 0x70, 0x61, 0x63, 0x6B, 0x65, 0x64, 0x20, 0x74, 0x65, 0x73, 0x74, 0x20, 0x23, 0x31 };

			byte[] packed_1_expected_2 = { 0x70, 0x61, 0x63, 0x6B, 0x65, 0x64, 0x20, 0x74, 0x65, 0x73, 0x74, 0x20, 0x23, 0x32 };

			byte[] packed_1_expected_3 = { 0x66, 0x69, 0x6E, 0x61, 0x6C, 0x20, 0x70, 0x61, 0x63, 0x6B, 0x69, 0x6E, 0x67, 0x20, 
											0x74, 0x65, 0x73, 0x74, 0x20, 0x23, 0x33 };

			byte[] packed_2 = { 0x00, 0x1D, 0x14, 0x62, 0x10, 0x80, 0x0A, 0x18, 0xC1, 0x35, 0xA2, 0x8B, 0x18, 0x31, 0x88, 0x41, 
									0x45, 0x8C, 0x15, 0x20, 0x66, 0x40, 0xF5, 0x02, 0x00, 0x00, 0x00, 0xFF, 0xFF };

			byte[] packed_2_expected_1 = { 0x70, 0x61, 0x63, 0x6B, 0x69, 0x6E, 0x67, 0x32, 0x20, 0x74, 0x65, 0x73, 0x74, 0x20, 0x23, 0x31 };

			byte[] packed_2_expected_2 = { 0x70, 0x61, 0x63, 0x6B, 0x69, 0x6E, 0x67, 0x32, 0x20, 0x74, 0x65, 0x73, 0x74, 0x20, 0x23, 0x32 };

			byte[] packed_2_expected_3 = { 0x70, 0x61, 0x63, 0x6B, 0x69, 0x6E, 0x67, 0x33, 0x20, 0x66, 0x69, 0x6E, 0x61, 0x6C, 0x20, 0x74, 
											0x65, 0x73, 0x74, 0x20, 0x23, 0x33 };

			InitializeConnection("14022", 10, 10, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ReadArgs readArgs = new ReadArgs();
				Error error = new Error();
				ITransportBuffer msgBuf = null;

				int cumulativeUncompressedBytesRead = 0;

				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(packed_1);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(1, (int)readArgs.ReadRetVal);
				Assert.Equal(packed_1.Length, readArgs.BytesRead); // compressed bytes read
				Assert.Equal(58, readArgs.UncompressedBytesRead);
				for (int i = 0; i < packed_1_expected_1.Length; i++)
					Assert.Equal(packed_1_expected_1[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);
				cumulativeUncompressedBytesRead += readArgs.UncompressedBytesRead;

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(1, (int)readArgs.ReadRetVal);
				Assert.Equal(0, readArgs.BytesRead); // compressed bytes read
				Assert.Equal(0, readArgs.UncompressedBytesRead);
				cumulativeUncompressedBytesRead += readArgs.UncompressedBytesRead;
				for (int i = 0; i < packed_1_expected_2.Length; i++)
					Assert.Equal(packed_1_expected_2[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(0, readArgs.BytesRead); // compressed bytes read
				Assert.Equal(0, readArgs.UncompressedBytesRead);
				cumulativeUncompressedBytesRead += readArgs.UncompressedBytesRead;
				for (int i = 0; i < packed_1_expected_3.Length; i++)
					Assert.Equal(packed_1_expected_3[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);

				serverBase.Socket.Send(packed_2);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(1, (int)readArgs.ReadRetVal);
				Assert.Equal(packed_2.Length, readArgs.BytesRead); // compressed bytes read
				Assert.Equal(63, readArgs.UncompressedBytesRead);
				for (int i = 0; i < packed_2_expected_1.Length; i++)
					Assert.Equal(packed_2_expected_1[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);
				cumulativeUncompressedBytesRead += readArgs.UncompressedBytesRead;

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(1, (int)readArgs.ReadRetVal);
				Assert.Equal(0, readArgs.BytesRead); // compressed bytes read
				Assert.Equal(0, readArgs.UncompressedBytesRead);
				cumulativeUncompressedBytesRead += readArgs.UncompressedBytesRead;
				for (int i = 0; i < packed_2_expected_2.Length; i++)
					Assert.Equal(packed_2_expected_2[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(0, readArgs.BytesRead); // compressed bytes read
				Assert.Equal(0, readArgs.UncompressedBytesRead);
				cumulativeUncompressedBytesRead += readArgs.UncompressedBytesRead;
				for (int i = 0; i < packed_2_expected_3.Length; i++)
					Assert.Equal(packed_2_expected_3[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);

				Assert.Equal(packed_1_expected_1.Length + packed_1_expected_2.Length + packed_1_expected_3.Length
					+ packed_2_expected_1.Length + packed_2_expected_2.Length + packed_2_expected_3.Length
					+ 2 * FirstPackedHeaderLength() + 4 * RipcOffsets.PACKED_MSG_DATA, cumulativeUncompressedBytesRead);
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}	
		}

		[Fact]
		public void DecompressFragmentedMessage()
        {
			byte[] fragmented_compressed = { 0x01, 0x58, 0x05, 0x08, 0x00, 0x00, 0x1B, 0xF8, 0x00, 0x01, 0x78, 0x5E, 0x62, 0x60, 0x64, 0x62,
												0x66, 0x61, 0x65, 0x63, 0xE7, 0xE0, 0xE4, 0xE2, 0xE6, 0xE1, 0xE5, 0xE3, 0x17, 0x10, 0x14, 0x12,
												0x16, 0x11, 0x15, 0x13, 0x97, 0x90, 0x94, 0x92, 0x96, 0x91, 0x95, 0x93, 0x57, 0x50, 0x54, 0x52,
												0x56, 0x51, 0x55, 0x53, 0xD7, 0xD0, 0xD4, 0xD2, 0xD6, 0xD1, 0xD5, 0xD3, 0x37, 0x30, 0x34, 0x32,
												0x36, 0x31, 0x35, 0x33, 0xB7, 0xB0, 0xB4, 0xB2, 0xB6, 0xB1, 0xB5, 0xB3, 0x77, 0x70, 0x74, 0x72,
												0x76, 0x71, 0x75, 0x73, 0xF7, 0xF0, 0xF4, 0xF2, 0xF6, 0xF1, 0xF5, 0xF3, 0x0F, 0x08, 0x0C, 0x0A,
												0x0E, 0x09, 0x0D, 0x0B, 0x8F, 0x88, 0x8C, 0x8A, 0x8E, 0x89, 0x8D, 0x8B, 0x4F, 0x48, 0x4C, 0x4A,
												0x4E, 0x49, 0x4D, 0x4B, 0xCF, 0xC8, 0xCC, 0xCA, 0xCE, 0xC9, 0xCD, 0xCB, 0x2F, 0x28, 0x2C, 0x2A,
												0x2E, 0x29, 0x2D, 0x2B, 0xAF, 0xA8, 0xAC, 0xAA, 0xAE, 0xA9, 0xAD, 0xAB, 0x6F, 0x68, 0x6C, 0x6A,
												0x6E, 0x69, 0x6D, 0x6B, 0xEF, 0xE8, 0xEC, 0xEA, 0xEE, 0xE9, 0xED, 0xEB, 0x9F, 0x30, 0x71, 0xD2,
												0xE4, 0x29, 0x53, 0xA7, 0x4D, 0x9F, 0x31, 0x73, 0xD6, 0xEC, 0x39, 0x73, 0xE7, 0xCD, 0x5F, 0xB0,
												0x70, 0xD1, 0xE2, 0x25, 0x4B, 0x97, 0x2D, 0x5F, 0xB1, 0x72, 0xD5, 0xEA, 0x35, 0x6B, 0xD7, 0xAD,
												0xDF, 0xB0, 0x71, 0xD3, 0xE6, 0x2D, 0x5B, 0xB7, 0x6D, 0xDF, 0xB1, 0x73, 0xD7, 0xEE, 0x3D, 0x7B,
												0xF7, 0xED, 0x3F, 0x70, 0xF0, 0xD0, 0xE1, 0x23, 0x47, 0x8F, 0x1D, 0x3F, 0x71, 0xF2, 0xD4, 0xE9,
												0x33, 0x67, 0xCF, 0x9D, 0xBF, 0x70, 0xF1, 0xD2, 0xE5, 0x2B, 0x57, 0xAF, 0x5D, 0xBF, 0x71, 0xF3,
												0xD6, 0xED, 0x3B, 0x77, 0xEF, 0xDD, 0x7F, 0xF0, 0xF0, 0xD1, 0xE3, 0x27, 0x4F, 0x9F, 0x3D, 0x7F,
												0xF1, 0xF2, 0xD5, 0xEB, 0x37, 0x6F, 0xDF, 0xBD, 0xFF, 0xF0, 0xF1, 0xD3, 0xE7, 0x2F, 0x5F, 0xBF,
												0x7D, 0xFF, 0xF1, 0xF3, 0xD7, 0xEF, 0x3F, 0x7F, 0xFF, 0xFD, 0x67, 0x18, 0xF5, 0xFF, 0xA8, 0xFF,
												0x47, 0xFD, 0x3F, 0xEA, 0xFF, 0x51, 0xFF, 0x8F, 0xFA, 0x7F, 0xD4, 0xFF, 0xA3, 0xFE, 0x1F, 0xF5,
												0xFF, 0xA8, 0xFF, 0x47, 0xFD, 0x3F, 0xEA, 0xFF, 0x51, 0xFF, 0x8F, 0xFA, 0x7F, 0xD4, 0xFF, 0xA3,
												0xFE, 0x1F, 0xF5, 0xFF, 0xA8, 0xFF, 0x47, 0xFD, 0x3F, 0xEA, 0xFF, 0x51, 0xFF, 0x8F, 0xFA, 0x7F,
												0x18, 0xF8, 0x1F, 0x00, 0x00, 0x00, 0xFF, 0xFF, 0x00, 0x16, 0x05, 0x04, 0x00, 0x01, 0x1A, 0xF5,
												0xFF, 0xA8, 0xFF, 0x47, 0xFD, 0x3F, 0xEA, 0x7F, 0x00, 0x00, 0x00, 0x00, 0xFF, 0xFF};

			InitializeConnection("14023", 10, 10, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ReadArgs readArgs = new ReadArgs();
				Error error = new Error();
				ITransportBuffer msgBuf = null;

				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(fragmented_compressed);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf == null);
				Assert.Equal(22, (int)readArgs.ReadRetVal);
				Assert.Equal(fragmented_compressed.Length, readArgs.BytesRead); // compressed bytes read
				Assert.Equal(6138, readArgs.UncompressedBytesRead);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(0, readArgs.BytesRead); // compressed bytes read
				Assert.Equal(1038, readArgs.UncompressedBytesRead);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void DecompressFragmented6144L0Message()
		{
			InitializeConnection("14023", 10, 10, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ReadArgs readArgs = new ReadArgs();
				Error error = new Error();
				ITransportBuffer msgBuf = null;

				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(TestMessages.compressed_fragmented_6144L0);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf == null);
				Assert.Equal(23, (int)readArgs.ReadRetVal);
				Assert.Equal(6164 + RipcLengths.HEADER, readArgs.BytesRead);
				Assert.Equal(3, readArgs.UncompressedBytesRead);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(0, readArgs.BytesRead);
				Assert.Equal(6147, readArgs.UncompressedBytesRead);

				for (int i = 0; i < TestMessages.fragmented_6144L0.Length; i++)
					Assert.Equal(TestMessages.fragmented_6144L0[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}		
		}

		[Fact]
		public void DecompressFragmented6144L9Message()
        {
			InitializeConnection("14024", 10, 10, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ReadArgs readArgs = new ReadArgs();
				Error error = new Error();
				ITransportBuffer msgBuf = null;

				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(TestMessages.compressed_fragmented_6144L9);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(337, readArgs.BytesRead);
				Assert.Equal(TestMessages.fragmented_6144L0.Length + RipcLengths.HEADER, readArgs.UncompressedBytesRead);

				for (int i = 0; i < TestMessages.fragmented_6144L0.Length; i++)
					Assert.Equal(TestMessages.fragmented_6144L0[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void DecompressFragmented6131L0Message()
		{
			InitializeConnection("14025", 10, 10, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ReadArgs readArgs = new ReadArgs();
				Error error = new Error();
				ITransportBuffer msgBuf = null;

				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(TestMessages.compressed_fragmented_6131L0);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf == null);
				Assert.Equal(15, (int)readArgs.ReadRetVal);
				Assert.Equal(6156 + RipcLengths.HEADER, readArgs.BytesRead);
				Assert.Equal(3, readArgs.UncompressedBytesRead);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(0, readArgs.BytesRead);
				Assert.Equal(6134, readArgs.UncompressedBytesRead);

				for (int i = 0; i < TestMessages.fragmented_6131L0.Length; i++)
					Assert.Equal(TestMessages.fragmented_6131L0[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void DecompressFragmented6131L9Message()
		{
			InitializeConnection("14026", 10, 10, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ReadArgs readArgs = new ReadArgs();
				Error error = new Error();
				ITransportBuffer msgBuf = null;

				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(TestMessages.compressed_fragmented6131L9);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(337, readArgs.BytesRead);
				Assert.Equal(TestMessages.fragmented_6131L0.Length + RipcLengths.HEADER, readArgs.UncompressedBytesRead);

				for (int i = 0; i < TestMessages.fragmented_6131L0.Length; i++)
					Assert.Equal(TestMessages.fragmented_6131L0[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);
			}
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void DecompressFragmented6131L0X2Message()
		{
			InitializeConnection("14027", 100, 100, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ReadArgs readArgs = new ReadArgs();
				Error error = new Error();
				ITransportBuffer msgBuf = null;

				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(TestMessages.compressed_fragmented6131L0X2_1);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf == null);
				Assert.Equal(15, (int)readArgs.ReadRetVal);
				Assert.Equal(6159, readArgs.BytesRead);
				Assert.Equal(3, readArgs.UncompressedBytesRead);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(0, readArgs.BytesRead);
				Assert.Equal(6134, readArgs.UncompressedBytesRead);

				for (int i = 0; i < TestMessages.fragmented_6131L0.Length; i++)
					Assert.Equal(TestMessages.fragmented_6131L0[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);

				serverBase.Socket.Send(TestMessages.compressed_fragmented6131L0X2_2);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf == null);
				Assert.Equal(13, (int)readArgs.ReadRetVal);
				Assert.Equal(6157, readArgs.BytesRead);
				Assert.Equal(3, readArgs.UncompressedBytesRead);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(0, readArgs.BytesRead);
				Assert.Equal(6134, readArgs.UncompressedBytesRead);

				for (int i = 0; i < TestMessages.fragmented_6131L0.Length; i++)
					Assert.Equal(TestMessages.fragmented_6131L0[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void DecompressFragmented6145L0Message()
		{
			InitializeConnection("14028", 10, 10, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ReadArgs readArgs = new ReadArgs();
				Error error = new Error();
				ITransportBuffer msgBuf = null;

				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(TestMessages.compressed_fragmented_6145L0);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf == null);
				Assert.Equal(40, (int)readArgs.ReadRetVal);
				Assert.Equal(6184, readArgs.BytesRead);
				Assert.Equal(6137, readArgs.UncompressedBytesRead);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf == null);
				Assert.Equal(23, (int)readArgs.ReadRetVal);
				Assert.Equal(0, readArgs.BytesRead);
				Assert.Equal(7, readArgs.UncompressedBytesRead);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(0, readArgs.BytesRead);
				Assert.Equal(23, readArgs.UncompressedBytesRead);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void DecompressFragmented6145L9Message()
		{
			InitializeConnection("14029", 10, 10, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);
			
			try
            {
				ReadArgs readArgs = new ReadArgs();
				Error error = new Error();
				ITransportBuffer msgBuf = null;

				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				serverBase.Socket.Send(TestMessages.compressed_fragmented_6145L9);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf == null);
				Assert.Equal(23, (int)readArgs.ReadRetVal);
				Assert.Equal(6138, readArgs.UncompressedBytesRead);
				Assert.Equal(TestMessages.compressed_fragmented_6145L9.Length, readArgs.BytesRead);

				msgBuf = clientChannel.Read(readArgs, out error);
				Assert.True(msgBuf != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
				Assert.Equal(23, readArgs.UncompressedBytesRead);
				Assert.Equal(0, readArgs.BytesRead);
				Assert.Equal(6145, msgBuf.Length());

				for (int i = 0; i < TestMessages.fragmented_6145L9.Length; i++)
					Assert.Equal(TestMessages.fragmented_6145L9[i], msgBuf.Data.Contents[i + msgBuf.GetDataStartPosition()]);
			} finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		#endregion

		#region Testcases for Connection/Binding scenarios

		[Fact]
		public void BlockedConnectAndAcceptTest()
        {
			InitializeConnection("14030", 10, 10, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server, false, true);

			if (clientChannel != null) clientChannel.Close(out var error);
			if (serverClientChannel != null) serverClientChannel.Close(out var error);
			if (server != null) server.Close(out var error);

			Transport.Uninitialize();
		}

		[Fact]
		public void ServerBufferUsageTest()
        {
			BindOptions bindOpts = new BindOptions();
			ConnectOptions connectOpts = new ConnectOptions();

			bindOpts.ServiceName = "14031";
			bindOpts.ChannelIsBlocking = true;
			bindOpts.ServerBlocking = true;
			bindOpts.MinorVersion = Codec.Codec.MinorVersion();
			bindOpts.MajorVersion = Codec.Codec.MajorVersion();
			bindOpts.ProtocolType = ProtocolType.RWF;
			bindOpts.MaxOutputBuffers = 10;
			bindOpts.SharedPoolSize = 100;
			bindOpts.GuaranteedOutputBuffers = 5;
			bindOpts.MaxFragmentSize = 6144;

			connectOpts.UnifiedNetworkInfo.Address = "localhost";
			connectOpts.ProtocolType = ProtocolType.RWF;
			connectOpts.UnifiedNetworkInfo.ServiceName = "14031";
			connectOpts.ConnectionType = ConnectionType.SOCKET;
			connectOpts.MajorVersion = Codec.Codec.MajorVersion();
			connectOpts.MinorVersion = Codec.Codec.MinorVersion();
			connectOpts.NumInputBuffers = 10;

			InitializeConnection(bindOpts, connectOpts, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ITransportBuffer tBuffer;
				Error error;

				for (int i = 0; i < 5; ++i)
				{
					tBuffer = serverClientChannel.GetBuffer(6142, false, out error);
				}

				TransportReturnCode ret = server.BufferUsage(out error);
				Assert.Equal(0, (int)ret);

				for (int i = 5; i < 10; ++i)
				{
					tBuffer = serverClientChannel.GetBuffer(6142, false, out error);
				}

				ret = server.BufferUsage(out error);
				Assert.Equal(5, (int)ret);
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void BlockedReadWriteTest()
        {
			BindOptions bindOpts = new BindOptions();
			ConnectOptions connectOpts = new ConnectOptions();

			bindOpts.ServiceName = "14032";
			bindOpts.ChannelIsBlocking = true;
			bindOpts.ServerBlocking = true;
			bindOpts.MinorVersion = Codec.Codec.MinorVersion();
			bindOpts.MajorVersion = Codec.Codec.MajorVersion();
			bindOpts.ProtocolType = ProtocolType.RWF;
			bindOpts.MaxOutputBuffers = 10;
			bindOpts.SharedPoolSize = 100;
			bindOpts.GuaranteedOutputBuffers = 5;
			bindOpts.MaxFragmentSize = 6144;

			connectOpts.UnifiedNetworkInfo.Address = "localhost";
			connectOpts.ProtocolType = ProtocolType.RWF;
			connectOpts.UnifiedNetworkInfo.ServiceName = "14032";
			connectOpts.ConnectionType = ConnectionType.SOCKET;
			connectOpts.MajorVersion = Codec.Codec.MajorVersion();
			connectOpts.MinorVersion = Codec.Codec.MinorVersion();
			connectOpts.NumInputBuffers = 10;
			connectOpts.Blocking = true;

			InitializeConnection(bindOpts, connectOpts, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);
			
			string msg1 = "transport blocking read/ write test 1";
			string msg2 = "transport blocking read/write test 2";
			string msg3 = "got it!";

			try
            {
				Task serverTask = Task.Factory.StartNew(delegate
				{
					ITransportBuffer readBuf;
					ReadArgs readArgs = new ReadArgs();
					WriteArgs writeArgs = new WriteArgs();
					Error error = new Error();
					ITransportBuffer writeBuf;

					Assert.True((readBuf = serverClientChannel.Read(readArgs, out error)) != null);
					Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);
					for (int i = 0; i < msg1.Length; i++)
						Assert.Equal(Encoding.ASCII.GetBytes(msg1)[i], readBuf.Data.Contents[i + readBuf.GetDataStartPosition()]);

					Thread.Sleep(10); // delay so the other side blocks on read

					writeArgs.Priority = WritePriorities.HIGH;
					writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
					Assert.True((writeBuf = serverClientChannel.GetBuffer(100, false, out error)) != null);
					writeBuf.Data.Put(Encoding.ASCII.GetBytes(msg2));
					Assert.Equal(TransportReturnCode.SUCCESS, serverClientChannel.Write(writeBuf, writeArgs, out error));

					Assert.True((readBuf = serverClientChannel.Read(readArgs, out error)) != null);
					Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);

					for (int i = 0; i < msg3.Length; i++)
						Assert.Equal(Encoding.ASCII.GetBytes(msg3)[i], readBuf.Data.Contents[i + readBuf.GetDataStartPosition()]);
				});

				ITransportBuffer readBuf;
				ReadArgs readArgs = new ReadArgs();
				WriteArgs writeArgs = new WriteArgs();
				Error error = new Error();
				ITransportBuffer writeBuf;

				Thread.Sleep(10); // delay so the other side blocks on read

				writeArgs.Priority = WritePriorities.HIGH;
				writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
				Assert.True((writeBuf = clientChannel.GetBuffer(100, false, out error)) != null);
				writeBuf.Data.Put(Encoding.ASCII.GetBytes(msg1));
				Assert.Equal(TransportReturnCode.SUCCESS, clientChannel.Write(writeBuf, writeArgs, out error));

				Assert.True((readBuf = clientChannel.Read(readArgs, out error)) != null);
				Assert.Equal(TransportReturnCode.SUCCESS, readArgs.ReadRetVal);

				for (int i = 0; i < msg2.Length; i++)
					Assert.Equal(Encoding.ASCII.GetBytes(msg2)[i], readBuf.Data.Contents[i + readBuf.GetDataStartPosition()]);

				Thread.Sleep(10); // delay so the other side blocks on read

				writeArgs.Priority = WritePriorities.HIGH;
				writeArgs.Flags = WriteFlags.DIRECT_SOCKET_WRITE;
				Assert.True((writeBuf = clientChannel.GetBuffer(100, false, out error)) != null);
				writeBuf.Data.Put(Encoding.ASCII.GetBytes(msg3));
				Assert.Equal(TransportReturnCode.SUCCESS, clientChannel.Write(writeBuf, writeArgs, out error));
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void BlockedConnectRIPCVersion12FallbackTest()
		{
			RIPCVersionFallback(ConnectionsVersions.VERSION12);
		}

		[Fact]
		public void BlockedConnectRIPCVersion11FallbackTest()
		{
			RIPCVersionFallback(ConnectionsVersions.VERSION11);
		}

		[Fact]
		public void ProviderNICBindTest()
        {
			IServer s = null;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = false
			};
			Transport.Initialize(initArgs, out var error);

			BindOptions bindOpts = new BindOptions();
			bindOpts.ServiceName = "14034";
			bindOpts.ChannelIsBlocking = false;
			bindOpts.MinorVersion = Codec.Codec.MinorVersion();
			bindOpts.MajorVersion = Codec.Codec.MajorVersion();
			bindOpts.ProtocolType = ProtocolType.RWF;
			bindOpts.MaxOutputBuffers = 10;
			bindOpts.SharedPoolSize = 100;
			bindOpts.GuaranteedOutputBuffers = 5;
			bindOpts.MaxFragmentSize = 6144;

			s = BindServer(bindOpts);

			IChannel clientChannel = null;
			IChannel serverChannel = null;

            try
            {
                Task clientTask = Task.Factory.StartNew(delegate
                {
                    clientChannel = ConnectChannel(GetDefaultConnectOptions("14034"));
                });

                Task serverTask = Task.Factory.StartNew(delegate
                {
                    serverChannel = WaitUntilServerAcceptNoInit(s);
                });

                Task.WaitAll(new Task[] { clientTask, serverTask });

                byte[] connectReq = { 0x00, 0x26, 0x00, 0x00, 0x00, 0x00, 0x15, 0x00, 0x26, 0x00, 0x3C, 0x00, 0x00, 0x0E, 0x00, 0x0A, 0x58,
                                    0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x58, 0x0B, 0x31, 0x30, 0x2E, 0x39, 0x31, 0x2E, 0x31,
                                    0x36, 0x31, 0x2E, 0x37 };
                ChannelBase clientBase = (ChannelBase)clientChannel;
                clientBase.Socket.Send(connectReq);

                InProgInfo inProg = new InProgInfo();
                while (serverChannel.State != ChannelState.ACTIVE)
                {
                    var ret = serverChannel.Init(inProg, out error);
                    Assert.True(ret >= TransportReturnCode.SUCCESS);
                }

                Assert.True(clientBase.Socket.Poll(TIMEOUTMS, SelectMode.SelectRead));
            }
            finally
            {
                if (clientChannel != null) clientChannel.Close(out error);
                if (serverChannel != null) serverChannel.Close(out error);
                if (s != null) s.Close(out error);

                Transport.Uninitialize();
            }
		}

		[Fact]
		public void ConsumerNICBindTest()
		{
			IServer s = null;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = false
			};
			Transport.Initialize(initArgs, out var error);

			BindOptions bindOpts = new BindOptions();
			bindOpts.ServiceName = "14034";
			bindOpts.ChannelIsBlocking = false;
			bindOpts.MinorVersion = Codec.Codec.MinorVersion();
			bindOpts.MajorVersion = Codec.Codec.MajorVersion();
			bindOpts.ProtocolType = ProtocolType.RWF;
			bindOpts.MaxOutputBuffers = 10;
			bindOpts.SharedPoolSize = 100;
			bindOpts.GuaranteedOutputBuffers = 5;
			bindOpts.MaxFragmentSize = 6144;

			s = BindServer(bindOpts);

			IChannel clientChannel = null;
			IChannel serverChannel = null;

            try
            {

                Task clientTask = Task.Factory.StartNew(delegate
                {
                    clientChannel = ConnectChannel(GetDefaultConnectOptions("14034"));
                });

                Task serverTask = Task.Factory.StartNew(delegate
                {
                    serverChannel = WaitUntilServerAcceptNoInit(s);
                });

                Task.WaitAll(new Task[] { clientTask, serverTask });

                byte[] connectMsg =
                {
                    0x00, 0x40, 0x01, 0x01, 0x0A, 0x00, 0x00, 0x00, 0x00, 0x09, 0x18, 0x00, 0x03, 0x3C, 0x0E, 0x01,
                    0x00, 0x00, 0x00, 0x08, 0x01, 0x18, 0x5B, 0x34, 0xE8, 0xD8, 0x31, 0xDC, 0x82, 0xDE, 0x00, 0x00,
                    0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x08, 0xD2, 0xCC, 0x77, 0xF6, 0x25, 0x4E, 0xC9, 0x11, 0x10,
                    0x45, 0x54, 0x41, 0x20, 0x4A, 0x61, 0x76, 0x61, 0x20, 0x45, 0x64, 0x69, 0x74, 0x69, 0x6F, 0x6E
                };
                InProgInfo inProg = new InProgInfo();

                Assert.Equal(ChannelState.INITIALIZING, clientChannel.State);
                Assert.Equal(TransportReturnCode.CHAN_INIT_IN_PROGRESS, clientChannel.Init(inProg, out error));

                ChannelBase serverBase = (ChannelBase)serverChannel;
                serverBase.Socket.Send(connectMsg);

                Assert.Equal(TransportReturnCode.SUCCESS, clientChannel.Init(inProg, out error));
                Assert.Equal(ChannelState.ACTIVE, clientChannel.State);

            }
            finally
            {
                if (clientChannel != null) clientChannel.Close(out error);
                if (serverChannel != null) serverChannel.Close(out error);
                if (s != null) s.Close(out error);

                Transport.Uninitialize();
            }
		}

        #endregion

        #region IOCtl Functionality

		[Fact]
		public void IOCtlInvalidCodeTest()
        {
			Error error = new Error();
			BindOptions bindOptions = GetDefaultBindOptions("14035");
			bindOptions.SharedPoolSize = 3;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = false
			};

			IServer server = null;

			try
            {
				Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out error));
				server = Transport.Bind(bindOptions, out error);
				Assert.NotNull(server);

				Assert.Equal(TransportReturnCode.FAILURE, server.IOCtl((IOCtlCode)99999, 0, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
				Assert.True("Code is not valid.".Equals(error.Text));

				Assert.Equal(TransportReturnCode.FAILURE, server.IOCtl((IOCtlCode)99999, 1, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
				Assert.True("Code is not valid.".Equals(error.Text));

				ChannelBase channel = new ChannelBase(new ConnectOptions(), new SocketChannel(), ChannelState.ACTIVE, 1000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
				channel.m_totalBytesQueued = 0;

				Assert.Equal(TransportReturnCode.FAILURE, channel.IOCtl((IOCtlCode)99999, 0, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
				Assert.True("Code is not valid.".Equals(error.Text));

				Assert.Equal(TransportReturnCode.FAILURE, channel.IOCtl((IOCtlCode)99999, 1, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
				Assert.True("Code is not valid.".Equals(error.Text));
			} 
			finally
            {
				if (server != null)
					server.Close(out error);

				Transport.Uninitialize();
            }
			
		}

		[Fact]
		public void IOCtlInvalidComponentInfoValueTest()
        {
			Error error = new Error();
			BindOptions bindOptions = GetDefaultBindOptions("14036");
			bindOptions.SharedPoolSize = 3;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = false
			};

			IServer server = null;

			try
            {
				Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out error));
				server = Transport.Bind(bindOptions, out error);
				Assert.NotNull(server);

				Assert.Equal(TransportReturnCode.FAILURE, server.IOCtl((IOCtlCode)13, 0, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
				Assert.True("Code is not valid.".Equals(error.Text));

				Assert.Equal(TransportReturnCode.FAILURE, server.IOCtl((IOCtlCode)13, 1, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
				Assert.True("Code is not valid.".Equals(error.Text));
			} 
			finally
            {
				if (server != null)
					server.Close(out error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void IOCtlNumGuaranteedBuffersTest()
        {
			int initialBufferCount = 20; 
			int bufferCount = 20;
			int growCount = 10;
			int shrinkCount = 10;

			Error error = new Error();

			ConnectOptions connectOpts = GetDefaultConnectOptions("14037");
			connectOpts.GuaranteedOutputBuffers = initialBufferCount;

			BindOptions bindOpts = GetDefaultBindOptions("14037");

			InitializeConnection(bindOpts, connectOpts, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ChannelBase clientBase = (ChannelBase)clientChannel;

				Assert.Equal(initialBufferCount, clientBase.m_AvailableBuffers.Size);
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.MaxOutputBuffers);

				bufferCount = initialBufferCount - shrinkCount;
				Assert.Equal(bufferCount, (int)clientChannel.IOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, bufferCount, out error));
				Assert.Equal(bufferCount, clientBase.m_AvailableBuffers.Size);
				Assert.Equal(bufferCount, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.MaxOutputBuffers);

				// grow guaranteedOutputBuffers
				bufferCount = initialBufferCount + growCount;
				Assert.Equal(bufferCount, (int)clientChannel.IOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, bufferCount, out error));
				Assert.Equal(bufferCount, clientBase.m_AvailableBuffers.Size);
				Assert.Equal(bufferCount, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(bufferCount, clientBase.m_ChannelInfo.MaxOutputBuffers);

				// call ioctl with  NUM_GUARANTEED_BUFFERS unchanged
				Assert.Equal(bufferCount, (int)clientChannel.IOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, bufferCount, out error));
				Assert.Equal(bufferCount, clientBase.m_AvailableBuffers.Size);
				Assert.Equal(bufferCount, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(bufferCount, clientBase.m_ChannelInfo.MaxOutputBuffers);

				// shrink guaranteedOutputBuffers to Zero
				Assert.Equal(0, (int)clientChannel.IOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, 0, out error));
				Assert.Equal(0, clientBase.m_AvailableBuffers.Size);
				Assert.Equal(0, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(bufferCount, clientBase.m_ChannelInfo.MaxOutputBuffers);

				// try using negative number
				Assert.Equal(TransportReturnCode.FAILURE, clientChannel.IOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, -1, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
			} finally
            {
				if (server != null)
					server.Close(out error);
				if (serverClientChannel != null)
					serverClientChannel.Close(out error);
				if (clientChannel != null)
					clientChannel.Close(out error);

				Transport.Uninitialize();
			}		
		}

		[Fact]
		public void IOCtlNumGuaranteedBuffersWithUsedBuffersTest()
        {
			int initialBufferCount = 25, bufferCount = 25;
			int shrinkCount = 20;
			int inUseCount = 15;
			int postShrinkBufferCount = 15;

			Error error = new Error();

			ConnectOptions connectOpts = GetDefaultConnectOptions("14038");
			connectOpts.GuaranteedOutputBuffers = initialBufferCount;

			BindOptions bindOpts = GetDefaultBindOptions("14038");

			InitializeConnection(bindOpts, connectOpts, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
			{
				ChannelBase clientBase = (ChannelBase)clientChannel;

				Assert.Equal(initialBufferCount, clientBase.m_AvailableBuffers.Size);
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.MaxOutputBuffers);

				for (int i = 0; i < inUseCount; i++)
				{
					Assert.NotNull(clientBase.GetBuffer(clientBase.InternalFragmentSize - 3, false, out error));
					Assert.Null(error);
				}

				// shrink guaranteedOutputBuffers, expect it to return a different value (postShrinkBufferCount) since there were not.
				bufferCount = initialBufferCount - shrinkCount;
				Assert.Equal(postShrinkBufferCount, (int)clientChannel.IOCtl(IOCtlCode.NUM_GUARANTEED_BUFFERS, bufferCount, out error));
				Assert.Null(error);
				Assert.Equal(0, clientBase.m_AvailableBuffers.Size);
				Assert.Equal(postShrinkBufferCount, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.MaxOutputBuffers);
			}
			finally
			{
				if (server != null) server.Close(out error);
				if (serverClientChannel != null) serverClientChannel.Close(out error);
				if (clientChannel != null) clientChannel.Close(out error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void IOCtlMaxNumBuffersTest()
        {
			Error error = new Error();
			int initialBufferCount = 50;
			int newMaxNumBuffers = 100;
			int newMinNumBuffers = 20;

			ConnectOptions connectOpts = GetDefaultConnectOptions("14039");
			connectOpts.GuaranteedOutputBuffers = initialBufferCount;

			BindOptions bindOpts = GetDefaultBindOptions("14039");
			InitializeConnection(bindOpts, connectOpts, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {				
				ChannelBase clientBase = (ChannelBase)clientChannel;

				// verify setup.
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.MaxOutputBuffers);

				// set MAX_NUM_BUFFERS to the same value.
				Assert.Equal(initialBufferCount, (int)clientBase.IOCtl(IOCtlCode.MAX_NUM_BUFFERS, initialBufferCount, out error));
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.MaxOutputBuffers);

				// modify MAX_NUM_BUFFERS to a larger value.
				Assert.Equal(newMaxNumBuffers, (int)clientBase.IOCtl(IOCtlCode.MAX_NUM_BUFFERS, newMaxNumBuffers, out error));
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(newMaxNumBuffers, clientBase.m_ChannelInfo.MaxOutputBuffers);

				// modify MAX_NUM_BUFFERS to a smaller value.
				Assert.Equal(initialBufferCount + 5, (int)clientBase.IOCtl(IOCtlCode.MAX_NUM_BUFFERS, initialBufferCount + 5, out error));
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(initialBufferCount + 5, clientBase.m_ChannelInfo.MaxOutputBuffers);

				// modify MAX_NUM_BUFFERS to a value smaller than guaranteedOutputBuffers
				Assert.Equal(initialBufferCount, (int)clientBase.IOCtl(IOCtlCode.MAX_NUM_BUFFERS, newMinNumBuffers, out error));
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.GuaranteedOutputBuffers);
				Assert.Equal(initialBufferCount, clientBase.m_ChannelInfo.MaxOutputBuffers);

				// modify MAX_NUM_BUFFERS to a negative value, expect it to fail.
				Assert.Equal(TransportReturnCode.FAILURE, clientBase.IOCtl(IOCtlCode.MAX_NUM_BUFFERS, -1, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
			}
			finally
			{
				if (serverClientChannel != null) serverClientChannel.Close(out error);
				if (server != null) server.Close(out error);
				if (clientChannel != null) clientChannel.Close(out error);
				Assert.Equal(TransportReturnCode.SUCCESS, Transport.Uninitialize());
			}
		}

		[Fact]
		public void IOCtlServerPeakBufferResetTest()
        {
			Error error = new Error();
			BindOptions bindOptions = GetDefaultBindOptions("14040");
			bindOptions.SharedPoolSize = 3;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = false
			};

			IServer server = null;
			ServerInfo serverInfo = new ServerInfo();
			SocketBuffer[] socketBuffer = new SocketBuffer[5];

			try
			{
				Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out error));
				server = Transport.Bind(bindOptions, out error);
				Assert.NotNull(server);

				GetAndVerifyServerInfo(server, serverInfo, error, 0, 0);
				
				socketBuffer[0] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.NotNull(socketBuffer[0]);
				GetAndVerifyServerInfo(server, serverInfo, error, 1, 1);

				socketBuffer[1] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.NotNull(socketBuffer[1]);
				GetAndVerifyServerInfo(server, serverInfo, error, 2, 2);

				socketBuffer[2] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.NotNull(socketBuffer[2]);
				GetAndVerifyServerInfo(server, serverInfo, error, 3, 3);

				// attempt to get more than shared pool size
				socketBuffer[3] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.Null(socketBuffer[3]);
				GetAndVerifyServerInfo(server, serverInfo, error, 3, 3);

				socketBuffer[2].ReturnToPool();
				GetAndVerifyServerInfo(server, serverInfo, error, 2, 3);

				// reset the server's peak buffer count
				Assert.Equal(TransportReturnCode.SUCCESS, server.IOCtl(IOCtlCode.SERVER_PEAK_BUF_RESET, 0, out error));
				GetAndVerifyServerInfo(server, serverInfo, error, 2, 2);
				
				socketBuffer[1].ReturnToPool();
				GetAndVerifyServerInfo(server, serverInfo, error, 1, 2);

				socketBuffer[0].ReturnToPool();
				GetAndVerifyServerInfo(server, serverInfo, error, 0, 2);

				// reset the server's peak buffer count
				Assert.Equal(TransportReturnCode.SUCCESS, server.IOCtl(IOCtlCode.SERVER_PEAK_BUF_RESET, 0, out error));
				GetAndVerifyServerInfo(server, serverInfo, error, 0, 0);
			}
			finally
			{
				if (server != null) server.Close(out error);
				Assert.Equal(TransportReturnCode.SUCCESS, Transport.Uninitialize());
			}
		}

		[Fact]
		public void IOCtlServerNumPoolBuffersTest()
        {
			Error error = new Error();
			BindOptions bindOptions = GetDefaultBindOptions("14041");
			bindOptions.SharedPoolSize = 3;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = false
			};

			IServer server = null;
			ServerInfo serverInfo = new ServerInfo();
			SocketBuffer[] socketBuffer = new SocketBuffer[6];

			try
			{
				Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out error));
				server = Transport.Bind(bindOptions, out error);
				Assert.NotNull(server);

				ServerImpl socketServer = (ServerImpl)server;
				// The "global" pool.
				Pool bufferPool = socketServer.m_ProtocolBase.GetPool(socketServer.BufferSize());
				Assert.Equal(3, socketServer.BindOptions.SharedPoolSize);
				Assert.Equal(0, socketServer.m_SharedPool.Size);
				Assert.Equal(0, bufferPool.Size);

				/*
				* allocate four SocketBuffers from sharedPool. The fourth should be
				* null since it passes the sharedPoolSize.
				*/
				Assert.Equal(0, socketServer.m_SharedPool.Size); // allocated on demand.
				socketBuffer[0] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.NotNull(socketBuffer[0]);
				socketBuffer[1] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.NotNull(socketBuffer[1]);
				Assert.True(socketBuffer[0] != socketBuffer[1]);
				socketBuffer[2] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.NotNull(socketBuffer[2]);
				Assert.True(socketBuffer[2] != socketBuffer[0]);
				Assert.True(socketBuffer[2] != socketBuffer[1]);
				socketBuffer[3] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.Null(socketBuffer[3]);

				Assert.Equal(3, (int)server.BufferUsage(out error));
				Assert.Equal(3, socketServer.BindOptions.SharedPoolSize);
				Assert.Equal(0, socketServer.m_SharedPool.Size);
				Assert.Equal(0, bufferPool.Size);

				/*
				 * use IoctlCodes.SERVER_NUM_POOL_BUFFERS to increase the size of
				 * the sharedPoolSize to 5, then allocate three more SocketBuffers,
				 * the third should fail since the sharedPoolSize is exceeded.
				 */
				Assert.Equal(5, (int)server.IOCtl(IOCtlCode.SERVER_NUM_POOL_BUFFERS, 5, out error));
				Assert.Equal(5, socketServer.BindOptions.SharedPoolSize);
				Assert.Equal(0, socketServer.m_SharedPool.Size);
				Assert.Equal(0, bufferPool.Size);

				socketBuffer[3] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.NotNull(socketBuffer[3]);
				Assert.True(socketBuffer[3] != socketBuffer[0]);
				Assert.True(socketBuffer[3] != socketBuffer[1]);
				Assert.True(socketBuffer[3] != socketBuffer[2]);
				socketBuffer[4] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.NotNull(socketBuffer[4]);
				Assert.True(socketBuffer[4] != socketBuffer[0]);
				Assert.True(socketBuffer[4] != socketBuffer[1]);
				Assert.True(socketBuffer[4] != socketBuffer[2]);
				Assert.True(socketBuffer[4] != socketBuffer[3]);
				socketBuffer[5] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.Null(socketBuffer[5]);
				Assert.Equal(5, (int)server.BufferUsage(out error));

				/*
				 * use IoctlCodes.SERVER_NUM_POOL_BUFFERS to decrease the size of
				 * the sharedPoolSize to 4, then allocate one more SocketBuffers,
				 * which should fail since the sharedPoolSize is exceeded.
				 */
				Assert.Equal(5, (int)server.IOCtl(IOCtlCode.SERVER_NUM_POOL_BUFFERS, 4, out error));
				// sharedPoolSize did not change since all buffers are in use.
				Assert.Equal(5, socketServer.BindOptions.SharedPoolSize);
				Assert.Equal(0, socketServer.m_SharedPool.Size);
				Assert.Equal(0, bufferPool.Size);
				socketBuffer[5] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.Null(socketBuffer[5]);
				Assert.Equal(5, (int)server.BufferUsage(out error));

				/*
				 * release two SocketBuffers, then allocate one more SocketBuffer
				 * which should succeeded since the SharedPoolSize is not exceeded.
				 */
				socketBuffer[4].ReturnToPool();
				socketBuffer[4] = null;
				Assert.Equal(4, (int)server.BufferUsage(out error));
				socketBuffer[3].ReturnToPool();
				socketBuffer[3] = null;
				Assert.Equal(3, (int)server.BufferUsage(out error));
				Assert.Equal(5, socketServer.BindOptions.SharedPoolSize);
				Assert.Equal(2, socketServer.m_SharedPool.Size);
				Assert.Equal(0, bufferPool.Size);

				socketBuffer[3] = ((ServerImpl)server).GetBufferFromServerPool();
				Assert.NotNull(socketBuffer[3]);
				Assert.True(socketBuffer[3] != socketBuffer[0]);
				Assert.True(socketBuffer[3] != socketBuffer[1]);
				Assert.True(socketBuffer[3] != socketBuffer[2]);

				Assert.Equal(4, (int)server.BufferUsage(out error));
				Assert.Equal(5, socketServer.BindOptions.SharedPoolSize);
				Assert.Equal(1, socketServer.m_SharedPool.Size);
				Assert.Equal(0, bufferPool.Size);

				/*
				 * decrease the sharedPoolSize to 2. Since there is only one free buffer in sharedPool,
				 * the sharedPoolSize should be reduced to 4.
				 */
				Assert.Equal(4, (int)server.IOCtl(IOCtlCode.SERVER_NUM_POOL_BUFFERS, 2, out error));
				// sharedPoolSize did not change since all buffers are in use.
				Assert.Equal(4, socketServer.BindOptions.SharedPoolSize);
				Assert.Equal(0, socketServer.m_SharedPool.Size);
				Assert.Equal(1, bufferPool.Size);

				/*
				 * release all of the buffers, then decrease the sharedPoolSize to 2. Since there are four
				 * free buffers in sharedPool, the sharedPoolSize should be reduced to 2, and the global
				 * pool will increase from 1 to 3.
				 */
				socketBuffer[3].ReturnToPool();
				socketBuffer[3] = null;
				Assert.Equal(3, (int)server.BufferUsage(out error));
				socketBuffer[2].ReturnToPool();
				socketBuffer[2] = null;
				Assert.Equal(2, (int)server.BufferUsage(out error));
				socketBuffer[1].ReturnToPool();
				socketBuffer[1] = null;
				Assert.Equal(1, (int)server.BufferUsage(out error));
				socketBuffer[0].ReturnToPool();
				socketBuffer[0] = null;
				Assert.Equal(0, (int)server.BufferUsage(out error));

				Assert.Equal(2, (int)server.IOCtl(IOCtlCode.SERVER_NUM_POOL_BUFFERS, 2, out error));
				Assert.Equal(2, socketServer.BindOptions.SharedPoolSize);
				Assert.Equal(2, socketServer.m_SharedPool.Size);
				Assert.Equal(3, bufferPool.Size);

				// call ioctl to set SERVER_NUM_POOL_BUFFERS to the same value that it already is.
				Assert.Equal(2, (int)server.IOCtl(IOCtlCode.SERVER_NUM_POOL_BUFFERS, 2, out error));
				Assert.Equal(2, socketServer.BindOptions.SharedPoolSize);
			}
			finally
			{
				if (server != null) server.Close(out error);
				Assert.Equal(TransportReturnCode.SUCCESS, Transport.Uninitialize());
			}
		}
		
		[Fact]
		public void IOCtlPriorityFlushOrderTest()
        {
			String testDataHigh = "High";
			String testDataMedium = "Medium";
			String testDataLow = "Low";
			int bufLenHigh = testDataHigh.Length;
			int bufLenMedium = testDataMedium.Length;
			int bufLenLow = testDataLow.Length;
			ByteBuffer byteBufHigh = new ByteBuffer(bufLenHigh);
			ByteBuffer byteBufMedium = new ByteBuffer(bufLenMedium);
			ByteBuffer byteBufLow = new ByteBuffer(bufLenLow);
			byteBufHigh.Put(Encoding.ASCII.GetBytes(testDataHigh));
			byteBufMedium.Put(Encoding.ASCII.GetBytes(testDataMedium));
			byteBufLow.Put(Encoding.ASCII.GetBytes(testDataLow));
			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();
			
			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = false
			};
			Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out error));

			try
            {
				MockChannel mockChannel = new MockChannel(0, 0);
				ChannelBase channel = new ChannelBase(new ConnectOptions(), mockChannel, ChannelState.ACTIVE, 10000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
				channel.m_IsJunitTest = true;

				TransportBuffer[] transBufHigh = new TransportBuffer[4];
				TransportBuffer[] transBufMedium = new TransportBuffer[4];
				TransportBuffer[] transBufLow = new TransportBuffer[4];

				for (int i = 0; i < 4; i++)
				{
					transBufHigh[i] = new TransportBuffer(bufLenHigh + 3);
					transBufHigh[i].SetDataPosition(3);
					byteBufHigh.Flip();
					transBufHigh[i].Data.Put(byteBufHigh);
					transBufHigh[i].IsReadMode = false;
					transBufHigh[i].IsOwnedByApp = true;
					transBufHigh[i].InPool = true;

					transBufMedium[i] = new TransportBuffer(bufLenMedium + 3);
					transBufMedium[i].SetDataPosition(3);
					byteBufMedium.Flip();
					transBufMedium[i].Data.Put(byteBufMedium);
					transBufMedium[i].IsReadMode = false;
					transBufMedium[i].IsOwnedByApp = true;
					transBufMedium[i].InPool = true;

					transBufLow[i] = new TransportBuffer(bufLenLow + 3);
					transBufLow[i].SetDataPosition(3);
					byteBufLow.Flip();
					transBufLow[i].Data.Put(byteBufLow);
					transBufLow[i].IsReadMode = false;
					transBufLow[i].IsOwnedByApp = true;
					transBufLow[i].InPool = true;
				}

				// Use Channel.IOCtl() to change Priority Flush Order to something invalid.
				// (Missing H)
				Assert.Equal(TransportReturnCode.FAILURE, channel.IOCtl(IOCtlCode.PRIORITY_FLUSH_ORDER, "LMLM", out error));

				// Use Channel.IOCtl() to change Priority Flush Order to something invalid.
				// (Missing M)
				Assert.Equal(TransportReturnCode.FAILURE, channel.IOCtl(IOCtlCode.PRIORITY_FLUSH_ORDER, "LHHL", out error));

				// Use Channel.IOCtl() to change Priority Flush Order to something invalid.
				// (larger than 32 characters)
				Assert.Equal(TransportReturnCode.FAILURE, channel.IOCtl(IOCtlCode.PRIORITY_FLUSH_ORDER, "HMLHMLHLMHLMHLMHLHLMHLMHLMHLMHLMHLMHLMHL", out error));

				// Use Channel.IOCtl() to change Priority Flush Order
				Assert.Equal(TransportReturnCode.SUCCESS, channel.IOCtl(IOCtlCode.PRIORITY_FLUSH_ORDER, "LMLHLM", out error));

				int cumulativeBytesQueued = 0;
				TransportReturnCode writeReturnVal = 0;

				// queue several buffers by calling the write method several times
				// with no write flags
				for (int i = 0; i < 4; i++)
				{
					// queue several buffers by calling the write method several times with no write flags
					cumulativeBytesQueued += bufLenHigh + 3;
					writeArgs.Priority = WritePriorities.HIGH;
					writeArgs.Flags = WriteFlags.NO_FLAGS;
					writeReturnVal = channel.Write(transBufHigh[i], writeArgs, out error);
					// write return value should be cumulative bytes queued
					Assert.Equal(cumulativeBytesQueued, (int)writeReturnVal);
					// bytesWritten should be scktBufHigh.getLength()
					Assert.True(writeArgs.BytesWritten == transBufHigh[i].Length() + 3);
					cumulativeBytesQueued += bufLenMedium + 3;
					writeArgs.Priority = WritePriorities.MEDIUM;
					writeArgs.Flags = WriteFlags.NO_FLAGS;
					writeReturnVal = channel.Write(transBufMedium[i], writeArgs, out error);
					// write return value should be cumulative bytes queued
					Assert.True((int)writeReturnVal == cumulativeBytesQueued);
					// bytesWritten should be scktBufMedium.getLength()
					Assert.True(writeArgs.BytesWritten == transBufMedium[i].Length() + 3);
					cumulativeBytesQueued += bufLenLow + 3;
					writeArgs.Priority = WritePriorities.LOW;
					writeArgs.Flags = WriteFlags.NO_FLAGS;
					writeReturnVal = channel.Write(transBufLow[i], writeArgs, out error);
					// write return value should be cumulative bytes queued
					Assert.True((int)writeReturnVal == cumulativeBytesQueued);
					// bytesWritten should be scktBufLow.GetLength()
					Assert.True(writeArgs.BytesWritten == transBufLow[i].Length() + 3);
				}

				// _totalBytesQueued in RsslSocketChannel should be cumulative bytes
				// queued
				Assert.True(channel.m_totalBytesQueued == cumulativeBytesQueued);

				mockChannel.SendReturnValue = cumulativeBytesQueued;
				mockChannel.SendListReturnValue = cumulativeBytesQueued;

				Assert.True(channel.Flush(out error) == TransportReturnCode.SUCCESS);

				// _totalBytesQueued in RsslSocketChannel should be 0
				Assert.True(channel.m_totalBytesQueued == 0);

				// priority queues in RsslSocketChannel should be empty
				Assert.True(channel.m_highPriorityQueue._head == null);
				Assert.True(channel.m_mediumPriorityQueue._head == null);
				Assert.True(channel.m_lowPriorityQueue._head == null);

				// verify that the _gatherWriteArray's order matches expected flush order.
				Assert.Equal(transBufLow[0].Data, channel.m_writeBufferArray[0].Data); // L
				Assert.Equal(transBufMedium[0].Data, channel.m_writeBufferArray[1].Data); // M
				Assert.Equal(transBufLow[1].Data, channel.m_writeBufferArray[2].Data); // L
				Assert.Equal(transBufHigh[0].Data, channel.m_writeBufferArray[3].Data); // H
				Assert.Equal(transBufLow[2].Data, channel.m_writeBufferArray[4].Data); // L
				Assert.Equal(transBufMedium[1].Data, channel.m_writeBufferArray[5].Data); // M
				Assert.Equal(transBufLow[3].Data, channel.m_writeBufferArray[6].Data); // L
				Assert.Equal(transBufMedium[2].Data, channel.m_writeBufferArray[7].Data); // M
				Assert.Equal(transBufHigh[1].Data, channel.m_writeBufferArray[8].Data); // H
				Assert.Equal(transBufMedium[3].Data, channel.m_writeBufferArray[9].Data); // M
				Assert.Equal(transBufHigh[2].Data, channel.m_writeBufferArray[10].Data); // H
				Assert.Equal(transBufHigh[3].Data, channel.m_writeBufferArray[11].Data); // H
			}
			finally 
			{
				Transport.Uninitialize();
            }
		}

        [Fact]
        public void IOCtlVerifyChannelReset()
        {
            Error error = new Error();
            IChannel chnl1 = null;
            IChannel chnl2 = null;
            string myFlushPriority = "LMLHLM";

            BindOptions bindOpts = GetDefaultBindOptions("14042");

            ConnectOptions connectOptions = GetDefaultConnectOptions("14042");

            InitArgs initArgs = new InitArgs
            {
                GlobalLocking = false
            };

            IServer server = null;

            try
            {
                Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out error));
                server = Transport.Bind(bindOpts, out error);
                Assert.NotNull(server);
                chnl1 = Transport.Connect(connectOptions, out error);
                ChannelBase base1 = (ChannelBase)chnl1;
                Assert.True(ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER.Equals(base1.m_ChannelInfo.PriorityFlushStrategy));
                Assert.Equal(0, base1.ComponentInfo.ComponentVersion.Length);

                // specify ComponentVersion
                String userSpecifiedComponentVersion = "User Specified Client Version 5.43";
                ByteBuffer componentVersionBB = new ByteBuffer(Encoding.ASCII.GetBytes(userSpecifiedComponentVersion));
                ComponentInfo myCi = new ComponentInfo();
                myCi.ComponentVersion.Data(componentVersionBB);
                // Assert.Equal(TransportReturnCode.SUCCESS, chnl1.IOCtl(IOCtlCode.COMPONENT_INFO, myCi, out error));

                // change flush priority order
                Assert.Equal(TransportReturnCode.SUCCESS, chnl1.IOCtl(IOCtlCode.PRIORITY_FLUSH_ORDER, myFlushPriority, out error));
                Assert.True(myFlushPriority.Equals(base1.m_ChannelInfo.PriorityFlushStrategy));

                // change High Water Mark to a small value
                Assert.Equal(TransportReturnCode.SUCCESS, chnl1.IOCtl(IOCtlCode.HIGH_WATER_MARK, 5, out error));
                Assert.Equal(5, base1.m_highWaterMark);

                // change System Read Buffer Size
                if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                {
                    Assert.Equal(TransportReturnCode.SUCCESS, chnl1.IOCtl(IOCtlCode.SYSTEM_READ_BUFFERS, 50000, out error));
                    Assert.Equal(50000, base1.Socket.ReceiveBufferSize);

                    // change System Write Buffer Size
                    Assert.Equal(TransportReturnCode.SUCCESS, chnl1.IOCtl(IOCtlCode.SYSTEM_WRITE_BUFFERS, 40000, out error));
                    Assert.Equal(40000, base1.Socket.SendBufferSize);
                }

                chnl1.Close(out error);
                server.Close(out error);

                server = Transport.Bind(bindOpts, out error);
                Assert.NotNull(server);

                chnl2 = Transport.Connect(connectOptions, out error);
                Assert.NotNull(chnl2);

                ChannelBase base2 = (ChannelBase)chnl2;

                // Verify that the component version info is reset.
                Assert.Equal(0, base2.ComponentInfo.ComponentVersion.Length);
                // Verify that the flush priority has been reset to the default value.
                Assert.True(ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER.Equals(base2.m_ChannelInfo.PriorityFlushStrategy));
                // Verify that the high water mark has been reset to the default value.
                Assert.Equal(6144, base2.m_highWaterMark);
                // Verify that the System Read Buffer size has been reset to the default value.

                if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows))
                {
                    Assert.Equal(65535, base2.Socket.ReceiveBufferSize); // Equal to READ_RECEIVE_BUFFER_SIZE in Java 
                    Assert.Equal(65535, base2.Socket.SendBufferSize);
                }
            }
            finally
            {
                chnl2?.Close(out _);
                server?.Close(out _);

                Transport.Uninitialize();
            }
        }

		[Fact]
		public void IOCtlHighWaterMarkTest()
        {
			// this testData will be used to verify
			// RsslSocketChannel.writeWithNoBuffersQueued(()
			string testData = "FlushMe";
			int bufLen = testData.Length;
			ByteBuffer byteBuf = new ByteBuffer(bufLen);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData));

			// create two testDatas that will look identical to testDat,
			// the first will have one char from testData, the second will
			// have the rest. These testDatas will be used to verify
			// RsslSocketChannel.writeWithBuffersQueued()
			string smallTestData1 = "F";
			int smallBufLen1 = smallTestData1.Length;
			ByteBuffer smallByteBuf1 = new ByteBuffer(smallBufLen1);
			smallByteBuf1.Put(Encoding.ASCII.GetBytes(smallTestData1));
			string smallTestData2 = "lushMe";
			int smallBufLen2 = smallTestData2.Length;
			ByteBuffer smallByteBuf2 = new ByteBuffer(smallBufLen2);
			smallByteBuf2.Put(Encoding.ASCII.GetBytes(smallTestData2));

			TransportReturnCode writeReturnVal = 0; 
			int cumulativeBytesQueued = bufLen + 3;
			int smallCumulativeBytesQueued = smallBufLen1 + 3 + smallBufLen2 + 3;

			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = false
			};
			Assert.Equal(TransportReturnCode.SUCCESS, Transport.Initialize(initArgs, out error));

			MockChannel mockChannel = new MockChannel(cumulativeBytesQueued, cumulativeBytesQueued);
			ChannelBase channel = new ChannelBase(new ConnectOptions(), mockChannel, ChannelState.ACTIVE, 10000, ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER);
			channel.m_IsJunitTest = true;

			// Use Channel.ioctl() to change High Water Mark to a small value
			Assert.Equal(TransportReturnCode.SUCCESS, channel.IOCtl(IOCtlCode.HIGH_WATER_MARK, 5, out error));

			// create SocketBuffer and set to test data
			TransportBuffer transBuf = new TransportBuffer(bufLen + 3);
			transBuf.SetDataPosition(3);
			byteBuf.Flip();
			transBuf.Data.Put(byteBuf);
			transBuf.IsReadMode = false;
			transBuf.InPool = true;
			transBuf.IsOwnedByApp = true;

			// call the write method with no write flags, queued bytes should
			// exceed high water mark.
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(transBuf, writeArgs, out error);
			
			// write return value should be 0 since all queued bytes were written.
			Assert.Equal(TransportReturnCode.SUCCESS, writeReturnVal);

			// The high water mark should have been hit.
			// Verify there are no bytes queued.
			Assert.Equal(0, channel.m_totalBytesQueued);

			/*
             * Do this again, but this time call write twice. The high water
             * mark should be exceeded on the second write.
             */

			mockChannel.SendReturnValue = smallCumulativeBytesQueued;
			mockChannel.SendListReturnValue = smallCumulativeBytesQueued;

			TransportBuffer smallTransBuf1 = new TransportBuffer(smallBufLen1 + 3);
			smallTransBuf1.SetDataPosition(3);
			smallByteBuf1.Flip();
			smallTransBuf1.Data.Put(smallByteBuf1);
			smallTransBuf1.IsReadMode = false;
			smallTransBuf1.InPool = true;
			smallTransBuf1.IsOwnedByApp = true;

			TransportBuffer smallTransBuf2 = new TransportBuffer(smallBufLen2 + 3);
			smallTransBuf2.SetDataPosition(3);
			smallByteBuf2.Flip();
			smallTransBuf2.Data.Put(smallByteBuf2);
			smallTransBuf2.IsReadMode = false;
			smallTransBuf2.InPool = true;
			smallTransBuf2.IsOwnedByApp = true;

			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(smallTransBuf1, writeArgs, out error);
			Assert.Equal(smallBufLen1 + 3, (int)writeReturnVal);

			// this write should exceed the high water mark
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;
			writeReturnVal = channel.Write(smallTransBuf2, writeArgs, out error);

			// The high water mark should have been hit.
			// Verify there are no bytes queued.
			Assert.Equal(0, channel.m_totalBytesQueued);

			// test invalid value
			Assert.Equal(TransportReturnCode.FAILURE, channel.IOCtl(IOCtlCode.HIGH_WATER_MARK, -9999, out error));

			Transport.Uninitialize();
		}

		[Fact]
		public void IOCtlSystemReadWriteBuffersDefaultServerTest()
        {
			InitializeConnection("14043", 10, 10, CompressionType.NONE, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);			
			
			try
            {
				ChannelBase serverBase = (ChannelBase)serverClientChannel;
				Assert.Equal(65535, serverBase.Socket.ReceiveBufferSize);
				Assert.Equal(65535, serverBase.Socket.SendBufferSize);

				int recvBufSize = 42000;
				int sendBufSize = 45000;
				Assert.Equal(TransportReturnCode.SUCCESS, server.IOCtl(IOCtlCode.SYSTEM_READ_BUFFERS, recvBufSize, out var error));
				Assert.Equal(recvBufSize, ((ServerImpl)server).Socket.ReceiveBufferSize);
				Assert.Equal(TransportReturnCode.SUCCESS, serverClientChannel.IOCtl(IOCtlCode.SYSTEM_READ_BUFFERS, recvBufSize, out error));
				Assert.Equal(recvBufSize, ((ChannelBase)serverClientChannel).Socket.ReceiveBufferSize);
				Assert.Equal(TransportReturnCode.SUCCESS, serverClientChannel.IOCtl(IOCtlCode.SYSTEM_WRITE_BUFFERS, sendBufSize, out error));
				Assert.Equal(sendBufSize, ((ChannelBase)serverClientChannel).Socket.SendBufferSize);
			} 
			finally
            {
				if (clientChannel!= null) clientChannel.Close(out var error);
				if (serverClientChannel != null) serverClientChannel.Close(out var error);
				if (server != null) server.Close(out var error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void IOCtlSystemReadWriteBuffersServerTest()
        {
			int recvBufSize = 50000;
			int sendBufSize = 40000;
			ConnectOptions conOpt = new ConnectOptions();
			BindOptions bindOpt = new BindOptions();
			InProgInfo inProgInfo = new InProgInfo();
			conOpt.UnifiedNetworkInfo.Address = "localhost";
			conOpt.ProtocolType = LSEG.Eta.Transports.ProtocolType.RWF;
			conOpt.UnifiedNetworkInfo.ServiceName = "14044";
			conOpt.ConnectionType = ConnectionType.SOCKET;
			conOpt.MajorVersion = Codec.Codec.MajorVersion();
			conOpt.MinorVersion = Codec.Codec.MinorVersion();
			conOpt.SysRecvBufSize = recvBufSize;
			conOpt.SysSendBufSize = sendBufSize;
			
			bindOpt.MinorVersion = Codec.Codec.MinorVersion();
			bindOpt.MajorVersion = Codec.Codec.MajorVersion();
			bindOpt.ServiceName = "TEST";
			bindOpt.ConnectionType = ConnectionType.SOCKET;
			bindOpt.ServiceName = "14044";
			bindOpt.ProtocolType = ProtocolType.RWF;
			bindOpt.SysRecvBufSize = recvBufSize;

			IChannel cc = null;
			IChannel sc = null;
			IServer s = null;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);

			try
            {
				s = BindServer(bindOpt);
				Assert.NotNull(s);

				Task acceptTask = Task.Factory.StartNew(delegate
				{
					InProgInfo inProg = new InProgInfo();

					s.Socket.Poll(-1, SelectMode.SelectRead);
					var acceptOptions = new AcceptOptions();
					acceptOptions.Clear();
					acceptOptions.SysSendBufSize = sendBufSize;
					acceptOptions.SysRecvBufSize = recvBufSize;

					sc = s.Accept(acceptOptions, out Error error);

					while (sc.State != ChannelState.ACTIVE)
					{
						var ret = sc.Init(inProg, out error);
						Assert.True(ret >= TransportReturnCode.SUCCESS);
					}
				});

				Task connectTask = Task.Factory.StartNew(delegate
				{
					cc = ConnectChannel(conOpt);
					Assert.NotNull(cc);
					WaitUntilChannelActive(cc, inProgInfo);
				});

				Task.WaitAll(new Task[2] { acceptTask, connectTask }, 5000);
				Assert.NotNull(s);
				Assert.NotNull(cc);
				Assert.NotNull(sc);
				Assert.True(cc.State == ChannelState.ACTIVE);
				Assert.True(sc.State == ChannelState.ACTIVE);

				ChannelBase serverBase = (ChannelBase)sc;
				Assert.Equal(sendBufSize, sc.Socket.SendBufferSize);
				Assert.Equal(recvBufSize, sc.Socket.ReceiveBufferSize);
				Assert.Equal(recvBufSize, ((ServerImpl)s).Socket.ReceiveBufferSize);

				// set ioctl SYSTEM_READ_BUFFERS with invalid data.
				Assert.Equal(TransportReturnCode.FAILURE, s.IOCtl(IOCtlCode.SYSTEM_READ_BUFFERS, -1, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
				Assert.Equal(TransportReturnCode.FAILURE, sc.IOCtl(IOCtlCode.SYSTEM_READ_BUFFERS, -1, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);

				// set ioctl SYSTEM_WRITE_BUFFERS with invalid data.
				Assert.Equal(TransportReturnCode.FAILURE, sc.IOCtl(IOCtlCode.SYSTEM_WRITE_BUFFERS, -1, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);
			} 
			finally
            {
				if (cc != null) cc.Close(out error);
				if (sc != null) sc.Close(out error);
				if (s != null) s.Close(out error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void IOCtlSystemReadWriteBuffersLargerThan64KServerTest()
        {
			int recvBufSize = 100000;
			int sendBufSize = 110000;
			ConnectOptions conOpt = new ConnectOptions();
			BindOptions bindOpt = new BindOptions();
			InProgInfo inProgInfo = new InProgInfo();
			conOpt.UnifiedNetworkInfo.Address = "localhost";
			conOpt.ProtocolType = LSEG.Eta.Transports.ProtocolType.RWF;
			conOpt.UnifiedNetworkInfo.ServiceName = "14044";
			conOpt.ConnectionType = ConnectionType.SOCKET;
			conOpt.MajorVersion = Codec.Codec.MajorVersion();
			conOpt.MinorVersion = Codec.Codec.MinorVersion();
			conOpt.SysRecvBufSize = recvBufSize;
			conOpt.SysSendBufSize = sendBufSize;

			bindOpt.MinorVersion = Codec.Codec.MinorVersion();
			bindOpt.MajorVersion = Codec.Codec.MajorVersion();
			bindOpt.ServiceName = "TEST";
			bindOpt.ConnectionType = ConnectionType.SOCKET;
			bindOpt.ServiceName = "14044";
			bindOpt.ProtocolType = ProtocolType.RWF;
			bindOpt.SysRecvBufSize = recvBufSize;

			IChannel cc = null;
			IChannel sc = null;
			IServer s = null;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = true
			};
			Transport.Initialize(initArgs, out var error);
			
			try
            {
				s = BindServer(bindOpt);
				Assert.NotNull(s);

				Task acceptTask = Task.Factory.StartNew(delegate
				{
					InProgInfo inProg = new InProgInfo();

					s.Socket.Poll(-1, SelectMode.SelectRead);
					var acceptOptions = new AcceptOptions();
					acceptOptions.Clear();
					acceptOptions.SysSendBufSize = sendBufSize;
					acceptOptions.SysRecvBufSize = recvBufSize;

					sc = s.Accept(acceptOptions, out Error error);

					while (sc.State != ChannelState.ACTIVE)
					{
						var ret = sc.Init(inProg, out error);
						Assert.True(ret >= TransportReturnCode.SUCCESS);
					}
				});

				Task connectTask = Task.Factory.StartNew(delegate
				{
					cc = ConnectChannel(conOpt);
					Assert.NotNull(cc);
					WaitUntilChannelActive(cc, inProgInfo);
				});

				Task.WaitAll(new Task[2] { acceptTask, connectTask }, 5000);
				Assert.NotNull(s);
				Assert.NotNull(cc);
				Assert.NotNull(sc);
				Assert.True(cc.State == ChannelState.ACTIVE);
				Assert.True(sc.State == ChannelState.ACTIVE);

				ChannelBase serverBase = (ChannelBase)sc;
				Assert.Equal(sendBufSize, sc.Socket.SendBufferSize);
				Assert.Equal(recvBufSize, sc.Socket.ReceiveBufferSize);
				Assert.Equal(recvBufSize, ((ServerImpl)s).Socket.ReceiveBufferSize);

				/*
				 * Use ioctl to change System Read Buffer size on server and System
				 * Read/Write Buffer size channel.
				 */
				recvBufSize = 52000;
				sendBufSize = 55000;

				Assert.Equal(TransportReturnCode.SUCCESS, s.IOCtl(IOCtlCode.SYSTEM_READ_BUFFERS, recvBufSize, out error));
				Assert.Equal(recvBufSize, ((ServerImpl)s).Socket.ReceiveBufferSize);
				Assert.Equal(TransportReturnCode.SUCCESS, sc.IOCtl(IOCtlCode.SYSTEM_READ_BUFFERS, recvBufSize, out error));
				Assert.Equal(recvBufSize, ((ChannelBase)sc).Socket.ReceiveBufferSize);
				Assert.Equal(TransportReturnCode.SUCCESS, sc.IOCtl(IOCtlCode.SYSTEM_WRITE_BUFFERS, sendBufSize, out error));
				Assert.Equal(sendBufSize, ((ChannelBase)sc).Socket.SendBufferSize);

				/*
				 * Use ioctl to change System Read Buffer size on server and System
				 * Read/Write Buffer size channel.
				 */
				recvBufSize = 128000;
				sendBufSize = 127000;
				Assert.Equal(TransportReturnCode.SUCCESS, s.IOCtl(IOCtlCode.SYSTEM_READ_BUFFERS, recvBufSize, out error));
				Assert.Equal(recvBufSize, ((ServerImpl)s).Socket.ReceiveBufferSize);
				Assert.Equal(TransportReturnCode.SUCCESS, sc.IOCtl(IOCtlCode.SYSTEM_READ_BUFFERS, recvBufSize, out error));
				Assert.Equal(recvBufSize, ((ChannelBase)sc).Socket.ReceiveBufferSize);
				Assert.Equal(TransportReturnCode.SUCCESS, sc.IOCtl(IOCtlCode.SYSTEM_WRITE_BUFFERS, sendBufSize, out error));
				Assert.Equal(sendBufSize, ((ChannelBase)sc).Socket.SendBufferSize);
			} 
			finally
            {
				s.Close(out error);

				Transport.Uninitialize();
			}
		}

		[Fact]
		public void IOCtlCompressionThresholdWriteTest()
        {
			String testData20 = "aaabbbcccdddeeefffgg";
			String testData80 = "gghhhiiijjjkkklllmmmnnnooopppqqqrrrssstttuuuvvvwwwxxxyyyzzzaaabbbcccdddeeefffggg";

			// populate byteBuf with testData20
			ByteBuffer byteBuf = new ByteBuffer(100);
			byteBuf.Put(Encoding.ASCII.GetBytes(testData20));

			WriteArgs writeArgs = new WriteArgs();
			Error error = new Error();

			InitializeConnection("14024", 10, 10, CompressionType.ZLIB, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

			try
            {
				ChannelBase channel = (ChannelBase)clientChannel;

				// set-up channel for compression
				channel.m_SessionOutCompression = CompressionType.ZLIB;
				channel.m_SessionCompLevel = 9;
				channel.m_Compressor = channel.m_ZlibCompressor;
				channel.m_Compressor.CompressionLevel = channel.m_SessionCompLevel;
				channel.m_SessionCompLowThreshold = ChannelBase.ZLIB_COMPRESSION_THRESHOLD;
				channel.m_ChannelInfo.CompressionType = CompressionType.ZLIB;

				// set m_totalBytesQueued to 0 for no buffers queued
				channel.m_totalBytesQueued = 0;

				TransportBuffer transBuf = new TransportBuffer(testData80.Length + 3);
				transBuf.SetDataPosition(3);
				byteBuf.Flip();
				transBuf.Data.Put(byteBuf);
				transBuf.IsReadMode = false;
				transBuf.IsOwnedByApp = true;
				transBuf.InPool = true;

				writeArgs.Priority = WritePriorities.HIGH;
				writeArgs.Flags = WriteFlags.NO_FLAGS;
				Assert.True(channel.Write(transBuf, writeArgs, out error) > 0);
				Assert.Equal(23, writeArgs.BytesWritten);
				Assert.Equal(23, writeArgs.UncompressedBytesWritten);

				TransportBuffer buf = (TransportBuffer)channel.m_highPriorityQueue._tail;
				ByteBuffer bb = buf.Data;
				// verify that the RIPC header has RipcFlags.Data (0x02) and is not RipcFlags.CompressedData (0x04).
				Assert.Equal(0x02, bb.Contents[2]);

				/*
				 * write over the default (30) compression threshold. The first
				 * compressed message is larger because it contains the full
				 * compression table.
				 */

				// set _totalBytesQueued to 0 for no buffers queued
				channel.m_totalBytesQueued = 0;

				// populate byteBuf with testData80
				byteBuf.Clear();
				byteBuf.Put(Encoding.ASCII.GetBytes(testData80));
				byteBuf.Flip();
				TransportBuffer transBuf1 = new TransportBuffer(100);
				transBuf1.IsReadMode = false;
				transBuf1.SetDataPosition(3);
				transBuf1.Data.Put(byteBuf);
				transBuf1.IsOwnedByApp = true;
				transBuf1.InPool = true;

				writeArgs.Priority = WritePriorities.HIGH;
				writeArgs.Flags = WriteFlags.NO_FLAGS;
				Assert.True(channel.Write(transBuf1, writeArgs, out error) > 0);
				Assert.Equal(75, writeArgs.BytesWritten);
				Assert.Equal(83, writeArgs.UncompressedBytesWritten);
				Assert.False(transBuf1.IsOwnedByApp);

				// end of queue should contain compressed message
				buf = (TransportBuffer)channel.m_highPriorityQueue._tail;
				bb = buf.Data;
				// verify that the RIPC header has RipcFlags.CompressedData (0x04)
				// and is not RipcFlags.Data (0x02).
				Assert.Equal(0x04, bb.Contents[2]);

				/*
				 * write over the default (30) compression threshold once more so
				 * that we see the compression in effect.
				 */

				// set _totalBytesQueued to 0 for no buffers queued
				channel.m_totalBytesQueued = 0;

				// populate byteBuf with testData80
				byteBuf.Clear();
				byteBuf.Put(Encoding.ASCII.GetBytes(testData80));
				byteBuf.Flip();
				transBuf1 = new TransportBuffer(100);
				transBuf1.IsReadMode = false;
				transBuf1.SetDataPosition(3);
				transBuf1.Data.Put(byteBuf);
				transBuf1.IsOwnedByApp = true;
				transBuf1.InPool = true;

				writeArgs.Priority = WritePriorities.HIGH;
				writeArgs.Flags = WriteFlags.NO_FLAGS;
				Assert.True(channel.Write(transBuf1, writeArgs, out error) > 0);
				Assert.Equal(13, writeArgs.BytesWritten);
				Assert.Equal(83, writeArgs.UncompressedBytesWritten);
				Assert.False(transBuf.IsOwnedByApp);

				// end of queue should contain compressed message
				buf = (TransportBuffer)channel.m_highPriorityQueue._tail;
				bb = buf.Data;
				// verify that the RIPC header has RipcFlags.CompressedData (0x04)
				// and is not RipcFlags.Data (0x02).
				Assert.Equal(0x04, bb.Contents[2]);

				/*
				 * Adjust the compression threshold to 100 and write under the
				 * compression threshold. Verify that the data was not compressed.
				 */

				Assert.Equal(TransportReturnCode.SUCCESS, clientChannel.IOCtl(IOCtlCode.COMPRESSION_THRESHOLD, 100, out error));

				// set _totalBytesQueued to 0 for no buffers queued
				channel.m_totalBytesQueued = 0;

				// populate byteBuf with testData80
				byteBuf.Clear();
				byteBuf.Put(Encoding.ASCII.GetBytes(testData80));
				byteBuf.Flip();
				transBuf1 = new TransportBuffer(100);
				transBuf1.IsReadMode = false;
				transBuf1.SetDataPosition(3);
				transBuf1.Data.Put(byteBuf);
				transBuf1.IsOwnedByApp = true;
				transBuf1.InPool = true;

				writeArgs.Priority = WritePriorities.HIGH;
				writeArgs.Flags = WriteFlags.NO_FLAGS;
				Assert.True(channel.Write(transBuf1, writeArgs, out error) > 0);
				Assert.Equal(83, writeArgs.BytesWritten);
				Assert.Equal(83, writeArgs.UncompressedBytesWritten);
				Assert.False(transBuf.IsOwnedByApp);

				// end of queue should contain compressed message
				buf = (TransportBuffer)channel.m_highPriorityQueue._tail;
				bb = buf.Data;
				// verify that the RIPC header has RipcFlags.Data (0x02)
				// and is not RipcFlags.CompressedData (0x05).
				Assert.Equal(0x02, bb.Contents[2]);


				// verify that Channel.ioctl(COMPRESSION_THRESHOLD) passes if value equals 30 (the minimum).
				Assert.Equal(TransportReturnCode.SUCCESS, clientChannel.IOCtl(IOCtlCode.COMPRESSION_THRESHOLD, 30, out error));

				// verify that Channel.ioctl(COMPRESSION_THRESHOLD) fails if value
				// is less than 30 (the minimum)
				Assert.Equal(TransportReturnCode.FAILURE, clientChannel.IOCtl(IOCtlCode.COMPRESSION_THRESHOLD, 29, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);

				Assert.Equal(TransportReturnCode.FAILURE, clientChannel.IOCtl(IOCtlCode.COMPRESSION_THRESHOLD, 0, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);

				Assert.Equal(TransportReturnCode.FAILURE, clientChannel.IOCtl(IOCtlCode.COMPRESSION_THRESHOLD, -1, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);

				// change compression type to LZ4 and try same COMPRESSION_THRESHOLD tests which should fail
				channel.m_SessionOutCompression = CompressionType.LZ4;
				channel.m_SessionCompLevel = 9;
				channel.m_Compressor = channel.m_Lz4Compressor;
				channel.m_Compressor.CompressionLevel = channel.m_SessionCompLevel;
				channel.m_SessionCompLowThreshold = ChannelBase.LZ4_COMPRESSION_THRESHOLD;
				channel.m_IsJunitTest = true;

				Assert.Equal(TransportReturnCode.FAILURE, clientChannel.IOCtl(IOCtlCode.COMPRESSION_THRESHOLD, 29, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);

				Assert.Equal(TransportReturnCode.FAILURE, clientChannel.IOCtl(IOCtlCode.COMPRESSION_THRESHOLD, 0, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);

				Assert.Equal(TransportReturnCode.FAILURE, clientChannel.IOCtl(IOCtlCode.COMPRESSION_THRESHOLD, -1, out error));
				Assert.Equal(TransportReturnCode.FAILURE, error.ErrorId);

				// now try 30 with LZ4 which should pass
				Assert.Equal(TransportReturnCode.SUCCESS, clientChannel.IOCtl(IOCtlCode.COMPRESSION_THRESHOLD, 30, out error));
			} 
			finally
            {
				if (clientChannel != null) clientChannel.Close(out error);
				if (serverClientChannel != null) serverClientChannel.Close(out error);
				if (server != null) server.Close(out error);

				Transport.Uninitialize();
			}
		}

        #endregion

        #region ChannelBase.ToString Tests

        [Fact]
        public void ChannelBaseToStringTest1()
        {
            BindOptions bindOpts = new BindOptions();
            ConnectOptions connectOpts = new ConnectOptions();
            Error error = new Error();
            WriteArgs writeArgs = new WriteArgs();
            writeArgs.Priority = WritePriorities.HIGH;
            writeArgs.Flags = WriteFlags.NO_FLAGS;

            bindOpts.ServiceName = "14056";
            bindOpts.MinorVersion = Codec.Codec.MinorVersion();
            bindOpts.MajorVersion = Codec.Codec.MajorVersion();
            bindOpts.ProtocolType = ProtocolType.RWF;
            bindOpts.PingTimeout = 75;

            connectOpts.UnifiedNetworkInfo.Address = "localhost";
            connectOpts.ProtocolType = ProtocolType.RWF;
            connectOpts.UnifiedNetworkInfo.ServiceName = "14056";
            connectOpts.ConnectionType = ConnectionType.SOCKET;
            connectOpts.MajorVersion = Codec.Codec.MajorVersion();
            connectOpts.MinorVersion = Codec.Codec.MinorVersion();
            connectOpts.PingTimeout = 75;

            IChannel clientChannel = null;
            IChannel serverClientChannel = null;
            IServer server = null;

            try
            {
                InitializeConnection(bindOpts, connectOpts, out clientChannel, out serverClientChannel, out server);

                // server channel test
                string serverChannelString = serverClientChannel.ToString();
                Assert.Contains("connectionType: SOCKET", serverChannelString);
                Assert.Contains("connected: True", serverChannelString);
                Assert.Contains("state: ACTIVE", serverChannelString);
                Assert.Contains("pingTimeout: 75", serverChannelString);
                Assert.Contains("majorVersion: 14", serverChannelString);
                Assert.Contains("minorVersion: 1", serverChannelString);
                Assert.Contains("protocolType: RWF", serverChannelString);

                // client channel test
                string clientChannelString = clientChannel.ToString();
                Assert.Contains("connectionType: SOCKET", clientChannelString);
                Assert.Contains("connected: True", clientChannelString);
                Assert.Contains("state: ACTIVE", clientChannelString);
                Assert.Contains("pingTimeout: 75", clientChannelString);
                Assert.Contains("majorVersion: 14", clientChannelString);
                Assert.Contains("minorVersion: 1", clientChannelString);
                Assert.Contains("protocolType: RWF", clientChannelString);

                // server test
                string serverString = server.ToString();
                Assert.Contains("state: ACTIVE", serverString);
                Assert.Contains("portNumber: 14056", serverString);
                Assert.Contains("srvrSckt: " + server.Socket.Handle.ToInt64(), serverString);
                Assert.Contains("userSpecObject: ", serverString);
            }
            finally
            {
                if (clientChannel != null) clientChannel.Close(out error);
                if (serverClientChannel != null) serverClientChannel.Close(out error);
                if (server != null) server.Close(out error);

                Transport.Uninitialize();
            }
        }

        [Fact]
		public void ChannelBaseToStringTest2()
		{
			BindOptions bindOpts = new BindOptions();
			ConnectOptions connectOpts = new ConnectOptions();
			Error error = new Error();
			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;

			bindOpts.ServiceName = "14057";
			bindOpts.MinorVersion = Codec.Codec.MinorVersion();
			bindOpts.MajorVersion = Codec.Codec.MajorVersion();
			bindOpts.ProtocolType = ProtocolType.RWF;
			bindOpts.PingTimeout = 92;

			connectOpts.UnifiedNetworkInfo.Address = "localhost";
			connectOpts.ProtocolType = ProtocolType.RWF;
			connectOpts.UnifiedNetworkInfo.ServiceName = "14057";
			connectOpts.ConnectionType = ConnectionType.SOCKET;
			connectOpts.MajorVersion = Codec.Codec.MajorVersion();
			connectOpts.MinorVersion = Codec.Codec.MinorVersion();
			connectOpts.PingTimeout = 92;

			InProgInfo inProgInfo = new InProgInfo();
			IChannel cc = null;
			IServer s = null;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = false
			};

            try
            {
                Transport.Initialize(initArgs, out error);
                s = BindServer(bindOpts);
                Assert.NotNull(s);
                cc = ConnectChannel(connectOpts);

                // client channel test
                string clientChannelString = cc.ToString();
                Assert.Contains("connectionType: SOCKET", clientChannelString);
                Assert.Contains("connected: True", clientChannelString);
                Assert.Contains("state: INITIALIZING", clientChannelString);
                Assert.Contains("pingTimeout: 92", clientChannelString);
                Assert.Contains("majorVersion: 14", clientChannelString);
                Assert.Contains("minorVersion: 1", clientChannelString);
                Assert.Contains("protocolType: RWF", clientChannelString);

                // server test
                string serverString = s.ToString();
                Assert.Contains("state: ACTIVE", serverString);
                Assert.Contains("portNumber: 14057", serverString);
                Assert.Contains("srvrSckt: " + s.Socket.Handle.ToInt64(), serverString);
                Assert.Contains("userSpecObject: ", serverString);

            }
            finally
            {
                if (cc != null) cc.Close(out error);
                if (s != null) s.Close(out error);

                Transport.Uninitialize();
            }
		}

		[Fact]
		public void ChannelBaseToStringTest3()
		{
			BindOptions bindOpts = new BindOptions();
			ConnectOptions connectOpts = new ConnectOptions();
			Error error = new Error();
			WriteArgs writeArgs = new WriteArgs();
			writeArgs.Priority = WritePriorities.HIGH;
			writeArgs.Flags = WriteFlags.NO_FLAGS;

			bindOpts.ServiceName = "14056";
			bindOpts.MinorVersion = Codec.Codec.MinorVersion();
			bindOpts.MajorVersion = Codec.Codec.MajorVersion();
			bindOpts.ProtocolType = ProtocolType.RWF;
			bindOpts.PingTimeout = 75;

			connectOpts.UnifiedNetworkInfo.Address = "localhost";
			connectOpts.ProtocolType = ProtocolType.RWF;
			connectOpts.UnifiedNetworkInfo.ServiceName = "14056";
			connectOpts.ConnectionType = ConnectionType.SOCKET;
			connectOpts.MajorVersion = Codec.Codec.MajorVersion();
			connectOpts.MinorVersion = Codec.Codec.MinorVersion();
			connectOpts.PingTimeout = 75;

            try
            {
                InitializeConnection(bindOpts, connectOpts, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server);

                Assert.NotNull(clientChannel);
                Assert.NotNull(serverClientChannel);
                Assert.NotNull(server);

                clientChannel.Close(out error);
                serverClientChannel.Close(out error);
                server.Close(out error);

                // server channel test
                string serverChannelString = serverClientChannel.ToString();
                Assert.Contains("connectionType: SOCKET", serverChannelString);
                Assert.Contains("connected: False", serverChannelString);
                Assert.Contains("state: CLOSED", serverChannelString);
                Assert.Contains("pingTimeout: 75", serverChannelString);
                Assert.Contains("majorVersion: 14", serverChannelString);
                Assert.Contains("minorVersion: 1", serverChannelString);
                Assert.Contains("protocolType: RWF", serverChannelString);

                // client channel test
                string clientChannelString = clientChannel.ToString();
                Assert.Contains("connectionType: SOCKET", clientChannelString);
                Assert.Contains("connected: False", clientChannelString);
                Assert.Contains("state: CLOSED", clientChannelString);
                Assert.Contains("pingTimeout: 75", clientChannelString);
                Assert.Contains("majorVersion: 14", clientChannelString);
                Assert.Contains("minorVersion: 1", clientChannelString);
                Assert.Contains("protocolType: RWF", clientChannelString);

                // server test
                string serverString = server.ToString();
                Assert.Contains("state: CLOSED", serverString);
                Assert.Contains("portNumber: 14056", serverString);
                Assert.Contains("srvrSckt: null", serverString);
                Assert.Contains("userSpecObject: ", serverString);

            }
            finally
            {
                Transport.Uninitialize();
            }
		}

		#endregion


		#region Helper Functions

		private void GetAndVerifyServerInfo(IServer server, ServerInfo serverInfo, Error error, int current, int peak)
		{
			Assert.Equal(current, (int)server.BufferUsage(out error));
			Assert.Equal(TransportReturnCode.SUCCESS, server.Info(serverInfo, out error));
			Assert.NotNull(serverInfo);
			Assert.Equal(current, serverInfo.CurrentBufferUsage);
			Assert.Equal(peak, serverInfo.PeakBufferUsage);
		}

		private void RIPCVersionFallback(ConnectionsVersions version)
        {
			ChannelBase cc = null;
			IChannel sc = null;
			IServer s = null;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = false
			};
			Transport.Initialize(initArgs, out var error);

			BindOptions bindOpts = new BindOptions();
			bindOpts.ServiceName = "14077";
			bindOpts.ChannelIsBlocking = true;
			bindOpts.MinorVersion = Codec.Codec.MinorVersion();
			bindOpts.MajorVersion = Codec.Codec.MajorVersion();
			bindOpts.ProtocolType = ProtocolType.RWF;
			bindOpts.MaxOutputBuffers = 10;
			bindOpts.SharedPoolSize = 100;
			bindOpts.GuaranteedOutputBuffers = 5;
			bindOpts.MaxFragmentSize = 6144;

			s = BindServer(bindOpts);
			Assert.NotNull(s);

			Task serverTask = Task.Factory.StartNew(delegate
			{
				sc = WaitUntilServerAcceptNoInit(s);
				Assert.NotNull(sc);
			});

			Task clientTask = Task.Factory.StartNew(delegate
			{
				ConnectOptions connectOpts = new ConnectOptions();
				connectOpts.UnifiedNetworkInfo.Address = "localhost";
				connectOpts.ProtocolType = ProtocolType.RWF;
				connectOpts.UnifiedNetworkInfo.Port = 14077;
				connectOpts.ConnectionType = ConnectionType.SOCKET;
				connectOpts.MajorVersion = Codec.Codec.MajorVersion();
				connectOpts.MinorVersion = Codec.Codec.MinorVersion();
				connectOpts.NumInputBuffers = 10;
				connectOpts.Blocking = true;
				cc = new ChannelBase(connectOpts, new SocketChannel(), ChannelState.INITIALIZING, 1000,
					ChannelBase.DEFAULT_PRIORITY_FLUSH_ORDER, ChannelBase.DEFAULT_MAX_FRAGMENT_SIZE, RipcVersions.VERSION14);
				cc.StartingConnectVersion(version);

				Assert.Equal(TransportReturnCode.SUCCESS, cc.Connect(out Error error));

				Thread.Sleep(10);
			});

			Task.WaitAll(new Task[] { serverTask, clientTask }, 1000);
			Assert.NotNull(cc);
			Assert.NotNull(sc);

			Assert.True(cc.State == ChannelState.ACTIVE);
			Assert.True(sc.State == ChannelState.ACTIVE);

			cc.Close(out error);
			sc.Close(out error);
			s.Close(out error);

			Transport.Uninitialize();
		}

		private ConnectOptions GetDefaultConnectOptions(string port)
		{
			ConnectOptions connectOpts = new ConnectOptions();
			connectOpts.UserSpecObject = "TEST CHANNEL";
			connectOpts.UnifiedNetworkInfo.Address = "localhost";
			connectOpts.UnifiedNetworkInfo.ServiceName = port;
			connectOpts.MajorVersion = Codec.Codec.MajorVersion();
			connectOpts.MinorVersion = Codec.Codec.MinorVersion();
			connectOpts.ProtocolType = (ProtocolType)Codec.Codec.ProtocolType();
			return connectOpts;
		}

		private BindOptions GetDefaultBindOptions(string port)
		{
			BindOptions bindOpts = new BindOptions();
			bindOpts.UserSpecObject = "TEST SERVER";
			bindOpts.ServiceName = port;
			bindOpts.MajorVersion = Codec.Codec.MajorVersion();
			bindOpts.MinorVersion = Codec.Codec.MinorVersion();
			bindOpts.ProtocolType = (ProtocolType)Codec.Codec.ProtocolType();
			return bindOpts;
		}

		private IChannel ConnectChannel(ConnectOptions options)
        {
            return Transport.Connect(options, out Error error);
        }

        private IChannel WaitUntilChannelActive(IChannel channel, InProgInfo inProg)
        {
            IChannel ch = channel;
            Assert.NotNull(channel);
            while (ch.State != ChannelState.ACTIVE)
            {
                var ret = ch.Init(inProg, out Error error);
                Assert.True(ret >= TransportReturnCode.SUCCESS);
                Thread.Sleep(10);
            }

            return ch;
        }

        private IServer BindServer(BindOptions bindOptions)
        {
            return Transport.Bind(bindOptions, out Error error);
        }

        private IChannel WaitUntilServerAccept(IServer server)
        {
            Socket socket = server.Socket;
            IChannel channel = null;
            InProgInfo inProg = new InProgInfo();
			socket.Poll(-1, SelectMode.SelectRead);
			var m_AcceptOptions = new AcceptOptions();
			m_AcceptOptions.Clear();
			channel = server.Accept(m_AcceptOptions, out Error error);

			while (channel.State != ChannelState.ACTIVE)
            {
                var ret = channel.Init(inProg, out error);
                Assert.True(ret >= TransportReturnCode.SUCCESS);
            }

            return channel;
        }

		private IChannel WaitUntilServerAcceptNoInit(IServer server)
		{
			try
            {
				var ready = server.Socket.Poll(-1, SelectMode.SelectRead);
				var m_AcceptOptions = new AcceptOptions();
				m_AcceptOptions.Clear();
				return server.Accept(m_AcceptOptions, out Error error);
			} catch (Exception e)
            {
				Console.WriteLine(e.Message);
				throw new Exception(e.Message);
            }			
		}

		private void InitializeConnection(string port, int guaranteedOutBuf, int inputBuffers, CompressionType compType, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server, bool globalLocking = true, bool blocking = false)
        {
			ConnectOptions conOpt = new ConnectOptions();
			BindOptions bindOpt = new BindOptions();
			InProgInfo inProgInfo = new InProgInfo();
			conOpt.UnifiedNetworkInfo.Address = "localhost";
			conOpt.ProtocolType = LSEG.Eta.Transports.ProtocolType.RWF;
			conOpt.UnifiedNetworkInfo.ServiceName = port;
			conOpt.ConnectionType = ConnectionType.SOCKET;
			conOpt.MajorVersion = Codec.Codec.MajorVersion();
			conOpt.MinorVersion = Codec.Codec.MinorVersion();
			conOpt.NumInputBuffers = inputBuffers;
			conOpt.CompressionType = compType;
			conOpt.Blocking = blocking;
			bindOpt.MinorVersion = Codec.Codec.MinorVersion();
			bindOpt.MajorVersion = Codec.Codec.MajorVersion();
			bindOpt.ServiceName = "TEST";
			bindOpt.ConnectionType = ConnectionType.SOCKET;
			bindOpt.ServiceName = port;
			bindOpt.ProtocolType = ProtocolType.RWF;
			bindOpt.GuaranteedOutputBuffers = guaranteedOutBuf;
			bindOpt.CompressionType = compType;
			bindOpt.ChannelIsBlocking = blocking;

			IChannel cc = null;
			IChannel sc = null;
			IServer s = null;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = globalLocking
			};
			Transport.Initialize(initArgs, out var error);
			s = BindServer(bindOpt);
			Assert.NotNull(s);
			Task acceptTask = Task.Factory.StartNew(delegate
			{
				sc = WaitUntilServerAccept(s);
			});
			Task connectTask = Task.Factory.StartNew(delegate
			{
				cc = ConnectChannel(conOpt);
				Assert.NotNull(cc);
				WaitUntilChannelActive(cc, inProgInfo);
			});
			Task.WaitAll(new Task[2] { acceptTask, connectTask }, 10000);
			Assert.NotNull(s);
			Assert.NotNull(cc);
			Assert.NotNull(sc);
			Assert.True(cc.State == ChannelState.ACTIVE);
			Assert.True(sc.State == ChannelState.ACTIVE);

			clientChannel = cc;
			serverClientChannel = sc;
			server = s;
		}

		private void InitializeConnection(BindOptions bindOpts, ConnectOptions connectOpts, out IChannel clientChannel, out IChannel serverClientChannel, out IServer server, bool globalLocking = false)
		{
			InProgInfo inProgInfo = new InProgInfo();
			IChannel cc = null;
			IChannel sc = null;
			IServer s = null;

			InitArgs initArgs = new InitArgs
			{
				GlobalLocking = globalLocking
			};
			Transport.Initialize(initArgs, out var error);
			s = BindServer(bindOpts);
			Assert.NotNull(s);
			Task acceptTask = Task.Factory.StartNew(delegate
			{
				sc = WaitUntilServerAccept(s);
			});
			Task connectTask = Task.Factory.StartNew(delegate
			{
				cc = ConnectChannel(connectOpts);
				Assert.NotNull(cc);
				WaitUntilChannelActive(cc, inProgInfo);
			});
			Task.WaitAll(new Task[2] { acceptTask, connectTask }, 10000);
			Assert.NotNull(s);
			Assert.NotNull(cc);
			Assert.NotNull(sc);
			Assert.True(cc.State == ChannelState.ACTIVE);
			Assert.True(sc.State == ChannelState.ACTIVE);

			clientChannel = cc;
			serverClientChannel = sc;
			server = s;
		}

		private int FirstPackedHeaderLength()
		{
			return RipcLengths.HEADER + RipcOffsets.PACKED_MSG_DATA;
		}

		private int InternalFragLength(ChannelBase channel)
        {
			if (channel.RipcVersion >= RipcVersions.VERSION13)
			{
				return RipcOffsets.FRAGMENTED_ID_LENGTH_RIPC13;
			}
			else
			{
				return RipcOffsets.FRAGMENTED_ID_LENGTH_RIPC12;
			}
		}

		private int FirstFragmentHeaderLength(ChannelBase channel)
		{
			return 8 + InternalFragLength(channel);
		}

		private int AdditionalFragmentHeaderLength(ChannelBase channel)
		{
			return 4 + InternalFragLength(channel);
		}

		#endregion

		#region Mock Objects

		internal class MockChannel : ISocketChannel, IDisposable
		{
			public MockChannel(int v1, int v2)
			{
				SendReturnValue = v1;
				SendListReturnValue = v2;
			}

			public int SendReturnValue;
			public int SendListReturnValue;
			public bool ThrowExceptionOnSend = false;

			public bool IsConnected { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
			public bool IsDisposed => false;
			public bool IsDataReady { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
			public EndPoint RemoteEP => throw new NotImplementedException();
			public int RemotePort => throw new NotImplementedException();
			public Socket Socket => null;
			public TcpClient TcpClient => throw new NotImplementedException();
			public SslStream SslStream { get => throw new NotImplementedException(); set => throw new NotImplementedException(); }
			public bool IsEncrypted => false;

			public bool IsAuthenFailure => false;

			public string AuthenFailureMessage { get => string.Empty; }

			public bool Connect(ConnectOptions connectOptions, out Error error) { error = null; return true; }
			public bool Connect(ConnectOptions connectOptions, IPAddress remoteAddr, int port, out Error error) { error = null; return true; }
			public void Disconnect() { }
			public void Dispose() { }
			public bool FinishConnect(ChannelBase channel) => true;
			public int Receive(ResultObject resultObject, out Error error)
			{
				throw new NotImplementedException();
			}

			public int Receive(ByteBuffer dstBuffer, out SocketError socketError)
			{
				throw new NotImplementedException();
			}

			public void Send(byte[] packet, object user_state = null)
			{
				throw new NotImplementedException();
			}

			public int Send(byte[] buffer, int position, int length, out Error error) { error = null; return SendFunc(buffer, position, length, ThrowExceptionOnSend, SendReturnValue); }

            public int Send(IList<ArraySegment<byte>> buffers, out Error error) { error = null; return SendListFunc(buffers, ThrowExceptionOnSend, SendReturnValue); }

			public void SetReadWriteHandlers(bool isSslStream)
            {
                throw new NotImplementedException();
            }

            bool ISocketChannel.Connect(ConnectOptions connectOptions, IPAddress remoteAddr, int port, bool isProxyEnabled, out Error error)
            {
                throw new NotImplementedException();
            }

            void ISocketChannel.PostProxyInit()
            {
                throw new NotImplementedException();
            }

            public Func<byte[], int, int, bool, int, int> SendFunc = 
				(array, pos, length, throwExceptionOnSend, sendReturnValue) =>
					{ if (!throwExceptionOnSend) return sendReturnValue; else throw new Exception(); };

			public Func<IList<ArraySegment<byte>>, bool, int, int> SendListFunc =
				(buffers, throwExceptionOnSend, sendListReturnValue) =>
				{ if (!throwExceptionOnSend) return sendListReturnValue; else throw new Exception(); };

		}

	
        #endregion
    }
}
