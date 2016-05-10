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

class ElementListImpl extends CollectionDataImpl implements ElementList
{
	private com.thomsonreuters.upa.codec.ElementList _rsslElementList = com.thomsonreuters.upa.codec.CodecFactory
			.createElementList();
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
		
		if (_rsslEncodeIter != null)
			_elementListCollection.clear();
	}

	@Override
	public ElementList info(int elementListNum)
	{
		if (elementListNum < -32768 || elementListNum > 32767)
			throw ommOORExcept().message("elementListNum is out of range [(-32768) - 32767].");

		_rsslElementList.elementListNum(elementListNum);
		_rsslElementList.applyHasInfo();
		return null;
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
			loadDataType = load.dataType();

			Utilities.addIndent(_toString.append("\n"), indent).append("ElementEntry name=\"")
					.append(elementEntry.name()).append("\" dataType=\"").append(DataType.asString(loadDataType));

			if (DataTypes.ARRAY >= loadDataType || DataTypes.ERROR == loadDataType)
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
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int majVer, int minVer,
			com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localElSeDefDb)
	{
		_fillCollection = true;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslLocalELSetDefDb = (com.thomsonreuters.upa.codec.LocalElementSetDefDb) localElSeDefDb;

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
		case com.thomsonreuters.upa.codec.CodecReturnCodes.NO_DATA:
			_errorCode = ErrorCode.NO_ERROR;
			_rsslElementList.flags(0);
			_fillCollection = false;
			clearCollection();
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS:
			_errorCode = ErrorCode.NO_ERROR;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.ITERATOR_OVERRUN:
			_errorCode = ErrorCode.ITERATOR_OVERRUN;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA:
			_errorCode = ErrorCode.INCOMPLETE_DATA;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.SET_SKIPPED:
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
				.decode(_rsslDecodeIter)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch (retCode)
			{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS:
				int dType = dataType(elementEntry._rsslElementEntry.dataType(), _rsslMajVer, _rsslMinVer,
						elementEntry._rsslElementEntry.encodedData());
				load = dataInstance(elementEntry._load, dType);

				if (DataTypes.ERROR > dType && DataTypes.OPAQUE <= dType)
					load.decode(elementEntry._rsslElementEntry.encodedData(), _rsslDecodeIter);
				else
					load.decode(elementEntry._rsslElementEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, null);
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA:
				load = dataInstance(elementEntry._load, DataTypes.ERROR);
				load.decode(elementEntry._rsslElementEntry.encodedData(), ErrorCode.INCOMPLETE_DATA);
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE:
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
		if (_encodeComplete)
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
			throw ommIUExcept().message(errText);
		}

		ret = _rsslElementList.encodeInit(_rsslEncodeIter, null, 0);
		while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
		{
			_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
			_rsslEncodeIter.realignBuffer(_rsslBuffer);
			ret = _rsslElementList.encodeInit(_rsslEncodeIter, null, 0);
		}

		if (ret != CodecReturnCodes.SUCCESS)
		{
			String errText = errorString().append("Failed to intialize encoding on rssl elementlist. Reason='")
					.append(CodecReturnCodes.toString(ret)).append("'").toString();
			throw ommIUExcept().message(errText);
		}

		ret = CodecReturnCodes.FAILURE;
		for (com.thomsonreuters.ema.access.ElementEntry elementEntry : _elementListCollection)
		{
			if ((ret = elementEntryEncode(((ElementEntryImpl) elementEntry)._rsslElementEntry,
					((ElementEntryImpl) elementEntry)._entryData)) != CodecReturnCodes.SUCCESS)
			{
				String errText = errorString().append("Failed to ").append("rsslElementEntry.encode()")
						.append(" while encoding rssl elementlist. Reason='").append(CodecReturnCodes.toString(ret))
						.append("'").toString();
				throw ommIUExcept().message(errText);
			}
		}

		ret = _rsslElementList.encodeComplete(_rsslEncodeIter, true);
		if (ret != CodecReturnCodes.SUCCESS)
		{
			String errText = errorString().append("Failed to complete encoding on rssl elementlist. Reason='")
					.append(CodecReturnCodes.toString(ret)).append("'").toString();
			throw ommIUExcept().message(errText);
		}

		_encodeComplete = true;
		return _rsslBuffer;
	}

	int elementEntryEncode(com.thomsonreuters.upa.codec.ElementEntry rsslElementEntry, Object cacheEntryData)
	{
		int ret;
		if (cacheEntryData == null)
		{
			ret = rsslElementEntry.encode(_rsslEncodeIter);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter);
			}
			return ret;
		}

		switch (rsslElementEntry.dataType())
		{
		case com.thomsonreuters.upa.codec.DataTypes.INT:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Int) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Int) cacheEntryData);
			}
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.UINT:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.UInt) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.UInt) cacheEntryData);
			}
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.REAL:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Real) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Real) cacheEntryData);
			}
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.DOUBLE:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Double) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Double) cacheEntryData);
			}
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.FLOAT:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Float) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Float) cacheEntryData);
			}
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.DATETIME:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.DateTime) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.DateTime) cacheEntryData);
			}
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.DATE:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Date) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Date) cacheEntryData);
			}
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.TIME:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Time) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Time) cacheEntryData);
			}
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.QOS:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Qos) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Qos) cacheEntryData);
			}
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.STATE:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.State) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.State) cacheEntryData);
			}
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.ENUM:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Enum) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Enum) cacheEntryData);
			}
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.BUFFER:
		case com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING:
		case com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING:
		case com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING:
			ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Buffer) cacheEntryData);
			while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity() * 2));
				_rsslEncodeIter.realignBuffer(_rsslBuffer);
				ret = rsslElementEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Buffer) cacheEntryData);
			}
			return ret;
		default:
			return CodecReturnCodes.FAILURE;
		}
	}
	
	private ElementEntryImpl elementEntryInstance()
	{
		ElementEntryImpl retData = (ElementEntryImpl)GlobalPool._elementEntryPool.poll();
        if(retData == null)
        {
        	retData = new ElementEntryImpl(com.thomsonreuters.upa.codec.CodecFactory.createElementEntry(), noDataInstance());
        	GlobalPool._elementEntryPool.updatePool(retData);
        }
        else
        	retData._rsslElementEntry.clear();
        
        return retData;
	}
	
	private void clearCollection()
	{
		if (_elementListCollection.size() > 0 )
		{
			Iterator<ElementEntry> iter = _elementListCollection.iterator();
			while ( iter.hasNext())
				((ElementEntryImpl)iter.next()).returnToPool();
				
			_elementListCollection.clear();
		}
	}
}
