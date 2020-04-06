package com.thomsonreuters.ema.access;

class ChannelConfig
{
	String				name;
	String				interfaceName;
	int					compressionType;
	int					compressionThreshold;
	int					rsslConnectionType;
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
		initializationTimeout = ActiveConfig.DEFAULT_INITIALIZATION_TIMEOUT;
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
