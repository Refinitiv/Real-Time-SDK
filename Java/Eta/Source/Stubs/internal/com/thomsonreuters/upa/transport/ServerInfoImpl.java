package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.ServerInfo;

public class ServerInfoImpl implements ServerInfo
{
	
	@Override
	public String toString()
	{
		return null;
	}

	@Override
    public int currentBufferUsage()
    {
	  return TransportReturnCodes.FAILURE;
    }

    @Override
    public int peakBufferUsage()
    {
    	return TransportReturnCodes.FAILURE;
    }

	@Override
	public void clear()
	{
		
	}
}
