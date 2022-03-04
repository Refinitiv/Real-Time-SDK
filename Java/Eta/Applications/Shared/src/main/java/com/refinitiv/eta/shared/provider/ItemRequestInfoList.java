/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.provider;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.refinitiv.eta.shared.ProviderDirectoryHandler;
import com.refinitiv.eta.shared.ProviderSession;
import com.refinitiv.eta.transport.Channel;

/**
 * Item request information list.
 */
public class ItemRequestInfoList implements Iterable<ItemRequestInfo>
{
    private static final int MAX_ITEM_REQ_INFO_LIST_SIZE = ProviderDirectoryHandler.OPEN_LIMIT * ProviderSession.NUM_CLIENT_SESSIONS;
    private List<ItemRequestInfo> _itemRequestInfoList = new ArrayList<ItemRequestInfo>(MAX_ITEM_REQ_INFO_LIST_SIZE);

    /**
     * Instantiates a new item request info list.
     */
    public ItemRequestInfoList()
    {
        for (int i = 0; i < MAX_ITEM_REQ_INFO_LIST_SIZE; i++)
        {
            _itemRequestInfoList.add(new ItemRequestInfo());
        }
    }

    /**
     * finds an item request info associated with a channel and stream.
     *
     * @param chnl the chnl
     * @param streamId the stream id
     * @return the item request info
     */
    public ItemRequestInfo get(Channel chnl, int streamId)
    {
        for (ItemRequestInfo itemReqInfo : _itemRequestInfoList)
        {
            if (itemReqInfo.channel == chnl && itemReqInfo.streamId == streamId && itemReqInfo.isInUse)
                return itemReqInfo;
        }
        return null;
    }

    /**
     * Initializes item request information list.
     */
    public void init()
    {
        for (ItemRequestInfo itemRequestInfo : _itemRequestInfoList)
        {
            itemRequestInfo.clear();
        }
    }

    /* (non-Javadoc)
     * @see java.lang.Iterable#iterator()
     */
    @Override
    public Iterator<ItemRequestInfo> iterator()
    {
        return _itemRequestInfoList.iterator();
    }
}
