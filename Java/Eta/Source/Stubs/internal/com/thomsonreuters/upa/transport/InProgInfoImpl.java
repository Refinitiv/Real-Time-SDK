package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.InProgInfo;

public class InProgInfoImpl implements InProgInfo
{
		
	@Override
	public String toString()
	{
		return null;
	}

    @Override
    public int flags()
    {
    	return TransportReturnCodes.FAILURE;
    }
 @Override
    public java.nio.channels.SocketChannel oldScktChannel()
    {
        return null;
    }
    @Override
    public java.nio.channels.SelectableChannel newScktChannel()
    {
        return null;
    }

    @Override
    public java.nio.channels.SelectableChannel oldSelectableChannel()
    {
        return null;
    }
    @Override
    public java.nio.channels.SelectableChannel newSelectableChannel()
    {
        return null;
    }

	@Override
	public void clear()
	{
		
	}
}
