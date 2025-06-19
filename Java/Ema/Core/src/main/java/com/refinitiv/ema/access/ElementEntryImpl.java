/*|-----------------------------------------------------------------------------
 *|            This source code is provided under the Apache 2.0 license
 *|  and is provided AS IS with no warranty or guarantee of fit for purpose.
 *|                See the project's LICENSE.md for details.
 *|           Copyright (C) 2020,2024 LSEG. All rights reserved.
 *|-----------------------------------------------------------------------------
 */

package com.refinitiv.ema.access;

import java.math.BigInteger;
import java.nio.ByteBuffer;

import com.refinitiv.eta.codec.Buffer;
import com.refinitiv.eta.codec.CodecFactory;
import com.refinitiv.eta.codec.CodecReturnCodes;

class ElementEntryImpl extends EntryImpl implements ElementEntry
{
	protected com.refinitiv.eta.codec.ElementEntry	_rsslElementEntry;
	
	ElementEntryImpl() 
	{
		_rsslElementEntry = CodecFactory.createElementEntry();
	}
	
	ElementEntryImpl(com.refinitiv.eta.codec.ElementEntry rsslElementEntry, DataImpl load)
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
		if ( _load == null )
			return "\nEntity is not encoded yet. Complete encoding to use this method.\n";
		
		_toString.setLength(0);
		_toString.append("ElementEntry ")
				.append(" name=\"").append(name()).append("\"")
				.append(" dataType=\"").append(DataType.asString(_load.dataType()));

		if ( _load.dataType() >= DataType.DataTypes.FIELD_LIST || _load.dataType() ==  DataType.DataTypes.ARRAY )
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
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry refreshMsg(String name, RefreshMsg value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry statusMsg(String name, StatusMsg value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry updateMsg(String name, UpdateMsg value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry postMsg(String name, PostMsg value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry ackMsg(String name, AckMsg value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry genericMsg(String name, GenericMsg value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.MSG, (DataImpl) value);
	}

	@Override
	public ElementEntry fieldList(String name, FieldList value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.FIELD_LIST, (DataImpl) value);
	}

	@Override
	public ElementEntry elementList(String name, ElementList value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.ELEMENT_LIST, (DataImpl) value);
	}

	@Override
	public ElementEntry map(String name, Map value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.MAP, (DataImpl) value);
	}

	@Override
	public ElementEntry vector(String name, Vector value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.VECTOR, (DataImpl) value);
	}

