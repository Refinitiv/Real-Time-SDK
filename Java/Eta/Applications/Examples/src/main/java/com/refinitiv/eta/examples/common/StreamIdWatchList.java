/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.examples.common;

import java.util.Iterator;
import java.util.LinkedHashMap;
import java.util.Map;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.DataStates;
import com.refinitiv.eta.codec.State;
import com.refinitiv.eta.codec.StreamStates;
import com.refinitiv.eta.examples.common.StreamIdWatchList.WatchListEntry;
import com.refinitiv.eta.rdm.DomainTypes;

/**
 * This is a hash map based WatchList for quick lookup of item states when
 * response is received. It is a map of (stream id -&gt; item states). Initial
 * state for a stream when request is sent is unspecified. State is updated with
 * state from status and refresh message.
 */
public class StreamIdWatchList implements Iterable<Map.Entry<StreamIdWatchList.StreamIdKey, WatchListEntry>>
{
    private Map<StreamIdKey, WatchListEntry> watchList = new LinkedHashMap<StreamIdKey, WatchListEntry>();

    private Integer nextStreamId = 5;// stream id for start of market price streams

    // 1 - login, 2- directory, 3-field dictionary, 4 - enum dictionary

    public StreamIdWatchList()
    {

    }

    public Iterator<Map.Entry<StreamIdKey, WatchListEntry>> iterator()
    {
        return watchList.entrySet().iterator();
    }

    public boolean isEmpty()
    {
        return watchList.isEmpty();
    }

    public int noOfItems()
    {
        return watchList.size();
    }

    public int add(int domainType, String itemName, boolean isPrivateStream)
    {
        WatchListEntry wle = new WatchListEntry();
        wle.isPrivateStream = isPrivateStream;
        wle.itemState = CodecFactory.createState();
        wle.itemState.dataState(DataStates.NO_CHANGE);
        wle.itemState.streamState(StreamStates.UNSPECIFIED);
        wle.itemName = itemName;
        wle.domainType = domainType;
        int thisStreamId = nextStreamId++;
        StreamIdKey key = new StreamIdKey();
        key.streamId = thisStreamId;
        watchList.put(key, wle);
        return thisStreamId;
    }

    public WatchListEntry get(int streamId)
    {
        StreamIdKey key = new StreamIdKey();
        key.streamId = streamId;
        return watchList.get(key);
    }

    public void removeAll()
    {
        watchList.clear();
        nextStreamId = 5;
    }
    
    public boolean remove(int streamId)
    {
        StreamIdKey key = new StreamIdKey();
        key.streamId = streamId;
        return watchList.containsKey(key) && watchList.remove(key) != null;
    }

    public int getFirstItem(Buffer mpItemName)
    {
        for (Map.Entry<StreamIdKey, WatchListEntry> entry : watchList.entrySet())
        {
            WatchListEntry wle = entry.getValue();
            State itemState = wle.itemState;
            if (itemState.dataState() == DataStates.OK
                    && itemState.streamState() == StreamStates.OPEN)
            {
                mpItemName.data(wle.itemName);
                return entry.getKey().streamId();
            }
        }

        /* no suitable items were found */
        mpItemName.clear();

        return 0;
    }

    public void clear()
    {
        watchList.clear();
        nextStreamId = 5;
    }

    /**
     * Watch list entry used by {@link StreamIdWatchList}. 
     */
    public static class WatchListEntry
    {
        public boolean isPrivateStream;
        public String itemName;
        public State itemState;
        public int domainType;
        public String toString()
        {
            return "isPrivateStream: " + isPrivateStream +
                    ", itemName: " + itemName + ", itemState: " + itemState +
                    ", domainType:" + DomainTypes.toString(domainType) + "\n";
        }
    }
    
    
    /*
     * Stream id key used by StreamIdWatchList. 
     */
    public static class StreamIdKey
    {
        private int streamId;

        public String toString()
        {
            return "streamId: " + streamId + "\n";
        }

        public int hashCode()
        {
            return streamId;
        }

        public boolean equals(Object obj)
        {
            if (obj.getClass() != this.getClass())
                return false;
            
            StreamIdKey k = (StreamIdKey)obj;
            return (k.streamId == streamId);
        }

        public void clear()
        {
            streamId = 0;
        }

        public int streamId()
        {
            return streamId;
        }

        public void streamId(int streamId)
        {
            this.streamId = streamId;
        }
    }
}
