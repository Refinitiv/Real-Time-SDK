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
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.MapEntryActions;

class MapImpl extends CollectionDataImpl implements Map
{
	private com.thomsonreuters.upa.codec.Map _rsslMap = com.thomsonreuters.upa.codec.CodecFactory.createMap();
	private com.thomsonreuters.upa.codec.DecodeIterator _keyDecodeIter = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
	private LinkedList<MapEntry> _mapCollection = new LinkedList<MapEntry>(); 
	private DataImpl _summaryDecoded = noDataInstance();
	private PayloadAttribSummaryImpl _summaryData;
	
	MapImpl() 
	{
		super(false);
	}
	
	MapImpl(boolean decoding)
	{
		super(decoding);
	} 
	
	@Override
	public int dataType()
	{
		return DataTypes.MAP;
	}

	@Override
	public boolean hasKeyFieldId()
	{
		return _rsslMap.checkHasKeyFieldId();
	}

	@Override
	public boolean hasTotalCountHint()
	{
		return _rsslMap.checkHasTotalCountHint();
	}

	@Override
	public int keyFieldId()
	{
		if (!hasKeyFieldId())
			throw ommIUExcept().message("Attempt to keyFieldId() while it is not set.");
		
		return _rsslMap.keyFieldId();
	}

	@Override
	public int totalCountHint()
	{
		if (!hasTotalCountHint())
			throw ommIUExcept().message("Attempt to totalCountHint() while it is not set.");
		
		return _rsslMap.totalCountHint();
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
	public String toString()
	{
		return toString(0);
	}
	

	@Override
	public boolean isEmpty()
	{
		if (_fillCollection)
			fillCollection();
		return _mapCollection.isEmpty();
	}

	@Override
	public Iterator<MapEntry> iterator()
	{
		if (_fillCollection)
			fillCollection();
		
		return new EmaIterator<MapEntry>(_mapCollection.iterator());
	}

	@Override
	public int size()
	{
		if (_fillCollection)
			fillCollection();
		return _mapCollection.size();
	}
	
	@Override
	public boolean add(MapEntry mapEntry)
	{
		if (mapEntry == null)
			throw new NullPointerException("Passed in mapEntry is null.");
		
		return _mapCollection.add(mapEntry);
	}

	@Override
	public boolean addAll(Collection<? extends MapEntry> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Map collection doesn't support this operation.");
	}

	@Override
	public boolean contains(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Map collection doesn't support this operation.");
	}

	@Override
	public boolean containsAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Map collection doesn't support this operation.");
	}

