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

import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.ArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

class OmmArrayImpl extends CollectionDataImpl implements OmmArray
{
	private com.thomsonreuters.upa.codec.Array	_rsslArray = com.thomsonreuters.upa.codec.CodecFactory.createArray();
	private LinkedList<OmmArrayEntry> _ommArrayCollection = new LinkedList<OmmArrayEntry>(); 

	OmmArrayImpl() 
	{
		super(null);
	}
	
	OmmArrayImpl(EmaObjectManager objManager)
	{
		super(objManager);
	} 
	
	@Override
	public int dataType()
	{
		return DataTypes.ARRAY;
	}

	@Override
	public boolean hasFixedWidth()
	{
		return _rsslArray.itemLength()  > 0 ? true : false;
	}

	@Override
	public int fixedWidth()
	{
		return _rsslArray.itemLength();
	}
	
	@Override
	public String toString()
	{
		return toString(0);
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		
		if (code() == DataCode.BLANK)
		{
			Utilities.addIndent(_toString, indent)
					.append("OmmArray");
			
			++indent;
			Utilities.addIndent(_toString.append("\n"), indent).append("blank array");
			--indent;
		}		
		else
		{
			Utilities.addIndent(_toString, indent)
			.append("OmmArray with entries of dataType=\"")
			.append(DataType.asString(_rsslArray.primitiveType())).append("\"");
			
			if (hasFixedWidth())
				_toString.append(" fixed width=\"").append(fixedWidth()).append("\"");
			
			if (_fillCollection)
				fillCollection();
			
			if ( _ommArrayCollection.isEmpty() )
			{
				Utilities.addIndent(_toString.append("\n"), indent).append("OmmArrayEnd\n");
				return _toString.toString();
			}
			
			++indent;
			
			DataImpl load;
			for (OmmArrayEntry arrayEntry : _ommArrayCollection)
			{
				load = (DataImpl) arrayEntry.load();
				if ( load == null )
					return "\nDecoding of just encoded object in the same application is not supported\n";				
			
				Utilities.addIndent(_toString.append("\n"), indent).append("value=\"");
	
				if (load.dataType() == DataTypes.BUFFER)
					_toString.append("\n").append(load.toString());
				else if (load.dataType() == DataTypes.ERROR)
					_toString.append("\n").append(load.toString(indent));
				else
					_toString.append("\"").append(load.toString()).append("\"");
			}
	
			--indent;
		}
	
		Utilities.addIndent(_toString.append("\n"), indent).append("OmmArrayEnd\n");

		return _toString.toString();
	}

	@Override
	public void clear()
	{
		if (_rsslEncodeIter != null)
		{
			super.clear();
		
			_rsslArray.clear();
			
			int collectionSize = _ommArrayCollection.size();
			if (collectionSize > 0)
			{
				OmmArrayEntryImpl arrayEntryImpl;
				GlobalPool.lock();
				for (int index = 0; index < collectionSize; ++index)
				{
					arrayEntryImpl = (OmmArrayEntryImpl)_ommArrayCollection.get(index);
					GlobalPool.returnPool(arrayEntryImpl._previousEncodingType, arrayEntryImpl._entryData);
					arrayEntryImpl._previousEncodingType = com.thomsonreuters.upa.codec.DataTypes.UNKNOWN;
				}
				GlobalPool.unlock();
		
				_ommArrayCollection.clear();
			}
		}
		else
			clearCollection();
	}

	@Override
	public OmmArray fixedWidth(int width)
	{
		if (width < 0 || width > 65535)
			throw ommOORExcept().message("width is out of range [0 - 65535].");
		
		_rsslArray.itemLength(width);
		return this;
	}

	@Override
	public boolean isEmpty()
	{
		if (_fillCollection)
			fillCollection();
		return _ommArrayCollection.isEmpty();
	}

	@Override
	public Iterator<OmmArrayEntry> iterator()
	{
		if (_fillCollection)
			fillCollection();
		
		return new EmaIterator<OmmArrayEntry>(_ommArrayCollection.iterator());
	}

	@Override
	public int size()
	{
		if (_fillCollection)
			fillCollection();
		return _ommArrayCollection.size();
	}
	
