///*|-----------------------------------------------------------------------------
// *|            This source code is provided under the Apache 2.0 license      --
// *|  and is provided AS IS with no warranty or guarantee of fit for purpose.  --
// *|                See the project's LICENSE.md for details.                  --
// *|           Copyright Thomson Reuters 2018. All rights reserved.            --
///*|-----------------------------------------------------------------------------

package com.thomsonreuters.ema.access;

import java.math.BigInteger;
import java.nio.ByteBuffer;

import com.thomsonreuters.upa.codec.Buffer;
import com.thomsonreuters.upa.codec.CodecFactory;
import com.thomsonreuters.upa.codec.CodecReturnCodes;

class MapEntryImpl extends EntryImpl implements MapEntry
{
	private final static String ADD_STRING 				= "Add";
	private final static String UPDATE_STRING 			= "Update";
	private final static String DELETE_STRING 			= "Delete";
	private final static String DEFAULTACTION_STRING 	= "Unknown MapAction value ";
	
	private ByteBuffer _permData;
	private EmaObjectManager _objManager;
	protected KeyImpl _keyDataDecoded = new KeyImpl();
	protected com.thomsonreuters.upa.codec.MapEntry	_rsslMapEntry;
	protected Object _keyData;
	protected int _keyDataType;
	protected int _entryDataType;

	MapEntryImpl() 
	{
		_rsslMapEntry = CodecFactory.createMapEntry();
	}
	
	MapEntryImpl(com.thomsonreuters.upa.codec.MapEntry rsslMapEntry, DataImpl mapEntryKey, DataImpl load, EmaObjectManager objManager)
	{
		super(load);
		_rsslMapEntry = rsslMapEntry;
		_keyDataDecoded.data(mapEntryKey);
		_objManager = objManager;
	}
	
	@Override
	public String mapActionAsString()
	{
		switch (action())
		{
			case MapAction.ADD:
				return ADD_STRING;
			case MapAction.UPDATE:
				return UPDATE_STRING;
			case MapAction.DELETE:
				return DELETE_STRING;
			default:
				return DEFAULTACTION_STRING + action();
		}
	}

	@Override
	public boolean hasPermissionData()
	{
		return _rsslMapEntry.checkHasPermData();
	}

	@Override
	public Key key()
	{
		return _keyDataDecoded;
	}

	@Override
	public int action()
	{
		return _rsslMapEntry.action();
	}

	@Override
	public ByteBuffer permissionData()
	{
		if (!hasPermissionData())
			throw ommIUExcept().message("Attempt to permissionData() while it is NOT set.");
		
		_permData = Utilities.copyFromPool( _rsslMapEntry.permData(), _permData, _objManager);
		return _permData;
	}
	
	@Override
	public String toString()
	{
		if ( _load == null )
			return "\nDecoding of just encoded object in the same application is not supported\n";
		
		_toString.setLength(0);
		_toString.append("MapEntry ")
				.append(" action=\"").append(mapActionAsString()).append("\"")
				.append(" value=\"").append(key().toString());
				
		if (hasPermissionData())
		{
			_toString.append("\" permissionData=\"").append(permissionData()).append("\"");
			Utilities.asHexString(_toString, permissionData()).append("\"");
		}
		
		_toString.append("\" dataType=\"").append(DataType.asString(_load.dataType())).append("\"\n");
		_toString.append(_load.toString(1));
		Utilities.addIndent(_toString, 0).append("MapEntryEnd\n");

		return _toString.toString();
	}
	
	@Override
	public MapEntry keyInt(long key, int action, ComplexType value)
	{
		return keyInt(key, action, value, null);
	}

	@Override
	public MapEntry keyInt(long key, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createInt();
		((com.thomsonreuters.upa.codec.Int) _keyData).value(key);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.INT;
		
		return this;
	}

	@Override
	public MapEntry keyUInt(long key, int action, ComplexType value)
	{
		return keyUInt(key, action, value, null);
	}

