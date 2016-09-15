package com.thomsonreuters.ema.perftools.common;

import com.thomsonreuters.ema.perftools.common.ItemAttributes;

/**
 * Contains information about a particular item being published.
 */
public class ItemInfo
{
	private int				_itemId;		// Item's Stream ID
	private Object			_itemData; 		// Holds information about the item's data. This data will be different depending on the domain of the item.
	private int				_itemFlags;		// See ItemFlags struct
	private ItemAttributes	_attributes;	// Attributes that uniquely identify this item
	
	public ItemInfo()
	{
		_attributes = new ItemAttributes();
	}
	
    /** Clears the item information. */
	public void clear()
	{
		_itemFlags = 0;
		_itemId = 0;
		_itemData = null;
		_attributes.domainType(0);
	}

    /** Item's Stream ID */
	public int itemId()
	{
		return _itemId;
	}

	public void itemId(int itemId)
	{
		_itemId = itemId;
	}

    /**
     * Holds information about the item's data. This data will be different
     * depending on the domain of the item.
     */
	public Object itemData()
	{
		return _itemData;
	}

    /**
     * Holds information about the item's data. This data will be different
     * depending on the domain of the item.
     */
	public void itemData(Object itemData)
	{
		_itemData = itemData;
	}

	/**
	 * 
	 * @return {@link ItemFlags}
	 */
    public int itemFlags()
	{
		return _itemFlags;
	}

    /**
     * 
     * @param itemFlags - {@link ItemFlags}
     */
	public void itemFlags(int itemFlags)
	{
		_itemFlags = itemFlags;
	}

	/**
	 * Attributes that uniquely identify this item.
	 * @return {@link ItemAttributes}
	 */
	public ItemAttributes attributes()
	{
		return _attributes;
	}

	/**
	 * Attributes that uniquely identify this item.
	 *  
	 * @param attributes {@link ItemAttributes}
	 */
	public void attributes(ItemAttributes attributes)
	{
		_attributes = attributes;
	}
}