///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019, 2024 Refinitiv. All rights reserved.        --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

class SeriesImpl extends CollectionDataImpl implements Series
{
	private com.refinitiv.eta.codec.Series	_rsslSeries = com.refinitiv.eta.codec.CodecFactory.createSeries();
	private LinkedList<SeriesEntry> _seriesCollection = new LinkedList<SeriesEntry>(); 
	private DataImpl _summaryDecoded;
	private PayloadAttribSummaryImpl _summaryData;
	private int _summaryDataType = com.refinitiv.eta.codec.DataTypes.NO_DATA;
	private boolean _summaryDataTypeSet = false;
	private SeriesIterImpl _seriesIterImpl = null;
	
	SeriesImpl() 
	{
		super(null);
	}
	
	SeriesImpl(EmaObjectManager objManager)
	{
		super(objManager);
		if (objManager != null)
			_summaryDecoded = noDataInstance();
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
			throw ommIUExcept().message("Attempt to totalCountHint() while it is not set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return _rsslSeries.totalCountHint();
	}

	@Override
	public SummaryData summaryData()
	{
		if (_summaryData == null)
			_summaryData = new PayloadAttribSummaryImpl();
		
		if (_summaryDecoded == null)
			_summaryDecoded = new NoDataImpl();
		
		_summaryData.data(_summaryDecoded);
		return _summaryData;
	}
	
	public String toString()
	{
		return toString(0);
	}
	
	@Override
	public void clear()
	{
		_summaryDataTypeSet = false;
		_summaryDataType = com.refinitiv.eta.codec.DataTypes.NO_DATA;
		
		if (_rsslEncodeIter != null)
		{
			super.clear();
		
			_rsslSeries.clear();
			_seriesCollection.clear();
		}
		else
			clearCollection();
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
			throw ommIUExcept().message("Passed in summaryData is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		_summaryDataType = summaryData.dataType();
		_summaryDataTypeSet = true;

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
	public Iterator<SeriesEntry> iteratorByRef()
	{
		if (_seriesIterImpl == null)
			_seriesIterImpl = new SeriesIterImpl(this);
		else
			_seriesIterImpl.clear();
		return _seriesIterImpl;
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
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public boolean contains(Object o)
	{
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public boolean containsAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}
	
	@Override
	public boolean remove(Object o)
	{
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public boolean removeAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public boolean retainAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public Object[] toArray()
	{
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public <T> T[] toArray(T[] a)
	{
		throw new UnsupportedOperationException("Series collection doesn't support this operation.");
	}

	@Override
	public String toString(DataDictionary dictionary)
	{
		if (!dictionary.isFieldDictionaryLoaded() || !dictionary.isEnumTypeDefLoaded())
			return "\nDictionary is not loaded.\n";

		if (_objManager == null) {
			_objManager = new EmaObjectManager();
			_objManager.initialize(((DataImpl)this).dataType());
		}

		Series series = new SeriesImpl(_objManager);

		((CollectionDataImpl) series).decode(((DataImpl)this).encodedData(), Codec.majorVersion(), Codec.minorVersion(), ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);
		if (_errorCode != ErrorCode.NO_ERROR)
		{
			return "\nFailed to decode Series with error: " + ((CollectionDataImpl) series).errorString() + "\n";
		}

		return series.toString();
	}

	String toString(int indent)
	{
		if ( _objManager == null )
			return "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n";

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
			load = (DataImpl) seriesEntry.load();
			if ( load == null )
				return "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n";
			
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
	void decode(com.refinitiv.eta.codec.Buffer rsslBuffer, int majVer, int minVer,
				com.refinitiv.eta.codec.DataDictionary rsslDictionary, Object obj)
	{
		_fillCollection = true;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslDecodeIter.clear();
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslSeries.decode(_rsslDecodeIter);
		switch (retCode)
		{
			case com.refinitiv.eta.codec.CodecReturnCodes.NO_DATA :
				_errorCode = ErrorCode.NO_ERROR;
				_rsslSeries.flags(0);
				_fillCollection = false;
				clearCollection();
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
				_errorCode = ErrorCode.NO_ERROR;
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.ITERATOR_OVERRUN :
				_errorCode = ErrorCode.ITERATOR_OVERRUN;
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
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
					case com.refinitiv.eta.codec.DataTypes.FIELD_LIST :
					{
						if (_rsslLocalFLSetDefDb != null)
							_rsslLocalFLSetDefDb.clear();
						else
								_rsslLocalFLSetDefDb = CodecFactory.createLocalFieldSetDefDb();
						
						_rsslLocalFLSetDefDb.decode(_rsslDecodeIter);
						_rsslLocalSetDefDb = _rsslLocalFLSetDefDb;
						break;
					}
					case com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST :
					{
						if (_rsslLocalELSetDefDb != null)
							_rsslLocalELSetDefDb.clear();
						else
								_rsslLocalELSetDefDb = CodecFactory.createLocalElementSetDefDb();
						
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

			int rsslContainerType = _rsslSeries.checkHasSummaryData() ? _rsslSeries.containerType() : com.refinitiv.eta.codec.DataTypes.NO_DATA;
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
		while ((retCode  = seriesEntry._rsslSeriesEntry.decode(_rsslDecodeIter)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
				int dType = dataType(_rsslSeries.containerType(), _rsslMajVer, _rsslMinVer, seriesEntry._rsslSeriesEntry.encodedData());
				load = dataInstance(seriesEntry._load, dType);
				load.decode(seriesEntry._rsslSeriesEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = dataInstance(seriesEntry._load, DataTypes.ERROR);
				load.decode(seriesEntry._rsslSeriesEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
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
		if (_encodeComplete || (_rsslEncodeIter == null) )
			return _rsslBuffer;
		
		int ret;
		int entryType = _summaryDataType;
		
		if (_seriesCollection.isEmpty())
		{
			_rsslSeries.containerType(entryType);
		}
		else
		{
		    SeriesEntryImpl firstEntry = (SeriesEntryImpl)_seriesCollection.get(0);
		    
		    if ( firstEntry._entryDataType != com.refinitiv.eta.codec.DataTypes.UNKNOWN )
		    {
		    	if ( _summaryDataTypeSet && (entryType !=  firstEntry._entryDataType) )
				{
					String errText = errorString().append("Attempt to add entry of ")
							.append(com.refinitiv.eta.codec.DataTypes.toString(firstEntry._entryDataType))
							.append(" while Series entry load type is set to ")
							.append(com.refinitiv.eta.codec.DataTypes.toString(entryType))
							.append(" with summaryData() method").toString();
					throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
				}
				else
				{
					entryType = firstEntry._entryDataType;
				}
		    	
		    	_rsslSeries.containerType(entryType);
		    }
		    else
		    {
		    	entryType = com.refinitiv.eta.codec.DataTypes.UNKNOWN;
		    	_rsslSeries.containerType(Utilities.toRsslDataType(firstEntry.loadType()));
		    }
		}
	    
	    setEncodedBufferIterator();
		
	    while ((ret = _rsslSeries.encodeInit(_rsslEncodeIter, 0, 0)) == CodecReturnCodes.BUFFER_TOO_SMALL)
	    {
	    	Buffer bigBuffer = CodecFactory.createBuffer();
			bigBuffer.data(ByteBuffer.allocate(_rsslBuffer.capacity() * 2));
			_rsslBuffer = bigBuffer;

			setEncodedBufferIterator();
	    }
	    
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to intialize encoding on rssl series. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText, ret);
	    }
	    
	    SeriesEntryImpl seriesEntry;
		for (com.refinitiv.ema.access.SeriesEntry entry  : _seriesCollection)
		{
			seriesEntry = ((SeriesEntryImpl)entry);
			if (entryType != seriesEntry._entryDataType)
			{
				String errText = errorString().append("Attempt to add entry of ")
						.append(com.refinitiv.eta.codec.DataTypes.toString(seriesEntry._entryDataType))
						.append(" while Series contains=")
						.append(com.refinitiv.eta.codec.DataTypes.toString(entryType)).toString();
				throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
			
			while ((ret = seriesEntry._rsslSeriesEntry.encode(_rsslEncodeIter) ) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			 
			if (ret != CodecReturnCodes.SUCCESS)
		    {
		    	String errText = errorString().append("Failed to ")
		    								.append("rsslSeriesEntry.encode()")
		    								.append(" while encoding rssl series. Reason='")
		    								.append(CodecReturnCodes.toString(ret))
		    								.append("'").toString();
		    	throw ommIUExcept().message(errText, ret);
		    }
		 }
		 
		ret =  _rsslSeries.encodeComplete(_rsslEncodeIter, true);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to complete encoding on rssl series. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	        throw ommIUExcept().message(errText, ret);
	    }
	    
	    _encodeComplete = true;
	    return _rsslBuffer;
	}
	
	void setEncodedBufferIterator()
	{
		_rsslEncodeIter.clear();
    	int ret = _rsslEncodeIter.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
    	
    	if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl encode iterator. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText, ret);
	    }
	}
	
	private SeriesEntryImpl seriesEntryInstance()
	{
		SeriesEntryImpl retData = (SeriesEntryImpl)_objManager._seriesEntryPool.poll();
        if(retData == null)
        {
        	retData = new SeriesEntryImpl(com.refinitiv.eta.codec.CodecFactory.createSeriesEntry(), noDataInstance());
        	_objManager._seriesEntryPool.updatePool(retData);
        }
        else
        	retData._rsslSeriesEntry.clear();
        
        return retData;
	}
	
	private void clearCollection()
	{
		int collectionSize = _seriesCollection.size();
		if (collectionSize > 0)
		{
			for (int index = 0; index < collectionSize; ++index)
				((SeriesEntryImpl)_seriesCollection.get(index)).returnToPool();
	
			_seriesCollection.clear();
		}
	}
}
