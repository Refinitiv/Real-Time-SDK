package com.thomsonreuters.upa.valueadd.cache;

import com.thomsonreuters.upa.valueadd.cache.PayloadCacheConfigOptions;

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
