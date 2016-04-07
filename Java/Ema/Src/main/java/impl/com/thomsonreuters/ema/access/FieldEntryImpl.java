///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.math.BigInteger;
import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

class FieldEntryImpl extends EntryImpl implements FieldEntry
{
	private com.thomsonreuters.upa.codec.DictionaryEntry _rsslDictionaryEntry;
	private FieldListImpl _fieldList;
	protected com.thomsonreuters.upa.codec.FieldEntry	_rsslFieldEntry;
	protected Object _entryData;
	
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

		if (_load.dataType() <= DataType.DataTypes.ARRAY)
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
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public FieldEntry refreshMsg(int fieldId, RefreshMsg value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public FieldEntry statusMsg(int fieldId, StatusMsg value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public FieldEntry updateMsg(int fieldId, UpdateMsg value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public FieldEntry postMsg(int fieldId, PostMsg value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public FieldEntry ackMsg(int fieldId, AckMsg value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public FieldEntry genericMsg(int fieldId, GenericMsg value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public FieldEntry fieldList(int fieldId, FieldList value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST, (DataImpl) value);
	}

	@Override
	public FieldEntry elementList(int fieldId, ElementList value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST, (DataImpl) value);
	}

	@Override
	public FieldEntry map(int fieldId, Map value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.MAP, (DataImpl) value);
	}

	@Override
	public FieldEntry vector(int fieldId, Vector value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.VECTOR, (DataImpl) value);
	}

	@Override
	public FieldEntry series(int fieldId, Series value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.SERIES, (DataImpl) value);
	}

	@Override
	public FieldEntry filterList(int fieldId, FilterList value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST, (DataImpl) value);
	}

	@Override
	public FieldEntry opaque(int fieldId, OmmOpaque value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.OPAQUE, (DataImpl) value);
	}

	@Override
	public FieldEntry xml(int fieldId, OmmXml value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.XML, (DataImpl) value);
	}

	@Override
	public FieldEntry ansiPage(int fieldId, OmmAnsiPage value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.ANSI_PAGE, (DataImpl) value);
	}

	@Override
	public FieldEntry intValue(int fieldId, long value)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.INT);

		_entryData = CodecFactory.createInt();
		((com.thomsonreuters.upa.codec.Int) _entryData).value(value);

		return this;
	}

	@Override
	public FieldEntry uintValue(int fieldId, long value)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.UINT);

		_entryData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt) _entryData).value(value);

		return this;
	}

	@Override
	public FieldEntry uintValue(int fieldId, BigInteger value)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.UINT);

		_entryData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt) _entryData).value(value);

		return this;
	}

	@Override
	public FieldEntry real(int fieldId, long mantissa, int magnitudeType)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.REAL);

		_entryData = CodecFactory.createReal();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Real)_entryData).value(mantissa, magnitudeType) )
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
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.REAL);

		_entryData = CodecFactory.createReal();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Real)_entryData).value(value, magnitudeType) )
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
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.FLOAT);

		_entryData = CodecFactory.createFloat();
		((com.thomsonreuters.upa.codec.Float)_entryData).value(value);
		
		return this;
	}

	@Override
	public FieldEntry doubleValue(int fieldId, double value)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.DOUBLE);

		_entryData = CodecFactory.createDouble();
		((com.thomsonreuters.upa.codec.Double)_entryData).value(value);
		
		return this;
	}

	@Override
	public FieldEntry date(int fieldId, int year, int month, int day)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.DATE);

		_entryData = dateValue(year, month, day);
		
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
		return time(fieldId, hour, minute, second, millisecond, 0, 0);
	}

	@Override
	public FieldEntry time(int fieldId, int hour, int minute, int second, int millisecond, int microsecond)
	{
		return time(fieldId, hour, minute, second, millisecond, microsecond, 0);
	}

	@Override
	public FieldEntry time(int fieldId, int hour, int minute, int second, int millisecond, int microsecond, int nanosecond)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.TIME);

		_entryData = timeValue(hour, minute, second, millisecond, microsecond, nanosecond);

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
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.DATETIME);

		_entryData = dateTimeValue(year, month, day, hour, minute, second, millisecond, microsecond, nanosecond);
		
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
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.QOS);

		_entryData = CodecFactory.createQos();
		Utilities.toRsslQos(rate, timeliness, (com.thomsonreuters.upa.codec.Qos)_entryData);
		
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
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.STATE);

		_entryData = stateValue(streamState, dataState, statusCode, statusText);
		
		return this;
	}

	@Override
	public FieldEntry enumValue(int fieldId, int value)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.ENUM);

		_entryData = CodecFactory.createEnum();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Enum)_entryData).value(value) )
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
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.BUFFER);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_entryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_entryData);
		
		return this;
	}

	@Override
	public FieldEntry ascii(int fieldId, String value)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_entryData = CodecFactory.createBuffer();
		((Buffer)_entryData).data(value);
		
		return this;
	}

	@Override
	public FieldEntry utf8(int fieldId, ByteBuffer value)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_entryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_entryData);

		return this;
	}

	@Override
	public FieldEntry utf8(int fieldId, String value)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_entryData = CodecFactory.createBuffer();
		((Buffer)_entryData).data(value);
		
		return this;
	}

	@Override
	public FieldEntry rmtes(int fieldId, ByteBuffer value)
	{
		entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_entryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_entryData);
		
		return this;
	}

	@Override
	public FieldEntry array(int fieldId, OmmArray value)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.ARRAY, (DataImpl) value);
	}

	@Override
	public FieldEntry codeInt(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.INT);
	}

	@Override
	public FieldEntry codeUInt(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.UINT);
	}

	@Override
	public FieldEntry codeReal(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.REAL);
	}

	@Override
	public FieldEntry codeFloat(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.FLOAT);
	}

	@Override
	public FieldEntry codeDouble(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.DOUBLE);
	}

	@Override
	public FieldEntry codeDate(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.DATE);
	}

	@Override
	public FieldEntry codeTime(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.TIME);
	}

	@Override
	public FieldEntry codeDateTime(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.DATETIME);
	}

	@Override
	public FieldEntry codeQos(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.QOS);
	}

	@Override
	public FieldEntry codeState(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.STATE);
	}

	@Override
	public FieldEntry codeEnum(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.ENUM);
	}

	@Override
	public FieldEntry codeBuffer(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.BUFFER);
	}

	@Override
	public FieldEntry codeAscii(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING);
	}

	@Override
	public FieldEntry codeUtf8(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING);
	}

	@Override
	public FieldEntry codeRmtes(int fieldId)
	{
		return entryValue(fieldId, com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING);
	}

	private FieldEntry entryValue(int fieldId, int rsslDataType, DataImpl value)
	{
		if (fieldId < -32768 || fieldId > 32767)
			throw ommOORExcept().message("fieldId is out of range [(-32768) - 32767].");
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");

		_rsslFieldEntry.fieldId(fieldId);
		_rsslFieldEntry.dataType(rsslDataType);
		Utilities.copy(value.encodedData(), _rsslFieldEntry.encodedData());

		return this;
	}

	private FieldEntry entryValue(int fieldId, int rsslDataType)
	{
		if (fieldId < -32768 || fieldId > 32767)
			throw ommOORExcept().message("fieldId is out of range [(-32768) - 32767].");

		_rsslFieldEntry.fieldId(fieldId);
		_rsslFieldEntry.dataType(rsslDataType);

		return this;
	}
}
