///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;

class SeriesImpl extends CollectionDataImpl implements Series
{
	private com.thomsonreuters.upa.codec.Series	_rsslSeries = com.thomsonreuters.upa.codec.CodecFactory.createSeries();
	private LinkedList<SeriesEntry> _seriesCollection = new LinkedList<SeriesEntry>(); 
	private DataImpl _summaryDecoded = noDataInstance();
	private PayloadAttribSummaryImpl _summaryData;
	
	SeriesImpl() 
	{
		super(false);
	}
	
	SeriesImpl(boolean decoding)
	{
		super(decoding);
	} 

	@Override
	public int dataType()
	{
		return DataTypes.SERIES;
	}

	@Override
	public boolean hasTotalCountHint()
	{
		return _rsslSeries.checkHasTotalCountHint();
	}

	@Override
	public int totalCountHint()
	{
		if (!hasTotalCountHint())
			throw ommIUExcept().message("Attempt to totalCountHint() while it is not set.");
		
		return _rsslSeries.totalCountHint();
	}

	@Override
	public SummaryData summaryData()
	{
		if (_summaryData == null)
			_summaryData = new PayloadAttribSummaryImpl();
		
		_summaryData.data(_summaryDecoded);
		return (SummaryData)_summaryData;
	}
	
	public String toString()
	{
		return toString(0);
	}
	
	@Override
	public void clear()
	{
		super.clear();
		
		_rsslSeries.clear();
		
		if (_rsslEncodeIter != null)
			_seriesCollection.clear();
	}

	@Override
	public Series totalCountHint(int totalCountHint)
	{
		if (totalCountHint < 0 || totalCountHint > 1073741823)
			throw ommOORExcept().message("totalCountHint is out of range [0 - 1073741823].");

		_rsslSeries.applyHasTotalCountHint();
		_rsslSeries.totalCountHint(totalCountHint);

		return this;
	}

	@Override
	public Series summaryData(ComplexType summaryData)
	{
		if (summaryData == null)
			throw ommIUExcept().message("Passed in summaryData is null");

		_rsslSeries.applyHasSummaryData();
		Utilities.copy(((DataImpl) summaryData).encodedData(), _rsslSeries.encodedSummaryData());

		return this;
	}

	@Override
	public boolean isEmpty()
	{
		if (_fillCollection)
			fillCollection();
		return _seriesCollection.isEmpty();
	}

	@Override
	public Iterator<SeriesEntry> iterator()
	{
		if (_fillCollection)
			fillCollection();
		
		return new EmaIterator<SeriesEntry>(_seriesCollection.iterator());
	}

	@Override
	public int size()
	{
		if (_fillCollection)
			fillCollection();
		return _seriesCollection.size();
	}
	
	@Override
	public boolean add(SeriesEntry seriesEntry)
	{
		if (seriesEntry == null)
			throw new NullPointerException("Passed in seriesEntry is null.");

		return _seriesCollection.add(seriesEntry);
	}

	@Override
	public boolean addAll(Collection<? extends SeriesEntry> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public boolean contains(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public boolean containsAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}
	
	@Override
	public boolean remove(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public boolean removeAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public boolean retainAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public Object[] toArray()
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public <T> T[] toArray(T[] a)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}
	

	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("Series");
				
		if (hasTotalCountHint())
			_toString.append(" totalCountHint=\"").append(totalCountHint()).append("\"");

		if (_rsslSeries.checkHasSummaryData())
		{
			++indent;
			Utilities.addIndent(_toString.append("\n"), indent).append("SummaryData dataType=\"")
					 .append(DataType.asString(summaryData().dataType())).append("\"\n");
			
			++indent;
			_toString.append(_summaryDecoded.toString(indent));
			--indent;
			
			Utilities.addIndent(_toString, indent).append("SummaryDataEnd");
			--indent;
		}
		
		if (_fillCollection)
			fillCollection();
		
		if ( _seriesCollection.isEmpty() )
		{
			Utilities.addIndent(_toString.append("\n"), indent).append("SeriesEnd\n");
			return _toString.toString();
		}
		
		++indent;
		
		DataImpl load;
		for (SeriesEntry seriesEntry : _seriesCollection)
		{
			load = (DataImpl)seriesEntry.load();
			Utilities.addIndent(_toString.append("\n"), indent)
					 .append("SeriesEntry dataType=\"").append(DataType.asString(load.dataType())).append("\"\n");
			
			++indent;
			_toString.append(load.toString(indent));
			--indent;
			
			Utilities.addIndent(_toString, indent).append("SeriesEntryEnd");
		}

		--indent;

		Utilities.addIndent(_toString.append("\n"), indent).append("SeriesEnd\n");

		return _toString.toString();
	}
	