	@Override
	public boolean add(OmmArrayEntry arrayEntry)
	{
		if (arrayEntry == null)
			throw new NullPointerException("Passed in arrayEntry is null.");
		
		return _ommArrayCollection.add(arrayEntry);
	}

	@Override
	public boolean addAll(Collection<? extends OmmArrayEntry> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public boolean contains(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public boolean containsAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public boolean remove(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public boolean removeAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public boolean retainAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public Object[] toArray()
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public <T> T[] toArray(T[] a)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}
	

	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int majVer, int minVer,
				com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localElSeDefDb)
	{
		_fillCollection = true;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		_rsslDecodeIter.clear();
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		_rsslArray.clear();
		retCode = _rsslArray.decode(_rsslDecodeIter);
		switch (retCode)
		{
		case com.thomsonreuters.upa.codec.CodecReturnCodes.BLANK_DATA :
			_dataCode = DataCode.BLANK;
			_errorCode = ErrorCode.NO_ERROR;
			_fillCollection = false;
			_ommArrayCollection.clear();
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			_dataCode = DataCode.NO_CODE;
			_errorCode = ErrorCode.NO_ERROR;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.ITERATOR_OVERRUN :
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_OVERRUN;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.INCOMPLETE_DATA;
			break;
		default :
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.UNKNOWN_ERROR;
			break;
		}
	}

	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		_fillCollection = true;

		_rsslMajVer = dIter.majorVersion();

		_rsslMinVer = dIter.minorVersion();

		_rsslBuffer = rsslBuffer;

		_rsslDecodeIter.clear();
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		_rsslArray.clear();
		retCode = _rsslArray.decode(_rsslDecodeIter);
		switch (retCode)
		{
		case com.thomsonreuters.upa.codec.CodecReturnCodes.BLANK_DATA :
			_dataCode = DataCode.BLANK;
			_errorCode = ErrorCode.NO_ERROR;
			_fillCollection = false;
			clearCollection();
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			_dataCode = DataCode.NO_CODE;
			_errorCode = ErrorCode.NO_ERROR;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.ITERATOR_OVERRUN :
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_OVERRUN;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.INCOMPLETE_DATA;
			break;
		default :
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.UNKNOWN_ERROR;
			break;
		}
	}

	void fillCollection()
	{
		DataImpl load;
				
		clearCollection();
		
		OmmArrayEntryImpl arrayEntry = ommArrayEntryInstance();

		if ( ErrorCode.NO_ERROR != _errorCode)
		{
			load =  dataInstance(arrayEntry._load, DataTypes.ERROR);
			load.decode(_rsslBuffer, _errorCode);
			arrayEntry._load = load;
			_ommArrayCollection.add(arrayEntry);
			_fillCollection = false;
			return;
		}
		
		int retCode;
		while ((retCode  = arrayEntry._rsslArrayEntry.decode(_rsslDecodeIter)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			load = dataInstance(arrayEntry._load, _rsslArray.primitiveType());
			load.decode(arrayEntry._rsslArrayEntry.encodedData(),_rsslDecodeIter);
			break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = dataInstance(arrayEntry._load, DataTypes.ERROR);
				load.decode(arrayEntry._rsslArrayEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				load = dataInstance(arrayEntry._load, DataTypes.ERROR);
				load.decode(arrayEntry._rsslArrayEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default :
				load = dataInstance(arrayEntry._load, DataTypes.ERROR);
				load.decode(arrayEntry._rsslArrayEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
				break;
			}
			
			arrayEntry._load = load;
			_ommArrayCollection.add(arrayEntry);
			
			arrayEntry = ommArrayEntryInstance();
			arrayEntry._rsslArrayEntry.clear();
		}
		
		arrayEntry.returnToPool();
		
		_fillCollection = false;
	}
	
	Buffer encodedData()
	{
		if (_encodeComplete || (_rsslEncodeIter == null))
			return _rsslBuffer; 
		
		if (_ommArrayCollection.isEmpty())
			throw ommIUExcept().message("OmmArray to be encoded is empty.");
		
		int ret = _rsslEncodeIter.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl encode iterator. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText);
	    }

	   OmmArrayEntryImpl firstEntry = (OmmArrayEntryImpl)_ommArrayCollection.get(0);
	    int primitiveType = firstEntry._entryDataType; 
		_rsslArray.primitiveType(primitiveType);
		
	    while ((ret = _rsslArray.encodeInit(_rsslEncodeIter)) == CodecReturnCodes.BUFFER_TOO_SMALL)
	    {
	    	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
	    }
	    
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to intialize encoding on rssl array. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText);
	    }
	    
