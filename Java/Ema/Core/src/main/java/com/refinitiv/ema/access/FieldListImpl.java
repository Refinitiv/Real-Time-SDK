///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecReturnCodes;

class FieldListImpl extends CollectionDataImpl implements FieldList
{
	private com.refinitiv.eta.codec.FieldList	_rsslFieldList = com.refinitiv.eta.codec.CodecFactory.createFieldList();
	private LinkedList<FieldEntry> _fieldListCollection = new LinkedList<FieldEntry>(); 
	DataDictionaryImpl _dataDictionaryImpl;
	
	FieldListImpl() 
	{
		super(null);
	}
	
	FieldListImpl(EmaObjectManager objManager)
	{
		super(objManager);
		
		_dataDictionaryImpl = new DataDictionaryImpl(false);
	} 
			
	@Override
	public int dataType()
	{
		return DataTypes.FIELD_LIST;
	}
	
	@Override
	public boolean hasInfo()
	{
		return _rsslFieldList.checkHasInfo();
	}

	@Override
	public int infoFieldListNum()
	{
		if (!hasInfo())
			throw ommIUExcept().message("Attempt to infoFieldListNum() while FieldList Info is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return _rsslFieldList.fieldListNum();
	}

	@Override
	public int infoDictionaryId()
	{
		if (!hasInfo())
			throw ommIUExcept().message("Attempt to infoDictionaryId() while FieldList Info is NOT set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);

		return _rsslFieldList.dictionaryId();
	}

	@Override
	public FieldList info(int dictionaryId, int fieldListNum)
	{
		if (dictionaryId < 0 || dictionaryId > 32767)
			throw ommOORExcept().message("dictionaryId is out of range [0 - 32767].");
		
		if (fieldListNum < -32768 || fieldListNum > 32767)
			throw ommOORExcept().message("fieldListNum is out of range [(-32768) - 32767].");

		_rsslFieldList.dictionaryId(dictionaryId);
		_rsslFieldList.fieldListNum(fieldListNum);
		_rsslFieldList.applyHasInfo();
		
		return this;
	}

	@Override
	public String toString()
	{
		return toString(0);
	}
	
	ByteBuffer dataBuffer()
	{
		return null;
	}
	
	@Override
	public Iterator<FieldEntry> iterator()
	{
		if (_fillCollection)
			fillCollection();
		
		return new EmaIterator<FieldEntry>(_fieldListCollection.iterator());
	}

	@Override
	public int size()
	{
		if (_fillCollection)
			fillCollection();
		
		return _fieldListCollection.size();
	}
	
	@Override
	public boolean isEmpty()
	{
		if (_fillCollection)
			fillCollection();
		return _fieldListCollection.isEmpty();
	}
	
	@Override
	public void clear()
	{
		if (_rsslEncodeIter != null)
		{
			super.clear();
		
			_rsslFieldList.clear();
			
			int collectionSize = _fieldListCollection.size();
			if (collectionSize > 0)
			{
				FieldEntryImpl fieldEntryImpl;
				GlobalPool.lock();
				for (int index = 0; index < collectionSize; ++index)
				{
					fieldEntryImpl = (FieldEntryImpl)_fieldListCollection.get(index);
					GlobalPool.returnPool(fieldEntryImpl._previousEncodingType, fieldEntryImpl._entryData);
					fieldEntryImpl._previousEncodingType = com.refinitiv.eta.codec.DataTypes.UNKNOWN;
				}
				GlobalPool.unlock();
		
				_fieldListCollection.clear();
			}
		}
		else
			clearCollection();
	}

	@Override
	public boolean add(FieldEntry fieldEntry)
	{
		if (fieldEntry == null)
			throw new NullPointerException("Passed in fieldEntry is null.");
		
		return _fieldListCollection.add(fieldEntry);
	}

	@Override
	public boolean addAll(Collection<? extends FieldEntry> c)
	{
		throw new UnsupportedOperationException("FieldList collection doesn't support this operation.");
	}

	@Override
	public boolean contains(Object o)
	{
		throw new UnsupportedOperationException("FieldList collection doesn't support this operation.");
	}

	@Override
	public boolean containsAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("FieldList collection doesn't support this operation.");
	}

	@Override
	public boolean remove(Object o)
	{
		throw new UnsupportedOperationException("FieldList collection doesn't support this operation.");
	}

	@Override
	public boolean removeAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("FieldList collection doesn't support this operation.");
	}

