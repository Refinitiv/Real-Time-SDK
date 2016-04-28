package com.thomsonreuters.upa.transport;

import java.nio.ByteBuffer;

import com.thomsonreuters.upa.transport.TransportBuffer;
import com.thomsonreuters.upa.transport.TransportReturnCodes;

class TransportBufferImpl implements TransportBuffer
{

	@Override
	public ByteBuffer data() 
	{
		return null;
	}

	@Override
	public int length()
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int copy(ByteBuffer destBuffer) 
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int capacity()
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int dataStartPosition()
	{
		
		return TransportReturnCodes.FAILURE;
	}
	
	
	
}
