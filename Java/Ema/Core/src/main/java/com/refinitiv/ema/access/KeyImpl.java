///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright (C) 2019 Refinitiv. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.refinitiv.ema.access;

import com.refinitiv.ema.access.Data;
import com.refinitiv.ema.access.DataType;
import com.refinitiv.ema.access.Key;
import com.refinitiv.ema.access.OmmAscii;
import com.refinitiv.ema.access.OmmBuffer;
import com.refinitiv.ema.access.OmmDate;
import com.refinitiv.ema.access.OmmDateTime;
import com.refinitiv.ema.access.OmmDouble;
import com.refinitiv.ema.access.OmmEnum;
import com.refinitiv.ema.access.OmmError;
import com.refinitiv.ema.access.OmmFloat;
import com.refinitiv.ema.access.OmmInt;
import com.refinitiv.ema.access.OmmQos;
import com.refinitiv.ema.access.OmmReal;
import com.refinitiv.ema.access.OmmRmtes;
import com.refinitiv.ema.access.OmmState;
import com.refinitiv.ema.access.OmmTime;
import com.refinitiv.ema.access.OmmUInt;
import com.refinitiv.ema.access.OmmUtf8;
import com.refinitiv.ema.access.DataType.DataTypes;

class KeyImpl implements Key
{
	private Data 						 _data;
	private OmmInvalidUsageExceptionImpl _ommIUExcept; 
	private StringBuilder 				 _errorString;

	@Override
	public int dataType()
	{
		return _data.dataType();
	}

	@Override
	public Data data()
	{
		return _data;
	}

	@Override
	public long intValue()
	{
		if (_data.dataType() != DataTypes.INT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to intValue() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return ((OmmInt)_data).intValue();
	}

	@Override
	public OmmInt ommIntValue()
	{
		if (_data.dataType() != DataTypes.INT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ommIntValue() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmInt)_data;
	}

	@Override
	public long uintValue()
	{
		if (_data.dataType() != DataTypes.UINT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to uintValue() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return ((OmmUInt)_data).longValue();
	}

	@Override
	public OmmUInt ommUIntValue()
	{
		if (_data.dataType() != DataTypes.UINT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ommUIntValue() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmUInt)_data;
	}

	@Override
	public OmmReal real()
	{
		if (_data.dataType() != DataTypes.REAL)
		{
			StringBuilder error = errorString();
			error.append("Attempt to real() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmReal)_data;
	}

	@Override
	public float floatValue()
	{
		if (_data.dataType() != DataTypes.FLOAT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to floatValue() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return ((OmmFloat)_data).floatValue();
	}

	@Override
	public OmmFloat ommFloatValue()
	{
		if (_data.dataType() != DataTypes.FLOAT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ommFloatValue() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmFloat)_data;
	}

	@Override
	public double doubleValue()
	{
		if (_data.dataType() != DataTypes.DOUBLE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to doubleValue() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return ((OmmDouble)_data).doubleValue();
	}

	@Override
	public OmmDouble ommDoubleValue()
	{
		if (_data.dataType() != DataTypes.DOUBLE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ommDoubleValue() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmDouble)_data;
	}

	@Override
	public OmmDate date()
	{
		if (_data.dataType() != DataTypes.DATE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to date() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmDate)_data;
	}

	@Override
	public OmmTime time()
	{
		if (_data.dataType() != DataTypes.TIME)
		{
			StringBuilder error = errorString();
			error.append("Attempt to time() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmTime)_data;
	}

	@Override
	public OmmDateTime dateTime()
	{
		if (_data.dataType() != DataTypes.DATETIME)
		{
			StringBuilder error = errorString();
			error.append("Attempt to dateTime() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmDateTime)_data;
	}

	@Override
	public OmmQos qos()
	{
		if (_data.dataType() != DataTypes.QOS)
		{
			StringBuilder error = errorString();
			error.append("Attempt to qos() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmQos)_data;
	}

	@Override
	public OmmState state()
	{
		if (_data.dataType() != DataTypes.STATE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to state() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmState)_data;
	}

	@Override
	public int enumValue()
	{
		if (_data.dataType() != DataTypes.ENUM)
		{
			StringBuilder error = errorString();
			error.append("Attempt to enumValue() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return ((OmmEnum)_data).enumValue();
	}

	@Override
	public OmmEnum ommEnumValue()
	{
		if (_data.dataType() != DataTypes.ENUM)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ommEnumValue() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmEnum)_data;
	}

	@Override
	public OmmBuffer buffer()
	{
		if (_data.dataType() != DataTypes.BUFFER)
		{
			StringBuilder error = errorString();
			error.append("Attempt to buffer() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}
		
		return (OmmBuffer)_data;
	}

	@Override
	public OmmAscii ascii()
	{
		if (_data.dataType() != DataTypes.ASCII)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ascii() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmAscii)_data;
	}

	@Override
	public OmmUtf8 utf8()
	{
		if (_data.dataType() != DataTypes.UTF8)
		{
			StringBuilder error = errorString();
			error.append("Attempt to utf8() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmUtf8)_data;
	}

	@Override
	public OmmRmtes rmtes()
	{
		if (_data.dataType() != DataTypes.RMTES)
		{
			StringBuilder error = errorString();
			error.append("Attempt to rmtes() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmRmtes)_data;
	}

	@Override
	public OmmError error()
	{
		if (_data.dataType() != DataTypes.ERROR)
		{
			StringBuilder error = errorString();
			error.append("Attempt to error() while actual data type is ")
				 .append(DataType.asString(_data.dataType()));
			throw ommIUExcept().message(error.toString(), OmmInvalidUsageException.ErrorCode.INVALID_OPERATION);
		}

		return (OmmError)_data;
	}
	
	Key data(Data data)
	{
		_data = data;
		return this;
	}
	
	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();
		
		return _ommIUExcept;
	}
	
	StringBuilder errorString()
	{
		if (_errorString == null)
			_errorString = new StringBuilder();
		else
			_errorString.setLength(0);
			
		return _errorString;
	}
}