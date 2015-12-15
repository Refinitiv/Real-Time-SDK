///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

import com.thomsonreuters.ema.access.ComplexType;
import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.Series;
import com.thomsonreuters.ema.access.SeriesEntry;
import com.thomsonreuters.ema.access.SummaryData;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;

public class SeriesImpl extends ComplexTypeImpl implements Series
{
	private com.thomsonreuters.upa.codec.Series	_rsslSeries = com.thomsonreuters.upa.codec.CodecFactory.createSeries();
	private com.thomsonreuters.upa.codec.SeriesEntry	_rsslSeriesEntry = com.thomsonreuters.upa.codec.CodecFactory.createSeriesEntry();
	private SeriesEntryImpl _seriesEntry = new SeriesEntryImpl(this, _rsslSeriesEntry, null);
	private com.thomsonreuters.upa.codec.Series	_rsslSeriesForCollection;
	private LinkedList<SeriesEntry> _seriesCollection = new LinkedList<SeriesEntry>(); 
	private DataImpl _summaryDecoded = new NoDataImpl();
	private PayloadAttribSummaryImpl _summaryData;
	
	public SeriesImpl()
	{
		_load = new NoDataImpl();
	}

	@Override
	public int dataType()
	{
		return DataTypes.SERIES;
	}

	@Override
	public boolean forth()
	{
		if (_atEnd) return false;

		if (!_decodingStarted && ErrorCode.NO_ERROR != _errorCode)
		{
			_atEnd = true;
			_decodingStarted = true;
			_load =  dataInstance(_load, DataTypes.ERROR);
			_load.decode(_rsslBuffer, _errorCode);
			_seriesEntry.load(_load);
			return true;
		}

		_decodingStarted = true;

		int retCode = _rsslSeriesEntry.decode(_rsslDecodeIter);

		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			{
				int dType = dataType(_rsslSeries.containerType(), _rsslMajVer, _rsslMinVer, _rsslSeriesEntry.encodedData());
							
				_load = dataInstance(_load, dType);
				_load.decode(_rsslSeriesEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
				
				_seriesEntry.load(_load);
				
				return true;
			}
			case com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER :
				_atEnd = true;
				return false;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslSeriesEntry.encodedData(), ErrorCode.INCOMPLETE_DATA); 
				_seriesEntry.load(_load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslSeriesEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				_seriesEntry.load(_load);
				return true;
			default :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslSeriesEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				_seriesEntry.load(_load);
				return true;
		}
	}

	@Override
	public void reset()
	{
		// TODO Auto-generated method stub
	}

	@Override
	public SeriesEntry entry()
	{
		if (!_decodingStarted)
			throw oommIUExcept().message("Attempt to entry() while decoding entry was NOT started.");
		
		return _seriesEntry;
	}

	@Override
	public boolean hasTotalCountHint()
	{
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public int totalCountHint()
	{
		// TODO Auto-generated method stub
		return 0;
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
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("Series");
				
		if (hasTotalCountHint())
			_toString.append(" totalCountHint=\"").append(totalCountHint()).append("\"");
		
		if (hasSummary())
		{
			++indent;
			Utilities.addIndent(_toString.append("\n"), indent).append("SummaryData dataType=\"")
					 .append(DataType.asString(summaryData().dataType())).append("\"\n");
			
			++indent;
			_toString.append(((DataImpl)summary()).toString(indent));
			--indent;
			
			Utilities.addIndent(_toString, indent).append("SummaryDataEnd");
			--indent;
		}
		
		++indent;
			
		while (forth())
		{
			Utilities.addIndent(_toString.append("\n"), indent)
					 .append("SeriesEntry dataType=\"").append(DataType.asString(_load.dataType())).append("\"\n");
			
			++indent;
			_toString.append(_load.toString(indent));
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
		_decodingStarted = false;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslDecodeIter.clear();
		
		_seriesCollection.clear();
		
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_atEnd = false;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslSeries.decode(_rsslDecodeIter);
		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.NO_DATA :
				_atEnd = true;
				_errorCode = ErrorCode.NO_ERROR;
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
				_atEnd = false;
				_fillCollection = true;
				_errorCode = ErrorCode.NO_ERROR;
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.ITERATOR_OVERRUN :
				_atEnd = false;
				_errorCode = ErrorCode.ITERATOR_OVERRUN;
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_atEnd = false;
				_errorCode = ErrorCode.INCOMPLETE_DATA;
				break;
			default :
				_atEnd = false;
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
		if (_rsslDecodeIterForCollection == null)
			_rsslDecodeIterForCollection = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
		else
			_rsslDecodeIterForCollection.clear();
		
		int retCode = _rsslDecodeIterForCollection.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
			return;
		
		if (_rsslSeriesForCollection == null)
			_rsslSeriesForCollection = com.thomsonreuters.upa.codec.CodecFactory.createSeries();
		
		if ((retCode = _rsslSeriesForCollection.decode(_rsslDecodeIterForCollection)) != CodecReturnCodes.SUCCESS)
			return;
		
		SeriesEntryImpl seriesEntry;
		DataImpl load;
		com.thomsonreuters.upa.codec.SeriesEntry rsslSeriesEntry = com.thomsonreuters.upa.codec.CodecFactory.createSeriesEntry();

		while ((retCode  = rsslSeriesEntry.decode(_rsslDecodeIterForCollection)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			if (retCode != CodecReturnCodes.SUCCESS)
				return;
		
			int dType = dataType(_rsslSeriesForCollection.containerType(), _rsslMajVer, _rsslMinVer, rsslSeriesEntry.encodedData());
			load = dataInstance(dType);
			load.decode(rsslSeriesEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
			
			seriesEntry = new SeriesEntryImpl(this, rsslSeriesEntry, load);
			_seriesCollection.add(seriesEntry);
			
			rsslSeriesEntry = com.thomsonreuters.upa.codec.CodecFactory.createSeriesEntry();
		}
		
		_fillCollection = false;
	}
	
	boolean hasSummary()
	{
		return _rsslSeries.checkHasSummaryData();
	}
	
	Data summary()
	{
		return _summaryDecoded;
	}
	
	@Override
	public void clear()
	{
		// TODO Auto-generated method stub
	}

	@Override
	public Series totalCountHint(int totalCountHint)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Series summaryData(ComplexType data)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Series add(ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Series complete()
	{
		// TODO Auto-generated method stub
		return null;
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
	public boolean add(SeriesEntry e)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
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
}