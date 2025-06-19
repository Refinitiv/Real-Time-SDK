/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2025 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.cache;

import com.refinitiv.eta.valueadd.cache.PayloadCacheConfigOptions;

/* Cache configuration class */
class PayloadCacheConfigOptionsImpl implements PayloadCacheConfigOptions
{
    private int _maxItems;

    @Override
    public int maxItems()
    {
        return _maxItems;
    }

    @Override
    public void maxItems(int maxItems)
    {
        _maxItems = maxItems;
    }

    public void clear()
    {
        _maxItems = 0;
    }

}
