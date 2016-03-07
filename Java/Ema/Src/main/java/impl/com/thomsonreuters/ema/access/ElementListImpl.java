///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.ElementEntry;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.Buffer;

class ElementListImpl extends CollectionDataImpl implements ElementList
{
	private com.thomsonreuters.upa.codec.ElementList _rsslElementList = com.thomsonreuters.upa.codec.CodecFactory.createElementList();
	private LinkedList<ElementEntry> _elementListCollection = new LinkedList<ElementEntry>(); 
	
	ElementListImpl() 
	{
		super(false);
	}
	
	ElementListImpl(boolean decoding)
	{
		super(decoding);
	} 
	
	@Override
	public int dataType()
	{
		return DataTypes.ELEMENT_LIST;
	}

	@Override
	public boolean hasInfo()
	{
		return _rsslElementList.checkHasInfo();
	}

	@Override
	public int infoElementListNum()
	{
		if (!hasInfo())
			throw ommIUExcept().message("Attempt to infoElementListNum() while ElementList Info is NOT set.");

		return _rsslElementList.elementListNum();
	}

	@Override
	public void clear()
	{
		super.clear();
		
		_rsslElementList.clear();
		_elementListCollection.clear();
	}

	@Override
	public ElementList info(int elementListNum)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public boolean isEmpty()
	{
		if (_fillCollection)
			fillCollection();
		return _elementListCollection.isEmpty();
	}

	@Override
	public Iterator<ElementEntry> iterator()
	{
		if (_fillCollection)
			fillCollection();
		
		return new EmaIterator<ElementEntry>(_elementListCollection.iterator());
	}

	@Override
	public int size()
	{
		if (_fillCollection)
			fillCollection();
		return _elementListCollection.size();
	}
	
	@Override
	public boolean add(ElementEntry e)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public boolean addAll(Collection<? extends ElementEntry> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public boolean contains(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public boolean containsAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public boolean remove(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public boolean removeAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public boolean retainAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public Object[] toArray()
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public <T> T[] toArray(T[] a)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent)
				.append("ElementList");
				
		if (hasInfo())
			_toString.append(" ElementListNum=\"")
					 .append(infoElementListNum())
					 .append("\"");

		if (_fillCollection)
			fillCollection();
		
		if ( _elementListCollection.isEmpty() )
		{
			Utilities.addIndent(_toString.append("\n"), indent).append("ElementListEnd\n");
			return _toString.toString();
		}
		
		++indent;
		
		DataImpl load;
		int loadDataType;
		for (ElementEntry elementEntry : _elementListCollection)
		{
			load = (DataImpl)elementEntry.load();
			loadDataType = load.dataType();
			
			Utilities.addIndent(_toString.append("\n"), indent).append("ElementEntry name=\"")
																  .append(elementEntry.name())
																  .append("\" dataType=\"")
																  .append(DataType.asString(loadDataType));

			if (DataTypes.ARRAY >= loadDataType || DataTypes.ERROR == loadDataType)
			{
				++indent; 
				_toString.append("\"\n")
						 .append(load.toString(indent));
				--indent;
				Utilities.addIndent(_toString, indent).append("ElementEntryEnd");
			}
			else if (loadDataType == DataTypes.BUFFER)
			{
				if (load.code() == DataCode.BLANK)
					_toString.append("\" value=\"").append(load.toString()).append("\"");
				else
					_toString.append("\"\n").append(load.toString());
			}
			else
				_toString.append("\" value=\"")
						 .append(load.toString())
					     .append("\"");
		}

		--indent;

		Utilities.addIndent(_toString.append("\n"), indent).append("ElementListEnd\n");

		return _toString.toString();
	}
	
	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int majVer, int minVer,
				com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localElSeDefDb)
	{
		_fillCollection = true;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslLocalELSetDefDb = (com.thomsonreuters.upa.codec.LocalElementSetDefDb)localElSeDefDb;

		_rsslDecodeIter.clear();
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslElementList.decode(_rsslDecodeIter, _rsslLocalELSetDefDb);
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
		case com.thomsonreuters.upa.codec.CodecReturnCodes.SET_SKIPPED :
			_errorCode = ErrorCode.NO_SET_DEFINITION;
			break;
		default :
			_errorCode = ErrorCode.UNKNOWN_ERROR;
			break;
		}
	}
	
	void fillCollection()
	{
		DataImpl load;
		com.thomsonreuters.upa.codec.ElementEntry rsslElementEntry = com.thomsonreuters.upa.codec.CodecFactory.createElementEntry();
		
		_elementListCollection.clear();
		
		if ( ErrorCode.NO_ERROR != _errorCode)
		{
			load =  dataInstance(DataTypes.ERROR);
			load.decode(_rsslBuffer, _errorCode);
			_elementListCollection.add(new ElementEntryImpl(rsslElementEntry, load));
			_fillCollection = false;
			return;
		}
		
		int retCode;
		while ((retCode  = rsslElementEntry.decode(_rsslDecodeIter)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
				int dType = dataType(rsslElementEntry.dataType(), _rsslMajVer, _rsslMinVer, rsslElementEntry.encodedData());
				load = dataInstance(dType);
				
				if (DataTypes.ERROR > dType && DataTypes.OPAQUE <= dType)
					load.decode(rsslElementEntry.encodedData(),_rsslDecodeIter);
				else
					load.decode(rsslElementEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, null);
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = dataInstance(DataTypes.ERROR);
				load.decode(rsslElementEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				load = dataInstance(DataTypes.ERROR);
				load.decode(rsslElementEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default :
				load = dataInstance(DataTypes.ERROR);
				load.decode(rsslElementEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
				break;
			}

			_elementListCollection.add(new ElementEntryImpl(rsslElementEntry, load));
			rsslElementEntry = com.thomsonreuters.upa.codec.CodecFactory.createElementEntry();
		}
		
		_fillCollection = false;
	}
	
	Buffer encodedData()
	{
		return null;
	}
}