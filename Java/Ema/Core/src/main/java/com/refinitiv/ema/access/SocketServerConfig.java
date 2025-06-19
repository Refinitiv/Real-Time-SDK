/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020-2021,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import com.refinitiv.eta.transport.ConnectionTypes;

class SocketServerConfig extends ServerConfig
{
	String serviceName;
	boolean tcpNodelay;
	boolean directWrite;
	String keystoreFile;
	String keystorePasswd;
	String securityProtocol;
	String securityProvider;
	String keyManagerAlgorithm;
	String trustManagerAlgorithm;
	
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
		
		keystoreFile = null;
		keystorePasswd = null;
		securityProtocol = null;
		securityProvider = null;
		keyManagerAlgorithm = null;
		trustManagerAlgorithm = null;
	}
	
}