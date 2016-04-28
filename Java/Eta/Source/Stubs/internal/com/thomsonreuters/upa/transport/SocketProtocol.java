package com.thomsonreuters.upa.transport;



class SocketProtocol implements Protocol 
{

	public SocketProtocol(ConnectOptions opts)
	{
	
	}

	public SocketProtocol() 
	{
		
	}

	@Override
	public Channel channel(ConnectOptions opts, Error error) 
	{
		return null;
	}

	@Override
	public Server server(BindOptions opts, Error error) 
	{
	
		return null;
	}

	@Override
	public Channel channel(AcceptOptions options, Server server, Object object) 
	{
	
		return null;
	}

	@Override
	public void uninitialize() 
	{
			
	}

	public void setHTTP()
	{
	
		
	}
	
}
