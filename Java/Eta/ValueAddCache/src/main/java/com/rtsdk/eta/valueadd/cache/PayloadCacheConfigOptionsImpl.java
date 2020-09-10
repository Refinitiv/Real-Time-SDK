package com.rtsdk.eta.valueadd.cache;

import com.rtsdk.eta.valueadd.cache.PayloadCacheConfigOptions;

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