	@Override
	public ElementEntry series(String name, Series value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.SERIES, (DataImpl) value);
	}

	@Override
	public ElementEntry filterList(String name, FilterList value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.FILTER_LIST, (DataImpl) value);
	}

	@Override
	public ElementEntry opaque(String name, OmmOpaque value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.OPAQUE, (DataImpl) value);
	}

	@Override
	public ElementEntry xml(String name, OmmXml value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.XML, (DataImpl) value);
	}

	@Override
	public ElementEntry json(String name, OmmJson value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.JSON, (DataImpl) value);
	}

	@Override
	public ElementEntry ansiPage(String name, OmmAnsiPage value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.ANSI_PAGE, (DataImpl) value);
	}

	@Override
	public ElementEntry intValue(String name, long value)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.INT);

		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.INT )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getInt();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.INT;
		}
		
		((com.refinitiv.eta.codec.Int)_entryData).value(value);
		
		return this;
	}

	@Override
	public ElementEntry uintValue(String name, long value)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.UINT);

		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.UINT )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getUInt();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.UINT;
		}
		
		((com.refinitiv.eta.codec.UInt)_entryData).value(value) ;
		
		return this;
	}

	@Override
	public ElementEntry uintValue(String name, BigInteger value)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.UINT);

		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.UINT )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getUInt();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.UINT;
		}
		
		((com.refinitiv.eta.codec.UInt)_entryData).value(value) ;
		
		return this;
	}

	@Override
	public ElementEntry real(String name, long mantissa, int magnitudeType)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.REAL);

		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.REAL )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getReal();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.REAL;
		}
		
		int ret;
		if (CodecReturnCodes.SUCCESS != (ret = ((com.refinitiv.eta.codec.Real)_entryData).value(mantissa, magnitudeType)) )
		{
			String errText = errorString().append("Attempt to specify invalid real value. Passed mantissa, magnitudeType are='" )
										.append( mantissa ).append( " / " )
										.append( magnitudeType ).append( "'." ).toString();
			throw ommIUExcept().message(errText, ret);
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
		entryValue(name, com.refinitiv.eta.codec.DataTypes.REAL);

		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.REAL )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getReal();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.REAL;
		}
		
		int ret;
		if (CodecReturnCodes.SUCCESS != (ret = ((com.refinitiv.eta.codec.Real)_entryData).value(value, magnitudeType)) )
		{
			String errText = errorString().append("Attempt to specify invalid real value. Passed in value,  magnitudeType are='" )
										.append( value ).append( " / " )
										.append( magnitudeType ).append( "'." ).toString();
			throw ommIUExcept().message(errText, ret);
		}
		
		return this;
	}

	@Override
	public ElementEntry floatValue(String name, float value)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.FLOAT);

		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.FLOAT )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getFloat();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.FLOAT;
		}
		
		((com.refinitiv.eta.codec.Float)_entryData).value(value);
		
		return this;
	}

	@Override
	public ElementEntry doubleValue(String name, double value)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.DOUBLE);

		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.DOUBLE )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getDouble();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.DOUBLE;
		}
		
		((com.refinitiv.eta.codec.Double)_entryData).value(value);
		
		return this;
	}

	@Override
	public ElementEntry date(String name, int year, int month, int day)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.DATE);

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
		entryValue(name, com.refinitiv.eta.codec.DataTypes.TIME);

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
		entryValue(name, com.refinitiv.eta.codec.DataTypes.DATETIME);

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
		entryValue(name, com.refinitiv.eta.codec.DataTypes.QOS);

		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.QOS )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getQos();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.QOS;
		}
		
		Utilities.toRsslQos(rate, timeliness, (com.refinitiv.eta.codec.Qos)_entryData);
		
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
		entryValue(name, com.refinitiv.eta.codec.DataTypes.STATE);

		_entryData = stateValue(streamState, dataState, statusCode, statusText);
		
		return this;
	}

	@Override
	public ElementEntry enumValue(String name, int value)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.ENUM);

		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.ENUM )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getEnum();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.ENUM; 
		}
		
		int ret;
		if (CodecReturnCodes.SUCCESS != (ret =((com.refinitiv.eta.codec.Enum)_entryData).value(value)) )
		{
			String errText = errorString().append("Attempt to specify invalid enum. Passed in value is='" )
					.append( value ).append( "." ).toString();
				throw ommIUExcept().message(errText, ret);
		}
		
		return this;
	}

	@Override
	public ElementEntry buffer(String name, ByteBuffer value)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.BUFFER);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.BUFFER )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getBuffer();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.BUFFER;
		}
		
		Utilities.copy(value, (Buffer)_entryData);
		
		return this;
	}

	@Override
	public ElementEntry ascii(String name, String value)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.ASCII_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.BUFFER )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getBuffer();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.BUFFER;
		}
		
		((Buffer)_entryData).data(value);
		
		return this;
	}

	@Override
	public ElementEntry utf8(String name, ByteBuffer value)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.UTF8_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.BUFFER )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getBuffer();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.BUFFER;
		}
		
		Utilities.copy(value, (Buffer)_entryData);

		return this;
	}

	@Override
	public ElementEntry utf8(String name, String value)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.UTF8_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.BUFFER )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getBuffer();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.BUFFER;
		}
		
		((Buffer)_entryData).data(value);
		
		return this;
	}

	@Override
	public ElementEntry rmtes(String name, ByteBuffer value)
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.RMTES_STRING);

		if (value == null)
			throw ommIUExcept().message("Passed in value is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		
		if ( _previousEncodingType != com.refinitiv.eta.codec.DataTypes.BUFFER )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			_entryData = GlobalPool.getBuffer();
			GlobalPool.unlock();
			
			_previousEncodingType = com.refinitiv.eta.codec.DataTypes.BUFFER;
		}
		
		Utilities.copy(value, (Buffer)_entryData);
		
		return this;
	}

	@Override
	public ElementEntry array(String name, OmmArray value)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.ARRAY, (DataImpl) value);
	}

	@Override
	public ElementEntry codeInt(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.INT);
	}

	@Override
	public ElementEntry codeUInt(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.UINT);
	}

	@Override
	public ElementEntry codeReal(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.REAL);
	}

	@Override
	public ElementEntry codeFloat(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.FLOAT);
	}

	@Override
	public ElementEntry codeDouble(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.DOUBLE);
	}

	@Override
	public ElementEntry codeDate(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.DATE);
	}

	@Override
	public ElementEntry codeTime(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.TIME);
	}

	@Override
	public ElementEntry codeDateTime(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.DATETIME);
	}

	@Override
	public ElementEntry codeQos(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.QOS);
	}

	@Override
	public ElementEntry codeState(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.STATE);
	}

	@Override
	public ElementEntry codeEnum(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.ENUM);
	}

	@Override
	public ElementEntry codeBuffer(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.BUFFER);
	}

	@Override
	public ElementEntry codeAscii(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.ASCII_STRING);
	}

	@Override
	public ElementEntry codeUtf8(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.UTF8_STRING);
	}

	@Override
	public ElementEntry codeRmtes(String name)
	{
		return entryValue(name, com.refinitiv.eta.codec.DataTypes.RMTES_STRING);
	}

	private ElementEntry entryValue(String name, int rsslDataType, DataImpl value)
	{
		if (name == null)
			throw ommIUExcept().message("Passed in name is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);
		if (value == null)
			throw ommIUExcept().message("Passed in value is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);

		_rsslElementEntry.name().data(name);
		_rsslElementEntry.dataType(rsslDataType);
		
		Utilities.copy(value.encodedData(), _rsslElementEntry.encodedData());

		return this;
	}

	private ElementEntry entryValue(String name, int rsslDataType)
	{
		if (name == null)
			throw ommIUExcept().message("Passed in name is null", OmmInvalidUsageException.ErrorCode.INVALID_ARGUMENT);

		_rsslElementEntry.name().data(name);
		_rsslElementEntry.dataType(rsslDataType);

		return this;
	}

	@Override
	public ElementEntry noData(String name) 
	{
		entryValue(name, com.refinitiv.eta.codec.DataTypes.NO_DATA);
		return this;
	}
}
