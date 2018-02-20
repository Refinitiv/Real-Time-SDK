package com.thomsonreuters.upa.shared.provider;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.thomsonreuters.upa.shared.ProviderDirectoryHandler;
import com.thomsonreuters.upa.shared.ProviderSession;
import com.thomsonreuters.upa.transport.Channel;

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
