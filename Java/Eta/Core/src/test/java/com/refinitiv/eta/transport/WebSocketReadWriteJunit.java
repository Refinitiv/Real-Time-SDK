/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import static com.refinitiv.eta.transport.WebSocketFrameParser._WS_MAX_HEADER_LEN;
import static org.junit.Assert.*;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Objects;
import java.util.zip.DataFormatException;
import java.util.zip.Deflater;
import java.util.zip.DeflaterOutputStream;
import java.util.zip.Inflater;

import org.junit.Test;

import com.refinitiv.eta.codec.Codec;

public class WebSocketReadWriteJunit
{
	String JSON_MSG_1 = "{ \"Fields\":{ \"PROD_PERM\":1, \"ASK\":101.54, \"BID\":101.5, \"ASKSIZE\":18, \"BIDSIZE\":18 }, \"Id\":555, \"Type\":\"Update\", \"UpdateType\":\"Quote\" }";
	ByteBuffer JSON_MSG_1_ByteBuffer = ByteBuffer.allocate(JSON_MSG_1.length());
	
	String JSON_MSG_1_READ = "[{ \"Fields\":{ \"PROD_PERM\":1, \"ASK\":101.54, \"BID\":101.5, \"ASKSIZE\":18, \"BIDSIZE\":18 }, \"Id\":555, \"Type\":\"Update\", \"UpdateType\":\"Quote\" }]";
	ByteBuffer JSON_MSG_1_READ_ByteBuffer = ByteBuffer.allocate(JSON_MSG_1_READ.length());
	
	String JSON_MSG_2 = "{ \"Fields\":{ \"ASK\":1201.54, \"BID\":1201.5, \"BIDSIZE\":200 }, \"Id\":999, \"Type\":\"Update\", \"UpdateType\":\"Quote\" }";
	ByteBuffer JSON_MSG_2_ByteBuffer = ByteBuffer.allocate(JSON_MSG_2.length());
	
	String PACKED_JSON_MSG_1_MSG_2 = "[" + JSON_MSG_1 + "," + JSON_MSG_2 + "]";
	ByteBuffer PACKED_JSON_MSG_1_MSG_2_ByteBuffer = ByteBuffer.allocate(PACKED_JSON_MSG_1_MSG_2.length());
	
	String RWF_MSG_1 = "123456RWFEncodedBufferxxxfweirfsdfkjl";
	ByteBuffer RWF_MSG_1_ByteBuffer = ByteBuffer.allocate(RWF_MSG_1.length());
	
	String RWF_MSG_2 = "999999999999988888888888888888998eqwewqerqwer9qwtqfafjakefjaskerjfra;9345353453433243324-fasdfj0000000NULL";
	ByteBuffer RWF_MSG_2_ByteBuffer = ByteBuffer.allocate(RWF_MSG_2.length());
	
	ByteBuffer PACKED_RWF_MSG_1_MSG_2_ByteBuffer = ByteBuffer.allocate(2 + RWF_MSG_1.length() + 2 +  RWF_MSG_2.length());
	
	ByteBuffer PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER = ByteBuffer.allocate(3 + 2 + RWF_MSG_1.length() + 2 +  RWF_MSG_2.length());
	
	ByteBuffer COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER = ByteBuffer.allocate(3 + 2 + RWF_MSG_1.length() + 2 +  RWF_MSG_2.length());
	
