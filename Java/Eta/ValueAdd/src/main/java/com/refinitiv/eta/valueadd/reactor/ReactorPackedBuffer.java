/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2021-2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.reactor;

import java.nio.ByteBuffer;
import java.util.Objects;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.transport.TransportBuffer;
import com.refinitiv.eta.valueadd.common.VaNode;

class ReactorPackedBuffer extends VaNode
{
	int totalSize;
	int remainingSize;
	private Buffer decodeBuffer;
	private Buffer jsonBuffer;
	private int nextRWFPosition;
	
	private final static int ADDITIONAL_BUFFER_SPACE = 300;
	
	ReactorPackedBuffer()
	{
		decodeBuffer = CodecFactory.createBuffer();
		jsonBuffer = CodecFactory.createBuffer();
		clear();
	}
	
	void clear()
	{
		totalSize = 0;
		remainingSize = 0;
		nextRWFPosition = 0;
	}
	
	Buffer decodeBuffer(TransportBuffer transportBuffer)
	{
		ByteBuffer byteBuffer = transportBuffer.data();
		
		if(nextRWFPosition == 0)
		{
			decodeBuffer.data(byteBuffer, transportBuffer.dataStartPosition(), transportBuffer.length());
		}
		else
		{
			decodeBuffer.data(transportBuffer.data(), nextRWFPosition, transportBuffer.data().position() - nextRWFPosition);
		}
		
		return decodeBuffer;
	}
	
	Buffer jsonBuffer(int bufferLength)
	{
		bufferLength += ADDITIONAL_BUFFER_SPACE;
		
		if(Objects.isNull(jsonBuffer.data()) || jsonBuffer.data().capacity() < bufferLength)
		{
			jsonBuffer.clear();
			jsonBuffer.data(ByteBuffer.allocate(bufferLength));
		}
		else
		{
			ByteBuffer byteBuffer = jsonBuffer.data();
        	byteBuffer.clear();
        	jsonBuffer.clear();
        	jsonBuffer.data(byteBuffer);
		}
		
		return jsonBuffer;
	}
	
	void nextRWFBufferPosition(int nextBufferPosition)
	{
		nextRWFPosition = nextBufferPosition;
	}
	
	int nextRWFBufferPosition()
	{
		return nextRWFPosition;
	}
}
