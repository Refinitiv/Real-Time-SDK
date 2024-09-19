///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
// *|                See the project's LICENSE.md for details.
// *|           Copyright (C) 2019 LSEG. All rights reserved.     
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.eta.transport.ConnectionTypes;

class SocketChannelConfig extends ChannelConfig
{
	String				hostName;
	String				serviceName;
	boolean				tcpNodelay;
	boolean				directWrite;
	EncryptionConfig 	encryptionConfig = new EncryptionConfig();
	Boolean 			httpProxy;
	String 				httpProxyHostName;
	String 				httpProxyPort;
	
	/* Credential configuration parameters */
	String				httpProxyUserName;
	String				httpproxyPasswd;
	String				httpProxyDomain;
	String 				httpProxyLocalHostName;
	String				httpProxyKRB5ConfigFile;
	
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
		httpProxy = ActiveConfig.DEFAULT_HTTP_PROXY;
	}
}
