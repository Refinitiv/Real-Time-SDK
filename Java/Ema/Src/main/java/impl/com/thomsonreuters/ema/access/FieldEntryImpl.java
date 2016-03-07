///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.math.BigInteger;
import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.FieldEntry;
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
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

class FieldEntryImpl extends EntryImpl implements FieldEntry
{
	private com.thomsonreuters.upa.codec.DictionaryEntry _rsslDictionaryEntry;
	private FieldListImpl _fieldList;
	protected com.thomsonreuters.upa.codec.FieldEntry	_rsslFieldEntry;
	protected Object _cacheEntryData;
	
	FieldEntryImpl() 
	{
		_rsslFieldEntry = CodecFactory.createFieldEntry();
	}
	
	FieldEntryImpl(FieldListImpl fieldList, com.thomsonreuters.upa.codec.FieldEntry rsslFieldEntry,
				   com.thomsonreuters.upa.codec.DictionaryEntry rsslDictionaryEntry, DataImpl load)
	{
		super(load);
		_fieldList = fieldList;
		_rsslFieldEntry = rsslFieldEntry;
		_rsslDictionaryEntry = rsslDictionaryEntry;
	}
	
	@Override
	public int fieldId()
	{
		return _rsslFieldEntry.fieldId();
	}

	@Override
	public String name()
	{
		return ((_rsslDictionaryEntry == null) ? DataImpl.EMPTY_STRING : (_rsslDictionaryEntry.acronym().toString()));
	}

	@Override
	public int rippleTo(int fieldId)
	{
		if (fieldId == 0)
			fieldId = _rsslFieldEntry.fieldId();
		
		com.thomsonreuters.upa.codec.DictionaryEntry rsslDictEntry = _fieldList != null ? _fieldList._rsslDictionary.entry(fieldId) : null;
		
		return ((rsslDictEntry == null) ? 0 : rsslDictEntry.rippleToField());
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("FieldEntry ")
				.append(" fid=\"").append(fieldId()).append("\"")
				.append(" name=\"").append(name()).append("\"")
				.append(" dataType=\"").append(DataType.asString(_load.dataType()));

		if (_load.dataType() <= DataTypes.ARRAY)
		{
			_toString.append("\"\n").append(_load.toString(1));
			Utilities.addIndent(_toString, 0).append("FieldEntryEnd\n");
		}
		else
			_toString.append("\" value=\"").append(_load.toString()).append("\"\n");
	
		return _toString.toString();
	}
	
	@Override
	public FieldEntry reqMsg(int fieldId, ReqMsg value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry refreshMsg(int fieldId, RefreshMsg value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry statusMsg(int fieldId, StatusMsg value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry updateMsg(int fieldId, UpdateMsg value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry postMsg(int fieldId, PostMsg value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry ackMsg(int fieldId, AckMsg value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry genericMsg(int fieldId, GenericMsg value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry fieldList(int fieldId, FieldList value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry elementList(int fieldId, ElementList value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry map(int fieldId, Map value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.MAP, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry vector(int fieldId, Vector value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.VECTOR, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry series(int fieldId, Series value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.SERIES, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry filterList(int fieldId, FilterList value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry opaque(int fieldId, OmmOpaque value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.OPAQUE, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry xml(int fieldId, OmmXml value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.XML, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry ansiPage(int fieldId, OmmAnsiPage value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.ANSI_PAGE, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry intValue(int fieldId, long value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.INT);
		
		_cacheEntryData = CodecFactory.createInt();
		((com.thomsonreuters.upa.codec.Int)_cacheEntryData).value(value);
		
		return this;
	}

	@Override
	public FieldEntry uintValue(int fieldId, long value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.UINT);
		
		_cacheEntryData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt)_cacheEntryData).value(value) ;
		
		return this;
	}

	@Override
	public FieldEntry uintValue(int fieldId, BigInteger value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.UINT);
		
		_cacheEntryData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt)_cacheEntryData).value(value) ;
		
		return this;
	}

	@Override
	public FieldEntry real(int fieldId, long mantissa, int magnitudeType)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.REAL);
		
		_cacheEntryData = CodecFactory.createReal();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Real)_cacheEntryData).value(mantissa, magnitudeType) )
		{
			String errText = errorString().append("Attempt to specify invalid real value. Passed mantissa, magnitudeType are='" )
										.append( mantissa ).append( " / " )
										.append( magnitudeType ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}
		
		return this;
	}

	@Override
	public FieldEntry realFromDouble(int fieldId, double value)
	{
		return realFromDouble(fieldId, value, OmmReal.MagnitudeType.EXPONENT_0);
	}

	@Override
	public FieldEntry realFromDouble(int fieldId, double value, int magnitudeType)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.REAL);
		
		_cacheEntryData = CodecFactory.createReal();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Real)_cacheEntryData).value(value, magnitudeType) )
		{
			String errText = errorString().append("Attempt to specify invalid real value. Passed in value,  magnitudeType are='" )
										.append( value ).append( " / " )
										.append( magnitudeType ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}
		
		return this;
	}

	@Override
	public FieldEntry floatValue(int fieldId, float value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.FLOAT);

		_cacheEntryData = CodecFactory.createFloat();
		((com.thomsonreuters.upa.codec.Float)_cacheEntryData).value(value);
		
		return this;
	}

