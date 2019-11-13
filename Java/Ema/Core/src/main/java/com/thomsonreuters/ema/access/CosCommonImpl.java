///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


public class CosCommonImpl implements CosCommon
{

	private int _maxMsgSize = com.thomsonreuters.upa.valueadd.reactor.CosCommon.DEFAULT_MAX_MSG_SIZE;

	@Override
	public CosCommon clear()
	{
		_maxMsgSize = com.thomsonreuters.upa.valueadd.reactor.CosCommon.DEFAULT_MAX_MSG_SIZE;
		return this;
	}

	@Override
	public CosCommon maxMsgSize(int maxMsgSize)
	{
		if (maxMsgSize <= 0)
		{
			return this;
		}

		_maxMsgSize = maxMsgSize;
		return this;
	}

	@Override
	public int maxMsgSize()
	{
		return _maxMsgSize;
	}

}
