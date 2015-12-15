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
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.FieldEntry;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.FilterList;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.OmmAnsiPage;
import com.thomsonreuters.ema.access.OmmArray;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.OmmOpaque;
import com.thomsonreuters.ema.access.OmmXml;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.Series;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.Vector;
import com.thomsonreuters.upa.codec.CodecReturnCodes;


public class FieldListImpl extends ComplexTypeImpl implements FieldList
{
	private com.thomsonreuters.upa.codec.FieldList	_rsslFieldList = com.thomsonreuters.upa.codec.CodecFactory.createFieldList();
	private com.thomsonreuters.upa.codec.FieldEntry	_rsslFieldEntry = com.thomsonreuters.upa.codec.CodecFactory.createFieldEntry();
	private FieldEntryImpl _fieldEntry;
	private com.thomsonreuters.upa.codec.FieldList	_rsslFieldListForCollection;
	private LinkedList<FieldEntry> _fieldListCollection = new LinkedList<FieldEntry>(); 
	
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
			throw oommIUExcept().message("Attempt to infoFieldListNum() while FieldList Info is NOT set.");
		
		return _rsslFieldList.fieldListNum();
	}

	@Override
	public int infoDictionaryId()
	{
		if (!hasInfo())
			throw oommIUExcept().message("Attempt to infoDictionaryId() while FieldList Info is NOT set.");

		return _rsslFieldList.dictionaryId();
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
			_fieldEntry.load(_load);
			return true;
		}

		_decodingStarted = true;

		int retCode = _rsslFieldEntry.decode(_rsslDecodeIter);

		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			{
				_rsslDictionaryEntry = _rsslDictionary.entry(_rsslFieldEntry.fieldId());
	
				if (_rsslDictionaryEntry == null)
				{
					_load = _loadPool[DataTypes.ERROR];
					_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.FIELD_ID_NOT_FOUND);
					_fieldEntry.load(_load);
					return true;
				}
				
				int dType = dataType(_rsslDictionaryEntry.rwfType(), _rsslMajVer, _rsslMinVer, _rsslFieldEntry.encodedData()); 
				_load = _loadPool[dType];
				if (DataTypes.ERROR > dType && DataTypes.OPAQUE <= dType)
					_load.decode(_rsslFieldEntry.encodedData(),_rsslDecodeIter);
				else
					_load.decode(_rsslFieldEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalFLSetDefDb);
				_fieldEntry.load(_rsslDictionaryEntry, _load);
				return true;
			}
			case com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER :
				_atEnd = true;
				return false;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.INCOMPLETE_DATA); 
				_fieldEntry.load(_load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				_fieldEntry.load(_load);
				return true;
			default :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				_fieldEntry.load(_load);
				return true;
		}
	}

	@Override
	public boolean forth(int fieldId)
	{
		int retCode = com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS;
		
		do {
			if (_atEnd) return false;
			
			if (!_decodingStarted && ErrorCode.NO_ERROR != _errorCode)
			{
				_atEnd = true;
				_decodingStarted = true;
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslBuffer, _errorCode);
				_fieldEntry.load(_load);
				return true;
			}
			
			_decodingStarted = true;
			
			retCode = _rsslFieldEntry.decode(_rsslDecodeIter);
			
			if(retCode == com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
			{
				_atEnd = true;
				return false;
			}
		}
		while (_rsslFieldEntry.fieldId() != fieldId);
		
		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
				_rsslDictionaryEntry = _rsslDictionary.entry(_rsslFieldEntry.fieldId());
	
				if (_rsslDictionaryEntry == null)
				{
					_load = _loadPool[DataTypes.ERROR];
					_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.FIELD_ID_NOT_FOUND);
					_fieldEntry.load(_load);
					return true;
				}
				
				int dType = dataType(_rsslDictionaryEntry.rwfType(), _rsslMajVer, _rsslMinVer, _rsslFieldEntry.encodedData()); 
				_load = _loadPool[dType];
				if (DataTypes.ERROR > dType && DataTypes.OPAQUE <= dType)
					_load.decode(_rsslFieldEntry.encodedData(),_rsslDecodeIter);
				else
					_load.decode(_rsslFieldEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalFLSetDefDb);
				_fieldEntry.load(_rsslDictionaryEntry, _load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER :
				_atEnd = true;
				return false;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.INCOMPLETE_DATA); 
				_fieldEntry.load(_load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				_fieldEntry.load(_load);
				return true;
			default :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				_fieldEntry.load(_load);
				return true;
		}
	}

	@Override
	public boolean forth(String name)
	{
		int retCode = com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS;
		boolean matchName = false;
		
		do {
			if (!_decodingStarted && ErrorCode.NO_ERROR != _errorCode)
			{
				_atEnd = true;
				_decodingStarted = true;
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslBuffer, _errorCode);
				_fieldEntry.load(_load);
				return true;
			}
			
			_decodingStarted = true;
			
			retCode = _rsslFieldEntry.decode(_rsslDecodeIter);

			if(retCode == com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
			{
				_atEnd = true;
				return false;
			}
	
			_rsslDictionaryEntry = _rsslDictionary.entry(_rsslFieldEntry.fieldId());
	
			if (_rsslDictionaryEntry != null)
			{
				if (name.equals(_rsslDictionaryEntry.acronym().toString()))
					matchName = true;
			}
		}
		while (!matchName);
		
		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
				if (_rsslDictionaryEntry == null)
				{
					_load = _loadPool[DataTypes.ERROR];
					_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.FIELD_ID_NOT_FOUND);
					_fieldEntry.load(_load);
					return true;
				}
				
				int dType = dataType(_rsslDictionaryEntry.rwfType(), _rsslMajVer, _rsslMinVer, _rsslFieldEntry.encodedData()); 
				_load = _loadPool[dType];
				if (DataTypes.ERROR > dType && DataTypes.OPAQUE <= dType)
					_load.decode(_rsslFieldEntry.encodedData(),_rsslDecodeIter);
				else
					_load.decode(_rsslFieldEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalFLSetDefDb);
				_fieldEntry.load(_rsslDictionaryEntry, _load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER :
				_atEnd = true;
				return false;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.INCOMPLETE_DATA); 
				_fieldEntry.load(_load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				_fieldEntry.load(_load);
				return true;
			default :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslFieldEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				_fieldEntry.load(_load);
				return true;
		}
	}

	@Override
	public boolean forth(Data data)
	{
		// TODO Auto-generated method stub
		return false;
	}

	@Override
	public FieldEntry entry()
	{
		if (!_decodingStarted)
			throw oommIUExcept().message("Attempt to entry() while decoding entry was NOT started.");

		return _fieldEntry;
	}

	@Override
	public void reset()
	{
		// TODO Auto-generated method stub
	}

	@Override
	public FieldList info(int dictionaryId, int fieldListNum)
	{
		if (dictionaryId < 0 || dictionaryId > 32767)
			throw oommIUExcept().message("dictionaryId is out of range [0 - 32767].");
		
		if (fieldListNum < -32768 || fieldListNum > 32767)
			throw oommIUExcept().message("fieldListNum is out of range [(-32768) - 32767].");

		_rsslFieldList.dictionaryId(dictionaryId);
		_rsslFieldList.fieldListNum(fieldListNum);
		_rsslFieldList.applyHasInfo();
		return this;
	}

	@Override
	public FieldList addReqMsg(int fieldId, ReqMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addRefreshMsg(int fieldId, RefreshMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addStatusMsg(int fieldId, StatusMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addUpdateMsg(int fieldId, UpdateMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addPostMsg(int fieldId, PostMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addAckMsg(int fieldId, AckMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addGenericMsg(int fieldId, GenericMsg value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addFieldList(int fieldId, FieldList value)
	{
		//TODO
		return this;
	}

	@Override
	public FieldList addElementList(int fieldId, ElementList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addMap(int fieldId, Map value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addVector(int fieldId, Vector value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addSeries(int fieldId, Series value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addFilterList(int fieldId, FilterList value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addOpaque(int fieldId, OmmOpaque value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addXml(int fieldId, OmmXml value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addAnsiPage(int fieldId, OmmAnsiPage value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addInt(int fieldId, long value)
	{
		if (!_encodingStarted)
			initEncode();
		
		// TODO Auto-generated method stub
		addPrimitiveEntry(fieldId, DataTypes.INT, "addInt()", value);
		
		return this;
	}

	@Override
	public FieldList addUInt(int fieldId, long value)
	{
		return null;
	}
	
	@Override
	public FieldList addUInt(int fieldId, BigInteger value)
	{
		return null;
	}

	@Override
	public FieldList addReal(int fieldId, long mantissa, int magnitudeType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addRealFromDouble(int fieldId, double value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addRealFromDouble(int fieldId, double value, int magnitudeType)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public FieldList addFloat(int fieldId, float value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addDouble(int fieldId, double value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addDate(int fieldId, int year, int month, int day)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addTime(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addTime(int fieldId, int hour)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addTime(int fieldId, int hour, int minute)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addTime(int fieldId, int hour, int minute, int second)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addTime(int fieldId, int hour, int minute, int second,
			int millisecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addTime(int fieldId, int hour, int minute, int second,
			int millisecond, int microsecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addTime(int fieldId, int hour, int minute, int second,
			int millisecond, int microsecond, int nanosecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addDateTime(int fieldId, int year, int month, int day)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addDateTime(int fieldId, int year, int month, int day,
			int hour)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addDateTime(int fieldId, int year, int month, int day,
			int hour, int minute)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addDateTime(int fieldId, int year, int month, int day,
			int hour, int minute, int second)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addDateTime(int fieldId, int year, int month, int day,
			int hour, int minute, int second, int millisecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addDateTime(int fieldId, int year, int month, int day,
			int hour, int minute, int second, int millisecond, int microsecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addDateTime(int fieldId, int year, int month, int day,
			int hour, int minute, int second, int millisecond, int microsecond,
			int nanosecond)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public FieldList addQos(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addQos(int fieldId, int timeliness)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addQos(int fieldId, int timeliness, int rate)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public FieldList addState(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addState(int fieldId, int streamState)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addState(int fieldId, int streamState, int dataState)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addState(int fieldId, int streamState, int dataState,
			int statusCode)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addState(int fieldId, int streamState, int dataState,
			int statusCode, String statusText)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addEnum(int fieldId, int value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addBuffer(int fieldId, ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addAscii(int fieldId, String value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addUtf8(int fieldId, ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public FieldList addUtf8(int fieldId, String value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addRmtes(int fieldId, ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addArray(int fieldId, OmmArray value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeInt(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeUInt(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeReal(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeFloat(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeDouble(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeDate(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeTime(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeDATETIME(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeQos(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeState(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeEnum(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeBuffer(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeASCII(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeUtf8(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList addCodeRmtes(int fieldId)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public FieldList complete()
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public String toString()
	{
		return toString(0);
	}
	
	String toString(int indent)
	{
		_toString.setLength(0);
		Utilities.addIndent(_toString, indent).append("FieldList");
				
		if (hasInfo())
			_toString.append(" FieldListNum=\"").append(infoFieldListNum()).append("\" DictionaryId=\"")
					 .append(infoDictionaryId()).append("\"");

		++indent;
			
		while (forth())
		{
			Utilities.addIndent(_toString.append("\n"), indent).append("FieldEntry fid=\"")
																  .append(_fieldEntry.fieldId())
																  .append("\" name=\"")
																  .append(_fieldEntry.name())
																  .append("\" dataType=\"")
																  .append(DataType.asString(_load.dataType()));

			if (DataTypes.ARRAY >= _load.dataType() || DataTypes.ERROR == _load.dataType())
			{
				++indent; 
				_toString.append("\"\n").append(_load.toString(indent));
				--indent;
				Utilities.addIndent(_toString, indent).append("FieldEntryEnd");
			}
			else if (_load.dataType() == DataTypes.BUFFER)
			{
				if (load().code() == DataCode.BLANK)
					_toString.append("\" value=\"").append(load().toString()).append("\"");
				else
					_toString.append("\"\n").append(load().toString());
			}
			else
				_toString.append("\" value=\"").append(_load.toString()).append("\"");
		}

		--indent;

		Utilities.addIndent(_toString.append("\n"), indent).append("FieldListEnd\n");

		return _toString.toString();
	}
	
	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int majVer, int minVer,
				com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localFlSetDefDb)
	{
		_decodingStarted = false;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		_rsslDictionary = rsslDictionary;

		_rsslLocalFLSetDefDb = (com.thomsonreuters.upa.codec.LocalFieldSetDefDb)localFlSetDefDb;

		if (_rsslDictionary == null)
		{
			_atEnd = false;
			_errorCode = ErrorCode.NO_DICTIONARY;
			return;
		}

		if (_fieldEntry == null)
		{
			decodeInitialize();
			_fieldEntry = new FieldEntryImpl(this, _rsslFieldEntry);
		}
		
		_rsslDecodeIter.clear();
		
		_fieldListCollection.clear();
		
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_atEnd = false;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslFieldList.decode(_rsslDecodeIter, _rsslLocalFLSetDefDb);
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
		
		if (_rsslFieldListForCollection == null)
			_rsslFieldListForCollection = com.thomsonreuters.upa.codec.CodecFactory.createFieldList();
		
		if ((retCode = _rsslFieldListForCollection.decode(_rsslDecodeIterForCollection, _rsslLocalFLSetDefDb)) != CodecReturnCodes.SUCCESS)
			return;
		
		FieldEntryImpl fieldEntry;
		DataImpl load;
		com.thomsonreuters.upa.codec.DictionaryEntry rsslDictionaryEntry;
		com.thomsonreuters.upa.codec.FieldEntry rsslFieldEntry = com.thomsonreuters.upa.codec.CodecFactory.createFieldEntry();

		while ((retCode  = rsslFieldEntry.decode(_rsslDecodeIterForCollection)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			if (retCode != CodecReturnCodes.SUCCESS)
				return;
			
			rsslDictionaryEntry = _rsslDictionary.entry(rsslFieldEntry.fieldId());
			if (rsslDictionaryEntry == null)
				return;
			
			int dType = dataType(rsslDictionaryEntry.rwfType(), _rsslMajVer, _rsslMinVer, rsslFieldEntry.encodedData());
			
			load = dataInstance(dType);
			fieldEntry = new FieldEntryImpl(this, rsslFieldEntry, rsslDictionaryEntry, load);
			
			if (DataTypes.ERROR > dType && DataTypes.OPAQUE <= dType)
				load.decode(rsslFieldEntry.encodedData(),_rsslDecodeIterForCollection);
			else
				load.decode(rsslFieldEntry.encodedData(), _rsslMajVer, _rsslMinVer, _rsslDictionary, _rsslLocalFLSetDefDb);
			
			_fieldListCollection.add(fieldEntry);
			
			rsslFieldEntry = com.thomsonreuters.upa.codec.CodecFactory.createFieldEntry();
		}
		
		_fillCollection = false;
	}
	
	void initEncode()
	{
//		_encodingStarted = true;
//		
//		if (_rsslFieldList.checkHasStandardData())
//			return;
//		
//		_rsslFieldList.applyHasStandardData();
//		
//		int retCode = _rsslFieldList.encodeInit(_rsslEncIter, 0, 0);
//
//		while (com.thomsonreuters.upa.codec.CodecReturnCodes.BUFFER_TOO_SMALL == retCode)
//		{
//			retCode = _rsslFieldList.encodeComplete(iter, false);
//
//			_pEncodeIter->reallocate();
//
//			retCode = _rsslFieldList.encodeInit(_rsslEncIter, 0, 0);
//		}
//
//		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS > retCode)
//		{
//			exceptionText.setLength(0);
//			exceptionText.append("Failed to initialize FieldList encoding. Reason='")
//						 .append(com.thomsonreuters.upa.codec.CodecReturnCodes.toString(retCode))
//						 .append("'. ");
//			throw oommIUExcept().message(exceptionText.toString());
//		}
	}
	
	void addPrimitiveEntry(int fieldId, int rsslDataType, String methodName, long value)
	{
	}
	
	void addEncodedEntry(int fieldId, int rsslDataType, String methodName, com.thomsonreuters.upa.codec.Buffer rsslBuffer)
	{
	}
	
	void fieldId (int fieldId)
	{
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
	}

	@Override
	public boolean add(FieldEntry e)
	{
		throw new UnsupportedOperationException("FieldList collection doesn't support this operation.");
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
}