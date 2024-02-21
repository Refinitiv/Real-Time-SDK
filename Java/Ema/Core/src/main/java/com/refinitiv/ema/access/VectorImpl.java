///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019, 2024 Refinitiv. All rights reserved.        --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

import com.refinitiv.ema.access.DataType.DataTypes;
import com.refinitiv.ema.access.OmmError.ErrorCode;
import com.refinitiv.ema.rdm.DataDictionary;
import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;
import com.refinitiv.eta.codec.VectorEntryActions;
import com.refinitiv.eta.codec.Codec;

class VectorImpl extends CollectionDataImpl implements Vector
{
	private com.refinitiv.eta.codec.Vector	_rsslVector = com.refinitiv.eta.codec.CodecFactory.createVector();
	private LinkedList<VectorEntry> _vectorCollection = new LinkedList<VectorEntry>(); 
	private DataImpl _summaryDecoded;
	private PayloadAttribSummaryImpl _summaryData;
	private int _summaryDataType = com.refinitiv.eta.codec.DataTypes.NO_DATA;
	private boolean _summaryDataTypeSet = false;
	private VectorIterImpl _vectorIterImpl = null;
	
	VectorImpl() 
	{
		super(null);
	}
	
	VectorImpl(EmaObjectManager objManager)
	{
		super(objManager);
		if (objManager != null)
			_summaryDecoded = noDataInstance();
	} 

	@Override
	public int dataType()
	{
		return DataTypes.VECTOR;
	}

	@Override
	public boolean hasTotalCountHint()
	{
		return _rsslVector.checkHasTotalCountHint();
	}

	@Override
	public boolean sortable()
	{
		return _rsslVector.checkSupportsSorting();
	}

	@Override
	public int totalCountHint()
	{
		if (!hasTotalCountHint())
			throw ommIUExcept().message("Attempt to totalCountHint() while it is not set.", OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		
		return _rsslVector.totalCountHint();
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
	
	@Override
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
			
			_rsslVector.clear();
			_vectorCollection.clear();
		}
		else
			clearCollection();
	}

	@Override
	public Vector sortable(boolean sortable)
	{
		if (sortable)
			_rsslVector.applySupportsSorting();

		return this;
	}

	@Override
	public Vector totalCountHint(int totalCountHint)
	{
		if (totalCountHint < 0 || totalCountHint > 1073741823)
			throw ommOORExcept().message("totalCountHint is out of range [0 - 1073741823].");

		_rsslVector.applyHasTotalCountHint();
		_rsslVector.totalCountHint(totalCountHint);

		return this;
	}

