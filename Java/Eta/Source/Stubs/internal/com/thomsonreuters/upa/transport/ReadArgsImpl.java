package com.thomsonreuters.upa.transport;


import com.thomsonreuters.upa.transport.ReadArgs;

public class ReadArgsImpl implements ReadArgs
{
    @Override
    public int readRetVal()
    {
    	return TransportReturnCodes.FAILURE;
    }

    @Override
    public void clear()
    {
       
    }

    @Override
    public int bytesRead()
    {
    	return TransportReturnCodes.FAILURE;
    }

    @Override
    public int uncompressedBytesRead()
    {
    	return TransportReturnCodes.FAILURE;
    }

	@Override
	public long seqNum() 
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public String senderAddress() 
	{
		return null;
	}

	@Override
	public int senderPort() 
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int instanceId() 
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
}
  