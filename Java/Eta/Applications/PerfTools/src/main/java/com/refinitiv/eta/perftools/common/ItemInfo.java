/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import com.refinitiv.eta.perftools.common.ItemAttributes;

/**
 * Contains information about a particular item being published.
 */
public class ItemInfo
{
	private int				_streamId;		// Item's Stream ID
	private Object			_itemData; 		// Holds information about the item's data. This data will be different depending on the domain of the item.
	private int				_itemFlags;		// See ItemFlags struct
	private ItemAttributes	_attributes;	// Attributes that uniquely identify this item
	
	/**
	 * Instantiates a new item info.
	 */
	public ItemInfo()
	{
		_attributes = new ItemAttributes();
	}
	
    /** Clears the item information. */
	public void clear()
	{
		_itemFlags = 0;
		_streamId = 0;
		_attributes.msgKey(null);
		_itemData = null;
		_attributes.domainType(0);
	}

    /**
     *  Item's Stream ID.
     *
     * @return the int
     */
	public int streamId()
	{
		return _streamId;
	}

	/**
	 *  Item's Stream ID.
	 *
	 * @param streamId the stream id
	 */
	public void streamId(int streamId)
	{
		_streamId = streamId;
	}

    /**
     * Holds information about the item's data. This data will be different
     * depending on the domain of the item.
     *
     * @return the object
     */
	public Object itemData()
	{
		return _itemData;
	}

    /**
     * Holds information about the item's data. This data will be different
     * depending on the domain of the item.
     *
     * @param itemData the item data
     */
	public void itemData(Object itemData)
	{
		_itemData = itemData;
	}

	/**
	 * Item flags.
	 *
	 * @return {@link ItemFlags}
	 */
    public int itemFlags()
	{
		return _itemFlags;
	}

    /**
     * Item flags.
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