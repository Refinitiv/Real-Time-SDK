/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2022 Refinitiv. All rights reserved.         	  --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.perftools.common;

/** Contains information about an item in the item list. */
public class XmlItemInfo
{
	private int			_domainType;	/* Domain Type of the item */
	private String		_name;			/* Name of the item */
	private boolean		_isPost;		/* Whether the item is to be used for consumer posting. */
	private boolean		_isGenMsg;		/* Whether the item is to be used for consumer to send generic msgs. */
	private boolean		_isSnapshot;	/* Whether the item is to be used to carry latency information. */

	/**
	 *  Domain Type of the item.
	 *
	 * @return the int
	 */
	public int domainType()
	{
		return _domainType;
	}
	
	/**
	 *  Domain Type of the item.
	 *
	 * @param domainType the domain type
	 */
	public void domainType(int domainType)
	{
		_domainType = domainType;
	}
	
	/**
	 *  Name of the item.
	 *
	 * @return the string
	 */
	public String name()
	{
		return _name;
	}
	
	/**
	 *  Name of the item.
	 *
	 * @param name the name
	 */
	public void name(String name)
	{
		_name = name;
	}
	
	/**
	 *  Whether the item is to be used for consumer posting.
	 *
	 * @return true, if is post
	 */
	public boolean isPost()
	{
		return _isPost;
	}
	
	/**
	 *  Whether the item is to be used for consumer posting.
	 *
	 * @param isPost the is post
	 */
	public void isPost(boolean isPost)
	{
		_isPost = isPost;
	}
	
	/**
	 *  Whether the item is to be used for consumer to send generic msgs.
	 *
	 * @return true, if is gen msg
	 */
	public boolean isGenMsg()
	{
		return _isGenMsg;
	}
	
	/**
	 *  Whether the item is to be used for consumer to send generic msgs.
	 *
	 * @param isGenMsg the is gen msg
	 */
	public void isGenMsg(boolean isGenMsg)
	{
		_isGenMsg = isGenMsg;
	}
	
	/**
	 *  Whether the item is to be used to carry latency information.
	 *
	 * @return true, if is snapshot
	 */
	public boolean isSnapshot()
	{
		return _isSnapshot;
	}
	
	/**
	 *  Whether the item is to be used to carry latency information.
	 *
	 * @param isSnapshot the is snapshot
	 */
	public void isSnapshot(boolean isSnapshot)
	{
		_isSnapshot = isSnapshot;
	}
}
