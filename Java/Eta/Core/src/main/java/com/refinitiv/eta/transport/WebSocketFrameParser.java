/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import java.nio.ByteBuffer;
import java.util.SplittableRandom;
import java.util.Objects;

class WebSocketFrameParser {
	
	static final int _WS_BIT_POS_FIN = 7;
	static final int _WS_BIT_POS_RSV1 = 6;
	static final int _WS_BIT_POS_RSV2 = 5;
	static final int _WS_BIT_POS_RSV3 = 4;
	
	static final int _WS_OPCODE_MASK = 0x0F;
	static final int _WS_BIT_POS_MASKKEY = 7;
	static final int _WS_PAYLOAD_LEN_MASK = 0x7F;
	
	static final int _WS_CONTROL_HEADER_LEN = 2;
	static final int _WS_MASK_KEY_FIELD_LEN = 4;
	static final int _WS_FLAG_2BYTE_EXT_PAYLOAD = 126;
	static final int _WS_FLAG_8BYTE_EXT_PAYLOAD = 127;
	static final int _WS_2BYTE_EXT_PAYLOAD = 2;
	static final int _WS_8BYTE_EXT_PAYLOAD = 8;
	
	static final int _WS_PAYLOAD_LEN_OFFSET	= 1; /* the MSbit is the mask flag */
	static final int _WS_EXTENDED_PAYLOAD_OFFSET = 2; /* for 2 and 8 byte payload lengths */
	
	/* Operation codes */
	static final int _WS_OPC_NONE = -1;
	static final int _WS_OPC_CONT = 0;
	static final int _WS_OPC_TEXT = 1;
	static final int _WS_OPC_BINARY = 2;
	static final int _WS_OPC_CLOSE = 8;
	static final int _WS_OPC_PING = 9;
	static final int _WS_OPC_PONG = 10;
	
	static final int _WS_126PAYLOAD_FIELD_LEN = _WS_2BYTE_EXT_PAYLOAD;
	static final int _WS_127PAYLOAD_FIELD_LEN =	_WS_8BYTE_EXT_PAYLOAD;

	static final int _WS_MASK_KEY_126PAY_OFFSET	= (_WS_CONTROL_HEADER_LEN + _WS_126PAYLOAD_FIELD_LEN);
	static final int _WS_MASK_KEY_127PAY_OFFSET	= (_WS_CONTROL_HEADER_LEN + _WS_127PAYLOAD_FIELD_LEN);
	
	static final int _WS_MIN_HEADER_LEN = _WS_CONTROL_HEADER_LEN;
	static final int _WS_126_HEADER_LEN	= (_WS_MIN_HEADER_LEN + _WS_126PAYLOAD_FIELD_LEN); /* 2 + 2 */
	static final int _WS_127_HEADER_LEN	= (_WS_MIN_HEADER_LEN + _WS_127PAYLOAD_FIELD_LEN); /* 2 + 8 */

	static final int _WS_MAX_HEADER_LEN	= (_WS_127_HEADER_LEN + _WS_MASK_KEY_FIELD_LEN); /* 2 + 8 + 4 */

	/**
	 * According to RFC6455 the close frame payload must start with a 2 byte, unsigned status code
	 * followed by optional payload no more than 125 bytes in length
	 */
	static final int _WS_CLOSE_STATUS_LENGTH = 2;
	static final int _WS_MAX_FRAME_LENGTH = 125;
	
	static final int _WS_SP_NONE = -1;
	static final int _WS_SP_RWF = 0;
	static final int _WS_SP_JSON2 = 2;
	
	private static final SplittableRandom _randGen = new SplittableRandom();
	
	private static final Lock _randomLock = new ReentrantLock();
	
	private static int _lastMaskValue;
	
	/* This is used for unit testing only. */
	static int getLastMaskValue()
	{
		return _lastMaskValue;
	}
	
	static int getHeaderLength(ByteBuffer buffer, int index)
	{
		int headerLength = _WS_CONTROL_HEADER_LEN;
		
		headerLength += (((buffer.get(index + 1) >> _WS_BIT_POS_MASKKEY ) & 0x01) != 0) ? _WS_MASK_KEY_FIELD_LEN : 0;
		headerLength += ((buffer.get(index + 1) & _WS_PAYLOAD_LEN_MASK) == _WS_FLAG_2BYTE_EXT_PAYLOAD) ? _WS_2BYTE_EXT_PAYLOAD :
			((buffer.get(index + 1) & _WS_PAYLOAD_LEN_MASK) == _WS_FLAG_8BYTE_EXT_PAYLOAD) ? _WS_8BYTE_EXT_PAYLOAD : 0;

		return headerLength;
	}
	
	static int getBit(byte value, int bitn)
	{
		return( (value >> bitn) & 0x01 );
	}
	
	static void setBit(byte[] data, int index, int bitn)
	{
		data[index] = (data[index] |= (1 << bitn)); 
	
	}
	
	static boolean getBitAsBoolean(byte value, int bitn)
	{
		return ( getBit(value, bitn) != 0 );
	}
	
