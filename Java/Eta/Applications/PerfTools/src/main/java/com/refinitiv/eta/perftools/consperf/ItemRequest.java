/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.consperf;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.MsgKey;
import com.refinitiv.eta.perftools.common.ItemInfo;

/** Item request information. */
public class ItemRequest
{
	private int _requestState;	/* Item state. */
	private String _itemName;	/* Storage for the item's name. */
	private ItemInfo _itemInfo;	/* Structure containing item information. */
	private MsgKey _msgKey;		/* Message key used to request this item. */
	
	{
		_msgKey = CodecFactory.createMsgKey();
		_itemInfo = new ItemInfo();
	}
	
	/** Clears an ItemRequest. */
	public void clear()
	{
		_msgKey.clear();
		_itemInfo.clear();
		_requestState = ItemRequestState.NOT_REQUESTED;
		_itemInfo.attributes().msgKey(_msgKey);
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

	/**
	 *  Message key used to request this item.
	 *
	 * @return the msg key
	 */
	public MsgKey msgKey()
	{
		return _msgKey;
	}

	/**
	 *  Message key used to request this item.
	 *
	 * @param msgKey the msg key
	 */
	public void msgKey(MsgKey msgKey)
	{
		_msgKey = msgKey;
	}
}
