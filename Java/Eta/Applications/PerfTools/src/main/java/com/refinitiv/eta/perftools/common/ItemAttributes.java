/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import com.refinitiv.eta.codec.MsgKey;

/** Class containing the attributes that uniquely identify an item. */
public class ItemAttributes
{
	private MsgKey _msgKey;    // Key for this item
	private int _domainType;   // Domain of this item
	
    /**
     *  Key for this item.
     *
     * @return the msg key
     */
	public MsgKey msgKey()
	{
		return _msgKey;
	}
	
    /**
     *  Key for this item.
     *
     * @param msgKey the msg key
     */
	public void msgKey(MsgKey msgKey)
	{
		_msgKey = msgKey;
	}
	
    /**
     *  Domain of this item.
     *
     * @return the int
     */
	public int domainType()
	{
		return _domainType;
	}
	
    /**
     *  Domain of this item.
     *
     * @param domainType the domain type
     */
	public void domainType(int domainType)
	{
		_domainType = domainType;
	}
}
