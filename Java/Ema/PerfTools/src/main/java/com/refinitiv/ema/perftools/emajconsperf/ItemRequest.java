package com.refinitiv.ema.perftools.emajconsperf;

import com.refinitiv.ema.perftools.common.ItemInfo;

/** Item request information. */
public class ItemRequest
{
	private int _requestState;	/* Item state. */
	private String _itemName;	/* Storage for the item's name. */
	private ItemInfo _itemInfo;	/* Structure containing item information. */
	
	{
		_itemInfo = new ItemInfo();
	}
	
	/** Clears an ItemRequest. */
	public void clear()
	{
		_itemInfo.clear();
		_requestState = ItemRequestState.NOT_REQUESTED;
	}

	/**
	 *  Item state.
	 *
	 * @return the int
	 */
	public int requestState()
	{
		return _requestState;
	}

	/**
	 *  Item state.
	 *
	 * @param requestState the request state
	 */
	public void requestState(int requestState)
	{
		_requestState = requestState;
	}

	/**
	 *  Storage for the item's name.
	 *
	 * @return the string
	 */
	public String itemName()
	{
		return _itemName;
	}

	/**
	 *  Storage for the item's name.
	 *
	 * @param itemName the item name
	 */
	public void itemName(String itemName)
	{
		_itemName = itemName;
	}

	/**
	 *  Structure containing item information.
	 *
	 * @return the item info
	 */
	public ItemInfo itemInfo()
	{
		return _itemInfo;
	}

	/**
	 *  Structure containing item information.
	 *
	 * @param itemInfo the item info
	 */
	public void itemInfo(ItemInfo itemInfo)
	{
		_itemInfo = itemInfo;
	}
}
