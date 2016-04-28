package com.thomsonreuters.upa.transport;

import com.thomsonreuters.upa.transport.ConnectOptions;
import com.thomsonreuters.upa.transport.TcpOpts;
import com.thomsonreuters.upa.transport.UnifiedNetworkInfo;

class ConnectOptionsImpl implements ConnectOptions
{
	
	@Override
    public void clear()
    {
		
    }
	
  
	@Override
	public String toString()
	{
		return  null;
	}
	
	@Override
	public void componentVersion(String componentVersion)
	{
		
	}
	
	public String componentVersion()
	{
		return null;
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
    public void compressionType(int compressionType)
    {
       
    }

    @Override
    public int compressionType()
    {
    	return TransportReturnCodes.FAILURE;
    }

    @Override
    public void blocking(boolean blocking)
    {
       
    }

    @Override
    public boolean blocking()
    {
        return false;
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
    public MCastOpts multicastOpts()
    {
        return null;
    }

    @Override
    public ShmemOpts shmemOpts()
    {
        return null;
    }

	@Override
	public UnifiedNetworkInfo unifiedNetworkInfo() 
	{
		return null;
	}
	
	@Override
	public SegmentedNetworkInfo segmentedNetworkInfo() 
	{
		return null;
	}

    @Override
    public TunnelingInfo tunnelingInfo() 
    {
        return null;
    }
    
    @Override
    public CredentialsInfo credentialsInfo() 
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
	public SeqMCastOpts seqMCastOpts()
	{
	    return null;
	}

    @Override
    public int copy(ConnectOptions destOpts)
    {
		return TransportReturnCodes.FAILURE;
    }
}