	@Override
	void decode(Buffer rsslBuffer, int majVer, int minVer,
			DataDictionary rsslDictionary, Object obj)
	{
		_fillCollection = true;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslDecodeIter.clear();
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslSeries.decode(_rsslDecodeIter);
		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.NO_DATA :
				_errorCode = ErrorCode.NO_ERROR;
				_rsslSeries.flags(0);
				_fillCollection = false;
				clearCollection();
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
				_errorCode = ErrorCode.NO_ERROR;
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.ITERATOR_OVERRUN :
				_errorCode = ErrorCode.ITERATOR_OVERRUN;
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_errorCode = ErrorCode.INCOMPLETE_DATA;
				break;
			default :
				_errorCode = ErrorCode.UNKNOWN_ERROR;
				break;
		}

		if (_errorCode == ErrorCode.NO_ERROR)
		{
			if (_rsslSeries.checkHasSetDefs())
			{
				switch (_rsslSeries.containerType())
				{
					case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST :
					{
						if (_rsslLocalFLSetDefDb != null)
							_rsslLocalFLSetDefDb.clear();
						else
						{
							if (GlobalPool._rsslFieldListSetDefList.size() > 0)
								_rsslLocalFLSetDefDb = GlobalPool._rsslFieldListSetDefList.get(0);
							else
								_rsslLocalFLSetDefDb = CodecFactory.createLocalFieldSetDefDb();
						}
						
						_rsslLocalFLSetDefDb.decode(_rsslDecodeIter);
						_rsslLocalSetDefDb = _rsslLocalFLSetDefDb;
						break;
					}
					case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST :
					{
						if (_rsslLocalELSetDefDb != null)
							_rsslLocalELSetDefDb.clear();
						else
						{
							if (GlobalPool._rsslElementListSetDefList.size() > 0)
								_rsslLocalELSetDefDb = GlobalPool._rsslElementListSetDefList.get(0);
							else
								_rsslLocalELSetDefDb = CodecFactory.createLocalElementSetDefDb();
						}
						
						_rsslLocalELSetDefDb.decode(_rsslDecodeIter);
						_rsslLocalSetDefDb = _rsslLocalELSetDefDb;
						break;
					}
					default :
						_rsslLocalSetDefDb = null;
						_errorCode = ErrorCode.UNSUPPORTED_DATA_TYPE;
						return;
				}
			}
			else
				_rsslLocalSetDefDb = null;

			int rsslContainerType = _rsslSeries.checkHasSummaryData() ? _rsslSeries.containerType() : com.thomsonreuters.upa.codec.DataTypes.NO_DATA;
			int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, _rsslSeries.encodedSummaryData());
			_summaryDecoded = dataInstance(_summaryDecoded, dType);
			_summaryDecoded.decode(_rsslSeries.encodedSummaryData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
		}
	}
	
	void fillCollection()
	{
		DataImpl load;
	
		clearCollection();

		SeriesEntryImpl seriesEntry = seriesEntryInstance();
		
		if ( ErrorCode.NO_ERROR != _errorCode)
		{
			load =  dataInstance(seriesEntry._load, DataTypes.ERROR);
			load.decode(_rsslBuffer, _errorCode);
			seriesEntry._load = load;
			_seriesCollection.add(seriesEntry);
			_fillCollection = false;
			return;
		}

		int retCode;
		while ((retCode  = seriesEntry._rsslSeriesEntry.decode(_rsslDecodeIter)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
				int dType = dataType(_rsslSeries.containerType(), _rsslMajVer, _rsslMinVer, seriesEntry._rsslSeriesEntry.encodedData());
				load = dataInstance(seriesEntry._load, dType);
				load.decode(seriesEntry._rsslSeriesEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = dataInstance(seriesEntry._load, DataTypes.ERROR);
				load.decode(seriesEntry._rsslSeriesEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				load = dataInstance(seriesEntry._load, DataTypes.ERROR);
				load.decode(seriesEntry._rsslSeriesEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default :
				load = dataInstance(seriesEntry._load, DataTypes.ERROR);
				load.decode(seriesEntry._rsslSeriesEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
				break;
			}
		
			seriesEntry._load = load;
			_seriesCollection.add(seriesEntry);
			
			seriesEntry = seriesEntryInstance();
		}
		
		seriesEntry.returnToPool();
		
		_fillCollection = false;
	}

	Buffer encodedData()
	{
		if (_encodeComplete)
			return _rsslBuffer; 
		
		if (_seriesCollection.isEmpty())
			throw ommIUExcept().message("Series to be encoded is empty.");
		
		int ret = _rsslEncodeIter.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl encode iterator. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText);
	    }
	    
	    SeriesEntryImpl firstEntry = (SeriesEntryImpl)_seriesCollection.get(0);
	    int entryType = firstEntry._entryDataType;
		_rsslSeries.containerType(entryType);
		
	    ret = _rsslSeries.encodeInit(_rsslEncodeIter, 0, 0);
	    while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
	    {
	    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
	    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
	    	ret = _rsslSeries.encodeInit(_rsslEncodeIter, 0, 0);
	    }
	    
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to intialize encoding on rssl series. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText);
	    }
	    
	    SeriesEntryImpl seriesEntry;
		for (com.thomsonreuters.ema.access.SeriesEntry entry  : _seriesCollection)
		{
			seriesEntry = ((SeriesEntryImpl)entry);
			if (entryType != seriesEntry._entryDataType)
			{
				String errText = errorString().append("Attempt to add entry of ")
						.append(com.thomsonreuters.upa.codec.DataTypes.toString(seriesEntry._entryDataType))
						.append("while Series contains=")
						.append(com.thomsonreuters.upa.codec.DataTypes.toString(entryType)).toString();
				throw ommIUExcept().message(errText);
			}
			
			ret = seriesEntry._rsslSeriesEntry.encode(_rsslEncodeIter) ;
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
			   	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			   	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			   	ret = seriesEntry._rsslSeriesEntry.encode(_rsslEncodeIter);
			}
			 
			if (ret != CodecReturnCodes.SUCCESS)
		    {
		    	String errText = errorString().append("Failed to ")
		    								.append("rsslSeriesEntry.encode()")
		    								.append(" while encoding rssl series. Reason='")
		    								.append(CodecReturnCodes.toString(ret))
		    								.append("'").toString();
		    	throw ommIUExcept().message(errText);
		    }
		 }
		 
		ret =  _rsslSeries.encodeComplete(_rsslEncodeIter, true);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to complete encoding on rssl series. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	        throw ommIUExcept().message(errText);
	    }
	    
	    _encodeComplete = true;
	    return _rsslBuffer;
	}
	
	private SeriesEntryImpl seriesEntryInstance()
	{
		SeriesEntryImpl retData = (SeriesEntryImpl)GlobalPool._seriesEntryPool.poll();
        if(retData == null)
        {
        	retData = new SeriesEntryImpl(com.thomsonreuters.upa.codec.CodecFactory.createSeriesEntry(), noDataInstance());
        	GlobalPool._seriesEntryPool.updatePool(retData);
        }
        else
        	retData._rsslSeriesEntry.clear();
        
        return retData;
	}
	
	private void clearCollection()
	{
		if (_seriesCollection.size() > 0 )
		{
			Iterator<SeriesEntry> iter = _seriesCollection.iterator();
			while ( iter.hasNext())
				((SeriesEntryImpl)iter.next()).returnToPool();
				
			_seriesCollection.clear();
		}
	}
}