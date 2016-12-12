package com.thomsonreuters.upa.perftools.common;

import com.thomsonreuters.upa.codec.MsgKey;

/** Class containing the attributes that uniquely identify an item. */
public class ItemAttributes
{
	private MsgKey _msgKey;    // Key for this item
	private int _domainType;   // Domain of this item
	
    /** Key for this item */
	public MsgKey msgKey()
	{
		return _msgKey;
	}
	
    /** Key for this item */
	public void msgKey(MsgKey msgKey)
	{
		_msgKey = msgKey;
	}
	
    /** Domain of this item */
	public int domainType()
	{
		return _domainType;
	}
	
    /** Domain of this item */
	public void domainType(int domainType)
	{
		_domainType = domainType;
	}
}
