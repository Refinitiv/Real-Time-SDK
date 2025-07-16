/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.transport;

/**
 * Limited Pool class. Extends {@link Pool} by providing the limit to the pool size.
 * @see Pool
 */
class LimitedPool extends Pool
{
    private int limit = -1;

    /**
     * Creates a pool. This pool is not thread safe. This pool is not limited by default.
     * 
     * @param o the object to provide as an owner of the pool
     * @see Pool
     */
    public LimitedPool(Object o)
    {
        super(o);
    }

    @Override
    /**
     * Adds a node to the pool. If pool limit is set and current pool size exceeds
     * the limit, the node will not be added to the pool.
     * 
     * @param node the node to add
     * @see Pool
     */
    void add(EtaNode node)
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
