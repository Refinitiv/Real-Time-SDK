/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2023-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

using LSEG.Eta.Codec;
using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using Buffer = LSEG.Eta.Codec.Buffer;

namespace LSEG.Eta.Example.Common
{
    public class StreamIdWatchList : IEnumerable
    {
        private Dictionary<StreamIdKey, WatchListEntry> watchList = new Dictionary<StreamIdKey, WatchListEntry>();

        private int nextStreamId = 5;// stream id for start of market price streams

        // 1 - login, 2- directory, 3-field dictionary, 4 - enum dictionary

        public StreamIdWatchList()
        {

        }

        public bool IsEmpty()
        {
            return watchList.Count == 0;
        }

        public int NoOfItems()
        {
            return watchList.Count;
        }

        public int Add(int domainType, string itemName, bool isPrivateStream)
        {
            WatchListEntry wle = new WatchListEntry();
            wle.IsPrivateStream = isPrivateStream;
            wle.ItemState = new State();
            wle.ItemState.DataState(DataStates.NO_CHANGE);
            wle.ItemState.StreamState(StreamStates.UNSPECIFIED);
            wle.ItemName = itemName;
            wle.DomainType = domainType;
            int thisStreamId = nextStreamId++;
            StreamIdKey key = new StreamIdKey();
            key.StreamId = thisStreamId;
            watchList.Add(key, wle);
            return thisStreamId;
        }

        public WatchListEntry? Get(int streamId)
        {
            StreamIdKey key = new StreamIdKey();
            key.StreamId = streamId;
            watchList.TryGetValue(key, out var wle);
            return wle;
        }

        public void RemoveAll()
        {
            watchList.Clear();
            nextStreamId = 5;
        }

        public bool Remove(int streamId)
        {
            StreamIdKey key = new StreamIdKey();
            key.StreamId = streamId;
            return watchList.ContainsKey(key) && watchList.Remove(key);
        }

        public int GetFirstItem(Buffer mpItemName)
        {
            foreach (var entry in watchList)
            {
                WatchListEntry wle = entry.Value;
                State itemState = wle.ItemState!;
                if (itemState.DataState() == DataStates.OK && itemState.StreamState() == StreamStates.OPEN)
                {
                    mpItemName.Data(wle.ItemName);
                    return entry.Key.StreamId;
                }
            }

            // no suitable items were found 
            mpItemName.Clear();
            return 0;
        }

        public void Clear()
        {
            watchList.Clear();
            nextStreamId = 5;
        }

        public IEnumerator GetEnumerator()
        {
            return watchList.GetEnumerator();
        }
    }
}