	static boolean isControlFrame(int code)
	{
		return (code == _WS_OPC_CLOSE || code == _WS_OPC_PING || code == _WS_OPC_PONG);
	}
	
	static void setMaskKey(byte[] mask, int mVal)
	{
		mask[0] = ((byte)((mVal >> 24) & 0xFF));
		mask[1] = ((byte)((mVal >> 16) & 0xFF));
		mask[2] = ((byte)((mVal >> 8) & 0xFF));
		mask[3] = ((byte)(mVal & 0xFF));
	}
	
	static int getMaskKey(byte[] mask, ByteBuffer buffer, int index)
	{
		int maskVal;
		
		maskVal = buffer.getInt(index);
		
		setMaskKey(mask, maskVal);
		
		return maskVal;
	}
	
	static int getRandomValue()
	{
		_randomLock.lock();
		
		try
		{
			return _randGen.nextInt();
		}
		finally
		{
			_randomLock.unlock();
		}
	}
	
	static void maskDataBlock(byte[] mask, byte[] data, int index, int length)
	{
		for(int i = 0; i < length; i++, index++)
		{
			data[index] = (byte) (data[index] ^ mask[(i%4)]);
		}
	}
	
	static void maskDataBlock(byte[] mask, ByteBuffer byteBuffer, int index, int length)
	{
		for(int i = 0; i < length; i++, index++)
		{
			byteBuffer.put(index, (byte)(byteBuffer.get(index) ^ mask[(i%4)]));
		}
	}
	
	static boolean decode(WebSocketFrameHdr frame, ByteBuffer buffer, int index, int length) 
	{
		if (!Objects.equals(frame.buffer, buffer)) {
			frame.reset();
		}

		/* Set the location of the WS control header if not set */
		if(Objects.isNull(frame.buffer) && length > 0) {
			frame.buffer = buffer;
			frame.ctrlHdrIndex = index;
		}
		
		/* If there is no 2 byte control header to read, return after
		 * flagging this as a partial header */
		
		if(length < 2)
		{
			frame.partial = true;
			return frame.partial;
		}
		
		/* Calculate the frame header length from the 2 byte control header portion */
		frame.hdrLen = getHeaderLength(frame.buffer, frame.ctrlHdrIndex);
		
		/* finSet bit for complete/fragmented frame */
		frame.finSet = getBitAsBoolean(frame.buffer.get(frame.ctrlHdrIndex), _WS_BIT_POS_FIN);
		
		/* RSV1 bit for any handshake negotiated extensions */
 		frame.rsv1Set = getBitAsBoolean(frame.buffer.get(frame.ctrlHdrIndex), _WS_BIT_POS_RSV1);
		
		/* Frame OpCode */
		frame.opcode = (frame.buffer.get(frame.ctrlHdrIndex) & _WS_OPCODE_MASK);

		/* Frame fragments should not be updated if a Control Frame is received in the
		 * middle of collecting/assembling frames */
		if ( !(frame.control = isControlFrame(frame.opcode)))
		{
			frame.fragment = (!frame.finSet ? true : false);
			
			if(frame.opcode != _WS_OPC_CONT)
			{
				/* Set the compressed flag for the payload for only fin or complete frames */
				/* The Compression bit (RSV1) is only set on the first frame and not set on the
				 * continuation frame if fragmented */
				frame.dataType = frame.opcode;
				frame.compressed = frame.rsv1Set;
			}
		}
		
		/* Flag to identify if payload is masked and key suffix to the frame header */
		frame.maskSet = getBitAsBoolean(frame.buffer.get(frame.ctrlHdrIndex + 1), _WS_BIT_POS_MASKKEY);
		
		/* PayloadLength or flag for 2 or 8 byte payload length fields */
		frame.payloadLen = (frame.buffer.get(frame.ctrlHdrIndex + 1) & _WS_PAYLOAD_LEN_MASK);
		
		if (frame.payloadLen == _WS_FLAG_2BYTE_EXT_PAYLOAD)
			frame.extHdrLen = _WS_2BYTE_EXT_PAYLOAD;
		else if (frame.payloadLen == _WS_FLAG_8BYTE_EXT_PAYLOAD)
			frame.extHdrLen = _WS_8BYTE_EXT_PAYLOAD;
		else
			frame.extHdrLen = 0;
		
		/* Get the values if the WS frame header has been read */
		if(length >= frame.hdrLen)
		{
			frame.extHdrIndex = frame.ctrlHdrIndex + _WS_CONTROL_HEADER_LEN;
			if(frame.extHdrLen != 0)
			{
				if(frame.extHdrLen == _WS_2BYTE_EXT_PAYLOAD)
				{
					frame.payloadLen = buffer.getShort(frame.extHdrIndex);
					
					if(frame.payloadLen < 0)
					{
						frame.payloadLen += 0x10000;
					}
					
					frame.extHdrIndex += 2;
				}
				else
				{
					frame.payloadLen = buffer.getLong(frame.extHdrIndex);
					frame.extHdrIndex += 8;
				}
			}
		
			/* Get the value of the mask key if it is set and not already set on a previous pass */
			if(frame.maskSet && frame.maskVal == 0)
				frame.maskVal = getMaskKey(frame.mask, frame.buffer, frame.extHdrIndex);
		
			/* Mark the beginning of the payload segment when enough bytes have been read */
			if (frame.payloadIndex == 0 && (frame.payloadLen != 0) && length >= (frame.hdrLen + 1))
				frame.payloadIndex = frame.ctrlHdrIndex + frame.hdrLen;
		
			frame.partial = (length < (frame.hdrLen + frame.payloadLen));
		}
		else
		{
			frame.partial = true;
			/* If this is a partial Frame Header, no reason to calculate the 
			 * extended payload length until the complete header has been read */
			if (frame.payloadLen == _WS_FLAG_2BYTE_EXT_PAYLOAD || 
				frame.payloadLen == _WS_FLAG_8BYTE_EXT_PAYLOAD)
				frame.payloadLen = 0;
		}
		
		return frame.partial;
	}
	
