///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecReturnCodes;

class ElementListImpl extends CollectionDataImpl implements ElementList
{
	private com.refinitiv.eta.codec.ElementList _rsslElementList = com.refinitiv.eta.codec.CodecFactory
			.createElementList();
	private LinkedList<ElementEntry> _elementListCollection = new LinkedList<ElementEntry>();

	ElementListImpl()
	{
		super(null);
	}

	ElementListImpl(EmaObjectManager objManager)
	{
		super(objManager);
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
			throw ommIUExcept().message("Attempt to infoElementListNum() while ElementList Info is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return _rsslElementList.elementListNum();
	}

	@Override
	public void clear()
	{
		if (_rsslEncodeIter != null)
		{
			super.clear();

			_rsslElementList.clear();
			
			int collectionSize = _elementListCollection.size();
			if (collectionSize > 0)
			{
				ElementEntryImpl elementEntryImpl;
				GlobalPool.lock();
				for (int index = 0; index < collectionSize; ++index)
				{
					elementEntryImpl = (ElementEntryImpl)_elementListCollection.get(index);
					GlobalPool.returnPool(elementEntryImpl._previousEncodingType, elementEntryImpl._entryData);
					elementEntryImpl._previousEncodingType = com.refinitiv.eta.codec.DataTypes.UNKNOWN;
				}
				GlobalPool.unlock();
		
				_elementListCollection.clear();
			}
		}
		else
			clearCollection();
	}

	@Override
	public ElementList info(int elementListNum)
	{
		if (elementListNum < -32768 || elementListNum > 32767)
			throw ommOORExcept().message("elementListNum is out of range [(-32768) - 32767].");

		_rsslElementList.elementListNum(elementListNum);
		_rsslElementList.applyHasInfo();
		return this;
	}

	@Override
	public String toString()
	{
		return toString(0);
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
	public boolean add(ElementEntry elementEntry)
	{
		if (elementEntry == null)
			throw new NullPointerException("Passed in elementEntry is null.");

		return _elementListCollection.add(elementEntry);
	}

	@Override
	public boolean addAll(Collection<? extends ElementEntry> c)
	{
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public boolean contains(Object o)
	{
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public boolean containsAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public boolean remove(Object o)
	{
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public boolean removeAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public boolean retainAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public Object[] toArray()
	{
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	@Override
	public <T> T[] toArray(T[] a)
	{
		throw new UnsupportedOperationException("ElementList collection doesn't support this operation.");
	}

	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("ElementList");

		if (hasInfo())
			_toString.append(" ElementListNum=\"").append(infoElementListNum()).append("\"");

		if (_fillCollection)
			fillCollection();

		if (_elementListCollection.isEmpty())
		{
			Utilities.addIndent(_toString.append("\n"), indent).append("ElementListEnd\n");
			return _toString.toString();
		}

		++indent;

		DataImpl load;
		int loadDataType;
		for (ElementEntry elementEntry : _elementListCollection)
		{
			load = (DataImpl) elementEntry.load();
			if ( load == null )
				return "\nDecoding of just encoded object in the same application is not supported\n";
			
			loadDataType = load.dataType();

			Utilities.addIndent(_toString.append("\n"), indent).append("ElementEntry name=\"")
					.append(elementEntry.name()).append("\" dataType=\"").append(DataType.asString(loadDataType));

			if (DataTypes.FIELD_LIST <= loadDataType || DataTypes.ARRAY == loadDataType  || DataTypes.ERROR == loadDataType) 
			{
				++indent;
				_toString.append("\"\n").append(load.toString(indent));
				--indent;
				Utilities.addIndent(_toString, indent).append("ElementEntryEnd");
			} else if (loadDataType == DataTypes.BUFFER)
			{
				if (load.code() == DataCode.BLANK)
					_toString.append("\" value=\"").append(load.toString()).append("\"");
				else
					_toString.append("\"\n").append(load.toString());
			} else
				_toString.append("\" value=\"").append(load.toString()).append("\"");
		}

		--indent;

		Utilities.addIndent(_toString.append("\n"), indent).append("ElementListEnd\n");

		return _toString.toString();
	}

	@Override
	void decode(com.refinitiv.eta.codec.Buffer rsslBuffer, int majVer, int minVer,
			com.refinitiv.eta.codec.DataDictionary rsslDictionary, Object localElSeDefDb)
	{
		_fillCollection = true;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslLocalELSetDefDb = (com.refinitiv.eta.codec.LocalElementSetDefDb) localElSeDefDb;

		_rsslDecodeIter.clear();
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}

		retCode = _rsslElementList.decode(_rsslDecodeIter, _rsslLocalELSetDefDb);
		switch (retCode)
		{
		case com.refinitiv.eta.codec.CodecReturnCodes.NO_DATA:
			_errorCode = ErrorCode.NO_ERROR;
			_rsslElementList.flags(0);
			_fillCollection = false;
			clearCollection();
			break;
		case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS:
			_errorCode = ErrorCode.NO_ERROR;
			break;
		case com.refinitiv.eta.codec.CodecReturnCodes.ITERATOR_OVERRUN:
			_errorCode = ErrorCode.ITERATOR_OVERRUN;
			break;
		case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA:
			_errorCode = ErrorCode.INCOMPLETE_DATA;
			break;
		case com.refinitiv.eta.codec.CodecReturnCodes.SET_SKIPPED:
			_errorCode = ErrorCode.NO_SET_DEFINITION;
			break;
		default:
			_errorCode = ErrorCode.UNKNOWN_ERROR;
			break;
		}
	}

	void fillCollection()
	{
		DataImpl load;

		clearCollection();
		
		ElementEntryImpl elementEntry = elementEntryInstance();

		if (ErrorCode.NO_ERROR != _errorCode)
		{
			load = dataInstance(elementEntry._load, DataTypes.ERROR);
			load.decode(_rsslBuffer, _errorCode);
			elementEntry._load = load;
			_elementListCollection.add(elementEntry);
			_fillCollection = false;
			return;
		}

		int retCode;
		while ((retCode = elementEntry._rsslElementEntry
				.decode(_rsslDecodeIter)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch (retCode)
			{
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS:
				int dType = dataType(elementEntry._rsslElementEntry.dataType(), _rsslMajVer, _rsslMinVer,
						elementEntry._rsslElementEntry.encodedData());
				load = dataInstance(elementEntry._load, dType);

				if ( dType < DataType.DataTypes.FIELD_LIST || dType == DataType.DataTypes.ANSI_PAGE )
				{
					int decodeRetVal = load.decode(elementEntry._rsslElementEntry.encodedData(), _rsslDecodeIter);
					if(decodeRetVal == com.refinitiv.eta.codec.CodecReturnCodes.INVALID_ARGUMENT ||
							decodeRetVal ==	com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA){
						load = dataInstance(load, DataTypes.ERROR);
						load.decode(elementEntry._rsslElementEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
					}
				}
				else
					load.decode(elementEntry._rsslElementEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, null);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA:
				load = dataInstance(elementEntry._load, DataTypes.ERROR);
				load.decode(elementEntry._rsslElementEntry.encodedData(), ErrorCode.INCOMPLETE_DATA);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE:
				load = dataInstance(elementEntry._load, DataTypes.ERROR);
				load.decode(elementEntry._rsslElementEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default:
				load = dataInstance(elementEntry._load, DataTypes.ERROR);
				load.decode(elementEntry._rsslElementEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				break;
			}

			elementEntry._load = load;
			_elementListCollection.add(elementEntry);
			
			elementEntry = elementEntryInstance();
		}

		elementEntry.returnToPool();
		
		_fillCollection = false;
	}

	Buffer encodedData()
	{
		if (_encodeComplete || (_rsslEncodeIter == null) )
			return _rsslBuffer; 
		else
		{
			_rsslEncodeIter.clear();
			_rsslBuffer.data().clear();
		}

		if (!_elementListCollection.isEmpty())
			_rsslElementList.applyHasStandardData();

		int ret = _rsslEncodeIter.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (ret != CodecReturnCodes.SUCCESS)
		{
			String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl encode iterator. Reason='")
					.append(CodecReturnCodes.toString(ret)).append("'").toString();
			throw ommIUExcept().message(errText, ret);
		}

		while ((ret = _rsslElementList.encodeInit(_rsslEncodeIter, null, 0)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		{
			_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		}

		if (ret != CodecReturnCodes.SUCCESS)
		{
			String errText = errorString().append("Failed to intialize encoding on rssl elementlist. Reason='")
					.append(CodecReturnCodes.toString(ret)).append("'").toString();
			throw ommIUExcept().message(errText, ret);
		}

		ret = CodecReturnCodes.FAILURE;
		for (com.refinitiv.ema.access.ElementEntry elementEntry : _elementListCollection)
		{
			if ((ret = elementEntryEncode(((ElementEntryImpl) elementEntry)._rsslElementEntry,
					((ElementEntryImpl) elementEntry)._entryData)) != CodecReturnCodes.SUCCESS)
			{
				String errText = errorString().append("Failed to ").append("rsslElementEntry.encode()")
						.append(" while encoding rssl elementlist. Reason='").append(CodecReturnCodes.toString(ret))
						.append("'").toString();
				throw ommIUExcept().message(errText, ret);
			}
		}

		ret = _rsslElementList.encodeComplete(_rsslEncodeIter, true);
		if (ret != CodecReturnCodes.SUCCESS)
		{
			String errText = errorString().append("Failed to complete encoding on rssl elementlist. Reason='")
					.append(CodecReturnCodes.toString(ret)).append("'").toString();
			throw ommIUExcept().message(errText, ret);
		}

		_encodeComplete = true;
		return _rsslBuffer;
	}

	int elementEntryEncode(com.refinitiv.eta.codec.ElementEntry rsslElementEntry, Object cacheEntryData)
	{
		int ret;
		if (cacheEntryData == null)
		{
			while ((ret  = rsslElementEntry.encode(_rsslEncodeIter) ) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		}

		switch (rsslElementEntry.dataType())
		{
		case com.refinitiv.eta.codec.DataTypes.INT:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Int) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		case com.refinitiv.eta.codec.DataTypes.UINT:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.UInt) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		case com.refinitiv.eta.codec.DataTypes.REAL:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Real) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		case com.refinitiv.eta.codec.DataTypes.DOUBLE:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Double) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		case com.refinitiv.eta.codec.DataTypes.FLOAT:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Float) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		case com.refinitiv.eta.codec.DataTypes.DATETIME:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.DateTime) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		case com.refinitiv.eta.codec.DataTypes.DATE:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Date) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		case com.refinitiv.eta.codec.DataTypes.TIME:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Time) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		case com.refinitiv.eta.codec.DataTypes.QOS:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Qos) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		case com.refinitiv.eta.codec.DataTypes.STATE:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.State) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		case com.refinitiv.eta.codec.DataTypes.ENUM:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Enum) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		case com.refinitiv.eta.codec.DataTypes.BUFFER:
		case com.refinitiv.eta.codec.DataTypes.UTF8_STRING:
		case com.refinitiv.eta.codec.DataTypes.ASCII_STRING:
		case com.refinitiv.eta.codec.DataTypes.RMTES_STRING:
			while ((ret = rsslElementEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Buffer) cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}
			return ret;
		default:
			return CodecReturnCodes.FAILURE;
		}
	}
	
	private ElementEntryImpl elementEntryInstance()
	{
		ElementEntryImpl retData = (ElementEntryImpl)_objManager._elementEntryPool.poll();
        if(retData == null)
        {
        	retData = new ElementEntryImpl(com.refinitiv.eta.codec.CodecFactory.createElementEntry(), noDataInstance());
        	_objManager._elementEntryPool.updatePool(retData);
        }
        else
        	retData._rsslElementEntry.clear();
        
        return retData;
	}
	
	private void clearCollection()
	{
		int collectionSize = _elementListCollection.size();
		if (collectionSize > 0)
		{
			for (int index = 0; index < collectionSize; ++index)
				((ElementEntryImpl)_elementListCollection.get(index)).returnToPool();
	
			_elementListCollection.clear();
		}
	}
}
