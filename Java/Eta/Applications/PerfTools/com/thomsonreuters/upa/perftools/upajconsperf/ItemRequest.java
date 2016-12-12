package com.thomsonreuters.upa.perftools.upajconsperf;

import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.MsgKey;
import com.thomsonreuters.upa.perftools.common.ItemInfo;

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

	/** Item state. */
	public int requestState()
	{
		return _requestState;
	}

	/** Item state. */
	public void requestState(int requestState)
	{
		_requestState = requestState;
	}

	/** Storage for the item's name. */
	public String itemName()
	{
		return _itemName;
	}

	/** Storage for the item's name. */
	public void itemName(String itemName)
	{
		_itemName = itemName;
	}

	/** Structure containing item information. */
	public ItemInfo itemInfo()
	{
		return _itemInfo;
	}

	/** Structure containing item information. */
	public void itemInfo(ItemInfo itemInfo)
	{
		_itemInfo = itemInfo;
	}

	/** Message key used to request this item. */
	public MsgKey msgKey()
	{
		return _msgKey;
	}

	/** Message key used to request this item. */
	public void msgKey(MsgKey msgKey)
	{
		_msgKey = msgKey;
	}
}
