///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.SeriesEntry;
import com.thomsonreuters.upa.codec.CodecFactory;

class SeriesEntryImpl extends EntryImpl implements SeriesEntry
{
	private com.thomsonreuters.upa.codec.SeriesEntry _rsslSeriesEntry;
	
	SeriesEntryImpl() 
	{
		_rsslSeriesEntry = CodecFactory.createSeriesEntry();
	}
	
	SeriesEntryImpl(com.thomsonreuters.upa.codec.SeriesEntry rsslSeriesEntry, DataImpl load)
	{
		super(load);
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

	@Override
	public SeriesEntry reqMsg(ReqMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry refreshMsg(RefreshMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry statusMsg(StatusMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry updateMsg(UpdateMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry postMsg(PostMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry ackMsg(AckMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry genericMsg(GenericMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry fieldList(FieldList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry elementList(ElementList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry map(Map value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry vector(Vector value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry series(Series value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry filterList(FilterList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry opaque(OmmOpaque value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry xml(OmmXml value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public SeriesEntry ansiPage(OmmAnsiPage value)
	{
		// TODO Auto-generated method stub
		return null;
	}
}