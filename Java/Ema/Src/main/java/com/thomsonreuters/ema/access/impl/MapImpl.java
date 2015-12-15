///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access.impl;

import java.math.BigInteger;
import java.nio.ByteBuffer;
import java.util.Collection;
import java.util.Iterator;
import java.util.LinkedList;

import com.thomsonreuters.ema.access.ComplexType;
import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.Key;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.MapEntry;
import com.thomsonreuters.ema.access.OmmError;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.ema.access.SummaryData;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.upa.codec.DataDictionary;
import com.thomsonreuters.upa.codec.MapEntryActions;

public class MapImpl extends ComplexTypeImpl implements Map
{
	private com.thomsonreuters.upa.codec.Map _rsslMap = com.thomsonreuters.upa.codec.CodecFactory.createMap();
	private com.thomsonreuters.upa.codec.MapEntry _rsslMapEntry = com.thomsonreuters.upa.codec.CodecFactory.createMapEntry();
	private com.thomsonreuters.upa.codec.DecodeIterator _keyDecodeIter = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
	private com.thomsonreuters.upa.codec.Map _rsslMapForCollection;
	private LinkedList<MapEntry> _mapCollection = new LinkedList<MapEntry>(); 
	private com.thomsonreuters.upa.codec.DecodeIterator _keyDecodeIterForCollection;
	private KeyImpl _key = new KeyImpl();
	private DataImpl _keyDecoded = new NoDataImpl();
	private MapEntryImpl _mapEntry = new MapEntryImpl(this, _rsslMapEntry, _keyDecoded, null);
	private DataImpl _summaryDecoded = new NoDataImpl();
	private PayloadAttribSummaryImpl _summaryData;
	
	public MapImpl()
	{
		_load = new NoDataImpl();
	}

	@Override
	public int dataType()
	{
		return DataTypes.MAP;
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
			_mapEntry.load(_load);
			return true;
		}

		_decodingStarted = true;

		int retCode = _rsslMapEntry.decode(_rsslDecodeIter, null);

		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			{
				_keyDecodeIter.clear();

				retCode = _keyDecodeIter.setBufferAndRWFVersion(_rsslMapEntry.encodedKey(), _rsslMajVer, _rsslMinVer);
				if (CodecReturnCodes.SUCCESS != retCode)
				{
					_atEnd = false;
					_errorCode = OmmError.ErrorCode.ITERATOR_SET_FAILURE;
					return true;
				}
			
				_keyDecoded = dataInstance(_keyDecoded, Utilities.toEmaDataType[_rsslMap.keyPrimitiveType()]);
				_keyDecoded.decode(_rsslMapEntry.encodedKey(), _keyDecodeIter);
				
				int rsslContainerType = (_rsslMapEntry.action() != MapEntryActions.DELETE)?
							_rsslMap.containerType() : com.thomsonreuters.upa.codec.DataTypes.NO_DATA;
							
				int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, _rsslMapEntry.encodedData());
							
