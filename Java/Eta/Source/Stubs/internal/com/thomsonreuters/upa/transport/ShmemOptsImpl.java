package com.thomsonreuters.upa.transport;

class ShmemOptsImpl implements ShmemOpts
{
	

	@Override
	public String toString()
	{
		return null;
	}

	@Override
	public void maxReaderLag(long maxReaderLag)
	{
		
	}

	@Override
	public long maxReaderLag()
	{
		return TransportReturnCodes.FAILURE;
	}
}
