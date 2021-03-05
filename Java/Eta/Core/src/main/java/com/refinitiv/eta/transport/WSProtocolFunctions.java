package com.refinitiv.eta.transport;

import com.refinitiv.eta.codec.Codec;

import java.io.IOException;
import java.nio.ByteBuffer;
import java.util.Objects;
import java.util.Optional;

class WSProtocolFunctions implements ProtocolFunctions {

	//[{\"Type\":\"Ping\"}]
	static final byte[] WEB_SOCKET_JSON_PING_MESSAGE = new byte[] {
			0X5B, 0X7B, 0X22, 0X54, 0X79, 0X70, 0X65, 0X22, 0X3A, 0X22, 0X50, 0X69, 0X6E, 0X67, 0X22, 0X7D, 0X5D
	};

	//[{\"Type\":\"Pong\"}]
	static final byte[] WEB_SOCKET_JSON_PONG_MESSAGE = new byte[] {
			0X5B, 0X7B, 0X22, 0X54, 0X79, 0X70, 0X65, 0X22, 0X3A, 0X22, 0X50, 0X6F, 0X6E, 0X67, 0X22, 0X7D, 0X5D
	};

	private static final byte[] RWF_PING_PONG_MESSAGE = new byte[] {0x00, 0x03, 0x02};

	private final RsslSocketChannel _rsslSocketChannel;
	private final WebSocketSession _webSocketSession;
	private final ReadBufferStateMachine _readBufferStateMachine;
	ByteBuffer pingBuffer;
	ByteBuffer pongBuffer;

	ByteBuffer wsFrameBuffer;
	int wsFrameHeaderLength;
	private final byte[] maskArray = new byte[4];
	
	public WSProtocolFunctions(RsslSocketChannel rsslSocketChannel)
	{
		this._rsslSocketChannel = rsslSocketChannel;
		this._webSocketSession = rsslSocketChannel.getWsSession();
		_readBufferStateMachine = rsslSocketChannel._readBufStateMachine;
	}
	
	public void initializeBufferAndFrameData()
	{
		initMsgBuffers();
		initFrameData();
	}

	private void initFrameData() {
		wsFrameHeaderLength = WebSocketFrameParser.calculateHeaderLength(WebSocketFrameParser._WS_MAX_FRAME_LENGTH, _webSocketSession.isClient);
		int wsFrameBufferLength = wsFrameHeaderLength + WebSocketFrameParser._WS_MAX_FRAME_LENGTH;
		if (Objects.equals(Codec.RWF_PROTOCOL_TYPE, _webSocketSession.getAcceptedProtocol())) {
			wsFrameBufferLength += TransportBufferImpl.RIPC_WRITE_POSITION;
		}
		wsFrameBuffer = ByteBuffer.allocate(wsFrameBufferLength);
	}

	private void initMsgBuffers() {
		final boolean isJsonProtocol = Objects.equals(Codec.JSON_PROTOCOL_TYPE, _webSocketSession.getAcceptedProtocol());
		final boolean isCompressed = isJsonProtocol
									 && _webSocketSession.isDeflate()
									 && _rsslSocketChannel._sessionOutCompression == Ripc.CompressionTypes.ZLIB;
		final byte[] pingMsgData = isJsonProtocol ? WEB_SOCKET_JSON_PING_MESSAGE : RWF_PING_PONG_MESSAGE;
		int payloadLength = pingMsgData.length;
		final int estimatedPayloadLength = isCompressed ? _rsslSocketChannel._compressor.getMaxCompressedLength(payloadLength) : payloadLength;
		final int pingPongWSHeaderLength = WebSocketFrameParser.calculateHeaderLength(payloadLength, _webSocketSession.isClient);
		final int pingPongLength = estimatedPayloadLength + pingPongWSHeaderLength;

		pingBuffer = preparePingPongBuffer(pingPongWSHeaderLength, pingPongLength, pingMsgData, isCompressed);
		if (isJsonProtocol) {
			pongBuffer = preparePingPongBuffer(pingPongWSHeaderLength, pingPongLength, WEB_SOCKET_JSON_PONG_MESSAGE, isCompressed);
		} else {
			pongBuffer = pingBuffer;
		}
	}

	private ByteBuffer preparePingPongBuffer(int headerLength, int bufferSize, byte[] data, boolean compressed) {
		int payloadLength = data.length;
		ByteBuffer toInit = ByteBuffer.allocate(bufferSize + 14);
		toInit.position(headerLength);
		toInit.put(data);
		toInit.rewind();
		
		boolean compressionFlag = false;

		//Check if compression enabled...
		if (compressed && _webSocketSession.getAcceptedProtocol() == Codec.JSON_PROTOCOL_TYPE) {
			payloadLength = _rsslSocketChannel._compressor.compress(toInit, headerLength, payloadLength);
			payloadLength -= 4;
			
			// Replaces the buffer with the compressed data
			toInit.position(headerLength);
			toInit.put(_rsslSocketChannel._compressor.compressedData(), 0, payloadLength);
			compressionFlag = true;
		}
		
		//Insert WebSocket Frame Header before ping msg.
		int hdrLength = WebSocketFrameParser.encode(toInit, 0, payloadLength, _webSocketSession.getAcceptedProtocol(),
				_webSocketSession.isClient, true, compressionFlag, WebSocketFrameParser._WS_SP_NONE);
		toInit.limit(hdrLength + payloadLength);
		return toInit;
	}
	