				_load = dataInstance(_load, dType);
				_load.decode(_rsslMapEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
				
				_mapEntry.load(_load, _keyDecoded);
				
				return true;
			}
			case com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER :
				_atEnd = true;
				return false;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslMapEntry.encodedData(), ErrorCode.INCOMPLETE_DATA); 
				_mapEntry.load(_load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslMapEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				_mapEntry.load(_load);
				return true;
			default :
				_load =  dataInstance(_load, DataTypes.ERROR);
				_load.decode(_rsslMapEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				_mapEntry.load(_load);
				return true;
		}
	}

	@Override
	public void reset()
	{
		// TODO Auto-generated method stub
	}

	@Override
	public MapEntry entry()
	{
		if (!_decodingStarted)
			throw oommIUExcept().message("Attempt to entry() while decoding entry was NOT started.");
		
		return _mapEntry;
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
			throw oommIUExcept().message("Attempt to keyFieldId() while it is not set.");
		
		return _rsslMap.keyFieldId();
	}

	@Override
	public int totalCountHint()
	{
		if (!hasTotalCountHint())
			throw oommIUExcept().message("Attempt to totalCountHint() while it is not set.");
		
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
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("Map");
				
		if (hasTotalCountHint())
			_toString.append(" totalCountHint=\"").append(totalCountHint()).append("\"");
		
		if (hasKeyFieldId())
			_toString.append(" keyFieldId=\"").append(keyFieldId()).append("\"");
		
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
			Utilities.addIndent(_toString.append("\n"), indent).append("MapEntry action=\"")
				  .append(_mapEntry.mapActionAsString()).append("\" key dataType=\"")
				  .append(DataType.asString(_keyDecoded.dataType()));
			
			if (_keyDecoded.dataType() == DataTypes.BUFFER)
			{
				_toString.append("\" value=\n\n").append(_keyDecoded.toString());
				Utilities.addIndent(_toString.append("\n"), indent);											  
			}
			else
				_toString.append("\" value=\n\n").append(_keyDecoded.toString()).append("\"");

			if (_mapEntry.hasPermissionData())
			{
				_toString.append(" permissionData=\"");
				Utilities.asHexString(_toString, _mapEntry.permissionData()).append("\"");
			}
				
			_toString.append(" dataType=\"").append(DataType.asString(_load.dataType()))
					 .append("\"\n");
			
			++indent;
			_toString.append(_load.toString(indent));
			--indent;
			
			Utilities.addIndent(_toString, indent).append("MapEntryEnd");
		}

		--indent;

		Utilities.addIndent(_toString.append("\n"), indent).append("MapEnd\n");

		return _toString.toString();
	}
	
	@Override
	public void clear()
	{
		// TODO Auto-generated method stub
	}

	@Override
	public Map keyFieldId(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map totalCountHint(int totalCountHint)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map summaryData(ComplexType summaryData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyInt(long key, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyUInt(long key, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyUInt(BigInteger key, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyReal(long mantissa, int magnitudeType, int action,
			ComplexType value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyReal(double key, int action, ComplexType value,
			int magnitudeType, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyFloat(float key, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyDouble(double key, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyDate(int year, int month, int day, int action,
			ComplexType value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyTime(int hour, int minute, int second, int millisecond,
			int microsecond, int nanosecond, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyDateTime(int year, int month, int day, int hour,
			int minute, int second, int millisecond, int microsecond,
			int nanosecond, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyQos(int timeliness, int rate, int action,
			ComplexType value, ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyState(int streamState, int dataState, int statusCode,
			String statusText, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyEnum(int key, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyBuffer(ByteBuffer key, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyAscii(String key, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyUtf8(ByteBuffer key, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyRmtes(ByteBuffer key, int action, ComplexType value,
			ByteBuffer permissionData)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map complete()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyInt(long key, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyUInt(long key, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyUInt(BigInteger key, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyReal(long mantissa, int magnitudeType, int action,
			ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyReal(double key, int action, ComplexType value,
			int magnitudeType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyFloat(float key, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyDouble(double key, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyDate(int year, int month, int day, int action,
			ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyTime(int hour, int minute, int second, int millisecond,
			int microsecond, int nanosecond, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyDateTime(int year, int month, int day, int hour,
			int minute, int second, int millisecond, int microsecond,
			int nanosecond, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyQos(int timeliness, int rate, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyState(int streamState, int dataState, int statusCode,
			String statusText, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyEnum(int key, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyBuffer(ByteBuffer key, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyAscii(String key, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyUtf8(ByteBuffer key, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public Map addKeyRmtes(ByteBuffer key, int action, ComplexType value)
	{
		// TODO Auto-generated method stub
		return null;
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

		_rsslDecodeIter.clear();
		
		_mapCollection.clear();
		
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_atEnd = false;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslMap.decode(_rsslDecodeIter);
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
	
	void fillCollection()
	{
		if (_rsslDecodeIterForCollection == null)
			_rsslDecodeIterForCollection = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
		else
			_rsslDecodeIterForCollection.clear();
		
		int retCode = _rsslDecodeIterForCollection.setBufferAndRWFVersion(_rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
			return;
		
		if (_rsslMapForCollection == null)
			_rsslMapForCollection = com.thomsonreuters.upa.codec.CodecFactory.createMap();
		
		if ((retCode = _rsslMapForCollection.decode(_rsslDecodeIterForCollection)) != CodecReturnCodes.SUCCESS)
			return;
		
		MapEntryImpl mapEntry;
		DataImpl load;
		DataImpl mapEntryKey;
		com.thomsonreuters.upa.codec.MapEntry rsslMapEntry = com.thomsonreuters.upa.codec.CodecFactory.createMapEntry();

		while ((retCode  = rsslMapEntry.decode(_rsslDecodeIterForCollection, null)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			if (retCode != CodecReturnCodes.SUCCESS)
				return;
		
			if (_keyDecodeIterForCollection == null)
				_keyDecodeIterForCollection = com.thomsonreuters.upa.codec.CodecFactory.createDecodeIterator();
			else
				_keyDecodeIterForCollection.clear();

			retCode = _keyDecodeIterForCollection.setBufferAndRWFVersion(rsslMapEntry.encodedKey(), _rsslMajVer, _rsslMinVer);
			if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
				return;
		
			mapEntryKey = dataInstance(Utilities.toEmaDataType[_rsslMapForCollection.keyPrimitiveType()]);
			mapEntryKey.decode(rsslMapEntry.encodedKey(), _keyDecodeIterForCollection);
			
			int rsslContainerType = (rsslMapEntry.action() != MapEntryActions.DELETE)? _rsslMapForCollection.containerType() : com.thomsonreuters.upa.codec.DataTypes.NO_DATA;
			int dType = dataType(rsslContainerType, _rsslMajVer, _rsslMinVer, rsslMapEntry.encodedData());
			
			load = dataInstance(dType);
			load.decode(rsslMapEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalSetDefDb);
			
			mapEntry = new MapEntryImpl(this, rsslMapEntry, mapEntryKey, load);
			_mapCollection.add(mapEntry);
			
			rsslMapEntry = com.thomsonreuters.upa.codec.CodecFactory.createMapEntry();
		}
		
		_fillCollection = false;
	}
	
	
	boolean hasSummary()
	{
		return _rsslMap.checkHasSummaryData();
	}
	
	Data summary()
	{
		return _summaryDecoded;
	}
	
	Key key()
	{
		return _key.data(_keyDecoded);
	}
	
	Key key(DataImpl mapEntryKey)
	{
		return _key.data(mapEntryKey);
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
	public boolean add(MapEntry e)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("Map collection doesn't support this operation.");
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
}