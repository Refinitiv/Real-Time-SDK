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
import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.SummaryData;
import com.thomsonreuters.ema.access.Vector;
import com.thomsonreuters.ema.access.VectorEntry;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.VectorEntryActions;

public class VectorImpl extends ComplexTypeImpl implements Vector
{
	private com.thomsonreuters.upa.codec.Vector	_rsslVector = com.thomsonreuters.upa.codec.CodecFactory.createVector();
	private com.thomsonreuters.upa.codec.VectorEntry _rsslVectorEntry = com.thomsonreuters.upa.codec.CodecFactory.createVectorEntry();
	private VectorEntryImpl _vectorEntry = new VectorEntryImpl(this, _rsslVectorEntry, null);
	private com.thomsonreuters.upa.codec.Vector	_rsslVectorForCollection;
	private LinkedList<VectorEntry> _vectorCollection = new LinkedList<VectorEntry>(); 
	private DataImpl _summaryDecoded = new NoDataImpl();
	private PayloadAttribSummaryImpl _summaryData;
	
	public VectorImpl()
	{
		_load = new NoDataImpl();
	}

	@Override
	public int dataType()
	{
		return DataTypes.VECTOR;
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
			_vectorEntry.load(_load);
			return true;
		}

		_decodingStarted = true;

		int retCode = _rsslVectorEntry.decode(_rsslDecodeIter);

		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			{
				int rsslContainerType = (_rsslVectorEntry.action() != VectorEntryActions.DELETE)?
							_rsslVector.containerType() : com.thomsonreuters.upa.codec.DataTypes.NO_DATA;
							
				int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, _rsslVectorEntry.encodedData());
							
