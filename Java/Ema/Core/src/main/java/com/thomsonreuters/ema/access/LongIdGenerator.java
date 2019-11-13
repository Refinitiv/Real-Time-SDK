///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

class LongIdGenerator
{
    private static final long MIN_LONG_VALUE = 1;
    private static final long MAX_LONG_VALUE = Long.MAX_VALUE;
    
	private static long _longId = MIN_LONG_VALUE;

    private static void reset()
    {
        _longId = MIN_LONG_VALUE;
    }

    synchronized static long nextLongId() 
    {
    	if (_longId == MAX_LONG_VALUE)
    		reset();
    	
    	return _longId++;
    }
}