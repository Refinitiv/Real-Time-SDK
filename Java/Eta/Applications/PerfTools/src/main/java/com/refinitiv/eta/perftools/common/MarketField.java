/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license      --
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
 *|                See the project's LICENSE.md for details.                  --
 *|           Copyright (C) 2019-2022 Refinitiv. All rights reserved.         --
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.eta.perftools.common;

import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.FieldEntry;

/**
 * Represents one field in a fieldList (used by MarketPrice and MarketByOrder
 * messages). Stores the FieldID and dataType of the desired data using a
 * FieldEntry, as well as the value to use.
 */
public class MarketField
{
	private FieldEntry _fieldEntry;    // The market field entry.
	private Object _value;             // The market field value.
	
	/**
	 * Instantiates a new market field.
	 */
	public MarketField()
	{
		_fieldEntry = CodecFactory.createFieldEntry();
	}

	/**
	 *  The market field entry.
	 *
	 * @return the field entry
	 */
	public FieldEntry fieldEntry()
	{
		return _fieldEntry;
	}

	/**
	 *  The market field value.
	 *
	 * @return the object
	 */
	public Object value()
	{
		return _value;
	}

	/**
	 *  The market field value.
	 *
	 * @param value the value
	 */
	public void value(Object value)
	{
		_value = value;
	}
}
