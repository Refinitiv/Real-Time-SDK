///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.ElementEntry;
import com.thomsonreuters.ema.access.DataType.DataTypes;

public class ElementEntryImpl extends EntryImpl implements ElementEntry
{
	private com.thomsonreuters.upa.codec.ElementEntry	_rsslElementEntry;
	
	ElementEntryImpl(ElementListImpl elementList, com.thomsonreuters.upa.codec.ElementEntry rsslElementEntry, DataImpl load)
	{
		super(elementList, load);
		_rsslElementEntry = rsslElementEntry;
	}
	
	@Override
	public String name()
	{
		return _rsslElementEntry.name().toString();
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("ElementEntry ")
				.append(" name=\"").append(name()).append("\"")
				.append(" dataType=\"").append(DataType.asString(_load.dataType()));

		if (_load.dataType() <= DataTypes.ARRAY)
		{
			_toString.append("\"\n").append(_load.toString(1));
			Utilities.addIndent(_toString, 0).append("ElementEntryEnd\n");
		}
		else
			_toString.append("\" value=\"").append(_load.toString()).append("\"\n");
	
		return _toString.toString();
	}
}