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
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.FilterEntryActions;

class FilterListImpl extends CollectionDataImpl implements FilterList
{
	private com.thomsonreuters.upa.codec.FilterList	_rsslFilterList = com.thomsonreuters.upa.codec.CodecFactory.createFilterList();
	private LinkedList<FilterEntry> _filterListCollection = new LinkedList<FilterEntry>(); 
	
	FilterListImpl() 
	{
		super(false);
	}
	
	FilterListImpl(boolean decoding)
	{
		super(decoding);
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
			throw ommIUExcept().message("Attempt to totalCountHint() while it is not set.");
		
		return _rsslFilterList.totalCountHint();
	}

	@Override
	public String toString()
	{
		return toString(0);
	}

	@Override
	public void clear()
	{
		super.clear();
		
		_rsslFilterList.clear();
		_filterListCollection.clear();
	}

	@Override
	public FilterList totalCountHint(int totalCountHint)
	{
		if (totalCountHint < 0 || totalCountHint > 1073741823)
			throw ommOORExcept().message("totalCountHint is out of range [0 - 1073741823].");

		_rsslFilterList.applyHasTotalCountHint();
		_rsslFilterList.totalCountHint(totalCountHint);

		return this;
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
	public boolean add(FilterEntry filterEntry)
	{
		if (filterEntry == null)
			throw new NullPointerException("Passed in filterEntry is null.");

		return _filterListCollection.add(filterEntry);
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
	
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent)
				.append("FilterList");
				
		if (hasTotalCountHint())
			_toString.append(" totalCountHint=\"")
					 .append(totalCountHint())
					 .append("\"");
		
		if (_fillCollection)
			fillCollection();
		
		if ( _filterListCollection.isEmpty() )
		{
			Utilities.addIndent(_toString.append("\n"), indent).append("FilterListEnd\n");
			return _toString.toString();
		}
		
		++indent;
		
		DataImpl load;
		for (FilterEntry filterEntry : _filterListCollection)
		{
			load = (DataImpl)filterEntry.load();
			Utilities.addIndent(_toString.append("\n"), indent).append("FilterEntry action=\"")
																  .append(filterEntry.filterActionAsString())
																  .append("\" filterId=\"")
																  .append(filterEntry.filterId());

			if (filterEntry.hasPermissionData())
			{
				_toString.append(" permissionData=\"");
				Utilities.asHexString(_toString, filterEntry.permissionData()).append("\"");
			}
				
			_toString.append(" dataType=\"").append(DataType.asString(load.dataType()))
					 .append("\"\n");
			
			++indent;
			_toString.append(load.toString(indent));
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
		
		retCode = _rsslFilterList.decode(_rsslDecodeIter);
		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.NO_DATA :
				_errorCode = ErrorCode.NO_ERROR;
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
	}
	
	void fillCollection()
	{
		DataImpl load;
		com.thomsonreuters.upa.codec.FilterEntry rsslFilterEntry = com.thomsonreuters.upa.codec.CodecFactory.createFilterEntry();

		_filterListCollection.clear();
		
		if ( ErrorCode.NO_ERROR != _errorCode)
		{
			load =  dataInstance(DataTypes.ERROR);
			load.decode(_rsslBuffer, _errorCode);
			_filterListCollection.add(new FilterEntryImpl(rsslFilterEntry, load));
			_fillCollection = false;
			return;
		}

		int retCode;
		while ((retCode  = rsslFilterEntry.decode(_rsslDecodeIter)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
				int rsslContainerType = (rsslFilterEntry.action() == FilterEntryActions.CLEAR) ?  com.thomsonreuters.upa.codec.DataTypes.NO_DATA : (rsslFilterEntry.checkHasContainerType() ?
						rsslFilterEntry.containerType() : _rsslFilterList.containerType());

				int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, rsslFilterEntry.encodedData());
				
				load = dataInstance(dType);
				load.decode(rsslFilterEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, null);
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = dataInstance(DataTypes.ERROR);
				load.decode(rsslFilterEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				load = dataInstance(DataTypes.ERROR);
				load.decode(rsslFilterEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default :
				load = dataInstance(DataTypes.ERROR);
				load.decode(rsslFilterEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
				break;
			}
			
			_filterListCollection.add(new FilterEntryImpl( rsslFilterEntry, load));
			rsslFilterEntry = com.thomsonreuters.upa.codec.CodecFactory.createFilterEntry();
		}
		
		_fillCollection = false;
	}
	
	Buffer encodedData()
	{
		if (_encodeComplete)
			return _rsslBuffer; 
		
		if (_filterListCollection.isEmpty())
			throw ommIUExcept().message("Series to be encoded is empty.");
		
		int ret = _rsslEncodeIter.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl encode iterator. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText);
	    }
	    
	    FilterEntryImpl firstEntry = (FilterEntryImpl)_filterListCollection.get(0);
		_rsslFilterList.containerType(firstEntry._rsslFilterEntry.containerType());

		ret = _rsslFilterList.encodeInit(_rsslEncodeIter);
	    while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
	    {
	    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
	    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
	    	ret = _rsslFilterList.encodeInit(_rsslEncodeIter);
	    }
	    
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to intialize encoding on rssl series. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText);
	    }
	    
		for (com.thomsonreuters.ema.access.FilterEntry entry  : _filterListCollection)
		{
			ret = ((FilterEntryImpl)entry)._rsslFilterEntry.encode(_rsslEncodeIter);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
			   	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			   	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			   	ret = ((FilterEntryImpl)entry)._rsslFilterEntry.encode(_rsslEncodeIter);
			}

			if (ret != CodecReturnCodes.SUCCESS)
		    {
				String errText = errorString().append("Failed to ")
						.append("rsslFilterEntry.encode()")
						.append(" while encoding rssl series. Reason='")
						.append(CodecReturnCodes.toString(ret))
						.append("'").toString();
				throw ommIUExcept().message(errText);
		    }
		 }
		 
		ret =  _rsslFilterList.encodeComplete(_rsslEncodeIter, true);
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
}