	{
		JSON_MSG_1_ByteBuffer.put(JSON_MSG_1.getBytes());
		JSON_MSG_1_ByteBuffer.rewind();
		
		JSON_MSG_1_READ_ByteBuffer.put(JSON_MSG_1_READ.getBytes());
		JSON_MSG_1_READ_ByteBuffer.rewind();
		
		JSON_MSG_2_ByteBuffer.put(JSON_MSG_2.getBytes());
		JSON_MSG_2_ByteBuffer.rewind();
		
		RWF_MSG_1_ByteBuffer.put(RWF_MSG_1.getBytes());
		RWF_MSG_1_ByteBuffer.rewind();
		
		RWF_MSG_2_ByteBuffer.put(RWF_MSG_2.getBytes());
		RWF_MSG_2_ByteBuffer.rewind();
		
		PACKED_JSON_MSG_1_MSG_2_ByteBuffer.put(PACKED_JSON_MSG_1_MSG_2.getBytes());
		PACKED_JSON_MSG_1_MSG_2_ByteBuffer.rewind();
		
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer.putShort((short)RWF_MSG_1.length());
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer.put(RWF_MSG_1.getBytes());
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer.putShort((short)RWF_MSG_2.length());
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer.put(RWF_MSG_2.getBytes());
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer.rewind();
		
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short)PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit());
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put((byte)(RsslSocketChannel.IPC_DATA | RsslSocketChannel.IPC_PACKING));
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short)RWF_MSG_1.length());
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put(RWF_MSG_1.getBytes());
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short)RWF_MSG_2.length());
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put(RWF_MSG_2.getBytes());
		PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.rewind();
		
		COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short)COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit());
		COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put((byte)(RsslSocketChannel.IPC_PACKING | Ripc.Flags.COMPRESSION));
		COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short)RWF_MSG_1.length());
		COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put(RWF_MSG_1.getBytes());
		COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.putShort((short)RWF_MSG_2.length());
		COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.put(RWF_MSG_2.getBytes());
		COMPRESSED_PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.rewind();
	}
	
	/* Provides ZLIB compression functionality to verify the result of testing */ 
	class ZlibCompressorTest
	{
		private Deflater _deflater;
	    private Inflater _inflater;
	    private DeflaterOutputStream _deflaterOutputStream;
	    private ByteArrayOutputStream _compressedBytesOutputStream;
	    private byte[] _compressedBytes;
	    private byte[] _decompressedBytes;
	    private byte[] _compressByteArray;
	    private int _numBytesAfterDecompress;
	    private int _maxCompressionLen;
	    private boolean _appendTrailing;
	    private byte[] _endingTrailing;
	    
	    ZlibCompressorTest(boolean appendTrailing)
	    {
	    	_deflater = new Deflater();
	        _inflater = new Inflater();
	        _numBytesAfterDecompress = 0;
	        _maxCompressionLen = 6144;
	        _deflater.setLevel(6);
	        _appendTrailing = appendTrailing;
	        _endingTrailing = new byte[4];
	        _endingTrailing[0] = 0;
	        _endingTrailing[1] = 0;
	        _endingTrailing[2] = -1;
	        _endingTrailing[3] = -1;
	    }
	    
	    int decompress(ByteBuffer bufferToDecompress, int offSet, ByteBuffer decompressedBuffer, int lenToDecompress)
	    {
	    	// lazily initialize _decompressedBytes buffer since we don't know size up front
	        if (_decompressedBytes == null)
	        {
	            _decompressedBytes = new byte[_maxCompressionLen];
	        }
	        
	        // copy bufferToDecompress contents to byte array
	        byte[] byteArray = new byte[lenToDecompress + 4];
	        int contentStartPos = offSet;
	        int i = 0;
	        for (; i < lenToDecompress; i++)
	        {
	            byteArray[i] = bufferToDecompress.get(contentStartPos + i);
	        }
	        
	        if(_appendTrailing)
	        {
	        	lenToDecompress += 4;
	        	for(int j = 0; j < 4; j++)
	        	{
	        		byteArray[i + j] = _endingTrailing[j];
	        	}
	        }
	        
	        _inflater.setInput(byteArray, 0, lenToDecompress);
	        try
	        {
	            _numBytesAfterDecompress = _inflater.inflate(_decompressedBytes);
	            decompressedBuffer.clear();
	            decompressedBuffer.put(_decompressedBytes, 0, _numBytesAfterDecompress);
	            decompressedBuffer.limit(decompressedBuffer.position());
	            decompressedBuffer.position(0);
	        }
	        catch (DataFormatException e)
	        {
	            throw new CompressorException(e.getLocalizedMessage());
	        }

	        return _numBytesAfterDecompress;
	    }
	    
	    int compress(ByteBuffer bufferToCompress, int dataStartPos, int lenToCompress)
	    {
	        // lazily initialize _compressedBytesOutputStream buffer since we don't know size up front
	        if (_compressedBytesOutputStream == null)
	        {
	            _compressedBytesOutputStream = new ByteArrayOutputStream(getMaxCompressedLength(_maxCompressionLen));
	        }
	        // lazily initialize _deflaterOutputStream buffer since we don't know size up front
	        if (_deflaterOutputStream == null)
	        {
	            _deflaterOutputStream = new DeflaterOutputStream(_compressedBytesOutputStream, _deflater,
	                                                             getMaxCompressedLength(_maxCompressionLen), true);
	        }
	        // lazily initialize _compressByteArray buffer since we don't know size up front
	        if (_compressByteArray == null)
	        {
	            _compressByteArray = new byte[_maxCompressionLen];
	        }
	        // copy bufferToCompress contents to _compressByteArray
	        for (int i = 0; i < lenToCompress; i++)
	        {
	            _compressByteArray[i] = bufferToCompress.get(dataStartPos + i);
	        }
	        _compressedBytesOutputStream.reset();
	        try
	        {
	            // write bytes to compress
	            _deflaterOutputStream.write(_compressByteArray, 0, lenToCompress);
	            // flush bytes to compress
	            _deflaterOutputStream.flush();
	        }
	        catch (Exception e)
	        {
	            throw new CompressorException(e.getLocalizedMessage());
	        }
	        // get compressed bytes
	        _compressedBytes = _compressedBytesOutputStream.toByteArray();

	        return _compressedBytes.length;
	    }
	    
	    int getMaxCompressedLength(int numBytesToCompress)
	    {
	        return (numBytesToCompress + 13);
	    }

	    byte[] compressedData()
	    {
	        return _compressedBytes;
	    }

	    int compressedDataLength()
	    {
	        return _compressedBytes.length;
	    }
	}
	
	class SocketHelperMock extends SocketHelper
	{
		private ByteBuffer _networkBuffer = ByteBuffer.allocate(RsslSocketChannel.MIN_READ_BUFFER_SIZE);
		private int _lastRead = 0;
		private int _maxWrite = -1;
		
		SocketHelperMock()
		{
		}
		
		@Override
		public int read(ByteBuffer dst) 
		{
			int byteRead = 0;
			
			for(int index = _lastRead; index < maxWrite(); index++)
			{
				dst.put(_networkBuffer.get(index));
				
				++byteRead;
			}
			
			_lastRead = byteRead;
			
			return byteRead;
		}
		
		public ByteBuffer networkBuffer()
		{
			return _networkBuffer;
		}
		
		public void maxWrite(int maxWrite)
		{
			_maxWrite = maxWrite;
		}
		
		@Override
		public long write(ByteBuffer[] srcs, int offset, int length)
		{
			long byteWritten = 0;
			
			for(int index = offset; index < length; index++)
			{
				byteWritten += (srcs[index].limit() - srcs[index].position());
				
				_networkBuffer.put(srcs[index]);
			}
			 
			return byteWritten;
		}

		@Override
		public int write(ByteBuffer src) throws IOException {
			int bytesWritten = src.limit() - src.position();
			_networkBuffer.put(src);
			return bytesWritten;
		}

		public void clear() {
			_lastRead = 0;
			_maxWrite = -1;
			networkBuffer().clear();
		}
		
		private int maxWrite()
		{
			if(_maxWrite == -1)
			{
				return _networkBuffer.position();
			}
			else
			{
				return _maxWrite <= _networkBuffer.position() ? _maxWrite : _networkBuffer.position();
			}
		}

		@Override
		public void close() throws IOException {
		}
	};
	
	@Test
    public void readParitialWSFrameHeaderForOneByte()
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		/* Write WebSocket frame and one JSON message */
		int hdrlen = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, JSON_MSG_1.length(), Codec.JSON_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().put(JSON_MSG_1.getBytes());
		
		int maxRead = 1;
		
		socketHelperMock.maxWrite(maxRead); /* Can read data only one byte */ 
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNull(readBuffer);
		assertEquals(maxRead, readArgs.readRetVal());  // More data to read
		assertEquals(maxRead, readArgs.bytesRead());

		socketHelperMock.maxWrite(-1); /* Read the remaining data from network */ 
		
		readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(JSON_MSG_1.length() + hdrlen - maxRead, readArgs.bytesRead());
		assertEquals(JSON_MSG_1.length() + hdrlen - maxRead, readArgs.uncompressedBytesRead());
		assertEquals(JSON_MSG_1.length(), readBuffer.length());
		assertEquals(hdrlen, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, JSON_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }
	
	@Test
    public void readParitialWSFrameHeader_UnknownPayloadLength()
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		/* Write WebSocket frame and one JSON message */
		int hdrlen = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, JSON_MSG_1.length(), Codec.JSON_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().put(JSON_MSG_1.getBytes());
		
		int maxRead = 2;
		
		socketHelperMock.maxWrite(maxRead); /* Can read data only one byte */ 
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNull(readBuffer);
		assertEquals(maxRead, readArgs.readRetVal());  // More data to read
		assertEquals(maxRead, readArgs.bytesRead());

		socketHelperMock.maxWrite(-1); /* Read the remaining data from network */ 
		
		readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(JSON_MSG_1.length() + hdrlen - maxRead, readArgs.bytesRead());
		assertEquals(JSON_MSG_1.length() + hdrlen - maxRead, readArgs.uncompressedBytesRead());
		assertEquals(JSON_MSG_1.length(), readBuffer.length());
		assertEquals(hdrlen, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, JSON_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }
	
	@Test
    public void readParitialJSONMessage()
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		/* Write WebSocket frame and one JSON message */
		int hdrlen = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, JSON_MSG_1.length(), Codec.JSON_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().put(JSON_MSG_1.getBytes());
		
		int maxRead = JSON_MSG_1.length() - 5;
		
		socketHelperMock.maxWrite(maxRead); /* Can read data only partial message */ 
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNull(readBuffer);
		assertEquals(maxRead, readArgs.readRetVal());  // More data to read
		assertEquals(maxRead, readArgs.bytesRead());

		socketHelperMock.maxWrite(-1); /* Read the remaining data from network */ 
		
		readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(JSON_MSG_1.length() + hdrlen - maxRead, readArgs.bytesRead());
		assertEquals(9, readArgs.uncompressedBytesRead());
		assertEquals(JSON_MSG_1.length(), readBuffer.length());
		assertEquals(hdrlen, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, JSON_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }
	
	@Test
    public void readPartialRWFMessage()
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		short ripcHdrLength = 3;
		short ripcMessageLength = (short)(ripcHdrLength + RWF_MSG_1.length());
		
		/* Write WebSocket frame and one RWF message */
		int hdrlen = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, ripcMessageLength, Codec.RWF_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_BINARY);
		
		/* Set RIPC header */
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().putShort(ripcMessageLength);
		socketHelperMock.networkBuffer().put((byte)RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */
		
		/* Set message body */
		socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());
		
		int maxRead = RWF_MSG_1.length() - 10;
		
		socketHelperMock.maxWrite(maxRead); /* Can read data only partial message */ 
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNull(readBuffer);
		assertEquals(maxRead, readArgs.readRetVal());  // More data to read
		assertEquals(maxRead, readArgs.bytesRead());

		socketHelperMock.maxWrite(-1); /* Read the remaining data from network */ 
		
		readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(ripcMessageLength + hdrlen - maxRead, readArgs.bytesRead());
		assertEquals(ripcMessageLength+ hdrlen, readArgs.uncompressedBytesRead());
		assertEquals(RWF_MSG_1.length(), readBuffer.length());
		assertEquals(hdrlen + ripcHdrLength, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }
	
	@Test
    public void readOneJSONMessage()
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		/* Write WebSocket frame and one JSON message */
		int hdrlen = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, JSON_MSG_1.length(), Codec.JSON_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().put(JSON_MSG_1.getBytes());
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(JSON_MSG_1.length() + hdrlen, readArgs.bytesRead());
		assertEquals(JSON_MSG_1.length() + hdrlen, readArgs.uncompressedBytesRead());
		assertEquals(JSON_MSG_1.length(), readBuffer.length());
		assertEquals(hdrlen, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, JSON_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }
	
	@Test
    public void readTwoJSONMessagesSameBuffer()
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		/* Write WebSocket frame and two JSON messages */
		int hdrlen = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, JSON_MSG_1.length(), Codec.JSON_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().put(JSON_MSG_1.getBytes());
		
		int hdrlen2 = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), hdrlen + JSON_MSG_1.length(), 
				JSON_MSG_2.length(), Codec.JSON_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		socketHelperMock.networkBuffer().position(hdrlen + JSON_MSG_1.length() + hdrlen2);
		socketHelperMock.networkBuffer().put(JSON_MSG_2.getBytes());
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(hdrlen2 + JSON_MSG_2.length(), readArgs.readRetVal());  // More data to read
		assertEquals(hdrlen + JSON_MSG_1.length() + hdrlen2 + JSON_MSG_2.length(), readArgs.bytesRead());
		assertEquals(hdrlen + JSON_MSG_1.length() + hdrlen2 + JSON_MSG_2.length(), readArgs.uncompressedBytesRead());
		assertEquals(JSON_MSG_1.length(), readBuffer.length());
		assertEquals(hdrlen, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, JSON_MSG_1_ByteBuffer.compareTo(msgBuffer));
		
		readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(0, readArgs.bytesRead());
		assertEquals(0, readArgs.uncompressedBytesRead());
		assertEquals(JSON_MSG_2.length(), readBuffer.length());
		assertEquals(hdrlen + JSON_MSG_1.length() + hdrlen2, readBuffer.dataStartPosition());
		
		msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, JSON_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }
	
	@Test
    public void readTwoJSONMessagesDiffrentBuffer()
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		/* Write WebSocket frame and two JSON messages */
		int hdrlen = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, JSON_MSG_1.length(), Codec.JSON_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().put(JSON_MSG_1.getBytes());
		
		int hdrlen2 = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), hdrlen + JSON_MSG_1.length(), 
				JSON_MSG_2.length(), Codec.JSON_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		socketHelperMock.networkBuffer().position(hdrlen + JSON_MSG_1.length() + hdrlen2);
		socketHelperMock.networkBuffer().put(JSON_MSG_2.getBytes());
		
		socketHelperMock.maxWrite(hdrlen + JSON_MSG_1.length());
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(JSON_MSG_1.length() + hdrlen, readArgs.bytesRead());
		assertEquals(JSON_MSG_1.length() + hdrlen, readArgs.uncompressedBytesRead());
		assertEquals(JSON_MSG_1.length(), readBuffer.length());
		assertEquals(hdrlen, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, JSON_MSG_1_ByteBuffer.compareTo(msgBuffer));
		
		socketHelperMock.maxWrite(-1);
		
		readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(JSON_MSG_2.length() + hdrlen2, readArgs.bytesRead());
		assertEquals(JSON_MSG_2.length() + hdrlen2, readArgs.uncompressedBytesRead());
		assertEquals(JSON_MSG_2.length(), readBuffer.length());
		assertEquals(hdrlen2, readBuffer.dataStartPosition());
		
		msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, JSON_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }
	
	@Test
    public void readOneRWFMessage()
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		short ripcHdrLength = 3;
		short ripcMessageLength = (short)(ripcHdrLength + RWF_MSG_1.length());
		
		/* Write WebSocket frame and one RWF message */
		int hdrlen = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, ripcMessageLength, Codec.RWF_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_BINARY);
		
		/* Set RIPC header */
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().putShort(ripcMessageLength);
		socketHelperMock.networkBuffer().put((byte)RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */
		
		/* Set message body */
		socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(ripcMessageLength + hdrlen, readArgs.bytesRead());
		assertEquals(ripcMessageLength+ hdrlen, readArgs.uncompressedBytesRead());
		assertEquals(RWF_MSG_1.length(), readBuffer.length());
		assertEquals(hdrlen + ripcHdrLength, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }
	
	@Test
    public void readTwoRWFMessagesSameBuffer()
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		short ripcHdrLength = 3;
		short ripcMessageLength = (short)(ripcHdrLength + RWF_MSG_1.length());
		
		/* Write WebSocket frame and two JSON messages */
		int hdrlen = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, ripcMessageLength, Codec.RWF_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_BINARY);
		
		/* Set RIPC header */
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().putShort(ripcMessageLength);
		socketHelperMock.networkBuffer().put((byte)RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */
		
		/* Set message body */
		socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());
		
		short ripcMessageLength2 = (short)(ripcHdrLength + RWF_MSG_2.length());
		
		int hdrlen2 = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), hdrlen + ripcMessageLength,
				ripcMessageLength2, Codec.RWF_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_BINARY);
		
		/* Set RIPC header */
		socketHelperMock.networkBuffer().position(hdrlen + ripcMessageLength + hdrlen2);
		socketHelperMock.networkBuffer().putShort(ripcMessageLength2);
		socketHelperMock.networkBuffer().put((byte)RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */
		
		/* Set message body */
		socketHelperMock.networkBuffer().put(RWF_MSG_2.getBytes());
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(hdrlen2 + ripcMessageLength2, readArgs.readRetVal());  // More data to read
		assertEquals(hdrlen + ripcMessageLength + hdrlen2 + ripcMessageLength2, readArgs.bytesRead());
		assertEquals(hdrlen + ripcMessageLength, readArgs.uncompressedBytesRead());
		assertEquals(RWF_MSG_1.length(), readBuffer.length());
		assertEquals(hdrlen + ripcHdrLength, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
		
		readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(0, readArgs.bytesRead());
		assertEquals(hdrlen2 + ripcHdrLength + RWF_MSG_2.length(), readArgs.uncompressedBytesRead());
		assertEquals(RWF_MSG_2.length(), readBuffer.length());
		assertEquals(hdrlen + ripcMessageLength + hdrlen2 + ripcHdrLength, readBuffer.dataStartPosition());
		
		msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }
	
	@Test
    public void readTwoRWFMessagesDiffrentBuffer()
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		short ripcHdrLength = 3;
		short ripcMessageLength = (short)(ripcHdrLength + RWF_MSG_1.length());
		
		/* Write WebSocket frame and two JSON messages */
		int hdrlen = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, ripcMessageLength, Codec.RWF_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_BINARY);
		
		/* Set RIPC header */
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().putShort(ripcMessageLength);
		socketHelperMock.networkBuffer().put((byte)RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */
		
		/* Set message body */
		socketHelperMock.networkBuffer().put(RWF_MSG_1.getBytes());
		
		short ripcMessageLength2 = (short)(ripcHdrLength + RWF_MSG_2.length());
		
		int hdrlen2 = WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), hdrlen + ripcMessageLength,
				ripcMessageLength2, Codec.RWF_PROTOCOL_TYPE,
				false, true, false, WebSocketFrameParser._WS_OPC_BINARY);
		
		/* Set RIPC header */
		socketHelperMock.networkBuffer().position(hdrlen + ripcMessageLength + hdrlen2);
		socketHelperMock.networkBuffer().putShort(ripcMessageLength2);
		socketHelperMock.networkBuffer().put((byte)RsslSocketChannel.IPC_DATA); /* Normal RIPC DATA */
		
		/* Set message body */
		socketHelperMock.networkBuffer().put(RWF_MSG_2.getBytes());
		
		socketHelperMock.maxWrite(hdrlen + ripcMessageLength);
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(hdrlen + ripcMessageLength, readArgs.bytesRead());
		assertEquals(hdrlen + ripcMessageLength, readArgs.uncompressedBytesRead());
		assertEquals(RWF_MSG_1.length(), readBuffer.length());
		assertEquals(hdrlen + ripcHdrLength, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
		
		socketHelperMock.maxWrite(-1);
		
		readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(hdrlen2 + ripcMessageLength2, readArgs.bytesRead());
		assertEquals(hdrlen2 + ripcMessageLength2, readArgs.uncompressedBytesRead());
		assertEquals(RWF_MSG_2.length(), readBuffer.length());
		assertEquals(hdrlen2 + ripcHdrLength, readBuffer.dataStartPosition());
		
		msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }
	
    public void writeOneJSONMessage(boolean directWrite, boolean client)
    {
		Transport._globalLock = new DummyLock();
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		channel.getWsSession().isClient = client;
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
		
		channel._transport = new SocketProtocol();
		channel.growGuaranteedOutputBuffers(100);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		WriteArgs writeArgs = TransportFactory.createWriteArgs();
		TransportBuffer writeBuffer = channel.getBuffer(JSON_MSG_1.length(), false, error);
			
		assertNotNull(writeBuffer);
		
		if(directWrite)
		{
			writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		}
		
		writeBuffer.data().put(JSON_MSG_1.getBytes());

		int retValue = channel.write(writeBuffer, writeArgs, error);
		
		if(directWrite == false)
		{
			retValue = channel.flush(error);
		}
		
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		boolean partial = WebSocketFrameParser.decode(frame, socketHelperMock.networkBuffer(), 0, writeArgs.bytesWritten());
		
		assertEquals(false, partial);
		assertEquals(JSON_MSG_1_READ.length(), frame.payloadLen);
		assertEquals(true, frame.finSet);
		assertEquals(false, frame.rsv1Set); // No compression
		assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.opcode);
		
		assertEquals(0, retValue);  // No bytes pending to flush
		assertEquals(JSON_MSG_1_READ.length() + frame.hdrLen, writeArgs.bytesWritten());
		assertEquals(JSON_MSG_1_READ.length() + frame.hdrLen, writeArgs.uncompressedBytesWritten());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(JSON_MSG_1_READ.length());
		msgBuffer.put(socketHelperMock.networkBuffer().array(), frame.hdrLen, JSON_MSG_1_READ.length());
		msgBuffer.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer.array(), 0, msgBuffer.array().length);
		}
		
		assertEquals(0, JSON_MSG_1_READ_ByteBuffer.compareTo(msgBuffer));
    }
	
    public void writeOneRWFMessage(boolean directWrite, boolean client)
    {
		Transport._globalLock = new DummyLock();
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		channel.getWsSession().isClient = client;
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
		
		channel._transport = new SocketProtocol();
		channel.growGuaranteedOutputBuffers(100);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		WriteArgs writeArgs = TransportFactory.createWriteArgs();
		TransportBuffer writeBuffer = channel.getBuffer(RWF_MSG_1.length(), false, error);
			
		assertNotNull(writeBuffer);
		
		if(directWrite)
		{
			writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		}
		
		writeBuffer.data().put(RWF_MSG_1.getBytes());

		int retValue = channel.write(writeBuffer, writeArgs, error);
		
		if(directWrite == false)
		{
			retValue = channel.flush(error);
		}
		
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		boolean partial = WebSocketFrameParser.decode(frame, socketHelperMock.networkBuffer(), 0, writeArgs.bytesWritten());
		
		assertEquals(false, partial);
		assertEquals(RWF_MSG_1.length() + RsslSocketChannel.RIPC_HDR_SIZE, frame.payloadLen);
		assertEquals(true, frame.finSet);
		assertEquals(false, frame.rsv1Set); // No compression
		assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame.opcode);
		
		assertEquals(0, retValue);  // No bytes pending to flush
		assertEquals(RWF_MSG_1.length() + frame.hdrLen + RsslSocketChannel.RIPC_HDR_SIZE, writeArgs.bytesWritten());
		assertEquals(RWF_MSG_1.length() + frame.hdrLen + RsslSocketChannel.RIPC_HDR_SIZE, writeArgs.uncompressedBytesWritten());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(RWF_MSG_1.length() + RsslSocketChannel.RIPC_HDR_SIZE);
		msgBuffer.put(socketHelperMock.networkBuffer().array(), frame.hdrLen, RWF_MSG_1.length() + RsslSocketChannel.RIPC_HDR_SIZE);
		msgBuffer.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer.array(), 0, msgBuffer.array().length);
		}
		
		/* Checks the RIPC header */
		assertEquals(RWF_MSG_1.length() + RsslSocketChannel.RIPC_HDR_SIZE, msgBuffer.getShort());
		assertEquals(RsslSocketChannel.IPC_DATA, msgBuffer.get());
		
		/* Checks RWF message */
		assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }
    
    public void writeJSONPackagingMessages(boolean directWrite, boolean client, boolean checkEndOfBuffer)
    {
		Transport._globalLock = new DummyLock();
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		channel.getWsSession().isClient = client;
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
		
		channel._transport = new SocketProtocol();
		channel.growGuaranteedOutputBuffers(100);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		WriteArgs writeArgs = TransportFactory.createWriteArgs();

		/* Plus one byte to include the message delimiter */
		TransportBuffer writeBuffer = channel.getBuffer(JSON_MSG_1.length() + JSON_MSG_2.length() + 1, true, error);
			
		assertNotNull(writeBuffer);
		
		if(directWrite)
		{
			writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		}
		
		writeBuffer.data().put(JSON_MSG_1.getBytes());
		
		int retValue = channel.packBuffer(writeBuffer, error);
		
		/* Checks to ensure there is enough buffer to pack another message */
		assertEquals(JSON_MSG_2.length(), retValue);
		
		writeBuffer.data().put(JSON_MSG_2.getBytes());
		
		if(checkEndOfBuffer)
		{
			/* Checks the error handling when there is no buffer left */
			retValue = channel.packBuffer(writeBuffer, error);
			assertEquals(0, retValue);
		}

		retValue = channel.write(writeBuffer, writeArgs, error);
		
		if(directWrite == false)
		{
			retValue = channel.flush(error);
		}
		
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		boolean partial = WebSocketFrameParser.decode(frame, socketHelperMock.networkBuffer(), 0, writeArgs.bytesWritten());
		
		assertEquals(false, partial);
		assertEquals(PACKED_JSON_MSG_1_MSG_2.length(), frame.payloadLen);
		assertEquals(true, frame.finSet);
		assertEquals(false, frame.rsv1Set); // No compression
		assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.opcode);
		
		assertEquals(0, retValue);  // No bytes pending to flush
		assertEquals(PACKED_JSON_MSG_1_MSG_2.length() + frame.hdrLen, writeArgs.bytesWritten());
		assertEquals(PACKED_JSON_MSG_1_MSG_2.length() + frame.hdrLen, writeArgs.uncompressedBytesWritten());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(PACKED_JSON_MSG_1_MSG_2.length());
		msgBuffer.put(socketHelperMock.networkBuffer().array(), frame.hdrLen, PACKED_JSON_MSG_1_MSG_2.length());
		msgBuffer.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer.array(), 0, msgBuffer.array().length);
		}
		
		assertEquals(0, PACKED_JSON_MSG_1_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }
    
    public void writeRWFPackagingMessages(boolean directWrite, boolean client, boolean checkEndOfBuffer)
    {	
    	Transport._globalLock = new DummyLock();
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		channel.getWsSession().isClient = client;
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
		
		channel._transport = new SocketProtocol();
		channel.growGuaranteedOutputBuffers(100);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		WriteArgs writeArgs = TransportFactory.createWriteArgs();

		TransportBuffer writeBuffer = channel.getBuffer(RWF_MSG_1.length() + RWF_MSG_2.length() + 2, true, error);
			
		assertNotNull(writeBuffer);
		
		if(directWrite)
		{
			writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		}
		
		writeBuffer.data().put(RWF_MSG_1.getBytes());
		
		int retValue = channel.packBuffer(writeBuffer, error);
		
		/* Checks to ensure there is enough buffer to pack another message */
		assertEquals(RWF_MSG_2.length(), retValue);
		
		writeBuffer.data().put(RWF_MSG_2.getBytes());
		
		if(checkEndOfBuffer)
		{
			/* Checks the error handling when there is no buffer left */
			retValue = channel.packBuffer(writeBuffer, error);
			assertEquals(0, retValue);
		}

		retValue = channel.write(writeBuffer, writeArgs, error);
		
		if(directWrite == false)
		{
			retValue = channel.flush(error);
		}
		
		int sendMaskValue = WebSocketFrameParser.getLastMaskValue();
		
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		boolean partial = WebSocketFrameParser.decode(frame, socketHelperMock.networkBuffer(), 0, writeArgs.bytesWritten());
		
		int messagelength = RWF_MSG_1.length() + RWF_MSG_2.length() + 4;
		int payloadLength = messagelength + RsslSocketChannel.RIPC_HDR_SIZE;
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame.payloadLen);
		assertEquals(true, frame.finSet);
		assertEquals(false, frame.rsv1Set); // No compression
		assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame.opcode);
		
		assertEquals(0, retValue);  // No bytes pending to flush
		assertEquals(payloadLength + frame.hdrLen, writeArgs.bytesWritten());
		assertEquals(payloadLength + frame.hdrLen, writeArgs.uncompressedBytesWritten());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength);
		msgBuffer.put(socketHelperMock.networkBuffer().array(), frame.hdrLen, payloadLength);
		msgBuffer.rewind();
		
		if(client)
		{
			assertEquals(sendMaskValue, frame.maskVal);
			
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer.array(), 0, msgBuffer.array().length);
		}
		
		/* Checks the RIPC header */
		assertEquals(payloadLength, msgBuffer.getShort());
		assertEquals(RsslSocketChannel.IPC_DATA | RsslSocketChannel.IPC_PACKING, msgBuffer.get());
		
		assertEquals(0, PACKED_RWF_MSG_1_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }
    
    public void writeCompressedRWFMessages(boolean directWrite, boolean client, boolean packing)
    {	
    	Transport._globalLock = new DummyLock();
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		channel.getWsSession().isClient = client;
		channel.getWsSession().setDeflate(true);
		channel.getWsSession().applyNoOutboundContextTakeOver();
		channel._compressor = new ZlibCompressor();
		channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
		channel._sessionCompLowThreshold = 30;
		
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
		
		channel._transport = new SocketProtocol();
		channel.growGuaranteedOutputBuffers(100);
		
		ZlibCompressorTest zlibCompressor = new ZlibCompressorTest(false);
		ByteBuffer decompressedBuffer = ByteBuffer.allocate(6144);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		WriteArgs writeArgs = TransportFactory.createWriteArgs();

		TransportBuffer writeBuffer = null;
				
		if(packing)
		{
			writeBuffer = channel.getBuffer(RWF_MSG_1.length() + RWF_MSG_2.length() + 2, true, error);
		}
		else
		{
			writeBuffer = channel.getBuffer(RWF_MSG_1.length(), false, error);
		}
			
		assertNotNull(writeBuffer);
		
		if(directWrite)
		{
			writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		}
		
		writeBuffer.data().put(RWF_MSG_1.getBytes());
		
		int retValue = 0;
		
		if(packing)
		{
			retValue = channel.packBuffer(writeBuffer, error);
		
			/* Checks to ensure there is enough buffer to pack another message */
			assertEquals(RWF_MSG_2.length(), retValue);
		
			writeBuffer.data().put(RWF_MSG_2.getBytes());
		}

		retValue = channel.write(writeBuffer, writeArgs, error);
		
		if(directWrite == false)
		{
			retValue = channel.flush(error);
		}
		
		int sendMaskValue = WebSocketFrameParser.getLastMaskValue();
		
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		boolean partial = WebSocketFrameParser.decode(frame, socketHelperMock.networkBuffer(), 0, writeArgs.bytesWritten());
		
		int messagelength = 0;
				
		if(packing)
		{
			messagelength = zlibCompressor.compress(PACKED_RWF_MSG_1_MSG_2_ByteBuffer, 0, PACKED_RWF_MSG_1_MSG_2_ByteBuffer.limit());
		}
		else
		{
			messagelength = zlibCompressor.compress(RWF_MSG_1_ByteBuffer, 0, RWF_MSG_1.length());
		}
		
		int payloadLength = messagelength + RsslSocketChannel.RIPC_HDR_SIZE;
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame.payloadLen);
		assertEquals(true, frame.finSet);
		assertEquals(false, frame.rsv1Set); // No compression
		assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame.opcode);
		
		assertEquals(0, retValue);  // No bytes pending to flush
		assertEquals(payloadLength + frame.hdrLen, writeArgs.bytesWritten());
		
		if(packing)
		{
			assertEquals(RWF_MSG_1.length() + RWF_MSG_2.length() + 4 + RsslSocketChannel.RIPC_HDR_SIZE + frame.hdrLen, writeArgs.uncompressedBytesWritten());
		}
		else
		{
			assertEquals(RWF_MSG_1.length() + RsslSocketChannel.RIPC_HDR_SIZE + frame.hdrLen, writeArgs.uncompressedBytesWritten());
		}
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength);
		msgBuffer.put(socketHelperMock.networkBuffer().array(), frame.hdrLen, payloadLength);
		msgBuffer.rewind();
		
		int numberOfBytpes = 0;
		
		if(client)
		{
			assertEquals(sendMaskValue, frame.maskVal);
			
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer.array(), 0, msgBuffer.array().length);
			
			numberOfBytpes = zlibCompressor.decompress(msgBuffer,RsslSocketChannel.RIPC_HDR_SIZE, decompressedBuffer, messagelength);
		}
		else
		{
			numberOfBytpes = zlibCompressor.decompress(msgBuffer,RsslSocketChannel.RIPC_HDR_SIZE, decompressedBuffer, messagelength);
		}
		
		/* Checks the RIPC header */
		assertEquals(payloadLength, msgBuffer.getShort());

		/* Checks the actual message length */
		if(packing)
		{
			assertEquals(RWF_MSG_1.length() + RWF_MSG_2.length() + 4, numberOfBytpes); // 4 is additional length for packing with the RWF protocol
			assertEquals(RsslSocketChannel.IPC_PACKING | Ripc.Flags.COMPRESSION, msgBuffer.get());
		}
		else
		{
			assertEquals(RWF_MSG_1.length(), numberOfBytpes);
			assertEquals(Ripc.Flags.COMPRESSION, msgBuffer.get());
		}
    }
    
    public void writeCompressedJSONMessages(boolean directWrite, boolean client, boolean packing)
    {	
    	Transport._globalLock = new DummyLock();
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		channel.getWsSession().isClient = client;
		channel.getWsSession().setDeflate(true);
		channel.getWsSession().applyNoOutboundContextTakeOver();
		channel._compressor = new ZlibCompressor();
		channel._compressor.appendCompressTrailing();
		channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
		channel._sessionCompLowThreshold = 30;
		
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
		
		channel._transport = new SocketProtocol();
		channel.growGuaranteedOutputBuffers(100);
		
		// Uncompressed data here
		ZlibCompressorTest zlibCompressor = new ZlibCompressorTest(true);
		ByteBuffer decompressedBuffer = ByteBuffer.allocate(6144);
		
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		WriteArgs writeArgs = TransportFactory.createWriteArgs();

		TransportBuffer writeBuffer = null;
				
		if(packing)
		{
			writeBuffer = channel.getBuffer(JSON_MSG_1.length() + JSON_MSG_2.length() + 1, true, error);
		}
		else
		{
			writeBuffer = channel.getBuffer(JSON_MSG_1.length(), false, error);
		}
			
		assertNotNull(writeBuffer);
		
		if(directWrite)
		{
			writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		}
		
		writeBuffer.data().put(JSON_MSG_1.getBytes());
		
		int retValue = 0;
		
		if(packing)
		{
			retValue = channel.packBuffer(writeBuffer, error);
		
			/* Checks to ensure there is enough buffer to pack another message */
			assertEquals(JSON_MSG_2.length(), retValue);
		
			writeBuffer.data().put(JSON_MSG_2.getBytes());
		}

		retValue = channel.write(writeBuffer, writeArgs, error);
		
		if(directWrite == false)
		{
			retValue = channel.flush(error);
		}
		
		int sendMaskValue = WebSocketFrameParser.getLastMaskValue();
		
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		boolean partial = WebSocketFrameParser.decode(frame, socketHelperMock.networkBuffer(), 0, writeArgs.bytesWritten());
		
		int messagelength = 0;
				
		if(packing)
		{
			messagelength =	zlibCompressor.compress(PACKED_JSON_MSG_1_MSG_2_ByteBuffer, 0, PACKED_JSON_MSG_1_MSG_2.length());
		}
		else
		{
			messagelength = zlibCompressor.compress(JSON_MSG_1_READ_ByteBuffer, 0, JSON_MSG_1_READ.length());
		}
		
		// Strip off the trailing 0x00 0x00 0xFF 0xFF as per PMCE spec
		messagelength -= 4;
		
		int payloadLength = messagelength;
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame.payloadLen);
		assertEquals(true, frame.finSet);
		assertEquals(true, frame.rsv1Set); // Checks Compression
		assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.opcode);
		
		assertEquals(0, retValue);  // No bytes pending to flush
		assertEquals(payloadLength + frame.hdrLen, writeArgs.bytesWritten());
		
		if(packing)
		{
			assertEquals(JSON_MSG_1_READ.length() + JSON_MSG_2.length() + 1 + frame.hdrLen, writeArgs.uncompressedBytesWritten());
		}
		else
		{
			assertEquals(JSON_MSG_1_READ.length() + frame.hdrLen, writeArgs.uncompressedBytesWritten());
		}
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength);
		msgBuffer.put(socketHelperMock.networkBuffer().array(), frame.hdrLen, payloadLength);
		msgBuffer.rewind();
		
		int numberOfBytpes = 0;
		
		if(client)
		{
			assertEquals(sendMaskValue, frame.maskVal);
			
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer.array(), 0, msgBuffer.array().length);
			
			numberOfBytpes = zlibCompressor.decompress(msgBuffer, 0, decompressedBuffer, messagelength);
		}
		else
		{
			numberOfBytpes = zlibCompressor.decompress(msgBuffer, 0, decompressedBuffer, messagelength);
		}

		/* Checks the actual message length */
		if(packing)
		{
			assertEquals(JSON_MSG_1_READ.length() + JSON_MSG_2.length() + 1, numberOfBytpes); // 1 is additional length for packing with the JSON protocol
		}
		else
		{
			assertEquals(JSON_MSG_1_READ.length(), numberOfBytpes);
		}
    }
    
    public void readPackingJSONMessages(boolean isClient)
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		channel.getWsSession().isClient = isClient;
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		int hdrlen = WebSocketFrameParser.calculateHeaderLength(PACKED_JSON_MSG_1_MSG_2.length(), !isClient);
		
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().put(PACKED_JSON_MSG_1_MSG_2.getBytes());
		
		/* Write WebSocket frame and one JSON message */
		WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, PACKED_JSON_MSG_1_MSG_2.length(), Codec.JSON_PROTOCOL_TYPE,
				!isClient, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(PACKED_JSON_MSG_1_MSG_2.length() + hdrlen, readArgs.bytesRead());
		assertEquals(PACKED_JSON_MSG_1_MSG_2.length() + hdrlen, readArgs.uncompressedBytesRead());
		assertEquals(PACKED_JSON_MSG_1_MSG_2.length(), readBuffer.length());
		assertEquals(hdrlen, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, PACKED_JSON_MSG_1_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }
    
    public void readPackingRWFMessages(boolean isClient)
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		channel.getWsSession().isClient = isClient;
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		int hdrlen = WebSocketFrameParser.calculateHeaderLength(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit(), !isClient);
		
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().put(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER);
		
		/* Write WebSocket frame and one JSON message */
		WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit(), Codec.RWF_PROTOCOL_TYPE,
				!isClient, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(1, readArgs.readRetVal());  // Has more data to read
		assertEquals(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit() + hdrlen, readArgs.bytesRead());
		assertEquals(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit(), readArgs.uncompressedBytesRead());
		assertEquals(RWF_MSG_1.length(), readBuffer.length());
		assertEquals(hdrlen + RsslSocketChannel.RIPC_HDR_SIZE + RsslSocketChannel.RIPC_PACKED_HDR_SIZE, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
		
		readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(0, readArgs.bytesRead());
		assertEquals(0, readArgs.uncompressedBytesRead());
		assertEquals(RWF_MSG_2.length(), readBuffer.length());
		assertEquals(hdrlen + RsslSocketChannel.RIPC_HDR_SIZE + RsslSocketChannel.RIPC_PACKED_HDR_SIZE
				+ RWF_MSG_1.length() + RsslSocketChannel.RIPC_PACKED_HDR_SIZE, readBuffer.dataStartPosition());
		
		msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }
    
    public void readCompressedPackingRWFMessages(boolean isClient)
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		channel.getWsSession().isClient = isClient;
		channel.getWsSession().setDeflate(true);
		channel.getWsSession().applyNoOutboundContextTakeOver();
		channel._compressor = new ZlibCompressor();
		channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
		channel._sessionCompLowThreshold = 30;
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		ZlibCompressorTest zlibCompressor = new ZlibCompressorTest(false);
		
		zlibCompressor.compress(PACKED_RWF_MSG_1_MSG_2_ByteBuffer, 0, PACKED_RWF_MSG_1_MSG_2_ByteBuffer.limit());
		
		int messageLength = zlibCompressor.compressedDataLength() + RsslSocketChannel.RIPC_HDR_SIZE;
		
		int hdrlen = WebSocketFrameParser.calculateHeaderLength(messageLength, !isClient);
		
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().putShort((short)messageLength);
		socketHelperMock.networkBuffer().put((byte)(RsslSocketChannel.IPC_PACKING | Ripc.Flags.COMPRESSION));
		socketHelperMock.networkBuffer().put(zlibCompressor.compressedData());
		
		WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, messageLength, Codec.RWF_PROTOCOL_TYPE,
				!isClient, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(1, readArgs.readRetVal());  // Has more data to read
		assertEquals(messageLength + hdrlen, readArgs.bytesRead());
		assertEquals(PACKED_RWF_MSG_1_MSG_2_ByteBuffer_RIPC_HEADER.limit() + hdrlen, readArgs.uncompressedBytesRead());
		assertEquals(RWF_MSG_1.length(), readBuffer.length());
		assertEquals(RsslSocketChannel.RIPC_PACKED_HDR_SIZE, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
		
		readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(0, readArgs.bytesRead());
		assertEquals(0, readArgs.uncompressedBytesRead());
		assertEquals(RWF_MSG_2.length(), readBuffer.length());
		assertEquals(RsslSocketChannel.RIPC_PACKED_HDR_SIZE + RWF_MSG_1.length() + RsslSocketChannel.RIPC_PACKED_HDR_SIZE, readBuffer.dataStartPosition());
		
		msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }
    
    public void readCompressedRWFMessage(boolean isClient)
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		channel.getWsSession().isClient = isClient;
		channel.getWsSession().setDeflate(true);
		channel.getWsSession().applyNoOutboundContextTakeOver();
		channel._compressor = new ZlibCompressor();
		channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
		channel._sessionCompLowThreshold = 30;
		channel._decompressBuffer =  new TransportBufferImpl(channel._internalMaxFragmentSize);
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		ZlibCompressorTest zlibCompressor = new ZlibCompressorTest(false);
		
		zlibCompressor.compress(RWF_MSG_1_ByteBuffer, 0, RWF_MSG_1_ByteBuffer.limit());
		
		int messageLength = zlibCompressor.compressedDataLength() + RsslSocketChannel.RIPC_HDR_SIZE;
		
		int hdrlen = WebSocketFrameParser.calculateHeaderLength(messageLength, !isClient);
		
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().putShort((short)messageLength);
		socketHelperMock.networkBuffer().put((byte)(Ripc.Flags.COMPRESSION));
		socketHelperMock.networkBuffer().put(zlibCompressor.compressedData());
		
		WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, messageLength, Codec.RWF_PROTOCOL_TYPE,
				!isClient, true, false, WebSocketFrameParser._WS_OPC_TEXT);
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // Has more data to read
		assertEquals(messageLength + hdrlen, readArgs.bytesRead());
		assertEquals(RWF_MSG_1_ByteBuffer.limit() + RsslSocketChannel.RIPC_HDR_SIZE + hdrlen, readArgs.uncompressedBytesRead());
		assertEquals(RWF_MSG_1.length(), readBuffer.length());
		assertEquals(0, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_1_ByteBuffer.compareTo(msgBuffer));
    }
    
    public void readCompressedJSONMessages(boolean isClient)
    {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		channel.getWsSession().isClient = isClient;
		channel.getWsSession().setDeflate(true);
		channel.getWsSession().applyNoOutboundContextTakeOver();
		channel._compressor = new ZlibCompressor();
		channel._compressor.appendCompressTrailing();
		channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
		channel._sessionCompLowThreshold = 30;
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		ZlibCompressorTest zlibCompressor = new ZlibCompressorTest(true);
		
		zlibCompressor.compress(PACKED_JSON_MSG_1_MSG_2_ByteBuffer, 0, PACKED_JSON_MSG_1_MSG_2_ByteBuffer.limit());
		
		int messageLength = zlibCompressor.compressedDataLength();
		
		int hdrlen = WebSocketFrameParser.calculateHeaderLength(messageLength, !isClient);
		
		socketHelperMock.networkBuffer().position(hdrlen);
		socketHelperMock.networkBuffer().put(zlibCompressor.compressedData());
		
		WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, messageLength, Codec.JSON_PROTOCOL_TYPE,
				!isClient, true, channel.getWsSession().isDeflate(), WebSocketFrameParser._WS_OPC_TEXT);
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNotNull(readBuffer);
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		assertEquals(messageLength + hdrlen, readArgs.bytesRead());
		assertEquals(PACKED_JSON_MSG_1_MSG_2_ByteBuffer.limit() + hdrlen, readArgs.uncompressedBytesRead());
		assertEquals(PACKED_JSON_MSG_1_MSG_2.length(), readBuffer.length());
		assertEquals(0, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, PACKED_JSON_MSG_1_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }

    public void executePingPongInteraction(int protocolType) {
		Transport._globalLock = new DummyLock();
		Error error = TransportFactory.createError();
		ReadArgs readArgs = TransportFactory.createReadArgs();
		final SocketHelperMock socket = new SocketHelperMock();
		RsslSocketChannel channel = createWebSocketChannelAndMakeActive(socket, true, protocolType);
		RsslSocketChannel serverChannel = createWebSocketChannelAndMakeActive(socket, false, protocolType);
		assertFalse(error.text(), channel.ping(error) < TransportReturnCodes.SUCCESS);

		//read ping msg.
		TransportBuffer transportBuffer = serverChannel.read(readArgs, error);
		if (Objects.equals(Codec.JSON_PROTOCOL_TYPE, protocolType)) {
			assertNotNull(transportBuffer);
			final byte[] expectedPingContent = WSProtocolFunctions.WEB_SOCKET_JSON_PING_MESSAGE;
			transportBuffer.data().position(transportBuffer.dataStartPosition());
			final byte[] content = new byte[transportBuffer.length()];
			transportBuffer.data().get(content);
			assertArrayEquals(expectedPingContent, content);
		} else {
			assertEquals(readArgs.readRetVal(), TransportReturnCodes.READ_PING);
		}

		socket.clear();

		assertFalse(testPong(serverChannel, error) < TransportReturnCodes.FAILURE);

		//read pong msg.
		transportBuffer = channel.read(readArgs, error);
		//assert that client received expected content PONG
		if (Objects.equals(Codec.JSON_PROTOCOL_TYPE, protocolType)) {
			final byte[] expectedPongContent = WSProtocolFunctions.WEB_SOCKET_JSON_PONG_MESSAGE;
			transportBuffer.data().position(transportBuffer.dataStartPosition());
			final byte[] content = new byte[transportBuffer.length()];
			transportBuffer.data().get(content);
			assertArrayEquals(expectedPongContent, content);
		} else {
			assertEquals(readArgs.readRetVal(), TransportReturnCodes.READ_PING);
		}
	}

	@Test
	public void givenActiveWSConnection_whenCloseConnection_thenCloseFrameSentAndReceived() {
		Transport._globalLock = new DummyLock();
		Error error = TransportFactory.createError();
		ReadArgs readArgs = TransportFactory.createReadArgs();
		final SocketHelperMock socket = new SocketHelperMock();
		RsslSocketChannel channel = createWebSocketChannelAndMakeActive(socket, true, Codec.RWF_PROTOCOL_TYPE);
		RsslSocketChannel serverChannel = createWebSocketChannelAndMakeActive(socket, false, Codec.RWF_PROTOCOL_TYPE);
		assertFalse(error.text(), channel.close(error) < TransportReturnCodes.SUCCESS);

		//read close frame.
		TransportBuffer transportBuffer = serverChannel.read(readArgs, error);
		assertNull(transportBuffer);
		assertEquals(TransportReturnCodes.FAILURE, readArgs.readRetVal());
		assertEquals("WS Close Code 1000 Normal Closure", error.text());

		//read replied close frame.
		transportBuffer = channel.read(readArgs, error);
		assertNull(transportBuffer);
		assertEquals(TransportReturnCodes.FAILURE, readArgs.readRetVal());
		assertEquals("WS Close Code 1001 Going Away", error.text());
	}

	private RsslSocketChannel createWebSocketChannelAndMakeActive(SocketHelperMock socket, boolean isClient, int protocolType) {
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, protocolType) {
			@Override
			protected int closeSocketChannel(Error error) {
				return TransportReturnCodes.SUCCESS;
			}
		};

		channel.getWsSession().isClient = isClient;
		if (Codec.JSON_PROTOCOL_TYPE == protocolType) {
			channel.getWsSession().setProtocolVersion("tr_json2");
		}
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.buffer());
		WSProtocolFunctions wsProtocolFunction = new WSProtocolFunctions(channel);
		wsProtocolFunction.initializeBufferAndFrameData();
		channel._protocolFunctions = wsProtocolFunction;
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
		channel._scktChannel = socket;
		channel._transport = new SocketProtocol();
		channel.growGuaranteedOutputBuffers(100);
		return channel;
	}

	private int testPong(RsslSocketChannel channel, Error error) {
		try {
			return channel._protocolFunctions.pong(error);
		} catch (IOException e) {
		}
		return TransportReturnCodes.FAILURE;
	}

    
    public void writeFragmentedJSONMessage(boolean directWrite, boolean client)
    {
    	Transport._globalLock = new DummyLock();
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		channel.getWsSession().isClient = client;
		
		/* Overrides the default max fragmentation size */
		channel._channelInfo._maxFragmentSize = (6 * 5) - RsslSocketChannel.RIPC_PACKED_HDR_SIZE;
		channel._internalMaxFragmentSize = (6 * 5) + RsslSocketChannel.RIPC_HDR_SIZE + _WS_MAX_HEADER_LEN;
		
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
		
		channel.createBigBufferPool(channel._internalMaxFragmentSize);
		
		channel._transport = new SocketProtocol();
		channel.growGuaranteedOutputBuffers(100);
		
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		WriteArgs writeArgs = TransportFactory.createWriteArgs();

		TransportBuffer writeBuffer = null;
				
		writeBuffer = channel.getBuffer(JSON_MSG_1.length(), false, error);
			
		assertNotNull(writeBuffer);
		
		if(directWrite)
		{
			writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		}
		
		assertEquals(JSON_MSG_1.length(), writeBuffer.length());
		
		writeBuffer.data().put(JSON_MSG_1.getBytes());
		
		int retValue = 0;

		retValue = channel.write(writeBuffer, writeArgs, error);
		
		if(directWrite == false)
		{
			retValue = channel.flush(error);
		}
		
		int readSize = client ? 39 : 35;
		
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		boolean partial = WebSocketFrameParser.decode(frame, socketHelperMock.networkBuffer(), 0, readSize);
		
		int payloadLength = readSize - frame.hdrLen;
		int nextReadIndex = (int) (frame.hdrLen + frame.payloadLen);
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame.payloadLen);
		assertEquals(false, frame.finSet);
		assertEquals(false, frame.rsv1Set); // Checks Compression
		assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.opcode);
		
		assertEquals(0, retValue);  // No bytes pending to flush
		
		int bytesWritten = client ? 166 : 146;
		
		assertEquals(bytesWritten, writeArgs.bytesWritten());
		
		assertEquals(bytesWritten, writeArgs.uncompressedBytesWritten());

		ByteBuffer msgBuffer = ByteBuffer.allocate((int)frame.payloadLen);
		msgBuffer.put(socketHelperMock.networkBuffer().array(), frame.hdrLen, (int)frame.payloadLen);
		msgBuffer.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer.array(), 0, msgBuffer.array().length);
		}
		
		int limit = JSON_MSG_1_READ_ByteBuffer.limit(); /* Keeps the original message length */
		
		JSON_MSG_1_READ_ByteBuffer.limit(payloadLength);
		
		// Checks the data of the first fragmented message
		assertEquals(0, JSON_MSG_1_READ_ByteBuffer.compareTo(msgBuffer));
		
		WebSocketFrameHdr frame2 = new WebSocketFrameHdr();

		partial = WebSocketFrameParser.decode(frame2, socketHelperMock.networkBuffer(), nextReadIndex, readSize);
		
		payloadLength = readSize - frame2.hdrLen;
		
		nextReadIndex += (int) (frame2.hdrLen + frame2.payloadLen);
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame2.payloadLen);
		assertEquals(false, frame2.finSet);
		assertEquals(false, frame2.rsv1Set); // Checks Compression
		assertEquals(WebSocketFrameParser._WS_OPC_CONT, frame2.opcode);

		ByteBuffer msgBuffer2 = ByteBuffer.allocate((int)frame2.payloadLen);
		msgBuffer2.put(socketHelperMock.networkBuffer().array(), nextReadIndex - payloadLength, (int)frame2.payloadLen);
		msgBuffer2.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame2.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer2.array(), 0, msgBuffer2.array().length);
		}
		
		JSON_MSG_1_READ_ByteBuffer.position(msgBuffer.limit());
		JSON_MSG_1_READ_ByteBuffer.limit(msgBuffer.limit() + (int)frame2.payloadLen);
		
		// Checks the data of the second fragmented message
		assertEquals(0, JSON_MSG_1_READ_ByteBuffer.compareTo(msgBuffer2));
		
		WebSocketFrameHdr frame3 = new WebSocketFrameHdr();

		partial = WebSocketFrameParser.decode(frame3, socketHelperMock.networkBuffer(), nextReadIndex, readSize);
		
		payloadLength = readSize - frame3.hdrLen;
		
		nextReadIndex += (int) (frame3.hdrLen + frame3.payloadLen);
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame3.payloadLen);
		assertEquals(false, frame3.finSet);
		assertEquals(false, frame3.rsv1Set); // Checks Compression
		assertEquals(WebSocketFrameParser._WS_OPC_CONT, frame3.opcode);

		ByteBuffer msgBuffer3 = ByteBuffer.allocate((int)frame3.payloadLen);
		msgBuffer3.put(socketHelperMock.networkBuffer().array(), nextReadIndex - payloadLength, (int)frame3.payloadLen);
		msgBuffer3.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame3.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer3.array(), 0, msgBuffer3.array().length);
		}
		
		JSON_MSG_1_READ_ByteBuffer.position(msgBuffer.limit() + msgBuffer2.limit() );
		JSON_MSG_1_READ_ByteBuffer.limit(msgBuffer.limit() + msgBuffer2.limit() + (int)frame3.payloadLen);
		
		// Checks the data of the third fragmented message
		assertEquals(0, JSON_MSG_1_READ_ByteBuffer.compareTo(msgBuffer3));
		
		WebSocketFrameHdr frame4 = new WebSocketFrameHdr();

		partial = WebSocketFrameParser.decode(frame4, socketHelperMock.networkBuffer(), nextReadIndex, readSize);
		
		payloadLength = readSize - frame4.hdrLen;
		
		nextReadIndex += (int) (frame4.hdrLen + frame4.payloadLen);
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame4.payloadLen);
		assertEquals(false, frame4.finSet);
		assertEquals(false, frame4.rsv1Set); // Checks Compression
		assertEquals(WebSocketFrameParser._WS_OPC_CONT, frame4.opcode);

		ByteBuffer msgBuffer4 = ByteBuffer.allocate((int)frame4.payloadLen);
		msgBuffer4.put(socketHelperMock.networkBuffer().array(), nextReadIndex - payloadLength, (int)frame4.payloadLen);
		msgBuffer4.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame4.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer4.array(), 0, msgBuffer4.array().length);
		}
		
		JSON_MSG_1_READ_ByteBuffer.position(msgBuffer.limit() + msgBuffer2.limit() + msgBuffer3.limit() );
		JSON_MSG_1_READ_ByteBuffer.limit(msgBuffer.limit() + msgBuffer2.limit() + msgBuffer3.limit() + (int)frame4.payloadLen);
		
		// Checks the data of the fourth fragmented message
		assertEquals(0, JSON_MSG_1_READ_ByteBuffer.compareTo(msgBuffer4));
		
		WebSocketFrameHdr frame5 = new WebSocketFrameHdr();
		
		readSize = client ? 10 : 6;

		partial = WebSocketFrameParser.decode(frame5, socketHelperMock.networkBuffer(), nextReadIndex, readSize);
		
		payloadLength = readSize - frame5.hdrLen;
		
		nextReadIndex += (int) (frame5.hdrLen + frame5.payloadLen);
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame5.payloadLen);
		assertEquals(true, frame5.finSet);
		assertEquals(false, frame5.rsv1Set); // Checks Compression
		assertEquals(WebSocketFrameParser._WS_OPC_CONT, frame5.opcode);

		ByteBuffer msgBuffer5 = ByteBuffer.allocate((int)frame5.payloadLen);
		msgBuffer5.put(socketHelperMock.networkBuffer().array(), nextReadIndex - payloadLength, (int)frame5.payloadLen);
		msgBuffer5.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame5.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer5.array(), 0, msgBuffer5.array().length);
		}
		
		JSON_MSG_1_READ_ByteBuffer.position(msgBuffer.limit() + msgBuffer2.limit() + msgBuffer3.limit() + msgBuffer4.limit() );
		JSON_MSG_1_READ_ByteBuffer.limit(msgBuffer.limit() + msgBuffer2.limit() + msgBuffer3.limit() + msgBuffer4.limit() + (int)frame5.payloadLen);
		
		// Checks the data of the fourth fragmented message
		assertEquals(0, JSON_MSG_1_READ_ByteBuffer.compareTo(msgBuffer5));
		
		/* Resets to the original value */
		JSON_MSG_1_READ_ByteBuffer.limit(limit);
		JSON_MSG_1_READ_ByteBuffer.position(0);
    }
    
    public void writeFragmentedRWFMessage(boolean directWrite, boolean client)
    {
    	Transport._globalLock = new DummyLock();
		RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		channel.getWsSession().isClient = client;
		
		/* Overrides the default max fragmentation size */
		channel._channelInfo._maxFragmentSize = (6 * 5) - RsslSocketChannel.RIPC_PACKED_HDR_SIZE;
		channel._internalMaxFragmentSize = (6 * 5) + RsslSocketChannel.RIPC_HDR_SIZE + _WS_MAX_HEADER_LEN;
		
		channel._ipcProtocol = new Ripc14Protocol();
		channel._ipcProtocol.channel(channel);
		
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
		channel._readBufStateMachine.ripcVersion(Ripc.RipcVersions.VERSION14);
		
		channel.createBigBufferPool(channel._internalMaxFragmentSize);
		
		channel._transport = new SocketProtocol();
		channel.growGuaranteedOutputBuffers(100);
		
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		WriteArgs writeArgs = TransportFactory.createWriteArgs();

		TransportBuffer writeBuffer = null;
				
		writeBuffer = channel.getBuffer(RWF_MSG_2.length(), false, error);
			
		assertNotNull(writeBuffer);
		
		assertEquals(RWF_MSG_2.length(), writeBuffer.length());
		
		if(directWrite)
		{
			writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		}
		
		writeBuffer.data().put(RWF_MSG_2.getBytes());
		
		int retValue = 0;

		retValue = channel.write(writeBuffer, writeArgs, error);
		
		if(directWrite == false)
		{
			retValue = channel.flush(error);
		}
		
		int readSize = client ? 39 : 35;
		int fragId = BigBuffer._ID;
		
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		boolean partial = WebSocketFrameParser.decode(frame, socketHelperMock.networkBuffer(), 0, readSize);
		
		int payloadLength = readSize - frame.hdrLen;
		int nextReadIndex = (int) (frame.hdrLen + frame.payloadLen);
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame.payloadLen);
		assertEquals(true, frame.finSet);
		assertEquals(false, frame.rsv1Set); // Checks Compression
		assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame.opcode);
		
		assertEquals(0, retValue);  // No bytes pending to flush
		
		int bytesWritten = client ? 170 : 150;
		
		assertEquals(bytesWritten, writeArgs.bytesWritten());
		
		assertEquals(bytesWritten, writeArgs.uncompressedBytesWritten());
		
		int ripcMsgPayload = (int)frame.payloadLen - TransportBufferImpl._firstFragmentHeaderLength;
		
		ByteBuffer msgBuffer = ByteBuffer.allocate((int)frame.payloadLen);
		msgBuffer.put(socketHelperMock.networkBuffer().array(), frame.hdrLen, (int)frame.payloadLen);
		msgBuffer.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer.array(), 0, msgBuffer.array().length);
		}
		
		/* Checks the first RIPC fragmented header */
		assertEquals((int)frame.payloadLen, msgBuffer.getShort());
		assertEquals(Ripc.Flags.HAS_OPTIONAL_FLAGS | RsslSocketChannel.IPC_DATA, msgBuffer.get());
		assertEquals(TransportBufferImpl.FRAGMENT_HEADER_RIPC_FLAGS, msgBuffer.get());
		assertEquals(RWF_MSG_2.length(), msgBuffer.getInt());
		assertEquals(fragId, msgBuffer.getShort());
		
		RWF_MSG_2_ByteBuffer.limit(ripcMsgPayload);
		
		// Checks the data of the first fragmented message
		assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
		
		WebSocketFrameHdr frame2 = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		partial = WebSocketFrameParser.decode(frame2, socketHelperMock.networkBuffer(), nextReadIndex, readSize);
		
		payloadLength = readSize - frame2.hdrLen;
		nextReadIndex += (int) (frame2.hdrLen + frame2.payloadLen);
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame2.payloadLen);
		assertEquals(true, frame2.finSet);
		assertEquals(false, frame2.rsv1Set); // Checks Compression
		assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame2.opcode);
		
		assertEquals(bytesWritten, writeArgs.bytesWritten());
		
		assertEquals(bytesWritten, writeArgs.uncompressedBytesWritten());
		
		int ripcMsgPayload2 = (int)frame2.payloadLen - TransportBufferImpl._nextFragmentHeaderLength;
		
		ByteBuffer msgBuffer2 = ByteBuffer.allocate((int)frame2.payloadLen);
		msgBuffer2.put(socketHelperMock.networkBuffer().array(), nextReadIndex - (int)frame2.payloadLen, (int)frame2.payloadLen);
		msgBuffer2.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame2.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer2.array(), 0, msgBuffer2.array().length);
		}
		
		/* Checks the next RIPC fragmented header */
		assertEquals((int)frame2.payloadLen, msgBuffer2.getShort());
		assertEquals(Ripc.Flags.HAS_OPTIONAL_FLAGS | RsslSocketChannel.IPC_DATA, msgBuffer2.get());
		assertEquals(TransportBufferImpl.FRAGMENT_RIPC_FLAGS, msgBuffer2.get());
		assertEquals(fragId, msgBuffer2.getShort());
		
		RWF_MSG_2_ByteBuffer.limit(ripcMsgPayload + ripcMsgPayload2);
		RWF_MSG_2_ByteBuffer.position(ripcMsgPayload);
		
		// Checks the data of the next fragmented message
		assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer2));
		
		WebSocketFrameHdr frame3 = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		partial = WebSocketFrameParser.decode(frame3, socketHelperMock.networkBuffer(), nextReadIndex, readSize);
		
		payloadLength = readSize - frame3.hdrLen;
		nextReadIndex += (int) (frame3.hdrLen + frame3.payloadLen);
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame3.payloadLen);
		assertEquals(true, frame3.finSet);
		assertEquals(false, frame3.rsv1Set); // Checks Compression
		assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame3.opcode);
		
		assertEquals(bytesWritten, writeArgs.bytesWritten());
		
		assertEquals(bytesWritten, writeArgs.uncompressedBytesWritten());
		
		int ripcMsgPayload3 = (int)frame3.payloadLen - TransportBufferImpl._nextFragmentHeaderLength;
		
		ByteBuffer msgBuffer3 = ByteBuffer.allocate((int)frame3.payloadLen);
		msgBuffer3.put(socketHelperMock.networkBuffer().array(), nextReadIndex - (int)frame3.payloadLen, (int)frame3.payloadLen);
		msgBuffer3.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame3.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer3.array(), 0, msgBuffer3.array().length);
		}
		
		/* Checks the next RIPC fragmented header */
		assertEquals((int)frame3.payloadLen, msgBuffer3.getShort());
		assertEquals(Ripc.Flags.HAS_OPTIONAL_FLAGS | RsslSocketChannel.IPC_DATA, msgBuffer3.get());
		assertEquals(TransportBufferImpl.FRAGMENT_RIPC_FLAGS, msgBuffer3.get());
		assertEquals(fragId, msgBuffer3.getShort());
		
		RWF_MSG_2_ByteBuffer.limit(ripcMsgPayload + ripcMsgPayload2 + ripcMsgPayload3);
		RWF_MSG_2_ByteBuffer.position(ripcMsgPayload + ripcMsgPayload2);
		
		// Checks the data of the next fragmented message
		assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer3));
		
		WebSocketFrameHdr frame4 = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		partial = WebSocketFrameParser.decode(frame4, socketHelperMock.networkBuffer(), nextReadIndex, readSize);
		
		payloadLength = readSize - frame4.hdrLen;
		nextReadIndex += (int) (frame4.hdrLen + frame4.payloadLen);
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame4.payloadLen);
		assertEquals(true, frame4.finSet);
		assertEquals(false, frame4.rsv1Set); // Checks Compression
		assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame4.opcode);
		
		assertEquals(bytesWritten, writeArgs.bytesWritten());
		
		assertEquals(bytesWritten, writeArgs.uncompressedBytesWritten());
		
		int ripcMsgPayload4 = (int)frame4.payloadLen - TransportBufferImpl._nextFragmentHeaderLength;
		
		ByteBuffer msgBuffer4 = ByteBuffer.allocate((int)frame4.payloadLen);
		msgBuffer4.put(socketHelperMock.networkBuffer().array(), nextReadIndex - (int)frame4.payloadLen, (int)frame4.payloadLen);
		msgBuffer4.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame4.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer4.array(), 0, msgBuffer4.array().length);
		}
		
		/* Checks the next RIPC fragmented header */
		assertEquals((int)frame4.payloadLen, msgBuffer4.getShort());
		assertEquals(Ripc.Flags.HAS_OPTIONAL_FLAGS | RsslSocketChannel.IPC_DATA, msgBuffer4.get());
		assertEquals(TransportBufferImpl.FRAGMENT_RIPC_FLAGS, msgBuffer4.get());
		assertEquals(fragId, msgBuffer4.getShort());
		
		RWF_MSG_2_ByteBuffer.limit(ripcMsgPayload + ripcMsgPayload2 + ripcMsgPayload3 + ripcMsgPayload4);
		RWF_MSG_2_ByteBuffer.position(ripcMsgPayload + ripcMsgPayload2 + ripcMsgPayload3);
		
		// Checks the data of the next fragmented message
		assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer4));
		
		// Reads the last fragmented message
		readSize = client ? 14 : 10;
		
		WebSocketFrameHdr frame5 = new WebSocketFrameHdr();
		
		/* Checks the WebSocket frame header */
		partial = WebSocketFrameParser.decode(frame5, socketHelperMock.networkBuffer(), nextReadIndex, readSize);
		
		payloadLength = readSize - frame5.hdrLen;
		nextReadIndex += (int) (frame5.hdrLen + frame5.payloadLen);
		
		assertEquals(false, partial);
		assertEquals(payloadLength, frame5.payloadLen);
		assertEquals(true, frame5.finSet);
		assertEquals(false, frame5.rsv1Set); // Checks Compression
		assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame5.opcode);
		
		assertEquals(bytesWritten, writeArgs.bytesWritten());
		
		assertEquals(bytesWritten, writeArgs.uncompressedBytesWritten());
		
		int ripcMsgPayload5 = (int)frame5.payloadLen - TransportBufferImpl._nextFragmentHeaderLength;
		
		ByteBuffer msgBuffer5 = ByteBuffer.allocate((int)frame5.payloadLen);
		msgBuffer5.put(socketHelperMock.networkBuffer().array(), nextReadIndex - (int)frame5.payloadLen, (int)frame5.payloadLen);
		msgBuffer5.rewind();
		
		if(client)
		{
			byte[] mask = new byte[4];
			
			WebSocketFrameParser.setMaskKey(mask, frame5.maskVal);
			
			/* Unmask the payload data */
			WebSocketFrameParser.maskDataBlock(mask, msgBuffer5.array(), 0, msgBuffer5.array().length);
		}
		
		/* Checks the next RIPC fragmented header */
		assertEquals((int)frame5.payloadLen, msgBuffer5.getShort());
		assertEquals(Ripc.Flags.HAS_OPTIONAL_FLAGS | RsslSocketChannel.IPC_DATA, msgBuffer5.get());
		assertEquals(TransportBufferImpl.FRAGMENT_RIPC_FLAGS, msgBuffer5.get());
		assertEquals(fragId, msgBuffer5.getShort());
		
		RWF_MSG_2_ByteBuffer.limit(ripcMsgPayload + ripcMsgPayload2 + ripcMsgPayload3 + ripcMsgPayload4 + ripcMsgPayload5);
		RWF_MSG_2_ByteBuffer.position(ripcMsgPayload + ripcMsgPayload2 + ripcMsgPayload3 + ripcMsgPayload4);
		
		// Checks the data of the next fragmented message
		assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer5));
    }
    
    public void readFragmentedJSONMessage(boolean client, boolean compressed)
    {
    	Transport._globalLock = new DummyLock();
    	RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.JSON_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		channel.getWsSession().isClient = client;
		
		if(compressed)
		{
			channel.getWsSession().setDeflate(true);
			channel.getWsSession().applyNoOutboundContextTakeOver();
			channel._compressor = new ZlibCompressor(6, true);
			channel._compressor.appendCompressTrailing();
			channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
			channel._sessionCompLowThreshold = 30;
		}
		
		/* Overrides the default max fragmentation size */
		channel._channelInfo._maxFragmentSize = (6 * 5) - RsslSocketChannel.RIPC_PACKED_HDR_SIZE;
		channel._internalMaxFragmentSize = (6 * 5) + RsslSocketChannel.RIPC_HDR_SIZE + _WS_MAX_HEADER_LEN;
		
		channel._ipcProtocol = new Ripc14Protocol();
		channel._ipcProtocol.channel(channel);
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
		
		channel.createBigBufferPool(channel._internalMaxFragmentSize);
		
		channel._transport = new SocketProtocol();
		channel.growGuaranteedOutputBuffers(100);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		WriteArgs writeArgs = TransportFactory.createWriteArgs();

		TransportBuffer writeBuffer = null;
				
		writeBuffer = channel.getBuffer(JSON_MSG_1.length(), false, error);
			
		assertNotNull(writeBuffer);
		
		assertEquals(JSON_MSG_1.length(), writeBuffer.length());
		
		writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		
		writeBuffer.data().put(JSON_MSG_1.getBytes());
		
		int retValue = 0;

		/* Simulate sending data from the server side */
		channel.getWsSession().isClient = !client;
		retValue = channel.write(writeBuffer, writeArgs, error);
		
		if(!client)
		{
			assertEquals(compressed ? 117 : 166, writeArgs.bytesWritten());
			assertEquals(compressed ? 154 : 166, writeArgs.uncompressedBytesWritten());
		}
		else
		{
			assertEquals(compressed ? 105 : 146, writeArgs.bytesWritten());
			assertEquals(compressed ? 142 : 146, writeArgs.uncompressedBytesWritten());
		}
		
		
		assertEquals(0, retValue); 
		
		channel.getWsSession().isClient = client;
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNull(readBuffer);
		
		assertNotNull(channel.getWsSession().reassemblyBuffer);
		
		if(!client)
		{
			assertEquals(compressed ? 78 : 127, readArgs.readRetVal());  // Has more data to read
			assertEquals(compressed ? 117 : 166, readArgs.bytesRead());
			assertEquals(compressed ? 0 : 39, readArgs.uncompressedBytesRead());
		}
		else
		{
			assertEquals(compressed ? 70 : 111, readArgs.readRetVal());  // Has more data to read
			assertEquals(compressed ? 105 : 146, readArgs.bytesRead());
			assertEquals(compressed ? 0 : 35, readArgs.uncompressedBytesRead());
		}
		
		readBuffer = channel.read(readArgs, error);
		assertNull(readBuffer);
		
		if(!client)
		{
			assertEquals(compressed ? 39 : 88, readArgs.readRetVal());  // Has more data to read
			assertEquals(0, readArgs.bytesRead());
			assertEquals(compressed ? 0 : 39, readArgs.uncompressedBytesRead());
		}
		else
		{
			assertEquals(compressed ? 35 : 76, readArgs.readRetVal());  // Has more data to read
			assertEquals(0, readArgs.bytesRead());
			assertEquals(compressed ? 0 : 35, readArgs.uncompressedBytesRead());
		}
		
		readBuffer = channel.read(readArgs, error);
		
		if(!compressed)
		{
			assertNull(readBuffer);
			
			if(!client)
			{
				assertEquals(49, readArgs.readRetVal());  // Has more data to read
				assertEquals(0, readArgs.bytesRead());
				assertEquals(39, readArgs.uncompressedBytesRead());
			}
			else
			{
				assertEquals(41, readArgs.readRetVal());  // Has more data to read
				assertEquals(0, readArgs.bytesRead());
				assertEquals(35, readArgs.uncompressedBytesRead());
			}	
			
			readBuffer = channel.read(readArgs, error);
			assertNull(readBuffer);
			
			if(!client)
			{
				assertEquals(10, readArgs.readRetVal());  // Has more data to read
				assertEquals(0, readArgs.bytesRead());
				assertEquals(39, readArgs.uncompressedBytesRead());
			}
			else
			{
				assertEquals(6, readArgs.readRetVal());  // Has more data to read
				assertEquals(0, readArgs.bytesRead());
				assertEquals(35, readArgs.uncompressedBytesRead());
			}
			
			readBuffer = channel.read(readArgs, error);
			assertNotNull(readBuffer);
			
		}
		else
		{
			assertNotNull(readBuffer);
			assertEquals(0, readArgs.bytesRead());
			assertEquals(client ? 142 : 154, readArgs.uncompressedBytesRead());
		}
			
		assertEquals(0, readArgs.readRetVal());  // No more data to read
		
		assertEquals(JSON_MSG_1_READ.length(), readBuffer.length());
		assertEquals(0, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, JSON_MSG_1_READ_ByteBuffer.compareTo(msgBuffer));
		
		/* Ensure that next read returns the assembly buffer back to the pool properly. */
		readBuffer = channel.read(readArgs, error);
		assertNull(readBuffer);
		
		assertNull(channel.getWsSession().reassemblyBuffer);
    }
    
    public void readFragmentedRWFMessages(boolean client, boolean compressed)
    {
    	Transport._globalLock = new DummyLock();
    	RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, Codec.RWF_PROTOCOL_TYPE);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		channel.getWsSession().isClient = client;
		
		if(compressed)
		{
			channel.getWsSession().setDeflate(true);
			channel.getWsSession().applyNoOutboundContextTakeOver();
			channel._compressor = new ZlibCompressor();
			channel._sessionOutCompression = Ripc.CompressionTypes.ZLIB;
			channel._sessionCompLowThreshold = 30;
		}
		
		
		/* Overrides the default max fragmentation size */
		channel._channelInfo._maxFragmentSize = (6 * 5) - RsslSocketChannel.RIPC_PACKED_HDR_SIZE;
		channel._internalMaxFragmentSize = (6 * 5) + RsslSocketChannel.RIPC_HDR_SIZE + _WS_MAX_HEADER_LEN;
		
		channel._ipcProtocol = new Ripc14Protocol();
		channel._ipcProtocol.channel(channel);
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		channel._protocolFunctions = new WSProtocolFunctions(channel);
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
		channel._readBufStateMachine.ripcVersion(Ripc.RipcVersions.VERSION14);
		
		channel.createBigBufferPool(channel._internalMaxFragmentSize);
		
		channel._transport = new SocketProtocol();
		channel.growGuaranteedOutputBuffers(100);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		WriteArgs writeArgs = TransportFactory.createWriteArgs();

		TransportBuffer writeBuffer = null;
				
		writeBuffer = channel.getBuffer(RWF_MSG_2.length(), false, error);
			
		assertNotNull(writeBuffer);
		
		assertEquals(RWF_MSG_2.length(), writeBuffer.length());
		
		writeArgs.flags(WriteFlags.DIRECT_SOCKET_WRITE);
		
		writeBuffer.data().put(RWF_MSG_2.getBytes());
		
		int retValue = 0;

		/* Simulate sending data from the server side */
		channel.getWsSession().isClient = !client;
		retValue = channel.write(writeBuffer, writeArgs, error);
		
		assertEquals(0, retValue); 
		
		channel.getWsSession().isClient = client;
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNull(channel.getWsSession().reassemblyBuffer);
		
		assertNull(readBuffer);
		
		if(!client)
		{
			assertEquals(compressed ? 143 : 131, readArgs.readRetVal());  // Has more data to read
			assertEquals(compressed ? 173 : 170, readArgs.bytesRead());
			assertEquals(33, readArgs.uncompressedBytesRead());
		}
		else
		{
			assertEquals(compressed ? 123 :115, readArgs.readRetVal());  // Has more data to read
			assertEquals(compressed ? 149 :150, readArgs.bytesRead());
			assertEquals(33, readArgs.uncompressedBytesRead());
		}
		
		readBuffer = channel.read(readArgs, error);
		assertNull(readBuffer);
		
		if(!client)
		{
			assertEquals(compressed ? 106 : 92, readArgs.readRetVal());  // Has more data to read
			assertEquals(0, readArgs.bytesRead());
			assertEquals(33, readArgs.uncompressedBytesRead());
		}
		else
		{
			assertEquals(compressed ? 90 : 80, readArgs.readRetVal());  // Has more data to read
			assertEquals(0, readArgs.bytesRead());
			assertEquals(33, readArgs.uncompressedBytesRead());
		}
		
		readBuffer = channel.read(readArgs, error);
		assertNull(readBuffer);
		
		if(!client)
		{
			assertEquals(compressed ? 67 : 53, readArgs.readRetVal());  // Has more data to read
			assertEquals(0, readArgs.bytesRead());
			assertEquals(compressed ? 0 : 33, readArgs.uncompressedBytesRead());
		}
		else
		{
			assertEquals(compressed ? 55 : 45, readArgs.readRetVal());  // Has more data to read
			assertEquals(0, readArgs.bytesRead());
			assertEquals(compressed ? 0 : 33, readArgs.uncompressedBytesRead());
		}
			
		
		readBuffer = channel.read(readArgs, error);
		assertNull(readBuffer);
		
		if(!client)
		{
			assertEquals(compressed ? 53 : 14, readArgs.readRetVal());  // Has more data to read
			assertEquals(0, readArgs.bytesRead());
			assertEquals(compressed ? 36 : 33, readArgs.uncompressedBytesRead());
		}
		else
		{
			assertEquals(compressed ? 45 : 10, readArgs.readRetVal());  // Has more data to read
			assertEquals(0, readArgs.bytesRead());
			assertEquals(compressed ? 36 : 33, readArgs.uncompressedBytesRead());
		}
		
		if(compressed)
		{
			readBuffer = channel.read(readArgs, error);
			assertNull(readBuffer);
			
			if(!client)
			{
				assertEquals(14, readArgs.readRetVal());  // Has more data to read
				assertEquals(0, readArgs.bytesRead());
				assertEquals(33, readArgs.uncompressedBytesRead());
			}
			else
			{
				assertEquals(10, readArgs.readRetVal());  // Has more data to read
				assertEquals(0, readArgs.bytesRead());
				assertEquals(33, readArgs.uncompressedBytesRead());
			}
			
			readBuffer = channel.read(readArgs, error);
			assertNotNull(readBuffer);
			assertEquals(0, readArgs.readRetVal());  // No more data to read
			
			if(!client)
			{
				assertEquals(0, readArgs.bytesRead());
				assertEquals(8, readArgs.uncompressedBytesRead());
			}
			else
			{
				assertEquals(0, readArgs.bytesRead());
				assertEquals(8, readArgs.uncompressedBytesRead());
			}
		}
		else
		{
			readBuffer = channel.read(readArgs, error);
			assertNotNull(readBuffer);
			assertEquals(0, readArgs.readRetVal());  // No more data to read
			
			if(!client)
			{
				assertEquals(0, readArgs.bytesRead());
				assertEquals(8, readArgs.uncompressedBytesRead());
			}
			else
			{
				assertEquals(0, readArgs.bytesRead());
				assertEquals(8, readArgs.uncompressedBytesRead());
			}
		}
		
		assertEquals(RWF_MSG_2.length(), readBuffer.length());
		assertEquals(0, readBuffer.dataStartPosition());
		
		ByteBuffer msgBuffer = ByteBuffer.allocate(readBuffer.length());
		msgBuffer.put(readBuffer.data());
		msgBuffer.rewind();
		
		assertEquals(0, RWF_MSG_2_ByteBuffer.compareTo(msgBuffer));
    }
    
    public void readPingOrPongFrames(boolean isClient, int opCode, int protocolType, boolean withPayload)
    {
    	RsslSocketChannel channel = new RsslSocketChannel(ConnectionTypes.WEBSOCKET, protocolType);
		ReadArgs readArgs = TransportFactory.createReadArgs();
		Error error = TransportFactory.createError();
		
		channel._state = ChannelState.ACTIVE;
		channel._readIoBuffer = channel.acquirePair(RsslSocketChannel.MIN_READ_BUFFER_SIZE); // the "read buffer" for network I/O
		channel._appReadBuffer.data(channel._readIoBuffer.readOnly());
		WSProtocolFunctions wsProtocolFunctions = new WSProtocolFunctions(channel);
		wsProtocolFunctions.initializeBufferAndFrameData();
		channel._protocolFunctions = wsProtocolFunctions;
		channel._readBufStateMachine.initialize(channel._readIoBuffer, channel._protocolFunctions);
				
		SocketHelperMock socketHelperMock = new SocketHelperMock();
		
		channel._scktChannel = socketHelperMock;
		
		String payload = withPayload ? "This is test payload of ping/pong." : "";
		
		int hdrlen = WebSocketFrameParser.calculateHeaderLength(payload.length(), isClient);
		
		socketHelperMock.networkBuffer().position(hdrlen);
		
		if(withPayload)
			socketHelperMock.networkBuffer().put(payload.getBytes());
		
		/* Write WebSocket frame  */
		WebSocketFrameParser.encode(socketHelperMock.networkBuffer(), 0, payload.length(), protocolType,
				isClient, true, false, opCode);
		
		int bytesRead = isClient ? 6 : 2;
		
		bytesRead += payload.length();
		
		TransportBuffer readBuffer = channel.read(readArgs, error);
		
		assertNull(readBuffer);
		
		if (protocolType == Codec.JSON_PROTOCOL_TYPE)
		{
			assertEquals(TransportReturnCodes.READ_PING, readArgs.readRetVal());
			assertEquals(bytesRead, readArgs.bytesRead());
			assertEquals(bytesRead, readArgs.uncompressedBytesRead());
			
			socketHelperMock.networkBuffer().clear();
			
			readBuffer = channel.read(readArgs, error);
			
			 assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
			 assertEquals(0, readArgs.bytesRead());
			 assertEquals(0, readArgs.uncompressedBytesRead());
		}
		else
		{
			assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
			assertEquals(bytesRead, readArgs.bytesRead());
			assertEquals(0, readArgs.uncompressedBytesRead());
			
			socketHelperMock.networkBuffer().clear();
			
			readBuffer = channel.read(readArgs, error);
			 
			assertEquals(TransportReturnCodes.READ_WOULD_BLOCK, readArgs.readRetVal());
			assertEquals(0, readArgs.bytesRead());
			assertEquals(0, readArgs.uncompressedBytesRead());
		}
    }
    
    @Test
    public void writeOneJSONMessage_DirectWrite_Server()
    {
    	writeOneJSONMessage(true, false);
    }
    
    @Test
    public void writeOneJSONMessage_Server()
    {
    	writeOneJSONMessage(false, false);
    }
    
    @Test
    public void writeOneRWFMessage_DirectWrite_Server()
    {
    	writeOneRWFMessage(true, false);
    }
    
    @Test
    public void writeOneRWFMessage_Server()
    {
    	writeOneRWFMessage(false, false);
    }
	
    @Test
    public void writeOneJSONMessage_DirectWrite_Client()
    {
    	writeOneJSONMessage(true, true);
    }
    
    @Test
    public void writeOneJSONMessage_Client()
    {
    	writeOneJSONMessage(false, true);
    }
    
    @Test
    public void writeOneRWFMessage_DirectWrite_Client()
    {
    	writeOneRWFMessage(true, true);
    }
    
    @Test
    public void writeOneRWFMessage_Client()
    {
    	writeOneRWFMessage(false, true);
    }
    
    @Test
    public void writeJSONPackagingMessages_Server()
    {
    	writeJSONPackagingMessages(false, false, false);
    }
    
    @Test
    public void writeJSONPackagingMessages_DirectWrite_Server()
    {
    	writeJSONPackagingMessages(true, false, true);
    }
    
    @Test
    public void writeJSONPackagingMessages_Cleint()
    {
    	writeJSONPackagingMessages(false, true, true);
    }
    
    @Test
    public void writeJSONPackagingMessages_DirectWrite_Client()
    {
    	writeJSONPackagingMessages(true, true, false);
    }
    
    @Test
    public void writeRWFPackagingMessages_Server()
    {
    	writeRWFPackagingMessages(false, false, false);
    }
    
    @Test
    public void writeRWFPackagingMessages_DirectWrite_Server()
    {
    	writeRWFPackagingMessages(true, false, true);
    }
    
    @Test
    public void writeRWFPackagingMessages_Client()
    {
    	writeRWFPackagingMessages(false, true, true);
    }
    
    @Test
    public void writeRWFPackagingMessages_DirectWrite_Client()
    {
    	writeRWFPackagingMessages(true, true, false);
    }
    
    @Test
    public void readPackingJSONMessages_Server()
    {
    	readPackingJSONMessages(false);
    }
    
    @Test
    public void readPackingJSONMessages_Client()
    {
    	readPackingJSONMessages(true);
    }
    
    @Test
    public void readPackingRWFNMessages_Server()
    {
    	readPackingRWFMessages(false);
    }
    
    @Test
    public void readPackingRWFMessages_Client()
    {
    	readPackingRWFMessages(true);
    }
    
    @Test
    public void writeCompressedOneRWFMessage_Server()
    {
    	writeCompressedRWFMessages(false, false, false);
    }
    
    @Test
    public void writeCompressedOneRWFMessage_DirectWrite_Server()
    {
    	writeCompressedRWFMessages(true, false, false);
    }
    
    @Test
    public void writeCompressedPackingRWFMessages_DirectWrite_Server()
    {
    	writeCompressedRWFMessages(true, false, true);
    }
    
    @Test
    public void writeCompressedOneRWFMessage_Client()
    {
    	writeCompressedRWFMessages(false, true, false);
    }
    
    @Test
    public void writeCompressedOneRWFMessage_DirectWrite_Client()
    {
    	writeCompressedRWFMessages(true, true, false);
    }
    
    @Test
    public void writeCompressedPackingRWFMessages_DirectWrite_Client()
    {
    	writeCompressedRWFMessages(true, true, true);
    }
    
    @Test
    public void writeCompressedOneJSONMessage_Server()
    {
    	writeCompressedJSONMessages(false, false, false);
    }
    
    @Test
    public void writeCompressedOneJSONMessage_DirectWrite_Server()
    {
    	writeCompressedJSONMessages(true, false, false);
    }
    
    @Test
    public void writeCompressedPackingJSONMessages_DirectWrite_Server()
    {
    	writeCompressedJSONMessages(true, false, true);
    }
    
    @Test
    public void writeCompressedOneJSONMessage_Client()
    {
    	writeCompressedJSONMessages(false, true, false);
    }
    
    @Test
    public void writeCompressedOneJSONMessage_DirectWrite_Client()
    {
    	writeCompressedJSONMessages(true, true, false);
    }
    
    @Test
    public void writeCompressedPackingJSONMessages_DirectWrite_Client()
    {
    	writeCompressedJSONMessages(true, true, true);
    }
    
    @Test
    public void readCompressedPackingRWFMessages_Server()
    {
    	readCompressedPackingRWFMessages(false);
    }
    
    @Test
    public void readCompressedPackingRWFMessages_Client()
    {
    	readCompressedPackingRWFMessages(true);
    }
    
    @Test
    public void readCompressedRWFMessage_Server()
    {
    	readCompressedRWFMessage(false);
    }
    
    @Test
    public void readCompressedRWFMessage_Client()
    {
    	readCompressedRWFMessage(true);
    }
    
    @Test
    public void readCompressedJSONMessage_Server()
    {
    	readCompressedJSONMessages(false);
    }
    
    @Test
    public void readCompressedJSONMessage_Client()
    {
    	readCompressedJSONMessages(true);
    }
    
    @Test
    public void writeFragmentedJSONMssage_DirectWrite_Server()
    {
    	writeFragmentedJSONMessage(true, false);
    }
    
    @Test
    public void writeFragmentedJSONMssage_Server()
    {
    	writeFragmentedJSONMessage(false, false);
    }
    
    @Test
    public void writeFragmentedJSONMssage_DirectWrite_Client()
    {
    	writeFragmentedJSONMessage(true, true);
    }
    
    @Test
    public void writeFragmentedJSONMssage_Client()
    {
    	writeFragmentedJSONMessage(false, true);
    }
    
    @Test
    public void writeFragmentedRWFMssage_DirectWrite_Server()
    {
    	writeFragmentedRWFMessage(true, false);
    }
    
    @Test
    public void writeFragmentedRWFMssage_Server()
    {
    	writeFragmentedRWFMessage(false, false);
    }
    
    @Test
    public void writeFragmentedRWFMssage_DirectWrite_Client()
    {
    	writeFragmentedRWFMessage(true, true);
    }
    
    @Test
    public void writeFragmentedRWFMssage_Client()
    {
    	writeFragmentedRWFMessage(false, true);
    }
    
    @Test
    public void readFragmentedJSONMssage_Server()
    {
    	readFragmentedJSONMessage(false, false);
    }
    
    @Test
    public void readFragmentedJSONMssage_Client()
    {
    	readFragmentedJSONMessage(true, false);
    }
    
    @Test
    public void readFragmentedJSONMssage_Server_Compressed()
    {
    	readFragmentedJSONMessage(false, true);
    }
    
    @Test
    public void readFragmentedJSONMssage_Client_Compressed()
    {
    	readFragmentedJSONMessage(true, true);
    }
    
    @Test
    public void readFragmentedRWFMssage_Server()
    {
    	readFragmentedRWFMessages(false, false);
    }
    
    @Test
    public void readFragmentedRWFNMssage_Client()
    {
    	readFragmentedRWFMessages(true, false);
    }
    
    @Test
    public void readFragmentedRWFMssage_Server_Compressed()
    {
    	readFragmentedRWFMessages(false, true);
    }
    
    @Test
    public void readFragmentedRWFNMssage_Client_Compressed()
    {
    	readFragmentedRWFMessages(true, true);
    }
    
    @Test
    public void readPingAndPongFrame_Client_RWFProtocol()
    {
    	readPingOrPongFrames(true, WebSocketFrameParser._WS_OPC_PING, Codec.RWF_PROTOCOL_TYPE, true);
    	
    	readPingOrPongFrames(true, WebSocketFrameParser._WS_OPC_PONG, Codec.RWF_PROTOCOL_TYPE, false);
    }
    
    @Test
    public void readPingAndPongFrame_Server_RWFProtocol()
    {
    	readPingOrPongFrames(false, WebSocketFrameParser._WS_OPC_PING, Codec.RWF_PROTOCOL_TYPE, false);
    	
    	readPingOrPongFrames(false, WebSocketFrameParser._WS_OPC_PONG, Codec.RWF_PROTOCOL_TYPE, true);
    }
    
    @Test
    public void readPingAndPongFrame_Client_JSONProtocol()
    {
    	readPingOrPongFrames(true, WebSocketFrameParser._WS_OPC_PING, Codec.JSON_PROTOCOL_TYPE, false);
    	
    	readPingOrPongFrames(true, WebSocketFrameParser._WS_OPC_PONG, Codec.JSON_PROTOCOL_TYPE, true);
    }
    
    @Test
    public void readPingAndPongFrame_Server_JSONProtocol()
    {
    	readPingOrPongFrames(false, WebSocketFrameParser._WS_OPC_PING, Codec.JSON_PROTOCOL_TYPE, true);
    	
    	readPingOrPongFrames(false, WebSocketFrameParser._WS_OPC_PONG, Codec.JSON_PROTOCOL_TYPE, false);
    }
    
    @Test
    public void givenActiveWSRWFConnection_whenPingPongConnection_thenPingSentAndPongReceived() {
		executePingPongInteraction(Codec.RWF_PROTOCOL_TYPE);
	}

	@Test
	public void givenActiveWSJSONConnection_whenPingPongConnection_thenPingSentAndPongReceived() {
		executePingPongInteraction(Codec.JSON_PROTOCOL_TYPE);
	}
}