	@Override
	@SuppressWarnings("fallthrough")
	public int readPrependTransportHdr(int bytesRead, ReadArgsImpl readArgs, Error error)
	{
		final ByteBuffer _readIoBuffer = _rsslSocketChannel._readIoBuffer.buffer();
		boolean partial = WebSocketFrameParser.decode(_webSocketSession.wsFrameHdr, _readIoBuffer,
				_readBufferStateMachine._currentMsgStartPos, _readIoBuffer.position() - _readBufferStateMachine._currentMsgStartPos);
		
		if(partial)
		{
			if ( (_webSocketSession.wsFrameHdr.hdrLen != 0) && (_readBufferStateMachine._currentMsgStartPos + _webSocketSession.wsFrameHdr.hdrLen < _readIoBuffer.position()) )
			{
				if(_rsslSocketChannel.protocolType() != Codec.RWF_PROTOCOL_TYPE)
				{
					if (_readBufferStateMachine._currentMsgStartPos + messageLength() > _readIoBuffer.limit())
					{
						_webSocketSession.wsFrameHdr.reset();
						_readBufferStateMachine._state = ReadBufferState.KNOWN_INSUFFICENT; /* Insufficient space to read more data*/
					}
					else
					{
						_readBufferStateMachine._state = ReadBufferState.KNOWN_INCOMPLETE; /* Known the length of the WebSocket payload */
					}
				}
				else
				{
					if (_readBufferStateMachine._currentMsgStartPos + _webSocketSession.wsFrameHdr.hdrLen + _webSocketSession.wsFrameHdr.payloadLen > _readIoBuffer.limit())
					{
						_webSocketSession.wsFrameHdr.reset();
						_readBufferStateMachine._state = ReadBufferState.UNKNOWN_INSUFFICIENT; /* Insufficient space to read more data*/
					}
					else
					{
						_readBufferStateMachine._state = ReadBufferState.UNKNOWN_INCOMPLETE; /* There is more buffer space to read data */
					}
				}
			}
			else if(_webSocketSession.wsFrameHdr.hdrLen != 0 && _readBufferStateMachine._currentMsgStartPos + _webSocketSession.wsFrameHdr.hdrLen < _readIoBuffer.limit())
			{
				_readBufferStateMachine._state = ReadBufferState.UNKNOWN_INCOMPLETE;
			}
			else
			{
				_webSocketSession.wsFrameHdr.reset();
				_readBufferStateMachine._state = ReadBufferState.UNKNOWN_INSUFFICIENT;
			}
			
			return 1;
		}
		else
		{
			final WebSocketFrameHdr frame = _webSocketSession.wsFrameHdr;
			final ByteBuffer readIoBuffer = _rsslSocketChannel._readIoBuffer.buffer();

			if (frame.maskSet) {
				WebSocketFrameParser.setMaskKey(maskArray, frame.maskVal);

				/* Unmask the payload data */
				WebSocketFrameParser.maskDataBlock(maskArray, readIoBuffer, frame.payloadIndex, (int) frame.payloadLen);
			}
			
			switch(_webSocketSession.wsFrameHdr.opcode)
			{
			case WebSocketFrameParser._WS_OPC_PING:
				if (sendWsFramePong(null, error) < TransportReturnCodes.FAILURE) {
					setError(error, TransportReturnCodes.FAILURE, "Failed sending WS Pong Frame.");
				}
				
				/* Fall through to notify application with TransportReturnCodes.READ_PING code */
				
			case WebSocketFrameParser._WS_OPC_PONG:
				
				if(_rsslSocketChannel.protocolType() != Codec.RWF_PROTOCOL_TYPE)
				{
					/* Handling ping or pong frame from network */
					_readBufferStateMachine._state = ReadBufferState.KNOWN_COMPLETE;
					_readBufferStateMachine._subState = ReadBufferSubState.NORMAL;
					return 1;
				}
				else
				{
					/* Checks whether this is the last message in the buffer */
					if( (_readBufferStateMachine._currentMsgStartPos + _webSocketSession.wsFrameHdr.hdrLen + _webSocketSession.wsFrameHdr.payloadLen) == _readIoBuffer.position())
					{
						_readBufferStateMachine._state = ReadBufferState.NO_DATA;
						_readBufferStateMachine._subState = ReadBufferSubState.NORMAL;
						return 1;
					}
					else
					{
						/* Move the buffer's position beyond this message */
						_readBufferStateMachine._currentMsgStartPos += (_webSocketSession.wsFrameHdr.hdrLen + _webSocketSession.wsFrameHdr.payloadLen);
						
						_webSocketSession.wsFrameHdr.reset();
						
						return readPrependTransportHdr(bytesRead, readArgs, error);
					}
				}
			case WebSocketFrameParser._WS_OPC_CLOSE:
				WebSocketCloseStatus closeStatus = null;
				_webSocketSession.recvClose = true;

				if (frame.payloadLen >= 2) {
					final int closeCode = frame.buffer.getShort(frame.payloadIndex);
					closeStatus = WebSocketCloseStatus.findByCode(closeCode);
					if (Objects.isNull(closeStatus)) {
						setError(error, TransportReturnCodes.FAILURE, "Unrecognized close code %d", closeCode);
					} else {
						setError(error, TransportReturnCodes.FAILURE, "WS Close Code %d %s", closeStatus.getCode(), closeStatus.getTextMessage());
					}
				}

				if (!_webSocketSession.sendClose) {
					if (sendWsFrameClose(WebSocketCloseStatus.WS_CFSC_ENDPOINT_GONE, error) < 0) {
						setError(error, TransportReturnCodes.FAILURE, "Failed to send close frame to the remote endpoint.");
					}
				}
				_readBufferStateMachine._state = ReadBufferState.END_OF_STREAM;
				return 1;
			default:
				break;
			}
		}
		
		return 0;
	}

	@Override
	public int prependInitChnlHdr(ByteBuffer sourceData, ByteBuffer destinationData) {
		destinationData.clear();
		final int dataLength = sourceData.limit();
		final int wsInitChnlLength = WebSocketFrameParser.calculateHeaderLength(dataLength, _webSocketSession.isClient);
		destinationData.position(wsInitChnlLength);
		destinationData.put(sourceData);
		int returnCode = WebSocketFrameParser.encode(destinationData, 0, dataLength, Codec.RWF_PROTOCOL_TYPE, _webSocketSession.isClient,
				true, false, WebSocketFrameParser._WS_SP_NONE);
		destinationData.flip();
		return returnCode;
	}

	@Override
	public int initChnlReadFromChannel(ByteBuffer dest, Error error) throws IOException {
		int bytesRead = _rsslSocketChannel.read(dest);
		if (bytesRead > 0) {
			final WebSocketFrameHdr frameHdr = _webSocketSession.wsFrameHdr;
			frameHdr.clear();
			//TODO continue parse
			boolean partitialFrame = WebSocketFrameParser.decode(_webSocketSession.wsFrameHdr, dest, 0, dest.position());
			if (!partitialFrame && _webSocketSession.wsFrameHdr.maskSet) {
				WebSocketFrameParser.setMaskKey(maskArray, frameHdr.maskVal);

				/* Unmask the payload data */
				dest.position(_webSocketSession.wsFrameHdr.hdrLen);
				WebSocketFrameParser.maskDataBlock(maskArray, dest, dest.position(), (int) frameHdr.payloadLen);
			}
			return bytesRead;
		}
		return bytesRead;
	}

	@Override
	public int initChnlReadFromChannelProvider(ByteBuffer dest, Error error) throws IOException {
		int bytesRead = _rsslSocketChannel.read(dest);
		if (bytesRead <= 1 && _rsslSocketChannel._providerHelper != null && _rsslSocketChannel._providerHelper.wininetStreamingComplete()) {
			if (RsslHttpSocketChannelProvider.debugPrint)
				System.out.println(" Got pipe notify  " + bytesRead);
			_rsslSocketChannel._providerHelper._pipeNode._pipe.sink().close();
			_rsslSocketChannel._providerHelper._pipeNode._pipe.source().close();
			_rsslSocketChannel._providerHelper._pipeNode.returnToPool();
			// fake active for later close() to distinguish if coming from this
			// notify or timeout & INITIALING state
			_rsslSocketChannel._state = ChannelState.ACTIVE;
			return -1;
		}

		if (bytesRead > 0) {
			final WebSocketFrameHdr frameHdr = _webSocketSession.wsFrameHdr;
			frameHdr.clear();
			WebSocketFrameParser.decode(_webSocketSession.wsFrameHdr, dest, 0, dest.position());
			if (frameHdr.payloadLen > 0 && _webSocketSession.wsFrameHdr.maskSet) {
				WebSocketFrameParser.setMaskKey(maskArray, frameHdr.maskVal);

				/* Unmask the payload data */
				dest.position(_webSocketSession.wsFrameHdr.hdrLen);
				final int dataLength = dest.limit() - _webSocketSession.wsFrameHdr.hdrLen;
				WebSocketFrameParser.maskDataBlock(maskArray, dest, dest.position(), dataLength);
			}
			return bytesRead;
		} else if (bytesRead == -1) {
			_rsslSocketChannel._initChnlState = RsslSocketChannel.InitChnlState.RECONNECTING;
			return -1;
		}
		return 0;
	}

