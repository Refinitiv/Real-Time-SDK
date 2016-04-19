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

class ElementEntryImpl extends EntryImpl implements ElementEntry
{
	protected com.thomsonreuters.upa.codec.ElementEntry	_rsslElementEntry;
	protected Object _entryData;
	
	ElementEntryImpl() 
	{
		_rsslElementEntry = CodecFactory.createElementEntry();
	}
	
	ElementEntryImpl(com.thomsonreuters.upa.codec.ElementEntry rsslElementEntry, DataImpl load)
	{
		super(load);
		_rsslElementEntry = rsslElementEntry;
	}
	
	@Override
	public String name()
	{
		if (_rsslElementEntry.name().length() == 0)
			return DataImpl.EMPTY_STRING;
		else
			return _rsslElementEntry.name().toString();
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("ElementEntry ")
				.append(" name=\"").append(name()).append("\"")
				.append(" dataType=\"").append(DataType.asString(_load.dataType()));

		if (_load.dataType() <= DataType.DataTypes.ARRAY)
		{
			_toString.append("\"\n").append(_load.toString(1));
			Utilities.addIndent(_toString, 0).append("ElementEntryEnd\n");
		}
		else
			_toString.append("\" value=\"").append(_load.toString()).append("\"\n");
	
		return _toString.toString();
	}

	@Override
	public ElementEntry reqMsg(String name, ReqMsg value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry refreshMsg(String name, RefreshMsg value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry statusMsg(String name, StatusMsg value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry updateMsg(String name, UpdateMsg value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry postMsg(String name, PostMsg value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry ackMsg(String name, AckMsg value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry genericMsg(String name, GenericMsg value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry fieldList(String name, FieldList value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.FIELD_LIST, (DataImpl) value);
	}

	@Override
	public ElementEntry elementList(String name, ElementList value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.ELEMENT_LIST, (DataImpl) value);
	}

	@Override
	public ElementEntry map(String name, Map value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.MAP, (DataImpl) value);
	}

	@Override
	public ElementEntry vector(String name, Vector value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.VECTOR, (DataImpl) value);
	}

	@Override
	public ElementEntry series(String name, Series value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.SERIES, (DataImpl) value);
	}

	@Override
	public ElementEntry filterList(String name, FilterList value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.FILTER_LIST, (DataImpl) value);
	}

	@Override
	public ElementEntry opaque(String name, OmmOpaque value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.OPAQUE, (DataImpl) value);
	}

