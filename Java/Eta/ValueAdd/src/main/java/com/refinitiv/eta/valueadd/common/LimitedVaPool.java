/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.common;

public class LimitedVaPool extends VaPool
{
	private int limit = -1;

	public LimitedVaPool()
	{
	}

	public LimitedVaPool(boolean useConcurrent)
	{
		super(useConcurrent);
	}

	public LimitedVaPool(boolean useConcurrent, boolean debug)
	{
		super(useConcurrent, debug);
	}

	@Override
	public void add(VaNode node)
	{
		if(checkPoolLimit(limit))
		{
			super.add(node);
		}
	}

	public void setLimit(int limit)
	{
		this.limit = limit;
	}

	private boolean checkPoolLimit(int limit)
	{
		return limit < 0 || size() < limit;
	}
}
