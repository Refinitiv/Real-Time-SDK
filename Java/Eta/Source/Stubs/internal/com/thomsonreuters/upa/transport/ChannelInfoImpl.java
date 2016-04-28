package com.thomsonreuters.upa.transport;

import java.util.List;

public class ChannelInfoImpl implements ChannelInfo
{
	
	@Override
	public String toString()
	{
	   return null;
	}

	
    @Override
    public int maxFragmentSize()
    {
    	return TransportReturnCodes.FAILURE;
    }

    
    @Override
    public int maxOutputBuffers()
    {
    	return TransportReturnCodes.FAILURE;
    }

   
    @Override
    public int guaranteedOutputBuffers()
    {
    	return TransportReturnCodes.FAILURE;
    }

  
    @Override
    public int numInputBuffers()
    {
    	return TransportReturnCodes.FAILURE;
    }

   
    @Override
    public int pingTimeout()
    {
    	return TransportReturnCodes.FAILURE;
    }

    @Override
    public boolean clientToServerPings()
    {
        return false;
    }

  
    @Override
    public boolean serverToClientPings()
    {
        return false;
    }

   
    @Override
    public int sysSendBufSize()
    {
    	return TransportReturnCodes.FAILURE;
    }

   
    @Override
    public int sysRecvBufSize()
    {
    	return TransportReturnCodes.FAILURE;
    }

   
    @Override
    public int compressionType()
    {
    	return TransportReturnCodes.FAILURE;
    }

   
    @Override
    public int compressionThreshold()
    {
    	return TransportReturnCodes.FAILURE;
    }

  
    @Override
    public String priorityFlushStrategy()
    {
        return null;
    }

    @Override
	public void clear() 
	{
		
	}

    @Override
    public List<ComponentInfo> componentInfo()
    {
        return null;
    }
    
    @Override
    public String clientIP()
    {
        return null;
    }
    
   
    @Override
    public String clientHostname()
    {
        return null;
    }
    
   
	@Override
	public MCastStats multicastStats()
	{
		return null;
	}
	
	
}