	@Override
	public ElementEntry xml(String name, OmmXml value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.XML, (DataImpl) value);
	}

	@Override
	public ElementEntry ansiPage(String name, OmmAnsiPage value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.ANSI_PAGE, (DataImpl) value);
	}

	@Override
	public ElementEntry intValue(String name, long value)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.INT);

		_entryData = CodecFactory.createInt();
		((com.thomsonreuters.upa.codec.Int)_entryData).value(value);
		
		return this;
	}

	@Override
	public ElementEntry uintValue(String name, long value)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.UINT);

		_entryData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt)_entryData).value(value) ;
		
		return this;
	}

	@Override
	public ElementEntry uintValue(String name, BigInteger value)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.UINT);

		_entryData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt)_entryData).value(value) ;
		
		return this;
	}

	@Override
	public ElementEntry real(String name, long mantissa, int magnitudeType)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.REAL);

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
	public ElementEntry realFromDouble(String name, double value)
	{
		return realFromDouble(name, value, OmmReal.MagnitudeType.EXPONENT_0);
	}

	@Override
	public ElementEntry realFromDouble(String name, double value, int magnitudeType)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.REAL);

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
	public ElementEntry floatValue(String name, float value)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.FLOAT);

		_entryData = CodecFactory.createFloat();
		((com.thomsonreuters.upa.codec.Float)_entryData).value(value);
		
		return this;
	}

	@Override
	public ElementEntry doubleValue(String name, double value)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.DOUBLE);

		_entryData = CodecFactory.createDouble();
		((com.thomsonreuters.upa.codec.Double)_entryData).value(value);
		
		return this;
	}

	@Override
	public ElementEntry date(String name, int year, int month, int day)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.DATE);

		_entryData = dateValue(year, month, day);

		return this;
	}

	@Override
	public ElementEntry time(String name, int hour, int minute)
	{
		return time(name, hour, minute, 0, 0, 0, 0);
	}

	@Override
	public ElementEntry time(String name, int hour, int minute, int second)
	{
		return time(name, hour, minute, second, 0, 0, 0);
	}

	@Override
	public ElementEntry time(String name, int hour, int minute, int second, int millisecond)
	{
		return time(name, hour, minute, second,  millisecond, 0, 0);
	}

	@Override
	public ElementEntry time(String name, int hour, int minute, int second, int millisecond, int microsecond)
	{
		return time(name, hour, minute, second,  millisecond, microsecond, 0);
	}

	@Override
	public ElementEntry time(String name, int hour, int minute, int second, int millisecond, int microsecond,
			int nanosecond)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.TIME);

		_entryData = timeValue(hour, minute, second, millisecond, microsecond, nanosecond);

		return this;
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day)
	{
		return dateTime(name, year, month, day, 0, 0, 0, 0, 0, 0);
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour)
	{
		return dateTime(name, year, month, day, hour, 0, 0, 0, 0, 0);
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute)
	{
		return dateTime(name, year, month, day, hour, minute, 0, 0, 0, 0);
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute, int second)
	{
		return dateTime(name, year, month, day, hour, minute, second, 0, 0, 0);
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute, int second,
			int millisecond)
	{
		return dateTime(name, year, month, day, hour, minute, second, millisecond, 0, 0);
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute, int second,
			int millisecond, int microsecond)
	{
		return dateTime(name, year, month, day, hour, minute, second, millisecond, microsecond, 0);
	}

	@Override
	public ElementEntry dateTime(String name, int year, int month, int day, int hour, int minute, int second,
			int millisecond, int microsecond, int nanosecond)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.DATETIME);

		_entryData = dateTimeValue(year, month, day, hour, minute, second, millisecond, microsecond, nanosecond);
		
		return this;
	}

	@Override
	public ElementEntry qos(String name, int timeliness)
	{
		return qos(name, timeliness, OmmQos.Rate.TICK_BY_TICK);
	}

	@Override
	public ElementEntry qos(String name, int timeliness, int rate)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.QOS);

		_entryData = CodecFactory.createQos();
		Utilities.toRsslQos(rate, timeliness, (com.thomsonreuters.upa.codec.Qos)_entryData);
		
		return this;
	}

	@Override
	public ElementEntry state(String name, int streamState)
	{
		return state(name, streamState, OmmState.DataState.OK, OmmState.StatusCode.NONE, DataImpl.EMPTY_STRING);
	}

	@Override
	public ElementEntry state(String name, int streamState, int dataState)
	{
		return state(name, streamState, dataState, OmmState.StatusCode.NONE, DataImpl.EMPTY_STRING);
	}

	@Override
	public ElementEntry state(String name, int streamState, int dataState, int statusCode)
	{
		return state(name, streamState, dataState, statusCode, DataImpl.EMPTY_STRING);
	}

	@Override
	public ElementEntry state(String name, int streamState, int dataState, int statusCode, String statusText)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.STATE);

		_entryData = stateValue(streamState, dataState, statusCode, statusText);
		
		return this;
	}

	@Override
	public ElementEntry enumValue(String name, int value)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.ENUM);

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
	public ElementEntry buffer(String name, ByteBuffer value)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.BUFFER);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_entryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_entryData);
		
		return this;
	}

	@Override
	public ElementEntry ascii(String name, String value)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_entryData = CodecFactory.createBuffer();
		((Buffer)_entryData).data(value);
		
		return this;
	}

	@Override
	public ElementEntry utf8(String name, ByteBuffer value)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_entryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_entryData);

		return this;
	}

	@Override
	public ElementEntry utf8(String name, String value)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_entryData = CodecFactory.createBuffer();
		((Buffer)_entryData).data(value);
		
		return this;
	}

	@Override
	public ElementEntry rmtes(String name, ByteBuffer value)
	{
		entryValue(name, com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_entryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_entryData);
		
		return this;
	}

	@Override
	public ElementEntry array(String name, OmmArray value)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.ARRAY, (DataImpl) value);
	}

	@Override
	public ElementEntry codeInt(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.INT);
	}

	@Override
	public ElementEntry codeUInt(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.UINT);
	}

	@Override
	public ElementEntry codeReal(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.REAL);
	}

	@Override
	public ElementEntry codeFloat(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.FLOAT);
	}

	@Override
	public ElementEntry codeDouble(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.DOUBLE);
	}

	@Override
	public ElementEntry codeDate(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.DATE);
	}

	@Override
	public ElementEntry codeTime(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.TIME);
	}

	@Override
	public ElementEntry codeDateTime(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.DATETIME);
	}

	@Override
	public ElementEntry codeQos(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.QOS);
	}

	@Override
	public ElementEntry codeState(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.STATE);
	}

	@Override
	public ElementEntry codeEnum(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.ENUM);
	}

	@Override
	public ElementEntry codeBuffer(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.BUFFER);
	}

	@Override
	public ElementEntry codeAscii(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING);
	}

	@Override
	public ElementEntry codeUtf8(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING);
	}

	@Override
	public ElementEntry codeRmtes(String name)
	{
		return entryValue(name, com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING);
	}

	private ElementEntry entryValue(String name, int rsslDataType, DataImpl value)
	{
		if (name == null)
			throw ommIUExcept().message("Passed in name is null");
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");

		_rsslElementEntry.name().data(name);
		_rsslElementEntry.dataType(rsslDataType);
		
		Utilities.copy(value.encodedData(), _rsslElementEntry.encodedData());

		return this;
	}

	private ElementEntry entryValue(String name, int rsslDataType)
	{
		if (name == null)
			throw ommIUExcept().message("Passed in name is null");

		_rsslElementEntry.name().data(name);
		_rsslElementEntry.dataType(rsslDataType);

		return this;
	}
}
