///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.SeriesEntry;

public class SeriesEntryImpl extends EntryImpl implements SeriesEntry
{
	private com.thomsonreuters.upa.codec.SeriesEntry _rsslSeriesEntry;
	
	SeriesEntryImpl(SeriesImpl series, com.thomsonreuters.upa.codec.SeriesEntry rsslSeriesEntry, DataImpl load)
	{
		super(series, load);
		_rsslSeriesEntry = rsslSeriesEntry;
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		
		_toString.append("SeriesEntry dataType=\"").append(DataType.asString(_load.dataType())).append("\"\n");
		_toString.append(_load.toString(1));
		Utilities.addIndent(_toString, 0).append("SeriesEntryEnd\n");

		return _toString.toString();
	}
}