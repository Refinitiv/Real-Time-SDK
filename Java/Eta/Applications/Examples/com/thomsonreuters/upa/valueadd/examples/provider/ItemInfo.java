package com.thomsonreuters.upa.valueadd.examples.provider;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.valueadd.examples.common.MarketByOrderItem;
import com.thomsonreuters.upa.valueadd.examples.common.MarketByPriceItem;
import com.thomsonreuters.upa.valueadd.examples.common.MarketPriceItem;
import com.thomsonreuters.upa.rdm.DomainTypes;


/**
 * Item information.
 */
public class ItemInfo
{
    boolean isRefreshRequired;
    Buffer itemName;
    int interestCount;
    int domainType;
    boolean isPrivateStream;
    /*
     * Holds information about the item's data. This data will be different
     * depending on the domain of the item.
     */
    Object itemData;
    
    public ItemInfo()
    {
        isRefreshRequired = true;
        itemName = CodecFactory.createBuffer();
        interestCount = 0;
        domainType = 0;
        isPrivateStream = false;
    }
    
    /**
     * clears item information.
     */
    public void clear()
    {
        switch (domainType)
        {
            case DomainTypes.MARKET_PRICE:
                ((MarketPriceItem)itemData).clear();
                break;
            case DomainTypes.MARKET_BY_ORDER:
                ((MarketByOrderItem)itemData).clear();
                break;
            case DomainTypes.MARKET_BY_PRICE:
                ((MarketByPriceItem)itemData).clear();
                 break;
            case DomainTypes.SYMBOL_LIST:
                break;
        }
        isRefreshRequired = true;
        itemName.clear();
        interestCount = 0;
        domainType = 0;
        isPrivateStream = false;
    }

    public boolean isRefreshRequired()
    {
    	return isRefreshRequired;
    }
    
    public void isRefreshRequired(boolean isRefreshRequired)
    {
    	this.isRefreshRequired = isRefreshRequired;
    }
    
    public Buffer itemName()
    {
    	return itemName;
    }
    
    public int interestCount()
    {
    	return interestCount;
    }
    
    public void interestCount(int interestCount)
    {
    	this.interestCount = interestCount;
    }
    
    public int domainType()
    {
    	return domainType;
    }
    
    public Object itemData()
    {
    	return itemData;
    }

    public void itemData(Object itemData)
    {
    	this.itemData = itemData;
    }
    
    public boolean isPrivateStream()
    {
    	return isPrivateStream;
    }
    
    public void isPrivateStream(boolean isPrivateStream)
    {
    	this.isPrivateStream = isPrivateStream;
    }
}
