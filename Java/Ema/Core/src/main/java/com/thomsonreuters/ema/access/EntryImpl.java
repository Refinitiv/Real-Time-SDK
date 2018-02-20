///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2015. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;


import com.thomsonreuters.ema.access.AckMsg;
import com.thomsonreuters.ema.access.Data.DataCode;
import com.thomsonreuters.ema.access.DataType;
import com.thomsonreuters.ema.access.DataType.DataTypes;
import com.thomsonreuters.upa.codec.CodecReturnCodes;
import com.thomsonreuters.ema.access.ElementList;
import com.thomsonreuters.ema.access.FieldList;
import com.thomsonreuters.ema.access.FilterList;
import com.thomsonreuters.ema.access.GenericMsg;
import com.thomsonreuters.ema.access.Map;
import com.thomsonreuters.ema.access.OmmAnsiPage;
import com.thomsonreuters.ema.access.OmmArray;
import com.thomsonreuters.ema.access.OmmAscii;
import com.thomsonreuters.ema.access.OmmBuffer;
import com.thomsonreuters.ema.access.OmmDate;
import com.thomsonreuters.ema.access.OmmDateTime;
import com.thomsonreuters.ema.access.OmmDouble;
import com.thomsonreuters.ema.access.OmmEnum;
import com.thomsonreuters.ema.access.OmmError;
import com.thomsonreuters.ema.access.OmmFloat;
import com.thomsonreuters.ema.access.OmmInt;
import com.thomsonreuters.ema.access.OmmOpaque;
import com.thomsonreuters.ema.access.OmmQos;
import com.thomsonreuters.ema.access.OmmReal;
import com.thomsonreuters.ema.access.OmmRmtes;
import com.thomsonreuters.ema.access.OmmState;
import com.thomsonreuters.ema.access.OmmTime;
import com.thomsonreuters.ema.access.OmmUInt;
import com.thomsonreuters.ema.access.OmmUtf8;
import com.thomsonreuters.ema.access.OmmXml;
import com.thomsonreuters.ema.access.PostMsg;
import com.thomsonreuters.ema.access.RefreshMsg;
import com.thomsonreuters.ema.access.UpdateMsg;
import com.thomsonreuters.ema.access.ReqMsg;
import com.thomsonreuters.ema.access.Series;
import com.thomsonreuters.ema.access.StatusMsg;
import com.thomsonreuters.ema.access.Vector;
import com.thomsonreuters.upa.codec.Date;
import com.thomsonreuters.upa.codec.DateTime;
import com.thomsonreuters.upa.codec.Time;
import com.thomsonreuters.upa.valueadd.common.VaNode;
import com.thomsonreuters.upa.codec.State;

abstract class EntryImpl extends VaNode
{
	private OmmInvalidUsageExceptionImpl 	_ommIUExcept; 
	private OmmOutOfRangeExceptionImpl 	_ommOORExcept;
	private StringBuilder _errorString;
	protected DataImpl 	_load;
	protected StringBuilder	_toString = new StringBuilder();
	protected int _previousEncodingType = com.thomsonreuters.upa.codec.DataTypes.UNKNOWN;
	protected Object _entryData;
	
	EntryImpl() {}
	
	EntryImpl(DataImpl load)
	{
		_load = load;
	}
	
	public DataImpl load()
	{
		return _load;
	}
	
	public int loadType()
	{
		return _load.dataType();
	}

	public int code()
	{
		return _load.code();
	}
	
