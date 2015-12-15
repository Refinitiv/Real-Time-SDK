///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

import com.thomsonreuters.ema.access.ComplexType;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.FilterEntry;
import com.thomsonreuters.ema.access.FilterList;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;

public class FilterListImpl extends ComplexTypeImpl implements FilterList
{
	private com.thomsonreuters.upa.codec.FilterList	_rsslFilterList = com.thomsonreuters.upa.codec.CodecFactory.createFilterList();
	private com.thomsonreuters.upa.codec.FilterEntry _rsslFilterEntry = com.thomsonreuters.upa.codec.CodecFactory.createFilterEntry();
	private FilterEntryImpl _filterEntry = new FilterEntryImpl(this, _rsslFilterEntry, null);
	private com.thomsonreuters.upa.codec.FilterList	_rsslFilterListForCollection;
	private LinkedList<FilterEntry> _filterListCollection = new LinkedList<FilterEntry>(); 
	
	public FilterListImpl()
	{
		_load = new NoDataImpl();
	}

	@Override
	public int dataType()
	{
		return DataTypes.FILTER_LIST;
	}

	@Override
	public boolean hasTotalCountHint()
	{
		return _rsslFilterList.checkHasTotalCountHint();
	}

	@Override
	public int totalCountHint()
	{
		if (!hasTotalCountHint())
			throw oommIUExcept().message("Attempt to totalCountHint() while it is not set.");
		
		return _rsslFilterList.totalCountHint();
	}

	@Override
	public FilterEntry entry()
	{
		if (!_decodingStarted)
			throw oommIUExcept().message("Attempt to entry() while decoding entry was NOT started.");
		
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public String toString()
	{
		return toString(0);
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent)
				.append("FilterList");
				
		if (hasTotalCountHint())
			_toString.append(" totalCountHint=\"")
					 .append(totalCountHint())
					 .append("\"");
		
		++indent;
			
		while (forth())
		{
			Utilities.addIndent(_toString.append("\n"), indent).append("FilterEntry action=\"")
																  .append(_filterEntry.filterActionAsString())
																  .append("\" filterId=\"")
																  .append(_filterEntry.filterId());

			if (_filterEntry.hasPermissionData())
			{
				_toString.append(" permissionData=\"");
				Utilities.asHexString(_toString, _filterEntry.permissionData()).append("\"");
			}
				
			_toString.append(" dataType=\"").append(DataType.asString(_load.dataType()))
					 .append("\"\n");
			
			++indent;
			_toString.append(_load.toString(indent));
			--indent;
			
			Utilities.addIndent(_toString, indent).append("FilterEntryEnd");
		}

		--indent;

		Utilities.addIndent(_toString.append("\n"), indent).append("FilterListEnd\n");

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
		
		_filterListCollection.clear();
		
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_atEnd = false;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslFilterList.decode(_rsslDecodeIter);
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
		
		if (_rsslFilterListForCollection == null)
			_rsslFilterListForCollection = com.thomsonreuters.upa.codec.CodecFactory.createFilterList();
		
		if ((retCode = _rsslFilterListForCollection.decode(_rsslDecodeIterForCollection)) != CodecReturnCodes.SUCCESS)
			return;
		
		FilterEntryImpl filterEntry;
		DataImpl load;
		com.thomsonreuters.upa.codec.FilterEntry rsslFilterEntry = com.thomsonreuters.upa.codec.CodecFactory.createFilterEntry();

		while ((retCode  = rsslFilterEntry.decode(_rsslDecodeIterForCollection)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			if (retCode != CodecReturnCodes.SUCCESS)
				return;
			
			
			int rsslContainerType = (rsslFilterEntry.checkHasContainerType() ?
					rsslFilterEntry.containerType() : _rsslFilterListForCollection.containerType());
			
			int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, rsslFilterEntry.encodedData());
			
			load = dataInstance(dType);
			load.decode(rsslFilterEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, null);

			filterEntry = new FilterEntryImpl(this, rsslFilterEntry, load);
			
			_filterListCollection.add(filterEntry);
			
			rsslFilterEntry = com.thomsonreuters.upa.codec.CodecFactory.createFilterEntry();
		}
		
		_fillCollection = false;
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
			_filterEntry.load(_load);
			return true;
		}

