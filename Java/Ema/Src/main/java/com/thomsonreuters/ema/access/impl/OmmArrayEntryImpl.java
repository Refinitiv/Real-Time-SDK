///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;


import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.OmmArrayEntry;


public class OmmArrayEntryImpl extends EntryImpl implements OmmArrayEntry
{
	OmmArrayEntryImpl(OmmArrayImpl array, DataImpl load)
	{
		super(array, load);
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("OmmArrayEntry ")
				 .append(" dataType=\"").append(DataType.asString(_load.dataType())).append("\"")
				 .append(" value=\"").append(_load.toString()).append("\"\n");

		return _toString.toString();
	}
}