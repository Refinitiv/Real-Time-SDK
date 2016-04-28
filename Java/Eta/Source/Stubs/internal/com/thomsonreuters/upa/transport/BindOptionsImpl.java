package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.BindOptions;
import com.thomsonreuters.upa.transport.TcpOpts;

class BindOptionsImpl implements BindOptions
{
  

	@Override
    public void clear()
    {
	
    }
	
    @Override
	public String toString()
	{
    	return null;
	}
    
    @Override 
    public void componentVersion(String componentVersion)
    {
    	
    }
    
    @Override 
    public String componentVersion()
    {
    	return null;
    }
    
    @Override
	public void serviceName(String serviceName)
	{
        
	}

    @Override
	public String serviceName()
	{
		return null;
	}

    int port()
    {
    	return TransportReturnCodes.FAILURE;
    }

    @Override
	public void interfaceName(String interfaceName)
	{
       
	}

    @Override
	public String interfaceName()
	{
		return null;
	}

    @Override
	public void compressionType(int compressionType)
	{
      
	}

    @Override
	public int compressionType()
	{
    	return TransportReturnCodes.FAILURE;
	}

    @Override
	public void compressionLevel(int compressionLevel)
	{
       
	}

    @Override
	public int compressionLevel()
	{
    	return TransportReturnCodes.FAILURE;
	}

    @Override
	public void forceCompression(boolean forceCompression)
	{
		
	}

    @Override
	public boolean forceCompression()
	{
		return false;
	}

    @Override
	public void serverBlocking(boolean serverBlocking)
	{
		
	}

    @Override
	public boolean serverBlocking()
	{
		return false;
	}

    @Override
	public void channelsBlocking(boolean channelsBlocking)
	{
		
	}

    @Override
	public boolean channelsBlocking()
	{
		return false;
	}

    @Override
	public void serverToClientPings(boolean serverToClientPings)
	{
		
	}

    @Override
	public boolean serverToClientPings()
	{
		return false;
	}

    @Override
	public void clientToServerPings(boolean clientToServerPings)
	{
		
	}

    @Override
	public boolean clientToServerPings()
	{
		return false;
	}

    @Override
	public void connectionType(int connectionType)
	{
       
	}

    @Override
	public int connectionType()
	{
		return TransportReturnCodes.FAILURE;
	}

    @Override
	public void pingTimeout(int pingTimeout)
	{
        
	}

    @Override
	public int pingTimeout()
	{
    	return TransportReturnCodes.FAILURE;
	}

    @Override
	public void minPingTimeout(int minPingTimeout)
	{
       
	}

    @Override
	public int minPingTimeout() 
    {
    	return TransportReturnCodes.FAILURE;
	}

    @Override
	public void maxFragmentSize(int maxFragmentSize)
	{
       
	}

    @Override
	public int maxFragmentSize()
	{
    	return TransportReturnCodes.FAILURE;
	}

    @Override
	public void maxOutputBuffers(int maxOutputBuffers)
	{
        
	}

    @Override
	public int maxOutputBuffers()
	{
    	return TransportReturnCodes.FAILURE;
	}

    @Override
	public void guaranteedOutputBuffers(int guaranteedOutputBuffers)
	{
       
	}

    @Override
	public int guaranteedOutputBuffers()
	{
    	return TransportReturnCodes.FAILURE;
	}

    @Override
	public void numInputBuffers(int numInputBuffers)
	{
       
	}

    @Override
	public int numInputBuffers() 
    {
    	return TransportReturnCodes.FAILURE;
	}

    @Override
	public void sharedPoolSize(int sharedPoolSize)
	{
        
	}

    @Override
	public int sharedPoolSize()
	{
    	return TransportReturnCodes.FAILURE;
	}

    @Override
	public void sharedPoolLock(boolean sharedPoolLock)
	{
		
	}

    @Override
	public boolean sharedPoolLock() 
    {
		return false;
	}

    @Override
	public void majorVersion(int majorVersion)
	{
		
	}

    @Override
	public int majorVersion()
	{
    	return TransportReturnCodes.FAILURE;
	}

    @Override
	public void minorVersion(int minorVersion)
	{
		
	}

    @Override
	public int minorVersion()
	{
    	return TransportReturnCodes.FAILURE;
	}

    @Override
	public void protocolType(int protocolType)
	{
        
	}

    @Override
	public int protocolType()
	{
    	return TransportReturnCodes.FAILURE;
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
    public TcpOpts tcpOpts()
    {
        return null;
    }

    @Override
    public void sysRecvBufSize(int sysRecvBufSize)
    {
    	
    }

    @Override
    public int sysRecvBufSize()
    {
    	return TransportReturnCodes.FAILURE;
    }
    
    @Override
    public void groupAddress(String groupAddress)
    {
       
    }
    
    @Override
    public String groupAddress()
    {
        return null;
    }
}
