///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

class ChannelConfig
{
	String				name;
	String				interfaceName;
	int					compressionType;
	int					compressionThreshold;
	boolean				compressionThresholdSet;
	int					rsslConnectionType;
	int 	 			encryptedProtocolType;
	int					connectionPingTimeout;
	int					guaranteedOutputBuffers;
	int					numInputBuffers;
	int					sysRecvBufSize;
	int					sysSendBufSize;
	int 				highWaterMark;
	ChannelInfo			channelInfo;
	int					initializationTimeout;

	ChannelConfig() 
	{
		clear();	 
	}
	
	void clear() 
	{
		interfaceName =  ActiveConfig.DEFAULT_INTERFACE_NAME;
		compressionType = ActiveConfig.DEFAULT_COMPRESSION_TYPE;
		compressionThreshold = ActiveConfig.DEFAULT_COMPRESSION_THRESHOLD;
		connectionPingTimeout = ActiveConfig.DEFAULT_CONNECTION_PINGTIMEOUT;
		guaranteedOutputBuffers = ActiveConfig.DEFAULT_GUARANTEED_OUTPUT_BUFFERS;
		numInputBuffers = ActiveConfig.DEFAULT_NUM_INPUT_BUFFERS;
		sysSendBufSize = ActiveConfig.DEFAULT_SYS_SEND_BUFFER_SIZE;
		sysRecvBufSize = ActiveConfig.DEFAULT_SYS_RECEIVE_BUFFER_SIZE;
		highWaterMark = ActiveConfig.DEFAULT_HIGH_WATER_MARK;
		rsslConnectionType = ActiveConfig.DEFAULT_CONNECTION_TYPE;
		encryptedProtocolType = ActiveConfig.DEFAULT_ENCRYPTED_PROTOCOL_TYPE;
		initializationTimeout = ActiveConfig.DEFAULT_INITIALIZATION_TIMEOUT;
		compressionThresholdSet = false;
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

