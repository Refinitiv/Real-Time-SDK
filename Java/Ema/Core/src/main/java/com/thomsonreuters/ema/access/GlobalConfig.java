package com.thomsonreuters.ema.access;

class GlobalConfig
{
	final static int DEFAULT_EVENT_POOL_LIMIT = -1;

	int reactorMsgEventPoolLimit;
	int reactorChannelEventPoolLimit;
	int workerEventPoolLimit;

	GlobalConfig()
	{
		super();
		reactorMsgEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		reactorChannelEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		workerEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
	}

	void clear()
	{
		reactorMsgEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		reactorChannelEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		workerEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
	}
}