	@Override
	public MapEntry keyUInt(long key, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt) _keyData).value(key);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.UINT;
						
		return this;
	}

	@Override
	public MapEntry keyUInt(BigInteger key, int action, ComplexType value)
	{
		return keyUInt(key, action, value, null);
	}

	@Override
	public MapEntry keyUInt(BigInteger key, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt) _keyData).value(key);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.UINT;
		
		return this;
	}

	@Override
	public MapEntry keyReal(long mantissa, int magnitudeType, int action, ComplexType value)
	{
		return keyReal(mantissa, magnitudeType, action, value, null);
	}

	@Override
	public MapEntry keyReal(long mantissa, int magnitudeType, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createReal();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Real)_keyData).value(mantissa, magnitudeType) )
		{
			String errText = errorString().append("Attempt to specify invalid real value. Passed in value is='" )
										.append( mantissa ).append( " / " )
										.append( magnitudeType ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.REAL;
		
		return this;
	}

	@Override
	public MapEntry keyReal(double key, int action, ComplexType value)
	{
		return keyReal(key, OmmReal.MagnitudeType.EXPONENT_0, action, value, null);
	}
	
	@Override
	public MapEntry keyReal(double key, int action, ComplexType value, ByteBuffer permissionData)
	{
		return keyReal(key, OmmReal.MagnitudeType.EXPONENT_0, action, value, permissionData);
	}
	
	@Override
	public MapEntry keyReal(double key, int magnitudeType, int action, ComplexType value)
	{
		return keyReal(key, magnitudeType, action, value, null);
	}

	@Override
	public MapEntry keyReal(double key, int magnitudeType, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createReal();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Real)_keyData).value(key, magnitudeType) )
		{
			String errText = errorString().append("Attempt to specify invalid real value. Passed in value is='" )
										.append( key ).append( " / " )
										.append( magnitudeType ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.REAL;
						
		return this;
	}

	@Override
	public MapEntry keyFloat(float key, int action, ComplexType value)
	{
		return keyFloat(key, action, value, null);
	}

	@Override
	public MapEntry keyFloat(float key, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createFloat();
		((com.thomsonreuters.upa.codec.Float) _keyData).value(key);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.FLOAT;
			
		return this;
	}

	@Override
	public MapEntry keyDouble(double key, int action, ComplexType value)
	{
		return keyDouble(key, action, value, null);
	}

	@Override
	public MapEntry keyDouble(double key, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createDouble();
		((com.thomsonreuters.upa.codec.Double) _keyData).value(key);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.DOUBLE;
						
		return this;
	}

	@Override
	public MapEntry keyDate(int year, int month, int day, int action, ComplexType value)
	{
		return keyDate(year, month, day, action, value, null);
	}

	@Override
	public MapEntry keyDate(int year, int month, int day, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData =  dateValue(year, month, day);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.DATE;
			
		return this;
	}

	@Override
	public MapEntry keyTime(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond,
			int action, ComplexType value)
	{
		return keyTime(hour, minute, second, millisecond, microsecond, nanosecond, action, value, null);
	}

	@Override
	public MapEntry keyTime(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond,
			int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = timeValue(hour, minute, second, millisecond, microsecond, nanosecond);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.TIME;
				
		return this;
	}

	@Override
	public MapEntry keyDateTime(int year, int month, int day, int hour, int minute, int second, int millisecond,
			int microsecond, int nanosecond, int action, ComplexType value)
	{
		return keyDateTime(year, month, day, hour, minute, second, millisecond, microsecond, nanosecond, action, value, null);
	}

	@Override
	public MapEntry keyDateTime(int year, int month, int day, int hour, int minute, int second, int millisecond,
			int microsecond, int nanosecond, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = dateTimeValue(year, month, day, hour, minute, second, millisecond, microsecond, nanosecond);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.DATETIME;
				
		return this;
	}

	@Override
	public MapEntry keyQos(int timeliness, int rate, int action, ComplexType value)
	{
		return keyQos(timeliness, rate, action, value, null);
	}

	@Override
	public MapEntry keyQos(int timeliness, int rate, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createQos();
		Utilities.toRsslQos(rate, timeliness, (com.thomsonreuters.upa.codec.Qos) _keyData);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.QOS;
		
		return this;
	}

	@Override
	public MapEntry keyState(int streamState, int dataState, int statusCode, String statusText, int action,
			ComplexType value)
	{
		return keyState(streamState, dataState, statusCode, statusText, action, value, null);
	}

	@Override
	public MapEntry keyState(int streamState, int dataState, int statusCode, String statusText, int action,
			ComplexType value, ByteBuffer permissionData)
	{
		Buffer bufferText = CodecFactory.createBuffer();
		_keyData = CodecFactory.createState();
		
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_keyData).streamState(streamState) ||
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_keyData).dataState(dataState) ||
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_keyData).code(statusCode) || statusText == null ||
				CodecReturnCodes.SUCCESS != bufferText.data(statusText) ||  CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_keyData).text(bufferText) )
		{
			String errText = errorString().append("Attempt to specify invalid state. Passed in value is='" )
					.append( streamState ).append( " / " )
					.append( dataState ).append( " / " )
					.append( statusCode ).append( "/ " )
					.append( statusText ).append( "." ).toString();
				throw ommIUExcept().message(errText);
		}

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.STATE;
		
		return this;
	}

	@Override
	public MapEntry keyEnum(int key, int action, ComplexType value)
	{
		return keyEnum(key, action, value, null);
	}

	@Override
	public MapEntry keyEnum(int key, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createEnum();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Enum)_keyData).value(key) )
		{
			String errText = errorString().append("Attempt to specify invalid enum. Passed in key is='" )
					.append( key ).append( "." ).toString();
				throw ommIUExcept().message(errText);
		}

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.ENUM;
			
		return this;
	}

	@Override
	public MapEntry keyBuffer(ByteBuffer key, int action, ComplexType value)
	{
		return keyBuffer(key, action, value, null);
	}

	@Override
	public MapEntry keyBuffer(ByteBuffer key, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createBuffer();
		Utilities.copy(key, (Buffer) _keyData);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.BUFFER;
			
		return this;
	}

	@Override
	public MapEntry keyAscii(String key, int action, ComplexType value)
	{
		return keyAscii(key, action, value, null);
	}

	@Override
	public MapEntry keyAscii(String key, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createBuffer();
		if (CodecReturnCodes.SUCCESS != ((Buffer)_keyData).data(key) )
		{
			String errText = errorString().append("Attempt to specify invalid string. Passed in key is='" )
					.append( key ).append( "." ).toString();
				throw ommIUExcept().message(errText);
		}

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING;
				
		return this;
	}

	@Override
	public MapEntry keyUtf8(ByteBuffer key, int action, ComplexType value)
	{
		return keyUtf8(key, action, value, null);
	}

	@Override
	public MapEntry keyUtf8(ByteBuffer key, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createBuffer();
		Utilities.copy(key, (Buffer) _keyData);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING;
	
		return this;
	}

	@Override
	public MapEntry keyRmtes(ByteBuffer key, int action, ComplexType value)
	{
		return keyRmtes(key, action, value, null);
	}

	@Override
	public MapEntry keyRmtes(ByteBuffer key, int action, ComplexType value, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createBuffer();
		Utilities.copy(key, (Buffer) _keyData);

		entryValue(action, value, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING;
	
		return this;
	}

	DataImpl decodedKey()
	{
		return (DataImpl)_keyDataDecoded.data();
	}

	private void entryValue(int action, ComplexType value, ByteBuffer permissionData)
	{
		if (action < 0 || action > 15)
			throw ommOORExcept().message("action is out of range [0 - 15].");
		if (value == null)
			throw ommIUExcept().message("Passed in value is null");

		_rsslMapEntry.action(action);
		_entryDataType = Utilities.toRsslDataType(value.dataType());	
		
		Utilities.copy(((DataImpl) value).encodedData(), _rsslMapEntry.encodedData());

		if (permissionData != null)
		{
			Utilities.copy(permissionData, _rsslMapEntry.permData());
			_rsslMapEntry.applyHasPermData();
		}
	}
	
	private void entryValue(int action, ByteBuffer permissionData)
	{
		if (action < 0 || action > 15)
			throw ommOORExcept().message("action is out of range [0 - 15].");

		_rsslMapEntry.action(action);
		_entryDataType = com.thomsonreuters.upa.codec.DataTypes.NO_DATA;	
		
		if (permissionData != null)
		{
			Utilities.copy(permissionData, _rsslMapEntry.permData());
			_rsslMapEntry.applyHasPermData();
		}
	}
	
	MapEntryImpl entryValue(DataImpl mapEntryKey, DataImpl load)
	{
		_load = load;
		_keyDataDecoded.data(mapEntryKey);
		
		return this;
	}

	@Override
	public MapEntry keyInt(long key, int action)
	{
		keyInt(key,action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyInt(long key, int action, ByteBuffer permissionData)
	{	
		_keyData = CodecFactory.createInt();
		((com.thomsonreuters.upa.codec.Int) _keyData).value(key);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.INT;
		
		return this;
	}

	@Override
	public MapEntry keyUInt(long key, int action)
	{
		keyUInt(key,action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyUInt(long key, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt) _keyData).value(key);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.UINT;
						
		return this;
	}

	@Override
	public MapEntry keyUInt(BigInteger key, int action)
	{
		keyUInt(key, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyUInt(BigInteger key, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createUInt();
		((com.thomsonreuters.upa.codec.UInt) _keyData).value(key);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.UINT;
		
		return this;
	}

	@Override
	public MapEntry keyReal(long mantissa, int magnitudeType, int action)
	{
		keyReal(mantissa, magnitudeType, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyReal(long mantissa, int magnitudeType, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createReal();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Real)_keyData).value(mantissa, magnitudeType) )
		{
			String errText = errorString().append("Attempt to specify invalid real value. Passed in value is='" )
										.append( mantissa ).append( " / " )
										.append( magnitudeType ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.REAL;
		
		return this;
	}

	@Override
	public MapEntry keyReal(double key, int action)
	{
		keyReal(key, OmmReal.MagnitudeType.EXPONENT_0, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyReal(double key, int action, ByteBuffer permissionData) {
		
		keyReal(key, OmmReal.MagnitudeType.EXPONENT_0, action, permissionData);
		return this;
	}

	@Override
	public MapEntry keyReal(double key, int magnitudeType, int action) {
		
		keyReal(key, magnitudeType, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyReal(double key, int magnitudeType, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createReal();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Real)_keyData).value(key, magnitudeType) )
		{
			String errText = errorString().append("Attempt to specify invalid real value. Passed in value is='" )
										.append( key ).append( " / " )
										.append( magnitudeType ).append( "'." ).toString();
			throw ommIUExcept().message(errText);
		}

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.REAL;
						
		return this;
	}

	@Override
	public MapEntry keyFloat(float key, int action)
	{
		keyFloat(key, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyFloat(float key, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createFloat();
		((com.thomsonreuters.upa.codec.Float) _keyData).value(key);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.FLOAT;
			
		return this;
	}

	@Override
	public MapEntry keyDouble(double key, int action)
	{
		keyDouble(key, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyDouble(double key, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createDouble();
		((com.thomsonreuters.upa.codec.Double) _keyData).value(key);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.DOUBLE;
						
		return this;
	}

	@Override
	public MapEntry keyDate(int year, int month, int day, int action)
	{
		keyDate(year,month, day, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyDate(int year, int month, int day, int action, ByteBuffer permissionData)
	{
		_keyData =  dateValue(year, month, day);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.DATE;
			
		return this;
	}

	@Override
	public MapEntry keyTime(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond,
			int action)
	{
		keyTime(hour, minute, second, millisecond, microsecond, nanosecond, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyTime(int hour, int minute, int second, int millisecond, int microsecond, int nanosecond,
			int action, ByteBuffer permissionData)
	{
		_keyData = timeValue(hour, minute, second, millisecond, microsecond, nanosecond);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.TIME;
				
		return this;
	}

	@Override
	public MapEntry keyDateTime(int year, int month, int day, int hour, int minute, int second, int millisecond,
			int microsecond, int nanosecond, int action)
	{
		keyDateTime(year, month, day, hour, minute, second, millisecond, microsecond, nanosecond, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyDateTime(int year, int month, int day, int hour, int minute, int second, int millisecond,
			int microsecond, int nanosecond, int action, ByteBuffer permissionData)
	{
		_keyData = dateTimeValue(year, month, day, hour, minute, second, millisecond, microsecond, nanosecond);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.DATETIME;
				
		return this;
	}

	@Override
	public MapEntry keyQos(int timeliness, int rate, int action)
	{
		keyQos(timeliness, rate, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyQos(int timeliness, int rate, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createQos();
		Utilities.toRsslQos(rate, timeliness, (com.thomsonreuters.upa.codec.Qos) _keyData);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.QOS;
		
		return this;
	}

	@Override
	public MapEntry keyState(int streamState, int dataState, int statusCode, String statusText, int action)
	{
		keyState(streamState, dataState, statusCode, statusText, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyState(int streamState, int dataState, int statusCode, String statusText, int action,
			ByteBuffer permissionData)
	{
		Buffer bufferText = CodecFactory.createBuffer();
		_keyData = CodecFactory.createState();
		
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_keyData).streamState(streamState) ||
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_keyData).dataState(dataState) ||
				CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_keyData).code(statusCode) || statusText == null ||
				CodecReturnCodes.SUCCESS != bufferText.data(statusText) ||  CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.State)_keyData).text(bufferText) )
		{
			String errText = errorString().append("Attempt to specify invalid state. Passed in value is='" )
					.append( streamState ).append( " / " )
					.append( dataState ).append( " / " )
					.append( statusCode ).append( "/ " )
					.append( statusText ).append( "." ).toString();
				throw ommIUExcept().message(errText);
		}
		
		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.STATE;
		
		return this;
	}

	@Override
	public MapEntry keyEnum(int key, int action)
	{
		keyEnum(key, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyEnum(int key, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createEnum();
		if (CodecReturnCodes.SUCCESS != ((com.thomsonreuters.upa.codec.Enum)_keyData).value(key) )
		{
			String errText = errorString().append("Attempt to specify invalid enum. Passed in key is='" )
					.append( key ).append( "." ).toString();
				throw ommIUExcept().message(errText);
		}

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.ENUM;
			
		return this;
	}

	@Override
	public MapEntry keyBuffer(ByteBuffer key, int action)
	{
		keyBuffer(key, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyBuffer(ByteBuffer key, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createBuffer();
		Utilities.copy(key, (Buffer) _keyData);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.BUFFER;
			
		return this;
	}

	@Override
	public MapEntry keyAscii(String key, int action)
	{
		keyAscii(key, action, (ByteBuffer)null);
		return this;
	}

	@Override
	public MapEntry keyAscii(String key, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createBuffer();
		if (CodecReturnCodes.SUCCESS != ((Buffer)_keyData).data(key) )
		{
			String errText = errorString().append("Attempt to specify invalid string. Passed in key is='" )
					.append( key ).append( "." ).toString();
				throw ommIUExcept().message(errText);
		}

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.ASCII_STRING;
				
		return this;
	}

	@Override
	public MapEntry keyUtf8(ByteBuffer key, int action)
	{
		return keyUtf8(key, action, (ByteBuffer)null);
	}

	@Override
	public MapEntry keyUtf8(ByteBuffer key, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createBuffer();
		Utilities.copy(key, (Buffer) _keyData);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.UTF8_STRING;
	
		return this;
	}

	@Override
	public MapEntry keyRmtes(ByteBuffer key, int action)
	{
		return keyRmtes(key, action, (ByteBuffer)null);
	}

	@Override
	public MapEntry keyRmtes(ByteBuffer key, int action, ByteBuffer permissionData)
	{
		_keyData = CodecFactory.createBuffer();
		Utilities.copy(key, (Buffer) _keyData);

		entryValue(action, permissionData);

		_keyDataType = com.thomsonreuters.upa.codec.DataTypes.RMTES_STRING;
	
		return this;
	}
}
