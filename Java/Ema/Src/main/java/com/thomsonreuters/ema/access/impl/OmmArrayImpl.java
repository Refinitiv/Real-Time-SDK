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

import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.OmmArray;
import com.thomsonreuters.ema.access.OmmArrayEntry;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.OmmError.ErrorCode;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

public class OmmArrayImpl extends ComplexTypeImpl implements OmmArray
{
	private com.thomsonreuters.upa.codec.Array	_rsslArray = com.thomsonreuters.upa.codec.CodecFactory.createArray();
	private com.thomsonreuters.upa.codec.ArrayEntry	_rsslArrayEntry = com.thomsonreuters.upa.codec.CodecFactory.createArrayEntry();
	private OmmArrayEntryImpl _arrayEntry;
	private com.thomsonreuters.upa.codec.Array	_rsslArrayForCollection;
	private LinkedList<OmmArrayEntry> _ommArrayCollection = new LinkedList<OmmArrayEntry>(); 

	@Override
	public int dataType()
	{
		return DataTypes.ARRAY;
	}

	@Override
	public boolean hasFixedWidth()
	{
		return _rsslArray.itemLength() > 0 ? true : false;
	}

	@Override
	public int fixedWidth()
	{
		return _rsslArray.itemLength();
	}

	@Override
	public OmmArrayEntry entry()
	{
		if (!_decodingStarted)
			throw oommIUExcept().message("Attempt to entry() while decoding entry was NOT started.");
		
		return _arrayEntry;
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
			.append(DataType.asString(Utilities.toEmaDataType[_rsslArray.primitiveType()])).append("\"");
			
			if (hasFixedWidth())
				_toString.append(" fixed width=\"").append(fixedWidth()).append("\"");

			++indent;
			
			while (forth())
			{
				Utilities.addIndent(_toString.append("\n"), indent).append("value=\"");
	
				if (_load.dataType() == DataTypes.BUFFER)
					_toString.append("\n").append(_load.toString());
				else if (_load.dataType() == DataTypes.ERROR)
					_toString.append("\n").append(_load.toString(indent));
				else
					_toString.append("\"").append(_load.toString()).append("\"");
			}
	
			--indent;
		}
	
		Utilities.addIndent(_toString.append("\n"), indent).append("OmmArrayEnd\n");

