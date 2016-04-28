package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.AcceptOptions;

class AcceptOptionsImpl implements AcceptOptions
{
  

    @Override
    public void clear()
    {
      
    }

    @Override
    public void nakMount(boolean nakMount)
    {
       
    }

    @Override
    public boolean nakMount()
    {
        return false;
    }

    @Override
    public void userSpecObject(Object userSpecObject)
    {
     
    }

    @Override
    public Object userSpecObject()
    {
        return null;
    }
    
	@Override
	public void channelReadLocking(boolean locking) 
	{
		
	}

	@Override
	public boolean channelReadLocking() 
	{
		return false;
	}

	@Override
	public void channelWriteLocking(boolean locking) 
	{
		
	}

	@Override
	public boolean channelWriteLocking()
	{
		return false;
	}
	
    @Override
    public void sysSendBufSize(int sysSendBufSize)
    {
    	
    }

    @Override
    public int sysSendBufSize()
    {
    	return TransportReturnCodes.FAILURE;
    }
}