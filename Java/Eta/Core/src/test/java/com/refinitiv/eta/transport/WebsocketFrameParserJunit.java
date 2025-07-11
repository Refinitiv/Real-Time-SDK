/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

import static org.junit.Assert.*;

import java.nio.ByteBuffer;

import com.refinitiv.eta.JUnitConfigVariables;
import com.refinitiv.eta.RetryRule;
import com.refinitiv.eta.transport.WebSocketFrameHdr;
import com.refinitiv.eta.transport.WebSocketFrameParser;
import org.junit.Before;
import org.junit.Rule;
import org.junit.Test;
import org.junit.rules.TestName;

public class WebsocketFrameParserJunit 
{
	@Rule
	public RetryRule retryRule = new RetryRule(JUnitConfigVariables.TEST_RETRY_COUNT);

	@Rule
	public TestName testName = new TestName();

	@Before
	public void printTestName() {
		System.out.println(">>>>>>>>>>>>>>>>>>>>  " + testName.getMethodName() + " Test <<<<<<<<<<<<<<<<<<<<<<<");
	}

	@Test
    public void encodeAndDecode4SmallPayloadOnClient()
    {
		int payloadLength = 124;
		ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength + WebSocketFrameParser._WS_MAX_HEADER_LEN);
		
		/*Simulate payload data */
		byte[] originalPayload = new byte[payloadLength];
		for(int index = 0; index < payloadLength; index++)
		{
			originalPayload[index] = (byte) (WebSocketFrameParser.getRandomValue() % 127);
		}
		
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
	
		msgBuffer.position(6);
		msgBuffer.put(originalPayload, 0, payloadLength);
		
		msgBuffer.rewind();
		
		int hdrLen = WebSocketFrameParser.encode(msgBuffer, 0, payloadLength, WebSocketFrameParser._WS_SP_JSON2, true, true, true, WebSocketFrameParser._WS_OPC_NONE);
		int maskValue = WebSocketFrameParser.getLastMaskValue();
		 
		WebSocketFrameParser.decode(frame, msgBuffer, 0, hdrLen + payloadLength);
		msgBuffer.position(6);
		
		byte[] payloadArray = new byte[payloadLength];
		msgBuffer.get(payloadArray, 0, payloadLength);
		
		byte[] mask = new byte[4];
		
		WebSocketFrameParser.setMaskKey(mask, frame.maskVal);
		
		/* Unmask the payload data */
		WebSocketFrameParser.maskDataBlock(mask, payloadArray, 0, payloadLength);
		
