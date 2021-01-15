///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

class ServerConfig
{
	String				name;
	String				interfaceName;
	int					compressionType;
	int					compressionThreshold;
	boolean				compressionThresholdSet;
	int					rsslConnectionType;
	int					connectionPingTimeout;
	int					guaranteedOutputBuffers;
	int					numInputBuffers;
	int					sysRecvBufSize;
	int					sysSendBufSize;
	int 				highWaterMark;
	int					connectionMinPingTimeout;
	int					initializationTimeout;
	String 				keystoreFile;
	String 				keystorePasswd;
	String				keystoreType;
	String 				securityProtocol;
	String 				securityProvider;
	String 				keyManagerAlgorithm;
	String 				trustManagerAlgorithm;
	
	ServerConfig()
	{
		clear();
	}
	
	void clear()
	{
		interfaceName =  ActiveConfig.DEFAULT_INTERFACE_NAME;
		compressionType = ActiveConfig.DEFAULT_COMPRESSION_TYPE;
		compressionThreshold = ActiveConfig.DEFAULT_COMPRESSION_THRESHOLD;
		guaranteedOutputBuffers = ActiveConfig.DEFAULT_GUARANTEED_OUTPUT_BUFFERS;
		numInputBuffers = ActiveConfig.DEFAULT_NUM_INPUT_BUFFERS;
		sysSendBufSize = ActiveServerConfig.DEFAULT_SERVER_SYS_SEND_BUFFER_SIZE;
		sysRecvBufSize = ActiveServerConfig.DEFAULT_SERVER_SYS_RECEIVE_BUFFER_SIZE;
		highWaterMark = ActiveConfig.DEFAULT_HIGH_WATER_MARK;
		rsslConnectionType = ActiveConfig.DEFAULT_CONNECTION_TYPE;
		
		connectionPingTimeout = ActiveServerConfig.DEFAULT_CONNECTION_PINGTIMEOUT;
		connectionMinPingTimeout = ActiveServerConfig.DEFAULT_CONNECTION_MINPINGTIMEOUT;
		initializationTimeout = ActiveConfig.DEFAULT_INITIALIZATION_ACCEPT_TIMEOUT;
		compressionThresholdSet = false;
		keystoreFile = null;
		keystorePasswd = null;
		keystoreType = null;
		securityProtocol = null;
		securityProvider = null;
		keyManagerAlgorithm = null;
		trustManagerAlgorithm = null;
	}
	
	void guaranteedOutputBuffers(long value) 
	{
		if ( value > 0 )
			guaranteedOutputBuffers = (int)(value > Integer.MAX_VALUE ? Integer.MAX_VALUE : value);
	}
	void numInputBuffers(long value)
	{
		if ( value > 0 )
			numInputBuffers = (int)(value > Integer.MAX_VALUE ? Integer.MAX_VALUE : value);
	}
}