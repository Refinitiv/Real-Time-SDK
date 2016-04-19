///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


class SeriesEntryImpl extends EntryImpl implements SeriesEntry
{
	protected com.thomsonreuters.upa.codec.SeriesEntry _rsslSeriesEntry;
	protected int _entryDataType;
	
	SeriesEntryImpl() 
	{
		_rsslSeriesEntry = com.thomsonreuters.upa.codec.CodecFactory.createSeriesEntry();
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
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry refreshMsg(RefreshMsg value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry statusMsg(StatusMsg value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry updateMsg(UpdateMsg value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry postMsg(PostMsg value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry ackMsg(AckMsg value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry genericMsg(GenericMsg value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry fieldList(FieldList value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry elementList(ElementList value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry map(Map value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry vector(Vector value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry series(Series value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry filterList(FilterList value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry opaque(OmmOpaque value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry xml(OmmXml value)
	{
		return entryValue((DataImpl) value);
	}

	@Override
	public SeriesEntry ansiPage(OmmAnsiPage value)
	{
		return entryValue((DataImpl) value);
	}

	private SeriesEntry entryValue(DataImpl value)
	{
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");

		_entryDataType = Utilities.toRsslDataType(value.dataType());	
		Utilities.copy(((DataImpl) value).encodedData(), _rsslSeriesEntry.encodedData());
		
		return this;
	}
}