	@Override
	public boolean retainAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("FieldList collection doesn't support this operation.");
	}

	@Override
	public Object[] toArray()
	{
		throw new UnsupportedOperationException("FieldList collection doesn't support this operation.");
	}

	@Override
	public <T> T[] toArray(T[] a)
	{
		throw new UnsupportedOperationException("FieldList collection doesn't support this operation.");
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("FieldList");
				
		if (hasInfo())
			_toString.append(" FieldListNum=\"").append(infoFieldListNum()).append("\" DictionaryId=\"")
					 .append(infoDictionaryId()).append("\"");

		if (_fillCollection)
			fillCollection();
		
		if ( _fieldListCollection.isEmpty() )
		{
			Utilities.addIndent(_toString.append("\n"), indent).append("FieldListEnd\n");
			return _toString.toString();
		}
		
		++indent;
		
		DataImpl load;
		int loadDataType;
		for (FieldEntry fieldEntry : _fieldListCollection)
		{
			load = (DataImpl) fieldEntry.load();
			if ( load == null )
				return "\nDecoding of just encoded object in the same application is not supported\n";
			
			loadDataType = load.dataType();
			Utilities.addIndent(_toString.append("\n"), indent).append("FieldEntry fid=\"")
																  .append(fieldEntry.fieldId())
																  .append("\" name=\"")
																  .append(fieldEntry.name())
																  .append("\" dataType=\"")
																  .append(DataType.asString(loadDataType));

			if (DataTypes.FIELD_LIST <= loadDataType || DataTypes.ARRAY == loadDataType  || DataTypes.ERROR == loadDataType) 
			{
				++indent; 
				_toString.append("\"\n").append(load.toString(indent));
				--indent;
				Utilities.addIndent(_toString, indent).append("FieldEntryEnd");
			}
			else if (loadDataType == DataTypes.BUFFER)
			{
				if (load.code() == DataCode.BLANK)
					_toString.append("\" value=\"").append(load.toString()).append("\"");
				else
					_toString.append("\"\n").append(load.toString());
			}
			else
				_toString.append("\" value=\"").append(load.toString()).append("\"");
		}

		--indent;

		Utilities.addIndent(_toString.append("\n"), indent).append("FieldListEnd\n");

		return _toString.toString();
	}
	
	@Override
	void decode(com.refinitiv.eta.codec.Buffer rsslBuffer, int majVer, int minVer,
				com.refinitiv.eta.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		_fillCollection = true;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslLocalFLSetDefDb = (com.refinitiv.eta.codec.LocalFieldSetDefDb)localFlSetDefDb;

		if (_rsslDictionary == null)
		{
			_errorCode = ErrorCode.NO_DICTIONARY;
			return;
		}
		
		_dataDictionaryImpl.rsslDataDictionary(rsslDictionary);

		_rsslDecodeIter.clear();
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslFieldList.decode(_rsslDecodeIter, _rsslLocalFLSetDefDb);
		switch (retCode)
		{
		case com.refinitiv.eta.codec.CodecReturnCodes.NO_DATA :
			_errorCode = ErrorCode.NO_ERROR;
			_rsslFieldList.flags(0);
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
		case com.refinitiv.eta.codec.CodecReturnCodes.SET_SKIPPED :
			_errorCode = ErrorCode.NO_SET_DEFINITION;
			break;
		default :
			_errorCode = ErrorCode.UNKNOWN_ERROR;
			break;
		}
	}
	
	private void fillCollection()
	{
		DataImpl load;
		com.refinitiv.eta.codec.DictionaryEntry rsslDictionaryEntry = null;
		
		clearCollection();
		
		FieldEntryImpl fieldEntry = fieldEntryInstance();
		
		if ( ErrorCode.NO_ERROR != _errorCode)
		{
			load =  dataInstance(fieldEntry._load, DataTypes.ERROR);
			load.decode(_rsslBuffer, _errorCode);
			_fieldListCollection.add(fieldEntry.entryValue(this, null, load));
			_fillCollection = false;
			return;
		}

		int retCode;
		while ((retCode  = fieldEntry._rsslFieldEntry.decode(_rsslDecodeIter)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
				rsslDictionaryEntry = _rsslDictionary.entry(fieldEntry._rsslFieldEntry.fieldId());
				if (rsslDictionaryEntry == null)
				{
					load = dataInstance(fieldEntry._load, DataTypes.ERROR);
					load.decode(fieldEntry._rsslFieldEntry.encodedData(),ErrorCode.FIELD_ID_NOT_FOUND);
				}
				else			
				{			
					int dType = dataType(rsslDictionaryEntry.rwfType(), _rsslMajVer, _rsslMinVer, fieldEntry._rsslFieldEntry.encodedData());
					load = dataInstance(fieldEntry._load, dType);
					if ( dType < DataType.DataTypes.FIELD_LIST || dType == DataType.DataTypes.ANSI_PAGE )
						load.decode(fieldEntry._rsslFieldEntry.encodedData(),_rsslDecodeIter);
					else
						load.decode(fieldEntry._rsslFieldEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalFLSetDefDb);
				}
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = dataInstance(fieldEntry._load, DataTypes.ERROR);
				load.decode(fieldEntry._rsslFieldEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				load = dataInstance(fieldEntry._load, DataTypes.ERROR);
				load.decode(fieldEntry._rsslFieldEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default :
				load = dataInstance(fieldEntry._load, DataTypes.ERROR);
				load.decode(fieldEntry._rsslFieldEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
				break;
			}
			
			_fieldListCollection.add(fieldEntry.entryValue(this, rsslDictionaryEntry, load));
			fieldEntry =  fieldEntryInstance();
		}
		
		fieldEntry.returnToPool();
		
		_fillCollection = false;
	}
	
	Buffer encodedData() 
	{
		if (_encodeComplete || (_rsslEncodeIter == null) )
			return _rsslBuffer; 
		
		if (!_fieldListCollection.isEmpty())
			_rsslFieldList.applyHasStandardData();

		int ret = _rsslEncodeIter.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl encode iterator. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText, ret);
	    }
	 
	    while (( ret = _rsslFieldList.encodeInit(_rsslEncodeIter, null, 0)) == CodecReturnCodes.BUFFER_TOO_SMALL)
	    {
	    	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
	    }
	    
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to intialize encoding on rssl fieldlist. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText, ret);
	    }
	    
	    ret = CodecReturnCodes.FAILURE;
		for (com.refinitiv.ema.access.FieldEntry fieldEntry  : _fieldListCollection)
		{
			 if ((ret = fieldEntryEncode(((FieldEntryImpl)fieldEntry)._rsslFieldEntry, ((FieldEntryImpl)fieldEntry)._entryData)) != CodecReturnCodes.SUCCESS)
			 {
			    	String errText = errorString().append("Failed to ")
			    								.append("rsslFieldEntry.encode()")
			    								.append(" while encoding rssl fieldlist. Reason='")
			    								.append(CodecReturnCodes.toString(ret))
			    								.append("'").toString();
			    	throw ommIUExcept().message(errText, ret);
			 }
		 }
		 
		ret =  _rsslFieldList.encodeComplete(_rsslEncodeIter, true);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to complete encoding on rssl fieldlist. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	        throw ommIUExcept().message(errText, ret);
	    }
	    
	    _encodeComplete = true;
	    return _rsslBuffer;
	}
	
	private int fieldEntryEncode(com.refinitiv.eta.codec.FieldEntry rsslFieldEntry, Object cacheEntryData)
	{
		int ret;
		if ( cacheEntryData == null )
		{
			while ((ret = rsslFieldEntry.encode(_rsslEncodeIter)) == CodecReturnCodes.BUFFER_TOO_SMALL)
		    {
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
		    }
			return ret;
		}
		
		switch (rsslFieldEntry.dataType())
		{
		case com.refinitiv.eta.codec.DataTypes.INT:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Int)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		case com.refinitiv.eta.codec.DataTypes.UINT:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.UInt)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		case com.refinitiv.eta.codec.DataTypes.REAL:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Real)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		case com.refinitiv.eta.codec.DataTypes.DOUBLE:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Double)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		case com.refinitiv.eta.codec.DataTypes.FLOAT:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Float)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		case com.refinitiv.eta.codec.DataTypes.DATETIME:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.DateTime)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		case com.refinitiv.eta.codec.DataTypes.DATE:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Date)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		case com.refinitiv.eta.codec.DataTypes.TIME:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Time)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		case com.refinitiv.eta.codec.DataTypes.QOS:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Qos)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		case com.refinitiv.eta.codec.DataTypes.STATE:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.State)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		case com.refinitiv.eta.codec.DataTypes.ENUM:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Enum)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		case com.refinitiv.eta.codec.DataTypes.BUFFER:
		case com.refinitiv.eta.codec.DataTypes.UTF8_STRING:
		case com.refinitiv.eta.codec.DataTypes.ASCII_STRING:
		case com.refinitiv.eta.codec.DataTypes.RMTES_STRING:
			 while ((ret =  rsslFieldEntry.encode(_rsslEncodeIter, (com.refinitiv.eta.codec.Buffer)cacheEntryData)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
				 	_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			    }
			 return ret;
		 default:
			return CodecReturnCodes.FAILURE;
		}
	}
	
	private FieldEntryImpl fieldEntryInstance()
	{
		FieldEntryImpl retData = (FieldEntryImpl)_objManager._fieldEntryPool.poll();
        if(retData == null)
        {
        	retData = new FieldEntryImpl(com.refinitiv.eta.codec.CodecFactory.createFieldEntry(), noDataInstance());
        	_objManager._fieldEntryPool.updatePool(retData);
        }
        else
        	retData._rsslFieldEntry.clear();
        
        return retData;
	}
	
	private void clearCollection()
	{
		int collectionSize = _fieldListCollection.size();
		if (collectionSize > 0)
		{
			for (int index = 0; index < collectionSize; ++index)
			{
				((FieldEntryImpl)_fieldListCollection.get(index)).returnToPool();
			}
	
			_fieldListCollection.clear();
		}
	}
}