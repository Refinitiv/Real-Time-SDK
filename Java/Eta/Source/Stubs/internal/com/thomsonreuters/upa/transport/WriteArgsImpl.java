package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.WriteArgs;

public class WriteArgsImpl implements WriteArgs
{
	
	@Override
	public void priority(int priority)
	{
      	}

	@Override
	public int priority()
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public void flags(int flags)
	{
	   	}

	@Override
	public int flags()
	{
		return TransportReturnCodes.FAILURE;
	}
	@Override
	public int bytesWritten()
	{
		return TransportReturnCodes.FAILURE;
	}

	    @Override
	public int uncompressedBytesWritten()
	{
	    	return TransportReturnCodes.FAILURE;
	}

	@Override
	public void clear()
	{
		
	}

	@Override
	public void seqNum(long seqNum) 
	{
		
		
	}

	@Override
	public long seqNum() 
	{
		return TransportReturnCodes.FAILURE;
	}
}
