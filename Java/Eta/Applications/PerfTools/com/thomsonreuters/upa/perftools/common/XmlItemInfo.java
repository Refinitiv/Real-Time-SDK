package com.thomsonreuters.upa.perftools.common;

/** Contains information about an item in the item list. */
public class XmlItemInfo
{
	private int			_domainType;	/* Domain Type of the item */
	private String		_name;			/* Name of the item */
	private boolean		_isPost;		/* Whether the item is to be used for consumer posting. */
	private boolean		_isGenMsg;		/* Whether the item is to be used for consumer to send generic msgs. */
	private boolean		_isSnapshot;	/* Whether the item is to be used to carry latency information. */

	/** Domain Type of the item */
	public int domainType()
	{
		return _domainType;
	}
	
	/** Domain Type of the item */
	public void domainType(int domainType)
	{
		_domainType = domainType;
	}
	
	/** Name of the item */
	public String name()
	{
		return _name;
	}
	
	/** Name of the item */
	public void name(String name)
	{
		_name = name;
	}
	
	/** Whether the item is to be used for consumer posting. */
	public boolean isPost()
	{
		return _isPost;
	}
	
	/** Whether the item is to be used for consumer posting. */
	public void isPost(boolean isPost)
	{
		_isPost = isPost;
	}
	
	/** Whether the item is to be used for consumer to send generic msgs. */
	public boolean isGenMsg()
	{
		return _isGenMsg;
	}
	
	/** Whether the item is to be used for consumer to send generic msgs. */
	public void isGenMsg(boolean isGenMsg)
	{
		_isGenMsg = isGenMsg;
	}
	
	/** Whether the item is to be used to carry latency information. */
	public boolean isSnapshot()
	{
		return _isSnapshot;
	}
	
	/** Whether the item is to be used to carry latency information. */
	public void isSnapshot(boolean isSnapshot)
	{
		_isSnapshot = isSnapshot;
	}
}
