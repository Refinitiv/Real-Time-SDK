///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


import java.math.BigInteger;
import java.nio.ByteBuffer;

import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.OmmArrayEntry;
import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

class OmmArrayEntryImpl extends EntryImpl implements OmmArrayEntry
{
	protected com.thomsonreuters.upa.codec.ArrayEntry	_rsslArrayEntry;
	protected Object _cacheEntryData;
	protected int _cacheEntryDataType;
	
	OmmArrayEntryImpl()
	{
		_rsslArrayEntry = CodecFactory.createArrayEntry();
	}
	
	OmmArrayEntryImpl(DataImpl load)
	{
		super(load);
	}
	
	@Override
	public String toString()
	{
		_toString.setLength(0);
		_toString.append("OmmArrayEntry ")
				 .append(" dataType=\"").append(DataType.asString(_load.dataType())).append("\"")
				 .append(" value=\"").append(_load.toString()).append("\"\n");

		return _toString.toString();
	}

	@Override
	public OmmArrayEntry intValue(long value)
	{
		_cacheEntryData = CodecFactory.createInt();
		((com.thomsonreuters.upa.codec.Int)_cacheEntryData).value(value);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.INT;
		return this;
	}

	@Override
	public OmmArrayEntry uintValue(long value)
	{
		_cacheEntryData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt)_cacheEntryData).value(value) ;
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.UINT;
		
		return this;
	}

	@Override
	public OmmArrayEntry uintValue(BigInteger value)
	{
		_cacheEntryData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt)_cacheEntryData).value(value) ;
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.UINT;
		
		return this;
	}

	@Override
	public OmmArrayEntry real(long mantissa, int magnitudeType)
	{
		_cacheEntryData = CodecFactory.createReal();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Real)_cacheEntryData).value(mantissa, magnitudeType) )
		{
			String errText = errorString().append("Attempt to specify invalid real value. Passed mantissa, magnitudeType are='" )
										.append( mantissa ).append( " / " )
										.append( magnitudeType ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.REAL;
		
		return this;
	}

	@Override
	public OmmArrayEntry realFromDouble(double value)
	{
		return realFromDouble(value, OmmReal.MagnitudeType.EXPONENT_0);
	}

	@Override
	public OmmArrayEntry realFromDouble(double value, int magnitudeType)
	{
		_cacheEntryData = CodecFactory.createReal();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Real)_cacheEntryData).value(value, magnitudeType) )
		{
			String errText = errorString().append("Attempt to specify invalid real value. Passed in value,  magnitudeType are='" )
										.append( value ).append( " / " )
										.append( magnitudeType ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.REAL;
		
		return this;
	}

	@Override
	public OmmArrayEntry floatValue(float value)
	{
		_cacheEntryData = CodecFactory.createFloat();
		((com.thomsonreuters.upa.codec.Float)_cacheEntryData).value(value);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.FLOAT;
		
		return this;
	}

	@Override
	public OmmArrayEntry doubleValue(double value)
	{
		_cacheEntryData = CodecFactory.createDouble();
		((com.thomsonreuters.upa.codec.Double)_cacheEntryData).value(value);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.DOUBLE;
		
		return this;
	}

	@Override
	public OmmArrayEntry date(int year, int month, int day)
	{
		_cacheEntryData = dateValue(year, month, day);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.DATE;
		
		return this;
	}

	@Override
	public OmmArrayEntry time(int hour, int minute)
	{
		return time(hour, minute, 0, 0, 0, 0);
	}

	@Override
	public OmmArrayEntry time(int hour, int minute, int second)
	{
		return time(hour, minute, second, 0, 0, 0);
	}

	@Override
	public OmmArrayEntry time(int hour, int minute, int second, int millisecond)
	{
		return time(hour, minute, second,  millisecond, 0, 0);
	}

	@Override
	public OmmArrayEntry time(int hour, int minute, int second, int millisecond, int microsecond)
	{
		return time(hour, minute, second,  millisecond, microsecond, 0);
	}

	@Override
	public OmmArrayEntry time(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond)
	{
		_cacheEntryData = timeValue(hour, minute, second, millisecond, microsecond, nanosecond);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.TIME;
		
		return this;
	}

	@Override
	public OmmArrayEntry dateTime(int year, int month, int day)
	{
		return dateTime(year, month, day, 0, 0, 0, 0, 0, 0);
	}

	@Override
	public OmmArrayEntry dateTime(int year, int month, int day, int hour)
	{
		return dateTime(year, month, day, hour, 0, 0, 0, 0, 0);
	}

	@Override
	public OmmArrayEntry dateTime(int year, int month, int day, int hour, int minute)
	{
		return dateTime(year, month, day, hour, minute, 0, 0, 0, 0);
	}

	@Override
	public OmmArrayEntry dateTime(int year, int month, int day, int hour, int minute, int second)
	{
		return dateTime(year, month, day, hour, minute, second, 0, 0, 0);
	}

	@Override
	public OmmArrayEntry dateTime(int year, int month, int day, int hour, int minute, int second, int millisecond)
	{
		return dateTime(year, month, day, hour, minute, second, millisecond, 0, 0);
	}

	@Override
	public OmmArrayEntry dateTime(int year, int month, int day, int hour, int minute, int second, int millisecond,
			int microsecond)
	{
		return dateTime(year, month, day, hour, minute, second, millisecond, microsecond, 0);
	}

	@Override
	public OmmArrayEntry dateTime(int year, int month, int day, int hour, int minute, int second, int millisecond,
			int microsecond, int nanosecond)
	{
		_cacheEntryData = dateTimeValue(year, month, day, hour, minute, second, millisecond, microsecond, nanosecond);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.DATETIME;
		
		return this;
	}

	@Override
	public OmmArrayEntry qos(int timeliness)
	{
		return qos(timeliness, OmmQos.Rate.TICK_BY_TICK);
	}

	@Override
	public OmmArrayEntry qos(int timeliness, int rate)
	{
		_cacheEntryData = CodecFactory.createQos();
		Utilities.toRsslQos(rate, timeliness, (com.thomsonreuters.upa.codec.Qos)_cacheEntryData);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.QOS;
		
		return this;
	}

	@Override
	public OmmArrayEntry state(int streamState)
	{
		return state(streamState, OmmState.DataState.OK, OmmState.StatusCode.NONE, DataImpl.EMPTY_STRING);
	}

	@Override
	public OmmArrayEntry state(int streamState, int dataState)
	{
		return state(streamState, dataState, OmmState.StatusCode.NONE, DataImpl.EMPTY_STRING);
	}

	@Override
	public OmmArrayEntry state(int streamState, int dataState, int statusCode)
	{
		return state(streamState, dataState, statusCode, DataImpl.EMPTY_STRING);
	}

	@Override
	public OmmArrayEntry state(int streamState, int dataState, int statusCode, String statusText)
	{
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
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.STATE;
		
		return this;
	}

	@Override
	public OmmArrayEntry enumValue(int value)
	{
		_cacheEntryData = CodecFactory.createEnum();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Enum)_cacheEntryData).value(value) )
		{
			String errText = errorString().append("Attempt to specify invalid enum. Passed in value is='" )
					.append( value ).append( "." ).toString();
				throw ommIUExcept().message(errText);
		}
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.ENUM;
		
		return this;
	}

	@Override
	public OmmArrayEntry buffer(ByteBuffer value)
	{
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_cacheEntryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_cacheEntryData);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.BUFFER;
		
		return this;
	}

	@Override
	public OmmArrayEntry ascii(String value)
	{
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_cacheEntryData = CodecFactory.createBuffer();
		((Buffer)_cacheEntryData).data(value);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING;
		
		return this;
	}

	@Override
	public OmmArrayEntry utf8(ByteBuffer value)
	{
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_cacheEntryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_cacheEntryData);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING;
		
		return this;
	}

	@Override
	public OmmArrayEntry utf8(String value)
	{
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_cacheEntryData = CodecFactory.createBuffer();
		((Buffer)_cacheEntryData).data(value);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING;
		
		return this;
	}

	@Override
	public OmmArrayEntry rmtes(ByteBuffer value)
	{
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");
		
		_cacheEntryData = CodecFactory.createBuffer();
		Utilities.copy(value, (Buffer)_cacheEntryData);
		
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeInt()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.INT;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeUInt()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.UINT;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeReal()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.REAL;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeFloat()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.FLOAT;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeDouble()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.DOUBLE;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeDate()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.DATE;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeTime()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.TIME;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeDateTime()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.DATETIME;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeQos()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.QOS;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeState()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.STATE;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeEnum()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.ENUM;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeBuffer()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.BUFFER;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeAscii()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeUtf8()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING;
		
		return this;
	}

	@Override
	public OmmArrayEntry codeRmtes()
	{
		_cacheEntryDataType = com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING;
		
		return this;
	}
}