	    ret = CodecReturnCodes.FAILURE;
	    int fixedItemLength =_rsslArray.itemLength();
	    OmmArrayEntryImpl arrayEntry;
		for (com.thomsonreuters.ema.access.OmmArrayEntry ommArrayEntry  : _ommArrayCollection)
		{
			arrayEntry = ((OmmArrayEntryImpl)ommArrayEntry);
			if (primitiveType != arrayEntry._entryDataType)
			{
				String errText = errorString().append("Attempt to add entry of ")
						.append(com.thomsonreuters.upa.codec.DataTypes.toString(arrayEntry._entryDataType))
						.append("while OmmArray contains=")
						.append(com.thomsonreuters.upa.codec.DataTypes.toString(primitiveType)).toString();
				throw ommIUExcept().message(errText);
			}
			
		  if ((ret = arrayEntryEncode(fixedItemLength, primitiveType, arrayEntry._rsslArrayEntry, arrayEntry._entryData)) != CodecReturnCodes.SUCCESS)
		    {
		    	String errText = errorString().append("Failed to ")
		    								.append("rsslommArrayEntry.encode()")
		    								.append(" while encoding rssl array. Reason='")
		    								.append(CodecReturnCodes.toString(ret))
		    								.append("'").toString();
		    	throw ommIUExcept().message(errText);
		    }
		 }
		 
