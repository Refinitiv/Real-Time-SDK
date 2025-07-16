/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022,2024-2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.common;

/**
 * Value Add Limited Pool class. Extends {@link VaPool} by providing the limit to the pool size.
 * @see VaPool
 */
public class LimitedVaPool extends VaPool
{
	private int limit = -1;

	/**
	 * Creates a pool. This pool is not thread safe. This pool is not limited by default.
	 * 
	 * @see VaPool
	 */
	public LimitedVaPool()
	{
	}

	/**
	 * Creates a pool. Param useConcurrent can be used to make this pool thread safe.
	 * @param useConcurrent if true, the pool is backed by a {@link VaConcurrentQueue}.
	 * @see VaPool
	 */
	public LimitedVaPool(boolean useConcurrent)
	{
		super(useConcurrent);
	}

	/**
	 * Creates a pool. Param useConcurrent can be used to make this pool thread safe.
	 * @param useConcurrent to set concurrent pool
	 * @param debug to set debug mode
	 * @see VaPool
	 */
	public LimitedVaPool(boolean useConcurrent, boolean debug)
	{
		super(useConcurrent, debug);
	}

	@Override
	/**
	 * Adds a node to the pool. If pool limit is set and current pool size exceeds
	 * the limit, the node will not be added to the pool.
	 * 
	 * @param node the node to add
	 * @see VaPool
	 */
	public void add(VaNode node)
	{
		if(checkPoolLimit(limit))
		{
			super.add(node);
		}
	}

	/**
	 * Sets the maximum size of the limited pool.
	 * 
	 * @param limit the maximum size of the limited pool
	 */
	public void setLimit(int limit)
	{
		this.limit = limit;
	}

	private boolean checkPoolLimit(int limit)
	{
		return limit < 0 || size() < limit;
	}
}