	@Override
	public Vector summaryData(ComplexType summaryData)
	{
		if (summaryData == null)
			throw ommIUExcept().message("Passed in summaryData is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		_summaryDataType = summaryData.dataType();
		_summaryDataTypeSet = true;

		_rsslVector.applyHasSummaryData();
		Utilities.copy(((DataImpl) summaryData).encodedData(), _rsslVector.encodedSummaryData());

		return this;
	}

	@Override
	public boolean isEmpty()
	{
		if (_fillCollection)
			fillCollection();
		return _vectorCollection.isEmpty();
	}

	@Override
	public Iterator<VectorEntry> iterator()
	{
		if (_fillCollection)
			fillCollection();
		
		return new EmaIterator<VectorEntry>(_vectorCollection.iterator());
	}
	
	@Override
	public Iterator<VectorEntry> iteratorByRef()
	{
		if (_vectorIterImpl == null)
			_vectorIterImpl = new VectorIterImpl(this);
		else
			_vectorIterImpl.clear();
		return _vectorIterImpl;
	}

	@Override
	public int size()
	{
		if (_fillCollection)
			fillCollection();
		return _vectorCollection.size();
	}
	
	@Override
	public boolean add(VectorEntry vectorEntry)
	{
		if (vectorEntry == null)
			throw new NullPointerException("Passed in vectorEntry is null.");

		return _vectorCollection.add(vectorEntry);
	}

	@Override
	public boolean addAll(Collection<? extends VectorEntry> c)
	{
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public boolean contains(Object o)
	{
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public boolean containsAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public boolean remove(Object o)
	{
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public boolean removeAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public boolean retainAll(Collection<?> c)
	{
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public Object[] toArray()
	{
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public <T> T[] toArray(T[] a)
	{
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public String toString(DataDictionary dictionary)
	{
		if (!dictionary.isFieldDictionaryLoaded() || !dictionary.isEnumTypeDefLoaded())
			return "\nDictionary is not loaded.\n";

		if (_objManager == null)
		{
			_objManager = new EmaObjectManager();
			_objManager.initialize(((DataImpl)this).dataType());
		}

		Vector vector = new VectorImpl(_objManager);

		((CollectionDataImpl) vector).decode(((DataImpl)this).encodedData(), Codec.majorVersion(), Codec.minorVersion(), ((DataDictionaryImpl)dictionary).rsslDataDictionary(), null);
		if (_errorCode != ErrorCode.NO_ERROR)
		{
			return "\nFailed to decode Vector with error: " + ((CollectionDataImpl) vector).errorString() + "\n";
		}

		return vector.toString();
	}

	String toString(int indent)
	{
		if ( _objManager == null )
			return "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n";

		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("Vector");
				
		_toString.append(" sortable=\"").append((sortable() ? "true" : "false")).append("\"");
		
		if (hasTotalCountHint())
			_toString.append(" totalCountHint=\"").append(totalCountHint()).append("\"");

		if (_rsslVector.checkHasSummaryData())
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
		
		if ( _vectorCollection.isEmpty() )
		{
			Utilities.addIndent(_toString.append("\n"), indent).append("VectorEnd\n");
			return _toString.toString();
		}
		
		++indent;
		
		DataImpl load;
		for (VectorEntry vectorEntry : _vectorCollection)
		{
			load = (DataImpl) vectorEntry.load();
			if ( load == null )
				return "\ntoString() method could not be used for just encoded object. Use toString(dictionary) for just encoded object.\n";
			
			Utilities.addIndent(_toString.append("\n"), indent).append("VectorEntry action=\"")
					.append(vectorEntry.vectorActionAsString()).append("\" index=\"").append(vectorEntry.position());
			
			if (vectorEntry.hasPermissionData())
			{
				_toString.append(" permissionData=\"");
				Utilities.asHexString(_toString, vectorEntry.permissionData()).append("\"");
			}
				
			_toString.append(" dataType=\"").append(DataType.asString(load.dataType())).append("\"\n");
			
			++indent;
			_toString.append(load.toString(indent));
			--indent;
			
			Utilities.addIndent(_toString, indent).append("VectorEntryEnd");
		}

		--indent;

		Utilities.addIndent(_toString.append("\n"), indent).append("VectorEnd\n");

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
		
		retCode = _rsslVector.decode(_rsslDecodeIter);
		switch (retCode)
		{
			case com.refinitiv.eta.codec.CodecReturnCodes.NO_DATA :
				_errorCode = ErrorCode.NO_ERROR;
				_rsslVector.flags(0);
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
			if (_rsslVector.checkHasSetDefs())
			{
				switch (_rsslVector.containerType())
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

			int rsslContainerType = _rsslVector.checkHasSummaryData() ? _rsslVector.containerType() : com.refinitiv.eta.codec.DataTypes.NO_DATA;
			int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, _rsslVector.encodedSummaryData());
			_summaryDecoded = dataInstance(_summaryDecoded, dType);
			_summaryDecoded.decode(_rsslVector.encodedSummaryData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
		}
	}
	
	void fillCollection()
	{
		DataImpl load;
	
		clearCollection();
		
		VectorEntryImpl vectorEntry = vectorEntryInstance();
		
		if ( ErrorCode.NO_ERROR != _errorCode)
		{
			load =  dataInstance(vectorEntry._load, DataTypes.ERROR);
			load.decode(_rsslBuffer, _errorCode);
			vectorEntry._load = load;
			_vectorCollection.add(vectorEntry);
			_fillCollection = false;
			return;
		}

		int retCode;
		while ((retCode  = vectorEntry._rsslVectorEntry.decode(_rsslDecodeIter)) != com.refinitiv.eta.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
			case com.refinitiv.eta.codec.CodecReturnCodes.SUCCESS :
				int rsslContainerType = (vectorEntry._rsslVectorEntry.action() != VectorEntryActions.DELETE && vectorEntry._rsslVectorEntry.action() != VectorEntryActions.CLEAR)?
														_rsslVector.containerType() : com.refinitiv.eta.codec.DataTypes.NO_DATA;
				int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, vectorEntry._rsslVectorEntry.encodedData());
				load = dataInstance(vectorEntry._load, dType);
				load.decode(vectorEntry._rsslVectorEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.INCOMPLETE_DATA :
				load = dataInstance(vectorEntry._load, DataTypes.ERROR);
				load.decode(vectorEntry._rsslVectorEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
				break;
			case com.refinitiv.eta.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				load = dataInstance(vectorEntry._load, DataTypes.ERROR);
				load.decode(vectorEntry._rsslVectorEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
				break;
			default :
				load = dataInstance(vectorEntry._load, DataTypes.ERROR);
				load.decode(vectorEntry._rsslVectorEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
				break;
			}
			
			vectorEntry._load = load;
			_vectorCollection.add(vectorEntry);
			
			vectorEntry = vectorEntryInstance();
		}
		
		vectorEntry.returnToPool();
		
		_fillCollection = false;
	}


	Buffer encodedData()
	{
		if (_encodeComplete || (_rsslEncodeIter == null))
			return _rsslBuffer;
		
		int ret;
		int entryType = _summaryDataType;
		
		if (_vectorCollection.isEmpty())
		{
			_rsslVector.containerType(entryType);
		}
		else
		{
		    VectorEntryImpl firstEntry = (VectorEntryImpl)_vectorCollection.get(0);
		    
		    if ( firstEntry._entryDataType != com.refinitiv.eta.codec.DataTypes.UNKNOWN )
		    {
		    	if ( _summaryDataTypeSet && (entryType !=  firstEntry._entryDataType) )
				{
					String errText = errorString().append("Attempt to add entry of ")
							.append(com.refinitiv.eta.codec.DataTypes.toString(firstEntry._entryDataType))
							.append(" while Vector entry load type is set to ")
							.append(com.refinitiv.eta.codec.DataTypes.toString(entryType))
							.append(" with summaryData() method").toString();
					throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
				}
				else
				{
					entryType = firstEntry._entryDataType;
				}
		    	
		    	_rsslVector.containerType(entryType);
		    }
		    else
		    {
		    	entryType = com.refinitiv.eta.codec.DataTypes.UNKNOWN;
		    	_rsslVector.containerType(Utilities.toRsslDataType(firstEntry.loadType()));
		    }
		}
		    
	    setEncodedBufferIterator();

	    while ((ret = _rsslVector.encodeInit(_rsslEncodeIter, 0, 0)) == CodecReturnCodes.BUFFER_TOO_SMALL)
	    {
	    	Buffer bigBuffer = CodecFactory.createBuffer();
			bigBuffer.data(ByteBuffer.allocate(_rsslBuffer.capacity() * 2));
			_rsslBuffer = bigBuffer;

			setEncodedBufferIterator();
	    }
	    
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to intialize encoding on rssl Vector. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText, ret);
	    }
	    
	    VectorEntryImpl vectorEntry;
		for (com.refinitiv.ema.access.VectorEntry entry  : _vectorCollection)
		{
			vectorEntry = ((VectorEntryImpl)entry);
			if (entryType != vectorEntry._entryDataType)
			{
				String errText = errorString().append("Attempt to add entry of ")
						.append(com.refinitiv.eta.codec.DataTypes.toString(vectorEntry._entryDataType))
						.append(" while Vector contains=")
						.append(com.refinitiv.eta.codec.DataTypes.toString(entryType)).toString();
				throw ommIUExcept().message(errText, OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
			}
			
			while ((ret = vectorEntry._rsslVectorEntry.encode(_rsslEncodeIter)) == CodecReturnCodes.BUFFER_TOO_SMALL)
			{
				_rsslBuffer = Utilities.realignBuffer(_rsslEncodeIter, _rsslBuffer.capacity() * 2);
			}

			if (ret != CodecReturnCodes.SUCCESS)
		    {
				String errText = errorString().append("Failed to ")
						.append("rsslVectorEntry.encode()")
						.append(" while encoding rssl Vector. Reason='")
						.append(CodecReturnCodes.toString(ret))
						.append("'").toString();
				throw ommIUExcept().message(errText, ret);
		    }
		}
		 
		ret =  _rsslVector.encodeComplete(_rsslEncodeIter, true);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to complete encoding on rssl Vector. Reason='")
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
	
	private VectorEntryImpl vectorEntryInstance()
	{
		VectorEntryImpl retData = (VectorEntryImpl)_objManager._vectorEntryPool.poll();
        if(retData == null)
        {
        	retData = new VectorEntryImpl(com.refinitiv.eta.codec.CodecFactory.createVectorEntry(), noDataInstance(), _objManager);
        	_objManager._vectorEntryPool.updatePool(retData);
        }
        else
        	retData._rsslVectorEntry.clear();
        
        return retData;
	}
	
	private void clearCollection()
	{
		int collectionSize = _vectorCollection.size();
		if (collectionSize > 0)
		{
			for (int index = 0; index < collectionSize; ++index)
				((VectorEntryImpl)_vectorCollection.get(index)).returnToPool();
	
			_vectorCollection.clear();
		}
	}
}