	static int calculateHeaderLength(long dataLength, boolean isClient)
	{
		if( dataLength < 126 )
		{
			return (_WS_MIN_HEADER_LEN + (isClient ? _WS_MASK_KEY_FIELD_LEN : 0));	/* 2 */
		}
		else if ( dataLength <= 65535 )
		{
			return (_WS_126_HEADER_LEN + (isClient ? _WS_MASK_KEY_FIELD_LEN : 0)); /* 2 + 2 */
		}
		else
		{
			return (_WS_127_HEADER_LEN + (isClient ? _WS_MASK_KEY_FIELD_LEN : 0)); /* 2 + 8 */
		}
	}
	
	static int encode(ByteBuffer msgBuffer, int index, long dataLength, int protocol, boolean isClient, boolean finBit, boolean compressed, int opCode)
	{
		int maskLen;
		int plHdrLen;
		int hdrLen;
		int hdrIndex = index;
		
		maskLen = isClient ? _WS_MASK_KEY_FIELD_LEN : 0;
		
		if( dataLength < 126 )
			plHdrLen = _WS_MIN_HEADER_LEN;	/* 2 */
		else if ( dataLength <= 65535 )
			plHdrLen = _WS_126_HEADER_LEN; /* 2 + 2 */
		else
			plHdrLen = _WS_127_HEADER_LEN; /* 2 + 8 */
		
		hdrLen = plHdrLen + maskLen; /* + [ 0 | 4 ] */
		
		/* Always reset the first byte of the buffer. */
		msgBuffer.array()[hdrIndex] = 0;
		
		/* Set FIN */
		if (finBit)
			setBit(msgBuffer.array(), hdrIndex, 7);
		
		/* Set RSV1 */
		if (compressed)
			setBit(msgBuffer.array(), hdrIndex, 6);
		
		if (opCode != _WS_OPC_NONE)
			/* set opcode for arg opcode frame */
			msgBuffer.array()[hdrIndex] |= (opCode & 0x0F);
		else if (protocol == _WS_SP_JSON2)
			/* set opcode for text frame */
			msgBuffer.array()[hdrIndex] |= (_WS_OPC_TEXT & 0x0F);
		else
			/* set opcode for binary frame */
			msgBuffer.array()[hdrIndex] |= (_WS_OPC_BINARY & 0x0F);
		
		/* Populate the WS payload length field or 
		 * the Extended payload length(126|127) */
		switch (plHdrLen)
		{
			case _WS_126_HEADER_LEN:
			{
				msgBuffer.put(hdrIndex + _WS_PAYLOAD_LEN_OFFSET, (byte)126);
				msgBuffer.putShort(hdrIndex + _WS_EXTENDED_PAYLOAD_OFFSET, (short)dataLength);
				break;
			}
			case _WS_127_HEADER_LEN:
			{
				msgBuffer.put(hdrIndex + _WS_PAYLOAD_LEN_OFFSET, (byte)127);
				msgBuffer.putLong(hdrIndex + _WS_EXTENDED_PAYLOAD_OFFSET, dataLength);
				break;
			}
			// if the case _WS_MIN_HEADER_LEN:
			default:
			{
				msgBuffer.put(hdrIndex + _WS_PAYLOAD_LEN_OFFSET, (byte)dataLength);
				break;
			}
		}
		
		/* The payload data for all Client to Server frames are masked */
		if (isClient)
		{
			int maskValue = getRandomValue();
			WebSocketFrameParser._lastMaskValue = maskValue;
			int maskOffSet = hdrLen - maskLen;
			byte[] mask = new byte[4];

			setBit(msgBuffer.array(), hdrIndex + 1, 7);
			msgBuffer.putInt(hdrIndex + maskOffSet, maskValue);
			
			setMaskKey(mask, maskValue);
			maskDataBlock(mask,msgBuffer.array(), hdrIndex + hdrLen, (int)dataLength);
		}
	
		return hdrLen;
	}
}