	@Override
	public FieldEntry doubleValue(int fieldId, double value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.DOUBLE);

		_cacheEntryData = CodecFactory.createDouble();
		((com.thomsonreuters.upa.codec.Double)_cacheEntryData).value(value);
		
		return this;
	}

	@Override
	public FieldEntry date(int fieldId, int year, int month, int day)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.DATE);
		
		_cacheEntryData = dateValue(year, month, day);
		
		return this;
	}

	@Override
	public FieldEntry time(int fieldId, int hour, int minute)
	{
		return time(fieldId, hour, minute, 0, 0, 0, 0);
	}

	@Override
	public FieldEntry time(int fieldId, int hour, int minute, int second)
	{
		return time(fieldId, hour, minute, second, 0, 0, 0);
	}

	@Override
	public FieldEntry time(int fieldId, int hour, int minute, int second, int millisecond)
	{
		return time(fieldId, hour, minute, second,  millisecond, 0, 0);
	}

	@Override
	public FieldEntry time(int fieldId, int hour, int minute, int second, int millisecond, int microsecond)
	{
		return time(fieldId, hour, minute, second,  millisecond, microsecond, 0);
	}

	@Override
	public FieldEntry time(int fieldId, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.TIME);
		
		_cacheEntryData = timeValue(hour, minute, second, millisecond, microsecond, nanosecond);

		return this;
	}

	@Override
	public FieldEntry dateTime(int fieldId, int year, int month, int day)
	{
		return dateTime(fieldId, year, month, day, 0, 0, 0, 0, 0, 0);
	}

	@Override
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour)
	{
		return dateTime(fieldId, year, month, day, hour, 0, 0, 0, 0, 0);
	}

	@Override
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour, int minute)
	{
		return dateTime(fieldId, year, month, day, hour, minute, 0, 0, 0, 0);
	}

	@Override
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour, int minute, int second)
	{
		return dateTime(fieldId, year, month, day, hour, minute, second, 0, 0, 0);
	}

	@Override
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour, int minute, int second,
			int millisecond)
	{
		return dateTime(fieldId, year, month, day, hour, minute, second, millisecond, 0, 0);
	}

	@Override
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour, int minute, int second,
			int millisecond, int microsecond)
	{
		return dateTime(fieldId, year, month, day, hour, minute, second, millisecond, microsecond, 0);
	}

	@Override
	public FieldEntry dateTime(int fieldId, int year, int month, int day, int hour, int minute, int second,
			int millisecond, int microsecond, int nanosecond)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.DATETIME);
		
		_cacheEntryData = dateTimeValue(year, month, day, hour, minute, second, millisecond, microsecond, nanosecond);
		
		return this;
	}

	@Override
	public FieldEntry qos(int fieldId, int timeliness)
	{
		return qos(fieldId, timeliness, OmmQos.Rate.TICK_BY_TICK);
	}

	@Override
	public FieldEntry qos(int fieldId, int timeliness, int rate)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.QOS);
		
		_cacheEntryData = CodecFactory.createQos();
		Utilities.toRsslQos(rate, timeliness, (com.thomsonreuters.upa.codec.Qos)_cacheEntryData);
		
		return this;
	}

	@Override
	public FieldEntry state(int fieldId, int streamState)
	{
		return state(fieldId, streamState, OmmState.DataState.OK, OmmState.StatusCode.NONE, DataImpl.EMPTY_STRING);
	}

	@Override
	public FieldEntry state(int fieldId, int streamState, int dataState)
	{
		return state(fieldId, streamState, dataState, OmmState.StatusCode.NONE, DataImpl.EMPTY_STRING);
	}

	@Override
	public FieldEntry state(int fieldId, int streamState, int dataState, int statusCode)
	{
		return state(fieldId, streamState, dataState, statusCode, DataImpl.EMPTY_STRING);
	}

	@Override
	public FieldEntry state(int fieldId, int streamState, int dataState, int statusCode, String statusText)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.STATE);

		_cacheEntryData = CodecFactory.createState();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_cacheEntryData).streamState(streamState) ||
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_cacheEntryData).dataState(dataState) ||
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_cacheEntryData).code(statusCode) || 
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_cacheEntryData).text().data(statusText))
		{
			String errText = errorString().append("Attempt to specify invalid state. Passed in value is='" )
					.append( streamState ).append( " / " )
					.append( dataState ).append( " / " )
					.append( statusCode ).append( "/ " )
					.append( statusText ).append( "." ).toString();
				throw ommIUExcept().message(errText);
		}
		
		return this;
	}

	@Override
	public FieldEntry enumValue(int fieldId, int value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.ENUM);

		_cacheEntryData = CodecFactory.createEnum();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Enum)_cacheEntryData).value(value) )
		{
			String errText = errorString().append("Attempt to specify invalid enum. Passed in value is='" )
					.append( value ).append( "." ).toString();
				throw ommIUExcept().message(errText);
		}
		
		return this;
	}

	@Override
	public FieldEntry buffer(int fieldId, ByteBuffer value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.BUFFER);
		
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_cacheEntryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_cacheEntryData);
		
		return this;
	}

	@Override
	public FieldEntry ascii(int fieldId, String value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_cacheEntryData = CodecFactory.createBuffer();
		((Buffer)_cacheEntryData).data(value);
		
		return this;
	}

	@Override
	public FieldEntry utf8(int fieldId, ByteBuffer value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING);
		
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_cacheEntryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_cacheEntryData);

		return this;
	}

	@Override
	public FieldEntry utf8(int fieldId, String value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING);
		
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_cacheEntryData = CodecFactory.createBuffer();
		((Buffer)_cacheEntryData).data(value);
		
		return this;
	}

	@Override
	public FieldEntry rmtes(int fieldId, ByteBuffer value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING);
		
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_cacheEntryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_cacheEntryData);
		
		return this;
	}

	@Override
	public FieldEntry array(int fieldId, OmmArray value)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.ARRAY, (DataImpl)value);
		return this;
	}

	@Override
	public FieldEntry codeInt(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.INT);
		
		return this;
	}

	@Override
	public FieldEntry codeUInt(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.UINT);
		
		return this;
	}

	@Override
	public FieldEntry codeReal(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.REAL);
		
		return this;
	}

	@Override
	public FieldEntry codeFloat(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.FLOAT);
		
		return this;
	}

	@Override
	public FieldEntry codeDouble(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.DOUBLE);
		
		return this;
	}

	@Override
	public FieldEntry codeDate(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.DATE);
		return this;
	}

	@Override
	public FieldEntry codeTime(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.TIME);
		return this;
	}

	@Override
	public FieldEntry codeDateTime(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.DATETIME);
		
		return this;
	}

	@Override
	public FieldEntry codeQos(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.QOS);
		
		return this;
	}

	@Override
	public FieldEntry codeState(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.STATE);
		
		return this;
	}

	@Override
	public FieldEntry codeEnum(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.ENUM);
		
		return this;
	}

	@Override
	public FieldEntry codeBuffer(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.BUFFER);
		
		return this;
	}

	@Override
	public FieldEntry codeAscii(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.ANSI_PAGE);
		
		return this;
	}

	@Override
	public FieldEntry codeUtf8(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING);
		
		return this;
	}

	@Override
	public FieldEntry codeRmtes(int fieldId)
	{
		fieldEntryValue(fieldId, _rsslFieldEntry, com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING);
		
		return this;
	}
}