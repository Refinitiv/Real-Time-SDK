package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.Channel;
import com.thomsonreuters.upa.transport.Error;

class ErrorImpl implements Error
{
		
	@Override
	public String toString()
	{
		return null;
	}

    @Override
    public void channel(Channel channel)
    {
     
    }

    @Override
    public Channel channel()
    {
        return null;
    }

    @Override
    public void errorId(int errorId)
    {
        
    }

    @Override
    public int errorId()
    {
    	return TransportReturnCodes.FAILURE;
    }

    @Override
    public void sysError(int sysError)
    {
       
    }

    @Override
    public int sysError()
    {
    	return TransportReturnCodes.FAILURE;
    }

    @Override
    public void text(String text)
    {
        
    }

    @Override
    public String text()
    {
        return null;
    }

	@Override
	public void clear()
	{
		
	}
}