		ret =  _rsslArray.encodeComplete(_rsslEncodeIter, true);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to complete encoding on rssl array. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	        throw ommIUExcept().message(errText);
	    }
	 
	    _encodeComplete = true;
	    return _rsslBuffer;
	}
	
	int arrayEntryEncode(int fixedItemLength, int dataType, ArrayEntry rsslArrayEntry, Object value)
	{
		int ret;
		if ( value == null )
		{
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		}
		
		switch(dataType)
		{
		case com.thomsonreuters.upa.codec.DataTypes.UINT:
			long uintValue = ((com.thomsonreuters.upa.codec.UInt)value).toLong(); 
			switch(fixedItemLength)
			{
				case 0 :
				case 8 :
					break;
				case 1 :
					if ( uintValue > 255 )
					{
						String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of UInt type. Fixed width='")
						.append( fixedItemLength ).append( "' value='" ).append( uintValue ).append( "'. " ).toString();
						 throw ommIUExcept().message(errText);
					}
					break;
				case 2 :
					if ( uintValue  > 65535 )
					{
						String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of UInt type. Fixed width='")
						.append( fixedItemLength ).append( "' value='" ).append( uintValue ).append( "'. " ).toString();
						 throw ommIUExcept().message(errText);
					}
					break;
				case 4 :
					if (  uintValue  > 4294967295L )
					{
						String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of UInt type. Fixed width='")
						.append( fixedItemLength ).append( "' value='" ).append( uintValue ).append( "'. " ).toString();
						 throw ommIUExcept().message(errText);
					}
					break;
				default :
					{
						String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of UInt type. Fixed width='")
						.append( fixedItemLength ).append( "'. " ).toString();
						 throw ommIUExcept().message(errText);
					}
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.UInt)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.INT:
			long intValue = ((com.thomsonreuters.upa.codec.Int)value).toLong(); 
			switch (fixedItemLength)
			{
			case 0 :
			case 8 :
				break;
			case 1 :
				if ( intValue > 127 || intValue < -127 )
				{
					String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of Int type. Fixed width='")
					.append( fixedItemLength ).append( "' value='" ).append( intValue ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText);
				}
				break;
			case 2 :
				if ( intValue > 32767 || intValue < -32767 )
				{
					String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of Int type. Fixed width='")
					.append( fixedItemLength ).append( "' value='" ).append( intValue ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText);
				}
				break;
			case 4 :
				if ( intValue > 2147483647  || intValue < -2147483647  )
				{
					String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of Int type. Fixed width='")
					.append( fixedItemLength ).append( "' value='" ).append( intValue ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText);
				}
				break;
			default :
				{
					String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Int type. Fixed width='")
					.append( fixedItemLength ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText);
				}
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Int)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.ENUM:
			int enumValue = ((com.thomsonreuters.upa.codec.Enum)value).toInt();
			switch (fixedItemLength)
			{
			case 0 :
				break;
			case 1 :
				if ( enumValue > 255 )
				{
					String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of Enum type. Fixed width='")
					.append( fixedItemLength ).append( "' value='" ).append( enumValue ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText);
				}
				break;
			case 2 :
				if ( enumValue > 65535 )
				{
					String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of Enum type. Fixed width='")
					.append( fixedItemLength ).append( "' value='" ).append( enumValue ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText);
				}
				break;
			default :
				{
					String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Enum type. Fixed width='")
					.append( fixedItemLength ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText);
				}
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Enum)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.REAL:
			if (fixedItemLength != 0)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Real type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Real)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.FLOAT:
			if (fixedItemLength != 0 && fixedItemLength != 4)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Float type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Float)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.DOUBLE:
			if (fixedItemLength != 0 && fixedItemLength != 8)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Double type . Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Double)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.DATE:
			if (fixedItemLength != 0 && fixedItemLength != 4)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Date type . Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Date)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.TIME:
			if ( fixedItemLength == 0 || fixedItemLength == 5 || (fixedItemLength == 3 && ((com.thomsonreuters.upa.codec.Time)value).millisecond() == 0)  )
			{
				while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Time)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
					_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
				return ret;
			}
			else
			{
				String errText = errorString().append("Unsupported fixedWidth encoding entry of Time type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText);
			}
		case com.thomsonreuters.upa.codec.DataTypes.DATETIME :
			if ( fixedItemLength == 0 || fixedItemLength == 9 || (fixedItemLength == 7 && ((com.thomsonreuters.upa.codec.DateTime)value).millisecond() == 0)  )
			{
				while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.DateTime)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
					_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
				return ret;
			}
			else
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in entry of DateTime type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText);
			}
		case com.thomsonreuters.upa.codec.DataTypes.QOS:
			if (fixedItemLength != 0)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in entry of Qos type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Qos)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.STATE:
			if (fixedItemLength != 0)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in entry of State type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.State)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.thomsonreuters.upa.codec.DataTypes.BUFFER:
		case com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING:
		case com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING:
		case com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING:
			if ( fixedItemLength != 0 && fixedItemLength < ((Buffer)value).length() )
			{
				String errText = errorString().append("Passed in value is longer than fixed width in encoding entry of Buffer type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Buffer)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		default:
			{
				String errText = errorString().append("Unsupported data type encoding in encoding entry. Data type='")
				.append( com.thomsonreuters.upa.codec.DataTypes.toString(dataType) ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText);
			}
		}
	}
	
	private OmmArrayEntryImpl ommArrayEntryInstance()
	{
		OmmArrayEntryImpl retData = (OmmArrayEntryImpl)_objManager._arrayEntryPool.poll();
        if(retData == null)
        {
        	retData = new OmmArrayEntryImpl(com.thomsonreuters.upa.codec.CodecFactory.createArrayEntry(), noDataInstance());
        	_objManager._arrayEntryPool.updatePool(retData);
        }
        else
        {
        	retData._rsslArrayEntry.clear();
        }
        
        retData._entryDataType = _rsslArray.primitiveType();
        
        return retData;
	}
	
	private void clearCollection()
	{
		int collectionSize = _ommArrayCollection.size();
		if (collectionSize > 0)
		{
			for (int index = 0; index < collectionSize; ++index)
				((OmmArrayEntryImpl)_ommArrayCollection.get(index)).returnToPool();
	
			_ommArrayCollection.clear();
		}
	}
}