	@Override
	public int prependTransportHdr(TransportBufferImpl buffer, int ripcFlags) {
		int wsHdrlen = 0;

		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE) /* RWF protocol */
		{
			int bufferLen = buffer._isPacked ? buffer.packedLen() : buffer.length();
			int ripcMessageLength = bufferLen + TransportBufferImpl.RIPC_WRITE_POSITION;
			wsHdrlen = WebSocketFrameParser.calculateHeaderLength(ripcMessageLength, _webSocketSession.isClient);

			buffer._startWsHeader = buffer._startPosition + (WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHdrlen);

			int lastPosition = buffer._startWsHeader + wsHdrlen + ripcMessageLength;
			buffer._length = wsHdrlen + ripcMessageLength;
			buffer._data.position(buffer._startWsHeader + wsHdrlen);
			buffer._data.putShort((short)ripcMessageLength);
			buffer._data.put((byte)ripcFlags);
			buffer._data.limit(lastPosition);
			buffer._data.position(buffer._startWsHeader);
			
			/* Set the WebSocket frame header */
			WebSocketFrameParser.encode(buffer._data, buffer._startWsHeader, ripcMessageLength, _rsslSocketChannel.protocolType(),
					_webSocketSession.isClient, true, false, WebSocketFrameParser._WS_OPC_NONE);
		}
		else
		{
			int bufferLen = buffer.packedLen();
			wsHdrlen = WebSocketFrameParser.calculateHeaderLength(bufferLen,
					_webSocketSession.isClient);

			buffer._startWsHeader = buffer._startPosition + (WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHdrlen);

			/* Set the WebSocket frame header */
			WebSocketFrameParser.encode(buffer._data, buffer._startWsHeader, bufferLen, _rsslSocketChannel.protocolType(),
					_webSocketSession.isClient, true, _webSocketSession.isDeflate(), WebSocketFrameParser._WS_OPC_NONE);
			
			int lastPosition = buffer._data.position();
			buffer._length = lastPosition - buffer._startWsHeader;
			buffer._data.position(buffer._startWsHeader);
			buffer._data.limit(lastPosition);
		}
		
		/* Set the start position according to the actual data */
		buffer._startPosition = buffer._startWsHeader;

		buffer.opCode = WebSocketFrameParser._WS_SP_NONE;

