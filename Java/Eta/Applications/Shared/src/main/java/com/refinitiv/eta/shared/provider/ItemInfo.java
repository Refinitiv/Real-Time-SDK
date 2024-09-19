/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.shared.provider;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.shared.rdm.marketbyorder.MarketByOrderItem;
import com.refinitiv.eta.shared.rdm.marketbyprice.MarketByPriceItem;
import com.refinitiv.eta.shared.rdm.marketprice.MarketPriceItem;
import com.refinitiv.eta.rdm.DomainTypes;


/**
 * Item information.
 */
public class ItemInfo
{
    public boolean isRefreshRequired;
    public Buffer itemName;
    public int interestCount;
    public int domainType;
    boolean isPrivateStream;
    
    /*
     * Holds information about the item's data. This data will be different
     * depending on the domain of the item.
     */
    public Object itemData;
    
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
            default:
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