		_decodingStarted = true;

		int retCode = _rsslFilterEntry.decode(_rsslDecodeIter);

		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			{
				int rsslContainerType = (_rsslFilterEntry.checkHasContainerType() ?
						_rsslFilterEntry.containerType() : _rsslFilterList.containerType());
				
				int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, _rsslFilterEntry.encodedData());
				
				_load = dataInstance(_load, dType);
				_load.decode(_rsslFilterEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, null);
				
				_filterEntry.load(_load);
				
				return true;
			}
			case com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER :
				_atEnd = true;
				return false;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslFilterEntry.encodedData(), ErrorCode.INCOMPLETE_DATA); 
				_filterEntry.load(_load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslFilterEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				_filterEntry.load(_load);
				return true;
			default :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslFilterEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				_filterEntry.load(_load);
				return true;
		}
	}

	@Override
	public boolean forth(int filterId)
	{
		int retCode = com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS;

		do
		{
			if (_atEnd) return false;

			if (!_decodingStarted && ErrorCode.NO_ERROR != _errorCode)
			{
				_atEnd = true;
				_decodingStarted = true;
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslBuffer, _errorCode);
				_filterEntry.load(_load);
				return true;
			}

			_decodingStarted = true;

			retCode = _rsslFilterEntry.decode(_rsslDecodeIter);

			if (retCode == com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
			{
				_atEnd = true;
				return false;
			}
		}
		while (	_rsslFilterEntry.id() != filterId);

		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			{
				int rsslContainerType = (_rsslFilterEntry.checkHasContainerType() ?
						_rsslFilterEntry.containerType() : _rsslFilterList.containerType());
				
				int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, _rsslFilterEntry.encodedData());
				
				_load = dataInstance(_load, dType);
				_load.decode(_rsslFilterEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, null);
				
				_filterEntry.load(_load);
				return true;
			}
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslFilterEntry.encodedData(), ErrorCode.INCOMPLETE_DATA); 
				_filterEntry.load(_load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslFilterEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				_filterEntry.load(_load);
				return true;
			default :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslFilterEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				_filterEntry.load(_load);
				return true;
		}
	}
	
	@Override
	public void reset()
	{
		// TODO Auto-generated method stub
	}

	@Override
	public void clear()
	{
		// TODO Auto-generated method stub
	}

	@Override
	public FilterList totalCountHint(int totalCountHint)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public FilterList add(int filterId, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterList add(int filterId, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FilterList complete()
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public boolean isEmpty()
	{
		if (_fillCollection)
			fillCollection();
		return _filterListCollection.isEmpty();
	}

	@Override
	public Iterator<FilterEntry> iterator()
	{
		if (_fillCollection)
			fillCollection();
		
		return new EmaIterator<FilterEntry>(_filterListCollection.iterator());
	}

	@Override
	public int size()
	{
		if (_fillCollection)
			fillCollection();
		return _filterListCollection.size();
	}

	@Override
	public boolean add(FilterEntry e)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("FilterList collection doesn't support this operation.");
	}

	@Override
	public boolean addAll(Collection<? extends FilterEntry> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("FilterList collection doesn't support this operation.");
	}

	@Override
	public boolean contains(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("FilterList collection doesn't support this operation.");
	}

	@Override
	public boolean containsAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("FilterList collection doesn't support this operation.");
	}

	@Override
	public boolean remove(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("FilterList collection doesn't support this operation.");
	}

	@Override
	public boolean removeAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("FilterList collection doesn't support this operation.");
	}

	@Override
	public boolean retainAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("FilterList collection doesn't support this operation.");
	}

	@Override
	public Object[] toArray()
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("FilterList collection doesn't support this operation.");
	}

	@Override
	public <T> T[] toArray(T[] a)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("FilterList collection doesn't support this operation.");
	}
}