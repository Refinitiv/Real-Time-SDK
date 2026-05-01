/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024-2026 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.common;

/**
 * Value Add Limited Pool class. Extends {@link VaPool} by providing the limit to the pool size.
 * @see VaPool
 */
public class LimitedVaPool extends VaPool
{
	int _limit = -1;
	/**
	 * Creates a pool. This pool is not thread safe. This pool is not limited by default.
	 * 
	 * @see VaPool
	 */
	public LimitedVaPool()
	{
		super(new LimitedVaQueue(), false);
	}

	/**
	 * Creates a pool. Param useConcurrent can be used to make this pool thread safe.
	 * @param useConcurrent if true, the pool is backed by a {@link VaConcurrentQueue}.
	 * @see VaPool
	 */
	public LimitedVaPool(boolean useConcurrent)
	{
		super(new LimitedVaQueue(), useConcurrent);
	}

	/**
	 * Creates a pool. Param useConcurrent can be used to make this pool thread safe.
	 * @param useConcurrent to set concurrent pool
	 * @param debug to set debug mode
	 * @see VaPool
	 */
	public LimitedVaPool(boolean useConcurrent, boolean debug)
	{
		super(new LimitedVaQueue(), useConcurrent, debug);
	}

	/**
	 * Sets the maximum size of the limited pool.
	 * 
	 * @param limit the maximum size of the limited pool
	 */
	public void setLimit(int limit)
	{
		_limit = limit;
		_queue.setLimit(limit);
	}

	/**
	 * Gets the currently set limit for this LimitedVaPool instance
	 * @return the limit currently set
	 */
	public int getLimit()
	{
		return _limit;
	}
}