		return _toString.toString();
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
			_arrayEntry.load(_load);
			return true;
		}

		_decodingStarted = true;

		int retCode = _rsslArrayEntry.decode(_rsslDecodeIter);

		switch (retCode)
		{
			case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			{
				int dType = Utilities.toEmaDataType[_rsslArray.primitiveType()]; 
				_load = _loadPool[dType];
				_load.decode(_rsslArrayEntry.encodedData(),_rsslDecodeIter);
				_arrayEntry.load(_load);
				return true;
			}
			case com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER :
				_atEnd = true;
				return false;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslArrayEntry.encodedData(), ErrorCode.INCOMPLETE_DATA); 
				_arrayEntry.load(_load);
				return true;
			case com.thomsonreuters.upa.codec.CodecReturnCodes.UNSUPPORTED_DATA_TYPE :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslArrayEntry.encodedData(), ErrorCode.UNSUPPORTED_DATA_TYPE);
				_arrayEntry.load(_load);
				return true;
			default :
				_load = _loadPool[DataTypes.ERROR];
				_load.decode(_rsslArrayEntry.encodedData(), ErrorCode.UNKNOWN_ERROR);
				_arrayEntry.load(_load);
				return true;
		}
	}

	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, int majVer, int minVer,
				com.thomsonreuters.upa.codec.DataDictionary rsslDictionary, Object localElSeDefDb)
	{
		_decodingStarted = false;

		_rsslMajVer = majVer;

		_rsslMinVer = minVer;

		_rsslBuffer = rsslBuffer;

		if (_arrayEntry == null)
		{
			decodeInitialize();
			_arrayEntry = new OmmArrayEntryImpl(this, null);
		}
		
		_rsslDecodeIter.clear();
		
		_ommArrayCollection.clear();
		
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_atEnd = false;
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslArray.decode(_rsslDecodeIter);
		switch (retCode)
		{
		case com.thomsonreuters.upa.codec.CodecReturnCodes.BLANK_DATA :
			_atEnd = true;
			_dataCode = DataCode.BLANK;
			_errorCode = ErrorCode.NO_ERROR;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			_atEnd = false;
			_fillCollection = true;
			_dataCode = DataCode.NO_CODE;
			_errorCode = ErrorCode.NO_ERROR;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.ITERATOR_OVERRUN :
			_atEnd = false;
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_OVERRUN;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
			_atEnd = false;
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.INCOMPLETE_DATA;
			break;
		default :
			_atEnd = false;
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.UNKNOWN_ERROR;
			break;
		}
	}

	@Override
	void decode(com.thomsonreuters.upa.codec.Buffer rsslBuffer, com.thomsonreuters.upa.codec.DecodeIterator dIter)
	{
		_decodingStarted = false;

		_rsslMajVer = dIter.majorVersion();

		_rsslMinVer = dIter.minorVersion();

		_rsslBuffer = rsslBuffer;

		if (_arrayEntry == null)
		{
			decodeInitialize();
			_arrayEntry = new OmmArrayEntryImpl(this, null);
		}
		
		_rsslDecodeIter.clear();
		
		int retCode = _rsslDecodeIter.setBufferAndRWFVersion(rsslBuffer, _rsslMajVer, _rsslMinVer);
		if (com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS != retCode)
		{
			_atEnd = false;
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_SET_FAILURE;
			return;
		}
		
		retCode = _rsslArray.decode(_rsslDecodeIter);
		switch (retCode)
		{
		case com.thomsonreuters.upa.codec.CodecReturnCodes.BLANK_DATA :
			_atEnd = true;
			_dataCode = DataCode.BLANK;
			_errorCode = ErrorCode.NO_ERROR;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.SUCCESS :
			_atEnd = false;
			_fillCollection = true;
			_dataCode = DataCode.NO_CODE;
			_errorCode = ErrorCode.NO_ERROR;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.ITERATOR_OVERRUN :
			_atEnd = false;
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.ITERATOR_OVERRUN;
			break;
		case com.thomsonreuters.upa.codec.CodecReturnCodes.INCOMPLETE_DATA :
			_atEnd = false;
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
			_errorCode = ErrorCode.INCOMPLETE_DATA;
			break;
		default :
			_atEnd = false;
			_dataCode = _rsslBuffer.length() > 0 ? DataCode.NO_CODE : DataCode.BLANK;
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
		
		if (_rsslArrayForCollection == null)
			_rsslArrayForCollection = com.thomsonreuters.upa.codec.CodecFactory.createArray();
		
		if ((retCode = _rsslArrayForCollection.decode(_rsslDecodeIterForCollection)) != CodecReturnCodes.SUCCESS)
			return;
		
		OmmArrayEntryImpl ommArrayEntry;
		DataImpl load;
		com.thomsonreuters.upa.codec.ArrayEntry rsslArrayEntry = com.thomsonreuters.upa.codec.CodecFactory.createArrayEntry();

		while ((retCode  = rsslArrayEntry.decode(_rsslDecodeIterForCollection)) != com.thomsonreuters.upa.codec.CodecReturnCodes.END_OF_CONTAINER)
		{
			if (retCode != CodecReturnCodes.SUCCESS)
				return;
			
			load = dataInstance(Utilities.toEmaDataType[_rsslArrayForCollection.primitiveType()]);
			load.decode(rsslArrayEntry.encodedData(),_rsslDecodeIterForCollection);
			
			ommArrayEntry = new OmmArrayEntryImpl(this, load);
			_ommArrayCollection.add(ommArrayEntry);
			
			rsslArrayEntry.clear();
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
	public OmmArray fixedWidth(int width)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addInt(long value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addUInt(long value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addUInt(BigInteger value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addReal(long mantissa, int magnitudeType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addRealFromDouble(double value, int magnitudeType)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addFloat(float value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addDouble(double value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addDate(int year, int month, int day)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public OmmArray addTime()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addTime(int hour)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addTime(int hour, int minute)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addTime(int hour, int minute, int second)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addTime(int hour, int minute, int second, int millisecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addTime(int hour, int minute, int second, int millisecond, int microsecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addTime(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addDateTime(int year, int month, int day)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addDateTime(int year, int month, int day, int hour)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addDateTime(int year, int month, int day, int hour,
			int minute)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addDateTime(int year, int month, int day, int hour,
			int minute, int second)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addDateTime(int year, int month, int day, int hour,
			int minute, int second, int millisecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addDateTime(int year, int month, int day, int hour,
			int minute, int second, int millisecond, int microsecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addDateTime(int year, int month, int day, int hour,
			int minute, int second, int millisecond, int microsecond,
			int nanosecond)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addQos()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addQos(int timeliness)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addQos(int timeliness, int rate)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addState()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addState(int streamState)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addState(int streamState, int dataState)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addState(int streamState, int dataState, int statusCode)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addState(int streamState, int dataState, int statusCode,
			String statusText)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addEnum(int value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addBuffer(ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addAscii(String value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addUtf8(String value)
	{
		// TODO Auto-generated method stub
		return null;
	}
	
	@Override
	public OmmArray addUtf8(ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addRmtes(ByteBuffer value)
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeInt()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeUInt()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeReal()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeFloat()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeDouble()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeDate()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeTime()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeDateTime()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeQos()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeState()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeEnum()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeBuffer()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeASCII()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeUtf8()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addCodeRmtes()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray complete()
	{
		// TODO Auto-generated method stub
		return null;
	}

	@Override
	public OmmArray addRealFromDouble(double value)
	{
		// TODO Auto-generated method stub
		return null;
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
	public boolean add(OmmArrayEntry e)
	{
		// TODO Auto-generated method stub
		throw new UnsupportedOperationException("OmmArray collection doesn't support this operation.");
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
}