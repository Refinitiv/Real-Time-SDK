package com.thomsonreuters.upa.examples.niprovider;

import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import com.thomsonreuters.upa.examples.niprovider.StreamIdWatchList.WatchListEntry;
import com.thomsonreuters.upa.examples.rdm.marketbyorder.MarketByOrderItem;
import com.thomsonreuters.upa.examples.rdm.marketprice.MarketPriceItem;
import com.thomsonreuters.upa.rdm.DomainTypes;

/**
 * This is a hash map based WatchList for quick lookup of items and their
 * associated streamId and domainType when sending updates or reissuing
 * refreshes. It is a map of (streamId -> item info).
 */
public class StreamIdWatchList implements Iterable<Map.Entry<Integer, WatchListEntry>>
{
    private Map<Integer, WatchListEntry> watchList = new LinkedHashMap<Integer, WatchListEntry>();

    // starting stream id for marketPrice / marketByOrder streams
    // 1 - login, -1 - directory
    private Integer nextStreamId = -2;

    public Iterator<Map.Entry<Integer, WatchListEntry>> iterator()
    {
        return watchList.entrySet().iterator();
    }

    public int add(int domainType, String itemName)
    {
        // check if item is already in list before adding
        Set<Entry<Integer, WatchListEntry>> wleSet = watchList.entrySet();
        for (Map.Entry<Integer, WatchListEntry> entry : wleSet)
        {
            if (entry.getValue().itemName.equals(itemName))
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

    public WatchListEntry get(int streamId)
    {
        return watchList.get(streamId);
    }

    public void clear()
    {
        watchList.clear();
    }

    /**
     * Watch list entry used by {@link StreamIdWatchList}.
     */
    public static class WatchListEntry
    {
        public String itemName;
        public int domainType;
        public MarketPriceItem marketPriceItem;
        public MarketByOrderItem marketByOrderItem;

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
