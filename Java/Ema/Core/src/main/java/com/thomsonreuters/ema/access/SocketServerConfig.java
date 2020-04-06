package com.thomsonreuters.ema.access;

import com.thomsonreuters.upa.transport.ConnectionTypes;

class SocketServerConfig extends ServerConfig
{
	String serviceName;
	boolean tcpNodelay;
	boolean directWrite;
	
	SocketServerConfig()
	{
		clear();
	}
	
	@Override
	void clear() 
	{
		super.clear();
		
		rsslConnectionType = ConnectionTypes.SOCKET;	
		serviceName = ActiveServerConfig.defaultServiceName;
		tcpNodelay = ActiveConfig.DEFAULT_TCP_NODELAY;
		directWrite = ActiveConfig.DEFAULT_DIRECT_SOCKET_WRITE;
	}
	
}