	@Override
	public boolean remove(Object o)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Map collection doesn't support this operation.");
	}

	@Override
	public boolean removeAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Map collection doesn't support this operation.");
	}

	@Override
	public boolean retainAll(Collection<?> c)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Map collection doesn't support this operation.");
	}

	@Override
	public Object[] toArray()
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Map collection doesn't support this operation.");
	}

	@Override
	public <T> T[] toArray(T[] a)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Map collection doesn't support this operation.");
	}
	
	@Override
	public void clear()
	{
		super.clear();
		
		_rsslMap.clear();
		
		if (_rsslEncodeIter != null)
			_mapCollection.clear();
	}

	@Override
	public Map keyFieldId(int keyFieldId)
	{
		if (keyFieldId < -32768 || keyFieldId > 32767)
			throw ommOORExcept().message("keyFieldId is out of range [(-32768) - 32767].");

		_rsslMap.applyHasKeyFieldId();
		_rsslMap.keyFieldId(keyFieldId);
		
		return this;
	}

	@Override
	public Map totalCountHint(int totalCountHint)
	{
		if (totalCountHint < 0 || totalCountHint > 1073741823)
			throw ommOORExcept().message("totalCountHint is out of range [0 - 1073741823].");

		_rsslMap.applyHasTotalCountHint();
		_rsslMap.totalCountHint(totalCountHint);
		
		return this;
	}

	@Override
	public Map summaryData(ComplexType summaryData)
	{
		if (summaryData == null)
			throw ommIUExcept().message("Passed in summaryData is null");

		_rsslMap.applyHasSummaryData();
		Utilities.copy(((DataImpl)summaryData).encodedData(), _rsslMap.encodedSummaryData());
		
		return this;
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
		
		retCode = _rsslMap.decode(_rsslDecodeIter);
		switch (retCode)
		{
		case com.thomsonreuters.upa.codec.CodecReturnCodes.NO_DATA :
			_errorCode = ErrorCode.NO_ERROR;
			_rsslMap.flags(0);
			_fillCollection = false;
			clearCollection();
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

		if (_errorCode == ErrorCode.NO_ERROR)
		{
			if (_rsslMap.checkHasSetDefs())
			{
				switch (_rsslMap.containerType())
				{
					case com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST :
					{
						if (_rsslLocalFLSetDefDb != null)
							_rsslLocalFLSetDefDb.clear();
						else
						{
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

			int rsslContainerType = _rsslMap.checkHasSummaryData() ? _rsslMap.containerType() : com.thomsonreuters.upa.codec.DataTypes.NO_DATA;
			int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, _rsslMap.encodedSummaryData());
			_summaryDecoded = dataInstance(_summaryDecoded, dType);
			_summaryDecoded.decode(_rsslMap.encodedSummaryData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
		}
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("Map");
				
		if (hasTotalCountHint())
			_toString.append(" totalCountHint=\"").append(totalCountHint()).append("\"");
		
		if (hasKeyFieldId())
			_toString.append(" keyFieldId=\"").append(keyFieldId()).append("\"");
		
		if (_rsslMap.checkHasSummaryData())
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
		
		if ( _mapCollection.isEmpty() )
		{
			Utilities.addIndent(_toString.append("\n"), indent).append("MapEnd\n");
			return _toString.toString();
		}
		
		++indent;
		
		DataImpl load;
		DataImpl key;
		for (MapEntry mapEntry : _mapCollection)
		{
			load = (DataImpl)mapEntry.load();
			key = ((MapEntryImpl)mapEntry).decodedKey();
			Utilities.addIndent(_toString.append("\n"), indent).append("MapEntry action=\"")
				  .append(mapEntry.mapActionAsString()).append("\" key dataType=\"")
				  .append(DataType.asString(key.dataType()));
			
			if (key.dataType() == DataTypes.BUFFER)
			{
				_toString.append("\" value=\n\n").append(key.toString());
				Utilities.addIndent(_toString.append("\n"), indent);											  
			}
			else
				_toString.append("\" value=\n\n").append(key.toString()).append("\"");

			if (mapEntry.hasPermissionData())
			{
				_toString.append(" permissionData=\"");
				Utilities.asHexString(_toString, mapEntry.permissionData()).append("\"");
			}
				
			_toString.append(" dataType=\"").append(DataType.asString(load.dataType()))
					 .append("\"\n");
			
			++indent;
			_toString.append(load.toString(indent));
			--indent;
			
			Utilities.addIndent(_toString, indent).append("MapEntryEnd");
		}

		--indent;

		Utilities.addIndent(_toString.append("\n"), indent).append("MapEnd\n");

		return _toString.toString();
	}
	
	void fillCollection()
	{
		DataImpl load;
		DataImpl mapEntryKey;
		
		clearCollection();
		
		MapEntryImpl mapEntry = mapEntryInstance();
		
		if ( ErrorCode.NO_ERROR != _errorCode)
		{
			load =  dataInstance(mapEntry._load, DataTypes.ERROR);
			load.decode(_rsslBuffer, _errorCode);
			
			mapEntryKey = dataInstance(((DataImpl)mapEntry._keyDataDecoded.data()), DataTypes.NO_DATA);
			
			_mapCollection.add(mapEntry.entryValue( mapEntryKey, load));
			_fillCollection = false;
			return;
		}

		int retCode;
		while ((retCode  = mapEntry._rsslMapEntry.decode(_rsslDecodeIter, null)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			switch(retCode)
			{
				case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
					_keyDecodeIter.clear();
					retCode = _keyDecodeIter.setBufferAndRWFVersion( mapEntry._rsslMapEntry.encodedKey(), _rsslMajVer, _rsslMinVer);
					if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
					{
						_errorCode = OmmError.ErrorCode.ITERATOR_SET_FAILURE;
						load =  dataInstance(mapEntry._load, DataTypes.ERROR);
						load.decode(_rsslBuffer, _errorCode);
						
						mapEntryKey = dataInstance(((DataImpl)mapEntry._keyDataDecoded.data()), DataTypes.NO_DATA);
						
						_mapCollection.add(mapEntry.entryValue( mapEntryKey, load));
						_fillCollection = false;
						return;
					}
					
					mapEntryKey = dataInstance(((DataImpl)mapEntry._keyDataDecoded.data()), Utilities.toEmaDataType[_rsslMap.keyPrimitiveType()]);
					mapEntryKey.decode(mapEntry._rsslMapEntry.encodedKey(), _keyDecodeIter);
					
					int rsslContainerType = ( mapEntry._rsslMapEntry.action() != MapEntryActions.DELETE)? _rsslMap.containerType() : com.thomsonreuters.upa.codec.DataTypes.NO_DATA;
					int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer,  mapEntry._rsslMapEntry.encodedData());
					
					load = dataInstance(mapEntry._load, dType);
					load.decode( mapEntry._rsslMapEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
					break;
				case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
					mapEntryKey = dataInstance(((DataImpl)mapEntry._keyDataDecoded.data()), DataTypes.NO_DATA);
					load = dataInstance(mapEntry._load, DataTypes.ERROR);
					load.decode( mapEntry._rsslMapEntry.encodedData(),ErrorCode.INCOMPLETE_DATA);
					break;
				case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
					mapEntryKey = dataInstance(((DataImpl)mapEntry._keyDataDecoded.data()), DataTypes.NO_DATA);
					load = dataInstance(mapEntry._load, DataTypes.ERROR);
					load.decode( mapEntry._rsslMapEntry.encodedData(),ErrorCode.UNSUPPORTED_DATA_TYPE);
					break;
				default :
					mapEntryKey = dataInstance(((DataImpl)mapEntry._keyDataDecoded.data()), DataTypes.NO_DATA);
					load = dataInstance(mapEntry._load, DataTypes.ERROR);
					load.decode( mapEntry._rsslMapEntry.encodedData(),ErrorCode.UNKNOWN_ERROR);
					break;
				}
			
				_mapCollection.add(mapEntry.entryValue( mapEntryKey, load));
				mapEntry = mapEntryInstance();
			}
		
		mapEntry.returnToPool();
		
		_fillCollection = false;
	}
	
	Buffer encodedData()
	{
		if (_encodeComplete)
			return _rsslBuffer; 
		
		if (_mapCollection.isEmpty())
			throw ommIUExcept().message("Map to be encoded is empty.");
		
		int ret = _rsslEncodeIter.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to setBufferAndRWFVersion on rssl encode iterator. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText);
	    }
	    
	    MapEntryImpl firstEntry = (MapEntryImpl)_mapCollection.get(0);
	    int keyType = firstEntry._keyDataType; 
	    int entryType = firstEntry._entryDataType;
		_rsslMap.keyPrimitiveType(keyType);
		_rsslMap.containerType(entryType);
	 
	    ret = _rsslMap.encodeInit(_rsslEncodeIter, 0, 0);
	    while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
	    {
	    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
	    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
	    	ret = _rsslMap.encodeInit(_rsslEncodeIter, 0, 0);
	    }
	    
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to intialize encoding on rssl map. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	    	throw ommIUExcept().message(errText);
	    }
	    
	    MapEntryImpl mapEntry;
		for (com.thomsonreuters.ema.access.MapEntry entry  : _mapCollection)
		{
			mapEntry = ((MapEntryImpl)entry);
			if (keyType != mapEntry._keyDataType)
			{
				String errText = errorString().append("Attempt to add entry of ")
						.append(com.thomsonreuters.upa.codec.DataTypes.toString(mapEntry._keyDataType))
						.append("while Map key contains=")
						.append(com.thomsonreuters.upa.codec.DataTypes.toString(keyType)).toString();
				throw ommIUExcept().message(errText);
			}
			
			if (entryType != mapEntry._entryDataType)
			{
				String errText = errorString().append("Attempt to add entry of ")
						.append(com.thomsonreuters.upa.codec.DataTypes.toString(mapEntry._entryDataType))
						.append("while Map contains=")
						.append(com.thomsonreuters.upa.codec.DataTypes.toString(entryType)).toString();
				throw ommIUExcept().message(errText);
			}
			
			if ((ret = mapEntryEncode(keyType, mapEntry._rsslMapEntry,  mapEntry._keyData)) != CodecReturnCodes.SUCCESS)
		    {
		    	String errText = errorString().append("Failed to ")
		    								.append("rsslMapEntry.encode()")
		    								.append(" while encoding rssl map. Reason='")
		    								.append(CodecReturnCodes.toString(ret))
		    								.append("'").toString();
		    	throw ommIUExcept().message(errText);
		    }
		 }
		 
		ret =  _rsslMap.encodeComplete(_rsslEncodeIter, true);
	    if (ret != CodecReturnCodes.SUCCESS)
	    {
	    	String errText = errorString().append("Failed to complete encoding on rssl map. Reason='")
	    								.append(CodecReturnCodes.toString(ret))
	    								.append("'").toString();
	        throw ommIUExcept().message(errText);
	    }
	    
	    _encodeComplete = true;
	    return _rsslBuffer;
	}
	
	private int mapEntryEncode(int rsslDataType, com.thomsonreuters.upa.codec.MapEntry rsslMapEntry, Object cacheKeyData)
	{
		int ret;
		if ( cacheKeyData == null )
			throw ommIUExcept().message("Map entry key to be encoded is empty.");
		
		switch (rsslDataType)
		{
		case com.thomsonreuters.upa.codec.DataTypes.INT:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Int)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Int)cacheKeyData);
			    }
			 return ret;
		case com.thomsonreuters.upa.codec.DataTypes.UINT:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.UInt)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.UInt)cacheKeyData);
			    }
			 return ret;
		case com.thomsonreuters.upa.codec.DataTypes.REAL:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Real)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Real)cacheKeyData);
			    }
			 return ret;
		case com.thomsonreuters.upa.codec.DataTypes.DOUBLE:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Double)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Double)cacheKeyData);
			    }
			 return ret;
		case com.thomsonreuters.upa.codec.DataTypes.FLOAT:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Float)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Float)cacheKeyData);
			    }
			 return ret;
		case com.thomsonreuters.upa.codec.DataTypes.DATETIME:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.DateTime)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.DateTime)cacheKeyData);
			    }
			 return ret;
		case com.thomsonreuters.upa.codec.DataTypes.DATE:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Date)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Date)cacheKeyData);
			    }
			 return ret;
		case com.thomsonreuters.upa.codec.DataTypes.TIME:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Time)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Time)cacheKeyData);
			    }
			 return ret;
		case com.thomsonreuters.upa.codec.DataTypes.QOS:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Qos)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Qos)cacheKeyData);
			    }
			 return ret;
		case com.thomsonreuters.upa.codec.DataTypes.STATE:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.State)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.State)cacheKeyData);
			    }
			 return ret;
		case com.thomsonreuters.upa.codec.DataTypes.ENUM:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Enum)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Enum)cacheKeyData);
			    }
			 return ret;
		case com.thomsonreuters.upa.codec.DataTypes.BUFFER:
		case com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING:
		case com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING:
		case com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING:
			ret =  rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Buffer)cacheKeyData);
			 while (ret == CodecReturnCodes.BUFFER_TOO_SMALL)
			    {
			    	_rsslBuffer.data(ByteBuffer.allocate(_rsslBuffer.data().capacity()*2)); 
			    	_rsslEncodeIter.realignBuffer(_rsslBuffer);
			    	ret = rsslMapEntry.encode(_rsslEncodeIter, (com.thomsonreuters.upa.codec.Buffer)cacheKeyData);
			    }
			 return ret;
		 default:
			return CodecReturnCodes.FAILURE;
		}
	}
	
	private MapEntryImpl mapEntryInstance()
	{
		MapEntryImpl retData = (MapEntryImpl)GlobalPool._mapEntryPool.poll();
        if(retData == null)
        {
        	retData = new MapEntryImpl(com.thomsonreuters.upa.codec.CodecFactory.createMapEntry(), noDataInstance(), noDataInstance());
        	GlobalPool._mapEntryPool.updatePool(retData);
        }
        else
        	retData._rsslMapEntry.clear();
        
        return retData;
	}
	
	private void clearCollection()
	{
		if (_mapCollection.size() > 0 )
		{
			Iterator<MapEntry> iter = _mapCollection.iterator();
			while ( iter.hasNext())
				((MapEntryImpl)iter.next()).returnToPool();
				
			_mapCollection.clear();
		}
	}
}