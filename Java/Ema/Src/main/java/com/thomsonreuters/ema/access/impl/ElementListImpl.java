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

import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.Data;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.ElementEntry;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.FilterList;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.OmmAnsiPage;
import com.thomsonreuters.ema.access.OmmArray;
import com.thomsonreuters.ema.access.OmmOpaque;
import com.thomsonreuters.ema.access.OmmXml;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.Series;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.Vector;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

public class ElementListImpl extends ComplexTypeImpl implements ElementList
{
	private com.thomsonreuters.upa.codec.ElementList _rsslElementList = com.thomsonreuters.upa.codec.CodecFactory.createElementList();
	private com.thomsonreuters.upa.codec.ElementEntry _rsslElementEntry = com.thomsonreuters.upa.codec.CodecFactory.createElementEntry();
	private ElementEntryImpl _elementEntry;
	private com.thomsonreuters.upa.codec.ElementList _rsslElementListForCollection;
	private LinkedList<ElementEntry> _elementListCollection = new LinkedList<ElementEntry>(); 
	
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
			throw oommIUExcept().message("Attempt to infoElementListNum() while ElementList Info is NOT set.");

		return _rsslElementList.elementListNum();
	}

	@Override
	public boolean forth()
	{
		if (_atEnd) return false;

		if (!_decodingStarted && ErrorCode.NO_ERROR != _errorCode)
		{
			_atEnd = true;
			_decodingStarted = true;
			_load = _loadPool[DataTypes.ERROR];
			_load.decode(_rsslBuffer, _errorCode);
			_elementEntry.load(_load);
			return true;
		}

		_decodingStarted = true;

		int retCode = _rsslElementEntry.decode(_rsslDecodeIter);

		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			{
				int dType = dataType(_rsslElementEntry.dataType(), _rsslMajVer, _rsslMinVer, _rsslElementEntry.encodedData()); 
				_load = _loadPool[dType];
				if (DataTypes.ERROR > dType && DataTypes.OPAQUE <= dType)
					_load.decode(_rsslElementEntry.encodedData(),_rsslDecodeIter);
				else
					_load.decode(_rsslElementEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, null);
				
				_elementEntry.load(_load);
				return true;
			}
			case com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER :
				_atEnd = true;
				return false;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslElementEntry.encodedData(), ErrorCode.INCOMPLETE_DATA); 
				_elementEntry.load(_load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslElementEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				_elementEntry.load(_load);
				return true;
			default :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslElementEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				_elementEntry.load(_load);
				return true;
		}
	}

	@Override
	public boolean forth(String name)
	{
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public boolean forth(Data data)
	{
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public ElementEntry entry()
	{
		if (!_decodingStarted)
			throw oommIUExcept().message("Attempt to entry() while decoding entry was NOT started.");
		
		return _elementEntry;
	}

	@Override
	public String toString()
	{
		return toString(0);
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

		++indent;
			
		while (forth())
		{
			Utilities.addIndent(_toString.append("\n"), indent).append("ElementEntry name=\"")
																  .append(_elementEntry.name())
																  .append("\" dataType=\"")
																  .append(DataType.asString(_load.dataType()));

			if (DataTypes.ARRAY >= _load.dataType() || DataTypes.ERROR == _load.dataType())
			{
				++indent; 
				_toString.append("\"\n")
						 .append(_load.toString(indent));
				--indent;
				Utilities.addIndent(_toString, indent).append("ElementEntryEnd");
			}
			else if (_load.dataType() == DataTypes.BUFFER)
			{
				if (load().code() == DataCode.BLANK)
					_toString.append("\" value=\"").append(load().toString()).append("\"");
				else
					_toString.append("\"\n").append(load().toString());
			}
			else
				_toString.append("\" value=\"")
						 .append(_load.toString())
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
		_decodingStarted = false;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslLocalELSetDefDb = (com.thomsonreuters.upa.codec.LocalElementSetDefDb)localElSeDefDb;

		if (_rsslDictionary == null)
		{
			_atEnd = false;
			_errorCode = ErrorCode.NO_DICTIONARY;
			return;
		}

		if (_elementEntry == null)
		{
			decodeInitialize();
			_elementEntry = new ElementEntryImpl(this, _rsslElementEntry, null);
		}
		
		_rsslDecodeIter.clear();
		
		_elementListCollection.clear();
		
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_atEnd = false;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslElementList.decode(_rsslDecodeIter, _rsslLocalELSetDefDb);
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
		case com.thomsonreuters.upa.codec.CodecReturnCodes.SET_SKIPPED :
			_atEnd = false;
			_errorCode = ErrorCode.NO_SET_DEFINITION;
			break;
		default :
			_atEnd = false;
			_errorCode = ErrorCode.UNKNOWN_ERROR;
			break;
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
		
		if (_rsslElementListForCollection == null)
			_rsslElementListForCollection = com.thomsonreuters.upa.codec.CodecFactory.createElementList();
		
		if ((retCode = _rsslElementListForCollection.decode(_rsslDecodeIterForCollection, _rsslLocalELSetDefDb)) != CodecReturnCodes.SUCCESS)
			return;
		
		ElementEntryImpl elementEntry;
		DataImpl load;
		com.thomsonreuters.upa.codec.ElementEntry rsslElementEntry = com.thomsonreuters.upa.codec.CodecFactory.createElementEntry();

		while ((retCode  = rsslElementEntry.decode(_rsslDecodeIterForCollection)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			if (retCode != CodecReturnCodes.SUCCESS)
				return;
			
			int dType = dataType(rsslElementEntry.dataType(), _rsslMajVer, _rsslMinVer, rsslElementEntry.encodedData());
			load = dataInstance(dType);
			elementEntry = new ElementEntryImpl(this, rsslElementEntry, load);
			
			if (DataTypes.ERROR > dType && DataTypes.OPAQUE <= dType)
				load.decode(rsslElementEntry.encodedData(),_rsslDecodeIterForCollection);
			else
				load.decode(rsslElementEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, null);
			
			_elementListCollection.add(elementEntry);
			
			rsslElementEntry = com.thomsonreuters.upa.codec.CodecFactory.createElementEntry();
		}
		
		_fillCollection = false;
	}
	
	@Override
	public void reset()
	{
		// TODO Auto-generated method stub
	}

	@Override
	public void clear()
	{
		// TODO Auto-generated method stub
	}

	@Override
	public ElementList info(int elementListNum)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addReqMsg(String name, ReqMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addRefreshMsg(String name, RefreshMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addStatusMsg(String name, StatusMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addUpdateMsg(String name, UpdateMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addPostMsg(String name, PostMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addAckMsg(String name, AckMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addGenericMsg(String name, GenericMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addFieldList(String name, FieldList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addElementList(String name, ElementList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addMap(String name, Map value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addVector(String name, Vector value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addSeries(String name, Series value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addFilterList(String name, FilterList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addOpaque(String name, OmmOpaque value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addXml(String name, OmmXml value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addAnsiPage(String name, OmmAnsiPage value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addInt(String name, long value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addUInt(String name, long value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addUInt(String name, BigInteger value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addReal(String name, long mantissa, int magnitudeType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addRealFromDouble(String name, double value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addRealFromDouble(String name, double value,
			int magnitudeType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addFloat(String name, float value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addDouble(String name, double value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addDate(String name, int year, int month, int day)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public ElementList addTime(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addTime(String name, int hour)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addTime(String name, int hour, int minute)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addTime(String name, int hour, int minute, int second)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addTime(String name, int hour, int minute, int second,
			int millisecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addTime(String name, int hour, int minute, int second,
			int millisecond, int microsecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addTime(String name, int hour, int minute, int second,
			int millisecond, int microsecond, int nanosecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addDateTime(String name, int year, int month, int day)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addDateTime(String name, int year, int month, int day,
			int hour)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addDateTime(String name, int year, int month, int day,
			int hour, int minute)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addDateTime(String name, int year, int month, int day,
			int hour, int minute, int second)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addDateTime(String name, int year, int month, int day,
			int hour, int minute, int second, int millisecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addDateTime(String name, int year, int month, int day,
			int hour, int minute, int second, int millisecond, int microsecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addDateTime(String name, int year, int month, int day,
			int hour, int minute, int second, int millisecond, int microsecond,
			int nanosecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addQos(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addQos(String name, int timeliness)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addQos(String name, int timeliness, int rate)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addState(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addState(String name, int streamState)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addState(String name, int streamState, int dataState)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addState(String name, int streamState, int dataState,
			int statusCode)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public ElementList addState(String name, int streamState, int dataState,
			int statusCode, String statusText)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addEnum(String name, int value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addBuffer(String name, ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addAscii(String name, String value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addUtf8(String name, String value)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public ElementList addUtf8(String name, ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addRmtes(String name, ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addArray(String name, OmmArray value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeInt(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeUInt(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeReal(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeFloat(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeDouble(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeDate(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeTime(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeDATETIME(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeQos(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeState(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeEnum(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeBuffer(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeASCII(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeUtf8(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList addCodeRmtes(String name)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public ElementList complete()
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
}