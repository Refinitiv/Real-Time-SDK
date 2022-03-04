/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.valueadd.examples.niprovider;

import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import com.refinitiv.eta.valueadd.cache.PayloadEntry;
import com.refinitiv.eta.valueadd.examples.niprovider.StreamIdWatchList.WatchListEntry;
import com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderItem;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceItem;
import com.refinitiv.eta.rdm.DomainTypes;

/*
 * This is a hash map based WatchList for quick lookup of items and their
 * associated streamId and domainType when sending updates or reissuing
 * refreshes. It is a map of (streamId -> item info).
 */
class StreamIdWatchList implements Iterable<Map.Entry<Integer, WatchListEntry>>
{
    private Map<Integer, WatchListEntry> watchList = new LinkedHashMap<Integer, WatchListEntry>();

    // starting stream id for marketPrice / marketByOrder streams
    // 1 - login, -1 - directory
    private Integer nextStreamId = -2;

    public Iterator<Map.Entry<Integer, WatchListEntry>> iterator()
    {
        return watchList.entrySet().iterator();
    }

    int add(int domainType, String itemName)
    {
        // check if item is already in list before adding
        Set<Entry<Integer, WatchListEntry>> wleSet = watchList.entrySet();
        for (Map.Entry<Integer, WatchListEntry> entry : wleSet)
        {
            if (entry.getValue().itemName.equals(itemName) && 
            		entry.getValue().domainType == domainType)
            {
                // return stream id for matching item
                return entry.getKey().intValue();
            }
        }

        // add new entry
        WatchListEntry wle = new WatchListEntry();
        wle.itemName = itemName;
        wle.domainType = domainType;

        if (domainType == DomainTypes.MARKET_PRICE)
        {
            wle.marketPriceItem = new MarketPriceItem();
            wle.marketPriceItem.initFields();
        }
        else if (domainType == DomainTypes.MARKET_BY_ORDER)
        {
            wle.marketByOrderItem = new MarketByOrderItem();
            wle.marketByOrderItem.initFields();
        }

        int thisStreamId = nextStreamId--;
        watchList.put(thisStreamId, wle);
        return thisStreamId;
    }

    WatchListEntry get(int streamId)
    {
        return watchList.get(streamId);
    }

    void clear()
    {
        watchList.clear();
    }

    /*
     * Watch list entry used by {@link StreamIdWatchList}.
     */
    static class WatchListEntry
    {
        public String itemName;
        public int domainType;
        public MarketPriceItem marketPriceItem;
        public MarketByOrderItem marketByOrderItem;
        public PayloadEntry cacheEntry;

        public String toString()
        {
            return "itemName: "
                    + itemName
                    + ", itemInfo:\n"
                    + ((domainType == DomainTypes.MARKET_PRICE) ? marketPriceItem
                            : marketByOrderItem);
        }
    }
}
