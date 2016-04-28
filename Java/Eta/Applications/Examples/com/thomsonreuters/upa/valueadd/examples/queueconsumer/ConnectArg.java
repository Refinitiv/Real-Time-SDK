package com.thomsonreuters.upa.valueadd.examples.queueconsumer;

import com.thomsonreuters.upa.rdm.DomainTypes;

/* Connection argument class for the Value Add queue consumer application. */
class ConnectArg
{
	int connectionType; /* type of the connection */
	String service; /* name of service */
	
	String hostname; /* hostname of provider to connect to */
	String port; /* port of provider to connect to */
	boolean tunnelAuth; /* Whether the consumer will request authentication on the tunnel stream. */
	int tunnelDomain; /* Domain type to use when opening the tunnel stream. */
	
	ConnectArg(int connectionType, String service, String hostname, String port)
	{
		this.connectionType = connectionType;
		this.service = service;
		this.hostname = hostname;
		this.port = port;
		this.tunnelAuth = false;
		this.tunnelDomain = DomainTypes.SYSTEM;
	}
	
	ConnectArg()
	{
		this.tunnelAuth = false;
		this.tunnelDomain = DomainTypes.SYSTEM;
	}

	int connectionType()
	{
		return connectionType;
	}

	String service()
	{
		return service;		
	}
	
	String hostname()
	{
		return hostname;
	}
	
	String port()
	{
		return port;		
	}

	boolean tunnelAuth()
	{
		return tunnelAuth;		
	}

	int tunnelDomain()
	{
		return tunnelDomain;		
	}
}