	public ReqMsg reqMsg()
	{
		if (_load.dataType() != DataTypes.REQ_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to reqMsg() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (ReqMsg)_load;
	}
	
	public RefreshMsg refreshMsg()
	{
		if (_load.dataType() != DataTypes.REFRESH_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to refreshMsg() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (RefreshMsg)_load;
	}
	
	public UpdateMsg updateMsg()
	{
		if (_load.dataType() != DataTypes.UPDATE_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to UpdateMsg() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (UpdateMsg)_load;
	}

	public StatusMsg statusMsg()
	{
		if (_load.dataType() != DataTypes.STATUS_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to statusMsg() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (StatusMsg)_load;
	}

	public PostMsg postMsg()
	{
		if (_load.dataType() != DataTypes.POST_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to postMsg() while actual entry data type is ")
			 	 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (PostMsg)_load;
	}
	
	public AckMsg ackMsg()
	{
		if (_load.dataType() != DataTypes.ACK_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ackMsg() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
	
		return (AckMsg)_load;
	}
	
	public GenericMsg genericMsg()
	{
		if (_load.dataType() != DataTypes.GENERIC_MSG)
		{
			StringBuilder error = errorString();
			error.append("Attempt to genericMsg() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
	
		return (GenericMsg)_load;
	}

	public FieldList fieldList()
	{
		if (_load.dataType() != DataTypes.FIELD_LIST)
		{
			StringBuilder error = errorString();
			error.append("Attempt to fieldList() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (FieldList)_load;
	}
	
	public ElementList elementList()
	{
		if (_load.dataType() != DataTypes.ELEMENT_LIST)
		{
			StringBuilder error = errorString();
			error.append("Attempt to elementList() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (ElementList)_load;
	}
	
	public OmmArray array()
	{
		if (_load.dataType() != DataTypes.ARRAY)
		{
			StringBuilder error = errorString();
			error.append("Attempt to array() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (OmmArray)_load;
	}

	public Map map()
	{
		if (_load.dataType() != DataTypes.MAP)
		{
			StringBuilder error = errorString();
			error.append("Attempt to map() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (Map)_load;
	}
	
	public Vector vector()
	{
		if (_load.dataType() != DataTypes.VECTOR)
		{
			StringBuilder error = errorString();
			error.append("Attempt to vector() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (Vector)_load;
	}
	
	public Series series()
	{
		if (_load.dataType() != DataTypes.SERIES)
		{
			StringBuilder error = errorString();
			error.append("Attempt to series() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (Series)_load;
	}
	
	public FilterList filterList()
	{
		if (_load.dataType() != DataTypes.FILTER_LIST)
		{
			StringBuilder error = errorString();
			error.append("Attempt to filterList() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (FilterList)_load;
	}
	
	public OmmOpaque opaque()
	{
		if (_load.dataType() != DataTypes.OPAQUE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to opaque() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (OmmOpaque)_load;
	}
	
	public OmmXml xml()
	{
		if (_load.dataType() != DataTypes.XML)
		{
			StringBuilder error = errorString();
			error.append("Attempt to xml() while actual entry data type is ")
			 	 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (OmmXml)_load;
	}
	
	public OmmAnsiPage ansiPage()
	{
		if (_load.dataType() != DataTypes.ANSI_PAGE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ansiPage() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		
		return (OmmAnsiPage)_load;
	}
	
	public long intValue()
	{
		if (_load.dataType() != DataTypes.INT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to intValue() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to intValue() while entry data is blank.");

		return ((OmmInt)_load).intValue();
	}
	
	public OmmInt ommIntValue()
	{
		if (_load.dataType() != DataTypes.INT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ommIntValue() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to ommIntValue() while entry data is blank.");

		return (OmmInt)_load;
	}
	
	public long uintValue()
	{
		if (_load.dataType() != DataTypes.UINT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to uintValue() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to uintValue() while entry data is blank.");

		return ((OmmUInt)_load).longValue();
	}
	
	public OmmUInt ommUIntValue()
	{
		if (_load.dataType() != DataTypes.UINT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ommUIntValue() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to ommUIntValue() while entry data is blank.");

		return (OmmUInt)_load;
	}
	
	public OmmReal real()
	{
		if (_load.dataType() != DataTypes.REAL)
		{
			StringBuilder error = errorString();
			error.append("Attempt to real() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to real() while entry data is blank.");

		return (OmmReal)_load;
	}

	public float floatValue()
	{
		if (_load.dataType() != DataTypes.FLOAT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to floatValue() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to floatValue() while entry data is blank.");

		return ((OmmFloat)_load).floatValue();
	}
	
	public OmmFloat ommFloatValue()
	{
		if (_load.dataType() != DataTypes.FLOAT)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ommFloatValue() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to ommFloatValue() while entry data is blank.");

		return (OmmFloat)_load;
	}
	
	public double doubleValue()
	{
		if (_load.dataType() != DataTypes.DOUBLE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to doubleValue() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to doubleValue() while entry data is blank.");

		return ((OmmDouble)_load).doubleValue();
	}

	public OmmDouble ommDoubleValue()
	{
		if (_load.dataType() != DataTypes.DOUBLE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ommDoubleValue() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to ommDoubleValue() while entry data is blank.");

		return (OmmDouble)_load;
	}

	public OmmDate date()
	{
		if (_load.dataType() != DataTypes.DATE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to date() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to date() while entry data is blank.");

		return (OmmDate)_load;
	}
	
	public OmmTime time()
	{
		if (_load.dataType() != DataTypes.TIME)
		{
			StringBuilder error = errorString();
			error.append("Attempt to time() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to time() while entry data is blank.");

		return (OmmTime)_load;
	}
	
	public OmmDateTime dateTime()
	{
		if (_load.dataType() != DataTypes.DATETIME)
		{
			StringBuilder error = errorString();
			error.append("Attempt to dateTime() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to dateTime() while entry data is blank.");

		return (OmmDateTime)_load;
	}

	public OmmQos qos()
	{
		if (_load.dataType() != DataTypes.QOS)
		{
			StringBuilder error = errorString();
			error.append("Attempt to qos() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to qos() while entry data is blank.");

		return (OmmQos)_load;
	}
	
	public OmmState state()
	{
		if (_load.dataType() != DataTypes.STATE)
		{
			StringBuilder error = errorString();
			error.append("Attempt to state() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to state() while entry data is blank.");

		return (OmmState)_load;
	}

	public int enumValue()
	{
		if (_load.dataType() != DataTypes.ENUM)
		{
			StringBuilder error = errorString();
			error.append("Attempt to enumValue() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to enumValue() while entry data is blank.");

		return ((OmmEnum)_load).enumValue();
	}
	
	public OmmEnum ommEnumValue()
	{
		if (_load.dataType() != DataTypes.ENUM)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ommEnumValue() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to ommEnumValue() while entry data is blank.");

		return (OmmEnum)_load;
	}
	
	public OmmBuffer buffer()
	{
		if (_load.dataType() != DataTypes.BUFFER)
		{
			StringBuilder error = errorString();
			error.append("Attempt to buffer() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to buffer() while entry data is blank.");

		return (OmmBuffer)_load;
	}
	
	public OmmAscii ascii()
	{
		if (_load.dataType() != DataTypes.ASCII)
		{
			StringBuilder error = errorString();
			error.append("Attempt to ascii() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to ascii() while entry data is blank.");

		return (OmmAscii)_load;
	}
	
	public OmmUtf8 utf8()
	{
		if (_load.dataType() != DataTypes.UTF8)
		{
			StringBuilder error = errorString();
			error.append("Attempt to utf8() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to utf8() while entry data is blank.");

		return (OmmUtf8)_load;
	}
	
	public OmmRmtes rmtes()
	{
		if (_load.dataType() != DataTypes.RMTES)
		{
			StringBuilder error = errorString();
			error.append("Attempt to rmtes() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}
		else if (DataCode.BLANK == _load.code())
			throw ommIUExcept().message("Attempt to rmtes() while entry data is blank.");

		return (OmmRmtes)_load;
	}

	public OmmError error()
	{
		if  (_load.dataType() != DataTypes.ERROR)
		{
			StringBuilder error = errorString();
			error.append("Attempt to error() while actual entry data type is ")
				 .append(DataType.asString(_load.dataType()));
			throw ommIUExcept().message(error.toString());
		}

		return (OmmError)_load;
	}
	
	void load(DataImpl load)
	{
		_load = load;
	}
	
	OmmInvalidUsageExceptionImpl ommIUExcept()
	{
		if (_ommIUExcept == null)
			_ommIUExcept = new OmmInvalidUsageExceptionImpl();
		
		return _ommIUExcept;
	}
	
	OmmOutOfRangeExceptionImpl ommOORExcept()
	{
		if (_ommOORExcept == null)
			_ommOORExcept = new OmmOutOfRangeExceptionImpl();
		
		return _ommOORExcept;
	}
	
	StringBuilder errorString()
	{
		if (_errorString == null)
			_errorString = new StringBuilder(64);
		else
			_errorString.setLength(0);
			
		return _errorString;
	}
	
	Object dateTimeValue(int year, int month, int day, int hour, int minute, int second,	int millisecond, int microsecond, int nanosecond)
	{
		DateTime cacheEntryData; 
		
		if ( _previousEncodingType != com.thomsonreuters.upa.codec.DataTypes.DATETIME )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			cacheEntryData = GlobalPool.getDateTime();
			GlobalPool.unlock();
			
			_previousEncodingType = com.thomsonreuters.upa.codec.DataTypes.DATETIME;
		}
		else
		{
			cacheEntryData = (DateTime)_entryData;
		}
		
		if (CodecReturnCodes.SUCCESS != (cacheEntryData).year(year) ||
				CodecReturnCodes.SUCCESS !=  (cacheEntryData).month(month) || 
				CodecReturnCodes.SUCCESS !=  (cacheEntryData).day(day) ||
				CodecReturnCodes.SUCCESS !=  (cacheEntryData).hour(hour) ||
				CodecReturnCodes.SUCCESS !=  (cacheEntryData).minute(minute) || 
				CodecReturnCodes.SUCCESS !=  (cacheEntryData).second(second) ||
				CodecReturnCodes.SUCCESS !=  (cacheEntryData).millisecond(millisecond) || 
				CodecReturnCodes.SUCCESS !=  (cacheEntryData).microsecond(microsecond) || 
				CodecReturnCodes.SUCCESS !=  (cacheEntryData).nanosecond(nanosecond) || 
				!(cacheEntryData).isValid())
		{
			String errText = errorString().append("Attempt to specify invalid time. Passed in value is='" )
																			.append( month ).append( " / " )
																			.append( day ).append( " / " )
																			.append( year ).append( " / " )
																			.append( hour ).append( ":" )
																			.append( minute ).append( ":" )
																			.append( second )	.append( "." )
																			.append( millisecond ).append( "." )
																			.append( microsecond ).append( "." )
																			.append( nanosecond ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}
		
		return cacheEntryData;
	}
	
	Object dateValue(int year, int month, int day)
	{
		Date  cacheEntryData;
		
		if ( _previousEncodingType != com.thomsonreuters.upa.codec.DataTypes.DATE )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			cacheEntryData = GlobalPool.getDate();
			GlobalPool.unlock();
			
			_previousEncodingType = com.thomsonreuters.upa.codec.DataTypes.DATE;
		}
		else
		{
			cacheEntryData = (Date)_entryData;
		}
		
		
		if (CodecReturnCodes.SUCCESS != (cacheEntryData).year(year) || 
				CodecReturnCodes.SUCCESS != (cacheEntryData).month(month) || 
				CodecReturnCodes.SUCCESS != (cacheEntryData).day(day) || 
				!(cacheEntryData).isValid())
		{
			String errText = errorString().append("Attempt to specify invalid date. Passed in value is='" )
										.append( month ).append( " / " )
										.append( day ).append( " / " )
										.append( year ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}
		
		return cacheEntryData;
	}
	
	Object  timeValue(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond)
	{
		Time cacheEntryData;
		
		if ( _previousEncodingType != com.thomsonreuters.upa.codec.DataTypes.TIME )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			cacheEntryData = GlobalPool.getTime();
			GlobalPool.unlock();
			
			_previousEncodingType = com.thomsonreuters.upa.codec.DataTypes.TIME;
		}
		else
		{
			cacheEntryData = (Time)_entryData;
		}
		
		if (CodecReturnCodes.SUCCESS != (cacheEntryData).hour(hour) ||
				CodecReturnCodes.SUCCESS != (cacheEntryData).minute(minute) || 
				CodecReturnCodes.SUCCESS != (cacheEntryData).second(second) || 
				CodecReturnCodes.SUCCESS != (cacheEntryData).millisecond(millisecond) ||
				CodecReturnCodes.SUCCESS != (cacheEntryData).microsecond(microsecond) || 
				CodecReturnCodes.SUCCESS != (cacheEntryData).nanosecond(nanosecond) || 
				!(cacheEntryData).isValid() )
		{
			String errText = errorString().append("Attempt to specify invalid time. Passed in value is='" )
																			.append( hour ).append( ":" )
																			.append( minute ).append( ":" )
																			.append( second )	.append( "." )
																			.append( millisecond ).append( "." )
																			.append( microsecond ).append( "." )
																			.append( nanosecond ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}
		
		return cacheEntryData;
	}

	Object stateValue(int streamState, int dataState, int statusCode, String statusText)
    {
        State cacheEntryData;
        
        if ( _previousEncodingType != com.thomsonreuters.upa.codec.DataTypes.STATE )
		{
			GlobalPool.lock();
			GlobalPool.returnPool(_previousEncodingType, _entryData);
			cacheEntryData = GlobalPool.getState();
			GlobalPool.unlock();
			
			_previousEncodingType = com.thomsonreuters.upa.codec.DataTypes.STATE;
		}
		else
		{
			cacheEntryData = (State)_entryData;
		}
        
        if (CodecReturnCodes.SUCCESS != (cacheEntryData).streamState(streamState) ||
                CodecReturnCodes.SUCCESS != (cacheEntryData).dataState(dataState) ||
                CodecReturnCodes.SUCCESS != (cacheEntryData).code(statusCode) || 
                CodecReturnCodes.SUCCESS != (cacheEntryData).text().data(statusText))
        {
            String errText = errorString().append("Attempt to specify invalid state. Passed in value is='" )
                .append( streamState ).append( " / " )
                .append( dataState ).append( " / " )
                .append( statusCode ).append( "/ " )
                .append( statusText ).append( "." ).toString();
            throw ommIUExcept().message(errText);
        }

        return cacheEntryData;
    }


}