		return wsHdrlen;
	}

	@Override
	public int messageLength() {
		
		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE)
		{	
			if(_readBufferStateMachine._currentMsgRipcLen != ReadBufferStateMachine.UNKNOWN_LENGTH)
			{
				return _webSocketSession.wsFrameHdr.hdrLen + _readBufferStateMachine._currentMsgRipcLen;
			}
			else
			{
				return _webSocketSession.wsFrameHdr.hdrLen;
			}
		}
		else
		{
			return (int) (_webSocketSession.wsFrameHdr.hdrLen + _webSocketSession.wsFrameHdr.payloadLen);
		}
	}

	@Override
	public void unsetMessageLength() {
		_webSocketSession.wsFrameHdr.reset();
		_readBufferStateMachine._currentMsgRipcLen = ReadBufferStateMachine.UNKNOWN_LENGTH;
	}

	@Override
	public int additionalHdrLength() {
			return _webSocketSession.wsFrameHdr.hdrLen;
	}

	@Override
	public int estimateHeaderLength()
	{	
		int headerLength = WebSocketFrameParser._WS_MAX_HEADER_LEN;
		
		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE)
		{
			headerLength += RsslSocketChannel.RIPC_HDR_SIZE;
		}
		
		return headerLength;
	}


	@Override
	public int entireHeaderLength() {
		
		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE)
		{
			if(_readBufferStateMachine._currentMsgRipcLen != ReadBufferStateMachine.UNKNOWN_LENGTH)
			{
				return _webSocketSession.wsFrameHdr.hdrLen + RsslSocketChannel.RIPC_HDR_SIZE;
			}
			else
			{
				return _webSocketSession.wsFrameHdr.hdrLen;
			}
		}
		else
		{
			return _webSocketSession.wsFrameHdr.hdrLen;
		}
	}

	@Override
	public int writeAdditionalMessagePrefix(TransportBufferImpl buffer) {

		if(_rsslSocketChannel.protocolType() == Codec.JSON_PROTOCOL_TYPE) /* JSON protocol */
		{
			buffer._data.put(buffer.dataStartPosition(), (byte)'[');
			buffer._dataStartOffSet = 1; /* Move data data start position beyond '[' */
			buffer._packedMsgOffSetPosition = buffer.dataStartPosition();
			buffer._data.position(buffer._packedMsgOffSetPosition);
		}
		
		return 0;
	}


	@Override
	public int packBuffer(TransportBufferImpl packedBuffer, boolean reserveNextPackedHdr, Channel chnl, Error error) {
		
		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE) /* RWF protocol */
		{
			return TransportBufferImpl.packBuffer(packedBuffer, reserveNextPackedHdr, chnl, error);
		}
		else
		{	
			if (reserveNextPackedHdr)
	        {
	            // attempt to reserve space for the next JSON message delimiter.
	            if ((packedBuffer.data().position() + TransportBufferImpl.PACKED_JSON_MSG_DELIMITER) > (packedBuffer.data().limit() - 1))
	                return 0;
	            
	            // write the JSON message delimiter
	         	packedBuffer.data().put((byte)',');
	
	         	// set new mark for the next message
	         	packedBuffer._packedMsgOffSetPosition = packedBuffer.data().position();
	        }
	        else
	        {
	            // don't reserve space for the next JSON message delimiter, we're done packing.
	        	
	        	if(packedBuffer._packedMsgOffSetPosition == packedBuffer.data().position())
	        	{ 
	        		/* Users doesn't write a JSON message so replace the delimiter with closing array. */
	        		packedBuffer.data().put(packedBuffer._packedMsgOffSetPosition - 1, (byte)']');	
	        	}
	        	else
	        	{
	        		packedBuffer.data().put((byte)']');
	        		packedBuffer._packedMsgOffSetPosition = packedBuffer.data().position();
	        	}

				packedBuffer._dataStartOffSet = 0; /* Remove the start data offset to start data at '[' */
	        	
	            return 0;
	        }
			
	        return packedBuffer.data().limit() - packedBuffer.data().position() - 1;
		}
	}

	@Override
	public int writeCompressed(TransportBufferImpl buffer, WriteArgs writeArgs, Error error)
	{
		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE)
		{
			return _rsslSocketChannel.writeNormalCompressed(buffer, writeArgs, error);
		}
		else
		{
			int retVal = 0;
			int bytesForBuffer = 0;
			int totalBytes = 0;
			int msgLen = buffer.packedLen();
			int hdrLen = estimateHeaderLength();
			final int MAX_BYTES_FOR_BUFFER = _rsslSocketChannel._internalMaxFragmentSize - hdrLen;
	        
			// An extra buffer might be needed: get it now before compression
			TransportBufferImpl compFragmentBuffer = _rsslSocketChannel.getBufferInternal(MAX_BYTES_FOR_BUFFER, false, hdrLen);
			if (compFragmentBuffer == null)
			{
				retVal = _rsslSocketChannel.flushInternal(error);
				if (retVal < TransportReturnCodes.SUCCESS)
					return retVal;
				compFragmentBuffer = _rsslSocketChannel.getBufferInternal(MAX_BYTES_FOR_BUFFER, false, hdrLen);
				if (compFragmentBuffer == null)
				{
					error.channel(_rsslSocketChannel);
					error.errorId(TransportReturnCodes.NO_BUFFERS);
					error.sysError(0);
					error.text("channel out of buffers");
					return TransportReturnCodes.NO_BUFFERS;
				}
			}

			compFragmentBuffer.headerLength(hdrLen);
			
			int compressedBytesLen = _rsslSocketChannel._compressor.compress(buffer, buffer.dataStartPosition(), msgLen);
			
			// Strip off the trailing 0x00 0x00 0xFF 0xFF as per PMCE spec
			compressedBytesLen -= 4;
			
			byte[] compressedBytes = _rsslSocketChannel._compressor.compressedData();
			boolean finBit = true;
			int compressedBytesLenNext = 0;
			byte[] compressedBytesNext = null;
			
			if (compressedBytesLen > MAX_BYTES_FOR_BUFFER)
			{
				compressedBytesLen = _rsslSocketChannel._compressor.compress(buffer, buffer.dataStartPosition(), msgLen - 13);
				
				// Strip off the trailing 0x00 0x00 0xFF 0xFF as per PMCE spec
				compressedBytesLen -= 4;
				
				compressedBytes = _rsslSocketChannel._compressor.compressedData().clone();
			    bytesForBuffer = compressedBytesLen;
			    finBit = false;
			    
			    compressedBytesLenNext = _rsslSocketChannel._compressor.compress(buffer, buffer.dataStartPosition() + (msgLen - 13), 13);
				
				// Strip off the trailing 0x00 0x00 0xFF 0xFF as per PMCE spec
			    compressedBytesLenNext -= 4;
				
			    compressedBytesNext = _rsslSocketChannel._compressor.compressedData();
			}
			else
			{
			    bytesForBuffer = compressedBytesLen;
			}

			// Transfer compressed bytes to the transport buffer
			buffer.data().position(buffer.dataStartPosition());
			buffer.data().limit(buffer.dataStartPosition() + bytesForBuffer);
			buffer.data().put(compressedBytes, 0, bytesForBuffer);
			// Do this last (before write), so that internal buffer length and position set
			
			int wsHdrlen = WebSocketFrameParser.calculateHeaderLength(bytesForBuffer,
					_webSocketSession.isClient);

			buffer._startWsHeader = buffer._startPosition + (WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHdrlen);

			/* Set the WebSocket frame header */
			WebSocketFrameParser.encode(buffer._data, buffer._startWsHeader, bytesForBuffer, _rsslSocketChannel.protocolType(),
					_webSocketSession.isClient, finBit, _webSocketSession.isDeflate(), WebSocketFrameParser._WS_OPC_NONE);
			
			int lastPosition = buffer._data.position();
			buffer._length = lastPosition - buffer._startWsHeader;
			buffer._data.position(buffer._startWsHeader);
			buffer._data.limit(lastPosition);
			
			/* Set the start position according to the actual data */
			buffer._startPosition = buffer._startWsHeader;

			int additionalHdrLen = wsHdrlen;

			if (_rsslSocketChannel._totalBytesQueued > 0)
			{
			    retVal = _rsslSocketChannel.writeWithBuffersQueued(buffer, writeArgs, error);
			}
			else
			{
			    retVal = _rsslSocketChannel.writeWithNoBuffersQueued(buffer, writeArgs, error);
			}

			// First part stats: User data bytes + overhead (ignore compression)
			((WriteArgsImpl)writeArgs).uncompressedBytesWritten(msgLen + additionalHdrLen);
			totalBytes = buffer._length;
			
			// Send extra message if there are bytes that did not fit in the first part
			if (retVal >= TransportReturnCodes.SUCCESS && (finBit == false))
			{
				bytesForBuffer = compressedBytesLenNext;
			
			    // Populate second message
			    compFragmentBuffer.data().position(compFragmentBuffer.dataStartPosition());
			    compFragmentBuffer.data().limit(compFragmentBuffer.dataStartPosition() + bytesForBuffer);
			    compFragmentBuffer.data().put(compressedBytesNext, 0, bytesForBuffer);
			
			    wsHdrlen = WebSocketFrameParser.calculateHeaderLength(bytesForBuffer,
						_webSocketSession.isClient);
			
			    compFragmentBuffer._startWsHeader = compFragmentBuffer._startPosition + (WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHdrlen);
			
				/* Set the WebSocket frame header */
				WebSocketFrameParser.encode(compFragmentBuffer._data, compFragmentBuffer._startWsHeader, bytesForBuffer, _rsslSocketChannel.protocolType(),
						_webSocketSession.isClient, true, _webSocketSession.isDeflate(), WebSocketFrameParser._WS_OPC_CONT);
				
				lastPosition = compFragmentBuffer._data.position();
				compFragmentBuffer._length = lastPosition - compFragmentBuffer._startWsHeader;
				compFragmentBuffer._data.position(compFragmentBuffer._startWsHeader);
				compFragmentBuffer._data.limit(lastPosition);
				
				/* Set the start position according to the actual data */
				compFragmentBuffer._startPosition = compFragmentBuffer._startWsHeader;
				
				additionalHdrLen = wsHdrlen;
			
			    ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(writeArgs.uncompressedBytesWritten() + additionalHdrLen);
			    totalBytes += compFragmentBuffer._length;
			
			    // Write second part and flush
			    if (_rsslSocketChannel._totalBytesQueued > 0)
			    {
			        retVal = _rsslSocketChannel.writeWithBuffersQueued(compFragmentBuffer, writeArgs, error);
			    }
			    else
			    {
			        retVal = _rsslSocketChannel.writeWithNoBuffersQueued(compFragmentBuffer, writeArgs, error);
			    }
			}
			else
			{
			    compFragmentBuffer.returnToPool();
			}

			// Total bytes on wire, for one or two messages sent
			((WriteArgsImpl)writeArgs).bytesWritten(totalBytes);

			return retVal;
		}
	}

	@Override
	public boolean compressedData(RsslSocketChannel rsslSocketChannel, int messageLength, WriteArgs writeArgs)
	{
		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE)
		{
			return ProtocolFunctions.super.compressedData(rsslSocketChannel, messageLength, writeArgs);
		}
		else if (_rsslSocketChannel._sessionOutCompression == Ripc.CompressionTypes.ZLIB)
        {
           return true;
        }

        return false;
	}

	@Override
	public int populateFragment(BigBuffer bigBuffer, boolean firstFragment, TransportBufferImpl writeBuffer, int flags, WriteArgs writeArgs)
	{
		int bytesCopied = 0;
		int position = bigBuffer._data.position();
		int limit = bigBuffer._data.limit();
		int uncompressedBytesWritten = writeArgs.uncompressedBytesWritten();
		int bytesWritten = writeArgs.bytesWritten();
		int wsHdrlen = 0;
		int messageLength = 0;
		
		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE)
		{
			int ripcMsgLength = writeBuffer._data.capacity() - WebSocketFrameParser._WS_MAX_HEADER_LEN;
			
			if(firstFragment)
			{
				limit = position;
				writeBuffer._data.limit(writeBuffer._data.capacity());
	        	bigBuffer._data.position(0);
	        	
	        	writeBuffer._data.position(WebSocketFrameParser._WS_MAX_HEADER_LEN);
	        	
	        	writeBuffer.populateRipcHeader(bigBuffer, true, flags, TransportBufferImpl.FRAGMENT_HEADER_RIPC_FLAGS, ripcMsgLength, limit);
	        	
	        	wsHdrlen = WebSocketFrameParser.calculateHeaderLength(ripcMsgLength, _webSocketSession.isClient);
	        	
	        	/* Calculate the number of bytes to copy for payload */
	        	bytesCopied = ripcMsgLength - TransportBufferImpl._firstFragmentHeaderLength;
	        	
	        	messageLength = ripcMsgLength + wsHdrlen;
	        	
	        	bigBuffer._data.limit(bytesCopied);
	        	
	        	 // copy the data from bigBuffer to this buffer
		        writeBuffer._data.put(bigBuffer._data);
		        
		        writeBuffer._startWsHeader = writeBuffer._startPosition + (WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHdrlen);
				
				/* Set the WebSocket frame header */
				WebSocketFrameParser.encode(writeBuffer._data, writeBuffer._startWsHeader, ripcMsgLength, _rsslSocketChannel.protocolType(),
						_webSocketSession.isClient, true, false, WebSocketFrameParser._WS_OPC_NONE);
				
				int lastPosition = writeBuffer._data.position();
				writeBuffer._length = lastPosition - writeBuffer._startWsHeader;
				
				writeBuffer._data.position(writeBuffer._startWsHeader);
				writeBuffer._data.limit(lastPosition);
				
				/* Set the start position according to the actual data */
				writeBuffer._startPosition = writeBuffer._startWsHeader;
			}
			else
			{
				int bytesToCopy = limit - position;
	            if (ripcMsgLength <= (bytesToCopy + TransportBufferImpl._nextFragmentHeaderLength))
	            {
	                bytesCopied = ripcMsgLength - TransportBufferImpl._nextFragmentHeaderLength;
	            }
	            else
	            {
	                bytesCopied = bytesToCopy;
	            }
	            
	            /* Updates the actual RIPC message length */
	            ripcMsgLength = bytesCopied + TransportBufferImpl._nextFragmentHeaderLength;
	         
	            writeBuffer._data.position(WebSocketFrameParser._WS_MAX_HEADER_LEN);
	            writeBuffer.populateRipcHeader(bigBuffer, false, flags, TransportBufferImpl.FRAGMENT_RIPC_FLAGS, ripcMsgLength, 0);

	            wsHdrlen = WebSocketFrameParser.calculateHeaderLength(ripcMsgLength, _webSocketSession.isClient);
                
                messageLength = ripcMsgLength + wsHdrlen;
                
                writeBuffer._data.limit(WebSocketFrameParser._WS_MAX_HEADER_LEN + ripcMsgLength);
             
                // copy the data from bigBuffer to this buffer if available
                bigBuffer._data.limit(position + bytesCopied);
                writeBuffer._data.put(bigBuffer._data);
    	        
    	        writeBuffer._startWsHeader = writeBuffer._startPosition + (WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHdrlen);
				
				/* Set the WebSocket frame header */
				WebSocketFrameParser.encode(writeBuffer._data, writeBuffer._startWsHeader, ripcMsgLength, _rsslSocketChannel.protocolType(),
						_webSocketSession.isClient, true, false, WebSocketFrameParser._WS_OPC_NONE);
				
				int lastPosition = writeBuffer._data.position();
				writeBuffer._length = lastPosition - writeBuffer._startWsHeader;
				
				writeBuffer._data.position(writeBuffer._startWsHeader);
				writeBuffer._data.limit(lastPosition);
				
				/* Set the start position according to the actual data */
				writeBuffer._startPosition = writeBuffer._startWsHeader;
			}
		}
		else
		{	        
	        if(firstFragment)
	        {
	        	limit = position;	
	        	writeBuffer._data.limit(writeBuffer._data.capacity());
	        	bigBuffer._data.position(0);
	        	
	        	/* Calculate the number of bytes to copy for payload */
	        	bytesCopied = writeBuffer._data.capacity() - WebSocketFrameParser._WS_MAX_HEADER_LEN;
	        	wsHdrlen = WebSocketFrameParser.calculateHeaderLength(bytesCopied, _webSocketSession.isClient);
	        	
	        	messageLength = bytesCopied + wsHdrlen;
	        	
	        	writeBuffer._data.position(WebSocketFrameParser._WS_MAX_HEADER_LEN);
	        	
	        	bigBuffer._data.limit(bytesCopied);
	        	
	        	 // copy the data from bigBuffer to this buffer
		        writeBuffer._data.put(bigBuffer._data);
		        
		        writeBuffer._startWsHeader = writeBuffer._startPosition + (WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHdrlen);
				
				/* Set the WebSocket frame header */
				WebSocketFrameParser.encode(writeBuffer._data, writeBuffer._startWsHeader, bytesCopied, _rsslSocketChannel.protocolType(),
						_webSocketSession.isClient, false, false, WebSocketFrameParser._WS_OPC_NONE);
				
				int lastPosition = writeBuffer._data.position();
				writeBuffer._length = lastPosition - writeBuffer._startWsHeader;
				
				writeBuffer._data.position(writeBuffer._startWsHeader);
				writeBuffer._data.limit(lastPosition);
				
				/* Set the start position according to the actual data */
				writeBuffer._startPosition = writeBuffer._startWsHeader;
	        }
	        else
	        {
	        	writeBuffer._data.position(WebSocketFrameParser._WS_MAX_HEADER_LEN);
	            int bytesToCopy = limit - position;
	            boolean finBit = false;
	            if (writeBuffer._data.capacity() < (bytesToCopy + WebSocketFrameParser._WS_MAX_HEADER_LEN))
	            {
	                bytesCopied = writeBuffer._data.capacity() - WebSocketFrameParser._WS_MAX_HEADER_LEN;
	            }
	            else if (writeBuffer._data.capacity() == (bytesToCopy + WebSocketFrameParser._WS_MAX_HEADER_LEN))
	            {
	            	bytesCopied = writeBuffer._data.capacity() - WebSocketFrameParser._WS_MAX_HEADER_LEN;
	            	finBit = true;
	            }
	            else
	            {
	                bytesCopied = bytesToCopy;
	                finBit = true;
	            }
                
                wsHdrlen = WebSocketFrameParser.calculateHeaderLength(bytesCopied, _webSocketSession.isClient);
                
                messageLength = bytesCopied + wsHdrlen; /* WS header + data length */
                
                writeBuffer._data.limit(WebSocketFrameParser._WS_MAX_HEADER_LEN + bytesCopied);
                
                bigBuffer._data.limit(position + bytesCopied);
                
                // copy the data from bigBuffer to this buffer
    	        writeBuffer._data.put(bigBuffer._data);
    	        
    	        writeBuffer._startWsHeader = writeBuffer._startPosition + (WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHdrlen);
				
				/* Set the WebSocket frame header */
				WebSocketFrameParser.encode(writeBuffer._data, writeBuffer._startWsHeader, bytesCopied, _rsslSocketChannel.protocolType(),
						_webSocketSession.isClient, finBit, false, WebSocketFrameParser._WS_OPC_CONT);
				
				int lastPosition = writeBuffer._data.position();
				writeBuffer._length = lastPosition - writeBuffer._startWsHeader;
				
				writeBuffer._data.position(writeBuffer._startWsHeader);
				writeBuffer._data.limit(lastPosition);
				
				/* Set the start position according to the actual data */
				writeBuffer._startPosition = writeBuffer._startWsHeader;
	        }
		}

		// update uncompressed bytes written
		((WriteArgsImpl)writeArgs).uncompressedBytesWritten(uncompressedBytesWritten + messageLength);
 
		bigBuffer._data.limit(limit);
 
		// update bytes written
		((WriteArgsImpl)writeArgs).bytesWritten(bytesWritten + messageLength);

		return bytesCopied;
	}

	@Override
	public int writeFragmentCompressed(BigBuffer bigBuffer, TransportBufferImpl fragment, WriteArgs writeArgs, boolean firstFragment, Error error)
	{
        int userBytesForFragment = 0;
        int position = bigBuffer._data.position();
        int limit = bigBuffer._data.limit();
        int flags = Ripc.Flags.HAS_OPTIONAL_FLAGS | Ripc.Flags.COMPRESSION;
        int optFlags = 0;
        int headerLength = 0;
        int compressedLen = 0;
        int maxPayloadSize = 0;
        byte[] compressedBytes;
        int extraBytes = 0;
        int extraTotalLength = 0;
        int extraHeaderLength = 0;
        int totalLength = 0;
        int retVal = TransportReturnCodes.SUCCESS;
        int MAX_BYTES_FOR_BUFFER = _rsslSocketChannel._internalMaxFragmentSize - estimateHeaderLength();
        TransportBufferImpl compFragmentBuffer = null;
        int wsHeaderLength = 0;
		
		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE)
		{
			// An extra buffer might be needed: get it now before compression to ensure it is available if needed
	        compFragmentBuffer = _rsslSocketChannel.getBufferInternal(MAX_BYTES_FOR_BUFFER, false, estimateHeaderLength());
	        if (compFragmentBuffer == null)
	        {
	            retVal = _rsslSocketChannel.flushInternal(error);
	            if (retVal < TransportReturnCodes.SUCCESS)
	                return retVal;
	            compFragmentBuffer = _rsslSocketChannel.getBufferInternal(MAX_BYTES_FOR_BUFFER, false, estimateHeaderLength());
	            if (compFragmentBuffer == null)
	            {
	                error.channel(_rsslSocketChannel);
	                error.errorId(TransportReturnCodes.NO_BUFFERS);
	                error.sysError(0);
	                error.text("channel out of buffers");
	                return TransportReturnCodes.NO_BUFFERS;
	            }
	        }
			
	        // Determine how many bytes can fit in the fragment,
	        // depending on if this is the first/next fragment, and how many bytes remain in the big buffer.
	        if (firstFragment)
	        {
	            // First time: position is the end of data in the big buffer
	            // -- make this the new limit
	            limit = position;
	            totalLength = position;
	            optFlags = Ripc.Flags.Optional.FRAGMENT_HEADER;
	            headerLength = TransportBufferImpl._firstFragmentHeaderLength + WebSocketFrameParser._WS_MAX_HEADER_LEN;
	            maxPayloadSize = fragment.data().capacity() - headerLength;

	            bigBuffer._data.position(0); // start at the beginning
	            userBytesForFragment = fragment.data().capacity() - headerLength;
	            bigBuffer._data.limit(userBytesForFragment);
	        }
	        else
	        {
	            optFlags = Ripc.Flags.Optional.FRAGMENT;
	            headerLength = TransportBufferImpl._nextFragmentHeaderLength + WebSocketFrameParser._WS_MAX_HEADER_LEN;
	            maxPayloadSize = fragment.data().capacity() - headerLength;

	            int bytesRemaining = limit - position; // bytes remaining in big buffer
	            if (fragment.data().capacity() <= (bytesRemaining + headerLength))
	            {
	                userBytesForFragment = fragment.data().capacity() - headerLength;
	            }
	            else
	            {
	                userBytesForFragment = bytesRemaining;
	            }
	        }

	        // Compress the selected number of bytes (userBytesForFragment) for the fragment
	        compressedLen = _rsslSocketChannel._compressor.compress(bigBuffer.data(),
	                                             bigBuffer.data().position(), // big buffer position points at data to be sent
	                                             userBytesForFragment); // number of bytes to compress

	        compressedBytes = _rsslSocketChannel._compressor.compressedData();
	        
	        int msgLength = 0;
	        int inDataLength = 0;
	        
	        if (compressedLen > maxPayloadSize)
	        {
	        	inDataLength = maxPayloadSize;
	        	
	        	// There is going to be an extra message after this, so set the COMP_FRAGMENT flag
	            flags |= Ripc.Flags.COMP_FRAGMENT;
	            
	            extraBytes = compressedLen - maxPayloadSize;
	        }
	        else
	        {
	        	inDataLength = compressedLen;
	        }
	     	                	
            if (firstFragment)
            {
            	msgLength = TransportBufferImpl._firstFragmentHeaderLength + inDataLength;
            	
            	wsHeaderLength = WebSocketFrameParser.calculateHeaderLength(msgLength, _webSocketSession.isClient);
            	
            	fragment._startWsHeader = fragment._startPosition + (WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHeaderLength);
            	
                fragment._data.position(fragment._startWsHeader + wsHeaderLength);
                fragment.populateRipcHeader(bigBuffer, firstFragment, flags, optFlags, msgLength, totalLength);
            }
            else
            {
            	msgLength = TransportBufferImpl._nextFragmentHeaderLength + inDataLength;
            	
            	wsHeaderLength = WebSocketFrameParser.calculateHeaderLength(msgLength, _webSocketSession.isClient);
            	
            	fragment._startWsHeader = fragment._startPosition + (WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHeaderLength);
            	
            	fragment._data.position(fragment._startWsHeader + wsHeaderLength);
                fragment.populateRipcHeader(bigBuffer, firstFragment, flags, optFlags, msgLength, 0);
            }
            
            fragment._data.put(compressedBytes, 0, inDataLength);
            
            /* Set the WebSocket frame header */
			WebSocketFrameParser.encode(fragment._data, fragment._startWsHeader, msgLength, _rsslSocketChannel.protocolType(),
					_webSocketSession.isClient, true, false, WebSocketFrameParser._WS_OPC_NONE);
            
            int lastPosition = fragment._data.position();
            fragment._length = lastPosition - fragment._startWsHeader;
            fragment._data.position(fragment._startWsHeader);
            fragment._data.limit(lastPosition);
            
            /* Set the start position according to the actual data */
            fragment._startPosition = fragment._startWsHeader;

	        // add to the priority queues
	        _rsslSocketChannel.writeFragment(fragment, writeArgs);

	        // If there are extra bytes that could not fit in the fragment, write the remainder of the compressed bytes into an extra message.
	        // Extra bytes start at position userBytesForFragment (after data sent in previous message)
	        if (extraBytes > 0)
	        {
	            // Populate second message
	        	msgLength = TransportBufferImpl.RIPC_WRITE_POSITION + extraBytes;
	        	wsHeaderLength = WebSocketFrameParser.calculateHeaderLength(msgLength, _webSocketSession.isClient);
	        
	        	compFragmentBuffer._startWsHeader = compFragmentBuffer._startPosition + (WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHeaderLength);
	        	
	        	compFragmentBuffer._data.position(compFragmentBuffer._startWsHeader + wsHeaderLength);
	        	compFragmentBuffer._data.putShort((short)msgLength); // RIPC msg length
	        	compFragmentBuffer._data.put((byte)Ripc.Flags.COMPRESSION);   // RIPC flag indicating data
	        	
	        	/* Sets position for putting compressed data */
	            compFragmentBuffer.data().limit(compFragmentBuffer._data.position() + extraBytes);
	            compFragmentBuffer.data().put(compressedBytes, userBytesForFragment, extraBytes);
	            
	            /* Set the WebSocket frame header */
				WebSocketFrameParser.encode(compFragmentBuffer._data, compFragmentBuffer._startWsHeader, msgLength, _rsslSocketChannel.protocolType(),
						_webSocketSession.isClient, true, false, WebSocketFrameParser._WS_OPC_NONE);
	            
	            lastPosition = compFragmentBuffer._data.position();
	            compFragmentBuffer._length = lastPosition - compFragmentBuffer._startWsHeader;	
	            compFragmentBuffer._data.position(compFragmentBuffer._startWsHeader);
	            compFragmentBuffer._data.limit(lastPosition);
	            
	            /* Set the start position according to the actual data */
	            compFragmentBuffer._startPosition = compFragmentBuffer._startWsHeader;

	            _rsslSocketChannel.writeFragment(compFragmentBuffer, writeArgs);

	            extraTotalLength = Ripc.Lengths.HEADER + wsHeaderLength + extraBytes; // actual length on wire
	            extraHeaderLength = Ripc.Lengths.HEADER + wsHeaderLength; // overhead (header) from sending extra part
	        }
	        else
	        {
	            compFragmentBuffer.returnToPool();
	        }
		}
		else
		{
			int remainingSpace = 0;
			boolean finBit = false;
			int opCode = WebSocketFrameParser._WS_OPC_CONT;
			maxPayloadSize = fragment.data().capacity();
			
			if(firstFragment)
			{
				// First time: position is the end of data in the big buffer
	            // -- make this the new limit
	        	limit = position;
	            totalLength = position;
	            bigBuffer._data.position(0); // start at the beginning
	            userBytesForFragment = maxPayloadSize - estimateHeaderLength();
	            wsHeaderLength = WebSocketFrameParser.calculateHeaderLength(userBytesForFragment, _webSocketSession.isClient);
	            remainingSpace = WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHeaderLength;
	            
	            bigBuffer._data.limit(userBytesForFragment);
	            opCode = WebSocketFrameParser._WS_OPC_NONE;
			}
			else
			{
                 headerLength = estimateHeaderLength();
                 userBytesForFragment = maxPayloadSize - headerLength;

                 int bytesRemaining = limit - position; // bytes remaining in big buffer
                 if (maxPayloadSize <= (bytesRemaining + headerLength))
                 {
                        userBytesForFragment = maxPayloadSize - headerLength;
                 }
                 else
                {
                        userBytesForFragment = bytesRemaining;
                        finBit = true;
                }
                 
                wsHeaderLength = WebSocketFrameParser.calculateHeaderLength(userBytesForFragment, _webSocketSession.isClient);
 	            remainingSpace = WebSocketFrameParser._WS_MAX_HEADER_LEN - wsHeaderLength;
            }

            // Compress the selected number of bytes (userBytesForFragment) for the fragment
            compressedLen = _rsslSocketChannel._compressor.compress(bigBuffer.data(),
                                                 bigBuffer.data().position(), // big buffer position points at data to be sent
                                                 userBytesForFragment); // number of bytes to compress
            
            // Strip off the trailing 0x00 0x00 0xFF 0xFF as per PMCE spec 
            compressedLen -= 4;
            
            int availableSpace = userBytesForFragment + remainingSpace;
            if(compressedLen > availableSpace )
            {
            	/* Reduced the user data if the required compressed length is larger than the available space */
            	userBytesForFragment -= (compressedLen - availableSpace);
            	bigBuffer._data.limit(userBytesForFragment);
            	
            	compressedLen = _rsslSocketChannel._compressor.compress(bigBuffer.data(),
                        bigBuffer.data().position(), // big buffer position points at data to be sent
                        userBytesForFragment); // number of bytes to compress
            	
            	// Strip off the trailing 0x00 0x00 0xFF 0xFF as per PMCE spec 
                compressedLen -= 4;
            }
            
            compressedBytes = _rsslSocketChannel._compressor.compressedData();
            
            wsHeaderLength = WebSocketFrameParser.calculateHeaderLength(compressedLen, _webSocketSession.isClient);
            
            fragment._startWsHeader = fragment._startPosition + (maxPayloadSize - (wsHeaderLength + compressedLen));
            
            fragment._data.position(fragment._startWsHeader + wsHeaderLength);
            
            fragment._data.put(compressedBytes, 0, compressedLen);
			
			/* Set the WebSocket frame header */
			WebSocketFrameParser.encode(fragment._data, fragment._startWsHeader, compressedLen, _rsslSocketChannel.protocolType(),
					_webSocketSession.isClient, finBit, true, opCode);
            
            int lastPosition = fragment._data.position();
            fragment._length = lastPosition - fragment._startWsHeader;
        				
            fragment._data.position(fragment._startWsHeader);
            fragment._data.limit(lastPosition);
            
            /* Set the start position according to the actual data */
            fragment._startPosition = fragment._startWsHeader;
		
            // add to the priority queues
            _rsslSocketChannel.writeFragment(fragment, writeArgs);
            
            headerLength = wsHeaderLength;
		}
        
        // Actual bytes on wire is total length of first fragment, plus total length on wire of extra bytes (if sent)
        ((WriteArgsImpl)writeArgs).bytesWritten(writeArgs.bytesWritten() + fragment._length + extraTotalLength);
        
        // Uncompressed bytes is the number of bytes taken from the big buffer before
        // compression, plus overhead for the one message sent on wire
        ((WriteArgsImpl)writeArgs).uncompressedBytesWritten(writeArgs.uncompressedBytesWritten() +
                                                            userBytesForFragment + headerLength + extraHeaderLength);

        // Adjust big buffer for next call
        // -- set the limit to end of big buffer user data
        bigBuffer.data().limit(limit);
        // -- new position will be set just after the data inserted in this
        // fragment
        bigBuffer.data().position(bigBuffer.data().position() + userBytesForFragment);
	
		// Tell the caller how many payload bytes were put in this fragment (uncompressed)
        return userBytesForFragment;
	}

	@Override
	public TransportBufferImpl getBigBuffer(int size)
	{
        BigBuffer buffer = null;
        SocketBuffer sBuffer = null;
        // get a transport buffer that will be first buffer where the big buffer will be copied to
        sBuffer = _rsslSocketChannel.getSocketBuffer();
        if (sBuffer != null)
        {
            sBuffer._dataBuffer.position(0);
            TransportBufferImpl tBuffer = sBuffer.getBufferSliceForFragment(_rsslSocketChannel._internalMaxFragmentSize);

            // get big buffer from the pool, it returns non null
            buffer = (BigBuffer)_rsslSocketChannel._bigBuffersPool.poll(size, true);
            buffer._data.position(0);
            
            if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE)
    		{
    			BigBuffer.ripcVersion(_rsslSocketChannel._ipcProtocol.ripcVersion());
    			buffer._length = size;
    		}
            else
            {
            	buffer._data.put((byte)'[');
            	buffer._startPosition = buffer._data.position();
            	buffer._length = size - 2;
            }
            
            buffer.id();
            buffer._firstBuffer = tBuffer;
            buffer._isOwnedByApp = true;
        }
        return buffer;
	}

	@Override
	public int writeFragmentSuffix(BigBuffer bigBuffer)
	{
		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE)
		{
			return 0; /* Do nothing */
		}
		else
		{
			bigBuffer._data.put((byte)']');
			return 1;
		}
	}

	@Override
	public boolean checkCompressionFragmentedMsg(int messageSize)
	{
		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE)
		{
			return (messageSize < _rsslSocketChannel._sessionCompLowThreshold);
		}
		else
		{
			return false;
		}

	}

	@Override
	public boolean isPingMessage() {
		
		if(_rsslSocketChannel.protocolType() == Codec.JSON_PROTOCOL_TYPE)
		{
			return (_webSocketSession.wsFrameHdr.opcode == WebSocketFrameParser._WS_OPC_PING ||
				_webSocketSession.wsFrameHdr.opcode == WebSocketFrameParser._WS_OPC_PONG);
		}
		else
		{
			return (_readBufferStateMachine._currentMsgRipcLen == RsslSocketChannel.RIPC_HDR_SIZE);
		}
	}

	public int ping(Error error) throws IOException {
        pingBuffer.rewind();
     	return _rsslSocketChannel._scktChannel.write(pingBuffer);
    }

	@Override
	public int pong(Error error) throws IOException {
		pongBuffer.rewind();
		return _rsslSocketChannel._scktChannel.write(pongBuffer);
	}

	public int sendWsFramePing(String pingMsg, Error error) {
		return prepareWsFramePingPong(pingMsg, WebSocketFrameParser._WS_OPC_PING, error);
    }

    public int sendWsFramePong(String pongMsg, Error error) {
		return prepareWsFramePingPong(pongMsg, WebSocketFrameParser._WS_OPC_PONG, error);
    }

    private int prepareWsFramePingPong(String msg, int opCode, Error error) {
    	
    	/* Checks whether the websocket connection is not initiated successfully */
    	if(Objects.isNull(wsFrameBuffer))
    	{
    		return TransportReturnCodes.SUCCESS;
    	}
    	
		wsFrameBuffer.clear();
		String payloadMsg = Optional.ofNullable(msg).orElse("");
		int payloadLength = payloadMsg.length();
		if (payloadLength > WebSocketFrameParser._WS_MAX_FRAME_LENGTH) {
			payloadMsg = msg.substring(0, WebSocketFrameParser._WS_MAX_FRAME_LENGTH);
			payloadLength = WebSocketFrameParser._WS_MAX_FRAME_LENGTH;
		}

		if (payloadLength > 0) {
			wsFrameBuffer.position(wsFrameHeaderLength);
			wsFrameBuffer.put(payloadMsg.getBytes());
			wsFrameBuffer.rewind();
			WebSocketFrameParser.encode(wsFrameBuffer, 0, payloadLength, _webSocketSession.getAcceptedProtocol(),
					_webSocketSession.isClient, true, false, opCode);
			
			wsFrameBuffer.limit(wsFrameHeaderLength + payloadLength);
		}
		else
		{
			WebSocketFrameParser.encode(wsFrameBuffer, 0, payloadLength, _webSocketSession.getAcceptedProtocol(),
					_webSocketSession.isClient, true, false, opCode);
			
			wsFrameBuffer.limit(wsFrameHeaderLength);
		}
		
		return sendWsFrame(opCode, error);
	}

    public int sendWsFrameClose(WebSocketCloseStatus closeStatus, Error error) {
    	
    	/* Checks whether the websocket connection is not initiated successfully */
    	if(Objects.isNull(wsFrameBuffer))
    	{
    		return TransportReturnCodes.SUCCESS;
    	}
    	
		_webSocketSession.sendClose = true;
		wsFrameBuffer.clear();
		final int dataLength = closeStatus.getTextMessageBytes().length + WebSocketFrameParser._WS_CONTROL_HEADER_LEN;
		wsFrameBuffer.position(wsFrameHeaderLength);
		wsFrameBuffer.putShort((short) closeStatus.getCode());
		wsFrameBuffer.put(closeStatus.getTextMessageBytes());
		wsFrameBuffer.flip();
		WebSocketFrameParser.encode(wsFrameBuffer, 0, dataLength, _webSocketSession.getAcceptedProtocol(),
				_webSocketSession.isClient, true, false, WebSocketFrameParser._WS_OPC_CLOSE);
		return sendWsFrame(WebSocketFrameParser._WS_OPC_CLOSE, error);

	}

	private int sendWsFrame(int opCode, Error error) {
		try {
			wsFrameBuffer.rewind();
			return _rsslSocketChannel._scktChannel.write(wsFrameBuffer);
		} catch (IOException e) {
			return setError(error, TransportReturnCodes.FAILURE,
					String.format("Op Code: %d - <sendWsFrame> failed: %s\n", opCode, e.getMessage()));
		}
	}

	@Override
	public int closeChannel(Error error) {
		
		sendWsFrameClose(WebSocketCloseStatus.WS_CFSC_NORMAL_CLOSE, error);

		return _rsslSocketChannel.closeSocketChannel(error);
	}

	@Override
	public boolean isRWFProtocol() {
		return Objects.equals(Codec.RWF_PROTOCOL_TYPE, _rsslSocketChannel.protocolType())
				&& !_webSocketSession.wsFrameHdr.control;
	}

	private int setError(Error error, int reason, String errorMessage, Object... formatArgs) {
        error.channel(_rsslSocketChannel);
        error.errorId(TransportReturnCodes.FAILURE);
        error.text(String.format(errorMessage, formatArgs));
        return reason;
    }

	@Override
	public boolean writeAsFragmentedMessage(TransportBufferImpl buffer)
	{
		if(_rsslSocketChannel.protocolType() == Codec.RWF_PROTOCOL_TYPE)
		{
			return ((buffer._data.position() + estimateHeaderLength()) > _rsslSocketChannel._internalMaxFragmentSize);
		}
		else
		{
			/* Plus 1 to include ']' at the end of buffer */
			return ((buffer._data.position() + estimateHeaderLength() + 1) > _rsslSocketChannel._internalMaxFragmentSize);
		}
	}
}
