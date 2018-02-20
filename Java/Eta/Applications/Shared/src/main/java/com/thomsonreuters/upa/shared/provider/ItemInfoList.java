package com.thomsonreuters.upa.shared.provider;

import java.nio.ByteBuffer;
import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.shared.ProviderDirectoryHandler;
import com.thomsonreuters.upa.shared.ProviderSession;
import com.thomsonreuters.upa.shared.rdm.marketbyorder.MarketByOrderItem;
import com.thomsonreuters.upa.shared.rdm.marketbyprice.MarketByPriceItem;
import com.thomsonreuters.upa.shared.rdm.marketprice.MarketPriceItem;
import com.thomsonreuters.upa.rdm.DomainTypes;
import com.thomsonreuters.upa.transport.Channel;

/**
 * Item Information list.
 * 
 * It uses a simple arraylist based watch list for its implementation and is
 * limited to OPEN_LIMIT items per channel.
 */
public class ItemInfoList implements Iterable<ItemInfo>
{
    private static final int MAX_ITEM_LIST_SIZE = ProviderDirectoryHandler.OPEN_LIMIT * ProviderSession.NUM_CLIENT_SESSIONS;
    private List<ItemInfo> _itemInfoList = new ArrayList<ItemInfo>(MAX_ITEM_LIST_SIZE);

    /**
     * Instantiates a new item info list.
     */
    public ItemInfoList()
    {
        for (int i = 0; i < MAX_ITEM_LIST_SIZE; i++)
        {
            _itemInfoList.add(new ItemInfo());
        }
    }

    /**
     * Retrieves an item info given the item name and domain.
     *
     * @param name - Itemname to search iteminfo for
     * @param domainType - domain type to search iteminfo for
     * @param isPrivateStream the is private stream
     * @return ItemInfo associated with itemName and domainType
     */
    public ItemInfo get(Buffer name, int domainType, boolean isPrivateStream)
    {
        for (ItemInfo itemInfo : _itemInfoList)
        {

            if (itemInfo.domainType == domainType && name.equals(itemInfo.itemName) 
            	&& itemInfo.isPrivateStream() == isPrivateStream)
                return itemInfo;
        }

        return null;
    }

    /**
     * Finds an item information associated with the item request.
     *
     * @param channel the channel
     * @param itemName - Itemname to search iteminfo for
     * @param domainType - domain type to search iteminfo for
     * @param isPrivateStream the is private stream
     * @return ItemInfo associated with itemName and domainType
     */
    public ItemInfo get(Channel channel, Buffer itemName, int domainType, boolean isPrivateStream)
    {
        /* first check for existing item */
        ItemInfo itemInfo = get(itemName, domainType, isPrivateStream);

        /* if no existing item, get new one */
        if (itemInfo == null)
        {
            for (ItemInfo tmpItemInfo : _itemInfoList)
            {
                if (tmpItemInfo.interestCount == 0)
                {
                    // copy item name buffer
                    ByteBuffer byteBuffer = ByteBuffer.allocate(itemName.length());
                    itemName.copy(byteBuffer);
                    tmpItemInfo.itemName.data(byteBuffer);
                    tmpItemInfo.domainType = domainType;
                    tmpItemInfo.isPrivateStream = isPrivateStream;
                    itemInfo = tmpItemInfo;
                    break;
                }
            }
        }

        return itemInfo;
    }

    /**
     * Updates any item information that's currently in use.
     */
    public void update()
    {
        for (ItemInfo itemInfo : _itemInfoList)
        {
            if (itemInfo.interestCount > 0)
            {
                switch (itemInfo.domainType)
                {
                    case DomainTypes.MARKET_PRICE:
                        ((MarketPriceItem)itemInfo.itemData).updateFields();
                        break;
                    case DomainTypes.MARKET_BY_ORDER:
                        ((MarketByOrderItem)itemInfo.itemData).updateFields();
                        break;
                    case DomainTypes.MARKET_BY_PRICE:
                        ((MarketByPriceItem)itemInfo.itemData).updateFields();
                        break;
                    case DomainTypes.SYMBOL_LIST:
                        break;
                    default:
                        System.out.println("Unknown domain\n");
                }
            }
        }
    }

    /* (non-Javadoc)
     * @see java.lang.Iterable#iterator()
     */
    @Override
    public Iterator<ItemInfo> iterator()
    {
        return _itemInfoList.iterator();
    }

    /**
     * Initializes item information list.
     */
    public void init()
    {
        for (ItemInfo itemInfo : _itemInfoList)
        {
            itemInfo.clear();
        }
    }

    /**
     * clears item information list.
     */
    public void clear()
    {
        for (ItemInfo itemInfo : _itemInfoList)
        {
            itemInfo.clear();
        }
    }
}
