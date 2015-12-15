///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.OmmConsumerErrorClient;

class OmmConsumerErrorClientImpl implements OmmConsumerErrorClient
{
	@Override
	public void onInvalidHandle(long handle, String text) {}

	@Override
	public void onInvalidUsage(String text) {}
}