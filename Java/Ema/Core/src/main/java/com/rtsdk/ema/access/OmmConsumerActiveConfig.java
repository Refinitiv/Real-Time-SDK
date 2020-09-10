///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.rtsdk.ema.access;

class OmmConsumerActiveConfig extends ActiveConfig
{
	static final String DEFAULT_CONSUMER_SERVICE_NAME = "14002";

	OmmConsumerActiveConfig()
	{
		super(DEFAULT_CONSUMER_SERVICE_NAME);
	}
	
	@Override
	void clear()
	{
		super.clear();
	}

}
