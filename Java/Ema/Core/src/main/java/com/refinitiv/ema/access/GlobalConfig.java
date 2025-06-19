/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

class GlobalConfig
{
	final static int DEFAULT_EVENT_POOL_LIMIT = -1;
	final static int DEFAULT_WATCHLIST_OBJECTS_POOL_LIMIT = -1;
	static final int JSON_CONVERTER_DEFAULT_POOLS_SIZE = 10;

	int reactorMsgEventPoolLimit;
	int reactorChannelEventPoolLimit;
	int workerEventPoolLimit;
	int tunnelStreamMsgEventPoolLimit;
	int tunnelStreamStatusEventPoolLimit;
	int jsonConverterPoolsSize;
	int watchlistObjectsPoolLimit;

	GlobalConfig()
	{
		super();
		reactorMsgEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		reactorChannelEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		workerEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		tunnelStreamMsgEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		tunnelStreamStatusEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		jsonConverterPoolsSize = JSON_CONVERTER_DEFAULT_POOLS_SIZE;
		watchlistObjectsPoolLimit = DEFAULT_WATCHLIST_OBJECTS_POOL_LIMIT;
	}

	void clear()
	{
		reactorMsgEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		reactorChannelEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		workerEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		tunnelStreamMsgEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		tunnelStreamStatusEventPoolLimit = DEFAULT_EVENT_POOL_LIMIT;
		jsonConverterPoolsSize = JSON_CONVERTER_DEFAULT_POOLS_SIZE;
		watchlistObjectsPoolLimit = DEFAULT_WATCHLIST_OBJECTS_POOL_LIMIT;
	}
}
