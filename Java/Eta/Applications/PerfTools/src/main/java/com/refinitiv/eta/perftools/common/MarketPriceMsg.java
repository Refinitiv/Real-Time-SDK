/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2019-2022 LSEG. All rights reserved.     
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

/**
 *  Represents one MarketPrice message.
 */
public class MarketPriceMsg
{	
	private MarketField[] _fieldEntries;   // List of fields.
	private int _fieldEntryCount;          // Number of fields in list.
	private int _estimatedContentLength;   // Estimated size of payload.
	private int _arrayCount;               // count for field entry array 
		
	/**
	 * Instantiates a new market price msg.
	 *
	 * @param count the count
	 */
	public MarketPriceMsg(int count)
	{
		_arrayCount = count;
		_fieldEntries = new MarketField[_arrayCount];
	}

	/**
	 *  List of fields.
	 *
	 * @return the market field[]
	 */
	public MarketField[] fieldEntries()
	{
		return _fieldEntries;
	}

	/**
	 *  Number of fields in list.
	 *
	 * @return the int
	 */
	public int fieldEntryCount()
	{
		return _fieldEntryCount;
	}

	/**
	 *  Number of fields in list.
	 *
	 * @param count the count
	 */
	public void fieldEntryCount(int count)
	{
		_fieldEntryCount = count;
	}

	/**
	 *  Estimated size of payload.
	 *
	 * @return the int
	 */
	public int estimatedContentLength()
	{
		return _estimatedContentLength;
	}

	/**
	 *  Estimated size of payload.
	 *
	 * @param length the length
	 */
	public void estimatedContentLength(int length)
	{
		_estimatedContentLength = length;
	}
}
