///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


class OmmConsumerEventImpl implements OmmConsumerEvent
{
	Item _item;
	
	@Override
	public long handle()
	{
		return _item.itemId();
	}

	@Override
	public Object closure()
	{
		return _item.closure();
	}

	@Override
	public long parentHandle()
	{
		return _item.parent().itemId();
	}
}