				_load = dataInstance(_load, dType);
				_load.decode(_rsslVectorEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
				
				_vectorEntry.load(_load);
				
				return true;
			}
			case com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER :
				_atEnd = true;
				return false;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslVectorEntry.encodedData(), ErrorCode.INCOMPLETE_DATA); 
				_vectorEntry.load(_load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslVectorEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				_vectorEntry.load(_load);
				return true;
			default :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslVectorEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				_vectorEntry.load(_load);
				return true;
		}
	}

	@Override
	public void reset()
	{
		// TODO Auto-generated method stub
	}

	@Override
	public boolean hasTotalCountHint()
	{
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean sortable()
	{
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public int totalCountHint()
	{
		// TODO Auto-generated method stub
		return 0;
	}

	@Override
	public SummaryData summaryData()
	{
		if (_summaryData == null)
			_summaryData = new PayloadAttribSummaryImpl();
		
		_summaryData.data(_summaryDecoded);
		return (SummaryData)_summaryData;
	}

	@Override
	public VectorEntry entry()
	{
		if (!_decodingStarted)
			throw oommIUExcept().message("Attempt to entry() while decoding entry was NOT started.");
		
		return _vectorEntry;
	}
	
	@Override
	public String toString()
	{
		return toString(0);
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("Vector");
				
		_toString.append(" sortable=\"").append((sortable() ? "true" : "false")).append("\"");
		
		if (hasTotalCountHint())
			_toString.append(" totalCountHint=\"").append(totalCountHint()).append("\"");
		
		if (hasSummary())
		{
			++indent;
			Utilities.addIndent(_toString.append("\n"), indent).append("SummaryData dataType=\"")
					 .append(DataType.asString(summaryData().dataType())).append("\"\n");
			
			++indent;
			_toString.append(((DataImpl)summary()).toString(indent));
			--indent;
			
			Utilities.addIndent(_toString, indent).append("SummaryDataEnd");
			--indent;
		}
		
		++indent;
			
		while (forth())
		{
			Utilities.addIndent(_toString.append("\n"), indent).append("VectorEntry action=\"")
					.append(_vectorEntry.vectorActionAsString()).append("\" index=\"").append(_vectorEntry.position());
			
			if (_vectorEntry.hasPermissionData())
			{
				_toString.append(" permissionData=\"");
				Utilities.asHexString(_toString, _vectorEntry.permissionData()).append("\"");
			}
				
			_toString.append(" dataType=\"").append(DataType.asString(_load.dataType())).append("\"\n");
			
			++indent;
			_toString.append(_load.toString(indent));
			--indent;
			
			Utilities.addIndent(_toString, indent).append("VectorEntryEnd");
		}

		--indent;

		Utilities.addIndent(_toString.append("\n"), indent).append("VectorEnd\n");

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

		if (_rsslDictionary == null)
		{
			_atEnd = false;
			_errorCode = ErrorCode.NO_DICTIONARY;
			return;
		}

		_rsslDecodeIter.clear();
		
		_vectorCollection.clear();
		
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_atEnd = false;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslVector.decode(_rsslDecodeIter);
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

		if (_errorCode == ErrorCode.NO_ERROR)
		{
			if (_rsslVector.checkHasSetDefs())
			{
				switch (_rsslVector.containerType())
				{
					case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST :
					{
						if (_rsslLocalFLSetDefDb != null)
							_rsslLocalFLSetDefDb.clear();
						else
						{
							//TODO need pooling
							if (GlobalPool._rsslFieldListSetDefList.size() > 0)
								_rsslLocalFLSetDefDb = GlobalPool._rsslFieldListSetDefList.get(0);
							else
								_rsslLocalFLSetDefDb = CodecFactory.createLocalFieldSetDefDb();
						}
						
						_rsslLocalFLSetDefDb.decode(_rsslDecodeIter);
						_rsslLocalSetDefDb = _rsslLocalFLSetDefDb;
						break;
					}
					case com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST :
					{
						if (_rsslLocalELSetDefDb != null)
							_rsslLocalELSetDefDb.clear();
						else
						{
							if (GlobalPool._rsslElementListSetDefList.size() > 0)
								_rsslLocalELSetDefDb = GlobalPool._rsslElementListSetDefList.get(0);
							else
								_rsslLocalELSetDefDb = CodecFactory.createLocalElementSetDefDb();
						}
						
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

			int rsslContainerType = _rsslVector.checkHasSummaryData() ? _rsslVector.containerType() : com.thomsonreuters.upa.codec.DataTypes.NO_DATA;
			int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, _rsslVector.encodedSummaryData());
			_summaryDecoded = dataInstance(_summaryDecoded, dType);
			_summaryDecoded.decode(_rsslVector.encodedSummaryData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
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
		
		if (_rsslVectorForCollection == null)
			_rsslVectorForCollection = com.thomsonreuters.upa.codec.CodecFactory.createVector();
		
		if ((retCode = _rsslVectorForCollection.decode(_rsslDecodeIterForCollection)) != CodecReturnCodes.SUCCESS)
			return;
		
		VectorEntryImpl vectorEntry;
		DataImpl load;
		com.thomsonreuters.upa.codec.VectorEntry rsslVectorEntry = com.thomsonreuters.upa.codec.CodecFactory.createVectorEntry();

		while ((retCode  = rsslVectorEntry.decode(_rsslDecodeIterForCollection)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			if (retCode != CodecReturnCodes.SUCCESS)
				return;
		
			int rsslContainerType = (rsslVectorEntry.action() != VectorEntryActions.DELETE)?
					_rsslVectorForCollection.containerType() : com.thomsonreuters.upa.codec.DataTypes.NO_DATA;
			int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, rsslVectorEntry.encodedData());
			load = dataInstance(dType);
			load.decode(rsslVectorEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
			
			vectorEntry = new VectorEntryImpl(this, rsslVectorEntry, load);
			_vectorCollection.add(vectorEntry);
			
			rsslVectorEntry = com.thomsonreuters.upa.codec.CodecFactory.createVectorEntry();
		}
		
		_fillCollection = false;
	}
	
	boolean hasSummary()
	{
		return _rsslVector.checkHasSummaryData();
	}
	
	Data summary()
	{
		return _summaryDecoded;
	}
	
	@Override
	public void clear()
	{
		// TODO Auto-generated method stub
	}

	@Override
	public Vector add(int position, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Vector add(int position, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Vector complete()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Vector sortable(boolean sortable)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Vector totalCountHint(int totalCountHint)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Vector summaryData(ComplexType data)
	{
		// TODO Auto-generated method stub
		return null;
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
	public int size()
	{
		if (_fillCollection)
			fillCollection();
		return _vectorCollection.size();
	}
	
	@Override
	public boolean add(VectorEntry e)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public boolean addAll(Collection<? extends VectorEntry> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public boolean contains(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public boolean containsAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public boolean remove(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public boolean removeAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public boolean retainAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public Object[] toArray()
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}

	@Override
	public <T> T[] toArray(T[] a)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Vector collection doesn't support this operation.");
	}
}