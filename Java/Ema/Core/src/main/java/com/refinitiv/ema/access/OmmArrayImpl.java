/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2022-2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.eta.codec.ArrayEntry;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.Codec;
import com.refinitiv.eta.codec.CodecReturnCodes;

class OmmArrayImpl extends CollectionDataImpl implements OmmArray
{
	private com.refinitiv.eta.codec.Array	_rsslArray = com.refinitiv.eta.codec.CodecFactory.createArray();
	private LinkedList<OmmArrayEntry> _ommArrayCollection = new LinkedList<OmmArrayEntry>(); 
	private OmmArrayIterImpl _ommArrayIterImpl = null;

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

	@Override
	public String toString (DataDictionary dictionary)
	{
		if (!dictionary.isFieldDictionaryLoaded() || !dictionary.isEnumTypeDefLoaded())
			return "\nDictionary is not loaded.\n";

		if (_objManager == null)
		{
			_objManager = new EmaObjectManager();
			_objManager.initialize(((DataImpl)this).dataType());
		}

		OmmArray ommArray = new OmmArrayImpl(_objManager);

		((CollectionDataImpl) ommArray).decode(((DataImpl)this).encodedData(), Codec.majorVersion(), Codec.minorVersion(), ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);
		if (_errorCode != ErrorCode.NO_ERROR)
		{
			return "\nFailed to decode OmmArray with error: " + ((CollectionDataImpl) ommArray).errorString() + "\n";
		}

		return ommArray.toString();
	}
	String toString(int indent)
	{
		if ( _objManager == null )
			return "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n";

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
					return "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n";
			
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
					arrayEntryImpl._previousEncodingType = com.refinitiv.eta.codec.DataTypes.UNKNOWN;
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
	public Iterator<OmmArrayEntry> iteratorByRef()
	{
		if (_ommArrayIterImpl == null)
			_ommArrayIterImpl = new OmmArrayIterImpl(this);
		else
			_ommArrayIterImpl.clear();
		return _ommArrayIterImpl;
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
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public boolean contains(Object o)
	{
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public boolean containsAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public boolean remove(Object o)
	{
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public boolean removeAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public boolean retainAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public Object[] toArray()
	{
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}

	@Override
	public <T> T[] toArray(T[] a)
	{
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
	}
	

	@Override
	void decode(com.refinitiv.eta.codec.Buffer rsslBuffer, int majVer, int minVer,
				com.refinitiv.eta.codec.DataDictionary rsslDictionary, Object localElSeDefDb)
	{
		_fillCollection = true;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		_rsslDecodeIter.clear();
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		_rsslArray.clear();
		retCode = _rsslArray.decode(_rsslDecodeIter);
		switch (retCode)
		{
		case com.refinitiv.eta.codec.CodecReturnCodes.BLANK_DATA :
			_dataCode = DataCode.BLANK;
			_errorCode = ErrorCode.NO_ERROR;
			_fillCollection = false;
			_ommArrayCollection.clear();
			break;
		case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
			_dataCode = DataCode.NO_CODE;
			_errorCode = ErrorCode.NO_ERROR;
			break;
		case com.refinitiv.eta.codec.CodecReturnCodes.ITERATOR_OVERRUN :
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_OVERRUN;
			break;
		case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
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
	int decode(com.refinitiv.eta.codec.Buffer rsslBuffer, com.refinitiv.eta.codec.DecodeIterator dIter)
	{
		_fillCollection = true;

		_rsslMajVer = dIter.majorVersion();

		_rsslMinVer = dIter.minorVersion();

		_rsslBuffer = rsslBuffer;

		_rsslDecodeIter.clear();
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
		}
		
		_rsslArray.clear();
		retCode = _rsslArray.decode(_rsslDecodeIter);
		switch (retCode)
		{
		case com.refinitiv.eta.codec.CodecReturnCodes.BLANK_DATA :
			_dataCode = DataCode.BLANK;
			_errorCode = ErrorCode.NO_ERROR;
			_fillCollection = false;
			clearCollection();
			break;
		case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
			_dataCode = DataCode.NO_CODE;
			_errorCode = ErrorCode.NO_ERROR;
			break;
		case com.refinitiv.eta.codec.CodecReturnCodes.ITERATOR_OVERRUN :
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_OVERRUN;
			break;
		case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.INCOMPLETE_DATA;
			break;
		default :
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.UNKNOWN_ERROR;
			break;
		}
		return com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS;
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
		while ((retCode  = arrayEntry._rsslArrayEntry.decode(_rsslDecodeIter)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
			load = dataInstance(arrayEntry._load, _rsslArray.primitiveType());
				int decodeRetVal = load.decode(arrayEntry._rsslArrayEntry.encodedData(), _rsslDecodeIter);
				if(decodeRetVal == com.refinitiv.eta.codec.CodecReturnCodes.INVALID_ARGUMENT ||
						decodeRetVal ==	com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA)
				{
					load = dataInstance(load, DataTypes.ERROR);
					load.decode(arrayEntry._rsslArrayEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				}
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = dataInstance(arrayEntry._load, DataTypes.ERROR);
				load.decode(arrayEntry._rsslArrayEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
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
			throw ommIUExcept().message("OmmArray to be encoded is empty.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		int ret = _rsslEncodeIter.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl encode iterator. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText, ret);
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
	    	throw ommIUExcept().message(errText, ret);
	    }
	    
	    ret = CodecReturnCodes.FAILURE;
	    int fixedItemLength =_rsslArray.itemLength();
	    OmmArrayEntryImpl arrayEntry;
		for (com.refinitiv.ema.access.OmmArrayEntry ommArrayEntry  : _ommArrayCollection)
		{
			arrayEntry = ((OmmArrayEntryImpl)ommArrayEntry);
			if (primitiveType != arrayEntry._entryDataType)
			{
				String errText = errorString().append("Attempt to add entry of ")
						.append(com.refinitiv.eta.codec.DataTypes.toString(arrayEntry._entryDataType))
						.append("while OmmArray contains=")
						.append(com.refinitiv.eta.codec.DataTypes.toString(primitiveType)).toString();
				throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
			
		  if ((ret = arrayEntryEncode(fixedItemLength, primitiveType, arrayEntry._rsslArrayEntry, arrayEntry._entryData)) != CodecReturnCodes.SUCCESS)
		    {
		    	String errText = errorString().append("Failed to ")
		    								.append("rsslommArrayEntry.encode()")
		    								.append(" while encoding rssl array. Reason='")
		    								.append(CodecReturnCodes.toString(ret))
		    								.append("'").toString();
		    	throw ommIUExcept().message(errText, ret);
		    }
		 }
		 
		ret =  _rsslArray.encodeComplete(_rsslEncodeIter, true);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to complete encoding on rssl array. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	        throw ommIUExcept().message(errText, ret);
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
		case com.refinitiv.eta.codec.DataTypes.UINT:
			long uintValue = ((com.refinitiv.eta.codec.UInt)value).toLong(); 
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
						 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
					}
					break;
				case 2 :
					if ( uintValue  > 65535 )
					{
						String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of UInt type. Fixed width='")
						.append( fixedItemLength ).append( "' value='" ).append( uintValue ).append( "'. " ).toString();
						 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
					}
					break;
				case 4 :
					if (  uintValue  > 4294967295L )
					{
						String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of UInt type. Fixed width='")
						.append( fixedItemLength ).append( "' value='" ).append( uintValue ).append( "'. " ).toString();
						 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
					}
					break;
				default :
					{
						String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of UInt type. Fixed width='")
						.append( fixedItemLength ).append( "'. " ).toString();
						 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
					}
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.UInt)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.refinitiv.eta.codec.DataTypes.INT:
			long intValue = ((com.refinitiv.eta.codec.Int)value).toLong(); 
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
					 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
				break;
			case 2 :
				if ( intValue > 32767 || intValue < -32767 )
				{
					String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of Int type. Fixed width='")
					.append( fixedItemLength ).append( "' value='" ).append( intValue ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
				break;
			case 4 :
				if ( intValue > 2147483647  || intValue < -2147483647  )
				{
					String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of Int type. Fixed width='")
					.append( fixedItemLength ).append( "' value='" ).append( intValue ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
				break;
			default :
				{
					String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Int type. Fixed width='")
					.append( fixedItemLength ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
				}
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Int)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.refinitiv.eta.codec.DataTypes.ENUM:
			int enumValue = ((com.refinitiv.eta.codec.Enum)value).toInt();
			switch (fixedItemLength)
			{
			case 0 :
				break;
			case 1 :
				if ( enumValue > 255 )
				{
					String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of Enum type. Fixed width='")
					.append( fixedItemLength ).append( "' value='" ).append( enumValue ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
				break;
			case 2 :
				if ( enumValue > 65535 )
				{
					String errText = errorString().append("Out of range value for the specified fixed width in encoding entry of Enum type. Fixed width='")
					.append( fixedItemLength ).append( "' value='" ).append( enumValue ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
				}
				break;
			default :
				{
					String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Enum type. Fixed width='")
					.append( fixedItemLength ).append( "'. " ).toString();
					 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
				}
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Enum)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.refinitiv.eta.codec.DataTypes.REAL:
			if (fixedItemLength != 0)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Real type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Real)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.refinitiv.eta.codec.DataTypes.FLOAT:
			if (fixedItemLength != 0 && fixedItemLength != 4)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Float type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Float)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.refinitiv.eta.codec.DataTypes.DOUBLE:
			if (fixedItemLength != 0 && fixedItemLength != 8)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Double type . Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Double)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.refinitiv.eta.codec.DataTypes.DATE:
			if (fixedItemLength != 0 && fixedItemLength != 4)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in encoding entry of Date type . Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Date)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.refinitiv.eta.codec.DataTypes.TIME:
			if ( fixedItemLength == 0 || fixedItemLength == 5 || (fixedItemLength == 3 && ((com.refinitiv.eta.codec.Time)value).millisecond() == 0)  )
			{
				while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Time)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
					_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
				return ret;
			}
			else
			{
				String errText = errorString().append("Unsupported fixedWidth encoding entry of Time type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
		case com.refinitiv.eta.codec.DataTypes.DATETIME :
			if ( fixedItemLength == 0 || fixedItemLength == 9 || (fixedItemLength == 7 && ((com.refinitiv.eta.codec.DateTime)value).millisecond() == 0)  )
			{
				while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.DateTime)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
					_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
				return ret;
			}
			else
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in entry of DateTime type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
		case com.refinitiv.eta.codec.DataTypes.QOS:
			if (fixedItemLength != 0)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in entry of Qos type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Qos)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.refinitiv.eta.codec.DataTypes.STATE:
			if (fixedItemLength != 0)
			{
				String errText = errorString().append("Unsupported fixedWidth encoding in entry of State type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.State)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		case com.refinitiv.eta.codec.DataTypes.BUFFER:
		case com.refinitiv.eta.codec.DataTypes.ASCII_STRING:
		case com.refinitiv.eta.codec.DataTypes.UTF8_STRING:
		case com.refinitiv.eta.codec.DataTypes.RMTES_STRING:
			if ( fixedItemLength != 0 && fixedItemLength < ((Buffer)value).length() )
			{
				String errText = errorString().append("Passed in value is longer than fixed width in encoding entry of Buffer type. Fixed width='")
				.append( fixedItemLength ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
			}
			while ((ret = rsslArrayEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Buffer)value)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		default:
			{
				String errText = errorString().append("Unsupported data type encoding in encoding entry. Data type='")
				.append( com.refinitiv.eta.codec.DataTypes.toString(dataType) ).append( "'. " ).toString();
				 throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
		}
	}
	
	private OmmArrayEntryImpl ommArrayEntryInstance()
	{
		OmmArrayEntryImpl retData = (OmmArrayEntryImpl)_objManager._arrayEntryPool.poll();
        if(retData == null)
        {
        	retData = new OmmArrayEntryImpl(com.refinitiv.eta.codec.CodecFactory.createArrayEntry(), noDataInstance());
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