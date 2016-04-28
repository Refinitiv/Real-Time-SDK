package com.thomsonreuters.upa.transport;

class MCastOptsImpl implements MCastOpts {

    @Override
	public void disconnectOnGaps(boolean disconnectOnGaps)
    {
        
	}

	@Override
	public boolean disconnectOnGaps()
	{
        return false;
	}

	@Override
	public void packetTTL(int packetTTL)
	{
		
	}

	@Override
	public int packetTTL()
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public void tcpControlPort(String tcpControlPort)
	{
		
	}

	@Override
	public String tcpControlPort()
	{
		return null;
	}


	@Override
	public void portRoamRange(int portRoamRange)
	{
		
	}
	
	@Override
	public int portRoamRange()
	{
		return TransportReturnCodes.FAILURE;
	}
}
