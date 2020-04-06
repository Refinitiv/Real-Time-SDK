package com.thomsonreuters.ema.access;

import com.thomsonreuters.upa.transport.ConnectionTypes;

class SocketChannelConfig extends ChannelConfig
{
	String				hostName;
	String				serviceName;
	boolean				tcpNodelay;
	boolean				directWrite;
	
	SocketChannelConfig() 
	{
		 clear();
	}

	@Override
	void clear() 
	{
		super.clear();
		
		rsslConnectionType = ConnectionTypes.SOCKET;	
		hostName = ActiveConfig.DEFAULT_HOST_NAME;
		serviceName = ActiveConfig.defaultServiceName;
		tcpNodelay = ActiveConfig.DEFAULT_TCP_NODELAY;
		directWrite = ActiveConfig.DEFAULT_DIRECT_SOCKET_WRITE;
	}
}
