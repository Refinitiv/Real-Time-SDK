package com.thomsonreuters.upa.transport;


import java.nio.channels.SelectableChannel;
import java.nio.channels.ServerSocketChannel;

class ServerImpl implements Server
{

	@Override
	public int info(ServerInfo info, Error error)
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int ioctl(int code, Object value, Error error)
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int ioctl(int code, int value, Error error)
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int bufferUsage(Error error)
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public int close(Error error) 
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public Channel accept(AcceptOptions opts, Error error) 
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ServerSocketChannel srvrScktChannel()
	{
		return null;
	}

	@Override
	public SelectableChannel selectableChannel()
	{
		return null;
	}

	@Override
	public int portNumber() 
	{
		return TransportReturnCodes.FAILURE;
	}

	@Override
	public Object userSpecObject()
	{
		return null;
	}

	@Override
	public int state() 
	{
		return TransportReturnCodes.FAILURE;
	}
 
}