		 assertEquals(0, ByteBuffer.wrap(originalPayload).compareTo(ByteBuffer.wrap(payloadArray)));
		 assertEquals(true, frame.finSet);
		 assertEquals(true, frame.rsv1Set);
		 assertEquals(true, frame.compressed);
		 assertEquals(hdrLen, frame.hdrLen);
		 assertEquals(false, frame.fragment);
		 assertEquals(false, frame.partial);
		 assertEquals(2, frame.extHdrIndex);
		 assertEquals(0, frame.extHdrLen);
		 assertEquals(6, frame.payloadIndex);
		 assertEquals(payloadLength, frame.payloadLen);
		 assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.opcode);
		 assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.dataType);
		 assertEquals(true, frame.maskSet);
		 assertEquals(maskValue, frame.maskVal);
    }
	
	@Test
    public void encodeAndDecode4SmallPayloadOnServer()
    {
		int payloadLength = 125;
		ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength + WebSocketFrameParser._WS_MAX_HEADER_LEN);
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		int hdrLen = WebSocketFrameParser.encode(msgBuffer, 0, payloadLength, WebSocketFrameParser._WS_SP_JSON2, false, true, false, WebSocketFrameParser._WS_OPC_NONE);
		
		WebSocketFrameParser.decode(frame, msgBuffer, 0, hdrLen + payloadLength);
		 
		 assertEquals(true, frame.finSet);
		 assertEquals(false, frame.rsv1Set);
		 assertEquals(false, frame.compressed);
		 assertEquals(hdrLen, frame.hdrLen);
		 assertEquals(false, frame.fragment);
		 assertEquals(false, frame.partial);
		 assertEquals(2, frame.extHdrIndex);
		 assertEquals(0, frame.extHdrLen);
		 assertEquals(2, frame.payloadIndex);
		 assertEquals(payloadLength, frame.payloadLen);
		 assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.opcode);
		 assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.dataType);
		 assertEquals(false, frame.maskSet);
		 assertEquals(0, frame.maskVal);
    }

	@Test
	public void encodeAndDecode4MediumPayloadOnClient()
    {
		int payloadLength = 255;
		ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength + WebSocketFrameParser._WS_MAX_HEADER_LEN);
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		int hdrLen = WebSocketFrameParser.encode(msgBuffer, 0, payloadLength, WebSocketFrameParser._WS_SP_RWF, true, false, false, WebSocketFrameParser._WS_OPC_NONE);
		int maskValue = WebSocketFrameParser.getLastMaskValue();
		
		WebSocketFrameParser.decode(frame, msgBuffer, 0, hdrLen + payloadLength);
		 
		 assertEquals(false, frame.finSet);
		 assertEquals(false, frame.rsv1Set);
		 assertEquals(false, frame.rsv2Set);
		 assertEquals(false, frame.rsv3Set);
		 assertEquals(false, frame.compressed);
		 assertEquals(hdrLen, frame.hdrLen);
		 assertEquals(true, frame.fragment);
		 assertEquals(false, frame.partial);
		 assertEquals(4, frame.extHdrIndex);
		 assertEquals(2, frame.extHdrLen);
		 assertEquals(8, frame.payloadIndex);
		 assertEquals(payloadLength, frame.payloadLen);
		 assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame.opcode);
		 assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame.dataType);
		 assertEquals(true, frame.maskSet);
		 assertEquals(maskValue, frame.maskVal);
    }
	
	@Test
	public void encodeAndDecode4MediumPayloadOnServer()
    {
		int payloadLength = 255;
		ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength + WebSocketFrameParser._WS_MAX_HEADER_LEN);
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		int hdrLen = WebSocketFrameParser.encode(msgBuffer, 0, payloadLength, WebSocketFrameParser._WS_SP_RWF, false, false, false, WebSocketFrameParser._WS_OPC_NONE);
		
		WebSocketFrameParser.decode(frame, msgBuffer, 0, hdrLen + payloadLength);
		 
		 assertEquals(false, frame.finSet);
		 assertEquals(false, frame.rsv1Set);
		 assertEquals(false, frame.rsv2Set);
		 assertEquals(false, frame.rsv3Set);
		 assertEquals(false, frame.compressed);
		 assertEquals(hdrLen, frame.hdrLen);
		 assertEquals(true, frame.fragment);
		 assertEquals(false, frame.partial);
		 assertEquals(4, frame.extHdrIndex);
		 assertEquals(2, frame.extHdrLen);
		 assertEquals(4, frame.payloadIndex);
		 assertEquals(payloadLength, frame.payloadLen);
		 assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame.opcode);
		 assertEquals(WebSocketFrameParser._WS_OPC_BINARY, frame.dataType);
		 assertEquals(false, frame.maskSet);
		 assertEquals(0, frame.maskVal);
    }
	
	@Test
	public void encodeAndDecode4LargePayloadOnClient()
    {
		int payloadLength = 65537;
		ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength + WebSocketFrameParser._WS_MAX_HEADER_LEN);
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		int hdrLen = WebSocketFrameParser.encode(msgBuffer, 0, payloadLength, WebSocketFrameParser._WS_SP_NONE, true, true, false, WebSocketFrameParser._WS_OPC_CLOSE);
		int maskValue = WebSocketFrameParser.getLastMaskValue();
		
		WebSocketFrameParser.decode(frame, msgBuffer, 0, hdrLen + payloadLength);
		 
		 assertEquals(true, frame.finSet);
		 assertEquals(false, frame.rsv1Set);
		 assertEquals(false, frame.rsv2Set);
		 assertEquals(false, frame.rsv3Set);
		 assertEquals(false, frame.compressed);
		 assertEquals(hdrLen, frame.hdrLen);
		 assertEquals(false, frame.fragment);
		 assertEquals(false, frame.partial);
		 assertEquals(10, frame.extHdrIndex);
		 assertEquals(8, frame.extHdrLen);
		 assertEquals(14, frame.payloadIndex);
		 assertEquals(payloadLength, frame.payloadLen);
		 assertEquals(WebSocketFrameParser._WS_OPC_CLOSE, frame.opcode);
		 assertEquals(0, frame.dataType); /* This is control frame. */
		 assertEquals(true, frame.maskSet);
		 assertEquals(maskValue, frame.maskVal);
    }
	
	@Test
	public void encodeAndDecode4LargePayloadOnServer()
    {
		int payloadLength = 65537;
		ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength + WebSocketFrameParser._WS_MAX_HEADER_LEN);
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		int hdrLen = WebSocketFrameParser.encode(msgBuffer, 0, payloadLength, WebSocketFrameParser._WS_SP_NONE, false, false, true, WebSocketFrameParser._WS_OPC_CONT);
		
		WebSocketFrameParser.decode(frame, msgBuffer, 0, hdrLen + payloadLength);
		 
		 assertEquals(false, frame.finSet);
		 assertEquals(true, frame.rsv1Set);
		 assertEquals(false, frame.rsv2Set);
		 assertEquals(false, frame.rsv3Set);
		 assertEquals(false, frame.compressed); /* continuation frame */
		 assertEquals(hdrLen, frame.hdrLen);
		 assertEquals(true, frame.fragment);
		 assertEquals(false, frame.partial);
		 assertEquals(10, frame.extHdrIndex);
		 assertEquals(8, frame.extHdrLen);
		 assertEquals(10, frame.payloadIndex);
		 assertEquals(payloadLength, frame.payloadLen);
		 assertEquals(WebSocketFrameParser._WS_OPC_CONT, frame.opcode);
		 assertEquals(0, frame.dataType);
		 assertEquals(false, frame.maskSet);
		 assertEquals(0, frame.maskVal);
    }
	
	@Test
	public void partialWebSocketHeaderReadForOneByte()
    {
		ByteBuffer msgBuffer = ByteBuffer.allocate(1);
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		/* Simulate to read data only one byte. */
		WebSocketFrameParser.decode(frame, msgBuffer, 0, 1);
		assertEquals(true, frame.partial);
    }
	
	@Test
	public void partialWSMsgLessThanWSHeader()
    {
		int payloadLength = 125;
		ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength + WebSocketFrameParser._WS_MAX_HEADER_LEN);
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		int hdrLen = WebSocketFrameParser.encode(msgBuffer, 0, payloadLength, WebSocketFrameParser._WS_SP_JSON2, true, true, true, WebSocketFrameParser._WS_OPC_NONE);
		 
		WebSocketFrameParser.decode(frame, msgBuffer, 0, hdrLen - 1);
		 
		 assertEquals(true, frame.finSet);
		 assertEquals(true, frame.rsv1Set);
		 assertEquals(true, frame.compressed);
		 assertEquals(hdrLen, frame.hdrLen);
		 assertEquals(false, frame.fragment);
		 assertEquals(true, frame.partial);
		 assertEquals(0, frame.extHdrIndex);
		 assertEquals(0, frame.extHdrLen);
		 assertEquals(0, frame.payloadIndex);
		 assertEquals(payloadLength, frame.payloadLen);
		 assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.opcode);
		 assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.dataType);
		 assertEquals(true, frame.maskSet);
		 assertTrue(frame.maskVal == 0);
    }
	
	@Test
	public void partialWebSocketMsgRead()
	{
		int payloadLength = 125;
		ByteBuffer msgBuffer = ByteBuffer.allocate(payloadLength + WebSocketFrameParser._WS_MAX_HEADER_LEN);
		WebSocketFrameHdr frame = new WebSocketFrameHdr();
		
		int hdrLen = WebSocketFrameParser.encode(msgBuffer, 0, payloadLength, WebSocketFrameParser._WS_SP_JSON2, true, true, true, WebSocketFrameParser._WS_OPC_NONE);
		int maskValue = WebSocketFrameParser.getLastMaskValue();
		
		WebSocketFrameParser.decode(frame, msgBuffer, 0, hdrLen + payloadLength - 1);
		 
		 assertEquals(true, frame.finSet);
		 assertEquals(true, frame.rsv1Set);
		 assertEquals(true, frame.compressed);
		 assertEquals(hdrLen, frame.hdrLen);
		 assertEquals(false, frame.fragment);
		 assertEquals(true, frame.partial);
		 assertEquals(2, frame.extHdrIndex);
		 assertEquals(0, frame.extHdrLen);
		 assertEquals(6, frame.payloadIndex);
		 assertEquals(payloadLength, frame.payloadLen);
		 assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.opcode);
		 assertEquals(WebSocketFrameParser._WS_OPC_TEXT, frame.dataType);
		 assertEquals(true, frame.maskSet);
		 assertEquals(maskValue, frame.maskVal);
	}
	
	@Test
	public void webSocketClientMask()
	{
		byte[] mask = new byte[4];
		int maskValue = WebSocketFrameParser.getRandomValue();
		ByteBuffer msgBuffer = ByteBuffer.allocate(256);
		
		for(int i = 0; i < 64; i++)
		{
			msgBuffer.putInt(WebSocketFrameParser.getRandomValue());
		}
		
		msgBuffer.rewind();
		
		byte[] originalArray = msgBuffer.array().clone();

		WebSocketFrameParser.setMaskKey(mask, maskValue);
		
		/* Mask the buffer */
		WebSocketFrameParser.maskDataBlock(mask, msgBuffer.array(), 0, 256);
		
		/* Unmask the buffer */
		WebSocketFrameParser.maskDataBlock(mask, msgBuffer.array(), 0, 256);
		
		ByteBuffer originalBufer = ByteBuffer.wrap(originalArray);
		
		assertEquals(0, originalBufer.compareTo(msgBuffer));
	}
}
